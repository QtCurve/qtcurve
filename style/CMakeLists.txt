set(qtcurve_style_common_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/config_file.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/colorutils.c)
set_source_files_properties(${qtcurve_style_common_SRCS} PROPERTIES LANGUAGE CXX)

set(qtcurve_SRCS
  qtcurve.cpp
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

if(Q_WS_X11)
  set(qtcurve_SRCS
    macmenu.cpp
    shadowhelper.cpp
    ${qtcurve_SRCS})
  set(qtcurve_MOC_HDRS
    macmenu.h
    macmenu-dbus.h
    shadowhelper.h
    ${qtcurve_MOC_HDRS})
endif()

add_definitions(-DQT_PLUGIN)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}/common")
# kde4_add_plugin(qtcurve ${qtcurve_SRCS})
# qt5_automoc(${qtcurve_SRCS})
qt5_wrap_cpp(qtcurve_MOC_SRCS ${qtcurve_MOC_HDRS})
add_library(qtcurve MODULE ${qtcurve_SRCS} ${qtcurve_MOC_SRCS})
set_target_properties(qtcurve PROPERTIES PREFIX "")

# if(NOT QTC_QT_ONLY)
#   install(FILES qtcurve.themerc DESTINATION ${DATA_INSTALL_DIR}/kstyle/themes)
#   set(QTC_KDE_LIBS ${KDE4_KDEUI_LIBS})
#   if(NOT QTC_DISABLE_KDEFILEDIALOG_CALLS)
#     set(QTC_KFILEDIALOG_LIBS ${KDE4_KIO_LIBS} ${KDE4_KDECORE_LIBS})
#   endif()
# endif()

target_link_libraries(qtcurve ${QTC5_LINK_LIBS})
if(Q_WS_X11)
  target_link_libraries(qtcurve ${X11_X11_LIB})
endif()
install(TARGETS qtcurve LIBRARY DESTINATION ${QTCURVE_STYLE_DIR})
