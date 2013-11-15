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

#include <stdio.h>
#include <QImage>
#include <QCoreApplication>

int
main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
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
            unsigned int alpha = qAlpha(pixel);
            unsigned int red = qRed(pixel) * alpha / 256;
            unsigned int blue = qBlue(pixel) * alpha / 256;
            unsigned int green = qGreen(pixel) * alpha / 256;
            uint32_t pixel_data =
                blue | (green << 8) | (red << 16) | (alpha << 24);
            uint8_t *p = (uint8_t*)&pixel_data;
            fprintf(outputfile, "%u,%u,%u,%u,",
                    (int)p[0], (int)p[1], (int)p[2], (int)p[3]);
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
            "};\n", varname, size * 4, width, height, 32, varname);
    fprintf(outputfile, "#endif\n");
    fclose(outputfile);
    return 0;
}
