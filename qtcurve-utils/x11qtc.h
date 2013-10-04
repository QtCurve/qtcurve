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

#ifndef _QTC_UTILS_X11QTC_H_
#define _QTC_UTILS_X11QTC_H_

#include "x11utils.h"

#define QTC_MENUBAR_SIZE "_QTCURVE_MENUBAR_SIZE_"
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
    qtcX11SetShortProp(win, qtc_x11_atoms[QTC_X11_ATOM_QTC_MENUBAR_SIZE], s);
}

static inline void
qtcX11SetStatusBar(xcb_window_t win)
{
    qtcX11SetShortProp(win, qtc_x11_atoms[QTC_X11_ATOM_QTC_STATUSBAR], 1);
}

static inline void
qtcX11SetOpacity(xcb_window_t win, unsigned short o)
{
    qtcX11SetShortProp(win, qtc_x11_atoms[QTC_X11_ATOM_QTC_OPACITY], o);
}

static inline void
qtcX11SetBgnd(xcb_window_t win, uint32_t prop)
{
    qtcX11CallVoid(change_property, XCB_PROP_MODE_REPLACE, win,
                   qtc_x11_atoms[QTC_X11_ATOM_QTC_BGND],
                   XCB_ATOM_CARDINAL, 32, 1, &prop);
}

#endif
