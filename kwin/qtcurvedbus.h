/*
  QtCurve KWin window decoration
  Copyright (C) 2010 Craig Drummond <craig.p.drummond@googlemail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
 */

#ifndef _QTCURVE_DBUS_H_
#define _QTCURVE_DBUS_H_

#include <QDBusAbstractAdaptor>
#include "qtcurvehandler.h"

namespace KWinQtCurve
{

class QtCurveDBus : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.QtCurve")

    public:

    QtCurveDBus(QtCurveHandler *handler) : QDBusAbstractAdaptor(handler) { }

    void emitBorderSizes()              { emit borderSizesChanged(); }
    void emitMbToggle(unsigned int xid) { emit toggleMenuBar(xid); }
    void emitSbToggle(unsigned int xid) { emit toggleStatusBar(xid); }

    Q_SIGNALS:

    void borderSizesChanged();
    void toggleMenuBar(unsigned int xid);
    void toggleStatusBar(unsigned int xid);

    public Q_SLOTS:

    Q_NOREPLY void menuBarSize(unsigned int xid, int size)      { Handler()->menuBarSize(xid, size); }
    Q_NOREPLY void statusBarState(unsigned int xid, bool state) { Handler()->statusBarState(xid, state); }
};

}

#endif
