/*****************************************************************************
 *   Copyright 2014 - 2014 Yichao Yu <yyc1992@gmail.com>                     *
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

#ifndef __QTC_CONFIG_OPTION_H__
#define __QTC_CONFIG_OPTION_H__

#include <qtcurve-utils/map.h>

// Int
typedef struct {
    long min;
    long max;
} QtcConfIntConstrain;

// Float
typedef struct {
    double min;
    double max;
} QtcConfFloatConstrain;

// String
typedef struct {
    unsigned max_len;
} QtcConfStrConstrain;

// Enum
typedef struct {
    unsigned num;
    QtcEnumItem *items;
} QtcConfEnumConstrain;

// String List
typedef struct {
    unsigned max_strlen;

    unsigned min_count;
    unsigned max_count;
} QtcConfStrListConstrain;

// Int List
typedef struct {
    long min_val;
    long max_val;

    unsigned min_count;
    unsigned max_count;
} QtcConfIntListConstrain;

// Float List
typedef struct {
    double min_val;
    double max_val;

    unsigned min_count;
    unsigned max_count;
} QtcConfFloatListConstrain;

#endif
