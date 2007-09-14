project(QtCurve-KDE4)
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

set(OLD_CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

# Try to find kde4-config...
file(TO_CMAKE_PATH "$ENV{KDEDIRS}" _KDEDIRS)

find_program(KDE4_KDECONFIG_EXECUTABLE NAMES kde4-config
             PATHS ${CMAKE_INSTALL_PREFIX}/bin ${_KDEDIRS} /opt/kde4/bin NO_DEFAULT_PATH)

if (NOT KDE4_KDECONFIG_EXECUTABLE)
   find_program(KDE4_KDECONFIG_EXECUTABLE NAMES kde4-config )
endif (NOT KDE4_KDECONFIG_EXECUTABLE)

# Then set install prefix...
if (KDE4_KDECONFIG_EXECUTABLE)
    exec_program(kde4-config
                 ARGS --prefix
                 OUTPUT_VARIABLE CMAKE_INSTALL_PREFIX)
endif (KDE4_KDECONFIG_EXECUTABLE)


set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(Qt4 REQUIRED)
find_package(KDE4)

configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

if (KDE4_FOUND)
    include(KDE4Defaults)

    message("** PREFIX=${CMAKE_INSTALL_PREFIX}")

    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
    include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
    set(QTCURVE_STYLE_DIR ${PLUGIN_INSTALL_DIR}/plugins/styles/)
    add_subdirectory(config)
else (KDE4_FOUND)
    set(CMAKE_INSTALL_PREFIX ${OLD_CMAKE_INSTALL_PREFIX})
    add_definitions(${QT_DEFINITIONS})
    include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
    set(QTCURVE_STYLE_DIR ${QT_PLUGINS_DIR}/styles/)
    message("** WARNING: KDE4 headers not found, QtCurve's config dialog will not be built")
endif (KDE4_FOUND)

add_subdirectory(style)


