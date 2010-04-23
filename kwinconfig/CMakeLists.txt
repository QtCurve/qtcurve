set(kwin_qtcurve_config_PART_SRCS qtcurvekwinconfig.cpp ../kwin/qtcurveconfig.cpp ../kwin/qtcurveshadowconfiguration.cpp)
kde4_add_ui_files(kwin_qtcurve_config_PART_SRCS qtcurvekwinconfigwidget.ui)
kde4_add_plugin(kwin_qtcurve_config ${kwin_qtcurve_config_PART_SRCS})
target_link_libraries(kwin_qtcurve_config ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY})
install(TARGETS kwin_qtcurve_config DESTINATION ${PLUGIN_INSTALL_DIR})

