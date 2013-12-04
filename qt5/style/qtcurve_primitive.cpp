/*****************************************************************************
 *   Copyright 2007 - 2010 Craig Drummond <craig.p.drummond@gmail.com>       *
 *   Copyright 2013 - 2013 Yichao Yu <yyc1992@gmail.com>                     *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU Lesser General Public License as          *
 *   published by the Free Software Foundation; either version 2.1 of the    *
 *   License, or (at your option) version 3, or any later version accepted   *
 *   by the membership of KDE e.V. (or its successor approved by the         *
 *   membership of KDE e.V.), which shall act as a proxy defined in          *
 *   Section 6 of version 3 of the license.                                  *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 *   Lesser General Public License for more details.                         *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with this library. If not,                                *
 *   see <http://www.gnu.org/licenses/>.                                     *
 *****************************************************************************/

#include "qtcurve_p.h"
#include <qtcurve-utils/qtutils.h>

#include <QMdiSubWindow>
#include <QTreeView>
#include <QSpinBox>
#include <QComboBox>
#include <QMainWindow>
#include <QListView>
#include <QPixmapCache>
#include <QDockWidget>

#include "shadowhelper.h"
#include "utils.h"
#include <common/config_file.h>

namespace QtCurve {

bool
Style::drawPrimitivePanelMenu(PrimitiveElement element,
                              const QStyleOption *option,
                              QPainter *painter,
                              const QWidget *widget) const
{
    QTC_UNUSED(element);
    QTC_UNUSED(option);
    QTC_UNUSED(painter);
    QTC_UNUSED(widget);
    return true;
}

bool
Style::drawPrimitiveIndicatorTabClose(PrimitiveElement element,
                                      const QStyleOption *option,
                                      QPainter *painter,
                                      const QWidget *widget) const
{
#ifdef QTC_QT5_ENABLE_KDE
    int size = pixelMetric(QStyle::PM_SmallIconSize);
    State state = option->state;
    QIcon::Mode mode = (state & State_Enabled ? state & State_Raised ?
                        QIcon::Active : QIcon::Normal : QIcon::Disabled);
    if (!(state & State_Raised) && !(state & State_Sunken) &&
        !(state & State_Selected))
        mode = QIcon::Disabled;
    drawItemPixmap(painter, option->rect, Qt::AlignCenter,
                   KIcon("dialog-close").pixmap(size, mode,
                                                state & State_Sunken ?
                                                QIcon::On : QIcon::Off));
    return true;
#else
    QTC_UNUSED(element);
    QTC_UNUSED(option);
    QTC_UNUSED(painter);
    QTC_UNUSED(widget);
    return false;
#endif
}

bool
Style::drawPrimitiveWidget(PrimitiveElement element,
                           const QStyleOption *option,
                           QPainter *painter,
                           const QWidget *widget) const
{
    QTC_UNUSED(element);
    QTC_UNUSED(option);
    if (widget && widget->testAttribute(Qt::WA_StyledBackground) &&
        ((!widget->testAttribute(Qt::WA_NoSystemBackground) &&
          (qtcIsDialog(widget) || qtcIsWindow(widget))) ||
         (qobject_cast<const QMdiSubWindow*>(widget)))) {
        bool isDialog = qtcIsDialog(widget);
        if (qtcIsCustomBgnd(&opts) || itsIsPreview ||
            (isDialog && opts.dlgOpacity != 100) ||
            (!isDialog && opts.bgndOpacity != 100)) {
            drawBackground(painter, widget,
                           isDialog ? BGND_DIALOG : BGND_WINDOW);
        }
    }
    return true;
}

bool
Style::drawPrimitivePanelScrollAreaCorner(PrimitiveElement element,
                                          const QStyleOption *option,
                                          QPainter *painter,
                                          const QWidget *widget) const
{
    QTC_UNUSED(element);
    // disable painting of PE_PanelScrollAreaCorner
    // the default implementation fills the rect with the window background
    // color which does not work for windows that have gradients.
    // ...but need to for WebView!!!
    if (!opts.gtkScrollViews || !qtcIsCustomBgnd(&opts) ||
        (widget && widget->inherits("WebView"))) {
        painter->fillRect(option->rect,
                          option->palette.brush(QPalette::Window));
    }
    return true;
}

bool
Style::drawPrimitiveIndicatorBranch(PrimitiveElement element,
                                    const QStyleOption *option,
                                    QPainter *painter,
                                    const QWidget *widget) const
{
    QTC_UNUSED(element);
    bool reverse = option->direction == Qt::RightToLeft;
    const QRect &r = option->rect;
    const QPalette &palette = option->palette;
    State state = option->state;
    int middleH = (r.x() + r.width() / 2) - 1;
    int middleV = r.y() + r.height() / 2;
    int beforeV = middleV;
    int afterH = middleH;
#if 0
    int beforeH = middleH;
#endif
    int afterV = middleV;
    painter->save();
    if (state & State_Children) {
        QRect ar(r.x() + (r.width() - LV_SIZE - 4) / 2,
                 r.y() + (r.height() - LV_SIZE - 4) / 2,
                 LV_SIZE + 4, LV_SIZE + 4);
        if (/*LV_OLD == */opts.lvLines) {
            beforeV = ar.y() - 1;
            afterH = ar.x() + LV_SIZE + 4;
            afterV = ar.y() + LV_SIZE + 4;
#if 0
            beforeH = ar.x();
            int lo(ROUNDED ? 2 : 0);

            painter->setPen(palette.mid().color());
            painter->drawLine(ar.x() + lo, ar.y(), ar.x() + ar.width() - 1 - lo,
                              ar.y());
            painter->drawLine(ar.x() + lo, ar.y() + ar.height() - 1,
                              ar.x() + ar.width() - 1 - lo,
                              ar.y() + ar.height() - 1);
            painter->drawLine(ar.x(), ar.y() + lo, ar.x(),
                              ar.y() + ar.height() - 1 - lo);
            painter->drawLine(ar.x() + ar.width() - 1, ar.y() + lo,
                              ar.x() + ar.width() - 1,
                              ar.y() + ar.height() - 1 - lo);
            if (ROUNDED) {
                painter->drawPoint(ar.x() + 1, ar.y() + 1);
                painter->drawPoint(ar.x() + 1, ar.y() + ar.height() - 2);
                painter->drawPoint(ar.x() + ar.width() - 2, ar.y() + 1);
                painter->drawPoint(ar.x() + ar.width() - 2,
                                   ar.y() + ar.height() - 2);

                QColor col(palette.mid().color());
                col.setAlphaF(0.5);
                painter->setPen(col);
                painter->drawLine(ar.x() + 1, ar.y() + 1, ar.x() + 2, ar.y());
                painter->drawLine(ar.x() + ar.width() - 2, ar.y(),
                                  ar.x() + ar.width() - 1, ar.y() + 1);
                painter->drawLine(ar.x() + 1, ar.y() + ar.height() - 2,
                                  ar.x() + 2, ar.y() + ar.height() - 1);
                painter->drawLine(ar.x() + ar.width() - 2,
                                  ar.y() + ar.height() - 1,
                                  ar.x() + ar.width() - 1,
                                  ar.y() + ar.height() - 2);
            }
#endif
        }
        drawArrow(painter, ar, state&State_Open ? PE_IndicatorArrowDown :
                  reverse ? PE_IndicatorArrowLeft :
                  PE_IndicatorArrowRight,
                  MOArrow(state, palette, QPalette::ButtonText));
    }
    const int constStep = (/*LV_OLD==*/opts.lvLines ? 0 :
                           widget && qobject_cast<const QTreeView*>(widget) ?
                           ((QTreeView*)widget)->indentation() : 20);

    if (opts.lvLines
        /*&& (LV_OLD==opts.lvLines || (r.x()>=constStep && constStep>0))*/) {
        painter->setPen(palette.mid().color());
        if (state & State_Item) {
            if (reverse) {
                painter->drawLine(r.left(), middleV, afterH, middleV);
            } else {
#if 0
                if (LV_NEW == opts.lvLines) {
                    if (state & State_Children) {
                        painter->drawLine(middleH - constStep, middleV,
                                          r.right() - constStep, middleV);
                    } else {
                        drawFadedLine(painter,
                                      QRect(middleH - constStep, middleV,
                                            r.right() - (middleH - constStep),
                                            middleV), palette.mid().color(),
                                      false, true, true);
                    }
                } else {
#endif
                    painter->drawLine(afterH, middleV, r.right(), middleV);
#if 0
                }
#endif
            }
        }
        if (state & State_Sibling && afterV < r.bottom())
            painter->drawLine(middleH - constStep, afterV,
                              middleH - constStep, r.bottom());
        if (state & (State_Open | State_Children | State_Item |
                     State_Sibling) &&
            (/*LV_NEW==opts.lvLines || */beforeV > r.y())) {
            painter->drawLine(middleH-constStep, r.y(),
                              middleH - constStep, beforeV);
        }
    }
    painter->restore();
    return true;
}

bool
Style::drawPrimitiveIndicatorViewItemCheck(PrimitiveElement element,
                                           const QStyleOption *option,
                                           QPainter *painter,
                                           const QWidget *widget) const
{
    QTC_UNUSED(element);
    QStyleOption opt = *option;
    opt.state &= ~State_MouseOver;
    opt.state |= STATE_VIEW;
    drawPrimitive(PE_IndicatorCheckBox, &opt, painter, widget);
    return true;
}

bool
Style::drawPrimitiveIndicatorHeaderArrow(PrimitiveElement element,
                                         const QStyleOption *option,
                                         QPainter *painter,
                                         const QWidget *widget) const
{
    QTC_UNUSED(element);
    QTC_UNUSED(widget);
    State state = option->state;
    const QPalette &palette = option->palette;
    if (const QStyleOptionHeader *header =
        qstyleoption_cast<const QStyleOptionHeader*>(option)) {
        drawArrow(painter, option->rect,
                  header->sortIndicator & QStyleOptionHeader::SortUp ?
                  PE_IndicatorArrowUp : PE_IndicatorArrowDown,
                  MOArrow(state, palette, QPalette::ButtonText));
    }
    return true;
}

bool
Style::drawPrimitiveIndicatorArrow(PrimitiveElement element,
                                   const QStyleOption *option,
                                   QPainter *painter,
                                   const QWidget *widget) const
{
    State state = option->state;
    QRect r = option->rect;
    const QPalette &palette = option->palette;
    if (state == State_None)
        state |= State_Enabled;
    if (state == (State_Enabled | QtC_StateKWin)) {
        drawArrow(painter, r, element, Qt::color1, false, true);
    } else {
        QColor col = MOArrow(state, palette, QPalette::Text);
        if (state & (State_Sunken | State_On) &&
            !(widget &&
              ((opts.unifySpin && qobject_cast<const QSpinBox*>(widget)) ||
               (opts.unifyCombo && qobject_cast<const QComboBox*>(widget) &&
                static_cast<const QComboBox*>(widget)->isEditable())))) {
            r.adjust(1, 1, 1, 1);
        }
        if (col.alpha() < 255 &&
            element == PE_IndicatorArrowRight &&
            widget && widget->inherits("KUrlButton")) {
            col = blendColors(col, palette.background().color(), col.alphaF());
        }
        drawArrow(painter, r, element, col, false, false);
    }
    return true;
}

bool
Style::drawPrimitiveIndicatorSpin(PrimitiveElement element,
                                   const QStyleOption *option,
                                   QPainter *painter,
                                   const QWidget *widget) const
{
    const QRect &r = option->rect;
    const QPalette &palette = option->palette;
    State state = option->state;
    QRect sr = r;
    const QColor *use = buttonColors(option);
    const QColor &col = MOArrow(state, palette, QPalette::ButtonText);
    bool down = (element == PE_IndicatorSpinDown ||
                 element == PE_IndicatorSpinMinus);
    bool reverse = option->direction == Qt::RightToLeft;

    if ((!opts.unifySpinBtns || state & State_Sunken) && !opts.unifySpin) {
        drawLightBevel(painter, sr, option, widget, down ? reverse ?
                       ROUNDED_BOTTOMLEFT : ROUNDED_BOTTOMRIGHT : reverse ?
                       ROUNDED_TOPLEFT : ROUNDED_TOPRIGHT,
                       getFill(option, use), use, true, WIDGET_SPIN);
    }
    if (element == PE_IndicatorSpinUp || element ==PE_IndicatorSpinDown) {
        sr.setY(sr.y() + (down ? -2 : 1));
        if (opts.unifySpin) {
            sr.adjust(reverse ? 1 : -1, 0, reverse ? 1 : -1, 0);
            if (!opts.vArrows) {
                sr.setY(sr.y() + (down ? -2 : 2));
            }
        } else if (state & State_Sunken) {
            sr.adjust(1, 1, 1, 1);
        }
        drawArrow(painter, sr, element == PE_IndicatorSpinUp ?
                  PE_IndicatorArrowUp : PE_IndicatorArrowDown,
                  col, !opts.unifySpin);
    } else {
        int l = qMin(r.width() - 6, r.height() - 6);
        QPoint c(r.x() + r.width() / 2, r.y() + r.height() / 2);
        l /= 2;
        if (l % 2 != 0) {
            --l;
        }
        if (state & State_Sunken && !opts.unifySpin)
            c += QPoint(1, 1);
        painter->setPen(col);
        painter->drawLine(c.x() - l, c.y(), c.x() + l, c.y());
        if (!down) {
            painter->drawLine(c.x(), c.y() - l, c.x(), c.y() + l);
        }
    }
    return true;
}

bool
Style::drawPrimitiveIndicatorToolBarSeparator(PrimitiveElement element,
                                              const QStyleOption *option,
                                              QPainter *painter,
                                              const QWidget *widget) const
{
    QTC_UNUSED(element);
    QTC_UNUSED(widget);
    const QRect &r = option->rect;
    State state = option->state;
    painter->save();
    switch (opts.toolbarSeparators) {
    case LINE_NONE:
        break;
    case LINE_FLAT:
    case LINE_SUNKEN:
        if (r.width() < r.height()) {
            int x = r.x() + (r.width() - 2) / 2;
            drawFadedLine(painter, QRect(x, r.y() + TOOLBAR_SEP_GAP, 1,
                                         r.height() - TOOLBAR_SEP_GAP * 2),
                          itsBackgroundCols[opts.toolbarSeparators ==
                                            LINE_SUNKEN ? 3 : 4],
                          true, true, false);

            if (opts.toolbarSeparators == LINE_SUNKEN) {
                drawFadedLine(painter, QRect(x + 1, r.y() + 6, 1,
                                             r.height() - 12),
                              itsBackgroundCols[0], true, true, false);
            }
        } else {
            int y = r.y() + (r.height() - 2) / 2;
            drawFadedLine(painter, QRect(r.x() + TOOLBAR_SEP_GAP, y,
                                         r.width() - TOOLBAR_SEP_GAP * 2, 1),
                          itsBackgroundCols[opts.toolbarSeparators ==
                                            LINE_SUNKEN ? 3 : 4],
                          true, true, true);
            if (opts.toolbarSeparators == LINE_SUNKEN) {
                drawFadedLine(painter,
                              QRect(r.x() + TOOLBAR_SEP_GAP, y + 1,
                                    r.width() - TOOLBAR_SEP_GAP * 2, 1),
                              itsBackgroundCols[0], true, true, true);
            }
        }
        break;
    default:
    case LINE_DOTS:
        drawDots(painter, r, !(state & State_Horizontal), 1, 5,
                 itsBackgroundCols, 0, 5);
    }
    painter->restore();
    return true;
}

bool
Style::drawPrimitiveFrameGroupBox(PrimitiveElement element,
                                  const QStyleOption *option,
                                  QPainter *painter,
                                  const QWidget *widget) const
{
    QTC_UNUSED(element);
    if (opts.groupBox == FRAME_NONE)
        return true;
    bool reverse = Qt::RightToLeft == option->direction;
    QRect r = option->rect;
    if (const QStyleOptionFrame *_frame =
        qstyleoption_cast<const QStyleOptionFrame*>(option)) {
        QStyleOptionFrame frame(*_frame);
        if (frame.features & QStyleOptionFrame::Flat ||
            opts.groupBox == FRAME_LINE) {
            drawFadedLine(painter, QRect(r.x(), r.y(), r.width(), 1),
                          backgroundColors(option)[QTC_STD_BORDER],
                          opts.gbLabel & GB_LBL_CENTRED || reverse,
                          opts.gbLabel & GB_LBL_CENTRED || !reverse, true);
        } else {
            if (opts.gbLabel & GB_LBL_OUTSIDE) {
                r.adjust(0, 2, 0, 0);
            }
            if (opts.groupBox == FRAME_SHADED ||
                opts.groupBox == FRAME_FADED) {
                int round = (opts.square & SQUARE_FRAME ?
                             ROUNDED_NONE : ROUNDED_ALL);
                QPainterPath path =
                    buildPath(r, WIDGET_FRAME, round,
                              ROUNDED_ALL == round ?
                              qtcGetRadius(&opts, r.width(), r.height(),
                                           WIDGET_FRAME,
                                           RADIUS_EXTERNAL) : 0.0);
                painter->save();
                painter->setClipping(false);
                if (opts.gbFactor != 0) {
                    QColor col = opts.gbFactor < 0 ? Qt::black : Qt::white;

                    col.setAlphaF(TO_ALPHA(opts.gbFactor));
                    if (FRAME_SHADED == opts.groupBox) {
                        painter->fillPath(path, col);
                    } else {
                        QLinearGradient grad(r.topLeft(), r.bottomLeft());
                        grad.setColorAt(0, col);
                        col.setAlphaF(0.0);
                        grad.setColorAt(1, col);
                        painter->fillPath(path, grad);
                    }
                }

                if (!(opts.gbLabel & (GB_LBL_INSIDE | GB_LBL_OUTSIDE)))
                    painter->restore();
                if (FRAME_SHADED == opts.groupBox) {
                    drawBorder(painter, r, option, round,
                               backgroundColors(option), WIDGET_FRAME,
                               /* state & State_Raised && opts.gbFactor < 0 ?
                                  BORDER_RAISED : */BORDER_SUNKEN);
                } else {
                    QColor col = backgroundColors(option)[QTC_STD_BORDER];
                    QLinearGradient grad(r.topLeft(), r.bottomLeft());
                    col.setAlphaF(1.0);
                    grad.setColorAt(0, col);
                    col.setAlphaF(0.0);
                    grad.setColorAt(1, col);
                    painter->setRenderHint(QPainter::Antialiasing, true);
                    painter->setPen(QPen(QBrush(grad), 1));
                    painter->drawPath(path);
                }
                if (opts.gbLabel & (GB_LBL_INSIDE|GB_LBL_OUTSIDE)) {
                    painter->restore();
                }
            } else {
                frame.state &= ~(State_Sunken | State_HasFocus);
                frame.rect = r;
                drawPrimitive(PE_Frame, &frame, painter, widget);
            }
        }
    }
    return true;
}

bool
Style::drawPrimitiveFrame(PrimitiveElement element,
                          const QStyleOption *option, QPainter *painter,
                          const QWidget *widget) const
{
    QTC_UNUSED(element);
    QRect r = option->rect;
    const QPalette &palette(option->palette);
    State state = option->state;

    // Dont draw OO.o status bar frames...
    if (isOOWidget(widget) && r.height() < 22)
        return true;
    if (widget && qtcCheckKDEType0(widget->parent(), KTitleWidget)) {
        return true;
    } else if (widget && qtcCheckType0<QComboBox>(widget->parent())) {
        if (opts.gtkComboMenus &&
            !((QComboBox*)(widget->parent()))->isEditable()) {
            drawPrimitive(PE_FrameMenu, option, painter, widget);
        } else if (opts.square & SQUARE_POPUP_MENUS) {
            const QColor *use = (APP_KRUNNER == theThemedApp ?
                                 itsBackgroundCols : backgroundColors(option));
            painter->save();
            painter->setPen(use[QTC_STD_BORDER]);
            drawRect(painter, r);
            painter->setPen(palette.base().color());
            drawRect(painter, r.adjusted(1, 1, -1, -1));
            painter->restore();
        }
    } else {
        const QStyleOptionFrame *fo =
            qstyleoption_cast<const QStyleOptionFrame*>(option);
        if (theThemedApp == APP_K3B &&
            !(state & (State_Sunken | State_Raised)) &&
            fo && fo->lineWidth == 1) {
            painter->save();
            painter->setPen(backgroundColors(option)[QTC_STD_BORDER]);
            drawRect(painter, r);
            painter->restore();
        } else if ((state == QtC_StateKWin ||
                    state == (QtC_StateKWin | State_Active)) && fo &&
                   fo->lineWidth == 1 && fo->midLineWidth == 1) {
            QColor border;
            if (fo->version == TBAR_BORDER_VERSION_HACK + 2) {
                border = palette.color(QPalette::Active, QPalette::Shadow);
            } else {
                const QColor *borderCols =
                    (opts.windowBorder & WINDOW_BORDER_COLOR_TITLEBAR_ONLY ?
                     backgroundColors(palette.color(QPalette::Active,
                                                    QPalette::Window)) :
                     theThemedApp == APP_KWIN ? buttonColors(option) :
                     getMdiColors(option, state & State_Active));
                border = borderCols[fo->version == TBAR_BORDER_VERSION_HACK ?
                                    0 : QTC_STD_BORDER];
            }
            border.setAlphaF(1.0);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->setPen(border);
            drawRect(painter, r);
            painter->restore();
        } else {
            bool kateView = isKateView(widget);
            bool kontactPreview = !kateView && isKontactPreviewPane(widget);
            bool sv = (isOOWidget(widget) ||
                       qobject_cast<const QAbstractScrollArea*>(widget) ||
                       ((opts.square & SQUARE_SCROLLVIEW) &&
                        (kateView || kontactPreview)));
            bool squareSv = (sv && ((opts.square & SQUARE_SCROLLVIEW) ||
                                    (widget && widget->isWindow())));
            bool inQAbstractItemView =
                (widget && widget->parentWidget() &&
                 isInQAbstractItemView(widget->parentWidget()));

            if (sv && (opts.etchEntry || squareSv || isOOWidget(widget))) {
                // For some reason, in KPackageKit, the KTextBrower when
                // polished is not in the scrollview, but is when painted.
                // So check here if it should not be etched.
                // Also, see not in getLowerEtchCol()
                if (DO_EFFECT && !USE_CUSTOM_ALPHAS(opts) && widget &&
                    widget->parentWidget() &&
                    !theNoEtchWidgets.contains(widget) &&
                    inQAbstractItemView) {
                    theNoEtchWidgets.insert(widget);
                }
                // If we are set to have sunken scrollviews, then the frame
                // width is set to 3. ...but it we are a scrollview within
                // a scrollview, then we dont draw sunken, therefore
                // need to draw inner border...
                bool doEtch = DO_EFFECT && opts.etchEntry;
                bool noEtchW = (doEtch && !USE_CUSTOM_ALPHAS(opts) &&
                                theNoEtchWidgets.contains(widget));
                if (doEtch && noEtchW) {
                    painter->setPen(palette.brush(QPalette::Base).color());
                    drawRect(painter, r.adjusted(2, 2, -2, -2));
                }
                if (!opts.highlightScrollViews && fo) {
                    QStyleOptionFrame opt(*fo);
                    opt.state &= ~State_HasFocus;
                    drawEntryField(painter, r, widget, &opt,
                                   squareSv ? ROUNDED_NONE : ROUNDED_ALL,
                                   false, doEtch && !noEtchW,
                                   WIDGET_SCROLLVIEW);
                } else {
                    drawEntryField(painter, r, widget, option,
                                   squareSv ? ROUNDED_NONE : ROUNDED_ALL, false,
                                   doEtch && !noEtchW, WIDGET_SCROLLVIEW);
                }
            }
            // K3b's Disk usage status bar, etc...
            // else if (APP_K3B == theThemedApp && widget &&
            //          widget->inherits("K3b::FillStatusDisplay"))
            else if (fo && fo->lineWidth > 0) {
                bool kwinTab = (theThemedApp == APP_KWIN && widget &&
                                !widget->parentWidget() &&
                                !strcmp(widget->metaObject()->className(),
                                        "KWin::TabBox"));
                QStyleOption opt = *option;
                painter->save();
                if (kwinTab) {
                    r.adjust(-1, -1, 1, 1);
                }
                if (!opts.highlightScrollViews) {
                    opt.state &= ~State_HasFocus;
                }
                if (opts.round && qtcIsFlatBgnd(opts.bgndAppearance) &&
                    opts.bgndOpacity == 100 && widget &&
                    widget->parentWidget() && !inQAbstractItemView//  &&
                    // widget->palette().background().color() !=
                    // widget->parentWidget()->palette().background().color()
                    ) {
                    painter->setPen(widget->parentWidget()->palette()
                                    .background().color());
                    painter->drawRect(r);
                    painter->drawRect(r.adjusted(1, 1, -1, -1));
                }
                if (sv || kateView || kontactPreview) {
                    painter->setRenderHint(QPainter::Antialiasing, true);
                    painter->setPen(
                        option->palette.brush(
                            opts.thin & THIN_FRAMES &&
                            !(opts.square & SQUARE_SCROLLVIEW) ?
                            QPalette::Window : QPalette::Base).color());
                    painter->drawPath(
                        buildPath(r.adjusted(1, 1, -1, -1),
                                  WIDGET_SCROLLVIEW, ROUNDED_ALL,
                                  qtcGetRadius(&opts, r.width() - 2,
                                               r.height() - 2,
                                               WIDGET_SCROLLVIEW,
                                               RADIUS_INTERNAL)));
                    painter->setRenderHint(QPainter::Antialiasing, false);
                }
                drawBorder(painter, r, &opt,
                           opts.round ?  getFrameRound(widget) : ROUND_NONE,
                           backgroundColors(option),
                           sv || kateView || kontactPreview ?
                           WIDGET_SCROLLVIEW : WIDGET_FRAME,
                           state & State_Sunken || state & State_HasFocus ?
                           BORDER_SUNKEN : state & State_Raised ? BORDER_RAISED :
                           BORDER_FLAT);
                painter->restore();
            }
        }
    }
    return true;
}

bool
Style::drawPrimitivePanelMenuBar(PrimitiveElement element,
                                 const QStyleOption *option,
                                 QPainter *painter,
                                 const QWidget *widget) const
{
    QTC_UNUSED(element);
    const QRect &r = option->rect;
    if (widget && widget->parentWidget() &&
        qobject_cast<const QMainWindow*>(widget->parentWidget())) {
        painter->save();
        drawMenuOrToolBarBackground(widget, painter, r, option);
        if (opts.toolbarBorders != TB_NONE) {
            const QColor *use = (itsActive ? itsMenubarCols :
                                 backgroundColors(option));
            bool dark = (opts.toolbarBorders == TB_DARK ||
                         opts.toolbarBorders == TB_DARK_ALL);
            if (opts.toolbarBorders == TB_DARK_ALL ||
                opts.toolbarBorders == TB_LIGHT_ALL) {
                painter->setPen(use[0]);
                painter->drawLine(r.x(), r.y(), r.x() + r.width() - 1, r.y());
                painter->drawLine(r.x(), r.y(), r.x(), r.y() + r.height() - 1);
                painter->setPen(use[dark ? 3 : 4]);
                painter->drawLine(r.x(), r.y() + r.height() - 1,
                                  r.x() + r.width() - 1,
                                  r.y() + r.height() - 1);
                painter->drawLine(r.x() + r.width() - 1, r.y(),
                                  r.x() + r.width() - 1,
                                  r.y() + r.height() - 1);
            } else {
                painter->setPen(use[dark ? 3 : 4]);
                painter->drawLine(r.x(), r.y() + r.height() - 1,
                                  r.x() + r.width() - 1,
                                  r.y() + r.height() - 1);
            }
        }
        painter->restore();
    }
    return true;
}

bool
Style::drawPrimitivePanelTipLabel(PrimitiveElement element,
                                  const QStyleOption *option,
                                  QPainter *painter,
                                  const QWidget *widget) const
{
    QTC_UNUSED(element);
    const QRect &r = option->rect;
    const QPalette &palette(option->palette);
    bool haveAlpha = Utils::hasAlphaChannel(widget);
    bool rounded = !(opts.square & SQUARE_TOOLTIPS);
    QPainterPath path =
        (rounded ? buildPath(QRectF(r), WIDGET_OTHER, ROUNDED_ALL,
                             MENU_AND_TOOLTIP_RADIUS) : QPainterPath());
    QColor col = palette.toolTipBase().color();

#ifdef QTC_ENABLE_X11
    if (widget && widget->window()) {
        itsShadowHelper->registerWidget(widget->window());
    }
#endif
    painter->save();
    if (rounded)
        painter->setRenderHint(QPainter::Antialiasing, true);
    if (haveAlpha)
        col.setAlphaF(0.875);
    drawBevelGradient(col, painter, r, path, true, false,
                      opts.tooltipAppearance, WIDGET_TOOLTIP, !haveAlpha);
    if (qtcIsFlat(opts.tooltipAppearance)) {
        painter->setPen(QPen(palette.toolTipText(), 0));
        drawRect(painter, r);
    }
    painter->restore();
    return true;
}

bool
Style::drawPrimitiveQtcBackground(PrimitiveElement element,
                                  const QStyleOption *option,
                                  QPainter *painter,
                                  const QWidget *widget) const
{
    QTC_UNUSED(element);
    QTC_UNUSED(widget);
    const QRect &r = option->rect;
    const QPalette &palette(option->palette);
    State state = option->state;
    if (const BgndOption *bgnd =
        qstyleoption_cast<const BgndOption*>(option)) {
        if (state & QtC_StateKWin) {
            QColor col(palette.brush(QPalette::Window).color());
            int opacity(col.alphaF() * 100);
            col.setAlphaF(1.0);
            drawBackground(painter, col, r, opacity, BGND_WINDOW,
                           bgnd->app, bgnd->path);
            // APPEARANCE_RAISED is used to signal flat background,
            // but have background image!
            if (bgnd->app != APPEARANCE_FLAT) {
                painter->save();
                painter->setClipRect(bgnd->rect, Qt::IntersectClip);
                drawBackgroundImage(painter, true,
                                    BGND_IMG_ON_BORDER ? bgnd->rect :
                                    bgnd->widgetRect);
                painter->restore();
            }
        }
    }
    return true;
}

bool
Style::drawPrimitivePanelItemViewItem(PrimitiveElement element,
                                      const QStyleOption *option,
                                      QPainter *painter,
                                      const QWidget *widget) const
{
    QTC_UNUSED(element);
    const QStyleOptionViewItemV4 *v4Opt =
        qstyleoption_cast<const QStyleOptionViewItemV4*>(option);
    const QAbstractItemView *view =
        qobject_cast<const QAbstractItemView*>(widget);
    QRect r = option->rect;
    const QPalette &palette(option->palette);
    State state = option->state;
    bool reverse = Qt::RightToLeft == option->direction;
    bool hover = (state & State_MouseOver && state & State_Enabled &&
                  (!view ||
                   QAbstractItemView::NoSelection != view->selectionMode()));
    bool hasCustomBackground = (v4Opt->backgroundBrush.style() != Qt::NoBrush &&
                                !(option->state & State_Selected));
    bool hasSolidBackground =
        (!hasCustomBackground ||
         v4Opt->backgroundBrush.style() == Qt::SolidPattern);
    if (!hover && !(state & State_Selected) && !hasCustomBackground &&
        !(v4Opt->features & QStyleOptionViewItem::Alternate)) {
        return true;
    }
    QPalette::ColorGroup cg(state & State_Enabled ? state & State_Active ?
                            QPalette::Normal : QPalette::Inactive :
                            QPalette::Disabled);
    if (v4Opt && (v4Opt->features & QStyleOptionViewItem::Alternate)) {
        painter->fillRect(r, option->palette.brush(cg,
                                                   QPalette::AlternateBase));
    }
    if (!hover && !(state & State_Selected) && !hasCustomBackground) {
        return true;
    }
    if (hasCustomBackground) {
        const QPointF prevOrigin = painter->brushOrigin();
        painter->setBrushOrigin(r.topLeft());
        painter->fillRect(r, v4Opt->backgroundBrush);
        painter->setBrushOrigin(prevOrigin);
    }
    if (state & State_Selected || hover) {
        if (!widget) {
            widget = getWidget(painter);
            if (widget) {
                widget = widget->parentWidget();
            }
        }
        QColor color = (hasCustomBackground && hasSolidBackground ?
                        v4Opt->backgroundBrush.color() :
                        palette.color(cg, QPalette::Highlight));
        bool square = ((opts.square & SQUARE_LISTVIEW_SELECTION) &&
                       (/*(!widget && r.height()<=40 && r.width()>=48) || */
                           (widget && !widget->inherits("KFilePlacesView") &&
                            (qobject_cast<const QTreeView*>(widget) ||
                             (qobject_cast<const QListView*>(widget) &&
                              ((const QListView*)widget)->viewMode() !=
                              QListView::IconMode)))));
        bool modAlpha = (!(state & State_Active) &&
                         itsInactiveChangeSelectionColor);
        if (hover && !hasCustomBackground) {
            if (!(state & State_Selected)) {
                color.setAlphaF(APP_PLASMA == theThemedApp && !widget ?
                                (0.5 * (modAlpha ? 0.75 : 1.0)) : 0.20);
            } else {
                color = color.lighter(110);
                if (modAlpha) {
                    color.setAlphaF(INACTIVE_SEL_ALPHA);
                }
            }
        } else if (modAlpha) {
            color.setAlphaF(color.alphaF() * INACTIVE_SEL_ALPHA);
        }
        if (square) {
            drawBevelGradient(color, painter, r, true, false,
                              opts.selectionAppearance, WIDGET_SELECTION);
        } else {
            QPixmap pix;
            QString key;
            key.sprintf("qtc-sel-%x-%x", r.height(), color.rgba());
            if (!itsUsePixmapCache || !QPixmapCache::find(key, pix)) {
                pix = QPixmap(QSize(24, r.height()));
                pix.fill(Qt::transparent);
                QPainter pixPainter(&pix);
                QRect border(0, 0, pix.width(), pix.height());
                double radius(qtcGetRadius(&opts, r.width(), r.height(),
                                           WIDGET_OTHER, RADIUS_SELECTION));
                pixPainter.setRenderHint(QPainter::Antialiasing, true);
                drawBevelGradient(color, &pixPainter, border,
                                  buildPath(QRectF(border), WIDGET_OTHER,
                                            ROUNDED_ALL, radius), true,
                                  false, opts.selectionAppearance,
                                  WIDGET_SELECTION, false);
                if (opts.borderSelection) {
                    pixPainter.setBrush(Qt::NoBrush);
                    pixPainter.setPen(color);
                    pixPainter.drawPath(buildPath(border, WIDGET_SELECTION,
                                                  ROUNDED_ALL, radius));
                }
                pixPainter.end();
                if (itsUsePixmapCache) {
                    QPixmapCache::insert(key, pix);
                }
            }
            bool roundedLeft = false;
            bool roundedRight = false;
            if (v4Opt) {
                roundedLeft = (QStyleOptionViewItemV4::Beginning ==
                               v4Opt->viewItemPosition);
                roundedRight = (QStyleOptionViewItemV4::End ==
                                v4Opt->viewItemPosition);
                if (QStyleOptionViewItemV4::OnlyOne == v4Opt->viewItemPosition ||
                    QStyleOptionViewItemV4::Invalid == v4Opt->viewItemPosition ||
                    (view && view->selectionBehavior() !=
                     QAbstractItemView::SelectRows)) {
                        roundedLeft = roundedRight = true;
                }
            }
            int size = (roundedLeft && roundedRight ?
                        qMin(8, r.width() / 2) : 8);
            if (!reverse ? roundedLeft : roundedRight) {
                painter->drawPixmap(r.topLeft(), pix.copy(0, 0, size,
                                                          r.height()));
                r.adjust(size, 0, 0, 0);
            }
            if (!reverse ? roundedRight : roundedLeft) {
                painter->drawPixmap(r.right() - size + 1, r.top(),
                                    pix.copy(24 - size, 0, size, r.height()));
                r.adjust(0, 0, -size, 0);
            }
            if (r.isValid()) {
                painter->drawTiledPixmap(r, pix.copy(7, 0, 8, r.height()));
            }
        }
    }
    return true;
}

bool
Style::drawPrimitiveFrameTabWidget(PrimitiveElement element,
                                   const QStyleOption *option,
                                   QPainter *painter,
                                   const QWidget *widget) const
{
    QTC_UNUSED(element);
    const QRect &r = option->rect;
    bool reverse = option->direction == Qt::RightToLeft;
    int round = opts.square & SQUARE_TAB_FRAME ? ROUNDED_NONE : ROUNDED_ALL;

    painter->save();
    if (const QStyleOptionTabWidgetFrame *twf =
        qstyleoption_cast<const QStyleOptionTabWidgetFrame*>(option)) {
        if ((opts.round || (/*qtcIsCustomBgnd(&opts) && */opts.tabBgnd == 0)) &&
            widget && qobject_cast<const QTabWidget*>(widget)) {
            struct QtcTabWidget : public QTabWidget {
                bool
                tabsVisible() const
                {
                    return tabBar() && tabBar()->isVisible();
                }
                QRect
                currentTabRect() const
                {
                    return tabBar()->tabRect(tabBar()->currentIndex());
                }
            };
            const QTabWidget *tw = (const QTabWidget*)widget;
            if (tw->count() > 0 &&
                ((const QtcTabWidget*)widget)->tabsVisible()) {
                if (!reverse && /*qtcIsCustomBgnd(&opts) && */
                    opts.tabBgnd == 0) {
                        // Does not work for reverse :-(
                    QRect tabRect =
                        ((const QtcTabWidget*)widget)->currentTabRect();
                    int adjust = (opts.tabMouseOver == TAB_MO_GLOW &&
                                  !(opts.thin&THIN_FRAMES) ? 2 : 1);
                    switch (tw->tabPosition()) {
                    case QTabWidget::South:
                        tabRect = QRect(tabRect.x() + adjust,
                                        r.y() + r.height() - 2,
                                        tabRect.width() - 2 * adjust, 4);
                        break;
                    case QTabWidget::North: {
                        int leftAdjust =
                            qtcMax(twf->leftCornerWidgetSize.width(), 0);
                        tabRect.adjust(leftAdjust + adjust, 0,
                                       leftAdjust - adjust, 2);
                        break;
                    }
                    case QTabWidget::West:
                        tabRect.adjust(0, adjust, 2, -adjust);
                        break;
                    case QTabWidget::East:
                        tabRect = QRect(r.x() + r.width() - 2,
                                        tabRect.y() + adjust, 4,
                                        tabRect.height() - 2 * adjust);
                        break;
                    }
                    painter->setClipRegion(QRegion(r).subtracted(tabRect),
                                           Qt::IntersectClip);
                }
                if (!(opts.square & SQUARE_TAB_FRAME) &&
                    tw->currentIndex() == 0) {
                    bool reverse = twf->direction == Qt::RightToLeft;
                    switch (tw->tabPosition()) {
                    case QTabWidget::North:
                        if (reverse && twf->rightCornerWidgetSize.isEmpty()) {
                            round -= CORNER_TR;
                        } else if (!reverse &&
                                   twf->leftCornerWidgetSize.isEmpty()) {
                            round-=CORNER_TL;
                        }
                        break;
                    case QTabWidget::South:
                        if (reverse && twf->rightCornerWidgetSize.isEmpty()) {
                            round -= CORNER_BR;
                        } else if (!reverse &&
                                   twf->leftCornerWidgetSize.isEmpty()) {
                            round -= CORNER_BL;
                        }
                        break;
                    case QTabWidget::West:
                        round -= CORNER_TL;
                        break;
                    case QTabWidget::East:
                        round -= CORNER_TR;
                        break;
                    }
                }
            }
        }
    }

    QStyleOption opt(*option);
    const QColor *use = backgroundColors(option);

    opt.state |= State_Enabled;
    if (opts.tabBgnd != 0) {
        QColor bgnd(shade(use[ORIGINAL_SHADE], TO_FACTOR(opts.tabBgnd)));
        painter->fillRect(r.adjusted(0, 1, 0, -1), bgnd);
        painter->fillRect(r.adjusted(1, 0, -1, 0), bgnd);
    }
    drawBorder(painter, r, &opt, round, use, WIDGET_TAB_FRAME,
               opts.borderTab ? BORDER_LIGHT : BORDER_RAISED, false);
    painter->restore();
    return true;
}

bool
Style::drawPrimitiveFrameWindow(PrimitiveElement element,
                                const QStyleOption *option,
                                QPainter *painter,
                                const QWidget *widget) const
{
    QTC_UNUSED(element);
    QTC_UNUSED(widget);
    const QRect &r = option->rect;
    State state = option->state;
    const QPalette &palette(option->palette);
    bool colTbarOnly = opts.windowBorder & WINDOW_BORDER_COLOR_TITLEBAR_ONLY;
    bool fillBgnd = (!(state & QtC_StateKWin) && !itsIsPreview &&
                     !qtcIsFlatBgnd(opts.bgndAppearance));
    const QColor *bgndCols =
        (colTbarOnly || fillBgnd ?
         backgroundColors(palette.color(QPalette::Active,
                                        QPalette::Window)) : 0L);
    const QColor *borderCols = (colTbarOnly ? bgndCols :
                                theThemedApp == APP_KWIN ?
                                buttonColors(option) :
                                getMdiColors(option, state & State_Active));
    QColor light = borderCols[0];
    QColor dark = (option->version == (TBAR_BORDER_VERSION_HACK + 2) ?
                   palette.color(QPalette::Active, QPalette::Shadow) :
                   borderCols[option &&
                              option->version == TBAR_BORDER_VERSION_HACK ?
                              0 : QTC_STD_BORDER]);
    bool isKWin = state & QtC_StateKWin;
    bool addLight = (opts.windowBorder & WINDOW_BORDER_ADD_LIGHT_BORDER &&
                     (!isKWin || qtcGetWindowBorderSize(false).sides > 1));
    light.setAlphaF(1.0);
    dark.setAlphaF(1.0);
    painter->save();
    if (fillBgnd) {
        painter->fillRect(r, bgndCols[ORIGINAL_SHADE]);
    }
    if (opts.round < ROUND_SLIGHT || !isKWin ||
        (state & QtC_StateKWinNotFull && state & QtC_StateKWin)) {
        painter->setRenderHint(QPainter::Antialiasing, false);
        if (addLight) {
            painter->setPen(light);
            painter->drawLine(r.x() + 1, r.y(), r.x() + 1,
                              r.y() + r.height() - 1);
        }
        painter->setPen(dark);
        drawRect(painter, r);
    } else {
        if (addLight) {
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->setPen(light);
            painter->drawLine(r.x() + 1, r.y(), r.x() + 1,
                              r.y() + r.height() -
                              (1 + (opts.round > ROUND_SLIGHT &&
                                    state & QtC_StateKWin ? 3 : 1)));
        }
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(dark);
        painter->drawPath(buildPath(r, WIDGET_OTHER, ROUNDED_ALL,
                                    opts.round > ROUND_SLIGHT &&
                                    state & QtC_StateKWin ? 6.0 : 2.0));
        if (FULLLY_ROUNDED && !(state & QtC_StateKWinCompositing)) {
            QColor col(opts.windowBorder & WINDOW_BORDER_COLOR_TITLEBAR_ONLY ?
                       backgroundColors(option)[QTC_STD_BORDER] :
                       buttonColors(option)[QTC_STD_BORDER]);
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->setPen(col);
            painter->drawPoint(r.x() + 2, r.y() + r.height() - 3);
            painter->drawPoint(r.x() + r.width() - 3, r.y() + r.height() - 3);
            painter->drawLine(r.x() + 1, r.y() + r.height() - 5,
                              r.x() + 1, r.y() + r.height() - 4);
            painter->drawLine(r.x() + 3, r.y() + r.height() - 2,
                              r.x() + 4, r.y() + r.height() - 2);
            painter->drawLine(r.x() + r.width() - 2, r.y() + r.height() - 5,
                              r.x() + r.width() - 2, r.y() + r.height() - 4);
            painter->drawLine(r.x() + r.width() - 4, r.y() + r.height() - 2,
                              r.x() + r.width() - 5, r.y() + r.height() - 2);
        }
    }
    painter->restore();
    return true;
}

bool
Style::drawPrimitiveButton(PrimitiveElement element, const QStyleOption *option,
                    QPainter *painter, const QWidget *widget) const
{
    QRect r = option->rect;
    State state = option->state;
    if (state & STATE_DWT_BUTTON &&
        (opts.dwtSettings & DWT_BUTTONS_AS_PER_TITLEBAR)) {
        return true;
    }
    bool doEtch = opts.buttonEffect != EFFECT_NONE;

    // This fixes the "Sign in" button at mail.lycos.co.uk
    // ...basically if KHTML gices us a fully transparent background colour,
    // then dont paint the button.
    if (option->palette.button().color().alpha() == 0) {
        if (state & State_MouseOver && state & State_Enabled &&
            opts.coloredMouseOver == MO_GLOW && doEtch &&
            !(opts.thin&THIN_FRAMES)) {
            drawGlow(painter, r, WIDGET_STD_BUTTON);
        }
        return true;
    }
    if (!widget)
        widget = getWidget(painter);
    const QColor *use = buttonColors(option);
    bool isDefault = false;
    bool isFlat = false;
    bool isKWin = state & QtC_StateKWin;
    bool isDown = state & State_Sunken || state & State_On;
    bool isOnListView = (!isKWin && widget &&
                         qobject_cast<const QAbstractItemView*>(widget));
    QStyleOption opt(*option);
    if (element == PE_PanelButtonBevel)
        opt.state |= State_Enabled;
    if (const QStyleOptionButton *button =
        qstyleoption_cast<const QStyleOptionButton*>(option)) {
        isDefault = ((button->features & QStyleOptionButton::DefaultButton) &&
                     (button->state & State_Enabled));
        isFlat = button->features & QStyleOptionButton::Flat;
    }
    if (!(opt.state & State_Enabled))
        opt.state &= ~State_MouseOver;
    // For some reason with OO.o not all buttons are set as raised!
    if (!(opt.state & State_AutoRaise))
        opt.state |= State_Raised;

    isDefault = (isDefault ||
                 (doEtch && FULL_FOCUS && MO_GLOW == opts.coloredMouseOver &&
                  opt.state & State_HasFocus && opt.state & State_Enabled));
    if (isFlat && !isDown && !(opt.state & State_MouseOver))
        return true;
    painter->save();

    if (isOnListView)
        opt.state |= State_Horizontal | State_Raised;
    if (isDefault && state&State_Enabled &&
        (IND_TINT == opts.defBtnIndicator ||
         IND_SELECTED == opts.defBtnIndicator)) {
        use = itsDefBtnCols;
    } else if (state & STATE_DWT_BUTTON && widget &&
               opts.titlebarButtons & TITLEBAR_BUTTON_COLOR &&
               coloredMdiButtons(state & State_Active,
                                 state & State_MouseOver) &&
               !(opts.titlebarButtons & TITLEBAR_BUTTON_COLOR_SYMBOL)) {
        if (constDwtClose == widget->objectName()) {
            use = itsTitleBarButtonsCols[TITLEBAR_CLOSE];
        } else if (constDwtFloat == widget->objectName()) {
            use = itsTitleBarButtonsCols[TITLEBAR_MAX];
        }else if (widget->parentWidget() &&
                  widget->parentWidget()->parentWidget() &&
                  widget->parentWidget()->inherits("KoDockWidgetTitleBar") &&
                  qobject_cast<QDockWidget*>(widget->parentWidget()
                                             ->parentWidget())) {
            QDockWidget *dw =
                (QDockWidget*)widget->parentWidget()->parentWidget();
            QWidget *koDw = widget->parentWidget();
            int fw = (dw->isFloating() ?
                      pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, dw) : 0);
            QRect geom = widget->geometry();
            QStyleOptionDockWidget dwOpt;
            dwOpt.initFrom(dw);
            dwOpt.rect = QRect(QPoint(fw, fw),
                               QSize(koDw->geometry().width() - (fw * 2),
                                     koDw->geometry().height() - (fw * 2)));
            dwOpt.title = dw->windowTitle();
            dwOpt.closable = ((dw->features() &
                               QDockWidget::DockWidgetClosable) ==
                              QDockWidget::DockWidgetClosable);
            dwOpt.floatable = ((dw->features() &
                                QDockWidget::DockWidgetFloatable) ==
                               QDockWidget::DockWidgetFloatable);
            if (dwOpt.closable &&
                subElementRect(
                    QStyle::SE_DockWidgetCloseButton, &dwOpt,
                    widget->parentWidget()->parentWidget()) == geom) {
                use = itsTitleBarButtonsCols[TITLEBAR_CLOSE];
            } else if (dwOpt.floatable &&
                       subElementRect(
                           QStyle::SE_DockWidgetFloatButton, &dwOpt,
                           widget->parentWidget()->parentWidget()) == geom) {
                use = itsTitleBarButtonsCols[TITLEBAR_MAX];
            } else {
                use = itsTitleBarButtonsCols[TITLEBAR_SHADE];
            }
        }
    }
    if (isKWin) {
        opt.state |= STATE_KWIN_BUTTON;
    }
    bool coloredDef = (isDefault && state&State_Enabled &&
                       IND_COLORED == opts.defBtnIndicator);
    if (widget && qobject_cast<const QAbstractButton*>(widget) &&
        static_cast<const QAbstractButton*>(widget)->isCheckable())
        opt.state |= STATE_TOGGLE_BUTTON;

    drawLightBevel(painter, r, &opt, widget, ROUNDED_ALL,
                   coloredDef ? itsDefBtnCols[MO_DEF_BTN] :
                   getFill(&opt, use, false,
                           isDefault && state & State_Enabled &&
                           IND_DARKEN == opts.defBtnIndicator),
                   coloredDef ? itsDefBtnCols : use, true,
                   isKWin || state&STATE_DWT_BUTTON ?
                   WIDGET_MDI_WINDOW_BUTTON :
                   isOnListView ? WIDGET_NO_ETCH_BTN :
                   isDefault && state & State_Enabled ?
                   WIDGET_DEF_BUTTON :
                   state & STATE_TBAR_BUTTON ? WIDGET_TOOLBAR_BUTTON :
                   WIDGET_STD_BUTTON);

    if (isDefault && state&State_Enabled)
        switch (opts.defBtnIndicator) {
        case IND_CORNER: {
            QPainterPath path;
            int offset = isDown ? 5 : 4;
            int etchOffset = doEtch ? 1 : 0;
            double xd = r.x() + 0.5;
            double yd = r.y() + 0.5;
            const QColor *cols = itsFocusCols ? itsFocusCols : itsHighlightCols;

            path.moveTo(xd + offset + etchOffset, yd + offset + etchOffset);
            path.lineTo(xd + offset + 6 + etchOffset, yd + offset + etchOffset);
            path.lineTo(xd + offset + etchOffset, yd + offset + 6 + etchOffset);
            path.lineTo(xd + offset + etchOffset, yd + offset + etchOffset);
            painter->setBrush(cols[isDown ? 0 : 4]);
            painter->setPen(cols[isDown ? 0 : 4]);
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawPath(path);
            painter->setRenderHint(QPainter::Antialiasing, false);
            break;
        }
        case IND_COLORED: {
            int offset = COLORED_BORDER_SIZE + (doEtch ? 1 : 0);
            QRect r2 = r.adjusted(offset, offset, -offset, -offset);
            drawBevelGradient(getFill(&opt, use), painter, r2, true,
                              state & (State_On | State_Sunken),
                              opts.appearance, WIDGET_STD_BUTTON);
        }
        default:
            break;
        }
    painter->restore();
    return true;
}

}
