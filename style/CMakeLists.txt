if(QTC_XBAR_SUPPORT)
    set(qtcurve_SRCS qtcurve.cpp macmenu.cpp)
    set(qtcurve_MOC_HDRS qtcurve.h macmenu.h macmenu-dbus.h)
else (QTC_XBAR_SUPPORT)
    set(qtcurve_SRCS qtcurve.cpp)
    set(qtcurve_MOC_HDRS qtcurve.h)
endif (QTC_XBAR_SUPPORT)

add_definitions(-DQT_PLUGIN)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} ${QT_INCLUDE_DIR}
                     ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/common)
#kde4_add_plugin(qtcurve ${qtcurve_SRCS} )
qt4_automoc(${qtcurve_SRCS})
qt4_wrap_cpp(qtcurve_MOC_SRCS ${qtcurve_MOC_HDRS})
add_library(qtcurve SHARED ${qtcurve_SRCS} ${qtcurve_MOC_SRCS})
set_target_properties(qtcurve PROPERTIES PREFIX "")

if (KDE4_FOUND)
    install(FILES qtcurve.themerc DESTINATION ${DATA_INSTALL_DIR}/kstyle/themes)
    if (NOT QTC_NO_KDE4_LINKING)
        set(QTC_KDE_LIBS ${KDE4_KDEUI_LIBS})
        if (NOT QTC_DISABLE_KDEFILEDIALOG_CALLS)
            set(QTC_KFILEDIALOG_LIBS ${KDE4_KIO_LIBS} ${KDE4_KDECORE_LIBS} ${KDE4_KFILE_LIBS})
        endif (NOT QTC_DISABLE_KDEFILEDIALOG_CALLS)
    endif (NOT QTC_NO_KDE4_LINKING)
endif (KDE4_FOUND)

if(QTC_XBAR_SUPPORT)
    target_link_libraries(qtcurve ${QTC_KDE_LIBS} ${QTC_KFILEDIALOG_LIBS} ${QT_QTGUI_LIBRARY}
                                  ${QT_QTDBUS_LIBRARY} ${QT_QTCORE_LIBRARY} ${X11_X11_LIB})
else (QTC_XBAR_SUPPORT)
    target_link_libraries(qtcurve ${QTC_KDE_LIBS} ${QTC_KFILEDIALOG_LIBS} ${QT_QTGUI_LIBRARY}
                                  ${QT_QTCORE_LIBRARY} ${X11_X11_LIB})
endif (QTC_XBAR_SUPPORT)

install(TARGETS qtcurve DESTINATION ${QTCURVE_STYLE_DIR})

