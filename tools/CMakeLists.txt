if(ENABLE_QT4 OR ENABLE_QT5)
  set(qt_dump_png_SRCS
    qt_dump_png.c)
  add_executable(qt-dump-png ${qt_dump_png_SRCS})
endif()

if(ENABLE_QT5)
  set(gen_image_header_SRCS
    gen_image_header_qt.cpp)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Gui REQUIRED)
  set(gen_image_header_LINKS ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES})
  set(gen_image_header_INCLUDES ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS})
  set(gen_image_header_DEFS ${Qt5Core_DEFINITIONS} ${Qt5Gui_DEFINITIONS})
elseif(ENABLE_QT4)
  find_package(Qt4 REQUIRED)
  set(gen_image_header_SRCS
    gen_image_header_qt.cpp)
  set(gen_image_header_INCLUDES
    "${QT_INCLUDE_DIR}"
    "${QT_QTGUI_INCLUDE_DIR}"
    "${QT_QTCORE_INCLUDE_DIR}")
  set(gen_image_header_LINKS
    ${QT_QTGUI_LIBRARY}
    ${QT_QTCORE_LIBRARY})
  set(gen_image_header_DEFS)
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GEN_IMG_GTK REQUIRED cairo-png)
  set(gen_image_header_SRCS
    gen_image_header_cairo.c)
  set(gen_image_header_INCLUDES
    ${GEN_IMG_GTK_INCLUDE_DIRS})
  set(gen_image_header_LINKS ${GEN_IMG_GTK_LDFLAGS}
    ${GEN_IMG_GTK_LIBRARIES})
  set(gen_image_header_DEFS ${GEN_IMG_GTK_CFLAGS})
endif()

string(REPLACE ";" " " gen_image_header_DEFS "${gen_image_header_DEFS}")

if("${CMAKE_VERSION}" VERSION_LESS 2.8.8)
  include_directories(${gen_image_header_INCLUDES})
endif()

add_executable(gen-image-header ${gen_image_header_SRCS})
set_target_properties(gen-image-header PROPERTIES
  INCLUDE_DIRECTORIES "${gen_image_header_INCLUDES}"
  COMPILE_FLAGS "${gen_image_header_DEFS}")
target_link_libraries(gen-image-header ${gen_image_header_LINKS})
