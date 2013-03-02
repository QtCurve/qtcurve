project(QtCurve-Qt5)
cmake_minimum_required(VERSION 2.8)

# set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
# set(CPACK_GENERATOR "DEB;RPM;TGZ")
# set(DEBIAN_PACKAGE_DESCRIPTION "QtCurve for Qt4/KDE4")
# set(DEBIAN_PACKAGE_SECTION "kde4")
# set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "8")
set(CPACK_PACKAGE_VERSION_PATCH "14")
# set(CPACK_PACKAGE_CONTACT "Craig Drummond <craig.p.drummond@gmail.com>")
set(QTCURVE_VERSION
  "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
set(QTCURVE_VERSION_FULL "${QTCURVE_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
# set(CPACK_SOURCE_PACKAGE_FILE_NAME
#   "${CMAKE_PROJECT_NAME}-${QTCURVE_VERSION_FULL}")
# set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${QTCURVE_VERSION_FULL}")
# include(CPack)

option(QTC_X11 "Enable X11" On)

set(OLD_CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

# # Try to find kde4-config...
# file(TO_CMAKE_PATH "$ENV{KDEDIRS}" _KDEDIRS)

# find_program(KDE4_KDECONFIG_EXECUTABLE NAMES kde4-config
#   PATHS ${CMAKE_INSTALL_PREFIX}/bin ${_KDEDIRS} /opt/kde4/bin NO_DEFAULT_PATH)

# if(NOT KDE4_KDECONFIG_EXECUTABLE)
#   find_program(KDE4_KDECONFIG_EXECUTABLE NAMES kde4-config )
# endif()

# # Then set install prefix...
# if(KDE4_KDECONFIG_EXECUTABLE)
#   if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#     exec_program(${KDE4_KDECONFIG_EXECUTABLE}
#       ARGS --prefix
#       OUTPUT_VARIABLE CMAKE_INSTALL_PREFIX)
#   endif()
#   exec_program(${KDE4_KDECONFIG_EXECUTABLE}
#     ARGS --prefix
#     OUTPUT_VARIABLE KDE4PREFIX)
# endif()

# if(NOT KDE4PREFIX)
#   set(KDE4PREFIX /usr)
# endif()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}"
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# # search packages used by KDE
# if(NOT QTC_QT_ONLY)
#   find_package(KDE4 REQUIRED)
# endif()
set(QTC5_LINK_LIBS)
set(QTC_QT_MODULES Qt5Core Qt5Gui Qt5Widgets Qt5Svg)
if(QTC_X11)
  set(QTC_QT_MODULES ${QTC_QT_MODULES} Qt5DBus)
endif()
foreach(QtModule ${QTC_QT_MODULES})
  find_package(${QtModule} REQUIRED)
  set(QTC5_LINK_LIBS ${QTC5_LINK_LIBS} ${${QtModule}_LIBRARIES})
  include_directories(${${QtModule}_INCLUDE_DIRS})
  add_definitions(${${QtModule}_DEFINITIONS})
endforeach()
if(QTC_X11)
  include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
endif()

set(QTC5_XCB_LINK_LIBS)
find_package(PkgConfig)
if(QTC_X11)
  if(NOT PKG_CONFIG_FOUND)
    message(FATAL_ERROR "Cannot find pkg-conig")
  endif()
  pkg_check_modules(XCB xcb xcb-image)
  set(QTC5_XCB_LINK_LIBS ${QTC5_XCB_LINK_LIBS} ${XCB_LIBRARIES})
  include_directories(${XCB_INCLUDE_DIRS})
  add_definitions(${XCB_CFLAGS})
endif()

configure_file(config.h.in config.h @ONLY)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}")

add_definitions(${QT_DEFINITIONS} "-std=c++0x")

# if(NOT QTC_QT_ONLY)
#   find_package(KDE4 REQUIRED)
#   include(KDE4Defaults)

#   add_definitions(${KDE4_DEFINITIONS})
#   include_directories(${KDE4_INCLUDES})
#   set(QTCURVE_STYLE_DIR "${PLUGIN_INSTALL_DIR}/plugins/styles/")
#   add_subdirectory(config)
#   add_subdirectory(themes)
#   add_subdirectory(colors)
#   add_subdirectory(po)

#   # kwinglobals.h is new for KDE4
#   find_file(KWINGLOBALS_H kwinglobals.h ${KDE4_INCLUDES})

#   if(KWINGLOBALS_H)
#     add_subdirectory(kwin)
#     add_subdirectory(kwinconfig)
#   else()
#     message("** KWin headers not found - KWin decoration will not be built")
#   endif()
# else()
set(CMAKE_INSTALL_PREFIX ${OLD_CMAKE_INSTALL_PREFIX})
get_target_property(REAL_QMAKE_EXECUTABLE ${Qt5Core_QMAKE_EXECUTABLE}
  IMPORTED_LOCATION)
execute_process(COMMAND "${REAL_QMAKE_EXECUTABLE}" -query QT_INSTALL_PLUGINS
  OUTPUT_VARIABLE _Qt5_PLUGIN_INSTALL_DIR
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
set(QTCURVE_STYLE_DIR "${_Qt5_PLUGIN_INSTALL_DIR}/styles/")
# endif()

add_subdirectory(style)

# message("** PREFIX=${CMAKE_INSTALL_PREFIX}\n")

# if(QTC_KWIN_MAX_BUTTON_HACK)
#   message("**************************************************************\n"
#     "Warning: You have enabled the hack to force drawing of maximise\n"
#     "         buttons for windows that can be minimised. This is a 100%\n"
#     "         hack, that may not work or compile, and may even crash\n"
#     "         kwin.\n"
#     "**************************************************************\n")
# endif()

# if(QTC_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT)
#   message("**************************************************************\n"
#     "Warning: You have enabled support for the 'fix parentless dialogs'\n"
#     "         option. This is known to BREAK some applications. Please\n"
#     "         DO NOT report errors to application authors when you have\n"
#     "         this enabled. Please reconsider DISABLING this option.\n"
#     "**************************************************************\n")
# endif()
