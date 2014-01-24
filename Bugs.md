1. Firefox doesn't support transparent menu or corner radius.
   *Related to firefox assuming menus and popups are opaque.*
   *Patching firefox can fix the problem, need to figure out whether it can be*
   *monkey patched.*
2. Partially set translucent background crashes amarok (in opengl).
   (Also affects Oxygen Transparent.)
   **Doesn't crash anymore after doing prepolish, need more tests.**
3. Transparent dialog break Qt MDI window.
   **Partially fixed by drawing background ourselves.**
   *Need to figure out what Oxygen Transparent is doing, what's wrong with*
   *preview window (preview window magically looks good now).*
4. Transparent background make chromium render incorrectly, *need to check if*
   *Oxygen-gtk's solution (set rgba on a pre-widget bases) works*.
   *Setting rgba only on menus should work around the problem.*
5. Cannot make QMainWindow in Qt5 transparent because of upstream bug.
   [Qt-Bug](https://bugreports.qt-project.org/browse/QTBUG-34064)
   **WORKED AROUND**. *Need more test*
6. Menubar in qt does not have translucent background! (NOT True, the problem is
   double background drawing)
   **Fixed by setting proper composition mode**.
   *TODO: need to add a better way to just use parent background. Useful*
   *especially when background images are used.*
7. Remove most application hacks if not all. **WIP**
8. PyQt5, Musescore and (occasionally) QtCreator seg fault on exit in a QtDBus
   destructor even when QtCurve is not linked to QtDBus.
9. QtCurve preference background blur has some problem.
10. combobox list background no blur. *Fixed by adding the widget to the*
    *whitelist, need to figure out whether this is the best way.*
11. QtQuickControl is a mess... *Need improvment on Qt5 side*
12. Plasma Kickoff (in panel) scrollbar has transparent background...
    **Seems fixed with prepolishing.**
13. KPartsplugin window does not have styled background.
    *KParsPlugin messes arround with some widget properties and flags*
    *Also, different browsers seem to create plugin windows with*
    *different depth.*
