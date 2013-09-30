#ifndef __QTC_GTK_COMMON_H__
#define __QTC_GTK_COMMON_H__

#include <gtk/gtk.h>

/*
  QtCurve (C) Yichao Yu, 2013-2013 yyc1992@gmail.com

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

static inline void
qtcConnectToData(GObject *obj, const char *name, const char *sig_name,
                 GCallback cb, gpointer data)
{
    g_object_set_data(obj, name, GINT_TO_POINTER(g_signal_connect(obj, sig_name,
                                                                  cb, data)));
}
#define qtcConnectToData(obj, name, sig_name, cb, data) \
    ((qtcConnectToData)(obj, name, sig_name, G_CALLBACK(cb), data))

static inline void
qtcDisconnectFromData(GObject *obj, const char *name)
{
    g_signal_handler_disconnect(
        obj, GPOINTER_TO_INT(g_object_steal_data(obj, name)));
}

#endif
