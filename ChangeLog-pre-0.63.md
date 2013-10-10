## 0.62.9-KDE4
1. Draw correct colour for popup of combo's in toolbars.

## 0.62.8-KDE4
1. Don't round KMix's popup, or the 'unlock screen' dialog.

## 0.62.8-gtk2
1. Treat 'abrowser' as Firefox.
2. Disable tab-mouse over for all tabs that are of the type
   `GtkNoteBook.GtkFixed.GtkWindow`
   (this is mainly for Mozilla and OO.o widgets)
3. Fix OK/Cancel buttons for all Firefox3 dialogs.
4. Removed `QTC_MODIFY_MOZILLA_USER_JS` cmake option, this is now set via the
   `QTC_MODIFY_MOZILLA` option - as the KDE button order is affected by the
   instantApply setting.
5. Draw Mozilla's scrolled windows square - as it seems to assume they are.
6. When determinging lower etch colour, ignore GtkBox widgets when looking up
   parent tree.
7. Set menu and toolbar fonts.
8. Don't darken disabled splitter.

## 0.62.7-KDE4
1. Dont draw hover for disabled listviews.
2. Stop animating progressbars when they reach max value.
3. Fix 'bleeding' of Arora's progressbar.
4. Don't round Arora's location bar popup.

## 0.62.7-gtk2
1. For right-to-left progressbars, make animation go right-to-left -
   matches KDE.
2. Detect seamonkey as a Mozilla app.
3. Try to determine if a Mozilla app is new, or not, by calling <app> --version
4. Fix OO.o scrollbars when using flat scrollbar buttons.
5. Improve Mozilla and OO.o spin widgets.
6. Fix OO.o comboboxes.
7. Shrink (by 2 pixels) size of Firefox toolbars - but only if
   `-DQTC_MODIFY_MOZILLA=true` is passed to cmake. This modifies your
   `~/.mozilla/firefox/???.default/chrome/userChrome.css`
   file to include `/usr/share/themes/QtCurve/mozilla/QtCurve.css` file.
8. Fix KDE button order in FireFox 3 dialogs - but only if
   `-DQTC_MODIFY_MOZILLA=true` is passed to cmake.

## 0.62.6-KDE4
1. Increase menubar item height by 1 pixel - seems to align better with Gtk2.
2. Improve look of comboboxes in Qt4 printer properties dialog listview.
3. Provide initial support for Bespin's XBar - disabled by default. To enable,
   call cmake with `-DQTC_XBAR_SUPPORT=true`
4. Draw popup menu title text!
5. Allow ability to specify if titlebar center alignment is for the full window
   width, or just the text area.
6. Fix weird highlighting behaviour in QtWebKit text edit fields.
7. Fix position of toolbar icons in Arora.
8. Fix disabled tabs.

## 0.62.6-gtk2
1. Alter meunbar item widths slightly to make more consistent with KDE4.
2. If `QTC_NEW_MOZILLA` is not set, don't allow faded menuitems for
   thunderbird - these just don't work here. In thunderbird it is not possible
   to detect if a menuitem is on a menubar, or in a popup menu :-(
3. Make file chooser's pathbar buttons more KDE4 like. This is not 100%, as
   there are no ">" arrows. To seperate items a light gray "/" is drawn.
4. Improve entry focus highlight when round>full.
5. Don't use max round for close buttons on GEdit's tabs.

## 0.62.5-KDE4
1. Use gradient for filled slider - unless appearance is flat/raised.
2. Reduce the amount of size adjustment to menubar items when toolbar
   borders are set.
3. Fix vertical toolbar borders.
4. Slightly alter listview header size.
5. Actually include the KWin titlebar alignment code!!!
6. Adjust tab widget scroll buttons slightly.
7. Fix crash with some pure Qt4 apps.

## 0.62.5-gtk2
1. Use gradient for filled slider - unless appearance is flat/raised.
2. Slightly alter listview header size.
3. Fix white-on-white text of disabled listviews - noticable in synaptic.
4. If a toolbar is disabled, so should the handle be.

## 0.62.4-KDE4
1. When drawing filled slider groove, use fill colour for border as well.
2. Better positioning of V arrows on secondary scrollbar buttons.

## 0.62.4-gtk2
1. When drawing filled slider groove, use fill colour for border as well.
2. Better positioning of V arrows on secondary scrollbar buttons.
3. Improve appearance of GtkCombo edit field.

## 0.62.3-KDE4
1. Dont draw focus for combobox listviews.
2. When menubars have a border, adjust the menubar items accordingly.
3. Reduce menubar margins.
4. When filling the used part of a slider groove, use the slider colour
   if set, otherwise use the highlight colour.

## 0.62.3-gtk2
1. Fix setting of custom non-default icon themes.
2. When creating icon map, check for icons in non-default style first,
   and then check default.
3. Improve appearance GtkCombo poup menu (even though this is actually
   a deprecated widget!)
4. When menubars have a border, adjust the menubar items accordingly.
5. When filling the used part of a slider groove, use the slider colour
   if set, otherwise use the highlight colour.

## 0.62.2-KDE4
1. Adjust `KMenu` titles.
2. Fixed (hopefully!) clipping of toolbutton text.
3. Fixed some more tab clipping (Qt 4.5)
4. Draw `KMenuTitle` more button like - is more consistent with KDE3.

## 0.62.2-gtk2
1. Improve appearance of progressbars in listviews (such as in d4x)
2. Call `kde-config` (KDE3) / `kde4-config` (KDE4) to determine location
   of system icons for creating icon map.

## 0.62.1-KDE4
1. Use highlight colour for unselected focused view items.
2. Fix saving/reading of custom shades.
3. Reduce tab width by 4 pixels.
4. Fix clipping of toolbar text.
5. Fix group box check box clipping.

## 0.62.1-gtk2
1. Use highlight colour for unselected focused view items.
2. Fix saving/reading of custom shades.
3. Fix reading of boolean values from kdeglobals.
4. Fix some slight tab drawing glitches.
5. Stop disabled menuitems from using custom menu text colour
   if custom colours have been set.

## 0.62.0-KDE4
1. Reduce gradient code complexity - makes predefined gradients
   work in the same vein as custom gradients.
2. Add scrollbar/slider groove and 'sunken' appearance options.
3. Added new soft and harsh gradients.
4. Change defaults:

    1. Soft gradient
    2. Fade menuitems
    3. Don't use highlight for menu.
    4. Default highlight set to 3%
    5. Toolbar separators set to sunken
    6. Flat menubar appearance
    7. Button like check/radios
    8. Supply, and use, predefined custom shades
    9. Plain progress
    10. Don't highlight active tab
    11. Sunken scrollviews
    12. Sunken appearance set to soft
    13. Line focus
    14. Set custom appearances for titlebars
    15. Extra rounded (only applies to Gtk2 and KDE4 variants)

5. Allow all bar flat and raised tabs to be coloured.
6. Only save appearance settings if different from default.
7. If a gradient does not define the values for positions 0 and 100,
   then add these.
8. Better colouring of selected tab.
9. Added new focus options - full and line.
10. Add new round options - extra and max.
11. Don't etch disabled items.
12. Use text colour for focus indicator in tree/list/etc views.
13. When specifying custom gradients, add the pssibility to have no
    internal border - options are now none, light, 3d
14. Only apply plastik style mouse-over scrollbars and sliders when plastik
    is set as the mouse-over style.
15. Only draw 1 arrow on combos - event when in Gtk style.
16. Add option to specify titlebar text alignment.
17. Move Qt4.5 scrollbars closer to scrollview when they are placed on the
    outside.
18. Provide icon mapping for pure-Qt apps, as well as KDE ones, if QtCurve is
    compiled against KDE.
19. Fix shading of menuitems when slider is set to 'orig selected'
20. Map more KDE4 icons.
21. When drawing arrows (such as in KDE4 colour selector), assume the arrow is
    enabled.
22. Style KCapacityBar - draw this as if it is a progressbar.
23. Fixed Qt4.5 tabs?
24. Add extra space for arrow on toolbuttons with menus.
25. Speed up kwin deocation when not using compositing.

## 0.62.0-gtk2
1. Reduce gradient code complexity - makes predefined gradients
   work in the same vein as custom gradients.
2. Add scrollbar/slider groove and 'sunken' appearance options.
3. Added new soft and harsh gradients.
4. Change defaults:

    1. Soft gradient
    2. Fade menuitems
    3. Don't use highlight for menu.
    4. Default highlight set to 3%
    5. Toolbar separators set to sunken
    6. Flat menubar appearance
    7. Button like check/radios
    8. Supply, and use, predefined custom shades
    9. Plain progress
    10. Don't highlight active tab
    11. Sunken scrollviews
    12. Sunken appearance set to soft
    13. Line focus
    14. Extra rounded (only applies to Gtk2 and KDE4 variants)

5. Allow all bar flat and raised tabs to bol coloured.
6. Only save appearance settings if different from default.
7. If a gradient does not define the values for positions 0 and 100,
   then add these.
8. Better colouring of selected tab.
9. Added new focus options - full and line.
10. Add new round options - extra and max.
11. Don't etch disabled items.
12. Use text colour for focus indicator in tree/list/etc views.
13. When specifying custom gradients, add the pssibility to have no
    internal border - options are now none, light, 3d
14. Only apply plastik style mouse-over scrollbars and sliders when
    plastik is set as the mouse-over style.
15. Only draw 1 arrow on Gtk combos.
16. Under KDE4, read palette, and font, from kdeglobals - needed because
    if Qt4.5 is set to 'Desktop settings aware', it will not store its palette
    in `~/.config/TrollTech.conf`
17. Fix Firefox issues with scrollbar slider when using flat scrolbar buttons.
18. Call kde-config (KDE3) / kde4-config (KDE4) to determine location of
    user's kde folder.

## 0.61.5-KDE4
1. Try to prevent toolbar button text being clipped.

## 0.61.5-gtk2
1. Fix crash when drawing focus - occurs when using alt-tab in xfwm4

## 0.61.4-KDE4
1. Fix settings for sliderThumbs, handles, toolbarSeparators, and splitters.
2. Fix separators on vertical toolbars.
3. Fix chopped off text on tabs with icons, but no close button (Qt < 4.5)
4. Use faded lines for combo separator.

## 0.61.4-gtk2
1. Make squared scrollview appearance consistent when round is set to none.
2. Fix missing pixels in squred progressbars with inverted gradient.
3. Fix Firefox/OO.o menu text when useHighlightForMenu is set to false.
4. Use faded lines for combo separator.

## 0.61.3-KDE4
1. Fix compilation with Qt4.5
2. Fix tab label shift under Qt4.5

## 0.61.3-gtk2
1. Allow QtCurve's colours to be overriden if the style name
   starts with the application name. Fixes Pidgin's tab labels.

## 0.61.2-KDE4
1. Fix setting of options if no `qtcurvestylerc` is found!
2. Respect the "Raised" flag of toolbuttons - fixes missing border of
   disabled buttons in `QJackCtl`.
3. Fix text in Arora (Qt4.5) tabs.
4. Fix border of tabs.

## 0.61.2-gtk2
1. Fix setting of options if no `qtcurvestylerc` is found!
2. Allow QtCurve's colours to be overriden - but only if stylename is
   empty. Fixes SooperLooperGUI's background.
3. Fix border of tabs.
4. If `KDE_SESSION_VERSION` is not set, but `KDE_FULL_SESSION` is, then
   use KDE/Qt3 settings.

## 0.61.1-KDE4
1. Fix setting of default style. Fixes odd behaviour of config dialog!
2. Fix position of dark part of sunken lines.
3. Fix scrollbar groove when squared and using flat scrollbar buttons.
4. Fix kontact crash due to tracking mouse events on frames containing
   scrollviews.
5. Read in custom gradients and shades from any system config file.

## 0.61.1-gtk2
1. Fix setting of default style.
2. Fix position of dark part of sunken lines.
3. Read in custom gradients and shades from any system config file.

## 0.61-KDE4
1. Only draw coloured border for moused-over items if they are enabled!
2. Fix squred off corner in KTabWidgets when tabs are hidden.
3. Fix corners of sunken Q3ScrollViews.
4. Draw a sunken border around KPopupMenu titles.
5. Draw toolbar arrows after icon, so that they do not get covered by the icon.
6. Fix some weird alignment problems with KDE4 HIG.
7. Fix default margin settings - these were too small.
8. Add option to draw line after frameless groupbox title.
9. Add option to use faded lines (toolbar separators, menutitem separators,
   etc.)
10. Support `QLabel` buddy widgets, as per `KStyle`.
11. Adjust focus margins on checks/radios that don't have text.
12. Fix misdrawn gradients in kwin's buttons of moveable maximised windows.
13. 'Fix' Konqueror's menubar height.
14. In config dialog, only enable 'colour seleected tab' checkbox if tab
    appearance is gradient or inverted.
15. Fix systemsettings crash with animated progressbars.
16. Fix animation of progressbars.
17. Fix applying of KDE4 colours to non-KDE4 applications that have a
    `KComponentData`.
18. Fix slight overpainting of corners of active tab.
19. Dont darken background of dock widget titlebars, just draw a line
    underneath.
20. Dont draw menuitem separator titles.
21. If print properties dialog title is empty, then use title from print dialog.
    This stops the properites window from having the executable name as its
    title.
22. Fix clipping of pressed combo box text.
23. Make 'filled focus' fill the complete widget for buttons and combos.
24. Use button text colour for combo boxes.
25. Add 'none' to toolbar handles and splitters style.
26. Change defaults:

    1. Button effect: Shadow
    2. Mouse over: Glow
    3. Default button indicator: Glow
    4. V arrows
    5. Flat toolbars
    6. Filled focus
    7. Gradient selection
    8. Flat scrollbar buttons
    9. No combo line
    10. Sunken toolbar handles
    11. Only lighten popupmenus by 2%
    12. Flat active tab
    13. Don't shade sliders
    14. Don't darken menubars
    15. Use darkened background for progressbars

27. Fix weird mouse-over behaviour of toolbar buttons under Qt4.5
28. Enable shadow, etch, and 'glow' effects for squared and slight rounded
    appearance.
29. When using scrollviews where the scrollbar is within the frame, track mouse
    events so that we can simulate the scrollbar being pressed. This extends the
    usable width of a vertial scrollbar to the right hand side of the frame.
30. Allow to darken popup menu background.
31. Fix some issues with KRunner - especially with flat scrollbar buttons.

## 0.61-gtk2
1. Only draw coloured border for moused-over items if they are enabled!
2. Fix slight redraw errors with scrolbar slider when using flat buttons.
3. Add option to draw line after frameless groupbox title.
4. Add option to use faded lines (toolbar separators, menutitem separators,
   etc.)
5. Fix mis-painted pixels on selected tab.
6. Custom gradient fix where "," is used for decimal - thanks to
   Cedric Bellegarde
7. Default to reading KDE4 settings if `KDE_SESSION_VERSION` is not set. To
   default to KDE3 instead, call cmake with `-DQTC_DEFAULT_TO_KDE3=true`
8. Fix reading in of Qt4 tooltip colours.
9. Don't allow QtCurve's colours to be overriden.
10. Make 'filled focus' fill the complete widget for buttons and combos.
11. Use button text colour for combo boxes.
12. Add 'none' to toolbar handles and splitters style.
13. Fix reading of KDE4 toolbar style.
14. Change defaults:

    1. Button effect: Shadow
    2. Mouse over: Glow
    3. Default button indicator: Glow
    4. V arrows
    5. Flat toolbars
    6. Filled focus
    7. Gradient selection
    8. Flat scrollbar buttons
    9. No combo line
    10. Sunken toolbar handles
    11. Only lighten popupmenus by 2%
    12. Flat active tab
    13. Don't shade sliders
    14. Don't darken menubars
    15. Use darkened background for progressbars

15. Enable shadow, etch, and 'glow' effects for squared and slight rounded
    appearance.
16. Allow to darken popup menu background.

## 0.60-KDE4
1. If appearance is rounded, also round the 'rubber band' selection.
2. Dont draw light border around selection.
3. Round all view selections.
4. Fix mouse-over selections with custom gradient.
5. Fix KDE app checking.
6. New focus rect options - standard, highlight, background, filled.
7. Add the ability to specify popup menu light factor.
8. Put more space between arrows on combos if using Gtk combo menus and V arrows.
9. Fix blurry arrows in url navigator.
10. Set menu palette.
11. Add option to use darkened background colour for menuitem selection.
12. Add 'fade' to menuitem appearance.
13. Option to have flat scrollbar buttons.
14. New slider styles - plain rotated, and round rotated.
15. Modified window decorations's 'on all desktops' button.
16. Fix plain stripes on flat progressbar.
17. Fix large border in kontact.
18. Use lighter shade for titlebar fill.
19. Add option to specify appearance of titlebar buttons.
20. Add option to specify appearance of inactive titlebars.
21. Recolour X of close button - not button background.
22. Dont draw sunken button for KMenu title background - just draw Menu background.
23. Alter the way the light part of etchibng is drawn - should help with
    darker colour schemes.
24. Don't draw light etch part for widget in QAbstractItemView's, KRunner,
    or Plasma dialogs.
25. Increase size of busy progressbar.
26. Draw border around filled progress.
27. Option to have button-like checks/radios.
28. Set KDE palette for Qt applications.
29. React to KDE font and palette changes for Qt only applications.
30. Style QToolBox
31. Use base for light part of lineedit border, and background for scrollview.
32. Nicer 'V' arrows.
33. Use large arrows for toolbuttons - as per KDE3.
34. Make kwin bottom as rounded as top.
35. Remove kwin 'Coloured Border' option. Replaced with a style option to do
    the same.
36. Add 'Coloured Glow' kwin option - for KDE>=4.2
    These shadows are taken from Oxygen - but the 'highlight' colour is used
    for the active window.
37. 'Fix' for some disabled icons being dithered. Use KIconEfect to convert to
    gray and make semi-transparent.
38. Increase width of pushbuttons with menus - fixes clipped text on kppp.
39. Round internal corners of plain coloured mouse over effect.
40. Remove reading of Qt3 palette - this 'hack' only worked for pure Qt apps.

## 0.60-gtk2
1. Fix "trough-lower" and "tough-upper" style slider troughs.
2. Fix reading of listview colours under KDE4.
3. If appearance is rounded, round view selections.
4. New focus rect options - standard, highlight, background, filled.
5. Add the ability to specify popup menu light factor.
6. Put more space between arrows on combos if using V arrows.
7. Add option to use darkened background colour for menuitem selection.
8. Add 'fade' to menuitem appearance.
9. Option to have flat scrollbar buttons.
10. New slider styles - plain rotated, and round rotated.
11. Fix crash if `QTC_STYLE` is set, but is empty.
12. Alter the way the light part of etchibng is drawn - should help with
    darker colour schemes.
13. Draw border around filled progress.
14. Fix light spinbuttons on dark background.
15. Option to have button-like checks/radios.
16. Draw square border for for frames that pass no detail and no widget to
    gtkDrawShadow.
17. Use base for light part of lineedit border, and background for scrollview.
18. Default to treating Firefox as firefox > 2.0 - cmake with
    `-DQTC_OLD_MOZILLA=true` to revert.
19. Draw statusbar frames, if enabled.
20. Nicer 'V' arrows.
21. When reading KDE settings, read (if they exist):

    1. `/etc/kderc`
    2. `/etc/kde4rc` or `/etc/kde3rc`
    3. `/etc/kde4/kdeglobals` or `/etc/kde3/kdeglobals`
    4. `<KDE4 prefix>/usr/share/config/kdeglobals` or
       `<KDE3 prefix>/usr/share/config/kdeglobals`
    5. `<KDE4 prefix>/usr/share/config/system.kdeglobals` or
       `<KDE3 prefix>/usr/share/config/system.kdeglobals`
    6. `$KDEHOME/share/config/kdeglobals`

22. Round internal corners of plain coloured mouse over effect.
