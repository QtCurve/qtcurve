include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/common ${CMAKE_BINARY_DIR} )
set(CMAKE_C_FLAGS "${GTK_CFLAGS}")
set(qtcurve_SRCS qtcurve.c)
add_library(qtcurve MODULE ${qtcurve_SRCS})
add_definitions(-DQTC_MOZILLA_DIR=\\\"${GTK_THEME_DIR}/mozilla\\\")
target_link_libraries(qtcurve ${GTK_LINK_FLAGS})

install(TARGETS qtcurve LIBRARY DESTINATION ${GTK_LIB_DIR}/gtk-${GTK_MAJOR_VERSION}.0/${GTK_VERSION}/engines )
install(FILES gtkrc icons3 icons4 map_kde_icons.pl kdeglobals
        DESTINATION ${GTK_THEME_DIR}/gtk-${GTK_MAJOR_VERSION}.0)
