/*
  QtCurve KWin window decoration
  Copyright (C) 2007 - 2009 Craig Drummond <craig_p_drummond@yahoo.co.uk>

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
#define QTC_DRAW_INTO_PIXMAPS

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
#ifdef QTC_DRAW_INTO_PIXMAPS
#include <KDE/KWindowSystem>
#endif
#include <qdesktopwidget.h>
#include "qtcurvehandler.h"
#include "qtcurveclient.h"
#include "qtcurvebutton.h"
#include "resizecorner.h"
#define QTC_KWIN
#include "common.h"

namespace KWinQtCurve
{

QtCurveClient::QtCurveClient(KDecorationBridge *bridge, KDecorationFactory *factory)
#if KDE_IS_VERSION(4,1,80) && !KDE_IS_VERSION(4,2,92)
             : KCommonDecorationUnstable(bridge, factory),
#else
             : KCommonDecoration(bridge, factory),
#endif
               itsResizeGrip(0L),
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
            return respectWindowState && maximized ? 0 : (Handler()->smallBorder() ? 1 : 3);
        case LM_TitleEdgeBottom:
            return /*respectWindowState && maximized ? 1 : */ (Handler()->smallBorder() ? 1 : 3);
        case LM_TitleEdgeLeft:
        case LM_TitleEdgeRight:
            return respectWindowState && maximized ? 0 : (Handler()->smallBorder() ? 1 : 3);
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
    // If WA_PaintOnScreen is set to false when not compositnig then get redraw errors
    // ...if set to true when compositing, then sometimes only part of the titlebar is updated!
    // ...hence in reset() we need to alter this setting
    // ... :-(
    widget()->setAttribute(Qt::WA_PaintOnScreen, !KWindowSystem::compositingActive());
    widget()->setAutoFillBackground(false);
    widget()->setAttribute(Qt::WA_OpaquePaintEvent, true);
    
    if(Handler()->showResizeGrip() && isResizable())
        itsResizeGrip=new ResizeCorner(this, KDecoration::options()->color(KDecoration::ColorTitleBar, isActive()));
}

void QtCurveClient::maximizeChange()
{
    reset(SettingBorder);
    KCommonDecoration::maximizeChange();
}

void QtCurveClient::activeChange()
{
    if (itsResizeGrip)
    {
        itsResizeGrip->setColor(KDecoration::options()->color(KDecoration::ColorTitleBar, isActive()));
        itsResizeGrip->update();
    }
    KCommonDecoration::activeChange();
}

void QtCurveClient::drawBtnBgnd(QPainter *p, const QRect &r, bool active)
{
    int    state(active ? 1 : 0);
    QColor col(KDecoration::options()->color(KDecoration::ColorTitleBar, active));
    bool   diffSize(itsButtonBackground[state].pix.width()!=r.width() ||
                    itsButtonBackground[state].pix.height()!=r.height());
    int    app(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleBarButtonAppearance, NULL, NULL));

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
#ifdef QTC_DRAW_INTO_PIXMAPS
    bool                 compositing=KWindowSystem::compositingActive();
#endif
    QPainter             painter(widget());
    QRect                r(widget()->rect()),
                         rx(r);
    QStyleOptionTitleBar opt;
    bool                 active(isActive()),
                         colorTitleOnly(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleBarColorTopOnly,
                                        NULL, NULL)),
                         mximised(maximizeMode()==MaximizeFull && !options()->moveResizeMaximizedWindows()),
                         roundBottom(Handler()->roundBottom()),
                         noBorder(Handler()->noBorder());
    const int            maximiseOffset(mximised ? 3 : 0),
                         titleHeight(layoutMetric(LM_TitleHeight)),
                         titleEdgeTop(layoutMetric(LM_TitleEdgeTop)),
                         titleEdgeBottom(layoutMetric(LM_TitleEdgeBottom)),
                         titleEdgeLeft(layoutMetric(LM_TitleEdgeLeft)),
                         titleEdgeRight(layoutMetric(LM_TitleEdgeRight)),
                         titleBarHeight(titleHeight+titleEdgeTop+titleEdgeBottom+maximiseOffset),
                         borderSize(Handler()->borderSize()),
                         round=Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_Round, NULL, NULL);
    int                  rectX, rectY, rectX2, rectY2;

    r.getCoords(&rectX, &rectY, &rectX2, &rectY2);

    QColor    col(KDecoration::options()->color(KDecoration::ColorTitleBar, active)),
              windowCol(widget()->palette().color(QPalette::Window));

#if KDE_IS_VERSION(4,1,80) && !KDE_IS_VERSION(4,2,80)
    if(!(Handler()->coloredShadow() && shadowsActive() && active))
#endif
    {
        painter.setClipRegion(e->region());
        painter.fillRect(r, windowCol); // Makes hings look nicer for kcmshell preview...
    }
    painter.setClipRegion(e->region().intersected(getMask(round, r.width(), r.height())));
    painter.fillRect(r, colorTitleOnly ? windowCol : col);

    opt.init(widget());

    if(mximised)
        r.adjust(-3, -3, 3, 0);
    opt.palette.setColor(QPalette::Button, col);
    opt.palette.setColor(QPalette::Window, windowCol);
    opt.rect=QRect(r.x(), r.y()+6, r.width(), r.height()-6);
    opt.state=QStyle::State_Horizontal|QStyle::State_Enabled|QStyle::State_Raised|
             (active ? QStyle::State_Active : QStyle::State_None)|QtC_StateKWin;

#if KDE_IS_VERSION(4,1,80) && !KDE_IS_VERSION(4,2,80)
    if(Handler()->coloredShadow() && shadowsActive())
    {
        opt.state|=QtC_StateKWinShadows;
        if(active)
            opt.state|=QtC_StateKWinHighlight;
    }
#endif

    if(!roundBottom)
        opt.state|=QtCStateKWinNotFull;
    if(noBorder)
        opt.state|=QtCStateKWinNoBorder;
    else
    {
#ifdef QTC_DRAW_INTO_PIXMAPS
        if(!compositing)
        {
            // For some reason, on Jaunty drawing directly is *hideously* slow on intel graphics card!
            QPixmap pix(32, 32);
            QPainter p2(&pix);
            opt.rect=QRect(0, 0, pix.width(), pix.height());
            p2.fillRect(opt.rect, colorTitleOnly ? windowCol : col);
            Handler()->wStyle()->drawPrimitive(QStyle::PE_FrameWindow, &opt, &p2, widget());
            p2.end();
            painter.drawTiledPixmap(r.x(), r.y()+10, 2, r.height()-18, pix.copy(0, 8, 2, 16));
            painter.drawTiledPixmap(r.x()+r.width()-2, r.y()+8, 2, r.height()-16, pix.copy(pix.width()-2, 8, 2, 16));
            painter.drawTiledPixmap(r.x()+8, r.y()+r.height()-2, r.width()-16, 2, pix.copy(8, pix.height()-2, 16, 2));
            painter.drawPixmap(r.x(), r.y()+r.height()-8, pix.copy(0, 24, 8, 8));
            painter.drawPixmap(r.x()+r.width()-8, r.y()+r.height()-8, pix.copy(24, 24, 8, 8));
        }
        else
#endif
            Handler()->wStyle()->drawPrimitive(QStyle::PE_FrameWindow, &opt, &painter, widget());
    }

    if(round>=ROUND_FULL && !colorTitleOnly && col!=windowCol && roundBottom)
    {
        QColor cornerCol(col);
        painter.setPen(windowCol);
        painter.drawRect(rx.x()+borderSize-1, rx.y()+borderSize-1,
                         rx.x()+rx.width()-((borderSize*2)-1), rx.y()+rx.height()-((borderSize*2)-1));
        painter.setPen(cornerCol);
        painter.drawPoint(rx.x()+borderSize-1, rx.y()+rx.height()-(borderSize));
        painter.drawPoint(rx.x()+rx.width()-borderSize, rx.y()+rx.height()-borderSize);
        cornerCol.setAlphaF(0.5);
        painter.setPen(cornerCol);
        painter.drawPoint(rx.x()+borderSize, rx.y()+rx.height()-(borderSize));
        painter.drawPoint(rx.x()+borderSize-1, rx.y()+rx.height()-(borderSize+1));
        painter.drawPoint(rx.x()+rx.width()-borderSize-1, rx.y()+rx.height()-borderSize);
        painter.drawPoint(rx.x()+rx.width()-borderSize, rx.y()+rx.height()-(borderSize+1));
    }

    opt.palette.setColor(QPalette::Button, col);
    opt.rect=QRect(r.x(), r.y(), r.width(), titleBarHeight);
    opt.titleBarState=(active ? QStyle::State_Active : QStyle::State_None)|QtC_StateKWin;
    if(KDecoration::options()->color(KDecoration::ColorTitleBar, true)!=windowCol ||
       KDecoration::options()->color(KDecoration::ColorTitleBar, false)!=windowCol)
       opt.titleBarState|=QtCStateKWinDrawLine;
#ifdef QTC_DRAW_INTO_PIXMAPS
    if(!compositing)
    {
        QPixmap  tPix(32, titleBarHeight);
        QPainter tPainter(&tPix);
        opt.rect=QRect(0, 0, tPix.width(), tPix.height());
        Handler()->wStyle()->drawComplexControl(QStyle::CC_TitleBar, &opt, &tPainter, widget());
        tPainter.end();
        painter.drawTiledPixmap(r.x()+12, r.y(), r.width()-24, tPix.height(), tPix.copy(8, 0, 16, tPix.height()));
        painter.drawPixmap(r.x(), r.y(), tPix.copy(0, 0, 16, tPix.height()));
        painter.drawPixmap(r.x()+r.width()-16, r.y(), tPix.copy(tPix.width()-16, 0, 16, tPix.height()));
    }
    else
#endif
        Handler()->wStyle()->drawComplexControl(QStyle::CC_TitleBar, &opt, &painter, widget());

    itsCaptionRect = captionRect(); // also update itsCaptionRect!
    bool     showIcon=TITLEBAR_ICON_NEXT_TO_TITLE==Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleBarIcon,
                                                                                    0L, 0L);
    int     iconSize=showIcon ? Handler()->wStyle()->pixelMetric(QStyle::PM_SmallIconSize) : 0,
            iconX=itsCaptionRect.x();
    QPixmap menuIcon;

    if(showIcon)
    {
        menuIcon=icon().pixmap(iconSize);

        if(menuIcon.isNull())
            showIcon=false;
    }
        
    if(!caption().isEmpty())
    {
        static const int constPad=4;

        painter.setFont(itsTitleFont);

        QFontMetrics  fm(painter.fontMetrics());
        QString       str(fm.elidedText(caption(), Qt::ElideRight,
                            itsCaptionRect.width()-(showIcon ? iconSize+constPad : 0), QPalette::WindowText));
        Qt::Alignment hAlign((Qt::Alignment)Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleAlignment, 0L, 0L)),
                      alignment(Qt::AlignVCenter|hAlign);
        const int     titleEdgeBottomBottom(rectY+titleEdgeTop+titleHeight+titleEdgeBottom);
        bool          alignFull(Qt::AlignHCenter==hAlign),
                      reverse=Qt::RightToLeft==QApplication::layoutDirection(),
                      iconRight((!reverse && alignment&Qt::AlignRight) || (reverse && alignment&Qt::AlignLeft));
        QRect         textRect(alignFull
                                    ? QRect(rectX+titleEdgeLeft, itsCaptionRect.y(),
                                            rectX2-titleEdgeRight-(rectX+titleEdgeLeft),
                                            itsCaptionRect.height())
                                    : itsCaptionRect);
        int           textWidth=alignFull || (showIcon && alignment&Qt::AlignHCenter)
                                    ? fm.boundingRect(str).width()+(showIcon ? iconSize+constPad : 0) : 0;

        if(alignFull)
            if(itsCaptionRect.left()>((textRect.width()-textWidth)>>1))
            {
                alignment=Qt::AlignVCenter|Qt::AlignLeft;
                textRect=itsCaptionRect;
            }
            else if(itsCaptionRect.right()<((textRect.width()+textWidth)>>1))
            {
                alignment=Qt::AlignVCenter|Qt::AlignRight;
                textRect=itsCaptionRect;
            }

        if(showIcon)
            if(alignment&Qt::AlignHCenter)
            {
                if(reverse)
                {
                    iconX=((textRect.width()-textWidth)/2.0)+0.5+textWidth+iconSize;
                    textRect.setX(textRect.x()-(iconSize+constPad));
//                     iconX=((textRect.width()-textWidth)/2.0)+0.5+(textWidth-constPad);
//                     textRect.setX(iconX-(textWidth-iconSize));
//                     textRect.setWidth(textWidth-iconSize);
//                     alignment=Qt::AlignVCenter|Qt::AlignRight;
                }
                else
                {
                    iconX=((textRect.width()-textWidth)/2.0)+0.5;
                    textRect.setX(iconX+iconSize+constPad);
                    alignment=Qt::AlignVCenter|Qt::AlignLeft;
                }
            }
            else if((!reverse && alignment&Qt::AlignLeft) || (reverse && alignment&Qt::AlignRight))
            {
                iconX=textRect.x();
                textRect.setX(textRect.x()+(iconSize+constPad));
            }
            else if((!reverse && alignment&Qt::AlignRight) || (reverse && alignment&Qt::AlignLeft))
            {
                if(iconRight)
                {
                    iconX=textRect.x()+textRect.width()-iconSize;
                    textRect.setWidth(textRect.width()-(iconSize+constPad));
                }
                else
                {
                    iconX=textRect.x()+textRect.width()-textWidth;
                    if(iconX<textRect.x())
                        iconX=textRect.x();
                }
            }

        painter.setClipRect(itsCaptionRect.adjusted(-2, 0, 2, 0));
        painter.setPen(shadowColor(KDecoration::options()->color(KDecoration::ColorFont, active)));
        painter.drawText(textRect.adjusted(1, 1, 1, 1), alignment, str);
        painter.setPen(KDecoration::options()->color(KDecoration::ColorFont, active));
        painter.drawText(textRect, alignment, str);
        painter.setClipping(false);
    }

    if(showIcon && iconX>=0)
        painter.drawPixmap(iconX, itsCaptionRect.y()+((itsCaptionRect.height()-iconSize)/2)+1, menuIcon);

    painter.end();
}

void QtCurveClient::updateWindowShape()
{
    setMask(getMask(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_Round, NULL, NULL),
                    widget()->width(), widget()->height(),
                    maximizeMode()==MaximizeFull && !options()->moveResizeMaximizedWindows()));
}

QRegion QtCurveClient::getMask(int round, int w, int h, bool maximised) const
{  
    switch(maximised ? ROUND_NONE : round)
    {
        case ROUND_NONE:
            return  QRegion(0, 0, w, h);
        case ROUND_SLIGHT:
        {
            QRegion mask(1, 0, w-2, h);
            mask += QRegion(0, 1, 1, h-2);
            mask += QRegion(w-1, 1, 1, h-2);

            return mask;
        }
        default: // ROUND_FULL
        {
            QRegion mask(5, 0, w-10, h);
            bool    roundBottom=Handler()->roundBottom();

            if(roundBottom)
            {
                mask += QRegion(0, 5, 1, h-10);
                mask += QRegion(1, 3, 1, h-6);
                mask += QRegion(2, 2, 1, h-4);
                mask += QRegion(3, 1, 2, h-2);
                mask += QRegion(w-1, 5, 1, h-10);
                mask += QRegion(w-2, 3, 1, h-6);
                mask += QRegion(w-3, 2, 1, h-4);
                mask += QRegion(w-5, 1, 2, h-2);
            }
            else
            {
                mask += QRegion(0, 5, 1, h-5);
                mask += QRegion(1, 3, 1, h-2);
                mask += QRegion(2, 2, 1, h-1);
                mask += QRegion(3, 1, 2, h);
                mask += QRegion(w-1, 5, 1, h-5);
                mask += QRegion(w-2, 3, 1, h-2);
                mask += QRegion(w-3, 2, 1, h-1);
                mask += QRegion(w-5, 1, 2, h);
            }

            return mask;
        }
    }

    return QRegion();
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

#if KDE_IS_VERSION(4,1,80) && !KDE_IS_VERSION(4,2,92)
// Taken form Oxygen! rev873805
QList<QRect> QtCurveClient::shadowQuads(ShadowType type) const
{
    Q_UNUSED(type)

    QSize size = widget()->size();
    int outside=20, underlap=5, cornersize=25;
    // These are underlap under the decoration so the corners look nicer 10px on the outside
    QList<QRect> quads;
    quads.append(QRect(-outside, size.height()-underlap, cornersize, cornersize));
    quads.append(QRect(underlap, size.height()-underlap, size.width()-2*underlap, cornersize));
    quads.append(QRect(size.width()-underlap, size.height()-underlap, cornersize, cornersize));
    quads.append(QRect(-outside, underlap, cornersize, size.height()-2*underlap));
    quads.append(QRect(size.width()-underlap, underlap, cornersize, size.height()-2*underlap));
    quads.append(QRect(-outside, -outside, cornersize, cornersize));
    quads.append(QRect(underlap, -outside, size.width()-2*underlap, cornersize));
    quads.append(QRect(size.width()-underlap, -outside, cornersize, cornersize));
    return quads;
}

double QtCurveClient::shadowOpacity(ShadowType type) const
{
    switch( type ) {
        case ShadowBorderedActive:
            return isActive() ? 1.0 : 0.0;
        case ShadowBorderedInactive:
            return isActive() ? 0.0 : 1.0;
        default:
            return 0;
    }
    return 0;
}
#endif
    
void QtCurveClient::reset(unsigned long changed)
{
    // Set note in init() above
    if(0==changed)
        widget()->setAttribute(Qt::WA_PaintOnScreen, !KWindowSystem::compositingActive());
    
    if (changed&SettingBorder)
        if (maximizeMode() == MaximizeFull)
        {
            if (!options()->moveResizeMaximizedWindows() && itsResizeGrip)
                itsResizeGrip->hide();
        }
        else if (itsResizeGrip)
                itsResizeGrip->show();

    if (changed&SettingColors)
    {
        // repaint the whole thing
        widget()->update();
        updateButtons();
    }
    else if (changed&SettingFont)
    {
        // font has changed -- update title height and font
        itsTitleFont = isToolWindow() ? Handler()->titleFontTool() : Handler()->titleFont();

        updateLayout();
        widget()->update();
    }
    
    KCommonDecoration::reset(changed);
}

}
