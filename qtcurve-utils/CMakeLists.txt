set(qtcurve_utils_SRCS
  color.c)

add_definitions("-DQTC_UTILS_INTERNAL")

add_library(qtcurve-utils SHARED
  ${qtcurve_utils_SRCS})

target_link_libraries(qtcurve-utils m)

set_target_properties(qtcurve-utils PROPERTIES
  VERSION 0.1
  SOVERSION 0
  COMPILE_FLAGS "-fvisibility=hidden"
  LINK_FLAGS "-Wl,--no-undefined"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
install(TARGETS qtcurve-utils LIBRARY
  DESTINATION "${LIB_INSTALL_DIR}")
