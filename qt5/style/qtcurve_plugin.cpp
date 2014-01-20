/*****************************************************************************
 *   Copyright 2013 - 2014 Yichao Yu <yyc1992@gmail.com>                     *
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
#  include <qtcurve-utils/x11shadow.h>
#  include <qtcurve-utils/x11blur.h>
#endif

#include <QQuickWindow>
#include <QQuickItem>
#include <QDebug>

namespace QtCurve {

static bool inited = false;

__attribute__((hot)) static bool
qtcEventCallback(void **cbdata)
{
    QObject *receiver = (QObject*)cbdata[0];
    QTC_RET_IF_FAIL(receiver, false);
    QEvent *event = (QEvent*)cbdata[1];
    if (qtcUnlikely(event->type() == QEvent::DynamicPropertyChange)) {
        QDynamicPropertyChangeEvent *prop_event =
            static_cast<QDynamicPropertyChangeEvent*>(event);
        // eat the property change events from ourselves
        if (prop_event->propertyName() == QTC_PROP_NAME) {
            return true;
        }
    }
    QWidget *widget = qtcToWidget(receiver);
    if (qtcUnlikely(widget && !qtcGetWid(widget))) {
        if (Style *style = qtcGetStyle(widget)) {
            style->prePolish(widget);
        }
    } else if (widget && event->type() == QEvent::UpdateRequest) {
        QtcQWidgetProps props(widget);
        props->opacity = 100;
    } else if (QQuickWindow *window = qobject_cast<QQuickWindow*>(receiver)) {
        // QtQuickControl support
        // This is still VERY experimental.
        // Need a lot more testing and refactoring.
        if (Style *style = qtcGetStyle(qApp)) {
            if (window->inherits("QQuickPopupWindow")) {
                if (window->inherits("QQuickMenuPopupWindow")) {
                    window->setColor(QColor(0, 0, 0, 0));
                }
#ifdef QTC_ENABLE_X11
                qtcX11ShadowInstall(window->winId());
#endif
            } else {
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
    } else if (QQuickItem *item = qobject_cast<QQuickItem*>(receiver)) {
        QQuickWindow *window = item->window();
        if (!window)
            return false;
        if (qtcGetStyle(qApp)) {
            window->setColor(QColor(0, 0, 0, 0));
#ifdef QTC_ENABLE_X11
            qtcX11BlurTrigger(window->winId(), true, 0, NULL);
#endif
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
