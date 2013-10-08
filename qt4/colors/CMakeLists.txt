# translate_add_sources(QtCurve.colors QtCurveAgua.colors)

if(NOT QTC_QT4_ENABLE_KDE)
  return()
endif()

install(FILES QtCurve.colors QtCurveAgua.colors
  DESTINATION "${DATA_INSTALL_DIR}/color-schemes")
