typedef struct
{
    int          id,
                 numRects;
    GdkRectangle *rects;
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
        int p;
            
        rv=(QtCTab *)malloc(sizeof(QtCTab));
        rv->numRects=gtk_notebook_get_n_pages(GTK_NOTEBOOK(hash));
        rv->rects=(GdkRectangle *)malloc(sizeof(GdkRectangle) * rv->numRects);
        rv->id=-1;
        
        for(p=0; p<rv->numRects; ++p)
        {
            rv->rects[p].x=rv->rects[p].y=0;
            rv->rects[p].width=rv->rects[p].height=-1;
        }
        g_hash_table_insert(tabHashTable, hash, rv);
        rv=g_hash_table_lookup(tabHashTable, hash);
    }

    return rv;
}

static void removeFromTabHash(void *hash)
{
    if(tabHashTable)
    {
        QtCTab *tab=GTK_IS_NOTEBOOK(hash) ? lookupTabHash(hash, FALSE) : NULL;
        
        if(tab)
            free(tab->rects);
        g_hash_table_remove(tabHashTable, hash);
    }
}

static gboolean qtcTabCurrentHoveredIndex(GtkWidget *widget)
{
    QtCTab *tab=GTK_IS_NOTEBOOK(widget) ? lookupTabHash(widget, FALSE) : NULL;

    return tab ? tab->id : -1;
}

static void qtcTabUpdateRect(GtkWidget *widget, int tabIndex, int x, int y, int width, int height)
{
    QtCTab *tab=GTK_IS_NOTEBOOK(widget) ? lookupTabHash(widget, FALSE) : NULL;

    if(tab && tabIndex>=0)
    {
        if(tabIndex>=tab->numRects)
        {
            int p;
            
            tab->rects=realloc(tab->rects, sizeof(GdkRectangle)*(tabIndex+8));
            for(p=tab->numRects; p<tabIndex+8; ++p)
            {
                tab->rects[p].x=tab->rects[p].y=0;
                tab->rects[p].width=tab->rects[p].height=-1;
            }
            tab->numRects=tabIndex+8;
        }
        tab->rects[tabIndex].x=x, tab->rects[tabIndex].y=y, tab->rects[tabIndex].width=width, tab->rects[tabIndex].height=height;
    }
}

static void qtcTabCleanup(GtkWidget *widget)
{
    if (widget)
    { 
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_MOTION_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_LEAVE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_PAGE_ADDED_ID"));
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

static void qtcSetHoveredTab(QtCTab *tab, GtkWidget *widget, int index)
{
    if(index!=tab->id)
    {
        GdkRectangle updateRect;
        int          p;
        
        updateRect.x=updateRect.y=0;
        updateRect.width=updateRect.height=-1;

        tab->id=index;

        for(p=0; p<tab->numRects; ++p)
            gdk_rectangle_union(&(tab->rects[p]), &updateRect, &updateRect);

        gtk_widget_queue_draw_area(widget, updateRect.x-4, updateRect.y-4, updateRect.width+8, updateRect.height+8);
    }
}

static gboolean qtcTabMotion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    QtCTab *tab=GTK_IS_NOTEBOOK(widget) ? lookupTabHash(widget, FALSE) : NULL;

    if (tab)
    {
        int px, py, t;
        gdk_window_get_pointer(qtcWidgetGetWindow(widget), &px, &py, NULL);

        for(t=0; t < tab->numRects; t++ )
        {
            if(tab->rects[t].x<=px && tab->rects[t].y<=py && (tab->rects[t].x+tab->rects[t].width)>px && (tab->rects[t].y+tab->rects[t].height)>py)
            {
                qtcSetHoveredTab(tab, widget, t);
                return FALSE;
            } 
        }

        qtcSetHoveredTab(tab, widget, -1);
    }

    return FALSE;
}

static gboolean qtcTabLeave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    QtCTab *prevTab=GTK_IS_NOTEBOOK(widget) ? lookupTabHash(widget, FALSE) : NULL;

    if(prevTab && prevTab->id>=0)
    {
        prevTab->id=-1;
        gtk_widget_queue_draw(widget);
    }
 
    return FALSE;
}

static void qtcTabUnRegisterChild(GtkWidget *widget)
{
    if (widget && g_object_get_data(G_OBJECT(widget), "QTC_TAB_HACK_CHILD_SET"))
    {
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_C_ENTER_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_C_LEAVE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_C_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_C_STYLE_SET_ID"));
        if(GTK_IS_CONTAINER(widget))
            g_signal_handler_disconnect(G_OBJECT(widget), (gint)g_object_steal_data (G_OBJECT(widget), "QTC_TAB_C_ADD_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_TAB_HACK_CHILD_SET");
    }
}

static void qtcTabUpdateChildren(GtkWidget *widget);

static gboolean qtcTabChildMotion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    qtcTabMotion((GtkWidget *)user_data, event, widget);
    return FALSE;
}

static gboolean qtcTabChildDestroy(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    qtcTabUnRegisterChild(widget);
    return FALSE;
}

static gboolean qtcTabChildStyleSet(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    qtcTabUnRegisterChild(widget);
    return FALSE;
}

static gboolean qtcTabChildAdd(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    qtcTabUpdateChildren((GtkWidget *)user_data);
    return FALSE;
}

static void qtcTabRegisterChild(GtkWidget *notebook, GtkWidget *widget)
{
    if (widget && !g_object_get_data(G_OBJECT(widget), "QTC_TAB_HACK_CHILD_SET"))
    {
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_C_ENTER_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "enter-notify-event", G_CALLBACK(qtcTabChildMotion), notebook));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_C_LEAVE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(qtcTabChildMotion), notebook));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_C_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "destroy", G_CALLBACK(qtcTabChildDestroy), notebook));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_C_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "style-set", G_CALLBACK(qtcTabChildStyleSet), notebook));
        if(GTK_IS_CONTAINER(widget))
        {
            GList *children=gtk_container_get_children(GTK_CONTAINER(widget)),
                  *child;

            g_object_set_data(G_OBJECT(widget), "QTC_TAB_C_ADD_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "add", G_CALLBACK(qtcTabChildAdd), notebook));
            for(child = g_list_first(children); child; child = g_list_next(child))
                qtcTabRegisterChild(notebook, GTK_WIDGET(child->data));

            if(children)
                g_list_free(children);
        }
    }
}

static void qtcTabUpdateChildren(GtkWidget *widget)
{
    if(widget && GTK_IS_NOTEBOOK(widget))
    {
        GtkNotebook *notebook=GTK_NOTEBOOK(widget);
        int i,
            numPages=gtk_notebook_get_n_pages(notebook);

        for(i = 0; i<numPages ; ++i)
        {
            qtcTabRegisterChild(widget, gtk_notebook_get_tab_label(notebook, gtk_notebook_get_nth_page(notebook, i)));
        }
    }
}

static gboolean qtcTabPageAdded(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    qtcTabUpdateChildren(widget);
    return FALSE;
}

static void qtcTabSetup(GtkWidget *widget)
{
    if (widget && !g_object_get_data(G_OBJECT(widget), "QTC_TAB_HACK_SET"))
    {
        lookupTabHash(widget, TRUE);

        g_object_set_data(G_OBJECT(widget), "QTC_TAB_MOTION_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(qtcTabMotion), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_LEAVE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(qtcTabLeave), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_PAGE_ADDED_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "page-added", G_CALLBACK(qtcTabPageAdded), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event", G_CALLBACK(qtcTabDestroy), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_UNREALIZE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "unrealize", G_CALLBACK(qtcTabDestroy), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "style-set", G_CALLBACK(qtcTabStyleSet), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_HACK_SET", (gpointer)1);
        qtcTabUpdateChildren(widget);
    }  
}
