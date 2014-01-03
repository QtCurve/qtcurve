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

#include "draw.h"
#include "utils_p.h"

QTC_EXPORT void
qtcCairoHLine(cairo_t *cr, int x, int y, int w, const GdkColor *col, double a)
{
    cairo_new_path(cr);
    qtcCairoSetColor(cr, col, a);
    cairo_move_to(cr, x, y + 0.5);
    cairo_line_to(cr, x + w, y + 0.5);
    cairo_stroke(cr);
}

QTC_EXPORT void
qtcCairoVLine(cairo_t *cr, int x, int y, int h, const GdkColor *col, double a)
{
    cairo_new_path(cr);
    qtcCairoSetColor(cr, col, a);
    cairo_move_to(cr, x + 0.5, y);
    cairo_line_to(cr, x + 0.5, y + h);
    cairo_stroke(cr);
}

QTC_EXPORT void
qtcCairoPolygon(cairo_t *cr, GdkColor *col, QtcRect *area, GdkPoint *points,
                int npoints, bool fill)
{
    cairo_save(cr);
    cairo_set_line_width(cr, 1);
    qtcCairoClipRectangle(cr, area);
    qtcCairoSetColor(cr, col);
    qtcCairoPathPoints(cr, points, npoints);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    if (fill) {
        cairo_fill(cr);
    }
    cairo_restore(cr);
}
