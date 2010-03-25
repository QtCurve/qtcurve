#include <gdk/gdkkeysyms.h>

static void qtcWindowCleanup(GtkWidget *widget)
{
    if (widget)
    {
        if(QTC_CUSTOM_BGND)
            g_signal_handler_disconnect(G_OBJECT(widget),
                                        (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_SIZE_REQUEST_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_STYLE_SET_ID"));
        if(opts.menubarHiding || opts.statusbarHiding)
            g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_KEY_RELEASE_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_HACK_SET");
    }
}

static gboolean qtcWindowStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcWindowCleanup(widget);
    return FALSE;
}

static gboolean qtcWindowDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcWindowCleanup(widget);
    return FALSE;
}

static gboolean qtcWindowSizeRequest(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    // Need to invalidate the whole of the window on a resize, as gradient needs to be redone.
    if(widget && QTC_CUSTOM_BGND)
    {
        GdkRectangle rect;

        if(IS_FLAT(opts.bgndAppearance))
            rect.x=0, rect.y=0, rect.width=widget->allocation.width, rect.height=opts.bgndImage.pix ? opts.bgndImage.height : QTC_RINGS_HEIGHT(opts.bgndImage.type);
        else
            rect.x=0, rect.y=0, rect.width=widget->allocation.width, rect.height=widget->allocation.height;
        gdk_window_invalidate_rect(widget->window, &rect, FALSE);
    }
    return FALSE;
}

static GtkWidget * qtcWindowGetMenuBar(GtkWidget *parent, int level)
{
    if(level<3 && GTK_IS_CONTAINER(parent))
    {
        GList *child=gtk_container_get_children(GTK_CONTAINER(parent));

        for(; child; child=child->next)
        {
            GtkBoxChild *boxChild=(GtkBoxChild *)child->data;

            if(GTK_IS_MENU_BAR(boxChild))
                return GTK_WIDGET(boxChild);
            else if(GTK_IS_CONTAINER(boxChild))
            {
                GtkWidget *w=qtcWindowGetMenuBar(GTK_WIDGET(boxChild), level+1);
                if(w)
                    return w;
            }
        }
    }

    return NULL;
}

static GtkWidget * qtcWindowGetStatusBar(GtkWidget *parent, int level)
{
    if(level<3 && GTK_IS_CONTAINER(parent))
    {
        GList *child=gtk_container_get_children(GTK_CONTAINER(parent));

        for(; child; child=child->next)
        {
            GtkBoxChild *boxChild=(GtkBoxChild *)child->data;

            if(GTK_IS_STATUSBAR(boxChild))
                return GTK_WIDGET(boxChild);
            else if(GTK_IS_CONTAINER(boxChild))
            {
                GtkWidget *w=qtcWindowGetStatusBar(GTK_WIDGET(boxChild), level+1);
                if(w)
                    return w;
            }
        }
    }

    return NULL;
}

static gboolean qtcWindowKeyRelease(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    if(GDK_CONTROL_MASK&event->state && GDK_MOD1_MASK&event->state && !event->is_modifier &&
       0==(event->state&0xFF00)) // Ensure only ctrl/alt/shift/capsLock are pressed...
    {
        gboolean toggled=FALSE;
        if(opts.menubarHiding && (GDK_m==event->keyval || GDK_M==event->keyval))
        {
            GtkWidget *menuBar=qtcWindowGetMenuBar(widget, 0);

            if(menuBar)
            {
                toggled=TRUE;
                qtcSetMenuBarHidden(qtSettings.appName, GTK_WIDGET_VISIBLE(menuBar));
                if(GTK_WIDGET_VISIBLE(menuBar))
                    gtk_widget_hide(menuBar);
                else
                    gtk_widget_show(menuBar);
            }
        }

        if(opts.statusbarHiding && (GDK_s==event->keyval || GDK_S==event->keyval))
        {
            GtkWidget *statusBar=qtcWindowGetStatusBar(widget, 0);

            if(statusBar)
            {
                toggled=TRUE;
                qtcSetStatusBarHidden(qtSettings.appName, GTK_WIDGET_VISIBLE(statusBar));
                if(GTK_WIDGET_VISIBLE(statusBar))
                    gtk_widget_hide(statusBar);
                else
                    gtk_widget_show(statusBar);
            }
        }

        if(toggled)
            gtk_widget_queue_draw(widget);
    }
    return FALSE;
}

static void qtcWindowSetup(GtkWidget *widget)
{
    if (widget && !g_object_get_data(G_OBJECT(widget), "QTC_WINDOW_HACK_SET"))
    {
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_HACK_SET", (gpointer)1);
        if(QTC_CUSTOM_BGND)
            g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_SIZE_REQUEST_ID",
                              (gpointer)g_signal_connect(G_OBJECT(widget), "size-request",
                                                        (GtkSignalFunc)qtcWindowSizeRequest, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event",
                                                     (GtkSignalFunc)qtcWindowDestroy, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "style-set",
                                                     (GtkSignalFunc)qtcWindowStyleSet, NULL));
        if(opts.menubarHiding || opts.statusbarHiding)
            g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_KEY_RELEASE_ID",
                              (gpointer)g_signal_connect(G_OBJECT(widget), "key-release-event",
                                                         (GtkSignalFunc)qtcWindowKeyRelease, NULL));
    }  
}
