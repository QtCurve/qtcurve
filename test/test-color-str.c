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

#include <qtcurve-utils/color.h>
#include <assert.h>

static void
test_short()
{
    char origin[10];
    char res[10];
    QtcColor color;
    for (int r = 0;r < 16;r++) {
        for (int g = 0;g < 16;g++) {
            for (int b = 0;b < 16;b++) {
                sprintf(origin, "#%01X%01X%01X", r, g, b);
                qtcColorFromStr(&color, origin);
                assert(color.red == r / 15.0);
                assert(color.green == g / 15.0);
                assert(color.blue == b / 15.0);
                qtcColorToStr(&color, res);
                sprintf(origin, "#%02X%02X%02X", r * 255 / 15, g * 255 / 15,
                        b * 255 / 15);
                assert(strcmp(origin, res) == 0);
            }
        }
    }
}

static void
test_long()
{
    char origin[10];
    char res[10];
    QtcColor color;
    for (int r = 0;r < 256;r++) {
        for (int g = 0;g < 256;g++) {
            for (int b = 0;b < 256;b++) {
                sprintf(origin, "#%02X%02X%02X", r, g, b);
                qtcColorFromStr(&color, origin);
                assert(color.red == r / 255.0);
                assert(color.green == g / 255.0);
                assert(color.blue == b / 255.0);
                qtcColorToStr(&color, res);
                assert(strcmp(origin, res) == 0);
            }
        }
    }
}

int
main()
{
    test_short();
    test_long();
    return 0;
}
