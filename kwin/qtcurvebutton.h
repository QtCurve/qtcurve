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

#ifndef QTCURVEBUTTON_H
#define QTCURVEBUTTON_H

#include <QImage>
#include "qtcurvehandler.h"
#include <kcommondecoration.h>

class QTimer;
class QStyle;

namespace KWinQtCurve
{

class QtCurveClient;

class QtCurveButton : public KCommonDecorationButton
{
    public:

    QtCurveButton(ButtonType type, QtCurveClient *parent);
    ~QtCurveButton() { }

    void reset(unsigned long changed);
    QtCurveClient * client() { return itsClient; }

    protected:

    void paintEvent(QPaintEvent *);

    private:

    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void drawButton(QPainter *painter);
    void updateMask();

    private:

    QtCurveClient *itsClient;
    ButtonIcon    itsIconType;
    bool          itsHover;

    friend class IconEngine;
};

/**
 * This class creates bitmaps which can be used as icons on buttons. The icons
 * are "hardcoded".
 * Over the previous "Gimp->xpm->QImage->recolor->SmoothScale->QPixmap" solution
 * it has the important advantage that icons are more scalable and at the same
 * time sharp and not blurred.
 */
class IconEngine
{
    public:

    static QBitmap icon(ButtonIcon icon, int size, QStyle *style);

    private:

    enum Object
    {
        HorizontalLine,
        VerticalLine,
        DiagonalLine,
        CrossDiagonalLine
    };

    static void drawObject(QPainter &p, Object object, int x, int y, int length, int lineWidth);
};

}

#endif
