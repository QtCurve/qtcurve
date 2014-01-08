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

#include "load.h"

static const QtcIniEntry*
qtcConfigFindEntry(const QtcIniFile *file, const char *grp, const char *name,
                   const QtcIniGroup **grp_cache, const QtcIniEntry **ety_cache)
{
    QTC_RET_IF_FAIL(file, NULL);
    if (ety_cache && *ety_cache) {
        return *ety_cache;
    }
    const QtcIniGroup *ini_grp = NULL;
    if (grp_cache && *grp_cache) {
        ini_grp = *grp_cache;
    } else {
        ini_grp = qtcIniFileFindGroup(file, grp);
        QTC_RET_IF_FAIL(ini_grp, NULL);
        if (grp_cache) {
            *grp_cache = qtcIniGroupRef((QtcIniGroup*)ini_grp);
        }
    }
    QtcIniEntry *ini_ety = qtcIniGroupFindEntry(ini_grp, name);
    QTC_RET_IF_FAIL(ini_ety, NULL);
    if (ety_cache) {
        *ety_cache = qtcIniEntryRef(ini_ety);
    }
    return ini_ety;
}

static void
qtcConfigFreeCache(QtcIniGroup **grp_cache, QtcIniEntry **ety_cache)
{
    if (ety_cache && *ety_cache) {
        qtcIniEntryUnref(*ety_cache);
    }
    if (grp_cache && *grp_cache) {
        qtcIniGroupUnref(*grp_cache);
    }
}

QTC_EXPORT bool
qtcConfigLoadBool(const QtcIniFile *file, const char *grp, const char *name,
                  const QtcIniGroup **grp_cache, const QtcIniEntry **ety_cache,
                  bool def)
{
    const QtcIniEntry *ety = qtcConfigFindEntry(file, grp, name,
                                                grp_cache, ety_cache);
    QTC_RET_IF_FAIL(ety && ety->value, def);
    return qtcStrToBool(ety->value, def);
}

QTC_EXPORT void
qtcConfigFreeBool(QtcIniGroup **grp_cache, QtcIniEntry **ety_cache)
{
    qtcConfigFreeCache(grp_cache, ety_cache);
}

QTC_EXPORT long
qtcConfigLoadInt(const QtcIniFile *file, const char *grp, const char *name,
                 const QtcIniGroup **grp_cache, const QtcIniEntry **ety_cache,
                 long def)
{
    const QtcIniEntry *ety = qtcConfigFindEntry(file, grp, name,
                                                grp_cache, ety_cache);
    QTC_RET_IF_FAIL(ety && ety->value, def);
    return qtcStrToInt(ety->value, def);
}

QTC_EXPORT void
qtcConfigFreeInt(QtcIniGroup **grp_cache, QtcIniEntry **ety_cache)
{
    qtcConfigFreeCache(grp_cache, ety_cache);
}

QTC_EXPORT char*
qtcConfigLoadStr(const QtcIniFile *file, const char *grp, const char *name,
                 const QtcIniGroup **grp_cache, const QtcIniEntry **ety_cache,
                 const char *def)
{
    if (!def) {
        def = "";
    }
    const QtcIniEntry *ety = qtcConfigFindEntry(file, grp, name,
                                                grp_cache, ety_cache);
    QTC_RET_IF_FAIL(ety && ety->value, strdup(def));
    return strdup(ety->value);
}

QTC_EXPORT void
qtcConfigFreeStr(QtcIniGroup **grp_cache, QtcIniEntry **ety_cache, char *val)
{
    qtcFree(val);
    qtcConfigFreeCache(grp_cache, ety_cache);
}
