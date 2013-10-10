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
