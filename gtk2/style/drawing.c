/*****************************************************************************
 *   Copyright 2003 - 2010 Craig Drummond <craig.p.drummond@gmail.com>       *
 *   Copyright 2013 - 2014 Yichao Yu <yyc1992@gmail.com>                     *
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

#include <qtcurve-utils/gtkprops.h>
#include <qtcurve-utils/color.h>
#include <qtcurve-utils/log.h>

#include "drawing.h"
#include "qt_settings.h"
#include "qtcurve.h"
#include <common/config_file.h>
#include "helpers.h"
#include "pixcache.h"
#include "entry.h"
#include "tab.h"
#include "animation.h"

#if GTK_CHECK_VERSION(2, 90, 0)
#define createRect(x, y, w, h) ((cairo_rectangle_int_t){(x), (y), (w), (h)})

static cairo_region_t*
windowMask(int x, int y, int w, int h, gboolean full)
{
    cairo_rectangle_int_t rects[4];
    int numRects = 4;

    if (full) {
        rects[0] = createRect(x + 4, y + 0, w - 4 * 2, h);
        rects[1] = createRect(x + 0, y + 4, w, h - 4 * 2);
        rects[2] = createRect(x + 2, y + 1, w - 2 * 2, h - 2);
        rects[3] = createRect(x + 1, y + 2, w - 2, h - 2 * 2);
    } else {
        rects[0] = createRect(x + 1, y + 1, w - 2, h - 2);
        rects[1] = createRect(x, y + 2, w, h - 4);
        rects[2] = createRect(x + 2, y, w - 4, h);
        numRects = 3;
    }
    return cairo_region_create_rectangles(rects, numRects);
}

void clipToRegion(cairo_t *cr, cairo_region_t *region)
{
    cairo_new_path(cr);
    int numRects = cairo_region_num_rectangles(region);

    while (numRects--) {
        cairo_rectangle_int_t rect;
        cairo_region_get_rectangle(region, numRects, &rect);
        cairo_rectangle(cr, rect.x, rect.y, rect.width, rect.height);
    }
    cairo_clip(cr);
}
#else
void clipToRegion(cairo_t *cr, GdkRegion *region)
{
    cairo_new_path(cr);
    int numRects;
    GdkRectangle *rects;

    gdk_region_get_rectangles(region, &rects, &numRects);
    while (numRects--) {
        GdkRectangle *rect = &rects[numRects];
        cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
    }
    g_free(rects);
    cairo_clip(cr);
}
#endif

void
setCairoClippingRegion(cairo_t *cr, GdkRegion *region)
{
    cairo_save(cr);
    if (region)
        clipToRegion(cr, region);
    cairo_new_path(cr);
}

void setCairoClipping(cairo_t *cr, GdkRectangle *area)
{
    cairo_save(cr);
    if (area) {
        cairo_rectangle(cr, area->x, area->y, area->width, area->height);
        cairo_clip(cr);
    }
    cairo_new_path(cr);
}

void
drawHLine(cairo_t *cr, const GdkColor *col, double a, int x, int y, int w)
{
    cairo_new_path(cr);
    qtcCairoSetColor(cr, col, a);
    cairo_move_to(cr, x, y + 0.5);
    cairo_line_to(cr, x + w, y + 0.5);
    cairo_stroke(cr);
}

void
drawVLine(cairo_t *cr, const GdkColor *col, double a, int x, int y, int h)
{
    cairo_new_path(cr);
    qtcCairoSetColor(cr, col, a);
    cairo_move_to(cr, x + 0.5, y);
    cairo_line_to(cr, x + 0.5, y + h);
    cairo_stroke(cr);
}

void drawAreaColor(cairo_t *cr, GdkRectangle *area, const GdkColor *col,
                   int x, int y, int width, int height, double alpha)
{
    setCairoClipping(cr, area);
    cairo_rectangle(cr, x, y, width, height);
    qtcCairoSetColor(cr, col, alpha);
    cairo_fill(cr);
    cairo_restore(cr);
}

void
drawBgnd(cairo_t *cr, GdkColor *col, GtkWidget *widget, GdkRectangle *area,
         int x, int y, int width, int height)
{
    GdkColor *parent_col = getParentBgCol(widget);
    drawAreaColor(cr, area, parent_col ? parent_col : col,
                  x, y, width, height);
}

void
drawAreaModColor(cairo_t *cr, GdkRectangle *area, GdkColor *orig,
                 double mod, int x, int y, int width, int height)
{
    GdkColor modified = shadeColor(orig, mod);

    drawAreaColor(cr, area, &modified, x, y, width, height);
}

void
drawBevelGradient(cairo_t *cr, GdkRectangle *area, int x, int y,
                  int width, int height, GdkColor *base, gboolean horiz,
                  gboolean sel, EAppearance bevApp, EWidget w, double alpha)
{
    /* EAppearance app = ((APPEARANCE_BEVELLED != bevApp || WIDGET_BUTTON(w) || */
    /*                     WIDGET_LISTVIEW_HEADER == w) ? bevApp : */
    /*                    APPEARANCE_GRADIENT); */

    if (qtcIsFlat(bevApp)) {
        if ((WIDGET_TAB_TOP != w && WIDGET_TAB_BOT != w) ||
            !qtcIsCustomBgnd(&opts) || opts.tabBgnd || !sel) {
            drawAreaColor(cr, area, base, x, y, width, height, alpha);
        }
    } else {
        cairo_pattern_t *pt =
            cairo_pattern_create_linear(x, y, horiz ? x : x+width - 1,
                                        horiz ? y + height - 1 : y);
        gboolean topTab = WIDGET_TAB_TOP == w;
        gboolean botTab = WIDGET_TAB_BOT == w;
        gboolean selected = (topTab || botTab) ? false : sel;
        EAppearance app = (selected ? opts.sunkenAppearance :
                           WIDGET_LISTVIEW_HEADER == w &&
                           APPEARANCE_BEVELLED == bevApp ?
                           APPEARANCE_LV_BEVELLED :
                           APPEARANCE_BEVELLED != bevApp ||
                           WIDGET_BUTTON(w) ||
                           WIDGET_LISTVIEW_HEADER == w ? bevApp :
                           APPEARANCE_GRADIENT);
        const Gradient *grad = qtcGetGradient(app, &opts);
        int i = 0;

        setCairoClipping(cr, area);

        for (i = 0;i < grad->numStops;++i) {
            GdkColor col;
            double pos = botTab ? 1.0 - grad->stops[i].pos : grad->stops[i].pos;

            if (/*sel && */(topTab || botTab) && i == grad->numStops - 1) {
                if (sel /*&& qtcIsCustomBgnd(&opts)*/ && 0 == opts.tabBgnd &&
                    !isMozilla()) {
                    alpha = 0.0;
                }
                col = *base;
            } else {
                double val = (botTab && opts.invertBotTab ?
                              INVERT_SHADE(grad->stops[i].val) :
                              grad->stops[i].val);
                qtcShade(base, &col, botTab && opts.invertBotTab ?
                         qtcMax(val, 0.9) : val, opts.shading);
            }

#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 10, 0))
            if (pos > 0.9999)
                pos = 0.999;
#endif
            qtcCairoPatternAddColorStop(pt, pos, &col,
                                        qtcOneOf(w, WIDGET_TOOLTIP,
                                                 WIDGET_LISTVIEW_HEADER) ?
                                        alpha : alpha * grad->stops[i].alpha);
        }

        if (APPEARANCE_AGUA == app && !(topTab || botTab) &&
            (horiz ? height : width) > AGUA_MAX) {
            GdkColor col;
            double pos = AGUA_MAX / ((horiz ? height : width) * 2.0);

#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 10, 0))
            if (pos > 0.9999)
                pos = 0.999;
#endif

            qtcShade(base, &col, AGUA_MID_SHADE, opts.shading);
            qtcCairoPatternAddColorStop(pt, pos, &col, alpha);
            /* *grad->stops[i].alpha); */
            qtcCairoPatternAddColorStop(pt, 1.0 - pos, &col, alpha);
            /* *grad->stops[i].alpha); */
        }

        cairo_set_source(cr, pt);
        cairo_rectangle(cr, x, y, width, height);
        cairo_fill(cr);
        cairo_pattern_destroy(pt);
        cairo_restore(cr);
    }
}

#if 0
gboolean drawBackgroundPng(const char *png)
{
    int size = 512;
    cairo_surface_t *surface =
        cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 512, 512);
    gboolean rv = FALSE;

    if (surface) {
        cairo_t *cr = cairo_create(surface);
        if (cr) {
            drawBevelGradient(cr, NULL, 0, 0, size, size,
                              &(qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW]),
                              opts.bgndGrad == GT_HORIZ, FALSE,
                              opts.bgndAppearance, WIDGET_OTHER, 1.0);
            rv = (cairo_surface_write_to_png(surface, png) ==
                  CAIRO_STATUS_SUCCESS);
            cairo_destroy(cr);
        }
    }
    return rv;
}
#endif

void
plotPoints(cairo_t *cr, GdkPoint *pts, int count)
{
    cairo_move_to(cr, pts[0].x + 0.5, pts[0].y + 0.5);
    for (int i = 1;i < count;i++) {
        cairo_line_to(cr, pts[i].x + 0.5, pts[i].y + 0.5);
    }
}

void
createTLPath(cairo_t *cr, double xd, double yd, double width,
             double height, double radius, int round)
{
    bool rounded = radius > 0.0;

    if (rounded && round & CORNER_BL) {
        cairo_arc(cr, xd + radius, yd + height - radius, radius,
                  M_PI * 0.75, M_PI);
    } else {
        cairo_move_to(cr, xd, yd + height);
    }
    if (rounded && round & CORNER_TL) {
        cairo_arc(cr, xd + radius, yd + radius, radius, M_PI, M_PI * 1.5);
    } else {
        cairo_line_to(cr, xd, yd);
    }
    if (rounded && round & CORNER_TR) {
        cairo_arc(cr, xd + width - radius, yd + radius,
                  radius, M_PI * 1.5, M_PI * 1.75);
    } else {
        cairo_line_to(cr, xd + width, yd);
    }
}

void
createBRPath(cairo_t *cr, double xd, double yd, double width, double height,
             double radius, int round)
{
    bool rounded = radius > 0.0;

    if (rounded && round & CORNER_TR) {
        cairo_arc(cr, xd + width - radius, yd + radius, radius, M_PI * 1.75, 0);
    } else {
        cairo_move_to(cr, xd + width, yd);
    }
    if (rounded && round & CORNER_BR) {
        cairo_arc(cr, xd + width - radius, yd + height - radius,
                  radius, 0, M_PI * 0.5);
    } else {
        cairo_line_to(cr, xd + width, yd + height);
    }
    if (rounded && round & CORNER_BL) {
        cairo_arc(cr, xd + radius, yd + height - radius,
                  radius, M_PI * 0.5, M_PI * 0.75);
    } else {
        cairo_line_to(cr, xd, yd + height);
    }
}

void
createPath(cairo_t *cr, double xd, double yd, double width, double height,
           double radius, int round)
{
    bool rounded = radius > 0.0;

    if (rounded && round & CORNER_TL) {
        cairo_move_to(cr, xd + radius, yd);
    } else {
        cairo_move_to(cr, xd, yd);
    }
    if (rounded && round & CORNER_TR) {
        cairo_arc(cr, xd + width - radius, yd + radius, radius,
                  M_PI * 1.5, M_PI * 2);
    } else {
        cairo_line_to(cr, xd + width, yd);
    }
    if (rounded && round & CORNER_BR) {
        cairo_arc(cr, xd + width - radius, yd + height - radius,
                  radius, 0, M_PI * 0.5);
    } else {
        cairo_line_to(cr, xd + width, yd + height);
    }
    if (rounded && round&CORNER_BL) {
        cairo_arc(cr, xd + radius, yd + height - radius,
                  radius, M_PI * 0.5, M_PI);
    } else {
        cairo_line_to(cr, xd, yd + height);
    }
    if (rounded && round & CORNER_TL) {
        cairo_arc(cr, xd + radius, yd + radius, radius, M_PI, M_PI * 1.5);
    } else {
        cairo_line_to(cr, xd, yd);
    }
}

void
drawBorder(cairo_t *cr, GtkStyle *style, GtkStateType state,
           GdkRectangle *area, int x, int y, int width, int height,
           GdkColor *c_colors, int round, EBorder borderProfile,
           EWidget widget, int flags, int borderVal)
{
    if (opts.round == ROUND_NONE && widget != WIDGET_RADIO_BUTTON) {
        round = ROUNDED_NONE;
    }
    double radius = qtcGetRadius(&opts, width, height, widget, RADIUS_EXTERNAL);
    double xd = x + 0.5;
    double yd = y + 0.5;
    /* EAppearance app = qtcWidgetApp(widget, &opts); */
    bool enabled = state != GTK_STATE_INSENSITIVE;
    bool useText = (enabled && widget == WIDGET_DEF_BUTTON &&
                    opts.defBtnIndicator == IND_FONT_COLOR);
    /* CPD USED TO INDICATE FOCUS! */
    bool hasFocus = (enabled && qtcPalette.focus &&
                     c_colors == qtcPalette.focus);
    bool hasMouseOver = (enabled && qtcPalette.mouseover &&
                         c_colors == qtcPalette.mouseover && ENTRY_MO);
    GdkColor *colors = c_colors ? c_colors : qtcPalette.background;
    int useBorderVal = (!enabled && WIDGET_BUTTON(widget) ?
                        QTC_DISABLED_BORDER :
                        qtcPalette.mouseover == colors && IS_SLIDER(widget) ?
                        SLIDER_MO_BORDER_VAL : borderVal);
    GdkColor *border_col = (useText ? &style->text[GTK_STATE_NORMAL] :
                            &colors[useBorderVal]);
    width--;
    height--;
    setCairoClipping(cr, area);

    if (qtcOneOf(widget, WIDGET_TAB_BOT, WIDGET_TAB_TOP)) {
        colors = qtcPalette.background;
    }
    if (!(opts.thin&THIN_FRAMES)) {
        switch (borderProfile) {
        case BORDER_FLAT:
            break;
        case BORDER_RAISED:
        case BORDER_SUNKEN:
        case BORDER_LIGHT: {
            double radiusi = qtcGetRadius(&opts, width - 2, height - 2,
                                          widget, RADIUS_INTERNAL);
            double xdi = xd + 1;
            double ydi = yd + 1;
            double alpha = ((hasMouseOver || hasFocus) &&
                            qtcOneOf(widget, WIDGET_ENTRY, WIDGET_SPIN,
                                     WIDGET_COMBO_BUTTON) ? ENTRY_INNER_ALPHA :
                            BORDER_BLEND_ALPHA(widget));
            int widthi = width - 2;
            int heighti = height - 2;

            if ((state != GTK_STATE_INSENSITIVE ||
                 borderProfile == BORDER_SUNKEN) /* && */
                /* (qtcOneOf(borderProfile, BORDER_RAISED, BORDER_LIGHT) || */
                /*  app != APPEARANCE_FLAT) */) {
                GdkColor *col = &colors[qtcOneOf(borderProfile, BORDER_RAISED,
                                                 BORDER_LIGHT) ? 0 :
                                        FRAME_DARK_SHADOW];
                if (flags & DF_BLEND) {
                    if (qtcOneOf(widget, WIDGET_SPIN, WIDGET_COMBO_BUTTON,
                                 WIDGET_SCROLLVIEW)) {
                        qtcCairoSetColor(cr, &style->base[state]);
                        createTLPath(cr, xdi, ydi, widthi, heighti, radiusi,
                                     round);
                        cairo_stroke(cr);
                    }
                    qtcCairoSetColor(cr, col, alpha);
                } else {
                    qtcCairoSetColor(cr, col);
                }
            } else {
                qtcCairoSetColor(cr, &style->bg[state]);
            }

            createTLPath(cr, xdi, ydi, widthi, heighti, radiusi, round);
            cairo_stroke(cr);
            if (widget != WIDGET_CHECKBOX) {
                if(!hasFocus && !hasMouseOver &&
                   borderProfile != BORDER_LIGHT) {
                    if (widget == WIDGET_SCROLLVIEW) {
                        /* Because of list view headers, need to draw dark
                         * line on right! */
                        cairo_save(cr);
                        qtcCairoSetColor(cr, &style->base[state]);
                        createBRPath(cr, xdi, ydi, widthi, heighti, radiusi,
                                     round);
                        cairo_stroke(cr);
                        cairo_restore(cr);
                    } else if (qtcOneOf(widget, WIDGET_SCROLLVIEW,
                                        WIDGET_ENTRY)) {
                        qtcCairoSetColor(cr, &style->base[state]);
                    } else if (state != GTK_STATE_INSENSITIVE &&
                               (borderProfile == BORDER_SUNKEN ||
                                /* app != APPEARANCE_FLAT ||*/
                                qtcOneOf(widget, WIDGET_TAB_TOP,
                                         WIDGET_TAB_BOT))) {
                        GdkColor *col = &colors[borderProfile == BORDER_RAISED ?
                                                FRAME_DARK_SHADOW : 0];
                        if (flags & DF_BLEND) {
                            qtcCairoSetColor(cr, col,
                                             borderProfile == BORDER_SUNKEN ?
                                             0.0 : alpha);
                        } else {
                            qtcCairoSetColor(cr, col);
                        }
                    } else {
                        qtcCairoSetColor(cr, &style->bg[state]);
                    }
                }

                createBRPath(cr, xdi, ydi, widthi, heighti, radiusi, round);
                cairo_stroke(cr);
            }
        }
        }
    }

    if (borderProfile == BORDER_SUNKEN &&
        (widget == WIDGET_FRAME ||
         (qtcOneOf(widget, WIDGET_ENTRY, WIDGET_SCROLLVIEW) &&
          !opts.etchEntry && !hasFocus && !hasMouseOver))) {
        qtcCairoSetColor(cr, border_col,
                         /*enabled ? */1.0/* : LOWER_BORDER_ALPHA*/);
        createTLPath(cr, xd, yd, width, height, radius, round);
        cairo_stroke(cr);
        qtcCairoSetColor(cr, border_col, LOWER_BORDER_ALPHA);
        createBRPath(cr, xd, yd, width, height, radius, round);
        cairo_stroke(cr);
    } else {
        qtcCairoSetColor(cr, border_col);
        createPath(cr, xd, yd, width, height, radius, round);
        cairo_stroke(cr);
    }
    cairo_restore(cr);
}

void
drawGlow(cairo_t *cr, GdkRectangle *area, int x, int y, int w, int h,
         int round, EWidget widget, const GdkColor *colors)
{
    if (qtcPalette.mouseover || qtcPalette.defbtn || colors) {
        double xd = x + 0.5;
        double yd = y + 0.5;
        double radius = qtcGetRadius(&opts, w, h, widget, RADIUS_ETCH);
        bool def = (widget == WIDGET_DEF_BUTTON &&
                        opts.defBtnIndicator == IND_GLOW);
        bool defShade =
            (def && (!qtcPalette.defbtn ||
                     (qtcPalette.mouseover &&
                      EQUAL_COLOR(qtcPalette.defbtn[ORIGINAL_SHADE],
                                  qtcPalette.mouseover[ORIGINAL_SHADE]))));
        const GdkColor *col =
            (colors ? &colors[GLOW_MO] : (def && qtcPalette.defbtn) ||
             !qtcPalette.mouseover ? &qtcPalette.defbtn[GLOW_DEFBTN] :
             &qtcPalette.mouseover[GLOW_MO]);

        setCairoClipping(cr, area);
        qtcCairoSetColor(cr, col, GLOW_ALPHA(defShade));
        createPath(cr, xd, yd, w - 1, h - 1, radius, round);
        cairo_stroke(cr);
        cairo_restore(cr);
    }
}

void
drawEtch(cairo_t *cr, GdkRectangle *area, GtkWidget *widget, int x, int y,
         int w, int h, gboolean raised, int round, EWidget wid)
{
    double xd = x + 0.5;
    double yd = y + 0.5;
    double radius = qtcGetRadius(&opts, w, h, wid, RADIUS_ETCH);
    GdkRectangle *a = area;
    GdkRectangle b;

    if (wid == WIDGET_TOOLBAR_BUTTON && opts.tbarBtnEffect == EFFECT_ETCH)
        raised = FALSE;

    if (wid == WIDGET_COMBO_BUTTON && qtSettings.app == GTK_APP_OPEN_OFFICE &&
        widget && isFixedWidget(gtk_widget_get_parent(widget))) {
        b.x = x + 2;
        b.y = y;
        b.width = w - 4;
        b.height = h;
        a = &b;
    }
    setCairoClipping(cr, a);

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0,
                          USE_CUSTOM_ALPHAS(opts) ?
                          opts.customAlphas[ALPHA_ETCH_DARK] : ETCH_TOP_ALPHA);
    if (!raised && wid != WIDGET_SLIDER) {
        createTLPath(cr, xd, yd, w - 1, h - 1, radius, round);
        cairo_stroke(cr);
        if (wid == WIDGET_SLIDER_TROUGH && opts.thinSbarGroove &&
            widget && GTK_IS_SCROLLBAR(widget)) {
            cairo_set_source_rgba(cr, 1.0, 1.0, 1.0,
                                  USE_CUSTOM_ALPHAS(opts) ?
                                  opts.customAlphas[ALPHA_ETCH_LIGHT] :
                                  ETCH_BOTTOM_ALPHA);
        } else {
            setLowerEtchCol(cr, widget);
        }
    }
    createBRPath(cr, xd, yd, w - 1, h - 1, radius, round);
    cairo_stroke(cr);
    cairo_restore(cr);
}

void
clipPathRadius(cairo_t *cr, double x, double y, int w, int h,
               double radius, int round)
{
    cairo_new_path(cr);
    cairo_save(cr);
    createPath(cr, x, y, w, h, radius, round);
    cairo_clip(cr);
}

void
clipPath(cairo_t *cr, int x, int y, int w, int h, EWidget widget,
         int rad, int round)
{
    clipPathRadius(cr, x + 0.5, y + 0.5, w - 1, h - 1,
                   qtcGetRadius(&opts, w, h, widget, rad), round);
}

void
addStripes(cairo_t *cr, int x, int y, int w, int h, bool horizontal)
{
    int endx = horizontal ? STRIPE_WIDTH : 0;
    int endy = horizontal ? 0 : STRIPE_WIDTH;

    cairo_pattern_t *pat =
        cairo_pattern_create_linear(x, y, x + endx, y + endy);

    cairo_pattern_add_color_stop_rgba(pat, 0.0, 1.0, 1.0, 1.0, 0.0);
    cairo_pattern_add_color_stop_rgba(pat, CAIRO_GRAD_END, 1.0, 1.0, 1.0, 0.15);
    cairo_pattern_set_extend(pat, CAIRO_EXTEND_REFLECT);
    cairo_set_source(cr, pat);
    cairo_rectangle(cr, x, y, w, h);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);
}

void
drawLightBevel(cairo_t *cr, GtkStyle *style, GtkStateType state,
               GdkRectangle *area, int x, int y,
               int width, int height, GdkColor *base, GdkColor *colors,
               int round, EWidget widget, EBorder borderProfile, int flags,
               GtkWidget *wid)
{
    EAppearance app = qtcWidgetApp(APPEARANCE_NONE != opts.tbarBtnAppearance &&
                                   (WIDGET_TOOLBAR_BUTTON == widget ||
                                    (WIDGET_BUTTON(widget) &&
                                     isOnToolbar(wid, NULL, 0))) ?
                                   WIDGET_TOOLBAR_BUTTON : widget, &opts);
    gboolean sunken = flags & DF_SUNKEN;
    gboolean doColouredMouseOver =
        (opts.coloredMouseOver && qtcPalette.mouseover &&
         WIDGET_SPIN != widget && WIDGET_SPIN_DOWN != widget &&
         WIDGET_SPIN_UP != widget && WIDGET_COMBO_BUTTON != widget &&
         WIDGET_SB_BUTTON != widget && (!SLIDER(widget) ||
                                        !opts.colorSliderMouseOver) &&
         WIDGET_UNCOLOURED_MO_BUTTON != widget && GTK_STATE_PRELIGHT == state &&
         (!sunken || IS_TOGGLE_BUTTON(widget) ||
          (WIDGET_TOOLBAR_BUTTON == widget && opts.coloredTbarMo)));
    gboolean plastikMouseOver = (doColouredMouseOver &&
                                 MO_PLASTIK == opts.coloredMouseOver);
    gboolean colouredMouseOver =
        doColouredMouseOver && qtcOneOf(opts.coloredMouseOver, MO_COLORED,
                                        MO_COLORED_THICK);
    gboolean flatWidget = (WIDGET_PROGRESSBAR == widget &&
                           !opts.borderProgress);
    gboolean lightBorder =
        !flatWidget && DRAW_LIGHT_BORDER(sunken, widget, app);
    gboolean draw3dfull = (!flatWidget && !lightBorder &&
                           DRAW_3D_FULL_BORDER(sunken, app));
    gboolean draw3d =
        (!flatWidget && (draw3dfull ||
                         (!lightBorder && DRAW_3D_BORDER(sunken, app))));
    gboolean drawShine = DRAW_SHINE(sunken, app);
    gboolean bevelledButton = (WIDGET_BUTTON(widget) &&
                               APPEARANCE_BEVELLED == app);
    gboolean doEtch =
        (flags & DF_DO_BORDER &&
         (ETCH_WIDGET(widget) || (WIDGET_COMBO_BUTTON == widget &&
                                  opts.etchEntry)) && DO_EFFECT);
    gboolean glowFocus =
        (doEtch && USE_GLOW_FOCUS(GTK_STATE_PRELIGHT == state) && wid &&
         ((flags&DF_HAS_FOCUS) || gtk_widget_has_focus(wid)) &&
         GTK_STATE_INSENSITIVE != state && !isComboBoxEntryButton(wid) &&
         ((WIDGET_RADIO_BUTTON != widget && WIDGET_CHECKBOX != widget) ||
          GTK_STATE_ACTIVE != state));
    gboolean glowFocusSunkenToggle =
        (sunken && (glowFocus || (doColouredMouseOver &&
                                  MO_GLOW == opts.coloredMouseOver)) &&
         wid && GTK_IS_TOGGLE_BUTTON(wid));
    gboolean horiz = !(flags & DF_VERT);
    int xe = x, ye = y;
    int we = width, he = height;
    int origWidth = width, origHeight = height;
    double xd = x + 0.5, yd = y + 0.5;
    if (CIRCULAR_SLIDER(widget))
        horiz = true;

    if (WIDGET_TROUGH == widget && !opts.borderSbarGroove &&
        flags & DF_DO_BORDER) {
        flags -= DF_DO_BORDER;
    }

    if (WIDGET_COMBO_BUTTON == widget && doEtch) {
        if (ROUNDED_RIGHT == round) {
            x--;
            xd -= 1;
            width++;
        } else if(ROUNDED_LEFT == round) {
            width++;
        }
    }

    if (doEtch) {
        xd += 1;
        x++;
        yd += 1;
        y++;
        width -= 2;
        height -= 2;
        xe = x;
        ye = y;
        we = width;
        he = height;
    }

    if (width > 0 && height > 0) {
        if (!(flags & DF_DO_BORDER)) {
            clipPathRadius(cr, x, y, width, height,
                           qtcGetRadius(&opts, width, height, widget,
                                        RADIUS_EXTERNAL), round);
        } else {
            clipPath(cr, x, y, width, height, widget, RADIUS_EXTERNAL, round);
        }

        drawBevelGradient(cr, area, x, y, width, height, base, horiz,
                          sunken && !IS_TROUGH(widget), app, widget);

        if (plastikMouseOver) {
            if (SLIDER(widget)) {
                int len = SB_SLIDER_MO_LEN(horiz ? width : height);
                int so = lightBorder ? SLIDER_MO_PLASTIK_BORDER : 1;
                int eo = len + so;
                int col = SLIDER_MO_SHADE;

                if (horiz) {
                    drawBevelGradient(cr, area, x + so, y, len, height,
                                      &qtcPalette.mouseover[col], horiz,
                                      sunken, app, widget);
                    drawBevelGradient(cr, area, x + width - eo, y, len,
                                      height, &qtcPalette.mouseover[col],
                                      horiz, sunken, app, widget);
                } else {
                    drawBevelGradient(cr, area, x, y + so, width, len,
                                      &qtcPalette.mouseover[col], horiz,
                                      sunken, app, widget);
                    drawBevelGradient(cr, area, x, y + height - eo, width,
                                      len, &qtcPalette.mouseover[col],
                                      horiz, sunken, app, widget);
                }
            } else {
                int mh = height;
                GdkColor *col = &qtcPalette.mouseover[MO_PLASTIK_DARK(widget)];
                gboolean horizontal =
                    ((horiz && !(WIDGET_SB_BUTTON == widget ||
                                 SLIDER(widget))) ||
                     (!horiz && (WIDGET_SB_BUTTON == widget ||
                                 SLIDER(widget))));
                gboolean thin =
                    (qtcOneOf(widget, WIDGET_SB_BUTTON, WIDGET_SPIN_UP,
                              WIDGET_SPIN_DOWN) ||
                     (horiz ? height : width) < 16);

                if (EFFECT_NONE != opts.buttonEffect &&
                    WIDGET_SPIN_UP == widget && horiz) {
                    mh--;
                }
                cairo_new_path(cr);
                qtcCairoSetColor(cr, col);
                if (horizontal) {
                    cairo_move_to(cr, x + 1, yd + 1);
                    cairo_line_to(cr, x + width -1, yd + 1);
                    cairo_move_to(cr, x + 1, yd + mh - 2);
                    cairo_line_to(cr, x + width - 1, yd + mh - 2);
                } else {
                    cairo_move_to(cr, xd + 1, y + 1);
                    cairo_line_to(cr, xd + 1, y + mh - 1);
                    cairo_move_to(cr, xd + width - 2, y + 1);
                    cairo_line_to(cr, xd + width - 2, y + mh - 1);
                }
                cairo_stroke(cr);
                if (!thin) {
                    col = &qtcPalette.mouseover[MO_PLASTIK_LIGHT(widget)];
                    cairo_new_path(cr);
                    qtcCairoSetColor(cr, col);
                    if (horizontal) {
                        cairo_move_to(cr, x + 1, yd + 2);
                        cairo_line_to(cr, x + width - 1, yd + 2);
                        cairo_move_to(cr, x + 1, yd + mh - 3);
                        cairo_line_to(cr, x + width - 1, yd + mh - 3);
                    } else {
                        cairo_move_to(cr, xd + 2, y + 1);
                        cairo_line_to(cr, xd + 2, y + mh - 1);
                        cairo_move_to(cr, xd + width - 3, y + 1);
                        cairo_line_to(cr, xd + width - 3, y + mh - 1);
                    }
                    cairo_stroke(cr);
                }
            }
        }

        if (drawShine) {
            gboolean mo = GTK_STATE_PRELIGHT == state && opts.highlightFactor;
            int xa = x, ya = y;
            int wa = width, ha = height;

            if (WIDGET_RADIO_BUTTON == widget || CIRCULAR_SLIDER(widget)) {
                double topSize = ha * 0.4;
                double topWidthAdjust = 3.5;
                double topGradRectX = xa + topWidthAdjust;
                double topGradRectY = ya;
                double topGradRectW = wa - topWidthAdjust * 2 - 1;
                double topGradRectH = topSize - 1;
                cairo_pattern_t *pt =
                    cairo_pattern_create_linear(topGradRectX, topGradRectY,
                                                topGradRectX,
                                                topGradRectY + topGradRectH);

                clipPathRadius(cr, topGradRectX + 0.5, topGradRectY + 0.5,
                               topGradRectW, topGradRectH, topGradRectW / 2.0,
                               ROUNDED_ALL);

                cairo_pattern_add_color_stop_rgba(
                    pt, 0.0, 1.0, 1.0, 1.0,
                    mo ? (opts.highlightFactor > 0 ? 0.8 : 0.7) : 0.75);
                cairo_pattern_add_color_stop_rgba(
                    pt, CAIRO_GRAD_END, 1.0, 1.0, 1.0,
                    /*mo ? (opts.highlightFactor>0 ? 0.3 : 0.1) :*/ 0.2);

                cairo_set_source(cr, pt);
                cairo_rectangle(cr, topGradRectX, topGradRectY, topGradRectW,
                                topGradRectH);
                cairo_fill(cr);
                cairo_pattern_destroy(pt);
                cairo_restore(cr);
            } else {
                double size = qtcMin((horiz ? ha : wa) / 2.0, 16);
                double rad = size / 2.0;
                cairo_pattern_t *pt = NULL;
                int mod = 4;

                if (horiz) {
                    if (!(ROUNDED_LEFT & round)) {
                        xa -= 8;
                        wa += 8;
                    }
                    if (!(ROUNDED_RIGHT & round)) {
                        wa += 8;
                    }
                } else {
                    if (!(ROUNDED_TOP & round)) {
                        ya -= 8;
                        ha += 8;
                    }
                    if (!(ROUNDED_BOTTOM & round)) {
                        ha += 8;
                    }
                }
                pt = cairo_pattern_create_linear(
                    xa, ya, xa + (horiz ? 0.0 : size),
                    ya + (horiz ? size : 0.0));

                if (qtcGetWidgetRound(&opts, origWidth, origHeight,
                                      widget) <ROUND_MAX ||
                    (!IS_MAX_ROUND_WIDGET(widget) && !IS_SLIDER(widget))) {
                    rad /= 2.0;
                    mod /= 2;
                }

                if (horiz) {
                    clipPathRadius(cr, xa + mod + 0.5, ya + 0.5,
                                   wa - mod * 2 - 1, size - 1, rad, round);
                } else {
                    clipPathRadius(cr, xa + 0.5, ya + mod + 0.5, size - 1,
                                   ha - mod * 2 - 1, rad, round);
                }

                cairo_pattern_add_color_stop_rgba(
                    pt, 0.0, 1.0, 1.0, 1.0,
                    mo ? (opts.highlightFactor > 0 ? 0.95 : 0.85) : 0.9);
                cairo_pattern_add_color_stop_rgba(
                    pt, CAIRO_GRAD_END, 1.0, 1.0, 1.0,
                    mo ? (opts.highlightFactor > 0 ? 0.3 : 0.1) : 0.2);

                cairo_set_source(cr, pt);
                cairo_rectangle(cr, xa, ya, horiz ? wa : size,
                                horiz ? size : ha);
                cairo_fill(cr);
                cairo_pattern_destroy(pt);
                cairo_restore(cr);
            }
        }
        cairo_restore(cr);
    }
    xd += 1;
    x++;
    yd += 1;
    y++;
    width -= 2;
    height -= 2;

    if (plastikMouseOver /* && !sunken */) {
        gboolean thin = (qtcOneOf(widget, WIDGET_SB_BUTTON, WIDGET_SPIN) ||
                         (horiz ? height : width) < 16);
        gboolean horizontal =
            (SLIDER(widget) ? !horiz : (horiz && WIDGET_SB_BUTTON != widget) ||
             (!horiz && WIDGET_SB_BUTTON == widget));
        int len = (SLIDER(widget) ? SB_SLIDER_MO_LEN(horiz ? width : height) :
                   (thin ? 1 : 2));
        GdkRectangle rect;
        if (horizontal) {
            rect.x = x;
            rect.y = y + len;
            rect.width = width;
            rect.height = height - (len * 2);
        } else {
            rect.x = x + len;
            rect.y = y;
            rect.width = width - len * 2;
            rect.height = height;
        }
        setCairoClipping(cr, &rect);
    } else {
        setCairoClipping(cr, area);
    }

    if (!colouredMouseOver && lightBorder) {
        GdkColor *col = &colors[LIGHT_BORDER(app)];

        cairo_new_path(cr);
        qtcCairoSetColor(cr, col);
        createPath(cr, xd, yd, width - 1, height - 1,
                   qtcGetRadius(&opts, width, height, widget, RADIUS_INTERNAL),
                   round);
        cairo_stroke(cr);
    }  else if (colouredMouseOver ||
                (!sunken && (draw3d || flags & DF_DRAW_INSIDE))) {
        int dark = /*bevelledButton ? */2/* : 4*/;
        GdkColor *col1 =
            (colouredMouseOver ?
             &qtcPalette.mouseover[MO_STD_LIGHT(widget, sunken)] :
             &colors[sunken ? dark : 0]);

        cairo_new_path(cr);
        qtcCairoSetColor(cr, col1);
        createTLPath(cr, xd, yd, width - 1, height - 1,
                     qtcGetRadius(&opts, width, height, widget,
                                  RADIUS_INTERNAL), round);
        cairo_stroke(cr);
        if (colouredMouseOver || bevelledButton || draw3dfull) {
            GdkColor *col2 = colouredMouseOver ?
                &qtcPalette.mouseover[MO_STD_DARK(widget)] :
                &colors[sunken ? 0 : dark];

            cairo_new_path(cr);
            qtcCairoSetColor(cr, col2);
            createBRPath(cr, xd, yd, width - 1, height - 1,
                         qtcGetRadius(&opts, width, height, widget,
                                      RADIUS_INTERNAL), round);
            cairo_stroke(cr);
        }
    }

    cairo_restore(cr);

    if ((doEtch || glowFocus) && !(flags & DF_HIDE_EFFECT)) {
        if ((!sunken || glowFocusSunkenToggle) &&
            GTK_STATE_INSENSITIVE != state && !(opts.thin&THIN_FRAMES) &&
            ((WIDGET_OTHER != widget && WIDGET_SLIDER_TROUGH != widget &&
              WIDGET_COMBO_BUTTON != widget &&
              MO_GLOW == opts.coloredMouseOver &&
              GTK_STATE_PRELIGHT == state) || glowFocus ||
             (WIDGET_DEF_BUTTON == widget &&
              IND_GLOW == opts.defBtnIndicator))) {
            drawGlow(cr, area, xe - 1, ye - 1, we + 2, he + 2, round,
                     (widget == WIDGET_DEF_BUTTON &&
                      state == GTK_STATE_PRELIGHT ? WIDGET_STD_BUTTON : widget),
                     glowFocus ? qtcPalette.focus : NULL);
        } else {
            drawEtch(cr, area, wid,
                     xe - (WIDGET_COMBO_BUTTON == widget ?
                           (ROUNDED_RIGHT==round ? 3 : 1) : 1), ye - 1,
                     we + (WIDGET_COMBO_BUTTON == widget ?
                           (ROUNDED_RIGHT == round ? 4 : 5) : 2), he + 2,
                     EFFECT_SHADOW == opts.buttonEffect &&
                     WIDGET_COMBO_BUTTON != widget && WIDGET_BUTTON(widget) &&
                     !sunken, round, widget);
        }
    }

    xd -= 1;
    x--;
    yd -= 1;
    y--;
    width += 2;
    height += 2;
    if (flags & DF_DO_BORDER && width > 2 && height > 2) {
        GdkColor *borderCols =
            (glowFocus && (!sunken || glowFocusSunkenToggle) ?
             qtcPalette.focus : qtcOneOf(widget, WIDGET_COMBO,
                                         WIDGET_COMBO_BUTTON) &&
             qtcPalette.combobtn == colors ?
             state == GTK_STATE_PRELIGHT && opts.coloredMouseOver == MO_GLOW &&
             !sunken ? qtcPalette.mouseover :
             qtcPalette.button[PAL_ACTIVE] : colors);

        cairo_new_path(cr);
        /* Yuck! this is a mess!!!! */
// Copied from KDE4 version...
        if (!sunken && state != GTK_STATE_INSENSITIVE && !glowFocus &&
            ((((doEtch && qtcNoneOf(widget, WIDGET_OTHER,
                                    WIDGET_SLIDER_TROUGH)) || SLIDER(widget) ||
               qtcOneOf(widget, WIDGET_COMBO, WIDGET_MENU_BUTTON)) &&
              (opts.coloredMouseOver == MO_GLOW
               /* || opts.colorMenubarMouseOver == MO_COLORED */) &&
              state == GTK_STATE_PRELIGHT) ||
             (doEtch && widget == WIDGET_DEF_BUTTON &&
              opts.defBtnIndicator == IND_GLOW))) {

// Previous Gtk2...
//         if(!sunken && (doEtch || SLIDER(widget)) &&
//             ( (WIDGET_OTHER!=widget && WIDGET_SLIDER_TROUGH!=widget && WIDGET_COMBO_BUTTON!=widget &&
//                 MO_GLOW==opts.coloredMouseOver && GTK_STATE_PRELIGHT==state) ||
//               (WIDGET_DEF_BUTTON==widget && IND_GLOW==opts.defBtnIndicator)))
            drawBorder(cr, style, state, area, x, y, width, height,
                       widget == WIDGET_DEF_BUTTON &&
                       opts.defBtnIndicator == IND_GLOW &&
                       (state != GTK_STATE_PRELIGHT || !qtcPalette.mouseover) ?
                       qtcPalette.defbtn : qtcPalette.mouseover,
                       round, borderProfile, widget, flags);
        } else {
            drawBorder(cr, style, state, area, x, y, width, height,
                       colouredMouseOver &&
                       opts.coloredMouseOver == MO_COLORED_THICK ?
                       qtcPalette.mouseover : borderCols,
                       round, borderProfile, widget, flags);
        }
    }

    if (WIDGET_SB_SLIDER == widget && opts.stripedSbar) {
        clipPathRadius(cr, x, y, width, height,
                       qtcGetRadius(&opts, width, height, WIDGET_SB_SLIDER,
                                    RADIUS_INTERNAL), round);
        addStripes(cr, x + 1, y + 1, width - 2, height - 2, horiz);
        cairo_restore(cr);
    }
}

void
drawFadedLine(cairo_t *cr, int x, int y, int width, int height, GdkColor *col,
              GdkRectangle *area, GdkRectangle *gap, gboolean fadeStart,
              gboolean fadeEnd, gboolean horiz, double alpha)
{
    double rx=x+0.5,
        ry=y+0.5;
    cairo_pattern_t *pt =
        cairo_pattern_create_linear(rx, ry, horiz ? rx + width - 1 : rx + 1,
                                    horiz ? ry + 1 : ry + height - 1);

    if (gap) {
        GdkRectangle r = {x, y, width, height};
        GdkRegion *region = gdk_region_rectangle(area ? area : &r);
        GdkRegion *inner = gdk_region_rectangle(gap);

        gdk_region_xor(region, inner);
        setCairoClippingRegion(cr, region);
        gdk_region_destroy(inner);
        gdk_region_destroy(region);
    } else {
        setCairoClipping(cr, area);
    }
    qtcCairoPatternAddColorStop(pt, 0, col,
                                fadeStart && opts.fadeLines ? 0.0 : alpha);
    qtcCairoPatternAddColorStop(pt, FADE_SIZE, col, alpha);
    qtcCairoPatternAddColorStop(pt, 1.0 - FADE_SIZE, col, alpha);
    qtcCairoPatternAddColorStop(pt, CAIRO_GRAD_END, col,
                                fadeEnd && opts.fadeLines ? 0.0 : alpha);
    cairo_set_source(cr, pt);
    if (horiz) {
        cairo_move_to(cr, x, ry);
        cairo_line_to(cr, x + width - 1, ry);
    } else {
        cairo_move_to(cr, rx, y);
        cairo_line_to(cr, rx, y + height - 1);
    }
    cairo_stroke(cr);
    cairo_pattern_destroy(pt);
    cairo_restore(cr);
}

void
drawHighlight(cairo_t *cr, int x, int y, int width, int height,
              GdkRectangle *area, gboolean horiz, gboolean inc)
{
    drawFadedLine(cr, x, y, width, height,
                  &qtcPalette.mouseover[ORIGINAL_SHADE], area, NULL, true,
                  true, horiz, inc ? 0.5 : 1.0);
    drawFadedLine(cr, x + (horiz ? 0 : 1), y + (horiz ? 1 : 0), width, height,
                  &qtcPalette.mouseover[ORIGINAL_SHADE],
                  area, NULL, true, true, horiz, inc ? 1.0 : 0.5);
}

void setLineCol(cairo_t *cr, cairo_pattern_t *pt, GdkColor *col)
{
    if (pt) {
        qtcCairoPatternAddColorStop(pt, 0, col, 0.0);
        qtcCairoPatternAddColorStop(pt, 0.4, col);
        qtcCairoPatternAddColorStop(pt, 0.6, col);
        qtcCairoPatternAddColorStop(pt, CAIRO_GRAD_END, col, 0.0);
        cairo_set_source(cr, pt);
    } else {
        qtcCairoSetColor(cr, col);
    }
}

void drawLines(cairo_t *cr, double rx, double ry, int rwidth, int rheight, gboolean horiz,
               int nLines, int offset, GdkColor *cols, GdkRectangle *area, int dark, ELine type)
{
    if(horiz)
        ry+=0.5,  rwidth+=1;
    else
        rx+=0.5,  rheight+=1;

    {
        int             space = (nLines*2)+(LINE_DASHES!=type ? (nLines-1) : 0),
            step = LINE_DASHES!=type ? 3 : 2,
            etchedDisp = LINE_SUNKEN==type ? 1 : 0;
        double x = horiz ? rx : rx + (rwidth - space) / 2;
        double y = horiz ? ry + (rheight - space) / 2 : ry;
        double x2 = rx + rwidth - 1;
        double y2 = ry + rheight - 1;
        GdkColor *col1 = &cols[dark];
        GdkColor *col2 = &cols[0];
        cairo_pattern_t *pt1=(opts.fadeLines && (horiz ? rwidth : rheight)>(16+etchedDisp))
            ? cairo_pattern_create_linear(rx, ry, horiz ? x2 : rx+1, horiz ? ry+1 : y2)
            : NULL,
            *pt2=(pt1 && LINE_FLAT!=type)
            ? cairo_pattern_create_linear(rx, ry, horiz ? x2 : rx+1, horiz ? ry+1 : y2)
            : NULL;

        setCairoClipping(cr, area);
        setLineCol(cr, pt1, col1);

        if (horiz) {
            for (int i = 0;i < space;i += step) {
                cairo_move_to(cr, x + offset, y + i);
                cairo_line_to(cr, x2 - offset, y + i);
            }
            cairo_stroke(cr);

            if (type != LINE_FLAT) {
                setLineCol(cr, pt2, col2);
                x+=etchedDisp;
                x2+=etchedDisp;
                for (int i = 1;i < space;i += step) {
                    cairo_move_to(cr, x + offset, y + i);
                    cairo_line_to(cr, x2 - offset, y + i);
                }
                cairo_stroke(cr);
            }
        } else {
            for (int i = 0;i < space;i += step) {
                cairo_move_to(cr, x + i, y + offset);
                cairo_line_to(cr, x + i, y2 - offset);
            }
            cairo_stroke(cr);
            if (type != LINE_FLAT) {
                setLineCol(cr, pt2, col2);
                y += etchedDisp;
                y2 += etchedDisp;
                for (int i = 1;i < space;i += step) {
                    cairo_move_to(cr, x + i, y + offset);
                    cairo_line_to(cr, x + i, y2 - offset);
                }
                cairo_stroke(cr);
            }
        }
        if (pt1) {
            cairo_pattern_destroy(pt1);
        }
        if (pt2) {
            cairo_pattern_destroy(pt2);
        }
        cairo_restore(cr);
    }
}


void drawDot(cairo_t *cr, int x, int y, int w, int h, GdkColor *cols)
{
    double dx = x + (w - 5) / 2;
    double dy = y + (h - 5) / 2;
    cairo_pattern_t *p1 = cairo_pattern_create_linear(dx, dy, dx + 4, dy + 4);
    cairo_pattern_t *p2 = cairo_pattern_create_linear(dx + 2, dy + 2,
                                                      dx + 4, dx + 4);

    qtcCairoPatternAddColorStop(p1, 0.0, &cols[QTC_STD_BORDER], 1.0);
    qtcCairoPatternAddColorStop(p1, CAIRO_GRAD_END, &cols[QTC_STD_BORDER], 0.4);
    cairo_pattern_add_color_stop_rgba(p2, CAIRO_GRAD_END, 1.0, 1.0, 1.0, 0.9);
    cairo_pattern_add_color_stop_rgba(p2, 0.0, 1.0, 1.0, 1.0, 0.7);

    cairo_new_path(cr);
    cairo_arc(cr, dx+2.5, dy+2.5, 2.5, 0, 2*M_PI);
    cairo_clip(cr);
    cairo_set_source(cr, p1);
    cairo_rectangle(cr, dx, dy, 5, 5);
    cairo_fill(cr);

    cairo_new_path(cr);
    cairo_arc(cr, dx+3, dy+3, 2, 0, 2*M_PI);
    cairo_clip(cr);
    cairo_set_source(cr, p2);
    cairo_rectangle(cr, dx+1, dy+1, 4, 4);
    cairo_fill(cr);
    cairo_pattern_destroy(p1);
    cairo_pattern_destroy(p2);
}

void drawDots(cairo_t *cr, int rx, int ry, int rwidth, int rheight, gboolean horiz, int nLines, int offset, GdkColor *cols, GdkRectangle *area,
              int startOffset, int dark)
{
    int space = nLines * 2 + nLines - 1;
    int x = horiz ? rx : rx + (rwidth - space) / 2;
    int y = horiz ? ry + (rheight - space) / 2 : ry;
    int numDots = ((horiz ? rwidth : rheight) - 2 * offset) / 3 + 1;
    GdkColor *col1 = &cols[dark];
    GdkColor *col2 = &cols[0];

    setCairoClipping(cr, area);
    if (horiz) {
        if (startOffset && y + startOffset > 0) {
            y += startOffset;
        }
        cairo_new_path(cr);
        qtcCairoSetColor(cr, col1);
        for (int i = 0;i < space;i += 3) {
            for (int j = 0;j < numDots;j++) {
                cairo_rectangle(cr, x + offset + 3 * j, y + i, 1, 1);
            }
        }
        cairo_fill(cr);

        cairo_new_path(cr);
        qtcCairoSetColor(cr, col2);
        for (int i = 1;i < space;i += 3) {
            for (int j = 0;j < numDots;j++) {
                cairo_rectangle(cr, x + offset + 1 + 3 * j, y + i, 1, 1);
            }
        }
        cairo_fill(cr);
    } else {
        if (startOffset && x + startOffset > 0) {
            x += startOffset;
        }
        cairo_new_path(cr);
        qtcCairoSetColor(cr, col1);
        for (int i = 0;i < space;i += 3) {
            for (int j = 0;j < numDots;j++) {
                cairo_rectangle(cr, x + i, y + offset + 3 * j, 1, 1);
            }
        }
        cairo_fill(cr);

        cairo_new_path(cr);
        qtcCairoSetColor(cr, col2);
        for (int i = 1;i < space;i += 3) {
            for(int j = 0;j < numDots;j++) {
                cairo_rectangle(cr, x + i, y + offset + 1 + 3 * j, 1, 1);
            }
        }
        cairo_fill(cr);
    }
    cairo_restore(cr);
}

void
drawEntryCorners(cairo_t *cr, GdkRectangle *area, int round, int x, int y,
                 int width, int height, const GdkColor *col, double a)
{
    setCairoClipping(cr, area);
    qtcCairoSetColor(cr, col, a);
    cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
    if(DO_EFFECT && opts.etchEntry)
        cairo_rectangle(cr, x+1.5, y+1.5, width-2, height-3);
    if(opts.round>ROUND_FULL)
    {
        if(round&CORNER_TL)
            cairo_rectangle(cr, x+2.5, y+2.5, 1, 1);
        if(round&CORNER_BL)
            cairo_rectangle(cr, x+2.5, y+height-3.5, 1, 1);
        if(round&CORNER_TR)
            cairo_rectangle(cr, x+width-2.5, y+2.5, 1, 1);
        if(round&CORNER_BR)
            cairo_rectangle(cr, x+width-2.5, y+height-3.5, 1, 1);
    }
    cairo_set_line_width(cr, opts.round>ROUND_FULL && GTK_APP_OPEN_OFFICE!=qtSettings.app ? 2 : 1);
    cairo_stroke(cr);
    cairo_restore(cr);
}

void drawBgndRing(cairo_t *cr, int x, int y, int size, int size2, gboolean isWindow)
{
    double width=(size-size2)/2.0,
        width2=width/2.0,
        radius=(size2+width)/2.0;

    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, RINGS_INNER_ALPHA(isWindow ? opts.bgndImage.type : opts.menuBgndImage.type));
    cairo_set_line_width(cr, width);
    cairo_arc(cr, x+radius+width2+0.5, y+radius+width2+0.5, radius, 0, 2*M_PI);
    cairo_stroke(cr);

    if(IMG_BORDERED_RINGS==(isWindow ? opts.bgndImage.type : opts.menuBgndImage.type))
    {
        cairo_set_line_width(cr, 1);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, RINGS_OUTER_ALPHA);
        cairo_arc(cr, x+radius+width2+0.5, y+radius+width2+0.5, size/2.0, 0, 2*M_PI);
        if(size2)
        {
            cairo_stroke(cr);
            cairo_arc(cr, x+radius+width2+0.5, y+radius+width2+0.5, size2/2.0, 0, 2*M_PI);
        }
        cairo_stroke(cr);
    }
}

void drawBgndRings(cairo_t *cr, int x, int y, int width, int height, gboolean isWindow)
{
    static cairo_surface_t *bgndImage=NULL;
    static cairo_surface_t *menuBgndImage=NULL;

    bool useWindow=isWindow || (opts.bgndImage.type==opts.menuBgndImage.type &&
                                (IMG_FILE!=opts.bgndImage.type ||
                                 (opts.bgndImage.height==opts.menuBgndImage.height &&
                                  opts.bgndImage.width==opts.menuBgndImage.width &&
                                  opts.bgndImage.pixmap.file==opts.menuBgndImage.pixmap.file)));
    QtCImage *img=useWindow ? &opts.bgndImage : &opts.menuBgndImage;
    int      imgWidth=IMG_FILE==img->type ? img->width : RINGS_WIDTH(img->type),
        imgHeight=IMG_FILE==img->type ? img->height : RINGS_HEIGHT(img->type);

    switch(img->type)
    {
    case IMG_NONE:
        break;
    case IMG_FILE:
        qtcLoadBgndImage(img);
        if(img->pixmap.img)
        {
            switch(img->pos)
            {
            case PP_TL:
                gdk_cairo_set_source_pixbuf(cr, img->pixmap.img, x, y);
                break;
            case PP_TM:
                gdk_cairo_set_source_pixbuf(cr, img->pixmap.img, x+((width-img->width)/2), y);
                break;
            default:
            case PP_TR:
                gdk_cairo_set_source_pixbuf(cr, img->pixmap.img, x+(width-img->width)-1, y);
                break;
            case PP_BL:
                gdk_cairo_set_source_pixbuf(cr, img->pixmap.img, x, y+(height-img->height));
                break;
            case PP_BM:
                gdk_cairo_set_source_pixbuf(cr, img->pixmap.img, x+((width-img->width)/2), y+(height-img->height)-1);
                break;
            case PP_BR:
                gdk_cairo_set_source_pixbuf(cr, img->pixmap.img, x+(width-img->width)-1, y+(height-img->height)-1);
                break;
            case PP_LM:
                gdk_cairo_set_source_pixbuf(cr, img->pixmap.img, x, y+((height-img->height)/2));
                break;
            case PP_RM:
                gdk_cairo_set_source_pixbuf(cr, img->pixmap.img, x+(width-img->width)-1, y+((height-img->height)/2));
                break;
            case PP_CENTRED:
                gdk_cairo_set_source_pixbuf(cr, img->pixmap.img, x+((width-img->width)/2), y+((height-img->height)/2));
            }
            cairo_paint(cr);
        }
        break;
    case IMG_PLAIN_RINGS:
    case IMG_BORDERED_RINGS:
    {
        cairo_surface_t *crImg=useWindow ? bgndImage : menuBgndImage;

        if(!crImg)
        {
            cairo_t *ci;
            crImg=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, imgWidth+1, imgHeight+1);
            ci=cairo_create(crImg);
            drawBgndRing(ci, 0, 0, 200, 140, isWindow);

            drawBgndRing(ci, 210, 10, 230, 214, isWindow);
            drawBgndRing(ci, 226, 26, 198, 182, isWindow);
            drawBgndRing(ci, 300, 100, 50, 0, isWindow);

            drawBgndRing(ci, 100, 96, 160, 144, isWindow);
            drawBgndRing(ci, 116, 112, 128, 112, isWindow);

            drawBgndRing(ci, 250, 160, 200, 140, isWindow);
            drawBgndRing(ci, 310, 220, 80, 0, isWindow);
            cairo_destroy(ci);
            if(useWindow)
                bgndImage=crImg;
            else
                menuBgndImage=crImg;
        }

        cairo_set_source_surface(cr, crImg, width-imgWidth, y+1);
        cairo_paint(cr);
        break;
    }
    case IMG_SQUARE_RINGS:
    {
        cairo_surface_t *crImg=useWindow ? bgndImage : menuBgndImage;

        if(!crImg)
        {
            cairo_t *ci;
            double  halfWidth=RINGS_SQUARE_LINE_WIDTH/2.0;
            crImg=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, imgWidth+1, imgHeight+1);
            ci=cairo_create(crImg);

            cairo_set_source_rgba(ci, 1.0, 1.0, 1.0, RINGS_SQUARE_SMALL_ALPHA);
            cairo_set_line_width(ci, RINGS_SQUARE_LINE_WIDTH);
            createPath(ci, halfWidth+0.5, halfWidth+0.5, RINGS_SQUARE_SMALL_SIZE, RINGS_SQUARE_SMALL_SIZE,
                       RINGS_SQUARE_RADIUS, ROUNDED_ALL);
            cairo_stroke(ci);

            cairo_new_path(ci);
            cairo_set_source_rgba(ci, 1.0, 1.0, 1.0, RINGS_SQUARE_SMALL_ALPHA);
            cairo_set_line_width(ci, RINGS_SQUARE_LINE_WIDTH);
            createPath(ci, halfWidth+0.5+(imgWidth-(RINGS_SQUARE_SMALL_SIZE+RINGS_SQUARE_LINE_WIDTH)),
                       halfWidth+0.5+(imgHeight-(RINGS_SQUARE_SMALL_SIZE+RINGS_SQUARE_LINE_WIDTH)),
                       RINGS_SQUARE_SMALL_SIZE, RINGS_SQUARE_SMALL_SIZE,
                       RINGS_SQUARE_RADIUS, ROUNDED_ALL);
            cairo_stroke(ci);

            cairo_new_path(ci);
            cairo_set_source_rgba(ci, 1.0, 1.0, 1.0, RINGS_SQUARE_LARGE_ALPHA);
            cairo_set_line_width(ci, RINGS_SQUARE_LINE_WIDTH);
            createPath(ci, halfWidth+0.5+((imgWidth-RINGS_SQUARE_LARGE_SIZE-RINGS_SQUARE_LINE_WIDTH)/2.0),
                       halfWidth+0.5+((imgHeight-RINGS_SQUARE_LARGE_SIZE-RINGS_SQUARE_LINE_WIDTH)/2.0),
                       RINGS_SQUARE_LARGE_SIZE, RINGS_SQUARE_LARGE_SIZE,
                       RINGS_SQUARE_RADIUS, ROUNDED_ALL);
            cairo_stroke(ci);

            cairo_destroy(ci);

            if(useWindow)
                bgndImage=crImg;
            else
                menuBgndImage=crImg;
        }

        cairo_set_source_surface(cr, crImg, width-imgWidth, y+1);
        cairo_paint(cr);
        break;
    }
    }
}

void
drawBgndImage(cairo_t *cr, GtkStyle *style, GdkRectangle *area, int x, int y,
              int w, int h, GdkColor *col, gboolean isWindow, double alpha)
{
    QTC_UNUSED(style);
    QTC_UNUSED(area);
    QTC_UNUSED(col);
    QTC_UNUSED(alpha);
    GdkPixbuf *pix = isWindow ? opts.bgndPixmap.img : opts.menuBgndPixmap.img;
    /* getPixbuf(getCheckRadioCol(style, ind_state, mnu), PIX_CHECK, 1.0); */
    /* int       pw=gdk_pixbuf_get_width(pix), */
    /*           ph=gdk_pixbuf_get_height(pix), */
    /*           dx=(x+(opts.crSize/2))-(pw/2), */
    /*           dy=(y+(opts.crSize/2))-(ph/2); */

    if (pix) {
        gdk_cairo_set_source_pixbuf(cr, pix, 0, 0);
        cairo_pattern_set_extend(cairo_get_source(cr), CAIRO_EXTEND_REPEAT);
        cairo_rectangle(cr, x, y, w, h);
        cairo_fill(cr);
    }
}

void
drawStripedBgnd(cairo_t *cr, GtkStyle *style, GdkRectangle *area,
                int x, int y, int w, int h, GdkColor *col,
                gboolean isWindow, double alpha)
{
    QTC_UNUSED(style);
    QTC_UNUSED(area);
    QTC_UNUSED(isWindow);
    GdkColor col2;
    qtcShade(col, &col2, BGND_STRIPE_SHADE, opts.shading);

    cairo_pattern_t *pat = cairo_pattern_create_linear(x, y, x, y + 4);
    qtcCairoPatternAddColorStop(pat, 0.0, col, alpha);
    qtcCairoPatternAddColorStop(pat, 0.25 - 0.0001, col, alpha);
    qtcCairoPatternAddColorStop(pat, 0.5, &col2, alpha);
    qtcCairoPatternAddColorStop(pat, 0.75 - 0.0001, &col2, alpha);
    col2.red=(3*col->red+col2.red)/4;
    col2.green=(3*col->green+col2.green)/4;
    col2.blue=(3*col->blue+col2.blue)/4;
    qtcCairoPatternAddColorStop(pat, 0.25, &col2, alpha);
    qtcCairoPatternAddColorStop(pat, 0.5 - 0.0001, &col2, alpha);
    qtcCairoPatternAddColorStop(pat, 0.75, &col2, alpha);
    qtcCairoPatternAddColorStop(pat, CAIRO_GRAD_END, &col2, alpha);

    cairo_pattern_set_extend(pat, CAIRO_EXTEND_REPEAT);
    cairo_set_source(cr, pat);
    cairo_rectangle(cr, x, y, w, h);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);
/*
  TODO: Use image? Would this speed up drawing???
  static cairo_surface_t *bgndStripedImage=NULL;
  static GdkColor        bgndStripedImageColor;
  static cairo_surface_t *menuBgndStripedImage=NULL;
  static GdkColor        menuBgndStripedImageColor;

  bool            useWindow=isWindow || (bgndStripedImage && *col==bgndStripedImageColor));
  GdkColor        *imgCol=useWindow ? &bgndStripedImageColor : &menuBgndStripedImageColor;
  cairo_surface_t **img=useWindow ? &bgndStripedImage : &menuBgndStripedImage;

  if(!(*img) || *imgCol!=*col)
  {
  static const inst constSize=64;

  GdkColor col2;
  cairo_t  *ci;

  qtcShade(&opts, col, &col2, BGND_STRIPE_SHADE);

  if(!(*img)
  *img=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, constSize, constSize);
  ci=cairo_create(crImg);
  }
  cairo_set_source_surface(cr, *img, width-imgWidth, y+1);
  cairo_paint(cr);
*/
}

gboolean drawWindowBgnd(cairo_t *cr, GtkStyle *style, GdkRectangle *area, GdkWindow *window, GtkWidget *widget,
                        int x, int y, int width, int height)
{
    GtkWidget *parent=NULL;
    if(widget && (parent=gtk_widget_get_parent(widget)) && isOnHandlebox(parent, NULL, 0))
        return TRUE;

    if(!isFixedWidget(widget) && !isGimpDockable(widget))
    {
        int wx=0, wy=0, wh, ww;

        if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d  ", __FUNCTION__, x, y, width, height), debugDisplayWidget(widget, 20);

        if (!mapToTopLevel(window, widget, &wx, &wy, &ww, &wh)) {
            return FALSE;
        } else {
            GdkRectangle clip;
            GtkWidget *topLevel = gtk_widget_get_toplevel(widget);
            int opacity = (!topLevel || !GTK_IS_DIALOG(topLevel) ?
                           opts.bgndOpacity : opts.dlgOpacity);
            int xmod=0, ymod=0, wmod=0, hmod=0;
            double alpha=1.0;
            gboolean useAlpha = (opacity < 100 && isRgbaWidget(topLevel) &&
                                 compositingActive(topLevel));
            gboolean flatBgnd = qtcIsFlatBgnd(opts.bgndAppearance);
            GdkColor      *col=NULL;
            GtkStyle      *topStyle=gtk_widget_get_style(topLevel);

            if(topStyle)
                col=&topStyle->bg[GTK_STATE_NORMAL];
            else
            {
                col=getParentBgCol(widget);
                if(!col)
                    col=&style->bg[GTK_STATE_NORMAL];
            }

            if(!flatBgnd || BGND_IMG_ON_BORDER)
            {
                WindowBorders borders=qtcGetWindowBorderSize(false);
                xmod=borders.sides;
                ymod=borders.titleHeight;
                wmod=2*borders.sides;
                hmod=borders.titleHeight+borders.bottom;
                wy+=ymod, wx+=xmod, wh+=hmod, ww+=wmod;
            }

            clip.x=x, clip.y=y, clip.width=width, clip.height=height;
            setCairoClipping(cr, &clip);

            if(useAlpha)
            {
                alpha=opacity/100.0;
                cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            }

            if(flatBgnd)
                drawAreaColor(cr, area, col, -wx, -wy, ww, wh, alpha);
            else if(APPEARANCE_STRIPED==opts.bgndAppearance)
                drawStripedBgnd(cr, style, area, -wx, -wy, ww, wh, col, TRUE, alpha);
            else if(APPEARANCE_FILE==opts.bgndAppearance)
            {
                cairo_save(cr);
                cairo_translate(cr, -wx, -wy);
                drawBgndImage(cr, style, area, 0, 0, ww, wh, col, TRUE, alpha);
                cairo_restore(cr);
            } else {
                drawBevelGradient(cr, area, -wx, -wy, ww, wh + 1, col,
                                  opts.bgndGrad == GT_HORIZ, FALSE,
                                  opts.bgndAppearance, WIDGET_OTHER, alpha);

                if(GT_HORIZ == opts.bgndGrad &&
                   GB_SHINE == qtcGetGradient(opts.bgndAppearance,
                                              &opts)->border) {
                    int size = qtcMin(BGND_SHINE_SIZE, qtcMin(wh * 2, ww));
                    double alpha = qtcShineAlpha(col);
                    cairo_pattern_t *pat=NULL;

                    size/=BGND_SHINE_STEPS;
                    size*=BGND_SHINE_STEPS;
                    pat=cairo_pattern_create_radial((ww/2.0)-wx, -wy, 0, (ww/2.0)-wx, -wy, size/2.0);
                    cairo_pattern_add_color_stop_rgba(pat, 0, 1, 1, 1, alpha);
                    cairo_pattern_add_color_stop_rgba(pat, 0.5, 1, 1, 1, alpha*0.625);
                    cairo_pattern_add_color_stop_rgba(pat, 0.75, 1, 1, 1, alpha*0.175);
                    cairo_pattern_add_color_stop_rgba(pat, CAIRO_GRAD_END, 1, 1, 1, 0.0);
                    cairo_set_source(cr, pat);
                    cairo_rectangle(cr, ((ww-size)/2.0)-wx, -wy, size, size);
                    cairo_fill(cr);
                    cairo_pattern_destroy(pat);
                }
            }
            if(useAlpha)
                cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

            if(!BGND_IMG_ON_BORDER)
                ww-=wmod+1, wh-=hmod, wx-=xmod, wy-=ymod;

            drawBgndRings(cr, -wx, -wy, ww, wh, TRUE);
            cairo_restore(cr);
            return TRUE;
        }
    }
    return FALSE;
}

void
drawEntryField(cairo_t *cr, GtkStyle *style, GtkStateType state,
               GdkWindow *window, GtkWidget *widget, GdkRectangle *area,
               int x, int y, int width, int height, int round, EWidget w)
{
    gboolean enabled = !(GTK_STATE_INSENSITIVE == state ||
                         (widget && !gtk_widget_is_sensitive(widget)));
    gboolean highlightReal =
        (enabled && widget && gtk_widget_has_focus(widget) &&
         GTK_APP_JAVA != qtSettings.app && qtcPalette.focus);
    gboolean mouseOver =
        (ENTRY_MO && enabled && (GTK_STATE_PRELIGHT == state ||
                                 qtcEntryIsLastMo(widget)) &&
         qtcPalette.mouseover && GTK_APP_JAVA != qtSettings.app);
    gboolean highlight = highlightReal || mouseOver;
    gboolean doEtch = DO_EFFECT && opts.etchEntry;
    gboolean comboOrSpin = qtcOneOf(w, WIDGET_SPIN, WIDGET_COMBO_BUTTON);
    GdkColor *colors = (mouseOver ? qtcPalette.mouseover : highlightReal ?
                        qtcPalette.focus : qtcPalette.background);

    if (GTK_APP_JAVA != qtSettings.app)
        qtcEntrySetup(widget);

    if ((doEtch || ROUND_NONE != opts.round) &&
        (!widget || !g_object_get_data(G_OBJECT(widget),
                                       "transparent-bg-hint"))) {
        if (qtcIsFlatBgnd(opts.bgndAppearance) || !widget ||
            !drawWindowBgnd(cr, style, area, window, widget, x, y,
                            width, height)) {
            GdkColor parentBgCol;
            getEntryParentBgCol(widget, &parentBgCol);
            drawEntryCorners(cr, area, round, x, y, width, height,
                             &parentBgCol, 1.0);
        }
    }

    if (0 != opts.gbFactor && qtcOneOf(opts.groupBox, FRAME_SHADED,
                                       FRAME_FADED) &&
        isInGroupBox(widget, 0)) {
        GdkColor col;
        col.red = col.green = col.blue = opts.gbFactor < 0 ? 0.0 : 1.0;
        drawEntryCorners(cr, area, round, x, y, width, height, &col,
                         TO_ALPHA(opts.gbFactor));
    }

    if (doEtch) {
        y++;
        x++;
        height -= 2;
        width -= 2;
    }

    if (DEBUG_ALL == qtSettings.debug) {
        printf(DEBUG_PREFIX "%s %d %d %d %d %d %d ", __FUNCTION__,
               state, x, y, width, height, round);
        debugDisplayWidget(widget, 10);
    }

    if (ROUNDED_ALL != round) {
        if (comboOrSpin) {
            x -= 2;
            width += 2;
        } else if (highlight) {
            if (doEtch) {
                if (ROUNDED_RIGHT == round) {
                    /* RtoL */
                    x--;
                } else {
                    width++;
                }
            }
        } else {
            if (ROUNDED_RIGHT == round) {
                /* RtoL */
                x -= 2;
            } else {
                width += 2;
            }
        }
    }

    /*if (GTK_APP_OPEN_OFFICE != qtSettings.app)*/ {
        if (opts.round > ROUND_FULL)
            clipPath(cr, x + 1, y + 1, width - 2, height - 2,
                     WIDGET_ENTRY, RADIUS_INTERNAL, ROUNDED_ALL);
        drawAreaColor(cr, area, enabled ? &style->base[GTK_STATE_NORMAL] :
                      &style->bg[GTK_STATE_INSENSITIVE], x + 1, y + 1,
                      width - 2, height - 2);

        if (opts.round > ROUND_FULL) {
            cairo_restore(cr);
        }
    }

    if (GTK_APP_OPEN_OFFICE == qtSettings.app && comboOrSpin) {
        GdkRectangle rect;

        rect.x = x;
        rect.width = width;
        rect.y = y;
        rect.height = height;
        x -= 4;
        width += 4;
        setCairoClipping(cr, &rect);
    }

    int xo = x;
    int yo = y;
    int widtho = width;
    int heighto = height;

    if (doEtch) {
        GdkRectangle rect;
        GdkRegion *region = NULL;

        y--;
        height+=2;
        x--;
        width+=2;

        rect.x=x; rect.y=y; rect.width=width; rect.height=height;
        region=gdk_region_rectangle(&rect);

        if(!(WIDGET_SPIN==w && opts.unifySpin) && !(WIDGET_COMBO_BUTTON==w && opts.unifyCombo))
        {
            if(!(round&CORNER_TR) && !(round&CORNER_BR))
                width+=4;
            if(!(round&CORNER_TL) && !(round&CORNER_BL))
                x-=4;
        }

        drawEtch(cr, area, widget, x, y, width, height, FALSE, round, WIDGET_ENTRY);
        gdk_region_destroy(region);
    }

    drawBorder(cr, style, !widget || gtk_widget_is_sensitive(widget) ? state : GTK_STATE_INSENSITIVE, area, xo, yo, widtho, heighto,
               colors, round, BORDER_SUNKEN, WIDGET_ENTRY, DF_BLEND);

    if(GTK_APP_OPEN_OFFICE==qtSettings.app && comboOrSpin)
        cairo_restore(cr);

    if(GTK_IS_ENTRY(widget) && !gtk_entry_get_visibility(GTK_ENTRY(widget)))
        gtk_entry_set_invisible_char(GTK_ENTRY(widget), opts.passwordChar);
}

void setProgressStripeClipping(cairo_t *cr, GdkRectangle *area, int x, int y, int width, int height, int animShift, gboolean horiz)
{
    int stripeOffset;

    switch(opts.stripedProgress)
    {
    default:
    case STRIPE_PLAIN:
    {
        GdkRectangle rect = {x, y, width - 2, height - 2};
        GdkRegion *region = NULL;

#if !GTK_CHECK_VERSION(2, 90, 0)
        constrainRect(&rect, area);
#endif
        region=gdk_region_rectangle(&rect);
        if(horiz)
            for(stripeOffset=0; stripeOffset<(width+PROGRESS_CHUNK_WIDTH); stripeOffset+=(PROGRESS_CHUNK_WIDTH*2))
            {
                GdkRectangle innerRect={x+stripeOffset+animShift, y+1, PROGRESS_CHUNK_WIDTH, height-2};

#if !GTK_CHECK_VERSION(2, 90, 0)
                constrainRect(&innerRect, area);
#endif
                if (innerRect.width > 0 && innerRect.height > 0) {
                    GdkRegion *innerRegion = gdk_region_rectangle(&innerRect);

                    gdk_region_xor(region, innerRegion);
                    gdk_region_destroy(innerRegion);
                }
            }
        else
            for(stripeOffset=0; stripeOffset<(height+PROGRESS_CHUNK_WIDTH); stripeOffset+=(PROGRESS_CHUNK_WIDTH*2))
            {
                GdkRectangle innerRect={x+1, y+stripeOffset+animShift, width-2, PROGRESS_CHUNK_WIDTH};

                /*constrainRect(&innerRect, area);*/
                if (innerRect.width > 0 && innerRect.height > 0) {
                    GdkRegion *innerRegion = gdk_region_rectangle(&innerRect);

                    gdk_region_xor(region, innerRegion);
                    gdk_region_destroy(innerRegion);
                }
            }
        setCairoClippingRegion(cr, region);
        gdk_region_destroy(region);
        break;
    }
    case STRIPE_DIAGONAL:
        cairo_new_path(cr);
        cairo_save(cr);
// #if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
//             if(area)
//                 cairo_rectangle(cr, area->x, area->y, area->width, area->height);
// #endif
        if(horiz)
            for(stripeOffset=0; stripeOffset<(width+height+2); stripeOffset+=(PROGRESS_CHUNK_WIDTH*2))
            {
                GdkPoint pts[4]={ {x+stripeOffset+animShift,                               y},
                                  {x+stripeOffset+animShift+PROGRESS_CHUNK_WIDTH,          y},
                                  {(x+stripeOffset+animShift+PROGRESS_CHUNK_WIDTH)-height, y+height-1},
                                  {(x+stripeOffset+animShift)-height,                      y+height-1}};

                plotPoints(cr, pts, 4);
            }
        else
            for(stripeOffset=0; stripeOffset<(height+width+2); stripeOffset+=(PROGRESS_CHUNK_WIDTH*2))
            {
                GdkPoint pts[4]={{x,         y+stripeOffset+animShift},
                                 {x+width-1, (y+stripeOffset+animShift)-width},
                                 {x+width-1, (y+stripeOffset+animShift+PROGRESS_CHUNK_WIDTH)-width},
                                 {x,         y+stripeOffset+animShift+PROGRESS_CHUNK_WIDTH}};

                plotPoints(cr, pts, 4);
            }

        cairo_clip(cr);
    }
}

void
drawProgress(cairo_t *cr, GtkStyle *style, GtkStateType state,
             GtkWidget *widget, GdkRectangle *area, int x, int y,
             int width, int height, gboolean rev, gboolean isEntryProg)
{
#if GTK_CHECK_VERSION(2, 90, 0)
    gboolean revProg = (widget && GTK_IS_PROGRESS_BAR(widget) &&
                        gtk_progress_bar_get_inverted(GTK_PROGRESS_BAR(widget)));
#else
    GtkProgressBarOrientation orientation = widget && GTK_IS_PROGRESS_BAR(widget)
        ? gtk_progress_bar_get_orientation(GTK_PROGRESS_BAR(widget))
        : GTK_PROGRESS_LEFT_TO_RIGHT;
    gboolean revProg = qtcOneOf(orientation, GTK_PROGRESS_RIGHT_TO_LEFT,
                                GTK_PROGRESS_BOTTOM_TO_TOP);
#endif
    const gboolean horiz = isHorizontalProgressbar(widget);
    int wid = isEntryProg ? WIDGET_ENTRY_PROGRESSBAR : WIDGET_PROGRESSBAR;
    int animShift = revProg ? 0 : -PROGRESS_CHUNK_WIDTH;
    int xo=x, yo=y, wo=width, ho=height;

    if (opts.fillProgress)
        x--, y--, width+=2, height+=2, xo=x, yo=y, wo=width, ho=height;

    if (opts.stripedProgress != STRIPE_NONE &&
        opts.animatedProgress && (isEntryProg || qtcIsProgressBar(widget))) {
#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
        if (isEntryProg || !GTK_PROGRESS(widget)->activity_mode)
#endif
            qtcAnimationAddProgressBar((void*)widget, isEntryProg);

        animShift+=(revProg ? -1 : 1)*
            (((int)(qtcAnimationElapsed(widget)*PROGRESS_CHUNK_WIDTH))%(PROGRESS_CHUNK_WIDTH*2));
    }

    gboolean grayItem = (GTK_STATE_INSENSITIVE == state &&
                         ECOLOR_BACKGROUND != opts.progressGrooveColor);
    GdkColor *itemCols = (grayItem ? qtcPalette.background :
                          qtcPalette.progress ? qtcPalette.progress :
                          qtcPalette.highlight);
    int new_state = GTK_STATE_PRELIGHT == state ? GTK_STATE_NORMAL : state;
    int fillVal = grayItem ? 4 : ORIGINAL_SHADE;

    x++, y++, width-=2, height-=2;

    if (opts.borderProgress && opts.round>ROUND_SLIGHT &&
        (horiz ? width : height) < 4) {
        clipPath(cr, x, y, width, height, wid, RADIUS_EXTERNAL, ROUNDED_ALL);
    }

    if ((horiz ? width : height) > 1)
        drawLightBevel(cr, style, new_state, area, x, y, width, height,
                       &itemCols[fillVal], itemCols, ROUNDED_ALL, wid,
                       BORDER_FLAT, (horiz ? 0 : DF_VERT), widget);
    if (opts.stripedProgress && width > 4 && height > 4) {
        if (STRIPE_FADE == opts.stripedProgress) {
            int posMod = opts.animatedProgress ? STRIPE_WIDTH - animShift : 0;
            int sizeMod = opts.animatedProgress ? (STRIPE_WIDTH * 2) : 0;
            addStripes(cr, x - (horiz ? posMod : 0), y - (horiz ? 0 : posMod),
                       width + (horiz ? sizeMod : 0),
                       height + (horiz ? 0 : sizeMod), horiz);
        } else {
            setProgressStripeClipping(cr, area, xo, yo, wo, ho,
                                      animShift, horiz);
            drawLightBevel(cr, style, new_state, NULL, x, y, width, height,
                           &itemCols[1], qtcPalette.highlight, ROUNDED_ALL,
                           wid, BORDER_FLAT,
                           (opts.fillProgress || !opts.borderProgress ? 0 :
                            DF_DO_BORDER) | (horiz ? 0 : DF_VERT), widget);
            cairo_restore(cr);
        }
    }
    if (opts.glowProgress && (horiz ? width : height)>3) {
        int offset = opts.borderProgress ? 1 : 0;
        cairo_pattern_t *pat =
            cairo_pattern_create_linear(x+offset, y+offset, horiz ?
                                        x + width - offset : x + offset,
                                        horiz ? y + offset :
                                        y + height - offset);
        gboolean inverted = FALSE;

        if (GLOW_MIDDLE != opts.glowProgress && widget &&
            GTK_IS_PROGRESS_BAR(widget)) {
            if (horiz) {
                if (revProg) {
                    inverted =! rev;
                } else {
                    inverted = rev;
                }
            } else {
                inverted = revProg;
            }
        }

        cairo_pattern_add_color_stop_rgba(
            pat, 0.0, 1.0, 1.0, 1.0,
            (inverted ? GLOW_END : GLOW_START) == opts.glowProgress ?
            GLOW_PROG_ALPHA : 0.0);

        if (GLOW_MIDDLE == opts.glowProgress)
            cairo_pattern_add_color_stop_rgba(pat, 0.5, 1.0, 1.0, 1.0,
                                              GLOW_PROG_ALPHA);

        cairo_pattern_add_color_stop_rgba(
            pat, CAIRO_GRAD_END, 1.0, 1.0, 1.0,
            (inverted ? GLOW_START : GLOW_END) == opts.glowProgress ?
            GLOW_PROG_ALPHA : 0.0);
        cairo_set_source(cr, pat);
        cairo_rectangle(cr, x + offset, y + offset, width - 2 * offset,
                        height - 2 * offset);
        cairo_fill(cr);
        cairo_pattern_destroy(pat);
        if (width > 2 && height > 2 && opts.borderProgress)
            drawBorder(cr, style, state, area, x, y, width, height, itemCols,
                       ROUNDED_ALL, BORDER_FLAT, wid, 0, PBAR_BORDER);

        if (!opts.borderProgress) {
            if (horiz) {
                drawHLine(cr, &itemCols[PBAR_BORDER], 1.0, x, y, width);
                drawHLine(cr, &itemCols[PBAR_BORDER], 1.0,
                          x, y + height - 1, width);
            } else {
                drawVLine(cr, &itemCols[PBAR_BORDER], 1.0, x, y, height);
                drawVLine(cr, &itemCols[PBAR_BORDER], 1.0,
                          x + width - 1, y, height);
            }
        }
    }
}

void drawProgressGroove(cairo_t *cr, GtkStyle *style, GtkStateType state, GdkWindow *window, GtkWidget *widget, GdkRectangle *area,
                        int x, int y, int width, int height, gboolean isList, gboolean horiz)
{
    gboolean doEtch=!isList && DO_EFFECT;
    GdkColor *col=&style->base[state];
    int      offset=opts.borderProgress ? 1 : 0;

    switch(opts.progressGrooveColor)
    {
    default:
    case ECOLOR_BASE:
        col=&style->base[state];
        break;
    case ECOLOR_BACKGROUND:
        col=&qtcPalette.background[ORIGINAL_SHADE];
        break;
    case ECOLOR_DARK:
        col=&qtcPalette.background[2];
    }

    if(!isList && (qtcIsFlatBgnd(opts.bgndAppearance) || !(widget && drawWindowBgnd(cr, style, area, window, widget, x, y, width, height)))
       && (!widget || !g_object_get_data(G_OBJECT (widget), "transparent-bg-hint")))
        drawAreaColor(cr, area, &qtcPalette.background[ORIGINAL_SHADE], x, y, width, height);

    if(doEtch && opts.borderProgress)
        x++, y++, width-=2, height-=2;

    /*clipPath(cr, x, y, width, height, WIDGET_PBAR_TROUGH, RADIUS_INTERNAL, ROUNDED_ALL);*/
    drawBevelGradient(cr, area, x+offset, y+offset, width-(2*offset), height-(2*offset), col,
                      horiz, FALSE, opts.progressGrooveAppearance, WIDGET_PBAR_TROUGH);
    /*cairo_restore(cr);*/

    if(doEtch && opts.borderProgress)
        drawEtch(cr, area, widget, x-1, y-1, width+2, height+2, FALSE, ROUNDED_ALL, WIDGET_PBAR_TROUGH);

    if(opts.borderProgress)
    {
        GtkWidget *parent=widget ? gtk_widget_get_parent(widget) : NULL;
        GtkStyle  *style=widget ? gtk_widget_get_style(parent ? parent : widget) : NULL;
        drawBorder(cr, style, state, area, x, y, width, height, NULL, ROUNDED_ALL,
                   qtcIsFlat(opts.progressGrooveAppearance) && ECOLOR_DARK!=opts.progressGrooveColor ? BORDER_SUNKEN : BORDER_FLAT,
                   WIDGET_PBAR_TROUGH, DF_BLEND);
    }
    else /* if(!opts.borderProgress) */
        if (horiz) {
            drawHLine(cr, &qtcPalette.background[QTC_STD_BORDER], 1.0,
                      x, y, width);
            drawHLine(cr, &qtcPalette.background[QTC_STD_BORDER], 1.0,
                      x, y + height - 1, width);
        } else {
            drawVLine(cr, &qtcPalette.background[QTC_STD_BORDER], 1.0,
                      x, y, height);
            drawVLine(cr, &qtcPalette.background[QTC_STD_BORDER], 1.0,
                      x + width - 1, y, height);
        }
}

#define SLIDER_TROUGH_SIZE 5

void
drawSliderGroove(cairo_t *cr, GtkStyle *style, GtkStateType state,
                 GdkWindow *window, GtkWidget *widget, const char *detail,
                 GdkRectangle *area, int x, int y, int width, int height,
                 gboolean horiz)
{
    QTC_UNUSED(window);
    GdkColor      *bgndcols=qtcPalette.background,
        *bgndcol=&bgndcols[2];
    GtkAdjustment *adjustment = gtk_range_get_adjustment(GTK_RANGE(widget));
    double        upper=gtk_adjustment_get_upper(adjustment),
        lower=gtk_adjustment_get_lower(adjustment),
        value=gtk_adjustment_get_value(adjustment);
    int           used_x=x, used_y=y, used_h=0, used_w=0,
        pos=(int)(((double)(horiz ? width : height) /
                   (upper - lower))  *
                  (value - lower));
    gboolean      inverted=gtk_range_get_inverted(GTK_RANGE(widget)),
        doEtch=DO_EFFECT,
        rev=reverseLayout(widget) || (widget && reverseLayout(gtk_widget_get_parent(widget)));
    int           troughSize=SLIDER_TROUGH_SIZE+(doEtch ? 2 : 0);
    GdkColor      *usedcols=opts.fillSlider && upper!=lower && state!=GTK_STATE_INSENSITIVE
        ? qtcPalette.slider
        ? qtcPalette.slider
        : qtcPalette.highlight
        : qtcPalette.background;
    EWidget       wid=WIDGET_SLIDER_TROUGH;

    if(horiz && rev)
        inverted=!inverted;

    if (horiz) {
        y += (height - troughSize) / 2;
        height = troughSize;
        used_y = y;
        used_h = height;
    } else {
        x += (width - troughSize) / 2;
        width = troughSize;
        used_x = x;
        used_w = width;
    }

    if(GTK_STATE_INSENSITIVE==state)
        bgndcol=&bgndcols[ORIGINAL_SHADE];
    else if (0==strcmp(detail, "trough-lower") && opts.fillSlider)
    {
        bgndcols=usedcols;
        bgndcol=&usedcols[ORIGINAL_SHADE];
        wid=WIDGET_FILLED_SLIDER_TROUGH;
    }
    drawLightBevel(cr, style, state, area, x, y, width, height,
                   bgndcol, bgndcols, opts.square&SQUARE_SLIDER ? ROUNDED_NONE : ROUNDED_ALL, wid,
                   BORDER_FLAT, DF_SUNKEN|DF_DO_BORDER|(horiz ? 0 : DF_VERT), NULL);

    if(opts.fillSlider && upper!=lower && state!=GTK_STATE_INSENSITIVE && 0==strcmp(detail, "trough"))
    {
        if(horiz)
        {
            pos+=(width>10 && pos<(width/2)) ? 3 : 0;

            if(inverted)   /* <rest><slider><used> */
                used_x+=width-pos;
            used_w=pos;
        }
        else
        {
            pos+=(height>10 && pos<(height/2)) ? 3 : 0;

            if(inverted)
                used_y+=height-pos;
            used_h=pos;
        }

        if(used_w>0 && used_h>0)
        {
            drawLightBevel(cr, style, state, area, used_x, used_y, used_w, used_h,
                           &usedcols[ORIGINAL_SHADE], usedcols,
                           opts.square&SQUARE_SLIDER ? ROUNDED_NONE : ROUNDED_ALL, WIDGET_FILLED_SLIDER_TROUGH,
                           BORDER_FLAT, DF_SUNKEN|DF_DO_BORDER|(horiz ? 0 : DF_VERT), NULL);
        }
    }
}

void
drawTriangularSlider(cairo_t *cr, GtkStyle *style, GtkStateType state,
                     const char *detail, GdkRectangle *area, int x, int y,
                     int width, int height)
{
    QTC_UNUSED(area);
    GdkColor newColors[TOTAL_SHADES + 1];
    GdkColor *btnColors = NULL;

    /* Fix Java swing sliders looking pressed */
    if (GTK_STATE_ACTIVE == state)
        state = GTK_STATE_PRELIGHT;

    if (useButtonColor(detail)) {
        if (GTK_STATE_INSENSITIVE == state) {
            btnColors = qtcPalette.background;
        } else if (QT_CUSTOM_COLOR_BUTTON(style)) {
            qtcShadeColors(&(style->bg[state]), newColors);
            btnColors = newColors;
        } else {
            GtkWidget *widget = NULL; /* Keep SET_BTN_COLS happy */
            SET_BTN_COLS(FALSE, TRUE, FALSE, state);
        }
    }

    gboolean coloredMouseOver = (GTK_STATE_PRELIGHT == state &&
                                 opts.coloredMouseOver &&
                                 !opts.colorSliderMouseOver);
    gboolean horiz = height > width || DETAIL("hscale");
    int bgnd = getFillReal(state, FALSE, SHADE_DARKEN == opts.shadeSliders);
    int xo = horiz ? 8 : 0;
    int yo = horiz ? 0 : 8;
    int size = 15;
    int light = APPEARANCE_DULL_GLASS == opts.sliderAppearance ? 1 : 0;
    GdkColor *colors = btnColors;
    GdkColor *borderCols = ((GTK_STATE_PRELIGHT == state &&
                             qtcOneOf(opts.coloredMouseOver, MO_GLOW,
                                      MO_COLORED)) ?
                            qtcPalette.mouseover : btnColors);
    GtkArrowType direction = horiz ? GTK_ARROW_DOWN : GTK_ARROW_RIGHT;
    gboolean drawLight=MO_PLASTIK!=opts.coloredMouseOver || !coloredMouseOver;
    int borderVal=qtcPalette.mouseover==borderCols ? SLIDER_MO_BORDER_VAL : BORDER_VAL(GTK_STATE_INSENSITIVE==state);
    if (MO_GLOW == opts.coloredMouseOver && DO_EFFECT) {
        x++;
        y++;
        xo++;
        yo++;
    }
    cairo_new_path(cr);
    cairo_save(cr);
    switch (direction) {
    case GTK_ARROW_UP:
    default:
    case GTK_ARROW_DOWN: {
        y += 2;
        GdkPoint pts[] = {{x, y}, {x, y + 2}, {x + 2, y}, {x + 8, y},
                          {x + 10, y + 2}, {x + 10, y + 9}, {x + 5, y + 14},
                          {x, y + 9}};
        plotPoints(cr, pts, 8);
    }
        break;
    case GTK_ARROW_RIGHT:
    case GTK_ARROW_LEFT: {
        x+=2;
        GdkPoint pts[] = {{x, y}, {x + 2, y}, {x, y + 2}, {x, y + 8},
                          {x + 2, y + 10}, {x + 9, y + 10}, {x + 14, y + 5},
                          {x + 9, y}};
        plotPoints(cr, pts, 8);
    }
    }
    cairo_clip(cr);
    if (qtcIsFlat(opts.sliderAppearance)) {
        drawAreaColor(cr, NULL, &colors[bgnd], x+1, y+1, width-2, height-2);
        if (MO_PLASTIK == opts.coloredMouseOver && coloredMouseOver) {
            int col = SLIDER_MO_SHADE;
            int len = SLIDER_MO_LEN;
            if (horiz) {
                drawAreaColor(cr, NULL, &qtcPalette.mouseover[col], x + 1,
                              y + 1, len, size-2);
                drawAreaColor(cr, NULL, &qtcPalette.mouseover[col],
                              x + width - (1 + len), y + 1, len, size - 2);
            } else {
                drawAreaColor(cr, NULL, &qtcPalette.mouseover[col], x + 1,
                              y + 1, size - 2, len);
                drawAreaColor(cr, NULL, &qtcPalette.mouseover[col], x + 1,
                              y + height - (1 + len), size - 2, len);
            }
        }
    } else {
        drawBevelGradient(cr, NULL, x, y, horiz ? width - 1 : size,
                          horiz ? size : height-1, &colors[bgnd], horiz, FALSE,
                          MODIFY_AGUA(opts.sliderAppearance), WIDGET_OTHER);
        if (MO_PLASTIK == opts.coloredMouseOver && coloredMouseOver) {
            int col = SLIDER_MO_SHADE;
            int len = SLIDER_MO_LEN;
            if (horiz) {
                drawBevelGradient(cr, NULL, x + 1, y + 1, len, size - 2,
                                  &qtcPalette.mouseover[col], horiz, FALSE,
                                  MODIFY_AGUA(opts.sliderAppearance),
                                  WIDGET_OTHER);
                drawBevelGradient(cr, NULL, x + width - (1 + len), y + 1, len,
                                  size - 2, &qtcPalette.mouseover[col], horiz,
                                  FALSE, MODIFY_AGUA(opts.sliderAppearance),
                                  WIDGET_OTHER);
            } else {
                drawBevelGradient(cr, NULL, x + 1, y + 1, size - 2, len,
                                  &qtcPalette.mouseover[col], horiz, FALSE,
                                  MODIFY_AGUA(opts.sliderAppearance),
                                  WIDGET_OTHER);
                drawBevelGradient(cr, NULL, x + 1, y + height - (1 + len),
                                  size - 2, len, &qtcPalette.mouseover[col],
                                  horiz, FALSE,
                                  MODIFY_AGUA(opts.sliderAppearance),
                                  WIDGET_OTHER);
            }
        }
    }
    cairo_restore(cr);
    double xd = x + 0.5;
    double yd = y + 0.5;
    double radius = 2.5;
    double xdg = xd - 1;
    double ydg = yd - 1;
    double radiusg = radius + 1;
    gboolean glowMo = (MO_GLOW == opts.coloredMouseOver &&
                       coloredMouseOver && DO_EFFECT);
    cairo_new_path(cr);
    if (glowMo) {
        qtcCairoSetColor(cr, &borderCols[GLOW_MO], GLOW_ALPHA(FALSE));
    } else {
        qtcCairoSetColor(cr, &borderCols[borderVal]);
    }
    switch (direction) {
    case GTK_ARROW_UP:
    default:
    case GTK_ARROW_DOWN:
        if (glowMo) {
            cairo_move_to(cr, xdg + radiusg, ydg);
            cairo_arc(cr, xdg + 12 - radiusg, ydg + radiusg, radiusg,
                      M_PI * 1.5, M_PI * 2);
            cairo_line_to(cr, xdg + 12, ydg + 10.5);
            cairo_line_to(cr, xdg + 6, ydg + 16.5);
            cairo_line_to(cr, xdg, ydg + 10.5);
            cairo_arc(cr, xdg + radiusg, ydg + radiusg, radiusg,
                      M_PI, M_PI * 1.5);
            cairo_stroke(cr);
            qtcCairoSetColor(cr, &borderCols[borderVal]);
        }
        cairo_move_to(cr, xd + radius, yd);
        cairo_arc(cr, xd + 10 - radius, yd + radius, radius,
                  M_PI * 1.5, M_PI * 2);
        cairo_line_to(cr, xd + 10, yd + 9);
        cairo_line_to(cr, xd + 5, yd + 14);
        cairo_line_to(cr, xd, yd + 9);
        cairo_arc(cr, xd + radius, yd + radius, radius, M_PI, M_PI * 1.5);
        cairo_stroke(cr);
        if (drawLight) {
            drawVLine(cr, &colors[light], 1.0, xd + 1, yd + 2, 7);
            drawHLine(cr, &colors[light], 1.0, xd + 2, yd + 1, 6);
        }
        break;
    case GTK_ARROW_RIGHT:
    case GTK_ARROW_LEFT:
        if (glowMo) {
            cairo_move_to(cr, xdg, ydg + 12-radiusg);
            cairo_arc(cr, xdg + radiusg, ydg + radiusg,
                      radiusg, M_PI, M_PI * 1.5);
            cairo_line_to(cr, xdg + 10.5, ydg);
            cairo_line_to(cr, xdg + 16.5, ydg + 6);
            cairo_line_to(cr, xdg + 10.5, ydg + 12);
            cairo_arc(cr, xdg + radiusg, ydg + 12 - radiusg,
                      radiusg, M_PI * 0.5, M_PI);
            cairo_stroke(cr);
            qtcCairoSetColor(cr, &borderCols[borderVal]);
        }
        cairo_move_to(cr, xd, yd + 10 - radius);
        cairo_arc(cr, xd + radius, yd + radius, radius, M_PI, M_PI * 1.5);
        cairo_line_to(cr, xd + 9, yd);
        cairo_line_to(cr, xd + 14, yd + 5);
        cairo_line_to(cr, xd + 9, yd + 10);
        cairo_arc(cr, xd + radius, yd + 10 - radius, radius, M_PI * 0.5, M_PI);
        cairo_stroke(cr);
        if (drawLight) {
            drawHLine(cr, &colors[light], 1.0, xd + 2, yd + 1, 7);
            drawVLine(cr, &colors[light], 1.0, xd + 1, yd + 2, 6);
        }
    }
}
#define SBAR_BTN_SIZE opts.sliderWidth

void
drawScrollbarGroove(cairo_t *cr, GtkStyle *style, GtkStateType state,
                    GdkWindow *window, GtkWidget *widget, const char *detail,
                    GdkRectangle *area, int x, int y, int width, int height,
                    gboolean horiz)
{
    QTC_UNUSED(window);
    QTC_UNUSED(detail);
    int sbarRound = ROUNDED_ALL;
    int xo=x, yo=y, wo=width, ho=height;
    gboolean drawBg=opts.flatSbarButtons/* && !qtcIsFlat(opts.sbarBgndAppearance) && SCROLLBAR_NONE!=opts.scrollbarType*/,
        thinner=opts.thinSbarGroove && (SCROLLBAR_NONE==opts.scrollbarType || opts.flatSbarButtons);

    if (opts.flatSbarButtons) {
#if GTK_CHECK_VERSION(2, 90, 0)
        gboolean lower=detail && strstr(detail, "-lower");
        sbarRound = (lower ? horiz ? ROUNDED_LEFT : ROUNDED_TOP : horiz ?
                     ROUNDED_RIGHT : ROUNDED_BOTTOM);

        switch (opts.scrollbarType) {
        case SCROLLBAR_KDE:
            if (lower) {
                if (horiz) {
                    x += SBAR_BTN_SIZE;
                } else {
                    y += SBAR_BTN_SIZE;
                }
            } else {
                if (horiz) {
                    width -= SBAR_BTN_SIZE * 2;
                } else {
                    height -= SBAR_BTN_SIZE * 2;
                }
            }
            break;
        case SCROLLBAR_WINDOWS:
            if (lower) {
                if (horiz) {
                    x += SBAR_BTN_SIZE;
                } else {
                    y += SBAR_BTN_SIZE;
                }
            } else {
                if (horiz) {
                    width -= SBAR_BTN_SIZE;
                } else {
                    height -= SBAR_BTN_SIZE;
                }
            }
            break;
        case SCROLLBAR_NEXT:
            if (lower) {
                if (horiz) {
                    x += SBAR_BTN_SIZE * 2;
                } else {
                    y += SBAR_BTN_SIZE * 2;
                }
            }
            break;
        case SCROLLBAR_PLATINUM:
            if (!lower) {
                if (horiz) {
                    width -= SBAR_BTN_SIZE * 2;
                } else {
                    height -= SBAR_BTN_SIZE * 2;
                }
            }
            break;
        }
#else
        switch (opts.scrollbarType) {
        case SCROLLBAR_KDE:
            if (horiz) {
                x += SBAR_BTN_SIZE;
                width -= SBAR_BTN_SIZE * 3;
            } else {
                y += SBAR_BTN_SIZE;
                height -= SBAR_BTN_SIZE * 3;
            }
            break;
        case SCROLLBAR_WINDOWS:
            if (horiz) {
                x += SBAR_BTN_SIZE;
                width -= SBAR_BTN_SIZE * 2;
            } else {
                y += SBAR_BTN_SIZE;
                height -= SBAR_BTN_SIZE * 2;
            }
            break;
        case SCROLLBAR_NEXT:
            if (horiz) {
                x+=SBAR_BTN_SIZE * 2;
                width -= SBAR_BTN_SIZE * 2;
            } else {
                y += SBAR_BTN_SIZE * 2;
                height -= SBAR_BTN_SIZE * 2;
            }
            break;
        case SCROLLBAR_PLATINUM:
            if (horiz) {
                width -= SBAR_BTN_SIZE * 2;
            } else {
                height -= SBAR_BTN_SIZE * 2;
            }
            break;
        default:
            break;
        }
#endif
    } else {
        switch (opts.scrollbarType) {
        default:
            break;
        case SCROLLBAR_NEXT:
            sbarRound = horiz ? ROUNDED_LEFT : ROUNDED_TOP;
            break;
        case SCROLLBAR_PLATINUM:
            sbarRound = horiz ? ROUNDED_RIGHT : ROUNDED_BOTTOM;
            break;
#ifdef SIMPLE_SCROLLBARS
        case SCROLLBAR_NONE:
            sbarRound = ROUNDED_NONE;
            break;
#endif
        }
    }

    if (opts.square & SQUARE_SB_SLIDER)
        sbarRound = ROUNDED_NONE;

    if(drawBg) {
        GtkWidget *parent=NULL;
        if(opts.gtkScrollViews && qtcIsFlat(opts.sbarBgndAppearance) && 0!=opts.tabBgnd && widget &&
           (parent=gtk_widget_get_parent(widget)) && GTK_IS_SCROLLED_WINDOW(parent) &&
           (parent=gtk_widget_get_parent(parent)) && GTK_IS_NOTEBOOK(parent))
            drawAreaModColor(cr, area, &qtcPalette.background[ORIGINAL_SHADE], TO_FACTOR(opts.tabBgnd), xo, yo, wo, ho);
        else if(!qtcIsFlat(opts.sbarBgndAppearance) || !opts.gtkScrollViews)
            drawBevelGradient(cr, area, xo, yo, wo, ho, &qtcPalette.background[ORIGINAL_SHADE],
                              horiz, FALSE, opts.sbarBgndAppearance, WIDGET_SB_BGND);
#if !GTK_CHECK_VERSION(2, 90, 0)
        /* This was the old (pre 1.7.1) else if... but it messes up Gtk3 scrollbars wheb have custom background. And dont think its needed */
        /* But re-added in 1.7.2 for Mozilla! */
        /*  else if(!qtcIsCustomBgnd(&opts) || !(opts.gtkScrollViews && qtcIsFlat(opts.sbarBgndAppearance) &&
            widget && drawWindowBgnd(cr, style, area, widget, xo, yo, wo, ho)))
        */ else if(isMozilla()) /* 1.7.3 added 'else' so as to not duplicate above! */
            drawBevelGradient(cr, area, xo, yo, wo, ho, &qtcPalette.background[ORIGINAL_SHADE],
                              horiz, FALSE, opts.sbarBgndAppearance, WIDGET_SB_BGND);
#endif
    }

    if(isMozilla())
    {
        if(!drawBg)
        {
            GdkColor *parent_col=getParentBgCol(widget),
                *bgnd_col=parent_col ? parent_col : &qtcPalette.background[ORIGINAL_SHADE];

            setCairoClipping(cr, area);

            drawAreaColor(cr, area, &qtcPalette.background[ORIGINAL_SHADE], x, y, width, height);
            if (horiz) {
                if (qtcOneOf(sbarRound, ROUNDED_LEFT, ROUNDED_ALL))
                    drawVLine(cr, bgnd_col, 1.0, x, y, height);
                if (qtcOneOf(sbarRound, ROUNDED_RIGHT, ROUNDED_ALL))
                    drawVLine(cr, bgnd_col, 1.0, x + width - 1, y, height);
            } else {
                if (qtcOneOf(sbarRound, ROUNDED_TOP, ROUNDED_ALL))
                    drawHLine(cr, bgnd_col, 1.0, x, y, width);
                if (qtcOneOf(sbarRound, ROUNDED_BOTTOM, ROUNDED_ALL))
                    drawHLine(cr, bgnd_col, 1.0, x, y + height - 1, width);
            }
            cairo_restore(cr);
        }
    }
    else if(GTK_APP_OPEN_OFFICE==qtSettings.app && opts.flatSbarButtons && isFixedWidget(widget))
    {
        if (horiz)
            width--;
        else
            height--;
    }

    if(thinner && !drawBg)
        drawBgnd(cr, &qtcPalette.background[ORIGINAL_SHADE], widget, area, x, y, width, height);

    drawLightBevel(cr, style, state, area,
                   thinner && !horiz ? x+THIN_SBAR_MOD : x,
                   thinner && horiz  ? y+THIN_SBAR_MOD : y,
                   thinner && !horiz ? width-(THIN_SBAR_MOD*2) : width,
                   thinner && horiz  ? height-(THIN_SBAR_MOD*2) : height,
                   &qtcPalette.background[2], qtcPalette.background, sbarRound,
                   thinner ? WIDGET_SLIDER_TROUGH : WIDGET_TROUGH,
                   BORDER_FLAT, DF_SUNKEN|DF_DO_BORDER|
                   (horiz ? 0 : DF_VERT), widget);
}

void
drawSelectionGradient(cairo_t *cr, GtkStyle *style, GtkStateType state,
                      GdkRectangle *area, int x, int y, int width, int height,
                      int round, gboolean isLvSelection,
                      double alpha, GdkColor *col, gboolean horiz)
{
    QTC_UNUSED(style);
    QTC_UNUSED(state);
    if((!isLvSelection || !(opts.square&SQUARE_LISTVIEW_SELECTION)) && ROUND_NONE!=opts.round)
        clipPathRadius(cr, x, y, width, height, qtcGetRadius(&opts, width, height, WIDGET_SELECTION, RADIUS_SELECTION), round);
    drawBevelGradient(cr, area, x, y, width, height, col, horiz, FALSE,
                      opts.selectionAppearance, WIDGET_SELECTION, alpha);
    if((!isLvSelection || !(opts.square&SQUARE_LISTVIEW_SELECTION)) && ROUND_NONE!=opts.round)
        cairo_restore(cr);
}

void drawSelection(cairo_t *cr, GtkStyle *style, GtkStateType state, GdkRectangle *area, GtkWidget *widget,
                   int x, int y, int width, int height, int round, gboolean isLvSelection, double alphaMod, int factor)
{
    gboolean hasFocus=gtk_widget_has_focus(widget);
    double   alpha=alphaMod*(GTK_STATE_PRELIGHT==state ? 0.20 : 1.0)*(hasFocus || !qtSettings.inactiveChangeSelectionColor ? 1.0 : INACTIVE_SEL_ALPHA);
    GdkColor col=style->base[hasFocus ? GTK_STATE_SELECTED : GTK_STATE_ACTIVE];

    if(factor!=0)
        col=shadeColor(&col, TO_FACTOR(factor));

    drawSelectionGradient(cr, style, state, area, x, y, width, height, round, isLvSelection, alpha, &col, TRUE);

    if(opts.borderSelection && (!isLvSelection || !(opts.square&SQUARE_LISTVIEW_SELECTION)))
    {
        double xd=x+0.5,
            yd=y+0.5,
            alpha=GTK_STATE_PRELIGHT==state || alphaMod<1.0 ? 0.20 : 1.0;
        int    xo=x, widtho=width;

        if(isLvSelection && !(opts.square&SQUARE_LISTVIEW_SELECTION) && ROUNDED_ALL!=round)
        {
            if(!(round&ROUNDED_LEFT))
            {
                x-=1;
                xd-=1;
                width+=1;
            }
            if(!(round&ROUNDED_RIGHT))
                width+=1;
        }

        cairo_save(cr);
        cairo_new_path(cr);
        cairo_rectangle(cr, xo, y, widtho, height);
        cairo_clip(cr);
        qtcCairoSetColor(cr, &col, alpha);
        createPath(cr, xd, yd, width - 1, height - 1,
                   qtcGetRadius(&opts, widtho, height, WIDGET_OTHER,
                                RADIUS_SELECTION), round);
        cairo_stroke(cr);
        cairo_restore(cr);
    }
}

void
createRoundedMask(cairo_t *cr, GtkWidget *widget, int x, int y, int width,
                  int height, double radius, gboolean isToolTip)
{
    QTC_UNUSED(cr);
    if (widget) {
        int size = ((width & 0xFFFF) << 16) + (height & 0xFFFF);
        QTC_DEF_WIDGET_PROPS(props, widget);
        int old = qtcWidgetProps(props)->widgetMask;

        if (size != old) {
#if GTK_CHECK_VERSION(2, 90, 0)
            GdkRegion *mask = windowMask(0, 0, width, height,
                                         opts.round > ROUND_SLIGHT);

            gtk_widget_shape_combine_region(widget, NULL);
            gtk_widget_shape_combine_region(widget, mask);
            gdk_region_destroy(mask);
#else
            GdkBitmap *mask = gdk_pixmap_new(NULL, width, height, 1);
            cairo_t *crMask = gdk_cairo_create((GdkDrawable*)mask);

            cairo_rectangle(crMask, 0, 0, width, height);
            cairo_set_source_rgba(crMask, 1, 1, 1, 0);
            cairo_set_operator(crMask, CAIRO_OPERATOR_SOURCE);
            cairo_paint(crMask);
            cairo_new_path(crMask);
            createPath(crMask, 0, 0, width, height, radius, ROUNDED_ALL);
            cairo_set_source_rgba(crMask, 0, 0, 0, 1);
            cairo_fill(crMask);
            if (isToolTip) {
                gtk_widget_shape_combine_mask(widget, mask, x, y);
            } else {
                gdk_window_shape_combine_mask(
                    gtk_widget_get_parent_window(widget), mask, 0, 0);
            }
            cairo_destroy(crMask);
            gdk_pixmap_unref(mask);
#endif
            qtcWidgetProps(props)->widgetMask = size;
            /* Setting the window type to 'popup menu' seems to
               re-eanble kwin shadows! */
            if (isToolTip && gtk_widget_get_window(widget)) {
                gdk_window_set_type_hint(gtk_widget_get_window(widget),
                                         GDK_WINDOW_TYPE_HINT_POPUP_MENU);
            }
        }
    }
}

void
clearRoundedMask(GtkWidget *widget, gboolean isToolTip)
{
    if (widget) {
        QTC_DEF_WIDGET_PROPS(props, widget);
        if (qtcWidgetProps(props)->widgetMask) {
#if GTK_CHECK_VERSION(2, 90, 0)
            gtk_widget_shape_combine_region(widget, NULL);
#else
            if(isToolTip)
                gtk_widget_shape_combine_mask(widget, NULL, 0, 0);
            else
                gdk_window_shape_combine_mask(gtk_widget_get_parent_window(widget), NULL, 0, 0);
#endif
            qtcWidgetProps(props)->widgetMask = 0;
        }
    }
}

void drawTreeViewLines(cairo_t *cr, GdkColor *col, int x, int y, int h, int depth, int levelIndent, int expanderSize,
                       GtkTreeView *treeView, GtkTreePath *path, GtkTreeViewColumn *column)
{
    QTC_UNUSED(column);
    int        cellIndent=levelIndent + expanderSize + 4,
        xStart=x + (cellIndent/2),
        i,
        isLastMask=0;
    gboolean   haveChildren=treeViewCellHasChildren(treeView, path),
        useBitMask=depth<33;
    GByteArray *isLast=depth && !useBitMask ? g_byte_array_sized_new(depth) : NULL;

    if (useBitMask || isLast) {
        GtkTreePath  *p = path ? gtk_tree_path_copy(path) : NULL;
        int          index=depth-1;

        while (p && gtk_tree_path_get_depth(p) > 0 && index>=0) {
            GtkTreePath *next=treeViewPathParent(treeView, p);
            uint8_t last=treeViewCellIsLast(treeView, p) ? 1 : 0;
            if (useBitMask) {
                if (last) {
                    isLastMask |= 1 << index;
                }
            } else {
                isLast = g_byte_array_prepend(isLast, &last, 1);
            }
            gtk_tree_path_free(p);
            p = next;
            index--;
        }
        qtcCairoSetColor(cr, col);
        for(i = 0;i < depth;++i) {
            gboolean isLastCell=(useBitMask ? isLastMask&(1<<i) : isLast->data[i]) ? TRUE : FALSE,
                last=i == depth -1;
            double   xCenter = xStart;
            if(last)
            {
                double yCenter = (int)(y+h/2);

                if(haveChildren)
                {
                    // first vertical line
                    cairo_move_to(cr, xCenter + 0.5, y);
                    cairo_line_to(cr, xCenter + 0.5, yCenter - (LV_SIZE-1)); // (int)(expanderSize/3));

                    // second vertical line
                    if(!isLastCell)
                    {
                        cairo_move_to(cr, xCenter + 0.5, y+h);
                        cairo_line_to(cr, xCenter + 0.5, yCenter + (LV_SIZE+1)); // (int)(expanderSize/3));
                    }

                    // horizontal line
                    cairo_move_to(cr, xCenter + (int)(expanderSize/3)+1, yCenter + 0.5);
                    cairo_line_to(cr, xCenter + expanderSize*2/3 -1, yCenter + 0.5);
                }
                else
                {
                    cairo_move_to(cr, xCenter + 0.5, y);
                    if(isLastCell)
                        cairo_line_to(cr, xCenter + 0.5, yCenter);
                    else
                        cairo_line_to(cr, xCenter + 0.5, y+h);

                    // horizontal line
                    cairo_move_to(cr, xCenter, yCenter + 0.5);
                    cairo_line_to(cr, xCenter + expanderSize*2/3 -1, yCenter + 0.5);
                }
            }
            else if(!isLastCell)
            {
                // vertical line
                cairo_move_to(cr, xCenter + 0.5, y);
                cairo_line_to(cr, xCenter + 0.5, y + h);
            }

            cairo_stroke(cr);
            xStart += cellIndent;
        }

        if(isLast)
            g_byte_array_free(isLast, FALSE);
    }
}

void
drawPolygon(GdkWindow *window, GtkStyle *style, GdkColor *col,
            GdkRectangle *area, GdkPoint *points, int npoints, gboolean fill)
{
    g_return_if_fail(GTK_IS_STYLE(style));
    g_return_if_fail(GDK_IS_DRAWABLE(window));
    cairo_t *cr = gdk_cairo_create(window);
    cairo_set_line_width(cr, 1);
    setCairoClipping(cr, area);
    qtcCairoSetColor(cr, col);
    plotPoints(cr, points, npoints);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    if (fill) {
        cairo_fill(cr);
    }
    cairo_destroy(cr);
}

void
drawArrow(GdkWindow *window, GtkStyle *style, GdkColor *col,
          GdkRectangle *area, GtkArrowType arrow_type, int x, int y,
          gboolean small, gboolean fill)
{
    if (small) {
        switch (arrow_type) {
        case GTK_ARROW_UP: {
            GdkPoint a[] = {{x + 2, y}, {x, y - 2}, {x - 2, y}, {x - 2, y + 1},
                            {x, y - 1}, {x + 2, y + 1}};
            drawPolygon(window, style, col, area, a,
                        opts.vArrows ? 6 : 3, fill);
            break;
        }
        case GTK_ARROW_DOWN: {
            GdkPoint a[] = {{x + 2, y}, {x, y + 2}, {x - 2, y}, {x - 2, y - 1},
                            {x, y + 1}, {x + 2, y - 1}};
            drawPolygon(window, style, col, area, a,
                        opts.vArrows ? 6 : 3, fill);
            break;
        }
        case GTK_ARROW_RIGHT: {
            GdkPoint a[] = {{x, y - 2}, {x + 2, y}, {x, y + 2}, {x - 1, y + 2},
                            {x + 1, y}, {x - 1, y - 2}};
            drawPolygon(window, style, col, area, a,
                        opts.vArrows ? 6 : 3, fill);
            break;
        }
        case GTK_ARROW_LEFT: {
            GdkPoint a[] = {{x, y - 2}, {x - 2, y}, {x, y + 2}, {x + 1, y + 2},
                            {x - 1, y}, {x + 1, y - 2}};
            drawPolygon(window, style, col, area, a,
                        opts.vArrows ? 6 : 3, fill);
            break;
        }
        default:
            return;
        }
    } else {
        /* Large arrows... */
        switch (arrow_type) {
        case GTK_ARROW_UP: {
            GdkPoint a[] = {{x + 3, y + 1}, {x, y - 2}, {x - 3, y + 1},
                            {x - 3, y + 2}, {x - 2, y + 2}, {x, y},
                            {x + 2, y + 2}, {x + 3, y + 2}};
            drawPolygon(window, style, col, area, a,
                        opts.vArrows ? 8 : 3, fill);
            break;
        }
        case GTK_ARROW_DOWN: {
            GdkPoint a[] = {{x + 3, y - 1}, {x, y + 2}, {x - 3, y - 1},
                            {x - 3, y - 2}, {x - 2, y - 2}, {x, y},
                            {x + 2, y - 2}, {x + 3,y - 2}};
            drawPolygon(window, style, col, area, a,
                        opts.vArrows ? 8 : 3, fill);
            break;
        }
        case GTK_ARROW_RIGHT: {
            GdkPoint a[] = {{x - 1, y + 3}, {x + 2, y}, {x - 1, y - 3},
                            {x - 2, y - 3}, {x - 2, y - 2}, {x, y},
                            {x - 2, y + 2}, {x - 2, y + 3}};
            drawPolygon(window, style, col, area, a,
                        opts.vArrows ? 8 : 3, fill);
            break;
        }
        case GTK_ARROW_LEFT: {
            GdkPoint a[] = {{x + 1, y - 3}, {x - 2, y}, {x + 1, y + 3},
                            {x + 2, y + 3}, {x + 2, y + 2}, {x, y},
                            {x + 2, y - 2}, {x + 2, y - 3}};
            drawPolygon(window, style, col, area, a,
                        opts.vArrows ? 8 : 3, fill);
            break;
        }
        default:
            return;
        }
    }
}

static void setGapClip(cairo_t *cr, GdkRectangle *area, GtkPositionType gapSide, int gapX, int gapWidth, int x, int y, int width,
                       int height, gboolean isTab)
{
    if(gapWidth>0)
    {
        GdkRectangle gapRect;
        int          adjust=isTab ? (gapX>1 ? 1 : 2) : 0;

        switch(gapSide)
        {
        case GTK_POS_TOP:
            gapRect.x=x+gapX+adjust, gapRect.y=y, gapRect.width=gapWidth-(2*adjust), gapRect.height=2;
            if(GTK_APP_JAVA==qtSettings.app && isTab)
                gapRect.width-=3;
            break;
        case GTK_POS_BOTTOM:
            gapRect.x=x+gapX+adjust, gapRect.y=y+height-2, gapRect.width=gapWidth-(2*adjust), gapRect.height=2;
            break;
        case GTK_POS_LEFT:
            gapRect.x=x, gapRect.y=y+gapX+adjust, gapRect.width=2, gapRect.height=gapWidth-(2*adjust);
            break;
        case GTK_POS_RIGHT:
            gapRect.x=x+width-2, gapRect.y=y+gapX+adjust, gapRect.width=2, gapRect.height=gapWidth-(2*adjust);
            break;
        }

        GdkRectangle r={x, y, width, height};
        GdkRegion *region = gdk_region_rectangle(area ? area : &r);
        GdkRegion *inner = gdk_region_rectangle(&gapRect);

        gdk_region_xor(region, inner);
        setCairoClippingRegion(cr, region);
        gdk_region_destroy(inner);
        gdk_region_destroy(region);
    }
}

static void
ge_cairo_transform_for_layout(cairo_t *cr, PangoLayout *layout, int x, int y)
{
    const PangoMatrix *matrix;

    matrix = pango_context_get_matrix(pango_layout_get_context(layout));
    if (matrix) {
        cairo_matrix_t cairo_matrix;
        PangoRectangle rect;

        cairo_matrix_init(&cairo_matrix, matrix->xx, matrix->yx,
                          matrix->xy, matrix->yy, matrix->x0, matrix->y0);
        pango_layout_get_extents(layout, NULL, &rect);
        pango_matrix_transform_rectangle(matrix, &rect);
        pango_extents_to_pixels(&rect, NULL);

        cairo_matrix.x0 += x - rect.x;
        cairo_matrix.y0 += y - rect.y;

        cairo_set_matrix(cr, &cairo_matrix);
    } else {
        cairo_translate(cr, x, y);
    }
}

void
drawLayout(GtkStyle *style, GdkWindow *window, GtkStateType state,
           gboolean use_text, GdkRectangle *area, int x, int y,
           PangoLayout *layout)
{
    g_return_if_fail(GTK_IS_STYLE(style));
    g_return_if_fail(GDK_IS_DRAWABLE(window));
    cairo_t *cr = gdk_cairo_create(window);
    setCairoClipping(cr, area);
    cairo_set_line_width(cr, 1.0);
    gdk_cairo_set_source_color(cr, use_text ||
                               state == GTK_STATE_INSENSITIVE ?
                               &style->text[state] : &style->fg[state]);
    ge_cairo_transform_for_layout(cr, layout, x, y);
    pango_cairo_show_layout(cr, layout);
    cairo_destroy(cr);
}

void fillTab(cairo_t *cr, GtkStyle *style, GtkWidget *widget, GdkRectangle *area, GtkStateType state,
             GdkColor *col, int x, int y, int width, int height, gboolean horiz, EWidget tab, gboolean grad)
{
    gboolean selected=GTK_STATE_NORMAL==state,
        flatBgnd=!qtcIsCustomBgnd(&opts) || 0!=opts.tabBgnd;

//     if(selected && !flatBgnd)
//         drawWindowBgnd(cr, style, area, widget, x, y, width, height);

    GdkColor *c=col,
        b;
    double   alpha=1.0;

    if (selected && 0 != opts.tabBgnd) {
        qtcShade(col, &b, TO_FACTOR(opts.tabBgnd), opts.shading);
        c = &b;
    }

    if(!selected && (100!=opts.bgndOpacity || 100!=opts.dlgOpacity))
    {
        GtkWidget *top=widget ? gtk_widget_get_toplevel(widget) : 0L;
        gboolean  isDialog=top && GTK_IS_DIALOG(top);

        // Note: opacity is divided by 150 to make dark inactive tabs more translucent
        if(isDialog && 100!=opts.dlgOpacity)
            alpha=opts.dlgOpacity/150.0;
        else if(!isDialog && 100!=opts.bgndOpacity)
            alpha=opts.bgndOpacity/150.0;
    }

    if(selected && APPEARANCE_INVERTED==opts.appearance)
    {
        if(flatBgnd)
            drawAreaColor(cr, area, &style->bg[GTK_STATE_NORMAL], x, y, width, height, alpha);
    }
    else if(grad)
        drawBevelGradient(cr, area, x, y, width, height, c, horiz, selected,
                          selected ? SEL_TAB_APP : NORM_TAB_APP, tab, alpha);
    else if(!selected || flatBgnd)
        drawAreaColor(cr, area, c, x, y, width, height, alpha);
}

void colorTab(cairo_t *cr, int x, int y, int width, int height, int round, EWidget tab, gboolean horiz)
{
    cairo_pattern_t *pt=cairo_pattern_create_linear(x, y, horiz ? x : x+width-1, horiz ? y+height-1 : y);

    clipPath(cr, x, y, width, height, tab, RADIUS_EXTERNAL, round);
    qtcCairoPatternAddColorStop(pt, 0, &qtcPalette.highlight[ORIGINAL_SHADE],
                                tab == WIDGET_TAB_TOP ?
                                TO_ALPHA(opts.colorSelTab) : 0.0);
    qtcCairoPatternAddColorStop(pt, CAIRO_GRAD_END,
                                &qtcPalette.highlight[ORIGINAL_SHADE],
                                tab == WIDGET_TAB_TOP ? 0.0 :
                                TO_ALPHA(opts.colorSelTab));
    cairo_set_source(cr, pt);
    cairo_rectangle(cr, x, y, width, height);
    cairo_fill(cr);
    cairo_pattern_destroy(pt);
    cairo_restore(cr);
}

void drawToolTip(cairo_t *cr, GtkWidget *widget, GdkRectangle *area, int x, int y, int width, int height)
{
    GdkColor *col=&qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP];

    gboolean nonGtk = isFakeGtk();
    gboolean rounded = !nonGtk && widget && !(opts.square & SQUARE_TOOLTIPS);
    gboolean useAlpha = (!nonGtk && qtSettings.useAlpha &&
                         isRgbaWidget(widget) && compositingActive(widget));

    if (!nonGtk && !useAlpha && GTK_IS_WINDOW(widget))
        gtk_window_set_opacity(GTK_WINDOW(widget), 0.875);

    if (rounded) {
        if (useAlpha) {
            cairo_rectangle(cr, x, y, width, height);
            cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
            cairo_set_source_rgba(cr, 0, 0, 0, 1);
            cairo_fill(cr);
            clearRoundedMask(widget, TRUE);
        } else {
            createRoundedMask(cr, widget, x, y, width, height,
                              MENU_AND_TOOLTIP_RADIUS, TRUE);
        }
        clipPathRadius(cr, x, y, width, height, MENU_AND_TOOLTIP_RADIUS,
                       ROUNDED_ALL);
    }
    if(useAlpha)
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

    drawBevelGradient(cr, area, x, y, width, height, col, true, FALSE,
                      opts.tooltipAppearance, WIDGET_TOOLTIP,
                      useAlpha ? 0.875 : 1.0);
    if (!rounded && qtcIsFlat(opts.tooltipAppearance)) {
        cairo_new_path(cr);
        /*if(qtcIsFlat(opts.tooltipAppearance))*/
        qtcCairoSetColor(cr,
                         &qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP_TEXT]);
        /*else
          cairo_set_source_rgba(cr, 0, 0, 0, 0.25);*/
        cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
        cairo_stroke(cr);
    }

    if (useAlpha) {
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    }
}

void drawSplitter(cairo_t *cr, GtkStateType state, GtkStyle *style, GdkRectangle *area, int x, int y, int width, int height)
{
    GdkColor *cols=opts.coloredMouseOver && GTK_STATE_PRELIGHT==state
        ? qtcPalette.mouseover
        : qtcPalette.background;

    if(GTK_STATE_PRELIGHT==state && opts.splitterHighlight)
    {
        GdkColor col=shadeColor(&style->bg[state], TO_FACTOR(opts.splitterHighlight));
        drawSelectionGradient(cr, style, state, area, x, y, width, height, ROUNDED_ALL, FALSE, 1.0, &col, width>height);
    }

    switch(opts.splitters)
    {
    case LINE_1DOT:
        drawDot(cr, x, y, width, height, cols);
        break;
    case LINE_NONE:
        break;
    case LINE_DOTS:
    default:
        drawDots(cr, x, y, width, height, height>width, NUM_SPLITTER_DASHES, 1, cols, area, 0, 5);
        break;
    case LINE_FLAT:
    case LINE_SUNKEN:
    case LINE_DASHES:
        drawLines(cr, x, y, width, height, height>width, NUM_SPLITTER_DASHES, 2, cols, area, 3, opts.splitters);
    }
}

void drawSidebarButton(cairo_t *cr, GtkStateType state, GtkStyle *style, GdkRectangle *area, int x, int y, int width, int height)
{
    if (qtcOneOf(state, GTK_STATE_PRELIGHT, GTK_STATE_ACTIVE)) {
        gboolean horiz=width>height;
        GdkColor *cols=GTK_STATE_ACTIVE==state ? qtcPalette.sidebar : qtcPalette.background;
        drawLightBevel(cr, style, state, area, x, y, width, height, &cols[getFill(state, false)], cols, ROUNDED_NONE, WIDGET_MENU_ITEM,
                       BORDER_FLAT, (horiz ? 0 : DF_VERT)|(GTK_STATE_ACTIVE==state ? DF_SUNKEN : 0), NULL);

        if(opts.coloredMouseOver && GTK_STATE_PRELIGHT==state)
        {
            GdkColor *col=&qtcPalette.mouseover[1];

            if(horiz || MO_PLASTIK!=opts.coloredMouseOver)
            {
                cairo_new_path(cr);
                qtcCairoSetColor(cr, col);
                cairo_move_to(cr, x, y+0.5);
                cairo_line_to(cr, x+width-1, y+0.5);
                cairo_move_to(cr, x+1, y+1.5);
                cairo_line_to(cr, x+width-2, y+1.5);
                cairo_stroke(cr);
            }

            if(!horiz || MO_PLASTIK!=opts.coloredMouseOver)
            {
                cairo_new_path(cr);
                qtcCairoSetColor(cr, col);
                cairo_move_to(cr, x+0.5, y);
                cairo_line_to(cr, x+0.5, y+height-1);
                cairo_move_to(cr, x+1.5, y+1);
                cairo_line_to(cr, x+1.5, y+height-2);
                cairo_stroke(cr);
                if(MO_PLASTIK!=opts.coloredMouseOver)
                    col=&qtcPalette.mouseover[2];
            }

            if(horiz || MO_PLASTIK!=opts.coloredMouseOver)
            {
                cairo_new_path(cr);
                qtcCairoSetColor(cr, col);
                cairo_move_to(cr, x, y+height-1.5);
                cairo_line_to(cr, x+width-1, y+height-1.5);
                cairo_move_to(cr, x+1, y+height-2.5);
                cairo_line_to(cr, x+width-2, y+height-2.5);
                cairo_stroke(cr);
            }

            if(!horiz || MO_PLASTIK!=opts.coloredMouseOver)
            {
                cairo_new_path(cr);
                qtcCairoSetColor(cr, col);
                cairo_move_to(cr, x+width-1.5, y);
                cairo_line_to(cr, x+width-1.5, y+height-1);
                cairo_move_to(cr, x+width-2.5, y+1);
                cairo_line_to(cr, x+width-2.5, y+height-2);
                cairo_stroke(cr);
            }
        }
    }
}

void drawMenuItem(cairo_t *cr, GtkStateType state, GtkStyle *style, GtkWidget *widget, GdkRectangle *area, int x, int y, int width, int height)
{
    GtkMenuBar *mb=isMenubar(widget, 0);
#if GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
    gboolean   active_mb=isFakeGtk() || gdk_pointer_is_grabbed();
#else
    gboolean   active_mb=isFakeGtk() || (mb ? GTK_MENU_SHELL(mb)->active : FALSE);

    // The handling of 'mouse pressed' in the menubar event handler doesn't seem to set the
    // menu as active, therefore the active_mb fails. However the check below works...
    if(mb && !active_mb && widget)
        active_mb=widget==GTK_MENU_SHELL(mb)->active_menu_item;
#endif

    /* The following 'if' is just a hack for a menubar item problem with pidgin. Sometime, a 12pix width
       empty menubar item is drawn on the right - and doesnt disappear! */
    if(!mb || width>12)
    {
        gboolean grayItem=(!opts.colorMenubarMouseOver && mb && !active_mb && GTK_APP_OPEN_OFFICE!=qtSettings.app) ||
            !opts.useHighlightForMenu;
        GdkColor *itemCols=grayItem ? qtcPalette.background : qtcPalette.highlight;
        /* GdkColor *bgnd = (qtcPalette.menubar && mb && !isMozilla() && */
        /*                   GTK_APP_JAVA != qtSettings.app ? */
        /*                   &qtcPalette.menubar[ORIGINAL_SHADE] : NULL); */
        int      round=mb
            ? active_mb && opts.roundMbTopOnly
            ? ROUNDED_TOP
            : ROUNDED_ALL
            : ROUNDED_ALL,
            new_state=GTK_STATE_PRELIGHT==state ? GTK_STATE_NORMAL : state;
        gboolean stdColors=!mb || (SHADE_BLEND_SELECTED!=opts.shadeMenubars && SHADE_SELECTED!=opts.shadeMenubars);
        int      fillVal=grayItem ? 4 : ORIGINAL_SHADE,
            borderVal=opts.borderMenuitems ? 0 : fillVal;

        if(grayItem && mb && !active_mb && !opts.colorMenubarMouseOver &&
           (opts.borderMenuitems || !qtcIsFlat(opts.menuitemAppearance)))
            fillVal=ORIGINAL_SHADE;

        if(mb && !opts.roundMbTopOnly && !(opts.square&SQUARE_POPUP_MENUS))
            x++, y++, width-=2, height-=2;

        if (grayItem && !mb &&
            (opts.lighterPopupMenuBgnd || opts.shadePopupMenu)) {
            itemCols = qtcPalette.menu;
        }
        if(!mb && APPEARANCE_FADE==opts.menuitemAppearance)
        {
            gboolean        reverse=FALSE; /* TODO !!! */
            cairo_pattern_t *pt=NULL;
            double          fadePercent=0.0;

            if (ROUNDED) {
                x++;
                y++;
                width -= 2;
                height -= 2;
                clipPathRadius(cr, x, y, width, height, 4,
                               reverse ? ROUNDED_RIGHT : ROUNDED_LEFT);
            }

            fadePercent = ((double)MENUITEM_FADE_SIZE) / (double)width;
            pt = cairo_pattern_create_linear(x, y, x + width - 1, y);

            qtcCairoPatternAddColorStop(pt, 0, &itemCols[fillVal],
                                        reverse ? 0.0 : 1.0);
            qtcCairoPatternAddColorStop(pt, (reverse ? fadePercent :
                                             1.0 - fadePercent),
                                        &itemCols[fillVal]);
            qtcCairoPatternAddColorStop(pt, CAIRO_GRAD_END, &itemCols[fillVal],
                                        reverse ? 1.0 : 0.0);
            cairo_set_source(cr, pt);
            cairo_rectangle(cr, x, y, width, height);
            cairo_fill(cr);
            cairo_pattern_destroy(pt);
            if(ROUNDED)
                cairo_restore(cr);
        } else if (!opts.borderMenuitems && !mb) {
            /*For now dont round combos - getting weird effects with shadow/clipping :-( */
            /*...but these work ok if we have an rgba colormap, so in that case we dont care if its a combo...*/
            gboolean isCombo=!(opts.square&SQUARE_POPUP_MENUS) && widget && isComboMenu(gtk_widget_get_parent(widget)) &&
                !(qtSettings.useAlpha && compositingActive(widget) && isRgbaWidget(widget)),
                roundedMenu=(!widget || !isCombo) && !(opts.square&SQUARE_POPUP_MENUS);

            if(roundedMenu)
                clipPathRadius(cr, x, y, width, height, MENU_AND_TOOLTIP_RADIUS-1.0, round);
            drawBevelGradient(cr, area, x, y, width, height, &itemCols[fillVal], TRUE, FALSE, opts.menuitemAppearance, WIDGET_MENU_ITEM);
            if(roundedMenu)
                cairo_restore(cr);
        }
        else if(stdColors && opts.borderMenuitems)
        {
            drawLightBevel(cr, style, new_state, area, x, y, width, height, &itemCols[fillVal],
                           itemCols, round, WIDGET_MENU_ITEM, BORDER_FLAT, DF_DRAW_INSIDE|(stdColors ? DF_DO_BORDER : 0), widget);
        }
        else
        {
            if(width>2 && height>2)
                drawBevelGradient(cr, area, x+1, y+1, width-2, height-2, &itemCols[fillVal],  TRUE, FALSE, opts.menuitemAppearance, WIDGET_MENU_ITEM);

            drawBorder(cr, style, state, area, x, y, width, height, itemCols,
                       round, BORDER_FLAT, WIDGET_MENU_ITEM, 0, borderVal);
        }
    }
}

void
drawMenu(cairo_t *cr, GtkStateType state, GtkStyle *style, GtkWidget *widget,
         GdkRectangle *area, int x, int y, int width, int height)
{
    QTC_UNUSED(state);
    double radius = 0.0;
    double alpha = 1.0;
    gboolean nonGtk = isFakeGtk();
    gboolean roundedMenu = /*!comboMenu &&*/
        !(opts.square & SQUARE_POPUP_MENUS) && !nonGtk;
    gboolean compsActive = compositingActive(widget);
    gboolean isAlphaWidget = compsActive && isRgbaWidget(widget);
    gboolean useAlpha = isAlphaWidget && opts.menuBgndOpacity < 100;
    gboolean useAlphaForCorners =
        !nonGtk && qtSettings.useAlpha && isAlphaWidget;
    gboolean comboMenu =
        useAlphaForCorners || !compsActive ? FALSE : isComboMenu(widget);
    /* Cant round combos, unless using rgba - getting weird effects with
       shadow/clipping :-(. If 'useAlphaForCorners', then dont care if its a
       combo menu - as it can still be rounded */

    if (roundedMenu && !comboMenu) {
        radius=MENU_AND_TOOLTIP_RADIUS;
        if(useAlphaForCorners)
        {
            cairo_rectangle(cr, x, y, width, height);
            cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
            cairo_set_source_rgba(cr, 0, 0, 0, 1);
            cairo_fill(cr);
            cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
            clearRoundedMask(widget, FALSE);
        }
        else
            createRoundedMask(cr, widget, x, y, width, height, radius-0.25, FALSE);
        clipPathRadius(cr, x, y, width, height, radius, ROUNDED_ALL);
    }

    if(useAlpha)
    {
        if(widget && /*!comboMenu && */100!=opts.menuBgndOpacity)
            enableBlurBehind(widget, TRUE);

        alpha=opts.menuBgndOpacity/100.0;
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    }

    if(/*!comboMenu && */!qtcIsFlatBgnd(opts.menuBgndAppearance))
    {
        if(APPEARANCE_STRIPED==opts.menuBgndAppearance)
            drawStripedBgnd(cr, style, area, x, y, width, height, &qtcPalette.menu[ORIGINAL_SHADE], FALSE, alpha);
        else if(APPEARANCE_FILE==opts.menuBgndAppearance)
            drawBgndImage(cr, style, area, x, y, width, height, &qtcPalette.menu[ORIGINAL_SHADE], FALSE, alpha);
        else
            drawBevelGradient(cr, area, x, y, width, height,
                              &qtcPalette.menu[ORIGINAL_SHADE],
                              opts.menuBgndGrad == GT_HORIZ, FALSE,
                              opts.menuBgndAppearance, WIDGET_OTHER, alpha);
    } else if (opts.shadePopupMenu || opts.lighterPopupMenuBgnd || useAlpha) {
        drawAreaColor(cr, area, &qtcPalette.menu[ORIGINAL_SHADE], x, y,
                      width, height, alpha);
    }
    if(/*!comboMenu && */IMG_NONE!=opts.menuBgndImage.type)
        drawBgndRings(cr, x, y, width, height, FALSE);

    if(opts.menuStripe && !comboMenu)
    {
        gboolean mozOo=isFakeGtk();
        int      stripeWidth=mozOo ? 22 : 21;

        // To determine stripe size, we iterate over all menuitems of this menu. If we find a GtkImageMenuItem then
        // we can a width of 20. However, we need to check that at least one enttry actually has an image! So, if
        // the first GtkImageMenuItem has an image then we're ok, otherwise we give it a blank pixmap.
        if(!mozOo && widget)
        {
            GtkMenuShell *menuShell=GTK_MENU_SHELL(widget);
            GList        *children=gtk_container_get_children(GTK_CONTAINER(menuShell)),
                *child=children;

            for(; child; child = child->next)
            {
                if(GTK_IS_IMAGE_MENU_ITEM(child->data))
                {
                    GtkImageMenuItem *item=GTK_IMAGE_MENU_ITEM(child->data);
                    stripeWidth=21;

                    if(0L==gtk_image_menu_item_get_image(item) ||
                       (GTK_IS_IMAGE(gtk_image_menu_item_get_image(item)) &&
                        GTK_IMAGE_EMPTY==gtk_image_get_storage_type(GTK_IMAGE(gtk_image_menu_item_get_image(item)))))
                    {
                        // Give it a blank icon - so that menuStripe looks ok, plus this matches KDE style!
                        if(0L==gtk_image_menu_item_get_image(item))
                            gtk_image_menu_item_set_image(item, gtk_image_new_from_pixbuf(getPixbuf(qtcPalette.check_radio,
                                                                                                    PIX_BLANK, 1.0)));
                        else
                            gtk_image_set_from_pixbuf(GTK_IMAGE(gtk_image_menu_item_get_image(item)),
                                                      getPixbuf(qtcPalette.check_radio, PIX_BLANK, 1.0));
                        break;
                    }
                    else // TODO: Check image size!
                        break;
                }
            }

            if(children)
                g_list_free(children);
        }

        drawBevelGradient(cr, area, x + 1, y + 1, stripeWidth + 1, height - 2,
                          &opts.customMenuStripeColor, FALSE, FALSE,
                          opts.menuStripeAppearance, WIDGET_OTHER, alpha);
    }

    if(opts.popupBorder)
    {
        EGradientBorder border=qtcGetGradient(opts.menuBgndAppearance, &opts)->border;

        if(roundedMenu && !comboMenu)
            cairo_restore(cr);

        if(useAlphaForCorners) /* cairo_restore reverts the cairo operator change */
            cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

        cairo_new_path(cr);
        qtcCairoSetColor(cr, &qtcPalette.menu[QTC_STD_BORDER]);
        /* For now dont round combos - getting weird effects with
         * shadow/clipping :-( */
        if(roundedMenu && !comboMenu)
            createPath(cr, x+0.5, y+0.5, width-1, height-1, radius-1, ROUNDED_ALL);
        else
            cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
        cairo_stroke(cr);
        if (qtcUseBorder(border) && APPEARANCE_FLAT!=opts.menuBgndAppearance) {
            if (roundedMenu) {
                if (border != GB_3D) {
                    cairo_new_path(cr);
                    qtcCairoSetColor(cr, qtcPalette.menu);
                    createTLPath(cr, x + 1.5, y + 1.5, width - 3, height - 3,
                                 radius - 2, ROUNDED_ALL);
                    cairo_stroke(cr);
                }
                cairo_new_path(cr);
                qtcCairoSetColor(cr, &qtcPalette.menu[border == GB_LIGHT ? 0 :
                                                      FRAME_DARK_SHADOW]);
                createBRPath(cr, x + 1.5, y + 1.5, width - 3, height - 3,
                             radius - 2, ROUNDED_ALL);
                cairo_stroke(cr);
            } else {
                if (border != GB_3D) {
                    drawHLine(cr, qtcPalette.menu, 1.0,
                              x + 1, y + 1, width - 2);
                    drawVLine(cr, qtcPalette.menu, 1.0,
                              x + 1, y + 1, height - 2);
                }
                drawHLine(cr, &qtcPalette.menu[border == GB_LIGHT ? 0 :
                                               FRAME_DARK_SHADOW], 1.0,
                          x + 1, y + height - 2, width - 2);
                drawVLine(cr, &qtcPalette.menu[border == GB_LIGHT ? 0 :
                                               FRAME_DARK_SHADOW], 1.0,
                          x+width-2, y+1, height-2);
            }
        }
    }

    if(useAlpha || useAlphaForCorners)
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
}

void drawBoxGap(cairo_t *cr, GtkStyle *style, GtkShadowType shadow, GtkStateType state,
                GtkWidget *widget, GdkRectangle *area, int x, int y, int width, int height, GtkPositionType gapSide,
                int gapX, int gapWidth, EBorder borderProfile, gboolean isTab)
{
    g_return_if_fail(GTK_IS_STYLE(style));

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %d %d %d ", __FUNCTION__, shadow, state, x, y, width, height, gapX,
                                           gapWidth, isTab),
                                        debugDisplayWidget(widget, 10);

    // *Very* hacky fix for tabs in thunderbird main window!!!
    if(isTab && isMozilla() && 250==gapWidth && (290==width || 270==width) && 6==height)
        return;

    if(isTab && 0!=opts.tabBgnd)
    {
        clipPath(cr, x-1, y-1, width+2, height+2, WIDGET_TAB_FRAME, RADIUS_EXTERNAL, ROUNDED_ALL);
        drawAreaMod(cr, style, state, area, TO_FACTOR(opts.tabBgnd), x, y, width, height);
        cairo_restore(cr);
    }

    if(TAB_MO_GLOW==opts.tabMouseOver && gapWidth>4 && isMozillaWidget(widget))
        gapWidth-=2;

    if(GTK_SHADOW_NONE!=shadow)
    {
        int       round=((!isTab && opts.square&SQUARE_FRAME) || (isTab && opts.square&SQUARE_TAB_FRAME)) ? ROUNDED_NONE : ROUNDED_ALL;
        GtkWidget *parent=widget ? gtk_widget_get_parent(widget) : NULL;

        if(!(opts.square&SQUARE_TAB_FRAME) && gapX<=0)
            switch(gapSide)
            {
            case GTK_POS_TOP:
                round=CORNER_TR|CORNER_BL|CORNER_BR;
                break;
            case GTK_POS_BOTTOM:
                round=CORNER_BR|CORNER_TL|CORNER_TR;
                break;
            case GTK_POS_LEFT:
                round=CORNER_TR|CORNER_BL|CORNER_BR;
                break;
            case GTK_POS_RIGHT:
                round=CORNER_TL|CORNER_BL|CORNER_BR;
                break;
            }

        setGapClip(cr, area, gapSide, gapX, gapWidth, x, y, width, height, isTab);
        drawBorder(cr, gtk_widget_get_style(parent ? parent : widget), state, area, x, y, width, height, NULL, round,
                   borderProfile, isTab ? WIDGET_TAB_FRAME : WIDGET_FRAME, (isTab ? 0 : DF_BLEND));
        if(gapWidth>0)
            cairo_restore(cr);
    }
}

void
drawBoxGapFixes(cairo_t *cr, GtkWidget *widget,  int x, int y,
                int width, int height, GtkPositionType gapSide, int gapX,
                int gapWidth)
{
    GdkColor *col1 = &qtcPalette.background[0],
        *col2 = &qtcPalette.background[opts.borderTab ? 0 : (APPEARANCE_FLAT==opts.appearance ? ORIGINAL_SHADE : FRAME_DARK_SHADOW)],
        *outer = &qtcPalette.background[QTC_STD_BORDER];
    gboolean rev = reverseLayout(widget),
        thin=opts.thin&THIN_FRAMES;
    int      rightPos=(width -(gapX + gapWidth));

    switch (gapSide) {
    case GTK_POS_TOP:
        if (gapX > 0) {
            if (!thin) {
                drawHLine(cr, col1, 1.0, x+gapX-1, y+1, 3);
                drawHLine(cr, col1, 1.0, x+gapX-1, y, 3);
            }
            drawHLine(cr, outer, 1.0, x + gapX - 1, y, 2);
        } else if (!thin) {
            drawVLine(cr, col1, 1.0, x + 1, y, 2);
        }
        if (rightPos >= 0) {
            if (!thin) {
                drawHLine(cr, col1, 1.0, x + gapX + gapWidth - 2, y + 1, 3);
                drawVLine(cr, col2, 1.0, x + gapX + gapWidth - 2,
                          y, rightPos ? 1 : 0);
            }
            drawHLine(cr, outer, 1.0, x + gapX + gapWidth - 1, y, 2);
        }
        if (!(opts.square & SQUARE_TAB_FRAME) && opts.round > ROUND_SLIGHT) {
            if (gapX>0 && TAB_MO_GLOW==opts.tabMouseOver) {
                drawVLine(cr, outer, 1.0, rev ? x + width - 2 : x + 1, y, 2);
            } else {
                drawVLine(cr, outer, 1.0, rev ? x + width - 1 : x, y, 3);
                if (gapX > 0 && !thin) {
                    drawHLine(cr, &qtcPalette.background[2], 1.0, x + 1, y, 1);
                }
            }
        }
        break;
    case GTK_POS_BOTTOM:
        if (gapX > 0) {
            if (!thin) {
                drawHLine(cr, col1, 1.0, x + gapX - 1, y + height - 1, 2);
                drawHLine(cr, col2, 1.0, x + gapX - 1, y + height - 2, 2);
            }
            drawHLine(cr, outer, 1.0, x + gapX - 1, y + height - 1, 2);
        } else if (!thin) {
            drawVLine(cr, col1, 1.0, x + 1, y + height - 1, 2);
        }

        if (rightPos >= 0) {
            if (!thin) {
                drawHLine(cr, col2, 1.0, x + gapX + gapWidth - 2,
                          y + height - 2, 3);
                drawVLine(cr, col2, 1.0, x + gapX + gapWidth - 2,
                          y + height - 1, rightPos ? 1 : 0);
            }
            drawHLine(cr, outer, 1.0, x + gapX + gapWidth - 1,
                      y + height - 1, 2);
        }
        if(!(opts.square&SQUARE_TAB_FRAME) && opts.round>ROUND_SLIGHT) {
            if (gapX > 0 && TAB_MO_GLOW == opts.tabMouseOver) {
                drawVLine(cr, outer, 1.0, rev ? x + width - 2 : x + 1,
                          y + height - 2, 2);
            } else {
                drawVLine(cr, outer, 1.0, rev ? x + width - 1 : x,
                          y + height - 3, 3);
            }
        }
        break;
    case GTK_POS_LEFT:
        if (gapX > 0) {
            if (!thin) {
                drawVLine(cr, col1, 1.0, x + 1, y + gapX - 1, 3);
                drawVLine(cr, col1, 1.0, x, y + gapX - 1, 3);
            }
            drawVLine(cr, outer, 1.0, x, y + gapX - 1, 2);
        } else if (!thin) {
            drawHLine(cr, col1, 1.0, x, y + 1, 2);
        }

        if ((height-(gapX + gapWidth)) > 0) {
            if (!thin) {
                drawVLine(cr, col1, 1.0, x + 1, y + gapX + gapWidth - 2, 3);
                drawVLine(cr, col2, 1.0, x, y + gapX + gapWidth - 2, 1);
            }
            drawVLine(cr, outer, 1.0, x, y + gapX + gapWidth - 1, 2);
        }
        if(!(opts.square&SQUARE_TAB_FRAME) && opts.round>ROUND_SLIGHT) {
            if (gapX > 0 && TAB_MO_GLOW == opts.tabMouseOver) {
                drawHLine(cr, outer, 1.0, x, y + 1, 2);
            } else {
                drawHLine(cr, outer, 1.0, x, y, 3);
                if (gapX > 0 && !thin) {
                    drawHLine(cr, &qtcPalette.background[2], 1.0, x, y + 1, 1);
                }
            }
        }
        break;
    case GTK_POS_RIGHT:
        if (gapX > 0) {
            if (!thin)
                drawVLine(cr, col2, 1.0, x + width - 2, y + gapX - 1, 2);
            drawVLine(cr, outer, 1.0, x + width - 1, y + gapX - 1, 2);
        } else if (!thin) {
            drawHLine(cr, col1, 1.0, x + width - 2, y + 1, 3);
        }

        if ((height-(gapX + gapWidth)) > 0) {
            if (!thin) {
                drawHLine(cr, col2, 1.0, x + width - 2,
                          y + gapX + gapWidth - 2, 3);
                drawVLine(cr, col2, 1.0, x + width - 2,
                          y + gapX + gapWidth - 1, 2);
            }
            drawVLine(cr, outer, 1.0, x + width - 1, y + gapX + gapWidth - 1, 2);
        }
        if (!(opts.square & SQUARE_TAB_FRAME) && opts.round > ROUND_SLIGHT) {
            if (gapX > 0 && TAB_MO_GLOW == opts.tabMouseOver) {
                drawHLine(cr, outer, 1.0, x + width - 2, y + 1, 2);
            } else {
                drawHLine(cr, outer, 1.0, x + width - 3, y, 3);
            }
        }
        break;
    }
}

void drawShadowGap(cairo_t *cr, GtkStyle *style, GtkShadowType shadow, GtkStateType state,
                   GtkWidget *widget, GdkRectangle *area, int x, int y, int width, int height, GtkPositionType gapSide,
                   int gapX, int gapWidth)
{
    gboolean drawFrame=TRUE,
        isGroupBox=IS_GROUP_BOX(widget);

    if(isGroupBox)
    {
        if(gapX<5)
            gapX+=5, gapWidth+=2;

        switch(opts.groupBox)
        {
        case FRAME_NONE:
            drawFrame=FALSE;
            return;
        case FRAME_LINE:
        case FRAME_SHADED:
        case FRAME_FADED:
            if(opts.gbLabel&(GB_LBL_INSIDE|GB_LBL_OUTSIDE) && widget && GTK_IS_FRAME(widget))
            {
                GtkFrame       *frame=GTK_FRAME(widget);
                GtkRequisition child_requisition;
                int           height_extra;
                GtkStyle       *style=frame ? gtk_widget_get_style(GTK_WIDGET(frame)) : NULL;
                GtkWidget      *label=frame ? gtk_frame_get_label_widget(frame) : NULL;

                if(style && label)
                {
                    gtk_widget_get_child_requisition(label, &child_requisition);
                    height_extra = (qtcMax(0, child_requisition.height - style->ythickness)
                                    - qtcFrameGetLabelYAlign(frame) * child_requisition.height) + 2;

                    if(opts.gbLabel&GB_LBL_INSIDE)
                        y-=height_extra, height+=height_extra, gapWidth=0;
                    else if(opts.gbLabel&GB_LBL_OUTSIDE)
                        y+=height_extra, height-=height_extra, gapWidth=0;
                }
            }
            if(FRAME_LINE==opts.groupBox)
            {
                GdkRectangle gap={x, y, gapWidth, 1};
                drawFadedLine(cr, x, y, width, 1, &qtcPalette.background[QTC_STD_BORDER], area, gapWidth>0 ? &gap : NULL, FALSE, TRUE, TRUE);
                drawFrame=FALSE;
            }
            else if(GTK_SHADOW_NONE!=shadow)
            {
                int             round=opts.square&SQUARE_FRAME ? ROUNDED_NONE : ROUNDED_ALL;
                double          col=opts.gbFactor<0 ? 0.0 : 1.0,
                    radius=ROUNDED_ALL==round ? qtcGetRadius(&opts, width, height, WIDGET_FRAME, RADIUS_EXTERNAL) : 0.0;
                cairo_pattern_t *pt=NULL;
                if(0!=opts.gbFactor)
                {
                    clipPathRadius(cr, x+0.5, y+0.5, width-1, height-1, radius, round);
                    cairo_rectangle(cr, x, y, width, height);
                    if(FRAME_SHADED==opts.groupBox)
                        cairo_set_source_rgba(cr, col, col, col, TO_ALPHA(opts.gbFactor));
                    else
                    {
                        pt=cairo_pattern_create_linear(x, y, x, y+height-1);
                        cairo_pattern_add_color_stop_rgba(pt, 0, col, col, col, TO_ALPHA(opts.gbFactor));
                        cairo_pattern_add_color_stop_rgba(pt, CAIRO_GRAD_END, col, col, col, 0);
                        cairo_set_source(cr, pt);
                    }
                    cairo_fill(cr);
                    cairo_restore(cr);
                    if(pt)
                        cairo_pattern_destroy(pt);
                }
                if(FRAME_FADED==opts.groupBox)
                {
                    pt = cairo_pattern_create_linear(x, y, x, y + height - 1);
                    qtcCairoPatternAddColorStop(
                        pt, 0, &qtcPalette.background[QTC_STD_BORDER]);
                    qtcCairoPatternAddColorStop(
                        pt, CAIRO_GRAD_END,
                        &qtcPalette.background[QTC_STD_BORDER], 0);
                    setGapClip(cr, area, gapSide, gapX, gapWidth, x, y, width, height, FALSE);
                    cairo_set_source(cr, pt);
                    createPath(cr, x+0.5, y+0.5, width-1, height-1, radius, round);
                    cairo_stroke(cr);
                    cairo_pattern_destroy(pt);
                    drawFrame=false;
                }
            }
            break;
        default:
            break;
        }
    }
    if(drawFrame)
    {
        drawBoxGap(cr, style, shadow, state, widget, area, x, y, width, height, gapSide, gapX, gapWidth,
                   isGroupBox && FRAME_SHADED==opts.groupBox && GTK_SHADOW_NONE!=shadow
                   ? /*opts.gbFactor<0
                       ?*/ BORDER_SUNKEN
                   /*: BORDER_RAISED*/
                   : shadowToBorder(shadow),
                   FALSE);
    }
}

void
drawCheckBox(cairo_t *cr, GtkStateType state, GtkShadowType shadow,
             GtkStyle *style, GtkWidget *widget, const char *detail,
             GdkRectangle *area, int x, int y, int width, int height)
{
    if (state == GTK_STATE_PRELIGHT &&
        qtcOneOf(qtSettings.app, GTK_APP_MOZILLA, GTK_APP_JAVA)) {
        state = GTK_STATE_NORMAL;
    }
    gboolean mnu = DETAIL("check");
    gboolean list = !mnu && isList(widget);
    gboolean on = shadow == GTK_SHADOW_IN;
    gboolean tri = shadow == GTK_SHADOW_ETCHED_IN;
    gboolean doEtch = DO_EFFECT;
    GdkColor newColors[TOTAL_SHADES + 1];
    GdkColor *btnColors;
    int ind_state = ((list || (!mnu && state == GTK_STATE_INSENSITIVE)) ?
                     state : GTK_STATE_NORMAL);
    int checkSpace = doEtch ? opts.crSize + 2 : opts.crSize;

    if (opts.crColor && state != GTK_STATE_INSENSITIVE && (on || tri)) {
        btnColors = qtcPalette.selectedcr;
    } else if (!mnu && !list && QT_CUSTOM_COLOR_BUTTON(style)) {
        qtcShadeColors(&(style->bg[state]), newColors);
        btnColors = newColors;
    } else {
        btnColors = qtcPalette.button[state == GTK_STATE_INSENSITIVE ?
                                      PAL_DISABLED : PAL_ACTIVE];
    }
    x += (width - checkSpace) / 2;
    y += (height - checkSpace) / 2;
    if (qtSettings.debug == DEBUG_ALL) {
        printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %d %s  ",
               __FUNCTION__, state, shadow, x, y, width, height, mnu,
               detail ? detail : "NULL");
        debugDisplayWidget(widget, 10);
    }
    if ((mnu && state == GTK_STATE_PRELIGHT) ||
        (list && state == GTK_STATE_ACTIVE))
        state = GTK_STATE_NORMAL;
    if (mnu && isMozilla()) {
        x -= 2;
    }
    if (!mnu || qtSettings.qt4) {
        if (opts.crButton) {
            drawLightBevel(cr, style, state, area, x, y, checkSpace,
                           checkSpace, &btnColors[getFill(state, false)],
                           btnColors, ROUNDED_ALL, WIDGET_CHECKBOX, BORDER_FLAT,
                           DF_DO_BORDER |
                           (state == GTK_STATE_ACTIVE ? DF_SUNKEN : 0),
                           list ? NULL : widget);
            if (doEtch) {
                x++;
                y++;
            }
        } else {
            gboolean coloredMouseOver =
                state == GTK_STATE_PRELIGHT && opts.coloredMouseOver;
            gboolean glow = (doEtch && GTK_STATE_PRELIGHT == state &&
                             MO_GLOW == opts.coloredMouseOver);
            gboolean lightBorder = DRAW_LIGHT_BORDER(FALSE, WIDGET_TROUGH,
                                                     APPEARANCE_INVERTED);
            /* gboolean draw3dFull = */
            /*     (!lightBorder && */
            /*      DRAW_3D_FULL_BORDER(FALSE, APPEARANCE_INVERTED)); */
            /* gboolean draw3d = */
            /*     draw3dFull || */
            /*     (!lightBorder && */
            /*      DRAW_3D_BORDER(FALSE, APPEARANCE_INVERTED)); */
            GdkColor *colors =
                coloredMouseOver ? qtcPalette.mouseover : btnColors;
            GdkColor *bgndCol = (qtcOneOf(state, GTK_STATE_INSENSITIVE,
                                          GTK_STATE_ACTIVE) ?
                                 &style->bg[GTK_STATE_NORMAL] :
                                 !mnu && state == GTK_STATE_PRELIGHT &&
                                 !coloredMouseOver && !opts.crHighlight ?
                                 &btnColors[CR_MO_FILL] :
                                 &style->base[GTK_STATE_NORMAL]);
            if (doEtch) {
                x++;
                y++;
            }

            drawBevelGradient(cr, area, x + 1, y + 1, opts.crSize - 2,
                              opts.crSize - 2, bgndCol, TRUE, FALSE,
                              APPEARANCE_INVERTED, WIDGET_TROUGH);

            if (coloredMouseOver && !glow) {
                cairo_new_path(cr);
                qtcCairoSetColor(cr, &colors[CR_MO_FILL]);
                cairo_rectangle(cr, x + 1.5, y + 1.5, opts.crSize - 3,
                                opts.crSize - 3);
                /* cairo_rectangle(cr, x + 2.5, y + 2.5, opts.crSize - 5, */
                /*                 opts.crSize - 5); */
                cairo_stroke(cr);
            } else {
                cairo_new_path(cr);
                if (lightBorder) {
                    qtcCairoSetColor(
                        cr, &btnColors[LIGHT_BORDER(APPEARANCE_INVERTED)]);
                    cairo_rectangle(cr, x + 1.5, y + 1.5, opts.crSize - 3,
                                    opts.crSize - 3);
                } else {
                    GdkColor mid = midColor(state == GTK_STATE_INSENSITIVE ?
                                            &style->bg[GTK_STATE_NORMAL] :
                                            &style->base[GTK_STATE_NORMAL],
                                            &colors[3]);
                    qtcCairoSetColor(cr, &mid);
                    cairo_move_to(cr, x + 1.5, y + opts.crSize - 1.5);
                    cairo_line_to(cr, x + 1.5, y + 1.5);
                    cairo_line_to(cr, x + opts.crSize - 1.5, y + 1.5);
                }
                cairo_stroke(cr);
            }
            if (doEtch && (!list || glow) && !mnu) {
                if(glow && !(opts.thin & THIN_FRAMES)) {
                    drawGlow(cr, area, x - 1, y - 1, opts.crSize + 2,
                             opts.crSize + 2, ROUNDED_ALL, WIDGET_CHECKBOX);
                } else {
                    drawEtch(cr, area, widget, x - 1, y - 1, opts.crSize + 2,
                             opts.crSize + 2, false,
                             ROUNDED_ALL, WIDGET_CHECKBOX);
                }
            }
            drawBorder(cr, style, state, area, x, y, opts.crSize,
                       opts.crSize, colors, ROUNDED_ALL, BORDER_FLAT,
                       WIDGET_CHECKBOX, 0);
        }
    }

    if (on) {
        GdkPixbuf *pix = getPixbuf(getCheckRadioCol(style, ind_state, mnu),
                                   PIX_CHECK, 1.0);
        int pw = gdk_pixbuf_get_width(pix);
        int ph = gdk_pixbuf_get_height(pix);
        int dx = x + opts.crSize / 2 - pw / 2;
        int dy = y + opts.crSize / 2 - ph/2;

        gdk_cairo_set_source_pixbuf(cr, pix, dx, dy);
        cairo_paint(cr);
    } else if (tri) {
        int ty = y + opts.crSize / 2;
        GdkColor *col = getCheckRadioCol(style, ind_state, mnu);

        drawHLine(cr, col, 1.0, x + 3, ty, opts.crSize - 6);
        drawHLine(cr, col, 1.0, x + 3, ty + 1, opts.crSize - 6);
    }
}

void drawRadioButton(cairo_t *cr, GtkStateType state, GtkShadowType shadow, GtkStyle *style, GtkWidget *widget, const char *detail,
                     GdkRectangle *area, int x, int y, int width, int height)
{
    if (state == GTK_STATE_PRELIGHT &&
        qtcOneOf(qtSettings.app, GTK_APP_MOZILLA, GTK_APP_JAVA)) {
        state = GTK_STATE_NORMAL;
    }
    gboolean mnu = DETAIL("option");
    gboolean list = !mnu && isList(widget);
    if ((mnu && state == GTK_STATE_PRELIGHT) ||
        (list && state == GTK_STATE_ACTIVE)) {
        state = GTK_STATE_NORMAL;
    }

    if (!qtSettings.qt4 && mnu) {
        drawCheckBox(cr, state, shadow, style, widget, "check", area,
                     x, y, width, height);
    } else {
        gboolean on = shadow == GTK_SHADOW_IN;
        gboolean tri = shadow == GTK_SHADOW_ETCHED_IN;
        /* gboolean set = on || tri; */
        gboolean doEtch = DO_EFFECT;
        int ind_state = (state == GTK_STATE_INSENSITIVE ?
                         state : GTK_STATE_NORMAL);
        int optSpace = doEtch ? opts.crSize + 2 : opts.crSize;
        GdkColor  newColors[TOTAL_SHADES + 1];
        GdkColor *btnColors;
        x += (width - optSpace) / 2;
        y += (height - optSpace) / 2;
        if (opts.crColor && state != GTK_STATE_INSENSITIVE && (on || tri)) {
            btnColors = qtcPalette.selectedcr;
        } else if (!mnu && !list && QT_CUSTOM_COLOR_BUTTON(style)) {
            qtcShadeColors(&(style->bg[state]), newColors);
            btnColors = newColors;
        } else {
            btnColors = qtcPalette.button[state == GTK_STATE_INSENSITIVE ?
                                          PAL_DISABLED : PAL_ACTIVE];
        }
        if (opts.crButton) {
            drawLightBevel(cr, style, state, area, x, y, optSpace, optSpace,
                           &btnColors[getFill(state, false)], btnColors,
                           ROUNDED_ALL, WIDGET_RADIO_BUTTON, BORDER_FLAT,
                           DF_DO_BORDER | (state == GTK_STATE_ACTIVE ?
                                           DF_SUNKEN : 0),
                           list ? NULL : widget);
            if (doEtch) {
                x++;
                y++;
            }
        } else {
            bool glow = (doEtch && state == GTK_STATE_PRELIGHT &&
                         opts.coloredMouseOver == MO_GLOW);
            gboolean lightBorder = DRAW_LIGHT_BORDER(FALSE, WIDGET_TROUGH,
                                                     APPEARANCE_INVERTED);
            /* gboolean draw3d = */
            /*     (!lightBorder && */
            /*      (DRAW_3D_BORDER(FALSE, APPEARANCE_INVERTED) || */
            /*       DRAW_3D_FULL_BORDER(FALSE, APPEARANCE_INVERTED))); */
            gboolean coloredMouseOver = (state == GTK_STATE_PRELIGHT &&
                                         opts.coloredMouseOver);
            gboolean doneShadow = false;
            /* int bgnd = 0; */
            GdkColor *colors = (coloredMouseOver ?
                                qtcPalette.mouseover : btnColors);
            GdkColor *bgndCol =
                (qtcOneOf(state, GTK_STATE_INSENSITIVE, GTK_STATE_ACTIVE) ?
                 &style->bg[GTK_STATE_NORMAL] :
                 !mnu && GTK_STATE_PRELIGHT == state &&
                 !coloredMouseOver && !opts.crHighlight ?
                 &colors[CR_MO_FILL] : &style->base[GTK_STATE_NORMAL]);
            double radius = (opts.crSize + 1) / 2.0;

            if (doEtch) {
                x++;
                y++;
            }
            clipPath(cr, x, y, opts.crSize, opts.crSize, WIDGET_RADIO_BUTTON,
                     RADIUS_EXTERNAL, ROUNDED_ALL);
            drawBevelGradient(cr, NULL, x + 1, y + 1, opts.crSize - 2,
                              opts.crSize - 2, bgndCol,TRUE, FALSE,
                              APPEARANCE_INVERTED, WIDGET_TROUGH);
            cairo_restore(cr);
            if (!mnu && coloredMouseOver && !glow) {
                double radius = (opts.crSize - 2) / 2.0;
                qtcCairoSetColor(cr, &colors[CR_MO_FILL]);
                cairo_arc(cr, x + radius + 1, y+radius + 1,
                          radius, 0, 2 * M_PI);
                cairo_stroke(cr);
                radius--;
                cairo_arc(cr, x + radius + 2, y + radius + 2,
                          radius, 0, 2 * M_PI);
                cairo_stroke(cr);
            }
            if (!doneShadow && doEtch && !mnu && (!list || glow)) {
                double radius = (opts.crSize + 1) / 2.0;
                if (glow) {
                    qtcCairoSetColor(cr, &qtcPalette.mouseover[GLOW_MO]);
                } else {
                    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0,
                                          ETCH_RADIO_TOP_ALPHA);
                }
                if (opts.buttonEffect != EFFECT_NONE || glow) {
                    cairo_arc(cr, x + radius - 0.5, y + radius - 0.5,
                              radius, 0.75 * M_PI, 1.75 * M_PI);
                    cairo_stroke(cr);
                    if (!glow) {
                        setLowerEtchCol(cr, widget);
                    }
                }
                cairo_arc(cr, x + radius - 0.5, y + radius - 0.5,
                          radius, 1.75 * M_PI, 0.75 * M_PI);
                cairo_stroke(cr);
            }
            qtcCairoSetColor(
                cr, &colors[coloredMouseOver ? 4 :
                            BORDER_VAL(state != GTK_STATE_INSENSITIVE)]);
            radius = (opts.crSize - 0.5)/2.0;
            cairo_arc(cr, x + 0.25 + radius, y + 0.25 + radius,
                      radius, 0, 2 * M_PI);
            cairo_stroke(cr);
            if (!coloredMouseOver) {
                radius = (opts.crSize - 1) / 2.0;
                qtcCairoSetColor(cr, &btnColors[coloredMouseOver ? 3 : 4]);
                cairo_arc(cr, x + 0.75 + radius, y + 0.75 + radius, radius,
                          lightBorder ? 0 : 0.75 * M_PI,
                          lightBorder ? 2 * M_PI : 1.75 * M_PI);
                cairo_stroke(cr);
            }
        }
        if (on) {
            GdkColor *col = getCheckRadioCol(style, ind_state, mnu);
            double radius = opts.smallRadio ? 2.5 : 3.5;
            double offset = opts.crSize / 2.0 - radius;

            qtcCairoSetColor(cr, col);
            cairo_arc(cr, x + offset + radius, y + offset + radius,
                      radius, 0, 2 * M_PI);
            cairo_fill(cr);
        } else if (tri) {
            int ty = y + opts.crSize / 2;
            GdkColor *col = getCheckRadioCol(style, ind_state, mnu);

            drawHLine(cr, col, 1.0, x + 3, ty, opts.crSize - 6);
            drawHLine(cr, col, 1.0, x + 3, ty + 1, opts.crSize - 6);
        }
    }
}

void
drawTab(cairo_t *cr, GtkStateType state, GtkStyle *style, GtkWidget *widget,
        const char *detail, GdkRectangle *area, int x, int y, int width,
        int height, GtkPositionType gapSide)
{
    QTC_UNUSED(detail);
    GtkNotebook *notebook=GTK_IS_NOTEBOOK(widget) ? GTK_NOTEBOOK(widget) : NULL;
    gboolean    highlightingEnabled=notebook && (opts.highlightFactor || opts.coloredMouseOver);
    gboolean    highlight=FALSE;
    int highlightedTabIndex = qtcTabCurrentHoveredIndex(widget);
    /* int dark = ((APPEARANCE_FLAT == opts.appearance) ? */
    /*             ORIGINAL_SHADE : FRAME_DARK_SHADOW); */
    int moOffset = ((ROUNDED_NONE == opts.round ||
                     TAB_MO_TOP != opts.tabMouseOver) ? 1 : opts.round);
    GtkWidget *parent=widget ? gtk_widget_get_parent(widget) : NULL;
    gboolean firstTab = !notebook,
        lastTab = !notebook,
        vertical = qtcOneOf(gapSide, GTK_POS_LEFT, GTK_POS_RIGHT),
        active = state == GTK_STATE_NORMAL, /* Normal -> active tab? */
        rev = qtcOneOf(gapSide, GTK_POS_TOP, GTK_POS_BOTTOM) && parent && reverseLayout(parent),
        mozTab=isMozillaTab(widget),
        glowMo=!active && notebook && opts.coloredMouseOver && TAB_MO_GLOW==opts.tabMouseOver,
        drawOuterGlow=glowMo && !(opts.thin&THIN_FRAMES);
    int         mod=active ? 1 : 0,
        highlightOffset=opts.highlightTab && opts.round>ROUND_SLIGHT ? 2 : 1,
        highlightBorder=(opts.round>ROUND_FULL ? 4 : 3),
        sizeAdjust=(!active || mozTab) && TAB_MO_GLOW==opts.tabMouseOver ? 1 : 0,
        tabIndex = -1;
    GdkColor *col = (active ? &(style->bg[GTK_STATE_NORMAL]) :
                     &(qtcPalette.background[2]));
    GdkColor *selCol1 = &qtcPalette.highlight[0];
    /* GdkColor *selCol2 = */
    /*     &qtcPalette.highlight[qtcIsFlat(opts.appearance) ? 0 : 3]; */
    GdkRectangle clipArea;
    EBorder     borderProfile=active || opts.borderInactiveTab
        ? opts.borderTab
        ? BORDER_LIGHT
        : BORDER_RAISED
        : BORDER_FLAT;

#if !GTK_CHECK_VERSION(2, 90, 0)
    /* Hacky fix for tabs in Thunderbird */
    if(mozTab && area && area->x<(x-10))
        return;
#endif

    /* f'in mozilla apps dont really use Gtk widgets - they just paint to a pixmap. So, no way of knowing
       the position of a tab! The 'best' look seems to be to round both corners. Not nice, but... */
    if(mozTab || GTK_APP_JAVA==qtSettings.app)
        firstTab=lastTab=TRUE;
    else if(notebook)
    {
        /* Borrowed from Qt engine... */
        GList      *children=gtk_container_get_children(GTK_CONTAINER(widget));
        int         num_children=children ? g_list_length(children) : 0,
            i, sdiff = 10000,
            first_shown=-1,
            last_shown=-1;
        /* GtkWidget *p=widget; */

        if(children)
            g_list_free(children);

        for (i = 0; i < num_children; i++ )
        {
            GtkWidget *page=gtk_notebook_get_nth_page(notebook, i),
                *tab_label=gtk_notebook_get_tab_label(notebook, page);
            int       diff=-1;

            if(tab_label)
            {
                GtkAllocation alloc=qtcWidgetGetAllocation(tab_label);
                diff=(vertical ? alloc.y-y : alloc.x-x);
            }

            if ((diff > 0) && (diff < sdiff))
            {
                sdiff = diff;
                tabIndex=i;
                /* p=tab_label; */
            }

            if(page && gtk_widget_get_visible(page))
            {
                if(i>last_shown)
                    last_shown=i;
                if(-1==first_shown)
                    first_shown=i;
            }
        }

        if (qtcOneOf(tabIndex, 0, first_shown)) {
            firstTab = TRUE;
        } else if (qtcOneOf(tabIndex, num_children - 1, last_shown)) {
            lastTab = TRUE;
        }
        if (qtcNoneOf(highlightedTabIndex, -1, tabIndex) && !active) {
            highlight = TRUE;
            col = &qtcPalette.background[SHADE_2_HIGHLIGHT];
        }
    }

    if (rev) {
        gboolean oldLast = lastTab;

        lastTab = firstTab;
        firstTab = oldLast;
    }

    if (!mozTab && qtSettings.app != GTK_APP_JAVA) {
        if(-1==highlightedTabIndex && (highlightingEnabled || opts.windowDrag>=WM_DRAG_ALL))
            qtcTabSetup(widget);
        qtcTabUpdateRect(widget, tabIndex, x, y, width, height);
    }

/*
  gtk_style_apply_default_background(style, window, widget && !qtcWidgetNoWindow(widget),
  GTK_STATE_NORMAL, area, x, y, width, height);
*/
    /*
      TODO: This is not good, should respect 'area' as passed in. However, it works for the moment - if the thunderbird hack
      above is used. Needs to be fixed tho.
    */

    /* In addition to the above, doing this section only for the active mozilla tab seems to fix some drawing errors
       with firefox3...*/
    /* CPD 28/02/2008 Dont need to do any of this for firefox 3beta4 */
    if(!mozTab) /* || active) */
    {
        clipArea.x=x;
        clipArea.y=y;
        clipArea.width=width;
        clipArea.height=height;
        area=&clipArea;
    }

    glowMo=glowMo && highlight;
    drawOuterGlow=drawOuterGlow && highlight;

    switch(gapSide)
    {
    case GTK_POS_TOP:  /* => tabs are on bottom !!! */
    {
        int round=active || (firstTab && lastTab) || TAB_MO_GLOW==opts.tabMouseOver || opts.roundAllTabs
            ? ROUNDED_BOTTOM
            : firstTab
            ? ROUNDED_BOTTOMLEFT
            : lastTab
            ? ROUNDED_BOTTOMRIGHT
            : ROUNDED_NONE;
        clipPath(cr, x, y-4, width, height+4, WIDGET_TAB_BOT, RADIUS_EXTERNAL, round);
        fillTab(cr, style, widget, area, state, col, x+mod+sizeAdjust, y, width-(2*mod+(sizeAdjust)), height-1, TRUE,
                WIDGET_TAB_BOT, NULL!=notebook);
        cairo_restore(cr);
        drawBorder(cr, style, state, area, x+sizeAdjust, y-4, width-(2*sizeAdjust), height+4,
                   glowMo ? qtcPalette.mouseover : qtcPalette.background, round,
                   borderProfile, WIDGET_TAB_BOT, 0);
        if(drawOuterGlow)
        {
            if(area)
                area->height++;
            drawGlow(cr, area, x, y-4, width, height+5, round, WIDGET_OTHER);
        }

        if (notebook && opts.highlightTab && active) {
            drawHLine(cr, selCol1, 0.5, x + 1, y + height - 3, width - 2);
            drawHLine(cr, selCol1, 1.0, x + highlightOffset, y + height - 2,
                      width - 2 * highlightOffset);

            clipArea.y=y+height-highlightBorder;
            clipArea.height=highlightBorder;
            drawBorder(cr, style, state, &clipArea, x, y, width, height,
                       qtcPalette.highlight, ROUNDED_BOTTOM, BORDER_FLAT,
                       WIDGET_OTHER, 0, 3);
        }

        if(opts.colorSelTab && notebook && active)
            colorTab(cr, x+mod+sizeAdjust, y, width-(2*mod+(sizeAdjust)), height-1, round, WIDGET_TAB_BOT, true);

        if(notebook && opts.coloredMouseOver && highlight && TAB_MO_GLOW!=opts.tabMouseOver)
            drawHighlight(cr, x+(firstTab ? moOffset : 1),
                          y+(TAB_MO_TOP==opts.tabMouseOver ? height-2 : -1), width-(firstTab || lastTab ? moOffset : 1), 2,
                          NULL, true, TAB_MO_TOP!=opts.tabMouseOver);

        break;
    }
    case GTK_POS_BOTTOM: /* => tabs are on top !!! */
    {
        int round=active || (firstTab && lastTab) || TAB_MO_GLOW==opts.tabMouseOver || opts.roundAllTabs
            ? ROUNDED_TOP
            : firstTab
            ? ROUNDED_TOPLEFT
            : lastTab
            ? ROUNDED_TOPRIGHT
            : ROUNDED_NONE;
        clipPath(cr, x+mod+sizeAdjust, y, width-(2*(mod+(mozTab ? 2 *sizeAdjust : sizeAdjust))), height+5, WIDGET_TAB_TOP, RADIUS_EXTERNAL, round);
        fillTab(cr, style, widget, area, state, col, x+mod+sizeAdjust, y+1,
                width-(2*(mod+(mozTab ? 2 *sizeAdjust : sizeAdjust))), height-1, TRUE, WIDGET_TAB_TOP, NULL!=notebook);
        cairo_restore(cr);
        drawBorder(cr, style, state, area, x+sizeAdjust, y, width-(2*(mozTab ? 2 : 1)*sizeAdjust), height+4,
                   glowMo ? qtcPalette.mouseover : qtcPalette.background, round, borderProfile, WIDGET_TAB_TOP, 0);
        if(drawOuterGlow)
        {
            if(area)
                area->y--, area->height+=2;
            drawGlow(cr, area, x, y-1, width, height+5, round, WIDGET_OTHER);
        }

        if(notebook && opts.highlightTab && active)
        {
            drawHLine(cr, selCol1, 0.5, x + 1, y + 2, width - 2);
            drawHLine(cr, selCol1, 1.0, x + highlightOffset, y + 1,
                      width - 2 * highlightOffset);

            clipArea.y=y;
            clipArea.height=highlightBorder;
            drawBorder(cr, style, state, &clipArea, x, y, width, height,
                       qtcPalette.highlight, ROUNDED_TOP, BORDER_FLAT,
                       WIDGET_OTHER, 0, 3);
        }

        if(opts.colorSelTab && notebook && active)
            colorTab(cr, x+mod+sizeAdjust, y+1, width-(2*(mod+(mozTab ? 2 *sizeAdjust : sizeAdjust))), height-1, round, WIDGET_TAB_TOP, true);

        if(notebook && opts.coloredMouseOver && highlight && TAB_MO_GLOW!=opts.tabMouseOver)
            drawHighlight(cr, x+(firstTab ? moOffset : 1), y+(TAB_MO_TOP==opts.tabMouseOver ? 0 : height-1),
                          width-(firstTab || lastTab ? moOffset : 1), 2, NULL, true, TAB_MO_TOP==opts.tabMouseOver);
        break;
    }
    case GTK_POS_LEFT: /* => tabs are on right !!! */
    {
        int round=active || (firstTab && lastTab) || TAB_MO_GLOW==opts.tabMouseOver || opts.roundAllTabs
            ? ROUNDED_RIGHT
            : firstTab
            ? ROUNDED_TOPRIGHT
            : lastTab
            ? ROUNDED_BOTTOMRIGHT
            : ROUNDED_NONE;
        clipPath(cr, x-4, y, width+4, height, WIDGET_TAB_BOT, RADIUS_EXTERNAL, round);
        fillTab(cr, style, widget, area, state, col, x, y+mod+sizeAdjust, width-1, height-(2*(mod+sizeAdjust)), FALSE,
                WIDGET_TAB_BOT, NULL!=notebook);
        cairo_restore(cr);
        drawBorder(cr, style, state, area, x-4, y+sizeAdjust, width+4, height-(2*sizeAdjust),
                   glowMo ? qtcPalette.mouseover : qtcPalette.background, round, borderProfile, WIDGET_TAB_BOT, 0);
        if(drawOuterGlow)
        {
            if(area)
                area->width++;
            drawGlow(cr, area, x-4, y, width+5, height, round, WIDGET_OTHER);
        }

        if (notebook && opts.highlightTab && active) {
            drawVLine(cr, selCol1, 0.5, x + width - 3, y + 1, height - 2);
            drawVLine(cr, selCol1, 1.0, x + width - 2, y + highlightOffset,
                      height - 2 * highlightOffset);

            clipArea.x=x+width-highlightBorder;
            clipArea.width=highlightBorder;
            drawBorder(cr, style, state, &clipArea, x, y, width, height,
                       qtcPalette.highlight, ROUNDED_RIGHT, BORDER_FLAT,
                       WIDGET_OTHER, 0, 3);
        }

        if(opts.colorSelTab && notebook && active)
            colorTab(cr, x, y+mod+sizeAdjust, width-1, height-(2*(mod+sizeAdjust)), round, WIDGET_TAB_BOT, false);

        if(notebook && opts.coloredMouseOver && highlight && TAB_MO_GLOW!=opts.tabMouseOver)
            drawHighlight(cr, x+(TAB_MO_TOP==opts.tabMouseOver ? width-2 : -1),
                          y+(firstTab ? moOffset : 1), 2, height-(firstTab || lastTab ? moOffset : 1),
                          NULL, false, TAB_MO_TOP!=opts.tabMouseOver);
        break;
    }
    case GTK_POS_RIGHT: /* => tabs are on left !!! */
    {
        int round=active || (firstTab && lastTab) || TAB_MO_GLOW==opts.tabMouseOver || opts.roundAllTabs
            ? ROUNDED_LEFT
            : firstTab
            ? ROUNDED_TOPLEFT
            : lastTab
            ? ROUNDED_BOTTOMLEFT
            : ROUNDED_NONE;
        clipPath(cr, x, y, width+4, height, WIDGET_TAB_TOP, RADIUS_EXTERNAL, round);
        fillTab(cr, style, widget, area, state, col, x+1, y+mod+sizeAdjust, width-1, height-(2*(mod+sizeAdjust)),
                FALSE, WIDGET_TAB_TOP, NULL!=notebook);
        cairo_restore(cr);
        drawBorder(cr, style, state, area, x, y+sizeAdjust, width+4, height-(2*sizeAdjust),
                   glowMo ? qtcPalette.mouseover : qtcPalette.background, round, borderProfile, WIDGET_TAB_TOP, 0);
        if(drawOuterGlow)
        {
            if(area)
                area->x--, area->width+=2;
            drawGlow(cr, area, x-1, y, width+5, height, round, WIDGET_OTHER);
        }
        if (notebook && opts.highlightTab && active) {
            drawVLine(cr, selCol1, 0.5, x + 2, y + 1, height - 2);
            drawVLine(cr, selCol1, 1.0, x + 1, y + highlightOffset,
                      height - 2 * highlightOffset);

            clipArea.x=x;
            clipArea.width=highlightBorder;
            drawBorder(cr, style, state, &clipArea, x, y, width, height,
                       qtcPalette.highlight, ROUNDED_LEFT, BORDER_FLAT,
                       WIDGET_OTHER, 0, 3);
        }

        if(opts.colorSelTab && notebook && active)
            colorTab(cr, x+1, y+mod+sizeAdjust, width-1, height-(2*(mod+sizeAdjust)), round, WIDGET_TAB_TOP, false);

        if(notebook && opts.coloredMouseOver && highlight && TAB_MO_GLOW!=opts.tabMouseOver)
            drawHighlight(cr, x+(TAB_MO_TOP==opts.tabMouseOver ? 0 : width-1),
                          y+(firstTab ? moOffset : 1), 2, height-(firstTab || lastTab ? moOffset : 1),
                          NULL, false, TAB_MO_TOP==opts.tabMouseOver);
        break;
    }
    }
}

void drawToolbarBorders(cairo_t *cr, GtkStateType state,  int x, int y, int width, int height, gboolean isActiveWindowMenubar, const char *detail)
{
    gboolean top = FALSE;
    gboolean bottom = FALSE;
    gboolean left = FALSE;
    gboolean right = FALSE;
    gboolean all = qtcOneOf(opts.toolbarBorders, TB_LIGHT_ALL, TB_DARK_ALL);
    int border = qtcOneOf(opts.toolbarBorders, TB_DARK, TB_DARK_ALL) ? 3 : 4;
    GdkColor *cols = (isActiveWindowMenubar &&
                      (state != GTK_STATE_INSENSITIVE ||
                       opts.shadeMenubars != SHADE_NONE) ?
                      menuColors(isActiveWindowMenubar) :
                      qtcPalette.background);
    if (DETAIL("menubar")) {
        if(all)
            top=bottom=left=right=TRUE;
        else
            bottom=TRUE;
    }
    else if(0==strcmp(detail,"toolbar")) /*  && (GTK_IS_TOOLBAR(widget) ||
                                             WIDGET_TYPE_NAME("BonoboUIToolbar"))) */
    {
//         if(GTK_IS_TOOLBAR(widget))
//         {
//             if(all)
//                 top=bottom=left=right=TRUE;
//             else if(GTK_ORIENTATION_HORIZONTAL==qtcToolbarGetOrientation(widget))
//                 top=bottom=TRUE;
//             else
//                 left=right=TRUE;
//         }
//         else
        if(all)
            if(width<height)
                left=right=bottom=TRUE;
            else
                top=bottom=right=TRUE;
        else
            if(width<height)
                left=right=TRUE;
            else
                top=bottom=TRUE;
    }
    else if(0==strcmp(detail,"dockitem_bin") || /* CPD: bit risky - what if only 1 item ??? */
            0==strcmp(detail, "handlebox_bin"))
    {
        if(all)
            if(width<height)
                left=right=bottom=TRUE;
            else
                top=bottom=right=TRUE;
        else
            if(width<height)
                left=right=TRUE;
            else
                top=bottom=TRUE;
    }
    else /* handle */
        if(all)
            if(width<height) /* on horiz toolbar */
                top=bottom=left=TRUE;
            else
                left=right=top=TRUE;
        else
            if(width<height) /* on horiz toolbar */
                top=bottom=TRUE;
            else
                left=right=TRUE;

    if (top) {
        drawHLine(cr, cols, 1.0, x, y, width);
    }
    if (left) {
        drawVLine(cr, cols, 1.0, x, y, height);
    }
    if (bottom) {
        drawHLine(cr, &cols[border], 1.0, x, y + height - 1, width);
    }
    if (right) {
        drawVLine(cr, &cols[border], 1.0, x + width - 1, y, height);
    }
}

void drawListViewHeader(cairo_t *cr, GtkStateType state, GdkColor *btnColors, int bgnd, GdkRectangle *area, int x, int y, int width, int height)
{
    drawBevelGradient(cr, area, x, y, width, height, &btnColors[bgnd], TRUE, GTK_STATE_ACTIVE==state || 2==bgnd || 3==bgnd,
                      opts.lvAppearance, WIDGET_LISTVIEW_HEADER);

    if (opts.lvAppearance == APPEARANCE_RAISED)
        drawHLine(cr, &qtcPalette.background[4], 1.0, x,
                  y + height-2, width);
    drawHLine(cr, &qtcPalette.background[QTC_STD_BORDER], 1.0,
              x, y + height - 1, width);

    if(GTK_STATE_PRELIGHT==state && opts.coloredMouseOver)
        drawHighlight(cr, x, y+height-2, width, 2, area, true, true);

#if GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
    drawFadedLine(cr, x+width-2, y+4, 1, height-8, &btnColors[QTC_STD_BORDER], area, NULL, TRUE, TRUE, FALSE);
    drawFadedLine(cr, x+width-1, y+4, 1, height-8, &btnColors[0], area, NULL, TRUE, TRUE, FALSE);
#else
    if(x>3 && height>10)
    {
        drawFadedLine(cr, x, y+4, 1, height-8, &btnColors[QTC_STD_BORDER], area, NULL, TRUE, TRUE, FALSE);
        drawFadedLine(cr, x+1, y+4, 1, height-8, &btnColors[0], area, NULL, TRUE, TRUE, FALSE);
    }
#endif
}

void drawDefBtnIndicator(cairo_t *cr, GtkStateType state, GdkColor *btnColors, int bgnd, gboolean sunken, GdkRectangle *area, int x, int y, int width, int height)
{
    if(IND_CORNER==opts.defBtnIndicator)
    {
        int      offset=sunken ? 5 : 4,
            etchOffset=DO_EFFECT ? 1 : 0;
        GdkColor *cols=qtcPalette.focus ? qtcPalette.focus : qtcPalette.highlight,
            *col=&cols[GTK_STATE_ACTIVE==state ? 0 : 4];

        cairo_new_path(cr);
        qtcCairoSetColor(cr, col);
        cairo_move_to(cr, x+offset+etchOffset, y+offset+etchOffset);
        cairo_line_to(cr, x+offset+6+etchOffset, y+offset+etchOffset);
        cairo_line_to(cr, x+offset+etchOffset, y+offset+6+etchOffset);
        cairo_fill(cr);
    }
    else if(IND_COLORED==opts.defBtnIndicator && (COLORED_BORDER_SIZE>2))
    {
        int o=COLORED_BORDER_SIZE+(DO_EFFECT ? 1 : 0); // offset needed because of etch

        drawBevelGradient(cr, area, x+o, y+o, width-(2*o), height-(2*o),
                          &btnColors[bgnd], TRUE, GTK_STATE_ACTIVE==state, opts.appearance, WIDGET_STD_BUTTON);
    }
}

static GdkPixbuf * scaleOrRef(GdkPixbuf *src, int width, int height)
{
    if (width == gdk_pixbuf_get_width(src) && height == gdk_pixbuf_get_height(src))
        return g_object_ref (src);
    else
        return gdk_pixbuf_scale_simple(src, width, height, GDK_INTERP_BILINEAR);
}

static GdkPixbuf*
setTransparency(const GdkPixbuf *pixbuf, double alpha_percent)
{
    GdkPixbuf *target;

    g_return_val_if_fail(pixbuf != NULL, NULL);
    g_return_val_if_fail(GDK_IS_PIXBUF (pixbuf), NULL);

    /* Returns a copy of pixbuf with it's non-completely-transparent pixels to
       have an alpha level "alpha_percent" of their original value. */

    target = gdk_pixbuf_add_alpha (pixbuf, FALSE, 0, 0, 0);

    if (alpha_percent == 1.0) {
        return target;
    } else {
        unsigned width = gdk_pixbuf_get_width(target);
        unsigned height = gdk_pixbuf_get_height(target);
        unsigned rowstride = gdk_pixbuf_get_rowstride(target);
        unsigned char *data = gdk_pixbuf_get_pixels(target);
        for (unsigned y = 0;y < height;y++) {
            for (unsigned x = 0;x < width;x++) {
                data[y * rowstride + x * 4 + 3] *= alpha_percent;
            }
        }
    }

    return target;
}

GdkPixbuf*
renderIcon(GtkStyle *style, const GtkIconSource *source,
           GtkTextDirection direction, GtkStateType state, GtkIconSize size,
           GtkWidget *widget, const char *detail)
{
    QTC_UNUSED(direction);
    QTC_UNUSED(detail);
    int         width = 1,
        height = 1;
    GdkPixbuf   *scaled,
        *stated,
        *base_pixbuf;
    GdkScreen   *screen;
    GtkSettings *settings;
    gboolean    scaleMozilla=opts.mapKdeIcons && isMozilla() && GTK_ICON_SIZE_DIALOG==size;

    /* Oddly, style can be NULL in this function, because GtkIconSet can be used without a style and if so
     * it uses this function. */
    base_pixbuf = gtk_icon_source_get_pixbuf(source);

    g_return_val_if_fail(base_pixbuf != NULL, NULL);

    if(widget && gtk_widget_has_screen(widget))
    {
        screen = gtk_widget_get_screen(widget);
        settings = screen ? gtk_settings_get_for_screen(screen) : NULL;
    }
#if GTK_CHECK_VERSION(2, 90, 0)
    else if (style->visual)
    {
        screen = gdk_visual_get_screen(style->visual);
        settings = screen ? gtk_settings_get_for_screen(screen) : NULL;
    }
#else
    else if(style->colormap)
    {
        screen = gdk_colormap_get_screen(style->colormap);
        settings = screen ? gtk_settings_get_for_screen(screen) : NULL;
    }
#endif
    else
    {
        settings = gtk_settings_get_default();
        GTK_NOTE(MULTIHEAD, g_warning("Using the default screen for gtk_default_render_icon()"));
    }

    if(scaleMozilla)
        width=height=48;
    else if(size !=(GtkIconSize) -1 && !gtk_icon_size_lookup_for_settings(settings, size, &width, &height))
    {
        g_warning(G_STRLOC ": invalid icon size '%d'", size);
        return NULL;
    }

    /* If the size was wildcarded, and we're allowed to scale, then scale; otherwise,
     * leave it alone. */
    if(scaleMozilla || (size !=(GtkIconSize)-1 && gtk_icon_source_get_size_wildcarded(source)))
        scaled = scaleOrRef(base_pixbuf, width, height);
    else
        scaled = g_object_ref(base_pixbuf);

    /* If the state was wildcarded, then generate a state. */
    if(gtk_icon_source_get_state_wildcarded(source))
    {
        if(GTK_STATE_INSENSITIVE==state)
        {
            stated = setTransparency(scaled, 0.5);
            gdk_pixbuf_saturate_and_pixelate(stated, stated, 0.0, FALSE);
            g_object_unref(scaled);
        }
#if 0 /* KDE does not highlight icons */
        else if(GTK_STATE_PRELIGHT==state)
        {
            stated = gdk_pixbuf_copy(scaled);
            gdk_pixbuf_saturate_and_pixelate(scaled, stated, 1.2, FALSE);
            g_object_unref(scaled);
        }
#endif
        else
            stated = scaled;
    }
    else
        stated = scaled;

    return stated;
}
