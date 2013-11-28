1. Firefox doesn't support transparent menu.
2. Partially set translucent background crashes amarok (in opengl).
   (Also affects Oxygen Transparent.)
   **Doesn't crash anymore after doing prepolish, need more test.**
3. Transparent dialog break Qt MDI window.
   **Partially fixed by drawing background ourselves.**
   *Need to figure out what Oxygen Transparent is doing, what's wrong with*
   *preview window (preview window magically looks good now).*
4. Transparent background make chromium render incorrectly, *need to check if*
   *Oxygen-gtk's solution (set rgba on a pre-widget bases) works*.
5. Cannot make QMainWindow in Qt5 transparent because of upstream bug.
   [Qt-Bug](https://bugreports.qt-project.org/browse/QTBUG-34064)
   **WORKED AROUND**. *Need more test*
6. Menubar in qt does not have translucent background! (NOT True, the problem is
   double background drawing)
   **Fixed by not inherit background from parent and only draw the menu
   background**. *TODO: need to add a way to just use parent background. Useful*
   *especially when background images are used.*
7. Remove most application hacks if not all.
8. PyQt5, Musescore and (occasionally) QtCreator seg fault on exit in a QtDBus
   destructor even when QtCurve is not linked to QtDBus.
9. KScreenLocker has rendering problems.
