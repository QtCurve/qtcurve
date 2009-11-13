install(FILES Silk.qtcurve Clear.qtcurve Dull_Glass.qtcurve Shiny_Glass.qtcurve Inverted.qtcurve Plain.qtcurve Glass.qtcurve
        Human.qtcurve Plastic.qtcurve Murrine.qtcurve Curve.qtcurve Clean.qtcurve Glow.qtcurve Agua.qtcurve Ozone.qtcurve DESTINATION ${DATA_INSTALL_DIR}/QtCurve)

if (QTC_STYLE_SUPPORT)
    install(FILES qtc_klearlooks.themerc DESTINATION ${DATA_INSTALL_DIR}/kstyle/themes)
else (QTC_STYLE_SUPPORT)
    install(FILES Klearlooks.qtcurve DESTINATION ${DATA_INSTALL_DIR}/QtCurve)
endif (QTC_STYLE_SUPPORT)