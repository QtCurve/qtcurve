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

#include <qtcurve-utils/strs.h>
#include <assert.h>

static const char *str1 = "abcdef;;\\;;aa\\\\a;a\\bb;";
static const char *str_list1[] = {"abcdef", "", ";", "aa\\a", "abb", ""};

static const char *str2 = "abcdef,ls,\\";
static const char *str_list2[] = {"abcdef", "ls", ""};

typedef struct {
    const char **strs;
    int index;
} QtcStrListTest;

static void
qtcStrListFunc(const char *str, size_t len, void *_data)
{
    QtcStrListTest *data = (QtcStrListTest*)_data;
    assert(strlen(str) == len);
    assert(strcmp(str, data->strs[data->index]) == 0);
    data->index++;
}

int
main()
{
    QtcStrListTest test1 = {
        .strs = str_list1,
        .index = 0,
    };
    qtcStrListForEach(str1, ';', '\\', qtcStrListFunc, &test1);
    assert(test1.index == (sizeof(str_list1) / sizeof(char*)));
    QtcStrListTest test2 = {
        .strs = str_list2,
        .index = 0,
    };
    qtcStrListForEach(str2, ',', '\\', qtcStrListFunc, &test2);
    assert(test2.index == (sizeof(str_list2) / sizeof(char*)));
    return 0;
}
