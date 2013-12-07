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

#ifndef __QTC_UTILS_QT_PROPS_H__
#define __QTC_UTILS_QT_PROPS_H__

#include "qtutils.h"
#include <QSharedPointer>
#include <QVariant>
#include <QMdiSubWindow>

struct _QtcWidgetProps {
    _QtcWidgetProps():
        opacity(100),
        prePolished(false),
        prePolishStarted(false)
    {
    }
    int opacity;
    bool prePolished: 1;
    bool prePolishStarted: 1;
};
Q_DECLARE_METATYPE(QSharedPointer<_QtcWidgetProps>)

#define QTC_PROP_NAME "_q__QTCURVE_WIDGET_PROPERTIES__"

QTC_ALWAYS_INLINE static inline QSharedPointer<_QtcWidgetProps>
qtcGetWidgetProps(const QWidget *w)
{
    // use _q_ to mimic qt internal properties and suppress QtDesigner
    // warning about unsupported properties.
    QVariant val(w->property(QTC_PROP_NAME));
    if (!val.isValid()) {
        val = QVariant::fromValue(QSharedPointer<_QtcWidgetProps>(
                                      new _QtcWidgetProps));
        const_cast<QWidget*>(w)->setProperty(QTC_PROP_NAME, val);
    }
    return val.value<QSharedPointer<_QtcWidgetProps> >();
}

class QtcWidgetProps {
public:
    QtcWidgetProps(const QWidget *widget): w(widget), p(0) {}
    inline _QtcWidgetProps*
    operator->() const
    {
        if (!p && w) {
            p = qtcGetWidgetProps(w);
        }
        return p.data();
    }
private:
    const QWidget *w;
    mutable QSharedPointer<_QtcWidgetProps> p;
};

static inline int
qtcGetOpacity(const QWidget *widget)
{
    for (const QWidget *w = widget;w;w = w->parentWidget()) {
        QtcWidgetProps props(w);
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
