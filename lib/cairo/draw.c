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
    qtcCairoClipRect(cr, area);
    qtcCairoSetColor(cr, col);
    qtcCairoPathPoints(cr, points, npoints);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    if (fill) {
        cairo_fill(cr);
    }
    cairo_restore(cr);
}

QTC_EXPORT void
qtcCairoRect(cairo_t *cr, const QtcRect *area, int x, int y,
             int width, int height, const GdkColor *col, double alpha)
{
    cairo_save(cr);
    qtcCairoClipRect(cr, area);
    cairo_rectangle(cr, x, y, width, height);
    qtcCairoSetColor(cr, col, alpha);
    cairo_fill(cr);
    cairo_restore(cr);
}

QTC_EXPORT void
qtcCairoFadedLine(cairo_t *cr, int x, int y, int width, int height,
                  const QtcRect *area, const QtcRect *gap, bool fadeStart,
                  bool fadeEnd, double fadeSize, bool horiz,
                  GdkColor *col, double alpha)
{
    double rx = x + 0.5;
    double ry = y + 0.5;
    cairo_pattern_t *pt =
        cairo_pattern_create_linear(rx, ry, horiz ? rx + width - 1 : rx + 1,
                                    horiz ? ry + 1 : ry + height - 1);
    cairo_save(cr);

    if (gap) {
        QtcRect r = {x, y, width, height};
        cairo_region_t *region =
            cairo_region_create_rectangle(area ? area : &r);
        cairo_region_xor_rectangle(region, gap);
        qtcCairoClipRegion(cr, region);
        cairo_region_destroy(region);
    } else {
        qtcCairoClipRect(cr, area);
    }
    qtcCairoPatternAddColorStop(pt, 0, col, fadeStart ? 0.0 : alpha);
    qtcCairoPatternAddColorStop(pt, fadeSize, col, alpha);
    qtcCairoPatternAddColorStop(pt, 1 - fadeSize, col, alpha);
    qtcCairoPatternAddColorStop(pt, 1, col, fadeEnd ? 0.0 : alpha);
    cairo_set_source(cr, pt);
    if (horiz) {
        cairo_move_to(cr, x, ry);
        cairo_line_to(cr, x + width - 1, ry);
    } else {
        cairo_move_to(cr, rx, y);
        cairo_line_to(cr, rx, y + height - 1);
    }
    cairo_stroke(cr);
    cairo_pattern_destroy(pt);
    cairo_restore(cr);
}

QTC_EXPORT void
qtcCairoStripes(cairo_t *cr, int x, int y, int w, int h,
                bool horizontal, int stripeWidth)
{
    int endx = horizontal ? stripeWidth : 0;
    int endy = horizontal ? 0 : stripeWidth;

    cairo_pattern_t *pat =
        cairo_pattern_create_linear(x, y, x + endx, y + endy);

    cairo_pattern_add_color_stop_rgba(pat, 0.0, 1.0, 1.0, 1.0, 0.0);
    cairo_pattern_add_color_stop_rgba(pat, 1, 1.0, 1.0, 1.0, 0.15);
    cairo_pattern_set_extend(pat, CAIRO_EXTEND_REFLECT);
    cairo_set_source(cr, pat);
    cairo_rectangle(cr, x, y, w, h);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);
}
