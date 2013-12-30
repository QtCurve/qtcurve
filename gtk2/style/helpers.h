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

#ifndef __QTC_HELPERS_H__
#define __QTC_HELPERS_H__

#include <common/common.h>
#include "config.h"
#include "qt_settings.h"

QTC_ALWAYS_INLINE static inline void
qtcCairoSetColor(cairo_t *cr, const GdkColor *col, double a)
{
    cairo_set_source_rgba(cr, col->red / 65535.0, col->green / 65535.0,
                          col->blue / 65535.0, a);
}
QTC_ALWAYS_INLINE static inline void
_qtcCairoSetColor(cairo_t *cr, const GdkColor *col)
{
    qtcCairoSetColor(cr, col, 1);
}
#define qtcCairoSetColor(cr, col, a...)                 \
    QTC_SWITCH_(a, qtcCairoSetColor)(cr, col, ##a)

QTC_ALWAYS_INLINE static inline void
qtcCairoPatternAddColorStop(cairo_pattern_t *pt, double offset,
                            const GdkColor *col, double a)
{
    cairo_pattern_add_color_stop_rgba(pt, offset, col->red / 65535.0,
                                      col->green / 65535.0,
                                      col->blue / 65535.0, a);
}

QTC_ALWAYS_INLINE static inline void
_qtcCairoPatternAddColorStop(cairo_pattern_t *pt, double offset,
                             const GdkColor *col)
{
    qtcCairoPatternAddColorStop(pt, offset, col, 1);
}
#define qtcCairoPatternAddColorStop(pt, offset, col, a...)              \
    QTC_SWITCH_(a, qtcCairoPatternAddColorStop)(pt, offset, col, ##a)

#define DETAIL(xx)    ((detail) &&(!strcmp(xx, detail)))
#define DETAILHAS(xx) ((detail) && (strstr(detail, xx)))

#ifndef GTK_IS_COMBO_BOX_ENTRY
#define GTK_IS_COMBO_BOX_ENTRY(x) 0
#endif
#ifndef GTK_IS_COMBO_BOX
#define GTK_IS_COMBO_BOX(x) 0
#endif

#if GTK_CHECK_VERSION(2, 90, 0)
#  define QTC_IS_COMBO(X) GTK_IS_COMBO_BOX_TEXT(X)
#  define QTC_COMBO_ENTRY(X) GTK_IS_COMBO_BOX_TEXT(X)
#elif GTK_CHECK_VERSION(2, 24, 0)
#  define QTC_IS_COMBO(X) (GTK_IS_COMBO(X) || GTK_IS_COMBO_BOX_TEXT(X))
#  define QTC_COMBO_ENTRY(X) (GTK_IS_COMBO_BOX_ENTRY(X) ||      \
                              GTK_IS_COMBO_BOX_TEXT(X))
#else
#  define QTC_IS_COMBO(X) GTK_IS_COMBO(X)
#  define QTC_COMBO_ENTRY(X) GTK_IS_COMBO_BOX_ENTRY(X)
#endif

#define QT_CUSTOM_COLOR_BUTTON(style)           \
    (style &&                                                           \
     !(COL_EQ(qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW].red,(style->bg[GTK_STATE_SELECTED==state ? state : GTK_STATE_NORMAL].red)) && \
      COL_EQ(qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW].green,(style->bg[GTK_STATE_SELECTED==state ? state : GTK_STATE_NORMAL].green)) && \
      COL_EQ(qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW].blue,(style->bg[GTK_STATE_SELECTED==state ? state : GTK_STATE_NORMAL].blue))))

#define SET_BTN_COLS(SCROLLBAR, SCALE, LISTVIEW, STATE) \
{ \
    if(SCROLLBAR || SCALE) \
        btnColors=GTK_STATE_INSENSITIVE==STATE \
                    ? qtcPalette.background \
                    : SHADE_NONE!=opts.shadeSliders && qtcPalette.slider && \
                      (!opts.colorSliderMouseOver || GTK_STATE_PRELIGHT==STATE) \
                        ? qtcPalette.slider \
                        : qtcPalette.button[PAL_ACTIVE]; \
    else if(LISTVIEW) \
    { \
        if(GTK_STATE_INSENSITIVE!=state && qtcPalette.sortedlv && isSortColumn(widget)) \
            btnColors=qtcPalette.sortedlv;  \
        else if(opts.lvButton) \
            btnColors=qtcPalette.button[GTK_STATE_INSENSITIVE==STATE ? PAL_DISABLED : PAL_ACTIVE]; \
        else \
            btnColors=qtcPalette.background; \
    } \
    else \
        btnColors=qtcPalette.button[GTK_STATE_INSENSITIVE==STATE ? PAL_DISABLED : PAL_ACTIVE]; \
}

#define ARROW_STATE(state) (GTK_STATE_INSENSITIVE==state ? state : GTK_STATE_NORMAL)
/* (GTK_STATE_ACTIVE==state ? GTK_STATE_NORMAL : state) */

void debugDisplayWidget(GtkWidget *widget, int level);
bool haveAlternateListViewCol();
gboolean isFixedWidget(GtkWidget *widget);
QTC_ALWAYS_INLINE static inline bool
isMozilla()
{
    return (qtSettings.app == GTK_APP_MOZILLA ||
            qtSettings.app == GTK_APP_NEW_MOZILLA);
}
QTC_ALWAYS_INLINE static inline bool
isMozillaTab(GtkWidget *widget)
{
    return isFixedWidget(widget) && GTK_IS_NOTEBOOK(widget);
}
QTC_ALWAYS_INLINE static inline bool
isFakeGtk()
{
    return (isMozilla() || qtSettings.app == GTK_APP_OPEN_OFFICE ||
            qtSettings.app == GTK_APP_JAVA);
}
GdkColor *menuColors(gboolean active);
EBorder shadowToBorder(GtkShadowType shadow);
gboolean useButtonColor(const char *detail);
void qtcShadeColors(GdkColor *base, GdkColor *vals);
gboolean isSortColumn(GtkWidget *button);
GdkColor * getCellCol(GdkColor *std, const char *detail);
gboolean reverseLayout(GtkWidget *widget);
gboolean isOnToolbar(GtkWidget *widget, gboolean *horiz, int level);
gboolean isOnHandlebox(GtkWidget *widget, gboolean *horiz, int level);
gboolean isButtonOnToolbar(GtkWidget *widget, gboolean *horiz);
gboolean isButtonOnHandlebox(GtkWidget *widget, gboolean *horiz);
gboolean isOnStatusBar(GtkWidget *widget, int level);
gboolean isList(GtkWidget *widget);
gboolean isListViewHeader(GtkWidget *widget);
gboolean isEvolutionListViewHeader(GtkWidget *widget, const char *detail);
gboolean isOnListViewHeader(GtkWidget *w, int level);
gboolean isPathButton(GtkWidget *widget);
GtkWidget * getComboEntry(GtkWidget *widget);
GtkWidget * getComboButton(GtkWidget *widget);
gboolean isSideBarBtn(GtkWidget *widget);
gboolean isComboBoxButton(GtkWidget *widget);
gboolean isComboBox(GtkWidget *widget);
gboolean isComboBoxEntry(GtkWidget *widget);
gboolean isComboBoxEntryButton(GtkWidget *widget);
gboolean isGimpCombo(GtkWidget *widget);
gboolean isOnComboEntry(GtkWidget *w, int level);
gboolean isOnComboBox(GtkWidget *w, int level);
gboolean isOnCombo(GtkWidget *w, int level);
#if !GTK_CHECK_VERSION(2, 90, 0)
gboolean isOnOptionMenu(GtkWidget *w, int level);
gboolean isActiveOptionMenu(GtkWidget *widget);
#endif
gboolean isOnMenuItem(GtkWidget *w, int level);
gboolean isSpinButton(GtkWidget *widget);
gboolean isStatusBarFrame(GtkWidget *widget);
GtkMenuBar * isMenubar(GtkWidget *w, int level);
gboolean isMenuitem(GtkWidget *w, int level);
#define IS_MENU_ITEM(WIDGET) isMenuitem(WIDGET, 0)
gboolean isMenuWindow(GtkWidget *w);
#define IS_GROUP_BOX(W) ((W) && GTK_IS_FRAME((W)) && (NULL!=gtk_frame_get_label(GTK_FRAME((W))) || \
                                                      NULL!=gtk_frame_get_label_widget(GTK_FRAME((W)))))

gboolean isInGroupBox(GtkWidget *w, int level);
gboolean isOnButton(GtkWidget *w, int level, gboolean *def);
void optionMenuGetProps(GtkWidget *widget, GtkRequisition *indicator_size, GtkBorder *indicator_spacing);

typedef enum
{
    STEPPER_A,
    STEPPER_B,
    STEPPER_C,
    STEPPER_D,
    STEPPER_NONE
} EStepper;

#if GTK_CHECK_VERSION(2, 90, 0)
#define sanitizeSize(A, B, C) sanitizeSizeReal(widget, B, C)
void gdk_drawable_get_size(GdkWindow *window, int *width, int *height);
void sanitizeSizeReal(GtkWidget *widget, int *width, int *height);
EStepper getStepper(const char *detail);
#else
EStepper getStepper(GtkWidget *widget, int x, int y, int width, int height);
void sanitizeSize(GdkWindow *window, int *width, int *height);
#endif

int getFillReal(GtkStateType state, gboolean set, gboolean darker);
#define getFill(state, set) getFillReal(state, set, FALSE)
gboolean isSbarDetail(const char *detail);
gboolean isHorizontalProgressbar(GtkWidget *widget);
gboolean isComboBoxPopupWindow(GtkWidget *widget, int level);
gboolean isComboBoxList(GtkWidget *widget);
gboolean isComboPopupWindow(GtkWidget *widget, int level);
gboolean isComboList(GtkWidget *widget);
gboolean isComboMenu(GtkWidget *widget);
gboolean isComboFrame(GtkWidget *widget);
gboolean isGimpDockable(GtkWidget *widget);
#define isMozillaWidget(widget) (isMozilla() && isFixedWidget(widget))
GdkColor * getParentBgCol(GtkWidget *widget);
int getOpacity(GtkWidget *widget);
gboolean eqRect(GdkRectangle *a, GdkRectangle *b);
void setLowerEtchCol(cairo_t *cr, GtkWidget *widget);
GdkColor shadeColor(GdkColor *orig, double mod);
void constrainRect(GdkRectangle *rect, GdkRectangle *con);
gboolean windowEvent(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void adjustToolbarButtons(GtkWidget *widget, int *x, int *y, int *width, int *height, int *round, gboolean horiz);
void getEntryParentBgCol(GtkWidget *widget, GdkColor *color);
gboolean compositingActive(GtkWidget *widget);
gboolean isRgbaWidget(GtkWidget *widget);
void enableBlurBehind(GtkWidget *w, gboolean enable);
void getTopLevelSize(GdkWindow *window, int *w, int *h);
void getTopLevelOrigin(GdkWindow *window, int *x, int *y);
gboolean mapToTopLevel(GdkWindow *window, GtkWidget *widget, int *x, int *y, int *w, int *h); //, gboolean frame)
int getRound(const char *detail, GtkWidget *widget, int x, int y,
             int width, int height, gboolean rev);

gboolean treeViewCellHasChildren(GtkTreeView *treeView, GtkTreePath *path);
gboolean treeViewCellIsLast(GtkTreeView *treeView, GtkTreePath *path);
GtkTreePath * treeViewPathParent(GtkTreeView *treeView, GtkTreePath *path);
void generateColors();
GdkColor * getCheckRadioCol(GtkStyle *style, GtkStateType state, gboolean mnu);

#endif
