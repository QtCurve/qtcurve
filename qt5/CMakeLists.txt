include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}")

set(QTC5_LINK_LIBS)
set(QTC_QT_MODULES Qt5Core Qt5Gui Qt5Widgets Qt5Svg)
if(QTC_ENABLE_X11)
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
if(QTC_ENABLE_X11)
  if(NOT PKG_CONFIG_FOUND)
    message(FATAL_ERROR "Cannot find pkg-conig")
  endif()
  pkg_check_modules(XCB xcb xcb-image)
  set(QTC5_XCB_LINK_LIBS ${QTC5_XCB_LINK_LIBS} ${XCB_LIBRARIES})
  include_directories(${XCB_INCLUDE_DIRS})
  add_definitions(${XCB_CFLAGS})
endif()

add_definitions(${QT_DEFINITIONS})

get_target_property(REAL_QMAKE_EXECUTABLE ${Qt5Core_QMAKE_EXECUTABLE}
  IMPORTED_LOCATION)
execute_process(COMMAND "${REAL_QMAKE_EXECUTABLE}" -query QT_INSTALL_PLUGINS
  OUTPUT_VARIABLE _Qt5_PLUGIN_INSTALL_DIR
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
set(QTCURVE_STYLE_DIR "${_Qt5_PLUGIN_INSTALL_DIR}/styles/")

add_subdirectory(style)
