/***************************************************************************
 *   Copyright (C) 2009~2010 Craig Drummond                                *
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

#include "color.h"

static void
qtc_color_HCY_fromColor(const QtcColor *color, QtcColorHCY *hcy)
{
    double r = qtc_color_HCY_gamma(color->red);
    double g = qtc_color_HCY_gamma(color->green);
    double b = qtc_color_HCY_gamma(color->blue);

    // luma component
    hcy->y = qtc_color_HCY_lumag(r, g, b);

    // hue component
    double p = QtcMax(QtcMax(r, g), b);
    double n = QtcMin(QtcMin(r, g), b);
    double d = 6.0 * (p - n);
    if (n == p) {
        hcy->h = 0.0;
    } else if (r == p) {
        hcy->h = ((g - b) / d);
    } else if (g == p) {
        hcy->h = ((b - r) / d) + (1.0 / 3.0);
    } else {
        hcy->h = ((r - g) / d) + (2.0 / 3.0);
    }

    // chroma component
    if (0.0 == hcy->y || 1.0 == hcy->y) {
        hcy->c = 0.0;
    } else {
        hcy->c = QtcMax((hcy->y - n) / hcy->y, (p - hcy->y) / (1 - hcy->y) );
    }
}

static void
qtc_color_HCY_toColor(const QtcColorHCY *hcy, QtcColor *color)
{
    // start with sane component values
    double _h = qtc_color_wrap(hcy->h, 1);
    double _c = qtc_color_normalize(hcy->c);
    double _y = qtc_color_normalize(hcy->y);

    // calculate some needed variables
    double _hs = _h * 6.0, th, tm;
    if (_hs < 1.0) {
        th = _hs;
        tm = _qtc_yc[0] + _qtc_yc[1] * th;
    } else if (_hs < 2.0) {
        th = 2.0 - _hs;
        tm = _qtc_yc[1] + _qtc_yc[0] * th;
    } else if (_hs < 3.0) {
        th = _hs - 2.0;
        tm = _qtc_yc[1] + _qtc_yc[2] * th;
    } else if (_hs < 4.0) {
        th = 4.0 - _hs;
        tm = _qtc_yc[2] + _qtc_yc[1] * th;
    } else if (_hs < 5.0) {
        th = _hs - 4.0;
        tm = _qtc_yc[2] + _qtc_yc[0] * th;
    } else {
        th = 6.0 - _hs;
        tm = _qtc_yc[0] + _qtc_yc[2] * th;
    }

    // calculate RGB channels in sorted order
    double tn, to, tp;
    if (tm >= _y) {
        tp = _y + _y * _c * (1.0 - tm) / tm;
        to = _y + _y * _c * (th - tm) / tm;
        tn = _y - (_y * _c);
    } else {
        tp = _y + (1.0 - _y) * _c;
        to = _y + (1.0 - _y) * _c * (th - tm) / (1.0 - tm);
        tn = _y - (1.0 - _y) * _c * tm / (1.0 - tm);
    }

    tp = qtc_color_HCY_igamma(tp);
    to = qtc_color_HCY_igamma(to);
    tn = qtc_color_HCY_igamma(tn);
    // return RGB channels in appropriate order
    if (_hs < 1.0) {
        qtc_color_fill(color, tp, to, tn);
    } else if (_hs < 2.0) {
        qtc_color_fill(color, to, tp, tn);
    } else if (_hs < 3.0) {
        qtc_color_fill(color, tn, tp, to);
    } else if (_hs < 4.0) {
        qtc_color_fill(color, tn, to, tp);
    } else if (_hs < 5.0) {
        qtc_color_fill(color, to, tn, tp);
    } else {
        qtc_color_fill(color, tp, tn, to);
    }
}

static double
qtc_color_contrastRatio(const QtcColor *c1, const QtcColor *c2)
{
    double y1 = qtc_color_HCY_luma(c1);
    double y2 = qtc_color_HCY_luma(c2);
    if (y1 > y2) {
        return (y1 + 0.05) / (y2 + 0.05);
    } else {
        return (y2 + 0.05) / (y1 + 0.05);
    }
}

QTC_EXPORT void
_qtc_color_lighten(QtcColor *color, double ky, double kc)
{
    QtcColorHCY hcy;
    qtc_color_HCY_fromColor(color, &hcy);

    hcy.y = 1.0 - qtc_color_normalize((1.0 - hcy.y) * (1.0 - ky));
    hcy.c = 1.0 - qtc_color_normalize((1.0 - hcy.c) * kc);
    qtc_color_HCY_toColor(&hcy, color);
}

QTC_EXPORT void
_qtc_color_darken(QtcColor *color, double ky, double kc)
{
    QtcColorHCY hcy;
    qtc_color_HCY_fromColor(color, &hcy);

    hcy.y = qtc_color_normalize(hcy.y * (1.0 - ky));
    hcy.c = qtc_color_normalize(hcy.c * kc);
    qtc_color_HCY_toColor(&hcy, color);
}

QTC_EXPORT void
_qtc_color_shade(QtcColor *color, double ky, double kc)
{
    QtcColorHCY hcy;
    qtc_color_HCY_fromColor(color, &hcy);

    hcy.y = qtc_color_normalize(hcy.y + ky);
    hcy.c = qtc_color_normalize(hcy.c + kc);
    return qtc_color_HCY_toColor(&hcy, color);
}

static void
qtc_color_tintHelper(const QtcColor *base, const QtcColor *col,
                     double amount, QtcColor *out)
{
    QtcColor mixed;
    _qtc_color_mix(base, col, pow(amount, 0.3), &mixed);
    QtcColorHCY hcy;
    qtc_color_HCY_fromColor(&mixed, &hcy);
    hcy.y = qtc_color_mixQreal(qtc_color_HCY_luma(base), hcy.y, amount);

    qtc_color_HCY_toColor(&hcy, out);
}

QTC_EXPORT void
_qtc_color_tint(const QtcColor *base, const QtcColor *col,
                double amount, QtcColor *out)
{
    if (qtc_unlikely(amount <= 0.0 || isnan(amount))) {
        *out = *base;
        return;
    }
    if (qtc_unlikely(amount >= 1.0)) {
        *out = *col;
        return;
    }

    double ri = qtc_color_contrastRatio(base, col);
    double rg = 1.0 + ((ri + 1.0) * amount * amount * amount);
    double u = 1.0, l = 0.0;
    int i;
    for (i = 12;i;i--) {
        double a = 0.5 * (l + u);
        qtc_color_tintHelper(base, col, a, out);
        double ra = qtc_color_contrastRatio(base, out);
        if (ra > rg) {
            u = a;
        } else {
            l = a;
        }
    }
}

QTC_EXPORT void
_qtc_color_mix(const QtcColor *c1, const QtcColor *c2,
               double bias, QtcColor *out)
{
    if (qtc_unlikely(bias <= 0.0 || isnan(bias))) {
        *out = *c1;
        return;
    }
    if (bias >= 1.0) {
        *out = *c2;
        return;
    }

    qtc_color_fill(out, qtc_color_mixQreal(c1->red, c2->red, bias),
                   qtc_color_mixQreal(c1->green, c2->green, bias),
                   qtc_color_mixQreal(c1->blue, c2->blue, bias));
}

static inline void
rgbToHsl(double r, double g, double b, double *h, double *s, double *l)
{
    double min = QtcMin(QtcMin(r, g), b);
    double max = QtcMax(QtcMax(r, g), b);

    *l = 0.5 * (max + min);
    *s = 0.0;
    *h = 0.0;

    if (max != min) {
        double delta = max - min;

        if (*l <= 0.5) {
            *s = delta / (max + min);
        } else {
            *s = delta / (2.0 - max - min);
        }

        if (r == max) {
            *h = (g - b) / delta;
        } else if (g == max) {
            *h = 2.0 + (b - r) / delta;
        } else if (b == max) {
            *h = 4.0 + (r - g) / delta;
        }

        *h /= 6.0;
        if (*h < 0.0) {
            (*h) += 1.0;
        }
    }
}

static inline double
h2c(double h, double m1, double m2)
{
    h = qtc_color_wrap(h, 6.0);

    if (h < 1.0)
        return qtc_color_mixf(m1, m2, h);
    if (h < 3.0)
        return m2;
    if (h < 4.0)
        return qtc_color_mixf(m1, m2, 4.0 - h);
    return m1;
}

static inline void
hslToRgb(double h, double s, double l, double *r, double *g, double *b)
{
    double m1, m2;

    // TODO h2rgb( h, r, g, b );
    h *= 6.0;

    if (l <= 0.5) {
        m2 = l * (1.0 + s);
    } else {
        m2 = l + s * (1.0 - l);
    }
    m1 = 2.0 * l - m2;

    *r = h2c(h + 2.0, m1, m2);
    *g = h2c(h, m1, m2);
    *b = h2c(h - 2.0, m1, m2);
}

QTC_EXPORT void
_qtc_shade(const QtcColor *ca, QtcColor *cb, double k, EShading shading)
{
    /* if (qtc_equal(k, 1.0)) { */
    /*     *cb = *ca; */
    /*     return; */
    /* } */
    switch (shading) {
    case SHADING_SIMPLE: {
        double v = k - 1;
        qtc_color_fill(cb, qtc_limit(ca->red + v, 1.0),
                       qtc_limit(ca->green + v, 1.0),
                       qtc_limit(ca->blue + v, 1.0));
        break;
    }
    case SHADING_HSL: {
        double r = ca->red;
        double g = ca->green;
        double b = ca->blue;
        double h, s, l;

        rgbToHsl(r, g, b, &h, &s, &l);
        l = qtc_color_normalize(l * k);
        s = qtc_color_normalize(s * k);
        hslToRgb(h, s, l, &r, &g, &b);
        qtc_color_fill(cb, qtc_limit(r, 1.0), qtc_limit(g, 1.0),
                       qtc_limit(b, 1.0));
        break;
    }
    case SHADING_HSV: {
        double r = ca->red;
        double g = ca->green;
        double b = ca->blue;
        double h, s, v;

        qtc_rgb_to_hsv(r, g, b, &h, &s, &v);

        v *= k;
        if (v > 1.0) {
            s -= v - 1.0;
            if (s < 0)
                s = 0;
            v = 1.0;
        }
        qtc_hsv_to_rgb(&r, &g, &b, h, s, v);
        qtc_color_fill(cb, qtc_limit(r, 1.0), qtc_limit(g, 1.0),
                       qtc_limit(b, 1.0));
        break;
    }
    case SHADING_HCY:
#define HCY_FACTOR 0.15
        *cb = *ca;
        if (k > 1) {
            _qtc_color_lighten(cb, (k * (1 + HCY_FACTOR)) - 1.0, 1.0);
        } else {
            _qtc_color_darken(cb, 1.0 - (k * (1 - HCY_FACTOR)), 1.0);
        }
    }
}

QTC_EXPORT double
_qtc_shine_alpha(const QtcColor *bgnd)
{
    double r = bgnd->red;
    double g = bgnd->green;
    double b = bgnd->blue;
    double h = 0, s = 0, v = 0;
    qtc_rgb_to_hsv(r, g, b, &h, &s, &v);
    return v * 0.8;
}

QTC_EXPORT double qtc_ring_alpha[3] = {0.125, 0.125, 0.5};

QTC_EXPORT void
_qtc_calc_ring_alphas(const QtcColor *bgnd)
{
    double r = bgnd->red;
    double g = bgnd->green;
    double b = bgnd->blue;
    double h = 0, s = 0, v = 0;
    qtc_rgb_to_hsv(r, g, b, &h, &s, &v);
    qtc_ring_alpha[0] = v * 0.26;
    qtc_ring_alpha[1] = v * 0.14;
    qtc_ring_alpha[2] = v * 0.55;
}
