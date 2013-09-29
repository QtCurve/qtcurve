//////////////////////////////////////////////////////////////////////////////
// oxygenshadowhelper.h
// handle shadow _pixmaps passed to window manager via X property
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

#include "shadowhelper.h"
#include "utils.h"
#include "debug.h"

#include <QDockWidget>
#include <QMenu>
#include <QPainter>
#include <QToolBar>
#include <QEvent>

#include "xcb_utils.h"
#include <xcb/xcb_image.h>
#include <data/shadow0-png.h>
#include <data/shadow1-png.h>
#include <data/shadow2-png.h>
#include <data/shadow3-png.h>
#include <data/shadow4-png.h>
#include <data/shadow5-png.h>
#include <data/shadow6-png.h>
#include <data/shadow7-png.h>

namespace QtCurve {

const char *const ShadowHelper::netWMShadowAtomName =
    "_KDE_NET_WM_SHADOW";
const char *const ShadowHelper::netWMForceShadowPropertyName =
    "_KDE_NET_WM_FORCE_SHADOW";
const char *const ShadowHelper::netWMSkipShadowPropertyName =
    "_KDE_NET_WM_SKIP_SHADOW";

//_____________________________________________________
ShadowHelper::ShadowHelper(QObject *parent):
    QObject(parent),
    _atom(0)
{
    createPixmapHandles();
}

//_______________________________________________________
ShadowHelper::~ShadowHelper( void )
{
    for (int i = 0;i < numPixmaps;++i) {
        XcbCallVoid(free_pixmap, _pixmaps[i]);
    }
    XcbUtils::flush();
}

//_______________________________________________________
bool ShadowHelper::registerWidget( QWidget* widget, bool force )
{
    // make sure widget is not already registered
    if (_widgets.contains(widget))
        return false;

    // check if widget qualifies
    if(!( force || acceptWidget(widget)))
        return false;

    // store in map and add destroy signal connection
    Utils::addEventFilter(widget, this);
    _widgets.insert(widget, 0);

    /*
      need to install shadow directly when widget "created" state is already set
      since WinID changed is never called when this is the case
    */
    if (widget->testAttribute(Qt::WA_WState_Created) &&
        installX11Shadows(widget)) {
        _widgets.insert(widget, widget->winId());
    }

    connect(widget, &QWidget::destroyed,
            this, &ShadowHelper::objectDeleted);
    return true;
}

//_______________________________________________________
void ShadowHelper::unregisterWidget( QWidget* widget )
{
    if (_widgets.remove(widget)) {
        uninstallX11Shadows(widget);
    }
}

//_______________________________________________________
bool ShadowHelper::eventFilter(QObject *object, QEvent *event)
{
    // check event type
    if (event->type() != QEvent::WinIdChange)
        return false;

    // cast widget
    QWidget *widget(static_cast<QWidget*>(object));

    // install shadows and update winId
    if (installX11Shadows(widget)) {
        _widgets.insert(widget, widget->winId());
    }

    return false;
}

//_______________________________________________________
void ShadowHelper::objectDeleted(QObject* object)
{
    _widgets.remove(static_cast<QWidget*>(object));
}

//_______________________________________________________
bool ShadowHelper::isMenu(QWidget* widget) const
{
    return qobject_cast<QMenu*>(widget);
}

//_______________________________________________________
bool ShadowHelper::acceptWidget(QWidget* widget) const
{
    if (widget->property(netWMSkipShadowPropertyName).toBool())
        return false;
    if (widget->property(netWMForceShadowPropertyName).toBool())
        return true;

    // menus
    if (qobject_cast<QMenu*>(widget))
        return true;

    // combobox dropdown lists
    if (widget->inherits("QComboBoxPrivateContainer"))
        return true;

    // tooltips
    if ((widget->inherits("QTipLabel") ||
         (widget->windowFlags() & Qt::WindowType_Mask) == Qt::ToolTip) &&
        !widget->inherits("Plasma::ToolTip"))
        return true;

    // detached widgets
    if (qobject_cast<QToolBar*>(widget) || qobject_cast<QDockWidget*>(widget))
        return true;

    // reject
    return false;
}

//______________________________________________
void ShadowHelper::createPixmapHandles()
{
    /*!
      shadow atom and property specification available at
      http://community.kde.org/KWin/Shadow
    */

    // create atom
    if (!_atom)
        _atom = XcbUtils::getAtom(netWMShadowAtomName);
    _pixmaps[0] = createPixmap(&qtc_shadow0);
    _pixmaps[1] = createPixmap(&qtc_shadow1);
    _pixmaps[2] = createPixmap(&qtc_shadow2);
    _pixmaps[3] = createPixmap(&qtc_shadow3);
    _pixmaps[4] = createPixmap(&qtc_shadow4);
    _pixmaps[5] = createPixmap(&qtc_shadow5);
    _pixmaps[6] = createPixmap(&qtc_shadow6);
    _pixmaps[7] = createPixmap(&qtc_shadow7);
    XcbUtils::flush();
}

//______________________________________________
xcb_pixmap_t
ShadowHelper::createPixmap(const QtcPixmap *data)
{
    xcb_pixmap_t pixmap = XcbUtils::generateId();
    _size = data->width;

    // create X11 pixmap
    XcbCallVoid(create_pixmap, 32, pixmap, XcbUtils::rootWindow(),
                data->width, data->height);
    xcb_gcontext_t cid = XcbUtils::generateId();
    XcbCallVoid(create_gc, cid, pixmap, 0, (const uint32_t*)0);
    XcbCallVoid(put_image, XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap, cid,
                data->width, data->height, 0, 0, 0, 32, data->len,
                (uchar*)data->data);
    XcbCallVoid(free_gc, cid);
    return pixmap;
}

//_______________________________________________________
bool ShadowHelper::installX11Shadows( QWidget* widget )
{
    // check widget and shadow
    if (!widget)
        return false;

    // TODO: also check for NET_WM_SUPPORTED atom, before installing shadow

    /*
      From bespin code. Supposibly prevent playing with some 'pseudo-widgets'
      that have winId matching some other -random- window
    */
    if (!(widget->testAttribute(Qt::WA_WState_Created) ||
          widget->internalWinId()))
        return false;

    // create data
    // add pixmap handles
    QVector<uint32_t> data;
    for (int i = 0;i < numPixmaps;++i) {
        data.push_back(_pixmaps[i]);
    }

    // add padding
    data << _size - 4 << _size - 4 << _size - 4 << _size - 4;

    XcbCallVoid(change_property, XCB_PROP_MODE_REPLACE,
                widget->winId(), _atom, XCB_ATOM_CARDINAL,
                32, data.size(), data.constData());
    XcbUtils::flush();
    return true;
}

//_______________________________________________________
void ShadowHelper::uninstallX11Shadows(QWidget *widget) const
{
    if (!(widget && widget->testAttribute(Qt::WA_WState_Created)))
        return;
    uninstallX11Shadows(widget->winId());
}

//_______________________________________________________
void ShadowHelper::uninstallX11Shadows(WId id) const
{
    XcbCallVoid(delete_property, id, _atom);
    XcbUtils::flush();
}

}
