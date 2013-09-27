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

#include "stdio.h"

#include <QImage>
#include <QApplication>

int
main(int argc, char **argv)
{
    QApplication app(argc, argv);
    if (argc < 4)
        return 1;
    const char *filename = argv[1];
    const char *varname = argv[2];
    const char *outputname = argv[3];

    QImage image(QString::fromLocal8Bit(filename));
    if (image.isNull())
        return 1;
    int height = image.height();
    int width = image.width();
    int size = height * width;

    FILE *outputfile = fopen(outputname, "w");
    fprintf(outputfile,
            "#ifndef __QTC_IMAGE_HDR_%s__\n", varname);
    fprintf(outputfile,
            "#define __QTC_IMAGE_HDR_%s__\n", varname);

    fprintf(outputfile, "static const unsigned char _%s_data[] = {", varname);
    for (int j = 0;j < height;j++) {
        for (int i = 0;i < width;i++) {
            QRgb pixel = image.pixel(i, j);
            int alpha = qAlpha(pixel);
            int red = qRed(pixel) * alpha / 256;
            int blue = qBlue(pixel) * alpha / 256;
            int green = qGreen(pixel) * alpha / 256;
            fprintf(outputfile, "%d,%d,%d,%d,", blue, green, red, alpha);
        }
    }
    fprintf(outputfile, "};\n");
    fprintf(outputfile,
            "static const QtcPixmap %s __attribute__((unused)) = {\n"
            "    %d,\n"
            "    %d,\n"
            "    %d,\n"
            "    %d,\n"
            "    _%s_data\n"
            "};\n", varname, size * 4, width, height, image.depth(), varname);
    fprintf(outputfile, "#endif\n");
    fclose(outputfile);
}
