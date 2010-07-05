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
#define DRAW_INTO_PIXMAPS

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
#include <QPainterPath>
#include <QLinearGradient>
#include <KDE/KColorUtils>
#include <KDE/KWindowInfo>
#include <KDE/KIconEffect>
#ifdef DRAW_INTO_PIXMAPS
#include <KDE/KWindowSystem>
#endif
#include <qdesktopwidget.h>
#include "qtcurveclient.h"
#include "qtcurvebutton.h"
#include "qtcurvetogglebutton.h"
#include "qtcurvesizegrip.h"
#include "common.h"
#if KDE_IS_VERSION(4, 3, 0)
#include "tileset.h"
#endif

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "../style/fixx11h.h"
#include <QX11Info>

#if KDE_IS_VERSION(4, 3, 85)
#include <KDE/KIconLoader>
#endif

#if KDE_IS_VERSION(4, 3, 0)
    #define COMPOSITING_ENABLED compositingActive()
#else
    #define COMPOSITING_ENABLED KWindowSystem::compositingActive()
#endif

namespace KWinQtCurve
{

static const int constTitlePad=4;

#if KDE_IS_VERSION(4, 3, 85)
#define TAB_CLOSE_ICON_SIZE tabCloseIconSize(layoutMetric(LM_TitleHeight))

static const int constInvalidTab=-1;
static const int constAddToEmpty=-2;

static inline int tabCloseIconSize(int titleHeight)
{
    int size=titleHeight*0.8;

    if(0==size%2)
        size++;
    return size;
}

#endif

static QPainterPath createPath(const QRectF &r, double radius, bool botOnly=false)
{
    double       dr(radius * 2);
    QPainterPath path;

    if(botOnly)
    {
        path.moveTo(r.right(), r.top());
        path.lineTo(r.left(), r.top());
    }
    else
    {
        path.moveTo(r.right(), r.top() + radius);
        path.arcTo(r.right() - dr, r.top(), dr, dr, 0.0, 90.0);
        path.lineTo(r.left() + radius, r.top());
        path.arcTo(r.left(), r.top(), dr, dr, 90.0, 90.0);
    }
    path.lineTo(r.left(), r.bottom() - radius);
    path.arcTo(r.left(), r.bottom() - dr, dr, dr, 180.0, 90.0);
    path.lineTo(r.right() - radius, r.bottom());
    path.arcTo(r.right() - dr, r.bottom() - dr, dr, dr,  270.0, 90.0);
    if(botOnly)
        path.lineTo(r.right(), r.top());

    return path;
}

#if KDE_IS_VERSION(4, 3, 0)
static QPainterPath createPath(const QRect &r, bool fullRound, bool inner=false, bool botOnly=false)
{
    double radius((fullRound ? 6.0 : 2.0) - (inner ? 1.0 : 0.0));
    int    adjust(botOnly ? 0 : 6);
    QRect  fr(inner ? r.adjusted(1, 1, -1, -1) : r);
    QRectF rf(fr.x(), fr.y()+adjust, fr.width(), fr.height() - adjust);

    return createPath(rf, radius, botOnly);
}
#endif

static void drawSunkenBevel(QPainter *p, const QRect &r, const QColor &bgnd, bool circular, int round)
{
    double          radius=circular
                            ? r.height()/2.0
                            : round>ROUND_FULL
                                ? 5.0
                                : round>ROUND_SLIGHT
                                    ? 3.0
                                    : 2.0;
    QPainterPath    path(createPath(QRectF(r), radius));
    QLinearGradient g(r.topLeft(), r.bottomLeft());
    QColor          black(Qt::black),
                    white(Qt::white);

    black.setAlphaF(SUNKEN_BEVEL_DARK_ALPHA(bgnd));
    white.setAlphaF(SUNKEN_BEVEL_LIGHT_ALPHA(bgnd));
    g.setColorAt(0, black);
    g.setColorAt(1, white);
    p->fillPath(path, QBrush(g));
}

static QColor blendColors(const QColor &foreground, const QColor &background, double alpha)
{
    return KColorUtils::mix(background, foreground, alpha);
}

static void drawFadedLine(QPainter *painter, const QRect &r, const QColor &col, bool horiz=false, bool fadeStart=true, bool fadeEnd=true)
{
    bool            aa(painter->testRenderHint(QPainter::Antialiasing));
    QPointF         start(r.x()+(aa ? 0.5 : 0.0), r.y()+(aa ? 0.5 : 0.0)),
                    end(r.x()+(horiz ? r.width()-1 : 0)+(aa ? 0.5 : 0.0),
                        r.y()+(horiz ? 0 : r.height()-1)+(aa ? 0.5 : 0.0));
    QLinearGradient grad(start, end);
    QColor          c(col),
                    blank(Qt::white);

    c.setAlphaF(horiz ? 0.6 : 0.3);
    blank.setAlphaF(0.0);
    grad.setColorAt(0, fadeStart ? blank : c);
    grad.setColorAt(FADE_SIZE, c);
    grad.setColorAt(1.0-FADE_SIZE, c);
    grad.setColorAt(1, fadeEnd ? blank : c);
    painter->setPen(QPen(QBrush(grad), 1));
    painter->drawLine(start, end);
}

#if KDE_IS_VERSION(4, 3, 85)
static void paintTabSeparator(QPainter *painter, const QRect &r)
{
    drawFadedLine(painter, r, Qt::white);
    drawFadedLine(painter, r.adjusted(1, 0, 1, 0), Qt::black);
    drawFadedLine(painter, r.adjusted(2, 0, 2, 0), Qt::white);
}
#endif

// static inline bool isModified(const QString &title)
// {
//     return title.indexOf(i18n(" [modified] ")) > 3 ||
//            (title.length()>3 && QChar('*')==title[0] && QChar(' ')==title[1]);
// }

QtCurveClient::QtCurveClient(KDecorationBridge *bridge, QtCurveHandler *factory)
#if KDE_IS_VERSION(4, 3, 0)
             : KCommonDecorationUnstable(bridge, factory)
#else
             : KCommonDecoration(bridge, factory)
#endif    
             , itsResizeGrip(0L)
             , itsTitleFont(QFont())
             , itsMenuBarSize(-1)
             , itsToggleMenuBarButton(0L)
             , itsToggleStatusBarButton(0L)
//              , itsHover(false)
#if KDE_IS_VERSION(4, 3, 85)
             , itsClickInProgress(false)
             , itsDragInProgress(false)
             , itsMouseButton(Qt::NoButton)
             , itsTargetTab(constInvalidTab)
#endif
{
    Handler()->addClient(this);
}

QtCurveClient::~QtCurveClient()
{
    Handler()->removeClient(this);
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
            return true;
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
            return respectWindowState && isMaximized() ? 0 : Handler()->borderSize(LM_BorderBottom==lm);
        case LM_TitleEdgeTop:
            return respectWindowState && isMaximized() ? 0 : Handler()->borderEdgeSize();
        case LM_TitleEdgeBottom:
            return respectWindowState && isMaximized() && Handler()->borderlessMax() ? 0 :  Handler()->borderEdgeSize();
        case LM_TitleEdgeLeft:
        case LM_TitleEdgeRight:
            return respectWindowState && isMaximized() ? 0 : Handler()->borderEdgeSize();
        case LM_TitleBorderLeft:
        case LM_TitleBorderRight:
            return 5;
        case LM_ButtonWidth:
        case LM_ButtonHeight:
        case LM_TitleHeight:
            return respectWindowState && isMaximized() && Handler()->borderlessMax()
                        ? 0
                        : respectWindowState && isToolWindow()
                            ? Handler()->titleHeightTool()
                            : Handler()->titleHeight();
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

#ifdef QTC_KWIN_MAX_BUTTON_HACK
static char typeToChar(ButtonType t)
{
    switch(t)
    {
        case MenuButton:
            return 'M';
        case OnAllDesktopsButton:
            return 'S';
        case HelpButton:
            return 'H';
        case MinButton:
            return 'I';
        case MaxButton:
            return 'A';
        case CloseButton:
            return 'X';
        case AboveButton:
           return 'F';
        case BelowButton:
           return 'B';
        case ShadeButton:
            return 'L';
        default:
            return '?';
    }
}
#endif

KCommonDecorationButton *QtCurveClient::createButton(ButtonType type)
{
#ifdef QTC_KWIN_MAX_BUTTON_HACK
    // If we're beng asked for a minimize button - then need to create a max button as well
    // - otherwise position changes!
    if(isMinimizable() && !isMaximizable())
    {
        QString left=options()->customButtonPositions() ? options()->titleButtonsLeft() : defaultButtonsLeft(),
                right=options()->customButtonPositions() ? options()->titleButtonsRight() : defaultButtonsRight();
        char    ch=typeToChar(type),
                mc=typeToChar(MaxButton);
        int     li=left.indexOf(ch),
                ri=-1==li ? right.indexOf(ch) : -1,
                lm=left.indexOf(mc),
                rm=-1==lm ? right.indexOf(mc) : -1,
                mod=0;
        bool    create=false,
                isLeft=false;

        if((-1!=li && lm<li) || (-1!=ri && rm<ri))
        {
            isLeft=-1!=li;
            const QString &str=isLeft ? left : right;
            int           i=isLeft ? li : ri,
                          m=isLeft ? lm : rm;

            for(m=m+1; str[m]=='_' && m<i; ++m)
                mod++;

            create=m==i;
        }

        if(create)
        {
            KCommonDecorationButton *btn = createButton(MaxButton);
            if (btn)
            {
                btn->setRealizeButtons(Qt::LeftButton|Qt::MidButton|Qt::RightButton);
                const bool max = maximizeMode()==MaximizeFull;
                btn->setTipText(max?i18n("Restore"):i18n("Maximize") );
                btn->setEnabled(false);
                // fix double deletion, see objDestroyed()
                connect(btn, SIGNAL(destroyed(QObject*)), this, SLOT(objDestroyed(QObject*)));
                KCommonDecoration::m_button[MaxButton] = btn;

                btn->setSize(QSize(layoutMetric(LM_ButtonWidth, true, btn), layoutMetric(LM_ButtonHeight, true, btn)) );
                btn->show();

                ButtonContainer &cont=isLeft ? m_buttonsLeft : m_buttonsRight;
                if(0==mod)
                    cont.append(btn);
                else
                {
                    int size=cont.size();
                    cont[size-mod]=btn;
                    cont.append(0);
                }
            }
        }
    }
#endif

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
    if (isPreview())
        itsCaption =  isActive() ? i18n("Active Window") : i18n("Inactive Window");
    else
        captionChange();
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

    informAppOfActiveChange();
    KCommonDecoration::activeChange();
}

void QtCurveClient::captionChange()
{
    itsCaption=caption();
    widget()->update();
}

void QtCurveClient::paintEvent(QPaintEvent *e)
{
    bool                 compositing=COMPOSITING_ENABLED;
    QPainter             painter(widget());
    QRect                r(widget()->rect());
    QStyleOptionTitleBar opt;
    int                  windowBorder(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_WindowBorder, 0L, 0L));
    bool                 active(isActive()),
                         colorTitleOnly(windowBorder&WINDOW_BORDER_COLOR_TITLEBAR_ONLY),
                         roundBottom(Handler()->roundBottom()),
                         outerBorder(Handler()->outerBorder()),
                         preview(isPreview()),
                         blend(!preview && Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_BlendMenuAndTitleBar, NULL, NULL)),
                         menuColor(windowBorder&WINDOW_BORDER_USE_MENUBAR_COLOR_FOR_TITLEBAR),
                         separator(active && windowBorder&WINDOW_BORDER_SEPARATOR);
    const int            border(Handler()->borderEdgeSize()),
                         titleHeight(layoutMetric(LM_TitleHeight)),
                         titleEdgeTop(layoutMetric(LM_TitleEdgeTop)),
                         titleEdgeBottom(layoutMetric(LM_TitleEdgeBottom)),
                         titleEdgeLeft(layoutMetric(LM_TitleEdgeLeft)),
                         titleEdgeRight(layoutMetric(LM_TitleEdgeRight)),
                         titleBarHeight(titleHeight+titleEdgeTop+titleEdgeBottom+(isMaximized() ? border : 0)),
                         round=Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_Round, NULL, NULL),
                         buttonFlags=Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleBarButtons, NULL, NULL);
    int                  rectX, rectY, rectX2, rectY2, shadowSize(0),
                         opacity(compositing ? Handler()->opacity(active) : 100);

    painter.setClipRegion(e->region());

#if KDE_IS_VERSION(4, 3, 0)
    if(Handler()->customShadows())
    {
        shadowSize=Handler()->shadowCache().shadowSize();

        if(compositing)
        {
            TileSet *tileSet=Handler()->shadowCache().tileSet(this, roundBottom);
            if(opacity<100)
            {
                painter.save();
                painter.setClipRegion(QRegion(r).subtract(getMask(round, r.adjusted(shadowSize, shadowSize, -shadowSize, -shadowSize))), Qt::IntersectClip);
            }
            
            if(!isMaximized())
                tileSet->render(r.adjusted(5, 5, -5, -5), &painter, TileSet::Ring);
            else if(isShade())
                tileSet->render(r.adjusted(0, 5, 0, -5), &painter, TileSet::Bottom);
            if(opacity<100)
                painter.restore();
        }
        r.adjust(shadowSize, shadowSize, -shadowSize, -shadowSize);
    }
#endif

    r.getCoords(&rectX, &rectY, &rectX2, &rectY2);

    QColor col(KDecoration::options()->color(KDecoration::ColorTitleBar, active)),
           windowCol(widget()->palette().color(QPalette::Window)),
           fillCol(colorTitleOnly ? windowCol : col);

    if(!preview && (blend||menuColor) && -1==itsMenuBarSize)
    {
        QString wc(windowClass());
        if(wc==QLatin1String("Navigator Firefox") ||
            wc==QLatin1String("Mail Thunderbird"))
            itsMenuBarSize=QFontMetrics(QApplication::font()).height()+8;
        else if(wc.startsWith(QLatin1String("VCLSalFrame.DocumentWindow OpenOffice.org")) ||
                wc.startsWith(QLatin1String("VCLSalFrame OpenOffice.org")) ||
                wc==QLatin1String("soffice.bin Soffice.bin"))
            itsMenuBarSize=QFontMetrics(QApplication::font()).height()+9;
        else
        {
            int val=getProperty();
            if(val>-1)
                itsMenuBarSize=val;
        }
    }

    if(menuColor && itsMenuBarSize>0 &&
       (active || !Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_ShadeMenubarOnlyWhenActive, NULL, NULL)))
        col=QColor(QRgb(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_MenubarColor, NULL, NULL)));

    if(opacity<100)
    {
        double alpha=opacity/100.0;
        col.setAlphaF(alpha);
        windowCol.setAlphaF(alpha);
        if(!Handler()->opaqueBorder())
            fillCol.setAlphaF(alpha);
    }

    if(isMaximized())
        painter.setClipRect(r, Qt::IntersectClip);
    else
#if KDE_IS_VERSION(4, 3, 0)
        if(!compositing && !preview)
#endif
        painter.setClipRegion(getMask(round, r), Qt::IntersectClip);

    if(!compositing)
        painter.fillRect(r, fillCol);
    painter.setRenderHint(QPainter::Antialiasing, true);
    if(compositing)
    {
#if KDE_IS_VERSION(4, 3, 0)
        if(roundBottom)
            painter.fillPath(createPath(r.adjusted(0, titleBarHeight-1, 0, 0), round>ROUND_SLIGHT, outerBorder, true),
                                        fillCol);
        else
#endif
            painter.fillRect(r.adjusted(0, titleBarHeight, 0, 0), fillCol);
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

    if(compositing && !preview)
        opt.state|=QtC_StateKWinCompositing;

    if(outerBorder)
    {
        if(opacity<100)
        {
            painter.save();
            painter.setClipRect(r.adjusted(0, titleBarHeight, 0, 0), Qt::IntersectClip);
        }
#ifdef DRAW_INTO_PIXMAPS
        if(!compositing && !preview)
        {
            // For some reason, on Jaunty drawing directly is *hideously* slow on intel graphics card!
            QPixmap pix(32, 32);
            QPainter p2(&pix);
            p2.setRenderHint(QPainter::Antialiasing, true);
            opt.rect=QRect(0, 0, pix.width(), pix.height());
            p2.fillRect(opt.rect, fillCol);
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
        if(opacity<100)
            painter.restore();

        if(Handler()->innerBorder())
        {
            QStyleOptionFrame frameOpt;
            int side(layoutMetric(LM_BorderLeft)),
                bot(layoutMetric(LM_BorderBottom));
                

            frameOpt.palette=opt.palette;
            frameOpt.rect=widget()->rect().adjusted(shadowSize+side, shadowSize+titleBarHeight, -(shadowSize+side), -(shadowSize+bot))
                                          .adjusted(-1, -1, 1, 1);
            frameOpt.state=(active ? QStyle::State_Active : QStyle::State_None)|QtC_StateKWin;
            frameOpt.lineWidth=frameOpt.midLineWidth=1;
            Handler()->wStyle()->drawPrimitive(QStyle::PE_Frame, &frameOpt, &painter, widget());
        }
    }
    else
        opt.state|=QtC_StateKWinNoBorder;

    opt.palette.setColor(QPalette::Button, col);
    opt.rect=QRect(r.x(), r.y(), r.width(), titleBarHeight);
    opt.titleBarState=(active ? QStyle::State_Active : QStyle::State_None)|QtC_StateKWin;
    
    if(!preview && blend && -1!=itsMenuBarSize)
        opt.rect.adjust(0, 0, 0, itsMenuBarSize);

#ifdef DRAW_INTO_PIXMAPS
    if(!compositing && !preview)
    {
        QPixmap  tPix(32, titleBarHeight+(!blend || itsMenuBarSize<0 ? 0 : itsMenuBarSize));
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

    if(buttonFlags&TITLEBAR_BUTTON_SUNKEN_BACKGROUND)
    {
        int hOffset=2,
            vOffset=hOffset+(outerBorder ? 1 :0),
            posAdjust=isMaximized() || outerBorder ? 2 : 0;
        
        if(buttonsLeftWidth()>(titleBarHeight-2*hOffset))
            drawSunkenBevel(&painter, QRect(r.left()+hOffset+posAdjust, r.top()+vOffset,
                                            buttonsLeftWidth()-hOffset, titleBarHeight-2*vOffset), col, buttonFlags&TITLEBAR_BUTTON_ROUND, round);
        if(buttonsRightWidth()>(titleBarHeight-2*hOffset))
            drawSunkenBevel(&painter, QRect(r.right()-(buttonsRightWidth()+posAdjust), r.top()+vOffset,
                                            buttonsRightWidth(), titleBarHeight-2*vOffset), col, buttonFlags&TITLEBAR_BUTTON_ROUND, round);
    }
    
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

            int iconSize(TAB_CLOSE_ICON_SIZE);

            if(0==i)
                paintTabSeparator(&painter, tabGeom);
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
            paintTabSeparator(&painter, tabGeom.adjusted(-1, 0, -1, 0));

            if(i!=activeTab)
            {
                QColor gray(Qt::black);
                gray.setAlphaF(0.1);
                painter.fillRect(br.adjusted(0==i ? 1 : 0, 0, 0, 0), gray);
            }

            if(itsDragInProgress && itsTargetTab>constInvalidTab &&
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
                                                   rectX2-(titleEdgeRight+rectX+titleEdgeLeft),
                                                   itsCaptionRect.height()),
                   itsCaption, showIcon ? icon().pixmap(iconSize) : QPixmap(), shadowSize);
#if KDE_IS_VERSION(4, 3, 85)

        if(constAddToEmpty==itsTargetTab)
        {
            QPixmap arrow(SmallIcon("list-add"));
            painter.drawPixmap(itsCaptionRect.x(),
                               itsCaptionRect.y()+((itsCaptionRect.height()-arrow.height())/2),
                               arrow);
        }
    }
#endif

    bool hideToggleButtons(true);
    int  toggleButtons(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_ToggleButtons, NULL, NULL));

    if(toggleButtons)
    {
        if(!itsToggleMenuBarButton && toggleButtons&0x01 && (Handler()->wasLastMenu(windowId()) || getProperty()>-1))
            itsToggleMenuBarButton=createToggleButton(true);
        if(!itsToggleStatusBarButton && toggleButtons&0x02 && (Handler()->wasLastStatus(windowId()) || getProperty(false)>-1))
            itsToggleStatusBarButton=createToggleButton(false);

    //     if(itsHover)
        {
            if(
#if KDE_IS_VERSION(4, 3, 85)
                1==tabCount &&
#endif
                active && (itsToggleMenuBarButton||itsToggleStatusBarButton))
            {
                if( (buttonsLeftWidth()+buttonsRightWidth()+constTitlePad+
                    (itsToggleMenuBarButton ? itsToggleMenuBarButton->width() : 0) +
                    (itsToggleStatusBarButton ? itsToggleStatusBarButton->width() : 0)) < r.width())
                {
                    int  align(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleAlignment, 0L, 0L));
                    bool onLeft(align&Qt::AlignRight);

                    if(align&Qt::AlignHCenter)
                    {
                        QString left=options()->customButtonPositions() ? options()->titleButtonsLeft() : defaultButtonsLeft(),
                                right=options()->customButtonPositions() ? options()->titleButtonsRight() : defaultButtonsRight();
                        onLeft=left.length()<right.length();
                    }

                    int     offset=2,
                            posAdjust=isMaximized() ? 2 : 0;
                    QRect   cr(onLeft
                                ? r.left()+buttonsLeftWidth()+posAdjust+constTitlePad+2
                                : r.right()-(buttonsRightWidth()+posAdjust+constTitlePad+2+
                                            (itsToggleMenuBarButton ? itsToggleMenuBarButton->width() : 0)+
                                            (itsToggleStatusBarButton ? itsToggleStatusBarButton->width() : 0)),
                            r.top()+offset,
                            (itsToggleMenuBarButton ? itsToggleMenuBarButton->width() : 0)+
                            (itsToggleStatusBarButton ? itsToggleStatusBarButton->width() : 0),
                            titleBarHeight-2*offset);

                    if(itsToggleMenuBarButton)
                    {
                        itsToggleMenuBarButton->move(cr.x(), r.y()+3+(outerBorder ? 2 : 0));
                        itsToggleMenuBarButton->show();
                    }
                    if(itsToggleStatusBarButton)
                    {
                        itsToggleStatusBarButton->move(cr.x()+(itsToggleMenuBarButton ? itsToggleMenuBarButton->width()+2 : 0),
                                                       r.y()+3+(outerBorder ? 2 : 0));
                        itsToggleStatusBarButton->show();
                    }
                    hideToggleButtons=false;
                }
            }
        }
    }
    if(hideToggleButtons)
    {
        if(itsToggleMenuBarButton)
            itsToggleMenuBarButton->hide();
        if(itsToggleStatusBarButton)
            itsToggleStatusBarButton->hide();
    }

    if(separator)
    {
        EEffect       effect((EEffect)(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleBarEffect)));
        QColor        color(KDecoration::options()->color(KDecoration::ColorFont, isActive())),
                      bgnd(KDecoration::options()->color(KDecoration::ColorTitleBar, isActive()));
        Qt::Alignment align((Qt::Alignment)Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleAlignment, 0L, 0L));

        r.adjust(16, titleBarHeight-1, -16, 0);
        if(EFFECT_NONE!=effect)
        {
            drawFadedLine(&painter, r,
                          blendColors(WINDOW_SHADOW_COLOR(effect), bgnd, WINDOW_TEXT_SHADOW_ALPHA(effect)), true,
                          align&(Qt::AlignHCenter|Qt::AlignRight), align&(Qt::AlignHCenter|Qt::AlignLeft));
            r.adjust(0, -1, 0, 0);
//             if (!isActive() && DARK_WINDOW_TEXT(color))
//                 color=blendColors(color, bgnd, ((255 * 180) >> 8)/256.0);
        }
        drawFadedLine(&painter, r, color, true, align&(Qt::AlignHCenter|Qt::AlignRight), align&(Qt::AlignHCenter|Qt::AlignLeft));
    }
    
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

        if(alignFull)
        {
            int halfWidth=(textWidth+(showIcon ? pix.width()+constTitlePad : 0))/2;

            if(capRect.left()>(textRect.x()+(textRect.width()/2)-halfWidth))
            {
                alignment=Qt::AlignVCenter|Qt::AlignLeft;
                textRect=capRect;
                hAlign=Qt::AlignLeft;
            }
            else if (capRect.right()<(textRect.x()+(textRect.width()/2)+halfWidth))
            {
                alignment=Qt::AlignVCenter|Qt::AlignRight;
                textRect=capRect;
                hAlign=Qt::AlignRight;
            }
        }

        if(showIcon)
        {
            if(alignment&Qt::AlignHCenter)
            {
                if(reverse)
                {
                    iconX=((textRect.width()-textWidth)/2.0)+0.5+textWidth+pix.width();
                    textRect.setX(textRect.x()-(pix.width()+constTitlePad));
                }
                else
                {
//                     iconX=(((textRect.width()-textWidth)/2.0)+0.5)+
//                             (shadowSize ? (Qt::AlignHCenter==hAlign ? shadowSize : capRect.x()) : 0)+
//                             (isTab ? capRect.x() : 0);

                    int adjustment=textRect==capRect ? capRect.x() : 0;

                    iconX=(((textRect.width()-textWidth)/2.0)+0.5)+
                            (shadowSize
                                ? (Qt::AlignHCenter==hAlign ? shadowSize : capRect.x())
                                : (isTab ? 0 : adjustment))+
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
            painter->drawText(EFFECT_SHADOW==effect
                                ? textRect.adjusted(1, 1, 1, 1)
                                : textRect.adjusted(0, 1, 0, 1), 
                              alignment, str);

            if (!isActive() && DARK_WINDOW_TEXT(color))
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
    {
//         if(isModified(cap))
//         {
//             QPixmap mod=pix;
//             KIconEffect::semiTransparent(mod);
//             painter->drawPixmap(iconX, capRect.y()+((capRect.height()-pix.height())/2)+1+(isTab && !isActiveTab ? 1 : 0), mod);
//         }
//         else
            painter->drawPixmap(iconX, capRect.y()+((capRect.height()-pix.height())/2)+1+(isTab && !isActiveTab ? 1 : 0), pix);
    }
}

void QtCurveClient::updateWindowShape()
{
    if(isMaximized())
        clearMask();
    else
    {
        QRect r(
#if KDE_IS_VERSION(4, 3, 0)
                Handler()->customShadows()
                    ? widget()->rect().adjusted(layoutMetric(LM_OuterPaddingLeft), layoutMetric(LM_OuterPaddingTop),
                                               -layoutMetric(LM_OuterPaddingRight),
                                                COMPOSITING_ENABLED ? 0 : -layoutMetric(LM_OuterPaddingBottom))
                    : 
#endif
                      widget()->rect());

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
//             if(!isPreview() && COMPOSITING_ENABLED)
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
                    mask += QRegion(x, y+5, 1, h-6);
                    mask += QRegion(x+1, y+3, 1, h-3);
                    mask += QRegion(x+2, y+2, 1, h-2);
                    mask += QRegion(x+3, y+1, 2, h-1);
                    mask += QRegion(x+w-1, y+5, 1, h-6);
                    mask += QRegion(x+w-2, y+3, 1, h-3);
                    mask += QRegion(x+w-3, y+2, 1, h-2);
                    mask += QRegion(x+w-5, y+1, 2, h-1);
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

//     if(widget()==o)
//     {
//         if(itsToggleMenuBarButton || itsToggleStatusBarButton)
//             switch(e->type())
//             {
//                 case QEvent::Enter:
//                     itsHover=true;
//                     widget()->update();
//                     break;
//                 case QEvent::Leave:
//                     itsHover=false;
//                     widget()->update();
//                 default:
//                     break;
//             }
//         return true;
//     }
    
#if KDE_IS_VERSION(4, 3, 85)
    if(Handler()->grouping())
    {
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
    }
#endif
    return KCommonDecoration::eventFilter(o, e);
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
    int item = itemClicked(widget()->mapToParent(e->pos()));

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
        QRect r(0, 0, geom.size().width()-(tabList.count() ? (TAB_CLOSE_ICON_SIZE+constTitlePad) : 0), geom.size().height());

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
        if(1==clientGroupItems().count() && widget()!=e->source())
            itsTargetTab = constAddToEmpty;
        else
            itsTargetTab = itemClicked(widget()->mapToParent(e->pos()), true, true);
        widget()->update();
        return true;
    }
    return false;
}

bool QtCurveClient::dropEvent(QDropEvent *e)
{
    QPoint point    = widget()->mapToParent(e->pos());
    int    tabClick = itemClicked(point, true, true);

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
            itsTargetTab=constInvalidTab;
            widget()->update();
            return true;
        }
    }
    else if(constInvalidTab!=itsTargetTab)
    {
        itsTargetTab=constInvalidTab;
        widget()->update();
    }
    return false;
}

bool QtCurveClient::dragMoveEvent(QDragMoveEvent *e)
{
    if(e->mimeData()->hasFormat(clientGroupItemDragMimeType()))
    {
        e->acceptProposedAction();
        int tt = 1==clientGroupItems().count() && widget()!=e->source()
                    ? constAddToEmpty
                    : itemClicked(widget()->mapToParent(e->pos()), true, true);
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
    itsTargetTab = constInvalidTab;
    widget()->update();
    return false;
}

int QtCurveClient::itemClicked(const QPoint &point, bool between, bool drag)
{
    QRect                  title = titleRect();
    QList<ClientGroupItem> list = clientGroupItems();
    int                    tabs = list.count(),
                           shadowSize = Handler()->customShadows() ? Handler()->shadowCache().shadowSize() : 0,
                           titleX = title.x()-shadowSize,
                           frameY = 0,
                           titleWidth = title.width(),
                           titleHeight = layoutMetric(LM_TitleEdgeTop) +
                                         layoutMetric(LM_TitleHeight) +
                                         layoutMetric(LM_TitleEdgeBottom) + shadowSize,
                           tabWidth = titleWidth/tabs;

    if(drag)
    {
        if(point.x() <= title.left())
            return 0;
        else if(point.x() >= titleRect().right())
            return tabs;
    }

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

    return constInvalidTab;
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
    {
        if (maximizeMode()==MaximizeFull)
        {
            if (!options()->moveResizeMaximizedWindows() && itsResizeGrip)
                itsResizeGrip->hide();
        }
        else if (itsResizeGrip)
                itsResizeGrip->show();
    }

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

void QtCurveClient::informAppOfTitlebarSizeChanged()
{
    static const Atom constQtCTitleBarSize = XInternAtom(QX11Info::display(), TITLEBAR_SIZE_ATOM, False);

    QX11Info info;
    XEvent xev;
    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = constQtCTitleBarSize;
    xev.xclient.display = QX11Info::display();
    xev.xclient.window = windowId();
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 0;
    XSendEvent(QX11Info::display(), windowId(), False, NoEventMask, &xev);
}

void QtCurveClient::informAppOfActiveChange()
{
    if(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_ShadeMenubarOnlyWhenActive, NULL, NULL))
    {
        static const Atom constQtCActiveWindow = XInternAtom(QX11Info::display(), ACTIVE_WINDOW_ATOM, False);

        QX11Info info;
        XEvent xev;
        xev.xclient.type = ClientMessage;
        xev.xclient.message_type = constQtCActiveWindow;
        xev.xclient.display = QX11Info::display();
        xev.xclient.window = windowId();
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = isActive() ? 1 : 0;
        XSendEvent(QX11Info::display(), windowId(), False, NoEventMask, &xev);
    }
}

void QtCurveClient::sendToggleToApp(bool menubar)
{
    //if(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_ShadeMenubarOnlyWhenActive, NULL, NULL))
    {
        static const Atom constQtCToggleMenuBar   = XInternAtom(QX11Info::display(), TOGGLE_MENUBAR_ATOM, False);
        static const Atom constQtCToggleStatusBar = XInternAtom(QX11Info::display(), TOGGLE_STATUSBAR_ATOM, False);

        QX11Info info;
        XEvent xev;
        xev.xclient.type = ClientMessage;
        xev.xclient.message_type = menubar ? constQtCToggleMenuBar : constQtCToggleStatusBar;
        xev.xclient.display = QX11Info::display();
        xev.xclient.window = windowId();
        xev.xclient.format = 32;
        xev.xclient.data.l[0]=0;
        XSendEvent(QX11Info::display(), windowId(), False, NoEventMask, &xev);
        if(menubar)
            Handler()->emitToggleMenuBar(windowId());
        else
            Handler()->emitToggleStatusBar(windowId());
    }
}

const QString & QtCurveClient::windowClass(bool normalWindowsOnly)
{
    if(itsWindowClass.isEmpty())
    {
        KWindowInfo info(windowId(), normalWindowsOnly ? NET::WMWindowType : 0, NET::WM2WindowClass);

        if(normalWindowsOnly && NET::Normal!=info.windowType(NET::AllTypesMask))
            itsWindowClass="<>";
        else
            itsWindowClass=info.windowClassName()+' '+info.windowClassClass();
    }

    return itsWindowClass;
}

void QtCurveClient::menuBarSize(int size)
{
    itsMenuBarSize=size;
    if(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_ToggleButtons, NULL, NULL) &0x01)
    {
        if(!itsToggleMenuBarButton)
            itsToggleMenuBarButton=createToggleButton(true);
        //if(itsToggleMenuBarButton)
        //    itsToggleMenuBarButton->setChecked(itsMenuBarSize>0);
    }
    KCommonDecoration::activeChange();
}

void QtCurveClient::statusBarState(bool state)
{
    Q_UNUSED(state)
    if(Handler()->wStyle()->pixelMetric((QStyle::PixelMetric)QtC_ToggleButtons, NULL, NULL) &0x02)
    {
        if(!itsToggleStatusBarButton)
            itsToggleStatusBarButton=createToggleButton(false);
        //if(itsToggleStatusBarButton)
        //    itsToggleStatusBarButton->setChecked(state);
    }
    KCommonDecoration::activeChange();
}

void QtCurveClient::toggleMenuBar()
{
    sendToggleToApp(true);
}

void QtCurveClient::toggleStatusBar()
{
    sendToggleToApp(false);
}
    
QtCurveToggleButton * QtCurveClient::createToggleButton(bool menubar)
{
    QtCurveToggleButton *button = new QtCurveToggleButton(menubar, this);
    int                 size    = layoutMetric(LM_TitleHeight)-6;

    button->setFixedSize(size, size);
    //button->setCheckable(true);
    //button->setChecked(false);
    connect(button, SIGNAL(clicked()), menubar ? SLOT(toggleMenuBar()) : SLOT(toggleStatusBar()));
//     widget()->setAttribute(Qt::WA_Hover, true);
//     widget()->installEventFilter(this);
    return button;
}

int QtCurveClient::getProperty(bool menubar)
{
    static const Atom constQtcMenuSize  = XInternAtom(QX11Info::display(), MENU_SIZE_ATOM, False);
    static const Atom constQtcStatusBar = XInternAtom(QX11Info::display(), STATUSBAR_ATOM, False);

    unsigned char *data;
    int           dummy;
    unsigned long num,
                  dummy2;
    int           rv(-1);

    if (Success==XGetWindowProperty(QX11Info::display(), windowId(), menubar ? constQtcMenuSize : constQtcStatusBar, 0L, 1, False,
                                    XA_CARDINAL, &dummy2, &dummy, &num, &dummy2, &data) && num>0)
    {
        unsigned short val=*((unsigned short*)data);

        if(val<512)
            rv=val;
        XFree(data);
    }
    //else
    //    *data = NULL; // superflous?!?
    return rv;
}

}
