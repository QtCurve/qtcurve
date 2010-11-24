#define GE_IS_EVENT_BOX(object) ((object) && objectIsA((GObject*)(object), "GtkEventBox"))

/**
 * Setting appears-as-list on a non-editable combo creas a view over the 'label' which
 * is of 'base' colour. gtk_cell_view_set_background_color removes this
 */
static void qtcComboBoxClearBgndColor(GtkWidget *widget)
{
    GList *children = gtk_container_get_children(GTK_CONTAINER(widget)),
          *child    = children;

    for(; child; child=child->next)
    {
        GtkWidget *boxChild=(GtkWidget *)child->data;

        if(GTK_IS_CELL_VIEW(boxChild))
            gtk_cell_view_set_background_color(GTK_CELL_VIEW(boxChild), 0L);
    }

    if(children)
        g_list_free(children);
}

static GtkWidget *qtcComboFocus=NULL;
static GtkWidget *qtcComboHover=NULL;

static qtcComboBoxIsFocusChanged(GtkWidget *widget)
{
    if(qtcComboFocus==widget)
    {
        if(!qtcWidgetHasFocus(widget))
        {
            qtcComboFocus=NULL;
            return TRUE;
        }
    }
    else if(qtcWidgetHasFocus(widget))
    {
        qtcComboFocus=widget;
        return TRUE;
    }

    return FALSE;
}

static qtcComboBoxHasFocus(GtkWidget *widget, GtkWidget *mapped)
{
    return qtcWidgetHasFocus(widget) || (mapped && mapped==qtcComboFocus);
}

static qtcComboBoxIsHovered(GtkWidget *widget)
{
    return widget==qtcComboHover;
}

static void qtcComboBoxCleanup(GtkWidget *widget)
{
    if (widget && g_object_get_data(G_OBJECT(widget), "QTC_COMBO_BOX_SET"))
    {
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_COMBO_BOX_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_COMBO_BOX_UNREALIZE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_COMBO_BOX_STYLE_SET_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_COMBO_BOX_ENTER_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_COMBO_BOX_LEAVE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_COMBO_BOX_STATE_CHANGE_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_COMBO_BOX_SET");
    }
}

static gboolean qtcComboBoxStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcComboBoxCleanup(widget);
    return FALSE;
}

static gboolean qtcComboBoxDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcComboBoxCleanup(widget);
    return FALSE;
}

static gboolean qtcComboBoxEnter(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    if(GE_IS_EVENT_BOX(widget))
    {
        GtkWidget *widget=(GtkWidget *)user_data;
        if(qtcComboHover!=widget)
        {
            qtcComboHover=widget;
            gtk_widget_queue_draw(widget);
        }
    }
    return FALSE;
}

static gboolean qtcComboBoxLeave(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    if(GE_IS_EVENT_BOX(widget))
    {
        GtkWidget *widget=(GtkWidget *)user_data;
        if(qtcComboHover==widget)
        {
            qtcComboHover=NULL;
            gtk_widget_queue_draw(widget);
        }
    }
    return FALSE;
}

static gboolean qtcComboBoxStateChange(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    if(GTK_IS_CONTAINER(widget))
        qtcComboBoxClearBgndColor(widget);
}

static void qtcComboBoxSetup(GtkWidget *frame, GtkWidget *combo)
{
    if (combo && frame && !g_object_get_data(G_OBJECT(combo), "QTC_COMBO_BOX_SET"))
    {
        GList *children = gtk_container_get_children(GTK_CONTAINER(frame)),
              *child    = children;

        qtcComboBoxClearBgndColor(combo);

        g_object_set_data(G_OBJECT(combo), "QTC_COMBO_BOX_SET", (gpointer)1);
        g_object_set_data(G_OBJECT(combo), "QTC_COMBO_BOX_STATE_CHANGE_ID",
                         (gpointer)g_signal_connect(G_OBJECT(combo), "state-change", G_CALLBACK(qtcComboBoxStateChange), NULL));

        for(; child; child=child->next)
        {
            GtkWidget *boxChild=(GtkWidget *)child->data;

            if(GTK_IS_EVENT_BOX(boxChild))
            {
                g_object_set_data(G_OBJECT(boxChild), "QTC_COMBO_BOX_DESTROY_ID",
                                (gpointer)g_signal_connect(G_OBJECT(boxChild), "destroy-event", G_CALLBACK(qtcComboBoxDestroy), NULL));
                g_object_set_data(G_OBJECT(boxChild), "QTC_COMBO_BOX_UNREALIZE_ID",
                                (gpointer)g_signal_connect(G_OBJECT(boxChild), "unrealize", G_CALLBACK(qtcComboBoxDestroy), NULL));
                g_object_set_data(G_OBJECT(boxChild), "QTC_COMBO_BOX_STYLE_SET_ID",
                                (gpointer)g_signal_connect(G_OBJECT(boxChild), "style-set", G_CALLBACK(qtcComboBoxStyleSet), NULL));
                g_object_set_data(G_OBJECT(boxChild), "QTC_COMBO_BOX_ENTER_ID",
                                (gpointer)g_signal_connect(G_OBJECT(boxChild), "enter-notify-event", G_CALLBACK(qtcComboBoxEnter), combo));
                g_object_set_data(G_OBJECT(boxChild), "QTC_COMBO_BOX_LEAVE_ID",
                                (gpointer)g_signal_connect(G_OBJECT(boxChild), "leave-notify-event", G_CALLBACK(qtcComboBoxLeave), combo));
            }
        }

        if(children)
            g_list_free(children);
    }
}
