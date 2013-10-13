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

#include "x11wmmove.h"

QTC_EXPORT void
qtcX11MoveTrigger(xcb_window_t wid, uint32_t x, uint32_t y)
{
    qtcX11FlushXlib();
    qtcX11CallVoid(ungrab_pointer, XCB_TIME_CURRENT_TIME);
    union {
        char _buff[32];
        xcb_client_message_event_t ev;
    } buff;
    memset(&buff, 0, sizeof(buff));
    //...Taken from bespin...
    // stolen... errr "adapted!" from QSizeGrip
    // Well now it is "ported"
    xcb_client_message_event_t *xev = &buff.ev;
    xev->response_type = XCB_CLIENT_MESSAGE;
    xev->format = 32;
    xev->window = wid;
    xev->type = qtc_x11_atoms[QTC_X11_ATOM_NET_WM_MOVERESIZE];
    xev->data.data32[0] = x;
    xev->data.data32[1] = y;
    xev->data.data32[2] = 8; // NET::Move
    xev->data.data32[3] = XCB_KEY_BUT_MASK_BUTTON_1;
    qtcX11CallVoid(send_event, false, qtcX11RootWindow(),
                   XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                   XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char*)xev);
    qtcX11Flush();
}
