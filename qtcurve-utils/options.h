/***************************************************************************
 *   Copyright (C) 2003~2010 Craig Drummond                                *
 *   craig.p.drummond@gmail.com                                            *
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

#ifndef QTC_UTILS_OPTIONS_H
#define QTC_UTILS_OPTIONS_H

#include "utils.h"

typedef enum {
    SHADING_SIMPLE,
    SHADING_HSL,
    SHADING_HSV,
    SHADING_HCY
} EShading;

typedef enum {
    SCROLLBAR_KDE,
    SCROLLBAR_WINDOWS,
    SCROLLBAR_PLATINUM,
    SCROLLBAR_NEXT,
    SCROLLBAR_NONE
} EScrollbar;

/**
 * Number of scrollbar buttons.
 **/
QTC_ALWAYS_INLINE static inline int
qtcScrollbarButtonNum(EScrollbar type)
{
    switch (type) {
    default:
    case SCROLLBAR_KDE:
        return 3;
    case SCROLLBAR_WINDOWS:
    case SCROLLBAR_PLATINUM:
    case SCROLLBAR_NEXT:
        return 2;
    case SCROLLBAR_NONE:
        return 0;
    }
}

/**
 * Number of scrollbar button we leave space for.
 * Some applications (e.g. choqok) use the minimum size of scrollbar to
 * determine the height of input area. Setting a lower limit on this makes
 * sure that the calculated minimum size is not too small.
 * See https://github.com/QtCurve/qtcurve-qt4/issues/7
 * and https://bugs.kde.org/show_bug.cgi?id=317690
 **/
QTC_ALWAYS_INLINE static inline int
qtcScrollbarButtonNumSize(EScrollbar type)
{
    return qtcMax(qtcScrollbarButtonNum(type), 2);
}

#endif
