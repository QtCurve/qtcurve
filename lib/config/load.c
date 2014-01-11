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
#include "option_p.h"
#include <qtcurve-utils/log.h>
#include <qtcurve-utils/number.h>

QTC_EXPORT bool
qtcConfigLoadBool(QtcIniFile *file, const char *grp, const char *name,
                  QtcIniGroup **grp_cache, QtcIniEntry **ety_cache, bool def,
                  bool *is_def)
{
    QtcIniEntry *ety = qtcConfigFindEntry(file, grp, name, grp_cache,
                                          ety_cache, false);
    if (ety && ety->value) {
        qtcAssign(is_def, false);
        return qtcStrToBool(ety->value, def);
    }
    qtcAssign(is_def, true);
    return def;
}

static void
_checkIntDef(const char *grp, const char *name,
             const QtcConfIntConstrain *c, long *def)
{
    if (qtcUnlikely(c->min > *def || c->max < *def)) {
        qtcWarn("Illegal default value %ld for option %s/%s.\n",
                *def, grp, name);
        *def = qtcBound(c->min, *def, c->max);
    }
}

QTC_EXPORT long
qtcConfigLoadInt(QtcIniFile *file, const char *grp, const char *name,
                 QtcIniGroup **grp_cache, QtcIniEntry **ety_cache,
                 const QtcConfIntConstrain *c, long def, bool *is_def)
{
    QtcIniEntry *ety = qtcConfigFindEntry(file, grp, name, grp_cache,
                                          ety_cache, false);
    if (c) {
        _checkIntDef(grp, name, c, &def);
    }
    if (!ety || !ety->value) {
        qtcAssign(is_def, true);
        return def;
    }
    return qtcBound(c->min, qtcStrToInt(ety->value, def, is_def), c->max);
}

static void
_checkFloatDef(const char *grp, const char *name,
               const QtcConfFloatConstrain *c, double *def)
{
    if (qtcUnlikely(c->min > *def || c->max < *def)) {
        qtcWarn("Illegal default value %lf for option %s/%s.\n",
                *def, grp, name);
        *def = qtcBound(c->min, *def, c->max);
    }
}

QTC_EXPORT double
qtcConfigLoadFloat(QtcIniFile *file, const char *grp, const char *name,
                   QtcIniGroup **grp_cache, QtcIniEntry **ety_cache,
                   const QtcConfFloatConstrain *c, double def, bool *is_def)
{
    QtcIniEntry *ety = qtcConfigFindEntry(file, grp, name, grp_cache,
                                          ety_cache, false);
    if (c) {
        _checkFloatDef(grp, name, c, &def);
    }
    if (!ety || !ety->value) {
        qtcAssign(is_def, true);
        return def;
    }
    return qtcBound(c->min, qtcStrToFloat(ety->value, def, is_def), c->max);
}

static char*
_setStrN(char *dest, const char *src, size_t max_len)
{
    if (max_len) {
        return qtcSetStr(dest, src);
    } else {
        return qtcSetStr(dest, src, qtcMin(strlen(src), max_len));
    }
}

QTC_EXPORT char*
qtcConfigLoadStr(QtcIniFile *file, const char *grp, const char *name,
                 QtcIniGroup **grp_cache, QtcIniEntry **ety_cache,
                 const QtcConfStrConstrain *c, const char *def,
                 char *buff, bool is_static, bool *is_def)
{
    unsigned max_len = c && c->max_len ? max_len : 0;
    if (!def) {
        def = "";
    } else if (max_len && max_len < strlen(def)) {
        qtcWarn("Illegal default value %s for option %s/%s.\n", def, grp, name);
    }
    QtcIniEntry *ety = qtcConfigFindEntry(file, grp, name, grp_cache,
                                          ety_cache, false);
    qtcAssign(is_def, !(ety && ety->value));
    if (is_static) {
        strncpy(buff, ety && ety->value ? ety->value : def, max_len);
        buff[max_len] = '\0';
        return buff;
    } else {
        QTC_RET_IF_FAIL(ety && ety->value, _setStrN(buff, def, max_len));
        return _setStrN(buff, ety->value, max_len);
    }
}

QTC_EXPORT void
qtcConfigFreeStr(char *val, bool is_static)
{
    if (!is_static) {
        qtcFree(val);
    }
}

QTC_EXPORT unsigned
qtcConfigLoadEnum(QtcIniFile *file, const char *grp, const char *name,
                  QtcIniGroup **grp_cache, QtcIniEntry **ety_cache,
                  const QtcConfEnumConstrain *c, unsigned def, bool *is_def)
{
    QtcIniEntry *ety = qtcConfigFindEntry(file, grp, name, grp_cache,
                                          ety_cache, false);
    if (!(ety && ety->value && c && c->num && c->items)) {
        qtcAssign(is_def, true);
        return def;
    }
    const QtcStrMap map = {
        .items = c->items,
        .num = c->num,
        .size = sizeof(QtcEnumItem),
        .inited = true,
        .case_sensitive = false
    };
    return qtcEnumSearch(&map, ety->value, def, is_def);
}
