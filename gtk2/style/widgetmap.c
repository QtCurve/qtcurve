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

static GHashTable *qtcWidgetMapHashTable[2] = {NULL, NULL};

#define MAP_ID_X(ID_STR) "QTC_WIDGET_MAP_HACK_HACK_SET"ID_STR
#define MAP_ID(ID) (ID ? MAP_ID_X("1") : MAP_ID_X("0"))

static GtkWidget * qtcWidgetMapLookupHash(void *hash, void *value, int map)
{
    GtkWidget *rv=NULL;

    if(!qtcWidgetMapHashTable[map])
        qtcWidgetMapHashTable[map]=g_hash_table_new(g_direct_hash, g_direct_equal);

    rv=(GtkWidget *)g_hash_table_lookup(qtcWidgetMapHashTable[map], hash);

    if(!rv && value)
    {
        g_hash_table_insert(qtcWidgetMapHashTable[map], hash, value);
        rv=value;
    }

    return rv;
}

static void qtcWidgetMapRemoveHash(void *hash)
{
    for (int i = 0;i < 2;++i) {
        if (qtcWidgetMapHashTable[i]) {
            g_hash_table_remove(qtcWidgetMapHashTable[i], hash);
        }
    }
}

GtkWidget * qtcWidgetMapGetWidget(GtkWidget *widget, int map)
{
    return widget && g_object_get_data(G_OBJECT(widget), MAP_ID(map))
            ? qtcWidgetMapLookupHash(widget, NULL, map) : NULL;
}

static void qtcWidgetMapCleanup(GtkWidget *widget)
{
    GObject *obj = G_OBJECT(widget);
    if (g_object_get_data(obj, MAP_ID(0)) ||
        g_object_get_data(obj, MAP_ID(1))) {
        qtcDisconnectFromData(obj, "QTC_WIDGET_MAP_HACK_DESTROY_ID");
        qtcDisconnectFromData(obj, "QTC_WIDGET_MAP_HACK_UNREALIZE_ID");
        qtcDisconnectFromData(obj, "QTC_WIDGET_MAP_HACK_STYLE_SET_ID");
        g_object_steal_data(obj, MAP_ID(0));
        g_object_steal_data(obj, MAP_ID(1));
        qtcWidgetMapRemoveHash(widget);
    }
}

static gboolean
qtcWidgetMapStyleSet(GtkWidget *widget, GtkStyle *prev_style, void *data)
{
    QTC_UNUSED(prev_style);
    QTC_UNUSED(data);
    qtcWidgetMapCleanup(widget);
    return FALSE;
}

static gboolean
qtcWidgetMapDestroy(GtkWidget *widget, GdkEvent *event, void *user_data)
{
    QTC_UNUSED(event);
    QTC_UNUSED(user_data);
    qtcWidgetMapCleanup(widget);
    return FALSE;
}

void qtcWidgetMapSetup(GtkWidget *from, GtkWidget *to, int map)
{
    GObject *from_obj;
    if (from && to && (from_obj = G_OBJECT(from)) &&
        !g_object_get_data(from_obj, MAP_ID(map))) {
        g_object_set_data(from_obj, MAP_ID(map), (void*)1);
        qtcConnectToData(from_obj, "QTC_WIDGET_MAP_HACK_DESTROY_ID",
                         "destroy-event", qtcWidgetMapDestroy, NULL);
        qtcConnectToData(from_obj, "QTC_WIDGET_MAP_HACK_UNREALIZE_ID",
                         "unrealize", qtcWidgetMapDestroy, NULL);
        qtcConnectToData(from_obj, "QTC_WIDGET_MAP_HACK_STYLE_SET_ID",
                         "style-set", qtcWidgetMapStyleSet, NULL);
        qtcWidgetMapLookupHash(from, to, map);
    }
}
