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

#include "xcb_utils.h"
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformwindow.h>
#include <QApplication>

namespace QtCurve
{
namespace XcbUtils
{

xcb_connection_t*
getConnection()
{
    static xcb_connection_t *connection = NULL;
    if (Q_UNLIKELY(!connection)) {
        QPlatformNativeInterface *native = qApp->platformNativeInterface();
        connection = reinterpret_cast<xcb_connection_t*>(
            native->nativeResourceForWindow("connection", 0));
    }
    return connection;
}

void
getAtoms(size_t n, xcb_atom_t *atoms, const char *const names[], bool create)
{
    xcb_connection_t *conn = getConnection();
    xcb_intern_atom_cookie_t *cookies =
        (xcb_intern_atom_cookie_t*)malloc(sizeof(xcb_intern_atom_cookie_t) * n);
    for (size_t i = 0;i < n;i++) {
        cookies[i] = xcb_intern_atom(conn, create, strlen(names[i]), names[i]);
    }
    memset(atoms, 0, sizeof(xcb_atom_t) * n);
    for (size_t i = 0;i < n;i++) {
        xcb_intern_atom_reply_t *r =
            xcb_intern_atom_reply(conn, cookies[i], 0);
        if (r) {
            atoms[i] = r->atom;
            free(r);
        }
    }
    free(cookies);
}

}
}
