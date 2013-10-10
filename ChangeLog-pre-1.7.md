## 1.6.4-KDE4
1. Add Chinese translations - thanks to LiShaohui.
2. Fix toolbar/menubar borders (if being drawn) when colouring menubars as per
   titlebars.
3. Allow faded groupbox frame even when not shading background.
4. If there is no group box frame, this is set to none, or the frame is below
   the text - then don't indent the checkbox (if there is one).
5. Improve inner 3d part of line edits, combos, spinboxes, etc.
6. Fix some scrollbar arrows not appearing disabled even though they should
   be - e.g. in 'About Dolphin' dialog.
7. Support loading of png, jpeg, etc file types for background image.
8. Fix menubar/titlebar blending for Firefox's 'page source' window.

## 1.6.4-gtk2
1. Fix Claws Mail listview headers.
2. When drawing text on an option menu, look deeper into parent heirarchy to
   determine if the label is on a GtkOptionMenu.
3. Fix menubar borders when menubars set to be coloured as per titlebar.
4. Allow faded groupbox frame even when not shading background.
5. Dont draw shaded/faded background groupboxes for Mozilla, Java, or
   OpenOffice.org
6. Improve inner 3d part of line edits, combos, spinboxes, etc.

## 1.6.3-KDE4
1. Fix sunken kwin button backgrounds when using background gradients.
2. In config dialog, dont enforce bordered popup menus if rounded.
3. Apply background opacity to menubars and toolbars.
4. Fix issues with konqueror's combo-boxes and dark menubars.
5. Fix display of rekonq titlebar when blending menubar and titlebar. Rekonq
   has a menubar, but it's hidden - QtCurve detected the menubar, and informed
   the kwin style of this, cuasing the titlebar to have an incorrect gradient.
6. Disable statusbar hiding for rekonq - it does not use standard statusbars.
7. When hiding status/menubars via kwin buttons, only re-act if there has been
   more then 1/2 a second between presses.
8. Dont hightlight menu item arrows when not enabled.
9. When initialising, selecting a preset, or importing a settings file, dont
   update preview until fully loaded.
10. Use preview windows close button to detatch/attach preview.
11. Fix menubar/titlebar blending for Thunderbird's 'show message
    in new window'
12. Disable the support for 'fix parent-less dialog' option - this causes too
    many issues with applications to be worth the fuss. If you really want
    this enabled, use the `-DQTC_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT=true`
    cmake option.

## 1.6.3-gtk2
1. Apply background opacity to menubars and toolbars.
2. Always use text colours when drawing labels on a GtkOptionMenu.
3. If using background opacity, draw rings and background gradient under
   menubars and toolbars - this matches the KDE4 behaviour.
4. Disable the support for 'fix parent-less dialog' option - this causes
   too many issues with applications to be worth the fuss. If you really
   want this enabled, use the `-DQTC_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT=true`
   cmake option.

## 1.6.2-KDE4
1. Fix light border on kwin titlebars when using background opacity
   and gradients.
2. Dont add light border to kwin titlebars if not using side borders.
3. Add option to use light, dark, or shadow for outer and inner kwin borders.
4. Write tooltipAppearance setting to config file!
5. Always used flat menus for OpenOffice.org
6. Allow to specify titlebar pad in the range -5 to +10
7. Calculate font size to use for hide menu/status bar kwin buttons based
   upon button height. If too small, do not draw border.
8. Restore the minimum titlebar heights to those of the 1.5 release.
9. When reading in shadow configuration, actually set the correct items! Custom
   shadows was always defaulting to using 'focus' colour, no matter what was in
   the config file!
10. Fix issues with duplicate gradient stops when editing gradients in config
    dialog.
11. Fix menubar/titlebar blending for Thunderbird's message composer, calendar,
    and tasks windows.
12. If drawing a background image, ensure that this can also be drawn onto kwin
    border - if it has a bottom edge.
13. When showing style preview in config dialog, do not save current settings
    to a temp file to pass to the style - pass the options via a style based
    function.
14. Make it possible to detach style preview from config dialog - this way
    changes can be seen immediately.
15. If preview is embedded in config dialog, then always set the menubar size
    XProperty to 0. This prevents the kwin decoration thinking that the main
    config dialog has a menubar.
16. Use same radius for all corners of progressbar.
17. Apply menu background settings to menus of non-editable combos. Rounding is
    not applied due to clipping/shadow issues with Gtk2)
18. Allow borderless rounded popup menus.
19. Allow rounded menus when rounding is set to slight.

## 1.6.2-gtk2
1. Theme evolution's headers as headers, and not buttons.
2. Set fallback icon theme to gnome instead of hicolor. This helps with some
   missing icons in evolution.
3. If drawing a background image, ensure that this can also be drawn onto kwin
   border - if it has a bottom edge.
4. Attempt to fix light text on dark background for OpenOffice.org
5. Use same radius for all corners of progressbar.
6. Apply menu background settings to menus of non-editable combos. (Rounding is
   not applied due to clipping/shadow issues with Gtk2)
7. Allow borderless rounded popup menus.
8. Don't draw double focus on Gimp combos.
9. Allow rounded menus when rounding is set to slight.

## 1.6.1-KDE4
1. Add option to have rounded popup menus.
2. Allow rounded tooltips when not compositing.
3. Fix kwin titlebar blending for striped backgrounds.
4. Simplify faded menu item code.
5. When drawing progressbar contents and label, use same calculations
   to determine position. Fixes odd text on some of Krita's progressbar/spin
   button widgets.
6. Fix menubar-hiding kwin icon when not blending menubar/titlebar.
7. 'Fix' konsole titlebar opacity. Note: Menubar is still opaque :-(
8. Fix background gradient in titlebars when using translucent backgrounds.
9. Square tooltips for OpenOffice.org
10. Fix windows build.

## 1.6.1-gtk2
1. Add option to have rounded popup menus.
2. Allow rounded tooltips when not compositing.
3. Use a hack to re-enable shadows for rounded tooltips.
4. Don't map/unmap tooltips - not required?.
5. Fix kwin titlebar blending for striped backgrounds.
6. Use flat selection for items of editable combo - matches KDE4 better.
7. Simplify faded menu item code.
8. Don't use `gtk_style_apply_default_background` for resize widgets -
   messes up translucency!

## 1.6.0-KDE4
1. Add 'glow' focus option - this is now the default.
2. Fix potential crash in `rgbToHsv`
3. Use square-ish focus for view items if using squared selection.
4. If titlebar `opacity=100`, and window/dialog opacity does not, then use
  window/dialog opacity for titlebar.
5. Fix some compiler warnings under openSUSE 11.3
6. Refactor blur-behind code.
7. Update XBar code to r1158
8. Remove `KFileDialog` code - has been in KDE since 4.1
9. Dont apply background opacity to kaffeine.
10. Remove amarok from list off apps not to apply opacity to.
11. Fix MDI window border when using opacity.
12. Improved glow mouse-over for non-standard sidebar buttons.
13. Fix dock widget titlebar alignment when set to center on full width (not
    just on text space width).
14. Add option to use slight rounding for window decorations - set in style
    config, as option also affects MDI windows.
15. Added option to specify tooltip background appearance.
16. Added options to draw shaded, or faded, group boxes.
17. Added options to specify location of group box label; inside, outside,
    centered.
18. Fix disabled looking slider in KDE colour dialog.
19. Draw coloured mouse over for 'checked' toggle buttons.
20. Just draw bold title for KMenu titles.
21. Use Highlight colour for default button glow.
22. When KDE4's 'Inactive selection changes colour' setting is enabled,
    dont change the selection colour - use same colour, but set 50% transparent.
23. Add 4 pixel padding to dock window titlebars.
24. Default sub-menu popup delay to 225 - matches Gtk2 standard default.
25. When saving, or selecting, a preset update the kwin settings.
26. Move opacity settings into their own page - and add warning that they may
    cause applications to crash.
27. Extend background gradients into window decoration.
28. Fix faded menuitems when using menu background gradients.
29. If background gradient has a 'shine', then when used for backgrounds draw a
    radial gradient top centered.
30. Always use windows-style scrollbars for OpenOffice.org - it seems to assume
    this button layout anyway, regardless of what is painted on screen.
31. In kwin decoration, allow to use outer border when border size set to none/no
    sides.
32. In kwin decoration, if disable compositing and not drawing outer border and
    colouring only the titlebar - then add outer border until compositing is
    re-enabled.
33. Alow to specify 'none' as appearance setting for titlebars - so that
    background gradient can be used.
34. Add a button in config UI to set the required config items to enable blended
    menubars and titlebars.
35. Add option to add extra padding around border of kwin titlebar contents.
36. If using compositing then use rounded and semi-transparent tooltips.
37. Add option to force square tooltips.
38. If drawing a separator in active titlebar, then only draw 1 line (and
    at 50% opacity)

## 1.6.0-gtk2
1. Add 'glow' focus option - this is now the default. (Note, for firefox
   the label of checkboxes and radiobuttons will have a rounded focus rect
   drawn - this is because the 'glow' does not seem to work for firefox's
   usage of checks and radios).
2. Fix potential crash in rgbToHsv
3. Use square-ish focus for view items if using squared selection.
4. Store dialog/window opacity in an XProperty - so that kwin theme can use
   this as the titlebar opacity.
5. Don't hide focus rect on listview headers when mouse-overed.
6. Fix crash due to missing `gtk_widget_get_allocation` - this was only
   introduced in Gtk 2.18.0
7. Group boxes need to respect 'square frames' setting.
8. Some Java Swing fixes/workarounds:

    1. always use plain slider style
    2. dont depress sliders
    3. slightly improve tabs

9. Added option to specify tooltip background appearance.
10. Added options to draw shaded, or faded, group boxes.
11. Added options to specify location of group box label; inside, outside.
12. Dont use the 'no buttons' scrollbar setting for mozilla apps. This setting
    seems to cause odd behaviour with the horizontal scrollbar (sometimes it is
    not displayed). Use signle top/bottom left/right buttons instead.
13. Draw coloured mouse over for 'checked' toggle buttons.
14. Fix look of some sunken toolbar button in firefox.
15. Use Highlight colour for default button glow.
16. When KDE4's 'Inactive selection changes colour' setting is enabled,
    dont change the selection colour - use same colour, but set 50% transparent.
17. Add "npviewer.bin" to list of apps that are brower plugin viewers - opacity
    settings are excluded from these.
18. Exclude opacity settings from SWT apps (e.g. eclipse), totem, and sonata.
19. Default sub-menu popup delay to 225 - matches Gtk2 standard default.
    The previous default of 100 seems to cause issues with Thunar.
20. Extend background gradients into window decoration.
21. Fix faded menuitems when using menu background gradients.
22. If background gradient has a 'shine', then when used for backgrounds draw a
    radial gradient top centered.
23. Use arrows on pathbar buttons.
24. If using compositing then use rounded and semi-transparent tooltips. (Note:
    when using rounded tooltips, the kwin shadow dissapears.)
25. Add option to force square tooltips.
26. Reduce GdkGC usage - use cairo for text layout.
27. Add a hack to work-around issues with NVIDIA and cairo 1.10. This 'fix'
    invloves duplicating the 1st and last stop in a gradient - and may have
    an impact on performance. To enable this, pass
    `-DQTC_CAIRO_1_10_HACK=true` to cmake.
28. Improve tab mouse-over code.

## 1.5.2-KDE4
1. Apply opacity settings to inactive tabs.
2. Add config option of apps that should not use menu opacity.
3. Fix 'white line' appearing at the bottom of kwin titlebars when not using
   an outer border.
4. If drawing an inner frame for kwin decorations, draw after titlebar has been
   drawn.
5. Fix blurry arrows in Amarok's collection browser.
6. qmake updates - thanks to Hugues Delorme.
7. Dont call `killTimer` if timer was never registered - fixes Qt warning on
   app closure under windows. Thanks to Hugues Delorme.
8. When setting KDE SC4.5 blur-behind hint, only set rects that can actually be
   blurred - should speed things up. (Idea and code stolen from Bespin)
9. Fix non-styled background of scroll areas with no frame - such as in plasma's
   configure desktop mouse actions page.
10. Don't force alternate listview colours for KDM - looks bad!
11. Add option to use same gradients for inactive kwin shadows as those used for
    active windows.
12. Draw stripes into kwin border, if colouring the titlebar only.

## 1.5.2-gtk2
1. Apply opacity settings to inactive tabs.
2. Add config option of apps that should not use menu opacity.
3. Add inkscape to list of apps to exclude opacity settings from.
4. Exclude opacity settings from browser plugins - nspluginviewer,
   plugin-container, and chrome (instance used to load plugins).
5. Exclude opacity settings for openoffice.org - crashes otherwise!
6. Check widget's colormap before drawing transparent.
7. Support non-integer font sizes - patch from Alyssa Hung

## 1.5.1-KDE4
1. Always use active palette to draw progressbar gradient.
2. Fix style-support enabled builds
3. Fix etched combos and spinbuttons when using round widgets, but square entry
   fields.
4. Fix etch of non-unified combos.
5. Fix config module compilation when kwin headers are not installed.
6. Don't draw thumbs on circular sliders - they dont look good!
7. Fix black bottom corners on fully rounded windows when not using compositing.
8. Dont apply background opacity setting to; plasma (seems to cause an icon for
   plasma to appear in the taskbar), kwin, or screensavers (prevents these from
   being displayed).
9. Add config option of apps to exclude from background opacity setting.
   Defaults to amarok, dragon, kscreenlocker, and smplayer.
10. Don't draw transparent windows if compositing is turned off.
11. Fix missing titlebar/taskbar icons in some apps (e.g. dolphin and kcalc) when
    using background transparency - thanks to Thomas Lübking.
12. Fix Qt4.5 compilation.
13. Fix colour of arrow-style min/max kwin buttons with dark colour schemes.
14. Base alpha values, used to create sunken titlebar button background, on the
    'value' of the background colour HSV.
15. Allow usage of sunken background even when not using circular
    titlebar buttons.
16. Fix qmake compilation - thanks to Hugues Delorme
17. Fix kwin painting of some shaded windows (e.g. YaST) when using unified
    titlebar and menubar
18. Fix clipping of etched/rounded editable combo frames that have icons.

## 1.5.1-gtk2
1. Fix drawing of square frames, when general setting set to round.
2. Dont draw thumbs on circular sliders - they dont look good!
3. Add options to set opacity for window and poup-menu backgrounds. The KDE SC4.5
   blur-behind hint is set when the `opacity != 100`
4. If hiding via kwin buttons, emit menubar & statusbar details when a window is
   mapped.

## 1.5.0-KDE4
1. Add option to set progressbar colour.
2. Add options to have squared sliders and/or scrollbar sliders even when
   general setting set to round.
3. Add option to use menubar colours for popup menus.
4. Add option to set alpha values used to draw etch/shadow effect.
5. Fixed background of Gtk2 style combo menus.
6. Add option to specify colour of titlebar button icons.
7. Add option to use menubar colour for titlebar when a window has a
   menubar visible. (Only valid when not blending titlebars and not using
   window border colours for menubars)
8. 'Fix' toolbar handles in OpenOffice.org
9. Add options to set opacity for window and poup-menu backgrounds. The
   KDE SC4.5 blur-behind hint is set when the opacity!=100
10. Don't alter agua 'shine' for mouse-overed disabled widgets.
11. Fix KDE SC4.5 sidebar buttons when set to not draw as standard buttons.
12. Display kwin options in style config dialog, and import/export these when
    importing/exporting QtCurve themes.
13. If drawing a spin widget with no edit field (as happens in KOffice's
    progress/spin widget), then dont draw the background behind the spin arrows.
14. Fix right margin of tab labels.
15. Fix scrollview frame when squared and etched.
16. Fix `QDial` 'handle' position.
17. Replace custom window-drag code with that of Oxygen.
18. Change window-drag options to allow dragging via toolbars and other
    empty areas.
19. Add option to draw separator between titlebar and window contents.
20. Make combo-boxes slightly thinner.
21. Update kwin shadow code.
22. Draw faded lines for vertical table header splitters.
23. Draw header gradients in top/left corner of table headers.

## 1.5.0-gtk2
1. Add option to set progressbar colour.
2. Add options to have squared sliders and/or scrollbar sliders even when
   general setting set to round.
3. Add option to use menubar colours for popup menus.
4. Add option to set alpha values used to draw etch/shadow effect.
5. Dont draw double shadow for coloured combo buttons.

## 1.4.3-KDE4
1. Fix crash (mainly on Arch linux) when apps (e.g. dolphin) exit.
2. Add VirtualBox to list of apps that need menubar state to be saved.
3. Dont force alternate colours for combo-box pop-up menus.
4. Fix left-hand sunken button background in kwin theme when drawing a dark
   outer border.

## 1.4.2-KDE4
1. Remove some warnings when building on windows.
2. Provide a qmake project to aid building on windows -
   thanks to Hugues Delorme
3. When compiled as Qt-only, embed oxygen PNGs for message box icons.
4. Add option to hide titlebar buttons on inactive windows.

## 1.4.1-KDE4
1. Fix compilation when `-DQTC_STYLE_SUPPORT=true`
2. Re-add support for keep above/below kwin buttons.
3. Fix compilation with KDE SC4.3
4. Add option to draw dark inner border in kwin decoration.
5. Fix garbled window decoration when not compositing.
6. For MDI window titles, if left-aligning ensure text is at least 6 pixels
   from edge.
7. Fix(?) compilation on non-X11 platforms - only link to, and use,
   `QtDBus` on unix.

## 1.4.1-gtk2
1. When drawing button-like checks and radios, do not drawn sunken if they are
   in a list/tree view.
2. Fix segfault on some systems due to `gdk_x11_get_default_xdisplay` usage.

## 1.4.0-KDE4
1. Add striped window and menu background options.
2. Added options to not square frames and tab frames.
3. Fix 'bleed' of progress stripes when using animated faded stripes.
4. Blend menubar and window titlebar gradients if:

    1. menubar, titlebar, and inactive titlebar gradients match AND
    2. not blending titlebar gradient AND
    3. using window titlebar colour for menubar AND
    4. extending window drag into menubar

5. Default titlebar colours to window colours - matches KDE4 default.
6. If using window border colours as menubar colours, and the active and inactive
   titlebar colours are the same, then always use the active titlebar text colour
   for the menubar.
7. When shading menubars only when active, or using window border colours as
   menubar colours, the kwin deocration will send an X11 event to the client
   window when the active window changes - this allws Gtk2 to keep track.
8. Don't etch window button icons unless not drawing the frame - doesn't look
   good otherwise.
9. Re-organised window decoration config items and module.
10. Get QtCreator to use QtCurve style for menubar, and for the toolbar
    in `KFileDialog` (`KFileDialog` is not 100%, but still looks better...)
11. Add sunken background to window titlebar button options.
12. Titlebar button modifications:

    1. Dont support keep above/below buttons
    2. Change shade/unshade to be an arrow with line above/below
    3. Add option to use arrows for the icons of min and max buttons.
    4. Better icon for 'on all desktops'

13. Always use a 1 pixel border for toolbars - as using a border of 0 can cause
    items of adjoining locked toolbars to be too close.
14. Add option to provide titlebar buttons to hide menubar and statusbars.
15. When hiding statusbar, hide *all* statusbars in a window (e.g. kate).
16. If window is a `KXmlGuiWindow`, and it contains the standard hide menubar
    and hide statatus bar actions, then use these to hide/show the menu/status
    bars. (Only when compiled against KDE)
17. Add translucency support to kwin theme.

## 1.4.0-gtk2
1. Add striped window and menu background options.
2. Added options to not square frames and tab frames.
3. Animate faded progressbars when animation enabled.
4. Blend menubar and window titlebar gradients if:

    1. menubar, titlebar, and inactive titlebar gradients match AND
    2. not blending titlebar gradient AND
    3. using window titlebar colour for menubar AND
    4. extending window drag into menubar

5. Default titlebar colours to window colours - matches KDE4 default.
6. If using window border colours as menubar colours, and the active and
   inactive titlebar colours are the same, then always use the active titlebar
   text colour for the menubar.
7. When shading menubars only when active, or using window border colours as
   menubar colours, track the active window using a X11 event sent from the
   QtCurve kwin decoration.
8. Fix border of radio buttons when general round setting is set to square.
9. Always use a 1 pixel border for toolbars - as using a border of 0 can cause
   items of adjoining locked toolbars to be too close.
10. Add separator to messafge dialog boxes - matches KDE.
11. Add option to act upon titlebar buttons to hide menubar and statusbars.
12. Fix plain/round sliders being rotated when using a custom slider width.

## 1.3.1-KDE4
1. Fix dotted focus rectangle.

## 1.3.0-KDE4
1. Add option to use kwin titlebar colours as menubar shade.
2. Add option to hide status bars via Ctrl-Alt-S
3. Add option to drag windows by menubar.
4. Set decoration (hover/focus) colours for preview.
5. Fix unreadable text when using progressbar glow and anti-aliased fonts.
6. Better event filter code for qwidget buddy focus painting.
7. Fix issues with light border in window decoration.
8. Fix blurred left edges of windows when not drawing border and rounding
   bottom.
9. Fix custom shadows when window is fully rounded.
10. If using squared entry fields, draw frames around lables square.
11. Derive from `QCommonStyle`, and not `QWindowsStyle`.
    (Deriving from `QWindowsStyle` seems to cause a crash in konqueror)
12. Remove linkage to libkfile
13. Set toolbar frame width to 0 if not drawing borders.
14. Add option to remove all borders from maximised windows.
15. Fix rounding top of menubar items in OO.o if this option is enabled.
16. If using highlight as menu item colour, then for oo.o if text r/g/b<50
    and highlightedText r/g/b<127 - then blend highlight colour with menu
    colour.
17. Fix konqueror crash due to highlighting of Q3ScrollViews.
18. Add option to turn of kwin window grouping.
19. Add kwin option to draw bottom border on windows when border size set to tiny.
20. Only show kwin resize grip if not drawng the bottom border - otherwise it just
    looks odd.
21. Use 'kde' to disable parent-less dialog fix for all KDE applications.
22. Fix shading of menubars and when using flat menubar appearance and background
    image/gradient.
23. Don't do mouse over of disalbed sliders.
24. When casting `QPainter` device, check the `QPaintDevice devType()`
    against `QInternal` - and use `static_cast`, and not `dynamic_cast`.
    (Fixes crash with tagainijisho)
25. Theme `QDial`
26. Fix some unitialised variables - caught via valgrind.

## 1.3.0-gtk2
1. Add option to use kwin titlebar colours as menubar shade.
2. Add option to hide status bars via `Ctrl-Alt-S`
3. Add option to drag windows by menubar.
4. Better check/radio positioning.
5. Fix background painting with some toolbars.
6. Draw window frames square.
7. Fix rounding of check/radio/splitter/expander highlight.
8. Set toolbar frame width to 0 if not drawing borders.
9. When toggling menubar, or statusbar, queue a redraw of the window.
10. Decrease size of SWT toolbars.
11. Disable background image and gradient for SWT apps.
12. Only allow QtCurve's colours to be overridden if they come from
    `QtCurveRcStyle` or GtkRcStyle. Also, in the case of root, disallow
    'ToolTip' and 'default' styles - as if app is run via kdesu/kdesudo,
    these could potentially be the user's colours. This should stop other
    theme's / user's colours from interfering.
13. If using KDE4/Qt4 colour settings, only force the colours for buttons
    that are disabled. This fixes the coloured text on GWave's buttons.
14. Fix shading of menubars and when using flat menubar appearance and
    background image/gradient.
15. Fix slider groove not always being painted when using background
    gradient/rings.
16. Fix drawing glitches with unified comboentries on mouse over.
17. Improve firefox menubar text colouring.
