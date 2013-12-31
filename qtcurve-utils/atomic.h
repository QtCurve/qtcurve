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

#ifndef _QTC_UTILS_ATOMIC_H_
#define _QTC_UTILS_ATOMIC_H_

/**
 * \file atomic.h
 * \author Yichao Yu <yyc1992@gmail.com>
 * \brief Atomic operations.
 */

#include "utils.h"

#ifdef __QTC_ATOMIC_USE_SYNC_FETCH
#  undef __QTC_ATOMIC_USE_SYNC_FETCH
#endif

#if defined __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
#  define __QTC_ATOMIC_USE_SYNC_FETCH
#elif defined __has_builtin
#if (__has_builtin(__sync_fetch_and_add) &&     \
     __has_builtin(__sync_fetch_and_and) &&     \
     __has_builtin(__sync_fetch_and_xor) &&     \
     __has_builtin(__sync_fetch_and_or))
#  define __QTC_ATOMIC_USE_SYNC_FETCH
#endif
#endif

#ifdef __QTC_ATOMIC_USE_SYNC_FETCH
#define QTC_DECLARE_ATOMIC(name1, name2, type)                  \
    type (qtcAtomic##name1)(volatile type *atomic, type val);   \
    QTC_ALWAYS_INLINE static inline type                        \
    __qtcAtomic##name1(volatile type *atomic, type val)         \
    {                                                           \
        return __sync_fetch_and_##name2(atomic, val);           \
    }
#else
#define QTC_DECLARE_ATOMIC(name1, name2, type)                  \
    type (qtcAtomic##name1)(volatile type *atomic, type val);   \
    QTC_ALWAYS_INLINE static inline type                        \
    __qtcAtomic##name1(volatile type *atomic, type val)         \
    {                                                           \
        return (qtcAtomic##name1)(atomic, val);                 \
    }
#endif

QTC_BEGIN_DECLS

QTC_DECLARE_ATOMIC(Add, add, int32_t);
QTC_DECLARE_ATOMIC(And, and, uint32_t);
QTC_DECLARE_ATOMIC(Or, or, uint32_t);
QTC_DECLARE_ATOMIC(Xor, xor, uint32_t);

/**
 * Add \param val to the variable pointed by \param atomic and return the old
 * value atomically.
 */
#define qtcAtomicAdd(atomic, val)               \
    __qtcAtomicAdd(atomic, val)
/**
 * "And" \param val with the variable pointed by \param atomic and return
 * the old value atomically.
 */
#define qtcAtomicAnd(atomic, val)               \
    __qtcAtomicAnd(atomic, val)
/**
 * "Or" \param val with the variable pointed by \param atomic and return
 * the old value atomically.
 */
#define qtcAtomicOr(atomic, val)                \
    __qtcAtomicOr(atomic, val)
/**
 * "Xor" \param val with the variable pointed by \param atomic and return
 * the old value atomically.
 */
#define qtcAtomicXor(atomic, val)               \
    __qtcAtomicXor(atomic, val)

QTC_END_DECLS

#endif
