set(qtcurve_style_common_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/config_file.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/colorutils.c )
set_source_files_properties(${qtcurve_style_common_SRCS}
  PROPERTIES LANGUAGE CXX)

set(qtcurve_SRCS
  qtcurve.cpp
  windowmanager.cpp
  blurhelper.cpp
  utils.cpp
  shortcuthandler.cpp
  ${qtcurve_style_common_SRCS})
set(qtcurve_MOC_HDRS
  qtcurve.h
  windowmanager.h
  blurhelper.h
  shortcuthandler.h)

if(Q_WS_X11)
  set(qtcurve_SRCS
    ${qtcurve_SRCS}
    macmenu.cpp
    shadowhelper.cpp)
  set(qtcurve_MOC_HDRS
    ${qtcurve_MOC_HDRS}
    macmenu.h
    macmenu-dbus.h
    shadowhelper.h)
endif()

add_definitions(-DQT_PLUGIN)
include_directories("${CMAKE_CURRENT_BINARY_DIR}"
  "${QT_INCLUDE_DIR}"
  "${QT_QTGUI_INCLUDE_DIR}"
  "${QT_QTCORE_INCLUDE_DIR}"
  "${QT_QTDBUS_INCLUDE_DIR}"
  "${QT_QTSVG_INCLUDE_DIR}")
# kde4_add_plugin(qtcurve ${qtcurve_SRCS})
# qt4_automoc(${qtcurve_SRCS})
qt4_wrap_cpp(qtcurve_MOC_SRCS ${qtcurve_MOC_HDRS})
add_library(qtcurve MODULE ${qtcurve_SRCS} ${qtcurve_MOC_SRCS})
add_dependencies(qtcurve qtc_shadow0_hdr qtc_shadow1_hdr qtc_shadow2_hdr
  qtc_shadow3_hdr qtc_shadow4_hdr qtc_shadow5_hdr qtc_shadow6_hdr
  qtc_shadow7_hdr qtc_check_on_hdr qtc_check_x_on_hdr)
set_target_properties(qtcurve PROPERTIES PREFIX "")

if(QTC_QT4_ENABLE_KDE4)
  install(FILES qtcurve.themerc DESTINATION ${DATA_INSTALL_DIR}/kstyle/themes)
  set(QTC_KDE_LIBS ${KDE4_KDEUI_LIBS})
  if (NOT QTC_DISABLE_KDEFILEDIALOG_CALLS)
    set(QTC_KFILEDIALOG_LIBS ${KDE4_KIO_LIBS} ${KDE4_KDECORE_LIBS})
  endif (NOT QTC_DISABLE_KDEFILEDIALOG_CALLS)
else()
  add_dependencies(qtcurve qtc_dialog_error_hdr qtc_dialog_information_hdr
    qtc_dialog_warning_hdr)
endif()

target_link_libraries(qtcurve
  ${QTC_KDE_LIBS}
  ${QTC_KFILEDIALOG_LIBS}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTSVG_LIBRARY})

if(Q_WS_X11)
  target_link_libraries(qtcurve
    ${QT_QTDBUS_LIBRARY}
    ${X11_X11_LIB})
endif()

if(QTC_QT4_ENABLE_KDE4)
  install(TARGETS qtcurve DESTINATION ${QTCURVE_STYLE_DIR})
else()
  install(TARGETS qtcurve LIBRARY DESTINATION ${QTCURVE_STYLE_DIR})
endif()
