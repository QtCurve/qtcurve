foreach(pngname shadow0 shadow1 shadow2 shadow3 shadow4 shadow5 shadow6 shadow7)
  add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${pngname}-png.h"
    COMMAND "${PROJECT_BINARY_DIR}/tools/gen-image-header"
    "${CMAKE_CURRENT_SOURCE_DIR}/${pngname}.png" "qtc_${pngname}"
    "${CMAKE_CURRENT_BINARY_DIR}/${pngname}-png.h"
    DEPENDS gen-image-header "${pngname}.png")
  add_custom_target(qtc_${pngname}_hdr
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${pngname}-png.h")
endforeach()
