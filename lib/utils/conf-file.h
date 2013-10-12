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

#ifndef _QTC_UTILS_CONF_FILE_H_
#define _QTC_UTILS_CONF_FILE_H_

#include "conf-desc.h"

typedef union {
    union {
        char *_alloc;
        char _static[0];
    } str_val;
    int int_val;
    int enum_val;
    double float_val;
    bool bool_val;
    QtcColor color_val;
    struct {
        int len;
        union {
            char **_vals;
            char *_buff1[0];
            char _buff2[0];
        };
    } str_list_val;
    struct {
        int len;
        union {
            int *_vals;
            int _buff[0];
        };
    } int_list_val;
    struct {
        int len;
        union {
            double *_vals;
            double _buff[0];
        };
    } float_list_val;
} QtcConfValue;

QTC_BEGIN_DECLS

size_t qtcConfValueSize(const QtcConfValueDesc *desc);
static inline QtcConfValue*
qtcConfValueNew(const QtcConfValueDesc *desc)
{
    return qtcNewSize(QtcConfValue, qtcConfValueSize(desc));
}
void qtcConfValueLoad(QtcConfValue *value, const char *str,
                      const QtcConfValueDesc *desc);
void qtcConfValueDone(QtcConfValue *value, const QtcConfValueDesc *desc);
void qtcConfValueFree(QtcConfValue *value);

QTC_END_DECLS

#endif
