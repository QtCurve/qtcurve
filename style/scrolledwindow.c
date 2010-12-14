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
#include "compatability.h"
#include "common.h"

extern Options opts;

static void qtcScrolledWindowCleanup(GtkWidget *widget)
{
    if (widget && g_object_get_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_SET"))
    {
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_UNREALIZE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_STYLE_SET_ID"));
        if(ENTRY_MO)
        {
            g_signal_handler_disconnect(G_OBJECT(widget),
                                        (gint)g_object_steal_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_ENTER_ID"));
            g_signal_handler_disconnect(G_OBJECT(widget),
                                        (gint)g_object_steal_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_LEAVE_ID"));
        }
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_FOCUS_IN_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_FOCUS_OUT_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_SET");
    }
}

static gboolean qtcScrolledWindowStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcScrolledWindowCleanup(widget);
    return FALSE;
}

static gboolean qtcScrolledWindowDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcScrolledWindowCleanup(widget);
    return FALSE;
}

static GtkWidget *qtcScrolledWindowFocus=NULL;
static GtkWidget *qtcScrolledWindowHover=NULL;

gboolean qtcScrolledWindowHasFocus(GtkWidget *widget)
{
    return widget && (qtcWidgetHasFocus(widget) || widget==qtcScrolledWindowFocus);
}

gboolean qtcScrolledWindowHovered(GtkWidget *widget)
{
    return widget && (GTK_STATE_PRELIGHT==qtcWidgetGetState(widget) || widget==qtcScrolledWindowHover);
}

static gboolean qtcScrolledWindowEnter(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    GtkWidget *w=user_data ? (GtkWidget *)user_data : widget;
    if(GTK_IS_SCROLLED_WINDOW(w) && qtcScrolledWindowHover!=w)
    {
        qtcScrolledWindowHover=w;
        gtk_widget_queue_draw(w);
    }
    return FALSE;
}

static gboolean qtcScrolledWindowLeave(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    GtkWidget *w=user_data ? (GtkWidget *)user_data : widget;
    if(GTK_IS_SCROLLED_WINDOW(w) && qtcScrolledWindowHover==w)
    {
        qtcScrolledWindowHover=NULL;
        gtk_widget_queue_draw(w);
    }
    return FALSE;
}

static gboolean qtcScrolledWindowFocusIn(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    GtkWidget *w=user_data ? (GtkWidget *)user_data : widget;
    if(GTK_IS_SCROLLED_WINDOW(w) && qtcScrolledWindowFocus!=w)
    {
        qtcScrolledWindowFocus=w;
        gtk_widget_queue_draw(w);
    }
    return FALSE;
}

static gboolean qtcScrolledWindowFocusOut(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    GtkWidget *w=user_data ? (GtkWidget *)user_data : widget;
    if(GTK_IS_SCROLLED_WINDOW(w) && qtcScrolledWindowFocus==w)
    {
        qtcScrolledWindowFocus=NULL;
        gtk_widget_queue_draw(w);
    }
    return FALSE;
}

static void qtcScrolledWindowSetupConnections(GtkWidget *widget, GtkWidget *parent)
{
    if (widget && !g_object_get_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_SET"))
    {
        gtk_widget_add_events(widget, GDK_LEAVE_NOTIFY_MASK|GDK_ENTER_NOTIFY_MASK|GDK_FOCUS_CHANGE_MASK);
        g_object_set_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_SET", (gpointer)1);
        g_object_set_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event", G_CALLBACK(qtcScrolledWindowDestroy), parent));
        g_object_set_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_UNREALIZE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "unrealize", G_CALLBACK(qtcScrolledWindowDestroy), parent));
        g_object_set_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "style-set", G_CALLBACK(qtcScrolledWindowStyleSet), parent));
        if(ENTRY_MO)
        {
            g_object_set_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_ENTER_ID",
                              (gpointer)g_signal_connect(G_OBJECT(widget), "enter-notify-event", G_CALLBACK(qtcScrolledWindowEnter), parent));
            g_object_set_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_LEAVE_ID",
                             (gpointer)g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(qtcScrolledWindowLeave), parent));
        }
        g_object_set_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_FOCUS_IN_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "focus-in-event", G_CALLBACK(qtcScrolledWindowFocusIn), parent));
        g_object_set_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_FOCUS_OUT_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "focus-out-event", G_CALLBACK(qtcScrolledWindowFocusOut), parent));

        if(parent && ENTRY_MO)
        {
            gint          x, y;
            GtkAllocation alloc=qtcWidgetGetAllocation(parent);

            gdk_window_get_pointer(qtcWidgetGetWindow(parent), &x, &y, 0L);
            if(x>=0 && x<alloc.width && y>=0 && y<alloc.height)
                qtcScrolledWindowHover=parent;
        }
    }
}

void qtcScrolledWindowRegisterChild(GtkWidget *child)
{
    GtkWidget *parent=child ? qtcWidgetGetParent(child) : NULL;

    if(parent && GTK_IS_SCROLLED_WINDOW(parent) && g_object_get_data(G_OBJECT(parent), "QTC_SCROLLED_WINDOW_SET"))
        qtcScrolledWindowSetupConnections(child, parent);
}

void qtcScrolledWindowSetup(GtkWidget *widget)
{
    if (widget && GTK_IS_SCROLLED_WINDOW(widget) && !g_object_get_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_SET"))
    {
        GtkScrolledWindow *scrolledWindow=GTK_SCROLLED_WINDOW(widget);
        GtkWidget         *child;

        if((child=gtk_scrolled_window_get_hscrollbar(scrolledWindow)))
            qtcScrolledWindowSetupConnections(child, widget);
        if((child=gtk_scrolled_window_get_vscrollbar(scrolledWindow)))
            qtcScrolledWindowSetupConnections(child, widget);
        if((child=gtk_bin_get_child(GTK_BIN(widget))))
        {
            if(GTK_IS_TREE_VIEW(child) || GTK_IS_TEXT_VIEW(child) || GTK_IS_ICON_VIEW(child))
                qtcScrolledWindowSetupConnections(child, widget);
            else
            {
                const gchar *type=g_type_name(qtcWidgetType(child));

                if(type && (0==strcmp(type, "ExoIconView") || 0==strcmp(type, "FMIconContainer")))
                    qtcScrolledWindowSetupConnections(child, widget);
            }
        }

        g_object_set_data(G_OBJECT(widget), "QTC_SCROLLED_WINDOW_SET", (gpointer)1);
    }
}
