## 0.29.1-gtk2
1. GTK1: Compile fix.

## 0.29-gtk2
1. KDE: More consistent, with Gtk, menu separators.
2. GTK1: Dont force to flat appearance! (Only menubars are forced to flat,
   as I havnt got round to implementing menubar shading on Gtk1)
3. GTK2: When looking for firefox/thunderbird's `userChrome.css`, look for
   `<blah blah>.default` and `default.<blah blah>`
4. KDE: Move handle section into menu bar tab.
5. ALL: Allow dotted handles - default for V6.
6. ALL: Allow dotted (V6 default), or no toolbar separators.
7. GTK: React to KDE style changes - i.e. update colours, fonts, options,
   etc.
8. GTK2: Fix for text on combos turning white on mouse over.
9. GTK: Use same menu colour as KDE when shading!
10. ALL: Fix for vertical toolbars.
11. GTK2: Also look in `~/.kde` (or `$KDEHOME`) for user icons.
12. GTK2: Only write `userChrome.css` if made changes.

## 0.28-gtk2
1. KDE: Remove some debug.
2. KDE: If selected colour is too dark, just don't recolour OO.o
   menubars - lightening the background colour doesn't look good.
3. ALL: When using glass gradient, round all corners of menubar entry
   selection.
4. KDE: Allow vArrow to be disabled.

## 0.27.1-gtk2
1. GTK2: Allow to compile with Gtk <= 2.2

## 0.27-gtk2
1. ALL: Slightly lighter non-selected tab, and scrollbar groove, shading.
2. KDE: Fix for sliders in kaffeine.
3. KDE: Finally fixed 'V' arrows!
4. GTK2: Firefox & thunderbird - edit user's userChrome.css file to set
   KDE buton order, and adjust menu text if using a dark background.
5. ALL: If using selected for background on menus, automatically use selected
   text colour - as opposed to try to see if its too dark.
6. GTK: Consistent menubar shade with KDE.
7. GTK2: Try to set toolbar style, icon size, icons-on-buttons from KDE
   settings.
8. GTK2: Set alternative button order.
9. GTK2: Map some KDE icons to GTK icons.
10. KDE: Increase OO.o menu selection brightness if selected colour is 'too dark'
11. GTK: Better AA'ing around edit fields.
12. GTK: Better toolbar borders with inkscape.
13. ALL: Only round menubar items on top.
14. GTK: Read `/etc/qt3/qtrc`, `/etc/qt3/qtcurvestylerc`, `/etc/qt/qtrc`,
    `/etc/qt/qtcurvestylerc` before reading `$HOME/.qt/qtrc`, etc.
15. KDE: More consistent, with Gtk, menu check boxes.
16. GTK2: Smaller toolbars for AbiWord
17. ALL: Un-revert header changes. Fixed KDE table look, and seem to have
    fixed listview redraw!

## 0.26-gtk2
1. GTK2: Fix for combobox separator not always re-drawing.
2. GTK2: More KDE-like comboboxes
3. GTK: Use black dashes for focus.
4. GTK2: Fix for button of some editable comboboxes.
5. KDE: Better combobox metrics, etc.
6. ALL: Option to gradient toolbars.
7. KDE: Fix for "Search" label in systemsettings toolbar.
8. ALL: Allow custom menubar and slider colours.
9. GTK: Fix for not drawing coloured slider when flat.
10. ALL: Revert the header changes - had redraw problems.
11. ALL: Fix for tab-bar highlight when using light selection colours.
12. KDE: Fix for non-bordered gradient radio buttons.
13. GTK: More KDE like menu sizing.
14. KDE: Fix/hack for OO.o2.x menubars. If the selected menu colour is toodark,
    then lighten. This is required as OO.o always draws the menu text
    dark! It seems to have a check for plastik style though, and then
    it draws selected popup menu items white.

## 0.25-gtk2
1. ALL: V5 - Gradient menubar.
2. KDE: Polish disabled palette, so that all frames, etc. use the theme.
3. KDE: When press 'Defaults' on settings dialog, set the version correctly,
   and disable the options frame.
4. ALL: Code cleanup.
5. ALL: New V6 - Glass like gradients. Not default for the moment.
6. ALL: Dont gradient non selected tabs.
7. KDE: Respect setting of "dark lines" for list views.
8. ALL: Better listview headers.
9. ALL: Only round 1 side of spinbuttons.
10. ALL: Rounded entry fields (lineedits, spinwidgets, comboboxes)
11. ALL: Highlight entry boxes, spin widget entry, and combobox entry on focus.
12. KDE: More consistent with Gtk toolbar separators
13. ALL: When using light popup menu background, also use a lighter colour
    for the background of checked menu items.
14. GTK: Use same base shade for menus/progress bars as for KDE.
15. GTK2: More KDE-like combobox lines
16. GTK: Fix for some combo box variants having a 1-pixel white border.
17. ALL: If light gradient and no border, need to add edges to buttons, etc.
18. KDE: Only highlight spinbutton that mouse is over - Gtk like.

## 0.24.2-gtk2
1. KDE: Compile fix.

## 0.24.1-gtk2
1. GTK1: Compile fix.

## 0.24-gtk2
1. ALL: Lighter background for pressed buttons, etc.
2. ALL: Use KDE's buttont text colour for buttons!
3. Remove seperate V1, V2, etc style files (`.themerc`, and `gtkrc` files),
   => needs `KControl` to switch variant.
4. ALL: New V5 style - has the follwing differences from V4:

    1. Darker menubar background
    2. Lighter popup menu background
    3. Flatter gradient in probress bar and selected menu items
    4. 'V' arrows
    5. Highlight strip on selected tabs
    6. Shade sliders to the 'selected' colour

5. KDE: Fixed (actually implememented!) drawing of spinbox +/- buttons.
6. KDE: Fixed drawing of V arrows - sometimes were not filled in.
7. GTK: Make default font & colours match KDE3.5's defaults.

## 0.23.1-gtk2
1. Compile fix.

## 0.23-gtk2
1. GTK: Fix colouring of check/radios.
2. GTK: Fix bonobo toolbars - e.g. on nautilus 2.4.
3. GTK: Draw handles on GNOME panel 2.4.
4. GTK: Fix arrows going white when kde exports colours.
5. GTK: Fix for some menu items in Gaim.
6. GTK: Fix V1 style check/radios sometimes having the selected colour as
   background
7. GTK: Fix anti-aliasing on rounded radios when using gradient but no border.
8. GTK2: Fix for rendering of toggle buttons in lists/cells - patch from
   Alfons Hoogervorst
9. ALL: Draw "pressed" background on all checked menu items, not just those
   with icons.
10. ALL: Highlight splitters on mouse-over
11. ALL: New V4 (default) style, with the following:

    1. Flatter gradient
    2. Font colour used to border default buttons
    3. No bold text on default button
    4. Raised gradient (like buttons) for progress and menu bar selections
    5. Rounded menubar selections
    6. No border on splitters, use dots instead

12. KDE: Polish application palette, so that all frames, etc. use the theme.
13. KDE: Align kicker's handles better with the little arrows.
14. KDE: Support mouse over for kicker taskbar buttons >= 3.4
15. KDE: Mouse over for spin widgets.
16. KDE: Increase toolbar button size - match GTK better.

## 0.22-gtk2
1. KDE: Remove scrollbar groove flicker.
2. KDE: Remove editable combo-box flicker.
3. ALL: New check/radio list code.
4. KDE: Custom checklist controller.
5. ALL: List view expanders - +/- (V1/V2), or arrows (V3)
6. KDE: List view lines - none, dotted (V1/V2), solid (V3)
   (GTK does not (?) support lines between elements - so this setting will
   not affect GTK apps)
7. KDE: Dark (V1/V2) or light (V3) list view lines. (Ditto)

## 0.21-gtk2
1. ALL: Adjust contrast settings to make lower-contrast the default.
2. ALL: Toolbar and menubar borders: none, light, and dark (previous default)
3: ALL: V3 now uses "light" toolbar/menubar borders.
4. KDE: Don't AA radiobuttons on HTML pages.
5. KDE: Squared-off splitters.
6. GTK: Fix base/prelight colour.
7. GTK: gcc 3.4 compile fix - thanks to Adam Gorzkiewicz
8. KDE: Make tabs more GTK like.
9: KDE: More GTK-like positioning of pushbutton icons - looks *much* nicer :-)

## 0.20-gtk2
1. GTK: Compile fix when compiled with KDE<3.2
2. KDE: Slightly thinner menuitems.
3. GTK: Match KDE's menuitem size better.
4. GTK: Allow checks and radios to be re-coloured.
5. ALL: Better AA for the edges of checks and radios - not perfect tho.
6. GTK: If `$HOME` is not set, then try to ascertain from passwd entry.
7. KDE: 5 pixel border around pushbutton contents.

## 0.19-gtk2
1. KDE: Fix for KDE not setting autoDefault() property of button - assume all
   buttons can be default, leaving space for indicator.
2. ALL: Optional triangle as default button indicator.
3. GTK: Match KDE's export gtkrc colours.
4. GTK2: Fix check/radio highlight so that it is not overridden by KDE's
   export colours setting.
5. ALL: Add option to enable/disable highlighting of check/radio labels.
6. ALL: V3 - disable check/radio label highlight.
7. ALL: V3 - use triangle as default button indicator.
8. KDE: Fix menu button icon on konqueror's sidebar.
9. ALL: Allow setting of slider thumbs: raised, sunken, or none.
10. ALL: Allow setting of handles: raised or sunken.
11. ALL: V3 - use sunken handles.
12. KDE: Fix for korn.
13. KDE: Fix for titlebutton on floating Qt windows - e.g. docks.
14. KDE: Fix for amaroK - buttons in player window were too large!

## 0.18-gtk2
1. KDE: Fix for colouring of kicker's task buttons - i.e. these should have
   highlight colour when an app is opened in the background.
2. KDE: Use `::qt_cast<type *>(widget)` as opposed to
   `widget->inherits("type")` whenever possible.
3. GTK2: Compile fix.

## 0.17.2-gtk2
1. GTK: Compile fix when compiled with KDE<3.2

## 0.17.1-gtk2
1. KDE: Use Qt 3.1 for ui file - so that will compile with Qt3.2, etc...
   (Using 3.1 as I only have 3.1 and 3.3 installed...)

## 0.17-gtk2
1. GTK: Compile fix - used C++ syntax in C code, oops...
2. GTK2: Make focus rects more like KDE's
3. KDE: Allow combo-boxes and scrollbars to be recoloured.
4. ALL: If compiled with KDE3.2 then there is only 1 style "QtCurve", and
   only 1 set of GTK gtkrc files installed. Variation (V1, V2, V3, or custom)
   can then be selected via KControl.

    (NOTE: When upgrading from KDE3.1, you may wish to uninstall the
    previous QtCurve release - to remove the old V2 and V3 config files)

5. ALL: Better non-bordered gradient radio/checks.
6. ALL: Option to use non gradient progress/menu bar indicator.
7. GTK: Don't shade paned widgets!
8. ALL: V1 has flat progress bar and menuitems - i.e. no gradient effect.
9. ALL: If no borders are selected and gradient selected, then use thin border
   for progress bar and menuitems.
10. KDE: Better AA'ing of edges of round buttons.
11. GTK: Fix for background colour of selected text, thanks to David RodrÃÂ¯ÃÂ¿ÃÂ½uez GarcÃÂ¯ÃÂ¿ÃÂ½
12. ALL: Use KDE's contrast setting.
13. KDE: Implement tri-state checkmarks.
14. ALL: Better bottom/right tab gradients.
15. KDE: (Qt>=3.2) the text/icon of a selected bottom tab moves down 1 pixel.
16. KDE: (Qt>=3.2) Only highlight text/check label if mouse is over
    sensitive area.
17. KDE: Mimic GTK's scrollbar highlighting. i.e. only highlight slider if
    mouse is over slider area, and likewise for the buttons.
18. KDE: When kicker is set to transparent (may need to restart kicker),
    use "harsh" rounded buttons in round mode. (i.e. don't AA the corners)
19. KDE: Modified button size code - KDE3.2's kcalc is smaller now!
20. KDE: Don't flatten combo box arrow area when selected - more GTK like.
21. KDE: Highlight clickable listview headers on mouse over - more GTK like.
22. GTK: HScrollBar fix.
23. KDE: More Gtk like shading of buttons - i.e. always dark when pressed.
24. ALL: Slight change to look of combo-box.
25. KDE: Fix for tabwidgets in konqueror, etc.

## 0.16-gtk2
1. ALL: Use "button text" colour for default button indicator, apart from when
   in rounded mode (i.e. V3)
2. ALL: V3 - Embolden font of default button.
3. ALL: V3 - Lighter background for disabled check/radios.
4. ALL: V3 - Correct AA colour for radio indicator.
5. KDE: Draw triangular tabs the same as rounded - prevous versions defaulted
   to `KStyle`.
6. KDE: Add 10pix (2*10) border to non-default buttons.
7. KDE: Implement masks for checks and radios - helps with khtml.

## 0.15-gtk2
1. KDE: Smaller, more Gtk-like toolbar separators.
2. ALL: When drawing rounded, use background colour for corner pixels.
3. ALL: V2, use a shade of the button colour for the default indicator.
4. ALL: V3, as for V2 above, but "round" the indicator.
5. KDE: When compiled for Qt 3.2, only highlight pixmap and text on mouse over
   for radio and checkboxes - as these are the only sensitive parts. (Qt < 3.2
   the whole widget is sensitive - and this can expand past text.)
6. KDE: Fix bottom tabs - i.e. movement of text/icon. (Qt >= 3.2 !)
7. GTK: Fix bottom tabs on V1
8. ALL: More rounded buttons, etc.
9. ALL: Rounded indicator boxes in rounded mode.
10. ALL: Better radio buttons.
11. KDE: Use rounded buttons for Kicker taskbar as well - looks OK now buttons
    are more rounded.
12. GTK: Fix gradient rendering bug - seemed to affect GTK1 glade buttons.
    (When a partial button had to be re-drawn, the whole button was re-drawn
    instead - clearing out the text!)
13. GTK: Use "check" for check and radio's in menus - more Qt like.
14. ALL: V3 - Gradient radio's and checks.

## 0.14-gtk2
1. GTK2: Set slider width to 16 pixels to better match KDE.
2. GTK: Draw border around toolbar buttons 1 pixel smaller - saves overlap with
   frame.
3. GTK: Better tabs - GIMP 1.3's tab icons should now move. (GIMP's tabs are
   shaded tho, hmm...)
4. ALL: V3 style uses "pyramid" (i.e. non "V" like) arrows.
5. KDE: Fix for menubar background if button colour != background colour.
6. KDE: Fix for background of "Location:" on konqueror.
7. GTK: Better lsitview headers - smaller, more KDE like.
8. GTK: Reduce differences in V1, V2, and V3 gtkrc files to the bare minimum -
   ready for only 1 (`KControl`) configurable style.
9. GTK1: Fix (hack really) for range grooves. For some reason these were not
   being drawn when first displayed. Works ok for ranges - but scrollbar
   grooves are still messed up! The redraw only happens after you leave
   the widget!

## 0.13-gtk2
1. ALL: New V3 style - has rounded buttons, and uses gradient effect on
   buttons, tabs, scrollbars, etc.
2. ALL: Code clean-up.

## 0.12-gtk2
1. KDE: Don't have flat buttons - i.e in printmgr, and kscd.
2. KDE: Default frame width of 1.
3. KDE: Fix mouse-over for some toolbar buttons - the on/off type.
   Affects most KDE styles, but a real KDE fix should be in KDE3.2
4. GTK2: Fix bug where V2 style options were being ignored.
5. GTK2 and KDE: When highlight a depressed button, use a lighter
   shade of the depressed colour - and not the standard
   highlight colour.
6. GTK: Active tab bar text is 1 pixel higher - a la KDE.
7. KDE: Fix for borded bottom tabs.

## 0.11-gtk2
1. GTK2: Fix possible memory corruption.
2. ALL: Square off splitter - more consistent.
3. GTK2: Fix for 2.2.x combo-boxes.

## 0.10-gtk2
1. GTK2: Better match of menubar height with KDE.
2. GTK:  Fix up/down/left/right arrows.

## 0.09-gtk2
1. GTK2: Fix scrollbars for 2.2.x - scrolled 1 pixel too much.
2. GTK1: Fix lower spinbutton height. This was OK, so maybe its a Gtk
   change. But from which version? I'm at 1.2.10

## 0.08-gtk2
1. GTK: Better menu selection.

## 0.07-gtk2
1. KDE: Reduce min-size of combo-boxes.
2. GTK: Fixes for sodipodi - works with V1, not too good wrt V2...
3. GTK2: Fix for GTK2.2 font setting.

## 0.06-gtk2
1. GTK: Make combo-boxes thinner.
2. GTK: Allow ussage of GTK1 font substitution file, either
   `/etc/X11/qt_gtk_fnt2fntrc` or `~/.qt/gtk_fnt2fntrc`, and format:

        <replace from> <with>

    e.g.

        Arial=Helvetica

    This would cause GTK1 apps to use Helvetica, even if KDE/KControl
    has specified Arial. Idea (& patch) supplied by Adrian Schroeter.

## 0.05-gtk2
1. ALL: Don't draw lines on scrollbars if less than 20 pixels.
2. GTK1: Allow to specify x and y thickness's in `gtkrc` file - as
         happens in GTK2. Fixes bug with small menu entries.
3. GTK2: Remove 1 pixel border from menus - more consistent with GTK1,
         and KDE - not for V2.
4. GTK: More KDE-like menu bar entries.
5. GTK: Re-do weight ranges - would mean 48 is accecpted as "Normal"
6. ALL: Now 2 styles:

    1. QtCurve      No borders around buttons, menus, and tab bars
    2. QtCurve V2   Has borders - more like original B???/FreeCurve

7. ALL: Number of lines on toolbar handles is now 2, and 4 for scrollbars
        and sliders.
8. ALL: Dark scrollbar, slider, and progress background - window colour
9. GTK1: Fix for slider background.
10. GTK2: Fix error with overlapping check/radio highlight on frame.
11. GTK2: Fix spinbuttons.

## 0.04-gtk2
1. KDE: Draw box around checked checkable menu item pixmaps.
2. KDE: Only use small arrows on spinbuttons, and only if size is
        too small for larger.
3. KDE: Set min button size to 54 and not 70.
4. ALL: Remove progress bar border.

## 0.03-gtk2
1. KDE: Don't highlight disabled menu items - mimics GTK behaviour.
2. KDE: Progress bar background now matches GTK's.
3. GTK: Progress bar gradient is now the same as menu bar items - as
        was KDE.
4. GTK: Progress bar contents now have 1 pixel border like KDE.
5. GTK: Better spinbutton boxes.
6. KDE: More GTK like spinbuttons.

## 0.02-gtk2
1. Use `gtk-config` and `pkg-config` to determine install location for
   GTK 1 and 2 files.

## 0.01-gtk2
Initial release.
