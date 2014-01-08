set(qtcurve_config_SRCS
  load.c)
set(qtcurve_config_LINKS qtcurve-utils)
add_definitions("-DQTC_UTILS_INTERNAL")

add_library(qtcurve-config SHARED ${qtcurve_config_SRCS})
target_link_libraries(qtcurve-config ${qtcurve_config_LINKS})

set_target_properties(qtcurve-config PROPERTIES
  VERSION 0.1
  SOVERSION 0
  COMPILE_FLAGS "-fvisibility=hidden"
  LINK_FLAGS "-Wl,--no-undefined"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
install(TARGETS qtcurve-config LIBRARY DESTINATION "${LIB_INSTALL_DIR}")
