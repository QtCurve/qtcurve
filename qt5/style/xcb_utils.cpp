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
#include "qtcurve_p.h"

namespace QtCurve
{
namespace XcbUtils
{

static inline QByteArray
getWMClass()
{
    QByteArray appname = appName.toLocal8Bit();
    return (appname + '\0' +
            QCoreApplication::instance()->applicationName().toLocal8Bit());
}

void
setWindowWMClass(WId wid)
{
    static QByteArray wmclass = getWMClass();
    qtc_x11_set_wmclass(wid, wmclass.constData(), wmclass.count());
}

}
}
