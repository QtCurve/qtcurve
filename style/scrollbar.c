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

#include "common.h"
#include "qtcurve-gtk-common.h"
#include "compatability.h"

static void
qtcScrollbarCleanup(GtkWidget *widget)
{
    GObject *obj = NULL;
    if (widget && (obj = G_OBJECT(widget)) &&
        g_object_get_data(obj, "QTC_SCROLLBAR_SET")) {
        qtcDisconnectFromData(obj, "QTC_SCROLLBAR_DESTROY_ID");
        qtcDisconnectFromData(obj, "QTC_SCROLLBAR_UNREALIZE_ID");
        qtcDisconnectFromData(obj, "QTC_SCROLLBAR_STYLE_SET_ID");
        qtcDisconnectFromData(obj, "QTC_SCROLLBAR_VALUE_CHANGED_ID");
        g_object_steal_data(obj, "QTC_SCROLLBAR_SET");
    }
}

static gboolean
qtcScrollbarStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer data)
{
    QTC_UNUSED(previous_style);
    QTC_UNUSED(data);
    qtcScrollbarCleanup(widget);
    return FALSE;
}

static gboolean
qtcScrollbarDestroy(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    QTC_UNUSED(event);
    QTC_UNUSED(data);
    qtcScrollbarCleanup(widget);
    return FALSE;
}

static GtkScrolledWindow*
qtcScrollbarParentScrolledWindow(GtkWidget *widget)
{
    GtkWidget *parent = widget;

    while (parent && (parent = qtcWidgetGetParent(parent))) {
        if (GTK_IS_SCROLLED_WINDOW(parent)) {
            return GTK_SCROLLED_WINDOW(parent);
        }
    }
    return NULL;
}

static gboolean
qtcScrollbarValueChanged(GtkWidget *widget, GdkEventMotion *event,
                         gpointer data)
{
    QTC_UNUSED(event);
    QTC_UNUSED(data);
    if (GTK_IS_SCROLLBAR(widget)) {
        GtkScrolledWindow *sw = qtcScrollbarParentScrolledWindow(widget);

        if (sw) {
            gtk_widget_queue_draw(GTK_WIDGET(sw));
        }
    }
    return FALSE;
}

static void
qtcScrollbarSetupSlider(GtkWidget *widget)
{
    GObject *obj = NULL;
    if (widget && (obj = G_OBJECT(widget)) &&
        !g_object_get_data(obj, "QTC_SCROLLBAR_SET")) {
        g_object_set_data(obj, "QTC_SCROLLBAR_SET", (gpointer)1);
        qtcConnectToData(obj, "QTC_SCROLLBAR_DESTROY_ID", "destroy-event",
                         qtcScrollbarDestroy, NULL);
        qtcConnectToData(obj, "QTC_SCROLLBAR_UNREALIZE_ID", "unrealize",
                         qtcScrollbarDestroy, NULL);
        qtcConnectToData(obj, "QTC_SCROLLBAR_STYLE_SET_ID", "style-set",
                         qtcScrollbarStyleSet, NULL);
        qtcConnectToData(obj, "QTC_SCROLLBAR_VALUE_CHANGED_ID", "value-changed",
                         qtcScrollbarValueChanged, NULL);
    }
}

void
qtcScrollbarSetup(GtkWidget *widget)
{
    GtkScrolledWindow *sw = qtcScrollbarParentScrolledWindow(widget);

    if (sw) {
        GtkWidget *slider;

        if ((slider = gtk_scrolled_window_get_hscrollbar(sw))) {
            qtcScrollbarSetupSlider(slider);
        }
        if ((slider = gtk_scrolled_window_get_vscrollbar(sw))) {
            qtcScrollbarSetupSlider(slider);
        }
    }
}
