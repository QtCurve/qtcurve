## 0.59.7-gtk2
1. Read tooltip colours from qt config file.

## 0.59.6-KDE4
1. In config dialog, rename the 'Fill' progressbar option to 'No border',
   and place it on the 'Bar' line.
2. Fix some drawing issues with menuitems and NVIDIA.
3. 'Fix' Skype's menu buttons.
4. Only do icon mapping for KDE apps. Fixes crash with qt-recordMyDesktop.

## 0.59.6-gtk2
1. Improve appearance of SWT scrollbars.
2. Fix colour of tri-state checkboxes.

## 0.59.5-KDE4
1. Fix corners of scrolbars in plasma 4.1's folderview.
2. Fix the dock widget titlebar buttons in KOffice 2
3. Dont activate menubar when Alt key is pressed alone - matches Gtk.
4. Make KOffice's dock widget titlebars look the same as Dolphin's.

## 0.59.5-gtk2
1. Draw disabled icons desaturated - more KDE like.
2. 'Fix' pixelation of firefox dialog icons. They look
   slightly blurry, due to being scaled, but this is nicer
   than the previous pixelated icons.
3. Removed KDE event filter (`QTC_ADD_EVENT_FILTER` cmake option) -
   causes way too many errors.

## 0.59.4-KDE4
1. If linked against KDE, honour the single/double click setting for listviews.

## 0.59.4-gtk2
1. Fix reading in of lightBorder for custom gradients.
2. Icon path fix by Ilya Paramonov.

## 0.59.3-KDE4
1. Fix Arora location field.
2. Override Qt file dialogs with KDE ones. This can be disabled by
   calling cmake with:

        -DQTC_DISABLE_KDEFILEDIALOG_CALLS=true

3. Link to KDE libraries (if KDE4 is installed) to read kwin settings,
   control the icons on buttons setting, and to load some icons. Disable
   by calling cmake with:

        -DQTC_NO_KDE4_LINKING=true

4. Fix vertical dockwidget titlebars.
5. Make busy progress thinner.
6. Left align tab text - or right align for RTL.
7. Fix positioning of tab icon for RTL.
8. Allow tabs to be recoloured.
9. Draw arrow on toolbar buttons with menus.
10. Fix missing pixels on scrollbars when slider is 1 pix away from buttons.
11. Fix custom itemview background painting.
12. Fix border of disabled progressbars.

## 0.59.3-gtk2
1. Also check in `share/kde4/apps/kstyle/themes` for `qtc_*.themerc` files.
2. Fix evolution calendar crash.
3. Fix evolution listview headers.
4. Slightly better check/radio positioning.

## 0.59.2-KDE4
1. Also check in `share/kde4/apps/kstyle/themes` for `qtc_*.themerc` files.

## 0.59.2-gtk2
1. Fix appearance of shiny glass defult buttons.
2. Give glass default buttons a light border.
3. New cmake option:

    1. `-DQTC_NEW_MOZILLA=true`

        When Firefox, and thunderbird, are being themed - treat them as if they
        are the newer versions. This sets 'newFirefox' and 'newThunderbird'
        to true by default.

## 0.59.1-KDE4
1. Fix appearance of shiny glass defult buttons.
2. Give glass default buttons a light border.
3. Fix crash when apps delete progressbars before hiding - thanks
   to Dirk Mueller.
4. Add `QFormLayout` settings for Qt >= 4.4

## 0.59.1-gtk2
1. Fix crash.

## 0.59-KDE4
1. When drawing gray focus rect, draw only slightly rounded.
2. Etch/shadow effect now applied to widget sides as well as
   top/bottom.
3. Add etching to checks, radios, slider grooves, and progressbars.
4. Option to have progress fill groove - default to true.
5. Option to display non-editable combo splitter - defaults to true.
6. Dont do coloured mouse over for mdi buttons, dock widget buttons,
   toolbar buttons, or kwin button.
7. Better coloured mouse over for checks and radios.
8. Don't use background colour for non-coloured mouse over of
   check/radios if highlighting text backgroud.
9. When colouring menubars, correctly draw menubar text colour
    of inactive windows.
10. Fix potential crash when using pixmap cache.
11. Fix menubar tracking when menubarMouseOver set to false.
12. Fix selection appearance usage.
13. Round listview selections.
14. Enable hover effects in all itemviews.
15. Fix spearator's on dolphin 4.1's info sidebar.
16. Fix mouseover of disabled menu items - visible when not
    lightening menu background.
17. Implement all size grips.
18. Fix border of non-rounded, selected tabs.
19. Fix tab coloured mouse-over when not rounded.
20. New mouse over effect - glow. Only applicable if etching or shadowing.
21. New default button indicator - glow. Only applicable if etching
    or shadowing.
22. Allow up to 10 custom defined gradients.
23. Allow custom shades to be specified.
24. Enable shadow for HTML and krunner.
25. Add option to specify active tab appearance.
26. Don't do coloured mouse-overs for sunken buttons.
27. Apply 'border menuitems' only to popup menus.
28. If lightening popup menus, and not bordering menuitems, then remove 1
    pix border arund popup menus.
29. Change default to not border menuitems.
30. Re-arrange config GUI.
31. Remove frame from kwrite.
32. Scrolview options: allow sunken, highlight, and square.
33. Option to specify progressbar groove appearance.

## 0.59-gtk2
1. When drawing gray focus rect, draw only slightly rounded.
2. Etch/shadow effect now applied to widget sides as well as top/bottom.
3. Add etching to checks, radios, slider grooves, and progressbars.
4. Option to have progress fill groove - default to true.
5. Option to display non-editable combo splitter - defaults to true.
6. Dont do coloured mouse over for toolbar buttons.
7. Better coloured mouse over for checks and radios.
8. Don't use background colour for non-coloured mouse over of
   check/radios if highlighting text backgroud.
9. If using Gtk2 >= 2.12, then respect the 'Gtk style scrollviews' setting.
10. Respect KDE's shade sorted list column setting.
11. When checking if app is firefox, check against 'firefox' as well as
    'firefox-bin'
12. Fix background of non-selected flat tabs.
13. Fix tab coloured mouse-over when not rounded.
14. Add a `QTC_STYLE` env var - used to quickly test `qtc_<name>.themerc`
    files.
15. New mouse over effect - glow. Only applicable if etching or shadowing.
16. New default button indicator - glow. Only applicable if etching
    or shadowing.
17. Allow up to 10 custom defined gradients.
18. Allow custom shades to be specified.
19. Add option to specify active tab appearance.
20. Apply 'border menuitems' only to popup menus.
21. If lightening popup menus, and not bordering menuitems, then remove 1
    pix border arund popup menus.
22. Change default to not border menuitems.
23. Assume 'xulrunner' is also Firefox.
24. Option to have squared scrollviews.
25. Option to specify progressbar groove appearance.

## 0.58-KDE4
1. Add a config option for menu stripe appearance - defaults to gradient.
2. Add a config option for selection appearance - defaults to flat. (Qt4.4+)
3. Use background colour for disabled scrollbar buttons.
4. Remove some frames from systemsettings.
5. Make list/tree view highlight more KDE3-like.
6. Don't draw menu separators through stripe.
7. Draw menu stripe on the right for RTL languages.
8. Qt4.4 fixes.
9. Fix blurry arrow on KDE's colour dialog.
10. Don't show table headers as sunken unless they actually are.
11. Don't do mouseover for disabled listviews, treeviews, or tabs.
12. Right-align QMessageBox buttons - more KDE like.
13. Supply a QtCurve colours file.
14. Lighter menustripe when not lightening menus.
15. Lighter menu separators.
16. Improve look of etch/shadow effect.
17. Disable etch/shadow for krunner.

## 0.58-gtk2
1. Add a config option for selection appearance - defaults to flat.
2. Fix entry fields and dark colour schemes.
3. Use background colour for disabled scrollbar buttons.
4. Make custom styles work under Qt4 as well.
5. Make 'slight' rounding more 'slight' -  as per pre-cairo version.
6. Help with temporary Gtk window beeing seen when QtCurve is
   configured with `-DQTC_ADD_EVENT_FILTER=true`
7. Fix memory leak - forgot to call cairo_destroy
8. Lighter menu separators.

## 0.57.1-KDE4
1. Fix border drawing when not rounding.
2. Fix for menu stripe and large icons.

## 0.57.1-gtk2
1. Better arrows positions on scrollbar steppers B and C.

## 0.57-KDE4
1. Modify defaults:

    1. Turn off shade menubar only of active window
    2. Set default button indicator to tint

2. Added option to highlight background of check/radio text on mouseover.
3. Use QPainterPath to draw borders and triangular sliders.
4. Removed plasmaHack. Drawing glitches improved by using antialising
   for certain lines.
5. Fix for kwin and Qt4.4 - thanks to Franz Fellner.
6. Don't allow to select bevelled for titlebar appearance.
7. 'Fix' for konqueror's 'show close button on tab' setting.

## 0.57-gtk2
1. Modify defaults:

    1. Turn off shade menubar only of active window
    2. Set default button indicator to tint

2. Re-added option to highlight background of check/radio text on mouseover.
3. Use cairo for all drawing.
4. Highlight expander arrows on mouse over.

## 0.56.3-KDE4
1. Don't set sliders to flat just because appearance is flat.

## 0.56.3-gtk2
1. Fix tabs on Firefox 3 beta4
2. Fix slight re-draw errors with scrollbar slider.
3. Nicer positioning of down/right scrollbar arrows.
4. Fix colours of tinted default button.
5. `QTC_MODIFY_MOZILLA` compile fix - thanks to Ben de Groot.
6. Don't set sliders to flat just because appearance is flat.
7. Map gtk-go-back-ltr, gtk-go-back-rtl, gtk-go-forward-ltr,
   and gtk-go-forward-rtl.
8. Modify gtk-about and gtk-home KDE icons mappings.
9. Fix checkbox shadow.
10. Don't allow Mozilla scrollbars to be disabled.

## 0.56.2-KDE4
1. Use case-insesitive string compares when checking Qt and KDE config files.

## 0.56.2-gtk2
1. Use case-insesitive string compares when checking Qt and KDE config files.
2. Fix some issues wth Firefox 3beta4.
3. Fix mouse over for up spin button.
4. To aid testing with Firefox 3, edit your `qtcurvestylerc` and add the
   following line:

        newFirefox=true

6. Don't indicate default button if it is disabled.

## 0.56.1-KDE4
1. Fix crash when using corner default button indicator and no coloured
   mouse over.

## 0.56.1-gtk2
1. Fix errors when using corner default button indicator and no coloured
   mouse over.
2. Fix for OpenOffice.org blanking combobox text when mouse over arrow.

## 0.56-KDE4
1. Change default settings:

    1. Dont animate progressbars
    2. Frameless groupboxes
    3. Gradient KDE4 window titlebars

2. Correctly position corner indicator.
3. Use mouse over colours for corner indicator.
4. Add option to control window titlebar appearance - default to
   gradient.
5. Fix for diagonal striped progressbars whose height!=20 (such as in
   konqueror's statusbar)
6. Dont draw line between window titlebar and contents when not rounded.
7. Option to tint the default button colour.
8. Option to draw a stripe on the left hand side of popupmenus.
9. Fix for very slim progressbars.

## 0.56-gtk2
1. Change default settings:

    1. Dont animate progressbars
    2. Frameless groupboxes

2. Correctly position corner indicator.
3. Use mouse over colours for corner indicator.
4. Fix for text of selected check/radios on menubars in Java swing apps.
5. Fix for toolbar handles of Java swing apps.
6. Fix for `GtkEntry`s within toolbars.
7. OpenOffice.org fixes - entry fields, menubar items, checks in menus.
8. Option to tint the default button colour.

## 0.55.3-KDE4
1. Fix issues with flat/raised style and titlebars.
2. Fix black square (where maximise icon would be) flashing when a window is
   maximised.
3. More rounded titlebars when fully rounded.
4. Fix 'font colour' default button indicator.
5. 'Fix' for plasma and NVIDIA cards (on my system at least). Buttons,
   scrollbars, etc., in plasma's dialogs have redraw errors on NVIDIA
   (but not on my works integrated intel chip). So, by default gradients
   on plasma widgets (of the Qt variety) are drawn directly, and not cached
   to pixmaps. To revert to the previous method, edit
   `$XDG_CONFIG_HOME/qtcurvestylerc` (e.g. `~/.config/qtcurvestylerc`)
   ...and add

        plasmaHack=false

6. Dont do mouse over for dockwidget title area.
7. Fix some slight tab drawing errors.
8. Reduced CPU usage of animated progressbars.
9. Correct reading in of contrast setting from `Trolltech.conf`
10. Read KDE3's contrast setting if not running under KDE4.
11. Fix arrows on gwenview's scrollbar buttons sometime being white.

## 0.55.3-gtk2
1. Hack around a slight menubar problem with pidgin.
2. Dont use mouseover colour for highlighted checks and radios
   in menus.
3. Fix `kde4-config` usage.
4. Draw borders for GtkViewports - fixes mising frame in compiz settings
   manager.
5. Read contrast setting from `TrollTech.conf` if running under KDE4.

## 0.55.2-KDE4
1. Fix look of disabled entry fields.
2. Add ability to import `qtc_*.themerc` settings into config dialog.
3. Try to prevent skiny toolbar buttons (such as konqueror's 'Up' button)
4. Sync with KDE4.0.
5. Improve styled KMultiTabBarTabs
6. Use alpha for blending frame borders. (Fixes problem with dolphin's
   column view)
7. Supply a KWin decoration.
8. Smaller MDI window titlebars - emphasises difference with KWin.
9. Fix text of buttons in KListView
10. Better menu button indicator code for RTL.
11. Use Qt's gradient classes to draw gradients.

## 0.55.2-gtk2
1. Fix look of disabled entry fields.
2. Add warning to `-DQTC_ADD_EVENT_FILTER` option.
3. Add cmake option: `-DQTC_REORDER_GTK_DIALOG_BUTTONS`
   When set QtCurve will try to manually re-order the buttons of Gtk
   dialogs. Note: This will cause errors to be printed to the console,
   and is only an experiment!
4. Style Gtk2.12 tooltips
5. Improve look of edit field under firefox3
6. Fix menubar items for Firefox 3
7. Fix KDE4 icon mapping to match KDE4.0
8. Use 32x32 as dialog icon size.
9. Nicer tabs for Firefox 3
10. Better (not perfect) scrollbar types for Firefox3. For the moment you must
    set `QTC_NEW_MOZILLA` (any value) before starting firefox so that QtCurve
    knows its the newer version. e.g.

        QTC_NEW_MOZILLA=1 firefox

11. Fixed reading of Qt4 font.

## 0.55.1-KDE4
1. Only allow coloured selected tabs if tab appearance is set to gradient.
2. Allow triangular sliders when not rounding.
3. Fix KDE4 cmake hassle - find KDE4 package before trying to find Qt4 package.
4. Fix clipping of triangular slider in konqueror's settings.
5. Remove usage of deprecated KConfig functions.

## 0.55.1-gtk2
1. Only allow coloured selected tabs if tab appearance is set to gradient.
2. Allow triangular sliders when not rounding.

## 0.55-KDE4
1. Ability to create custom themes. See Theme details in `README` file.
2. Supply a `Klearlooks` QtCurve theme.
3. Allow 'flat' lines in scrollbar handles, toolbar handles, toolbar
   separators, and splitters.
4. Option for 'X' style checkmarks.
5. Option to have colour the selcted tab.
6. Optional diagonal progressbar sripes.
7. Use alternating dark/light for dashed toolbar handles.
8. New 'split' style gradient.
9. Option to specify slider style: plain, round (only when `appearance=round`),
   and triangular (plastik-ish)
10. Modify default style: flat splitter lines, flat slider thumbs, no
    toolbar separators, triangular slider, diagonal progressbar stripes.

## 0.55-gtk2
1. Ability to create custom themes. See Theme details in 'README' file.
2. Fix for 'inactiveHighlight' and KDE's apply colours to non-KDE apps.
3. Allow 'flat' lines in scrollbar handles, toolbar handles, toolbar
   separators, and splitters.
4. Option for 'X' style checkmarks.
5. Option to have colour the selcted tab.
6. Optional diagonal progressbar sripes.
7. Use alternating dark/light for dashed toolbar handles.
8. New 'split' style gradient.
9. Option to specify slider style: plain, round (only when
   `appearance=round`), and triangular (plastik-ish)
10. Modify default style: flat splitter lines, flat slider thumbs, no
    toolbar separators, triangular slider, diagonal progressbar stripes.

## 0.54.1-KDE4
1. Supply a simple `mkpkg` script to create packages with checkinstall.
2. Fix drawing of menubar on KDE4's konqueror.

## 0.54.1-gtk2
1. Supply a simple `mkpkg` script to create packages with `checkinstall`.
2. Fix compilation when cmake is called with `-DQTC_MODIFY_MOZILLA=true`
3. Don't `free()` the values returned from `gtk_widget_style_get()`, use
   `gtk_requisition_free()` and `gtk_border_free()`
4. Fix frameless groupboxes in Gimp 2.4
5. After `free()`'ing memory, set var to `NULL`.

## 0.54-KDE4
1. Qt4 build now uses CMake also. i.e. CMake will determine if to compile
   KDE4 config dialog or not.
2. New option `inactiveHighlight`, if set then use a mix of highlight and
   background colour as highlight for inactive windows/elements.
3. Set KDE4 colours from Q3 settings if running under KDE3.
4. Fix for KDE4's setting of inactive palette!=active palette
   (Left highlight and highlightedText as configurable)
5. Use 'highlightedText' for highlighted part of progressbar label.
6. Increase height of spinboxes.
7. Alter `KTitleWidget` (again!)
8. Add gui to set shading option.
9. Nicer look for selected tab highlight.

## 0.54-gtk2
1. Converted buildsystem to CMake.
2. New option 'inactiveHighlight', if set then use a mix of highlight and
   background colour as highlight for inactive windows/elements.
3. Fix broken 'Thinner Menuitems' option.
4. Better code for alternate list view background.
5. Also read in Qt's inactive palette.
6. If read a font setting from `/etc/qt3/qtrc`, and there is not font setting
   in `~/.qt/qtrc` - then use the setting from `/etc/qt3/qtrc`, as opposed to
   setting a default.
7. When reading Qt4 settings, also read /etc/xdg/Trolltech.conf
8. Nicer look for selected tab highlight.
9. `realloc()` fix - thanks to 'hoodedone'
