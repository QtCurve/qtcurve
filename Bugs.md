1. Firefox doesn't support transparent menu.
2. Partially set translucent background crashes amarok (in opengl).
   (Also affects Oxygen Transparent.)
   Workarounded by setting WA_TranslucentBackground on all QGLWidget
   (Need to check Qt5)
3. Transparent dialog break Qt MDI window.
   Partially fixed by drawing background ourselves.
   Need to figure out what Oxygen Transparent is doing, what's wrong with
   preview window (preview window magically looks good now).
4. Transparent background make chromium render incorrectly, *need to check if*
   *Oxygen-gtk's solution works* (set rgba on a pre-widget bases).
5. Cannot make QMainWindow in Qt5 transparent because of upstream bug.
   [Qt-Bug](https://bugreports.qt-project.org/browse/QTBUG-34064)
   **WORKED AROUND**. *Need to improve (QtCreator still doesn't work).*
6. Menubar in qt does not have translucent background!
7. Remove most application hacks if not all.
8. PyQt5, Musescore and (occasionally) QtCreator seg fault on exit in a QtDBus
   destructor even when QtCurve is not linked to QtDBus.
