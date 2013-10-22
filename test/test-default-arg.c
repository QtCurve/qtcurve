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

#include <qtcurve-utils/utils.h>
#include <assert.h>

static int default1_times = 0;

static int
default1()
{
    default1_times++;
    return -20;
}

static int default2_times = 0;

static int
default2()
{
    default2_times++;
    return 15;
}

static int value_times = 0;

static int
value()
{
    value_times++;
    return 1;
}

static int
f(int i, int j)
{
    return i + j;
}

#define f(i, j)                                                 \
    (f)(QTC_DEFAULT(i, default1()), QTC_DEFAULT(j, default2()))

int
main()
{
    assert(f(,) == -5);
    assert(f(value(),) == 16);
    assert(f(, value()) == -19);
    assert(f(value(), value()) == 2);
    assert(default1_times == 2);
    assert(default2_times == 2);
    assert(value_times == 4);
    return 0;
}
