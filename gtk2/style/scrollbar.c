/*****************************************************************************
 *   Copyright 2003 - 2010 Craig Drummond <craig.p.drummond@gmail.com>       *
 *   Copyright 2013 - 2013 Yichao Yu <yyc1992@gmail.com>                     *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU Lesser General Public License as          *
 *   published by the Free Software Foundation; either version 2.1 of the    *
 *   License, or (at your option) version 3, or any later version accepted   *
 *   by the membership of KDE e.V. (or its successor approved by the         *
 *   membership of KDE e.V.), which shall act as a proxy defined in          *
 *   Section 6 of version 3 of the license.                                  *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 *   Lesser General Public License for more details.                         *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with this library. If not,                                *
 *   see <http://www.gnu.org/licenses/>.                                     *
 *****************************************************************************/

#include <qtcurve-utils/gtkutils.h>

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
qtcScrollbarStyleSet(GtkWidget *widget, GtkStyle *previous_style, void *data)
{
    QTC_UNUSED(previous_style);
    QTC_UNUSED(data);
    qtcScrollbarCleanup(widget);
    return FALSE;
}

static gboolean
qtcScrollbarDestroy(GtkWidget *widget, GdkEvent *event, void *data)
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

    while (parent && (parent = gtk_widget_get_parent(parent))) {
        if (GTK_IS_SCROLLED_WINDOW(parent)) {
            return GTK_SCROLLED_WINDOW(parent);
        }
    }
    return NULL;
}

static gboolean
qtcScrollbarValueChanged(GtkWidget *widget, GdkEventMotion *event, void *data)
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
        g_object_set_data(obj, "QTC_SCROLLBAR_SET", (void*)1);
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
