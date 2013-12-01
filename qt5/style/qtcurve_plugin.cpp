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

#include "qtcurve_plugin.h"
#include "qtcurve.h"

#include <qtcurve-utils/qtutils.h>

#ifdef QTC_ENABLE_X11
#  include <QApplication>
#  include <QX11Info>
#  include <qtcurve-utils/x11utils.h>
#endif

namespace QtCurve {

static bool inited = false;

__attribute__((hot)) static bool
qtcEventCallback(void **cbdata)
{
    QObject *receiver = (QObject*)cbdata[0];
    if (qtcUnlikely(!receiver))
        return false;
    QWidget *widget = qtcToWidget(receiver);
    if (qtcUnlikely(widget && !widget->testAttribute(Qt::WA_WState_Polished) &&
                    (!qtcGetWid(widget) || qtcGetPrePolishStarted(widget)))) {
        QStyle *qstyle = widget->style();
        Style *style = qstyle ? qobject_cast<Style*>(qstyle) : NULL;
        if (style) {
            style->prePolish(widget);
        }
    }
    return false;
}

QStyle*
StylePlugin::create(const QString &key)
{
    init();
    return "qtcurve" == key.toLower() ? new Style : 0;
}

void
StylePlugin::init()
{
    if (inited)
        return;
    inited = true;
    QInternal::registerCallback(QInternal::EventNotifyCallback,
                                qtcEventCallback);
#ifdef QTC_ENABLE_X11
    if (qApp->platformName() == "xcb") {
        qtcX11InitXcb(QX11Info::connection(), QX11Info::appScreen());
    }
#endif
}
}
