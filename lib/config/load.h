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

#ifndef __QTC_CONFIG_LOAD_H__
#define __QTC_CONFIG_LOAD_H__

#include "option.h"
#include <qtcurve-utils/ini-parse.h>

QTC_BEGIN_DECLS

bool qtcConfigLoadBool(QtcIniFile *file, const char *grp,
                       const char *name, QtcIniGroup **grp_cache,
                       QtcIniEntry **ety_cache, bool def, bool *is_def);
long qtcConfigLoadInt(QtcIniFile *file, const char *grp,
                      const char *name, QtcIniGroup **grp_cache,
                      QtcIniEntry **ety_cache,
                      const QtcConfIntConstrain *c, long def, bool *is_def);
double qtcConfigLoadFloat(QtcIniFile *file, const char *grp,
                          const char *name, QtcIniGroup **grp_cache,
                          QtcIniEntry **ety_cache,
                          const QtcConfFloatConstrain *c,
                          double def, bool *is_def);
char *qtcConfigLoadStr(QtcIniFile *file, const char *grp,
                       const char *name, QtcIniGroup **grp_cache,
                       QtcIniEntry **ety_cache,
                       const QtcConfStrConstrain *c, const char *def,
                       char *buff, bool is_static, bool *is_def);
void qtcConfigFreeStr(char *val, bool is_static);
unsigned qtcConfigLoadEnum(QtcIniFile *file, const char *grp,
                           const char *name, QtcIniGroup **grp_cache,
                           QtcIniEntry **ety_cache,
                           const QtcConfEnumConstrain *c,
                           unsigned def, bool *is_def);

QTC_END_DECLS

#endif
