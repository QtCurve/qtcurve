#!/usr/bin/env python

import png
from pylab import *

def convert_png(iname):
    im = png.Reader(iname).asRGBA()
    return (im[0], im[1],
            ', '.join(', '.join(', '.join((str(b * int(a) // 128),
                                           str(g * int(a) // 128),
                                           str(r * int(a) // 128),
                                           str(a)))
                                for r, g, b, a in
                                array(row).reshape(len(row) / 4, 4))
                                for row in im[2]))

if __name__ == '__main__':
    import sys
    oname, vname, *inames = sys.argv[1:]
    with open(oname, 'w') as ofd:
        for i, iname in enumerate(inames):
            width, height, data = convert_png(iname)
            ofd.write('static const unsigned char __%s_imgdata_%d[]'
                      ' = {%s};\n' % (vname, i, data))
            ofd.write('static const size_t __%s_imgdata_len_%d'
                      ' = sizeof(__%s_imgdata_%d);\n' % (vname, i, vname, i))
            ofd.write('static const size_t __%s_imgdata_width_%d = %d;\n' %
                      (vname, i, width))
            ofd.write('static const size_t __%s_imgdata_height_%d = %d;\n' %
                      (vname, i, height))
        ofd.write('static const unsigned char *const %s_data[] = {\n' % vname)
        for i, iname in enumerate(inames):
            ofd.write('    __%s_imgdata_%d,\n' % (vname, i))
        ofd.write('};\n')
        ofd.write('static const size_t %s_len[] = {\n' % vname)
        for i, iname in enumerate(inames):
            ofd.write('    __%s_imgdata_len_%d,\n' % (vname, i))
        ofd.write('};\n')
        ofd.write('static const size_t %s_width[] = {\n' % vname)
        for i, iname in enumerate(inames):
            ofd.write('    __%s_imgdata_width_%d,\n' % (vname, i))
        ofd.write('};\n')
        ofd.write('static const size_t %s_height[] = {\n' % vname)
        for i, iname in enumerate(inames):
            ofd.write('    __%s_imgdata_height_%d,\n' % (vname, i))
        ofd.write('};\n')
