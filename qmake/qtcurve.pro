TEMPLATE = lib
CONFIG  += plugin
CONFIG  += debug_and_release build_all
DESTDIR  = $(QTDIR)/plugins/styles
CONFIG(debug, debug|release) {
  TARGET   = qtcurved
}
else {
  TARGET   = qtcurve
}
QT += svg
unix:QT += dbus
INCLUDEPATH += . .. ../common ../config
HEADERS += \
           ../style/qtcurve.h \
           ../style/windowmanager.h \

SOURCES += \
           ../style/qtcurve.cpp \
           ../style/windowmanager.cpp \

win32-msvc200*:QMAKE_CXXFLAGS += -wd4996