## 0.65.4-KDE4
1. Use 'MS Windows' and not 'Windows' for scrollbar type - to aid
   translations.
2. Apply old-style listview lines to `Q3ListView`'s as well.
3. Fix KDE4.3 compilation - patch from Markus Ullmann

## 0.65.3-KDE4
1. When not using button-like check/radio boxes, use view text as the
   indicator colour.
2. Fix slight tab glitches when using glow tab mouse over and square tabs.
3. Fix usage of custom menu text colours.
4. Fix custom menu stripe colour.
5. In config dialog - for gradient directions, use "Top to bottom" instead of
   "Horizontal" and "Left to right" instead of "Vertical"
6. Fix square tabs when using background gradients.
7. Fix clipped triangular sliders.
8. Add support for translations of config dialogs.
9. Turkish translation by Necmettin Begiter

## 0.65.3-gtk2
1. When not using button-like check/radio boxes, use view text as the
   indicator colour.
2. Fix slight tab glitches when using glow tab mouse over and square tabs.
3. Fix background of some widgets when using "left to right" background
   gradients.
4. Fix backgrounds of entry field when etching, but not rounded.
5. Dont draw frame for entry fields within list/trees.

## 0.65.2-KDE4
1. Add option to draw list view lines in pre 0.65 style.
2. Fix some inner gradient bleeding when using max rounding.
3. Fix arora location bar for non gradient backgrounds.
4. Fix 'stripe effect' on scrollbars when using plastik style mouse over.

## 0.65.2-gtk2
1. If there is not enough space for large arrows in FireFox's menuitems,
   use small ones.
2. Fix incorrect scrollbar trough border.
3. Better, non-hacky, fix for Firefox's location bar.
4. Make Firefox's entry fields smaller.

## 0.65.1-KDE4
1. Use a border width of 1 if gradienting popupmenus.
2. When using darkened menu stripe use popupmenu background colour as the
   base colour.
3. Match Gtk2 button size better.
4. Fix gradient menus when not using lighter popup menu backgrounds.
5. Slightly better looking listview lines.
6. Use `QMetaObject::invokeMethod`/`Qt::QueuedConnection` instead of `QTimer`
   to initialise KDE4 settings. Thanks to Eike Hein for the idea.
7. When exporting KDE4 palette to KDE3, call `kde-config` to ascertain KDE3's
   config dir.
8. Export activeBackground, activeForeground, inactiveBackground, and
   inactiveForeground of window manager colours  (if KDE4 home != KDE3 home)
9. Export KDE4 fonts to KDE3 (if KDE4 home != KDE3 home)
10. Fix radius of internal border when max rounding.
11. Fix some text-clipping on combos.
12. Still draw menu/toolbar gradients if using background gradient.

## 0.65.1-gtk2
1. Use a border width of 1 if gradienting popupmenus.
2. When using darkened menu stripe use popupmenu background colour as the
   base colour.
3. Draw menu stripe flush with the edge of menu.
4. Match 'darken' menuStripe to KDE4.
5. When using menu stripe, adjust separators by 20 (2 for Mozilla/Oo.o)
   pixels left.
6. Increase width of menu stripe for Mozilla apps.
7. Draw menu stripe for menus associated with dialogs.
8. Match KDE4 button text position better.
9. Fix some issues with oxygen-style combo buttons. (QtCurve creates maps from
   the parent -> entry, and parent -> button - these maps were not being cleared
   when the widget was destroyed. Also, dont keep reference to last
   moused-overed entry after its unrealized).
10. Dont highlight background of checks/radios when this is not enabled, but
    gradient background is.
11. Disable background gradients for Java, Mozilla, OpenOffice, GIMP toolbox,
    and tooltip windows.
12. Improve background gradients when drawing parts of entries, etc.
13. Very hackish fix to the border around the icons within firefox's location and
    search entry fields. Disabled by default, to enable pass
    `-DQTC_FIX_FIREFOX_LOCATION_BAR=true` to cmake. Note that enabling
    this may cause issues with other entry fields in firefox.
14. Adjust position of checks and radios in menus.
15. Fix memory leak by calling parent class's fianlize method in QtCurve's.

## 0.65.0-KDE4
1. Add options to draw editable combos, and spin widgets, as edit fields with
   embedded arrows - oxygen style.
2. Add option to draw a light border around tab widgets, as opposed to 3d look.
3. Added ability to specify scrollbar width (11 -> 31, default 15)
4. Base slider dimensions off of scrollbar width, unless using triangular slider.
5. Add option to have thinner buttons.
6. Add option to specify gradient for background of popupmenus.
7. Enable mouse-over for entry fields.
8. For mouse-over and focus, use same colour for top and bottom inner parts
   of entry fields.
9. When using filled/full focus with coloured mouse over, then mouse over takes
   precedence.
10. Alter default settings:

    1. Place arrows of spin buttons, and editable combos, in the edit field.
    2. Draw a light border around tab widget.
    3. Thinner buttons.
    4. Use soft gradient for active tab.
    5. Use glow style highlight for tab mouse-over (this introduces gaps
       between tabs).

11. Modified how button, and combo, heights are calculated - no longer set a
    minimum height.
12. Fix some K apps crashing due to `KComponentData`. Use a
    `QTimer::singleShot` to set up KDE settings.
13. Dont use KDE4's disabled background colour.
14. Disable QtCurve's coloures shadows for KDE>=4.3 - needs rewriting to support
    new kwin API.
15. Only use mouse-over colour for menu-toolbutton arrows when mouse is over menu
    segment.
16. Make Kate/Kwrite's option toolbuton have the same size as the next/prev
    buttons.
17. Make listview lines match Gtk2 better.
18. Experimental support for drawing of background gradients.
19. Cache complete buttons, etc, pixmaps - as opposed to just the gradient fill.

## 0.65.0-gtk2
1. Add options to draw editable combos, and spin widgets, as edit fields with
   embedded arrows - oxygen style.
2. Add option to draw a light border around tab widgets, as opposed to 3d look.
3. Added ability to specify scrollbar width (11 -> 31, default 15)
4. Base slider dimensions off of scrollbar width, unless using
   triangular slider.
5. Add option to have thinner buttons.
6. Add option to specify gradient for background of popupmenus.
7. Enable mouse-over for entry fields.
8. For mouse-over and focus, use same colour for top and bottom inner parts
   of entry fields.
9. When using filled/full focus with coloured mouse over, then mouse over takes
   precedence.
10. Alter default settings:

    1. Place arrows of spin buttons, and editable combos, in the edit field.
    2. Draw a light border around tab widget.
    3. Thinner buttons.
    4. Use soft gradient for active tab.
    5. Use glow style highlight for tab mouse-over (this introduces gaps
       between tabs).

11. Re-add fix for icons on GEdit tabs - but only for GeditNotebook widgets.
12. Draw entry field backgrounds.
13. Dont use KDE4's disabled background colour.
14. Experimental support for menu-stripe. (Needs menu stipe also enabled
    in KDE GUI)
15. Enable drawing of list (tree) view lines.
16. Experimental support for drawing of background gradients.

## 0.64.2-KDE4
1. When using shaded combo buttons, use button colours for border.
2. Fix compilation with Qt4 < 4.5 (`PM_SubMenuOverlap` was introduced in Qt 4.5)
3. Allow style to be compile for Qt4 only, need to pass `-DQTC_QT_ONLY=true`
   to cmake
4. Fix etched radio buttons.
5. Draw a horizontal gradient for checkable menuitems.

## 0.64.2-gtk2
1. When using shaded combo buttons, use button colours for border.
2. Disable button order userChrome.css mods for Thunderbird >= 3
3. Added a verbose warning about the usage of `QTC_MODIFY_MOZILLA`
4. Fix spin widget entry field when widget is on a notebook - bug was caused
   by fix for GEdit tabs.
5. Disable coloured slider hack for Firefox apps. This hack attempts to solve
   the miscolured top/bottom line of sliders when they are at the top/bottom.

## 0.64.1-KDE4
1. When using shaded combo buttons, use the combo splitter setting to
   determine if there should be a border between the shaded and unshaded parts.

## 0.64.1-gtk2
1. When using shaded combo buttons, use the combo splitter setting to
   determine if there should be a border between the shaded and unshaded parts.
2. Make menubar items selectable right up to the top edge of the menubar.
3. Fix clipping of close icon on GEdit tabs.

## 0.64.0-KDE4
1. Add option to not draw border around popupmenus.
2. Add option to unify spinbuttons.
3. Add option to set sub-menu delay (default 100ms)
4. Use equal top/bottom padding on popupmenu separators.
5. Add option to colour arrow part of combobox.
6. Draw resize grip as a darkend triangle.
7. Add a 'thick coloured border' mouse over type.
8. Allow to darken sliders.
9. Add new default button indicator - darken.
10. Harmonize KDE4 & Gtk2 popupmenu overlaps.
11. Slightly changed min width/height of when buttons can be max rounded.
12. Alter position of arrow in toolbar buttons with menus.
13. Fix selection artifact issues with details view in dolphin.
14. Fix setting of check/radio shade.
15. Fix storing of custom menu stripe colour.
16. Fix max/restore button being 'stuck'
17. Fix `QPainter` errors displayed when running qtconfig.
18. Fix reading of kwin button positions for MDI windows when
    custom postitions are set, but the flag to use these is not.
19. Add an option to export the KDE4 palette, and font,
    for pure-Qt3 applications.
20. Set Kontact's toolbuttons to be auto-raise.

## 0.64.0-gtk2
1. Add option to not draw border around popupmenus.
2. Add option to unify spinbuttons.
3. Add option to set sub-menu delay (default 100ms)
4. Use equal top/bottom padding on popupmenu separators.
5. Add option to colour arrow part of combobox.
6. Draw resize grip as a darkend triangle.
7. Add a 'thick coloured border' mouse over type.
8. Allow to darken sliders.
9. Add new default button indicator - darken.
10. Harmonize KDE4 & Gtk2 popupmenu overlaps.
11. Slightly changed min width/height of when buttons can be max
    rounded - fixes some issues with firefox.
12. Fix scrollbar background when using thinner grooves.
13. Under KDE4 (`KDE_SESSION_VERSION >= 4`), default hover/focus colours to the
    KDE4 defaults.
14. Fix firefox crashing when scrollbar slider move to top. This only occurs
    when not using flat scrollbar buttons, and a coloured scrollbar slider.
15. Dont draw frame around GEdits combos in its status bar - unless
    drawStatusBarFrames is set to true.
16. Dont attempt alter OK/Cancel button for Firefox 3.5 - seems to mess up
    preferences dialog.
17. Replace QtCurve's menubar mouse-over code with that from the glide engine.

## 0.63.0-KDE4
1. Add '3dfull' to gradient border options. This forces the
   dark, as well as the light, portion to be drawn.
2. Use a lighter shade for the dark portion of gradient border.
3. Flat dot for radio buttons.
4. Option to use button colour for listview headers.
5. Use KDE4 colours for focus and mouse-over.
6. Removal of background focus option.
7. Softer, and much nicer, colouration of selected tab.
8. Tab mouse over options; top (as before), below, or glow.
9. Modified defaults:

    1. Tab mouse-over highlight drawn at the bottom
    2. Plain slider style
    3. Align titlebars text in center

10. Add appearance setting for background of flat scrollbar buttons.
11. Add HCY colour space.
12. Use tint and mix colour routines from KDE4's KColourUtils.
13. Remove QtCurve's 'inactiveHighlight' option, and use KDE4s setting
    instead.
14. Add option to specify appearance setting of filler part of sliders.
15. Increase number of custom gradients - now matches number of gradient
    config items.
16. When using glow style mouse over, use mouse over colour for arrows.
17. Option to use a thinner groove for scrollbars (only when using flat,
    or no, buttons).
18. Option to only colour sliders on mouse over.
19. Option to round all tabs.
20. Add option to specify menu stripe colour.
21. Better tab highlighting.
22. Update gradient preview when change colour space.
23. Don't max/extra round progressbars.
24. Use faded lines for tab and listview header mouse-over.
25. Remove line from `KTitleWidget`
26. Slightly improve highlight in corner of kwin windows.
27. Align pressed box drwn for checkable icon menuitems better
    with menuitem highlight.
28. Remove 'Custom sunken gradient' setting, as this was not
    actually used.
29. Better handling of 'full centered' titlebar texts when window size is small.
30. Don't round KWin's tab box.
31. Improve look of widgets in KPackageKit.
32. Don't use `QTimer` to setup KDE settings.
33. Fix line under coloured MDI window titlebars.
34. Fix some KWin issues; rounding of coloured window bar borders,
    incorrect colouring of buttom of windows when only colouring titlebar.
35. When navigating popupmenus with the keyboard, skip over disabled items.
36. Add option to control light border on windows.
37. Move kwin menu icon when pressed.
38. Only draw mouse-over effect on sliders when mouse is over slider handle.
39. Titlebar button options:

    1. Show border on hover only
    2. Show icon on hover only
    3. Round buttons
    4. No border
    5. Colouration

40. Titlebar app icon options:

    1. None
    2. On menu button
    3. NExt to title

41. Fix selection appearance when using raster graphicssystem.
42. Add an option to export the KDE4 palette for KDE3 applications.
43. Tell KWin to re-load its config if the kwin style is set to QtCurve, and
    QtCurve's settings are changed.
44. Fix artifacts with very thin selections.
45. Translation support - thanks to Jonathan Riddell
46. Use focus colour for coloured window shadows.
47. Reduce toolbutton pop-up menu delay from 600ms to 250ms
48. Fix edit field in KDE4.3's kickoff menu.
49. Turn kwin coloured shadow off by default - seems to have issues with
    KDE4.3Beta and/or Qt 4.5.1
50. Dont max round toolbar buttons.
51. Fix slight drawing glitch with Qt4.5 document mode tabs.
52. Added 'Tiny' to kwin style border options.
53. Added option to control whether bottom of KWin windows are rounded.
54. Added option to show a resize grip on kwin windows - taken from
    Bespin KWin decoration.
55. Added option to not draw outer dark border on KWin windows.

## 0.63.0-gtk2
1. Add '3dfull' to gradient border options. This forces the
   dark, as well as the light, portion to be drawn.
2. Use a lighter shade for the dark portion of gradient border.
3. Flat dot for radio buttons.
4. Option to use button colour for listview headers.
5. Use KDE4 colours for focus and mouse-over.
6. Removal of background focus option.
7. Softer, and much nicer, colouration of selected tab.
8. Tab mouse over options; top (as before), below, or glow.
9. Modified defaults:

    1. Tab mouse-over highlight drawn at the bottom
    2. Plain slider style
    3. Align titlebars text in center

10. Add appearance setting for background of flat scrollbar buttons.
11. Add HCY colour space.
12. Use tint and mix colour routines from KDE4's KColourUtils.
13. Remove QtCurve's 'inactiveHighlight' option, and use KDE4s setting
    instead.
14. Add option to specify appearance setting of filler part of sliders.
15. Increase number of custom gradients - now matches number of gradient
    config items.
16. When using glow style mouse over, use mouse over colour for arrows.
17. Option to use a thinner groove for scrollbars (only when using flat,
    or no, buttons).
18. Option to only colour sliders on mouse over.
19. Option to round all tabs.
20. Better tab highlighting.
21. Don't max/extra round progressbars.
22. Use faded lines for tab and listview header mouse-over.
23. Create icon-mapping file in a more robust manner.
24. Set `gtk-icon-theme-name` to user's theme (or the KDE default if not set),
    and set `gtk-fallback-icon-theme` to `highcolour`.
    This seems to map more icons.
25. Fix spin widget entry highlighting under Gtk 2.16
26. Don't round rulers.
27. Fix faded menuitems when not rounding.
28. Fix alternate listview colours when running under KDE3
    (`KDE_SESSION_VERSION=3`)
29. Make buttons slightly thinner, matches KDE4 better.
30. Fix settnig of KDE icons when QtCurve is configured by just having
    gtk-theme-name="QtCurve" in the gtkrc file.
31. Fix painting of scrollbar slider ends when sliders are shaded, and scrollbar
    buttons are not flat.
32. When not using highlight colour for menus, get OpenOffice to use the correct
    colour for selected menubar items.
33. Fix setting of custom shades, and gradients, when reading system config file.
34. Don't crash when `/etc/qtcurvestylerc` is only readable by root!
35. Use tooltip text colour to draw its border, as per KDE4.
36. Dont draw border around rulers.
