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

struct QtcWidgetProps {
    QtcWidgetProps(): opacity(100) {}
    int opacity;
    bool prePolished: 1;
    bool prePolishStarted: 1;
};
Q_DECLARE_METATYPE(QSharedPointer<QtcWidgetProps>)

QTC_ALWAYS_INLINE static inline QSharedPointer<QtcWidgetProps>
qtcGetWidgetProps(const QWidget *w)
{
    QVariant val(w->property("__QTCURVE_WIDGET_PROPERTIES__"));
    if (!val.isValid()) {
        val = QVariant::fromValue(QSharedPointer<QtcWidgetProps>(
                                      new QtcWidgetProps));
        const_cast<QWidget*>(w)->setProperty(
            "__QTCURVE_WIDGET_PROPERTIES__", val);
    }
    return val.value<QSharedPointer<QtcWidgetProps> >();
}

class QtcWidgetPropsP {
public:
    QtcWidgetPropsP(const QWidget *widget): w(widget), p(0) {}
    inline QtcWidgetProps&
    operator*() const
    {
        if (!p) {
            p = qtcGetWidgetProps(w);
        }
        return *p;
    }
    inline QtcWidgetProps*
    operator->() const
    {
        if (!p && w) {
            p = qtcGetWidgetProps(w);
        }
        return p.data();
    }
private:
    const QWidget *w;
    mutable QSharedPointer<QtcWidgetProps> p;
};

#endif
