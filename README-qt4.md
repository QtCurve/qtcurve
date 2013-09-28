# Installation
## Build and install

    mkdir build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr
    make
    make install

## CMake configure options
1. `QTC_QT4_ENABLE_KDE`:

    Compile QtCurve qt4 style with KDE support and dependencies.

    (Default: `On`)

2. `QTC_QT4_ENABLE_KWIN`:

    Compile QtCurve qt4 kwin style. This will only have effect when KDE support
    is turned on.

    (Default: `On` if kwin header is found and KDE support is on,
    `Off` otherwise)

3. `QTC_QT4_OLD_NVIDIA_ARROW_FIX`:

    Due to QtCurve mixing AA-drawing, with non-AA drawing (specifically for
    arrows) - sometimes the arrows would not draw correctly under NVidia. To
    work-around this, QtCurve filled the arrows using AA-ed drawing, and used
    non-AA drawing for the edges. As of 0.69.0 this code is no longer enabled
    by default - use this config option to re-enable the code.

    (Default: `Off`)

4. `QTC_QT4_STYLE_SUPPORT`:

    Support QtCurve style files. These are stored as
    `<kde prefix>/share/apps/kstyle/themes/qtc_<stylename>.themerc`
    Note that this is not supported in Qt5.

    (Default: `Off`)

5. `QTC_QT4_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT`:

    Enable support for the 'fixParentlessDialogs' config option. NOTE: This is
    known to break some applications - hence is disabled by default!

    (Default: `Off`)

6. `QTC_QT4_KWIN_MAX_BUTTON_HACK`:

    Hack to force kwin drawing maximize buttons for windows that can be
    minimised. This is a 100% hack, that may not work or compile, and may even
    crash kwin.

    (Default `Off`)

## Creating Distribution Packages
Support for creating `deb` or `rpm` package have been removed. Please make
package in the same way as any other cmake packages.

# Additional Features
## XBar/MacMenu
The XBar support was copied directly from Bespin. The relevant files are named
`macmenu.*`. These were taken from revision 652.

**Deprecated** in favor of appmenu since KWin has builtin support for that
after `4.10` and `appmenu-qt` does not depend on a certain theme.

## Themes
As of v0.55, you can create QtCurve based themes. These will appear with KDE's
style control panel's combobox of styles. To create a new theme, select
'QtCurve' from within KDE's style panel, then configure QtCurve as required.
After this, in QtCurve's config dialog, select 'Export theme' from the options
menu button. You will then be prompted to give your new 'theme' a name, and a
comment. QtCurve will then create a file named `qtc_<name>.themerc`
(e.g. `qtc_klearlooks.themerc`) - this will have the following format:

    [Misc]
    Name=Klearlooks
    Comment=Clearlooks inspired style
    [KDE]
    WidgetStyle=qtc_klearlooks
    [Settings]
    animatedProgress=false
    appearance=gradient
    ....rest of qtcurve settings...

To use this theme, either copy `qtc_<name>.themerc` to
`$KDEHOME/share/apps/kstyle/themes/`
(usually `~/.kde/share/apps/kstyle/themes/`)
or copy to `<kde install prefix>/share/apps/kstyle/themes/`
(usually `/usr/share/apps/kstyle/themes/`)

When KDE's style panel is restarted, your new theme should appear in the list.

NOTE: As of QtCurve 1.0.0 style support has been disabled by default (enable
via `QTC_QT4_STYLE_SUPPORT`) and this is not supported by Qt5.

## Testing
As of v1.2.1, QtCurve can be forced to read its settings from an alternate
config file via the `QTCURVE_CONFIG_FILE` environment variable. This is only
really useful for testing alternate config settings without changing the users
current settings.

Usage:

    QTCURVE_CONFIG_FILE=~/testfile kcalc
