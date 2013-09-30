/***************************************************************************
 *   Copyright (C) 2013~2013 by Yichao Yu                                  *
 *   yyc1992@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include "kdex11shadow_p.h"
#include <xcb/xcb_image.h>
#include <data/shadow0-png.h>
#include <data/shadow1-png.h>
#include <data/shadow2-png.h>
#include <data/shadow3-png.h>
#include <data/shadow4-png.h>
#include <data/shadow5-png.h>
#include <data/shadow6-png.h>
#include <data/shadow7-png.h>

/*!
  shadow atom and property specification available at
  http://community.kde.org/KWin/Shadow
*/

static uint32_t shadow_pixmaps[8];
static xcb_atom_t shadow_atom;
static int shadow_size = 0;

static xcb_pixmap_t
qtc_shadow_create_pixmap(const QtcPixmap *data)
{
    xcb_pixmap_t pixmap = qtc_x11_generate_id();
    shadow_size = data->width;

    // create X11 pixmap
    qtc_x11_call_void(create_pixmap, 32, pixmap, qtc_x11_root_window(),
                      data->width, data->height);
    xcb_gcontext_t cid = qtc_x11_generate_id();
    qtc_x11_call_void(create_gc, cid, pixmap, 0, (const uint32_t*)0);
    qtc_x11_call_void(put_image, XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap, cid,
                      data->width, data->height, 0, 0, 0, 32, data->len,
                      (unsigned char*)data->data);
    qtc_x11_call_void(free_gc, cid);
    return pixmap;
}

void
qtc_kde_x11_shadow_init()
{
    shadow_atom = qtc_x11_atoms[QTC_X11_ATOM_KDE_NET_WM_SHADOW];
    shadow_pixmaps[0] = qtc_shadow_create_pixmap(&qtc_shadow0);
    shadow_pixmaps[1] = qtc_shadow_create_pixmap(&qtc_shadow1);
    shadow_pixmaps[2] = qtc_shadow_create_pixmap(&qtc_shadow2);
    shadow_pixmaps[3] = qtc_shadow_create_pixmap(&qtc_shadow3);
    shadow_pixmaps[4] = qtc_shadow_create_pixmap(&qtc_shadow4);
    shadow_pixmaps[5] = qtc_shadow_create_pixmap(&qtc_shadow5);
    shadow_pixmaps[6] = qtc_shadow_create_pixmap(&qtc_shadow6);
    shadow_pixmaps[7] = qtc_shadow_create_pixmap(&qtc_shadow7);
}

// Necessary?
void
qtc_kde_x11_shadow_destroy()
{
    for (unsigned int i = 0;
         i < sizeof(shadow_pixmaps) / sizeof(shadow_pixmaps[0]);i++) {
        qtc_x11_call_void(free_pixmap, shadow_pixmaps[i]);
    }
    qtc_x11_flush();
}

QTC_EXPORT void
qtc_kde_x11_shadow_install(xcb_window_t win)
{
    uint32_t data[8 + 4];
    memcpy(data, shadow_pixmaps, sizeof(shadow_pixmaps));
    data[8] = data[9] = data[10] = data[11] = shadow_size - 4;
    qtc_x11_call_void(change_property, XCB_PROP_MODE_REPLACE, win,
                      shadow_atom, XCB_ATOM_CARDINAL, 32, 12, data);
    qtc_x11_flush();
}

QTC_EXPORT void
qtc_kde_x11_shadow_uninstall(xcb_window_t win)
{
    qtc_x11_call_void(delete_property, win, shadow_atom);
    qtc_x11_flush();
}
