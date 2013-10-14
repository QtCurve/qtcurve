/***************************************************************************
 *   Copyright (C) 2012~2013 by Yichao Yu                                  *
 *   yyc1992@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#ifndef _QTC_UTILS_STRS_H_
#define _QTC_UTILS_STRS_H_

#include "utils.h"

QTC_ALWAYS_INLINE static inline size_t
_qtcCatStrsCalLens(int n, const char **strs, size_t *lens)
{
    size_t total_len = 0;
    for (int i = 0;i < n;i++) {
        lens[i] = strlen(strs[i]);
        total_len += lens[i];
    }
    return total_len;
}

QTC_ALWAYS_INLINE static inline char*
_qtcCatStrsFill(int n, const char **strs, size_t *lens,
                size_t total_len, char *res)
{
    char *p = res;
    for (int i = 0;i < n;i++) {
        memcpy(p, strs[i], lens[i]);
        p += lens[i];
    }
    res[total_len] = 0;
    return res;
}

#define _qtcCatStrs(var, strs...)                                       \
    do {                                                                \
        const char *__strs[] = {strs};                                  \
        int __strs_n = sizeof(__strs) / sizeof(const char*);            \
        size_t __strs_lens[sizeof(__strs) / sizeof(const char*)];       \
        size_t __strs_total_len =                                       \
            _qtcCatStrsCalLens(__strs_n, __strs, __strs_lens);          \
        var = _qtcCatStrsFill(__strs_n, __strs, __strs_lens,            \
                              __strs_total_len,                         \
                              malloc(__strs_total_len + 1));            \
    } while (0)

#define _qtcFillStrs(var, buff, strs...)                                \
    do {                                                                \
        const char *__strs[] = {strs};                                  \
        int __strs_n = sizeof(__strs) / sizeof(const char*);            \
        size_t __strs_lens[sizeof(__strs) / sizeof(const char*)];       \
        size_t __strs_total_len =                                       \
            _qtcCatStrsCalLens(__strs_n, __strs, __strs_lens);          \
        var = _qtcCatStrsFill(__strs_n, __strs, __strs_lens,            \
                              __strs_total_len,                         \
                              realloc(buff, __strs_total_len + 1));     \
    } while (0)

// Gcc 4.4.5 produce error when using sizeof array in variadic template
#if defined __cplusplus && defined __GNUC__ && !defined __clang__
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8)
#define __QTC_CAT_STR_NO_TEMPLATE
#endif
#endif

#if !defined __cplusplus || defined __QTC_CAT_STR_NO_TEMPLATE
#define qtcCatStrs(strs...)                     \
    ({                                          \
        char *__cat_str_res;                    \
        _qtcCatStrs(__cat_str_res, strs);       \
        __cat_str_res;                          \
    })

#define qtcFillStrs(buff, strs...)                      \
    ({                                                  \
        char *__fill_str_res;                           \
        _qtcFillStrs(__fill_str_res, buff, strs);       \
        __fill_str_res;                                 \
    })
#else
template <typename... ArgTypes>
QTC_ALWAYS_INLINE static inline char*
qtcCatStrs(ArgTypes... strs...)
{
    char *res;
    _qtcCatStrs(res, strs...);
    return res;
}
template <typename... ArgTypes>
QTC_ALWAYS_INLINE static inline char*
qtcFillStrs(char *buff, ArgTypes... strs...)
{
    char *res;
    _qtcFillStrs(res, buff, strs...);
    return res;
}
#endif

QTC_ALWAYS_INLINE static inline char*
qtcSetStrWithLen(char *dest, const char *src, size_t len)
{
    dest = realloc(dest, len + 1);
    memcpy(dest, src, len);
    dest[len] = '\0';
    return dest;
}

QTC_ALWAYS_INLINE static inline char*
qtcSetStr(char *dest, const char *src)
{
    return qtcSetStrWithLen(dest, src, strlen(src));
}

#endif
