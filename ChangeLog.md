## 1.8.16-gtk2
1. fix memleak with newer versions of cairo.

## 1.8.15-gtk2
1. Fix tab activation (via mouse) when configured to drag windows from all
   empty areas and either highlight is set to 0% or coloured mousr over is set
   to none.
2. Check widget is realized before attempting to find menubar/statusbar in
   GhbCompositor class. Fixes startup crash wih Handbrake when menu/statusbar
   hiding is enabled.

## 1.8.14-KDE4
1. Fix loading of kwin decoration/config under KDE4.9

## 1.8.14-gtk2
1. Fix reading of KDE settings.
2. Fix button on editable combos with Gtk 2.24

## 1.8.13-KDE4
1. Fix KDE4.9 build - window grouping disabled.

## 1.8.13-gtk2
1. Fix builing with newer glibs - only glib.h itself can be included.
2. Use squared menus and tooltips if using XRender
3. Minimum 1ms delay for sub menus.

## 1.8.12-KDE4
1. Dont setup KDE4 fonts/colours in constructor - seems to mess things up when
   using proxy styles.
   See http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=638629

## 1.8.12-gtk2
1. Fix first shown menu not having a shadow in firefox.
2. Fix eclipse crash due to resetting of shadows.
3. Disable shadows for SWT where Gtk<2.12.

## 1.8.11-KDE4
1. Fix flashing of clementine's position slider tooltip.

## 1.8.11-gtk2
1. Fix broken titlebars in GIMP, etc.

## 1.8.10-KDE4
1. Fix some toolbutton icon clipping.
2. Korean translation - thanks to YoungUk Kim.
3. Fix kwin button corruption when compositing is disabled under KDE SC4.8
4. Minimum 1ms delay for sub menus.

## 1.8.10-gtk2
1. Fix compilation against Gtk 2.11
2. Implement shadows for KDE SC4.7

## 1.8.9-KDE4
1. Dont force alternate listview colours for `KAboutApplicationDialog`
2. Fix issues with konqueror style menu toolbar buttons and the joined
   toolbar button style.
3. Implement shadows for KDE SC4.7

## 1.8.9-gtk2
1. Disable resize grips in Ubuntu 11.04
2. Remove GimpDock font settings from gtkrc

## 1.8.8-KDE4
1. Disable window dragging from `QGraphicsView`.
2. Respect `_kde_no_window_grab` property to disable window drag for
   certain widgets.
3. Workaround dialog placement issues when using transparent dialogs.
   Attempt to place dialog in centre of its parent - if it has one.
   This only works for dialogs, windows will still (unfortunately)
   be moved to top left :-(

## 1.8.8-gtk2
1. Make timer callbacks threadsafe. Fixes
   https://bugzilla.redhat.com/show_bug.cgi?id=669155
2. Allow scrollbar sliders to be as thin as 5 pixels. At this setting, sliders
   will be squared.

## 1.8.7-KDE4
1. Fix QtDesigner crash when using transparent background - thanks to
   Friedemann Kleint.
2. When installing background images, need to remove and previous files
   first (as `QFile::copy` will not overwrite files)
3. Fix incorrent window-drag trigger when dragging dock widget title
   widgets (set via `QDockWidget::setTitleBarWidget()`)
4. Fix crash when opening style config dialog whilst kwin config dialog is
   open.
5. Allow scrollbar sliders to be as thin as 5 pixels. At this setting, sliders
   will be squared.

## 1.8.7-gtk2
1. Fix some segfaults with evolution under Fedora 14.
2. Don't force widnows style scrollbars for Firefox4 - this workaround
   is no longer required.

## 1.8.6-KDE4
1. Fix potential crash when using translucency and the raster engine.
   (Fix taken from oxygen-transparent)
2. Fix crash when changing QtCurve settings - handle free'ing of colours
   better.

## 1.8.6-gtk2
1. Fix compilation against Gtk2 older than 2.20 - yet again!
2. Fix drawing of background underneath menubars - noticeable if menubar
   gradient uses alpha.
3. Fix highlighted menu items when shading poup menus.
4. Don't use gradient alpha settings for tooltips or rulers.
5. Fix background of popup menus when set to be shaded as per menubars, was
   using inactive and not active titlebar colour!
6. When drawing popup text, where shaded as per titlebar, no need to check
   if window is active - it always is.
7. Improve window drag code.

## 1.8.5-KDE4
1. Fix fill of bottom corners of kwin decoration when not compositing.
2. Fix sunken left background of kwin decoration when not compositing.
3. Don't force toolbar icons to be square.
4. Fix drag issue with MusicScore.
5. Don't use gradient alpha settings for tooltips.
6. Don't do rounded/semi-transparent tooltips for opera.

## 1.8.5-gtk2
1. Fix compilation against Gtk2 older than 2.20
2. Remove some Gtk3 hacks.
3. Dont draw frame when shadow is set to NONE.
4. Use correct shade for highlighted menuitems when not using the highlight
   colour, and the popup is shaded.
5. If using custom menu colours, when drawing selected menuitems always draw as
   such - even if not using the highlight colour.

## 1.8.4-KDE4
1. Fix saving of custom alpha values.
2. Fix crash upon exit - due to double free.

## 1.8.4-gtk2
1. Fix issues with image based backgrounds.
2. Don't start drag on widget tab labels.
3. Don't remove mouse over colour for pressed combo buttons.
4. Use `gtk_rc_parse_string` to hide shortcuts, and not GtkSettings
5. Fix KDE style non-editable combos which have has-frame set to FALSE.

## 1.8.3-KDE4
1. Fix import of ZIP qtcurve files.
2. Fix display of background images, etc, in embedded preview.
3. Fix QMake compile - thanks, once again, to Hugues Delorme.

## 1.8.3-gtk2
1. Better fix for problems with Pidgin's tabs and window dragging.
2. Fix crash when using background gradients/images and built using 'Release'
   build type.
3. Fix titles in GIMP's preferences dialog.

## 1.8.2-KDE4
1. When hiding shortcuts, only show these if widget is enabled.
2. Don't draw titlebar button frame for menu button if this is where the
   window icon is to be. This was already the case for circular buttons,
   but not for square buttons.
3. If drawing sunken background behind titlebar buttons, need to reduce size
   of square buttons. This already happens for circular buttons.
4. Don't draw sunken background around icon based menu button, if it is the
   only button on the left/right.
5. Draw menubar and statusbar kwin toggle buttons square if using square
   titlebar buttons.
6. Draw box around checked Libre/Open Office menu checkboxes.
7. Ukrainian translation - thanks to Yuri Chornoivan
8. Slight drawing fix for square scrollviews.
9. When compiled against KDE, react to style changes.
10. Slightly clean-up code.

## 1.8.2-gtk2
1. Improve window drag code to take care of GtkPizza, and other, widgets.
2. Use Gdk window to determine offsets when drawing background gradients.
3. Use SHADOW_IN for treeviews.
4. Fix background gradients in scrollviews.
5. When drawing background gradients, use the background colour of the top-level
   widget's style.
6. Fix slight glitch with striped scrollbar sliders.
7. Hacky fix for tabs in Thunderbird main window.
8. Better detection of toolbar buttons in Thunderbird.
9. Slightly clean-up code.
10. Can only use KDE-style non-editable combo popup if also using glow focus - as
    Gtk2 modifes the focus rect.
11. Don't recolour checks/radios in listviews when row is selected.

## 1.8.1-KDE4
1. Safer handling of hidden shortcut underscore in popup menus. Only keep
   track of menu widget, not its ancestors.
2. If hiding keyboard shortcut underlines: Keep track of open popup menus,
   and only show keyboard short cut for the current one.
3. Fix corrupted menus when using a gradient that uses an alpha of less than
   100.

## 1.8.1-gtk2
1. Alter CMake linker flags to detect undefined symbols.
2. Fix compile with older Gtk2 versions.

## 1.8.0-KDE4
1. Add option to not display keyboard shortcurt underline until 'Alt' is
   pressed.
2. Add options to specify appearance of toolbar buttons.
3. Allow to use popup menu shade factor when colouring as per menubar.
4. Colour listview arrows on mouse-over.
5. Allow to force alternate colours for combo-box pop-up menus - matches Gtk.
6. Fix background rings alignment on kwin borders.
7. If highlighting scrollviews, and allow mouse-over for entries, also allow
   mouse-over for scrollviews.
8. Allow rounded list-style combo popups.

## 1.8.0-gtk2
1. Add option to not display keyboard shortcurt underline until 'Alt' is
   pressed.
2. Add options to specify appearance of toolbar buttons.
3. Allow to use popup menu shade factor when colouring as per menubar.
4. Colour listview arrows on mouse-over.
5. Fix scrollbar background gradient in flat scrollbar buttons with firefox.
6. Use windows style scrollbars for Firefox 4
7. Fix painting of alternate listview rows in SWT apps. For some reason even
   cells are painted twice, once with the correct "cell_XX" setting, and then
   with an incorrect one!
8. Fix size of focus indicator in treeviews with SWT apps.
9. Use list style pop-up for non-editable combos when not using Gtk-style
   combos - matches KDE behaviour.
10. Fix buttons in chromium's menu - mouse over colour, and text colour when
    using darker colour schemes.
11. Remove `QTC_CAIRO_1_10_HACK` cmake option. For cairo 1.10, an easier fix
    is to just ensure there is no colour stop at 1.0. Thanks to Hugo Pereira
    Da Costa for pointing this out.
12. Allow window drag in toolbars and other areas.
13. Implement hover and focus for scrolled windows.
14. Add support for KDE4 StartDragDist and StartDragTime config items to
    control when window drag starts.
15. Fix rounded border of combo popups when configured to not draw border
    and be square.
16. Use configure-event, and not resize-request, to detect window size changes.
17. Fix splitter background when highlight set to 0.
18. Fix tab mouse-over highlight when tab has a child widget.

## 1.7.2-KDE4
1. Allow editable combo-box popup to be rounded.
2. For windows build, set FramelessWindowHint when setting WA_TranslucentBackground
3. Always use flat background kwin, custom background messes up some aurorae
   decorations.
4. Update Chinese translations.
5. Set/clear widget masks when compositing toggled.
6. Fix kwin custom button icon colours.
7. Fix treeview line dimensions.

## 1.7.2-gtk2
1. Allow editable combo-box popup to be rounded.
2. Fix initial treeview highlight.
3. Fix rounded popup border when using compositing.
4. Add vmware, vmplayer, and gtk to list of app to exclude from opacity
   settings. This also prevents using RGBA for rounded tooltips and popup menus.
5. Fix opacity setting of squared popup menus.
6. Improve group-box label positioning.
7. Fix line-style group box when text is above or below the line.
8. Fix crash when using a tiled image for background appearance, but not for
   menu appearance - was using wrong config item!
9. Fix border of hovered treeview items.
10. Allow combo popupmenus to be rounded if not using compositing.
11. Set/clear widget masks when compositing toggled.
12. Fix treeview line end detection.
13. Fix treeview selection, and line, corruption.
14. Fix treeview line dimensions.
15. Fix size of fonts in GIMPs dock.
16. Fix firefox scrollbar background when using flat buttons and flat
    background.
17. Fix clipping issues with full and filled focus types.
18. For configs older than 1.7.2, disable usage of alpha channel to draw
    rounded tooltips and popup menus. Too many issues with apps to enable
    this by default.

## 1.7.1-KDE4
1. Remove kmix 'fix', and ue a proper fix! Dont access `QWidget`s' winId unless
   `w->testAttribute(Qt::WA_WState_Created) && w->internalWinId()` Thanks to
   Thomas Lübking
2. Set XProperties on show event.
3. Fix saving of 'thin' options.
4. Only disable opacity for titlebar, if using kwin configured setting and
   blending menubars (or using menubar colour).
5. Set KDE SC 4.6 'AbilityUsesBlurBehind' kwin setting if decoration or
   style supports opacity.
6. Fix 'rasied and joined' toolbar buttons.
7. Allow 'menus' of non-editable combos to be rounded.
8. Turn config dialog's 'whats this' help messages into tooltips.
9. Remove box around arrow in listview lines - to match Gtk better.
10. Remove 'new' listview lines setting - always use 'older' style.
11. Add warning about opacity settings and compiz.
12. Fix drawing of background images onto titlebar.

## 1.7.1-gtk2
1. Use rgba colormap to draw rounded tooltips and popup menus - gives nicer
   border. This may be disabled via KDE4 config dialog.
2. Allow 'menus' of non-editable combos to be rounded. (But *only* if the app
   can be set to use an rgba colormap)
3. Remove resize grips - kwin handles this.
4. Add mouse tracking to treeview lists.
5. Indent treeview selection as per KDE.
6. Dont use GtkStyle to draw treeview lines. Use custom code which matches
   previous KDE 'old' style.
7. When loading images for background/menus - check that image was loaded before
   attempting to ascertain its width/height.
8. Fix drawing or progressbar groove in treeview.

## 1.7.0-KDE4
1. Add option to use tiled pixmap as background for windows and menus.
2. Draw line focus for items that cannot use glow focus.
3. Use popup menu gradient's border setting to determine border type of menus.
4. Add option to draw standard, raised, or joined toolbar buttons.
5. Add option to use thinner frames, focus, etc.
6. If not rounding, enable all 'square' options.
7. Store background colour setting in XProperty - so that this can be used by
   QtCurve kwin decoration.
8. Add position setting for background image; top left, top middle, top right,
   bottom left, bottom middle, bottom right, left middle, right middle,
   and centred.
9. Use square tooltips by default. (Problems have been reported with Pidgin and
   rounded tooltip - but I cannot reproduce these!)
10. In preview window, draw background rings/image down from titlebar.
11. Add option to use KDE4 palette's hover colour for kwin button symbol
    mouse-over.
12. Fix slight drawing glitch when not using 'glow' for tab mouse-over.
13. RTL fixes/workarounds for table headers.
14. Workaround kcmshell terminating immediately after QtCurve config dialog is
    closed - increase `KGlobal` reference count.
15. When shading tab background, use `fillRect()` and not `fillPath()` -
    seems to workaround glithches with intel 2.12 graphics driver.
16. Fix kwin option to only apply opacity to titlebar.
17. Add option to control whether window titlebars are filled with the
    background *before* the titlebar gradient is applied.
18. Altered check/radio contents size - so as to better align cehckbox texts
    when used in left part of a form-layout.
19. Slightly better kwin button icon size calculation.
20. Replace "..." in QPrintDialog with "document-open" icon.
21. Don't overwrite user supplied `CMAKE_INSTALL_PREFIX`
22. Don't set background XProperties for kmix - seem to cause a crash on exit
    when using raster graphicssystem.
23. For OpenOffice, fill background before drawing radio buttons - seems to fix
    the 'checkbox' that seems to get painted behind some radios!

## 1.7.0-gtk2
1. Add option to use tiled pixmap as background for windows and menus.
2. Draw line focus for items that cannot use glow focus.
3. Use popup menu gradient's border setting to determine border type of menus.
4. Add option to draw standard, raised, or joined toolbar buttons.
5. Add option to use thinner frames, focus, etc.
6. If not rounding, enable all 'square' options.
7. Store background colour setting in XProperty - so that this can be used by
   QtCurve kwin decoration. (At the moment, this is just the palette background)
8. Add position setting for background image; top left, top middle, top right,
   bottom left, bottom middle, bottom right, left middle, right middle, and
   centred.
9. Use square tooltips by default. (Problems have been reported with Pidgin and
   rounded tooltip - but I cannot reproduce these!)
10. Fix combo-box menu text when shading popup menus.
11. Fix reading of QtCurve supplied default kdeglobals file.
12. Fix rounding of troughs of progressbars within treeview.
13. When using glow focus, dont draw glow focus and line focus on button of
    editable combo - jsut draw line.
14. Improve background gradient performance - only re-draw on size-request
    if size is different from last time.
15. Don't overwrite user supplied `CMAKE_INSTALL_PREFIX`
16. OpenOffice.org fixes:

    1. toolbar buttons when using max rounded buttons.
    2. vertical scrollbar button detection
    3. background of unified spin widgets and combos

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

## 1.2.0-KDE4
1. Draw etch effect on radios if using a button effect, and not using
   button-like check/radios.
2. If drawing square non-gtk style scrollviews, use a dark/light borders.
3. If drawing square scrollviews, set frame width to 1 if using thin scrollbar
   groove.
4. Add options to draw square progress and entry fields when rounding.
5. Add option to control whether progressbars have a border on all sides,
   or just top/bottom.
6. Add option to colour default buttons using highlight colour.
7. Expand selected check/radio colour option so as to be able to specify colour.
8. Add circular slider style.
9. Add glow to mouse over of slider handles (not scrollbars).
10. Add option to draw stripes on scrollbar sliders.
11. Add a 'faded' style porgressbar stripe option.
12. Add option to draw agua shine on custom gradients.
13. Allow to draw focus highlight for square scrollviews.
14. Improve appearance of button style check and radio buttons.
15. Add option to control whether scrollbar groove has a border or not.
16. Dont use pixmaps to draw rounded slider, or radio buttons.
17. Add option to increase size of check/radios - default is now 15pixels.
18. Really fix Qt only compile.
19. Increase default shadow sizes.
20. If using gtk style combos and bordering menu items, increase frame width by
    1 pixel.
21. Remove border from preview colour square in KDE colour selection dialog.
22. Only offset vertical position of etch text.
23. OpenOffice.org improvements:

    1. only draw check of checkboxes in menus
    2. draw check for radios in menus
    3. draw frames (general, line edits, scrollviews)
    4. fix pressed behaviour of toolbar buttons
    5. fix drawing of full width scrollbar
    6. reduce padding within frames
    7. dont etch entries or scrollviews
    8. fix spin buttons appearing disabled
    9. only apply menu fix to oo.o menus, not file selector menus
    10. use square entry fields

24. Set Qt toolbar font to KDE setting - but not for lineedits and comboboxes.
25. Fix text position in non-unified editable combos.
26. In settings dialog, add the ability to copy gradient settings to new
    custom gradient.
27. In settings dialog, draw borders, etc, around gradient preview.

## 1.2.0-gtk2
1. Draw etch effect on radios if using a button effect, and not using button-like
   check/radios.
2. If drawing square non-gtk style scrollviews, use a dark/light borders.
3. If drawing square scrollviews, set frame width to 1 if using thin scrollbar
   groove.
4. Add options to draw square progress and entry fields when rounding.
5. Add option to control whether progressbars have a border on all sides, or just
   top/bottom.
6. Add option to colour default buttons using highlight colour.
7. Expand selected check/radio colour option so as to be able to specify colour.
8. Add circular slider style.
9. Add glow to mouse over of slider handles (not scrollbars).
10. Add option to draw stripes on scrollbar sliders.
11. Add a 'faded' style porgressbar stripe option.
12. Add option to draw agua shine on custom gradients.
13. Improve appearance of button style check and radio buttons.
14. Add option to control whether scrollbar groove has a border or not.
15. Dont use pixmaps to draw rounded slider, or radio buttons.
16. Add option to increase size of check/radios - default is now 15pixels.
    (For Mozilla apps, the old 13pixel size will be used).
17. Fix calculation of alpha values for square rings.
18. Fix progressbar glow central position.
19. Draw more frames.
20. Fix image/gradient background issues with wxWidgets applications.
    (Use `gtk_widget_translate_coordinates` to translate widgets coordinates
     relative to window.)

## 1.1.1-KDE4
1. Alter alpha settings of square rings.
2. Fix Qt only compile.
3. Fix painting of MDI titlebar texts.
4. Fix right border of MDI windows when rounded.

## 1.1.1-gtk2
1. Alter alpha settings of square rings.
2. Fix progressbar glow sometimes disappearing with Firefox.

## 1.1.0-KDE4
1. Add option to not embolden progressbar text.
2. Add option to allow coloured mouse over for toolbar buttons.
3. Add option to border selection.
4. Add option to draw square rings on background.
5. Under KDE SC >= 4.3, use alpha blending for window borders.
6. Implement coloured shadows - code taken from Oxygen window decoration
   (but slightly modified)
7. Add support for window grouping with KDE SC >= 4.4.
8. In Arora, if using thin scrollbar grooves, fill background of scrolbars
   in `WebView` with standard background colour.
9. When drawing focus, attempt to determine if this is in a KpageListView.
10. Increase tabbar tab size so as to allow more space around icons.
11. Fix clipping issues when drawing combos with a coloured area under arrow
    (sometimes arrows would not be redrawn).
12. Use standard application font for combos and line-edits that are
    in toolbars.
13. Use `KIcon("dialog-close")` as tab-close icon.
14. When drawing frames - if frame size = window size, then dont round.
15. Add option to align tab text in the centre.
16. Dont fade tabbar base on KDevelop's tabbar that is in the menubar -
    KDevelop itself is doing fading.
17. If not using standard buttons for sidebar buttons, make KDevelop's appear
    more kate-like.

## 1.1.0-gtk2
1. Add option to not embolden progressbar text.
2. Add option to allow coloured mouse over for toolbar buttons.
3. Add option to border selection.
4. Add option to draw square rings on background.

## 1.0.2-KDE4
1. Apply cygwin patches from Yaakov S - use MODULE istead of SHARED.
2. For OpenOffice.org, if menuitem style is set to fade, then use flat.
3. Respect highlight colour setting when drawing progressbars.
4. Fix background of KOffice's dock widget titlebars.
5. Elide progressbar text if there is not enough space.
6. Replace some `dynamic_cast`'s with `qobject_cast`'s - fixes a reported
   crash in Gambatte (thanks to Andreas Pakulat)
7. Fix menuitems with Qt4.6 - these were drawn 1 pixel to thin.
8. Fix plasma calendar's week spinwidet mouse-over under KDE SC4.4
9. If using a background gradient, or cannot determine a widget's parent's
   background colour - use a 10% alpha white as the etch colour.
10. Fix cropping issues with `KIntNumInput` in `QFormLayout` in Qt4.6
11. Remove dock widget title from all `KFileWidget`'s,
    not just those in `KFileDialog`.

## 1.0.2-gtk2
1. Apply cygwin patches from Yaakov S - use MODULE istead of SHARED.
2. For older mozilla apps, if menuitem style is set to fade, then use flat.
3. Map edit-select-all.png to gtk-select-all
4. Fix check, radio, expander, and splitter background highlighting - forgot
   to convert configured value to a percentage!
5. If using a background gradient, or cannot determine a widget's parent's
   background colour - use a 10% alpha white as the etch colour.

## 1.0.1-KDE4
1. Remove `QWidget`->`int` casting that was causing compile failures for some.
2. Fix Qt only compilation due to `KColorUtils` usage.
3. Fix KTorrent's progressbars appearing as disabled.
4. For OpenOffice.org, dont use faded or bordered menuitems - as there are
   still redraw errors on the first/last item. This is all OO.o's fault, but
   these changes improve the situation a little.
5. Fix kwin compilation with KDE<4.3
6. Apply visual studio patch from Benjamin Sonnemann

## 1.0.1-gtk2
1. Fix crash when colouring selected checks and radios, shading sliders, and
   non coloured default buttons.

## 1.0.0-KDE4
1. Added option to colour selected check/radios.
2. Added option to use a smaller dot for radios.
3. Made check/radio and splitter highlight options a configurable - i.e.
   not based on standard hightlight.
4. When highlighting check/radio/splitter background, round the corners and
   use the selection gradient..
5. Added new splitter/handle/thumb style: single dot.
6. Default check/radio/splitter/expander highlight set to 3%
7. Don't do coloured mouse over for buttons of editable combos, spin buttons,
   or scrollbar buttons.
8. Don't do regular coloured mouse over of sliders when these are set to be
   coloured on mouse over.
9. Allow to specify strength when colouring selected tab.
10. Remove support for QtCurve styles (these appear as unique KDE styles in the
    main KDE style selection dialog). Re-enable support by passing
    `-DQTC_STYLE_SUPPORT=true` to cmake.
11. Slightly lighter shade used for coloured mouse over of arrows.
12. Dont draw border around rounded selection.
13. Add option to specify appearance of dock-widget titlebars.
14. Made XBar support a config, and not cmake, option.
15. Added option to specify effect for window titlebar text/icon:
    shadow/etch/none.
16. Fix window corners when 'shaded'.
17. Work-around issue with Intel 2.9 Xorg driver - doesnt seem to like drawing
    lines with an alpha channel set but anti-aliasing disabled. Affects MDI
    window icons (when 'hover icon' is enabled) - instead of using an alpha
    channel, the colour is mixed with the background colour.
18. Fix KDE4.4's palette changes to `KFilePlacesView`.
19. Fix slight redraw error with the inner shadow of sunken frames.
20. Get kwin decoration to update on style change. Config file has changed to
    qtcurve/stylerc - so need to monitor that one!
21. Remove 1 pixel background colour border from kwin decoration - was only used
    to provide softer rounded corners at the bottom.
22. Completely remove old code for coloured shadows.
23. Dont draw dark line between titlebar and contents when colouring
    titlebar only.
24. Only draw toolbar gradients for toolbars in main window. Removes gradient from
    file selector toolbar.
25. Style toolbar-extension icons (the arrows that appear when window is too
    small for toolbar).
26. Better positioning of close button on tabs.
27. When saving settings to `$XDG_CONFIG_HOME/qtcurve/stylerc`, remove the older
    `$XDG_CONFIG_HOME/qtcurvestylerc`, and `$XDG_CONFIG_HOME/qtcurve.gtk-icons`
    if they exist.
28. Setup KDE4 palette and fonts in constructor.
29. Fix K3B buttons.
30. Only draw toolbuttons as toolbar buttons if they are set to auto raise
    or have a menu.
31. Add arora and kaffeine to default list of KDE4 apps that needs manual saving
    of menubar state.
32. Fix hidding of kaffeine menu when exiting full screen.
33. Enable 'Apply' button on kwin config when change padding setting.
34. Add options to use titlebar button style, colour, font, text aligntment,
    and text effect for dock widget titlebars.
35. Add option to only round the top/left of dock widget titlebars.
36. If compiled against KDE, use configured window font for MDI windows and
    dock widget titlebars.
37. Use custum gradient 1 for dock widget titlebars.
38. Round gradients of dock-widget titlebars - top/left only be default.
39. Reduce number of supplied presets.
40. Add a 'Presets and Preview' initial page to the settings dialog - allowing to
    save custom presets, and preview before applying.
41. Nicer 'Agua' look for MDI/window/dock widget buttons.
42. Added option to hide MDI/window/dock widget button icons until mouse-over.
43. Added 'Whats This?' text to most config items.
44. Correctly theme menu separators with titles.
45. Draw titles in KMenu's the same as menu separators with titles - bold, and
    with no icon.
46. Fade ends of tab widget base - used in document mode tabs.
47. Added option to draw Air-like background rings in windows and/or menus.
    Note: to use an SVG instead of the hard-coded air-like rings, edit
    `~/.config/qtcurve/stylerc` (or `/etc/qtcurvestylerc`) and add:

          bgndImage=file
          bgndImage.file=/path/to/svg/file.svg
          bgndImage.width=120
          bgndImage.height=160

    (Use `menuBgndImage` for menus)

48. Added option to add a 'glow' to progressbars.
49. Fix QToolBox background when using background image and/or gradients.
50. Unset non-editable combobox popup menu palette when unpolishing the widget.
51. To disable background gradient, or image,  for a particular app, edit
    `~/.config/qtcurve/stylerc` and add:

        noBgndGradientApps=inkscape,gimp
        noBgndImageApps=inkscape,gimp

    ...using 'gtk' (e.g. `noBgndImageApps=gtk`) will disable for all Gtk apps.
52. Added a option to manually re-order the buttons of Gtk dialogs to match
    KDE style. Note: This is experimental, and may produce unexpected results!
53. Disable parentless-dialog fix for Kate (not required anyway), as this causes
    disappearing pointers when using embedded konsole!
54. Alter the size of a `QtoolBarExtension` if its width/height is greater than
    its parent - as occurs on Amarok's thin toolbars.
55. Add option to control whether tabs that are in toolbars are drawn as tabs, or
    as per side bar button settings.
56. Googleearth crashes when using file dialogs, if linked to KDE4. So, for
    googleearth the usage of KDE4 dialogs is removed. To remove for other apps,
    edit `~/.config/qtcurve/stylerc` and add:

        useQtFileDialogApps=googleearth-bin

57. Added 'Applications' page to config dialog where you can set which apps
    will not have background gradients, background image, should use Qt file
    dialog, need menubar state saving, not use menustripe, and should not use
    'parentless dialog' fix.
58. If drawing standard buttons for sidebar buttons, then draw these as per
    toolbar buttons.
59. Dont use highlighted text for selected sidebar buttons if drawing as per
    toolbar buttons.
60. Better Agua gradients for large buttons.
61. Fix flat flat square in bottom right of scrollviews when using external
    scrollbars and background gradient.
62. Remove titlebar from 'Places' dock widget in `KFileDialog`.
63. Fix icon position on konsole toolbuttons.
64. Add option to blend titlebar into background colour.
65. Fix appearance of some disabled icons.
66. Fix borders of large circular window buttons.
67. Improve OpenOffice.org menus a little - disable menu stripe, border to left
    and right, and if drawing borderd menu items then add gaps to the left and
    right.

## 1.0.0-gtk2
1. Added option to colour selected check/radios.
2. Added option to use a smaller dot for radios.
3. Made check/radio and splitter highlight options a configurable - i.e.
   not based on standard hightlight.
4. Added option to set expander highlight factor.
5. When highlighting check/radio/splitter/expander background, round the
   corners and use the selection gradient.
6. Default check/radio/splitter/expander highlight set to 3%
7. Added new splitter/handle/thumb style: single dot.
8. Don't do coloured mouse over for buttons of editable combos, spin buttons,
   or scrollbar buttons.
9. Don't do regular coloured mouse over of sliders when these are set to be
   coloured on mouse over.
10. Allow to specify strength when colouring selected tab.
11. Remove support for QtCurve styles (these appear as unique KDE styles in the
    main KDE style selection dialog). Re-enable support by passing
    `-DQTC_STYLE_SUPPORT=true` to cmake.
12. Slightly lighter shade used for coloured mouse over of arrows.
13. Dont draw border around rounded selection.
14. Fix internal padding of scrolled windows where scrollbars are within window.
15. When drawing frames, respect shadow state (in/out).
16. Draw Inkscape and Anjuta sidebar buttons as per QtCurve's KDE style.
17. Improve appearance of unified spin widgets under Firefox.
18. Don't draw background of dock widgets as if they were toolbars!
19. Only draw toolbar/menubar background if shadow type is not set to none.
    (Removes toolbar gradient from Rhythmbox's search buttons)
20. Reduce thickness of menu toolbar buttons (e.g. the buttons with just the arrow)
21. Better match of line-edit and spinbutton heights to KDE4.
22. If user is using menubar colouring, and this would require changing the text
    colour, then need to modify the user's userChrome.css for this - regardless
    of the `QTC_MODIFY_MOZILLA` cmake option.
23. If editing/creating `userChrome.css` and chrome folder does not exist, then
    create it.
24. Theme icon view selections.
25. Theme entry progress.
26. Supply a default kdeglobs file (installed into QtCurve's gtk-2.0 folder) so
    as to allow easy modifications of default settings.
27. Added option to draw Air-like background rings in windows and/or menus.
    Note: to use an SVG instead of the hard-coded air-like rings, edit
    `~/.config/qtcurve/stylerc` (or `/etc/qtcurvestylerc`) and add:

          bgndImage=file
          bgndImage.file=/path/to/svg/file.svg
          bgndImage.width=120
          bgndImage.height=160

    (Use menuBgndImage for menus)

28. Added option to add a 'glow' to progressbars.
29. Use listview header appearance setting for background of rulers.
30. To disable background gradient, or image, for a particular app, edit

    `~/.config/qtcurve/stylerc` and add:

        noBgndGradientApps=inkscape,gimp
        noBgndImageApps=inkscape,gimp

    ...using `gtk` (e.g. `noBgndImageApps=gtk`) will disable for all Gtk apps.

31. Removed the `-DQTC_REORDER_GTK_DIALOG_BUTTONS=true` cmake option,
    and replaced with a config option.
32. Draw Preview button in GIMP file dialog as a listview header.
33. Fix crashes with 'deadbeef' (??) 0.3.1 - need to check widget pointer before
    calling `GTK_WIDGET_???` macros.
34. Don't max round toggle buttons that are almost square, or GIMPs
    `GimpViewableButtons`
35. gtkMenuStripe option replaced by noMenuStripeApps option.
    `noMenuStripeApps=gtk` is
    the equivalent of `gtkMenuStripe=false`
36. Better Agua gradients for large buttons.

## 0.69.2-KDE4
1. Change 'Dont draw outer dark border' to 'Draw dark outer border'
2. Invalidate button background cache on settings change.
3. Fix Qt only compile - don't include kstandardirs.h!
4. Better postion of dots on slider handles.
5. If custom button colours have been defined for window buttons, use the same
   colours to colour dock widget buttons.
6. Add option to add extra padding to kwin titlebars.
7. Add vlc and smplayer to default list of KDE4 apps that needs manual
   saving of menubar state.

## 0.69.2-gtk2
1. If a scrollbar slider is maxed-out, then shade it as disabled - matches KDE.
2. When detecting `Ctrl-Alt-M` for menubar hiding, check for uppercase M
   as well as lower case. Also, ignore state of shift and caps-lock keys.
3. Set default contrast to 7

## 0.69.1-KDE4
1. Remove line beneath dock widget titles.
2. Fix glow-style mouse over for combos with coloured buttons.
3. Improve appearance of plastik style mouse-over on scrollbar sliders.
4. Fix issue with faded menuitems when not rounding.
5. Fix crash with Dolphin and square list view selection.
6. Fix Qt only compile.
7. Fix inner border of some buttons in OpenOffice.org
8. Use QtCurve's icons for dock widget buttons.
9. Use same button shape for dock widget buttons as per window buttons.
10. Don't draw shadows for window button icons if 'hover icons' is set to true.
11. Remove border around scrollbar in popup of combobox when not drawing popupmenu
    borders.
12. Use 'base' colour as background for scrollbars with thin groove, or
    flat buttons, that are part of a combo-box popup or are within
    qbstractscrollarea.

## 0.69.1-gtk2
1. Fix glow-style mouse over for combos with coloured buttons.
2. Improve appearance of plastik style mouse-over on scrollbar sliders.

## 0.69.0-KDE4
1. Add option to control whether bottom tabs should have their gradient
   shades inverted.
2. Config files now stored under `$XDG_CONFIG_HOME/qtcurve`
   (e.g. `~/.config/qtcurve`)
3. Fix some issue with 'flat' appearance.
4. Add an experimental option to hide/unhide menubars using Ctrl-Alt-M.
   Some applications, such as kcalc, do not save the menubar's state -
   and so this will be reshown when the apps starts. If this happens,
   edit `~/.config/qtcurve/stylerc` and add application name to 'menubarApps',
   e.g:

        menubarApps=app1,app2,app3

5. Update versions of XBar menu files - taken from Bespin r652
6. Disable XBar support for KDevelop - as per Bespin.
7. Classic style for QWizzard.
8. Fix issues with Amarok nightly's splitters - where the horizontal
   flag does not seem to be always set.
9. Fix size of plastik-style mouse over on sliders.
10. Fix issues with left-hand side of editable combos with
    icons when round<=full
11. Fix compile on OSX - remove unused `#include <QX11Info>`
12. Fix Arora's new style gradiented location bar.
13. Fix gap between combo button and edit field when round < max.
14. Dont move icon in editable combobox when button pressed.
15. Remove nvidia arrow work-around. (Old plmasma and nvidia releases used to
    have re-draw errors with some arrows.) To re-enable this, use
    `-DQTC_OLD_NVIDIA_ARROW_FIX=true` at cmake time.
16. Fix shaded listview headers sometime being uninitialised.

## 0.69.0-gtk2
1. Add option to control whether bottom tabs should have their gradient
   shades inverted.
2. Config files now stored under `$XDG_CONFIG_HOME/qtcurve`
   (e.g. `~/.config/qtcurve`)
3. Fix some issue with 'flat' appearance.
4. Add an experimental option to hide/unhide menubars using Ctrl-Alt-M.
5. Respect double arrow setting for non-editable combos on more Gtk2
   combo types. (Affects combos in pidgin.)
6. Colour arrows on mouse over of non-editable combos on more Gtk2
   combo types. (Affects combos in pidgin.)
7. Fix disabled editable combos.
8. It has been reported that under intel Xorg drivers > 2.8, that QtCurve's
   arrows are not appearing. This seems to be due to the fact that QtCurve is
   using cairo to draw non-antialised lines in this case. To work-around this
   issue QtCurve has been changed to use the older Gdk to draw arrows. To
   revert back to cairo, use the following cmake option:

        -DQTC_USE_CAIRO_FOR_ARROWS=true

## 0.68.1-KDE4
1. Fix reading, and saving, of 'dark inverted gradient' setting.
2. Better button metrics - especially for buttons with menus.
3. Fix different sized buttons in VirtualBox dialogs. (When determing if
   button should take icon size into account (a la Gtk2), dont use the
   'auto-default' setting, just check if its in a `QDialogButtonBox`).
4. Add 2 pixels to the left/right of max rounded buttons.
5. Dont draw window border/buttons into pixmaps when compositing is active.
6. Use `KIconLoader` to determine icon sizes - if compiled against KDE4.
7. Don't do 'fix corners of round frames where frame background is different
   to window background' (from 0.67.6) for frames within `QAbstractItemView`.
   Improves appearance of KPackageKit.
8. Workaround some issues with window style and compositing - where parts of
   the frame are not updated.
9. Speed up background gradients by drawing into a 16 * 256 size pixmap,
   scaling this to the correct height, and then tiling the pixmap.

## 0.68.1-gtk2
1. Fix reading of 'dark inverted gradient' setting.
2. Allow toggle buttons to be max rounded.
3. Fix combo-box arrow position when not etched/shadowed.
4. Fix default value for 'double Gtk combo arrow' setting.
5. Fix https://bugzilla.novell.com/show_bug.cgi?id=529607

## 0.68.0-KDE4
1. New options:

    1. Control whether icons are displayed in menus
    2. Force alternate colours in listviews
    3. Square selections in listviews
    4. Dont make 'auto-default' buttons larger - makes them less Gtk2
       like though.

2. Fix compilation under KDE 4.2

## 0.68.0-gtk2
1. New options:

    1. Control whether icons are displayed in menus
    2. Force alternate colours in listviews
    3. Square selections in listviews

## 0.67.6-KDE4
1. Fix corners of round frames where frame background is different to window
   background.
2. Only use icon metrics for auto-deault buttons that are either within a
   `QDialogButtonBox`, or are on a `KFileWidget`.
3. Give toolbar items a 1 pixel spacing - so that there is at least a gap between
   konqueror's location and search edit fields.
4. Fix saving of menubar item 'colour on mouse over' setting.
5. Fix bleed of Agua shine in scrollbar buttons.
6. Offset sunken slider thumbs to improve appearance.
7. When shading tab background - dont shade tabs of document mode tab widgets.

## 0.67.5-KDE4
1. If drawing square scrollviews, give these a similar 3d effect to
   rounded ones.
2. If using squared scrollviews, also square Kate/Kwrite view.
3. Disable parentless dialog fix for plasma.
4. Fix button of non-editable combos when not etching entries.
5. Fix clipping of plastik style mouse over for max/extra round.
6. Only do mouse over for entries if combos and spin widgets are unified.
7. If not etching entries, reduce height of editable combo by 2 pixels.
8. If not etching/shadowing, when mouse over set to 'glow' use 'thick coloured'
9. If not etching/shadowing, decrease thinner scrollbar groove.
10. Remove hard-coding of light border in progressbars - use gradient setting.
11. Use smaller kwin title bar, if not bordered or round < full.
12. Fix agua buttons when round < full.

## 0.67.5-gtk2
1. Use same shade for all menu item separators.
2. Fix clipping of non-V style right arrow.
3. Dont draw 3D border around poup menus when border has been disabled, but
   shading set to 0%
4. Offset sunken slider thumbs to improve appearance.

## 0.67.4-KDE4
1. Don't create a `KComponentData` if `KGlobal::hasMainComponent()` is true.
   Fixes plasma crash on logout.
2. Move menu text 2 pixels closer to icon - matches Gtk2 better.
3. Improve the appearance of `QWhatsThis` shadows.

## 0.67.4-gtk2
1. If drawing square scrollviews, give these a similar 3d effect to rounded ones.
2. Move menuitem arrows 2 pixels to the left, to match KDE4 better.
3. Fix button of non-editable combos when not etching entries.
4. Fix spin buttons when not etching entries.
5. Only do mouse over for entries if combos and spin widgets are unified.
6. If not etching/shadowing, when mouse over set to 'glow' use 'thick coloured'
7. If not etching/shadowing, decrease thinner scrollbar groove.
8. Fix clipping of shaded button on combos when not etching/shadowing.
9. Remove hard-coding of light border in progressbars - use gradient setting.
10. Slightly better button metrics when not etching/shadowing.

## 0.67.3-KDE4
1. Fix crash in system settings when changing colours.
2. Yet another attempt to better align button text with Gtk2.
3. Fix plastik style mouse over on scrollbars.
4. Draw arrows in popupmenus closer to edge - matches Gtk2 better.
5. Fix border width of Gtk style combobox menus.

## 0.67.3-gtk2
1. If a buttons requested size is not a multiple of 2, then shift the text 1
   pixel lower.
2. Fix plastik style mouse over on scrollbars.

## 0.67.2-KDE4
1. Fix titlebar gradients when inactive != active
2. Fix garbage on window decoration under KDE4.3 when not compositing

## 0.67.2-gtk2
1. Fix text placement in pushbuttons.
2. Set the insensitive foreground colour to the disabled text colour, and not
   the mid colour. Fixes appearance of disabled items in Mozilla apps.

## 0.67.1-KDE4
1. Adjust Agua overlay if widget is not max rounded.
2. Fix rounding of scrollbar groove.
3. Use icon metrics for all 'AutoDefault' buttons. This should now better match
   Gtk2 - where 'can default' buttons always seem to reserve enough height
   for an icon.
4. If linked against KDE, use KDE4's icon size settings to control the size of
   icons in menus, buttons, and toolbars.

## 0.67.1-gtk2
1. Adjust Agua overlay if widget is not max rounded.
2. Fix flat/sunken/dashed splitter handles!
3. Fix check/radio position in Thunderbird2 menus.

## 0.67.0-KDE4
1. Add option to draw insider border of inactive tabs.
2. Add option to draw double arrows for Gtk style combos.
3. Add option to colour the sorted listview header.
4. Disable spin button when at min/max - matches Gtk2.
5. Disable scrollbar buttons when at min/max - to match spin buttons.
6. Place non-V arrows of unified spin widgets closer together.
7. For scrollviews that are windows, dont round and always place scrollbar within
   frame.
8. Improve appearance of max rounded pushbuttons in Arora.
9. Increase the amount of rounding when round set to max.
10. Only selected tabs need to fade to 100% alpha when using gradient backgrounds.

## 0.67.0-gtk2
1. Add option to draw insider border of inactive tabs.
2. Add option to draw double arrows for non-editable combos.
3. Add option to colour the sorted listview header.
4. Disable scrollbar buttons when at min/max - to match spin buttons.
5. Place non-V arrows of unified spin widgets closer together.
6. Increase the amount of rounding when round set to max.
7. Match gradient width/height to KDE4.
8. Only selected tabs need to fade to 100% alpha when using gradient backgrounds.
9. Fix entry borders of standard entry widgets in Mozilla apps. (The Mozilla
   widgets need a thinner border than the Gtk ones)

## 0.66.1-KDE4
1. Fix some issues with the config dialog.
2. Improve text alignment in `QFormLayouts` by copying the polish code from
   Skulpture.
3. Allow scrollbars to be extra rounded!

## 0.66.0-KDE4
1. Add a new 'Dark Inverted Gradient' and use this as the default gradient
   for menu stripes.
2. New 'Agua' style gradient - looks better under KDE4 and Gtk2.
3. Add option to lighten/darken the background of tabs.
4. Make inactive tabs more consistent with active tabs.
5. Remove sunken scroll views config item, and replace with option to control
   if entries and scroll views should be etched (sunken).
6. Modify defaults:

    1. Scrollbars outside of scrollview (Gtk/oxygen style)
    2. Thin scrollbar groove.
    3. Round titlbar buttons, hover symbol.
    4. Titlebar icon next to text.

7. When using scrollbars outside of scrollview, reduce gap to 2 pixels if sunken.
8. Use `KDE_SESSION_VERSION` to determine which KDE prefix (KDE3, or KDE4) to
   seach first for `qtc_xxx.themerc` files.
9. Update Klearlooks to be more consistent with Clearlooks (lighten tab
   background, use highlight strip on active tab, dont colour active tab)
10. Cache selection pixmaps to speed up drawing.
11. Dont round rubber band, makes things more consistent with
    Gtk2 - and is faster.
12. If using squared scrollviews, also square kontact's preview pane.
13. If using scrollbars outside of scrollview, draw frame around kwrite/kate
    contents.
14. Improve config dialog - use KDE4 HIG.

## 0.66.0-gtk2
1. Add a new 'Dark Inverted Gradient' and use this as the default gradient
   for menu stripes.
2. New 'Agua' style gradient - looks better under KDE4 and Gtk2.
3. Add option to lighten/darken the background of tabs.
4. Make inactive tabs more consistent with active tabs.
5. Remove sunken scroll views config item, and replace with option to control
   if entries and scroll views should be etched (sunken).
6. Modify defaults:

    1. Scrollbars outside of scrollview (Gtk/oxygen style)
    2. Thin scrollbar groove.

7. When using scrollbars outside of scrollview, reduce gap to 2 pixels
   if sunken.
8. Set, and use, honors-transparent-bg-hint / transparent-bg-hint. Fixes corners
   of entry fields in Firefox 3.5
9. Remove `userChrome.css` settings if firefox executable name is firefox-3.5

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

## 0.46.4-gtk2
1. Only draw gradients if `width > 0 && height > 0`

## 0.46.3-gtk2
1. Fix look of flat/raised style menuitems and progressbars.
2. Use `pkg-config` to obtain Gtk2 libdir.

## 0.46.2-gtk2
1. Remove `rgb2Hls()` and `hls2Rgb()` unless using old style shading.
2. Use `fileno()` to obtain file descriptor of `FILE*` stream.
3. Fix look of slider grooves for 'flat' appearance.
4. Fix appearance of checkboxes for 'bevelled' appearance.

## 0.46.1-gtk2
1. Fix location of arrows on secondary scrollbar buttons.
2. New shading routine - works *much* better with dark colour
   schemes. This is enabled for all colours be default, to
   enable only for dark colours (i.e. where red, green,
   and blue < 96), then configure with `--enable-old-shading`
3. More 64-bit fixes - thanks to Will Stephenson.

## 0.46-gtk2
1. Allow negative highlight factors.
2. Allow usage of light borders on menuitems and progressbars,
   as well as a custom colour setting. Patch by Frederic Van Assche
3. Dont fill in slider grooves of disabled sliders. Patch by Frederic Van Assche
4. Use encrypted.png for gtk-dialog-authentication if password.png
   is not found.
5. Fix for Novell Bug 220205 - gtk-window-decorater crashes when
   right-clicking windows decoration with qtcurve-gtk2 style
   Thanks to Dirk Mueller.
6. Fix for 64-bit crashes - thanks to Will Stephenson.

## 0.45.3-gtk2
1. When determinging background of popup menu for AA'ing, use
   shade window colour, not button.
2. Fix for "-1" warnings reported by some users.

## 0.45.2-gtk2
1. Fix coloured menubars.
2. Fix firefox 2's "stack smashing detected" errors.
3. Remove ok/Cancel button swapping from QtCurve.css, does not
   work for firefox 2.x

## 0.45.1-gtk2
1. Restore pre 0.45 inactive window highlight. Option is still there to
   re-activate.
2. Use listview header settings for listview headers!

## 0.45-gtk2
1. Option to control whether highlighted items should use
   the highlight colour in inactive windows.
2. Option to control whether menubars should be shaded in
   inactive windows.
3. Fix non-bevelling of toggle buttons.

## 0.44.3-gtk2
1. Dont colour sliders when disabled/maxed-out.
2. When drawing light slider border, draw around all 4 sides.
3. Fix Firefox 2.x toolbar buttons.
4. Evolution fix - thanks to Thomas Siegmund

## 0.44.2-gtk2
1. Restore 0.43 scrollbar trough shade.

## 0.44.1-gtk2
1. Fix appearance of filled sliders.
2. Improve right-to-left support.
3. Fix OO.o crash - disable it from using scrollbar styles next and none
4. Fix missing frames when not rounded.
5. Disable toolbar handle mouse-over, not working very well.
6. Fix weird looking eclipse toolbars.

## 0.44-gtk2
1. Specifiable colours for check and radio indicators.
2. Options to control whether menu items should be rounded.
3. Options to control whether menu items should be bordered.
4. Option to enable mouse-over for menubar items.
5. Option to have thinner menu items.
6. More pronounced gradients for menuitems and progressbars.
7. Option to use large dots.
8. Option to set scrollbar button type: kde, windows, platinum,
   next, or none.
9. Fix popup menu borders when not using lighter background.
10. Fix OO.o drawing bugs.
11. Fix GIMP 2.3 notebook crash.

## 0.43.2-gtk2
1. Fix mozilla progressbars, again...

## 0.43.1-gtk2
1. Supplied pre-compiled pixmaps.

## 0.43-gtk2
1. Fix AA'ing of menubar items when colouring the menubar.
2. Option to draw light border around sliders.
3. Dialog fix is now a config option, not compile option.
4. Seperate specification of scollbar slider, and range slider, settings.
5. Option to specify menubar text colours.
6. New 'flat' appearance.
7. Dont create custom gcs per widget class, use globals instead - should
   use less resources.
8. Options to use a coloured border for mouse-over.
9. Mouse over for tabs!
10. Mouse over for toolbar handles.
11. Nicer looking check and radio buttons - using pixmaps.

## 0.42.2-gtk2
1. Slight modification to aa code.
2. Make slider appearance consistent with KDE when slider min == slider max
3. Fix firefox menu colours when C locale indicates to use a comma
   as decimal separator - patch supplied by Valentine Sinitsyn
4. Fix striped progressbar bleeding in Mozilla apps.
5. More GIMP dialog hackery if `--enable-parentless-dialogs-fix` is
   specified.
6. Treat SiftFox the same as FireFox

## 0.42.1-gtk2
1. Fix slight round when none selected.
2. Fix menu and progressbar appearance when not bevelled.
3. Fix progress always being striped.

## 0.42-gtk2
1. Optimisation to some drawing routines.
2. Removal of sunken gradients for progress and menubar items.
3. Progressbar and menuitem look now set via appearance setting.
4. Flat style check/radios match non-flat style more.
5. Remove "Border Splitters" option.
6. Removal of "Light Gradient" and "Gradient" -> replaced with just
   "Gradient" (which is the previous "Light Gradient")
7. Nicer progressbar style - options to have striped and animated.
8. Option to have dots for slider thumb.
9. Configurable splitter style - sunken lines, raised lines, dashes, dots.
10. Apply `--enable-parentless-dialogs-fix` to non-modal dialogs as well.
    These will now not get a taskbar entry.
11. Selected/normal tab appearance selectable.
12. Option to control the roundedness.
13. Option to fill in the used portion of slider groove.
14. Gradient slider and scrollbar troughs.
15. Try to make progressbar text bold as per Qt.

## 0.41.1-gtk2
1. Compile fix.

## 0.41-gtk2
1. Dont limit the max area of glass gradient.
2. Discover home folder via `getpwuid(geteuid())` before `$HOME`
3. For root, check `$XDG_CONFIG_HOME` is in ~root - if not, then
   set to `~/.config`
4. Experimental hack to fix parentless modal dialogs (i.e. kate's
   close warnings, most kaffeine dialogs). Disabled by default,
   enable with `--enable-parentless-dialogs-fix`
5. Option to have a coloured border for default button
6. Fix "leaking" progress bars in thunderbird, etc.
7. Don't allow to shade VMPlayer's menubar  - looks weird as it
   does not cover the whole usual menubar area.
8. Fix notebook crash in GIMP 2.3.x

## 0.40-gtk2
1. Better rounded tabs
2. Configurable highlight factor - 0% to 50%
3. Move scrollbar, and spinbutton, arrows down/left when pressed
4. Depress combos when active

## 0.39.1-gtk2
1. Fix reading of custom slider colour

## 0.39-gtk2
1. KDE: Really theme dock window handles - i.e. no more text. Forgot
   to actually include this in 0.37!
2. KDE: Nice dock window resize handles.
3. KDE: On konqueror's active tab, draw light line at bottom.
4. ALL: Dont round the focus rect, seems to cause problems.
5. ALL: Dont shrink focus rect for listview entries.
6. ALL: Split into KDE and Gtk packages.
7. ALL: Option to not gradient selected tab.
8. ALL: Store/read config settings from `$XDG_CONFIG_HOME/qtcurvestylerc`
9. KDE: Use a '-' for tristate checkboxes
10. GTK: Implement tristate for checkboxes and radios
11. ALL: Round tab widgets
12. ALL: Round frames
13. ALL: Rounder progress bars
14. ALL: Wider splitters
15. ALL: Remove non-bordered option.

## 0.38-gtk2
1. GTK1: Compile fix.
2. GTK2: Don't turn firefox text white when mouse over!
3. ALL: Inverted gradients option.

## 0.37-gtk2
1. KDE: Better +/- spinbuttons.
2. ALL: Better bevelled gradients.
3. ALL: Option to draw a coloured focus rectangle.
4. ALL: Modified default:

    1. Bevelled gradient
    2. Coloured focus
    3. Dotted handles/serparators
    4. Normal arrows
    5. No listview lines

5. ALL: Make glass extend to full width and height of widgets - no 3d border.
6. ALL: In listviews, draw focus rectangle within item.
7. KDE: Use dots also for general handles - more consistent.
8. KDE: Theme dock window handles - i.e. no more text.
9. KDE: Fix/hack look of MDI window buttons.
10. GTK2: Hackish fix for firefox and KDE's "apply colours to non-KDE apps"
    setting.
11. GTK: Fix for tear of menu background.

## 0.36-gtk2
1. KDE: Fix for "Search" label in ksysguard, etc.
2. KDE: Fix pixmap based menu items - e.g. Kig's colour sub-menu.
3. KDE: Use a slider width of 16 for kpresenter, it seems to assume
   this regardless of the style used :-(
4. GTK: Fix line-edits always being rounded.
5. GTK2: Fix for OO.o??

## 0.35-gtk2
1. KDE: Prevent MainActor from using its horrible colour scheme.
2. GTK2: Fix for larger "Help" buttons in GIMP.
3. KDE: Extend gradients to border when border level set to none
   for toolbars and menubars.
4. KDE: Fix for non-rounded +/- buttons on Karbon toolbar.
5. ALL: Remove "border" form default button options.
6. KDE: Fix possible crash with hover widgets.
7. GTK2: Supply a `QtCurve.css` file for FireFox 1.5.x, which will:

    1. Fix Firefox 1.5.x's button order. The xml file needed to do this is
       taken from PlastikFox - thanks to VÃ­ctor FernÃ¡ndez
    2. Disable emboldening of text on selected tab
    3. Use KDE's message/info/error/question icons in dialogs.
    4. Move button/toolbar contents when pressed.

8. GTK2: Custom `user.js` file to modify FireFox's behaviour:

    1. Remove 'instant apply'
    2. Use KDE's prefered email client

    This can be enabeld via `--enable-mozilla-userjs` `./configure` option.
    Disabled by default.

9. GTK2: Fix check marks in FireFox menus.
10. KDE: Use similar style for table headers as for listview headers.

## 0.34-gtk2
1. GTK: Fix for darkening of menubars.
2. GTK: Fix progressbar text.
3. KDE: Hack to get white selected menu item text in OO.o2. Still can't
   do coloured menubars...
4. ALL: Option to disable drawing of statusbar frames.
5. KDE: Dont use dots or dashes for general handles - only for toolbar handles.
6. GTK: Fix for not colouting flat menubars.
7. KDE: Slight improvement to tab highlighting.

## 0.33-gtk2
1. ALL: Use lighter shading for glass.
2. GTK2: Use default.kde as KDE icon folder.
3. KDE: Fix for listview lines sometims drawing over arrows.
4. KDE: Dont lighten konqueror's status bar - copied from lipstik.
5. ALL: Allow seperate specification of menubar item and progress
   bar looks.
6. ALL: New gradient style: bevelled - gradient top/bot, and plain in the
   middle. Affects listview headers, buttons, and combos.
7. ALL: Allow setting of listview appearance.
8. ALL: Allow setting of listview header colour - background, button, or
   custom.
9. ALL: Option to only round top of selected menubar items.
10. ALL: New toolbar handle style: dashes
11. ALL: Remove Gtk1 dependancy for debian `.deb` file

## 0.32-gtk2
1. ALL: "Thinner" looking non-selected tabs for glass styles.
2. GTK2: Add `./configure` argument

    1. `--disable-mozilla-mods`

        Dont alter user's userChrome.css

3. KDE: Support saving, and loading, of custom schemes.
4. KDE: Predefiend styles are now read in from .qtcurve files
5. ALL: New scheme with old glass look, flat toolbars, and normal arrows.
6. KDE: Re-design of config dialog.
7. ALL: Remove "V?" style naming.

## 0.31.1-gtk2
1. GTK2: added 2 `./configure` arguments:

    1. `--disable-gtk-icons`

        Dont do KDE-Gtk icon mapping

    2. `--enable-kde-event-filter`

        Add the event filter to intercept KDE style changes.

   (Thanks to Vaclav Slavik for the `--disable-gtk-icons` patch)

## 0.31-gtk2
1. ALL: Better "glass" gradients.
2. ALL: Dont use highlighted text colour to border selected menubar items.
3. GTL2: Remove event filter added in 0.29, this seems to cause problems
   for some people with some apps. So, colour, font, etc. changes
   wont happen in Gtk2 apps until they are restarted.
4. GTK: Dont draw dividers in listview headers, unless header is >10 pixels
5. ALL: Gradient non-selected tabs as well as selected.
6. GTK2: Also look in "hicolour" for icons. Search order will be:
   <chosen theme>, crystalsvg, hicolour
7. GTK2: Map gt-add and gtk-remove stock icons

## 0.30-gtk2
1. KDE: Shrink menu entries by 1 pixel - to align with Gtk.
2. GTK2: If switch from coloured menus to non-coloured, remove from
   `userChrome.css`
3. GTK: Fix for slight redraw error on the top of toolbar buttons.
4. GTK: Better toolbar buttons, etc.

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
