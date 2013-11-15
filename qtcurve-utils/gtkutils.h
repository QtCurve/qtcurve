/*****************************************************************************
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

#ifndef __QTC_UTILS_GTK_UTILS_H__
#define __QTC_UTILS_GTK_UTILS_H__

#include "utils.h"
#include <gtk/gtk.h>

static inline void
qtcConnectToData(GObject *obj, const char *name, const char *sig_name,
                 GCallback cb, gpointer data)
{
    g_object_set_data(obj, name, GINT_TO_POINTER(g_signal_connect(obj, sig_name,
                                                                  cb, data)));
}

#define qtcConnectToData(obj, name, sig_name, cb, data)                 \
    ((qtcConnectToData)(obj, name, sig_name, G_CALLBACK(cb), data))

static inline void
qtcDisconnectFromData(GObject *obj, const char *name)
{
    g_signal_handler_disconnect(
        obj, GPOINTER_TO_INT(g_object_steal_data(obj, name)));
}

static inline GtkAllocation
qtcWidgetGetAllocation(GtkWidget *widget)
{
    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);
    return alloc;
}

static inline GtkRequisition
qtcWidgetGetRequisition(GtkWidget *widget)
{
    GtkRequisition req;
    gtk_widget_get_requisition(widget, &req);
    return req;
}

static inline gfloat
qtcFrameGetLabelYAlign(GtkFrame *f)
{
    gfloat x, y;
    gtk_frame_get_label_align(f, &x, &y);
    return y;
}

#endif
