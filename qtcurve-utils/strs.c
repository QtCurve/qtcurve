/***************************************************************************
 *   Copyright (C) 2013~2013 by Yichao Yu                                  *
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

#include "strs.h"
#include "number.h"

QTC_EXPORT char*
_qtcSPrintf(char *buff, size_t *size, bool allocated, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char *res = _qtcSPrintfV(buff, size, allocated, fmt, ap);
    va_end(ap);
    return res;
}

QTC_EXPORT char*
_qtcSPrintfV(char *buff, size_t *_size, bool allocated,
             const char *fmt, va_list ap)
{
    if (!buff || !_size || !*_size) {
        char *res = NULL;
        vasprintf(&res, fmt, ap);
        return res;
    }
    va_list _ap;
    va_copy(_ap, ap);
    size_t size = *_size;
    size_t new_size = vsnprintf(buff, size, fmt, ap) + 1;
    if (new_size > size) {
        new_size = qtcAlignTo(new_size, 1024);
        if (allocated) {
            buff = realloc(buff, new_size);
        } else {
            buff = malloc(new_size);
        }
        *_size = new_size;
        new_size = vsnprintf(buff, new_size, fmt, _ap);
    }
    return buff;
}

QTC_EXPORT void
qtcStrListForEach(const char *str, char delim, char escape,
                  QtcListForEachFunc func, void *data)
{
    QTC_DEF_STR_BUFF(str_buff, 1024, 1024);
    if (qtcUnlikely(escape == delim)) {
        escape = '\0';
    }
    const char key[] = {delim, escape, '\0'};
    const char *p = str;
    while (true) {
        size_t len = 0;
        while (true) {
            size_t sub_len = strcspn(p, key);
            QTC_RESIZE_LOCAL_BUFF(str_buff, len + sub_len + 2);
            memcpy(str_buff.p + len, p, sub_len);
            len += sub_len;
            p += sub_len;
            if (escape && *p == escape) {
                str_buff.p[len] = p[1];
                if (qtcUnlikely(!p[1])) {
                    p++;
                    break;
                }
                len++;
                p += 2;
            } else {
                str_buff.p[len] = '\0';
                break;
            }
        }
        func(str_buff.p, len, data);
        if (!*p) {
            break;
        }
        p++;
    }
    QTC_FREE_LOCAL_BUFF(str_buff);
}

typedef struct {
    size_t size;
    size_t nele;
    void *buff;
    QtcListEleLoader loader;
    void *data;
    size_t offset;
} QtcStrLoadListData;

static void
qtcStrListLoader(const char *str, size_t len, void *_data)
{
    QtcStrLoadListData *data = (QtcStrLoadListData*)_data;
    if (data->nele <= data->offset) {
        data->nele += 8;
        data->buff = realloc(data->buff, data->nele * data->size);
    }
    data->loader((char*)data->buff + data->offset * data->size,
                 str, len, data->data);
    data->offset++;
}

QTC_EXPORT void*
qtcStrLoadList(const char *str, char delim, char escape, size_t size,
               size_t *_nele, void *buff, QtcListEleLoader loader, void *data)
{
    if (qtcUnlikely(!_nele || !size || !loader)) {
        return NULL;
    }
    QtcStrLoadListData loader_data = {
        .size = size,
        .nele = *_nele,
        .buff = buff,
        .loader = loader,
        .data = data,
        .offset = 0,
    };
    if (!(loader_data.nele && loader_data.buff)) {
        loader_data.nele = 16;
        loader_data.buff = malloc(16 * size);
    }
    qtcStrListForEach(str, delim, escape, qtcStrListLoader, &loader_data);
    *_nele = loader_data.offset;
    return loader_data.buff;
}

static void
qtcStrListStrLoader(void *ele, const char *str, size_t len, void *data)
{
    const char *def = data;
    if (def && !str[0]) {
        *(char**)ele = strdup(def);
    } else {
        *(char**)ele = qtcSetStrWithLen(NULL, str, len);
    }
}

QTC_EXPORT char**
qtcStrLoadStrList(const char *str, char delim, char escape, size_t *nele,
                  char **buff, const char *def)
{
    return qtcStrLoadList(str, delim, escape, sizeof(char*), nele, buff,
                          qtcStrListStrLoader, (void*)def);
}

static void
qtcStrListIntLoader(void *ele, const char *str, size_t len, void *data)
{
    QTC_UNUSED(len);
    long def = (long)(intptr_t)data;
    str += strspn(str, " \t\b\n\f\v");
    char *end = NULL;
    long res = strtol(str, &end, 0);
    if (end == str) {
        res = def;
    }
    *(long*)ele = res;
}

QTC_EXPORT long*
qtcStrLoadIntList(const char *str, char delim, char escape, size_t *nele,
                  long *buff, long def)
{
    return qtcStrLoadList(str, delim, escape, sizeof(long), nele, buff,
                          qtcStrListIntLoader, (void*)(intptr_t)def);
}

static void
qtcStrListFloatLoader(void *ele, const char *str, size_t len, void *data)
{
    QTC_UNUSED(len);
    double def = *(double*)data;
    str += strspn(str, " \t\b\n\f\v");
    char *end = NULL;
    double res = strtod(str, &end);
    if (end == str) {
        res = def;
    }
    *(double*)ele = res;
}

QTC_EXPORT double*
qtcStrLoadFloatList(const char *str, char delim, char escape, size_t *nele,
                    double *buff, double def)
{
    return qtcStrLoadList(str, delim, escape, sizeof(double), nele, buff,
                          qtcStrListFloatLoader, &def);
}
