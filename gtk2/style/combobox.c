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
#include "combobox.h"

/**
 * Setting appears-as-list on a non-editable combo creates a view over the
 * 'label' which is of 'base' colour. gtk_cell_view_set_background_color
 * removes this
 */
static bool
qtcComboBoxCellViewHasBgnd(GtkWidget *view)
{
    gboolean val;
    g_object_get(view, "background-set", &val, NULL);
    return val;
}

static void
qtcComboBoxClearBgndColor(GtkWidget *widget)
{
    GList *children = gtk_container_get_children(GTK_CONTAINER(widget));

    for (GList *child = children;child;child = child->next) {
        GtkWidget *boxChild = (GtkWidget*)child->data;

        if (GTK_IS_CELL_VIEW(boxChild) &&
            qtcComboBoxCellViewHasBgnd(boxChild)) {
            gtk_cell_view_set_background_color(GTK_CELL_VIEW(boxChild), 0L);
        }
    }

    if (children) {
        g_list_free(children);
    }
}

static GtkWidget *qtcComboFocus = NULL;
static GtkWidget *qtcComboHover = NULL;

bool
qtcComboBoxIsFocusChanged(GtkWidget *widget)
{
    if (qtcComboFocus == widget) {
        if (!gtk_widget_has_focus(widget)) {
            qtcComboFocus = NULL;
            return true;
        }
    } else if (gtk_widget_has_focus(widget)) {
        qtcComboFocus = widget;
        return true;
    }
    return false;
}

bool
qtcComboBoxHasFocus(GtkWidget *widget, GtkWidget *mapped)
{
    return gtk_widget_has_focus(widget) || (mapped && mapped == qtcComboFocus);
}

bool
qtcComboBoxIsHovered(GtkWidget *widget)
{
    return widget == qtcComboHover;
}

#if 0
static bool
qtcComboAppearsAsList(GtkWidget *widget)
{
    gboolean rv;
    gtk_widget_style_get(widget, "appears-as-list", &rv, NULL);
    return rv;
}
#endif

static void
qtcComboBoxCleanup(GtkWidget *widget)
{
    if (!widget) {
        return;
    }
    GObject *obj = G_OBJECT(widget);
    if (g_object_get_data(obj, "QTC_COMBO_BOX_SET")) {
        qtcDisconnectFromData(obj, "QTC_COMBO_BOX_DESTROY_ID");
        qtcDisconnectFromData(obj, "QTC_COMBO_BOX_UNREALIZE_ID");
        qtcDisconnectFromData(obj, "QTC_COMBO_BOX_STYLE_SET_ID");
        qtcDisconnectFromData(obj, "QTC_COMBO_BOX_ENTER_ID");
        qtcDisconnectFromData(obj, "QTC_COMBO_BOX_LEAVE_ID");
        qtcDisconnectFromData(obj, "QTC_COMBO_BOX_STATE_CHANGE_ID");
        g_object_steal_data(G_OBJECT(widget), "QTC_COMBO_BOX_SET");
    }
}

static gboolean
qtcComboBoxStyleSet(GtkWidget *widget, GtkStyle *prev_style, void *data)
{
    QTC_UNUSED(data);
    QTC_UNUSED(prev_style);
    qtcComboBoxCleanup(widget);
    return FALSE;
}

static gboolean
qtcComboBoxDestroy(GtkWidget *widget, GdkEvent *event, void *data)
{
    QTC_UNUSED(data);
    QTC_UNUSED(event);
    qtcComboBoxCleanup(widget);
    return FALSE;
}

static gboolean
qtcComboBoxEnter(GtkWidget *widget, GdkEventMotion *event, void *data)
{
    QTC_UNUSED(event);
    if (GTK_IS_EVENT_BOX(widget)) {
        GtkWidget *widget = (GtkWidget*)data;
        if (qtcComboHover != widget) {
            qtcComboHover = widget;
            gtk_widget_queue_draw(widget);
        }
    }
    return FALSE;
}

static gboolean
qtcComboBoxLeave(GtkWidget *widget, GdkEventMotion *event, void *data)
{
    QTC_UNUSED(event);
    if (GTK_IS_EVENT_BOX(widget)) {
        GtkWidget *widget = (GtkWidget*)data;
        if (qtcComboHover == widget) {
            qtcComboHover = NULL;
            gtk_widget_queue_draw(widget);
        }
    }
    return FALSE;
}

static void
qtcComboBoxStateChange(GtkWidget *widget, GdkEventMotion *event, void *data)
{
    QTC_UNUSED(data);
    QTC_UNUSED(event);
    if (GTK_IS_CONTAINER(widget)) {
        qtcComboBoxClearBgndColor(widget);
    }
}

void
qtcComboBoxSetup(GtkWidget *frame, GtkWidget *combo)
{
    if (!combo || (!frame && qtcComboHasFrame(combo))) {
        return;
    }
    GObject *combo_obj = G_OBJECT(combo);
    if (!g_object_get_data(combo_obj, "QTC_COMBO_BOX_SET")) {
        g_object_set_data(combo_obj, "QTC_COMBO_BOX_SET", GINT_TO_POINTER(1));
        qtcComboBoxClearBgndColor(combo);
        qtcConnectToData(combo_obj, "QTC_COMBO_BOX_STATE_CHANGE_ID",
                         "state-changed", qtcComboBoxStateChange, NULL);

        if (frame) {
            GList *children = gtk_container_get_children(GTK_CONTAINER(frame));
            for (GList *child = children;child;child = child->next) {
                GObject *boxChild = (GObject*)child->data;

                if (GTK_IS_EVENT_BOX(boxChild)) {
                    qtcConnectToData(boxChild, "QTC_COMBO_BOX_DESTROY_ID",
                                     "destroy-event", qtcComboBoxDestroy, NULL);
                    qtcConnectToData(boxChild, "QTC_COMBO_BOX_UNREALIZE_ID",
                                     "unrealize", qtcComboBoxDestroy, NULL);
                    qtcConnectToData(boxChild, "QTC_COMBO_BOX_STYLE_SET_ID",
                                     "style-set", qtcComboBoxStyleSet, NULL);
                    qtcConnectToData(boxChild, "QTC_COMBO_BOX_ENTER_ID",
                                     "enter-notify-event", qtcComboBoxEnter,
                                     combo);
                    qtcConnectToData(boxChild, "QTC_COMBO_BOX_LEAVE_ID",
                                     "leave-notify-event", qtcComboBoxLeave,
                                     combo);
                }
            }

            if (children) {
                g_list_free(children);
            }
        }
    }
}

bool
qtcComboHasFrame(GtkWidget *widget)
{
    gboolean val;
    g_object_get(widget, "has-frame", &val, NULL);
    return val;
}
