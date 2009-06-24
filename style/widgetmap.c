static GHashTable *widgetMapHashTable[2] = {NULL, NULL};

#define QTC_MAP_ID_X(ID_STR) "QTC_WIDGET_MAP_HACK_HACK_SET"ID_STR
#define QTC_MAP_ID(ID) (ID ? QTC_MAP_ID_X("1") : QTC_MAP_ID_X("0"))

static GtkWidget * lookupWidgetMapHash(void *hash, void *value, int map)
{
    GtkWidget *rv=NULL;

    if(!widgetMapHashTable[map])
        widgetMapHashTable[map]=g_hash_table_new(g_direct_hash, g_direct_equal);

    rv=(GtkWidget *)g_hash_table_lookup(widgetMapHashTable[map], hash);

    if(!rv && value)
    {
        g_hash_table_insert(widgetMapHashTable[map], hash, value);
        rv=value;
    }

    return rv;
}

static void removeFromWidgetMapHash(void *hash)
{
    int i;

    for(i=0; i<2; ++i)
        if(widgetMapHashTable[i])
            g_hash_table_remove(widgetMapHashTable[i], hash);
}

static GtkWidget * getMappedWidget(GtkWidget *widget, int map)
{
    return widget && g_object_get_data(G_OBJECT(widget), QTC_MAP_ID(map))
            ? lookupWidgetMapHash(widget, NULL, map) : NULL;
}

static void qtcWidgetMapCleanup(GtkWidget *widget)
{
    if(g_object_get_data(G_OBJECT(widget), QTC_MAP_ID(0)) || g_object_get_data(G_OBJECT(widget), QTC_MAP_ID(1)))
    {
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WIDGET_MAP_HACK_DELETE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WIDGET_MAP_HACK_STYLE_SET_ID"));
        g_object_steal_data(G_OBJECT(widget), QTC_MAP_ID(0));
        g_object_steal_data(G_OBJECT(widget), QTC_MAP_ID(1));
        removeFromWidgetMapHash(widget);
    }
}

static gboolean qtcWidgetMapStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcWidgetMapCleanup(widget);
    return FALSE;
}
 
static gboolean qtcWidgetMapDelete(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcWidgetMapCleanup(widget);
    return FALSE;
}

static void qtcWidgetMapSetup(GtkWidget *from, GtkWidget *to, int map)
{
    if (from && to && !g_object_get_data(G_OBJECT(from), QTC_MAP_ID(map)))
    {
        g_object_set_data(G_OBJECT(from), QTC_MAP_ID(map), (gpointer)1);
        g_object_set_data(G_OBJECT(from), "QTC_WIDGET_MAP_HACK_DELETE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(from), "unrealize",
                                                     (GtkSignalFunc)qtcWidgetMapDelete, NULL));
        g_object_set_data(G_OBJECT(from), "QTC_WIDGET_MAP_HACK_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(from), "style-set",
                                                     (GtkSignalFunc)qtcWidgetMapStyleSet, NULL));
        lookupWidgetMapHash(from, to, map);
    }  
}
