static int       qtcWMMoveLastX=-1;
static int       qtcWMMoveLastY=-1;
static int       qtcWMMoveTimer=0;
static GtkWidget *qtcWMMoveDragWidget=NULL;

static void qtcWMMoveStopTimer()
{
    if(qtcWMMoveTimer)
        g_source_remove(qtcWMMoveTimer);
    qtcWMMoveTimer=0;
}

static void qtcWMMoveReset()
{
    qtcWMMoveLastX=-1;
    qtcWMMoveLastY=-1;
    qtcWMMoveDragWidget=NULL;
    qtcWMMoveStopTimer();
}

static void qtcWMMoveStore(GtkWidget *widget, GdkEventButton *event)
{
    qtcWMMoveLastX=event ? event->x_root : -1;
    qtcWMMoveLastY=event ? event->y_root : -1;
    qtcWMMoveDragWidget=widget;
}

static gboolean qtcWMMoveButtonRelease(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

static void qtcWMMoveTrigger(GtkWidget *w, int x, int y)
{
    XEvent     xev;
    GtkWindow  *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(w));
    GdkWindow  *window=gtk_widget_get_window(GTK_WIDGET(topLevel));
    GdkDisplay *display=gtk_widget_get_display(GTK_WIDGET(topLevel));
    GdkWindow  *root=gdk_screen_get_root_window(gtk_window_get_screen(topLevel));

    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = gdk_x11_get_xatom_by_name_for_display(display, "_NET_WM_MOVERESIZE");
    xev.xclient.display = GDK_DISPLAY_XDISPLAY(display);
    xev.xclient.window = GDK_WINDOW_XID(window);
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = x;
    xev.xclient.data.l[1] = y;
    xev.xclient.data.l[2] = 8; // NET::Move
    xev.xclient.data.l[3] = Button1;
    xev.xclient.data.l[4] = 0;
    XUngrabPointer(GDK_DISPLAY_XDISPLAY(display), CurrentTime);

    XSendEvent(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XID(root), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
    /* force a release as some widgets miss it... */
    qtcWMMoveButtonRelease(w, NULL, NULL);
}

static gboolean qtcWMMoveWithinWidget(GtkWidget *widget, GdkEventButton *event)
{
    GdkWindow *window = gtk_widget_get_parent_window(widget);

    if(window)
    {
        GtkAllocation alloc=qtcWidgetGetAllocation(widget);
        int           nx=0,
                      ny=0,
                      adjust=0; /* TODO !!! */

        // Need to get absolute co-ordinates...
        alloc.x-=adjust;
        alloc.y-=adjust;
        alloc.width+=adjust;
        alloc.height+=adjust;
        gdk_window_get_origin(gtk_widget_get_parent_window(widget), &nx, &ny);
        alloc.x+=nx;
        alloc.y+=ny;

        return alloc.x<=event->x_root && alloc.y<=event->y_root &&
               (alloc.x+alloc.width)>event->x_root &&(alloc.y+alloc.height)>event->y_root;
    }
    return true;
}

static gboolean qtcWMMoveUseEvent(GtkWidget *widget, GdkEventButton *event)
{
    bool use=TRUE;
    if(GTK_IS_NOTEBOOK(widget)) /* Check if there is a hovered tab */
    {
        if(-1!=qtcTabCurrentHoveredIndex(widget))
            use = false;
    }
    else if(GTK_IS_CONTAINER(widget))
    {
        GList *containers = NULL;

        containers=g_list_prepend(containers, widget);

        // Check all widget children for event
        while(g_list_length(containers))
        {
            GtkContainer *c        = GTK_CONTAINER(g_list_nth_data(containers, 0));
            GList        *children = gtk_container_get_children(GTK_CONTAINER(c)),
                         *child    = NULL;

            for(child = g_list_first(children); child && use; child = g_list_next(child))
            {
                if(child->data && GTK_IS_WIDGET(child->data))
                {
                    GtkWidget *childWidget=GTK_WIDGET(child->data);

                    if(GTK_IS_CONTAINER(childWidget))
                        containers = g_list_prepend(containers, child->data);

                    // if widget is prelight, we don't need to check where event happen,
                    //  any prelight widget indicate we can't do a move */
                    if(GTK_STATE_PRELIGHT==qtcWidgetGetState(childWidget))
                        use = false;
                    // if event happen in widget
                    // check not a notebook: event in but notebook don't get it...
                    else if(!GTK_IS_NOTEBOOK(childWidget) && event && qtcWMMoveWithinWidget(childWidget, event))
                    {
                        // here deal with notebook: widget may be not visible
                        GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(childWidget));
                        if(window && gdk_window_is_visible(window))
                        {
                            // widget listening to press event
                            if(gtk_widget_get_events(GTK_WIDGET(childWidget)) & GDK_BUTTON_PRESS_MASK)
                                use = false;
                            // deal with menu item, GtkMenuItem only listen to
                            // GDK_BUTTON_PRESS_MASK when state == GTK_STATE_PRELIGHT
                            // so previous check are invalids :(
                            //
                            // same for ScrolledWindow, they do not send motion events
                            // to parents so not usable
                            else if(GTK_IS_MENU_ITEM(childWidget) || GTK_IS_SCROLLED_WINDOW(childWidget))
                                use = false;
                        }
                    }
                }
            }
            if(children)
                g_list_free(children);
            containers = g_list_remove(containers, c);
        }
    }
    
    return use;
}

static gboolean qtcWWMoveStartDelayedDrag(gpointer data)
{
    if(qtcWMMoveDragWidget)
        qtcWMMoveTrigger(qtcWMMoveDragWidget, qtcWMMoveLastX, qtcWMMoveLastY);
}

static gboolean qtcWMMoveIsWindowDragWidget(GtkWidget *widget, GdkEventButton *event)
{
    if(opts.windowDrag && (!event || (qtcWMMoveWithinWidget(widget, event) && qtcWMMoveUseEvent(widget, event))))
    {
        qtcWMMoveStore(widget, event);
        // Start timer
        qtcWMMoveStopTimer();
        qtcWMMoveTimer=g_timeout_add(qtSettings.startDragTime, (GSourceFunc)qtcWWMoveStartDelayedDrag, NULL);

        return TRUE;
    }
    return FALSE;
}

static gboolean qtcWMMoveButtonPress(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    if (GDK_BUTTON_PRESS==event->type && 1==event->button && qtcWMMoveIsWindowDragWidget(widget, event))
    {
        qtcWMMoveDragWidget=widget;
        return TRUE;
    }

    return FALSE;
}

static gboolean qtcWMMoveDragEnd(GtkWidget *widget)
{
    if (widget==qtcWMMoveDragWidget)
    {
        gtk_grab_remove(widget);
        gdk_pointer_ungrab(CurrentTime);
        qtcWMMoveReset();
        return TRUE;
    }

    return FALSE;
}

static gboolean qtcWMMoveButtonRelease(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    return qtcWMMoveDragEnd(widget);
}

static void qtcWMMoveCleanup(GtkWidget *widget)
{
    if (g_object_get_data(G_OBJECT(widget), "QTC_WM_MOVE_HACK_SET"))
    {
        if(widget==qtcWMMoveDragWidget)
            qtcWMMoveReset();
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_MOTION_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_LEAVE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_STYLE_SET_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_BUTTON_PRESS_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_BUTTON_RELEASE_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_HACK_SET");
    }
}

static gboolean qtcWMMoveStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcWMMoveCleanup(widget);
    return FALSE;
}
 
static gboolean qtcWMMoveDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcWMMoveCleanup(widget);
    return FALSE;
}

static gboolean qtcWMMoveMotion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    if (qtcWMMoveDragWidget==widget)
    {
        // check displacement with respect to drag start
        const int distance=abs(qtcWMMoveLastX - event->x_root) + abs(qtcWMMoveLastY - event->y_root);

        if( distance > 0)
            qtcWMMoveStopTimer();

        if(distance < qtSettings.startDragDist)
            return FALSE;
        qtcWMMoveTrigger(widget, event->x_root, event->y_root);
        return TRUE;
    }

    return FALSE;
}

static gboolean qtcWMMoveLeave(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    return qtcWMMoveDragEnd(widget);
}

static void qtcWMMoveSetup(GtkWidget *widget)
{
    if (widget && !isFakeGtk() && !g_object_get_data(G_OBJECT(widget), "QTC_WM_MOVE_HACK_SET"))
    {
        gtk_widget_add_events(widget, GDK_BUTTON_RELEASE_MASK|GDK_BUTTON_PRESS_MASK|GDK_LEAVE_NOTIFY_MASK|GDK_BUTTON1_MOTION_MASK);
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_HACK_SET", (gpointer)1);
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_MOTION_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(qtcWMMoveMotion), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_LEAVE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(qtcWMMoveLeave), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event", G_CALLBACK(qtcWMMoveDestroy), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "style-set", G_CALLBACK(qtcWMMoveStyleSet), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_BUTTON_PRESS_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "button-press-event", G_CALLBACK(qtcWMMoveButtonPress), widget));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_BUTTON_RELEASE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "button-release-event", G_CALLBACK(qtcWMMoveButtonRelease), widget));
    }  
}
