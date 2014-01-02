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

#ifndef __QTC_UTILS_QT_PROPS_H__
#define __QTC_UTILS_QT_PROPS_H__

#include "qtutils.h"
#include <QSharedPointer>
#include <QVariant>
#include <QMdiSubWindow>

struct _QtcQWidgetProps {
    _QtcQWidgetProps():
        opacity(100),
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        prePolishing(false),
#else
        prePolished(false),
        prePolishStarted(false),
#endif
        shadowRegistered(false),
        noEtch(false)
    {
    }
    int opacity;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    bool prePolishing: 1;
#else
    bool prePolished: 1;
    bool prePolishStarted: 1;
#endif
    bool shadowRegistered: 1;
    // OK, Etching looks cr*p on plasma widgets, and khtml...
    // CPD:TODO WebKit?
    bool noEtch: 1;
};
Q_DECLARE_METATYPE(QSharedPointer<_QtcQWidgetProps>)

#define QTC_PROP_NAME "_q__QTCURVE_WIDGET_PROPERTIES__"

QTC_ALWAYS_INLINE static inline QSharedPointer<_QtcQWidgetProps>
qtcGetWidgetProps(const QWidget *w)
{
    // use _q_ to mimic qt internal properties and suppress QtDesigner
    // warning about unsupported properties.
    QVariant val(w->property(QTC_PROP_NAME));
    if (!val.isValid()) {
        val = QVariant::fromValue(QSharedPointer<_QtcQWidgetProps>(
                                      new _QtcQWidgetProps));
        const_cast<QWidget*>(w)->setProperty(QTC_PROP_NAME, val);
    }
    return val.value<QSharedPointer<_QtcQWidgetProps> >();
}

class QtcQWidgetProps {
public:
    QtcQWidgetProps(const QWidget *widget): w(widget), p(0) {}
    inline _QtcQWidgetProps*
    operator ->() const
    {
        if (!p && w) {
            p = qtcGetWidgetProps(w);
        }
        return p.data();
    }
private:
    const QWidget *w;
    mutable QSharedPointer<_QtcQWidgetProps> p;
};

static inline int
qtcGetOpacity(const QWidget *widget)
{
    for (const QWidget *w = widget;w;w = w->parentWidget()) {
        QtcQWidgetProps props(w);
        if (qobject_cast<const QMdiSubWindow*>(w)) {
            // don't use opacity on QMdiSubWindow menu for now, as it will
            // draw through the background as well.
            return 100;
        }
        if (props->opacity < 100) {
            return props->opacity;
        }
        if (w->isWindow()) {
            break;
        }
    }
    return 100;
}

#endif
