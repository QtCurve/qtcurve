/*****************************************************************************
 *   Copyright 2003 - 2011 Craig Drummond <craig.p.drummond@gmail.com>       *
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

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <common/common.h>
#include "qt_settings.h"
#include <qtcurve-utils/x11shadow.h>
#include <qtcurve-utils/gtkutils.h>

static guint realizeSignalId = 0;
static gulong realizeHookId = 0;

static void
installX11Shadows(GtkWidget* widget)
{
    if (DEBUG_ALL == qtSettings.debug)
        printf(DEBUG_PREFIX "%s\n", __FUNCTION__);
    GdkWindow *window = gtk_widget_get_window(widget);
    qtcX11ShadowInstall(GDK_WINDOW_XID(window));
}

static bool
acceptWidget(GtkWidget* widget)
{
    if (DEBUG_ALL == qtSettings.debug)
        printf(DEBUG_PREFIX "%s %p\n", __FUNCTION__, widget);

    if (widget && GTK_IS_WINDOW(widget)) {
        if (qtSettings.app == GTK_APP_OPEN_OFFICE) {
            return true;
        } else {
            GdkWindowTypeHint hint =
                gtk_window_get_type_hint(GTK_WINDOW(widget));
            if (DEBUG_ALL == qtSettings.debug)
                printf(DEBUG_PREFIX "%s %d\n", __FUNCTION__, (int)hint);
            return (hint == GDK_WINDOW_TYPE_HINT_MENU ||
                    hint == GDK_WINDOW_TYPE_HINT_DROPDOWN_MENU ||
                    hint == GDK_WINDOW_TYPE_HINT_POPUP_MENU ||
                    hint == GDK_WINDOW_TYPE_HINT_COMBO ||
                    hint == GDK_WINDOW_TYPE_HINT_TOOLTIP /* || */
                    /* (hint == GDK_WINDOW_TYPE_HINT_UTILITY && */
                    /*  !qtcWidgetGetParent(widget) && isMozilla()) */
                    /* // Firefox URL combo */);
        }
    }
    return false;
}

static gboolean
shadowDestroy(GtkWidget *widget, gpointer data)
{
    QTC_UNUSED(data);
    if (DEBUG_ALL == qtSettings.debug)
        printf(DEBUG_PREFIX "%s %p\n", __FUNCTION__, widget);

    GObject *obj = G_OBJECT(widget);
    if (g_object_get_data(obj, "QTC_SHADOW_SET")) {
        qtcDisconnectFromData(obj, "QTC_SHADOW_DESTROY_ID");
        g_object_steal_data(obj, "QTC_SHADOW_SET");
    }
    return FALSE;
}

static gboolean
registerWidget(GtkWidget* widget)
{
    if (DEBUG_ALL == qtSettings.debug)
        printf(DEBUG_PREFIX "%s %p\n", __FUNCTION__, widget);
    // check widget
    if (!(widget && GTK_IS_WINDOW(widget)))
        return FALSE;

    GObject *obj = G_OBJECT(widget);
    // make sure that widget is not already registered
    if (g_object_get_data(obj, "QTC_SHADOW_SET"))
        return FALSE;

    // check if window is accepted
    if (!acceptWidget(widget))
        return FALSE;

    // try install shadows
    installX11Shadows(widget);

    g_object_set_data(obj, "QTC_SHADOW_SET", (gpointer)1);
    qtcConnectToData(obj, "QTC_SHADOW_DESTROY_ID", "destroy",
                     shadowDestroy, NULL);
    return TRUE;
}

static gboolean
realizeHook(GSignalInvocationHint *sih, guint x, const GValue *params,
            gpointer data)
{
    QTC_UNUSED(sih);
    QTC_UNUSED(x);
    QTC_UNUSED(data);
    GtkWidget *widget = GTK_WIDGET(g_value_get_object(params));

    if (DEBUG_ALL == qtSettings.debug)
        printf(DEBUG_PREFIX "%s %p\n", __FUNCTION__, widget);

    if (!GTK_IS_WIDGET(widget))
        return FALSE;
    registerWidget(widget);
    return TRUE;
}

void qtcShadowInitialize()
{
#if !GTK_CHECK_VERSION(2, 12, 0)
    // Getting crashes with old Gtk and eclipse :-(
    if (GTK_APP_JAVA_SWT == qtSettings.app)
        return;
#endif
    if (DEBUG_ALL == qtSettings.debug)
        printf(DEBUG_PREFIX "%s %d\n", __FUNCTION__, qtSettings.app);
    if (!realizeSignalId) {
        realizeSignalId = g_signal_lookup("realize", GTK_TYPE_WIDGET);
        if (realizeSignalId) {
            realizeHookId = g_signal_add_emission_hook(
                realizeSignalId, (GQuark)0L, (GSignalEmissionHook)realizeHook,
                0, 0L);
        }
    }
}
