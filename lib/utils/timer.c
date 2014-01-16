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

#include "timer.h"
#include "list.h"
#include "number.h"
#include <time.h>
#include <pthread.h>

#ifdef CLOCK_THREAD_CPUTIME_ID
#  define CLOCK_ID CLOCK_THREAD_CPUTIME_ID
#else
#  define CLOCK_ID CLOCK_MONOTONIC
#endif

QTC_EXPORT uint64_t
qtcGetTime()
{
    struct timespec time_spec;
    clock_gettime(CLOCK_ID, &time_spec);
    return ((uint64_t)time_spec.tv_sec) * 1000000000 + time_spec.tv_nsec;
}

QTC_EXPORT uint64_t
qtcGetElapse(uint64_t prev)
{
    return qtcGetTime() - prev;
}

typedef struct {
    size_t alloc;
    size_t num;
    uint64_t tics[0];
} QtcTics;

static QtcTics*
qtcTicsResize(QtcTics *tics, size_t new_size)
{
    new_size = qtcAlignTo(new_size, 16);
    if (!tics || new_size > tics->alloc || new_size * 2 < tics->alloc) {
        tics = (QtcTics*)realloc(tics, (sizeof(QtcTics) +
                                        sizeof(uint64_t) * new_size));
        tics->alloc = new_size;
    }
    return tics;
}

static pthread_key_t qtc_tics_key;
static pthread_once_t qtc_tics_key_once = PTHREAD_ONCE_INIT;

static void
qtcMakeTicsKey()
{
    pthread_key_create(&qtc_tics_key, free);
}

static QtcTics*
qtcGetTics()
{
    pthread_once(&qtc_tics_key_once, qtcMakeTicsKey);
    return (QtcTics*)pthread_getspecific(qtc_tics_key);
}

static void
qtcSetTics(QtcTics *tics)
{
    pthread_once(&qtc_tics_key_once, qtcMakeTicsKey);
    pthread_setspecific(qtc_tics_key, tics);
}

QTC_EXPORT void
qtcTic()
{
    QtcTics *old_tics = qtcGetTics();
    size_t num = old_tics ? old_tics->num : 0;
    QtcTics *tics = qtcTicsResize(old_tics, num + 1);
    tics->num = num + 1;
    if (tics != old_tics) {
        qtcSetTics(tics);
    }
    tics->tics[num] = qtcGetTime();
}

QTC_EXPORT uint64_t
qtcToc()
{
    uint64_t cur_time = qtcGetTime();
    QtcTics *old_tics = qtcGetTics();
    if (!old_tics || !old_tics->num) {
        return 0;
    }
    old_tics->num--;
    uint64_t old_time = old_tics->tics[old_tics->num];
    QtcTics *tics = qtcTicsResize(old_tics, old_tics->num);
    if (tics != old_tics) {
        qtcSetTics(tics);
    }
    return cur_time - old_time;
}
