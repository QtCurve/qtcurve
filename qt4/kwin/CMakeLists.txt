if(NOT QTC_QT4_ENABLE_KWIN)
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
kde4_add_plugin(kwin3_qtcurve_kde4 ${kwin3_qtcurve_PART_SRCS})
set_target_properties(kwin3_qtcurve_kde4 PROPERTIES
  LINK_FLAGS "-Wl,--no-undefined"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  OUTPUT_NAME "kwin3_qtcurve")
target_link_libraries(kwin3_qtcurve_kde4 kdecorations
  ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES} ${QTC_QT4_XCB_LINK_LIBS}
  qtcurve-utils)
install(TARGETS kwin3_qtcurve_kde4 DESTINATION "${PLUGIN_INSTALL_DIR}")
install(FILES qtcurve.desktop DESTINATION "${DATA_INSTALL_DIR}/kwin/")
