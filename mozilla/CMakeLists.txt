configure_file(QtCurve.css.cmake ${CMAKE_BINARY_DIR}/QtCurve.css)
configure_file(QtCurve-KDEButtonOrder.css.cmake ${CMAKE_BINARY_DIR}/QtCurve-KDEButtonOrder.css)

install(FILES ${CMAKE_BINARY_DIR}/QtCurve.css ${CMAKE_BINARY_DIR}/QtCurve-KDEButtonOrder.css preferences-rev.xml
        DESTINATION ${GTK_THEME_DIR}/mozilla)
