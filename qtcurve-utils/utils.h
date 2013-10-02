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

#ifndef _QTC_UTILS_UTILS_H_
#define _QTC_UTILS_UTILS_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef int32_t boolean;
#ifndef __cplusplus
#  ifndef true
#    define true (1)
#  endif
#  ifndef false
#    define false (0)
#  endif
#endif

typedef struct {
    int len;
    int width;
    int height;
    int depth;
    const unsigned char *data;
} QtcPixmap;

#define qtcContainerOf(ptr, type, member)             \
    ((type*)(((void*)(ptr)) - offsetof(type, member)))

#if (defined(__GNUC__) && (__GNUC__ > 2))
#  define qtcExpect(exp, var) __builtin_expect(exp, var)
#else
#  define qtcExpect(exp, var) (exp)
#endif

#define qtcLikely(x) qtcExpect(!!(x), 1)
#define qtcUnlikely(x) qtcExpect(!!(x), 0)

#define QTC_EXPORT __attribute__((visibility("default")))

#ifdef __cplusplus
#  define QTC_BEGIN_DECLS extern "C" {
#  define QTC_END_DECLS }
#else
#  define QTC_BEGIN_DECLS
#  define QTC_END_DECLS
#endif

#define QTC_ALWAYS_INLINE __attribute__((always_inline))
#define QTC_UNUSED(x) ((void)(x))

QTC_BEGIN_DECLS

QTC_ALWAYS_INLINE static inline void*
qtcUtilsAlloc0(size_t size)
{
    void *p = malloc(size);
    memset(p, 0, size);
    return p;
}

#define qtcUtilsNewSize(type, size) ((type*)qtcUtilsAlloc0(size))
#define qtcUtilsNew(type) qtcUtilsNewSize(type, sizeof(type))
#define qtcUtilsNewN(type, n) qtcUtilsNewSize(type, sizeof(type) * n)

QTC_ALWAYS_INLINE static inline double
qtcLimit(double d, double l)
{
    if (d <= 0)
        return 0;
    if (d >= l)
        return l;
    return d;
}

QTC_ALWAYS_INLINE static inline boolean
qtcEqual(double d1, double d2)
{
    return (fabs(d1 - d2) < 0.0001);
}

QTC_END_DECLS

#ifdef __cplusplus
template<typename T>
static inline const T&
qtcMax(const T &a, const T &b)
{
    return (a > b) ? a : b;
}
template<typename T>
static inline const T&
qtcMin(const T &a, const T &b)
{
    return (a < b) ? a : b;
}
template<typename T>
static inline const T&
qtcBound(const T &a, const T &b, const T &c)
{
    return qtcMax(a, qtcMin(b, c));
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
#define qtcBound(a, b, c) qtcMax(a, qtcMin(b, c))

#endif

#endif
