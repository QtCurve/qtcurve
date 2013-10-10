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
