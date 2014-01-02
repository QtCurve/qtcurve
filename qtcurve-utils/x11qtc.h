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

#ifndef _QTC_UTILS_X11QTC_H_
#define _QTC_UTILS_X11QTC_H_

#include "x11utils.h"

#define QTC_STATUSBAR "_QTCURVE_STATUSBAR_"

static inline void
qtcX11SetShortProp(xcb_window_t win, xcb_atom_t atom, unsigned short prop)
{
    qtcX11CallVoid(change_property, XCB_PROP_MODE_REPLACE, win,
                   atom, XCB_ATOM_CARDINAL, 16, 1, &prop);
}

static inline void
qtcX11SetMenubarSize(xcb_window_t win, unsigned short s)
{
    qtcX11SetShortProp(win, qtc_x11_qtc_menubar_size, s);
}

static inline void
qtcX11SetStatusBar(xcb_window_t win)
{
    qtcX11SetShortProp(win, qtc_x11_qtc_statusbar, 1);
}

static inline void
qtcX11SetOpacity(xcb_window_t win, unsigned short o)
{
    qtcX11SetShortProp(win, qtc_x11_qtc_opacity, o);
}

static inline void
qtcX11SetBgnd(xcb_window_t win, uint32_t prop)
{
    qtcX11CallVoid(change_property, XCB_PROP_MODE_REPLACE, win,
                   qtc_x11_qtc_bgnd, XCB_ATOM_CARDINAL, 32, 1, &prop);
}

#endif
