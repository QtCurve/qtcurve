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

#include <klocale.h>
#include <QBitmap>
#include <QDateTime>
#include <QFontMetrics>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionTitleBar>
#include <QStyle>
#include <qdesktopwidget.h>
#include "qtcurvehandler.h"
#include "qtcurveclient.h"
#include "qtcurvebutton.h"
#define QTC_KWIN
#include "common.h"

namespace KWinQtCurve
{

QtCurveClient::QtCurveClient(KDecorationBridge *bridge, KDecorationFactory *factory)
             : KCommonDecoration (bridge, factory),
               itsTitleFont(QFont())
{
}

QString QtCurveClient::visibleName() const
{
    return i18n("QtCurve");
}

bool QtCurveClient::decorationBehaviour(DecorationBehaviour behaviour) const
{
    switch (behaviour)
    {
        case DB_MenuClose:
            return Handler()->menuClose();
        case DB_WindowMask:
            return false;
        default:
            return KCommonDecoration::decorationBehaviour(behaviour);
    }
}

int QtCurveClient::layoutMetric(LayoutMetric lm, bool respectWindowState,
                                const KCommonDecorationButton *btn) const
{
    bool maximized(maximizeMode()==MaximizeFull && !options()->moveResizeMaximizedWindows());

    switch (lm)
    {
        case LM_BorderLeft:
        case LM_BorderRight:
        case LM_BorderBottom:
            return respectWindowState && maximized ? 0 : Handler()->borderSize();
        case LM_TitleEdgeTop:
            return respectWindowState && maximized ? 0 : 3;
        case LM_TitleEdgeBottom:
            return /*respectWindowState && maximized ? 1 : */ 3;
        case LM_TitleEdgeLeft:
        case LM_TitleEdgeRight:
            return respectWindowState && maximized ? 0 : 3;
        case LM_TitleBorderLeft:
        case LM_TitleBorderRight:
            return 5;
        case LM_ButtonWidth:
        case LM_ButtonHeight:
        case LM_TitleHeight:
            return respectWindowState && isToolWindow() ? Handler()->titleHeightTool() : Handler()->titleHeight();
        case LM_ButtonSpacing:
            return 0;
        case LM_ButtonMarginTop:
            return 0;
        default:
            return KCommonDecoration::layoutMetric(lm, respectWindowState, btn);
    }
}

KCommonDecorationButton *QtCurveClient::createButton(ButtonType type)
{
    switch (type)
    {
        case MenuButton:
        case OnAllDesktopsButton:
        case HelpButton:
        case MinButton:
        case MaxButton:
        case CloseButton:
        case AboveButton:
        case BelowButton:
        case ShadeButton:
            return new QtCurveButton(type, this);
        default:
            return 0;
    }
}

void QtCurveClient::init()
{
    itsTitleFont = isToolWindow() ? Handler()->titleFontTool() : Handler()->titleFont();

    KCommonDecoration::init();
    widget()->setAutoFillBackground(false);
    widget()->setAttribute(Qt::WA_OpaquePaintEvent);
    widget()->setAttribute(Qt::WA_PaintOnScreen, false);
}

void QtCurveClient::drawBtnBgnd(QPainter *p, const QRect &r, bool active)
{
    int    state(active ? 1 : 0);
    QColor col(KDecoration::options()->color(KDecoration::ColorTitleBar, active));
    bool   diffSize(itsButtonBackground[state].pix.width()!=r.width() ||
                    itsButtonBackground[state].pix.height()!=r.height());
    int    app(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleBarAppearance, NULL, NULL));

    if(diffSize || itsButtonBackground[state].col!=col || itsButtonBackground[state].app!=app)
    {
        if(diffSize)
            itsButtonBackground[state].pix=QPixmap(r.width(), r.height());

        QRect                br(r);
        QStyleOptionTitleBar opt;
        QPainter             pixPainter(&(itsButtonBackground[state].pix));

        br.adjust(-3, -3, 3, 3);
        opt.rect=br;

        opt.state=QStyle::State_Horizontal|QStyle::State_Enabled|QStyle::State_Raised|
                 (active ? QStyle::State_Active : QStyle::State_None);
        opt.titleBarState=(active ? QStyle::State_Active : QStyle::State_None);
        opt.palette.setColor(QPalette::Button, col);
        Handler()->wStyle()->drawComplexControl(QStyle::CC_TitleBar, &opt, &pixPainter, widget());
        itsButtonBackground[state].col=col;
        itsButtonBackground[state].app=app;
    }

    p->drawPixmap(r, itsButtonBackground[state].pix);
}

void QtCurveClient::paintEvent(QPaintEvent *e)
{
    doShape();

    QPainter             painter(widget());
    QRect                r(widget()->rect());
    QStyleOptionTitleBar opt;
    bool                 active(isActive());
    const int            maximiseOffset(MaximizeFull==maximizeMode() ? 3 : 0),
                         titleHeight(layoutMetric(LM_TitleHeight)),
                         titleEdgeTop(layoutMetric(LM_TitleEdgeTop)),
                         titleEdgeBottom(layoutMetric(LM_TitleEdgeBottom)),
                         titleEdgeLeft(layoutMetric(LM_TitleEdgeLeft)),
                         titleEdgeRight(layoutMetric(LM_TitleEdgeRight)),
                         titleBarHeight(titleHeight+titleEdgeTop+titleEdgeBottom+maximiseOffset);
    int                  rectX, rectY, rectX2, rectY2;

    r.getCoords(&rectX, &rectY, &rectX2, &rectY2);

    const int titleEdgeBottomBottom(rectY+titleEdgeTop+titleHeight+titleEdgeBottom);
    QRect     titleRect(rectX+titleEdgeLeft+buttonsLeftWidth(), rectY+titleEdgeTop,
                        rectX2-titleEdgeRight-buttonsRightWidth()-(rectX+titleEdgeLeft+buttonsLeftWidth()),
                        titleEdgeBottomBottom-(rectY+titleEdgeTop));
    QColor    col(KDecoration::options()->color(KDecoration::ColorTitleBar, active)),
              windowCol(widget()->palette().color(QPalette::Window));

    painter.setClipRegion(e->region());
    painter.fillRect(QRect(0, 0, r.width(), 6), windowCol); // Makes hings look nicer for kcmshell preview...
    painter.fillRect(r.adjusted(0, 6, 0, 0), Handler()->coloredBorder() ? col : windowCol);

    opt.init(widget());

    if(MaximizeFull==maximizeMode())
        r.adjust(-3, -3, 3, 0);
    opt.palette.setColor(QPalette::Button, col);
    opt.rect=QRect(r.x(), r.y()+6, r.width(), r.height()-6);
    opt.state=QStyle::State_Horizontal|QStyle::State_Enabled|QStyle::State_Raised|
             (active ? QStyle::State_Active : QStyle::State_None);

    Handler()->wStyle()->drawPrimitive(QStyle::PE_FrameWindow, &opt, &painter, widget());

    opt.rect=QRect(r.x(), r.y(), r.width(), titleBarHeight);
    opt.titleBarState=(active ? QStyle::State_Active : QStyle::State_None)|QtC_StateKWin;
    Handler()->wStyle()->drawComplexControl(QStyle::CC_TitleBar, &opt, &painter, widget());

    itsCaptionRect = captionRect(); // also update itsCaptionRect!

    if(!caption().isEmpty())
    {
        painter.setFont(itsTitleFont);

        QString str(painter.fontMetrics().elidedText(caption(), Qt::ElideRight, itsCaptionRect.width(), QPalette::WindowText));

        painter.setClipRect(titleRect);
        painter.setPen(shadowColor(KDecoration::options()->color(KDecoration::ColorFont, active)));
        painter.drawText(itsCaptionRect.adjusted(1, 1, 1, 1), Qt::AlignLeft | Qt::AlignVCenter, str);
        painter.setPen(KDecoration::options()->color(KDecoration::ColorFont, active));
        painter.drawText(itsCaptionRect, Qt::AlignLeft | Qt::AlignVCenter, str);
    }
    painter.end();
}

void QtCurveClient::doShape()
{
    int  round=Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_Round, NULL, NULL),
         w(widget()->width()),
         h(widget()->height());
    bool maximized(maximizeMode()==MaximizeFull && !options()->moveResizeMaximizedWindows());

    if(maximized)
        round=ROUND_NONE;

    switch(round)
    {
        case ROUND_NONE:
        {
            QRegion mask(0, 0, w, h);
            setMask(mask);
            break;
        }
        case ROUND_SLIGHT:
        {
            QRegion mask(1, 0, w-2, h);
            mask += QRegion(0, 1, 1, h-2);
            mask += QRegion(w-1, 1, 1, h-2);

            setMask(mask);
            break;
        }
        default: // ROUND_FULL
        {
            QRegion mask(5, 0, w-10, h);

            mask += QRegion(0, 5, 1, h-6);
            mask += QRegion(1, 3, 1, h-3);
            mask += QRegion(2, 2, 1, h-2);
            mask += QRegion(3, 1, 2, h-1);

            mask += QRegion(w-1, 5, 1, h-6);
            mask += QRegion(w-2, 3, 1, h-3);
            mask += QRegion(w-3, 2, 1, h-2);
            mask += QRegion(w-5, 1, 2, h-1);

            setMask(mask);
        }
    }
}

QRect QtCurveClient::captionRect() const
{
    QRect     r(widget()->rect());
    const int titleHeight(layoutMetric(LM_TitleHeight)),
              titleEdgeTop(layoutMetric(LM_TitleEdgeTop)),
              titleEdgeLeft(layoutMetric(LM_TitleEdgeLeft)),
              marginLeft(layoutMetric(LM_TitleBorderLeft)),
              marginRight(layoutMetric(LM_TitleBorderRight)),
              titleLeft(r.left() + titleEdgeLeft + buttonsLeftWidth() + marginLeft),
              titleWidth(r.width() -
                           titleEdgeLeft - layoutMetric(LM_TitleEdgeRight) -
                           buttonsLeftWidth() - buttonsRightWidth() -
                           marginLeft - marginRight);

    return QRect(titleLeft, r.top()+titleEdgeTop, titleWidth, titleHeight);
}

void QtCurveClient::updateCaption()
{
    QRect oldCaptionRect(itsCaptionRect);

    itsCaptionRect = QtCurveClient::captionRect();

    if (oldCaptionRect.isValid() && itsCaptionRect.isValid())
        widget()->update(oldCaptionRect|itsCaptionRect);
    else
        widget()->update();
}

bool QtCurveClient::eventFilter(QObject *o, QEvent *e)
{
    if(QEvent::StyleChange==e->type())
        Handler()->setStyle();

    return KCommonDecoration::eventFilter(o, e);
}

void QtCurveClient::reset(unsigned long changed)
{
    if (changed & SettingColors)
    {
        // repaint the whole thing
        widget()->update();
        updateButtons();
    }
    else if (changed & SettingFont)
    {
        // font has changed -- update title height and font
        itsTitleFont = isToolWindow() ? Handler()->titleFontTool() : Handler()->titleFont();

        updateLayout();
        widget()->update();
    }

    KCommonDecoration::reset(changed);
}

}
