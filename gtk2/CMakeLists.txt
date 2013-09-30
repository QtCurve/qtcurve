find_package(PkgConfig)
pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
if(GTK2_THEME_PREFIX)
  set(GTK2_PREIX "${GTK2_THEME_PREFIX}")
else()
  if(NOT GTK2_PREFIX)
    set(GTK2_PREFIX "${CMAKE_INSTALL_PREFIX}")
  endif()
endif()
if(NOT GTK2_LIBDIR)
  set(GTK2_LIBDIR "${GTK2_PREFIX}/lib")
endif()

_pkgconfig_invoke(gtk+-2.0 GTK2 BIN_VERSION "" --variable=gtk_binary_version)
set(GTK2_THEME_DIR ${GTK2_PREFIX}/share/themes/QtCurve)

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}")
add_subdirectory(style)
add_subdirectory(mozilla)
