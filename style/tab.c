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

static void qtcTabCleanup(GtkWidget *widget)
{
    if (widget)
    { 
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_MOTION_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_LEAVE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_DESTROY_ID"));
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
    qtcTabCleanup (widget);
    return FALSE;
}

static gboolean qtcTabMotion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    if (widget)
    {
        static int last_x=-100, last_y=-100;

        if(abs(last_x-event->x_root)>4 || abs(last_y-event->y_root)>4)
        {
            last_x=event->x_root;
            last_y=event->y_root;

            GtkNotebook *notebook=GTK_NOTEBOOK(widget);

            if(notebook)
            {
                /* TODO! check if mouse is over tab portion! */
                /* Find tab that mouse is currently over...*/
                QtCTab *prevTab=lookupTabHash(widget, TRUE),
                       currentTab;
                int    numChildren=g_list_length(notebook->children),
                       i,
                       nx, ny;

                currentTab.id=currentTab.rect.x=currentTab.rect.y=
                currentTab.rect.width=currentTab.rect.height=-1;
                gdk_window_get_origin(GTK_WIDGET(notebook)->window, &nx, &ny);
                for (i = 0; i < numChildren; i++ )
                {
                    GtkWidget *page=gtk_notebook_get_nth_page(notebook, i),
                              *tabLabel=gtk_notebook_get_tab_label(notebook, page);
                    int       tx=(tabLabel->allocation.x+nx)-2,
                              ty=(tabLabel->allocation.y+ny)-2,
                              tw=(tabLabel->allocation.width)+4,
                              th=(tabLabel->allocation.height)+4;

                    if(tx<=event->x_root && ty<=event->y_root &&
                        (tx+tw)>event->x_root && (ty+th)>event->y_root)
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
                    if(currentTab.rect.x<0)
                    {
                        prevTab->id=currentTab.id;
                        prevTab->rect=currentTab.rect;
                        gtk_widget_queue_draw(widget);
                    }
                    else
                    {
                        GdkRectangle area;

                        if(prevTab->rect.x<0)
                            area=currentTab.rect;
                        else
                            gdk_rectangle_union(&(prevTab->rect), &(currentTab.rect), &area);
                        prevTab->id=currentTab.id;
                        prevTab->rect=currentTab.rect;
                        area.x-=12;
                        area.y-=12;
                        area.width+=24;
                        area.height+=24;
                        gtk_widget_queue_draw_area(widget, area.x, area.y, area.width, area.height);
                    }
                }
            }
        }
    }
 
    return FALSE;
}

static gboolean qtcTabLeave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    if (widget)
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
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_MOTION_ID", (gpointer)g_signal_connect(G_OBJECT(widget), "motion-notify-event",
                                                                                            (GtkSignalFunc)qtcTabMotion,
                                                                                            NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_LEAVE_ID", (gpointer)g_signal_connect(G_OBJECT(widget), "leave-notify-event",
                                                                                           (GtkSignalFunc)qtcTabLeave,
                                                                                            NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_DESTROY_ID", (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event",
                                                                                             (GtkSignalFunc)qtcTabDestroy,
                                                                                              NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_STYLE_SET_ID", (gpointer)g_signal_connect(G_OBJECT(widget), "style-set",
                                                                                               (GtkSignalFunc)qtcTabStyleSet,
                                                                                               NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_HACK_SET", (gpointer)1);
    }  
}
