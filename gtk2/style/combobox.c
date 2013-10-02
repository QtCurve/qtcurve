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

#include <qtcurve-utils/gtkutils.h>
#include "combobox.h"
#include "compatability.h"

/**
 * Setting appears-as-list on a non-editable combo creates a view over the 'label' which
 * is of 'base' colour. gtk_cell_view_set_background_color removes this
 */
static gboolean
qtcComboBoxCellViewHasBgnd(GtkWidget *view)
{
    GValue val = {0};

    g_value_init(&val, G_TYPE_BOOLEAN);
    g_object_get_property(G_OBJECT(view), "background-set", &val);
    return g_value_get_boolean(&val);
}

static void
qtcComboBoxClearBgndColor(GtkWidget *widget)
{
    GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
    GList *child = children;

    for (;child;child=child->next) {
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

gboolean
qtcComboBoxIsFocusChanged(GtkWidget *widget)
{
    if (qtcComboFocus == widget) {
        if (!gtk_widget_has_focus(widget)) {
            qtcComboFocus = NULL;
            return TRUE;
        }
    } else if (gtk_widget_has_focus(widget)) {
        qtcComboFocus = widget;
        return TRUE;
    }
    return FALSE;
}

gboolean
qtcComboBoxHasFocus(GtkWidget *widget, GtkWidget *mapped)
{
    return gtk_widget_has_focus(widget) || (mapped && mapped == qtcComboFocus);
}

gboolean
qtcComboBoxIsHovered(GtkWidget *widget)
{
    return widget == qtcComboHover;
}

/* static gboolean */
/* qtcComboAppearsAsList(GtkWidget *widget) */
/* { */
/*     gboolean rv; */
/*     gtk_widget_style_get(widget, "appears-as-list", &rv, NULL); */
/*     return rv; */
/* } */

static void
qtcComboBoxCleanup(GtkWidget *widget)
{
    if (!widget)
        return;
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
qtcComboBoxStyleSet(GtkWidget *widget, GtkStyle *prev_style, gpointer data)
{
    QTC_UNUSED(data);
    QTC_UNUSED(prev_style);
    qtcComboBoxCleanup(widget);
    return FALSE;
}

static gboolean
qtcComboBoxDestroy(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    QTC_UNUSED(data);
    QTC_UNUSED(event);
    qtcComboBoxCleanup(widget);
    return FALSE;
}

static gboolean
qtcComboBoxEnter(GtkWidget *widget, GdkEventMotion *event, gpointer data)
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
qtcComboBoxLeave(GtkWidget *widget, GdkEventMotion *event, gpointer data)
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
qtcComboBoxStateChange(GtkWidget *widget, GdkEventMotion *event, gpointer data)
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
    if (!combo || (!frame && qtcComboHasFrame(combo)))
        return;
    GObject *combo_obj = G_OBJECT(combo);
    if (!g_object_get_data(combo_obj, "QTC_COMBO_BOX_SET")) {
        g_object_set_data(combo_obj, "QTC_COMBO_BOX_SET", GINT_TO_POINTER(1));
        qtcComboBoxClearBgndColor(combo);
        qtcConnectToData(combo_obj, "QTC_COMBO_BOX_STATE_CHANGE_ID",
                         "state-changed", qtcComboBoxStateChange, NULL);

        if (frame) {
            GList *children = gtk_container_get_children(GTK_CONTAINER(frame));
            GList *child = children;

            for (;child;child = child->next) {
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

            if(children) {
                g_list_free(children);
            }
        }
    }
}

gboolean
qtcComboHasFrame(GtkWidget *widget)
{
    GValue val = {0};
    g_value_init(&val, G_TYPE_BOOLEAN);
    g_object_get_property(G_OBJECT(widget), "has-frame", &val);
    return g_value_get_boolean(&val);
}
