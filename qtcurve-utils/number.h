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
#ifndef _QTC_UTILS_NUMBER_H_
#define _QTC_UTILS_NUMBER_H_

#include "utils.h"

QTC_ALWAYS_INLINE static inline int
_qtcMakeVersionReal(int a, int b, int c)
{
    return a << 16 | b << 8 | c;
}
#define _qtcMakeVersionV(a, b, c, ...)          \
    _qtcMakeVersionReal(a, b, c)
#define qtcMakeVersion(a, b, arg...)            \
    _qtcMakeVersionV(a, b, ##arg, 0)

QTC_ALWAYS_INLINE static inline double
qtcLimit(double d, double l)
{
    if (d <= 0) {
        return 0;
    }
    if (d >= l) {
        return l;
    }
    return d;
}

QTC_ALWAYS_INLINE static inline bool
qtcEqual(double d1, double d2)
{
    return (fabs(d1 - d2) < 0.0001);
}

#ifdef __cplusplus
template <typename T>
QTC_ALWAYS_INLINE static inline const T&
qtcMax(const T &a, const T &b)
{
    return (a > b) ? a : b;
}
template <typename T>
QTC_ALWAYS_INLINE static inline const T&
qtcMin(const T &a, const T &b)
{
    return (a < b) ? a : b;
}
template <typename T1, typename T2>
QTC_ALWAYS_INLINE static inline _QTC_COMP_TYPE(T1, T2)
qtcMax(const T1 &a, const T2 &b)
{
    return (a > b) ? a : b;
}
template <typename T1, typename T2>
QTC_ALWAYS_INLINE static inline _QTC_COMP_TYPE(T1, T2)
qtcMin(const T1 &a, const T2 &b)
{
    return (a < b) ? a : b;
}
#else
#define qtcMax(a, b)                            \
    ({                                          \
        typeof(a) _a = (a);                     \
        typeof(b) _b = (b);                     \
        (_a > _b) ? _a : _b;                    \
    })
#define qtcMin(a, b)                            \
    ({                                          \
        typeof(a) _a = (a);                     \
        typeof(b) _b = (b);                     \
        (_a < _b) ? _a : _b;                    \
    })
#endif
#define qtcBound(a, b, c) qtcMax(a, qtcMin(b, c))

QTC_ALWAYS_INLINE static inline uintptr_t
qtcGetPadding(uintptr_t len, uintptr_t align)
{
    uintptr_t left;
    if ((left = len % align))
        return align - left;
    return 0;
}

QTC_ALWAYS_INLINE static inline uintptr_t
qtcAlignTo(uintptr_t len, uintptr_t align)
{
    return len + qtcGetPadding(len, align);
}

#endif
