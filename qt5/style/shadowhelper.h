#ifndef shadowhelper_h
#define shadowhelper_h

//////////////////////////////////////////////////////////////////////////////
// oxygenshadowhelper.h
// handle shadow pixmaps passed to window manager via X property
// -------------------
//
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include <QObject>
#include <QMap>
#include <qwindowdefs.h>

namespace QtCurve {
//! handle shadow pixmaps passed to window manager via X property
class ShadowHelper: public QObject {
    Q_OBJECT
public:
    //!@name property names
    static const char *const netWMForceShadowPropertyName;
    static const char *const netWMSkipShadowPropertyName;

    //! constructor
    ShadowHelper(QObject *parent): QObject(parent) {};

    //! destructor
    virtual ~ShadowHelper() {};

    //! register widget
    bool registerWidget(QWidget*, bool force=false);

    //! unregister widget
    void unregisterWidget(QWidget*);

    //! event filter
    virtual bool eventFilter(QObject*, QEvent*) override;

protected:
    //! unregister widget
    void objectDeleted(QObject*);

    //! true if widget is a menu
    bool isMenu(QWidget*) const;

    //! accept widget
    bool acceptWidget(QWidget*) const;

    //! install shadow X11 property on given widget
    bool installX11Shadows(QWidget*);

    //! uninstall shadow X11 property on given widget
    void uninstallX11Shadows(QWidget*) const;

private:
    //! set of registered widgets
    QMap<QWidget*, WId> _widgets;
};

}

#endif
