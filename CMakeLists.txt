project(QtCurve-Gtk2)
cmake_minimum_required(VERSION 2.4.0)

set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_GENERATOR "TBZ2")
set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "53")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(QTCURVE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
set(QTCURVE_VERSION_FULL "${QTCURVE_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${QTCURVE_VERSION_FULL}")
include(CPack)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

include (UsePkgConfig)
pkgconfig(gtk+-2.0 GTK_INCLUDE_DIR GTK_LIB_DIR GTK_LINK_FLAGS GTK_CFLAGS)

if (GTK_INCLUDE_DIR)
    exec_program(${PKGCONFIG_EXECUTABLE}
                 ARGS --variable=gtk_binary_version gtk+-2.0
                 OUTPUT_VARIABLE GTK_VERSION)
    exec_program(${PKGCONFIG_EXECUTABLE}
                 ARGS --variable=prefix gtk+-2.0
                 OUTPUT_VARIABLE GTK_PREFIX)
    set(GTK_THEME_DIR ${GTK_PREFIX}/share/themes/QtCurve)
    set(CMAKE_INSTALL_PREFIX ${GTK_PREFIX})
    configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h)
    add_subdirectory(style)
else (GTK_INCLUDE_DIR)
    MESSAGE(FATAL_ERROR "Could not locate Gtk2 include files")
endif (GTK_INCLUDE_DIR)



