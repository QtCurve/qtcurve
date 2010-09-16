/*
  QtCurve KWin window decoration
  Copyright (C) 2010 Craig Drummond <craig.p.drummond@googlemail.com>

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

#include "qtcurvetogglebutton.h"
#include <QAbstractButton>
#include <QStyle>
#include <QStyleOption>
#include <QBitmap>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QX11Info>
#include <KDE/KLocale>
#include "qtcurveclient.h"
#include "common.h"

#include <stdio.h>
namespace KWinQtCurve
{

static int point2Pixel(double point)
{
    return (int)(((point*QX11Info::appDpiY())/72.0)+0.5);
}

QtCurveToggleButton::QtCurveToggleButton(bool menubar, QtCurveClient *parent)
                   : KCommonDecorationButton(AboveButton, parent),
                     itsClient(parent),
                     isMenuBar(menubar),
                     itsHover(false)
{
//     setAttribute(Qt::WA_PaintOnScreen, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAutoFillBackground(false);
//     setFocusPolicy(Qt::NoFocus);
//     setAttribute(Qt::WA_OpaquePaintEvent, false);
//     setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::ArrowCursor);
    setToolTip(menubar ? i18n("Toggle Menubar") : i18n("Toggle Statusbar"));
    reset(DecorationReset);
}

void QtCurveToggleButton::reset(unsigned long changed)
{
    if (changed&DecorationReset || changed&ManualReset || changed&SizeChange || changed&StateChange)
        this->update();
}

void QtCurveToggleButton::enterEvent(QEvent *e)
{
    itsHover = true;
    KCommonDecorationButton::enterEvent(e);
    update();
    // Hacky NVIDIA fix - sometimes mouseover state gets 'stuck' - but only for some windows!!!
    QTimer::singleShot(50, this, SLOT(update()));
}

void QtCurveToggleButton::leaveEvent(QEvent *e)
{
    itsHover = false;
    KCommonDecorationButton::leaveEvent(e);
    update();
    // Hacky NVIDIA fix - sometimes mouseover state gets 'stuck' - but only for some windows!!!
    QTimer::singleShot(50, this, SLOT(update()));
}

void QtCurveToggleButton::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.setClipRect(rect().intersected(ev->rect()));
    drawButton(&p);
}

void QtCurveToggleButton::drawButton(QPainter *painter)
{
    QRect  r(0, 0, width(), height());
    bool   active(itsClient->isActive()),
           sunken(isDown());
    QColor col(KDecoration::options()->color(KDecoration::ColorFont, active/* || faded*/));

    col.setAlphaF(itsHover ? 0.99 : 0.15);
    painter->setRenderHint(QPainter::Antialiasing, true);
    //painter->setPen(QPen(col, (isChecked() ? 2.0 : 1.0)));
    painter->setPen(col);
    r.adjust(1, 1, -1, -1);

    QFont font(Handler()->titleFont());
    int   maxPixelSize=r.height()-2,
          fontPixelSize=font.pixelSize();
    bool  drawBorder=true;

    if(maxPixelSize<9)
    {
        maxPixelSize=r.height()+2;
        drawBorder=false;
        r.adjust(-1, -1, 1, 1);
    }

    if(-1==fontPixelSize)
        fontPixelSize=point2Pixel(font.pointSizeF());

    if(fontPixelSize>maxPixelSize)
        font.setPixelSize(maxPixelSize-2);
    painter->setFont(font);

    QRectF       ellipse(r.x()+0.5, r.y()+0.5, r.width(), r.height());
    QColor       bgnd(KDecoration::options()->color(KDecoration::ColorTitleBar, active));
    QPainterPath path;
//    EEffect      effect((EEffect)(style()->pixelMetric((QStyle::PixelMetric)QtC_TitleBarEffect)));

    bgnd.setAlphaF(itsHover ? 0.9 : 0.4);
    path.addEllipse(ellipse);
    painter->fillPath(path, bgnd);
    if(sunken)
    {
        bgnd=col;
        bgnd.setAlphaF(0.2);
        painter->fillPath(path, bgnd);
    }
    if(drawBorder)
        painter->drawEllipse(ellipse);

//    if(EFFECT_ETCH==effect)
//        effect=EFFECT_SHADOW;

    if(sunken)
        r.adjust(1, 1, 1, 1);
//     else if (EFFECT_NONE!=effect && itsHover)
//     {
//         QColor shadow(WINDOW_SHADOW_COLOR(effect));
// 
//         shadow.setAlphaF(WINDOW_TEXT_SHADOW_ALPHA(effect));
//         painter->setPen(shadow);
//         painter->drawText(r.adjusted(1, 1, 1, 1), Qt::AlignVCenter|Qt::AlignHCenter, isMenuBar ? i18n("M") : i18n("S"));
//     }

    painter->setPen(col);
    painter->drawText(r, Qt::AlignVCenter|Qt::AlignHCenter, isMenuBar ? i18n("M") : i18n("S"));
}

}
