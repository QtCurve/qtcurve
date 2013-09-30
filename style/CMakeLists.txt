set(qtcurve_style_common_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/config_file.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/colorutils.c)

include_directories(
  ${GTK2_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/common
  ${PROJECT_BINARY_DIR})

add_definitions(${GTK2_CFLAGS})

set(qtcurve_SRCS
  qtcurve.c
  qt_settings.c
  drawing.c
  helpers.c
  pixcache.c
  animation.c
  combobox.c
  entry.c
  menu.c
  scrollbar.c
  scrolledwindow.c
  tab.c
  treeview.c
  widgetmap.c
  window.c
  wmmove.c
  shadowhelper.c
  ${qtcurve_style_common_SRCS})
add_library(qtcurve MODULE ${qtcurve_SRCS})

# The following fixes odd segfaults with evolution under Fedora 14.
# See https://bugzilla.redhat.com/show_bug.cgi?id=676837
add_definitions(-Dopts=qtcurve_gtk2_opts)

target_link_libraries(qtcurve
  ${GTK2_LDFLAGS}
  ${GTK2_LIBRARIES}
  m X11)

if(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME STREQUAL GNU)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_SHARED_LINKER_FLAGS
      "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS
      "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS
      "-Wl,--enable-new-dtags ${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS
      "-Wl,--enable-new-dtags ${CMAKE_MODULE_LINKER_FLAGS}")
  endif()
  if(CMAKE_C_COMPILER MATCHES "icc")
    set(CMAKE_SHARED_LINKER_FLAGS
      "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS
      "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_MODULE_LINKER_FLAGS}")
  endif()
endif()

install(TARGETS qtcurve LIBRARY DESTINATION
  ${GTK2_LIBDIR}/gtk-2.0/${GTK2_BIN_VERSION}/engines)
install(FILES gtkrc icons3 icons4 map_kde_icons.pl kdeglobals
  DESTINATION ${GTK2_THEME_DIR}/gtk-2.0)
