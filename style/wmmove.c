#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>

// #define QTC_GE_IS_TOOL_BAR(object) ((object) && objectIsA((GObject*)(object), "GtkToolbar"))
// #define QTC_GE_IS_STATUS_BAR(object) ((object) && objectIsA((GObject*)(object), "GtkStatusbar"))
// #define QTC_GE_IS_LABEL(object) ((object) && objectIsA((GObject*)(object), "GtkLabel"))

static void qtcTriggerWMMove(GtkWidget *w, int x, int y)
{
    Atom      netMoveResize = XInternAtom(gdk_x11_get_default_xdisplay(), "_NET_WM_MOVERESIZE", False);
    XEvent    xev;
    GtkWindow *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(w));

    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = netMoveResize;
    xev.xclient.display = gdk_x11_get_default_xdisplay();
    xev.xclient.window = GDK_WINDOW_XID(GTK_WIDGET(topLevel)->window);
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = x;
    xev.xclient.data.l[1] = y;
    xev.xclient.data.l[2] = 8; // NET::Move
    xev.xclient.data.l[3] = Button1;
    xev.xclient.data.l[4] = 0;
    XUngrabPointer(gdk_x11_get_default_xdisplay(), CurrentTime);
    XSendEvent(gdk_x11_get_default_xdisplay(), gdk_x11_get_default_root_xwindow(), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

// static gboolean isOnStatusBar(GtkWidget *widget, int level);

static gboolean withinWidget(GtkWidget *widget, GdkEventButton *event, int adjust)
{
    GtkAllocation alloc;

    alloc.x-=adjust;
    alloc.y-=adjust;
    alloc.width+=adjust;
    alloc.height+=adjust;

    gtk_widget_get_allocation(widget, &alloc);

    return alloc.x<=event->x && alloc.y<=event->y &&
           (alloc.x+alloc.width)>event->x &&(alloc.y+alloc.height)>event->y;
}

static gboolean pointerOnItem(GtkWidget *widget, GdkEventButton *event)
{
    bool onItem=FALSE;
    if(QTC_GE_IS_MENU_SHELL(widget))
    {
        GdkModifierType pointer_mask;

        if(QTC_GE_IS_CONTAINER(widget))
        {
            GList *children = gtk_container_get_children(GTK_CONTAINER(widget)),
                  *child;
              
            for(child = g_list_first(children); child /*&& !onItem*/; child = g_list_next(child))
            {
                if((child->data) && QTC_GE_IS_WIDGET(child->data) &&
                    withinWidget(GTK_WIDGET(child->data), event,
#ifdef QTC_EXTEND_MENUBAR_ITEM_HACK
                                 constMenuAdjust
#else
                                 0
#endif
                                 ) )
                    onItem=TRUE;
            }
         
            if(children)   
                g_list_free(children);
        }
    }
    
    return onItem;
}

static gboolean qtcIsWindowDragWidget(GtkWidget *widget, GdkEventButton *event)
{
    return opts.windowDrag &&
           (!event || withinWidget(widget, event, 0)) &&
           ((QTC_GE_IS_MENU_BAR(widget) && (!event || !pointerOnItem(widget, event)))
//             || QTC_GE_IS_TOOL_BAR(widget)
//             || QTC_GE_IS_STATUS_BAR(widget)
//             || (QTC_GE_IS_LABEL(widget) && isOnStatusBar(widget, 0))
            );
}

static GtkWidget *dragWidget=NULL;
// static gboolean  dragWidgetHadMouseTracking=FALSE;

static gboolean qtcWMMoveButtonPress(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    if (qtcIsWindowDragWidget(widget, event))
    {
        dragWidget=widget;
//         dragWidgetHadMouseTracking=gdk_window_get_events(gtk_widget_get_window(widget))&GDK_BUTTON1_MOTION_MASK;
//         gdk_window_set_events(gtk_widget_get_window(widget), gdk_window_get_events(gtk_widget_get_window(widget))|GDK_BUTTON1_MOTION_MASK);
        return TRUE;
    }

    return FALSE;
}

static gboolean qtcWMMoveButtonRelease(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    if (widget==dragWidget)
    {
//         if(!dragWidgetHadMouseTracking);
//             gdk_window_set_events(gtk_widget_get_window(widget), gdk_window_get_events(gtk_widget_get_window(widget))-GDK_BUTTON1_MOTION_MASK);
        dragWidget=NULL;
        return TRUE;
    }

    return FALSE;
}

static void qtcWMMoveCleanup(GtkWidget *widget)
{
    if (qtcIsWindowDragWidget(widget, NULL))
    {
        if(widget==dragWidget)
            dragWidget=NULL;
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_MOTION_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_LEAVE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_STYLE_SET_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_BUTTON_PRESS_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_BUTTON_RELEASE_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_HACK_SET");
    }
}

static gboolean qtcWMMoveStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcWMMoveCleanup(widget);
    return FALSE;
}
 
static gboolean qtcWMMoveDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcWMMoveCleanup(widget);
    return FALSE;
}

static gboolean qtcWMMoveMotion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    if (dragWidget==widget /*&& qtcIsWindowDragWidget(widget, event)*/)
    {
        qtcTriggerWMMove(widget, event->x_root, event->y_root);
        gtk_grab_remove(widget);
        gdk_pointer_ungrab(CurrentTime);
        dragWidget=NULL;
        return TRUE;
    }

    return FALSE;
}

static gboolean qtcWMMoveLeave(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    if (dragWidget==widget)
    {
        gtk_grab_remove(widget);
        gdk_pointer_ungrab(CurrentTime);
        dragWidget=NULL;
        return TRUE;
    }

    return FALSE;
}

static void qtcWMMoveSetup(GtkWidget *widget)
{
    if (qtcIsWindowDragWidget(widget, NULL) && !g_object_get_data(G_OBJECT(widget), "QTC_WM_MOVE_HACK_SET"))
    {
        gdk_window_set_events(gtk_widget_get_window(widget), gdk_window_get_events(gtk_widget_get_window(widget))|GDK_BUTTON1_MOTION_MASK);
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_HACK_SET", (gpointer)1);
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_MOTION_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "motion-notify-event",
                                                     (GtkSignalFunc)qtcWMMoveMotion, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_LEAVE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "leave-notify-event",
                                                     (GtkSignalFunc)qtcWMMoveLeave, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event",
                                                     (GtkSignalFunc)qtcWMMoveDestroy, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "style-set",
                                                     (GtkSignalFunc)qtcWMMoveStyleSet, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_BUTTON_PRESS_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "button-press-event",
                                                     G_CALLBACK(qtcWMMoveButtonPress), widget));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_BUTTON_RELEASE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "button-release-event",
                                                     G_CALLBACK(qtcWMMoveButtonRelease), widget));
    }  
}
