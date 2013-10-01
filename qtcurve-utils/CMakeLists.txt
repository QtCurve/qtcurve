set(qtcurve_utils_SRCS
  color.c)
set(qtcurve_utils_LINKS
  m)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

if(QTC_ENABLE_X11)
  set(qtcurve_utils_SRCS
    ${qtcurve_utils_SRCS}
    kdex11shadow.c
    x11utils.c)
  set(qtcurve_utils_LINKS
    ${qtcurve_utils_LINKS}
    ${QTC_X11_LIBRARIES})
  include_directories(QTC_X11_INCLUDE_DIRS)
  add_definitions(${QTC_X11_CFLAGS})
endif()

add_definitions("-DQTC_UTILS_INTERNAL")

add_library(qtcurve-utils SHARED
  ${qtcurve_utils_SRCS})

if(QTC_ENABLE_X11)
  foreach(num RANGE 7)
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shadow${num}-png.h"
      COMMAND "${PROJECT_BINARY_DIR}/tools/gen-image-header"
      "${PROJECT_SOURCE_DIR}/data/shadow${num}.png" "qtc_shadow${num}"
      "${CMAKE_CURRENT_BINARY_DIR}/shadow${num}-png.h"
      DEPENDS gen-image-header "${PROJECT_SOURCE_DIR}/data/shadow${num}.png")
    add_custom_target(qtc_shadow${num}_hdr
      DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/shadow${num}-png.h")
    add_dependencies(qtcurve-utils qtc_shadow${num}_hdr)
  endforeach()
endif()

target_link_libraries(qtcurve-utils ${qtcurve_utils_LINKS})

set_target_properties(qtcurve-utils PROPERTIES
  VERSION 0.1
  SOVERSION 0
  COMPILE_FLAGS "-fvisibility=hidden"
  LINK_FLAGS "-Wl,--no-undefined"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
install(TARGETS qtcurve-utils LIBRARY
  DESTINATION "${LIB_INSTALL_DIR}")
