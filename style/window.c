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
        GdkRectangle rect;

        rect.x=0;
        rect.y=0;
        rect.width=widget->allocation.width;

        if(IS_FLAT(opts.bgndAppearance) && IMG_NONE!=opts.bgndImage.type)
        {
            if(IMG_FILE==opts.bgndImage.type)
                loadBgndImage(&opts.bgndImage);
            rect.height=(IMG_FILE==opts.bgndImage.type ? opts.bgndImage.height : RINGS_HEIGHT(opts.bgndImage.type))+1;
            if(widget->allocation.height<rect.height)
                rect.height=widget->allocation.height;
        }
        else
            rect.height=widget->allocation.height;
        gdk_window_invalidate_rect(widget->window, &rect, FALSE);
    }
    return FALSE;
}

static GtkWidget * qtcWindowGetMenuBar(GtkWidget *parent, int level)
{
    if(level<3 && GTK_IS_CONTAINER(parent))
    {
        GList *child=gtk_container_get_children(GTK_CONTAINER(parent));

        for(; child; child=child->next)
        {
            GtkBoxChild *boxChild=(GtkBoxChild *)child->data;

            if(GTK_IS_MENU_BAR(boxChild))
                return GTK_WIDGET(boxChild);
            else if(GTK_IS_CONTAINER(boxChild))
            {
                GtkWidget *w=qtcWindowGetMenuBar(GTK_WIDGET(boxChild), level+1);
                if(w)
                    return w;
            }
        }
    }

    return NULL;
}

static GtkWidget * qtcWindowGetStatusBar(GtkWidget *parent, int level)
{
    if(level<3 && GTK_IS_CONTAINER(parent))
    {
        GList *child=gtk_container_get_children(GTK_CONTAINER(parent));

        for(; child; child=child->next)
        {
            GtkBoxChild *boxChild=(GtkBoxChild *)child->data;

            if(GTK_IS_STATUSBAR(boxChild))
                return GTK_WIDGET(boxChild);
            else if(GTK_IS_CONTAINER(boxChild))
            {
                GtkWidget *w=qtcWindowGetStatusBar(GTK_WIDGET(boxChild), level+1);
                if(w)
                    return w;
            }
        }
    }

    return NULL;
}

static void qtcWindowMenuBarDBus(GtkWidget *widget, int size)
{
    GtkWindow    *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(widget));
    unsigned int xid=GDK_WINDOW_XID(GTK_WIDGET(topLevel)->window);

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
    unsigned int xid=GDK_WINDOW_XID(GTK_WIDGET(topLevel)->window);

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
            size=menuBar->allocation.height;
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
    if(w &&!g_object_get_data(G_OBJECT(w), STATUSBAR_ATOM))
    {
        GtkWindow  *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(w));
        GdkDisplay *display=gtk_widget_get_display(GTK_WIDGET(topLevel));
    
        unsigned short setting=1;
        g_object_set_data(G_OBJECT(w), STATUSBAR_ATOM, (gpointer)1);
        XChangeProperty(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XID(GTK_WIDGET(topLevel)->window),
                        gdk_x11_get_xatom_by_name_for_display(display, STATUSBAR_ATOM),
                        XA_CARDINAL, 16, PropModeReplace, (unsigned char *)&setting, 1);
        return TRUE;
    }

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
    GtkWindow      *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(w));
    GdkDisplay     *display=gtk_widget_get_display(GTK_WIDGET(topLevel));

    if(100!=opacity)
        XChangeProperty(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XID(GTK_WIDGET(topLevel)->window),
                        gdk_x11_get_xatom_by_name_for_display(display, OPACITY_ATOM),
                        XA_CARDINAL, 16, PropModeReplace, (unsigned char *)&opacity, 1);

    if(!IS_FLAT_BGND(opts.bgndAppearance) || BGND_IMG_ON_BORDER)
    {
        unsigned short app=IS_FLAT_BGND(opts.bgndAppearance) ? APPEARANCE_RAISED : opts.bgndAppearance;
        XChangeProperty(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XID(GTK_WIDGET(topLevel)->window),
                        gdk_x11_get_xatom_by_name_for_display(display, BGND_ATOM),
                        XA_CARDINAL, 16, PropModeReplace, (unsigned char *)&app, 1);
    }
}

static gboolean qtcWindowKeyRelease(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    if(GDK_CONTROL_MASK&event->state && GDK_MOD1_MASK&event->state && !event->is_modifier &&
       0==(event->state&0xFF00)) // Ensure only ctrl/alt/shift/capsLock are pressed...
    {
        gboolean toggled=FALSE;
        if(opts.menubarHiding&HIDE_KEYBOARD && (GDK_m==event->keyval || GDK_M==event->keyval))
            toggled=qtcWindowToggleMenuBar(widget);

        if(opts.statusbarHiding&HIDE_KEYBOARD && (GDK_s==event->keyval || GDK_S==event->keyval))
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
            int size=GTK_WIDGET_VISIBLE(menuBar) ? menuBar->allocation.height : 0;

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
                              (gpointer)g_signal_connect(G_OBJECT(widget), "size-request",
                                                        (GtkSignalFunc)qtcWindowSizeRequest, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event",
                                                     (GtkSignalFunc)qtcWindowDestroy, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "style-set",
                                                     (GtkSignalFunc)qtcWindowStyleSet, NULL));
        if((opts.menubarHiding&HIDE_KEYBOARD) || (opts.statusbarHiding&HIDE_KEYBOARD))
            g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_KEY_RELEASE_ID",
                              (gpointer)g_signal_connect(G_OBJECT(widget), "key-release-event",
                                                         (GtkSignalFunc)qtcWindowKeyRelease, NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_OPACITY", (gpointer)opacity);
        qtcWindowSetProperties(widget, (unsigned short)opacity);

        if((opts.menubarHiding&HIDE_KWIN) || (opts.statusbarHiding&HIDE_KWIN) || 100!=opacity)
            g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_MAP_ID",
                              (gpointer)g_signal_connect(G_OBJECT(widget), "map-event",
                                                         (GtkSignalFunc)qtcWindowMap, NULL));
        if(opts.shadeMenubarOnlyWhenActive || BLEND_TITLEBAR || opts.menubarHiding || opts.statusbarHiding)
            g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_CLIENT_EVENT_ID",
                              (gpointer)g_signal_connect(G_OBJECT(widget), "client-event",
                                                         (GtkSignalFunc)qtcWindowClientEvent, NULL));
        return TRUE;
    }

    return FALSE;
}
