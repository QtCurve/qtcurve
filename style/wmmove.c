// #define GE_IS_TOOL_BAR(object) ((object) && objectIsA((GObject*)(object), "GtkToolbar"))
// #define GE_IS_STATUS_BAR(object) ((object) && objectIsA((GObject*)(object), "GtkStatusbar"))
// #define GE_IS_LABEL(object) ((object) && objectIsA((GObject*)(object), "GtkLabel"))

static void qtcTriggerWMMove(GtkWidget *w, int x, int y)
{
    XEvent     xev;
    GtkWindow  *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(w));
    GdkWindow  *window=gtk_widget_get_window(GTK_WIDGET(topLevel));
    GdkDisplay *display=gtk_widget_get_display(GTK_WIDGET(topLevel));
    GdkWindow  *root=gdk_screen_get_root_window(gtk_window_get_screen(topLevel));

    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = gdk_x11_get_xatom_by_name_for_display(display, "_NET_WM_MOVERESIZE");
    xev.xclient.display = GDK_DISPLAY_XDISPLAY(display);
    xev.xclient.window = GDK_WINDOW_XID(window);
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = x;
    xev.xclient.data.l[1] = y;
    xev.xclient.data.l[2] = 8; // NET::Move
    xev.xclient.data.l[3] = Button1;
    xev.xclient.data.l[4] = 0;
    XUngrabPointer(GDK_DISPLAY_XDISPLAY(display), CurrentTime);

    XSendEvent(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XID(root), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

// static gboolean isOnStatusBar(GtkWidget *widget, int level);

static gboolean withinWidget(GtkWidget *widget, GdkEventButton *event, int adjust)
{
    GtkAllocation alloc;
    int           nx=0,
                  ny=0;

    alloc.x-=adjust;
    alloc.y-=adjust;
    alloc.width+=adjust;
    alloc.height+=adjust;

    // Need to get absolute co-ordinates...
    gtk_widget_get_allocation(widget, &alloc);
    gdk_window_get_origin(widget->window, &nx, &ny);
    alloc.x+=nx;
    alloc.y+=ny;

    return alloc.x<=event->x_root && alloc.y<=event->y_root &&
           (alloc.x+alloc.width)>event->x_root &&(alloc.y+alloc.height)>event->y_root;
}

static gboolean useEvent(GtkWidget *widget, GdkEventButton *event)
{
    bool use=TRUE;
    if(GE_IS_MENU_SHELL(widget))
    {
        GdkModifierType pointer_mask;

        if(GE_IS_CONTAINER(widget))
        {
            GList *children = gtk_container_get_children(GTK_CONTAINER(widget)),
                  *child;
              
            for(child = g_list_first(children); child /*&& use*/; child = g_list_next(child))
            {
                // Can only use this 'press' event if
                //  1. There is no active menu being displayed - as the 'press' will be to cancel the menu
                //  2. The click is not where a menu item is
                if((child->data) && GE_IS_WIDGET(child->data) &&
                    ((GTK_STATE_NORMAL!=GTK_WIDGET_STATE(GTK_WIDGET(child->data)) &&
                      GTK_STATE_INSENSITIVE!=GTK_WIDGET_STATE(GTK_WIDGET(child->data))) ||
                     withinWidget(GTK_WIDGET(child->data), event,
#ifdef EXTEND_MENUBAR_ITEM_HACK
                                 constMenuAdjust
#else
                                 0
#endif
                                 ) ) )
                    use=FALSE;
            }
         
            if(children)   
                g_list_free(children);
        }
    }
    
    return use;
}

static gboolean qtcIsWindowDragWidget(GtkWidget *widget, GdkEventButton *event)
{
    return opts.windowDrag &&
           (!event || withinWidget(widget, event, 0)) &&
           ((GE_IS_MENU_BAR(widget) && (!event || useEvent(widget, event)))
//             || GE_IS_TOOL_BAR(widget)
//             || GE_IS_STATUS_BAR(widget)
//             || (GE_IS_LABEL(widget) && isOnStatusBar(widget, 0))
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
