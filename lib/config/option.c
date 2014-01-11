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

#include "option_p.h"

static QtcIniGroup*
qtcConfigFindGroup(QtcIniFile *file, const char *grp,
                   QtcIniGroup **grp_cache, bool write)
{
    QTC_RET_IF_FAIL(file, NULL);
    if (grp_cache && *grp_cache) {
        if (qtcIniFileHasGroup(file, *grp_cache)) {
            return *grp_cache;
        }
        if (!write) {
            return NULL;
        }
        qtcIniFileInsertGroup(file, NULL, *grp_cache, false, false);
        return *grp_cache;
    }
    QtcIniGroup *ini_grp = qtcIniFileFindGroup(file, grp);
    if (ini_grp) {
        qtcAssign(grp_cache, qtcIniGroupRef(ini_grp));
        return ini_grp;
    }
    if (!write) {
        return NULL;
    }
    ini_grp = qtcIniFileEnsureGroup(file, grp);
    qtcAssign(grp_cache, qtcIniGroupRef(ini_grp));
    return ini_grp;
}

static QtcIniEntry*
qtcConfigGroupFindEntry(QtcIniGroup *grp, const char *ety,
                        QtcIniEntry **ety_cache, bool write)
{
    QTC_RET_IF_FAIL(grp, NULL);
    if (ety_cache && *ety_cache) {
        if (qtcIniGroupHasEntry(grp, *ety_cache)) {
            return *ety_cache;
        }
        if (!write) {
            return NULL;
        }
        qtcIniGroupInsertEntry(grp, NULL, *ety_cache, false, false);
        return *ety_cache;
    }
    QtcIniEntry *ini_ety = qtcIniGroupFindEntry(grp, ety);
    if (ini_ety) {
        qtcAssign(ety_cache, qtcIniEntryRef(ini_ety));
        return ini_ety;
    }
    if (!write) {
        return NULL;
    }
    ini_ety = qtcIniGroupEnsureEntry(grp, ety);
    qtcAssign(ety_cache, qtcIniEntryRef(ini_ety));
    return ini_ety;
}

QtcIniEntry*
qtcConfigFindEntry(QtcIniFile *file, const char *grp, const char *ety,
                   QtcIniGroup **grp_cache, QtcIniEntry **ety_cache, bool write)
{
    return qtcConfigGroupFindEntry(
        qtcConfigFindGroup(file, grp, grp_cache, write), ety, ety_cache, write);
}

QTC_EXPORT void
_qtcConfigFreeGroupCaches(unsigned num, QtcIniGroup ***caches)
{
    for (unsigned i = 0;i < num;i++) {
        if (caches[i] && *caches[i]) {
            qtcIniGroupUnref(*caches[i]);
            *caches[i] = NULL;
        }
    }
}

QTC_EXPORT void
_qtcConfigFreeEntryCaches(unsigned num, QtcIniEntry ***caches)
{
    for (unsigned i = 0;i < num;i++) {
        if (caches[i] && *caches[i]) {
            qtcIniEntryUnref(*caches[i]);
            *caches[i] = NULL;
        }
    }
}
