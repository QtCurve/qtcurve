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

#include "config.h"

#include <qtcurve-utils/color.h>
#include <qtcurve-utils/gtkutils.h>
#include <qtcurve-utils/x11utils.h>

#include <gmodule.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <cairo.h>
#include "compatability.h"
#include "qtcurve.h"
#include <common/config_file.h>

#define MO_ARROW(MENU, COL)                             \
    (!MENU && MO_NONE != opts.coloredMouseOver &&       \
     GTK_STATE_PRELIGHT == state ?                      \
     &qtcPalette.mouseover[ARROW_MO_SHADE] : (COL))

#include "qt_settings.h"
#include "animation.h"
#include "menu.h"
#include "tab.h"
#include "widgetmap.h"
#include "window.h"
#include "entry.h"
#include "treeview.h"
#include "combobox.h"
#include "scrolledwindow.h"
#include "scrollbar.h"
#include "wmmove.h"
#include "helpers.h"
#include "drawing.h"
#include "pixcache.h"
#include "shadowhelper.h"
#include "config.h"

static GtkStyleClass *parent_class=NULL;

#ifdef INCREASE_SB_SLIDER
typedef struct
{
    GtkStyle *style;
#if GTK_CHECK_VERSION(2, 90, 0)
    cairo_t *cr;
#else
    GdkWindow *window;
#endif
    GtkStateType state;
    GtkShadowType shadow;
    GtkWidget *widget;
    const gchar *detail;
    gint x;
    gint y;
    gint width;
    gint height;
    GtkOrientation orientation;
} QtCSlider;

static QtCSlider lastSlider;
#endif

#define WIDGET_TYPE_NAME(xx) (widget && !strcmp(g_type_name (G_TYPE_FROM_INSTANCE(widget)), (xx)))

static void gtkDrawBox(GtkStyle *style, GdkWindow *window, GtkStateType state, GtkShadowType shadow, GdkRectangle *area,
                       GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height);



static void gtkDrawSlider(GtkStyle *style, GdkWindow *window, GtkStateType state, GtkShadowType shadow, GdkRectangle *area,
                          GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height, GtkOrientation orientation);

static void
qtcLogHandler(const gchar *domain, GLogLevelFlags level, const gchar *msg,
              gpointer data)
{
    QTC_UNUSED(domain);
    QTC_UNUSED(level);
    QTC_UNUSED(msg);
    QTC_UNUSED(data);
}

static void gtkDrawFlatBox(GtkStyle *style, GdkWindow *window, GtkStateType state, GtkShadowType shadow, GdkRectangle *area,
                           GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height)
{
    g_return_if_fail(GTK_IS_STYLE(style));
    g_return_if_fail(GDK_IS_DRAWABLE(window));
    cairo_t *cr = gdk_cairo_create(window);
    setCairoClipping(cr, area);
    cairo_set_line_width(cr, 1.0);

    gboolean isMenuOrToolTipWindow =
        (widget && GTK_IS_WINDOW(widget) &&
         ((gtk_widget_get_name(widget) &&
           strcmp(gtk_widget_get_name(widget), "gtk-tooltip") == 0) ||
          isMenuWindow(widget)));

    if (DEBUG_ALL == qtSettings.debug)
        printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %s  ", __FUNCTION__, state, shadow, x, y, width, height, detail ? detail : "NULL"),
            debugDisplayWidget(widget, 10);

    sanitizeSize(window, &width, &height);

#define MODAL_HACK "QTC_MODAL_HACK_SET"
#define BUTTON_HACK "QTC_BUTTON_ORDER_HACK_SET"

#if GTK_CHECK_VERSION(2, 6, 0)
    if (!opts.gtkButtonOrder && opts.reorderGtkButtons && GTK_IS_WINDOW(widget) && detail && 0==strcmp(detail, "base"))
    {
        GtkWidget *topLevel=gtk_widget_get_toplevel(widget);

        if(topLevel && GTK_IS_DIALOG(topLevel) && !g_object_get_data(G_OBJECT(topLevel), BUTTON_HACK))
        {
            // gtk_dialog_set_alternative_button_order will cause errors to be
            // logged, but dont want these so register ur own error handler,
            // and then unregister afterwards...
            guint id=g_log_set_handler("Gtk", G_LOG_LEVEL_CRITICAL, qtcLogHandler, NULL);
            g_object_set_data(G_OBJECT(topLevel), BUTTON_HACK, (gpointer)1);

            gtk_dialog_set_alternative_button_order(GTK_DIALOG(topLevel), GTK_RESPONSE_HELP,
                                                    GTK_RESPONSE_OK, GTK_RESPONSE_YES, GTK_RESPONSE_ACCEPT, GTK_RESPONSE_APPLY,
                                                    GTK_RESPONSE_REJECT, GTK_RESPONSE_CLOSE, GTK_RESPONSE_NO, GTK_RESPONSE_CANCEL, -1);
            g_log_remove_handler("Gtk", id);
            g_log_set_handler("Gtk", G_LOG_LEVEL_CRITICAL, g_log_default_handler, NULL);
        }
    }
#endif

    if(opts.windowDrag>WM_DRAG_MENU_AND_TOOLBAR && (DETAIL("base") || DETAIL("eventbox") || DETAIL("viewportbin")))
        qtcWMMoveSetup(widget);

    if(widget && ((100!=opts.bgndOpacity && GTK_IS_WINDOW(widget)) || (100!=opts.dlgOpacity && GTK_IS_DIALOG(widget))) &&
       !isFixedWidget(widget) && isRgbaWidget(widget))
        enableBlurBehind(widget, TRUE);

    if ((opts.menubarHiding || opts.statusbarHiding || BLEND_TITLEBAR ||
         opts.windowBorder & WINDOW_BORDER_USE_MENUBAR_COLOR_FOR_TITLEBAR) &&
        widget && GTK_IS_WINDOW(widget) && !isFixedWidget(widget) &&
        !isGimpDockable(widget) && !isMenuOrToolTipWindow) {
        if (qtcWindowSetup(widget, GTK_IS_DIALOG(widget) ? opts.dlgOpacity :
                           opts.bgndOpacity)) {
            GtkWidget *menuBar = qtcWindowGetMenuBar(widget, 0);
            GtkWidget *statusBar =
                opts.statusbarHiding ? qtcWindowGetStatusBar(widget, 0) : NULL;

            if (menuBar) {
                bool hiddenMenubar =
                    (opts.menubarHiding ?
                     qtcMenuBarHidden(qtSettings.appName) : FALSE);
                GtkAllocation alloc = qtcWidgetGetAllocation(menuBar);

                if (hiddenMenubar)
                    gtk_widget_hide(menuBar);

                if(BLEND_TITLEBAR || opts.menubarHiding&HIDE_KWIN || opts.windowBorder&WINDOW_BORDER_USE_MENUBAR_COLOR_FOR_TITLEBAR)
                    qtcMenuEmitSize(menuBar, hiddenMenubar ? 0 : alloc.height);

                if(opts.menubarHiding&HIDE_KWIN)
                    qtcWindowMenuBarDBus(widget, hiddenMenubar ? 0 : alloc.height);
            }

#if GTK_CHECK_VERSION(2, 90, 0)
            if(gtk_window_get_has_resize_grip(GTK_WINDOW(widget)))
                gtk_window_set_has_resize_grip(GTK_WINDOW(widget), FALSE);
#else
            if(statusBar && gtk_statusbar_get_has_resize_grip(GTK_STATUSBAR(statusBar)))
                gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(statusBar), FALSE);
#endif

            if(opts.statusbarHiding && statusBar)
            {
                gboolean hiddenStatusBar=qtcStatusBarHidden(qtSettings.appName);
                if(hiddenStatusBar)
                    gtk_widget_hide(statusBar);
                if(opts.statusbarHiding&HIDE_KWIN)
                {
                    qtcWindowStatusBarDBus(widget, !hiddenStatusBar);
                    qtcWindowSetStatusBarProp(widget);
                }
            }
        }
    }

    if(widget && qtcIsCustomBgnd(&opts) && (DETAIL("base") || DETAIL("eventbox")))
        qtcScrollbarSetup(widget);

    if(qtcIsCustomBgnd(&opts) && DETAIL("viewportbin"))
    {
        GtkRcStyle *st=widget ? gtk_widget_get_modifier_style(widget) : NULL;
        // if the app hasn't modified bg, draw background gradient
        if(st && !(st->color_flags[state]&GTK_RC_BG))
        {
            drawWindowBgnd(cr, style, area, window, widget, x, y, width, height);
            qtcScrollbarSetup(widget);
        }
        else
            parent_class->draw_flat_box(style, window, state, shadow, area, widget, detail, x, y, width, height);
    } else if (qtcIsCustomBgnd(&opts) && widget && GTK_IS_WINDOW(widget) &&
             !isMenuOrToolTipWindow &&
             drawWindowBgnd(cr, style, area, window, widget,
                            x, y, width, height)) {
        qtcWindowSetup(widget, GTK_IS_DIALOG(widget) ? opts.dlgOpacity :
                       opts.bgndOpacity);
    } else if(widget && GTK_IS_TREE_VIEW(widget)) {
        gboolean    isCombo=isComboBoxPopupWindow(widget, 0);
        GtkTreeView *treeView=GTK_TREE_VIEW(widget);
        gboolean    checkRules=opts.forceAlternateLvCols || gtk_tree_view_get_rules_hint(treeView),
                    isEven=checkRules && DETAILHAS("cell_even");

        if(GTK_APP_JAVA_SWT==qtSettings.app)
            area=NULL;

        /* SWT seems to draw a 'cell_even', and then 'cell_odd' at the same position. This causes the view painting
         * to be messed up. Try and hack around this... */
        if(GTK_APP_JAVA_SWT==qtSettings.app && GTK_STATE_SELECTED==state && checkRules && !isCombo && widget && detail)
        {
            static GtkWidget *lastWidget=NULL;
            static int       lastEven=-1;

            if(DETAILHAS("cell_even"))
            {
                lastWidget=widget;
                lastEven=y;
            }
            else if(DETAILHAS("cell_odd"))
            {
                if(lastWidget==widget)
                {
                    if(y==lastEven)
                        isEven=TRUE;
                }
                lastWidget=NULL;
                lastEven=-1;
            }
        }

        if(!isCombo || GTK_STATE_SELECTED!=state)
            drawAreaColor(cr, area,
                          getCellCol(haveAlternateListViewCol() && checkRules && !isEven
                                        ? &qtSettings.colors[PAL_ACTIVE][COLOR_LV]
                                        : &style->base[GTK_STATE_NORMAL], detail),
                          x, y, width, height);

        if(isCombo)
        {
            if(GTK_STATE_SELECTED==state)
                drawAreaColor(cr, area, &style->base[widget && gtk_widget_has_focus(widget) ? GTK_STATE_SELECTED : GTK_STATE_ACTIVE],
                              x, y, width, height);
        }
        else
        {
            double   alpha=1.0;
            int      selX=x,
                     selW=width,
                     factor=0;
            gboolean forceCellStart=FALSE,
                     forceCellEnd=FALSE;

#if GTK_CHECK_VERSION(2, 12, 0)
            if(!isFixedWidget(widget))
            {
                GtkTreePath       *path=NULL;
                GtkTreeViewColumn *column=NULL,
                                  *expanderColumn=gtk_tree_view_get_expander_column(treeView);
                int               levelIndent=0,
                                  expanderSize=0,
                                  depth=0;

                qtcTreeViewGetCell(treeView, &path, &column, x, y, width, height);
                qtcTreeViewSetup(widget);
                if(path && qtcTreeViewIsCellHovered(widget, path, column))
                {
                    if(GTK_STATE_SELECTED==state)
                        factor=10;
                    else
                        alpha=0.2;
                }

                if(column==expanderColumn)
                {
                    gtk_widget_style_get(widget, "expander-size", &expanderSize, NULL);
                    levelIndent=gtk_tree_view_get_level_indentation(treeView),
                    depth=path ? (int)gtk_tree_path_get_depth(path) : 0;

                    forceCellStart=TRUE;
                    if(opts.lvLines)
                        drawTreeViewLines(cr, &style->mid[GTK_STATE_ACTIVE], x, y, height, depth, levelIndent, expanderSize, treeView,
                                          path, column);
                }
                else if(column && qtcTreeViewCellIsLeftOfExpanderColumn(treeView, column))
                    forceCellEnd=TRUE;

                if((GTK_STATE_SELECTED==state || alpha<1.0) && column==expanderColumn)
                {
                    int offset=3 + expanderSize * depth + ( 4 + levelIndent)*(depth-1);
                    selX += offset;
                    selW -= offset;
                }

                if(path)
                    gtk_tree_path_free(path);
            }
#endif

            if(GTK_STATE_SELECTED==state || alpha<1.0)
            {
                int round=detail && ROUNDED
                                ? forceCellStart && forceCellEnd
                                    : ROUNDED_ALL
                                    ? forceCellStart || 0!=strstr(detail, "_start")
                                        ? ROUNDED_LEFT
                                        : forceCellEnd || 0!=strstr(detail, "_end")
                                            ? ROUNDED_RIGHT
                                            : 0!=strstr(detail, "_middle")
                                                ? ROUNDED_NONE
                                                : ROUNDED_ALL
                                : ROUNDED_NONE;

                drawSelection(cr, style, state, area, widget, selX, y, selW, height, round, TRUE, alpha, factor);
            }
        }
    }
    else if(detail && 0==strcmp(detail, "checkbutton"))
    {
        if(GTK_STATE_PRELIGHT==state && opts.crHighlight && width>(opts.crSize*2))
        {
            GdkColor col=shadeColor(&style->bg[state], TO_FACTOR(opts.crHighlight));
            drawSelectionGradient(cr, style, state, area, x, y, width, height, ROUNDED_ALL, FALSE, 1.0, &col, TRUE);
        }
    }
    else if(detail && 0==strcmp(detail, "expander"))
    {
        if(GTK_STATE_PRELIGHT==state && opts.expanderHighlight)
        {
            GdkColor col=shadeColor(&style->bg[state], TO_FACTOR(opts.expanderHighlight));
            drawSelectionGradient(cr, style, state, area, x, y, width, height, ROUNDED_ALL, FALSE, 1.0, &col, TRUE);
        }
    }
    else if(DETAIL("tooltip"))
        drawToolTip(cr, widget, area, x, y, width, height);
    else if(DETAIL("icon_view_item"))
        drawSelection(cr, style, state, area, widget, x, y, width, height, ROUNDED_ALL, FALSE, 1.0, 0);
    else if(GTK_STATE_SELECTED!=state && qtcIsCustomBgnd(&opts) && DETAIL("eventbox"))
        drawWindowBgnd(cr, style, NULL, window, widget, x, y, width, height);
    else if(!(GTK_APP_JAVA==qtSettings.app && widget && GTK_IS_LABEL(widget)))
    {
        if(GTK_STATE_PRELIGHT==state && !opts.crHighlight && 0==strcmp(detail, "checkbutton"))
            ;
        else
            parent_class->draw_flat_box(style, window, state, shadow, area, widget, detail, x, y, width, height);

        /* For SWT (e.g. eclipse) apps. For some reason these only seem to allow a ythickness of at max 2 - but
           for etching we need 3. So we fake this by drawing the 3rd lines here...*/

/*
        if(DO_EFFECT && GTK_STATE_INSENSITIVE!=state && DETAIL("entry_bg") &&
           isSwtComboBoxEntry(widget) && gtk_widget_has_focus(widget))
        {
            drawHLine(cr, CAIRO_COL(qtcPalette.highlight[FRAME_DARK_SHADOW]), 1.0, x, y, width);
            drawHLine(cr, CAIRO_COL(qtcPalette.highlight[0]), 1.0, x, y+height-1, width);
        }
*/
    }
    cairo_destroy(cr);
}

static void gtkDrawHandle(GtkStyle *style, GdkWindow *window, GtkStateType state, GtkShadowType shadow, GdkRectangle *area,
                          GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height, GtkOrientation orientation)
{
    QTC_UNUSED(orientation);
    g_return_if_fail(GTK_IS_STYLE(style));
    g_return_if_fail(GDK_IS_WINDOW(window));
    gboolean paf = WIDGET_TYPE_NAME("PanelAppletFrame");
    cairo_t *cr = gdk_cairo_create(window);
    setCairoClipping(cr, area);
    cairo_set_line_width(cr, 1.0);

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %s  ", __FUNCTION__, state, shadow, width, height, detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 10);

    sanitizeSize(window, &width, &height);
    if(qtcIsFlatBgnd(opts.bgndAppearance) || !(widget && drawWindowBgnd(cr, style, area, window, widget, x, y, width, height)))
    {
//         gtk_style_apply_default_background(style, window, widget && !qtcWidgetNoWindow(widget), state,
//                                            area, x, y, width, height);
        if(widget && IMG_NONE!=opts.bgndImage.type)
            drawWindowBgnd(cr, style, area, window, widget, x, y, width, height);
    }

    if(detail && (!strcmp(detail, "paned") || !strcmp(detail+1, "paned")))
        drawSplitter(cr, state, style, area, x, y, width, height);
    /* Note: I'm not sure why the 'widget && GTK_IS_HANDLE_BOX(widget)' is in the following 'if' - its been there for a while.
             But this breaks the toolbar handles for Java Swing apps. I'm leaving it in for non Java apps, as there must've been
             a reason for it.... */
    else if((DETAIL("handlebox") && (GTK_APP_JAVA==qtSettings.app || (widget && GTK_IS_HANDLE_BOX(widget)))) ||
            DETAIL("dockitem") || paf)
    {
        if(widget && GTK_STATE_INSENSITIVE!=state)
            state=gtk_widget_get_state(widget);

        if(paf)  /* The paf here is expected to be on the gnome panel */
            if(height<width)
                y++;
            else
                x++;
        else
            gtkDrawBox(style, window, state, shadow, area, widget, "handlebox", x, y, width, height);

        switch(opts.handles)
        {
            case LINE_1DOT:
                drawDot(cr, x, y, width, height, qtcPalette.background);
                break;
            case LINE_NONE:
                break;
            case LINE_DOTS:
                drawDots(cr, x, y, width, height, height<width, 2, 5, qtcPalette.background, area, 2, 5);
                break;
            case LINE_DASHES:
                if(height>width)
                    drawLines(cr, x+3, y, 3, height, TRUE, (height-8)/2, 0, qtcPalette.background, area, 5, opts.handles);
                else
                    drawLines(cr, x, y+3, width, 3, FALSE, (width-8)/2, 0, qtcPalette.background, area, 5, opts.handles);
                break;
            case LINE_FLAT:
                drawLines(cr, x, y, width, height, height<width, 2, 4, qtcPalette.background, area, 4, opts.handles);
                break;
            default:
                drawLines(cr, x, y, width, height, height<width, 2, 4, qtcPalette.background, area, 3, opts.handles);
        }
    }
    cairo_destroy(cr);
}

static void gtkDrawArrow(GtkStyle *style, GdkWindow *window, GtkStateType state, GtkShadowType shadow, GdkRectangle *area,
                         GtkWidget *widget, const gchar *detail, GtkArrowType arrow_type, gboolean fill, gint x, gint y,
                         gint width, gint height)
{
    QTC_UNUSED(fill);
    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %d %s  ", __FUNCTION__, state, shadow, arrow_type, x, y, width, height,
                                           detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 10);

    if(DETAIL("arrow"))
    {
        gboolean onComboEntry=isOnComboEntry(widget, 0);

        if(isOnComboBox(widget, 0) && !onComboEntry)
        {
            if (GTK_STATE_ACTIVE==state)
                state=GTK_STATE_PRELIGHT;

            {
            GdkColor *arrowColor=MO_ARROW(false, &qtSettings.colors[GTK_STATE_INSENSITIVE==state
                                                                            ? PAL_DISABLED : PAL_ACTIVE]
                                                                       [COLOR_BUTTON_TEXT]);
            //gboolean moz=isMozilla() && widget && gtk_widget_get_parent(widget) && gtk_widget_get_parent(widget)->parent && gtk_widget_get_parent(widget)->parent->parent &&
            //             isFixedWidget(gtk_widget_get_parent(widget)->parent->parent);
            x++;

#if !GTK_CHECK_VERSION(2, 90, 0)
            // NOTE: Dont do this for moz - as looks odd fir widgets in HTML pages - arrow is shifted too much :-(
            if(!DO_EFFECT) // || moz)
                x+=2;
#endif

            if(opts.doubleGtkComboArrow)
            {
                int pad=opts.vArrows ? 0 : 1;
                drawArrow(window, style, arrowColor, area,  GTK_ARROW_UP,
                          x+(width>>1), y+(height>>1)-(LARGE_ARR_HEIGHT-pad), FALSE, TRUE);
                drawArrow(window, style, arrowColor, area,  GTK_ARROW_DOWN,
                          x+(width>>1), y+(height>>1)+(LARGE_ARR_HEIGHT-pad), FALSE, TRUE);
            }
            else
            {
                GtkWidget *parent=NULL;
                if(!opts.gtkComboMenus && !((parent=gtk_widget_get_parent(widget)) && (parent=gtk_widget_get_parent(parent)) && !qtcComboHasFrame(parent)))
                    x+=2;
                drawArrow(window, style, arrowColor, area,  GTK_ARROW_DOWN, x+(width>>1), y+(height>>1), FALSE, TRUE);
            }
            }
        }
        else
        {
            gboolean combo=onComboEntry || isOnCombo(widget, 0);
            int      origState=state;

            if (combo && GTK_STATE_ACTIVE==state)
                state=GTK_STATE_PRELIGHT;

            {
            GdkColor *col=combo || isOnListViewHeader(widget, 0) || isOnButton(widget, 0, 0L)
                                ? &qtSettings.colors[GTK_STATE_INSENSITIVE==state ? PAL_DISABLED : PAL_ACTIVE]
                                                    [COLOR_BUTTON_TEXT]
                                : &style->text[ARROW_STATE(state)];
            if(onComboEntry && GTK_STATE_ACTIVE==origState && opts.unifyCombo)
                x--, y--;
            drawArrow(window, style, MO_ARROW(false, col), area,  arrow_type, x+(width>>1), y+(height>>1), FALSE, TRUE);
            }
        }
    }
    else
    {
        int isSpinButton = DETAIL("spinbutton");
        int isMenuItem = DETAIL("menuitem");
        /* int a_width = LARGE_ARR_WIDTH; */
        /* int a_height = LARGE_ARR_HEIGHT; */
        gboolean sbar=isSbarDetail(detail),
                 smallArrows=isSpinButton && !opts.unifySpin;
        int      stepper=sbar ? getStepper(
#if GTK_CHECK_VERSION(2, 90, 0)
                                            detail
#else
                                            widget, x, y, opts.sliderWidth, opts.sliderWidth
#endif
                                          ) : STEPPER_NONE;

        sanitizeSize(window, &width, &height);

#if 0
        if(isSpinButton)
        {
            /*if(GTK_ARROW_UP==arrow_type)
                y-=1;
            else*/ if(GTK_ARROW_DOWN==arrow_type)
                y+=1;
        }
#endif
        if (isSpinButton)  {
            /* if (GTK_ARROW_UP == arrow_type) */
            /*     y++; */
            /* a_height = SMALL_ARR_HEIGHT; */
            /* a_width = SMALL_ARR_WIDTH; */
        } else if (GTK_ARROW_LEFT == arrow_type ||
                   GTK_ARROW_RIGHT == arrow_type || DETAIL("menuitem")) {
            /* a_width = LARGE_ARR_HEIGHT; */
            /* a_height = LARGE_ARR_WIDTH; */
            if (isMozilla() && opts.vArrows /* && a_height */ &&
                height < LARGE_ARR_WIDTH) {
                smallArrows = true;
            }
        }

        x+=width>>1;
        y+=height>>1;
/*
    CPD 28/02/2008 Commented out as it messes up scrollbar button look

        if(GTK_ARROW_RIGHT==arrow_type && (width-a_width)%2)
            x++;

        if(GTK_ARROW_DOWN==arrow_type && (height-a_height)%2)
            y++;
*/

        if(GTK_STATE_ACTIVE==state && ((sbar && !opts.flatSbarButtons) || (isSpinButton && !opts.unifySpin)))
        {
            x++;
            y++;
        }

        if (sbar) {
            switch (stepper) {
            case STEPPER_B:
                if (opts.flatSbarButtons || !opts.vArrows) {
                    if (GTK_ARROW_RIGHT == arrow_type) {
                        x--;
                    } else {
                        y--;
                    }
                }
                break;
            case STEPPER_C:
                if (opts.flatSbarButtons || !opts.vArrows) {
                    if (GTK_ARROW_LEFT == arrow_type) {
                        x++;
                    } else {
                        y++;
                    }
                }
            default:
                break;
            }
        }

        if(isSpinButton && isFixedWidget(widget) && isFakeGtk())
            x--;

        if(isSpinButton && !DO_EFFECT)
            y+=(GTK_ARROW_UP==arrow_type ? -1 : 1);

        if(opts.unifySpin && isSpinButton && !opts.vArrows && GTK_ARROW_DOWN==arrow_type)
            y--;

        if(GTK_STATE_ACTIVE==state && (sbar  || isSpinButton) && MO_GLOW==opts.coloredMouseOver)
            state=GTK_STATE_PRELIGHT;

        if(isMenuItem && GTK_ARROW_RIGHT==arrow_type && !isFakeGtk())
            x-=2;

        {
        gboolean isMenuItem=IS_MENU_ITEM(widget);
        GdkColor *col=isSpinButton || sbar
                        ? &qtSettings.colors[GTK_STATE_INSENSITIVE==state ? PAL_DISABLED : PAL_ACTIVE][COLOR_BUTTON_TEXT]
                        : &style->text[isMenuItem && GTK_STATE_PRELIGHT==state
                                        ? GTK_STATE_SELECTED : ARROW_STATE(state)];
        if(isMenuItem && GTK_STATE_PRELIGHT!=state && opts.shadePopupMenu)
        {
            if(SHADE_WINDOW_BORDER==opts.shadeMenubars)
                col=&qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW_BORDER_TEXT];
            else if(opts.customMenuTextColor)
                col=&opts.customMenuNormTextColor;
            else if (SHADE_BLEND_SELECTED==opts.shadeMenubars || SHADE_SELECTED==opts.shadeMenubars ||
                        (SHADE_CUSTOM==opts.shadeMenubars && TOO_DARK(qtcPalette.menubar[ORIGINAL_SHADE])))
                col=&style->text[GTK_STATE_SELECTED];
        }

        drawArrow(window, style, MO_ARROW(isMenuItem, col), area, arrow_type, x, y, smallArrows, TRUE);
        }
    }
}

static void
drawBox(GtkStyle *style, GdkWindow *window, GtkStateType state,
        GtkShadowType shadow, GdkRectangle *area, GtkWidget *widget,
        const gchar *detail, gint x, gint y, gint width, gint height,
        gboolean btnDown)
{
    g_return_if_fail(style != NULL);
    gboolean sbar = isSbarDetail(detail);
    gboolean pbar = DETAIL("bar"); //  && GTK_IS_PROGRESS_BAR(widget);
    gboolean qtcSlider = !pbar && DETAIL("qtc-slider");
    gboolean slider = qtcSlider || (!pbar && DETAIL("slider"));
    gboolean hscale = !slider && DETAIL("hscale");
    gboolean vscale = !hscale && DETAIL("vscale");
    gboolean menubar = !vscale && DETAIL("menubar");
    gboolean button = !menubar && DETAIL("button");
    gboolean togglebutton = !button && DETAIL("togglebutton");
    gboolean optionmenu = !togglebutton && DETAIL("optionmenu");
    gboolean stepper = !optionmenu && DETAIL("stepper");
    gboolean vscrollbar = (!optionmenu && detail &&
                           strstr(detail, "vscrollbar") == detail);
    gboolean hscrollbar = (!vscrollbar && detail &&
                           strstr(detail, "hscrollbar") == detail);
    gboolean spinUp = !hscrollbar && DETAIL("spinbutton_up");
    gboolean spinDown = !spinUp && DETAIL("spinbutton_down");
    gboolean menuScroll = (detail &&
                           strstr(detail, "menu_scroll_arrow_") != NULL);
    gboolean rev = (reverseLayout(widget) ||
                    (widget && reverseLayout(gtk_widget_get_parent(widget))));
    gboolean activeWindow = TRUE;
    GdkColor new_cols[TOTAL_SHADES + 1];
    GdkColor *btnColors = qtcPalette.background;
    int bgnd = getFill(state, btnDown);
    int round = getRound(detail, widget, x, y, width, height, rev);
    gboolean lvh = (isListViewHeader(widget) ||
                    isEvolutionListViewHeader(widget, detail));
    gboolean sunken = (btnDown || (GTK_IS_BUTTON(widget) &&
                                   qtcButtonIsDepressed(widget)) ||
                       state == GTK_STATE_ACTIVE || (bgnd == 2 || bgnd == 3));
    GtkWidget *parent = NULL;

    if (button && GTK_IS_TOGGLE_BUTTON(widget)) {
        button = FALSE;
        togglebutton = TRUE;
    }

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %d %s  ", __FUNCTION__, btnDown, state, shadow, x, y, width, height,
                                           detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 10);

    // FIXME, need to update useButtonColor if the logic below changes right now
    if (useButtonColor(detail)) {
        if (slider | hscale | vscale | sbar && state == GTK_STATE_INSENSITIVE) {
            btnColors = qtcPalette.background;
        } else if (QT_CUSTOM_COLOR_BUTTON(style)) {
            qtcShadeColors(&(style->bg[state]), new_cols);
            btnColors = new_cols;
        } else {
            SET_BTN_COLS(slider, hscale | vscale, lvh, state);
        }
    }

    if (menubar && !isFakeGtk() && opts.shadeMenubarOnlyWhenActive) {
        GtkWindow *topLevel = GTK_WINDOW(gtk_widget_get_toplevel(widget));

        if (topLevel && GTK_IS_WINDOW(topLevel)) {
#define SHADE_ACTIVE_MB_HACK_SET "QTC_SHADE_ACTIVE_MB_HACK_SET"
            if (!g_object_get_data(G_OBJECT(topLevel), SHADE_ACTIVE_MB_HACK_SET)) {
                g_object_set_data(G_OBJECT(topLevel), SHADE_ACTIVE_MB_HACK_SET, (gpointer)1);
                g_signal_connect(G_OBJECT(topLevel), "event", G_CALLBACK(windowEvent), widget);
            }
            activeWindow = qtcWindowIsActive(GTK_WIDGET(topLevel));
        }
    }

    if (opts.menubarMouseOver && GTK_IS_MENU_SHELL(widget) && !isFakeGtk())
        qtcMenuShellSetup(widget);

    CAIRO_BEGIN
    if (spinUp || spinDown) {
        if(!opts.unifySpin && (!opts.unifySpinBtns || sunken/* || GTK_STATE_PRELIGHT==state*/)) {
            EWidget      wid=spinUp ? WIDGET_SPIN_UP : WIDGET_SPIN_DOWN;
            GdkRectangle *a=area,
                         b,
                         unified;
            gboolean     ooOrMoz=isFakeGtk();

            if(!a && isFixedWidget(widget) && ooOrMoz)
            {
                b.x=x; b.y=y; b.width=width; b.height=height;
                a=&b;
            }

            if(WIDGET_SPIN_UP==wid)
            {
                if(DO_EFFECT && opts.etchEntry)
                {
                    if(!opts.unifySpinBtns)
                        drawEtch(cr, a, widget, x-2, y, width+2, height*2, FALSE, ROUNDED_RIGHT, WIDGET_SPIN_UP);
                    y++;
                    width--;
                }
                height++;

                if(opts.unifySpinBtns)
                {
                    unified.x=x, unified.y=y, unified.width=width, unified.height=height-(GTK_STATE_PRELIGHT==state ? 2 : 1);
                    height*=2;
                    area=&unified;
                }
                else if(!opts.etchEntry)
                    height++;
            }
            else if (DO_EFFECT && opts.etchEntry)
            {
                GdkRectangle clip;

                clip.x=x-2, clip.y=y, clip.width=width+2, clip.height=height;
                if(!opts.unifySpinBtns)
                    drawEtch(cr, ooOrMoz ? a : &clip, widget, x-2, y-2, width+2, height+2, FALSE,
                             ROUNDED_RIGHT, WIDGET_SPIN_DOWN);
                height--;
                width--;
                if(opts.unifySpinBtns)
                {
                    unified.x=x, unified.y=y+((GTK_STATE_PRELIGHT==state ? 1 : 0)),
                        unified.width=width, unified.height=height-(GTK_STATE_PRELIGHT==state ? 1 : 0);
                    y-=height, height*=2;
                    area=&unified;
                }
            }

            drawBgnd(cr, &btnColors[bgnd], widget, area, x+1, y+1, width-2, height-2);
            drawLightBevel(cr, style, state, area, x, y, width, height-(WIDGET_SPIN_UP==wid && DO_EFFECT ? 1 : 0), &btnColors[bgnd],
                           btnColors, round, wid, BORDER_FLAT, DF_DO_BORDER|(sunken ? DF_SUNKEN : 0), widget);
        }
    }
    else if(DETAIL("spinbutton"))
    {
        if(qtcIsFlatBgnd(opts.bgndAppearance) || !(widget && drawWindowBgnd(cr, style, area, window, widget, x, y, width, height)))
        {
            qtcStyleApplyDefBgnd(widget && gtk_widget_get_has_window(widget),
                                 GTK_STATE_INSENSITIVE==state ? GTK_STATE_INSENSITIVE : GTK_STATE_NORMAL);
            if(widget && IMG_NONE!=opts.bgndImage.type)
                drawWindowBgnd(cr, style, area, window, widget, x, y, width, height);
        }

        if(opts.unifySpin)
        {
            gboolean rev=reverseLayout(widget) || (widget && reverseLayout(gtk_widget_get_parent(widget))),
                     moz=isMozillaWidget(widget);

            if(!rev)
                x-=4;
            width+=4;

#if !GTK_CHECK_VERSION(2, 90, 0)
            if(moz)
#endif
            {
                GdkRectangle a;

                a.x=x+2, a.y=y, a.width=width-2, a.height=height;
                setCairoClipping(cr, &a);
            }
            drawEntryField(cr, style, state, window, widget, area, x, y, width, height, rev ? ROUNDED_LEFT : ROUNDED_RIGHT, WIDGET_SPIN);
#if !GTK_CHECK_VERSION(2, 90, 0)
            if(moz)
#endif
                cairo_restore(cr);
        }
        else if(opts.unifySpinBtns)
        {
            int offset=(DO_EFFECT && opts.etchEntry ? 1 : 0);
            if(offset)
                drawEtch(cr, area, widget, x, y, width, height, FALSE,
                         ROUNDED_RIGHT, WIDGET_SPIN);
#if GTK_CHECK_VERSION(2, 90, 0)
            bgnd=getFill(GTK_STATE_ACTIVE==state ? GTK_STATE_NORMAL : state, FALSE);
#endif
            drawLightBevel(cr, style, state, area, x, y+offset, width-offset, height-(2*offset), &btnColors[bgnd],
                           btnColors, ROUNDED_RIGHT, WIDGET_SPIN, BORDER_FLAT,
                           DF_DO_BORDER|(sunken ? DF_SUNKEN : 0), widget);
            drawFadedLine(cr, x+2, y+(height>>1), width-(offset+4), 1, &btnColors[QTC_STD_BORDER], area, NULL, TRUE, TRUE, TRUE);
        }

    }
    else if(!opts.stdSidebarButtons && (button || togglebutton) && isSideBarBtn(widget))
        drawSidebarButton(cr, state, style, area, x, y, width, height);
    else if(lvh)
    {
        if(opts.highlightScrollViews && widget)
        {
            GtkWidget *parent=gtk_widget_get_parent(widget);

            if(parent && GTK_IS_TREE_VIEW(parent))
                qtcScrolledWindowRegisterChild(parent);
        }

        drawListViewHeader(cr, state, btnColors, bgnd, area, x, y, width, height);
    }
    else if(isPathButton(widget))
    {
        if(GTK_STATE_PRELIGHT==state)
            drawSelection(cr, style, state, area, widget, x, y, width, height, ROUNDED_ALL, FALSE, 1.0, 0);

        if(opts.windowDrag>WM_DRAG_MENU_AND_TOOLBAR)
            qtcWMMoveSetup(widget);

        if(GTK_IS_TOGGLE_BUTTON(widget))
            drawArrow(window, style, &qtcPalette.background[5], area, GTK_ARROW_RIGHT,
                      x+width-((LARGE_ARR_WIDTH>>1)+4), y+((height-(LARGE_ARR_HEIGHT>>1))>>1)+1, FALSE, TRUE);
    }
    else if(detail &&( button || togglebutton || optionmenu || sbar || hscale || vscale || stepper || slider))
    {
        gboolean combo=0==strcmp(detail, "optionmenu") || isOnComboBox(widget, 0),
                 combo_entry=combo && isOnComboEntry(widget, 0),
                 horiz_tbar,
                 tbar_button=isButtonOnToolbar(widget, &horiz_tbar),
                 handle_button=!tbar_button && isButtonOnHandlebox(widget, &horiz_tbar);
        int      xAdjust=0, yAdjust=0, wAdjust=0, hAdjust=0;
        gboolean horiz=(tbar_button || handle_button) && IS_GLASS(opts.appearance) &&
                        IS_GLASS(opts.toolbarAppearance)
                            ? horiz_tbar
                            : (slider && width<height) || vscrollbar || vscale || (stepper && widget && GTK_IS_VSCROLLBAR(widget))
                                ? FALSE
                                : TRUE,
                    defBtn=GTK_STATE_INSENSITIVE!=state && (button || togglebutton) && widget && gtk_widget_has_default(widget);
//        drawBgnd(cr, &btnColors[bgnd], widget, area, x, y, width, height); // CPD removed as it messes up toolbars and firefox3

#if !GTK_CHECK_VERSION(2, 90, 0)
        if(combo && !sunken && isActiveOptionMenu(widget))
        {
            sunken=TRUE;
            bgnd=4;
        }
#endif

        if(tbar_button && TBTN_JOINED==opts.tbarBtns)
        {
            adjustToolbarButtons(widget, &xAdjust, &yAdjust, &wAdjust, &hAdjust, &round, horiz_tbar);
            x+=xAdjust, y+=yAdjust, width+=wAdjust, height+=hAdjust;
        }

        {
            /* Yuck this is a horrible mess!!!!! */
            gboolean glowFocus=widget && gtk_widget_has_focus(widget) && MO_GLOW==opts.coloredMouseOver && FULL_FOCUS;
            EWidget  widgetType=isComboBoxButton(widget)
                                ? WIDGET_COMBO_BUTTON
                                : slider
                                    ? qtcSlider ? WIDGET_SLIDER : WIDGET_SB_SLIDER
                                    : hscale||vscale
                                        ? WIDGET_SLIDER
                                        : combo || optionmenu
                                            ? WIDGET_COMBO
                                            : tbar_button
                                                ? (opts.coloredTbarMo ? WIDGET_TOOLBAR_BUTTON : WIDGET_UNCOLOURED_MO_BUTTON)
                                                : togglebutton
                                                    ? (glowFocus && !sunken ? WIDGET_DEF_BUTTON : WIDGET_TOGGLE_BUTTON)
                                                    : button
                                                        ? defBtn || glowFocus
                                                            ? WIDGET_DEF_BUTTON
                                                            : WIDGET_STD_BUTTON
                                                        : stepper || sbar
                                                            ? WIDGET_SB_BUTTON
                                                            : WIDGET_OTHER;
            int xo=x, yo=y, wo=width, ho=height, stepper=STEPPER_NONE;

            /* Try and guess if this button is a toolbar button... */
            if((WIDGET_STD_BUTTON==widgetType || WIDGET_TOGGLE_BUTTON==widgetType) && isMozillaWidget(widget) &&
                GTK_IS_BUTTON(widget) && DETAIL("button") && ((width>22 && width<56 && height>30) || height>=32 || ((30==width || 45==width) && 30==height)))
                widgetType=opts.coloredTbarMo ? WIDGET_TOOLBAR_BUTTON : WIDGET_UNCOLOURED_MO_BUTTON;

            if(ROUND_MAX==opts.round &&
                ( (WIDGET_TOGGLE_BUTTON==widgetType && height>(opts.crSize+8) && width<(height+10)) ||
                    (GTK_APP_GIMP==qtSettings.app && WIDGET_STD_BUTTON==widgetType && WIDGET_TYPE_NAME("GimpViewableButton")) ||
                    (opts.stdSidebarButtons && WIDGET_STD_BUTTON==widgetType && widget && isSideBarBtn(widget)) ||
                    (WIDGET_STD_BUTTON==widgetType && GTK_APP_OPEN_OFFICE==qtSettings.app && isFixedWidget(widget) &&
                    height>30 && height<40 && width>16 && width<50) ) )
                widgetType=WIDGET_TOOLBAR_BUTTON;

            /* For some reason SWT combo's dont un-prelight when activated! So dont pre-light at all! */
/*
            if(GTK_APP_JAVA_SWT==qtSettings.app && WIDGET_STD_BUTTON==widgetType && GTK_STATE_PRELIGHT==state && WIDGET_COMBO==widgetType)
            {
                state=GTK_STATE_NORMAL;
                bgnd=getFill(state, btnDown);
            }
            else */ if(WIDGET_SB_BUTTON==widgetType && GTK_APP_MOZILLA!=qtSettings.app)
            {
                stepper=getStepper(
#if GTK_CHECK_VERSION(2, 90, 0)
                                    detail
#else
                                    widget, x, y, width, height
#endif
                                    );
                switch(stepper)
                {
                    case STEPPER_B:
                        if(horiz)
                            x--, width++;
                        else
                            y--, height++;
                        break;
                    case STEPPER_C:
                        if(horiz)
                            width++;
                        else
                            height++;
                    default:
                        break;
                }
            }

            if(/*GTK_APP_JAVA_SWT==qtSettings.app && */
                widget && !isFixedWidget(widget) && /* Don't do for Firefox, etc. */
                WIDGET_SB_SLIDER==widgetType && GTK_STATE_INSENSITIVE!=state && GTK_IS_RANGE(widget))
            {
                GtkAllocation alloc = qtcWidgetGetAllocation(widget);
                gboolean horizontal = qtcRangeGetOrientation(widget) != GTK_ORIENTATION_HORIZONTAL;
                int sbarTroughLen = (horizontal ? alloc.height : alloc.width) -
                    ((qtcRangeHasStepperA(widget) ? opts.sliderWidth : 0) +
                     (qtcRangeHasStepperB(widget) ? opts.sliderWidth : 0) +
                     (qtcRangeHasStepperC(widget) ? opts.sliderWidth : 0) +
                     (qtcRangeHasStepperD(widget) ? opts.sliderWidth : 0));
                int sliderLen = horizontal ? height : width;

                if(sbarTroughLen==sliderLen)
                {
                    state=GTK_STATE_INSENSITIVE;
                    btnColors=qtcPalette.background;
                    bgnd=getFill(state, FALSE);
                }
            }
#ifdef INCREASE_SB_SLIDER
            if(slider && widget && GTK_IS_RANGE(widget) && !opts.flatSbarButtons && SCROLLBAR_NONE!=opts.scrollbarType
                /*&& !(GTK_STATE_PRELIGHT==state && MO_GLOW==opts.coloredMouseOver)*/)
            {
                GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(widget));
                gboolean horizontal = GTK_ORIENTATION_HORIZONTAL==qtcRangeGetOrientation(widget),
#if GTK_CHECK_VERSION(2, 90, 0)
                              hasStartStepper = SCROLLBAR_PLATINUM!=opts.scrollbarType,
                              hasEndStepper   = SCROLLBAR_NEXT!=opts.scrollbarType,
#else
                              hasStartStepper = qtcRangeHasStepperA(widget) || qtcRangeHasStepperB(widget),
                              hasEndStepper   = qtcRangeHasStepperC(widget) || qtcRangeHasStepperD(widget),
#endif
                              atEnd      = FALSE;
                double        value      = gtk_adjustment_get_value(adj);

                if(hasStartStepper && value <= gtk_adjustment_get_lower(adj))
                {
                    if (horizontal)
                        x--, width++;
                    else
                        y--, height++;

                    atEnd=TRUE;
                }
                if(hasEndStepper && value >= gtk_adjustment_get_upper(adj) - gtk_adjustment_get_page_size(adj))
                {
                    if (horizontal)
                        width++;
                    else
                        height++;
                    atEnd=TRUE;
                }

                if(!isMozilla() && widget && lastSlider.widget==widget && !atEnd)
                    lastSlider.widget=NULL;
            }
#endif

#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
            if(GTK_APP_OPEN_OFFICE==qtSettings.app && opts.flatSbarButtons && slider &&
                (SCROLLBAR_KDE==opts.scrollbarType || SCROLLBAR_WINDOWS==opts.scrollbarType) &&
                widget && GTK_IS_RANGE(widget) && isFixedWidget(widget))
            {
                if (qtcRangeGetOrientation(widget)!=GTK_ORIENTATION_HORIZONTAL)
                    y++, height--;
                else
                    x+=2, width-=2;
            }
#endif
            if(WIDGET_COMBO==widgetType && !opts.gtkComboMenus && !isMozilla() &&
                ((parent=gtk_widget_get_parent(widget)) && GTK_IS_COMBO_BOX(parent) && !QTC_COMBO_ENTRY(parent)))
            {
                GtkWidget *mapped=NULL;
                gboolean  changedFocus=FALSE,
                          draw=TRUE;
                int       mod=7;

                if(!opts.gtkComboMenus && !qtcComboHasFrame(parent))
                    mod=0, draw=GTK_STATE_ACTIVE==state || GTK_STATE_PRELIGHT==state, qtcComboBoxSetup(NULL, parent);
                else
                {
                    changedFocus=qtcComboBoxIsFocusChanged(widget);
                    mapped=qtcWidgetMapGetWidget(parent, 1);
                    qtcWidgetMapSetup(parent, widget, 0);

                    if(parent && qtcComboBoxIsHovered(parent))
                        state=GTK_STATE_PRELIGHT;
                }

                if(draw)
                    drawLightBevel(cr, style, state, area, x-mod, y, width+mod, height, &btnColors[bgnd], btnColors, round,
                                   WIDGET_TOOLBAR_BUTTON, BORDER_FLAT, (GTK_STATE_ACTIVE==state ? DF_SUNKEN : 0)|DF_DO_BORDER, widget);

                if(mapped)
                {
                    if(changedFocus)
                        gtk_widget_queue_draw(mapped);
                    else
                    {
                        GtkStateType mappedState=gtk_widget_get_state(mapped);
                        if(GTK_STATE_INSENSITIVE==state && GTK_STATE_INSENSITIVE!=mappedState)
                            state=mappedState;
                        if(mappedState!=gtk_widget_get_state(widget) && GTK_STATE_INSENSITIVE!=mappedState && GTK_STATE_INSENSITIVE!=state)
                            gtk_widget_set_state(mapped, state);
                    }
                }
            }
            else if(opts.unifyCombo && WIDGET_COMBO_BUTTON==widgetType)
            {
                GtkWidget    *parent=widget ? gtk_widget_get_parent(widget) : NULL;
                GtkWidget    *entry=parent ? getComboEntry(parent) : NULL;
                GtkStateType entryState=entry ? gtk_widget_get_state(entry) : GTK_STATE_NORMAL;
                gboolean     rev=FALSE,
                             mozToolbar=isMozilla() && parent &&
                                        GTK_IS_TOGGLE_BUTTON(widget) &&
                                        QTC_COMBO_ENTRY(parent) &&
                                        (parent=gtk_widget_get_parent(parent)) && GTK_IS_FIXED(parent) &&
                                        (parent=gtk_widget_get_parent(parent)) && GTK_IS_WINDOW(parent) &&
                                        0==strcmp(gtk_widget_get_name(parent), "MozillaGtkWidget");

                if(!entry && widget && gtk_widget_get_parent(widget))
                    entry=qtcWidgetMapGetWidget(gtk_widget_get_parent(widget), 1);

                if(entry)
                    rev=reverseLayout(entry);

                if(!rev)
                    x-=4;
                width+=4;
                if((mozToolbar && state==GTK_STATE_PRELIGHT) || state==GTK_STATE_ACTIVE)
                    state=GTK_STATE_NORMAL;

                // When we draw the entry, if its highlighted we want to highlight this button as well.
                // Unfortunately, when the entry of a GtkComboBoxEntry draws itself, there is no way to
                // determine the button associated with it. So, we store the mapping here...
                if(!mozToolbar && parent && QTC_COMBO_ENTRY(parent))
                    qtcWidgetMapSetup(parent, widget, 0);
                // If the button is disabled, but the entry field is not - then use entry field's state
                // for the button. This fixes an issue with LinuxDC++ and Gtk 2.18
                if(GTK_STATE_INSENSITIVE==state && entry && GTK_STATE_INSENSITIVE!=entryState)
                    state=entryState;

                drawEntryField(cr, style, state, window, entry, area, x, y, width, height, rev ? ROUNDED_LEFT : ROUNDED_RIGHT, WIDGET_COMBO_BUTTON);

                // Get entry to redraw by setting its state...
                // ...cant do a queue redraw, as then entry does for the button, else we get stuck in a loop!
                if(!mozToolbar && widget && entry && entryState!=gtk_widget_get_state(widget) && GTK_STATE_INSENSITIVE!=entryState &&
                    GTK_STATE_INSENSITIVE!=state)
                    gtk_widget_set_state(entry, state);
            }
            else if(opts.flatSbarButtons && WIDGET_SB_BUTTON==widgetType)
            {
#if !GTK_CHECK_VERSION(2, 90, 0)
                if(isMozilla())
                {
                    /* This section messes up Gtk3 scrollbars with custom background - and doesnt seem to be required for Gtk2 either. remove at 1.7.1 */
                    /* Re-added in 1.7.2 as needed by Mozilla! */
                    if(opts.gtkScrollViews && qtcIsFlat(opts.sbarBgndAppearance) && 0!=opts.tabBgnd && widget && gtk_widget_get_parent(widget) && gtk_widget_get_parent(widget)->parent &&
                       GTK_IS_SCROLLED_WINDOW(gtk_widget_get_parent(widget)) && GTK_IS_NOTEBOOK(gtk_widget_get_parent(widget)->parent))
                        drawAreaModColor(cr, area, &qtcPalette.background[ORIGINAL_SHADE], TO_FACTOR(opts.tabBgnd), xo, yo, wo, ho);
                    else if(qtcIsFlatBgnd(opts.bgndAppearance) || !(opts.gtkScrollViews && qtcIsFlat(opts.sbarBgndAppearance) &&
                                                              widget && drawWindowBgnd(cr, style, area, window, widget, xo, yo, wo, ho)))
                    {
                        if(!qtcIsFlat(opts.sbarBgndAppearance) && SCROLLBAR_NONE!=opts.scrollbarType)
                            drawBevelGradient(cr, area, xo, yo, wo, ho, &qtcPalette.background[ORIGINAL_SHADE],
                                              horiz, FALSE, opts.sbarBgndAppearance, WIDGET_SB_BGND);
                         else
                             drawBgnd(cr, &qtcPalette.background[ORIGINAL_SHADE], widget, area, xo, yo, wo, ho);
                    }
                }
#endif
            }
            else
            {
                GdkColor *cols=defBtn && (IND_TINT==opts.defBtnIndicator || IND_COLORED==opts.defBtnIndicator || IND_SELECTED==opts.defBtnIndicator)
                                ? qtcPalette.defbtn
                                : WIDGET_COMBO_BUTTON==widgetType && qtcPalette.combobtn && GTK_STATE_INSENSITIVE!=state
                                    ? qtcPalette.combobtn
                                    : btnColors;
                int      bg=(WIDGET_COMBO_BUTTON==widgetType &&
                                (SHADE_DARKEN==opts.comboBtn ||
                                    (SHADE_NONE!=opts.comboBtn && GTK_STATE_INSENSITIVE==state))) ||
                            (WIDGET_SB_SLIDER==widgetType && SHADE_DARKEN==opts.shadeSliders) ||
                            (defBtn && IND_DARKEN==opts.defBtnIndicator)
                                ? getFillReal(state, btnDown, true) : bgnd;

                drawLightBevel(cr, style, state, area, x, y, width, height, &cols[bg], cols, round, widgetType,
                                BORDER_FLAT, (sunken ? DF_SUNKEN : 0)|DF_DO_BORDER|(horiz ? 0 : DF_VERT), widget);

                if(tbar_button && TBTN_JOINED==opts.tbarBtns)
                {
                    const int constSpace=4;
                    int xo=x-xAdjust, yo=y-yAdjust, wo=width-wAdjust, ho=height-hAdjust;

                    if(xAdjust)
                        drawFadedLine(cr, xo, yo+constSpace, 1, ho-(2*constSpace), &btnColors[0], area, NULL, TRUE, TRUE, FALSE);
                    if(yAdjust)
                        drawFadedLine(cr, xo+constSpace, yo, wo-(2*constSpace), 1, &btnColors[0], area, NULL, TRUE, TRUE, TRUE);
                    if(wAdjust && ROUNDED_RIGHT!=round)
                        drawFadedLine(cr, xo+wo-1, yo+constSpace, 1, ho-(2*constSpace), &btnColors[QTC_STD_BORDER], area, NULL, TRUE, TRUE, FALSE);
                    if(hAdjust && ROUNDED_BOTTOM!=round)
                        drawFadedLine(cr, xo+constSpace, yo+ho-1, wo-(2*constSpace), 1, &btnColors[QTC_STD_BORDER], area, NULL, TRUE, TRUE, TRUE);
                }
            }

#ifdef INCREASE_SB_SLIDER
            /* Gtk draws slider first, and then the buttons. But if we have a shaded slider, and extend this so that it
                overlaps (by 1 pixel) the buttons, then the top/bottom is vut off if this is shaded...
                So, work-around this by re-drawing the slider here! */
            if(!opts.flatSbarButtons && SHADE_NONE!=opts.shadeSliders && SCROLLBAR_NONE!=opts.scrollbarType &&
                WIDGET_SB_BUTTON==widgetType && widget && widget==lastSlider.widget && !isMozilla() &&
                ( (SCROLLBAR_NEXT==opts.scrollbarType && STEPPER_B==stepper) || STEPPER_D==stepper))
            {
#if GTK_CHECK_VERSION(2, 90, 0)
                gtkDrawSlider(lastSlider.style, lastSlider.cr, lastSlider.state, lastSlider.shadow, lastSlider.widget,
                                lastSlider.detail, lastSlider.x, lastSlider.y, lastSlider.width, lastSlider.height, lastSlider.orientation);
#else
                gtkDrawSlider(lastSlider.style, lastSlider.window, lastSlider.state, lastSlider.shadow, NULL, lastSlider.widget,
                                lastSlider.detail, lastSlider.x, lastSlider.y, lastSlider.width, lastSlider.height, lastSlider.orientation);
#endif
                lastSlider.widget=NULL;
            }
#endif
        }

        if(defBtn)
            drawDefBtnIndicator(cr, state, btnColors, bgnd, sunken, area, x, y, width, height);

        if(opts.comboSplitter || SHADE_NONE!=opts.comboBtn)
        {
            if(optionmenu)
            {
                GtkRequisition indicator_size;
                GtkBorder      indicator_spacing;
                int            cx=x, cy=y, cheight=height, cwidth=width,
                               ind_width=0,
                               darkLine=BORDER_VAL(GTK_STATE_INSENSITIVE!=state);

                optionMenuGetProps(widget, &indicator_size, &indicator_spacing);

                ind_width=indicator_size.width+indicator_spacing.left+indicator_spacing.right;

                if(DO_EFFECT)
                    cx--;

        #if (GTK_MAJOR_VERSION>1) && (GTK_MINOR_VERSION<2)
                cy++;
                cheight-=2;
        #endif
                cy+=3;
                cheight-=6;

                if(SHADE_NONE!=opts.comboBtn)
                {
                    GdkRectangle btn;
                    GdkColor     *cols=qtcPalette.combobtn && GTK_STATE_INSENSITIVE!=state ? qtcPalette.combobtn : btnColors;
                    int          bg=SHADE_DARKEN==opts.comboBtn || (GTK_STATE_INSENSITIVE==state && SHADE_NONE!=opts.comboBtn)
                                        ? getFillReal(state, btnDown, true) : bgnd;

                    btn.x=cx + (rev ? ind_width + style->xthickness
                                    : (cwidth - ind_width - style->xthickness)+1),
                    btn.y=y, btn.width=ind_width+3, btn.height=height;

                    if(!opts.comboSplitter)
                        setCairoClipping(cr, &btn);
                    if(rev)
                        btn.width+=3;
                    else
                    {
                        btn.x-=3;
                        if(DO_EFFECT)
                            btn.width+=3;
                        else
                            btn.width+=1;
                    }
                    drawLightBevel(cr, style, state, area, btn.x, btn.y, btn.width, btn.height,
                                   &cols[bg], cols, rev ? ROUNDED_LEFT : ROUNDED_RIGHT, WIDGET_COMBO,
                                   BORDER_FLAT, (sunken ? DF_SUNKEN : 0)|DF_DO_BORDER|DF_HIDE_EFFECT, widget);
                    if(!opts.comboSplitter)
                        cairo_restore(cr);
                }
                else if(opts.comboSplitter)
                {
                    if(sunken)
                        cx++, cy++, cheight--;

                    drawFadedLine(cr, cx + (rev ? ind_width+style->xthickness : (cwidth - ind_width - style->xthickness)),
                                      cy + style->ythickness-1, 1, cheight-3,
                                  &btnColors[darkLine], area, NULL, TRUE, TRUE, FALSE);

                    if(!sunken)
                        drawFadedLine(cr, cx + 1 + (rev ? ind_width+style->xthickness : (cwidth - ind_width - style->xthickness)),
                                          cy + style->ythickness-1, 1, cheight-3,
                                      &btnColors[0], area, NULL, TRUE, TRUE, FALSE);
                }
            } else if((button || togglebutton) && (combo || combo_entry)) {
                int vx = x + (width - (1 + (combo_entry ? 24 : 20)));
                /* int vwidth = width - (vx - x); */
                int darkLine = BORDER_VAL(GTK_STATE_INSENSITIVE != state);

                if (rev) {
                    vx = x + LARGE_ARR_WIDTH;
                    if (combo_entry) {
                        vx += 2;
                    }
                }

                if(DO_EFFECT)
                    vx-=2;

                if(!combo_entry)
                {
                    if(SHADE_NONE!=opts.comboBtn)
                    {
                        GdkRectangle btn;
                        GdkColor     *cols=qtcPalette.combobtn && GTK_STATE_INSENSITIVE!=state
                                        ? qtcPalette.combobtn : btnColors;
                        int          bg=SHADE_DARKEN==opts.comboBtn ||
                                            (GTK_STATE_INSENSITIVE==state && SHADE_NONE!=opts.comboBtn)
                                        ? getFillReal(state, btnDown, true) : bgnd;

                        btn.x=vx+(rev ? LARGE_ARR_WIDTH+4 : 0),
                        btn.y=y, btn.width=20+3, btn.height=height;

                        if(!opts.comboSplitter)
                            setCairoClipping(cr, &btn);
                        if(rev)
                            btn.width+=3;
                        else
                        {
                            btn.x-=3;
                            if(DO_EFFECT)
                                btn.width+=3;
                        }
                        drawLightBevel(cr, style, state, area, btn.x, btn.y, btn.width, btn.height,
                                       &cols[bg], cols, rev ? ROUNDED_LEFT : ROUNDED_RIGHT, WIDGET_COMBO,
                                       BORDER_FLAT, (sunken ? DF_SUNKEN : 0)|DF_DO_BORDER|DF_HIDE_EFFECT, widget);
                        if(!opts.comboSplitter)
                            cairo_restore(cr);
                    }
                    else if(opts.comboSplitter)
                    {
                        drawFadedLine(cr, vx+(rev ? LARGE_ARR_WIDTH+4 : 0), y+4, 1, height-8,
                                      &btnColors[darkLine], area, NULL, TRUE, TRUE, FALSE);

                        if(!sunken)
                            drawFadedLine(cr, vx+1+(rev ? LARGE_ARR_WIDTH+4 : 0), y+4, 1, height-8,
                                          &btnColors[0], area, NULL, TRUE, TRUE, FALSE);
                    }
                }
            }
        }
    }
    else if(detail && (0==strcmp(detail, "buttondefault") || 0==strcmp(detail, "togglebuttondefault")))
    {
    }
    else if(widget && detail && (0==strcmp(detail, "trough") || detail==strstr(detail, "trough-")))
    {
        gboolean list=isList(widget),
                 pbar=list || GTK_IS_PROGRESS_BAR(widget),
                 scale=!pbar && GTK_IS_SCALE(widget);
        /* int border = BORDER_VAL(GTK_STATE_INSENSITIVE != state || !scale); */
        gboolean horiz=GTK_IS_RANGE(widget)
                        ? GTK_ORIENTATION_HORIZONTAL==qtcRangeGetOrientation(widget)
                        : width>height;

        if(scale)
            drawSliderGroove(cr, style, state, window, widget, detail, area, x, y, width, height, horiz);
        else if(pbar)
            drawProgressGroove(cr, style, state, window, widget, area, x, y, width, height, list, horiz);
        else
            drawScrollbarGroove(cr, style, state, window, widget, detail, area, x, y, width, height, horiz);
    }
    else if(DETAIL("entry-progress"))
    {
        int adjust=(opts.fillProgress ? 4 : 3)-(opts.etchEntry ? 1 : 0);
        drawProgress(cr, style, state, widget, area, x-adjust, y-adjust, width+adjust, height+(2*adjust), rev, TRUE);
    }
    else if(detail && (0==strcmp(detail,"dockitem") || 0==strcmp(detail,"dockitem_bin")))
    {
        if(qtcIsCustomBgnd(&opts) && widget)
            drawWindowBgnd(cr, style, area, window, widget, x, y, width, height);
    }
    else if(widget && ( (detail && ( menubar || 0==strcmp(detail, "toolbar") || 0==strcmp(detail, "handlebox") ||
                                     0==strcmp(detail,"handlebox_bin") ) )
                        || WIDGET_TYPE_NAME("PanelAppletFrame")))
    {
        //if(GTK_SHADOW_NONE!=shadow)
        {
            GdkColor    *col=menubar && (GTK_STATE_INSENSITIVE!=state || SHADE_NONE!=opts.shadeMenubars)
                                ? &menuColors(activeWindow)[ORIGINAL_SHADE]
                                : &style->bg[state];
            EAppearance app=menubar ? opts.menubarAppearance : opts.toolbarAppearance;
            int         menuBarAdjust=0,
                        opacity=getOpacity(widget);
            double      alpha=opacity!=100 ? (opacity/100.00) : 1.0;
            gboolean    drawGradient=GTK_SHADOW_NONE!=shadow && !qtcIsFlat(app),
                        fillBackground=menubar && SHADE_NONE!=opts.shadeMenubars;

            if ((menubar && opts.windowDrag) ||
                opts.windowDrag > WM_DRAG_MENUBAR)
                qtcWMMoveSetup(widget);

            if(menubar && BLEND_TITLEBAR)
            {
                menuBarAdjust=qtcGetWindowBorderSize(false).titleHeight;
                if(widget && qtcMenuEmitSize(widget, height) && (opts.menubarHiding || opts.windowBorder&WINDOW_BORDER_USE_MENUBAR_COLOR_FOR_TITLEBAR))
                    qtcWindowMenuBarDBus(widget, height);
            }

            if (widget && (opacity!=100 || qtcIsCustomBgnd(&opts)))
                drawWindowBgnd(cr, style, area, window, widget, x, y,
                               width, height);

            if (drawGradient) {
                drawBevelGradient(cr, area, x, y - menuBarAdjust, width,
                                  height + menuBarAdjust, col,
                                  (menubar ? TRUE : DETAIL("handlebox") ?
                                   width < height : width > height),
                                  FALSE, MODIFY_AGUA(app), WIDGET_OTHER, alpha);
            } else if (fillBackground) {
                drawAreaColor(cr, area, col, x, y, width, height, alpha);
            }

            if(GTK_SHADOW_NONE!=shadow && TB_NONE!=opts.toolbarBorders)
                drawToolbarBorders(cr, state, x, y, width, height, menubar && activeWindow, detail);
        }
    }
    else if(widget && pbar)
        drawProgress(cr, style, state, widget, area, x, y, width, height, rev, FALSE);
    else if(DETAIL("menuitem"))
        drawMenuItem(cr, state, style, widget, area, x, y, width, height);
    else if(DETAIL("menu"))
        drawMenu(cr, state, style, widget, area, x, y, width, height);
    else if(detail &&(!strcmp(detail, "paned") || !strcmp(detail+1, "paned")))
    {
        GtkOrientation orientation = GTK_ORIENTATION_HORIZONTAL;

        if(*detail == 'h')
            orientation = GTK_ORIENTATION_VERTICAL;

        gtkDrawHandle(style, window, state, shadow, area, widget, detail, x, y, width, height, orientation);
    }
    else if(detail && 0==strcmp(detail+1, "ruler"))
    {
        drawBevelGradient(cr, area, x, y, width, height, &qtcPalette.background[ORIGINAL_SHADE], 'h'==detail[0], FALSE, opts.lvAppearance, WIDGET_LISTVIEW_HEADER);

//        if(qtcIsFlatBgnd(opts.bgndAppearance) || !widget || !drawWindowBgnd(cr, style, area, widget, x, y, width, height))
//        {
//             drawAreaColor(cr, area, &style->bg[state], x, y, width, height);
//             if(widget && IMG_NONE!=opts.bgndImage.type)
//                 drawWindowBgnd(cr, style, area, widget, x, y, width, height);
//        }
    }
    else if(DETAIL("hseparator"))
    {
        gboolean isMenuItem=widget && GTK_IS_MENU_ITEM(widget);
        GdkColor *cols=qtcPalette.background;
        int      offset=opts.menuStripe && (isMozilla() || isMenuItem) ? 20 : 0;

        if(offset && isFakeGtk())
            offset+=2;

        if (isMenuItem && (opts.lighterPopupMenuBgnd || opts.shadePopupMenu)) {
            cols = qtcPalette.menu;
        }

        drawFadedLine(cr, x+1+offset, y+(height>>1), width-(1+offset), 1, &cols[isMenuItem ? MENU_SEP_SHADE : QTC_STD_BORDER], area, NULL,
                      TRUE, TRUE, TRUE);
    }
    else if(DETAIL("vseparator"))
        drawFadedLine(cr, x+(width>>1), y, 1, height, &qtcPalette.background[QTC_STD_BORDER], area, NULL, TRUE, TRUE, FALSE);
    else
    {
        EWidget wt=!detail && GTK_IS_TREE_VIEW(widget) ? WIDGET_PBAR_TROUGH : WIDGET_FRAME;
        clipPath(cr, x+1, y+1, width-2, height-2, WIDGET_OTHER, RADIUS_INTERNAL, round);
        if(qtcIsFlatBgnd(opts.bgndAppearance) || !widget || !drawWindowBgnd(cr, style, area, window, widget, x+1, y+1, width-2, height-2))
        {
            drawAreaColor(cr, area, &style->bg[state], x+1, y+1, width-2, height-2);
            if(widget && IMG_NONE!=opts.bgndImage.type)
                drawWindowBgnd(cr, style, area, window, widget, x, y, width, height);
        }
        cairo_restore(cr);

        if(WIDGET_PBAR_TROUGH==wt)
            drawProgressGroove(cr, style, state, window, widget, area, x, y, width, height, TRUE, TRUE);
        else
            drawBorder(cr, style, state, area, x, y, width, height, NULL,
                       menuScroll || opts.square&SQUARE_FRAME ? ROUNDED_NONE : ROUNDED_ALL, shadowToBorder(shadow), wt, QTC_STD_BORDER);
    }
    CAIRO_END
}

static void gtkDrawBox(GtkStyle *style, GdkWindow *window, GtkStateType state, GtkShadowType shadow, GdkRectangle *area,
                       GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height)
{
    sanitizeSize(window, &width, &height);
    drawBox(style, window, state, shadow, area, widget, detail, x, y, width, height,
            GTK_STATE_ACTIVE==state || (GTK_IS_BUTTON(widget) && qtcButtonIsDepressed(widget)));
}

static void gtkDrawShadow(GtkStyle *style, GdkWindow *window, GtkStateType state, GtkShadowType shadow, GdkRectangle *area, GtkWidget *widget,
                          const gchar *detail, gint x, gint y, gint width, gint height)
{
    sanitizeSize(window, &width, &height);

    CAIRO_BEGIN

    gboolean comboBoxList=isComboBoxList(widget),
             comboList=!comboBoxList && isComboList(widget);

    if(comboBoxList || comboList)
    {
        gboolean square=opts.square&SQUARE_POPUP_MENUS;

        if((!square || opts.popupBorder) && (!comboList || !DETAIL("viewport")))
        {
            gboolean nonGtk=square || isFakeGtk(),
                     composActive=!nonGtk && compositingActive(widget),
                     isAlphaWidget=!nonGtk && composActive && isRgbaWidget(widget),
                     useAlpha=!nonGtk && qtSettings.useAlpha && isAlphaWidget;

            if(/*(composActive && !useAlpha) || */(opts.popupBorder && square))
            {
                drawAreaColor(cr, area, &style->base[state], x, y, width, height);
                cairo_new_path(cr);
                cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
                cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.background[QTC_STD_BORDER]));
                cairo_stroke(cr);
            }
            else // if(!opts.popupBorder || !(opts.square&SQUARE_POPUP_MENUS))
            {
                if(useAlpha)
                {
                    cairo_rectangle(cr, x, y, width, height);
                    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
                    cairo_set_source_rgba(cr, 0, 0, 0, 1);
                    cairo_fill(cr);
                    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
                    clearRoundedMask(widget, FALSE);
                }
                else
                    createRoundedMask(cr, widget, x, y, width, height, MENU_AND_TOOLTIP_RADIUS, FALSE);

                clipPathRadius(cr, x, y, width, height, MENU_AND_TOOLTIP_RADIUS, ROUNDED_ALL);
                drawAreaColor(cr, area, &style->base[state], x, y, width, height);
                if(useAlpha)
                    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
                if(opts.popupBorder)
                {
                    createPath(cr, x+0.5, y+0.5, width-1, height-1, MENU_AND_TOOLTIP_RADIUS-1, ROUNDED_ALL);
                    cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.background[QTC_STD_BORDER]));
                    cairo_stroke(cr);
                }
            }
        }
        else
            drawAreaColor(cr, area, &style->base[state], x, y, width, height);
    }
    else if(!opts.gtkComboMenus && !isMozilla() && isComboFrame(widget))
    {
        GdkColor newColors[TOTAL_SHADES+1],
                *btnColors;
        int      bgnd=getFill(state, FALSE); // TODO!!! btnDown???
        gboolean sunken=//btnDown || (GTK_IS_BUTTON(widget) && qtcButtonIsDepressed(widget)) ||
                    GTK_STATE_ACTIVE==state || (2==bgnd || 3==bgnd);
        GtkWidget *parent=gtk_widget_get_parent(widget),
                  *mapped=parent ? qtcWidgetMapGetWidget(parent, 0) : NULL;

        if(parent && qtcComboBoxIsHovered(parent))
            state=GTK_STATE_PRELIGHT;

        if(QT_CUSTOM_COLOR_BUTTON(style))
        {
            qtcShadeColors(&(style->bg[state]), newColors);
            btnColors=newColors;
        }
        else
            btnColors=qtcPalette.button[GTK_STATE_INSENSITIVE==state ? PAL_DISABLED : PAL_ACTIVE];

        drawLightBevel(cr, style, state, area, x, y, width+4, height, &btnColors[bgnd], btnColors, ROUNDED_LEFT, WIDGET_TOOLBAR_BUTTON,
                       BORDER_FLAT, (sunken ? DF_SUNKEN : 0)|DF_DO_BORDER|(qtcComboBoxHasFocus(widget, mapped) ? DF_HAS_FOCUS : 0), widget);

        if(GTK_STATE_PRELIGHT!=state)
        {
            if(mapped && GTK_STATE_PRELIGHT==gtk_widget_get_state(mapped))
                state=GTK_STATE_PRELIGHT, gtk_widget_set_state(widget, GTK_STATE_PRELIGHT);
        }
        if(mapped && GTK_STATE_INSENSITIVE!=gtk_widget_get_state(widget))
            gtk_widget_queue_draw(mapped);

        qtcWidgetMapSetup(parent, widget, 1);
        qtcComboBoxSetup(widget, parent);
    }
    else if(DETAIL("entry") || DETAIL("text"))
    {
        GtkWidget *parent=widget ? gtk_widget_get_parent(widget) : NULL;
        if(parent && isList(parent))
        {
            // Dont draw shadow for entries in listviews...
            // Fixes RealPlayer's in-line editing of its favourites.
        }
        else
        {
            gboolean     combo=isComboBoxEntry(widget),
                         isSpin=!combo && isSpinButton(widget),
                         rev=reverseLayout(widget) || (combo && parent && reverseLayout(parent));
            GtkWidget    *btn=NULL;
            /* GtkStateType savedState = state; */

#if GTK_CHECK_VERSION(2, 16, 0)
#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
            if(isSpin && widget && width==qtcWidgetGetAllocation(widget).width)
            {
                int btnWidth, dummy;
                gdk_drawable_get_size(GTK_SPIN_BUTTON(widget)->panel, &btnWidth, &dummy);
                width-=btnWidth;
                if(rev)
                    x+=btnWidth;
            }
#endif
#endif
            if((opts.unifySpin && isSpin) || (combo && opts.unifyCombo))
                width+=(combo ? 4 : 2);

            // If we're a combo entry, and not prelight, check to see if the button is
            // prelighted, if so so are we!
            if(GTK_STATE_PRELIGHT!=state && combo && opts.unifyCombo && parent)
            {
                btn=getComboButton(parent);
                if(!btn && parent)
                    btn=qtcWidgetMapGetWidget(parent, 0);
                if(btn && GTK_STATE_PRELIGHT==gtk_widget_get_state(btn))
                    state=GTK_STATE_PRELIGHT, gtk_widget_set_state(widget, GTK_STATE_PRELIGHT);
            }

#if GTK_CHECK_VERSION(2, 90, 0)
            if(!opts.unifySpin && isSpin)
                width-=16;
#endif
            drawEntryField(cr, style, state, window, widget, area, x, y, width, height,
                        combo || isSpin
                            ? rev
                                    ? ROUNDED_RIGHT
                                    : ROUNDED_LEFT
                            : ROUNDED_ALL,
                        WIDGET_ENTRY);
            if(combo && opts.unifyCombo && parent)
            {
                if(btn && GTK_STATE_INSENSITIVE!=gtk_widget_get_state(widget))
                    gtk_widget_queue_draw(btn);

                if(QTC_COMBO_ENTRY(parent))
                    qtcWidgetMapSetup(parent, widget, 1);
            }
        }
    }
    else
    {
        gboolean frame=!detail || 0==strcmp(detail, "frame"),
                 scrolledWindow=DETAIL("scrolled_window"),
                 viewport=!scrolledWindow && detail && NULL!=strstr(detail, "viewport"),
                 drawSquare=(frame && opts.square&SQUARE_FRAME) || (!viewport && !scrolledWindow && !detail && !widget),
                 statusBar=isFakeGtk()
                            ? frame : isStatusBarFrame(widget),
                 checkScrollViewState=opts.highlightScrollViews && widget && GTK_IS_SCROLLED_WINDOW(widget),
                 isHovered=FALSE,
                 hasFocus=FALSE;
        GdkColor *cols=NULL;

        if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %s  ", __FUNCTION__, state, shadow, x, y, width, height,
                                               detail ? detail : "NULL"),
                                        debugDisplayWidget(widget, 10);


        if(scrolledWindow && GTK_SHADOW_IN!=shadow && widget && GTK_IS_SCROLLED_WINDOW(widget) &&
           GTK_IS_TREE_VIEW(gtk_bin_get_child(GTK_BIN(widget))))
            gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget), GTK_SHADOW_IN), shadow=GTK_SHADOW_IN;
        else if(frame && !statusBar && GTK_IS_FRAME(widget))
        {
            /*
            check for scrolled windows embedded in frames, that contain a treeview.
            if found, change the shadowtypes for consistency with normal -sunken- scrolled windows.
            this should improve rendering of most mandriva drake tools
            */
            GtkWidget *child=gtk_bin_get_child(GTK_BIN(widget));
            if(GTK_IS_SCROLLED_WINDOW(child) && GTK_IS_TREE_VIEW(gtk_bin_get_child(GTK_BIN(child))))
            {
                gtk_frame_set_shadow_type(GTK_FRAME(widget), GTK_SHADOW_NONE);
                shadow=GTK_SHADOW_NONE;

                // also change scrolled window shadow if needed
                GtkScrolledWindow *sw=GTK_SCROLLED_WINDOW(child);
                if(GTK_SHADOW_IN!=gtk_scrolled_window_get_shadow_type(sw))
                    gtk_scrolled_window_set_shadow_type(sw, GTK_SHADOW_IN);
            }
        }

        if(checkScrollViewState)
        {
            qtcScrolledWindowSetup(widget);
            isHovered=qtcScrolledWindowHovered(widget);
            hasFocus=!isHovered && qtcScrolledWindowHasFocus(widget);
        }

        cols=isHovered ? qtcPalette.mouseover
                       : hasFocus
                            ? qtcPalette.focus
                            : qtcPalette.background;
        if(!statusBar && !drawSquare && (frame || scrolledWindow || viewport/* || drawSquare*/)) // && ROUNDED)
        {
            if(GTK_SHADOW_NONE!=shadow &&
               (!frame || opts.drawStatusBarFrames || !isFakeGtk()))
            {
                GtkWidget *parent=widget ? gtk_widget_get_parent(widget) : NULL;
                gboolean  doBorder=!viewport && !drawSquare,
                          windowFrame=parent && !isFixedWidget(widget) && GTK_IS_FRAME(widget) && GTK_IS_WINDOW(parent);

                if(windowFrame)
                {
                    GtkAllocation wAlloc=qtcWidgetGetAllocation(widget),
                                  pAlloc=qtcWidgetGetAllocation(parent);
                    windowFrame=eqRect(&wAlloc, &pAlloc);
                }

//                 if(!drawSquare && widget && gtk_widget_get_parent(widget) && !isFixedWidget(widget) &&
//                    GTK_IS_FRAME(widget) && GTK_IS_WINDOW(gtk_widget_get_parent(widget)))
//                     drawSquare=true;

                if(scrolledWindow)
                {
                    /* See code in qt_settings.c as to isMozill part */
                    if((opts.square&SQUARE_SCROLLVIEW && !opts.highlightScrollViews) || isMozillaWidget(widget))
                    {
                        /* Flat style...
                        drawBorder(cr, style, state, area, x, y, width, height,
                                   NULL, ROUNDED_NONE, BORDER_FLAT, WIDGET_SCROLLVIEW, 0);
                        */
                        /* 3d... */
                        cairo_set_source_rgb(cr, CAIRO_COL(cols[QTC_STD_BORDER]));
                        createTLPath(cr, x+0.5, y+0.5, width-1, height-1, 0.0, ROUNDED_NONE);
                        cairo_stroke(cr);
                        if(!opts.gtkScrollViews)
                            cairo_set_source_rgba(cr, CAIRO_COL(cols[QTC_STD_BORDER]), LOWER_BORDER_ALPHA);
                            //cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.background[STD_BORDER_BR]));
                        createBRPath(cr, x+0.5, y+0.5, width-1, height-1, 0.0, ROUNDED_NONE);
                        cairo_stroke(cr);
                        doBorder=false;
                    }
                    else if(opts.etchEntry)
                    {
                        drawEtch(cr, area, widget, x, y, width, height, FALSE, ROUNDED_ALL, WIDGET_SCROLLVIEW);
                        x++, y++, width-=2, height-=2;
                    }
                }
                if(viewport || windowFrame/* || drawSquare*/)
                {
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
                    if(windowFrame)
                        cairo_set_source_rgb(cr, CAIRO_COL(cols[QTC_STD_BORDER]));
                    else
                        cairo_set_source_rgb(cr, CAIRO_COL(cols[ORIGINAL_SHADE]));
                    cairo_stroke(cr);
                }
                else if(doBorder)
                    drawBorder(cr, style, state, area, x, y, width, height,
                               cols, ROUNDED_ALL, scrolledWindow ? BORDER_SUNKEN : BORDER_FLAT,
                               scrolledWindow ? WIDGET_SCROLLVIEW : WIDGET_FRAME, DF_BLEND);
            }
        }
        else if(!statusBar || opts.drawStatusBarFrames)
        {
            int c1=0,
                c2=0;

            switch(shadow)
            {
                case GTK_SHADOW_NONE:
                    if(statusBar)
                        shadow=GTK_SHADOW_IN;
                    else
                        break;
                case GTK_SHADOW_IN:
                case GTK_SHADOW_ETCHED_IN:
                    c1 = 0;
                    c2 = QTC_STD_BORDER;
                    break;
                case GTK_SHADOW_OUT:
                case GTK_SHADOW_ETCHED_OUT:
                    c1 = QTC_STD_BORDER;
                    c2 = 0;
                    break;
            }

            switch(shadow)
            {
                case GTK_SHADOW_NONE:
                    if(!frame)
                    {
                        cairo_new_path(cr);
                        cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
                        cairo_set_source_rgb(cr, CAIRO_COL(cols[QTC_STD_BORDER]));
                        cairo_stroke(cr);
                    }
                    break;
                case GTK_SHADOW_IN:
                case GTK_SHADOW_OUT:
                    //if(drawSquare || frame || !ROUNDED)
                    {
                        double c2Alpha=GTK_SHADOW_IN==shadow ? 1.0 : LOWER_BORDER_ALPHA,
                               c1Alpha=GTK_SHADOW_OUT==shadow ? 1.0 : LOWER_BORDER_ALPHA;
                        drawHLine(cr, CAIRO_COL(cols[QTC_STD_BORDER]), c2Alpha, x, y, width);
                        drawVLine(cr, CAIRO_COL(cols[QTC_STD_BORDER]), c2Alpha, x, y, height);
                        if(APPEARANCE_FLAT!=opts.appearance)
                        {
                            drawHLine(cr, CAIRO_COL(cols[QTC_STD_BORDER]), c1Alpha, x, y+height-1, width);
                            drawVLine(cr, CAIRO_COL(cols[QTC_STD_BORDER]), c1Alpha, x+width-1, y, height);
                        }
                    }
                    break;
                case GTK_SHADOW_ETCHED_IN:
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+1.5, y+1.5, width-2, height-2);
                    cairo_set_source_rgb(cr, CAIRO_COL(cols[c1]));
                    cairo_stroke(cr);
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+0.5, y+0.5, width-2, height-2);
                    cairo_set_source_rgb(cr, CAIRO_COL(cols[c2]));
                    cairo_stroke(cr);
                    break;
                case GTK_SHADOW_ETCHED_OUT:
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+1.5, y+1.5, width-2, height-2);
                    cairo_set_source_rgb(cr, CAIRO_COL(cols[c2]));
                    cairo_stroke(cr);
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+0.5, y+0.5, width-2, height-2);
                    cairo_set_source_rgb(cr, CAIRO_COL(cols[c1]));
                    cairo_stroke(cr);
                    break;
            }
        }
    }
    CAIRO_END
}

// static gboolean isHoveredCell(GtkWidget *widget, int x, int y, int width, int height)
// {
//     gboolean hovered=FALSE;
//
//     if(widget && GTK_IS_TREE_VIEW(widget))
//     {
//         GtkTreePath       *path=NULL;
//         GtkTreeViewColumn *column=NULL;
//
//         qtcTreeViewSetup(widget);
//         qtcTreeViewGetCell(GTK_TREE_VIEW(widget), &path, &column, x, y, width, height);
//         hovered=path && qtcTreeViewIsCellHovered(widget, path, column);
//     }
//
//     return hovered;
// }

static void gtkDrawCheck(GtkStyle *style, GdkWindow *window, GtkStateType state, GtkShadowType shadow, GdkRectangle *area, GtkWidget *widget,
                         const gchar *detail, gint x, gint y, gint width, gint height)
{
    CAIRO_BEGIN
    drawCheckBox(cr, state, shadow, style, widget, detail, area, x, y, width, height);
    CAIRO_END
}

static void gtkDrawOption(GtkStyle *style, GdkWindow *window, GtkStateType state, GtkShadowType shadow, GdkRectangle *area,
                          GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height)
{
    CAIRO_BEGIN
    drawRadioButton(cr, state, shadow, style, widget, detail, area, x, y, width, height);
    CAIRO_END
}

#define NUM_GCS 5

static void gtkDrawLayout(GtkStyle *style, GdkWindow *window, GtkStateType state, gboolean use_text, GdkRectangle *area, GtkWidget *widget,
                          const gchar *detail, gint x, gint y, PangoLayout *layout)
{
    if(IS_PROGRESS)
        drawLayout(style, window, state, use_text, area, x, y, layout);
    else
    {
        QtCurveStyle *qtcurveStyle=(QtCurveStyle *)style;
        gboolean     isMenuItem=IS_MENU_ITEM(widget);
        GtkMenuBar   *mb=isMenuItem ? isMenubar(widget, 0) : NULL;
#if GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
        gboolean     activeMb=FALSE;
#else
        gboolean     activeMb=mb ? GTK_MENU_SHELL(mb)->active : FALSE;
#endif
        gboolean     selectedText=(opts.useHighlightForMenu || opts.customMenuTextColor) && isMenuItem &&
                                  (opts.colorMenubarMouseOver
                                      ? GTK_STATE_PRELIGHT==state
                                      : ((!mb || activeMb) && GTK_STATE_PRELIGHT==state)),
                     def_but=FALSE,
                     but=isOnButton(widget, 0, &def_but),
                     swapColors=FALSE;
        GdkRectangle area2;
        GtkWidget    *parent=widget ? gtk_widget_get_parent(widget) : NULL;

        if(!opts.colorMenubarMouseOver && mb && !activeMb && GTK_STATE_PRELIGHT==state)
            state=GTK_STATE_NORMAL;

#if GTK_CHECK_VERSION(2, 10, 0) && !GTK_CHECK_VERSION(2, 10, 11)
        GtkNotebook *nb=mb || isMenuItem || !GTK_IS_LABEL(widget) || !parent || !GTK_IS_NOTEBOOK(parent) ? NULL : GTK_NOTEBOOK(parent);
#endif
        GdkColor      prevColors[NUM_GCS];
        int           i=0;

        if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %s %d %d %d %d %d %s  ", __FUNCTION__, pango_layout_get_text(layout), x, y, state, use_text,
                                               IS_MENU_ITEM(widget), detail ? detail : "NULL"),
                                        debugDisplayWidget(widget, 10);

        if(DETAIL("cellrenderertext") && widget && GTK_STATE_INSENSITIVE==gtk_widget_get_state(widget))
             state=GTK_STATE_INSENSITIVE;

#ifndef READ_INACTIVE_PAL /* If we reead the inactive palette, then there is no need for the following... */
        /* The following fixes the text in list views... if not used, when an item is selected it
           gets the selected text color - but when the window changes focus it gets the normal
           text color! */
         if(DETAIL("cellrenderertext") && GTK_STATE_ACTIVE==state)
             state=GTK_STATE_SELECTED;
#endif

        if(!isMenuItem && GTK_STATE_PRELIGHT==state)
            state=GTK_STATE_NORMAL;

#if !GTK_CHECK_VERSION(2, 90, 0)
        if(!use_text && parent && GTK_IS_LABEL(widget) && (isOnOptionMenu(parent, 0) || (GTK_IS_BUTTON(parent) && isOnMenuItem(parent, 0))))
            use_text=TRUE;
#endif

        /*
           This check of 'requisition' size (and not 'allocation') seems to match better
           with Qt4's text positioning. For example, 10pt verdana - no shift is required
           9pt DejaVu Sans requires the shift
        */
        if(but && widget)
        {
            GtkRequisition req=qtcWidgetGetRequisition(widget);

            if(req.height<qtcWidgetGetAllocation(widget).height && req.height%2)
                y++;
        }

        but= but || isOnComboBox(widget, 0);

        if(isOnListViewHeader(widget, 0))
            y--;

        if(but && ((qtSettings.qt4 && GTK_STATE_INSENSITIVE==state) || (!qtSettings.qt4  && GTK_STATE_INSENSITIVE!=state)))
        {
            use_text=TRUE;
            swapColors=TRUE;
            for(i=0; i<NUM_GCS; ++i)
            {
                prevColors[i]=style->text[i];
                style->text[i]=*qtcurveStyle->button_text[GTK_STATE_INSENSITIVE==state ? PAL_DISABLED : PAL_ACTIVE];
            }
            if(state==GTK_STATE_INSENSITIVE)
                state=GTK_STATE_NORMAL;
        }
        else if(isMenuItem)
        {
            gboolean activeWindow=mb && opts.shadeMenubarOnlyWhenActive && widget ? qtcWindowIsActive(gtk_widget_get_toplevel(widget)) : TRUE;

            if((opts.shadePopupMenu && GTK_STATE_PRELIGHT==state) || (mb && (activeWindow || SHADE_WINDOW_BORDER==opts.shadeMenubars)))
            {
                if(SHADE_WINDOW_BORDER==opts.shadeMenubars)
                {
                    for(i=0; i<NUM_GCS; ++i)
                        prevColors[i]=style->text[i];
                    swapColors=TRUE;
                    style->text[GTK_STATE_NORMAL]=*qtcurveStyle->menutext[activeWindow ? 1 : 0];
                    use_text=TRUE;
                }
                else if(opts.customMenuTextColor && qtcurveStyle->menutext[0])
                {
                    for(i=0; i<NUM_GCS; ++i)
                        prevColors[i]=style->text[i];
                    swapColors=TRUE;
                    style->text[GTK_STATE_NORMAL]=*qtcurveStyle->menutext[0];
                    style->text[GTK_STATE_ACTIVE]=*qtcurveStyle->menutext[1];
                    style->text[GTK_STATE_PRELIGHT]=*qtcurveStyle->menutext[0];
                    style->text[GTK_STATE_SELECTED]=*qtcurveStyle->menutext[1];
                    style->text[GTK_STATE_INSENSITIVE]=*qtcurveStyle->menutext[0];
                    use_text=TRUE;
                }
                else if (SHADE_BLEND_SELECTED==opts.shadeMenubars || SHADE_SELECTED==opts.shadeMenubars ||
                         (SHADE_CUSTOM==opts.shadeMenubars && TOO_DARK(qtcPalette.menubar[ORIGINAL_SHADE])))
                    selectedText=TRUE;
            }
        }
#if GTK_CHECK_VERSION(2, 10, 0) && !GTK_CHECK_VERSION(2, 10, 11)
        else if(nb)
        {
            int      numChildren=g_list_length(nb->children),
                     i;
            gboolean active=TRUE;

            for(i=0; i<numChildren; ++i)
            {
                GtkWidget *page=gtk_notebook_get_nth_page(nb, i),
                          *tabLabel=gtk_notebook_get_tab_label(nb, page);

                if(tabLabel==widget)
                {
                    active=GTK_STATE_NORMAL==gtk_widget_get_state(tabLabel);
                    break;
                }
            }

            if(active)
                switch(gtk_notebook_get_tab_pos(nb))
                {
                    case GTK_POS_LEFT:
                        x-=1;
                        break;
                    case GTK_POS_RIGHT:
                        x+=1;
                        break;
                    case GTK_POS_TOP:
                        y-=1;
                        break;
                    case GTK_POS_BOTTOM:
                        y+=1;
                        break;
                }
            else
                switch(gtk_notebook_get_tab_pos(nb))
                {
                    case GTK_POS_LEFT:
                        x+=1;
                        break;
                    case GTK_POS_RIGHT:
                        x-=1;
                        break;
                    case GTK_POS_TOP:
                        y+=1;
                        break;
                    case GTK_POS_BOTTOM:
                        y-=1;
                        break;
                }
        }
#endif

        if(parent && GTK_IS_LABEL(widget) && GTK_IS_FRAME(parent) && !isOnStatusBar(widget, 0))
        {
            int diff=qtcWidgetGetAllocation(widget).x-qtcWidgetGetAllocation(parent).x;

            if (qtcNoFrame(opts.groupBox)) {
                x -= qtcBound(0, diff, 8);
            } else if (opts.gbLabel&GB_LBL_OUTSIDE) {
                x -= qtcBound(0, diff, 4);
            } else if(opts.gbLabel&GB_LBL_INSIDE) {
                x -= qtcBound(0, diff, 2);
            } else {
                x += 5;
            }
#if GTK_CHECK_VERSION(2, 90, 0)
            cairo_reset_clip(cr);
#else
            if(area)
            {
                area2=*area;
                if (qtcNoFrame(opts.groupBox)) {
                    area2.x -= qtcBound(0, diff, 8);
                } else if (opts.gbLabel & GB_LBL_OUTSIDE) {
                    area2.x -= qtcBound(0, diff, 4);
                } else if(opts.gbLabel&GB_LBL_INSIDE) {
                    area2.x -= qtcBound(0, diff, 2);
                } else {
                    area2.x += 5;
                }
                area=&area2;
            }
#endif
        }

        if(!opts.useHighlightForMenu && (isMenuItem || mb) && GTK_STATE_INSENSITIVE!=state)
            state=GTK_STATE_NORMAL;

        drawLayout(style, window, selectedText ? GTK_STATE_SELECTED : state, use_text || selectedText, area, x, y, layout);

        if(opts.embolden && def_but)
            drawLayout(style, window, selectedText ? GTK_STATE_SELECTED : state, use_text || selectedText, area, x+1, y, layout);

        if(swapColors)
            for(i=0; i<5; ++i)
                style->text[i]=prevColors[i];
    }
}

static GdkPixbuf * gtkRenderIcon(GtkStyle *style, const GtkIconSource *source, GtkTextDirection direction,
                                 GtkStateType state, GtkIconSize size, GtkWidget *widget, const char *detail)
{
    return renderIcon(style, source, direction, state, size, widget, detail);
}

static void gtkDrawTab(GtkStyle *style, GdkWindow *window, GtkStateType state, GtkShadowType shadow, GdkRectangle *area,
                       GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height)
{
    /* QtCurveStyle *qtcurveStyle = (QtCurveStyle *)style; */
    GdkColor     *arrowColor=MO_ARROW(false, &qtSettings.colors[GTK_STATE_INSENSITIVE==state
                                                                            ? PAL_DISABLED : PAL_ACTIVE]
                                                                   [COLOR_BUTTON_TEXT]);
    //if(DO_EFFECT)
    //    x--;
    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %s  ", __FUNCTION__, state, shadow, detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 10);

#if !GTK_CHECK_VERSION(2, 90, 0)
    if(isActiveOptionMenu(widget))
        x++, y++;
#endif

    x=reverseLayout(widget) || ((widget=gtk_widget_get_parent(widget)) && reverseLayout(widget))
                ? x+1
                : x+(width>>1);

    if(opts.doubleGtkComboArrow)
    {
        int pad=opts.vArrows ? 0 : 1;
        drawArrow(window, style, arrowColor, area,  GTK_ARROW_UP, x, y+(height>>1)-(LARGE_ARR_HEIGHT-pad), FALSE, TRUE);
        drawArrow(window, style, arrowColor, area,  GTK_ARROW_DOWN, x, y+(height>>1)+(LARGE_ARR_HEIGHT-pad), FALSE, TRUE);
    }
    else
        drawArrow(window, style, arrowColor, area,  GTK_ARROW_DOWN, x, y+(height>>1), FALSE, TRUE);
}

static void
gtkDrawBoxGap(GtkStyle *style, GdkWindow *window, GtkStateType state,
              GtkShadowType shadow, GdkRectangle *area, GtkWidget *widget,
              const gchar *detail, gint x, gint y, gint width, gint height,
              GtkPositionType gapSide, gint gapX, gint gapWidth)
{
    QTC_UNUSED(shadow);
    g_return_if_fail(GTK_IS_STYLE(style));
    g_return_if_fail(window != NULL);
    CAIRO_BEGIN

    if((opts.thin&THIN_FRAMES) && 0==gapX)
        gapX--, gapWidth+=2;

    sanitizeSize(window, &width, &height);
    drawBoxGap(cr, style, GTK_SHADOW_OUT, state, widget, area, x, y,
               width, height, gapSide, gapX, gapWidth, opts.borderTab ? BORDER_LIGHT : BORDER_RAISED, TRUE);

    if(opts.windowDrag>WM_DRAG_MENU_AND_TOOLBAR && DETAIL("notebook"))
        qtcWMMoveSetup(widget);

    if(!isMozilla())
        drawBoxGapFixes(cr, widget, x, y, width, height, gapSide, gapX, gapWidth);

    CAIRO_END
}

static void gtkDrawExtension(GtkStyle *style, GdkWindow *window, GtkStateType state, GtkShadowType shadow, GdkRectangle *area,
                             GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height, GtkPositionType gapSide)
{
    g_return_if_fail(GTK_IS_STYLE(style));
    g_return_if_fail(window != NULL);
    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %d %s  ", __FUNCTION__, state, shadow, gapSide, x, y, width, height,
                                           detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 10);

    sanitizeSize(window, &width, &height);

    if (DETAIL("tab")) {
        CAIRO_BEGIN;
        drawTab(cr, state, style, widget, detail, area, x, y, width, height,
                gapSide);
        CAIRO_END;
    } else {
        parent_class->draw_extension(style, window, state, shadow, area, widget,
                                     detail, x, y, width, height, gapSide);
    }
}

static void
gtkDrawSlider(GtkStyle *style, GdkWindow *window, GtkStateType state,
              GtkShadowType shadow, GdkRectangle *area, GtkWidget *widget,
              const gchar *detail, gint x, gint y, gint width, gint height,
              GtkOrientation orientation)
{
    g_return_if_fail(GTK_IS_STYLE(style));
    g_return_if_fail(window != NULL);
    gboolean scrollbar=DETAIL("slider"),
             scale=DETAIL("hscale") || DETAIL("vscale");

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %s  ", __FUNCTION__, state, shadow, x, y, width, height,
                                           detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 10);

    CAIRO_BEGIN

    sanitizeSize(window, &width, &height);

    if (scrollbar || SLIDER_TRIANGULAR!=opts.sliderStyle) {
        GdkColor newColors[TOTAL_SHADES + 1];
        GdkColor *btnColors = qtcPalette.background;
        int min = MIN_SLIDER_SIZE(opts.sliderThumbs);

#ifdef INCREASE_SB_SLIDER
        if (scrollbar)
            lastSlider.widget = NULL;
#endif
        /* Fix Java swing sliders looking pressed */
        if(!scrollbar && GTK_STATE_ACTIVE==state)
            state=GTK_STATE_PRELIGHT;

        // FIXME, need to update useButtonColor if the logic below changes
        if (useButtonColor(detail)) {
            if(scrollbar|scale && GTK_STATE_INSENSITIVE==state)
                btnColors=qtcPalette.background;
            else if(QT_CUSTOM_COLOR_BUTTON(style))
            {
                qtcShadeColors(&(style->bg[state]), newColors);
                btnColors=newColors;
            }
            else
                SET_BTN_COLS(scrollbar, scale, FALSE, state)
        }

#ifdef INCREASE_SB_SLIDER
        if(scrollbar && !opts.flatSbarButtons && SHADE_NONE!=opts.shadeSliders && SCROLLBAR_NONE!=opts.scrollbarType && !isMozilla())
        {
            lastSlider.style=style;
#if GTK_CHECK_VERSION(2, 90, 0)
            lastSlider.cr=cr;
#else
            lastSlider.window=window;
#endif
            lastSlider.state=state;
            lastSlider.shadow=shadow;
            lastSlider.widget=widget;
            lastSlider.detail=detail;
            lastSlider.x=x;
            lastSlider.y=y;
            lastSlider.width=width;
            lastSlider.height=height;
            lastSlider.orientation=orientation;
        }
#endif

#if GTK_CHECK_VERSION(2, 90, 0)
        if(scrollbar && GTK_STATE_ACTIVE==state)
            state=GTK_STATE_PRELIGHT;
#endif

        drawBox(style, window, state, shadow, area, widget, !scrollbar ? "qtc-slider" : "slider", x, y, width, height, FALSE);

       /* Orientation is always vertical with Mozilla, why? Anyway this hack should be OK - as we only draw
          dashes when slider is larger than 'min' pixels... */
        orientation=width<height ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;
        if(LINE_NONE!=opts.sliderThumbs && (scrollbar || SLIDER_CIRCULAR!=opts.sliderStyle) &&
           (scale || ((GTK_ORIENTATION_HORIZONTAL==orientation && width>=min) || height>=min)))
        {
            GdkColor *markers=/*opts.coloredMouseOver && GTK_STATE_PRELIGHT==state
                                ? qtcPalette.mouseover
                                : */btnColors;
            gboolean horiz=GTK_ORIENTATION_HORIZONTAL==orientation;

            if(LINE_SUNKEN==opts.sliderThumbs)
                if(horiz)
                    y--, height++;
                else
                    x--, width++;
            else
                if(horiz)
                    x++;
                else
                    y++;

            switch(opts.sliderThumbs)
            {
                case LINE_1DOT:
                    drawDot(cr, x, y, width, height, markers);
                    break;
                case LINE_FLAT:
                    drawLines(cr, x, y, width, height, !horiz, 3, 5, markers, area, 5, opts.sliderThumbs);
                    break;
                case LINE_SUNKEN:
                    drawLines(cr, x, y, width, height, !horiz, 4, 3, markers, area, 3, opts.sliderThumbs);
                    break;
                default:
                case LINE_DOTS:
                    drawDots(cr, x, y, width, height, !horiz, scale ? 3 : 5, scale ? 4 : 2, markers, area, 0, 5);
            }
        }
    }
    else
        drawTriangularSlider(cr, style, state, detail, area, x, y, width, height);

    CAIRO_END
}

static void
gtkDrawShadowGap(GtkStyle *style, GdkWindow *window, GtkStateType state,
                 GtkShadowType shadow, GdkRectangle *area, GtkWidget *widget,
                 const gchar *detail, gint x, gint y, gint width, gint height,
                 GtkPositionType gapSide, gint gapX, gint gapWidth)
{
    QTC_UNUSED(detail);
    CAIRO_BEGIN
    sanitizeSize(window, &width, &height);
    drawShadowGap(cr, style, shadow, state, widget, area, x, y, width, height, gapSide, gapX, gapWidth);
    CAIRO_END
}

static void gtkDrawHLine(GtkStyle *style, GdkWindow *window, GtkStateType state, GdkRectangle *area, GtkWidget *widget,
                         const gchar *detail, gint x1, gint x2, gint y)
{
    g_return_if_fail(GTK_IS_STYLE(style));
    g_return_if_fail(window != NULL);
    gboolean tbar=DETAIL("toolbar");
    int      light=0,
             dark=tbar ? (LINE_FLAT==opts.toolbarSeparators ? 4 : 3) : 5;

    CAIRO_BEGIN

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %s  ", __FUNCTION__, state, x1, x2, y, detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 10);

    if(tbar)
    {
        switch(opts.toolbarSeparators)
        {
            default:
            case LINE_DOTS:
                drawDots(cr, x1, y, x2-x1, 2, FALSE, (((x2-x1)/3.0)+0.5), 0,
                         qtcPalette.background, area, 0, 5);
                break;
            case LINE_NONE:
                break;
            case LINE_FLAT:
            case LINE_SUNKEN:
            {
                drawFadedLine(cr, x1<x2 ? x1 : x2, y, abs(x2-x1), 1, &qtcPalette.background[dark], area, NULL, true, true, true);
                //drawHLine(cr, CAIRO_COL(qtcPalette.background[dark]), 1.0, x1<x2 ? x1 : x2, y, abs(x2-x1));
                if(LINE_SUNKEN==opts.toolbarSeparators)
                {
                    cairo_new_path(cr);
                    //drawHLine(cr, CAIRO_COL(qtcPalette.background[light]), 1.0, x1<x2 ? x1 : x2, y+1, abs(x2-x1));
                    drawFadedLine(cr, x1<x2 ? x1 : x2, y+1, abs(x2-x1), 1, &qtcPalette.background[light], area, NULL, true, true, true);
                }
            }
        }
    }
    else if(DETAIL("label"))
    {
        if(state == GTK_STATE_INSENSITIVE)
            //drawHLine(cr, CAIRO_COL(qtcPalette.background[light]), 1.0, (x1<x2 ? x1 : x2)+1, y+1, abs(x2-x1));
            drawFadedLine(cr, x1<x2 ? x1 : x2, y+1, abs(x2-x1), 1, &qtcPalette.background[light], area, NULL, true, true, true);
        //drawHLine(cr, CAIRO_COL(style->text[state]), 1.0, x1<x2 ? x1 : x2, y, abs(x2-x1));
        drawFadedLine(cr, x1<x2 ? x1 : x2, y, abs(x2-x1), 1, &qtcPalette.background[dark], area, NULL, true, true, true);
    }
    else if(DETAIL("menuitem") || (widget && DETAIL("hseparator") && IS_MENU_ITEM(widget)))
    {
        int       offset=opts.menuStripe && (isMozilla() || (widget && GTK_IS_MENU_ITEM(widget))) ? 20 : 0;
        GdkColor *cols=qtcPalette.background;

        if(offset && isFakeGtk())
            offset+=2;

        if (opts.lighterPopupMenuBgnd || opts.shadePopupMenu) {
            cols = qtcPalette.menu;
        }
        if (offset && isFakeGtk()) {
            offset += 2;
        }

        //drawHLine(cr, CAIRO_COL(qtcPalette.background[MENU_SEP_SHADE]), 1.0, x1<x2 ? x1 : x2, y, abs(x2-x1));
        drawFadedLine(cr, offset+(x1<x2 ? x1 : x2), y+1, abs(x2-x1)-offset, 1, &cols[MENU_SEP_SHADE], area, NULL, true, true, true);
    }
    else
        //drawHLine(cr, CAIRO_COL(qtcPalette.background[dark]), 1.0, x1<x2 ? x1 : x2, y, abs(x2-x1));
        drawFadedLine(cr, x1<x2 ? x1 : x2, y, abs(x2-x1), 1, &qtcPalette.background[dark],  area, NULL, true, true, true);

    CAIRO_END
}

static void gtkDrawVLine(GtkStyle *style, GdkWindow *window, GtkStateType state, GdkRectangle *area, GtkWidget *widget,
                         const gchar *detail, gint y1, gint y2, gint x)
{
    g_return_if_fail(GTK_IS_STYLE(style));
    g_return_if_fail(window != NULL);
    CAIRO_BEGIN

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %s  ", __FUNCTION__, state, x, y1, y2, detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 10);

    if(!(DETAIL("vseparator") && isOnComboBox(widget, 0))) /* CPD: Combo handled in drawBox */
    {
        gboolean tbar=DETAIL("toolbar");
        int      dark=tbar ? 3 : 5,
                 light=0;

        if(tbar)
        {
            switch(opts.toolbarSeparators)
            {
                default:
                case LINE_DOTS:
                    drawDots(cr, x, y1, 2, y2-y1, TRUE, (((y2-y1)/3.0)+0.5), 0, qtcPalette.background, area, 0, 5);
                    break;
                case LINE_NONE:
                    break;
                case LINE_FLAT:
                case LINE_SUNKEN:
                {
//                     drawVLine(cr, CAIRO_COL(qtcPalette.background[dark]), 1.0, x, y1<y2 ? y1 : y2, abs(y2-y1));
                    drawFadedLine(cr, x, y1<y2 ? y1 : y2, 1, abs(y2-y1), &qtcPalette.background[dark], area, NULL, true, true, false);
                    if(LINE_SUNKEN==opts.toolbarSeparators)
//                         drawVLine(cr, CAIRO_COL(qtcPalette.background[light]), 1.0, x+1, y1<y2 ? y1 : y2, abs(y2-y1));
                        drawFadedLine(cr, x+1, y1<y2 ? y1 : y2, 1, abs(y2-y1), &qtcPalette.background[light], area, NULL, true, true, false);
                }
            }
        }
        else
//             drawVLine(cr, CAIRO_COL(qtcPalette.background[dark]), 1.0, x, y1<y2 ? y1 : y2, abs(y2-y1));
            drawFadedLine(cr, x, y1<y2 ? y1 : y2, 1, abs(y2-y1), &qtcPalette.background[dark], area, NULL, true, true, false);
    }
    CAIRO_END
}

static void gtkDrawFocus(GtkStyle *style, GdkWindow *window, GtkStateType state, GdkRectangle *area, GtkWidget *widget, const gchar *detail,
                         gint x, gint y, gint width, gint height)
{
    if(GTK_IS_EDITABLE(widget))
        return;

    sanitizeSize(window, &width, &height);

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %s ", __FUNCTION__, state, x, y, width, height, detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 10);

    {
    GtkWidget *parent=widget ? gtk_widget_get_parent(widget) : NULL;
    gboolean  doEtch=DO_EFFECT,
              btn=false,
              comboButton=false,
              rev=parent && reverseLayout(parent),
              view=isList(widget),
              listViewHeader=isListViewHeader(widget),
              dummy,
              toolbarBtn=!listViewHeader && !view && isButtonOnToolbar(widget, &dummy);

    if(opts.comboSplitter && !FULL_FOCUS && isComboBox(widget))
    {
/*
        x++;
        y++;
        height-=2;
*/
        width+=2; /* Remove if re-add the above */

        if(widget && rev)
            x+=20;
        width-=22;

        if(isGimpCombo(widget))
            x+=2, y+=2, width-=4, height-=4;
        btn=true;
    }
#if !GTK_CHECK_VERSION(2, 90, 0)
    else if(GTK_IS_OPTION_MENU(widget))
    {
        if((!opts.comboSplitter || FULL_FOCUS) && widget)
        {
            GtkAllocation alloc=qtcWidgetGetAllocation(widget);

            if(alloc.width>width)
                width=alloc.width-(doEtch ? 8 : 4);
        }

        x++, y++, width-=2, height-=2;
        btn=true;
    }
#endif
    if(isComboBoxEntryButton(widget))
    {
#if GTK_CHECK_VERSION(2, 90, 0)
        if(doEtch)
            x-=2, y++, width+=3, height-=2;
        else
            x-=2, width+=4;
#else
        if(doEtch)
            x++, y+=2, width-=3, height-=4;
        else
            x++, y++, width-=2, height-=2;
#endif
        btn=comboButton=true;
    }
    else if(isGimpCombo(widget))
    {
        if(FOCUS_GLOW==opts.focus)
            return;

        x-=2, width+=4;
        if(!doEtch)
            x--, y--, width+=2, height+=2;
    }
    else if(GTK_IS_BUTTON(widget))
    {
        if(GTK_IS_RADIO_BUTTON(widget) || GTK_IS_CHECK_BUTTON(widget))
        {
            // Gimps buttons in its toolbox are
            const gchar *text=NULL;
            toolbarBtn=GTK_APP_GIMP==qtSettings.app && (NULL==(text=gtk_button_get_label(GTK_BUTTON(widget))) ||
                                                        '\0'==text[0]);

            if(!toolbarBtn && FOCUS_GLOW==opts.focus && !isMozilla())
                return;

            if(toolbarBtn)
            {
                if(GTK_APP_GIMP==qtSettings.app && FOCUS_GLOW==opts.focus && toolbarBtn)
                    x-=2, width+=4, y-=1, height+=2;
            }
#if 0 /* Removed in 1.7.2 */
            else
            {
                if(FOCUS_LINE==opts.focus)
                    height--;
                else if(FULL_FOCUS)
                {
                    y--, x-=3, width+=6, height+=2;
                    if(!doEtch)
                        y--, x--, width+=2, height+=2;
                }
            }
#endif
        }
        else if(FOCUS_GLOW==opts.focus && toolbarBtn)
            x-=2, width+=4, y-=2, height+=4;
        else
        {
            if(doEtch)
                x--, width+=2;
            else
                x-=2, width+=4;
            if(doEtch && (opts.thin&THIN_BUTTONS))
                y++, height-=2;
            btn=true;
        }
    }

    if(GTK_STATE_PRELIGHT==state && FULL_FOCUS && MO_NONE!=opts.coloredMouseOver && !listViewHeader && (btn || comboButton))
        return;

    if(FOCUS_GLOW==opts.focus && !comboButton && !listViewHeader && !toolbarBtn && (btn || GTK_IS_SCALE(widget)))
        return;

    if(FOCUS_GLOW==opts.focus && toolbarBtn && GTK_STATE_NORMAL!=state)
        return;

    if(FOCUS_STANDARD==opts.focus)
        parent_class->draw_focus(style, window, state, area, widget, detail, x, y, width, height);
    else
    {
        gboolean drawRounded=ROUNDED;
        GdkColor *col=view && GTK_STATE_SELECTED==state
                           ? &style->text[state]
                           : &qtcPalette.focus[FOCUS_SHADE(GTK_STATE_SELECTED==state)];

        CAIRO_BEGIN


#if GTK_CHECK_VERSION(2, 12, 0)
        if(GTK_APP_JAVA_SWT==qtSettings.app && view && widget && GTK_IS_TREE_VIEW(widget))
        {
            GtkTreeView       *treeView=GTK_TREE_VIEW(widget);
            GtkTreePath       *path=NULL;
            GtkTreeViewColumn *column=NULL,
                              *expanderColumn=gtk_tree_view_get_expander_column(treeView);

            qtcTreeViewGetCell(treeView, &path, &column, x, y, width, height);
            if(column==expanderColumn)
            {
                int expanderSize=0;
                gtk_widget_style_get(widget, "expander-size", &expanderSize, NULL);
                if(expanderSize>0)
                {
                    int depth=path ? (int)gtk_tree_path_get_depth(path) : 0,
                        offset=3 + expanderSize * depth + ( 4 + gtk_tree_view_get_level_indentation(treeView))*(depth-1);
                    x += offset;
                    width -= offset;
                }
            }

            if(path)
                gtk_tree_path_free(path);
        }
#endif

        if(FOCUS_LINE==opts.focus || FOCUS_GLOW==opts.focus)
        {
            if(view || listViewHeader)
                height-=2;
            drawFadedLine(cr, x, y+height-1, width, 1, col, area, NULL, TRUE, TRUE, TRUE);
        }
        else
        {
            /*double alpha=FOCUS_GLOW==opts.focus ? FOCUS_GLOW_LINE_ALPHA : 1.0;*/

            if(width<3 || height < 3)
                drawRounded=FALSE;

            cairo_new_path(cr);

            if(isListViewHeader(widget))
            {
                btn=false;
                y++, x++, width-=2, height-=3;
            }
            if(FULL_FOCUS)
            {
                if(btn)
                {
                    if(toolbarBtn)
                    {
                        x-=2, y-=2, width+=4, height+=4;
                        if(!doEtch)
                            x-=2, width+=4, y--, height+=2;
                    }
                    else if(!widget  || !(GTK_IS_RADIO_BUTTON(widget) || GTK_IS_CHECK_BUTTON(widget))) /* 1.7.2 - dont asjust fot check/radio */
                        x-=3, y-=3, width+=6, height+=6;
                }

                if(FOCUS_FILLED==opts.focus)
                {
                    if(drawRounded)
                        createPath(cr, x+0.5, y+0.5, width-1, height-1, qtcGetRadius(&opts, width, height, WIDGET_OTHER,
                                   RADIUS_EXTERNAL), comboButton ? (rev ? ROUNDED_LEFT : ROUNDED_RIGHT) : ROUNDED_ALL);
                    else
                        cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
                    cairo_set_source_rgba(cr, CAIRO_COL(*col), FOCUS_ALPHA);
                    cairo_fill(cr);
                    cairo_new_path(cr);
                }
            }
            if(drawRounded)
                createPath(cr, x+0.5, y+0.5, width-1, height-1,
                           (view && opts.square&SQUARE_LISTVIEW_SELECTION) && ROUNDED
                            ? SLIGHT_INNER_RADIUS
                            : qtcGetRadius(&opts, width, height, WIDGET_OTHER,
                                        FULL_FOCUS ? RADIUS_EXTERNAL : RADIUS_SELECTION),
                           FULL_FOCUS && comboButton ? (rev ? ROUNDED_LEFT : ROUNDED_RIGHT) :
                           ROUNDED_ALL);
            else
                cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
            /*cairo_set_source_rgba(cr, CAIRO_COL(*col), alpha);*/
            cairo_set_source_rgb(cr, CAIRO_COL(*col));
            cairo_stroke(cr);
        }
        CAIRO_END
    }
    }
}

static void
gtkDrawResizeGrip(GtkStyle *style, GdkWindow *window, GtkStateType state,
                  GdkRectangle *area, GtkWidget *widget, const gchar *detail,
                  GdkWindowEdge edge, gint x, gint y, gint width, gint height)
{
    g_return_if_fail(GTK_IS_STYLE(style));
    g_return_if_fail(GDK_IS_DRAWABLE(window));
    cairo_t *cr = gdk_cairo_create(window);
    setCairoClipping(cr, area);
    cairo_set_line_width(cr, 1.0);

    int size = SIZE_GRIP_SIZE - 2;

    /* Clear background */
    if (qtcIsFlatBgnd(opts.bgndAppearance) ||
        !(widget && drawWindowBgnd(cr, style, area, window, widget,
                                   x, y, width, height))) {
        /* gtk_style_apply_default_background(style, window, FALSE, state, */
        /*                                    area, x, y, width, height); */
        if (widget && opts.bgndImage.type != IMG_NONE) {
            drawWindowBgnd(cr, style, area, window, widget,
                           x, y, width, height);
        }
    }

    switch (edge) {
    case GDK_WINDOW_EDGE_SOUTH_EAST: {
        // Adjust Firefox's resize grip so that it can be completely covered
        // by QtCurve's KWin resize grip.
        if (isMozilla()) {
            x++;
            y++;
        }
        GdkPoint a[] = {{x + width, y + height - size},
                        {x + width, y + height},
                        {x + width - size,  y + height}};
        drawPolygon(window, style, &qtcPalette.background[2], area, a, 3, TRUE);
        break;
    }
    case GDK_WINDOW_EDGE_SOUTH_WEST: {
        GdkPoint a[]={{x + width - size, y + height - size},
                      {x + width, y + height},
                      {x + width - size, y + height}};
        drawPolygon(window, style, &qtcPalette.background[2], area, a, 3, TRUE);
        break;
    }
    case GDK_WINDOW_EDGE_NORTH_EAST:
        // TODO!!
    case GDK_WINDOW_EDGE_NORTH_WEST:
        // TODO!!
    default:
        parent_class->draw_resize_grip(style, window, state, area, widget,
                                       detail, edge, x, y, width, height);
    }
    cairo_destroy(cr);
}

static void gtkDrawExpander(GtkStyle *style, GdkWindow *window, GtkStateType state, GdkRectangle *area, GtkWidget *widget,
                            const gchar *detail, gint x, gint y, GtkExpanderStyle expander_style)
{
    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %s  ", __FUNCTION__, state, detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 10);

    gboolean isExpander=widget && (GTK_IS_EXPANDER(widget) || GTK_IS_TREE_VIEW(widget)),
             fill=!isExpander || opts.coloredMouseOver || GTK_STATE_PRELIGHT!=state;
    GdkColor *col=isExpander && opts.coloredMouseOver && GTK_STATE_PRELIGHT==state
                    ? &qtcPalette.mouseover[ARROW_MO_SHADE]
                    : &style->text[ARROW_STATE(state)];

    x-=(LV_SIZE/2.0)+0.5;
    x+=2;
    y-=(LV_SIZE/2.0)+0.5;

    if(GTK_EXPANDER_COLLAPSED==expander_style)
        drawArrow(window, style, col, area, reverseLayout(widget) ? GTK_ARROW_LEFT : GTK_ARROW_RIGHT,
                  x+(LARGE_ARR_WIDTH>>1), y+LARGE_ARR_HEIGHT, FALSE, fill);
    else
        drawArrow(window, style, col, area, GTK_ARROW_DOWN, x+(LARGE_ARR_WIDTH>>1), y+LARGE_ARR_HEIGHT, FALSE, fill);
}

static void styleRealize(GtkStyle *style)
{
    QtCurveStyle *qtcurveStyle = (QtCurveStyle *)style;

    parent_class->realize(style);

    qtcurveStyle->button_text[PAL_ACTIVE]=&qtSettings.colors[PAL_ACTIVE][COLOR_BUTTON_TEXT];
    qtcurveStyle->button_text[PAL_DISABLED]=qtSettings.qt4
                            ? &qtSettings.colors[PAL_DISABLED][COLOR_BUTTON_TEXT]
                            : &style->text[GTK_STATE_INSENSITIVE];

    if(SHADE_WINDOW_BORDER==opts.shadeMenubars)
    {
        qtcurveStyle->menutext[0]=&qtSettings.colors[PAL_INACTIVE][COLOR_WINDOW_BORDER_TEXT];
        qtcurveStyle->menutext[1]=&qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW_BORDER_TEXT];
    }
    else if(opts.customMenuTextColor)
    {
        qtcurveStyle->menutext[0]=&opts.customMenuNormTextColor;
        qtcurveStyle->menutext[1]=&opts.customMenuSelTextColor;
    }
    else
        qtcurveStyle->menutext[0]=NULL;

#if !GTK_CHECK_VERSION(2, 90, 0) && !defined QTC_GTK2_USE_CAIRO_FOR_ARROWS
    qtcurveStyle->arrow_gc = NULL;
#endif
}

static void styleUnrealize(GtkStyle *style)
{
    QtCurveStyle *qtcurveStyle = (QtCurveStyle *)style;

    parent_class->unrealize(style);

#if !GTK_CHECK_VERSION(2, 90, 0) && !defined QTC_GTK2_USE_CAIRO_FOR_ARROWS
    if(qtcurveStyle->arrow_gc)
    {
        g_object_unref(qtcurveStyle->arrow_gc);
        qtcurveStyle->arrow_gc=NULL;
    }
#endif
}

static void qtcurve_style_init_from_rc(GtkStyle *style, GtkRcStyle *rc_style)
{
    parent_class->init_from_rc(style, rc_style);
}

void qtcurve_style_class_init(QtCurveStyleClass *klass)
{
    GtkStyleClass *style_class = GTK_STYLE_CLASS(klass);

    parent_class = g_type_class_peek_parent(klass);

    style_class->realize = styleRealize;
    style_class->unrealize = styleUnrealize;
    style_class->init_from_rc = qtcurve_style_init_from_rc;
    style_class->draw_resize_grip = gtkDrawResizeGrip;
    style_class->draw_expander = gtkDrawExpander;
    style_class->draw_arrow = gtkDrawArrow;
    style_class->draw_tab = gtkDrawTab;
    style_class->draw_shadow = gtkDrawShadow;
    style_class->draw_box_gap = gtkDrawBoxGap;
    style_class->draw_extension = gtkDrawExtension;
    style_class->draw_handle = gtkDrawHandle;
    style_class->draw_box = gtkDrawBox;
    style_class->draw_flat_box = gtkDrawFlatBox;
    style_class->draw_check = gtkDrawCheck;
    style_class->draw_slider = gtkDrawSlider;
    style_class->draw_option = gtkDrawOption;
    style_class->draw_shadow_gap = gtkDrawShadowGap;
    style_class->draw_hline = gtkDrawHLine;
    style_class->draw_vline = gtkDrawVLine;
    style_class->draw_focus = gtkDrawFocus;
    style_class->draw_layout = gtkDrawLayout;
    style_class->render_icon = gtkRenderIcon;
}

static GtkRcStyleClass *parent_rc_class;
GType qtcurve_type_rc_style = 0;

static guint
qtcurve_rc_style_parse(GtkRcStyle *rc_style, GtkSettings *settings,
                       GScanner *scanner)
{
    QTC_UNUSED(rc_style);
    QTC_UNUSED(settings);
    static GQuark scope_id = 0;
    guint old_scope,
          token;

    /* Set up a new scope in this scanner. */
    if(!scope_id)
        scope_id = g_quark_from_string("qtcurve_theme_engine");

    /* If we bail out due to errors, we *don't* reset the scope, so the error messaging code can make
       sense of our tokens. */
    old_scope = g_scanner_set_scope(scanner, scope_id);

    token = g_scanner_peek_next_token(scanner);
    while(token != G_TOKEN_RIGHT_CURLY)
    {
        switch(token)
        {
            default:
                g_scanner_get_next_token(scanner);
                token = G_TOKEN_RIGHT_CURLY;
        }

        if(token != G_TOKEN_NONE)
            return token;

        token = g_scanner_peek_next_token(scanner);
    }

    g_scanner_get_next_token(scanner);
    g_scanner_set_scope(scanner, old_scope);

    return G_TOKEN_NONE;
}

static void qtcurve_rc_style_merge(GtkRcStyle *dest, GtkRcStyle *src)
{

    GtkRcStyle  copy;
    const gchar *typeName=src ? g_type_name(G_TYPE_FROM_INSTANCE(src)) : NULL;
    bool        destIsQtc=QTCURVE_IS_RC_STYLE(dest),
                srcIsQtc=!src->name || src->name==strstr(src->name, RC_SETTING) ||
                         (getAppName() && src->name==strstr(src->name, getAppName())),
                isQtCNoteBook=0!=opts.tabBgnd && src->name && 0==strcmp(src->name, "qtcurve-notebook_bg"),
                dontChangeColors=destIsQtc && !srcIsQtc && !isQtCNoteBook &&
                                 // Only allow GtkRcStyle and QtCurveRcStyle to change colours
                                 // ...this should catch most cases whre another themes gtkrc is in the
                                 // GTK2_RC_FILES path
                                ( (typeName && strcmp(typeName, "GtkRcStyle") && strcmp(typeName, "QtCurveRcStyle")) ||
                                 // If run as root (probably via kdesu/kdesudo) then dont allow KDE settings to take
                                 // effect - as these are sometimes from the user's settings, not roots!
                                  (0==getuid() && src && src->name && (0==strcmp(src->name, "ToolTip") ||
                                                                      0==strcmp(src->name, "default"))));

    if (isQtCNoteBook) {
        qtcShade(&qtcPalette.background[ORIGINAL_SHADE],
                 &src->bg[GTK_STATE_NORMAL], TO_FACTOR(opts.tabBgnd),
                 opts.shading);
    }

    if(dontChangeColors)
    {
        memcpy(copy.color_flags, dest->color_flags, sizeof(GtkRcFlags)*5);
        memcpy(copy.fg, dest->fg, sizeof(GdkColor)*5);
        memcpy(copy.bg, dest->bg, sizeof(GdkColor)*5);
        memcpy(copy.text, dest->text, sizeof(GdkColor)*5);
        memcpy(copy.base, dest->base, sizeof(GdkColor)*5);
    }

    parent_rc_class->merge(dest, src);

    if(dontChangeColors)
    {
        memcpy(dest->color_flags, copy.color_flags, sizeof(GtkRcFlags)*5);
        memcpy(dest->fg, copy.fg, sizeof(GdkColor)*5);
        memcpy(dest->bg, copy.bg, sizeof(GdkColor)*5);
        memcpy(dest->text, copy.text, sizeof(GdkColor)*5);
        memcpy(dest->base, copy.base, sizeof(GdkColor)*5);
    }
}

/* Create an empty style suitable to this RC style */
static GtkStyle * qtcurve_rc_style_create_style(GtkRcStyle *rc_style)
{
    GtkStyle *style=g_object_new(QTCURVE_TYPE_STYLE, NULL);

    qtSettingsSetColors(style, rc_style);
    return style;
}

GType qtcurve_type_style = 0;

static void qtcurve_style_init(QtCurveStyle *style)
{
    QTC_UNUSED(style);
    qtcShadowInitialize();
}

void qtcurve_style_register_type(GTypeModule *module)
{
    static const GTypeInfo object_info =
    {
        sizeof(QtCurveStyleClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) qtcurve_style_class_init,
        NULL,            /* class_finalize */
        NULL,            /* class_data */
        sizeof(QtCurveStyle),
        0,                /* n_preallocs */
        (GInstanceInitFunc) qtcurve_style_init,
        NULL
    };

    qtcurve_type_style = g_type_module_register_type(module, GTK_TYPE_STYLE, "QtCurveStyle", &object_info, 0);
}

static void qtcurve_rc_style_init(QtCurveRcStyle *qtcurve_rc)
{
    QTC_UNUSED(qtcurve_rc);
#ifdef INCREASE_SB_SLIDER
    lastSlider.widget = NULL;
#endif
    if (qtSettingsInit()) {
        generateColors();
#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
        if (opts.dlgOpacity < 100 || opts.bgndOpacity < 100 ||
            opts.menuBgndOpacity < 100 || qtSettings.useAlpha) {
            GdkScreen   *screen = gdk_screen_get_default();
            GdkColormap *colormap =
                screen ? gdk_screen_get_rgba_colormap(screen) : NULL;

            if (colormap) {
                gtk_widget_push_colormap(colormap);
                gtk_widget_set_default_colormap(colormap);
            }
        }
#endif
    }
}

static void qtcurve_rc_style_finalize(GObject *object)
{
    qtcAnimationCleanup();
    if (G_OBJECT_CLASS(parent_rc_class)->finalize != NULL)
        G_OBJECT_CLASS(parent_rc_class)->finalize(object);
}

static void qtcurve_rc_style_class_init(QtCurveRcStyleClass *klass)
{
    GtkRcStyleClass *rc_style_class = GTK_RC_STYLE_CLASS(klass);
    GObjectClass    *object_class = G_OBJECT_CLASS(klass);

    parent_rc_class = g_type_class_peek_parent(klass);

    rc_style_class->parse = qtcurve_rc_style_parse;
    rc_style_class->create_style = qtcurve_rc_style_create_style;
    rc_style_class->merge = qtcurve_rc_style_merge;

    object_class->finalize = qtcurve_rc_style_finalize;
}

void qtcurve_rc_style_register_type(GTypeModule *module)
{
    static const GTypeInfo object_info =
    {
        sizeof(QtCurveRcStyleClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) qtcurve_rc_style_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof(QtCurveRcStyle),
        0,              /* n_preallocs */
        (GInstanceInitFunc) qtcurve_rc_style_init,
        NULL
    };

    qtcurve_type_rc_style = g_type_module_register_type(module, GTK_TYPE_RC_STYLE, "QtCurveRcStyle", &object_info, 0);
}

G_MODULE_EXPORT void theme_init(GTypeModule *module)
{
    qtcX11InitXlib(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()));
    qtcurve_rc_style_register_type(module);
    qtcurve_style_register_type(module);
}

G_MODULE_EXPORT void theme_exit()
{
}

G_MODULE_EXPORT GtkRcStyle * theme_create_rc_style()
{
    return GTK_RC_STYLE(g_object_new(QTCURVE_TYPE_RC_STYLE, NULL));
}

/* The following function will be called by GTK+ when the module is loaded and
 * checks to see if we are compatible with the version of GTK+ that loads us.
 */
G_MODULE_EXPORT const gchar * g_module_check_init(GModule *module);

const gchar*
g_module_check_init(GModule *module)
{
    QTC_UNUSED(module);
    return gtk_check_version(GTK_MAJOR_VERSION, GTK_MINOR_VERSION,
                             GTK_MICRO_VERSION - GTK_INTERFACE_AGE);
}
