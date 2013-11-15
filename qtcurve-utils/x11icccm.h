/*****************************************************************************
 *   Copyright 2013 - 2013 Yichao Yu <yyc1992@gmail.com>                     *
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

#ifndef _QTC_UTILS_X11ICCCM_H_
#define _QTC_UTILS_X11ICCCM_H_

#include "x11utils.h"

QTC_BEGIN_DECLS

// Copied from xcb-icccm
typedef enum {
    QTC_X11_SIZE_HINT_US_POSITION = 1 << 0,
    QTC_X11_SIZE_HINT_US_SIZE = 1 << 1,
    QTC_X11_SIZE_HINT_P_POSITION = 1 << 2,
    QTC_X11_SIZE_HINT_P_SIZE = 1 << 3,
    QTC_X11_SIZE_HINT_P_MIN_SIZE = 1 << 4,
    QTC_X11_SIZE_HINT_P_MAX_SIZE = 1 << 5,
    QTC_X11_SIZE_HINT_P_RESIZE_INC = 1 << 6,
    QTC_X11_SIZE_HINT_P_ASPECT = 1 << 7,
    QTC_X11_SIZE_HINT_BASE_SIZE = 1 << 8,
    QTC_X11_SIZE_HINT_P_WIN_GRAVITY = 1 << 9,
} QtcX11SizeHintFlags;

typedef struct {
    /** User specified flags */
    uint32_t flags;
    /** User-specified position */
    int32_t x;
    int32_t y;
    /** User-specified size */
    int32_t width;
    int32_t height;
    /** Program-specified minimum size */
    int32_t min_width;
    int32_t min_height;
    /** Program-specified maximum size */
    int32_t max_width;
    int32_t max_height;
    /** Program-specified resize increments */
    int32_t width_inc;
    int32_t height_inc;
    /** Program-specified minimum aspect ratios */
    int32_t min_aspect_num;
    int32_t min_aspect_den;
    /** Program-specified maximum aspect ratios */
    int32_t max_aspect_num;
    int32_t max_aspect_den;
    /** Program-specified base size */
    int32_t base_width;
    int32_t base_height;
    /** Program-specified window gravity */
    uint32_t win_gravity;
} QtcX11SizeHint;

#define QTC_X11_SIZE_HINTS_ELEMENTS 18

void qtcX11GetSizeHint(xcb_window_t wid, QtcX11SizeHint *hint);
void qtcX11SetSizeHint(xcb_window_t wid, const QtcX11SizeHint *hint);

QTC_END_DECLS

#endif
