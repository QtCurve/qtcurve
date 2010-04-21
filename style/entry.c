#define GE_IS_ENTRY(object) ((object) && objectIsA((GObject*)(object), "GtkEntry"))

static GtkWidget *lastMoEntry=NULL;

static void qtcEntryCleanup(GtkWidget *widget)
{
    if(lastMoEntry==widget)
        lastMoEntry=NULL;
    if (GE_IS_ENTRY(widget))
    {
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_ENTRY_ENTER_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_ENTRY_LEAVE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_ENTRY_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_ENTRY_UNREALIZE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_ENTRY_STYLE_SET_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_ENTRY_HACK_SET");
    }
}

static gboolean qtcEntryStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcEntryCleanup(widget);
    return FALSE;
}

static gboolean qtcEntryDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcEntryCleanup(widget);
    return FALSE;
}

static gboolean qtcEntryEnter(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    if (GE_IS_ENTRY(widget))
    {
        lastMoEntry=widget;
        gtk_widget_queue_draw(widget);
    }
 
    return FALSE;
}

static gboolean qtcEntryLeave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    if (GE_IS_ENTRY(widget))
    {
        lastMoEntry=NULL;
        gtk_widget_queue_draw(widget);
    }
 
    return FALSE;
}

static void qtcEntrySetup(GtkWidget *widget)
{
    if (GE_IS_ENTRY(widget) && !g_object_get_data(G_OBJECT(widget), "QTC_ENTRY_HACK_SET"))
    {
        g_object_set_data(G_OBJECT(widget), "QTC_ENTRY_HACK_SET", (gpointer)1);
        g_object_set_data(G_OBJECT(widget), "QTC_ENTRY_ENTER_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "enter-notify-event",
                                                     (GtkSignalFunc)qtcEntryEnter, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_ENTRY_LEAVE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "leave-notify-event",
                                                     (GtkSignalFunc)qtcEntryLeave, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_ENTRY_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event",
                                                     (GtkSignalFunc)qtcEntryDestroy, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_ENTRY_UNREALIZE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "unrealize",
                                                     (GtkSignalFunc)qtcEntryDestroy, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_ENTRY_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "style-set",
                                                     (GtkSignalFunc)qtcEntryStyleSet, NULL));
    }  
}
