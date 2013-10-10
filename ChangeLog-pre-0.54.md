## 0.53-KDE4
1. Removed 'Shadow buttons' option, and replaced with none/shadow/etch setting
   (default to 'none').
2. Added 'passwordChar' option to set character used for password entries.
3. Option to have frameless groupboxes - Gtk like.
4. Add an 'Advanced' tab to config dialog.
5. Add config item `gtkButtonOrder` set to 'true' to use Gtk/GNOME button order.
6. Modified contrast settings to be more varied.
7. Gradient background of checks and radios, if appearance is not flat/raised.
8. Slight improvement to etching under krunner.
9. Fix for `PE_PanelLineEdit`
10. Better look for toolbuttons with drop down indicators.
11. Fix mouseover for flat pushbuttons.

## 0.53-gtk2
1. Removed 'Shadow buttons' option, and replaced with none/shadow/etch setting -
   default to 'none'.
2. Added 'passwordChar' option to set character used for password entries.
3. Option to have frameless groupboxes - Gtk like.
4. Add config item 'gtkButtonOrder' set to 'true' to use Gtk/GNOME button order.
5. Added config item 'mapKdeIcons' to control whether to map KDE icons or not.
6. Modified contrast settings to be more varied.
7. Gradient background of checks and radios, if appearance is not flat/raised.
8. Create kde-icon map on the fly - allows icon sizes to be read from kdeglobals.
9. KDE's "apply colours to non-KDE apps" setting seems to mess up the text on
   progressbars, workaround this.
10. Read alternate listview colour from KDE settings.
11. More KDE like framed groupboxes.

## 0.52.4-KDE4
1. Update `CMakeLists.txt` to better match KDE4.
2. Fix frames in systemsettings.
3. Fix mouse over of disabled toolbutton drop down indicators.
4. Fix combobox & spinbutton 'raised' appearance.
5. Better button widths.

## 0.52.3-KDE4
1. Don't lighten border of disabled check/radio buttons.
2. Lighten trough of disabled slider.
3. Store no etch widgets in a set, to speed up checking.
4. When building for Qt4 only, you **must** build from within the qt4 folder.
   This is due to the different ways qmake and cmake create moc files.

## 0.52.3-gtk2
1. Fix 1st stripe on vertical progress bar.
2. Fix very small progress bar chunks.
3. Fix blanked out widgets in tovid.
4. Fix DeVeDe crash when 'fix parentless dialogs' is enabled.

## 0.52.2-KDE4
1. Fix crash when slider colour == button.

## 0.52.2-gtk2
1. Don't lighten border of disabled check/radio buttons.
2. Lighten trough of disabled slider.

## 0.52.1-KDE4
1. Draw emphasis around menus when not lightening.
2. Use button colours to border entry fields.
3. Dont allow scrollbars to be recoloured.
4. Nicer etching/shadows - use alpha as opposed to mixing colours.
5. Disable etch/shadow for plasma and khtml widgets.
6. Adjust tab heights.

## 0.52.1-gtk2
1. Draw emphasis around menus when not lightening.
2. Use button colours to border entry fields.
3. Fix menu standard background.

## 0.52-KDE4
1. Use 'dull glass' as the default gradient - previous default is now
   called 'Clean'.
2. Darken slider mouse over colour if slider is not shaded.
3. Use thinner slider mouse over sections if slider is not shaded.
4. Improve look of small V arrows.
5. Even duller dull glass, but much more useable.
6. Fix setting of check/radio colour.
7. Only highlight editable combo arrow when mouse over arrow, not over
   edit field - more Gtk like.
8. Added two config file options (no gui):
    1. `gtkScrollViews`

        set to `true` to have the scrollbars drawn outside of scrollviews.

    2. `gtkComboMenus`

        set to `true` so that non-editable behave as per Gtk, draw up and down
        arrow as per Gtk, and combo list uses menu colour for background.

9. Style `KTitleWidget`
10. Fix combo boxes in listviews.
11. Fix crash in designer when 'Fix parentless dialogs' is enabled.
12. Use 64bit ints as index for pixmap cache - should be faster
    than strings.
13. Fix Qt4.3 crash - thanks to Jakub Dvorak.
14. Treat 'NoToolBarArea' as top/bottom toolbars.
15. Make spinbutton height more consistent with KDE3.
16. Implement `PE_PanelButtonBevel` - theme's QtConfig's colour buttons.

## 0.52-gtk2
1. Use 'dull glass' as the default gradient - previous default is now
   called 'Clean'.
2. Darken slider mouse over colour if slider is not shaded.
3. Use thinner slider mouse over sections if slider is not shaded.
4. Improve look of small V arrows.
5. Even duller dull glass, but much more useable.
6. Implement 'plastik' style mouse over for square sliders.
7. Fix bottom/right scrollbar arrow position.
8. Fix spinbutton arrow position.
9. Fix SWT combo arrows.
10. Don't prelight SWT combos - they don't un-prelight when activated!
11. For non editable combos, draw both an up and a down arrow.
12. For editable combos - make menu more KDE list like.

## 0.51-KDE4
1. Changed shading to use HSL colour space. This can be altered by
   editing `$XDG_CONFIG_HOME/qtcurvestylerc` and setting `shading=simple`
   for the previous method, or `shading=hsv` to use HSV.
2. Add options:

    1. Border all of menu/toolbars.
    2. Darker borders.
    3. 'V' arrows.

3. Fix raised listview headers.
4. Fix glass style menuitem appearance.
5. Modifed look of dullglass, looks "softer"
6. Improve look of plastik mouse-over for non coloured scrollbars.
7. For disabled buttons, use standard fill but lighten border.
8. Use darker colours for mouse-over and default button - helps with
   light colour schemes.
9. Fix listviews!
10. Imlement `Q3ListView` styling.
11. Fix menuitem height to be more consistent with KDE3.

## 0.51-gtk2
1. Changed shading to use HSL colour space. This can be altered by
   editing `$XDG_CONFIG_HOME/qtcurvestylerc` and setting `shading=simple`
   for the previous method, or `shading=hsv` to use HSV.
2. Add options:

    1. Border all of menu/toolbars.
    2. Darker borders.
    3. 'V' arrows.

3. Fix raised listview headers.
4. Fix glass style menuitem appearance.
5. Modifed look of dullglass, looks "softer"
6. Improve look of plastik mouse-over for non coloured scrollbars.
7. For disabled buttons, use standard fill but lighten border.
8. Use darker colours for mouse-over and default button - helps with
   light colour schemes.
9. Dont draw sunken panel around checked menuitems.
10. If the app is a Java app, and its `g_get_application_name() != "unknown"`,
    then assume its a SWT java app - in which case treat as a standard app. For
    Swing apps some functionality is disabled.
11. Fix tabs in thunderbird.

## 0.50-KDE4
1. Add settings for:

    1. Fill used slider
    2. Round menubar item top only
    3. Menuitem appearance
    4. Border menuitems
    5. Progressbar appearance
    6. Gradient progressbar groove
    7. Use standard buttons for sidebar buttons
    8. Check/radio colour
    9. Plastik style mouse-over

2. Dont colour menubar items on mouse over if not colouring menubars.
3. When drawing menubar borders, only draw bottom line.
4. When drawing toolbar borders, only draw top/bottom or left/right
   depending upon orientation.
5. Dont draw text under dockwidget buttons.
6. Use 'foreground' colour for menu text.
7. Dont't focus highlight for scrollview widgets - makes more consistent
   with KDE3 and Gtk2.
8. Don't draw toolbar borders for non QMainWindow parented toolbars,
   makes more consistent with KDE3
9. Fix some drawing errors with top tabs.
10. Darken dock widget title area.
11. Fix button shift

## 0.50-gtk2
1. Add settings for:

    1. Fill used slider
    2. Round menubar item top only
    3. Menuitem appearance
    4. Border menuitems
    5. Progressbar appearance
    6. Gradient progressbar groove
    7. Check/radio colour
    8. Plastik style mouse-over

2. Dont colour menubar items on mouse over if not colouring menubars.
3. When drawing menubar borders, only draw bottom line.
4. When drawing toolbar borders, only draw top/bottom or left/right -
   depending upon orientation.
5. Draw checks/radios within listviews the same as standard.
6. If run under KDE4, then draw checks in menus the same as standard
   checks.
7. Move upper spin button down 1 pixel.
8. Default to KDE3/Qt3 settings when not run under KDE.
9. Improve (slightly) appearance of java apps.

## 0.49-KDE4
1. Initial port to Qt4

## 0.49-gtk2
1. Add settings for:

    1. Tab appearance
    2. Listview appearance
    3. Slider appearance

2. If `$KDE_SESSION_VERSION` is set to >=4, then read settings from
   `$XDG_CONFIG_HOME/Trolltech.conf`
3. Lighten focus rectangle.
4. Fixed tab shift for GTK 2.10.11. Possibly should be applied to an earlier
   version, but not sure.
5. When run under KDE4, allow radio buttons in menus.
6. Add `/usr/share/icons` into icon search path

## 0.48.5-gtk2
1. Fix slight slider drawing errors.
2. Set cursor colours.

## 0.48.4-gtk2
1. Make light border consistent with KDE - thanks to Daniel Bausch

## 0.48.3-gtk2
1. Fix focus rectangles when not fully rounded.

## 0.48.2-gtk2
1. Modifed the glass variants. Dull is a bit 'duller' in the top 1/2, and
   fades away at the bottom. Shiny is now more like dull, but with more
   pronounced gradients.
2. Fix borders of selected menubar item when colouring menubar.
3. Only darken menubar colour when using selcted backgound colour, if
   using glass gradients.

## 0.48.1-gtk2
1. Draw light border around all of progressbar.
2. Fix look of vertical progressbars.
3. Work-around for SWT combos.

## 0.48-gtk2
1. Fix coloured mouse over for glass styles.
2. Re-introduce the following options:

    1. Roundedness
    2. List view lines
    3. Striped progress bars
    4. Drawing of statusbar frames

3. Draw light border around progressbar elements when not in
   flat/raised/inverted mode.
4. Oval shaped sliders when fully round, otherwise rectangular
5. Proper ythickness settings for frames.
6. By default do not alter user's `userChrome.css` file. This now has to be
   explicitly enabled via `--enable-mozilla-mods` `./configure` option.

## 0.47-gtk2
1. Always draw light border around glass elements.
2. Removed the following config options:

    1. Custom light border colour
    2. Fill used slider groove - always filled.
    3. Stripped progress - always striped
    4. V Arrows
    5. Check/radio background highlight
    6. Round menubar item top only
    7. Draw statusbar frames - never drawn.
    8. Highlight selected text fields - always.
    9. Standard highlight for inactive windows - always
    10. Listview lines are either on/off, not off/dotted/solid
    11. Scrollbars and sliders share same config options
    12. Check radio colour setting - always text colour
    13. Border/round menubar/items - always rounded/bordered.
    14. Roundedness - always rounded.
    15. Listview settings - always arrows, no lines, and header
        follows general appearance.
    16. Tab appearance - set from general appearace.

3. Changed appearance of check/radios - now filled with base colour.
4. Etch look for button, combos, line edits, and spin boxes.
5. Fill check/radio background on mouse over.
6. Remove `--enable-old-shading`
7. Modify shade settings so that what was "Shade selected" becomes
   "Shade blended selected", and add a new "Shade selected" that just
   uses the selected background colour without blending.
8. Round slider thumbs.
9. Option to draw a shadow underneath buttons.
10. Draw a rounded gray rectangle for focus - option to set this to
    windows-like focus.
11. Fix `qtcExit()->qtExit()` when compiled with event filter.
12. Don't do check/radio mouse over for mozilla apps - doesn't work very well.
