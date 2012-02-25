project(QtCurve-Gtk2)
cmake_minimum_required(VERSION 2.4.4)

set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_GENERATOR "DEB;RPM;TGZ")
set(DEBIAN_PACKAGE_DESCRIPTION "QtCurve for Gtk2")
set(DEBIAN_PACKAGE_SECTION "gtk")
set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "8")
set(CPACK_PACKAGE_VERSION_PATCH "14")
set(CPACK_PACKAGE_CONTACT "Craig Drummond <craig.p.drummond@gmail.com>")
set(QTCURVE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
set(QTCURVE_VERSION_FULL "${QTCURVE_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${QTCURVE_VERSION_FULL}")
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${QTCURVE_VERSION_FULL}")
include(CPack)

include(UsePkgConfig)

set(GTK_MAJOR_VERSION "2")
pkgconfig(gtk+-${GTK_MAJOR_VERSION}.0 GTK_INCLUDE_DIR GTK_LIB_DIR GTK_LINK_FLAGS GTK_CFLAGS)

if (GTK_INCLUDE_DIR)
    # Try to determine KDE3's icon prefix...
    find_program(KDE3CONFIG_EXECUTABLE NAMES kde-config
                 PATHS $ENV{KDEDIR}/bin /opt/kde3/bin /opt/kde/bin NO_DEFAULT_PATH)

    find_program(KDE3CONFIG_EXECUTABLE kde-config)

    if(KDE3CONFIG_EXECUTABLE)
        exec_program(${KDE3CONFIG_EXECUTABLE}
                    ARGS --expandvars --install icon
                    OUTPUT_VARIABLE KDE3_ICONS_PREFIX)
        exec_program(${KDE3CONFIG_EXECUTABLE}
                     ARGS --prefix
                     OUTPUT_VARIABLE KDE3PREFIX)
    endif(KDE3CONFIG_EXECUTABLE)

    # Try to determine KDE4's icon prefix...
    file(TO_CMAKE_PATH "$ENV{KDEDIRS}" _KDEDIRS)

    find_program(KDE4CONFIG_EXECUTABLE NAMES kde4-config
                 PATHS ${CMAKE_INSTALL_PREFIX}/bin ${_KDEDIRS} /opt/kde4/bin /usr/lib/kde4/bin NO_DEFAULT_PATH )

    if(NOT KDE4CONFIG_EXECUTABLE)
        find_program(KDE4CONFIG_EXECUTABLE NAMES kde4-config )
    endif(NOT KDE4CONFIG_EXECUTABLE)

    if(KDE4CONFIG_EXECUTABLE)
        exec_program(${KDE4CONFIG_EXECUTABLE}
                    ARGS --expandvars --install icon
                    OUTPUT_VARIABLE KDE4_ICONS_PREFIX)
        exec_program(${KDE4CONFIG_EXECUTABLE}
                     ARGS --prefix
                     OUTPUT_VARIABLE KDE4PREFIX)
    endif(KDE4CONFIG_EXECUTABLE)

    exec_program(${PKGCONFIG_EXECUTABLE}
                 ARGS --variable=gtk_binary_version gtk+-${GTK_MAJOR_VERSION}.0
                 OUTPUT_VARIABLE GTK_VERSION)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        exec_program(${PKGCONFIG_EXECUTABLE}
                     ARGS --variable=prefix gtk+-${GTK_MAJOR_VERSION}.0
                     OUTPUT_VARIABLE GTK_PREFIX)
        set(CMAKE_INSTALL_PREFIX ${GTK_PREFIX})
    else(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(GTK_PREFIX ${CMAKE_INSTALL_PREFIX})
    endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(GTK_THEME_DIR ${GTK_PREFIX}/share/themes/QtCurve)

    if(NOT KDE3PREFIX)
        if(KDE4PREFIX)
            set(KDE3PREFIX ${KDE4PREFIX})
        else(KDE4PREFIX)
            set(KDE3PREFIX /usr)
        endif(KDE4PREFIX)
    endif(NOT KDE3PREFIX)

    if(NOT KDE4PREFIX)
        if(KDE3PREFIX)
            set(KDE4PREFIX ${KDE3PREFIX})
        else(KDE3PREFIX)
            set(KDE4PREFIX /usr)
        endif(KDE3PREFIX)
    endif(NOT KDE4PREFIX)

    if(NOT KDE3_ICONS_PREFIX)
        if(KDE4_ICONS_PREFIX)
            set(KDE3_ICONS_PREFIX ${KDE4_ICONS_PREFIX})
        else(KDE4_ICONS_PREFIX)
            set(KDE3_ICONS_PREFIX ${KDE3PREFIX}/share/icons)
        endif(KDE4_ICONS_PREFIX)
    endif(NOT KDE3_ICONS_PREFIX)

    if(NOT KDE4_ICONS_PREFIX)
        if(KDE3_ICONS_PREFIX)
            set(KDE4_ICONS_PREFIX ${KDE3_ICONS_PREFIX})
        else(KDE3_ICONS_PREFIX)
            set(KDE4_ICONS_PREFIX ${KDE4PREFIX}/share/icons)
        endif(KDE3_ICONS_PREFIX)
    endif(NOT KDE4_ICONS_PREFIX)

    configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h)
    add_subdirectory(style)
    add_subdirectory(mozilla)
    message("** PREFIX=${CMAKE_INSTALL_PREFIX}\n")

    if(QTC_REORDER_GTK_DIALOG_BUTTONS)
        message("**************************************************************\n"
                "Warning: You have enabled the option for QtCurve to try to manually\n"
                "         re-order the buttons of Gtk dialogs. Note: This will cause\n"
                "         errors to be printed to the console, and is only an\n"
                "         experiment!\n"
                "**************************************************************\n")
    endif(QTC_REORDER_GTK_DIALOG_BUTTONS)
    if(QTC_OLD_MOZILLA)
        message("**************************************************************\n"
                "Information: Defaulting to treating Firefox as Firefox < 3.0\n"
                "**************************************************************\n")
    else(QTC_OLD_MOZILLA)
        message("**************************************************************\n"
                "Information: Defaulting to treating Firefox as Firefox >= 3.0\n"
                "**************************************************************\n")
    endif(QTC_OLD_MOZILLA)
    if(QTC_DEFAULT_TO_KDE3)
        message("**************************************************************\n"
                "Information: Defaulting to reading KDE3 settings if \n"
                "             KDE_SESSION_VERSION is not set\n"
                "**************************************************************\n")
    else(QTC_DEFAULT_TO_KDE3)
        message("**************************************************************\n"
                "Information: Defaulting to reading KDE4 settings if \n"
                "             KDE_SESSION_VERSION is not set\n"
                "**************************************************************\n")
    endif(QTC_DEFAULT_TO_KDE3)
    if(QTC_MODIFY_MOZILLA)
        message("**************************************************************\n"
                "**************************************************************\n"
                "Warning: You have enabled the option for QtCurve to modify the\n"
                "         configuration files of Mozilla applications (Firefox, \n"
                "         Thunderbird, Seamonkey). Note: This sometimes causes\n"
                "         issues with newer versions of these applications. Should\n"
                "         such issues occur, you should recompile QtCurve without\n"
                "         this option - and you will have to remove BY HAND the \n"
                "         changes that QtCurve has made. This will entail removing\n"
                "         lines from userChrom.css that indicate 'Added by QtCurve'\n"
                "         userChrome.css is usually in a folder such as:\n"
                "             ~/.mozilla/firefox/xxxxxxx.default/chrome/userChrome.css\n"
                "             ~/.thunderbird/xxxxxxx.default/chrome/userChrome.css\n"
                "         Where xxxxxxx is a series of random characters.\n"
                "**************************************************************\n"
                "**************************************************************\n")
    endif(QTC_MODIFY_MOZILLA)

    if(QTC_USE_CAIRO_FOR_ARROWS)
        message("**************************************************************\n"
                "Warning: Using ciaro to draw arrows - this has been reported to \n"
                "         fail with intel Xorg driver > 2.8\n"
                "**************************************************************\n")
    else(QTC_USE_CAIRO_FOR_ARROWS)
        message("**************************************************************\n"
                "Information: Using Gdk to draw arrows.\n"
                "**************************************************************\n")
    endif(QTC_USE_CAIRO_FOR_ARROWS)

    if (QTC_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT)
        message("**************************************************************\n"
                "Warning: You have enabled support for the 'fix parentless dialogs'\n"
                "         option. This is known to BREAK some applications. Please\n"
                "         DO NOT report errors to application authors when you have\n"
                "         this enabled. Please reconsider DISABLING this option.\n"
                "**************************************************************\n")
    endif (QTC_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT)

else (GTK_INCLUDE_DIR)
    message(FATAL_ERROR "Could not locate Gtk2 include files")
endif (GTK_INCLUDE_DIR)
