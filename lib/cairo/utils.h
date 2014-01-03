/*****************************************************************************
 *   Copyright 2003 - 2010 Craig Drummond <craig.p.drummond@gmail.com>       *
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

#ifndef __QTC_CAIRO_UTILS_H__
#define __QTC_CAIRO_UTILS_H__

#include <qtcurve-utils/utils.h>
#include <qtcurve-utils/options.h>
#include <cairo.h>

QTC_BEGIN_DECLS

typedef struct _GdkColor GdkColor;
typedef struct _GdkPoint GdkPoint;
// cairo_rectangle_int_t is toooo long...
typedef cairo_rectangle_int_t QtcRect;

void qtcCairoPathPoints(cairo_t *cr, GdkPoint *pts, int count);
void qtcCairoClipRegion(cairo_t *cr, const cairo_region_t *region);
void qtcCairoClipRectangle(cairo_t *cr, const QtcRect *rect);
void qtcCairoSetColor(cairo_t *cr, const GdkColor *col, double a);
#define qtcCairoSetColor(cr, col, a...)                 \
    qtcCairoSetColor(cr, col, QTC_DEFAULT(a, 1))
void qtcCairoPatternAddColorStop(cairo_pattern_t *pt, double offset,
                                 const GdkColor *col, double a);
#define qtcCairoPatternAddColorStop(pt, offset, col, a...)              \
    qtcCairoPatternAddColorStop(pt, offset, col, QTC_DEFAULT(a, 1))

QTC_ALWAYS_INLINE static inline bool
qtcRectEqual(const QtcRect *a, const QtcRect *b)
{
    return (a->x == b->x && a->y == b->y &&
            a->width == b->width && a->height == b->height);
}
void qtcRectUnion(const QtcRect *src1, const QtcRect *src2, QtcRect *dest);
bool qtcRectIntersect(const QtcRect *src1, const QtcRect *src2, QtcRect *dest);
QTC_ALWAYS_INLINE static inline void
qtcRectConstrain(QtcRect *rect, const QtcRect *con)
{
    qtcRectIntersect(rect, con, rect);
}
void qtcCairoPathTopLeft(cairo_t *cr, double xd, double yd, double width,
                         double height, double radius, ECornerBits round);
void qtcCairoPathBottomRight(cairo_t *cr, double xd, double yd, double width,
                             double height, double radius, ECornerBits round);
void qtcCairoPathWhole(cairo_t *cr, double xd, double yd, double width,
                       double height, double radius, ECornerBits round);

QTC_END_DECLS

#endif
