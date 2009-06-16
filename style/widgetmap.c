static GHashTable *widgetMapHashTable = NULL;

static GtkWidget * lookupWidgetMapHash(void *hash, void *value)
{
    GtkWidget *rv=NULL;

    if(!widgetMapHashTable)
        widgetMapHashTable=g_hash_table_new(g_direct_hash, g_direct_equal);

    rv=(GtkWidget *)g_hash_table_lookup(widgetMapHashTable, hash);

    if(!rv && value)
    {
        g_hash_table_insert(widgetMapHashTable, hash, value);
        rv=value;
    }

    return rv;
}

static void removeFromWidgetMapHash(void *hash)
{
    if(widgetMapHashTable)
        g_hash_table_remove(widgetMapHashTable, hash);
}

static GtkWidget * getMappedWidget(GtkWidget *widget)
{
    return widget && g_object_get_data(G_OBJECT(widget), "QTC_WIDGET_MAP_HACK_HACK_SET")
            ? lookupWidgetMapHash(widget, NULL) : NULL;
}

static void qtcWidgetMapCleanup(GtkWidget *widget)
{
    if (QTC_GE_IS_TOGGLEBUTTON(widget))
    {
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WIDGET_MAP_HACK_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WIDGET_MAP_HACK_STYLE_SET_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_WIDGET_MAP_HACK_HACK_SET");
        if(widget->parent)
            removeFromWidgetMapHash(widget);
    }
}

static gboolean qtcWidgetMapStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcWidgetMapCleanup(widget);
    return FALSE;
}
 
static gboolean qtcWidgetMapDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcWidgetMapCleanup(widget);
    return FALSE;
}

static void qtcWidgetMapSetup(GtkWidget *from, GtkWidget *to)
{
    if (from && to && !g_object_get_data(G_OBJECT(from), "QTC_WIDGET_MAP_HACK_HACK_SET"))
    {
        g_object_set_data(G_OBJECT(from), "QTC_WIDGET_MAP_HACK_HACK_SET", (gpointer)1);
        g_object_set_data(G_OBJECT(from), "QTC_WIDGET_MAP_HACK_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(from), "destroy-event",
                                                     (GtkSignalFunc)qtcWidgetMapDestroy, NULL));
        g_object_set_data(G_OBJECT(from), "QTC_WIDGET_MAP_HACK_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(from), "style-set",
                                                     (GtkSignalFunc)qtcWidgetMapStyleSet, NULL));
        lookupWidgetMapHash(from, to);
    }  
}
