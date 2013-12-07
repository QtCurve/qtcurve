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

#include <qtcurve-utils/qtprops.h>

#ifdef QTC_ENABLE_X11
#  include <QApplication>
#  include <QX11Info>
#  include <qtcurve-utils/x11utils.h>
#  include <qtcurve-utils/x11blur.h>
#endif

#include <QQuickWindow>

namespace QtCurve {

static bool inited = false;

__attribute__((hot)) static bool
qtcEventCallback(void **cbdata)
{
    QObject *receiver = (QObject*)cbdata[0];
    if (qtcUnlikely(!receiver))
        return false;
    QWidget *widget = qtcToWidget(receiver);
    QEvent *event = (QEvent*)cbdata[1];
    QtcWidgetPropsP props(widget);
    if (qtcUnlikely(widget && !widget->testAttribute(Qt::WA_WState_Polished) &&
                    (!qtcGetWid(widget) || props->prePolishStarted))) {
        if (Style *style = qtcGetStyle(widget)) {
            style->prePolish(widget);
        }
    } else if (widget && event->type() == QEvent::UpdateRequest) {
        props->opacity = 100;
    } else if (QQuickWindow *window = qobject_cast<QQuickWindow*>(receiver)) {
        // QtQuickControl support
        // This is still VERY experimental.
        // Need a lot more testing and refactoring.
        if (Style *style = qtcGetStyle(qApp)) {
            QColor color = window->color();
            int opacity = style->options().bgndOpacity;
            if (color.alpha() == 255 && opacity != 100) {
                qreal opacityF = opacity / 100.0;
                window->setColor(QColor::fromRgbF(color.redF() * opacityF,
                                                  color.greenF() * opacityF,
                                                  color.blueF() * opacityF,
                                                  opacityF));
#ifdef QTC_ENABLE_X11
                qtcX11BlurTrigger(window->winId(), true, 0, NULL);
#endif
            }
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
    QQuickWindow::setDefaultAlphaBuffer(true);
#ifdef QTC_ENABLE_X11
    if (qApp->platformName() == "xcb") {
        qtcX11InitXcb(QX11Info::connection(), QX11Info::appScreen());
    }
#endif
}
}
