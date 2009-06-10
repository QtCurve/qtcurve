#define QTC_EXTEND_MENUBAR_ITEM_HACK

#ifdef QTC_EXTEND_MENUBAR_ITEM_HACK
static gboolean menuIsSelectable(GtkWidget *menu)
{
    return !((!GTK_BIN(menu)->child &&
             G_OBJECT_TYPE(menu) == GTK_TYPE_MENU_ITEM) ||
             GTK_IS_SEPARATOR_MENU_ITEM(menu) ||
             !GTK_WIDGET_IS_SENSITIVE(menu) ||
             !GTK_WIDGET_VISIBLE(menu));
}

static gboolean qtcMenuShellButtonPress(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    if(GDK_BUTTON_PRESS==event->type)
    {
        // QtCurve's menubars have a 2 pixel border -> but want the left/top to be 'active'...
        int nx, ny;
        gdk_window_get_origin(widget->window, &nx, &ny);
        if((event->button.x_root-nx)<=2.0 || (event->button.y_root-ny)<=2.0)
        {
            GtkMenuShell *menuShell=GTK_MENU_SHELL(widget);
            GList        *children=menuShell->children;

            if((event->button.x_root-nx)<=2.0)
                event->button.x_root+=2.0;
            if((event->button.y_root-ny)<=2.0)
                event->button.y_root+=2.0;

            while (children)
            {
                GtkWidget *item = children->data;
                int cx=(item->allocation.x+nx),
                    cy=(item->allocation.y+ny),
                    cw=(item->allocation.width),
                    ch=(item->allocation.height);

                if(cx<=event->button.x_root && cy<=event->button.y_root &&
                   (cx+cw)>event->button.x_root && (cy+ch)>event->button.y_root)
                {
                    if(menuIsSelectable(item))
                    {
                        if(GDK_BUTTON_PRESS==event->type)
                        {
                            if(item!=menuShell->active_menu_item)
                                gtk_menu_shell_select_item(menuShell, item);
                            else
                                gtk_menu_shell_deselect(menuShell);
                        }
                        return TRUE;
                    }

                    break;
                }
                children = children->next;
            }
        }
    }

    return FALSE;
}
#endif

#define QTC_GE_IS_MENU(object) ((object) && objectIsA ((GObject*)(object), "GtkMenu"))
#define QTC_GE_IS_MENU_SHELL(object) ((object) && objectIsA((GObject*)(object), "GtkMenuShell"))
#define QTC_GE_IS_MENU_BAR(object) ((object) && objectIsA((GObject*)(object), "GtkMenuBar"))
#define QTC_GE_IS_MENU_ITEM(object) ((object) && objectIsA ((GObject*)(object), "GtkMenuItem"))
#define QTC_GE_IS_CONTAINER(object) ((object)  && objectIsA ((GObject*)(object), "GtkContainer"))
#define QTC_GE_IS_WIDGET(object) ((object)  && objectIsA ((GObject*)(object), "GtkWidget"))

static gboolean objectIsA(const GObject * object, const gchar * type_name)
{
  gboolean result = FALSE;

  if ((object))
    {
      GType tmp = g_type_from_name (type_name);

      if (tmp)
        result = g_type_check_instance_is_a ((GTypeInstance *) object, tmp);
    }

  return result;
}

/* Taken from glide Gtk2 engine */

/***********************************************
 *   Cleanup/remove Menu Shell signals
 ***********************************************/
static void qtcMenuShellCleanup(GtkWidget *widget)
{
  if (QTC_GE_IS_MENU_BAR(widget))
    {
      gint id = 0;
 
      id = (gint)g_object_steal_data (G_OBJECT(widget), "QTC_MENU_SHELL_MOTION_ID");
      g_signal_handler_disconnect(G_OBJECT(widget), id);
                                             
      id = (gint)g_object_steal_data (G_OBJECT(widget), "QTC_MENU_SHELL_LEAVE_ID");
      g_signal_handler_disconnect(G_OBJECT(widget), id);
     
      id = (gint)g_object_steal_data (G_OBJECT(widget), "QTC_MENU_SHELL_DESTROY_ID");
      g_signal_handler_disconnect(G_OBJECT(widget), id);
       
      id = (gint)g_object_steal_data (G_OBJECT(widget), "QTC_MENU_SHELL_STYLE_SET_ID");
      g_signal_handler_disconnect(G_OBJECT(widget), id);

#ifdef QTC_EXTEND_MENUBAR_ITEM_HACK
      id = (gint)g_object_steal_data (G_OBJECT(widget), "QTC_MENU_SHELL_BUTTON_PRESS_ID");
      g_signal_handler_disconnect(G_OBJECT(widget), id);
#endif

      g_object_steal_data (G_OBJECT(widget), "QTC_MENU_SHELL_HACK_SET");      
    }
}

/***********************************************
 *   Style set signal to ensure menushell signals
 *   get cleaned up if the theme changes
 ***********************************************/
static gboolean qtcMenuShellStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
  qtcMenuShellCleanup(widget);
  
  return FALSE;
}
 
/***********************************************
 *   Destroy signal to ensure menushell signals
 *   get cleaned if it is destroyed
 ***********************************************/
static gboolean qtcMenuShellDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
 qtcMenuShellCleanup (widget);
  
  return FALSE;
}
 
/***********************************************
 *   Motion signal to ensure menushell items
 *   prelight state changes on mouse move.
 ***********************************************/
static gboolean qtcMenuShellMotion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
  if (QTC_GE_IS_MENU_SHELL(widget))
    {
      gint pointer_x, pointer_y;
      GdkModifierType pointer_mask;
      GList *children = NULL, *child = NULL;
     
      gdk_window_get_pointer(widget->window, &pointer_x, &pointer_y, &pointer_mask);
        
      if (QTC_GE_IS_CONTAINER(widget))
        {
          children = gtk_container_get_children(GTK_CONTAINER(widget));
              
          for (child = g_list_first(children); child; child = g_list_next(child))
            {
          if ((child->data) && QTC_GE_IS_WIDGET(child->data) && 
                  (GTK_WIDGET_STATE(GTK_WIDGET(child->data)) != GTK_STATE_INSENSITIVE))
            {
              if ((pointer_x >= GTK_WIDGET(child->data)->allocation.x) && 
                  (pointer_y >= GTK_WIDGET(child->data)->allocation.y) &&
                  (pointer_x < (GTK_WIDGET(child->data)->allocation.x + 
                                  GTK_WIDGET(child->data)->allocation.width)) && 
                  (pointer_y < (GTK_WIDGET(child->data)->allocation.y +
                                  GTK_WIDGET(child->data)->allocation.height)))
                {
                      gtk_widget_set_state (GTK_WIDGET(child->data), GTK_STATE_PRELIGHT);
                }
              else
                    {
                      gtk_widget_set_state (GTK_WIDGET(child->data), GTK_STATE_NORMAL);
                    }
                 }
             }              
         
           if (children)   
             g_list_free(children);
        }
    }
 
  return FALSE;
}
 
/***********************************************
 *   Leave signal to ensure menushell items
 *   normal state on mouse leave.
 ***********************************************/
static gboolean qtcMenuShellLeave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
  if (QTC_GE_IS_MENU_SHELL(widget))
    {
      GList *children = NULL, *child = NULL;
 
      if (QTC_GE_IS_CONTAINER(widget))
        {
          children = gtk_container_get_children(GTK_CONTAINER(widget));
              
          for (child = g_list_first(children); child; child = g_list_next(child))
            {
          if ((child->data) && QTC_GE_IS_MENU_ITEM(child->data) && 
                  (GTK_WIDGET_STATE(GTK_WIDGET(child->data)) != GTK_STATE_INSENSITIVE))
            {
                  if ((!QTC_GE_IS_MENU(GTK_MENU_ITEM(child->data)->submenu)) || 
                      (!(GTK_WIDGET_REALIZED(GTK_MENU_ITEM(child->data)->submenu) && 
                         GTK_WIDGET_VISIBLE(GTK_MENU_ITEM(child->data)->submenu) &&
                         GTK_WIDGET_REALIZED(GTK_MENU(GTK_MENU_ITEM(child->data)->submenu)->toplevel) &&
                         GTK_WIDGET_VISIBLE(GTK_MENU(GTK_MENU_ITEM(child->data)->submenu)->toplevel))))
              {
                    gtk_widget_set_state (GTK_WIDGET(child->data), GTK_STATE_NORMAL);
                  }
                }               
            }         
            
          if (children)   
        g_list_free(children);
        }
    }
 
  return FALSE;
}
 
/***********************************************
 *   Setup Menu Shell with signals to ensure
 *   prelight works on items
 ***********************************************/
static void qtcMenuShellSetup(GtkWidget *widget)
{
  if (QTC_GE_IS_MENU_BAR(widget))
    {
      gint id = 0;
 
      if (!g_object_get_data(G_OBJECT(widget), "QTC_MENU_SHELL_HACK_SET"))
      {
        id = g_signal_connect(G_OBJECT(widget), "motion-notify-event",
                                             (GtkSignalFunc)qtcMenuShellMotion,
                                             NULL);
                                  
        g_object_set_data(G_OBJECT(widget), "QTC_MENU_SHELL_MOTION_ID", (gpointer)id);
        
        id = g_signal_connect(G_OBJECT(widget), "leave-notify-event",
                                             (GtkSignalFunc)qtcMenuShellLeave,
                                             NULL);
        g_object_set_data(G_OBJECT(widget), "QTC_MENU_SHELL_LEAVE_ID", (gpointer)id);
                                             
        id = g_signal_connect(G_OBJECT(widget), "destroy-event",
                                             (GtkSignalFunc)qtcMenuShellDestroy,
                                             NULL);
        g_object_set_data(G_OBJECT(widget), "QTC_MENU_SHELL_DESTROY_ID", (gpointer)id);
 
        g_object_set_data(G_OBJECT(widget), "QTC_MENU_SHELL_HACK_SET", (gpointer)1);
        
        id = g_signal_connect(G_OBJECT(widget), "style-set",
                                             (GtkSignalFunc)qtcMenuShellStyleSet,
                                             NULL);
        g_object_set_data(G_OBJECT(widget), "QTC_MENU_SHELL_STYLE_SET_ID", (gpointer)id);
        
        
#ifdef QTC_EXTEND_MENUBAR_ITEM_HACK
        id=g_signal_connect(G_OBJECT(widget), "button-press-event", G_CALLBACK(qtcMenuShellButtonPress), widget);
        g_object_set_data(G_OBJECT(widget), "QTC_MENU_SHELL_BUTTON_PRESS_ID", (gpointer)id);
#endif
      }
    }  
}
