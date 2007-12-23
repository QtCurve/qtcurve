# add_subdirectory(config)
set(kwin3_qtcurve_PART_SRCS qtcurvehandler.cpp qtcurveclient.cpp qtcurvebutton.cpp)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/common ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} ${QT_INCLUDE_DIR}
                     ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/common ${CMAKE_SOURCE_DIR}/style)
kde4_add_plugin(kwin3_qtcurve ${kwin3_qtcurve_PART_SRCS})
target_link_libraries(kwin3_qtcurve kdecorations ${KDE4_KDEUI_LIBS})
install(TARGETS kwin3_qtcurve  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES qtcurve.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kwin/)
