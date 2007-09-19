include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/common ${CMAKE_BINARY_DIR} )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GTK_CFLAGS}")
set(qtcurve_SRCS qtcurve.c)
add_library(qtcurve SHARED ${qtcurve_SRCS})
target_link_libraries(qtcurve ${GTK_LINK_FLAGS})

install(TARGETS qtcurve LIBRARY DESTINATION ${GTK_LIB_DIR}/gtk-2.0/${GTK_VERSION}/engines )
install(FILES gtkrc icons3 icons4 map_kde_icons.pl
        DESTINATION ${GTK_THEME_DIR}/gtk-2.0)
