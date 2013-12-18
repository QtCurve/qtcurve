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

#ifndef _QTC_UTILS_MACROS_H_
#define _QTC_UTILS_MACROS_H_

#define __QTC_USE_3(_1, _2, _3, ...) _3
#define __QTC_HAS_COMMA1(ret_true, ret_false, args...)  \
    __QTC_USE_3(args, ret_true, ret_false)
#define __QTC_CONVERT_PAREN(...) ,
#define __QTC_IS_PAREN_(ret_true, ret_false, arg)                       \
    __QTC_HAS_COMMA1(ret_true, ret_false, __QTC_CONVERT_PAREN arg)
// Extra layer just to make sure more evaluation (if any) is done than the
// seperator path.
#define __QTC_IS_PAREN(ret_true, ret_false, arg)        \
    __QTC_IS_PAREN_(ret_true, ret_false, arg)
#define __QTC_IS_SEP(ret_true, ret_false, arg)                          \
    __QTC_HAS_COMMA1(ret_false, ret_true, __QTC_CONVERT_PAREN arg ())
#define __QTC_IS_EMPTY_PAREN_TRUE(ret_true, ret_false, arg) ret_false
#define __QTC_IS_EMPTY_PAREN_FALSE(ret_true, ret_false, arg)    \
    __QTC_IS_SEP(ret_false, ret_true, arg)
// Test if arg is empty, evaluate to ret_true if is empty,
// evaluate to ret_false otherwise. NOTE, this may not work if arg is a macro
// that can be evaluated to a comma separate list without parentheses
#define QTC_SWITCH(arg, ret_true, ret_false)                            \
    __QTC_IS_PAREN(__QTC_IS_EMPTY_PAREN_TRUE, __QTC_IS_EMPTY_PAREN_FALSE, arg) \
    (ret_false, ret_true, arg)
#define QTC_DEFAULT(v, def) QTC_SWITCH(v, v, def)
#define QTC_SWITCH_(v, f) QTC_SWITCH(v, f, _##f)

#endif
