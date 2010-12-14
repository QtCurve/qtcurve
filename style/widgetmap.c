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
    int i;

    for(i=0; i<2; ++i)
        if(qtcWidgetMapHashTable[i])
            g_hash_table_remove(qtcWidgetMapHashTable[i], hash);
}

GtkWidget * qtcWidgetMapGetWidget(GtkWidget *widget, int map)
{
    return widget && g_object_get_data(G_OBJECT(widget), MAP_ID(map))
            ? qtcWidgetMapLookupHash(widget, NULL, map) : NULL;
}

static void qtcWidgetMapCleanup(GtkWidget *widget)
{
    if(g_object_get_data(G_OBJECT(widget), MAP_ID(0)) || g_object_get_data(G_OBJECT(widget), MAP_ID(1)))
    {
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WIDGET_MAP_HACK_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WIDGET_MAP_HACK_UNREALIZE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WIDGET_MAP_HACK_STYLE_SET_ID"));
        g_object_steal_data(G_OBJECT(widget), MAP_ID(0));
        g_object_steal_data(G_OBJECT(widget), MAP_ID(1));
        qtcWidgetMapRemoveHash(widget);
    }
}

static gboolean qtcWidgetMapStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcWidgetMapCleanup(widget);
    return FALSE;
}
 
static gboolean qtcWidgetMapDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcWidgetMapCleanup(widget);
    return FALSE;
}

void qtcWidgetMapSetup(GtkWidget *from, GtkWidget *to, int map)
{
    if (from && to && !g_object_get_data(G_OBJECT(from), MAP_ID(map)))
    {
        g_object_set_data(G_OBJECT(from), MAP_ID(map), (gpointer)1);
        g_object_set_data(G_OBJECT(from), "QTC_WIDGET_MAP_HACK_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(from), "destroy-event", G_CALLBACK(qtcWidgetMapDestroy), NULL));
        g_object_set_data(G_OBJECT(from), "QTC_WIDGET_MAP_HACK_UNREALIZE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(from), "unrealize", G_CALLBACK(qtcWidgetMapDestroy), NULL));
        g_object_set_data(G_OBJECT(from), "QTC_WIDGET_MAP_HACK_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(from), "style-set", G_CALLBACK(qtcWidgetMapStyleSet), NULL));
        qtcWidgetMapLookupHash(from, to, map);
    }  
}
