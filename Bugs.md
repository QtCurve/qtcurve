1. Firefox doesn't support transparent menu.
2. Partially set translucent background crashes amarok (in opengl).
   (Also affects Oxygen Transparent.)
   **Doesn't crash anymore after doing prepolish, need more tests.**
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
   **Fixed by setting proper composition mode**.
   *TODO: need to add a better way to just use parent background. Useful*
   *especially when background images are used.*
7. Remove most application hacks if not all.
8. PyQt5, Musescore and (occasionally) QtCreator seg fault on exit in a QtDBus
   destructor even when QtCurve is not linked to QtDBus.
9. QtCurve preference background blur has some problem
10. combobox list background no blur.
11. QtQuickControl is a mess...
12. Plasma Kickoff (in panel) scrollbar has transparent background...
