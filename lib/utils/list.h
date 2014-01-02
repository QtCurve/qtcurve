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

#ifndef _QTC_UTILS_LIST_H_
#define _QTC_UTILS_LIST_H_

#include "utils.h"

typedef struct _QtcList QtcList;

struct _QtcList {
    QtcList *prev;
    QtcList *next;
};

QTC_ALWAYS_INLINE static inline QtcList*
qtcListInit(QtcList *list)
{
    return (list->prev = list->next = list);
}

QTC_ALWAYS_INLINE static inline void
qtcListInsert(QtcList *list, QtcList *ele)
{
    ele->prev = list;
    ele->next = list->next;
    list->next = ele;
    ele->next->prev = ele;
}

QTC_ALWAYS_INLINE static inline void
qtcListRemove(QtcList *ele)
{
    ele->prev->next = ele->next;
    ele->next->prev = ele->prev;
    ele->next = NULL;
    ele->prev = NULL;
}

QTC_ALWAYS_INLINE static inline bool
qtcListEmpty(const QtcList *list)
{
    return list->next == list;
}

QTC_ALWAYS_INLINE static inline int
qtcListLength(const QtcList *list)
{
    int count = 0;
    for (QtcList *e = list->next;e != list;e = e->next) {
        count++;
    }
    return count;
}

QTC_ALWAYS_INLINE static inline void
qtcListInsertList(QtcList *list, QtcList *other)
{
    if (qtcListEmpty(other)) {
        return;
    }
    other->next->prev = list;
    other->prev->next = list->next;
    list->next->prev = other->prev;
    list->next = other->next;
}

#endif
