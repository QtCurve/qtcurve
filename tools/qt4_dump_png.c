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

int
main(int argc, char **argv)
{
    if (argc < 4)
        return 1;
    const char *filename = argv[1];
    const char *varname = argv[2];
    const char *outputname = argv[3];
    FILE *inputfile = fopen(filename, "r");
    FILE *outputfile = fopen(outputname, "w");

    fprintf(outputfile,
            "#ifndef __QTC_IMAGE_HDR_%s__\n", varname);
    fprintf(outputfile,
            "#define __QTC_IMAGE_HDR_%s__\n", varname);

    fprintf(outputfile, "static const unsigned char _%s_data[] = {", varname);
    int size = 0;
    unsigned char buff;
    while (fread(&buff, 1, 1, inputfile)) {
        fprintf(outputfile, "%u,", (unsigned int)buff);
        size++;
    }
    fprintf(outputfile, "};\n");
    fprintf(outputfile,
            "static const QImage %s __attribute__((unused)) = "
            "QImage::fromData(_%s_data, %d);\n", varname, varname, size);
    fprintf(outputfile, "#endif\n");
    fclose(outputfile);
    return 0;
}
