TEMPLATE = lib
CONFIG  += plugin
CONFIG  += debug_and_release build_all
DESTDIR  = $(QTDIR)/plugins/styles
CONFIG(debug, debug|release) {
  TARGET = qtcurved
}
else {
  TARGET = qtcurve
}
QT *= svg
unix:QT *= dbus
INCLUDEPATH *= . .. ../common ../config
HEADERS += \
           ../common/colorutils.h \
           ../common/common.h \
           ../common/config_file.h \
           ../style/blurhelper.h \
           ../style/dialogpixmaps.h \
           ../style/fixx11h.h \
           ../style/pixmaps.h \
           ../style/qtcurve.h \
           ../style/shortcuthandler.h \
           ../style/utils.h \
           ../style/windowmanager.h \

SOURCES += \
           ../common/colorutils.c \
           ../common/common.c \
           ../common/config_file.c \
           ../style/blurhelper.cpp \
           ../style/qtcurve.cpp \
           ../style/shortcuthandler.cpp \
           ../style/utils.cpp \
           ../style/windowmanager.cpp \

win32-msvc*:DEFINES *= _CRT_SECURE_NO_WARNINGS

# Force C++ language
*g++*:QMAKE_CFLAGS *= -x c++
*msvc*:QMAKE_CFLAGS *= -TP