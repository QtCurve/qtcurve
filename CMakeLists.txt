project(QtCurve)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

set(QTCURVE_VERSION_MAJOR "1")
set(QTCURVE_VERSION_MINOR "8")
set(QTCURVE_VERSION_PATCH "14")
set(QTCURVE_VERSION "${QTCURVE_VERSION_MAJOR}.${QTCURVE_VERSION_MINOR}")
set(QTCURVE_VERSION_FULL "${QTCURVE_VERSION}.${QTCURVE_VERSION_PATCH}")

function(def_compat_option new_opt old_opt)
  if("${ARGC}" GREATER 2 AND "${ARGV2}")
    set(reverse True)
  else()
    set(reverse False)
  endif()
  if(DEFINED "${old_opt}" AND NOT DEFINED "${new_opt}")
    if(reverse)
      if("${${old_opt}}")
        set("${new_opt}" Off PARENT_SCOPE)
      else()
        set("${new_opt}" On PARENT_SCOPE)
      endif()
    else()
      set("${new_opt}" "${${old_opt}}" PARENT_SCOPE)
    endif()
  endif()
endfunction()

option(ENABLE_QT4 "Building Qt4 theme." On)
def_compat_option(QTC_QT4_ENABLE_KDE QTC_QT_ONLY True)
option(QTC_QT4_ENABLE_KDE "Building Qt4 style with KDE4 support." On)
def_compat_option(QTC_QT4_OLD_NVIDIA_ARROW_FIX QTC_OLD_NVIDIA_ARROW_FIX)
option(QTC_QT4_OLD_NVIDIA_ARROW_FIX "Qt4 old nvidia arrow fix." Off)
def_compat_option(QTC_QT4_STYLE_SUPPORT QTC_STYLE_SUPPORT)
option(QTC_QT4_STYLE_SUPPORT
  "Qt4 support for creating a style for each QtCurve theme." Off)
def_compat_option(QTC_QT4_KWIN_MAX_BUTTON_HACK QTC_KWIN_MAX_BUTTON_HACK)
option(QTC_QT4_KWIN_MAX_BUTTON_HACK
  "Qt4 hack to force kwin drawing maximize buttons for minimizable windows."
  Off)
def_compat_option(QTC_QT4_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT
  QTC_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT)
option(QTC_QT4_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT
  "Qt4 support for the 'fix parentless dialogs'." Off)

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

configure_file(config.h.in config.h @ONLY)

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory(tools)
add_subdirectory(po)
add_subdirectory(qt4)
add_subdirectory(data)

if(QTC_QT4_KWIN_MAX_BUTTON_HACK)
  message(WARNING
    "You have enabled the hack to force drawing of maximise "
    "buttons for windows that can be minimised. This is a 100% "
    "hack, that may not work or compile, and may even crash kwin.")
endif()

if(QTC_QT4_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT)
  message(WARNING
    "You have enabled support for the 'fix parentless dialogs' option. "
    "This is known to BREAK some applications. Please "
    "DO NOT report errors to application authors when you have "
    "this enabled. Please reconsider DISABLING this option.")
endif()
