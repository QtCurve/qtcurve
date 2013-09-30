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

#ifndef _QTC_UTILS_X11UTILS_H_
#define _QTC_UTILS_X11UTILS_H_

#include <xcb/xcb.h>
#include "utils.h"

QTC_BEGIN_DECLS

typedef enum {
    QTC_X11_ATOM_WM_CLASS,

    QTC_X11_ATOM_NET_WM_MOVERESIZE,
    QTC_X11_ATOM_NET_WM_CM_S_DEFAULT,

    QTC_X11_ATOM_KDE_NET_WM_SKIP_SHADOW,
    QTC_X11_ATOM_KDE_NET_WM_FORCE_SHADOW,
    QTC_X11_ATOM_KDE_NET_WM_SHADOW,
    QTC_X11_ATOM_KDE_NET_WM_BLUR_BEHIND_REGION,

    QTC_X11_ATOM_QTC_MENUBAR_SIZE,
    QTC_X11_ATOM_QTC_STATUSBAR,
    QTC_X11_ATOM_QTC_TITLEBAR_SIZE,
    QTC_X11_ATOM_QTC_ACTIVE_WINDOW,
    QTC_X11_ATOM_QTC_TOGGLE_MENUBAR,
    QTC_X11_ATOM_QTC_TOGGLE_STATUSBAR,
    QTC_X11_ATOM_QTC_OPACITY,
    QTC_X11_ATOM_QTC_BGND,

    _QTC_X11_ATOM_NUMBER,
} QtcX11AtomId;

extern xcb_atom_t qtc_x11_atoms[_QTC_X11_ATOM_NUMBER];

typedef struct _XDisplay Display;

void qtc_x11_init_xcb(xcb_connection_t *conn, int screen_no);
void qtc_x11_init_xlib(Display *disp);
xcb_connection_t *qtc_x11_get_conn();
int qtc_x11_default_screen_no();
xcb_screen_t *qtc_x11_default_screen();
xcb_window_t qtc_x11_root_window();
void qtc_x11_flush();
uint32_t qtc_x11_generate_id();
void qtc_x11_get_atoms(size_t n, xcb_atom_t *atoms,
                       const char *const names[], boolean create);
QTC_ALWAYS_INLINE static inline xcb_atom_t
qtc_x11_get_atom(const char *name, boolean create)
{
    xcb_atom_t atom;
    qtc_x11_get_atoms(1, &atom, &name, create);
    return atom;
}
#define qtc_x11_call(name, args...)                             \
    ({                                                          \
        xcb_connection_t *conn = qtc_x11_get_conn();            \
        xcb_##name##_reply(conn, xcb_##name(conn, args), 0);    \
    })
#define qtc_x11_call_void(name, args...)                \
    ({                                                  \
        xcb_connection_t *conn = qtc_x11_get_conn();    \
        xcb_##name(conn, args).sequence;                \
    })
void qtc_x11_set_wmclass(xcb_window_t win, const char *wmclass, size_t len);

QTC_END_DECLS

#endif
