if(ENABLE_QT4)
  find_package(Qt4 REQUIRED)
  set(gen_image_header_SRCS
    gen_image_header_qt4.cpp)
  set(gen_image_header_INCLUDES
    "${QT_INCLUDE_DIR}"
    "${QT_QTGUI_INCLUDE_DIR}"
    "${QT_QTCORE_INCLUDE_DIR}")
  set(gen_image_header_LINKS
    ${QT_QTGUI_LIBRARY}
    ${QT_QTCORE_LIBRARY})
endif()

include_directories(${gen_image_header_INCLUDES})
add_executable(gen-image-header ${gen_image_header_SRCS})
target_link_libraries(gen-image-header ${gen_image_header_LINKS})

if(ENABLE_QT4)
  set(qt4_dump_png_SRCS
    qt4_dump_png.c)
  add_executable(qt4-dump-png ${qt4_dump_png_SRCS})
endif()
