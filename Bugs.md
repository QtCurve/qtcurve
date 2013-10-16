1. Firefox doesn't support transparent menu.
2. Partially set translucent background crashes amarok (in opengl).
   (Also affects Oxygen Transparent.)
   Workarounded by setting WA_TranslucentBackground on all QGLWidget
   (Need to check Qt5)
3. Transparent dialog break Qt MDI window.
   Partially fixed by drawing background ourselves.
   Need to figure out what Oxygen Transparent is doing, what's wrong with
   preview window and what we need to do in Qt5.
4. Transparent background make chromium render incorrectly.
5. Cannot make QMainWindow in Qt5 transparent because of upstream bug.
   [Qt-Bug](https://bugreports.qt-project.org/browse/QTBUG-34064)
6. Menubar does not have translucent background!
7. Remove most application hacks if not all.
