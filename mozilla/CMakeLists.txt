configure_file(QtCurve.css.cmake ${CMAKE_BINARY_DIR}/QtCurve.css)
configure_file(firefox-user.js.cmake ${CMAKE_BINARY_DIR}/firefox-user.js)

install(FILES ${CMAKE_BINARY_DIR}/QtCurve.css preferences-rev.xml ${CMAKE_BINARY_DIR}/firefox-user.js mailto.sh
        DESTINATION ${GTK_THEME_DIR}/mozilla)
