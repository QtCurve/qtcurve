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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cairo.h>

int
main(int argc, char **argv)
{
    if (argc < 4)
        return 1;
    const char *filename = argv[1];
    const char *varname = argv[2];
    const char *outputname = argv[3];

    cairo_surface_t *image = cairo_image_surface_create_from_png(filename);
    if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS)
        return 1;
    int height = cairo_image_surface_get_height(image);
    int width = cairo_image_surface_get_width(image);

    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
    unsigned char *buff = malloc(stride * height);
    cairo_surface_t *new_img =
        cairo_image_surface_create_for_data(buff, CAIRO_FORMAT_ARGB32,
                                            width, height, stride);
    cairo_t *cr = cairo_create(new_img);
    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS)
        return 1;
    cairo_set_source_surface(cr, image, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
    cairo_surface_flush(new_img);
    cairo_surface_destroy(image);

    FILE *outputfile = fopen(outputname, "w");
    fprintf(outputfile,
            "#ifndef __QTC_IMAGE_HDR_%s__\n", varname);
    fprintf(outputfile,
            "#define __QTC_IMAGE_HDR_%s__\n", varname);

    fprintf(outputfile, "static const unsigned char _%s_data[] = {", varname);
    for (int j = 0;j < height;j++) {
        for (int i = 0;i < width;i++) {
            uint32_t pixel;
            memcpy(&pixel, buff + (stride * j) + i * sizeof(uint32_t),
                   sizeof(uint32_t));
            unsigned int alpha = (pixel & 0xff000000) >> 24;
            unsigned int red = (pixel & 0xff0000) >> 16;
            unsigned int blue = (pixel & 0xff00) >> 8;
            unsigned int green = (pixel & 0xff);
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
            "};\n", varname, width * height * 4, width, height, 32, varname);
    fprintf(outputfile, "#endif\n");
    fclose(outputfile);
    return 0;
}
