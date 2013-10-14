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

#include "utils.h"

#ifdef __QTC_ATOMIC_USE_SYNC_FETCH
/**
 * Also define lib function when there is builtin function for
 * atomic operation in case the function address is needed or the builtin
 * is not available when compiling other modules.
 **/
#define QTC_DEFINE_ATOMIC(name, op, type)               \
    QTC_EXPORT type                                     \
    (qtcAtomic##name)(volatile type *atomic, type val)  \
    {                                                   \
        return __qtcAtomic##name(atomic, val);          \
    }
#else
#include <pthread.h>
static pthread_mutex_t __qtc_atomic_lock = PTHREAD_MUTEX_INITIALIZER;
#define QTC_DEFINE_ATOMIC(name, op, type)               \
    QTC_EXPORT type                                     \
    (qtcAtomic##name)(volatile type *atomic, type val)  \
    {                                                   \
        type oldval;                                    \
        pthread_mutex_lock(&__qtc_atomic_lock);         \
        oldval = *atomic;                               \
        *atomic = oldval op val;                        \
        pthread_mutex_unlock(&__qtc_atomic_lock);       \
        return oldval;                                  \
    }
#endif

QTC_DEFINE_ATOMIC(Add, +, int32_t)
QTC_DEFINE_ATOMIC(And, &, uint32_t)
QTC_DEFINE_ATOMIC(Or, |, uint32_t)
QTC_DEFINE_ATOMIC(Xor, ^, uint32_t)

#undef QTC_DEFINE_ATOMIC
