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

#ifndef _XCB_UTILS_H_
#define _XCB_UTILS_H_

#include <xcb/xcb.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QWindow>

namespace QtCurve {
namespace XcbUtils {

xcb_connection_t *getConnection();

static inline WId
rootWindow()
{
    return QApplication::desktop()->windowHandle()->winId();
}

static inline void
flush()
{
    xcb_flush(getConnection());
}

static inline uint32_t
generateId()
{
    return xcb_generate_id(getConnection());
};

void getAtoms(size_t n, xcb_atom_t *atoms, const char *const names[],
              bool create=false);

static inline xcb_atom_t
getAtom(const char *name, bool create=false)
{
    xcb_atom_t atom;
    XcbUtils::getAtoms(1, &atom, &name, create);
    return atom;
}

template<typename RetType, typename CookieType, typename... ArgTypes,
         typename... ArgTypes2>
static inline RetType*
XcbCall(CookieType (*func)(xcb_connection_t*, ArgTypes...),
        RetType *(reply_func)(xcb_connection_t*, CookieType,
                              xcb_generic_error_t**),
        ArgTypes2... args...)
{
    xcb_connection_t *conn = getConnection();
    CookieType cookie = func(conn, args...);
    return reply_func(conn, cookie, 0);
}

#define XcbCall(name, args...)                                  \
    XcbUtils::XcbCall(xcb_##name, xcb_##name##_reply, args)

template<typename... ArgTypes, typename... ArgTypes2>
static inline void
XcbCallVoid(xcb_void_cookie_t (*func)(xcb_connection_t*, ArgTypes...),
            ArgTypes2... args...)
{
    xcb_connection_t *conn = getConnection();
    func(conn, args...);
}

#define XcbCallVoid(name, args...)              \
    XcbUtils::XcbCallVoid(xcb_##name, args)

}
}

#endif
