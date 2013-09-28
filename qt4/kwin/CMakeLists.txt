if(NOT QTC_QT4_ENABLE_KDE4 OR NOT QTC_QT4_ENABLE_KWIN)
  return()
endif()

# add_subdirectory(config)
set(kwin3_qtcurve_PART_SRCS
  qtcurvehandler.cpp
  qtcurveclient.cpp
  qtcurvebutton.cpp
  qtcurvesizegrip.cpp
  qtcurveshadowcache.cpp
  qtcurveconfig.cpp
  qtcurveshadowconfiguration.cpp
  tileset.cpp
  qtcurvetogglebutton.cpp)
include_directories(
  "${CMAKE_CURRENT_BINARY_DIR}"
  ${QT_QTGUI_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR})
kde4_add_plugin(kwin3_qtcurve ${kwin3_qtcurve_PART_SRCS})
target_link_libraries(kwin3_qtcurve kdecorations
  ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES})
install(TARGETS kwin3_qtcurve DESTINATION "${PLUGIN_INSTALL_DIR}")
install(FILES qtcurve.desktop DESTINATION "${DATA_INSTALL_DIR}/kwin/")
