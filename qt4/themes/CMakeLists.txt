if(NOT QTC_QT4_ENABLE_KDE)
  return()
endif()

install(FILES
  Agua.qtcurve Agua_II.qtcurve Curve.qtcurve Flat.qtcurve Human.qtcurve
  Inverted.qtcurve Klearlooks.qtcurve Milk.qtcurve Murrine.qtcurve
  Ozone.qtcurve Plastic.qtcurve Silk.qtcurve
  DESTINATION "${DATA_INSTALL_DIR}/QtCurve")
