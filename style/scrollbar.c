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

static void qtcScrollbarCleanup(GtkWidget *widget)
{
    if (widget && g_object_get_data(G_OBJECT(widget), "QTC_SCROLLBAR_SET"))
    {
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_SCROLLBAR_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_SCROLLBAR_UNREALIZE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_SCROLLBAR_STYLE_SET_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_SCROLLBAR_VALUE_CHANGED_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_SCROLLBAR_SET");
    }
}

static gboolean qtcScrollbarStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcScrollbarCleanup(widget);
    return FALSE;
}

static gboolean qtcScrollbarDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcScrollbarCleanup(widget);
    return FALSE;
}

static GtkScrolledWindow * qtcScrollbarParentScrolledWindow(GtkWidget *widget)
{
    GtkWidget *parent=widget;

    while(parent && (parent=qtcWidgetGetParent(parent)))
    {
        if(GTK_IS_SCROLLED_WINDOW(parent))
            return GTK_SCROLLED_WINDOW(parent);
    }
    
    return NULL;
}
    
static gboolean qtcScrollbarValueChanged(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    if(GTK_IS_SCROLLBAR(widget))
    {
        GtkScrolledWindow *sw=qtcScrollbarParentScrolledWindow(widget);

        if(sw)
            gtk_widget_queue_draw(GTK_WIDGET(sw));
    }
    return FALSE;
}

static void qtcScrollbarSetupSlider(GtkWidget *widget)
{
    if (widget && !g_object_get_data(G_OBJECT(widget), "QTC_SCROLLBAR_SET"))
    {
        g_object_set_data(G_OBJECT(widget), "QTC_SCROLLBAR_SET", (gpointer)1);
        g_object_set_data(G_OBJECT(widget), "QTC_SCROLLBAR_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event", G_CALLBACK(qtcScrollbarDestroy), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_SCROLLBAR_UNREALIZE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "unrealize", G_CALLBACK(qtcScrollbarDestroy), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_SCROLLBAR_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "style-set", G_CALLBACK(qtcScrollbarStyleSet), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_SCROLLBAR_VALUE_CHANGED_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(qtcScrollbarValueChanged), NULL));
    }
}

void qtcScrollbarSetup(GtkWidget *widget)
{
    GtkScrolledWindow *sw=qtcScrollbarParentScrolledWindow(widget);

    if(sw)
    {
        GtkWidget *slider;

        if((slider=gtk_scrolled_window_get_hscrollbar(sw)))
            qtcScrollbarSetupSlider(slider);
        if((slider=gtk_scrolled_window_get_vscrollbar(sw)))
            qtcScrollbarSetupSlider(slider);
    }
}
