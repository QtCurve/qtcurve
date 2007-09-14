set(qtcurve_SRCS qtcurve.cpp)
add_definitions(-DQT_PLUGIN)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} ${QT_INCLUDE_DIR}
                     ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/common)
#kde4_add_plugin(qtcurve ${qtcurve_SRCS} )
qt4_automoc(${qtcurve_SRCS})
add_library(qtcurve SHARED ${qtcurve_SRCS})
set_target_properties(qtcurve PROPERTIES PREFIX "")
target_link_libraries(qtcurve ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${X11_X11_LIB})
install(TARGETS qtcurve DESTINATION ${QTCURVE_STYLE_DIR})
if (KDE4_FOUND)
    install(FILES qtcurve.themerc DESTINATION ${DATA_INSTALL_DIR}/kstyle/themes)
endif (KDE4_FOUND)


