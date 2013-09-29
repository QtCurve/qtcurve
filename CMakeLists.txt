project(QtCurve-Qt5)
cmake_minimum_required(VERSION 2.8)

set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "8")
set(CPACK_PACKAGE_VERSION_PATCH "14")
# set(CPACK_PACKAGE_CONTACT "Craig Drummond <craig.p.drummond@gmail.com>")
set(QTCURVE_VERSION
  "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
set(QTCURVE_VERSION_FULL "${QTCURVE_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")

option(QTC_X11 "Enable X11" On)

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}")

set(OLD_CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}"
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

set(QTC5_LINK_LIBS)
set(QTC_QT_MODULES Qt5Core Qt5Gui Qt5Widgets Qt5Svg)
if(QTC_X11)
  set(QTC_QT_MODULES ${QTC_QT_MODULES} Qt5DBus Qt5X11Extras)
endif()
foreach(QtModule ${QTC_QT_MODULES})
  find_package(${QtModule} REQUIRED)
  set(QTC5_LINK_LIBS ${QTC5_LINK_LIBS} ${${QtModule}_LIBRARIES})
  include_directories(${${QtModule}_INCLUDE_DIRS})
  add_definitions(${${QtModule}_DEFINITIONS})
endforeach()

add_definitions("-DQT_DISABLE_DEPRECATED_BEFORE=0x050100")

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

add_definitions(${QT_DEFINITIONS} "-std=c++0x")

set(CMAKE_INSTALL_PREFIX ${OLD_CMAKE_INSTALL_PREFIX})
get_target_property(REAL_QMAKE_EXECUTABLE ${Qt5Core_QMAKE_EXECUTABLE}
  IMPORTED_LOCATION)
execute_process(COMMAND "${REAL_QMAKE_EXECUTABLE}" -query QT_INSTALL_PLUGINS
  OUTPUT_VARIABLE _Qt5_PLUGIN_INSTALL_DIR
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
set(QTCURVE_STYLE_DIR "${_Qt5_PLUGIN_INSTALL_DIR}/styles/")

add_subdirectory(qt5)
