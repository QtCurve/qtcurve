/*****************************************************************************
 *   Copyright 2013 - 2014 Yichao Yu <yyc1992@gmail.com>                     *
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

#ifndef _QTC_UTILS_MAP_H_
#define _QTC_UTILS_MAP_H_

#include "utils.h"

QTC_BEGIN_DECLS

typedef struct {
    void *items;
    const unsigned num;
    const unsigned size;
    bool inited: 1;
    const bool case_sensitive: 1;
} QtcStrMap;
void qtcStrMapInit(QtcStrMap *map);
void *qtcStrMapSearch(const QtcStrMap *map, const char *key);

typedef struct {
    const char *key;
    unsigned val;
} QtcEnumItem;

#define QTC_DEF_ENUM(map, case_sense, items...)          \
    static QtcEnumItem __##map##_items[] = {items};      \
    static QtcStrMap map = {                             \
        __##map##_items,                                 \
        sizeof(__##map##_items) / sizeof(QtcEnumItem),   \
        sizeof(QtcEnumItem),                             \
        false,                                           \
        case_sense                                       \
    };                                                   \
    if (!map.inited) {                                   \
        qtcStrMapInit(&map);                             \
    }

static inline void
_qtcEnumInitKeys(QtcStrMap *map, const char **keys)
{
    if (map->inited) {
        return;
    }
    QtcEnumItem *items = (QtcEnumItem*)map->items;
    for (unsigned i = 0;i < map->num;i++) {
        items[i].key = keys[i];
        items[i].val = i;
    }
    qtcStrMapInit(map);
}

#define QTC_DEF_ENUM_AUTO(map, case_sense, keys...)               \
    static const char *__##map##_keys[] = {keys};                 \
    static QtcEnumItem __##map##_items[sizeof(__##map##_keys) /   \
                                       sizeof(const char*)];      \
    static QtcStrMap map = {                                      \
        __##map##_items,                                          \
        sizeof(__##map##_items) / sizeof(QtcEnumItem),            \
        sizeof(QtcEnumItem),                                      \
        false,                                                    \
        case_sense                                                \
    };                                                            \
    _qtcEnumInitKeys(&map, __##map##_keys)

QTC_ALWAYS_INLINE static inline unsigned
qtcEnumSearch(const QtcStrMap *map, const char *key, unsigned def, bool *is_def)
{
    QtcEnumItem *item = (QtcEnumItem*)qtcStrMapSearch(map, key);
    qtcAssign(is_def, !item);
    return item ? item->val : def;
}
#define _qtcEnumSearch(map, key, def, is_def, ...)              \
    qtcEnumSearch(map, key, (unsigned)(def), (bool*)is_def)
#define qtcEnumSearch(map, key, def...)         \
    _qtcEnumSearch(map, key, ##def, 0, 0)

QTC_END_DECLS

#endif
