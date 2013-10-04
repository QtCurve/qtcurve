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

void qtcX11InitXcb(xcb_connection_t *conn, int screen_no);
void qtcX11InitXlib(Display *disp);
xcb_connection_t *qtcX11GetConn();
int qtcX11DefaultScreenNo();
xcb_screen_t *qtcX11DefaultScreen();
xcb_window_t qtcX11RootWindow();
void qtcX11Flush();
uint32_t qtcX11GenerateId();
void qtcX11GetAtoms(size_t n, xcb_atom_t *atoms,
                    const char *const names[], boolean create);
QTC_ALWAYS_INLINE static inline xcb_atom_t
qtcX11GetAtom(const char *name, boolean create)
{
    xcb_atom_t atom;
    qtcX11GetAtoms(1, &atom, &name, create);
    return atom;
}
#define qtcX11Call(name, args...)                               \
    ({                                                          \
        xcb_connection_t *conn = qtcX11GetConn();               \
        xcb_##name##_reply(conn, xcb_##name(conn, args), 0);    \
    })
#define qtcX11CallVoid(name, args...)                   \
    ({                                                  \
        xcb_connection_t *conn = qtcX11GetConn();       \
        xcb_##name(conn, args).sequence;                \
    })

void qtcX11SetWMClass(xcb_window_t win, const char *wmclass, size_t len);
int32_t qtcX11GetShortProp(xcb_window_t win, xcb_atom_t atom);
void qtcX11MapRaised(xcb_window_t win);
boolean qtcX11CompositingActive();
boolean qtcX11HasAlpha(xcb_window_t win);

QTC_END_DECLS

#endif
