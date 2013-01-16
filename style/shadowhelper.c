/*
  QtCurve (C) Craig Drummond, 2003 - 2011 craig.p.drummond@gmail.com

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

#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <cairo-xlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include "common.h"
#include "qt_settings.h"
#include "shadow.h"
#include "compatability.h"

#define NUM_SHADOW_PIXMAPS 8
static unsigned long shadowPixmaps[NUM_SHADOW_PIXMAPS];
static int shadowSize=0;
static guint realizeSignalId=0;
static gulong realizeHookId=0;
static Atom shadowAtom=0;

static Pixmap createPixmap(const guint8 *pix)
{
    GdkPixbuf *pixbuf=gdk_pixbuf_new_from_inline(-1, pix, TRUE, NULL);
    
    if(pixbuf)
    {
        shadowSize=gdk_pixbuf_get_width(pixbuf);

        GdkScreen       *screen  = gdk_screen_get_default();
        Display         *display = GDK_DISPLAY_XDISPLAY(gdk_screen_get_display(screen));
        Window          root     = GDK_WINDOW_XID(gdk_screen_get_root_window(screen));
        Pixmap          pixmap   = XCreatePixmap(display, root, shadowSize, shadowSize, 32);
        cairo_surface_t *dest    = cairo_xlib_surface_create(display, pixmap, GDK_VISUAL_XVISUAL(gdk_screen_get_rgba_visual(screen)),
                                                             shadowSize, shadowSize);
        cairo_t         *cr      = cairo_create(dest);

        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
        cairo_rectangle(cr, 0, 0, shadowSize, shadowSize);
        cairo_fill(cr);
        cairo_destroy(cr);
        cairo_surface_destroy(dest);
        g_object_unref(pixbuf);
        return pixmap;
    }

    return 0;
}

static gboolean createPixmapHandles()
{
    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s\n", __FUNCTION__);

    // create atom
    if(!shadowAtom)
    {
        GdkScreen *screen = gdk_screen_get_default();
        Display   *display = screen ? GDK_DISPLAY_XDISPLAY(gdk_screen_get_display(screen)) : 0;

        if(!display)
        {
            return FALSE;
        }

        shadowAtom = XInternAtom(display, "_KDE_NET_WM_SHADOW", False);
    }

    if(0==shadowSize)
    {
        GdkScreen* screen = gdk_screen_get_default();
        if(gdk_screen_get_rgba_visual(screen))
        {
            shadowPixmaps[0]=createPixmap(shadow0);
            shadowPixmaps[1]=createPixmap(shadow1);
            shadowPixmaps[2]=createPixmap(shadow2);
            shadowPixmaps[3]=createPixmap(shadow3);
            shadowPixmaps[4]=createPixmap(shadow4);
            shadowPixmaps[5]=createPixmap(shadow5);
            shadowPixmaps[6]=createPixmap(shadow6);
            shadowPixmaps[7]=createPixmap(shadow7);
        }
    }
    
    return TRUE;
}

static void installX11Shadows(GtkWidget* widget)
{
    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s\n", __FUNCTION__);

    if(createPixmapHandles())
    {
        GdkWindow  *window = gtk_widget_get_window(widget);
        GdkDisplay *display = gtk_widget_get_display(widget);

        unsigned long data[NUM_SHADOW_PIXMAPS+4];
        
        memcpy(&data, &shadowPixmaps, sizeof(unsigned long)*NUM_SHADOW_PIXMAPS);
        data[NUM_SHADOW_PIXMAPS]=data[NUM_SHADOW_PIXMAPS+1]=data[NUM_SHADOW_PIXMAPS+2]=data[NUM_SHADOW_PIXMAPS+3]=shadowSize-4;

        XChangeProperty(
            GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XID(window), shadowAtom, XA_CARDINAL, 32, PropModeReplace,
            (unsigned char *)(&data[0]), (NUM_SHADOW_PIXMAPS+4));
    }
}

static gboolean acceptWidget(GtkWidget* widget)
{
    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %X\n", __FUNCTION__, (int)widget);

    if(widget && GTK_IS_WINDOW(widget))
    {
        if(GTK_APP_OPEN_OFFICE==qtSettings.app)
            return TRUE;
        else
        {
            GdkWindowTypeHint hint=gtk_window_get_type_hint(GTK_WINDOW(widget));
            if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d\n", __FUNCTION__, (int)hint);
            return
                hint == GDK_WINDOW_TYPE_HINT_MENU ||
                hint == GDK_WINDOW_TYPE_HINT_DROPDOWN_MENU ||
                hint == GDK_WINDOW_TYPE_HINT_POPUP_MENU ||
                hint == GDK_WINDOW_TYPE_HINT_COMBO ||
                hint == GDK_WINDOW_TYPE_HINT_TOOLTIP ||
                (hint == GDK_WINDOW_TYPE_HINT_UTILITY && !qtcWidgetGetParent(widget) && isMozilla()) ; // Firefox URL combo
        }
    }
    return FALSE;
}

static gboolean shadowDestroy(GtkWidget* widget, gpointer data)
{
    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %X\n", __FUNCTION__, (int)widget);
    
    if (g_object_get_data(G_OBJECT(widget), "QTC_SHADOW_SET"))
    {
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_SHADOW_DESTROY_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_SHADOW_SET");
    }
    return FALSE;
}

static gboolean registerWidget(GtkWidget* widget)
{
    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %X\n", __FUNCTION__, (int)widget);
    // check widget
    if(!(widget && GTK_IS_WINDOW(widget))) return FALSE;

    // make sure that widget is not already registered
    if(g_object_get_data(G_OBJECT(widget), "QTC_SHADOW_SET")) return FALSE;

    // check if window is accepted
    if(!acceptWidget(widget)) return FALSE;

    // try install shadows
    installX11Shadows(widget);

    g_object_set_data(G_OBJECT(widget), "QTC_SHADOW_SET", (gpointer)1);
    g_object_set_data(G_OBJECT(widget), "QTC_SHADOW_DESTROY_ID",
                      (gpointer)g_signal_connect(G_OBJECT(widget), "destroy", G_CALLBACK(shadowDestroy), NULL));

    return TRUE;
}

static gboolean realizeHook(GSignalInvocationHint *sih, guint x, const GValue* params, gpointer data)
{
    GtkWidget* widget=GTK_WIDGET(g_value_get_object(params));

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %X\n", __FUNCTION__, (int)widget);

    if(!GTK_IS_WIDGET(widget)) return FALSE;
    registerWidget(widget);
    return TRUE;
}

void qtcShadowInitialize()
{
#if !GTK_CHECK_VERSION(2, 12, 0)
    if(GTK_APP_JAVA_SWT==qtSettings.app) return;  // Getting crashes with old Gtk and eclipse :-(
#endif
    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d\n", __FUNCTION__, qtSettings.app);
    if(!realizeSignalId)
    {
        realizeSignalId = g_signal_lookup("realize", GTK_TYPE_WIDGET);
        if(realizeSignalId)
            realizeHookId = g_signal_add_emission_hook(realizeSignalId, (GQuark)0L, (GSignalEmissionHook)realizeHook, 0, 0L);
    }
}
