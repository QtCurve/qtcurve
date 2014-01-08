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

#include <qtcurve-utils/ini-parse.h>

QTC_BEGIN_DECLS

bool qtcConfigLoadBool(const QtcIniFile *file, const char *grp,
                       const char *name, const QtcIniGroup **grp_cache,
                       const QtcIniEntry **ety_cache, bool def);
void qtcConfigFreeBool(QtcIniGroup **grp_cache, QtcIniEntry **ety_cache);
long qtcConfigLoadInt(const QtcIniFile *file, const char *grp,
                      const char *name, const QtcIniGroup **grp_cache,
                      const QtcIniEntry **ety_cache, long def);
void qtcConfigFreeInt(QtcIniGroup **grp_cache, QtcIniEntry **ety_cache);
double qtcConfigLoadFloat(const QtcIniFile *file, const char *grp,
                          const char *name, const QtcIniGroup **grp_cache,
                          const QtcIniEntry **ety_cache, double def);
void qtcConfigFreeFloat(QtcIniGroup **grp_cache, QtcIniEntry **ety_cache);
char *qtcConfigLoadStr(const QtcIniFile *file, const char *grp,
                       const char *name, const QtcIniGroup **grp_cache,
                       const QtcIniEntry **ety_cache, const char *def);
void qtcConfigFreeStr(QtcIniGroup **grp_cache, QtcIniEntry **ety_cache,
                      char *val);

QTC_END_DECLS

#endif
