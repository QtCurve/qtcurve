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

#ifndef QTC_UTILS_COLOR_H
#define QTC_UTILS_COLOR_H

#include "utils.h"
#include "options.h"

QTC_BEGIN_DECLS

typedef struct {
    double red;
    double green;
    double blue;
} QtcColor;

typedef struct {
    double h;
    double c;
    double y;
} QtcColorHCY;

extern double qtc_ring_alpha[3];

// use 709 for now
#define HCY_REC 709
#if HCY_REC == 601
static const double _qtc_yc[3] = {0.299, 0.587, 0.114};
#elif HCY_REC == 709
static const double _qtc_yc[3] = {0.2126, 0.7152, 0.0722};
#else // use Qt values
static const double _qtc_yc[3] = {0.34375, 0.5, 0.15625};
#endif
#undef HCY_REC

QTC_ALWAYS_INLINE static inline double
qtc_color_normalize(double a)
{
    if (a >= 1.0)
        return 1;
    if (qtc_unlikely(a < 0.0))
        return 0;
    return a;
}

QTC_ALWAYS_INLINE static inline double
qtc_color_wrap(double a, double d)
{
    double r = fmod(a, d);
    return (r < 0.0 ? d + r : (r > 0.0 ? r : 0.0));
}

QTC_ALWAYS_INLINE static inline double
qtc_color_mixf(double a, double b, double k)
{
    return a + ((b - a) * k);
}

QTC_ALWAYS_INLINE static inline double
qtc_color_HCY_gamma(double n)
{
    return pow(qtc_color_normalize(n), 2.2);
}

QTC_ALWAYS_INLINE static inline double
qtc_color_HCY_igamma(double n)
{
    return pow(qtc_color_normalize(n), 1.0 / 2.2);
}

QTC_ALWAYS_INLINE static inline double
qtc_color_HCY_lumag(double r, double g, double b)
{
    return r * _qtc_yc[0] + g * _qtc_yc[1] + b * _qtc_yc[2];
}

QTC_ALWAYS_INLINE static inline void
qtc_color_fill(QtcColor *color, double r, double g, double b)
{
    color->red = r;
    color->green = g;
    color->blue = b;
}

QTC_ALWAYS_INLINE static inline double
qtc_color_HCY_luma(const QtcColor *color)
{
    return qtc_color_HCY_lumag(qtc_color_HCY_gamma(color->red),
                               qtc_color_HCY_gamma(color->green),
                               qtc_color_HCY_gamma(color->blue));
}

QTC_ALWAYS_INLINE static inline double
qtc_color_mixQreal(double a, double b, double bias)
{
    return a + (b - a) * bias;
}

static inline void
qtc_hsv_to_rgb(double *r, double *g, double *b, double h, double s, double v)
{
    if (0 == s) {
        *r = *g = *b = v;
    } else {
        int i;
        double f;
        double p;

        h /= 60; /* sector 0 to 5 */
        i = (int)floor(h);
        f = h - i; /* factorial part of h */
        p = v * (1 - s);
        switch (i) {
        case 0:
            *r = v;
            *g = v * (1 - s * (1 - f));
            *b = p;
            break;
        case 1:
            *r = v * (1 - s * f);
            *g = v;
            *b = p;
            break;
        case 2:
            *r = p;
            *g = v;
            *b = v * (1 - s * (1 - f));
            break;
        case 3:
            *r = p;
            *g = v * (1 - s * f);
            *b = v;
            break;
        case 4:
            *r = v * (1 - s * (1 - f));
            *g = p;
            *b = v;
            break;
        case 5:
        default:
            *r = v;
            *g = p;
            *b = v * (1 - s * f);
            break;
        }
    }
}

static inline void
qtc_rgb_to_hsv(double r, double g, double b, double *h, double *s, double *v)
{
    double min = QtcMin(QtcMin(r, g), b);
    double max = QtcMax(QtcMax(r, g), b);
    double delta = max - min;

    *v = max;
    if (max != 0) {
        *s = delta / max;
    } else {
        *s = 0;
    }

    if (*s == 0.0) {
        *h = 0.0;
    } else {
        if (r == max) {
            *h = (g - b) / delta; /* between yellow & magenta */
        } else if (g == max) {
            *h = 2 + (b - r) / delta; /* between cyan & yellow */
        } else {
            *h = 4 + (r - g) / delta; /* between magenta & cyan */
        }
        *h *= 60; /* degrees */
        if (*h < 0) {
            *h += 360;
        }
    }
}

void _qtc_color_lighten(QtcColor *color, double ky, double kc);
void _qtc_color_darken(QtcColor *color, double ky, double kc);
void _qtc_color_shade(QtcColor *color, double ky, double kc);
void _qtc_color_tint(const QtcColor *base, const QtcColor *col,
                     double amount, QtcColor *out);
void _qtc_color_mix(const QtcColor *c1, const QtcColor *c2,
                    double bias, QtcColor *out);
void _qtc_shade(const QtcColor *ca, QtcColor *cb, double k, EShading shading);
double _qtc_shine_alpha(const QtcColor *bgnd);
void _qtc_calc_ring_alphas(const QtcColor *bgnd);

#ifndef QTC_UTILS_INTERNAL

// Use __cplusplus to distinguish between gtk and qt for now.
#ifdef __cplusplus
#include <QColor>

QTC_ALWAYS_INLINE static inline QColor
qtc_color_lighten(const QColor *color, double ky, double kc)
{
    QtcColor qtc_color = {color->redF(), color->greenF(), color->blueF()};
    _qtc_color_lighten(&qtc_color, ky, kc);
    return QColor::fromRgbF(qtc_color.red, qtc_color.green, qtc_color.blue);
}

QTC_ALWAYS_INLINE static inline QColor
qtc_color_darken(const QColor *color, double ky, double kc)
{
    QtcColor qtc_color = {color->redF(), color->greenF(), color->blueF()};
    _qtc_color_darken(&qtc_color, ky, kc);
    return QColor::fromRgbF(qtc_color.red, qtc_color.green, qtc_color.blue);
}

QTC_ALWAYS_INLINE static inline QColor
qtc_color_shade(const QColor *color, double ky, double kc)
{
    QtcColor qtc_color = {color->redF(), color->greenF(), color->blueF()};
    _qtc_color_shade(&qtc_color, ky, kc);
    return QColor::fromRgbF(qtc_color.red, qtc_color.green, qtc_color.blue);
}

QTC_ALWAYS_INLINE static inline QColor
qtc_color_tint(const QColor *base, const QColor *col, double amount)
{
    if (amount <= 0.0) {
        return *base;
    } else if (amount >= 1.0) {
        return *col;
    } else if (isnan(amount)) {
        return *base;
    }
    const QtcColor qtc_base = {base->redF(), base->greenF(), base->blueF()};
    const QtcColor qtc_col = {col->redF(), col->greenF(), col->blueF()};
    QtcColor out;
    _qtc_color_tint(&qtc_base, &qtc_col, amount, &out);
    return QColor::fromRgbF(out.red, out.green, out.blue);
}

QTC_ALWAYS_INLINE static inline QColor
qtc_color_mix(const QColor *c1, const QColor *c2, double bias)
{
    if (bias <= 0.0) {
        return *c1;
    } else if (bias >= 1.0) {
        return *c2;
    } else if (isnan(bias)) {
        return *c1;
    }
    const QtcColor qtc_c1 = {c1->redF(), c1->greenF(), c1->blueF()};
    const QtcColor qtc_c2 = {c2->redF(), c2->greenF(), c2->blueF()};
    QtcColor out;
    _qtc_color_mix(&qtc_c1, &qtc_c2, bias, &out);
    return QColor::fromRgbF(out.red, out.green, out.blue);
}

QTC_ALWAYS_INLINE static inline double
qtc_color_luma(const QColor *color)
{
    QtcColor qtc_color = {color->redF(), color->greenF(), color->blueF()};
    return qtc_color_HCY_luma(&qtc_color);
}

QTC_ALWAYS_INLINE static inline void
qtc_shade(const QColor *ca, QColor *cb, double k, EShading shading)
{
    if (qtc_equal(k, 1.0)) {
        *cb = *ca;
        return;
    }
    const QtcColor qtc_ca = {ca->redF(), ca->greenF(), ca->blueF()};
    QtcColor qtc_cb;
    _qtc_shade(&qtc_ca, &qtc_cb, k, shading);
    cb->setRgbF(qtc_cb.red, qtc_cb.green, qtc_cb.blue, ca->alphaF());
}

QTC_ALWAYS_INLINE static inline double
qtc_shine_alpha(const QColor *bgnd)
{
    const QtcColor qtc_bgnd = {bgnd->redF(), bgnd->greenF(), bgnd->blueF()};
    return _qtc_shine_alpha(&qtc_bgnd);
}

QTC_ALWAYS_INLINE static inline void
qtc_calc_ring_alphas(const QColor *bgnd)
{
    const QtcColor qtc_bgnd = {bgnd->redF(), bgnd->greenF(), bgnd->blueF()};
    _qtc_calc_ring_alphas(&qtc_bgnd);
}

#else
#include <gdk/gdk.h>

QTC_ALWAYS_INLINE static inline GdkColor
_qtc_color_to_gdk(const QtcColor *qtc_color)
{
    GdkColor color;
    color.red = qtc_color->red * 65535;
    color.green = qtc_color->green * 65535;
    color.blue = qtc_color->blue * 65535;
    return color;
}

QTC_ALWAYS_INLINE static inline QtcColor
_qtc_color_from_gdk(const GdkColor *color)
{
    QtcColor qtc_color;
    qtc_color.red = color->red / 65535.0;
    qtc_color.green = color->green / 65535.0;
    qtc_color.blue = color->blue / 65535.0;
    return qtc_color;
}

QTC_ALWAYS_INLINE static inline GdkColor
qtc_color_lighten(const GdkColor *color, double ky, double kc)
{
    QtcColor qtc_color = _qtc_color_from_gdk(color);
    _qtc_color_lighten(&qtc_color, ky, kc);
    return _qtc_color_to_gdk(&qtc_color);
}

QTC_ALWAYS_INLINE static inline GdkColor
qtc_color_darken(const GdkColor *color, double ky, double kc)
{
    QtcColor qtc_color = _qtc_color_from_gdk(color);
    _qtc_color_darken(&qtc_color, ky, kc);
    return _qtc_color_to_gdk(&qtc_color);
}

QTC_ALWAYS_INLINE static inline GdkColor
qtc_color_shade(const GdkColor *color, double ky, double kc)
{
    QtcColor qtc_color = _qtc_color_from_gdk(color);
    _qtc_color_shade(&qtc_color, ky, kc);
    return _qtc_color_to_gdk(&qtc_color);
}

QTC_ALWAYS_INLINE static inline GdkColor
qtc_color_tint(const GdkColor *base, const GdkColor *col, double amount)
{
    if (amount <= 0.0) {
        return *base;
    } else if (amount >= 1.0) {
        return *col;
    } else if (isnan(amount)) {
        return *base;
    }
    QtcColor qtc_base = _qtc_color_from_gdk(base);
    QtcColor qtc_col = _qtc_color_from_gdk(col);
    QtcColor out;
    _qtc_color_tint(&qtc_base, &qtc_col, amount, &out);
    return _qtc_color_to_gdk(&out);
}

QTC_ALWAYS_INLINE static inline GdkColor
qtc_color_mix(const GdkColor *c1, const GdkColor *c2, double bias)
{
    if (bias <= 0.0) {
        return *c1;
    } else if (bias >= 1.0) {
        return *c2;
    } else if (isnan(bias)) {
        return *c1;
    }
    QtcColor qtc_c1 = _qtc_color_from_gdk(c1);
    QtcColor qtc_c2 = _qtc_color_from_gdk(c2);
    QtcColor out;
    _qtc_color_mix(&qtc_c1, &qtc_c2, bias, &out);
    return _qtc_color_to_gdk(&out);
}

QTC_ALWAYS_INLINE static inline double
qtc_color_luma(const GdkColor *color)
{
    QtcColor qtc_color = _qtc_color_from_gdk(color);
    return qtc_color_HCY_luma(&qtc_color);
}

QTC_ALWAYS_INLINE static inline void
qtc_shade(const GdkColor *ca, GdkColor *cb, double k, EShading shading)
{
    if (qtc_equal(k, 1.0)) {
        *cb = *ca;
        return;
    }
    QtcColor qtc_ca = _qtc_color_from_gdk(ca);
    QtcColor qtc_cb;
    _qtc_shade(&qtc_ca, &qtc_cb, k, shading);
    *cb = _qtc_color_to_gdk(&qtc_cb);
}

QTC_ALWAYS_INLINE static inline double
qtc_shine_alpha(const GdkColor *bgnd)
{
    QtcColor qtc_bgnd = _qtc_color_from_gdk(bgnd);
    return _qtc_shine_alpha(&qtc_bgnd);
}

QTC_ALWAYS_INLINE static inline void
qtc_calc_ring_alphas(const GdkColor *bgnd)
{
    QtcColor qtc_bgnd = _qtc_color_from_gdk(bgnd);
    _qtc_calc_ring_alphas(&qtc_bgnd);
}

#endif

#endif

QTC_END_DECLS

#endif
