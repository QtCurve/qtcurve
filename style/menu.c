#define GE_IS_MENU(object)((object) && objectIsA((GObject*)(object), "GtkMenu"))
#define GE_IS_MENU_SHELL(object)((object) && objectIsA((GObject*)(object), "GtkMenuShell"))
#define GE_IS_MENU_BAR(object)((object) && objectIsA((GObject*)(object), "GtkMenuBar"))
#define GE_IS_MENU_ITEM(object)((object) && objectIsA((GObject*)(object), "GtkMenuItem"))
#define GE_IS_CONTAINER(object)((object)  && objectIsA((GObject*)(object), "GtkContainer"))
#define GE_IS_WIDGET(object)((object)  && objectIsA((GObject*)(object), "GtkWidget"))

static gboolean qtcEmitMenuSize(GtkWidget *w, unsigned int size)
{
#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
    if(w)
    {
        unsigned int oldSize=(unsigned int)g_object_get_data(G_OBJECT(w), MENU_SIZE_ATOM);

        if(oldSize!=size)
        {
            GtkWindow  *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(w));
            GdkDisplay *display=gtk_widget_get_display(GTK_WIDGET(topLevel));

            if(0xFFFF==size)
                size=0;
            g_object_set_data(G_OBJECT(w), MENU_SIZE_ATOM, (gpointer)size);
            unsigned short ssize=size;
            XChangeProperty(gdk_x11_display_get_xdisplay(display), GDK_WINDOW_XID(qtcWidgetGetWindow(GTK_WIDGET(topLevel))),
                            gdk_x11_get_xatom_by_name_for_display(display, MENU_SIZE_ATOM),
                            XA_CARDINAL, 16, PropModeReplace, (unsigned char *)&ssize, 1);
            return TRUE;
        }
    }
#endif
    return FALSE;
}

static gboolean objectIsA(const GObject * object, const gchar * type_name)
{
    if((object))
    {
        GType tmp = g_type_from_name(type_name);

        if(tmp)
            return g_type_check_instance_is_a((GTypeInstance *) object, tmp);
    }

    return FALSE;
}

#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
#define EXTEND_MENUBAR_ITEM_HACK
#endif

#ifdef EXTEND_MENUBAR_ITEM_HACK
static const int constMenuAdjust=2;

static gboolean menuIsSelectable(GtkWidget *menu)
{
    return !((!qtcBinGetChild(GTK_BIN(menu)) && G_OBJECT_TYPE(menu) == GTK_TYPE_MENU_ITEM) ||
             GTK_IS_SEPARATOR_MENU_ITEM(menu) ||
             !GTK_WIDGET_IS_SENSITIVE(menu) ||
             !GTK_WIDGET_VISIBLE(menu));
}

static gboolean qtcMenuShellButtonPress(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    if(GE_IS_MENU_BAR(widget))
    {
        // QtCurve's menubars have a 2 pixel border -> but want the left/top to be 'active'...
        int nx, ny;
        gdk_window_get_origin(qtcWidgetGetWindow(widget), &nx, &ny);
        if((event->x_root-nx)<=2.0 ||(event->y_root-ny)<=2.0)
        {
            GtkMenuShell *menuShell=GTK_MENU_SHELL(widget);
            GList        *children=gtk_container_get_children(GTK_CONTAINER(menuShell)),
                         *child=children;
            gboolean     rv=FALSE;

            if((event->x_root-nx)<=2.0)
                event->x_root+=2.0;
            if((event->y_root-ny)<=2.0)
                event->y_root+=2.0;

            while(child && !rv)
            {
                GtkWidget     *item = child->data;
                GtkAllocation alloc = qtcWidgetGetAllocation(item);
                
                int cx=(alloc.x+nx),
                    cy=(alloc.y+ny),
                    cw=(alloc.width),
                    ch=(alloc.height);

                if(cx<=event->x_root && cy<=event->y_root &&
                  (cx+cw)>event->x_root &&(cy+ch)>event->y_root)
                {
                    if(menuIsSelectable(item))
                    {
                        if(GDK_BUTTON_PRESS==event->type)
                            if(item!=menuShell->active_menu_item)
                            {
                                menuShell->active=FALSE;
                                gtk_menu_shell_select_item(menuShell, item);
                                menuShell->active=TRUE;
                            }
                            else
                            {
                                menuShell->active=TRUE;
                                gtk_menu_shell_deselect(menuShell);
                                menuShell->active=FALSE;
                            }
                        rv=TRUE;
                    }

                    break;
                }
                child = child->next;
            }

            if(children)
                g_list_free(children);
            return rv;
        }
    }

    return FALSE;
}
#endif

/* Taken from glide Gtk2 engine */

/***********************************************
 *   Cleanup/remove Menu Shell signals
 ***********************************************/
static void qtcMenuShellCleanup(GtkWidget *widget)
{
    if(GE_IS_MENU_BAR(widget))
    {
        g_signal_handler_disconnect(G_OBJECT(widget),
                                   (gint)g_object_steal_data(G_OBJECT(widget), "QTC_MENU_SHELL_MOTION_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                   (gint)g_object_steal_data(G_OBJECT(widget), "QTC_MENU_SHELL_LEAVE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                   (gint)g_object_steal_data(G_OBJECT(widget), "QTC_MENU_SHELL_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                   (gint)g_object_steal_data(G_OBJECT(widget), "QTC_MENU_SHELL_STYLE_SET_ID"));
#ifdef EXTEND_MENUBAR_ITEM_HACK
        g_signal_handler_disconnect(G_OBJECT(widget),
                                   (gint)g_object_steal_data(G_OBJECT(widget), "QTC_MENU_SHELL_BUTTON_PRESS_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                   (gint)g_object_steal_data(G_OBJECT(widget), "QTC_MENU_SHELL_BUTTON_RELEASE_ID"));
#endif
      g_object_steal_data(G_OBJECT(widget), "QTC_MENU_SHELL_HACK_SET");
    }
}

/***********************************************
 *   Style set signal to ensure menushell signals get cleaned up if the theme changes
 ***********************************************/
static gboolean qtcMenuShellStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcMenuShellCleanup(widget);
    return FALSE;
}
 
/***********************************************
 *   Destroy signal to ensure menushell signals get cleaned if it is destroyed
 ***********************************************/
static gboolean qtcMenuShellDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcMenuShellCleanup(widget);
    return FALSE;
}

/***********************************************
 *   Motion signal to ensure menushell items prelight state changes on mouse move.
 ***********************************************/
static gboolean qtcMenuShellMotion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    if(GE_IS_MENU_SHELL(widget))
    {
        gint            pointer_x,
                        pointer_y;
        GdkModifierType pointer_mask;
     
        gdk_window_get_pointer(qtcWidgetGetWindow(widget), &pointer_x, &pointer_y, &pointer_mask);

        if(GE_IS_CONTAINER(widget))
        {
            GList *children = gtk_container_get_children(GTK_CONTAINER(widget)),
                  *child;
              
            for(child = g_list_first(children); child; child = g_list_next(child))
            {
                if((child->data) && GE_IS_WIDGET(child->data) &&
                   (GTK_WIDGET_STATE(GTK_WIDGET(child->data)) != GTK_STATE_INSENSITIVE))
                {
                    GtkAllocation alloc=qtcWidgetGetAllocation(GTK_WIDGET(child->data));

                    if((pointer_x >= alloc.x) && (pointer_y >= alloc.y) && (pointer_x <(alloc.x + alloc.width)) &&
                       (pointer_y <(alloc.y + alloc.height)))
                        gtk_widget_set_state(GTK_WIDGET(child->data), GTK_STATE_PRELIGHT);
                    else
                        gtk_widget_set_state(GTK_WIDGET(child->data), GTK_STATE_NORMAL);
                 }
             }    
         
            if(children)   
                g_list_free(children);
        }
    }
 
    return FALSE;
}
 
/***********************************************
 *   Leave signal to ensure menushell items normal state on mouse leave.
 ***********************************************/
static gboolean qtcMenuShellLeave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    if(GE_IS_MENU_SHELL(widget) && GE_IS_CONTAINER(widget))
    { 
        GList *children = gtk_container_get_children(GTK_CONTAINER(widget)),
              *child    = NULL;
              
        for(child = g_list_first(children); child; child = g_list_next(child))
        {
            if((child->data) && GE_IS_MENU_ITEM(child->data) &&
               (GTK_WIDGET_STATE(GTK_WIDGET(child->data)) != GTK_STATE_INSENSITIVE))
            {
                GtkWidget *submenu  = qtcMenuItemGetSubMenu(GTK_MENU_ITEM(child->data)),
                          *topLevel = submenu ? qtcMenuGetTopLevel(submenu) : NULL;
                
               if(submenu &&
                  ((!GE_IS_MENU(submenu)) ||
                        (!(GTK_WIDGET_REALIZED(submenu) && GTK_WIDGET_VISIBLE(submenu) &&
                           GTK_WIDGET_REALIZED(topLevel) && GTK_WIDGET_VISIBLE(topLevel)))))
                gtk_widget_set_state(GTK_WIDGET(child->data), GTK_STATE_NORMAL);
            }
        }
            
        if(children)   
            g_list_free(children);
    }
 
  return FALSE;
}
 
/***********************************************
 *   Setup Menu Shell with signals to ensure
 *   prelight works on items
 ***********************************************/
static void qtcMenuShellSetup(GtkWidget *widget)
{
    if(GE_IS_MENU_BAR(widget) && !g_object_get_data(G_OBJECT(widget), "QTC_MENU_SHELL_HACK_SET"))
    {
        g_object_set_data(G_OBJECT(widget), "QTC_MENU_SHELL_HACK_SET",(gpointer)1);
        g_object_set_data(G_OBJECT(widget), "QTC_MENU_SHELL_MOTION_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(qtcMenuShellMotion), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_MENU_SHELL_LEAVE_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(qtcMenuShellLeave), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_MENU_SHELL_DESTROY_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event", G_CALLBACK(qtcMenuShellDestroy), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_MENU_SHELL_STYLE_SET_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "style-set", G_CALLBACK(qtcMenuShellStyleSet), NULL));
#ifdef EXTEND_MENUBAR_ITEM_HACK
        g_object_set_data(G_OBJECT(widget), "QTC_MENU_SHELL_BUTTON_PRESS_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "button-press-event", G_CALLBACK(qtcMenuShellButtonPress), widget));
        g_object_set_data(G_OBJECT(widget), "QTC_MENU_SHELL_BUTTON_RELEASE_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "button-release-event",
                                                    G_CALLBACK(qtcMenuShellButtonPress), widget));
#endif
    }  
}
