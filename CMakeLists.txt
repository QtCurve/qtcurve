project(QtCurve)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

set(QTCURVE_VERSION_MAJOR "1")
set(QTCURVE_VERSION_MINOR "8")
set(QTCURVE_VERSION_PATCH "14")
set(QTCURVE_VERSION "${QTCURVE_VERSION_MAJOR}.${QTCURVE_VERSION_MINOR}")
set(QTCURVE_VERSION_FULL "${QTCURVE_VERSION}.${QTCURVE_VERSION_PATCH}")

# Compatible option.
if(DEFINED QTC_QT_ONLY AND NOT DEFINED QTC_QT4_ENABLE_KDE4)
  if(QTC_QT_ONLY)
    set(QTC_QT4_ENABLE_KDE4 Off)
  else()
    set(QTC_QT4_ENABLE_KDE4 On)
  endif()
endif()
option(QTC_QT4_ENABLE_KDE4 "Enable building Qt4 style with KDE4 support." On)

# Only try to figure out what kde prefix is if not specified at command line.
if(NOT DEFINED QTC_KDE4_PREFIX)
  find_package(KDE4)
  find_program(KDE4_KDECONFIG_EXECUTABLE NAMES kde4-config
    PATHS "${CMAKE_INSTALL_PREFIX}/bin" ${_KDEDIRS} /opt/kde4/bin
    NO_DEFAULT_PATH)
  if(NOT KDE4_KDECONFIG_EXECUTABLE)
    find_program(KDE4_KDECONFIG_EXECUTABLE NAMES kde4-config)
  endif()
  if(KDE4_KDECONFIG_EXECUTABLE)
    execute_process(COMMAND "${KDE4_KDECONFIG_EXECUTABLE}" --prefix
      OUTPUT_VARIABLE QTC_KDE4_PREFIX)
    string(STRIP "${QTC_KDE4_PREFIX}" QTC_KDE4_PREFIX)
  else()
    set(QTC_KDE4_PREFIX "${CMAKE_INSTALL_PREFIX}")
  endif()
endif()

find_package(Qt4 REQUIRED)

configure_file(config.h.in config.h @ONLY)

# search packages used by KDE
if(QTC_QT4_ENABLE_KDE4)
  find_package(KDE4 REQUIRED)
  include(KDE4Defaults)

  add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
  include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${KDE4_INCLUDES})
  set(QTCURVE_STYLE_DIR "${PLUGIN_INSTALL_DIR}/plugins/styles/")
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
  add_definitions(${QT_DEFINITIONS})
  include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}")
  set(QTCURVE_STYLE_DIR "${QT_PLUGINS_DIR}/styles/")
endif()

add_subdirectory(qt4)

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
