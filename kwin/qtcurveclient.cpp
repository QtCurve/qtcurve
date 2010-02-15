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

#include <KDE/KLocale>
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
#include <KDE/KColorUtils>
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

#if KDE_IS_VERSION(4, 3, 85)
#include <KDE/KIconLoader>
#endif

#if KDE_IS_VERSION(4, 3, 0)
    #define QTC_COMPOSITING compositingActive()
#else
    #define QTC_COMPOSITING KWindowSystem::compositingActive()
#endif

namespace KWinQtCurve
{

static const int constTitlePad=4;

#if KDE_IS_VERSION(4, 3, 85)
#define QTC_TAB_CLOSE_ICON_SIZE tabCloseIconSize(layoutMetric(LM_TitleHeight))

static inline int tabCloseIconSize(int titleHeight)
{
    int size=titleHeight*0.8;

    if(0==size%2)
        size++;
    return size;
}

#endif

#if KDE_IS_VERSION(4, 3, 0)
static QPainterPath createPath(const QRect &r, bool fullRound, bool inner=false)
{
    double       radius((fullRound ? 6.0 : 2.0) - (inner ? 1.0 : 0.0)),
                 dr(radius * 2);
    QRect        fr(inner ? r.adjusted(1, 1, -1, -1) : r);
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
    return path;
}
#endif

static QColor blendColors(const QColor &foreground, const QColor &background, double alpha)
{
    QColor col(foreground);

    col.setAlpha(255);

    return KColorUtils::mix(background, foreground, alpha);
}

QtCurveClient::QtCurveClient(KDecorationBridge *bridge, QtCurveHandler *factory)
#if KDE_IS_VERSION(4, 3, 0)
             : KCommonDecorationUnstable(bridge, factory)
#else
             : KCommonDecoration(bridge, factory)
#endif    
             , itsResizeGrip(0L)
             , itsTitleFont(QFont())
#if KDE_IS_VERSION(4, 3, 85)
             , itsClickInProgress(false)
             , itsDragInProgress(false)
             , itsMouseButton(Qt::NoButton)
             , itsTargetTab(-1)
#endif
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
    widget()->setAutoFillBackground(false);
    widget()->setAttribute(Qt::WA_OpaquePaintEvent, true);
    widget()->setAttribute(Qt::WA_NoSystemBackground);

#if KDE_IS_VERSION(4, 3, 85)
    widget()->setAcceptDrops(true);
#endif

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
    int                  rectX, rectY, rectX2, rectY2, shadowSize(0);

    painter.setClipRegion(e->region());

#if KDE_IS_VERSION(4, 3, 0)
    if(Handler()->customShadows())
    {
        if(compositing)
        {
            TileSet *tileSet(0);
    //         if(configuration().useOxygenShadows() && glowIsAnimated() && !isForcedActive())
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
        }
        shadowSize=Handler()->shadowCache().shadowSize();
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

    if(!compositing)
        painter.fillRect(r, colorTitleOnly ? windowCol : col);
    painter.setRenderHint(QPainter::Antialiasing, true);
    if(compositing)
    {
#if KDE_IS_VERSION(4, 3, 0)
        if(roundBottom)
            painter.fillPath(createPath(r, round>ROUND_SLIGHT, outerBorder), colorTitleOnly ? windowCol : col);
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

    bool showIcon=TITLEBAR_ICON_NEXT_TO_TITLE==Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleBarIcon,  0L, 0L);
    int  iconSize=showIcon ? Handler()->wStyle()->pixelMetric(QStyle::PM_SmallIconSize) : 0;

#if KDE_IS_VERSION(4, 3, 85)
    QList<ClientGroupItem> tabList  = clientGroupItems();
    const int              tabCount = tabList.count();

    // Delete unneeded tab close buttons
    while(tabCount < itsCloseButtons.size() || (1==tabCount && itsCloseButtons.size() > 0))
    {
        QtCurveButton *btn = itsCloseButtons.takeFirst();
        btn->hide();
        btn->deleteLater();
    }

    if(tabCount>1)
    {
        QRect allTabGeom = titleRect().adjusted(-1, -titleEdgeTop, 1, 0),
              tabGeom    = allTabGeom;
        int   activeTab  = visibleClientGroupItem();

        tabGeom.setWidth(tabGeom.width() / tabCount + 1); // Split titlebar evenly
        for(int i = 0; i < tabCount; ++i)
        {
            // Last tab may have a different width due to rounding
            if(i==tabCount - 1)
                tabGeom.setWidth(allTabGeom.width() - tabGeom.width() * i + i - 1);

            int iconSize(QTC_TAB_CLOSE_ICON_SIZE);

            if(0==i)
                paintSeparator(&painter, tabGeom);
            paintTitle(&painter, tabGeom.adjusted(showIcon ? constTitlePad : 0, 0,
                                                  -(iconSize+(showIcon ? constTitlePad : 0)), 0), QRect(), tabList[i].title(),
                       showIcon ? tabList[i].icon().pixmap(iconSize) : QPixmap(),  0, true, activeTab==i);

            if(i >= itsCloseButtons.size())
                itsCloseButtons.append(new QtCurveButton(ItemCloseButton, this));
            //itsCloseButtons[i]->setActive(isActive() && visibleClientGroupItem()==i);
            itsCloseButtons[i]->setFixedSize(iconSize, iconSize);
            itsCloseButtons[i]->move(tabGeom.right() - iconSize,
                                     tabGeom.top() + titleEdgeTop + ((tabGeom.height()-iconSize)/2));
            itsCloseButtons[i]->installEventFilter(this);
            itsCloseButtons[i]->show();

            QRect br(tabGeom);

            tabGeom.translate(tabGeom.width() - 1, 0);
            paintSeparator(&painter, tabGeom.adjusted(-1, 0, -1, 0));

            if(i!=activeTab)
            {
                QColor gray(Qt::black);
                gray.setAlphaF(0.1);
                painter.fillRect(br.adjusted(0==i ? 1 : 0, 0, 0, 0), gray);
            }

            if(itsDragInProgress && itsTargetTab>-1 &&
                (i==itsTargetTab || ((i==tabCount-1) && itsTargetTab==tabCount)))
            {
                QPixmap arrow(SmallIcon("arrow-down"));
                painter.drawPixmap(br.x()+(itsTargetTab==tabCount ? tabGeom.width() : 0)-(arrow.width()/2), br.y()+2, arrow);
            }
        }
    }
    else
    {
#endif
        itsCaptionRect=captionRect(); // also update itsCaptionRect!
        paintTitle(&painter, itsCaptionRect, QRect(rectX+titleEdgeLeft, itsCaptionRect.y(),
                                                   rectX2-titleEdgeRight-(rectX+titleEdgeLeft),
                                                   itsCaptionRect.height()),
                   caption(), showIcon ? icon().pixmap(iconSize) : QPixmap(), shadowSize);
#if KDE_IS_VERSION(4, 3, 85)
    }
#endif
    painter.end();
}

void QtCurveClient::paintTitle(QPainter *painter, const QRect &capRect, const QRect &alignFullRect,
                               const QString &cap, const QPixmap &pix, int shadowSize, bool isTab, bool isActiveTab)
{
    int  iconX=capRect.x();
    bool showIcon=!pix.isNull() && capRect.width()>pix.width();

    if(!cap.isEmpty())
    {
        painter->setFont(itsTitleFont);

        QFontMetrics  fm(painter->fontMetrics());
        QString       str(fm.elidedText(cap, Qt::ElideRight,
                            capRect.width()-(showIcon ? pix.width()+constTitlePad : 0), QPalette::WindowText));
        Qt::Alignment hAlign((Qt::Alignment)Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleAlignment, 0L, 0L)),
                      alignment(Qt::AlignVCenter|hAlign);
        bool          alignFull(!isTab && Qt::AlignHCenter==hAlign),
                      reverse=Qt::RightToLeft==QApplication::layoutDirection(),
                      iconRight((!reverse && alignment&Qt::AlignRight) || (reverse && alignment&Qt::AlignLeft));
        QRect         textRect(alignFull ? alignFullRect : capRect);
        int           textWidth=alignFull || (showIcon && alignment&Qt::AlignHCenter)
                                    ? fm.boundingRect(str).width()+(showIcon ? pix.width()+constTitlePad : 0) : 0;
        EEffect       effect((EEffect)(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleBarEffect)));

        if(alignFull &&
            ( ( (capRect.left()+shadowSize)>((textRect.width()-textWidth)>>1) ) ||
              (  capRect.right()<((textRect.width()+textWidth)>>1) ) ) )
        {
            alignment=Qt::AlignVCenter|Qt::AlignHCenter;
            textRect=capRect;
            hAlign=Qt::AlignLeft;
        }

        if(showIcon)
            if(alignment&Qt::AlignHCenter)
            {
                if(reverse)
                {
                    iconX=((textRect.width()-textWidth)/2.0)+0.5+textWidth+pix.width();
                    textRect.setX(textRect.x()-(pix.width()+constTitlePad));
                }
                else
                {
                    iconX=(((textRect.width()-textWidth)/2.0)+0.5)+
                            (shadowSize ? (Qt::AlignHCenter==hAlign ? shadowSize : capRect.x()) : 0)+
                            (isTab ? capRect.x() : 0);
                    textRect.setX(iconX+pix.width()+constTitlePad);
                    alignment=Qt::AlignVCenter|Qt::AlignLeft;
                }
            }
            else if((!reverse && alignment&Qt::AlignLeft) || (reverse && alignment&Qt::AlignRight))
            {
                iconX=textRect.x();
                textRect.setX(textRect.x()+(pix.width()+constTitlePad));
            }
            else if((!reverse && alignment&Qt::AlignRight) || (reverse && alignment&Qt::AlignLeft))
            {
                if(iconRight)
                {
                    iconX=textRect.x()+textRect.width()-pix.width();
                    textRect.setWidth(textRect.width()-(pix.width()+constTitlePad));
                }
                else
                {
                    iconX=textRect.x()+textRect.width()-(textWidth+constTitlePad);
                    if(iconX<textRect.x())
                        iconX=textRect.x();
                }
            }

//         painter->setClipRect(capRect.adjusted(-2, 0, 2, 0));

        QColor color(KDecoration::options()->color(KDecoration::ColorFont, isActive())),
               bgnd(KDecoration::options()->color(KDecoration::ColorTitleBar, isActive()));

        if(isTab && !isActiveTab)
            textRect.adjust(0, 1, 0, 1);

        if((!isTab || isActiveTab) && EFFECT_NONE!=effect)
        {
//             QColor shadow(WINDOW_SHADOW_COLOR(effect));
//             shadow.setAlphaF(WINDOW_TEXT_SHADOW_ALPHA(effect));
//             painter->setPen(shadow);
            painter->setPen(blendColors(WINDOW_SHADOW_COLOR(effect), bgnd, WINDOW_TEXT_SHADOW_ALPHA(effect)));
            painter->drawText(textRect.adjusted(1, 1, 1, 1), alignment, str);

            if (!isActive() && QTC_DARK_WINDOW_TEXT(color))
            {
                //color.setAlpha((color.alpha() * 180) >> 8);
                color=blendColors(color, bgnd, ((255 * 180) >> 8)/256.0);
            }
        }

//         if(isTab && !isActiveTab)
//             color.setAlphaF(0.45);
//         painter->setPen(color);
        painter->setPen(isTab && !isActiveTab ? blendColors(color, bgnd, 0.45) : color);
        painter->drawText(textRect, alignment, str);
//         painter->setClipping(false);
    }

    if(showIcon && iconX>=0)
        painter->drawPixmap(iconX, capRect.y()+((capRect.height()-pix.height())/2)+1+(isTab && !isActiveTab ? 1 : 0), pix);
}

#if KDE_IS_VERSION(4, 3, 85)
static void drawFadedLine(QPainter *painter, const QRect &r, const QColor &col)
{
    bool            aa(painter->testRenderHint(QPainter::Antialiasing));
    QPointF         start(r.x()+(aa ? 0.5 : 0.0), r.y()+(aa ? 0.5 : 0.0)),
                    end(r.x()+(aa ? 0.5 : 0.0),
                        r.y()+(r.height()-1)+(aa ? 0.5 : 0.0));
    QLinearGradient grad(start, end);
    QColor          c(col),
                    outer(Qt::white),
                    blank(Qt::white);

    c.setAlphaF(0.3);
    blank.setAlphaF(0.0);
    grad.setColorAt(0, blank);
    grad.setColorAt(0.4, c);
    grad.setColorAt(0.6, c);
    grad.setColorAt(1, blank);
    painter->setPen(QPen(QBrush(grad), 1));
    painter->drawLine(start, end);
}

void QtCurveClient::paintSeparator(QPainter *painter, const QRect &r)
{
    drawFadedLine(painter, r, Qt::white);
    drawFadedLine(painter, r.adjusted(1, 0, 1, 0), Qt::black);
    drawFadedLine(painter, r.adjusted(2, 0, 2, 0), Qt::white);
}
#endif

void QtCurveClient::updateWindowShape()
{
    if(isMaximized())
        clearMask();
    else
    {
        QRect r(Handler()->customShadows()
                    ? widget()->rect().adjusted(layoutMetric(LM_OuterPaddingLeft), layoutMetric(LM_OuterPaddingTop),
                                               -layoutMetric(LM_OuterPaddingRight), 0) // -layoutMetric(LM_OuterPaddingBottom))
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
                    mask += QRegion(x, y+5, 1, h-5);
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
    if(Handler()->customShadows())
    {
        int shadowSize=Handler()->shadowCache().shadowSize();
        return QRect(titleLeft+shadowSize, r.top()+titleEdgeTop+shadowSize, titleWidth-(shadowSize*2), titleHeight);
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

#if KDE_IS_VERSION(4, 3, 85)
    if(QtCurveButton *btn = dynamic_cast<QtCurveButton *>(o))
    {
        if(QEvent::MouseButtonPress==e->type())
            return true; // No-op
        else if(QEvent::MouseButtonRelease==e->type())
        {
            const QMouseEvent *me = static_cast<QMouseEvent *>(e);
            if(Qt::LeftButton==me->button() && btn->rect().contains(me->pos()))
                closeClientGroupItem(itsCloseButtons.indexOf(btn));
            return true;
        }
    }

    bool state = false;
    if(QEvent::MouseButtonPress==e->type())
        state = mouseButtonPressEvent(static_cast<QMouseEvent *>(e));
    else if(QEvent::MouseButtonRelease==e->type() && widget()==o)
        state = mouseButtonReleaseEvent(static_cast<QMouseEvent *>(e));
    else if(QEvent::MouseMove==e->type())
        state = mouseMoveEvent(static_cast<QMouseEvent *>(e));
    else if(QEvent::DragEnter==e->type() && widget()==o)
        state = dragEnterEvent(static_cast<QDragEnterEvent *>(e));
    else if(QEvent::DragMove==e->type() && widget()==o)
        state = dragMoveEvent(static_cast<QDragMoveEvent *>(e));
    else if(QEvent::DragLeave==e->type() && widget()==o)
        state = dragLeaveEvent(static_cast<QDragLeaveEvent *>(e));
    else if(QEvent::Drop==e->type() && widget()==o)
        state = dropEvent(static_cast<QDropEvent *>(e));

    return state || KCommonDecorationUnstable::eventFilter(o, e);
#else
    return KCommonDecoration::eventFilter(o, e);
#endif
}

#if KDE_IS_VERSION(4, 3, 85)
bool QtCurveClient::mouseButtonPressEvent(QMouseEvent *e)
{
    itsClickPoint = widget()->mapToParent(e->pos());

    int item = itemClicked(itsClickPoint);

    if(OperationsOp==buttonToWindowOperation(e->button()))
    {
        displayClientMenu(item, widget()->mapToGlobal(itsClickPoint));
        return true;
    }
    if(item >= 0)
    {
        itsClickInProgress = true;
        itsMouseButton = e->button();
        return true;
    }
    itsClickInProgress = false;
    return false;
}

bool QtCurveClient::mouseButtonReleaseEvent(QMouseEvent *e)
{
    int item = itemClicked(e->pos());

    if(itsClickInProgress && item >= 0)
    {
        itsClickInProgress = false;
        setVisibleClientGroupItem(item);
        return true;
    }
    itsClickInProgress = false;
    return false;
}

bool QtCurveClient::mouseMoveEvent(QMouseEvent *e)
{
    QPoint c    = e->pos();
    int    item = itemClicked(c);

    if(item >= 0 && itsClickInProgress && ClientGroupDragOp==buttonToWindowOperation(itsMouseButton) &&
       (c - itsClickPoint).manhattanLength() >= 4)
    {
        itsClickInProgress = false;
        itsDragInProgress = true;
    
        QDrag     *drag      = new QDrag(widget());
        QMimeData *groupData = new QMimeData();
        
        groupData->setData(clientGroupItemDragMimeType(), QString().setNum(itemId(item)).toAscii());
        drag->setMimeData(groupData);

        // Create draggable tab pixmap
        QList<ClientGroupItem> tabList  = clientGroupItems();
        const int              tabCount = tabList.count();
        QRect frame(QPoint(0, 0), widget()->frameGeometry().size()),
              titlebar(frame.topLeft(), QSize(frame.width(),
                       layoutMetric(LM_TitleEdgeTop) + layoutMetric(LM_TitleHeight) +
                       layoutMetric(LM_TitleEdgeBottom) - 1 // Titlebar and main frame overlap by 1px
                       )),
              geom = titleRect().adjusted(-1, -layoutMetric(LM_TitleEdgeTop), 1, 0);

        geom.setWidth(geom.width() / tabCount + 1); // Split titlebar evenly
        geom.translate(geom.width() * item - item, 0);
        QPixmap pix(geom.size());
        pix.fill(Qt::transparent);
        QPainter painter(&pix);

        bool  showIcon=TITLEBAR_ICON_NEXT_TO_TITLE==Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleBarIcon,  0L, 0L);
        int   iconSize=showIcon ? Handler()->wStyle()->pixelMetric(QStyle::PM_SmallIconSize) : 0;
        QRect r(0, 0, geom.size().width()-(tabList.count() ? (QTC_TAB_CLOSE_ICON_SIZE+constTitlePad) : 0), geom.size().height());

        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, true);

        QStyleOptionTitleBar opt;
        QColor               col(KDecoration::options()->color(KDecoration::ColorTitleBar, isActive()));

        opt.init(widget());
        opt.palette.setColor(QPalette::Window, col);
        opt.palette.setColor(QPalette::Button, col);
        opt.rect=r;
        opt.titleBarState=(isActive() ? QStyle::State_Active : QStyle::State_None)|QtC_StateKWin;
        opt.state|=QtC_StateKWin|QtC_StateKWinNoBorder|QtC_StateKWinTabDrag;
        Handler()->wStyle()->drawComplexControl(QStyle::CC_TitleBar, &opt, &painter, widget());
        painter.restore();
        paintTitle(&painter, r, QRect(), tabList[item].title(), showIcon ? tabList[item].icon().pixmap(iconSize) : QPixmap(), 0,
                   true, true);

        drag->setPixmap(pix);
        // If the cursor is on top of the pixmap then it makes the movement jerky on some systems
        //drag->setHotSpot(QPoint(c.x() - geom.x(), c.y() - geom.y()));
        drag->setHotSpot(QPoint(c.x() - geom.x(), -1));

        drag->exec(Qt::MoveAction);
        itsDragInProgress = false;
        if(drag->target()==0 && tabList.count() > 1)
        { // Remove window from group and move to where the cursor is located
            QPoint pos = QCursor::pos();
            frame.moveTo(pos.x() - c.x(), pos.y() - c.y());
            removeFromClientGroup(itemClicked(itsClickPoint), frame);
        }
        return true;
    }
    return false;
}

bool QtCurveClient::dragEnterEvent(QDragEnterEvent *e)
{
    if(e->mimeData()->hasFormat(clientGroupItemDragMimeType()))
    {
        itsDragInProgress = true;
        e->acceptProposedAction();
        itsTargetTab = itemClicked(widget()->mapToParent(e->pos()), true, true);
        widget()->update();
        return true;
    }
    return false;
}

bool QtCurveClient::dropEvent(QDropEvent *e)
{
    QPoint point    = widget()->mapToParent(e->pos());
    int    tabClick = itemClicked(point);

    itsDragInProgress = false;
    if(tabClick >= 0)
    {
        const QMimeData *groupData = e->mimeData();
        if(groupData->hasFormat(clientGroupItemDragMimeType()))
        {
            if(widget()==e->source())
                moveItemInClientGroup(itemClicked(itsClickPoint), itemClicked(point, true, true));
            else
                moveItemToClientGroup(QString(groupData->data(clientGroupItemDragMimeType())).toLong(), itemClicked(point, true, true));
            widget()->update();
            return true;
        }
    }
    return false;
}


bool QtCurveClient::dragMoveEvent(QDragMoveEvent *e)
{
    if(e->mimeData()->hasFormat(clientGroupItemDragMimeType()))
    {
        e->acceptProposedAction();
        int tt = itemClicked(widget()->mapToParent(e->pos()), true, true);
        if(itsTargetTab!=tt)
        {
            itsTargetTab=tt;
            widget()->update();
        }
        return true;
    }
    return false;
}

bool QtCurveClient::dragLeaveEvent(QDragLeaveEvent *)
{
    itsDragInProgress = false;
    widget()->update();
    return false;
}

int QtCurveClient::itemClicked(const QPoint &point, bool between, bool drag)
{
    QRect                  frame = widget()->frameGeometry();
    QList<ClientGroupItem> list = clientGroupItems();
    int                    tabs = list.count(),
                           shadowSize = Handler()->customShadows() ? Handler()->shadowCache().shadowSize() : 0,
                           titleX = titleRect().x()-shadowSize,
                           frameY = 0, // frame.y(),
                           titleWidth = titleRect().width()/*+(2*shadowSize)*/,
                           titleHeight = layoutMetric(LM_TitleEdgeTop) +
                                         layoutMetric(LM_TitleHeight) +
                                         layoutMetric(LM_TitleEdgeBottom) + shadowSize,
                           tabWidth = titleWidth/tabs;

    if(between) // We are inserting a new tab between two existing ones
        titleX -= tabWidth / 2;

    int rem  = titleWidth%(tabs+(drag ? 1 : 0)),
        tabX = titleX;

    for(int i = 0; i < tabs+(drag ? 1 : 0); ++i)
    {
        QRect tabRect(tabX, frameY, i<rem?tabWidth+1:tabWidth, titleHeight);

        if(tabRect.contains(point))
            return i;
        tabX += tabRect.width();
    }

    return -1;
}
#endif

void QtCurveClient::reset(unsigned long changed)
{
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
        if (maximizeMode()==MaximizeFull)
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
