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

#ifndef __QTC_UTILS_QT_UTILS_H__
#define __QTC_UTILS_QT_UTILS_H__

#include "utils.h"
#include <QtGlobal>
#include <QObject>
#include <QWidget>
#include <config.h>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) && defined QTC_QT5_ENABLE_KDE) || \
    (QT_VERSION < QT_VERSION_CHECK(5, 0, 0) && defined QTC_QT4_ENABLE_KDE)
#define _QTC_QT_ENABLE_KDE 1
#else
#define _QTC_QT_ENABLE_KDE 0
#endif

#define qtcCheckKDEType(obj, type) (qtcCheckKDETypeFull(obj, type, #type))
#define qtcCheckKDEType0(obj, type) (qtcCheckKDETypeFull0(obj, type, #type))

template <class T, class T2> static inline bool
qtcCheckType0(T2 *obj)
{
    return obj && qobject_cast<const T*>(obj);
}
template <class T2> static inline bool
qtcCheckType0(T2 *obj, const char *name)
{
    return obj && obj->inherits(name);
}

#if _QTC_QT_ENABLE_KDE
#define qtcCheckKDETypeFull(obj, type, name) (qobject_cast<const type*>(obj))
#define qtcCheckKDETypeFull0(obj, type, name) (qtcCheckType0<type>(obj))
#else
#define qtcCheckKDETypeFull(obj, type, name) (obj->inherits(name))
#define qtcCheckKDETypeFull0(obj, type, name) (qtcCheckType0(obj, name))
#endif

QTC_ALWAYS_INLINE static inline WId
qtcGetQWidgetWid(const QWidget *w)
{
    if (!(w && w->testAttribute(Qt::WA_WState_Created)))
        return (WId)0;
    return w->internalWinId();
}

#endif
