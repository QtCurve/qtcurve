#if GTK_CHECK_VERSION(2, 12, 0)

#define GE_IS_TREEVIEW(object) ((object) && objectIsA((GObject*)(object), "GtkTreeView"))

typedef struct
{
    GtkTreePath       *path;
    GtkTreeViewColumn *column;
    gboolean          fullWidth;
} QtCTreeView;

static GHashTable *qtcTreeViewTable=NULL;

static QtCTreeView * qtcTreeViewLookupHash(void *hash, gboolean create)
{
    QtCTreeView *rv=NULL;

    if(!qtcTreeViewTable)
        qtcTreeViewTable=g_hash_table_new(g_direct_hash, g_direct_equal);

    rv=(QtCTreeView *)g_hash_table_lookup(qtcTreeViewTable, hash);

    if(!rv && create)
    {
        rv=(QtCTreeView *)malloc(sizeof(QtCTreeView));
        rv->path=NULL;
        rv->column=NULL;
        rv->fullWidth=FALSE;
        g_hash_table_insert(qtcTreeViewTable, hash, rv);
        rv=g_hash_table_lookup(qtcTreeViewTable, hash);
    }

    return rv;
}

static void qtcTreeViewRemoveFromHash(void *hash)
{
    if(tabHashTable)
    {
        QtCTreeView *tv=qtcTreeViewLookupHash(hash, FALSE);
        if(tv)
        {
            if(tv->path)
                gtk_tree_path_free(tv->path);
            g_hash_table_remove(qtcTreeViewTable, hash);
        }
    }
}

static void qtcTreeViewCleanup(GtkWidget *widget)
{
    if (widget && g_object_get_data(G_OBJECT(widget), "QTC_TREE_VIEW_SET"))
    {
        qtcTreeViewRemoveFromHash(widget);
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_TREE_VIEW_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_TREE_VIEW_UNREALIZE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_TREE_VIEW_STYLE_SET_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_TREE_VIEW_MOTION_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_TREE_VIEW_LEAVE_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_TREE_VIEW_SET");
    }
}

static gboolean qtcTreeViewStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcTreeViewCleanup(widget);
    return FALSE;
}

static gboolean qtcTreeViewDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcTreeViewCleanup(widget);
    return FALSE;
}

static gboolean qtcTreeViewSamePath(GtkTreePath *a, GtkTreePath *b)
{
    return a ? (b && !gtk_tree_path_compare(a, b)) : !b;
}

static void qtcTreeViewUpdatePosition(GtkWidget *widget, int x, int y)
{
    if(GTK_IS_TREE_VIEW(widget))
    {
        QtCTreeView *tv=qtcTreeViewLookupHash(widget, FALSE);
        if(tv)
        {
            GtkTreeView       *treeView=GTK_TREE_VIEW(widget);
            GtkTreePath       *path=NULL;
            GtkTreeViewColumn *column=NULL;

            gtk_tree_view_get_path_at_pos(treeView, x, y, &path, &column, 0L, 0L );

            if(!qtcTreeViewSamePath(tv->path, path))
            {
                // prepare update area
                // get old rectangle
                GdkRectangle  oldRect={0, 0, -1, -1 },
                              newRect={0, 0, -1, -1 },
                              updateRect;
                GtkAllocation alloc=qtcWidgetGetAllocation(widget);

                if(tv->path && tv->column)
                    gtk_tree_view_get_background_area(treeView, tv->path, tv->column, &oldRect);
                if(tv->fullWidth)
                    oldRect.x = 0, oldRect.width = alloc.width;

                // get new rectangle and update position
                if(path && column)
                    gtk_tree_view_get_background_area(treeView, path, column, &newRect);
                if(path && column && tv->fullWidth)
                    newRect.x = 0, newRect.width = alloc.width;

                // take the union of both rectangles
                if(oldRect.width > 0 && oldRect.height > 0)
                {
                    if(newRect.width > 0 && newRect.height > 0)
                        gdk_rectangle_union(&oldRect, &newRect, &updateRect);
                    else
                        updateRect = oldRect;
                }
                else
                    updateRect = newRect;

                // store new cell info
                if(tv->path)
                    gtk_tree_path_free(tv->path);
                tv->path=path ? gtk_tree_path_copy(path) : NULL;
                tv->column=column;

                // convert to widget coordinates and schedule redraw
                gtk_tree_view_convert_bin_window_to_widget_coords(treeView, updateRect.x, updateRect.y, &updateRect.x, &updateRect.y);
                gtk_widget_queue_draw_area(widget, updateRect.x, updateRect.y, updateRect.width, updateRect.height);
            }

            if(path)
                gtk_tree_path_free(path);
        }
    }
}

static gboolean qtcTreeViewIsCellHovered(GtkWidget *widget, GtkTreePath *path, GtkTreeViewColumn *column)
{
    QtCTreeView *tv=qtcTreeViewLookupHash(widget, FALSE);
    return tv && (tv->fullWidth || tv->column==column) && qtcTreeViewSamePath(path, tv->path);
}

static gboolean qtcTreeViewMotion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    if(event && event->window && GTK_IS_TREE_VIEW(widget) && gtk_tree_view_get_bin_window(GTK_TREE_VIEW(widget)) == event->window)
        qtcTreeViewUpdatePosition(widget, (int)event->x, (int)event->y);

    return FALSE;
}

static gboolean qtcTreeViewLeave(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    if(GTK_IS_TREE_VIEW(widget))
    {
        QtCTreeView *tv=qtcTreeViewLookupHash(widget, FALSE);
        if(tv)
        {
            GtkTreeView   *treeView=GTK_TREE_VIEW(widget);
            GdkRectangle  rect={0, 0, -1, -1 };
            GtkAllocation alloc=qtcWidgetGetAllocation(widget);

            if(tv->path && tv->column)
                gtk_tree_view_get_background_area(treeView, tv->path, tv->column, &rect);
            if(tv->fullWidth)
                rect.x = 0, rect.width = alloc.width;

            if(tv->path)
                gtk_tree_path_free(tv->path);
            tv->path=NULL;
            tv->column=NULL;

            gtk_tree_view_convert_bin_window_to_widget_coords(treeView, rect.x, rect.y, &rect.x, &rect.y);
            gtk_widget_queue_draw_area(widget, rect.x, rect.y, rect.width, rect.height);
        }
    }
    return FALSE;
}

static void qtcTreeViewSetup(GtkWidget *widget)
{
    if (widget && !g_object_get_data(G_OBJECT(widget), "QTC_TREE_VIEW_SET"))
    {
        QtCTreeView *tv=qtcTreeViewLookupHash(widget, TRUE);
        GtkTreeView *treeView=GTK_TREE_VIEW(widget);

        if(tv)
        {
            gint x, y;

#if GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
            tv->fullWidth=TRUE;
#else
            gtk_widget_style_get(widget, "row_ending_details", &tv->fullWidth, NULL);
#endif
            gdk_window_get_pointer(qtcWidgetGetWindow(widget), &x, &y, 0L);
            gtk_tree_view_convert_widget_to_bin_window_coords(treeView, x, y, &x, &y);
            qtcTreeViewUpdatePosition(widget, x, y);
                
            g_object_set_data(G_OBJECT(widget), "QTC_TREE_VIEW_SET", (gpointer)1);
            g_object_set_data(G_OBJECT(widget), "QTC_TREE_VIEW_DESTROY_ID",
                              (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event", G_CALLBACK(qtcTreeViewDestroy), NULL));
            g_object_set_data(G_OBJECT(widget), "QTC_TREE_VIEW_UNREALIZE_ID",
                              (gpointer)g_signal_connect(G_OBJECT(widget), "unrealize", G_CALLBACK(qtcTreeViewDestroy), NULL));
            g_object_set_data(G_OBJECT(widget), "QTC_TREE_VIEW_STYLE_SET_ID",
                              (gpointer)g_signal_connect(G_OBJECT(widget), "style-set", G_CALLBACK(qtcTreeViewStyleSet), NULL));
            g_object_set_data(G_OBJECT(widget), "QTC_TREE_VIEW_MOTION_ID",
                              (gpointer)g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(qtcTreeViewMotion), NULL));
            g_object_set_data(G_OBJECT(widget), "QTC_TREE_VIEW_LEAVE_ID",
                             (gpointer)g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(qtcTreeViewLeave), NULL));
        }

        if(!gtk_tree_view_get_show_expanders(treeView))
            gtk_tree_view_set_show_expanders(treeView, TRUE);
        if(gtk_tree_view_get_enable_tree_lines(treeView))
            gtk_tree_view_set_enable_tree_lines(treeView, FALSE);
    }  
}

#endif // GTK_CHECK_VERSION(2, 12, 0)
