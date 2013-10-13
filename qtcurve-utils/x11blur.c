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

#include "x11blur.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>

QTC_EXPORT void
qtcX11BlurTrigger(xcb_window_t wid, bool enable, unsigned prop_num,
                  const uint32_t *props)
{
    Display *disp = qtcX11GetDisp();
    xcb_atom_t atom = qtc_x11_atoms[QTC_X11_ATOM_KDE_NET_WM_BLUR_BEHIND_REGION];
    if (enable) {
        if (disp) {
            QTC_DEF_LOCAL_BUFF(unsigned long, xlib_props, 256, prop_num);
            for (unsigned i = 0;i < prop_num;i++) {
                xlib_props[i] = props[i];
            }
            XChangeProperty(disp, wid, atom, XA_CARDINAL, 32, PropModeReplace,
                            (unsigned char*)xlib_props, prop_num);
            QTC_FREE_LOCAL_BUFF(xlib_props);
        } else {
            qtcX11CallVoid(change_property, XCB_PROP_MODE_REPLACE, wid, atom,
                           XCB_ATOM_CARDINAL, 32, prop_num, props);
        }
    } else {
        qtcX11CallVoid(delete_property, wid, atom);
    }
    qtcX11Flush();
}
