set(qtcurve_style_common_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/config_file.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/colorutils.c)
set_source_files_properties(${qtcurve_style_common_SRCS} PROPERTIES LANGUAGE CXX)

set(qtcurve_SRCS
  qtcurve.cpp
  qtcurve_api.cpp
  qtcurve_utils.cpp
  debug.cpp
  windowmanager.cpp
  blurhelper.cpp
  utils.cpp
  shortcuthandler.cpp
  ${qtcurve_style_common_SRCS})
set(qtcurve_MOC_HDRS
  qtcurve.h
  qtcurve_plugin.h
  windowmanager.h
  blurhelper.h
  shortcuthandler.h)

if(QTC_ENABLE_X11)
  set(qtcurve_SRCS
    shadowhelper.cpp
    xcb_utils.cpp
    ${qtcurve_SRCS})
  set(qtcurve_MOC_HDRS
    shadowhelper.h
    ${qtcurve_MOC_HDRS})
endif()

add_definitions(-DQT_PLUGIN)
include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}")
qt5_wrap_cpp(qtcurve_MOC_SRCS ${qtcurve_MOC_HDRS})
add_library(qtcurve-qt5 MODULE ${qtcurve_SRCS} ${qtcurve_MOC_SRCS})
set_target_properties(qtcurve-qt5 PROPERTIES
  LINK_FLAGS "-Wl,--no-undefined"
  OUTPUT_NAME "qtcurve"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  PREFIX "")
target_link_libraries(qtcurve-qt5 ${QTC5_LINK_LIBS} ${QTC5_XCB_LINK_LIBS})
install(TARGETS qtcurve-qt5 LIBRARY DESTINATION ${QTCURVE_STYLE_DIR})
