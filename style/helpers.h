#ifndef __QTC_HELPERS_H__
#define __QTC_HELPERS_H__

/*
  QtCurve (C) Craig Drummond, 2003 - 2010 craig.p.drummond@gmail.com

  ----

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
 */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "common.h"
#include "config.h"
#include "compatability.h"

#define CAIRO_COL(A)  (A).red/65535.0, (A).green/65535.0, (A).blue/65535.0
#define DETAIL(xx)    ((detail) &&(!strcmp(xx, detail)))
#define DETAILHAS(xx) ((detail) && (strstr(detail, xx)))

#ifndef GTK_IS_COMBO_BOX_ENTRY
#define GTK_IS_COMBO_BOX_ENTRY(x) 0
#endif
#ifndef GTK_IS_COMBO_BOX
#define GTK_IS_COMBO_BOX(x) 0
#endif

#if GTK_CHECK_VERSION(2, 90, 0)
    #define QTC_IS_COMBO(X)    GTK_IS_COMBO_BOX_TEXT(X)
    #define QTC_COMBO_ENTRY(X) GTK_IS_COMBO_BOX_TEXT(X)
#elif GTK_CHECK_VERSION(2, 24, 0)
    #define QTC_IS_COMBO(X)    (GTK_IS_COMBO(X) || GTK_IS_COMBO_BOX_TEXT(X))
    #define QTC_COMBO_ENTRY(X) (GTK_IS_COMBO_BOX_ENTRY(X) || GTK_IS_COMBO_BOX_TEXT(X))
#else
    #define QTC_IS_COMBO(X)    GTK_IS_COMBO(X)
    #define QTC_COMBO_ENTRY(X) GTK_IS_COMBO_BOX_ENTRY(X)
#endif

#define QT_CUSTOM_COLOR_BUTTON(style) \
    (style && \
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

extern void debugDisplayWidget(GtkWidget *widget, int level);
extern bool haveAlternateListViewCol();
extern gboolean isMozilla();
extern gboolean isMozillaTab(GtkWidget *widget);
extern gboolean isFakeGtk();
extern GdkColor * menuColors(gboolean active);
extern EBorder shadowToBorder(GtkShadowType shadow);
extern gboolean useButtonColor(const gchar *detail);
extern void qtcShadeColors(GdkColor *base, GdkColor *vals);
extern gboolean isSortColumn(GtkWidget *button);
extern GdkColor * getCellCol(GdkColor *std, const gchar *detail);
extern gboolean reverseLayout(GtkWidget *widget);
extern gboolean isOnToolbar(GtkWidget *widget, gboolean *horiz, int level);
extern gboolean isOnHandlebox(GtkWidget *widget, gboolean *horiz, int level);
extern gboolean isButtonOnToolbar(GtkWidget *widget, gboolean *horiz);
extern gboolean isButtonOnHandlebox(GtkWidget *widget, gboolean *horiz);
extern gboolean isOnStatusBar(GtkWidget *widget, int level);
extern gboolean isList(GtkWidget *widget);
extern gboolean isListViewHeader(GtkWidget *widget);
extern gboolean isEvolutionListViewHeader(GtkWidget *widget, const gchar *detail);
extern gboolean isOnListViewHeader(GtkWidget *w, int level);
extern gboolean isPathButton(GtkWidget *widget);
extern GtkWidget * getComboEntry(GtkWidget *widget);
extern GtkWidget * getComboButton(GtkWidget *widget);
extern gboolean isSideBarBtn(GtkWidget *widget);
extern gboolean isComboBoxButton(GtkWidget *widget);
extern gboolean isComboBox(GtkWidget *widget);
extern gboolean isComboBoxEntry(GtkWidget *widget);
extern gboolean isComboBoxEntryButton(GtkWidget *widget);
extern gboolean isGimpCombo(GtkWidget *widget);
extern gboolean isOnComboEntry(GtkWidget *w, int level);
extern gboolean isOnComboBox(GtkWidget *w, int level);
extern gboolean isOnCombo(GtkWidget *w, int level);
#if !GTK_CHECK_VERSION(2, 90, 0)
extern gboolean isOnOptionMenu(GtkWidget *w, int level);
extern gboolean isActiveOptionMenu(GtkWidget *widget);
#endif
extern gboolean isOnMenuItem(GtkWidget *w, int level);
extern gboolean isSpinButton(GtkWidget *widget);
extern gboolean isStatusBarFrame(GtkWidget *widget);
extern GtkMenuBar * isMenubar(GtkWidget *w, int level);
extern gboolean isMenuitem(GtkWidget *w, int level);
#define IS_MENU_ITEM(WIDGET) isMenuitem(WIDGET, 0)
extern gboolean isMenuWindow(GtkWidget *w);
#define IS_GROUP_BOX(W) ((W) && GTK_IS_FRAME((W)) && (NULL!=gtk_frame_get_label(GTK_FRAME((W))) || \
                                                      NULL!=gtk_frame_get_label_widget(GTK_FRAME((W)))))

extern gboolean isInGroupBox(GtkWidget *w, int level);
extern gboolean isOnButton(GtkWidget *w, int level, gboolean *def);
extern void optionMenuGetProps(GtkWidget *widget, GtkRequisition *indicator_size, GtkBorder *indicator_spacing);

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
extern void gdk_drawable_get_size(GdkWindow *window, gint *width, gint *height);
extern void sanitizeSizeReal(GtkWidget *widget, gint *width, gint *height);
extern EStepper getStepper(const char *detail);
#else
extern EStepper getStepper(GtkWidget *widget, int x, int y, int width, int height);
extern void sanitizeSize(GdkWindow *window, gint *width, gint *height);
#endif

extern int getFillReal(GtkStateType state, gboolean set, gboolean darker);
#define getFill(state, set) getFillReal(state, set, FALSE)
extern gboolean isSbarDetail(const char *detail);
extern gboolean isHorizontalProgressbar(GtkWidget *widget);
extern gboolean isComboBoxPopupWindow(GtkWidget *widget, int level);
extern gboolean isComboBoxList(GtkWidget *widget);
extern gboolean isComboPopupWindow(GtkWidget *widget, int level);
extern gboolean isComboList(GtkWidget *widget);
extern gboolean isComboMenu(GtkWidget *widget);
extern gboolean isComboFrame(GtkWidget *widget);
extern gboolean isFixedWidget(GtkWidget *widget);
extern gboolean isGimpDockable(GtkWidget *widget);
#define isMozillaWidget(widget) (isMozilla() && isFixedWidget(widget))
extern GdkColor * getParentBgCol(GtkWidget *widget);
extern int getOpacity(GtkWidget *widget);
extern gboolean eqRect(GdkRectangle *a, GdkRectangle *b);
extern void setLowerEtchCol(cairo_t *cr, GtkWidget *widget);
extern GdkColor shadeColor(GdkColor *orig, double mod);
extern void constrainRect(QtcRect *rect, QtcRect *con);
extern gboolean windowEvent(GtkWidget *widget, GdkEvent *event, gpointer user_data);
extern void adjustToolbarButtons(GtkWidget *widget, int *x, int *y, int *width, int *height, int *round, gboolean horiz);
extern void getEntryParentBgCol(GtkWidget *widget, GdkColor *color);
extern gboolean compositingActive(GtkWidget *widget);
extern gboolean isRgbaWidget(GtkWidget *widget);
extern void enableBlurBehind(GtkWidget *w, gboolean enable);
extern getTopLevelSize(GdkWindow *window, gint *w, gint *h);
extern void getTopLevelOrigin(GdkWindow *window, gint *x, gint *y);
extern gboolean mapToTopLevel(GdkWindow *window, GtkWidget *widget, gint *x, gint *y, gint *w, gint *h); //, gboolean frame)

#ifdef QTC_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT
extern GtkWidget * getParentWindow(GtkWidget *widget);
extern void dialogMapEvent(GtkWidget *widget, gpointer user_data);
#endif
extern gboolean treeViewCellHasChildren(GtkTreeView *treeView, GtkTreePath *path);
extern gboolean treeViewCellIsLast(GtkTreeView *treeView, GtkTreePath *path);
extern GtkTreePath * treeViewPathParent(GtkTreeView *treeView, GtkTreePath *path);
extern void generateColors();
extern GdkColor * getCheckRadioCol(GtkStyle *style, GtkStateType state, gboolean mnu);

#endif
