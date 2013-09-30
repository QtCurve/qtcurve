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
    if (amount <= 0.0)
        return *base;
    if (amount >= 1.0)
        return *col;
    if (isnan(amount))
        return *base;
    const QtcColor qtc_base = {base->redF(), base->greenF(), base->blueF()};
    const QtcColor qtc_col = {col->redF(), col->greenF(), col->blueF()};
    QtcColor out;
    _qtc_color_tint(&qtc_base, &qtc_col, amount, &out);
    return QColor::fromRgbF(out.red, out.green, out.blue);
}

QTC_ALWAYS_INLINE static inline QColor
qtc_color_mix(const QColor *c1, const QColor *c2, double bias)
{
    if (bias <= 0.0)
        return *c1;
    if (bias >= 1.0)
        return *c2;
    if (isnan(bias))
        return *c1;
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
    const QtcColor qtc_ca = {ca->redF(), ca->greenF(), ca->blueF()};
    QtcColor qtc_cb;
    _qtc_shade(&qtc_ca, &qtc_cb, k, shading);
    cb->setRgbF(qtc_cb.red, qtc_cb.green, qtc_cb.blue, ca->alpha());
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

#endif

#endif

QTC_END_DECLS

#endif
