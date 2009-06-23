static void qtcWindowCleanup(GtkWidget *widget)
{
    if (widget)
    {
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_SIZE_REQUEST_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_STYLE_SET_ID"));
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
    if(widget && !IS_FLAT(opts.bgndAppearance))
    {
        GdkRectangle rect;
        rect.x=0, rect.y=0, rect.width=widget->allocation.width, rect.height=widget->allocation.height;
        gdk_window_invalidate_rect(widget->window, &rect, FALSE);
    }
    return FALSE;
}

static void qtcWindowSetup(GtkWidget *widget)
{
    if (widget && !g_object_get_data(G_OBJECT(widget), "QTC_WINDOW_HACK_SET"))
    {
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_HACK_SET", (gpointer)1);
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_SIZE_REQUEST_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "size-request",
                                                     (GtkSignalFunc)qtcWindowSizeRequest, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event",
                                                     (GtkSignalFunc)qtcWindowDestroy, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "style-set",
                                                     (GtkSignalFunc)qtcWindowStyleSet, NULL));
    }  
}
