project(QtCurve)
cmake_minimum_required(VERSION 2.8)

set(QTCURVE_VERSION_MAJOR "1")
set(QTCURVE_VERSION_MINOR "8")
set(QTCURVE_VERSION_PATCH "14")
set(QTCURVE_VERSION "${QTCURVE_VERSION_MAJOR}.${QTCURVE_VERSION_MINOR}")
set(QTCURVE_VERSION_FULL "${QTCURVE_VERSION}.${QTCURVE_VERSION_PATCH}")

option(QTC_QT_ONLY "Only build Qt4 style without KDE4 support." Off)

if(QTC_QT_ONLY)
  find_package(KDE4)
else()
  find_package(KDE4 REQUIRED)
endif()

find_program(KDE4_KDECONFIG_EXECUTABLE NAMES kde4-config
  PATHS ${CMAKE_INSTALL_PREFIX}/bin ${_KDEDIRS} /opt/kde4/bin NO_DEFAULT_PATH)

if(NOT KDE4_KDECONFIG_EXECUTABLE)
  find_program(KDE4_KDECONFIG_EXECUTABLE NAMES kde4-config )
endif()

# Then set install prefix...
if(KDE4_KDECONFIG_EXECUTABLE)
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    exec_program(${KDE4_KDECONFIG_EXECUTABLE}
      ARGS --prefix
      OUTPUT_VARIABLE CMAKE_INSTALL_PREFIX)
  endif()
  exec_program(${KDE4_KDECONFIG_EXECUTABLE}
    ARGS --prefix
    OUTPUT_VARIABLE KDE4PREFIX)
endif()

if(NOT KDE4PREFIX)
  set(KDE4PREFIX ${KDE3PREFIX})
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# search packages used by KDE
if(NOT QTC_QT_ONLY)
  find_package(KDE4 REQUIRED)
endif()
find_package(Qt4 REQUIRED)

configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h @ONLY)

if(NOT QTC_QT_ONLY)
  find_package(KDE4 REQUIRED)
  include(KDE4Defaults)

  add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
  include_directories(${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
  set(QTCURVE_STYLE_DIR ${PLUGIN_INSTALL_DIR}/plugins/styles/)
  add_subdirectory(config-qt4)
  add_subdirectory(themes)
  add_subdirectory(colors)
  add_subdirectory(po)

  if("x${ENABLE_KWIN}" STREQUAL "x")
    find_file(KWINGLOBALS_H kwinglobals.h ${KDE4_INCLUDES})
    if(KWINGLOBALS_H)
      set(ENABLE_KWIN On)
    else()
      set(ENABLE_KWIN Off)
    endif()
  endif()

  if(ENABLE_KWIN)
    add_subdirectory(kwin)
    add_subdirectory(kwinconfig)
  endif()
else()
  set(CMAKE_INSTALL_PREFIX ${OLD_CMAKE_INSTALL_PREFIX})
  add_definitions(${QT_DEFINITIONS})
  include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
  set(QTCURVE_STYLE_DIR ${QT_PLUGINS_DIR}/styles/)
endif()

add_subdirectory(qt4)

message("** PREFIX=${CMAKE_INSTALL_PREFIX}\n")

if(QTC_KWIN_MAX_BUTTON_HACK)
  message(
    "**************************************************************\n"
    "Warning: You have enabled the hack to force drawing of maximise\n"
    "         buttons for windows that can be minimised. This is a 100%\n"
    "         hack, that may not work or compile, and may even crash\n"
    "         kwin.\n"
    "**************************************************************\n")
endif()

if(QTC_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT)
  message(
    "**************************************************************\n"
    "Warning: You have enabled support for the 'fix parentless dialogs'\n"
    "         option. This is known to BREAK some applications. Please\n"
    "         DO NOT report errors to application authors when you have\n"
    "         this enabled. Please reconsider DISABLING this option.\n"
    "**************************************************************\n")
endif()
