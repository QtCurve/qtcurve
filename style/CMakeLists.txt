set(qtcurve_SRCS qtcurve.cpp)
add_definitions(-DQT_PLUGIN)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} ${QT_INCLUDE_DIR}
                     ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/common)
#kde4_add_plugin(qtcurve ${qtcurve_SRCS} )
qt4_automoc(${qtcurve_SRCS})
add_library(qtcurve SHARED ${qtcurve_SRCS})
set_target_properties(qtcurve PROPERTIES PREFIX "")

if (KDE4_FOUND)
    install(FILES qtcurve.themerc DESTINATION ${DATA_INSTALL_DIR}/kstyle/themes)
    set(QTC_KDE_LIBS ${KDE4_KDEUI_LIBS})
    if (NOT QTC_DISABLE_KDEFILEDIALOG_CALLS)
        set(QTC_KFILEDIALOG_LIBS ${KDE4_KIO_LIBS} ${KDE4_KDECORE_LIBS} ${KDE4_KFILE_LIBS})
    endif (NOT QTC_DISABLE_KDEFILEDIALOG_CALLS)
endif (KDE4_FOUND)

target_link_libraries(qtcurve ${QTC_KDE_LIBS} ${QTC_KFILEDIALOG_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${X11_X11_LIB})
install(TARGETS qtcurve DESTINATION ${QTCURVE_STYLE_DIR})

