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
    if (qtcListEmpty(other))
        return;
    other->next->prev = list;
    other->prev->next = list->next;
    list->next->prev = other->prev;
    list->next = other->next;
}

#endif
