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

#include <qtcurve-utils/utils.h>
#include "qtcurve-gtk-common.h"
#include "compatability.h"

static GtkWidget *qtcEntryLastMo = NULL;

gboolean
qtcEntryIsLastMo(GtkWidget *widget)
{
    return qtcEntryLastMo && widget == qtcEntryLastMo;
}

static void
qtcEntryCleanup(GtkWidget *widget)
{
    if (qtcEntryLastMo == widget)
        qtcEntryLastMo = NULL;
    if (GTK_IS_ENTRY(widget)) {
        GObject *obj = G_OBJECT(widget);
        qtcDisconnectFromData(obj, "QTC_ENTRY_ENTER_ID");
        qtcDisconnectFromData(obj, "QTC_ENTRY_LEAVE_ID");
        qtcDisconnectFromData(obj, "QTC_ENTRY_DESTROY_ID");
        qtcDisconnectFromData(obj, "QTC_ENTRY_UNREALIZE_ID");
        qtcDisconnectFromData(obj, "QTC_ENTRY_STYLE_SET_ID");
        g_object_steal_data(obj, "QTC_ENTRY_HACK_SET");
    }
}

static gboolean
qtcEntryStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer data)
{
    QTC_UNUSED(previous_style);
    QTC_UNUSED(data);
    qtcEntryCleanup(widget);
    return FALSE;
}

static gboolean
qtcEntryDestroy(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    QTC_UNUSED(event);
    QTC_UNUSED(data);
    qtcEntryCleanup(widget);
    return FALSE;
}

static gboolean
qtcEntryEnter(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
    QTC_UNUSED(event);
    QTC_UNUSED(data);
    if (GTK_IS_ENTRY(widget)) {
        qtcEntryLastMo = widget;
        gtk_widget_queue_draw(widget);
    }
    return FALSE;
}

static gboolean
qtcEntryLeave(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
    QTC_UNUSED(event);
    QTC_UNUSED(data);
    if (GTK_IS_ENTRY(widget)) {
        qtcEntryLastMo = NULL;
        gtk_widget_queue_draw(widget);
    }
    return FALSE;
}

void qtcEntrySetup(GtkWidget *widget)
{
    GObject *obj = NULL;
    if (GTK_IS_ENTRY(widget) && (obj = G_OBJECT(widget)) &&
        !g_object_get_data(obj, "QTC_ENTRY_HACK_SET")) {
        g_object_set_data(G_OBJECT(widget), "QTC_ENTRY_HACK_SET", (gpointer)1);
        qtcConnectToData(obj, "QTC_ENTRY_ENTER_ID", "enter-notify-event",
                         qtcEntryEnter, NULL);
        qtcConnectToData(obj, "QTC_ENTRY_LEAVE_ID", "leave-notify-event",
                         qtcEntryLeave, NULL);
        qtcConnectToData(obj, "QTC_ENTRY_DESTROY_ID", "destroy-event",
                         qtcEntryDestroy, NULL);
        qtcConnectToData(obj, "QTC_ENTRY_UNREALIZE_ID", "unrealize",
                         qtcEntryDestroy, NULL);
        qtcConnectToData(obj, "QTC_ENTRY_STYLE_SET_ID", "style-set",
                         qtcEntryStyleSet, NULL);
    }
}
