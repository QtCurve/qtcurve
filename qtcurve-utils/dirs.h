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

#define qtcGetConfFile(file, buff...)           \
    _qtcGetConfFile(file, ##buff, NULL)
#define _qtcGetConfFile(file, buff, ...)        \
    (qtcGetConfFile)(file, buff)

static inline boolean
qtcIsDir(const char *path)
{
    struct stat stats;
    return (stat(path, &stats) == 0 && S_ISDIR(stats.st_mode) &&
            access(path, R_OK | X_OK) == 0);
}

QTC_END_DECLS

#endif
