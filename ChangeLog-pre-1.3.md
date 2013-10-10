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
