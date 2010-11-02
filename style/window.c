#include <gdk/gdkkeysyms.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>

static GtkWidget *qtcCurrentActiveWindow=NULL;

static void qtcWindowCleanup(GtkWidget *widget)
{
    if (widget)
    {
        if(CUSTOM_BGND)
            g_signal_handler_disconnect(G_OBJECT(widget),
                                        (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_SIZE_REQUEST_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_STYLE_SET_ID"));               
        if((opts.menubarHiding&HIDE_KEYBOARD) || (opts.statusbarHiding&HIDE_KEYBOARD))
            g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_KEY_RELEASE_ID"));
        if((opts.menubarHiding&HIDE_KWIN) || (opts.statusbarHiding&HIDE_KWIN))
            g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_MAP_ID"));
        if(opts.shadeMenubarOnlyWhenActive || BLEND_TITLEBAR|| opts.menubarHiding || opts.statusbarHiding)
            g_signal_handler_disconnect(G_OBJECT(widget),
                                        (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_CLIENT_EVENT_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_HACK_SET");
    }
}

static gboolean qtcWindowStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcWindowCleanup(widget);
    return FALSE;
}

static GtkWidget * qtcWindowGetMenuBar(GtkWidget *parent, int level);
static gboolean    qtcWindowToggleMenuBar(GtkWidget *widget);
static gboolean    qtcWindowToggleStatusBar(GtkWidget *widget);

static gboolean qtcWindowClientEvent(GtkWidget *widget, GdkEventClient *event, gpointer user_data)
{
    if(gdk_x11_atom_to_xatom(event->message_type)==GDK_ATOM_TO_POINTER(gdk_x11_get_xatom_by_name_for_display(
                                                                        gtk_widget_get_display(widget), ACTIVE_WINDOW_ATOM)))
    {
        if(event->data.l[0])
            qtcCurrentActiveWindow=widget;
        else if(qtcCurrentActiveWindow==widget)
            qtcCurrentActiveWindow=0L;
        gtk_widget_queue_draw(widget);
    }
    else if(gdk_x11_atom_to_xatom(event->message_type)==GDK_ATOM_TO_POINTER(gdk_x11_get_xatom_by_name_for_display(
                                                                             gtk_widget_get_display(widget), TITLEBAR_SIZE_ATOM)))
    {
        qtcGetWindowBorderSize(TRUE);

        GtkWidget *menubar=qtcWindowGetMenuBar(widget, 0);

        if(menubar)
            gtk_widget_queue_draw(menubar);
    }
    else if(gdk_x11_atom_to_xatom(event->message_type)==GDK_ATOM_TO_POINTER(gdk_x11_get_xatom_by_name_for_display(
                                                                             gtk_widget_get_display(widget), TOGGLE_MENUBAR_ATOM)))
    {
        if(opts.menubarHiding&HIDE_KWIN && qtcWindowToggleMenuBar(widget))
            gtk_widget_queue_draw(widget);
    }
    else if(gdk_x11_atom_to_xatom(event->message_type)==GDK_ATOM_TO_POINTER(gdk_x11_get_xatom_by_name_for_display(
                                                                             gtk_widget_get_display(widget), TOGGLE_STATUSBAR_ATOM)))
    {
        if(opts.statusbarHiding&HIDE_KWIN && qtcWindowToggleStatusBar(widget))
            gtk_widget_queue_draw(widget);
    }

    return FALSE;
}

static gboolean qtcWindowDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcWindowCleanup(widget);
    return FALSE;
}

static gboolean qtcWindowIsActive(GtkWidget *widget)
{
    return widget && (gtk_window_is_active(GTK_WINDOW(widget)) || qtcCurrentActiveWindow==widget);
}

static gboolean qtcWindowSizeRequest(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    // Need to invalidate the whole of the window on a resize, as gradient needs to be redone.
    if(widget && (!(IS_FLAT_BGND(opts.bgndAppearance)) || IMG_NONE!=opts.bgndImage.type))
    {
        GdkRectangle  rect;
        GtkAllocation alloc=qtcWidgetGetAllocation(widget);

        rect.x=0;
        rect.y=0;

        if(IS_FLAT(opts.bgndAppearance) && IMG_NONE!=opts.bgndImage.type)
        {
            EPixPos pos=IMG_FILE==opts.bgndImage.type ? opts.bgndImage.pos : PP_TR;

            if(IMG_FILE==opts.bgndImage.type)
                loadBgndImage(&opts.bgndImage);

            switch(pos)
            {
                case PP_TL:
                    rect.width=opts.bgndImage.width+1;
                    rect.height=opts.bgndImage.height+1;
                    break;
                case PP_TM:
                case PP_TR:
                    rect.width=alloc.width;
                    rect.height=(IMG_FILE==opts.bgndImage.type ? opts.bgndImage.height : RINGS_HEIGHT(opts.bgndImage.type))+1;
                    break;
                case PP_LM:
                case PP_BL:
                    rect.width=opts.bgndImage.width+1;
                    rect.height=alloc.height;
                    break;
                case PP_CENTRED:
                case PP_BR:
                case PP_BM:
                case PP_RM:
                    rect.width=alloc.width;
                    rect.height=alloc.height;
                    break;
            }
            if(alloc.width<rect.width)
                rect.width=alloc.width;
            if(alloc.height<rect.height)
                rect.height=alloc.height;
        }
        else
            rect.width=alloc.width, rect.height=alloc.height;
        gdk_window_invalidate_rect(qtcWidgetGetWindow(widget), &rect, FALSE);
    }
    return FALSE;
}

static GtkWidget * qtcWindowGetMenuBar(GtkWidget *parent, int level)
{
    if(level<3 && GTK_IS_CONTAINER(parent))
    {
        GtkWidget *rv       = NULL;
        GList     *children = gtk_container_get_children(GTK_CONTAINER(parent)),
                  *child    = children;

        for(; child && !rv; child=child->next)
        {
            GtkWidget *boxChild=(GtkWidget *)child->data;

            if(GTK_IS_MENU_BAR(boxChild))
                rv=GTK_WIDGET(boxChild);
            else if(GTK_IS_CONTAINER(boxChild))
                rv=qtcWindowGetMenuBar(GTK_WIDGET(boxChild), level+1);
        }

        if(children)
            g_list_free(children);
        return rv;
    }

    return NULL;
}

static GtkWidget * qtcWindowGetStatusBar(GtkWidget *parent, int level)
{
    if(level<3 && GTK_IS_CONTAINER(parent))
    {
        GtkWidget *rv       = NULL;
        GList     *children = gtk_container_get_children(GTK_CONTAINER(parent)),
                  *child    = children;

        for(; child && !rv; child=child->next)
        {
            GtkWidget *boxChild=(GtkWidget *)child->data;

            if(GTK_IS_STATUSBAR(boxChild))
                rv=GTK_WIDGET(boxChild);
            else if(GTK_IS_CONTAINER(boxChild))
                rv=qtcWindowGetStatusBar(GTK_WIDGET(boxChild), level+1);
        }

        if(children)
            g_list_free(children);
        return rv;
    }

    return NULL;
}

static void qtcWindowMenuBarDBus(GtkWidget *widget, int size)
{
    GtkWindow    *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(widget));
    unsigned int xid=GDK_WINDOW_XID(qtcWidgetGetWindow(GTK_WIDGET(topLevel)));

    char         cmd[160];
    //sprintf(cmd, "qdbus org.kde.kwin /QtCurve menuBarSize %u %d", xid, size);
    sprintf(cmd, "dbus-send --type=method_call --session --dest=org.kde.kwin /QtCurve org.kde.QtCurve.menuBarSize uint32:%u int32:%d",
            xid, size);
    system(cmd);
    /*
    char         xidS[16],
                 sizeS[16];
    char         *args[]={"qdbus", "org.kde.kwin", "/QtCurve", "menuBarSize", xidS, sizeS, NULL};

    sprintf(xidS, "%u", xid);
    sprintf(sizeS, "%d", size);
    g_spawn_async("/tmp", args, NULL, (GSpawnFlags)0, NULL, NULL, NULL, NULL);
    */
}

static void qtcWindowStatusBarDBus(GtkWidget *widget, gboolean state)
{
    GtkWindow    *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(widget));
    unsigned int xid=GDK_WINDOW_XID(qtcWidgetGetWindow(GTK_WIDGET(topLevel)));

    char         cmd[160];
    //sprintf(cmd, "qdbus org.kde.kwin /QtCurve statusBarState %u %s", xid, state ? "true" : "false");
    sprintf(cmd, "dbus-send --type=method_call --session --dest=org.kde.kwin /QtCurve org.kde.QtCurve.statusBarState uint32:%u boolean:%s",
            xid, state ? "true" : "false");
    system(cmd);
    /*
    char         xidS[16],
                 stateS[6];
    char         *args[]={"qdbus", "org.kde.kwin", "/QtCurve", "statusBarState", xidS, stateS, NULL};

    sprintf(xidS, "%u", xid);
    sprintf(stateS, "%s", state ? "true" : "false");
    g_spawn_async("/tmp", args, NULL, (GSpawnFlags)0, NULL, NULL, NULL, NULL);
    */
}

static gboolean qtcWindowToggleMenuBar(GtkWidget *widget)
{
    GtkWidget *menuBar=qtcWindowGetMenuBar(widget, 0);

    if(menuBar)
    {
        int size=0;
        qtcSetMenuBarHidden(qtSettings.appName, GTK_WIDGET_VISIBLE(menuBar));
        if(GTK_WIDGET_VISIBLE(menuBar))
            gtk_widget_hide(menuBar);
        else
        {
            size=qtcWidgetGetAllocation(menuBar).height;
            gtk_widget_show(menuBar);
        }

        qtcEmitMenuSize(menuBar, size);
        qtcWindowMenuBarDBus(widget, size);

        return TRUE;
    }
    return FALSE;
}

static GtkWidget * qtcWindowGetStatusBar(GtkWidget *parent, int level);

static gboolean qtcWindowSetStatusBarProp(GtkWidget *w)
{
#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
    if(w &&!g_object_get_data(G_OBJECT(w), STATUSBAR_ATOM))
    {
        GtkWindow  *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(w));
        GdkDisplay *display=gtk_widget_get_display(GTK_WIDGET(topLevel));
    
        unsigned short setting=1;
        g_object_set_data(G_OBJECT(w), STATUSBAR_ATOM, (gpointer)1);
        XChangeProperty(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XID(qtcWidgetGetWindow(GTK_WIDGET(topLevel))),
                        gdk_x11_get_xatom_by_name_for_display(display, STATUSBAR_ATOM),
                        XA_CARDINAL, 16, PropModeReplace, (unsigned char *)&setting, 1);
        return TRUE;
    }
#endif
    return FALSE;
}

static gboolean qtcWindowToggleStatusBar(GtkWidget *widget)
{
    GtkWidget *statusBar=qtcWindowGetStatusBar(widget, 0);

    if(statusBar)
    {
        gboolean state=GTK_WIDGET_VISIBLE(statusBar);
        qtcSetStatusBarHidden(qtSettings.appName, state);
        if(state)
            gtk_widget_hide(statusBar);
        else
            gtk_widget_show(statusBar);

        qtcWindowStatusBarDBus(widget, state);
        return TRUE;
    }
    return FALSE;
}

static void qtcWindowSetProperties(GtkWidget *w, unsigned short opacity)
{
#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
    GtkWindow     *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(w));
    GdkDisplay    *display=gtk_widget_get_display(GTK_WIDGET(topLevel));
    unsigned long prop=(IS_FLAT_BGND(opts.bgndAppearance) ? (IMG_NONE!=opts.bgndImage.type ? APPEARANCE_RAISED : APPEARANCE_FLAT)
                                                          : opts.bgndAppearance)&0xFF;
    //GtkRcStyle    *rcStyle=gtk_widget_get_modifier_style(w);
    GdkColor      *bgnd=/*rcStyle ? &rcStyle->bg[GTK_STATE_NORMAL] : */&qtcPalette.background[ORIGINAL_SHADE];

    if(100!=opacity)
        XChangeProperty(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XID(qtcWidgetGetWindow(GTK_WIDGET(topLevel))),
                        gdk_x11_get_xatom_by_name_for_display(display, OPACITY_ATOM),
                        XA_CARDINAL, 16, PropModeReplace, (unsigned char *)&opacity, 1);

    prop|=((toQtColor(bgnd->red)&0xFF)<<24)|((toQtColor(bgnd->green)&0xFF)<<16)|((toQtColor(bgnd->blue)&0xFF)<<8);
    XChangeProperty(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XID(qtcWidgetGetWindow(GTK_WIDGET(topLevel))),
                    gdk_x11_get_xatom_by_name_for_display(display, BGND_ATOM),
                    XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&prop, 1);
#endif
}

static gboolean qtcWindowKeyRelease(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    if(GDK_CONTROL_MASK&event->state && GDK_MOD1_MASK&event->state && !event->is_modifier &&
       0==(event->state&0xFF00)) // Ensure only ctrl/alt/shift/capsLock are pressed...
    {
        gboolean toggled=FALSE;
        if(opts.menubarHiding&HIDE_KEYBOARD && (QTC_KEY_m==event->keyval || QTC_KEY_M==event->keyval))
            toggled=qtcWindowToggleMenuBar(widget);

        if(opts.statusbarHiding&HIDE_KEYBOARD && (QTC_KEY_s==event->keyval || QTC_KEY_S==event->keyval))
            toggled=qtcWindowToggleStatusBar(widget);

        if(toggled)
            gtk_widget_queue_draw(widget);
    }
    return FALSE;
}

static gboolean qtcWindowMap(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    int opacity=(int)g_object_get_data(G_OBJECT(widget), "QTC_WINDOW_OPACITY");
    qtcWindowSetProperties(widget, (unsigned short)opacity);

    if(opts.menubarHiding&HIDE_KWIN)
    {
        GtkWidget *menuBar=qtcWindowGetMenuBar(widget, 0);

        if(menuBar)
        {
            int size=GTK_WIDGET_VISIBLE(menuBar) ? qtcWidgetGetAllocation(menuBar).height : 0;

            qtcEmitMenuSize(menuBar, size);
            qtcWindowMenuBarDBus(widget, size);
        }
    }

    if(opts.statusbarHiding&HIDE_KWIN)
    {
        GtkWidget *statusBar=qtcWindowGetStatusBar(widget, 0);

        if(statusBar)
            qtcWindowStatusBarDBus(widget, !GTK_WIDGET_VISIBLE(statusBar));
    }
    return FALSE;
}

static gboolean qtcWindowSetup(GtkWidget *widget, int opacity)
{
    if (widget && !g_object_get_data(G_OBJECT(widget), "QTC_WINDOW_HACK_SET"))
    {
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_HACK_SET", (gpointer)1);
        if(!(IS_FLAT_BGND(opts.bgndAppearance)) || IMG_NONE!=opts.bgndImage.type)
            g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_SIZE_REQUEST_ID",
                              (gpointer)g_signal_connect(G_OBJECT(widget), "size-request", G_CALLBACK(qtcWindowSizeRequest), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event", G_CALLBACK(qtcWindowDestroy), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "style-set", G_CALLBACK(qtcWindowStyleSet), NULL));
        if((opts.menubarHiding&HIDE_KEYBOARD) || (opts.statusbarHiding&HIDE_KEYBOARD))
            g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_KEY_RELEASE_ID",
                              (gpointer)g_signal_connect(G_OBJECT(widget), "key-release-event", G_CALLBACK(qtcWindowKeyRelease), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_OPACITY", (gpointer)opacity);
        qtcWindowSetProperties(widget, (unsigned short)opacity);

        if((opts.menubarHiding&HIDE_KWIN) || (opts.statusbarHiding&HIDE_KWIN) || 100!=opacity)
            g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_MAP_ID",
                              (gpointer)g_signal_connect(G_OBJECT(widget), "map-event", G_CALLBACK(qtcWindowMap), NULL));
        if(opts.shadeMenubarOnlyWhenActive || BLEND_TITLEBAR || opts.menubarHiding || opts.statusbarHiding)
            g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_CLIENT_EVENT_ID",
                              (gpointer)g_signal_connect(G_OBJECT(widget), "client-event", G_CALLBACK(qtcWindowClientEvent), NULL));
        return TRUE;
    }

    return FALSE;
}
