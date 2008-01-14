/*
  QtCurve KWin window decoration
  Copyright (C) 2007 Craig Drummond <Craig.Drummond@lycos.co.uk>

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

#include <kcommondecoration.h>
#include <QtGui/QPixmap>
#include <QtGui/QColor>

namespace KWinQtCurve
{

class QtCurveClient : public KCommonDecoration
{
    public:

    static QColor shadowColor(const QColor col)
    {
        return qGray(col.rgb()) < 100 ? QColor(255, 255, 255, 75) : QColor(0, 0, 0, 75);
    }

    QtCurveClient(KDecorationBridge *bridge, KDecorationFactory *factory);
    ~QtCurveClient() { }

    QString                   visibleName() const;
    bool                      decorationBehaviour(DecorationBehaviour behaviour) const;
    int                       layoutMetric(LayoutMetric lm, bool respectWindowState = true,
                                           const KCommonDecorationButton *btn= 0) const;
    KCommonDecorationButton * createButton(ButtonType type);
    void                      init();
    void                      reset(unsigned long changed);
    void                      drawBtnBgnd(QPainter *p, const QRect &r, bool active);
    void                      paintEvent(QPaintEvent *e);
    void                      doShape();
    void                      updateCaption();
    bool                      eventFilter(QObject *o, QEvent *e);

    private:

    QRect                     captionRect() const;

    private:

    struct ButtonBgnd
    {
        QPixmap pix;
        QColor  col;
    };

    ButtonBgnd itsButtonBackground[2];
    QRect      itsCaptionRect;
    QString    itsOldCaption;
    QFont   itsTitleFont;
};

}

#endif
