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
#include <stdarg.h>

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
        const int __strs_n = sizeof(__strs) / sizeof(const char*);      \
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
        const int __strs_n = sizeof(__strs) / sizeof(const char*);      \
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

QTC_BEGIN_DECLS

__attribute__((format(printf, 4, 5)))
char *_qtcSPrintf(char *buff, size_t *size, bool allocated,
                  const char *fmt, ...);
__attribute__((format(printf, 4, 0)))
char *_qtcSPrintfV(char *buff, size_t *size, bool allocated,
                   const char *fmt, va_list ap);

#define qtcSPrintfV(buff, size, fmt, ap)        \
    _qtcSPrintfV(buff, size, true, fmt, ap)
#define qtcSPrintf(buff, size, fmt, ap, args...)        \
    _qtcSPrintf(buff, size, true, fmt, ap, ##args)

__attribute__((format(printf, 3, 0)))
QTC_ALWAYS_INLINE static inline char*
qtcASNPrintfV(char *buff, size_t size, const char *fmt, va_list ap)
{
    return qtcSPrintfV(buff, &size, fmt, ap);
}

__attribute__((format(printf, 3, 4)))
QTC_ALWAYS_INLINE static inline char*
qtcASNPrintf(char *buff, size_t size, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char *res = qtcASNPrintfV(buff, size, fmt, ap);
    va_end(ap);
    return res;
}

#define qtcASPrintfV(fmt, ap)                   \
    qtcASNPrintfV(NULL, 0, fmt, ap)
#define qtcASPrintf(fmt, args...)               \
    qtcASNPrintf(NULL, 0, fmt, ##args)

QTC_END_DECLS

typedef QTC_BUFF_TYPE(char) QtcStrBuff;

#define QTC_DEF_STR_BUFF(name, stack_size, size)                \
    char __##qtc_local_buff##name[stack_size];                  \
    QtcStrBuff name = {                                         \
        {__##qtc_local_buff##name},                             \
        sizeof(__##qtc_local_buff##name) / sizeof(char),        \
        __##qtc_local_buff##name,                               \
        sizeof(__##qtc_local_buff##name) / sizeof(char)         \
    };                                                          \
    QTC_RESIZE_LOCAL_BUFF(name, size)

#define _QTC_LOCAL_BUFF_PRINTF(name, fmt, args...) do {                 \
        if ((name).p == (name).static_p) {                              \
            size_t _size = (name).l;                                    \
            char *__res = _qtcSPrintf((name).p, &_size, false, fmt, ##args); \
            if (__res != (name).p) {                                    \
                (name).l = _size;                                       \
                (name).p = __res;                                       \
            }                                                           \
            break;                                                      \
        }                                                               \
        (name).p = _qtcSPrintf((name).p, &(name).l, true, fmt, ##args); \
    } while (0)

#define _QTC_LOCAL_BUFF_CAT_STR(name, strs...) do {                     \
        const char *__strs[] = {strs};                                  \
        const int __strs_n = sizeof(__strs) / sizeof(const char*);      \
        size_t __strs_lens[sizeof(__strs) / sizeof(const char*)];       \
        size_t __strs_total_len =                                       \
            _qtcCatStrsCalLens(__strs_n, __strs, __strs_lens);          \
        QTC_RESIZE_LOCAL_BUFF(name, __strs_total_len + 1);              \
        _qtcCatStrsFill(__strs_n, __strs, __strs_lens,                  \
                        __strs_total_len, (name).p);                    \
    } while (0)

#define QTC_LOCAL_BUFF_PRINTF(name, fmt, args...)       \
    ({                                                  \
        _QTC_LOCAL_BUFF_PRINTF(name, fmt, ##args);      \
        (name).p;                                       \
    })

#define QTC_LOCAL_BUFF_CAT_STR(name, strs...)   \
    ({                                          \
        _QTC_LOCAL_BUFF_CAT_STR(name, ##strs);  \
        (name).p;                               \
    })

#endif
