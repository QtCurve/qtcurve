/*****************************************************************************
 *   Copyright 2014 - 2014 Yichao Yu <yyc1992@gmail.com>                     *
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

#include "x11wrap.h"

QTC_EXPORT void
qtcX11ChangeProperty(uint8_t mode, xcb_window_t win, xcb_atom_t prop,
                     xcb_atom_t type, uint8_t format, uint32_t len,
                     const void *data)
{
    qtcX11CallVoid(change_property, mode, win, prop, type, format, len, data);
}

QTC_EXPORT xcb_query_tree_reply_t*
qtcX11QueryTree(xcb_window_t win)
{
    return qtcX11Call(query_tree, win);
}

QTC_EXPORT void
qtcX11ReparentWindow(xcb_window_t win, xcb_window_t parent,
                     int16_t x, int16_t y)
{
    qtcX11CallVoid(reparent_window, win, parent, x, y);
}

QTC_EXPORT void
qtcX11SendEvent(uint8_t propagate, xcb_window_t destination,
                uint32_t event_mask, const void *event)
{
    qtcX11CallVoid(send_event, propagate, destination, event_mask,
                   (const char*)event);
}

QTC_EXPORT xcb_get_property_reply_t*
qtcX11GetProperty(uint8_t del, xcb_window_t win, xcb_atom_t prop,
                  xcb_atom_t type, uint32_t offset, uint32_t len)
{
    return qtcX11Call(get_property, del, win, prop, type, offset, len);
}

QTC_EXPORT void*
qtcX11GetPropertyValue(const xcb_get_property_reply_t *reply)
{
    return xcb_get_property_value(reply);
}

QTC_EXPORT int
qtcX11GetPropertyValueLength(const xcb_get_property_reply_t *reply)
{
    return xcb_get_property_value_length(reply);
}
