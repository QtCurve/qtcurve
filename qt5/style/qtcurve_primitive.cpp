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

}
