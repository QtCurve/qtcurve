#define GE_IS_NOTEBOOK(object) ((object) && objectIsA((GObject*)(object), "GtkNotebook"))

typedef struct
{
    int          id;
    GdkRectangle rect;
} QtCTab;

static GHashTable *tabHashTable = NULL;

static QtCTab * lookupTabHash(void *hash, gboolean create)
{
    QtCTab *rv=NULL;

    if(!tabHashTable)
        tabHashTable=g_hash_table_new(g_direct_hash, g_direct_equal);

    rv=(QtCTab *)g_hash_table_lookup(tabHashTable, hash);

    if(!rv && create)
    {
        rv=(QtCTab *)malloc(sizeof(QtCTab));
        rv->id=rv->rect.x=rv->rect.y=rv->rect.width=rv->rect.height=-1;
        g_hash_table_insert(tabHashTable, hash, rv);
        rv=g_hash_table_lookup(tabHashTable, hash);
    }

    return rv;
}

static void removeFromTabHash(void *hash)
{
    if(tabHashTable)
        g_hash_table_remove(tabHashTable, hash);
}

static gboolean qtcTabCurrentHoveredIndex(GtkWidget *widget)
{
    QtCTab *tab=GE_IS_NOTEBOOK(widget) ? lookupTabHash(widget, FALSE) : NULL;

    return tab ? tab->id : -1;
}

static void qtcTabCleanup(GtkWidget *widget)
{
    if (widget)
    { 
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_MOTION_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_LEAVE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_UNREALIZE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_STYLE_SET_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_TAB_HACK_SET");
        removeFromTabHash(widget);
    }
}

static gboolean qtcTabStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcTabCleanup(widget);
    return FALSE;
}

static gboolean qtcTabDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcTabCleanup(widget);
    return FALSE;
}

static gboolean qtcTabMotion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    if (GE_IS_NOTEBOOK(widget))
    {
        GtkNotebook *notebook=GTK_NOTEBOOK(widget);

        if(notebook)
        {
            /* TODO! check if mouse is over tab portion! */
            /* Find tab that mouse is currently over...*/
            GList  *children=gtk_container_get_children(GTK_CONTAINER(notebook));
            QtCTab *prevTab=lookupTabHash(widget, TRUE),
                   currentTab;
            int    numChildren=g_list_length(children),
                   i,
                   nx, ny;

            currentTab.id=currentTab.rect.x=currentTab.rect.y=
            currentTab.rect.width=currentTab.rect.height=-1;
            gdk_window_get_origin(qtcWidgetGetWindow(GTK_WIDGET(notebook)), &nx, &ny);
            for (i = 0; i < numChildren; i++ )
            {
                GtkWidget     *page=gtk_notebook_get_nth_page(notebook, i),
                              *tabLabel=gtk_notebook_get_tab_label(notebook, page);
                GtkAllocation alloc=qtcWidgetGetAllocation(tabLabel);
                int           tx=(alloc.x+nx)-4,
                              ty=(alloc.y+ny)-3,
                              tw=(alloc.width)+10,
                              th=(alloc.height)+7;

                if(tx<=event->x_root && ty<=event->y_root && (tx+tw)>event->x_root && (ty+th)>event->y_root)
                {
                    currentTab.rect.x=tx-nx;
                    currentTab.rect.y=ty-ny;
                    currentTab.rect.width=tw;
                    currentTab.rect.height=th;
                    currentTab.id=i;
                    break;
                }
            }

            if(currentTab.id!=prevTab->id)
            {
                prevTab->id=currentTab.id;
                prevTab->rect=currentTab.rect;
                gtk_widget_queue_draw(widget);
            }

            if(children)
                g_list_free(children);
        }
    }
 
    return FALSE;
}

static gboolean qtcTabLeave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    if (GE_IS_NOTEBOOK(widget))
    {
        QtCTab *prevTab=lookupTabHash(widget, FALSE);

        if(prevTab && prevTab->id>=0)
        {
            prevTab->id=prevTab->rect.x=prevTab->rect.y=
            prevTab->rect.width=prevTab->rect.height=-1;
            gtk_widget_queue_draw(widget);
        }
    }
 
    return FALSE;
}

static void qtcTabSetup(GtkWidget *widget)
{
    if (widget && !g_object_get_data(G_OBJECT(widget), "QTC_TAB_HACK_SET"))
    { 
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_MOTION_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(qtcTabMotion), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_LEAVE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(qtcTabLeave), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event", G_CALLBACK(qtcTabDestroy), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_UNREALIZE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "unrealize", G_CALLBACK(qtcTabDestroy), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "style-set", G_CALLBACK(qtcTabStyleSet), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_HACK_SET", (gpointer)1);
    }  
}
