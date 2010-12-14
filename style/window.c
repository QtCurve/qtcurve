/*
  QtCurve (C) Craig Drummond, 2003 - 2010 craig.p.drummond@gmail.com

  ----

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
 */

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <stdlib.h>
#include "compatability.h"
#include "common.h"
#include "config_file.h"
#include "qt_settings.h"
#include "window.h"
#include "menu.h"

extern Options opts;

static GtkWidget *qtcCurrentActiveWindow=NULL;

typedef struct
{
    int       width, 
              height,
              timer;
    GtkWidget *widget;
    gboolean  locked;
} QtCWindow;

static GHashTable *qtcWindowTable=NULL;

static QtCWindow * qtcWindowLookupHash(void *hash, gboolean create)
{
    QtCWindow *rv=NULL;

    if(!qtcWindowTable)
        qtcWindowTable=g_hash_table_new(g_direct_hash, g_direct_equal);

    rv=(QtCWindow *)g_hash_table_lookup(qtcWindowTable, hash);

    if(!rv && create)
    {
        rv=(QtCWindow *)malloc(sizeof(QtCWindow));
        rv->width=rv->height=rv->timer=0;
        rv->widget=NULL;
        rv->locked=FALSE;
        g_hash_table_insert(qtcWindowTable, hash, rv);
        rv=g_hash_table_lookup(qtcWindowTable, hash);
    }

    return rv;
}

static void qtcWindowRemoveFromHash(void *hash)
{
    if(qtcWindowTable)
    {
        QtCWindow *tv=qtcWindowLookupHash(hash, FALSE);
        if(tv)
        {
            if(tv->timer)
                g_source_remove(tv->timer);
            g_hash_table_remove(qtcWindowTable, hash);
        }
    }
}

static void qtcWindowCleanup(GtkWidget *widget)
{
    if (widget)
    {
        if(!(IS_FLAT_BGND(opts.bgndAppearance)) || IMG_NONE!=opts.bgndImage.type)
        {
            qtcWindowRemoveFromHash(widget);
            g_signal_handler_disconnect(G_OBJECT(widget),
                                        (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WINDOW_CONFIGURE_ID"));
        }
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

static gboolean qtcWindowToggleMenuBar(GtkWidget *widget);
static gboolean qtcWindowToggleStatusBar(GtkWidget *widget);

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

gboolean qtcWindowIsActive(GtkWidget *widget)
{
    return widget && (gtk_window_is_active(GTK_WINDOW(widget)) || qtcCurrentActiveWindow==widget);
}

static gboolean qtcWindowSizeRequest(GtkWidget *widget)
{
    if(widget && (!(IS_FLAT_BGND(opts.bgndAppearance)) || IMG_NONE!=opts.bgndImage.type))
    {
        GtkAllocation alloc=qtcWidgetGetAllocation(widget);
        GdkRectangle  rect;

        rect.x=0;
        rect.y=0;

        if(IS_FLAT(opts.bgndAppearance) && IMG_NONE!=opts.bgndImage.type)
        {
            EPixPos pos=IMG_FILE==opts.bgndImage.type ? opts.bgndImage.pos : PP_TR;

            if(IMG_FILE==opts.bgndImage.type)
                qtcLoadBgndImage(&opts.bgndImage);

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

static gboolean qtcWindowDelayedUpdate(gpointer user_data)
{
    QtCWindow *window=(QtCWindow *)user_data;
    
    if(window)
    {
        if(window->locked)
        {
            window->locked = FALSE;
            return TRUE;
        }
        else
        {
            g_source_remove(window->timer);
            window->timer=0;
            // otherwise, trigger update
            qtcWindowSizeRequest(window->widget);
            return FALSE;
        }
    }
    
    return FALSE;
}

static gboolean qtcWindowConfigure(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
{
    QtCWindow *window=(QtCWindow *)user_data;
    
    if(window && (event->width != window->width || event->height != window->height))
    {
        window->width = event->width;
        window->height = event->height;

        // schedule delayed timeOut
        if(!window->timer)
        {
            window->timer=g_timeout_add(50, (GSourceFunc)qtcWindowDelayedUpdate, window);
            window->locked = FALSE;
        }
        else
            window->locked = TRUE;
    }
    return FALSE;
}

GtkWidget * qtcWindowGetMenuBar(GtkWidget *parent, int level)
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

GtkWidget * qtcWindowGetStatusBar(GtkWidget *parent, int level)
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

void qtcWindowMenuBarDBus(GtkWidget *widget, int size)
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

void qtcWindowStatusBarDBus(GtkWidget *widget, gboolean state)
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
        qtcSetMenuBarHidden(qtSettings.appName, qtcWidgetVisible(menuBar));
        if(qtcWidgetVisible(menuBar))
            gtk_widget_hide(menuBar);
        else
        {
            size=qtcWidgetGetAllocation(menuBar).height;
            gtk_widget_show(menuBar);
        }

        qtcMenuEmitSize(menuBar, size);
        qtcWindowMenuBarDBus(widget, size);

        return TRUE;
    }
    return FALSE;
}

gboolean qtcWindowSetStatusBarProp(GtkWidget *w)
{
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
    return FALSE;
}

static gboolean qtcWindowToggleStatusBar(GtkWidget *widget)
{
    GtkWidget *statusBar=qtcWindowGetStatusBar(widget, 0);

    if(statusBar)
    {
        gboolean state=qtcWidgetVisible(statusBar);
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
            int size=qtcWidgetVisible(menuBar) ? qtcWidgetGetAllocation(menuBar).height : 0;

            qtcMenuEmitSize(menuBar, size);
            qtcWindowMenuBarDBus(widget, size);
        }
    }

    if(opts.statusbarHiding&HIDE_KWIN)
    {
        GtkWidget *statusBar=qtcWindowGetStatusBar(widget, 0);

        if(statusBar)
            qtcWindowStatusBarDBus(widget, !qtcWidgetVisible(statusBar));
    }
    return FALSE;
}

gboolean qtcWindowSetup(GtkWidget *widget, int opacity)
{
    if (widget && !g_object_get_data(G_OBJECT(widget), "QTC_WINDOW_HACK_SET"))
    {
        g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_HACK_SET", (gpointer)1);
        if(!(IS_FLAT_BGND(opts.bgndAppearance)) || IMG_NONE!=opts.bgndImage.type)
        {
            QtCWindow *window=qtcWindowLookupHash(widget, TRUE);
            if(window)
            {
                GtkAllocation alloc=qtcWidgetGetAllocation(widget);

                g_object_set_data(G_OBJECT(widget), "QTC_WINDOW_CONFIGURE_ID",
                                  (gpointer)g_signal_connect(G_OBJECT(widget), "configure-event", G_CALLBACK(qtcWindowConfigure), window));
                window->width=alloc.width;
                window->height=alloc.height;
                window->widget=widget;
            }
        }
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
