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

#ifndef _QTC_UTILS_DIRS_H_
#define _QTC_UTILS_DIRS_H_

#include "utils.h"
#include <unistd.h>
#include <sys/stat.h>

QTC_BEGIN_DECLS

const char *qtcGetHome();
const char *qtcGetXDGDataHome();
const char *qtcGetXDGConfigHome();
const char *qtcConfDir();
void qtcMakePath(const char *path, int mode);
char *qtcGetConfFile(const char *file, char *buff);
QTC_ALWAYS_INLINE static inline char*
_qtcGetConfFile(const char *file)
{
    return qtcGetConfFile(file, NULL);
}
#define qtcGetConfFile(file, buff...)                   \
    QTC_SWITCH_(buff, qtcGetConfFile)(file, ##buff)

static inline bool
qtcIsDir(const char *path)
{
    struct stat stats;
    return (stat(path, &stats) == 0 && S_ISDIR(stats.st_mode) &&
            access(path, R_OK | X_OK) == 0);
}

static inline bool
qtcIsRegFile(const char *path)
{
    struct stat stats;
    return (stat(path, &stats) == 0 && S_ISREG(stats.st_mode) &&
            access(path, R_OK) == 0);
}

static inline bool
qtcIsSymLink(const char *path)
{
    struct stat stats;
    return lstat(path, &stats) == 0 && S_ISLNK(stats.st_mode);
}

QTC_END_DECLS

#endif
