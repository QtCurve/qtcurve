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

#include "pixcache.h"
#include "check_on-png.h"
#include "check_x_on-png.h"
#include "blank16x16-png.h"
#include "qt_settings.h"

typedef struct {
    GdkColor col;
    EPixmap pix;
    double shade;
} QtcPixKey;

static GHashTable *_pixbufTable = NULL;

static guint
pixbufCacheHashKey(gconstpointer k)
{
    const QtcPixKey *key = k;
    int hash = ((key->pix << 24) + ((key->col.red >> 8) << 16) +
                ((key->col.green >> 8) << 8) + (key->col.blue >> 8));
    return g_int_hash(&hash);
}

static gboolean
pixbufCacheKeyEqual(gconstpointer k1, gconstpointer k2)
{
    const QtcPixKey *a = k1;
    const QtcPixKey *b = k2;

    return (a->pix == b->pix && a->col.red == b->col.red &&
            a->col.green == b->col.green && a->col.blue == b->col.blue);
}

static inline GHashTable*
getPixbufTable()
{
    if (!_pixbufTable) {
        _pixbufTable = g_hash_table_new_full(pixbufCacheHashKey,
                                             pixbufCacheKeyEqual,
                                             g_free, g_object_unref);
    }
    return _pixbufTable;
}

__attribute__((constructor)) static void
_qtcPixcacheInit()
{
    getPixbufTable();
}

__attribute__((destructor)) static void
_qtcPixcacheDone()
{
    if (_pixbufTable) {
        g_hash_table_destroy(_pixbufTable);
        _pixbufTable = NULL;
    }
}

static GdkPixbuf*
pixbufCacheValueNew(const QtcPixKey *key)
{
    GdkPixbuf *res = NULL;

    switch (key->pix) {
    case PIX_CHECK:
        res = gdk_pixbuf_new_from_inline(-1, opts.xCheck ? check_x_on :
                                         check_on, TRUE, NULL);
        break;
    case PIX_BLANK:
        return gdk_pixbuf_new_from_inline(-1, blank16x16, TRUE, NULL);
    }
    qtcAdjustPix(gdk_pixbuf_get_pixels(res), gdk_pixbuf_get_n_channels(res),
                 gdk_pixbuf_get_width(res), gdk_pixbuf_get_height(res),
                 gdk_pixbuf_get_rowstride(res),
                 key->col.red >> 8, key->col.green >> 8,
                 key->col.blue >> 8, key->shade);
    return res;
}

GdkPixbuf*
getPixbuf(GdkColor *widgetColor, EPixmap p, double shade)
{
    const QtcPixKey key = {
        .col = *widgetColor,
        .pix = p,
        .shade = shade
    };
    GHashTable *table = getPixbufTable();
    GdkPixbuf *pixbuf = g_hash_table_lookup(table, &key);
    if (pixbuf)
        return pixbuf;
    // TODO: Thread safe?
    pixbuf = pixbufCacheValueNew(&key);
    g_hash_table_insert(table, g_memdup(&key, sizeof(key)), pixbuf);
    return pixbuf;
}
