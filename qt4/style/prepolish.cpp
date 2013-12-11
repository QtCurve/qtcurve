/*****************************************************************************
 *   Copyright 2013 - 2013 Yichao Yu <yyc1992@gmail.com>                     *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU Lesser General Public License as          *
 *   published by the Free Software Foundation; either version 2.1 of the    *
 *   License, or (at your option) version 3, or any later version accepted   *
 *   by the membership of KDE e.V. (or its successor approved by the         *
 *   membership of KDE e.V.), which shall act as a proxy defined in          *
 *   Section 6 of version 3 of the license.                                  *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 *   Lesser General Public License for more details.                         *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with this library. If not,                                *
 *   see <http://www.gnu.org/licenses/>.                                     *
 *****************************************************************************/

#include "config.h"
#include <qtcurve-utils/qtprops.h>

#include "qtcurve_p.h"

#include <QMainWindow>
#include <QDialog>
#include <QX11Info>
#include <QApplication>
#include <QDesktopWidget>

// Copied from qt_x11_p.h.
// This is not part of the public API but should be stable enough to use
// because it had never changed since the first git commit of Qt.
struct QX11InfoData {
    uint ref;
    int screen;
    int dpiX;
    int dpiY;
    int depth;
    int cells;
    unsigned long colormap;
    void *visual;
    bool defaultColormap;
    bool defaultVisual;
    int subpixel;
};

namespace QtCurve {

// Access protected functions.
struct QtcX11Info: public QX11Info {
    static bool creatingDummy;
    static QtcX11Info *getInfo(const QWidget *w);
    QWidget *getRgbaDummy();
    QWidget *getRgbDummy();
    void fixVisual();
    void setRgba(bool rgba=true);
};
bool QtcX11Info::creatingDummy = false;

inline QtcX11Info*
QtcX11Info::getInfo(const QWidget *w)
{
    return static_cast<QtcX11Info*>(const_cast<QX11Info*>(&w->x11Info()));
}

// Qt uses XCreateSimpleWindow when defaultVisual and defaultColormap
// are true. This confuses QGLWidget when recreating window caused by
// reparenting to a widget with different depth, result in a mismatch
// in x11info and native window.
inline void
QtcX11Info::fixVisual()
{
    if (qtcUnlikely(!x11data))
        setX11Data(getX11Data(true));
    x11data->defaultVisual = false;
    x11data->defaultColormap = false;
}

inline QWidget*
QtcX11Info::getRgbaDummy()
{
    static QWidget **rgbaDummies = NULL;
    creatingDummy = true;
    QDesktopWidget *desktop = qApp->desktop();
    int screenCount = desktop->screenCount();
    int screen = this->screen();
    if (qtcUnlikely(!rgbaDummies))
        rgbaDummies = (QWidget**)calloc(screenCount, sizeof(QWidget*));
    if (qtcUnlikely(!rgbaDummies[screen])) {
        rgbaDummies[screen] = new QWidget(desktop->screen(screen));
        rgbaDummies[screen]->setAttribute(Qt::WA_TranslucentBackground);
        rgbaDummies[screen]->setAttribute(Qt::WA_WState_Polished);
        rgbaDummies[screen]->winId();
    }
    creatingDummy = false;
    return rgbaDummies[screen];
}

inline QWidget*
QtcX11Info::getRgbDummy()
{
    static QWidget **rgbDummies = NULL;
    creatingDummy = true;
    QDesktopWidget *desktop = qApp->desktop();
    int screenCount = desktop->screenCount();
    int screen = this->screen();
    if (qtcUnlikely(!rgbDummies))
        rgbDummies = (QWidget**)calloc(screenCount, sizeof(QWidget*));
    if (qtcUnlikely(!rgbDummies[screen])) {
        rgbDummies[screen] = new QWidget(desktop->screen(screen));
        rgbDummies[screen]->setAttribute(Qt::WA_WState_Polished);
        rgbDummies[screen]->winId();
    }
    creatingDummy = false;
    return rgbDummies[screen];
}

inline void
QtcX11Info::setRgba(bool rgba)
{
    setX11Data(getInfo(rgba ? getRgbaDummy() : getRgbDummy())->x11data);
}

__attribute__((hot)) void
Style::prePolish(QWidget *widget) const
{
    if (QtcX11Info::creatingDummy || !widget || theThemedApp == APP_KWIN) {
        return;
    }

    QtcX11Info *x11Info = QtcX11Info::getInfo(widget);
    QtcWidgetProps props(widget);
    // Don't use XCreateSimpleWindow
    x11Info->fixVisual();
    // HACK:
    // Modify X11Info of toplevel widgets before they create native windows.
    // This way we won't interfere with widgets that set this property
    // themselves, e.g. plasma, kscreenlock.
    // We do this on windows that are typically shown right after being
    // created before entering the main loop and therefore do not have a
    // chance to be polished before creating window id. In this way, we can
    // avoid recreating native window which breaks a lot of applications.
    // This way should work for all applicaitons except when the applicaiton
    // relies on a native RGB window since the children of a RGBA window in
    // Qt are usually also RGBA. The only example of such application I have
    // found so far is kaffeine. See workaround bellow. (NOTE: gl widget works
    // because it is treated differently in Qt) (NOTE2: gl widget will not work
    // straightforwardly when reparenting to a 32bit window due to a bug in
    // Qt4, which causes a 24bit x11info and 32bit gl window to be created
    // The x11Info->fixVisual() above should work around it, too lazy to
    // report upstream..... :-P).

    // TODO:
    //     use all informations to check if a widget should be transparent.
    //     Maybe we can also do sth to their parents' and/or children as well
    if (!widget->testAttribute(Qt::WA_WState_Polished) &&
        !(widget->windowFlags() & Qt::MSWindowsOwnDC) &&
        !qtcGetWid(widget) && !props->prePolished) {
        // Skip MSWindowsOwnDC since it is set for QGLWidget and not likely to
        // be used in other cases.

        // Fix for kaffeine. Kaffeine needs a RGB window for the XV extension.
        // Setting parent to NULL forces a native RGB window to be created
        // for MediaWidget so that its children will also have RGB visual.
        // Kaffeine later sets the parent again (when adding the to layout)
        // after the native RGB children has already been created and in this
        // case, Qt does not create recreate the children window. This seems to
        // be the only way in Qt4 to have a RGB non-OpenGL window in a RGBA
        // window.
        // TODO: maybe we can hack around parent x11info
        if (opts.bgndOpacity != 100 && widget->inherits("MediaWidget")) {
            widget->setAttribute(Qt::WA_DontCreateNativeAncestors);
            widget->setAttribute(Qt::WA_NativeWindow);
            if (!qtcGetWid(widget)) {
                props->prePolished = true;
                // Kaffeine set parent back after children window has been
                // created.
                widget->setParent(NULL);
                widget->createWinId();
            }
            return;
        }
        // the result of qobject_cast may change if we are called in
        // constructor (which is usually the case we want here) so we only
        // set the prePolished property if we have done something.
        if ((opts.bgndOpacity != 100 && (qtcIsWindow(widget) ||
                                         qtcIsToolTip(widget))) ||
            (opts.dlgOpacity != 100 && qtcIsDialog(widget))) {
            props->prePolished = true;
            x11Info->setRgba();
            // Set this for better efficiency for now
            widget->setAutoFillBackground(false);
        }
    }
}

}
