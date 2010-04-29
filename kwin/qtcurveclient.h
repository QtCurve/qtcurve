/*
  QtCurve KWin window decoration
  Copyright (C) 2007 - 2010 Craig Drummond <craig.p.drummond@googlemail.com>

  based on the window decoration "Plastik":
  Copyright (C) 2003-2005 Sandro Giessl <sandro@giessl.com>

  based on the window decoration "Web":
  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
 */

#ifndef QTCURVECLIENT_H
#define QTCURVECLIENT_H

#include <kdeversion.h>
#include <kcommondecoration.h>
#include <QtGui/QPixmap>
#include <QtGui/QColor>
#include "qtcurvehandler.h"
#include "config.h"

namespace KWinQtCurve
{

class QtCurveSizeGrip;
class QtCurveButton;

class QtCurveClient :
#if KDE_IS_VERSION(4, 3, 0)
                       public KCommonDecorationUnstable
#else
                       public KCommonDecoration
#endif    
{
    public:

    QtCurveClient(KDecorationBridge *bridge, QtCurveHandler *factory);
    virtual ~QtCurveClient();

    QString                   visibleName() const;
    bool                      decorationBehaviour(DecorationBehaviour behaviour) const;
    int                       layoutMetric(LayoutMetric lm, bool respectWindowState = true,
                                           const KCommonDecorationButton *btn= 0) const;
    KCommonDecorationButton * createButton(ButtonType type);
    void                      init();
    void                      maximizeChange();
    void                      shadeChange();
    void                      activeChange();
    void                      captionChange();
    void                      reset(unsigned long changed);
    void                      paintEvent(QPaintEvent *e);
    void                      paintTitle(QPainter *painter, const QRect &capRect, const QRect &alignFullRect,
                                         const QString &cap, const QPixmap &pix, int shadowSize=0,
                                         bool isTab=false, bool activeTab=false);
#if KDE_IS_VERSION(4, 3, 85)
    void                      paintSeparator(QPainter *painter, const QRect &r);
#endif
    void                      updateWindowShape();
    QRegion                   getMask(int round, const QRect &r) const;
    void                      updateCaption();
    bool                      eventFilter(QObject *o, QEvent *e);
    bool isMaximized() const { return maximizeMode()==MaximizeFull && !options()->moveResizeMaximizedWindows();  }
    void                      menubarSize(int size);

    private:

#if KDE_IS_VERSION(4, 3, 85)
    bool                      mouseSingleClickEvent(QMouseEvent *e);
    bool                      mouseMoveEvent(QMouseEvent *e);
    bool                      mouseButtonPressEvent(QMouseEvent *e);
    bool                      mouseButtonReleaseEvent(QMouseEvent *e);
    bool                      dragMoveEvent(QDragMoveEvent *e);
    bool                      dragLeaveEvent(QDragLeaveEvent *e);
    bool                      dragEnterEvent(QDragEnterEvent *e);
    bool                      dropEvent(QDropEvent *e);
    int                       itemClicked(const QPoint &point, bool between=false, bool drag=false);
#endif

    QRect                     captionRect() const;
    void                      createSizeGrip();
    void                      deleteSizeGrip();
    void                      informAppOfActiveChange();
    const QString &           windowClass();

    private:

    struct ButtonBgnd
    {
        QPixmap pix;
        int     app;
        QColor  col;
    };

    static const int constNumButtonStates=2;

    QtCurveSizeGrip *itsResizeGrip;
    ButtonBgnd      itsButtonBackground[constNumButtonStates];
    QRect           itsCaptionRect;
    QString         itsCaption,
                    itsWindowClass;
    QFont           itsTitleFont;
    int             itsMenuBarSize;

#if KDE_IS_VERSION(4, 3, 85)
    QList<QtCurveButton *> itsCloseButtons;
    bool                   itsClickInProgress,
                           itsDragInProgress;
    Qt::MouseButton        itsMouseButton;
    QPoint                 itsClickPoint;
    int                    itsTargetTab;
#endif
};

}

#endif
