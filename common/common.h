#ifndef __COMMON_H__
#define __COMMON_H__

/*
  QtCurve (C) Craig Drummond, 2003 - 2009 craig_p_drummond@yahoo.co.uk

  ----

  This program is free software; you can redistr ibute it and/or
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

/* This file is quite 'hacky' as it contains lots of static function definitions - but I want to share the code
   between Qt and Gtk, but not polute the namespace with exported functions... */

#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "config.h"

/*
    The following #define disables the rounding when scrollbar type==none.
#define QTC_SIMPLE_SCROLLBARS
*/

/*
    The following #define controls whether a scrollbar's slider should overlap
    the scrollbar buttons when at min/max. This removes the thick looking line
    between the slider and the buttons.
*/
#define QTC_INCREASE_SB_SLIDER

/*
    Control whether toolbar, window decoration, and dock window, buttons should have coloured mouse-over
*/
#define QTC_DONT_COLOUR_MOUSEOVER_TBAR_BUTTONS
/*
    Control shading used for glass variants.
    0 => As used in 0.51.1 +
    1 => As used in 0.51
    2 => As used in <0.51
*/
#define QTC_GLASS_SHADING 0

typedef enum
{
    SHADING_SIMPLE=0,
    SHADING_HSL=1,
    SHADING_HSV=2,
    SHADING_HCY=3
} EShading;

#ifdef __cplusplus
#include <qconfig.h>
#ifdef QTC_CONFIG_DIALOG
#include <qapplication.h>
#endif
#include <map>
#include <set>
#else
#include <glib.h>
#endif

#ifdef __cplusplus
typedef QColor color;
#else
typedef gboolean bool;
typedef GdkColor color;
#define true TRUE
#define false FALSE
#endif

#define QTC_GROUP        "Settings"
/*#define QTC_DESCR_GROUP  "Description"*/

/* qtc_<theme name>.themerc support */
#define KDE_PREFIX(V) ((4==(V)) ? KDE4PREFIX : KDE3PREFIX)
#define QTC_THEME_DIR    "/share/apps/kstyle/themes/"
#define QTC_THEME_DIR4   "/share/kde4/apps/kstyle/themes/"
#define QTC_THEME_PREFIX "qtc_"
#define QTC_THEME_SUFFIX ".themerc"


#define QTC_CHECK_SIZE   13
#define QTC_RADIO_SIZE   13
#define QTC_LV_SIZE      7

#define LARGE_ARR_WIDTH  7
#define LARGE_ARR_HEIGHT 4
#define SMALL_ARR_WIDTH  5
#define SMALL_ARR_HEIGHT 3

#define NUM_STD_SHADES   6
#define NUM_EXTRA_SHADES 3

#define TOTAL_SHADES     NUM_STD_SHADES+NUM_EXTRA_SHADES
#define ORIGINAL_SHADE   TOTAL_SHADES

#define SHADE_ORIG_HIGHLIGHT NUM_STD_SHADES
#define SHADE_4_HIGHLIGHT    NUM_STD_SHADES+1
#define SHADE_2_HIGHLIGHT    NUM_STD_SHADES+2

/* 3d effect - i.e. buttons, etc */
#define QTC_SHADES \
    static const double shades[2][11][NUM_STD_SHADES]=\
    { \
        { /* HSV & HSL */ \
            { 1.05, 1.04, 0.90, 0.800, 0.830, 0.82 }, \
            { 1.06, 1.04, 0.90, 0.790, 0.831, 0.78 }, \
            { 1.07, 1.04, 0.90, 0.785, 0.832, 0.75 }, \
            { 1.08, 1.05, 0.90, 0.782, 0.833, 0.72 }, \
            { 1.09, 1.05, 0.90, 0.782, 0.834, 0.70 }, \
            { 1.10, 1.06, 0.90, 0.782, 0.836, 0.68 }, \
            { 1.12, 1.06, 0.90, 0.782, 0.838, 0.63 }, \
            { 1.16, 1.07, 0.90, 0.782, 0.840, 0.62 }, /* default */ \
            { 1.18, 1.07, 0.90, 0.783, 0.842, 0.60 }, \
            { 1.20, 1.08, 0.90, 0.784, 0.844, 0.58 }, \
            { 1.22, 1.08, 0.90, 0.786, 0.848, 0.55 }  \
        }, \
        { /* SIMPLE */ \
            { 1.07, 1.03, 0.91, 0.780, 0.834, 0.75 }, \
            { 1.08, 1.03, 0.91, 0.781, 0.835, 0.74 }, \
            { 1.09, 1.03, 0.91, 0.782, 0.836, 0.73 }, \
            { 1.10, 1.04, 0.91, 0.783, 0.837, 0.72 }, \
            { 1.11, 1.04, 0.91, 0.784, 0.838, 0.71 }, \
            { 1.12, 1.05, 0.91, 0.785, 0.840, 0.70 }, \
            { 1.13, 1.05, 0.91, 0.786, 0.842, 0.69 }, \
            { 1.14, 1.06, 0.91, 0.787, 0.844, 0.68 }, /* default */ \
            { 1.16, 1.06, 0.91, 0.788, 0.846, 0.66 }, \
            { 1.18, 1.07, 0.91, 0.789, 0.848, 0.64 }, \
            { 1.20, 1.07, 0.91, 0.790, 0.850, 0.62 }  \
        } \
    } ;

#define QTC_SIMPLE_SHADING (!shading)

#define QTC_THIN_SBAR_MOD  (opts.sliderWidth<DEFAULT_SLIDER_WIDTH ? 3 : opts.sliderWidth>DEFAULT_SLIDER_WIDTH ? (opts.sliderWidth-9)/2 : 4)
#define QTC_SLIDER_SIZE (opts.sliderWidth<DEFAULT_SLIDER_WIDTH ? DEFAULT_SLIDER_WIDTH-2 : opts.sliderWidth)
#define QTC_GLOW_MO        1 /*ORIGINAL_SHADE*/
#define QTC_GLOW_DEFBTN    1
#define QTC_GLOW_ALPHA(DEF) ((DEF) ? 0.5 : 0.65)
#define QTC_DEF_BNT_TINT   0.4
#define QTC_ENTRY_INNER_ALPHA 0.4

#define QT_STD_BORDER      5
#define QT_PBAR_BORDER     4
#define QT_LOWER_BORDER_ALPHA 0.65
#define QT_DISABLED_BORDER QT_STD_BORDER /*3*/
#define QT_BORDER(E) (/*(E) ?*/ QT_STD_BORDER/* : QT_DISABLED_BORDER*/)
#define QT_SLIDER_MO_BORDER 3

#define QT_FRAME_DARK_SHADOW 2
#define QT_FOCUS(SEL)         ((SEL) ? 3 : ORIGINAL_SHADE)
#define QTC_MENU_STRIPE_SHADE (USE_LIGHTER_POPUP_MENU ? ORIGINAL_SHADE : 2)
#define QTC_MENU_SEP_SHADE    (USE_LIGHTER_POPUP_MENU ? 4 : 3)

#define QTC_SHADE(c, s) \
    (c>10 || c<0 || s>=NUM_STD_SHADES || s<0 \
        ? 1.0 \
        : opts.darkerBorders && (QT_STD_BORDER==i || QT_DISABLED_BORDER==i) \
            ? shades[SHADING_SIMPLE==opts.shading ? 1 : 0][c][s] - 0.1 \
            : shades[SHADING_SIMPLE==opts.shading ? 1 : 0][c][s] )

#define TAB_APPEARANCE(A)   (A) /* (APPEARANCE_GLASS==(A) ? APPEARANCE_GRADIENT : (A)) */
#define QTC_COLOR_SEL_TAB_FACTOR 0.25

#define INVERT_SHADE(A) (1.0+(1.0-(A)))

#define QTC_ROUNDED (ROUND_NONE!=opts.round)

#define QTC_TOOLBAR_SEP_GAP        (opts.fadeLines ? 5 : 6)
#define QTC_FADE_SIZE              0.4
#define QTC_ETCHED_DARK            0.95

#define IS_GLASS(A) (APPEARANCE_DULL_GLASS==(A) || APPEARANCE_SHINY_GLASS==(A))
#define IS_CUSTOM(A) ((A)>=APPEARANCE_CUSTOM1 && (A)<(APPEARANCE_CUSTOM1+QTC_NUM_CUSTOM_GRAD))
#define IS_FLAT(A)  (APPEARANCE_FLAT==(A) || APPEARANCE_RAISED==(A) || APPEARANCE_FADE==(A))

#ifdef __cplusplus
#define MENUBAR_DARK_LIMIT 160
#define TOO_DARK(A) ((A).red()<MENUBAR_DARK_LIMIT || (A).green()<MENUBAR_DARK_LIMIT || (A).blue()<MENUBAR_DARK_LIMIT)
#else
#define MENUBAR_DARK_LIMIT (160<<8)
#define TOO_DARK(A) ((A).red<MENUBAR_DARK_LIMIT || (A).green<MENUBAR_DARK_LIMIT || (A).blue<MENUBAR_DARK_LIMIT)
#endif

#define QTC_TO_FACTOR(A) ((100.0+((double)(A)))/100.0)
#define DEFAULT_HIGHLIGHT_FACTOR                   3
#define MAX_HIGHLIGHT_FACTOR                      50
#define MIN_HIGHLIGHT_FACTOR                     -50
#define MENUBAR_DARK_FACTOR        QTC_TO_FACTOR(-3)
#define INACTIVE_HIGHLIGHT_FACTOR  QTC_TO_FACTOR(20)
#define LV_HEADER_DARK_FACTOR     QTC_TO_FACTOR(-10)
#define DEF_POPUPMENU_LIGHT_FACTOR                 2
#define MIN_LIGHTER_POPUP_MENU                  -100
#define MAX_LIGHTER_POPUP_MENU                   100

#define DEF_TAB_BGND         0
#define MIN_TAB_BGND        -5
#define MAX_TAB_BGND         5

#define DEFAULT_MENU_DELAY 100
#define MIN_MENU_DELAY       0
#define MAX_MENU_DELAY     500

#define DEFAULT_SLIDER_WIDTH 15
#define MIN_SLIDER_WIDTH     11
#define MAX_SLIDER_WIDTH     31

#define SIZE_GRIP_SIZE 12

#define USE_LIGHTER_POPUP_MENU (opts.lighterPopupMenuBgnd)

#define USE_SHADED_MENU_BAR_COLORS (SHADE_CUSTOM==opts.shadeMenubars || SHADE_BLEND_SELECTED==opts.shadeMenubars)
#define MENUBAR_GLASS_SELECTED_DARK_FACTOR 0.9

#define MENUITEM_FADE_SIZE 48

#define NUM_SPLITTER_DASHES 21

#ifdef __cplusplus
#define WIDGET_BUTTON(w) (WIDGET_STD_BUTTON==(w) || WIDGET_DEF_BUTTON==(w) || WIDGET_TOGGLE_BUTTON==(w) || WIDGET_CHECKBOX==(w) || \
                          WIDGET_COMBO==(w) || WIDGET_COMBO_BUTTON==(w) || WIDGET_MDI_WINDOW_BUTTON==(w) || \
                          WIDGET_TOOLBAR_BUTTON==(w) )
#define ETCH_WIDGET(w) (WIDGET_STD_BUTTON==(w) || WIDGET_DEF_BUTTON==(w) || WIDGET_TOGGLE_BUTTON==(w) || WIDGET_SLIDER_TROUGH==(w) || \
                        WIDGET_FILLED_SLIDER_TROUGH==(w) || WIDGET_MDI_WINDOW_BUTTON==(w) || WIDGET_TOOLBAR_BUTTON==(w))
#define AGUA_WIDGET(w) (WIDGET_STD_BUTTON==(w) || WIDGET_DEF_BUTTON==(w) || WIDGET_TOGGLE_BUTTON==(w) || IS_SLIDER((w)) || \
                        WIDGET_COMBO==(w) WIDGET_COMBO_BUTTON==(w) || WIDGET_MDI_WINDOW_BUTTON==(w))
#else
#define WIDGET_BUTTON(w) (WIDGET_STD_BUTTON==(w) || WIDGET_DEF_BUTTON==(w) || WIDGET_TOGGLE_BUTTON==(w) || WIDGET_CHECKBOX==(w) || \
                          WIDGET_COMBO==(w) || WIDGET_COMBO_BUTTON==(w) || WIDGET_UNCOLOURED_MO_BUTTON==(w) || \
                          WIDGET_TOOLBAR_BUTTON==(w))
#define ETCH_WIDGET(w) (WIDGET_STD_BUTTON==(w) || WIDGET_DEF_BUTTON==(w) || WIDGET_TOGGLE_BUTTON==(w) || WIDGET_SLIDER_TROUGH==(w) || \
                        WIDGET_FILLED_SLIDER_TROUGH==(w) || WIDGET_COMBO==(w) || WIDGET_UNCOLOURED_MO_BUTTON==(w) || \
                        WIDGET_TOOLBAR_BUTTON==(w))
#define AGUA_WIDGET(w) (WIDGET_STD_BUTTON==(w) || WIDGET_DEF_BUTTON==(w) || WIDGET_TOGGLE_BUTTON==(w) || IS_SLIDER((w)) || \
                        WIDGET_COMBO==(w) WIDGET_COMBO_BUTTON==(w))
#endif

#define MODIFY_AGUA_X(A, X) (APPEARANCE_AGUA==(A) ?  (X) : (A))
#define MODIFY_AGUA(A)      MODIFY_AGUA_X((A), APPEARANCE_AGUA_MOD)

#define COLORED_BORDER_SIZE 3
#define PROGRESS_CHUNK_WIDTH 10
#define QTC_DRAW_LIGHT_BORDER(SUKEN, WIDGET, APP) \
    ((!(SUKEN) && (GB_LIGHT==getGradient(APP, &opts)->border) && WIDGET_MENU_ITEM!=(WIDGET) && !IS_TROUGH(WIDGET) && \
                          (WIDGET_DEF_BUTTON!=(WIDGET) || IND_COLORED!=opts.defBtnIndicator)) || \
                          (WIDGET_PROGRESSBAR==(WIDGET) && APPEARANCE_FLAT!=(APP) && \
                           APPEARANCE_RAISED!=(APP) && APPEARANCE_INVERTED!=(APP) && APPEARANCE_BEVELLED!=(APP)))

#define QTC_DRAW_3D_FULL_BORDER(SUNKEN, APP) \
    (!(SUNKEN) && GB_3D_FULL==getGradient((APP), &opts)->border)
    
#define QTC_DRAW_3D_BORDER(SUNKEN, APP) \
    (!(SUNKEN) && GB_3D==getGradient((APP), &opts)->border)

#define QTC_LIGHT_BORDER(APP) (APPEARANCE_DULL_GLASS==(APP) ? 1 : 0)

#define PROGRESS_ANIMATION 100
#define MIN_SLIDER_SIZE(A) (LINE_DOTS==(A) ? 24 : 20)

#define QTC_TAB_APP(A)   (APPEARANCE_BEVELLED==(A) || APPEARANCE_SPLIT_GRADIENT==(A) ? APPEARANCE_GRADIENT : (A))
#define QTC_NORM_TAB_APP QTC_TAB_APP(opts.tabAppearance)
#define QTC_SEL_TAB_APP  QTC_TAB_APP(opts.activeTabAppearance)

#define QTC_SLIDER_MO_SHADE  (SHADE_SELECTED==opts.shadeSliders ? 1 : (SHADE_BLEND_SELECTED==opts.shadeSliders ? 0 : ORIGINAL_SHADE))
#define QTC_SLIDER_MO_BORDER (SHADE_SELECTED==opts.shadeSliders || SHADE_BLEND_SELECTED==opts.shadeSliders ? 2 : 1)
#define QTC_SLIDER_MO_LEN (SLIDER_TRIANGULAR==opts.sliderStyle ? 2 : (SHADE_SELECTED==opts.shadeSliders || SHADE_BLEND_SELECTED==opts.shadeSliders ? 4 : 3))
#define QTC_SB_SLIDER_MO_LEN(A) ((A)<22 && !QTC_FULLLY_ROUNDED \
                                    ? 2 \
                                    : ((A)<32 || (SHADE_SELECTED!=opts.shadeSliders && SHADE_BLEND_SELECTED!=opts.shadeSliders) \
                                        ? 4 \
                                        : 6))

#define QTC_CR_MO_FILL          1
#define QTC_MO_DEF_BTN          2
#define QTC_MO_PLASTIK_DARK(W)  (WIDGET_DEF_BUTTON==(W) && IND_COLORED==opts.defBtnIndicator ? 3 : 2) /*? 2 : 1) */
#define QTC_MO_PLASTIK_LIGHT(W) (WIDGET_DEF_BUTTON==(W) && IND_COLORED==opts.defBtnIndicator ? 4 : 1) /*? 2 : 0) */

#define QTC_MO_STD_DARK(W)     (MO_GLOW==opts.coloredMouseOver \
                                    ? 1 \
                                    : QTC_MO_PLASTIK_DARK(W))
#define QTC_MO_STD_LIGHT(W, S) (MO_GLOW==opts.coloredMouseOver \
                                    ? 1 \
                                    : QTC_MO_PLASTIK_LIGHT(W))

#define QTC_FULLLY_ROUNDED     (opts.round>=ROUND_FULL)
#if !defined __cplusplus || (defined QT_VERSION && (QT_VERSION >= 0x040000))
#define QTC_DO_EFFECT          (EFFECT_NONE!=opts.buttonEffect)
#else
#define QTC_DO_EFFECT          (QTC_FULLLY_ROUNDED && EFFECT_NONE!=opts.buttonEffect)
#endif

#if !defined __cplusplus || (defined QT_VERSION && (QT_VERSION >= 0x040000))
#define QTC_FOCUS_ALPHA              0.08
#define QTC_BORDER_BLEND_ALPHA       0.7
#define QTC_ETCH_TOP_ALPHA           0.055
#define QTC_ETCH_BOTTOM_ALPHA        0.80
// #if defined QT_VERSION && (QT_VERSION >= 0x040000)
// #define QTC_ETCH_RADIO_TOP_ALPHA     0.055
// #define QTC_ETCH_RADIO_BOTTOM_ALPHA  0.80
// #else
#define QTC_ETCH_RADIO_TOP_ALPHA     0.09
#define QTC_ETCH_RADIO_BOTTOM_ALPHA  1.0
// #endif
#endif

#if defined __cplusplus && defined QT_VERSION && (QT_VERSION >= 0x040000)

#include <qstyle.h>
typedef enum
{
    QtC_Round = QStyle::PM_CustomBase,
    QtC_TitleBarButtonAppearance,
    QtC_TitleBarColorTopOnly,
    QtC_TitleAlignment,
    QtC_TitleBarButtons,
    QtC_TitleBarIcon,
    QtC_TitleBarIconColor
} QtCMetrics;

#define QtC_StateKWin          ((QStyle::StateFlag)0x10000000)
#define QtC_StateKWinHighlight ((QStyle::StateFlag)0x20000000)
#define QtC_StateKWinShadows   ((QStyle::StateFlag)0x40000000)
#define QtCStateKWinDrawLine   ((QStyle::StateFlag)0x80000000)
#define QtCStateKWinNotFull    ((QStyle::StateFlag)0x80000000)
#define QtCStateKWinNoBorder   ((QStyle::StateFlag)0x08000000)

#define CLOSE_COLOR QColor(191, 82, 82)
#define HOVER_BUTTON_ALPHA 0.2
#endif

#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000))
typedef enum
{
    QTC_TITLEBAR_BUTTON_ROUND            = 0x0001,
    QTC_TITLEBAR_BUTTON_HOVER_FRAME      = 0x0002,
    QTC_TITLEBAR_BUTTON_HOVER_SYMBOL     = 0x0004,
    QTC_TITLEBAR_BUTTON_NO_FRAME         = 0x0008,
    QTC_TITLEBAR_BUTTON_COLOR            = 0x0010,
    QTC_TITLEBAR_BUTTON_COLOR_INACTIVE   = 0x0020,
    QTC_TITLEBAR_BUTTON_COLOR_MOUSE_OVER = 0x0040,
    QTC_TITLEBAR_BUTTON_STD_COLOR        = 0x0080,
    QTC_TITLEBAR_BUTTON_COLOR_SYMBOL     = 0x0100
} ETitleBarButtonFlags;

typedef enum
{
    TITLEBAR_ICON_NONE,
    TITLEBAR_ICON_MENU_BUTTON,
    TITLEBAR_ICON_NEXT_TO_TITLE
} ETitleBarIcon;

typedef enum
{
    TITLEBAR_CLOSE,
    TITLEBAR_MIN,
    TITLEBAR_MAX,
    TITLEBAR_HELP,
    TITLEBAR_MENU,
    TITLEBAR_SHADE,
    TITLEBAR_ALL_DESKTOPS,
    TITLEBAR_KEEP_ABOVE,
    TITLEBAR_KEEP_BELOW,
    NUM_TITLEBAR_BUTTONS
} ETitleBarButtons;

#define QTC_TBAR_VERSION_HACK 65535

typedef std::map<ETitleBarButtons, QColor> TBCols;
#endif

typedef enum
{
    EFFECT_NONE,
    EFFECT_ETCH,
    EFFECT_SHADOW
} EEffect;

typedef enum
{
    PIX_RADIO_BORDER,
    PIX_RADIO_INNER,
    PIX_RADIO_LIGHT,
    PIX_RADIO_ON,
    PIX_CHECK,
    PIX_SLIDER,
    PIX_SLIDER_LIGHT,
    PIX_SLIDER_V,
    PIX_SLIDER_LIGHT_V
    //PIX_DOT
#if !defined __cplusplus
    , PIX_BLANK
#endif
} EPixmap;

typedef enum
{
    WIDGET_TAB_TOP,
    WIDGET_TAB_BOT,
    WIDGET_STD_BUTTON,
    WIDGET_DEF_BUTTON,
    WIDGET_TOOLBAR_BUTTON,
    WIDGET_LISTVIEW_HEADER,
    WIDGET_SLIDER,
    WIDGET_SLIDER_TROUGH,
    WIDGET_FILLED_SLIDER_TROUGH,
    WIDGET_SB_SLIDER,
    WIDGET_SB_BUTTON,
    WIDGET_SB_BGND,
    WIDGET_TROUGH,
    WIDGET_CHECKBOX,
    WIDGET_TOGGLE_BUTTON,
    WIDGET_COMBO,
    WIDGET_COMBO_BUTTON,
    WIDGET_MENU_ITEM,
    WIDGET_PROGRESSBAR,
    WIDGET_PBAR_TROUGH,
#ifndef __cplusplus
    WIDGET_SPIN_UP,
    WIDGET_SPIN_DOWN,
    WIDGET_UNCOLOURED_MO_BUTTON,
#endif
    WIDGET_SPIN,
    WIDGET_ENTRY,
    WIDGET_SCROLLVIEW,
#ifdef __cplusplus
    WIDGET_CHECKBUTTON,        // Qt4 only
    WIDGET_MDI_WINDOW,         // Qt4 only
    WIDGET_MDI_WINDOW_TITLE,   // Qt4 only
    WIDGET_MDI_WINDOW_BUTTON,  // Qt4 only
#endif
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
    WIDGET_SELECTION,
//    WIDGET_RUBBER_BAND,
#endif
    WIDGET_FRAME,
    WIDGET_NO_ETCH_BTN,
    WIDGET_MENU_BUTTON,        // Qt4 only
    WIDGET_FOCUS,
    WIDGET_TAB_FRAME,
    WIDGET_OTHER
} EWidget;

typedef enum
{
    APPEARANCE_CUSTOM1,
    APPEARANCE_CUSTOM2,
    APPEARANCE_CUSTOM3,
    APPEARANCE_CUSTOM4,
    APPEARANCE_CUSTOM5,
    APPEARANCE_CUSTOM6,
    APPEARANCE_CUSTOM7,
    APPEARANCE_CUSTOM8,
    APPEARANCE_CUSTOM9,
    APPEARANCE_CUSTOM10,
    APPEARANCE_CUSTOM11,
    APPEARANCE_CUSTOM12,
    APPEARANCE_CUSTOM13,
    APPEARANCE_CUSTOM14,
    APPEARANCE_CUSTOM15,
    APPEARANCE_CUSTOM16,
    APPEARANCE_CUSTOM17,
    APPEARANCE_CUSTOM18,
    APPEARANCE_CUSTOM19,
    APPEARANCE_CUSTOM20,
    APPEARANCE_CUSTOM21,

        QTC_NUM_CUSTOM_GRAD,

    APPEARANCE_FLAT = QTC_NUM_CUSTOM_GRAD,
    APPEARANCE_RAISED,
    APPEARANCE_DULL_GLASS,
    APPEARANCE_SHINY_GLASS,
    APPEARANCE_AGUA,
    APPEARANCE_SOFT_GRADIENT,
    APPEARANCE_GRADIENT,
    APPEARANCE_HARSH_GRADIENT,
    APPEARANCE_INVERTED,
    APPEARANCE_DARK_INVERTED,
    APPEARANCE_SPLIT_GRADIENT,
    APPEARANCE_BEVELLED,
        APPEARANCE_FADE, /* Only for poupmenu items! */
        APPEARANCE_LV_BEVELLED, /* To be used only with getGradient */
        APPEARANCE_AGUA_MOD,
        APPEARANCE_LV_AGUA,
    QTC_NUM_STD_APP = APPEARANCE_LV_AGUA-QTC_NUM_CUSTOM_GRAD
} EAppearance;

#define IS_SLIDER(W)        (WIDGET_SLIDER==(W) || WIDGET_SB_SLIDER==(W))
#define IS_TROUGH(W)        (WIDGET_SLIDER_TROUGH==(W) || WIDGET_PBAR_TROUGH==(W) || WIDGET_TROUGH==(W) || WIDGET_FILLED_SLIDER_TROUGH==(W))
#define IS_TOGGLE_BUTTON(W) (WIDGET_TOGGLE_BUTTON==(W) || WIDGET_CHECKBOX==(W))

typedef enum
{
    CORNER_TL = 0x1,
    CORNER_TR = 0x2,
    CORNER_BR = 0x4,
    CORNER_BL = 0x8
} ECornerBits;

#define ROUNDED_NONE            0x0
#define ROUNDED_TOP             (CORNER_TL|CORNER_TR)
#define ROUNDED_BOTTOM          (CORNER_BL|CORNER_BR)
#define ROUNDED_LEFT            (CORNER_TL|CORNER_BL)
#define ROUNDED_RIGHT           (CORNER_TR|CORNER_BR)
#define ROUNDED_TOPRIGHT        CORNER_TR
#define ROUNDED_BOTTOMRIGHT     CORNER_BR
#define ROUNDED_TOPLEFT         CORNER_TL
#define ROUNDED_BOTTOMLEFT      CORNER_BL
#define ROUNDED_ALL             (CORNER_TL|CORNER_TR|CORNER_BR|CORNER_BL)

typedef enum
{
    IND_CORNER,
    IND_FONT_COLOR,
    IND_COLORED,
    IND_TINT,
    IND_GLOW,
    IND_DARKEN,
    IND_NONE
} EDefBtnIndicator;

typedef enum
{
    LINE_NONE,
    LINE_SUNKEN,
    LINE_FLAT,
    LINE_DOTS,
    LINE_DASHES
    //LINE_1DOT
} ELine;

typedef enum
{
    TB_NONE,
    TB_LIGHT,
    TB_DARK,
    TB_LIGHT_ALL,
    TB_DARK_ALL
} ETBarBorder;

typedef enum
{
    BORDER_FLAT,
    BORDER_RAISED,
    BORDER_SUNKEN,
    BORDER_LIGHT
} EBorder;

/*
    This whole EShade enum is a complete mess!
    For menubars, we dont blend - so blend is selected, and selected is darken
    For check/radios - we dont blend, so blend is selected, and we dont allow darken
*/
typedef enum
{
    SHADE_NONE,
    SHADE_CUSTOM,
    SHADE_SELECTED,
    SHADE_BLEND_SELECTED,
    SHADE_DARKEN
} EShade;

typedef enum
{
    ECOLOR_BASE,
    ECOLOR_BACKGROUND,
    ECOLOR_DARK,
} EColor;

typedef enum
{
    ROUND_NONE,
    ROUND_SLIGHT,
    ROUND_FULL,
    ROUND_EXTRA,
    ROUND_MAX
} ERound;

typedef enum
{
    SCROLLBAR_KDE,
    SCROLLBAR_WINDOWS,
    SCROLLBAR_PLATINUM,
    SCROLLBAR_NEXT,
    SCROLLBAR_NONE
} EScrollbar;

typedef enum
{
    MO_NONE,
    MO_COLORED,
    MO_COLORED_THICK,
    MO_PLASTIK,
    MO_GLOW
} EMouseOver;

typedef enum
{
    STRIPE_NONE,
    STRIPE_PLAIN,
    STRIPE_DIAGONAL
} EStripe;

typedef enum
{
    SLIDER_PLAIN,
    SLIDER_ROUND,
    SLIDER_PLAIN_ROTATED,
    SLIDER_ROUND_ROTATED,
    SLIDER_TRIANGULAR,
} ESliderStyle;

#define QTC_ROTATED_SLIDER (SLIDER_PLAIN_ROTATED==opts.sliderStyle || SLIDER_ROUND_ROTATED==opts.sliderStyle)

typedef enum
{
    FOCUS_STANDARD,
    FOCUS_RECTANGLE,
    FOCUS_FULL,
    FOCUS_FILLED,
    FOCUS_LINE
} EFocus;

typedef enum
{
    TAB_MO_TOP,
    TAB_MO_BOTTOM,
    TAB_MO_GLOW
} ETabMo;

typedef enum
{
    GT_HORIZ,
    GT_VERT
} EGradType;

#define QTC_FULL_FOCUS     (FOCUS_FULL==opts.focus  || FOCUS_FILLED==opts.focus)

#if defined __cplusplus
typedef enum
{
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_FULL_CENTER,
    ALIGN_RIGHT
} EAlign;
#endif

#ifdef __cplusplus
inline
#else
static
#endif
bool equal(double d1, double d2)
{
    return (fabs(d1 - d2) < 0.0001);
}

#ifdef __cplusplus
struct GradientStop
#else
typedef struct
#endif
{
#ifdef __cplusplus
    GradientStop(double p=0.0, double v=0.0) : pos(p), val(v) { }

    bool operator==(const GradientStop &o) const
    {
        return equal(pos, o.pos) && equal(val, o.val);
    }

    bool operator<(const GradientStop &o) const
    {
        return pos<o.pos || (equal(pos, o.pos) && val<o.val);
    }
#endif

    double pos,
           val;

}
#ifndef __cplusplus
GradientStop
#endif
;

typedef enum
{
    GB_NONE,
    GB_LIGHT,
    GB_3D,
    GB_3D_FULL
} EGradientBorder;

typedef enum
{
    LV_NONE,
    LV_NEW,
    LV_OLD
} ELvLines;

#ifdef __cplusplus
struct GradientStopCont : public std::set<GradientStop>
{
    GradientStopCont fix() const
    {
        GradientStopCont c(*this);
        if(size())
        {
            GradientStopCont::const_iterator   first(c.begin());
            GradientStopCont::reverse_iterator last(c.rbegin());

            if((*first).pos>0.001)
                c.insert(GradientStop(0.0, 1.0));
            if((*last).pos<0.999)
                c.insert(GradientStop(1.0, 1.0));
        }
        return c;                                  
    }
};
struct Gradient
#else
typedef struct
#endif
{
#ifdef __cplusplus
    Gradient() : border(GB_3D) { }

#ifdef QTC_CONFIG_DIALOG
    bool operator==(const Gradient &o) const
    {
        return border==o.border && stops==o.stops;
    }
#endif
#endif
    EGradientBorder  border;
#ifdef __cplusplus
    GradientStopCont stops;
#else
    int              numStops;
    GradientStop     *stops;
#endif
}
#ifndef __cplusplus
Gradient
#endif
;

#define QTC_USE_CUSTOM_SHADES(A) ((A).customShades[0]>0.00001)

#ifdef __cplusplus
typedef std::map<EAppearance, Gradient> GradientCont;
struct Options
#else
typedef struct
#endif
{
    int              contrast,
                     passwordChar,
                     highlightFactor,
                     lighterPopupMenuBgnd,
                     menuDelay,
                     sliderWidth,
                     tabBgnd;
    ERound           round;
    bool             embolden,
                     highlightTab,
                     colorSelTab,
                     roundAllTabs,
                     animatedProgress,
                     fixParentlessDialogs,
                     customMenuTextColor,
                     menubarMouseOver,
                     useHighlightForMenu,
                     shadeMenubarOnlyWhenActive,
                     thinnerMenuItems,
                     thinnerBtns,
                     lvButton,
                     drawStatusBarFrames,
                     fillSlider,
                     roundMbTopOnly,
                     gtkScrollViews,
#ifdef __cplusplus
                     stdSidebarButtons,
                     gtkComboMenus,
                     colorTitlebarOnly,
/*
#else
                     setDialogButtonOrder,
*/
#endif
#if !defined __cplusplus || defined QTC_CONFIG_DIALOG
                     mapKdeIcons,
                     gtkMenuStripe,
#endif
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
                     gtkButtonOrder,
#endif
                     borderMenuitems,
                     colorMenubarMouseOver,
                     darkerBorders,
                     vArrows,
                     xCheck,
                     framelessGroupBoxes,
                     groupBoxLine,
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
                     fadeLines,
#endif
                     crHighlight,
                     crButton,
                     fillProgress,
                     comboSplitter,
                     squareScrollViews,
                     highlightScrollViews,
                     sunkenScrollViews,
                     etchEntry,
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000))
                     titlebarBorder,
#endif
                     colorSliderMouseOver,
                     thinSbarGroove,
                     flatSbarButtons,
                     popupBorder,
                     unifySpinBtns,
                     unifyCombo,
                     unifySpin,
                     borderTab,
                     borderInactiveTab,
                     doubleGtkComboArrow,
                     menuIcons,
                     forceAlternateLvCols;
    ELvLines         lvLines;
    EGradType        bgndGrad,
                     menuBgndGrad;
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000))
    int              titlebarButtons;
    TBCols           titlebarButtonColors;
    ETitleBarIcon    titlebarIcon;
#endif
    EStripe          stripedProgress;
    ESliderStyle     sliderStyle;
    EMouseOver       coloredMouseOver;
    ETBarBorder      toolbarBorders;
    EDefBtnIndicator defBtnIndicator;
    ELine            sliderThumbs,
                     handles,
                     toolbarSeparators,
                     splitters;
    ETabMo           tabMouseOver;
/* NOTE: If add an appearance setting, increase the number of custmo gradients to match! */
    EAppearance      appearance,
                     bgndAppearance,
                     menuBgndAppearance,
                     menubarAppearance,
                     menuitemAppearance,
                     toolbarAppearance,
                     lvAppearance,
                     tabAppearance,
                     activeTabAppearance,
                     sliderAppearance,
#ifdef __cplusplus
                     titlebarAppearance,
                     inactiveTitlebarAppearance,
                     titlebarButtonAppearance,
#endif
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
                     selectionAppearance,
#endif
                     menuStripeAppearance,
                     progressAppearance,
                     progressGrooveAppearance,
                     grooveAppearance,
                     sunkenAppearance,
                     sbarBgndAppearance,
                     sliderFill;
    EShade           shadeSliders,
                     shadeMenubars,
                     menuStripe,
                     shadeCheckRadio,
                     comboBtn,
                     sortedLv;
    EColor           progressGrooveColor;
    EEffect          buttonEffect;
    EScrollbar       scrollbarType;
    EFocus           focus;
    color            customMenubarsColor,
                     customSlidersColor,
                     customMenuNormTextColor,
                     customMenuSelTextColor,
                     customMenuStripeColor,
                     customCheckRadioColor,
                     customComboBtnColor,
                     customSortedLvColor;
#if defined __cplusplus
    EAlign           titlebarAlignment;
#endif
    EShading         shading;
#ifdef __cplusplus
    GradientCont     customGradient;
#else
    Gradient         *customGradient[QTC_NUM_CUSTOM_GRAD];
#endif
    double           customShades[NUM_STD_SHADES];
#ifndef __cplusplus
} Options;
#else
};
#endif

#ifdef QTC_COMMON_FUNCTIONS

#define QTC_MIN(a, b) ((a) < (b) ? (a) : (b))
#define QTC_MAX(a, b) ((b) < (a) ? (a) : (b))


/* Taken from rgb->hsl routines taken from KColor
    Copyright 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
*/
static inline double normalize(double a)
{
    return (a < 0.0 ? 0.0 : a > 1.0 ? 1.0 : a);
}

static inline double mix(double a, double b, double k)
{
    return a + ( ( b - a ) * k );
}

static inline double wrap(double a, double d)
{
    register double r = fmod( a, d );
    return ( r < 0.0 ? d + r : ( r > 0.0 ? r : 0.0 ) );
}

static inline double h2c(double h, double m1, double m2)
{
    h = wrap( h, 6.0 );

    if ( h < 1.0 )
        return mix( m1, m2, h );
    if ( h < 3.0 )
        return m2;
    if ( h < 4.0 )
        return mix( m1, m2, 4.0 - h );
    return m1;
}

static inline void rgbToHsl(double r, double g, double b, double *h, double *s, double *l)
{
    double min=QTC_MIN(QTC_MIN(r, g), b),
           max=QTC_MAX(QTC_MAX(r, g), b);

    *l = 0.5 * (max + min);
    *s = 0.0;
    *h = 0.0;

    if (max != min)
    {
        double delta = max - min;

        if ( *l <= 0.5 )
            *s = delta / ( max + min );
        else
            *s = delta / ( 2.0 - max - min );

        if ( r == max )
            *h = ( g - b ) / delta;
        else if ( g == max )
            *h = 2.0 + ( b - r ) / delta;
        else if ( b == max )
            *h = 4.0 + ( r - g ) / delta;

        *h /= 6.0;
        if ( *h < 0.0 )
            (*h) += 1.0;
    }
}

static inline void hslToRgb(double h, double s, double l, double *r, double *g, double *b)
{
    double m1, m2;

    // TODO h2rgb( h, r, g, b );
    h *= 6.0;

    if ( l <= 0.5 )
        m2 = l * ( 1.0 + s );
    else
        m2 = l + s * ( 1.0 - l );
    m1 = 2.0 * l - m2;

    *r = h2c( h + 2.0, m1, m2 );
    *g = h2c( h,       m1, m2 );
    *b = h2c( h - 2.0, m1, m2 );
}

static void rgbToHsv(double r, double g, double b, double *h, double *s, double *v)
{
    double min=QTC_MIN(QTC_MIN(r, g), b),
           max=QTC_MAX(QTC_MAX(r, g), b),
           delta=max - min;

    *v=max;
    if(max != 0)
        *s=delta / max;
    else
    {
        /* r=g=b=0                  s=0, v is undefined */
        *s=0;
        *h=-1;
        return;
    }
    if(r == max)
        *h=(g - b) / delta;         /* between yellow & magenta */
    else if(g == max)
        *h=2 + (b - r) / delta;     /* between cyan & yellow */
    else
        *h=4 + (r - g) / delta;     /* between magenta & cyan */
    *h *= 60;                       /* degrees */
    if(*h < 0)
        *h += 360;
}

static void hsvToRgb(double *r, double *g, double *b, double h, double s, double v)
{
    if(0==s)
        *r=*g=*b=v;
    else
    {
        int    i;
        double f,
               p;

        h /= 60;                      /* sector 0 to 5 */
        i=(int)floor(h);
        f=h - i;                      /* factorial part of h */
        p=v * (1 - s);
        switch(i)
        {
            case 0:
                *r=v;
                *g=v * (1 - s * (1 - f));
                *b=p;
                break;
            case 1:
                *r=v * (1 - s * f);
                *g=v;
                *b=p;
                break;
            case 2:
                *r=p;
                *g=v;
                *b=v * (1 - s * (1 - f));
                break;
            case 3:
                *r=p;
                *g=v * (1 - s * f);
                *b=v;
                break;
            case 4:
                *r=v * (1 - s * (1 - f));
                *g=p;
                *b=v;
                break;
            /* case 5: */
            default:
                *r=v;
                *g=p;
                *b=v * (1 - s * f);
                break;
        }
    }
}

#ifdef __cplusplus
inline int limit(double c)
{
    return c < 0.0 ? 0 : (c > 255.0  ? 255 : (int)c);
}
#else
inline int limit(double c)
{
    return c < 0.0
               ? 0
               : c > 65535.0
                     ? 65535
                     : (int)c;
}
#endif

#if defined QT_VERSION && (QT_VERSION >= 0x040000) && !defined QTC_QT_ONLY
#include <KDE/KColorUtils>
#define tint(COLA, COLB, FACTOR) KColorUtils::tint((COLA), (COLB), (FACTOR))
#define midColor(COLA, COLB) KColorUtils::mix((COLA), (COLB), 0.5)
#else
#include "colorutils.c"
#ifdef __cplusplus
#define tint(COLA, COLB, FACTOR) ColorUtils_tint(&(COLA), &(COLB), (FACTOR))
#define midColor(COLA, COLB) ColorUtils_mix(&(COLA), &(COLB), 0.5)
#define midColorF(COLA, COLB, FACTOR) ColorUtils_mix(&(COLA), &(COLB), FACTOR-0.5)
#else
#define tint(COLA, COLB, FACTOR) ColorUtils_tint((COLA), (COLB), (FACTOR))
#define midColor(COLA, COLB) ColorUtils_mix((COLA), (COLB), 0.5)
#endif
#endif

#ifdef __cplusplus
static void shade(const Options *opts, const color &ca, color *cb, double k)
#else
static void shade(const Options *opts, const color *ca, color *cb, double k)
#endif
{
    if(equal(k, 1.0))
    {
#ifdef __cplusplus
        *cb=ca;
#else
        cb->red = ca->red;
        cb->green = ca->green;
        cb->blue = ca->blue;
#endif
    }
    else
        switch(opts->shading)
        {
            case SHADING_SIMPLE:
            {
    #ifdef __cplusplus
                int v=(int)(255.0*(k-1.0));

                cb->setRgb(limit(ca.red()+v), limit(ca.green()+v), limit(ca.blue()+v));
    #else
                double v=65535.0*(k-1.0);

                cb->red = limit(ca->red+v);
                cb->green = limit(ca->green+v);
                cb->blue = limit(ca->blue+v);
    #endif
                break;
            }
            case SHADING_HSL:
            {
    #ifdef __cplusplus
                double r(ca.red()/255.0),
                       g(ca.green()/255.0),
                       b(ca.blue()/255.0);
    #else
                double r=ca->red/65535.0,
                       g=ca->green/65535.0,
                       b=ca->blue/65535.0;
    #endif
                double h, s, l;

                rgbToHsl(r, g, b, &h, &s, &l);
                l=normalize(l*k);
                s=normalize(s*k);
                hslToRgb(h, s, l, &r, &g, &b);
    #ifdef __cplusplus
                cb->setRgb(limit(r*255.0), limit(g*255.0), limit(b*255.0));
    #else
                cb->red=limit(r*65535.0);
                cb->green=limit(g*65535.0);
                cb->blue=limit(b*65535.0);
    #endif
                break;
            }
            case SHADING_HSV:
            {
    #ifdef __cplusplus
                double r(ca.red()/255.0),
                       g(ca.green()/255.0),
                       b(ca.blue()/255.0);
    #else
                double r=ca->red/65535.0,
                       g=ca->green/65535.0,
                       b=ca->blue/65535.0;
    #endif
                double h, s, v;

                rgbToHsv(r, g, b, &h, &s, &v);

                v*=k;
                if (v > 1.0)
                {
                    s -= v - 1.0;
                    if (s < 0)
                        s = 0;
                    v = 1.0;
                }
                hsvToRgb(&r, &g, &b, h, s, v);
    #ifdef __cplusplus
                cb->setRgb(limit(r*255.0), limit(g*255.0), limit(b*255.0));
    #else
                cb->red=limit(r*65535.0);
                cb->green=limit(g*65535.0);
                cb->blue=limit(b*65535.0);
    #endif
                break;
            }
            case SHADING_HCY:
            {
    #define QTC_HCY_FACTOR 0.15
    #if defined QT_VERSION && (QT_VERSION >= 0x040000) && !defined QTC_QT_ONLY
                if(k>1.0)
                    *cb=KColorUtils::lighten(ca, (k*(1+QTC_HCY_FACTOR))-1.0, 1.0);
                else
                    *cb=KColorUtils::darken(ca, 1.0-(k*(1-QTC_HCY_FACTOR)), 1.0);
    #elif defined __cplusplus
                if(k>1.0)
                    *cb=ColorUtils_lighten(&ca, (k*(1+QTC_HCY_FACTOR))-1.0, 1.0);
                else
                    *cb=ColorUtils_darken(&ca, 1.0-(k*(1-QTC_HCY_FACTOR)), 1.0);
    #else
                if(k>1.0)
                    *cb=ColorUtils_lighten(ca, (k*(1+QTC_HCY_FACTOR))-1.0, 1.0);
                else
                    *cb=ColorUtils_darken(ca, 1.0-(k*(1-QTC_HCY_FACTOR)), 1.0);
    #endif
            }
        }
#if defined __cplusplus && defined QT_VERSION && (QT_VERSION >= 0x040000)
    cb->setAlpha(ca.alpha());
#endif
#ifndef __cplusplus
    cb->pixel = ca->pixel;
#endif
}

#if (!defined QTC_CONFIG_DIALOG)
static unsigned char checkBounds(int num)
{
    return num < 0   ? 0   :
           num > 255 ? 255 :
                       num;
}

static void adjustPix(unsigned char *data, int numChannels, int w, int h, int stride,
                      int ro, int go, int bo, double shade)
{
    int width=w*numChannels,
        offset=0,
        row,
        r=(int)((ro*shade)+0.5),
        g=(int)((go*shade)+0.5),
        b=(int)((bo*shade)+0.5);

    for(row=0; row<h; ++row)
    {
        int column;

        for(column=0; column<width; column+=numChannels)
        {
            unsigned char source=data[offset+column+1];

#if defined  __cplusplus
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
            /* ARGB */
            data[offset+column+1] = checkBounds(r-source);
            data[offset+column+2] = checkBounds(g-source);
            data[offset+column+3] = checkBounds(b-source);
#else
            /* BGRA */
            data[offset+column] = checkBounds(b-source);
            data[offset+column+1] = checkBounds(g-source);
            data[offset+column+2] = checkBounds(r-source);
#endif
#else
            /* GdkPixbuf is RGBA */
            data[offset+column] = checkBounds(r-source);
            data[offset+column+1] = checkBounds(g-source);
            data[offset+column+2] = checkBounds(b-source);
#endif

        }
        offset+=stride;
    }
}
#endif

static void setupGradient(Gradient *grad, EGradientBorder border, int numStops, ...)
{
    va_list  ap;
    int      i;

    grad->border=border;
#ifndef __cplusplus
    grad->numStops=numStops;
    grad->stops=malloc(sizeof(GradientStop) * numStops*2);
#endif
    va_start(ap, numStops);
    for(i=0; i<numStops; ++i)
    {
        double pos=va_arg(ap, double),
               val=va_arg(ap, double);
#ifdef __cplusplus
        grad->stops.insert(GradientStop(pos, val));
#else
        grad->stops[i].pos=pos;
        grad->stops[i].val=val;
#endif
    }
    va_end(ap);
}
#endif

#if defined QTC_COMMON_FUNCTIONS && !defined QTC_CONFIG_DIALOG

#ifdef __cplusplus
static EAppearance widgetApp(EWidget w, const Options *opts, bool active=true)
#else
static EAppearance widgetApp(EWidget w, const Options *opts)
#endif
{
    switch(w)
    {
        case WIDGET_SB_BGND:
            return opts->sbarBgndAppearance;
        case WIDGET_LISTVIEW_HEADER:
            return opts->lvAppearance;
        case WIDGET_SB_BUTTON:
        case WIDGET_SLIDER:
        case WIDGET_SB_SLIDER:
            return opts->sliderAppearance;
        case WIDGET_FILLED_SLIDER_TROUGH:
            return opts->sliderFill;
        case WIDGET_TAB_TOP:
        case WIDGET_TAB_BOT:
            return opts->tabAppearance;
        case WIDGET_MENU_ITEM:
            return opts->menuitemAppearance;
        case WIDGET_PROGRESSBAR:
            return opts->progressAppearance;
        case WIDGET_PBAR_TROUGH:
            return opts->progressGrooveAppearance;
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
        case WIDGET_SELECTION:
            return opts->selectionAppearance;
#endif
#ifdef __cplusplus
        case WIDGET_MDI_WINDOW:
        case WIDGET_MDI_WINDOW_TITLE:
            return active ? opts->titlebarAppearance : opts->inactiveTitlebarAppearance;
        case WIDGET_MDI_WINDOW_BUTTON:
            return opts->titlebarButtonAppearance;
#endif
        case WIDGET_TROUGH:
        case WIDGET_SLIDER_TROUGH:
            return opts->grooveAppearance;
#ifndef __cplusplus     
        case WIDGET_SPIN_UP:
        case WIDGET_SPIN_DOWN:
#endif
        case WIDGET_SPIN:
            return MODIFY_AGUA(opts->appearance);
        default:
            break;
    }

    return opts->appearance;
};

static const Gradient * getGradient(EAppearance app, const Options *opts)
{
    if(IS_CUSTOM(app))
    {
#ifdef __cplusplus
        GradientCont::const_iterator grad(opts->customGradient.find(app));

        if(grad!=opts->customGradient.end())
            return &((*grad).second);
#else
        Gradient *grad=opts->customGradient[app-APPEARANCE_CUSTOM1];

        if(grad)
            return grad;
#endif
        app=APPEARANCE_RAISED;
    }

    {
    static Gradient stdGradients[QTC_NUM_STD_APP];
    static bool     init=false;

    if(!init)
    {
        setupGradient(&stdGradients[APPEARANCE_RAISED-APPEARANCE_RAISED], GB_3D_FULL,2,0.0,1.0,1.0,1.0);
        setupGradient(&stdGradients[APPEARANCE_DULL_GLASS-APPEARANCE_RAISED], GB_LIGHT,4,0.0,1.05,0.499,0.984,0.5,0.928,1.0,1.0);
        setupGradient(&stdGradients[APPEARANCE_SHINY_GLASS-APPEARANCE_RAISED], GB_LIGHT,4,0.0,1.2,0.499,0.984,0.5,0.9,1.0,1.06);
        setupGradient(&stdGradients[APPEARANCE_AGUA-APPEARANCE_RAISED], GB_NONE, 2,0.0,0.6,1.0,1.1);
        setupGradient(&stdGradients[APPEARANCE_SOFT_GRADIENT-APPEARANCE_RAISED], GB_3D,2,0.0,1.04,1.0,0.98);
        setupGradient(&stdGradients[APPEARANCE_GRADIENT-APPEARANCE_RAISED], GB_3D,2,0.0,1.1,1.0,0.94);
        setupGradient(&stdGradients[APPEARANCE_HARSH_GRADIENT-APPEARANCE_RAISED], GB_3D,2,0.0,1.3,1.0,0.925);
        setupGradient(&stdGradients[APPEARANCE_INVERTED-APPEARANCE_RAISED], GB_3D,2,0.0,0.93,1.0,1.04);
        setupGradient(&stdGradients[APPEARANCE_DARK_INVERTED-APPEARANCE_RAISED], GB_NONE,3,0.0,0.8,0.7,0.95,1.0,1.0);
        setupGradient(&stdGradients[APPEARANCE_SPLIT_GRADIENT-APPEARANCE_RAISED], GB_3D,4,0.0,1.06,0.499,1.004,0.5,0.986,1.0,0.92);
        setupGradient(&stdGradients[APPEARANCE_BEVELLED-APPEARANCE_RAISED], GB_3D,4,0.0,1.05,0.1,1.02,0.9,0.985,1.0,0.94);
        setupGradient(&stdGradients[APPEARANCE_LV_BEVELLED-APPEARANCE_RAISED], GB_3D,3,0.0,1.00,0.85,1.0,1.0,0.90);
        setupGradient(&stdGradients[APPEARANCE_AGUA_MOD-APPEARANCE_RAISED], GB_LIGHT,3,0.0,1.5,0.49,0.85,1.0,1.3);
        setupGradient(&stdGradients[APPEARANCE_LV_AGUA-APPEARANCE_RAISED], GB_NONE,4,0.0,0.98,0.35,0.95,0.4,0.93,1.0,1.15);
        init=true;
    }

    if(APPEARANCE_FLAT==app)
        app=APPEARANCE_RAISED;
    return &stdGradients[app-APPEARANCE_RAISED];
    }

    return 0L; /* Will never happen! */
}

#define QTC_MIN_ROUND_FULL_SIZE     8
#ifdef __cplusplus
#define QTC_MIN_ROUND_EXTRA_SIZE(W) (WIDGET_SPIN==(W) ? 7 : 14)
#else
#define QTC_MIN_ROUND_EXTRA_SIZE(W) (WIDGET_SPIN_UP==(W) || WIDGET_SPIN_DOWN==(W) || WIDGET_SPIN==(W) ? 7 : 14)
#endif
#define QTC_MIN_ROUND_MAX_HEIGHT    17
#define QTC_MIN_ROUND_MAX_WIDTH     32

#if !defined __cplusplus || (defined QT_VERSION && (QT_VERSION >= 0x040000))

#if defined __cplusplus
#define QTC_EXTRA_INNER_RADIUS   3.5
#define QTC_EXTRA_OUTER_RADIUS   4.5
#define QTC_EXTRA_ETCH_RADIUS    5.5
#define QTC_FULL_INNER_RADIUS    1.5
#define QTC_FULL_OUTER_RADIUS    2.5
#define QTC_FULL_ETCH_RADIUS     3.5
#define QTC_SLIGHT_INNER_RADIUS  0.5
#define QTC_SLIGHT_OUTER_RADIUS  1.5
#define QTC_SLIGHT_ETCH_RADIUS   2.5
#else
#define QTC_EXTRA_INNER_RADIUS   4
#define QTC_EXTRA_OUTER_RADIUS   5
#define QTC_EXTRA_ETCH_RADIUS    6
#define QTC_FULL_INNER_RADIUS    2
#define QTC_FULL_OUTER_RADIUS    3
#define QTC_FULL_ETCH_RADIUS     4
#define QTC_SLIGHT_INNER_RADIUS  1
#define QTC_SLIGHT_OUTER_RADIUS  2
#define QTC_SLIGHT_ETCH_RADIUS   3
#endif

#define QTC_MAX_RADIUS_INTERNAL 9.0
#define QTC_MAX_RADIUS_EXTERNAL (QTC_MAX_RADIUS_INTERNAL+2.0)

typedef enum
{
    RADIUS_SELECTION,
    RADIUS_INTERNAL,
    RADIUS_EXTERNAL,
    RADIUS_ETCH
} ERadius;

#define QTC_IS_MAX_ROUND_WIDGET(A) \
            (WIDGET_STD_BUTTON==A || WIDGET_DEF_BUTTON==A /*|| WIDGET_MENU_BUTTON==A*/)

#ifdef __cplusplus
#define QTC_IS_EXTRA_ROUND_WIDGET(A) \
            (A!=WIDGET_MENU_ITEM && A!=WIDGET_TAB_FRAME && A!=WIDGET_PBAR_TROUGH && A!=WIDGET_PROGRESSBAR && \
             A!=WIDGET_MDI_WINDOW && A!=WIDGET_MDI_WINDOW_TITLE)
#else
#define QTC_IS_EXTRA_ROUND_WIDGET(A) \
            (A!=WIDGET_MENU_ITEM && A!=WIDGET_TAB_FRAME && A!=WIDGET_PBAR_TROUGH && A!=WIDGET_PROGRESSBAR)
#endif

#define QTC_CAN_EXTRA_ROUND(MOD) \
            (QTC_IS_EXTRA_ROUND_WIDGET(widget) && \
            (WIDGET_SB_SLIDER==widget || WIDGET_TROUGH==widget || \
            ( ( (w>(QTC_MIN_ROUND_EXTRA_SIZE(widget)+MOD)) || (WIDGET_NO_ETCH_BTN==widget || WIDGET_MENU_BUTTON==widget) ) &&\
                                             (h>(QTC_MIN_ROUND_EXTRA_SIZE(widget)+MOD)))))
#define QTC_CAN_FULL_ROUND(MOD) (w>(QTC_MIN_ROUND_FULL_SIZE+MOD) && h>(QTC_MIN_ROUND_FULL_SIZE+MOD))

// **NOTE** MUST KEEP IN SYNC WITH getRadius/RADIUS_ETCH !!!
ERound getWidgetRound(const Options *opts, int w, int h, EWidget widget)
{
    ERound r=opts->round;

    if((WIDGET_CHECKBOX==widget || WIDGET_FOCUS==widget) && ROUND_NONE!=r)
        r=ROUND_SLIGHT;

#if defined __cplusplus && (defined QT_VERSION && (QT_VERSION >= 0x040000))
    if(WIDGET_MDI_WINDOW_BUTTON==widget && (opts->titlebarButtons&QTC_TITLEBAR_BUTTON_ROUND))
       r=ROUND_MAX;
#endif
    switch(r)
    {
        case ROUND_MAX:
            if(WIDGET_SB_SLIDER==widget || WIDGET_TROUGH==widget
#if defined __cplusplus && (defined QT_VERSION && (QT_VERSION >= 0x040000))
                    || (WIDGET_MDI_WINDOW_BUTTON==widget && (opts->titlebarButtons&QTC_TITLEBAR_BUTTON_ROUND))
#endif
                || (w>(QTC_MIN_ROUND_MAX_WIDTH+2) && h>(QTC_MIN_ROUND_MAX_HEIGHT+2) && QTC_IS_MAX_ROUND_WIDGET(widget)))
            {
                return ROUND_MAX;
            }
        case ROUND_EXTRA:
            if(QTC_CAN_EXTRA_ROUND(2))
                return ROUND_EXTRA;
        case ROUND_FULL:
            if(QTC_CAN_FULL_ROUND(2))
                return ROUND_FULL;
        case ROUND_SLIGHT:
            return ROUND_SLIGHT;
        case ROUND_NONE:
            return ROUND_NONE;
    }
    
    return ROUND_NONE;
}

static double getRadius(const Options *opts, int w, int h, EWidget widget, ERadius rad)
{
    ERound r=opts->round;

    if((WIDGET_CHECKBOX==widget || WIDGET_FOCUS==widget) && ROUND_NONE!=r)
        r=ROUND_SLIGHT;

#if defined __cplusplus && (defined QT_VERSION && (QT_VERSION >= 0x040000))
    if(WIDGET_MDI_WINDOW_BUTTON==widget && (opts->titlebarButtons&QTC_TITLEBAR_BUTTON_ROUND))
       r=ROUND_MAX;
#endif

    switch(rad)
    {
        case RADIUS_SELECTION:
            switch(r)
            {
                case ROUND_MAX:
                case ROUND_EXTRA:
                    if(/* (WIDGET_RUBBER_BAND==widget && w>14 && h>14) || */(w>48 && h>48))
                        return 6.0;
                case ROUND_FULL:
                    if( /*(WIDGET_RUBBER_BAND==widget && w>11 && h>11) || */(w>48 && h>48))
                        return 3.0;
                    if(w>QTC_MIN_ROUND_FULL_SIZE && h>QTC_MIN_ROUND_FULL_SIZE)
                        return QTC_FULL_OUTER_RADIUS;
                case ROUND_SLIGHT:
                    return QTC_SLIGHT_OUTER_RADIUS;
                case ROUND_NONE:
                    return 0;
            }
        case RADIUS_INTERNAL:
            switch(r)
            {
                case ROUND_MAX:
                    if(WIDGET_SB_SLIDER==widget || WIDGET_TROUGH==widget
#if defined __cplusplus && (defined QT_VERSION && (QT_VERSION >= 0x040000))
                       || (WIDGET_MDI_WINDOW_BUTTON==widget && (opts->titlebarButtons&QTC_TITLEBAR_BUTTON_ROUND))
#endif
                       )
                    {
                        double r=(w>h ? h : w)/2.0;
                        return r>QTC_MAX_RADIUS_INTERNAL ? QTC_MAX_RADIUS_INTERNAL : r;
                    }
                    if(w>(QTC_MIN_ROUND_MAX_WIDTH-2) && h>(QTC_MIN_ROUND_MAX_HEIGHT-2) && QTC_IS_MAX_ROUND_WIDGET(widget))
                    {
                        double r=((w>h ? h : w)-2.0)/2.0;
                        return r>9.5 ? 9.5 : r;
                    }
                case ROUND_EXTRA:
                    if(QTC_CAN_EXTRA_ROUND(-2))
                        return QTC_EXTRA_INNER_RADIUS;
                case ROUND_FULL:
                    if(QTC_CAN_FULL_ROUND(-2))
                        return QTC_FULL_INNER_RADIUS;
                case ROUND_SLIGHT:
                    return QTC_SLIGHT_INNER_RADIUS;
                case ROUND_NONE:
                    return 0;
            }
        case RADIUS_EXTERNAL:
            switch(r)
            {
                case ROUND_MAX:
                    if(WIDGET_SB_SLIDER==widget || WIDGET_TROUGH==widget
#if defined __cplusplus && (defined QT_VERSION && (QT_VERSION >= 0x040000))
                       || (WIDGET_MDI_WINDOW_BUTTON==widget && (opts->titlebarButtons&QTC_TITLEBAR_BUTTON_ROUND))
#endif
                      )
                    {
                        double r=(w>h ? h : w)/2.0;
                        return r>QTC_MAX_RADIUS_EXTERNAL ? QTC_MAX_RADIUS_EXTERNAL : r;
                    }
                    if(w>QTC_MIN_ROUND_MAX_WIDTH && h>QTC_MIN_ROUND_MAX_HEIGHT && QTC_IS_MAX_ROUND_WIDGET(widget))
                    {
                        double r=((w>h ? h : w)-2.0)/2.0;
                        return r>10.5 ? 10.5 : r;
                    }
                case ROUND_EXTRA:
                    if(QTC_CAN_EXTRA_ROUND(0))
                        return QTC_EXTRA_OUTER_RADIUS;
                case ROUND_FULL:
                    if(QTC_CAN_FULL_ROUND(0))
                        return QTC_FULL_OUTER_RADIUS;
                case ROUND_SLIGHT:
                    return QTC_SLIGHT_OUTER_RADIUS;
                case ROUND_NONE:
                    return 0;
            }
        case RADIUS_ETCH:
            // **NOTE** MUST KEEP IN SYNC WITH getWidgetRound !!!
            switch(r)
            {
                case ROUND_MAX:
                    if(WIDGET_SB_SLIDER==widget || WIDGET_TROUGH==widget
#if defined __cplusplus && (defined QT_VERSION && (QT_VERSION >= 0x040000))
                       || (WIDGET_MDI_WINDOW_BUTTON==widget && (opts->titlebarButtons&QTC_TITLEBAR_BUTTON_ROUND))
#endif
                      )
                    {
                        double r=(w>h ? h : w)/2.0;
                        return r>QTC_MAX_RADIUS_EXTERNAL ? QTC_MAX_RADIUS_EXTERNAL : r;
                    }
                    if(w>(QTC_MIN_ROUND_MAX_WIDTH+2) && h>(QTC_MIN_ROUND_MAX_HEIGHT+2) && QTC_IS_MAX_ROUND_WIDGET(widget))
                    {
                        double r=((w>h ? h : w)-2.0)/2.0;
                        return r>11.5 ? 11.5 : r;
                    }
                case ROUND_EXTRA:
                    if(QTC_CAN_FULL_ROUND(2))
                        return QTC_EXTRA_ETCH_RADIUS;
                case ROUND_FULL:
                    if(w>(QTC_MIN_ROUND_FULL_SIZE+2) && h>(QTC_MIN_ROUND_FULL_SIZE+2))
                        return QTC_FULL_ETCH_RADIUS;
                case ROUND_SLIGHT:
                    return QTC_SLIGHT_ETCH_RADIUS;
                case ROUND_NONE:
                    return 0;
            }
    }

    return 0;
}
#endif

#endif

#endif

