/*****************************************************************************
 *   Copyright 2014 - 2014 Yichao Yu <yyc1992@gmail.com>                     *
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

#ifndef _QTC_UTILS_PROCESS_H_
#define _QTC_UTILS_PROCESS_H_

#include "utils.h"

QTC_BEGIN_DECLS

bool qtcForkBackground(QtcCallback cb, void *data, QtcCallback fail_cb);
#define qtcForkBackground(cb, data, fail_cb...)                 \
    qtcForkBackground(cb, data, QTC_DEFAULT(fail_cb, NULL))
bool qtcSpawn(const char *file, char *const *argv,
              QtcCallback cb, void *cb_data, QtcCallback fail_cb);
#define qtcSpawn(file, argv, cb, cb_data, fail_cb...)                   \
    qtcSpawn(file, argv, cb, cb_data, QTC_DEFAULT(fail_cb, NULL))
typedef enum {
    QTC_POPEN_NONE = 0,
    QTC_POPEN_READ = 1 << 0,
    QTC_POPEN_WRITE = 1 << 1,
    QTC_POPEN_RDWR = QTC_POPEN_READ | QTC_POPEN_WRITE
} QtcPopenFDMode;
typedef struct {
    int orig;
    int replace;
    QtcPopenFDMode mode;
} QtcPopenFD;
bool qtcPopen(const char *file, char *const *argv,
              unsigned fd_num, QtcPopenFD *fds);

QTC_END_DECLS

#endif
