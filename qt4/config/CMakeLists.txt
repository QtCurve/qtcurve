if(NOT QTC_QT4_ENABLE_KDE)
  return()
endif()

set(kstyle_qtcurve_config_common_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/config_file.c )

set_source_files_properties(${kstyle_qtcurve_config_common_SRCS}
  PROPERTIES LANGUAGE CXX)

set(kstyle_qtcurve_config_PART_SRCS
  qtcurveconfig.cpp
  imagepropertiesdialog.cpp
  ../kwinconfig/qtcurvekwinconfig.cpp
  ../kwin/qtcurveconfig.cpp
  ../kwin/qtcurveshadowconfiguration.cpp
  ${kstyle_qtcurve_config_common_SRCS})

if(QTC_QT4_STYLE_SUPPORT)
  set(kstyle_qtcurve_config_PART_SRCS
    ${kstyle_qtcurve_config_PART_SRCS}
    exportthemedialog.cpp)
endif()

add_definitions(-DCONFIG_WRITE -DCONFIG_DIALOG)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
kde4_add_ui_files(kstyle_qtcurve_config_PART_SRCS
  qtcurveconfigbase.ui
  stylepreview.ui
  imageproperties.ui
  ../kwinconfig/qtcurvekwinconfigwidget.ui)
kde4_add_plugin(kstyle_qtcurve_config_kde4
  ${kstyle_qtcurve_config_PART_SRCS})
set_target_properties(kstyle_qtcurve_config_kde4 PROPERTIES
  LINK_FLAGS "-Wl,--no-undefined"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  OUTPUT_NAME "kstyle_qtcurve_config")
target_link_libraries(kstyle_qtcurve_config_kde4
  ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS}
  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
install(TARGETS kstyle_qtcurve_config_kde4
  DESTINATION "${PLUGIN_INSTALL_DIR}")
install(FILES QtCurveui.rc DESTINATION "${DATA_INSTALL_DIR}/QtCurve")
