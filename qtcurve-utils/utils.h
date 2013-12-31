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

#ifndef _QTC_UTILS_UTILS_H_
#define _QTC_UTILS_UTILS_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#ifndef __cplucplus
#  include <stdbool.h>
#endif

#include "macros.h"

QTC_ALWAYS_INLINE static inline void*
qtcAlloc0(size_t size)
{
    void *p = malloc(size);
    memset(p, 0, size);
    return p;
}

QTC_ALWAYS_INLINE static inline void
qtcFree(void *p)
{
    if (p) {
        free(p);
    }
}

#define qtcNewSize(type, size) ((type*)qtcAlloc0(size))
#define qtcNew(type, n...)                              \
    qtcNewSize(type, sizeof(type) * QTC_DEFAULT(n, 1))

#ifdef __cplusplus

#define __QTC_USE_DECLVAL
#if defined __cplusplus && defined __GNUC__ && !defined __clang__
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8) ||            \
    (__GNUC__ == 4 && __GNUC_MINOR__ == 8 && __GNUC_PATCHLEVEL__ < 1)
#undef __QTC_USE_DECLVAL
#endif
#endif

#ifdef __QTC_USE_DECLVAL
#include <utility>
#define _QTC_COMP_TYPE(T1, T2)                                  \
    decltype(0 ? std::declval<T1>() : std::declval<T2>())
#else
#define _QTC_COMP_TYPE(T1, T2)                  \
    decltype(0 ? T1() : T2())
#endif
#endif

#ifdef __cplusplus
template <class T>
static inline const T&
const_(const T &t)
{
    return t;
}
#endif

#define QTC_BUFF_TYPE(type)                     \
    struct {                                    \
        union {                                 \
            type *p;                            \
            void *_p;                           \
        };                                      \
        size_t l;                               \
        type *const static_p;                   \
        const size_t static_l;                  \
    }

#define QTC_DEF_LOCAL_BUFF(type, name, stack_size, size)                \
    type __##qtc_local_buff##name[stack_size];                          \
    QTC_BUFF_TYPE(type) name = {                                        \
        {__##qtc_local_buff##name},                                     \
        sizeof(__##qtc_local_buff##name) / sizeof(type),                \
        __##qtc_local_buff##name,                                       \
        sizeof(__##qtc_local_buff##name) / sizeof(type)                 \
    };                                                                  \
    QTC_RESIZE_LOCAL_BUFF(name, size)

#define QTC_RESIZE_LOCAL_BUFF(name, size) do {                          \
        size_t __new_size = (size);                                     \
        if (__new_size <= (name).l || __new_size <= (name).static_l)    \
            break;                                                      \
        (name).l = __new_size;                                          \
        size_t __alloc_size = sizeof(*(name).p) * __new_size;           \
        if ((name).p == (name).static_p) {                              \
            (name)._p = malloc(__alloc_size);                           \
        } else {                                                        \
            (name)._p = realloc((name)._p, __alloc_size);               \
        }                                                               \
    } while (0)

#define QTC_FREE_LOCAL_BUFF(name) do {          \
        if ((name).p != (name).static_p) {      \
            free((name)._p);                    \
        }                                       \
    } while (0)

QTC_BEGIN_DECLS

void *qtcBSearch(const void *key, const void *base, size_t nmemb, size_t size,
                 int (*compar)(const void*, const void*));

typedef struct {
    const char *key;
    int val;
} QtcStrMapItem;

typedef struct {
    unsigned num;
    QtcStrMapItem *items;
    bool auto_val;
    bool inited;
    bool case_sensitive;
} QtcStrMap;

void qtcStrMapInit(QtcStrMap *map);
#define QTC_DECL_STR_MAP(map, case_sense, items...)      \
    static QtcStrMapItem __##map##_items[] = {items};    \
    static QtcStrMap map = {                             \
        sizeof(__##map##_items) / sizeof(QtcStrMapItem), \
        __##map##_items,                                 \
        false,                                           \
        false,                                           \
        case_sense,                                      \
    };                                                   \
    if (!map.inited) {                                   \
        qtcStrMapInit(&map);                             \
    }

static inline void
qtcStrMapInitKeys(QtcStrMap *map, const char **keys)
{
    if (map->inited) {
        return;
    }
    for (unsigned i = 0;i < map->num;i++) {
        map->items[i].key = keys[i];
    }
    qtcStrMapInit(map);
}

#define QTC_DECL_STR_MAP_AUTO(map, case_sense, keys...)           \
    static const char *__##map##_keys[] = {keys};                 \
    static QtcStrMapItem __##map##_items[sizeof(__##map##_keys) / \
                                         sizeof(const char*)];    \
    static QtcStrMap map = {                                      \
        sizeof(__##map##_keys) / sizeof(const char*),             \
        __##map##_items,                                          \
        true,                                                     \
        false,                                                    \
        case_sense,                                               \
    };                                                            \
    qtcStrMapInitKeys(&map, __##map##_keys);

int qtcStrMapSearch(const QtcStrMap *map, const char *key, int def);
#define _qtcStrMapSearch(map, key, def, ...)    \
    (qtcStrMapSearch)(map, key, def)
#define qtcStrMapSearch(map, key, def...)       \
    _qtcStrMapSearch(map, key, ##def, 0)

QTC_END_DECLS

#ifdef __cplusplus
template<typename T>
QTC_ALWAYS_INLINE static inline bool
qtcOneOf(T &&value)
{
    return false;
}
template<typename T, typename First>
QTC_ALWAYS_INLINE static inline bool
qtcOneOf(T &&value, First &&first)
{
    return value == first;
}
template<typename T, typename First, typename... Rest>
QTC_ALWAYS_INLINE static inline bool
qtcOneOf(T &&value, First &&first, Rest &&...rest...)
{
    return value == first || qtcOneOf(std::forward<T>(value),
                                      std::forward<Rest>(rest)...);
}
#else
#define qtcOneOf(exp, args...)                                  \
    ({                                                          \
        const typeof((exp)) __val = (exp);                      \
        const typeof((exp)) __args[] = {args};                  \
        size_t __arg_n = sizeof(__args) / sizeof(__args[0]);    \
        bool __res = false;                                     \
        for (size_t __i = 0;__i < __arg_n;__i++) {              \
            if (__val == __args[__i]) {                         \
                __res = true;                                   \
                break;                                          \
            }                                                   \
        }                                                       \
        __res;                                                  \
    })
#endif
#define qtcNoneOf(args...) (!qtcOneOf(args))

#endif
