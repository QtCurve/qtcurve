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
#include "qtcurveclient.h"
#include "qtcurvebutton.h"
#include "qtcurvesizegrip.h"
#define QTC_KWIN
#include "common.h"
#if KDE_IS_VERSION(4, 3, 0)
#include "tileset.h"
#endif

#if KDE_IS_VERSION(4, 3, 0)
    #define QTC_COMPOSITING compositingActive()
#else
    #define QTC_COMPOSITING KWindowSystem::compositingActive()
#endif

namespace KWinQtCurve
{

QtCurveClient::QtCurveClient(KDecorationBridge *bridge, QtCurveHandler *factory)
#if KDE_IS_VERSION(4, 3, 0)
             : KCommonDecorationUnstable(bridge, factory),
#else
             : KCommonDecoration(bridge, factory),
#endif    
               itsResizeGrip(0L),
               itsTitleFont(QFont())
{
}

QtCurveClient::~QtCurveClient()
{
    deleteSizeGrip();
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
    switch (lm)
    {
        case LM_BorderLeft:
        case LM_BorderRight:
        case LM_BorderBottom:
            return respectWindowState && isMaximized() ? 0 : Handler()->borderSize();
        case LM_TitleEdgeTop:
            return respectWindowState && isMaximized() ? 0 : Handler()->borderEdgeSize();
        case LM_TitleEdgeBottom:
            return /*respectWindowState && isMaximized() ? 1 : */ Handler()->borderEdgeSize();
        case LM_TitleEdgeLeft:
        case LM_TitleEdgeRight:
            return respectWindowState && isMaximized() ? 0 : Handler()->borderEdgeSize();
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
#if KDE_IS_VERSION(4, 3, 0)
        case LM_OuterPaddingLeft:
        case LM_OuterPaddingRight:
        case LM_OuterPaddingTop:
        case LM_OuterPaddingBottom:
            if(Handler()->customShadows())
                return Handler()->shadowCache().shadowSize();
#endif
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
    itsTitleFont=isToolWindow() ? Handler()->titleFontTool() : Handler()->titleFont();

    KCommonDecoration::init();
    // If WA_PaintOnScreen is set to false when not compositnig then get redraw errors
    // ...if set to true when compositing, then sometimes only part of the titlebar is updated!
    // ...hence in reset() we need to alter this setting
    // ... :-(
    widget()->setAttribute(Qt::WA_PaintOnScreen, !QTC_COMPOSITING);
    widget()->setAutoFillBackground(false);
    widget()->setAttribute(Qt::WA_OpaquePaintEvent, true);
    widget()->setAttribute(Qt::WA_NoSystemBackground);

    if(Handler()->showResizeGrip())
        createSizeGrip();
}

void QtCurveClient::maximizeChange()
{
    reset(SettingBorder);
    if(itsResizeGrip)
        itsResizeGrip->setVisible(!(isShade() || isMaximized()));
    KCommonDecoration::maximizeChange();
}

void QtCurveClient::shadeChange()
{
    if(itsResizeGrip)
        itsResizeGrip->setVisible(!(isShade() || isMaximized()));
    KCommonDecoration::shadeChange();
}

void QtCurveClient::activeChange()
{
    if(itsResizeGrip && !(isShade() || isMaximized()))
    {
        itsResizeGrip->activeChange();
        itsResizeGrip->update();
    }
    KCommonDecoration::activeChange();
}

void QtCurveClient::paintEvent(QPaintEvent *e)
{
    bool                 compositing=QTC_COMPOSITING;
    QPainter             painter(widget());
    QRect                r(widget()->rect());
    QStyleOptionTitleBar opt;
    bool                 active(isActive()),
                         colorTitleOnly(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleBarColorTopOnly,
                                        NULL, NULL)),
                         roundBottom(Handler()->roundBottom()),
                         outerBorder(Handler()->outerBorder());
    const int            borderSize(Handler()->borderSize()),
                         border(Handler()->borderEdgeSize()),
                         titleHeight(layoutMetric(LM_TitleHeight)),
                         titleEdgeTop(layoutMetric(LM_TitleEdgeTop)),
                         titleEdgeBottom(layoutMetric(LM_TitleEdgeBottom)),
                         titleEdgeLeft(layoutMetric(LM_TitleEdgeLeft)),
                         titleEdgeRight(layoutMetric(LM_TitleEdgeRight)),
                         titleBarHeight(titleHeight+titleEdgeTop+titleEdgeBottom+(isMaximized() ? border : 0)),
                         round=Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_Round, NULL, NULL);
    int                  rectX, rectY, rectX2, rectY2;

    painter.setClipRegion(e->region());

#if KDE_IS_VERSION(4, 3, 0)
    if(Handler()->customShadows() && compositing)
    {
        TileSet *tileSet( 0 );
//         if( configuration().useOxygenShadows() && glowIsAnimated() && !isForcedActive() )
//       {
//
//         int frame = ;
//         tileSet = shadowCache().tileSet(this, frame);=
//       }
//       else
          tileSet = Handler()->shadowCache().tileSet(this);

        if(!isMaximized())
            tileSet->render(r.adjusted(5, 5, -5, -5), &painter, TileSet::Ring);
        else if(isShade())
            tileSet->render(r.adjusted(0, 5, 0, -5), &painter, TileSet::Bottom);

        int shadowSize=Handler()->shadowCache().shadowSize();
        r.adjust(shadowSize, shadowSize, -shadowSize, -shadowSize);
    }
#endif

    r.getCoords(&rectX, &rectY, &rectX2, &rectY2);

    QColor col(KDecoration::options()->color(KDecoration::ColorTitleBar, active)),
           windowCol(widget()->palette().color(QPalette::Window));

    if(isMaximized())
        painter.setClipRect(r, Qt::IntersectClip);
    else
#if KDE_IS_VERSION(4, 3, 0)
        if(!compositing && !isPreview())
#endif
        painter.setClipRegion(getMask(round, r), Qt::IntersectClip);

    painter.fillRect(r, compositing ? Qt::transparent : (colorTitleOnly ? windowCol : col));
    painter.setRenderHint(QPainter::Antialiasing, true);
    if(compositing)
    {
#if KDE_IS_VERSION(4, 3, 0)
        if(roundBottom)
        {
            double       radius((round>ROUND_SLIGHT ? 6.0 : 2.0) - (outerBorder ? 1.0 : 0.0)),
                         dr(radius * 2);
            QRect        fr(outerBorder ? r.adjusted(1, 1, -1, -1) : r);
            QRectF       rf(fr.x()-0.5, fr.y()+5.5, fr.width()+0.5, fr.height() - 5.5);
            QPainterPath path;

            path.moveTo(rf.right(), rf.top() + radius);
            path.arcTo(rf.right() - dr, rf.top(), dr, dr, 0.0, 90.0);
            path.lineTo(rf.left() + radius, rf.top());
            path.arcTo(rf.left(), rf.top(), dr, dr, 90.0, 90.0);
            path.lineTo(rf.left(), rf.bottom() - radius);
            path.arcTo(rf.left(), rf.bottom() - dr, dr, dr, 180.0, 90.0);
            path.lineTo(rf.right() - radius, rf.bottom());
            path.arcTo(rf.right() - dr, rf.bottom() - dr, dr, dr,  270.0, 90.0);
            painter.fillPath(path, colorTitleOnly ? windowCol : col);
        }
        else
#endif
            painter.fillRect(r.adjusted(0, 5, 0, 0), colorTitleOnly ? windowCol : col);
    }

    opt.init(widget());

    if(isMaximized())
        r.adjust(-3, -border, 3, 0);
    opt.palette.setColor(QPalette::Button, col);
    opt.palette.setColor(QPalette::Window, windowCol);
    opt.rect=QRect(r.x(), r.y()+6, r.width(), r.height()-6);
    opt.state=QStyle::State_Horizontal|QStyle::State_Enabled|QStyle::State_Raised|
             (active ? QStyle::State_Active : QStyle::State_None)|QtC_StateKWin;

    if(!roundBottom)
        opt.state|=QtC_StateKWinNotFull;

    if(compositing && !isPreview())
        opt.state|=QtC_StateKWinCompositing;

    if(outerBorder)
    {
#ifdef QTC_DRAW_INTO_PIXMAPS
        if(!compositing && !isPreview())
        {
            // For some reason, on Jaunty drawing directly is *hideously* slow on intel graphics card!
            QPixmap pix(32, 32);
            QPainter p2(&pix);
            p2.setRenderHint(QPainter::Antialiasing, true);
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
    else
        opt.state|=QtC_StateKWinNoBorder;

    opt.palette.setColor(QPalette::Button, col);
    opt.rect=QRect(r.x(), r.y(), r.width(), titleBarHeight);
    opt.titleBarState=(active ? QStyle::State_Active : QStyle::State_None)|QtC_StateKWin;

#ifdef QTC_DRAW_INTO_PIXMAPS
    if(!compositing && !isPreview())
    {
        QPixmap  tPix(32, titleBarHeight);
        QPainter tPainter(&tPix);
        tPainter.setRenderHint(QPainter::Antialiasing, true);
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

    itsCaptionRect=captionRect(); // also update itsCaptionRect!
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
        EEffect       effect((EEffect)(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleBarEffect)));

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
                    iconX=textRect.x()+textRect.width()-(textWidth+constPad);
                    if(iconX<textRect.x())
                        iconX=textRect.x();
                }
            }

        painter.setClipRect(itsCaptionRect.adjusted(-2, 0, 2, 0));

        QColor color(KDecoration::options()->color(KDecoration::ColorFont, active));

        if(EFFECT_NONE!=effect)
        {
            QColor shadow(WINDOW_SHADOW_COLOR(effect));
            shadow.setAlphaF(WINDOW_TEXT_SHADOW_ALPHA(effect));
            painter.setPen(shadow);
            painter.drawText(textRect.adjusted(1, 1, 1, 1), alignment, str);

            if (!active && QTC_DARK_WINDOW_TEXT(color))
                color.setAlpha((color.alpha() * 180) >> 8);
        }

        painter.setPen(color);
        painter.drawText(textRect, alignment, str);
        painter.setClipping(false);
    }

    if(showIcon && iconX>=0)
        painter.drawPixmap(iconX, itsCaptionRect.y()+((itsCaptionRect.height()-iconSize)/2)+1, menuIcon);

    painter.end();
}

void QtCurveClient::updateWindowShape()
{
    if(isMaximized())
        clearMask();
    else
    {
        QRect r(Handler()->customShadows() && QTC_COMPOSITING
                    ? widget()->rect().adjusted(layoutMetric(LM_OuterPaddingLeft), layoutMetric(LM_OuterPaddingTop),
                                               -layoutMetric(LM_OuterPaddingRight), -layoutMetric( LM_OuterPaddingBottom))
                    : widget()->rect());

        setMask(getMask(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_Round, NULL, NULL), r));
    }
}

QRegion QtCurveClient::getMask(int round, const QRect &r) const
{
    int x, y, w, h;

    r.getRect(&x, &y, &w, &h);

    switch(round)
    {
        case ROUND_NONE:
            return  QRegion(x, y, w, h);
        case ROUND_SLIGHT:
        {
            QRegion mask(x+1, y, w-2, h);
            mask += QRegion(x, y+1, 1, h-2);
            mask += QRegion(x+w-1, y+1, 1, h-2);

            return mask;
        }
        default: // ROUND_FULL
        {
            bool    roundBottom=!isShade() && Handler()->roundBottom();

// #if KDE_IS_VERSION(4, 3, 0)
//             if(!isPreview() && QTC_COMPOSITING)
//             {
//                 QRegion mask(x+4, y, w-8, h);
//
//                 if(roundBottom)
//                 {
//                     mask += QRegion(x, y+4, 1, h-8);
//                     mask += QRegion(x+1, y+2, 1, h-4);
//                     mask += QRegion(x+2, y+1, 1, h-2);
//                     mask += QRegion(x+3, y+1, 1, h-2);
//                     mask += QRegion(x+w-1, y+4, 1, h-8);
//                     mask += QRegion(x+w-2, y+2, 1, h-4);
//                     mask += QRegion(x+w-3, y+1, 1, h-2);
//                     mask += QRegion(x+w-4, y+1, 1, h-2);
//                 }
//                 else
//                 {
//                     mask += QRegion(x, y+4, 1, h-4);
//                     mask += QRegion(x+1, y+2, 1, h-1);
//                     mask += QRegion(x+2, y+1, 1, h);
//                     mask += QRegion(x+3, y+1, 1, h);
//                     mask += QRegion(x+w-1, y+4, 1, h-4);
//                     mask += QRegion(x+w-2, y+2, 1, h-1);
//                     mask += QRegion(x+w-3, y+1, 1, h-0);
//                     mask += QRegion(x+w-4, y+1, 1, h-0);
//                 }
//                 return mask;
//             }
//             else
// #endif
            {
                QRegion mask(x+5, y, w-10, h);

                if(roundBottom)
                {
                    mask += QRegion(x, y+5, 1, h-10);
                    mask += QRegion(x+1, y+3, 1, h-6);
                    mask += QRegion(x+2, y+2, 1, h-4);
                    mask += QRegion(x+3, y+1, 2, h-2);
                    mask += QRegion(x+w-1, y+5, 1, h-10);
                    mask += QRegion(x+w-2, y+3, 1, h-6);
                    mask += QRegion(x+w-3, y+2, 1, h-4);
                    mask += QRegion(x+w-5, y+1, 2, h-2);
                }
                else
                {
                    mask += QRegion(x, 5, y+1, h-5);
                    mask += QRegion(x+1, y+3, 1, h-2);
                    mask += QRegion(x+2, y+2, 1, h-1);
                    mask += QRegion(x+3, y+1, 2, h);
                    mask += QRegion(x+w-1, y+5, 1, h-5);
                    mask += QRegion(x+w-2, y+3, 1, h-2);
                    mask += QRegion(x+w-3, y+2, 1, h-1);
                    mask += QRegion(x+w-5, y+1, 2, h);
                }
                return mask;
            }
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
#if KDE_IS_VERSION(4, 3, 0)
    if(Handler()->customShadows() && QTC_COMPOSITING)
    {
        int shadowSize=Handler()->shadowCache().shadowSize();
        return QRect(titleLeft, r.top()+titleEdgeTop+shadowSize, titleWidth, titleHeight);
    }
#endif
    return QRect(titleLeft, r.top()+titleEdgeTop, titleWidth, titleHeight);
}

void QtCurveClient::updateCaption()
{
    QRect oldCaptionRect(itsCaptionRect);

    itsCaptionRect=QtCurveClient::captionRect();

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
    // Set note in init() above
    if(0==changed)
        widget()->setAttribute(Qt::WA_PaintOnScreen, !QTC_COMPOSITING);
#if KDE_IS_VERSION(4, 3, 85)
    if(changed & SettingCompositing)
    {
        updateWindowShape();
        widget()->update();
    }
#endif
    if (changed&(SettingColors|SettingFont|SettingBorder))
    {
        // Reset button backgrounds...
        for(int i=0; i<constNumButtonStates; ++i)
           itsButtonBackground[i].pix=QPixmap();
    }
    
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
        itsTitleFont=isToolWindow() ? Handler()->titleFontTool() : Handler()->titleFont();

        updateLayout();
        widget()->update();
    }

    if(Handler()->showResizeGrip())
        createSizeGrip();
    else
        deleteSizeGrip();

    KCommonDecoration::reset(changed);
}

void QtCurveClient::createSizeGrip()
{
    if(!itsResizeGrip && ((isResizable() && 0!=windowId()) || isPreview()))
    {
        itsResizeGrip=new QtCurveSizeGrip(this);
        itsResizeGrip->setVisible(!(isMaximized() || isShade()));
    }
}

void QtCurveClient::deleteSizeGrip()
{
    if(itsResizeGrip)
    {
        delete itsResizeGrip;
        itsResizeGrip=0L;
    }
}

}
