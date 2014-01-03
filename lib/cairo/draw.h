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

#ifndef __QTC_CAIRO_DRAW_H__
#define __QTC_CAIRO_DRAW_H__

#include "utils.h"

QTC_BEGIN_DECLS

void qtcCairoHLine(cairo_t *cr, int x, int y, int w,
                   const GdkColor *col, double a);
#define qtcCairoHLine(cr, x, y, w, col, a...)           \
    qtcCairoHLine(cr, x, y, w, col, QTC_DEFAULT(a, 1))
void qtcCairoVLine(cairo_t *cr, int x, int y, int w,
                   const GdkColor *col, double a);
#define qtcCairoVLine(cr, x, y, w, col, a...)           \
    qtcCairoVLine(cr, x, y, w, col, QTC_DEFAULT(a, 1))
void qtcCairoPolygon(cairo_t *cr, GdkColor *col, QtcRect *area,
                     GdkPoint *points, int npoints, bool fill);
void qtcCairoRect(cairo_t *cr, const QtcRect *area, int x, int y,
                  int width, int height, const GdkColor *col, double alpha);
#define qtcCairoRect(cr, area, x, y, width, height, col, alpha...)      \
    qtcCairoRect(cr, area, x, y, width, height, col, QTC_DEFAULT(alpha, 1))

void qtcCairoFadedLine(cairo_t *cr, int x, int y, int width, int height,
                       const QtcRect *area, const QtcRect *gap, bool fadeStart,
                       bool fadeEnd, double fadeSize, bool horiz,
                       GdkColor *col, double alpha);
#define qtcCairoFadedLine(cr, x, y, width, height, area, gap, fadeStart, \
                          fadeEnd, fadeSize, horiz, col, alpha...)      \
    qtcCairoFadedLine(cr, x, y, width, height, area, gap, fadeStart,    \
                      fadeEnd, fadeSize, horiz, col, QTC_DEFAULT(alpha, 1))

QTC_END_DECLS

#endif
