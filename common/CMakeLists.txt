set(gen_image_header_qt4_SRCS
  gen_image_header_qt4.cpp)
include_directories("${QT_INCLUDE_DIR}"
  "${QT_QTGUI_INCLUDE_DIR}"
  "${QT_QTCORE_INCLUDE_DIR}")
add_executable(gen-image-header ${gen_image_header_qt4_SRCS})
target_link_libraries(gen-image-header
  ${QT_QTGUI_LIBRARY}
  ${QT_QTCORE_LIBRARY})

foreach(pngname shadow0 shadow1 shadow2 shadow3 shadow4 shadow5 shadow6 shadow7)
  add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${pngname}-png.h"
    COMMAND "${CMAKE_CURRENT_BINARY_DIR}/gen-image-header"
    "${CMAKE_CURRENT_SOURCE_DIR}/${pngname}.png" "qtc_${pngname}"
    "${CMAKE_CURRENT_BINARY_DIR}/${pngname}-png.h"
    DEPENDS gen-image-header "${pngname}.png")
  add_custom_target(qtc_${pngname}_hdr
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${pngname}-png.h")
endforeach()

set(qt4_dump_png_SRCS
  qt4_dump_png.c)
add_executable(qt4-dump-png ${qt4_dump_png_SRCS})

foreach(pngname check_on check_x_on dialog_error dialog_information
    dialog_warning)
  add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${pngname}-png.h"
    COMMAND "${CMAKE_CURRENT_BINARY_DIR}/qt4-dump-png"
    "${CMAKE_CURRENT_SOURCE_DIR}/${pngname}.png" "qtc_${pngname}"
    "${CMAKE_CURRENT_BINARY_DIR}/${pngname}-png.h"
    DEPENDS qt4-dump-png "${pngname}.png")
  add_custom_target(qtc_${pngname}_hdr
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${pngname}-png.h")
endforeach()
