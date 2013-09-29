find_package(Qt4 REQUIRED)
add_definitions(${QT_DEFINITIONS})
include_directories(
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  ${QT_INCLUDE_DIR})

add_definitions("-DQT_DISABLE_DEPRECATED_BEFORE=0x050000")

# search packages used by KDE
if(QTC_QT4_ENABLE_KDE)
  find_package(KDE4 REQUIRED)
  include(KDE4Defaults)

  add_definitions(${KDE4_DEFINITIONS})
  include_directories(${KDE4_INCLUDES})
  set(QTCURVE_STYLE_DIR "${PLUGIN_INSTALL_DIR}/plugins/styles/")

  if(NOT DEFINED QTC_QT4_ENABLE_KWIN)
    find_file(KWINGLOBALS_H kwinglobals.h ${KDE4_INCLUDES})
    if(KWINGLOBALS_H)
      set(QTC_QT4_ENABLE_KWIN On)
    else()
      set(QTC_QT4_ENABLE_KWIN Off)
    endif()
  endif()
else()
  set(QTCURVE_STYLE_DIR "${QT_PLUGINS_DIR}/styles/")
endif()

add_subdirectory(config)
add_subdirectory(themes)
add_subdirectory(colors)
add_subdirectory(kwin)
add_subdirectory(kwinconfig)
add_subdirectory(style)
