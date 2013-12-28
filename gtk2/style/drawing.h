/*****************************************************************************
 *   Copyright 2003 - 2010 Craig Drummond <craig.p.drummond@gmail.com>       *
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

#ifndef __QTC_DRAWING_H__
#define __QTC_DRAWING_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <common/common.h>
#include "compatability.h"

#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 10, 0))
#define CAIRO_GRAD_END 0.999
#else
#define CAIRO_GRAD_END 1.0
#endif

void clipToRegion(cairo_t *cr, QtcRegion *region);
void setCairoClippingRegion(cairo_t *cr, QtcRegion *region);
void setCairoClipping(cairo_t *cr, GdkRectangle *area);
void drawHLine(cairo_t *cr, double r, double g, double b, double a,
               int x, int y, int w);
void drawVLine(cairo_t *cr, double r, double g, double b, double a,
               int x, int y, int h);
void drawAreaColor(cairo_t *cr, GdkRectangle *area, GdkColor *col,
                   gint x, gint y, gint width, gint height, double alpha);
QTC_ALWAYS_INLINE static inline void
_drawAreaColor(cairo_t *cr, GdkRectangle *area, GdkColor *col,
               gint x, gint y, gint width, gint height)
{
    drawAreaColor(cr, area, col, x, y, width, height, 1);
}
#define drawAreaColor(cr, area, col, x, y, width, height, alpha...)     \
    QTC_SWITCH_(alpha, drawAreaColor)                                   \
    (cr, area, col, x, y, width, height, ##alpha)

void drawBgnd(cairo_t *cr, GdkColor *col, GtkWidget *widget,
              GdkRectangle *area, int x, int y, int width, int height);
void drawAreaModColor(cairo_t *cr, GdkRectangle *area, GdkColor *orig,
                      double mod, gint x, gint y, gint width, gint height);
QTC_ALWAYS_INLINE static inline void
drawAreaMod(cairo_t *cr, GtkStyle *style, GtkStateType state,
            GdkRectangle *area, double mod, gint x, gint y,
            gint width, gint height)
{
    drawAreaModColor(cr, area, &style->bg[state], mod, x, y, width, height);
}

void drawBevelGradient(cairo_t *cr, GdkRectangle *area, int x, int y, int width,
                       int height, GdkColor *base, gboolean horiz, gboolean sel,
                       EAppearance bevApp, EWidget w, double alpha);
QTC_ALWAYS_INLINE static inline void
_drawBevelGradient(cairo_t *cr, GdkRectangle *area, int x, int y, int width,
                   int height, GdkColor *base, gboolean horiz, gboolean sel,
                   EAppearance bevApp, EWidget w)
{
    drawBevelGradient(cr, area, x, y, width, height, base, horiz, sel,
                      bevApp, w, 1);
}
#define drawBevelGradient(cr, area, x, y, width, height, base, horiz, sel, \
                          bevApp, w, alpha...)                          \
    QTC_SWITCH_(alpha, drawBevelGradient)                               \
    (cr, area, x, y, width, height, base, horiz, sel, bevApp, w, ##alpha)

typedef enum {
    DF_DRAW_INSIDE = 0x001,
    DF_BLEND = 0x002,
    DF_SUNKEN = 0x004,
    DF_DO_BORDER = 0x008,
    DF_VERT = 0x010,
    DF_HIDE_EFFECT = 0x020,
    DF_HAS_FOCUS = 0x040
} EDrawFlags;

void plotPoints(cairo_t *cr, GdkPoint *pts, int count);
void createTLPath(cairo_t *cr, double xd, double yd, double width,
                  double height, double radius, int round);
void createBRPath(cairo_t *cr, double xd, double yd, double width,
                  double height, double radius, int round);
void createPath(cairo_t *cr, double xd, double yd, double width,
                double height, double radius, int round);

void drawBorder(cairo_t *cr, GtkStyle *style, GtkStateType state,
                GdkRectangle *area, gint x, gint y, gint width, gint height,
                GdkColor *c_colors, int round, EBorder borderProfile,
                EWidget widget, int flags, int borderVal);
QTC_ALWAYS_INLINE static inline void
_drawBorder(cairo_t *cr, GtkStyle *style, GtkStateType state,
            GdkRectangle *area, gint x, gint y, gint width, gint height,
            GdkColor *c_colors, int round, EBorder borderProfile,
            EWidget widget, int flags)
{
    drawBorder(cr, style, state, area, x, y, width, height, c_colors, round,
               borderProfile, widget, flags, QTC_STD_BORDER);
}
#define drawBorder(cr, style, state, area, x, y, width, height, c_colors, \
                   round, borderProfile, widget, flags, borderVal...)   \
    QTC_SWITCH_(borderVal, drawBorder)                                  \
    (cr, style, state, area, x, y, width, height, c_colors,             \
     round, borderProfile, widget, flags, ##borderVal)

void drawGlow(cairo_t *cr, GdkRectangle *area, int x, int y, int w, int h,
              int round, EWidget widget, const GdkColor *colors);
QTC_ALWAYS_INLINE static inline void
_drawGlow(cairo_t *cr, GdkRectangle *area, int x, int y, int w, int h,
          int round, EWidget widget)
{
    drawGlow(cr, area, x, y, w, h, round, widget, NULL);
}
#define drawGlow(cr, area, x, y, w, h, round, widget, colors...)        \
    QTC_SWITCH_(colors, drawGlow)(cr, area, x, y, w, h, round, widget, ##colors)

void drawEtch(cairo_t *cr, GdkRectangle *area, GtkWidget *widget,
              int x, int y, int w, int h, gboolean raised, int round, EWidget wid);
void clipPathRadius(cairo_t *cr, double x, double y, int w, int h,
                    double radius, int round);
void clipPath(cairo_t *cr, int x, int y, int w, int h, EWidget widget,
              int rad, int round);
void addStripes(cairo_t *cr, int x, int y, int w, int h, bool horizontal);
void drawLightBevel(cairo_t *cr, GtkStyle *style, GtkStateType state,
                    GdkRectangle *area, gint x, gint y, gint width, gint height,
                    GdkColor *base, GdkColor *colors, int round, EWidget widget,
                    EBorder borderProfile, int flags, GtkWidget *wid);

void drawFadedLine(cairo_t *cr, int x, int y, int width, int height,
                   GdkColor *col, GdkRectangle *area, GdkRectangle *gap,
                   gboolean fadeStart, gboolean fadeEnd, gboolean horiz,
                   double alpha);
QTC_ALWAYS_INLINE static inline void
_drawFadedLine(cairo_t *cr, int x, int y, int width, int height, GdkColor *col,
               GdkRectangle *area, GdkRectangle *gap, gboolean fadeStart,
               gboolean fadeEnd, gboolean horiz)
{
    drawFadedLine(cr, x, y, width, height, col, area, gap, fadeStart,
                  fadeEnd, horiz, 1.0);
}
#define drawFadedLine(cr, x, y, width, height, col, area, gap, fadeStart, \
                      fadeEnd, horiz, alpha...)                         \
    QTC_SWITCH_(alpha, drawFadedLine)(cr, x, y, width, height, col, area, \
                                      gap, fadeStart, fadeEnd, horiz, ##alpha)

void drawHighlight(cairo_t *cr, int x, int y, int width, int height, GdkRectangle *area, gboolean horiz, gboolean inc);
void setLineCol(cairo_t *cr, cairo_pattern_t *pt, GdkColor *col);
void drawLines(cairo_t *cr, double rx, double ry, int rwidth, int rheight, gboolean horiz,
                      int nLines, int offset, GdkColor *cols, GdkRectangle *area, int dark, ELine type);
void drawDot(cairo_t *cr, int x, int y, int w, int h, GdkColor *cols);
void drawDots(cairo_t *cr, int rx, int ry, int rwidth, int rheight, gboolean horiz, int nLines, int offset, GdkColor *cols, GdkRectangle *area,
                     int startOffset, int dark);
void drawEntryCorners(cairo_t *cr, GdkRectangle *area, int round, int x, int y, int width, int height, double r, double g, double b, double a);
void drawBgndRing(cairo_t *cr, int x, int y, int size, int size2, gboolean isWindow);
void drawBgndRings(cairo_t *cr, gint x, gint y, gint width, gint height, gboolean isWindow);
void drawBgndImage(cairo_t *cr, GtkStyle *style, GdkRectangle *area, gint x, gint y, gint w, gint h, GdkColor *col,
                          gboolean isWindow, double alpha);
void drawStripedBgnd(cairo_t *cr, GtkStyle *style, GdkRectangle *area, gint x, gint y, gint w, gint h, GdkColor *col,
                            gboolean isWindow, double alpha);
gboolean drawWindowBgnd(cairo_t *cr, GtkStyle *style, GdkRectangle *area, GdkWindow *window, GtkWidget *widget,
                               gint x, gint y, gint width, gint height);
void drawEntryField(cairo_t *cr, GtkStyle *style, GtkStateType state, GdkWindow *window, GtkWidget *widget, GdkRectangle *area,
                           gint x, gint y, gint width, gint height, int round, EWidget w);
void setProgressStripeClipping(cairo_t *cr, GdkRectangle *area, int x, int y, int width, int height, int animShift, gboolean horiz);
void drawProgress(cairo_t *cr, GtkStyle *style, GtkStateType state, GtkWidget *widget, GdkRectangle *area, int x, int y, int width, int height,
                         gboolean rev, gboolean isEntryProg);
void drawProgressGroove(cairo_t *cr, GtkStyle *style, GtkStateType state, GdkWindow *window, GtkWidget *widget, GdkRectangle *area,
                               int x, int y, int width, int height, gboolean isList, gboolean horiz);
void drawSliderGroove(cairo_t *cr, GtkStyle *style, GtkStateType state, GdkWindow *window, GtkWidget *widget, const gchar *detail,
                             GdkRectangle *area, int x, int y, int width, int height, gboolean horiz);
void drawTriangularSlider(cairo_t *cr, GtkStyle *style, GtkStateType state, const gchar *detail, GdkRectangle *area, int x, int y, int width, int height);
void drawScrollbarGroove(cairo_t *cr, GtkStyle *style, GtkStateType state, GdkWindow *window, GtkWidget *widget, const gchar *detail,
                                GdkRectangle *area, int x, int y, int width, int height, gboolean horiz);
void drawSelectionGradient(cairo_t *cr, GtkStyle *style, GtkStateType state, GdkRectangle *area,
                                  int x, int y, int width, int height, int round, gboolean isLvSelection,
                                  double alpha, GdkColor *col, gboolean horiz);
void drawSelection(cairo_t *cr, GtkStyle *style, GtkStateType state, GdkRectangle *area, GtkWidget *widget,
                          int x, int y, int width, int height, int round, gboolean isLvSelection, double alphaMod, int factor);
void createRoundedMask(cairo_t *cr, GtkWidget *widget, gint x, gint y, gint width, gint height, double radius, gboolean isToolTip);
void clearRoundedMask(GtkWidget *widget, gboolean isToolTip);
void drawTreeViewLines(cairo_t *cr, GdkColor *col, int x, int y, int h, int depth, int levelIndent, int expanderSize,
                              GtkTreeView *treeView, GtkTreePath *path, GtkTreeViewColumn *column);
void drawPolygon(GdkWindow *window, GtkStyle *style, GdkColor *col, GdkRectangle *area, GdkPoint *points, int npoints, gboolean fill);
void drawArrow(GdkWindow *window, GtkStyle *style, GdkColor *col, GdkRectangle *area, GtkArrowType arrow_type,
                      gint x, gint y, gboolean small, gboolean fill);
void drawLayout(GtkStyle *style, GdkWindow *window, GtkStateType state, gboolean use_text, GdkRectangle *area, gint x, gint y, PangoLayout *layout);
void fillTab(cairo_t *cr, GtkStyle *style, GtkWidget *widget, GdkRectangle *area, GtkStateType state,
                    GdkColor *col, int x, int y, int width, int height, gboolean horiz, EWidget tab, gboolean grad);
void colorTab(cairo_t *cr, int x, int y, int width, int height, int round, EWidget tab, gboolean horiz);
void drawToolTip(cairo_t *cr, GtkWidget *widget, GdkRectangle *area, int x, int y, int width, int height);
void drawSplitter(cairo_t *cr, GtkStateType state, GtkStyle *style, GdkRectangle *area, int x, int y, int width, int height);
void drawSidebarButton(cairo_t *cr, GtkStateType state, GtkStyle *style, GdkRectangle *area, int x, int y, int width, int height);
void drawMenuItem(cairo_t *cr, GtkStateType state, GtkStyle *style, GtkWidget *widget, GdkRectangle *area, int x, int y, int width, int height);
void drawMenu(cairo_t *cr, GtkStateType state, GtkStyle *style, GtkWidget *widget, GdkRectangle *area, int x, int y, int width, int height);
void drawBoxGap(cairo_t *cr, GtkStyle *style, GtkShadowType shadow, GtkStateType state,
                       GtkWidget *widget, GdkRectangle *area, gint x, gint y, gint width, gint height, GtkPositionType gap_side,
                       gint gapX, gint gapWidth, EBorder borderProfile, gboolean isTab);
void drawBoxGapFixes(cairo_t *cr, GtkWidget *widget, gint x, gint y, gint width, gint height, GtkPositionType gapSide, gint gapX, gint gapWidth);
void drawShadowGap(cairo_t *cr, GtkStyle *style, GtkShadowType shadow, GtkStateType state,
                          GtkWidget *widget, GdkRectangle *area, gint x, gint y, gint width, gint height, GtkPositionType gapSide,
                          gint gapX, gint gapWidth);
void drawCheckBox(cairo_t *cr, GtkStateType state, GtkShadowType shadow, GtkStyle *style, GtkWidget *widget, const gchar *detail,
                         GdkRectangle *area, int x, int y, int width, int height);
void drawTab(cairo_t *cr, GtkStateType state, GtkStyle *style, GtkWidget *widget, const gchar *detail,
                    GdkRectangle *area, int x, int y, int width, int height, GtkPositionType gapSide);
void drawRadioButton(cairo_t *cr, GtkStateType state, GtkShadowType shadow, GtkStyle *style, GtkWidget *widget, const gchar *detail,
                            GdkRectangle *area, int x, int y, int width, int height);
void drawToolbarBorders(cairo_t *cr, GtkStateType state, int x, int y, int width, int height, gboolean isActiveWindowMenubar, const char *detail);
void drawListViewHeader(cairo_t *cr, GtkStateType state, GdkColor *btnColors, int bgnd, GdkRectangle *area, int x, int y, int width, int height);
void drawDefBtnIndicator(cairo_t *cr, GtkStateType state, GdkColor *btnColors, int bgnd, gboolean sunken, GdkRectangle *area, int x, int y, int width, int height);
GdkPixbuf * renderIcon(GtkStyle *style, const GtkIconSource *source, GtkTextDirection direction,
                              GtkStateType state, GtkIconSize size, GtkWidget *widget, const char *detail);
#endif
