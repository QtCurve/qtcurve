set(qtcurve_utils_SRCS
  color.c
  dirs.c
  log.c
  utils.c
  strs.c
  shadow.c)
set(qtcurve_utils_LINKS
  m ${LIBEXECINFO_LIBRARIES})

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

if(QTC_ENABLE_X11)
  set(qtcurve_utils_SRCS
    ${qtcurve_utils_SRCS}
    x11utils.c
    x11helpers.c)
  set(qtcurve_utils_LINKS
    ${qtcurve_utils_LINKS}
    ${QTC_X11_LIBRARIES})
  include_directories(${QTC_X11_INCLUDE_DIRS})
  add_definitions(${QTC_X11_CFLAGS})
endif()

add_definitions("-DQTC_UTILS_INTERNAL")

add_library(qtcurve-utils SHARED
  ${qtcurve_utils_SRCS})

target_link_libraries(qtcurve-utils ${qtcurve_utils_LINKS})

set_target_properties(qtcurve-utils PROPERTIES
  VERSION 1.0
  SOVERSION 1
  COMPILE_FLAGS "-fvisibility=hidden"
  LINK_FLAGS "-Wl,--no-undefined"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
install(TARGETS qtcurve-utils LIBRARY
  DESTINATION "${LIB_INSTALL_DIR}")
