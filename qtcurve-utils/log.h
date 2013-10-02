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

#ifndef _QTC_UTILS_LOG_H_
#define _QTC_UTILS_LOG_H_

#include "utils.h"

QTC_BEGIN_DECLS

typedef enum {
    QTC_LOG_DEBUG,
    QTC_LOG_INFO,
    QTC_LOG_WARN,
    QTC_LOG_ERROR,
} QtcLogLevel;

QtcLogLevel _qtcCheckLogLevel();
boolean _qtcCheckLogColor();

static inline QtcLogLevel
qtcCheckLogLevel()
{
    static boolean inited = false;
    static QtcLogLevel level = QTC_LOG_ERROR;
    if (!inited) {
        level = _qtcCheckLogLevel();
        inited = true;
    }
    return level;
}

static inline boolean
qtcCheckLogColor()
{
    static boolean inited = false;
    static boolean color = false;
    if (!inited) {
        color = _qtcCheckLogColor();
        inited = true;
    }
    return color;
}

#define qtcLogLevel (qtcCheckLogLevel())
#define qtcLogColor (qtcCheckLogColor())

__attribute__((format(printf, 5, 6)))
void _qtcLog(QtcLogLevel level, const char *fname, int line, const char *func,
             const char *fmt, ...);

__attribute__((format(printf, 5, 0)))
void _qtcLogV(QtcLogLevel level, const char *fname, int line, const char *func,
              const char *fmt, va_list ap);

#define qtcLog(__level, fmt, args...) do {                              \
        QtcLogLevel level = (__level);                                  \
        if (level < 0 || level > QTC_LOG_ERROR || level < qtcLogLevel)  \
            break;                                                      \
        _qtcLog(level, __FILE__, __LINE__, __FUNCTION__, fmt, ##args);  \
    } while (0)

#define qtcDebug(fmt, args...)                  \
    qtcLog(QTC_LOG_DEBUG, fmt, ##args)
#define qtcInfo(fmt, args...)                   \
    qtcLog(QTC_LOG_INFO, fmt, ##args)
#define qtcWarn(fmt, args...)                   \
    qtcLog(QTC_LOG_WARN, fmt, ##args)
#define qtcError(fmt, args...)                  \
    qtcLog(QTC_LOG_ERROR, fmt, ##args)

QTC_END_DECLS

#endif
