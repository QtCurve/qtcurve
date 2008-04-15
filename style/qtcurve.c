/*
  QtCurve (C) Craig Drummond, 2003 - 2008 Craig.Drummond@lycos.co.uk

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

/*
 * Menu stripe is disabled for Gtk2, as I'm not sure what todo about menus without icons!
#define QTC_GTK2_MENU_STRIPE
#define QTC_GTK2_MENU_STRIPE_HACK_MENU
*/

#include <gmodule.h>
#include <gtk/gtk.h>
#include <gtk/gtkstyle.h>
#include <gdk/gdktypes.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#define QTC_COMMON_FUNCTIONS
#include "qtcurve.h"
#include "qt_settings.c"
#include "animation.c"
#include "pixmaps.h"
#include "config.h"
#include <cairo.h>

static struct
{
    GdkColor background[TOTAL_SHADES+1],
             button[TOTAL_SHADES+1],
             *slider,
             *defbtn,
             *mouseover,
             menubar[TOTAL_SHADES+1],
             menuitem[TOTAL_SHADES+1],
             *check_radio;
} qtcPalette;

#define M_PI 3.14159265358979323846
#define QTC_CAIRO_COL(A) (A).red/65535.0, (A).green/65535.0, (A).blue/65535.0

#define QTC_CAIRO_BEGIN \
    { \
    cairo_t *cr=(cairo_t*)gdk_cairo_create(window); \
    setCairoClipping(cr, area, NULL); \
    cairo_set_line_width(cr, 1.0);

#define QTC_CAIRO_END \
    cairo_destroy(cr); \
    }

/*
 * Disabled, for the moment, due to not working very well...
 *    1. Seems to mouse over for the whole toolbar
 *    2. When a toolbar is made floating, the mouse over effect does not turn "off" :-(
#define QTC_MOUSEOVER_HANDLES
*/

#define QT_STYLE style
#define WIDGET_TYPE_NAME(xx) (widget && !strcmp(g_type_name (G_TYPE_FROM_INSTANCE(widget)), (xx)))
#define FN_CHECK g_return_if_fail(GTK_IS_STYLE(style)); g_return_if_fail(window != NULL);

#ifndef GTK_IS_COMBO_BOX_ENTRY
#define GTK_IS_COMBO_BOX_ENTRY(x) 0
#endif
#ifndef GTK_IS_COMBO_BOX
#define GTK_IS_COMBO_BOX(x) 0
#endif

#define SLIDER_TROUGH_SIZE 5

static void gtkDrawBox(GtkStyle *style, GdkWindow *window, GtkStateType state,
                       GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget,
                       const gchar *detail, gint x, gint y, gint width, gint height);

#ifdef QTC_DEBUG
static void dumpChildren(GtkWidget *widget, int level)
{
    if(level<5)
    {
        GList *child=GTK_BOX(widget)->children;

        for(; child; child=child->next)
        {
            GtkBoxChild *boxChild=(GtkBoxChild *)child->data;

            printf(":[%d]%s:", level, gtk_type_name(GTK_WIDGET_TYPE(boxChild->widget)));
            if(GTK_IS_BOX(boxChild->widget))
                dumpChildren(boxChild->widget, ++level);
        }
    }
}

static void debugDisplayWidget(GtkWidget *widget, int level)
{
    if(level>=0)
    {
        printf("%s(%s)[%x] ", widget ? gtk_type_name(GTK_WIDGET_TYPE(widget)) : "NULL",
               widget && widget->name ? widget->name : "NULL", (int)widget);
        /*if(widget)
            printf("[%d, %dx%d : %d,%d , %0X] ", widget->state, widget->allocation.x,
                   widget->allocation.y,
                   widget->allocation.width, widget->allocation.height, widget->window);*/
#ifdef QTC_GTK2_MENU_STRIPE
        if(GTK_IS_WINDOW(widget))
        {
            printf("{%X}", (int)GTK_WINDOW(widget)->transient_parent);
            if(GTK_WINDOW(widget)->transient_parent && GTK_BIN(GTK_WINDOW(widget)->transient_parent)->child)
            {
                printf("/%s(%s)[%x]/",
                          gtk_type_name(GTK_WIDGET_TYPE(GTK_BIN(GTK_WINDOW(widget)->transient_parent)->child)),
                          GTK_BIN(GTK_WINDOW(widget)->transient_parent)->child->name
                             ? GTK_BIN(GTK_WINDOW(widget)->transient_parent)->child->name : "NULL",
                         (int)GTK_BIN(GTK_WINDOW(widget)->transient_parent)->child);
                if(GTK_IS_BOX(GTK_BIN(GTK_WINDOW(widget)->transient_parent)->child))
                    dumpChildren(GTK_BIN(GTK_WINDOW(widget)->transient_parent)->child, 0);
            }
        }
#endif
        if(widget && widget->parent)
            debugDisplayWidget(widget->parent, --level);
        else
            printf("\n");
    }
    else
        printf("\n");
}
#endif

typedef struct
{
    int          id;
    GdkRectangle rect;
} QtCTab;

typedef struct
{
    GdkColor col;
    EPixmap  pix;
    double   shade;
} QtCPixKey;

static GtkStyleClass  *parent_class=NULL;
static Options        opts;
static GtkRequisition defaultOptionIndicatorSize    = { 6, 13 };
static GtkBorder      defaultOptionIndicatorSpacing = { 7, 5, 1, 1 };
static GHashTable     *tabHashTable                 = NULL;
#ifdef QTC_MOUSEOVER_HANDLES
static GHashTable     *toolbarHandleHashTable       = NULL;
#endif
static GHashTable     *menubarHashTable             = NULL;
static GCache         *pixbufCache                  = NULL;

#define DETAIL(xx) ((detail) &&(!strcmp(xx, detail)))
#define DETAILHAS(xx) ((detail) && (strstr(detail, xx)))

#define QTC_PANED "paned-qtc"
#define QTC_CHECKBOX "checkbox-qtc"
#define IS_QTC_PANED DETAIL(QTC_PANED)

static void clip_to_region(cairo_t *cr, GdkRegion *region)
{
    int          numRects;
    GdkRectangle *rects;

    gdk_region_get_rectangles(region, &rects, &numRects);

    cairo_new_path(cr);
    while(numRects--)
    {
        GdkRectangle *rect=&(rects[numRects]);
        cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
    }
    cairo_clip(cr);

    g_free(rects);
}

void setCairoClipping(cairo_t *cr, GdkRectangle *area, GdkRegion *region)
{
    cairo_save(cr);
    if(area)
    {
        cairo_rectangle(cr, area->x, area->y, area->width, area->height);
        cairo_clip(cr);
    }
    else if(region)
        clip_to_region(cr, region);
    cairo_new_path(cr);
}

#define unsetCairoClipping(A) cairo_restore(A)

static void drawHLine(cairo_t *cr, double r, double g, double b, double a, int x, int y, int w)
{
    cairo_new_path(cr);
    cairo_set_source_rgba(cr, r, g, b, a);
    cairo_move_to(cr, x, y+0.5);
    cairo_line_to(cr, x+w, y+0.5);
    cairo_stroke(cr);
}

static void drawVLine(cairo_t *cr, double r, double g, double b, double a, int x, int y, int h)
{
    cairo_new_path(cr);
    cairo_set_source_rgba(cr, r, g, b, a);
    cairo_move_to(cr, x+0.5, y);
    cairo_line_to(cr, x+0.5, y+h);
    cairo_stroke(cr);
}

static gboolean useButtonColor(const gchar *detail)
{
    return detail &&( 0==strcmp(detail, "optionmenu") ||
                       0==strcmp(detail, "button") ||
                       0==strcmp(detail, QTC_CHECKBOX) ||
                       0==strcmp(detail, "buttondefault") ||
                       0==strcmp(detail, "togglebuttondefault") ||
                       0==strcmp(detail, "togglebutton") ||
                       0==strcmp(detail, "hscale") ||
                       0==strcmp(detail, "vscale") ||
                       0==strcmp(detail, "spinbutton") ||
                       0==strcmp(detail, "spinbutton_up") ||
                       0==strcmp(detail, "spinbutton_down") ||
                       0==strcmp(detail, "slider") ||
                       0==strcmp(detail, "qtc-slider") ||
                       0==strcmp(detail, "vscrollbar") ||
                       0==strcmp(detail, "hscrollbar") ||
                       0==strcmp(detail, "stepper") ||
                       0==strcmp(detail, QTC_PANED) );
}

#define QTC_COL_EQ(A, B)(abs(A-B)<(3<<8))

#define QT_CUSTOM_COLOR_BUTTON(style) \
    (style && \
    !(QTC_COL_EQ(qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW].red,(style->bg[GTK_STATE_NORMAL].red)) && \
      QTC_COL_EQ(qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW].green,(style->bg[GTK_STATE_NORMAL].green)) && \
      QTC_COL_EQ(qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW].blue,(style->bg[GTK_STATE_NORMAL].blue))))

#define QTC_EQUAL_COLOR(A, B) \
   (QTC_COL_EQ(A.red, B.red) && QTC_COL_EQ(A.green, B.green) && QTC_COL_EQ(A.blue, B.blue))

static void shadeColors(GdkColor *base, GdkColor *vals)
{
    QTC_SHADES

    int      i;
    gboolean useCustom=0L!=opts.customShades;

    for(i=0; i<NUM_STD_SHADES; ++i)
        shade(base, &vals[i], useCustom ? opts.customShades[i] : QTC_SHADE(opts.contrast, i));
    shade(base, &vals[SHADE_ORIG_HIGHLIGHT], opts.highlightFactor);
    shade(&vals[4], &vals[SHADE_4_HIGHLIGHT], opts.highlightFactor);
    shade(&vals[2], &vals[SHADE_2_HIGHLIGHT], opts.highlightFactor);
    vals[ORIGINAL_SHADE]=*base;
}

static GdkGC * realizeColors(GtkStyle *style, GdkColor *color)
{
    GdkGCValues gc_values;

    gdk_colormap_alloc_color(style->colormap, color, FALSE, TRUE);
    gc_values.foreground = *color;

    return gtk_gc_get(style->depth, style->colormap, &gc_values, GDK_GC_FOREGROUND);
}

#define QTC_SET_BTN_COLS(SCROLLBAR, SCALE, LISTVIEW) \
{ \
    if(SCROLLBAR || SCALE) \
        btn_colors=SHADE_NONE!=opts.shadeSliders \
                    ? qtcPalette.slider \
                    : qtcPalette.button; \
    else if(LISTVIEW) \
        btn_colors=qtcPalette.background; \
    else \
        btn_colors=qtcPalette.button; \
}

static void generateMidColor(GdkColor *a, GdkColor *b, GdkColor *mid, double factor)
{
    *mid=*b;

    mid->red=(a->red+limit(b->red*factor))>>1;
    mid->green=(a->green+limit(b->green*factor))>>1;
    mid->blue=(a->blue+limit(b->blue*factor))>>1;
}

static void tintColor(GdkColor *a, GdkColor *b, GdkColor *mid, double factor)
{
    *mid=*b;

    mid->red=limit((a->red+(factor*b->red))/(1+factor));
    mid->green=limit((a->green+(factor*b->green))/(1+factor));
    mid->blue=limit((a->blue+(factor*b->blue))/(1+factor));
}

static GdkColor * getCellCol(GdkColor *std, const gchar *detail)
{
    static GdkColor shaded;

    if(!qtSettings.shadeSortedList || !strstr(detail, "_sorted"))
        return std;

    shaded=*std;

    if(QTC_IS_BLACK(shaded))
        shaded.red=shaded.green=shaded.blue=55<<8;
    else
    {
        double r=shaded.red/65535.0,
               g=shaded.green/65535.0,
               b=shaded.blue/65535.0;
        double h, s, v;

        rgbToHsv(r, g, b, &h, &s, &v);

        if (v > 175.0/255.0)
            v*=100.0/104.0;
        else
            v*=120.0/100.0;

        if (v > 1.0)
        {
            s -= v - 1.0;
            if (s < 0)
                s = 0;
            v = 1.0;
        }

        hsvToRgb(&r, &g, &b, h, s, v);
        shaded.red=r*65535.0;
        shaded.green=g*65535.0;
        shaded.blue=b*65535.0;
    }
    return &shaded;
}

#define QTC_ARROW_STATE(state) (GTK_STATE_INSENSITIVE==state ? state : GTK_STATE_NORMAL)
/* (GTK_STATE_ACTIVE==state ? GTK_STATE_NORMAL : state) */

gboolean reverseLayout(GtkWidget *widget)
{
    return widget
        ? GTK_TEXT_DIR_RTL==gtk_widget_get_direction(widget)
        : FALSE;
}

static gboolean isOnToolbar(GtkWidget *widget, gboolean *horiz, int level)
{
    if(widget)
    {
        if(GTK_IS_TOOLBAR(widget))
        {
            if(horiz)
                *horiz=GTK_ORIENTATION_HORIZONTAL==gtk_toolbar_get_orientation(GTK_TOOLBAR(widget));
            return TRUE;
        }
        else if(level<4)
            return isOnToolbar(widget->parent, horiz, ++level);
    }

    return FALSE;
}

static gboolean isOnHandlebox(GtkWidget *widget, gboolean *horiz, int level)
{
    if(widget)
    {
        if(GTK_IS_HANDLE_BOX(widget))
        {
            if(horiz)
            {
                GtkPositionType pos=gtk_handle_box_get_handle_position(GTK_HANDLE_BOX(widget));
                *horiz=GTK_POS_LEFT==pos || GTK_POS_RIGHT==pos;
            }
            return TRUE;
        }
        else if(level<4)
            return isOnHandlebox(widget->parent, horiz, ++level);
    }

    return FALSE;
}

static gboolean isButtonOnToolbar(GtkWidget *widget, gboolean *horiz)
{
    return (widget && widget->parent && GTK_IS_BUTTON(widget))
               ? isOnToolbar(widget->parent, horiz, 0)
               : FALSE;
}

static gboolean isButtonOnHandlebox(GtkWidget *widget, gboolean *horiz)
{
    return (widget && widget->parent && GTK_IS_BUTTON(widget))
               ? isOnHandlebox(widget->parent, horiz, 0)
               : FALSE;
}

static gboolean isOnStatusBar(GtkWidget *widget, int level)
{
    if(widget->parent)
        if(GTK_IS_STATUSBAR(widget->parent))
            return TRUE;
        else if(level<4)
            return isOnStatusBar(widget->parent, ++level);

    return FALSE;
}

static gboolean isList(GtkWidget *widget)
{
    return widget &&
           (GTK_IS_CLIST(widget) ||
            GTK_IS_LIST(widget) ||
            GTK_IS_TREE_VIEW(widget) ||
#ifdef GTK_ENABLE_BROKEN
            GTK_IS_TREE(widget) ||
#endif
            GTK_IS_CTREE(widget));
}

static gboolean isListViewHeader(GtkWidget *widget)
{
    return widget && GTK_IS_BUTTON(widget) && widget->parent && isList(widget->parent);
}

static gboolean isComboBoxButton(GtkWidget *widget)
{
    return widget && GTK_IS_BUTTON(widget) && widget->parent &&
           (GTK_IS_COMBO_BOX_ENTRY(widget->parent) || GTK_IS_COMBO(widget->parent));
}

static gboolean isComboBox(GtkWidget *widget)
{
    return widget && GTK_IS_BUTTON(widget) && widget->parent &&
           !GTK_IS_COMBO_BOX_ENTRY(widget->parent) &&
           (GTK_IS_COMBO_BOX(widget->parent) || GTK_IS_COMBO(widget->parent));
}

static gboolean isComboBoxEntry(GtkWidget *widget)
{
    return widget && GTK_IS_ENTRY(widget) && widget->parent &&
           (GTK_IS_COMBO_BOX_ENTRY(widget->parent) || GTK_IS_COMBO(widget->parent));
}

static gboolean isComboBoxEntryButton(GtkWidget *widget)
{
    return widget && widget->parent && GTK_IS_TOGGLE_BUTTON(widget) &&
           GTK_IS_COMBO_BOX_ENTRY(widget->parent);
}

static gboolean isSwtComboBoxEntry(GtkWidget *widget)
{
    return GTK_APP_JAVA_SWT==qtSettings.app &&
           isComboBoxEntry(widget) &&
           widget->parent->parent && 0==strcmp(gtk_type_name(GTK_WIDGET_TYPE(widget->parent->parent)), "SwtFixed");
}

static gboolean isGimpCombo(GtkWidget *widget)
{
    return GTK_APP_GIMP==qtSettings.app &&
           widget && widget->parent && GTK_IS_TOGGLE_BUTTON(widget) &&
           0==strcmp(gtk_type_name(GTK_WIDGET_TYPE(widget->parent)), "GimpEnumComboBox");
}

static gboolean isOnComboEntry(GtkWidget *w, int level)
{
    if(w)
    {
        if(GTK_IS_COMBO_BOX_ENTRY(w))
            return TRUE;
        else if(level<4)
            return isOnComboEntry(w->parent, ++level);
    }
    return FALSE;
}

static gboolean isOnCombo(GtkWidget *w, int level)
{
    if(w)
    {
        if(GTK_IS_COMBO_BOX(w))
            return TRUE;
        else if(level<4)
            return isOnCombo(w->parent, ++level);
    }
    return FALSE;
}

static gboolean isActiveCombo(GtkWidget *widget)
{
    if(GTK_IS_OPTION_MENU(widget))
    {
        GtkWidget *menu=gtk_option_menu_get_menu(GTK_OPTION_MENU(widget));
        if(menu && GTK_WIDGET_VISIBLE(menu) && GTK_WIDGET_REALIZED(menu))
            return TRUE;
    }
    return FALSE;
}

static gboolean isSpinButton(GtkWidget *widget)
{
    return widget && GTK_IS_SPIN_BUTTON(widget);
}

static gboolean isStatusBarFrame(GtkWidget *widget)
{
    return widget && widget->parent && GTK_IS_FRAME(widget) && GTK_IS_STATUSBAR(widget->parent);
}

static GtkMenuBar * isMenubar(GtkWidget *w, int level)
{
    if(w)
    {
        if(GTK_IS_MENU_BAR(w))
            return (GtkMenuBar*)w;
        else if(level<3)
            return isMenubar(w->parent, level++);
    }

    return NULL;
}

static gboolean isMenuitem(GtkWidget *w, int level)
{
    if(w)
    {
        if(GTK_IS_MENU_ITEM(w))
            return TRUE;
        else if(level<3)
            return isMenuitem(w->parent, level++);
    }

    return FALSE;
}

#define QTC_IS_MENU_ITEM(WIDGET) isMenuitem(WIDGET, 0)

static gboolean isOnButton(GtkWidget *w, int level, gboolean *def)
{
    if(w)
    {
        if(GTK_IS_BUTTON(w) &&(!(GTK_IS_RADIO_BUTTON(w) || GTK_IS_CHECK_BUTTON(w) ||
                               GTK_IS_OPTION_MENU(w))))
        {
            if(def)
                *def=GTK_WIDGET_HAS_DEFAULT(w);
            return TRUE;
        }
        else if(level<3)
            return isOnButton(w->parent, level++, def);
    }

    return FALSE;
}

static void optionMenuGetProps(GtkWidget *widget, GtkRequisition *indicator_size,
                               GtkBorder *indicator_spacing)
{
    GtkRequisition *tmp_size = NULL;
    GtkBorder      *tmp_spacing = NULL;

    if(widget)
        gtk_widget_style_get(widget, "indicator_size", &tmp_size, "indicator_spacing", &tmp_spacing,
                             NULL);
    *indicator_size= tmp_size ? *tmp_size : defaultOptionIndicatorSize;
    *indicator_spacing = tmp_spacing ? *tmp_spacing : defaultOptionIndicatorSpacing;

    if (tmp_size)
        gtk_requisition_free(tmp_size);
    if (tmp_spacing)
        gtk_border_free(tmp_spacing);
}

typedef enum
{
    QTC_STEPPER_A,
    QTC_STEPPER_B,
    QTC_STEPPER_C,
    QTC_STEPPER_D,
    QTC_STEPPER_NONE
} EStepper;

static EStepper getStepper(GtkWidget *widget, int x, int y, int width, int height)
{
    if(GTK_IS_RANGE(widget))
    {
        GdkRectangle   tmp;
        GdkRectangle   check_rectangle,
                       stepper;
        GtkOrientation orientation=GTK_RANGE(widget)->orientation;

        stepper.x=x;
        stepper.y=y;
        stepper.width=width;
        stepper.height=height;
        check_rectangle.x      = widget->allocation.x;
        check_rectangle.y      = widget->allocation.y;
        check_rectangle.width  = stepper.width;
        check_rectangle.height = stepper.height;

        if (-1==widget->allocation.x && -1==widget->allocation.y)
            return QTC_STEPPER_NONE;

        if (gdk_rectangle_intersect(&stepper, &check_rectangle, &tmp))
            return QTC_STEPPER_A;

        if (GTK_ORIENTATION_HORIZONTAL==orientation)
            check_rectangle.x = widget->allocation.x + stepper.width;
        else
            check_rectangle.y = widget->allocation.y + stepper.height;

        if (gdk_rectangle_intersect(&stepper, &check_rectangle, &tmp))
            return QTC_STEPPER_B;

        if (GTK_ORIENTATION_HORIZONTAL==orientation)
            check_rectangle.x = widget->allocation.x + widget->allocation.width - (stepper.width * 2);
        else
            check_rectangle.y = widget->allocation.y + widget->allocation.height - (stepper.height * 2);

        if (gdk_rectangle_intersect(&stepper, &check_rectangle, &tmp))
            return QTC_STEPPER_C;

        if (GTK_ORIENTATION_HORIZONTAL==orientation)
            check_rectangle.x = widget->allocation.x + widget->allocation.width - stepper.width;
        else
            check_rectangle.y = widget->allocation.y + widget->allocation.height - stepper.height;

        if (gdk_rectangle_intersect(&stepper, &check_rectangle, &tmp))
            return QTC_STEPPER_D;
    }

    return QTC_STEPPER_NONE;
}

static int getFill(GtkStateType state, gboolean set/*, gboolean allow_mouse_over_set*/)
{
    return GTK_STATE_INSENSITIVE==state
               ? ORIGINAL_SHADE
               : GTK_STATE_PRELIGHT==state
                   ? set /*&& allow_mouse_over_set*/
                       ? SHADE_4_HIGHLIGHT
                       : SHADE_ORIG_HIGHLIGHT
                   : set || GTK_STATE_ACTIVE==state
                       ? 4
                       : ORIGINAL_SHADE;
}

static int getRound(const char *detail, GtkWidget *widget, int x, int y, int width, int height, gboolean rev)
{
    if(detail)
    {
        if(0==strcmp(detail, "slider"))
            return
#ifndef QTC_SIMPLE_SCROLLBARS
                    SCROLLBAR_NONE==opts.scrollbarType ? ROUNDED_ALL :
#endif
                    ROUNDED_NONE;
        else if(0==strcmp(detail, "qtc-slider") ||
                0==strcmp(detail, "splitter") || 0==strcmp(detail, "optionmenu")  ||
                0==strcmp(detail, "togglebutton") || 0==strcmp(detail, "hscale") ||
                0==strcmp(detail, "vscale") || 0==strcmp(detail, QTC_CHECKBOX)
                /* || 0==strcmp(detail, "paned") || 0==strcmp(detail, QTC_PANED)*/ )
            return ROUNDED_ALL;
        else if(0==strcmp(detail, "spinbutton_up"))
            return rev ? ROUNDED_TOPLEFT : ROUNDED_TOPRIGHT;
        else if(0==strcmp(detail, "spinbutton_down"))
            return rev ? ROUNDED_BOTTOMLEFT : ROUNDED_BOTTOMRIGHT;
        else if(0==strcmp(detail, "vscrollbar") || 0==strcmp(detail, "hscrollbar") ||
                0==strcmp(detail, "stepper"))
        {
            switch(getStepper(widget, x, y, width, height))
            {
                case QTC_STEPPER_A:
                    return 'h'==detail[0] ? ROUNDED_LEFT : ROUNDED_TOP;
                case QTC_STEPPER_D:
                    return 'v'==detail[0]  ? ROUNDED_BOTTOM : ROUNDED_RIGHT;
                default:
                    return ROUNDED_NONE;
            }
        }
        else if(0==strcmp(detail, "button"))
        {
            if(isListViewHeader(widget))
                return ROUNDED_NONE;
            else if(isComboBoxButton(widget))
                return rev ? ROUNDED_LEFT : ROUNDED_RIGHT;
            else
                return ROUNDED_ALL;
        }
    }

    return ROUNDED_NONE;
}

static gboolean isHorizontalProgressbar(GtkWidget *widget)
{
    if(!widget || isMozilla() ||!GTK_IS_PROGRESS_BAR(widget))
        return TRUE;

    switch(GTK_PROGRESS_BAR(widget)->orientation)
    {
        default:
        case GTK_PROGRESS_LEFT_TO_RIGHT:
        case GTK_PROGRESS_RIGHT_TO_LEFT:
            return TRUE;
        case GTK_PROGRESS_BOTTOM_TO_TOP:
        case GTK_PROGRESS_TOP_TO_BOTTOM:
            return FALSE;
    }
}

static gboolean isComboboxPopupWindow(GtkWidget *widget)
{
    return widget && widget->name && GTK_IS_WINDOW(widget) &&
           0==strcmp(widget->name, "gtk-combobox-popup-window");
}

static gboolean isComboList(GtkWidget *widget)
{
    return widget && widget->parent && /*GTK_IS_FRAME(widget) && */isComboboxPopupWindow(widget->parent);
}

#ifdef QTC_GTK2_MENU_STRIPE
static gboolean isComboMenu(GtkWidget *widget)
{
    if(widget && widget->name && GTK_IS_MENU(widget) && 0==strcmp(widget->name, "gtk-combobox-popup-menu"))
        return TRUE;
    else
    {
        GtkWidget *top=gtk_widget_get_toplevel(widget);

        return top && (isComboboxPopupWindow(GTK_BIN(top)->child) ||
                       GTK_IS_DIALOG(top) || /* Dialogs should not have menus! */
                       (GTK_IS_WINDOW(top) && GTK_WINDOW(top)->transient_parent &&
                        GTK_BIN(GTK_WINDOW(top)->transient_parent)->child &&
                        isComboMenu(GTK_BIN(GTK_WINDOW(top)->transient_parent)->child)));
    }
}
#endif

#if 0
static gboolean isComboFrame(GtkWidget *widget)
{
    return !GTK_IS_COMBO_BOX_ENTRY(widget) && GTK_IS_FRAME(widget) && widget->parent && GTK_IS_COMBO_BOX(widget->parent);
}
#endif

static int progressbarRound(GtkWidget *widget, gboolean rev)
{
    if(!widget || !GTK_IS_PROGRESS_BAR(widget) || isMozilla() ||
       equal(gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(widget)), 100.0))
        return ROUNDED_NONE;

    switch(GTK_PROGRESS_BAR(widget)->orientation)
    {
        default:
        case GTK_PROGRESS_LEFT_TO_RIGHT:
            return rev ? ROUNDED_LEFT : ROUNDED_RIGHT;
        case GTK_PROGRESS_RIGHT_TO_LEFT:
            return rev ? ROUNDED_RIGHT : ROUNDED_LEFT;
        case GTK_PROGRESS_BOTTOM_TO_TOP:
            return ROUNDED_TOP;
        case GTK_PROGRESS_TOP_TO_BOTTOM:
            return ROUNDED_BOTTOM;
    }
}

static gboolean isMozillaWidget(GtkWidget *widget)
{
    return isMozilla() && widget && widget->parent && widget->parent->parent &&
           GTK_IS_FIXED(widget->parent) && GTK_IS_WINDOW(widget->parent->parent);
}

static void setState(GtkWidget *widget, GtkStateType *state, gboolean *btn_down)
{
    if(isMozillaWidget(widget))
    {
        if(GTK_STATE_INSENSITIVE==*state)
        {
            *state=GTK_STATE_NORMAL;
            if(btn_down)
                *btn_down=FALSE;
        }
    }
    else
    {
#define BTN_SIZE 15

        GtkRange *range=GTK_RANGE(widget);
        gboolean horiz=range->orientation,
                    disableLeft=FALSE,
                    disableRight=FALSE;
        int      max=horiz ? range->range_rect.height
                            : range->range_rect.width,
                    leftBtns=0,
                    rightBtns=0;

        switch(opts.scrollbarType)
        {
            case SCROLLBAR_KDE:
                leftBtns=BTN_SIZE;
                rightBtns=BTN_SIZE*2;
                break;
            default:
            case SCROLLBAR_WINDOWS:
                leftBtns=BTN_SIZE;
                rightBtns=BTN_SIZE;
                break;
            case SCROLLBAR_PLATINUM:
                leftBtns=0;
                rightBtns=BTN_SIZE*2;
                break;
            case SCROLLBAR_NEXT:
                leftBtns=BTN_SIZE*2;
                rightBtns=0;
                break;
            case SCROLLBAR_NONE:
                break;
        }

        if(range->slider_start==leftBtns)
            disableLeft=TRUE;
        if(range->slider_end+rightBtns==max)
            disableRight=TRUE;

        if(disableLeft && disableRight)
            *state=GTK_STATE_INSENSITIVE;
        else if(GTK_STATE_INSENSITIVE==*state)
        {
            *state=GTK_STATE_NORMAL;
            if(btn_down)
                *btn_down=FALSE;
        }
    }
}

static void drawAreaColor(cairo_t *cr, GdkRectangle *area, GdkRegion *region, GdkColor *col, gint x, gint y, gint width, gint height)
{
    setCairoClipping(cr, area, region);
    cairo_rectangle(cr, x, y, width, height);
    cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col));
    cairo_fill(cr);
    unsetCairoClipping(cr);
}

static GdkColor * getParentBgCol(GtkWidget *widget)
{
    if(GTK_IS_SCROLLBAR(widget))
        widget=widget->parent;

    return widget && widget->parent && widget->parent->style
               ? &(widget->parent->style->bg[widget->parent->state])
               : NULL;
}

static void drawBgnd(cairo_t *cr, GdkColor *col, GtkWidget *widget,
                     GdkRectangle *area, int x, int y, int width, int height)
{
    GdkColor *parent_col=getParentBgCol(widget),
             *bgnd_col=parent_col ? parent_col : col;

    drawAreaColor(cr, area, NULL, parent_col ? parent_col : col, x, y, width, height);
}

static void drawAreaModColor(cairo_t *cr, GdkRectangle *area,
                             GdkRegion *region, GdkColor *orig, double mod, gint x, gint y,
                             gint width, gint height)
{
    GdkColor modified;

    if(!equal(mod, 0.0))
        shade(orig, &modified, mod);
    else
        modified=*orig;

    drawAreaColor(cr, area, region, &modified, x, y, width, height);
}

#define drawAreaMod(/*cairo_t *       */  cr, \
                    /* GtkStyle *     */  style, \
                    /* GtkStateType   */  state, \
                    /* GdkRectangle * */  area, \
                    /* GdkRegion *    */  region, \
                    /* double         */  mod, \
                    /* gint           */  x, \
                    /* gint           */  y, \
                    /* gint           */  width, \
                    /* gint           */  height) \
    drawAreaModColor(cr, area, region, &style->bg[state], mod, x, y, width, height)

static void constrainRect(GdkRectangle *rect, GdkRectangle *con)
{
    if(rect && con)
    {
        if(rect->x<con->x)
        {
            rect->width-=(con->x-rect->x);
            rect->x=con->x;
        }
        if(rect->y<con->y)
        {
            rect->height-=(rect->y-con->y);
            rect->y=con->y;
        }
        if((rect->x+rect->width)>(con->x+con->width))
            rect->width-=(rect->x+rect->width)-(con->x+con->width);
        if((rect->y+rect->height)>(con->y+con->height))
            rect->height-=(rect->y+rect->height)-(con->y+con->height);
    }
}

static QtCTab * lookupTabHash(void *hash, gboolean create)
{
    QtCTab *rv=NULL;

    if(!tabHashTable)
        tabHashTable=g_hash_table_new(g_direct_hash, g_direct_equal);

    rv=(QtCTab *)g_hash_table_lookup(tabHashTable, hash);

    if(!rv && create)
    {
        rv=(QtCTab *)malloc(sizeof(QtCTab));
        rv->id=rv->rect.x=rv->rect.y=rv->rect.width=rv->rect.height=-1;
        g_hash_table_insert(tabHashTable, hash, rv);
        rv=g_hash_table_lookup(tabHashTable, hash);
    }

    return rv;
}

/* This function is called whenever the mouse moves within a tab - but the whole tab widget! */
static gboolean tabEvent(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    if(GDK_MOTION_NOTIFY==event->type)
    {
        static int last_x=-100, last_y=-100;

        if(abs(last_x-event->motion.x_root)>4 || abs(last_y-event->motion.y_root)>4)
        {
            last_x=event->motion.x_root;
            last_y=event->motion.y_root;

            GtkNotebook *notebook=GTK_NOTEBOOK(widget);

            if(notebook)
            {
                /* TODO! check if mouse is over tab portion! */
                /* Find tab that mouse is currently over...*/
                QtCTab *prevTab=lookupTabHash(widget, TRUE),
                       currentTab;
                int    numChildren=g_list_length(notebook->children),
                       i,
                       nx, ny;

                currentTab.id=currentTab.rect.x=currentTab.rect.y=
                currentTab.rect.width=currentTab.rect.height=-1;
                gdk_window_get_origin(GTK_WIDGET(notebook)->window, &nx, &ny);
                for (i = 0; i < numChildren; i++ )
                {
                    GtkWidget *page=gtk_notebook_get_nth_page(notebook, i),
                              *tabLabel=gtk_notebook_get_tab_label(notebook, page);
                    int       tx=(tabLabel->allocation.x+nx)-2,
                              ty=(tabLabel->allocation.y+ny)-2,
                              tw=(tabLabel->allocation.width)+4,
                              th=(tabLabel->allocation.height)+4;

                    if(tx<=event->motion.x_root && ty<=event->motion.y_root &&
                       (tx+tw)>event->motion.x_root && (ty+th)>event->motion.y_root)
                    {
                        currentTab.rect.x=tx-nx;
                        currentTab.rect.y=ty-ny;
                        currentTab.rect.width=tw;
                        currentTab.rect.height=th;
                        currentTab.id=i;
                        break;
                    }
                }

                if(currentTab.id!=prevTab->id)
                {
                    if(currentTab.rect.x<0)
                    {
                        prevTab->id=currentTab.id;
                        prevTab->rect=currentTab.rect;
                        gtk_widget_queue_draw(widget);
                    }
                    else
                    {
                        GdkRectangle area;

                        if(prevTab->rect.x<0)
                            area=currentTab.rect;
                        else
                            gdk_rectangle_union(&(prevTab->rect), &(currentTab.rect), &area);
                        prevTab->id=currentTab.id;
                        prevTab->rect=currentTab.rect;
                        area.x-=12;
                        area.y-=12;
                        area.width+=24;
                        area.height+=24;
                        gtk_widget_queue_draw_area(widget, area.x, area.y, area.width, area.height);
                    }
                }
            }
        }
    }
    else if(GDK_LEAVE_NOTIFY==event->type)
    {
        QtCTab *prevTab=lookupTabHash(widget, FALSE);

        if(prevTab && prevTab->id>=0)
        {
            prevTab->id=prevTab->rect.x=prevTab->rect.y=
            prevTab->rect.width=prevTab->rect.height=-1;
            gtk_widget_queue_draw(widget);
        }
    }

    return FALSE;
}

static gboolean tabDeleteEvent(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    if(lookupTabHash(widget, FALSE))
        g_hash_table_remove(tabHashTable, widget);
    return FALSE;
}

#ifdef QTC_MOUSEOVER_HANDLES
static int * lookupToolbarHandleHash(void *hash, gboolean create)
{
    int *rv=NULL;

    if(!toolbarHandleHashTable)
        toolbarHandleHashTable=g_hash_table_new(g_direct_hash, g_direct_equal);

    rv=(int *)g_hash_table_lookup(toolbarHandleHashTable, hash);

    if(!rv && create)
    {
        rv=(int *)malloc(sizeof(int));
        *rv=0;
        g_hash_table_insert(toolbarHandleHashTable, hash, rv);
        rv=g_hash_table_lookup(toolbarHandleHashTable, hash);
    }

    return rv;
}

static gboolean toolbarHandleEvent(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    if(GDK_MOTION_NOTIFY==event->type)
    {
        int *handle=lookupToolbarHandleHash(widget, FALSE);

#if 0
        if(handle)
        {
            static int last_x=-100, last_y=-100;

            if(abs(last_x-event->motion.x_root)>4 || abs(last_y-event->motion.y_root)>4)
            {
                int nx, ny;

                gdk_window_get_origin(widget->window, &nx, &ny);
                {
                int      tx=(widget->allocation.x+nx),
                         ty=(widget->allocation.y+ny),
                         tw=(widget->allocation.width),
                         th=(widget->allocation.height);
                gboolean inHandle=(tx<=event->motion.x_root && ty<=event->motion.y_root &&
                                    (tx+tw)>event->motion.x_root && (ty+th)>event->motion.y_root);

                last_x=event->motion.x_root;
                last_y=event->motion.y_root;

                if( (inHandle && !(*handle)) || (!inHandle && *handle))
                {
                    *handle=!(*handle);
                    gtk_widget_queue_draw(widget);
                }
                }
            }
        }
#else
        if(handle && 0==*handle)
        {
            *handle=1;
            gtk_widget_queue_draw(widget);
        }
#endif
    }
    else if(GDK_LEAVE_NOTIFY==event->type)
    {
        int *handle=lookupToolbarHandleHash(widget, FALSE);
        if(handle && 1==*handle)
        {
            *handle=0;
            gtk_widget_queue_draw(widget);
        }
    }

    return FALSE;
}

static gboolean toolbarHandleDeleteEvent(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    if(lookupToolbarHandleHash(widget, FALSE))
        g_hash_table_remove(toolbarHandleHashTable, widget);
    return FALSE;
}
#endif

static GtkWidget **lookupMenubarHash(void *hash, gboolean create)
{
    GtkWidget **rv=NULL;

    if(!menubarHashTable)
        menubarHashTable=g_hash_table_new(g_direct_hash, g_direct_equal);

    rv=(GtkWidget **)g_hash_table_lookup(menubarHashTable, hash);

    if(!rv && create)
    {
        rv=malloc(sizeof(GtkWidget *));
        *rv=0;
        g_hash_table_insert(menubarHashTable, hash, rv);
        rv=g_hash_table_lookup(menubarHashTable, hash);
    }

    return rv;
}

static gboolean menubarEvent(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    if(GDK_MOTION_NOTIFY==event->type)
    {
        static int last_x=-100, last_y=-100;

        if(abs(last_x-event->motion.x_root)>4 || abs(last_y-event->motion.y_root)>4)
        {
            GtkWidget **item=lookupMenubarHash(widget, FALSE);

            if(item)
            {
                GtkMenuShell *menuShell = GTK_MENU_SHELL (widget);
                GList        *children=menuShell->children;
                GtkWidget    *current=NULL;
                int          nx, ny;

                gdk_window_get_origin(widget->window, &nx, &ny);

                while (children)
                {
                    current = children->data;
                    {
                    int cx=(current->allocation.x+nx),
                        cy=(current->allocation.y+ny),
                        cw=(current->allocation.width),
                        ch=(current->allocation.height);

                    if(cx<=event->motion.x_root && cy<=event->motion.y_root &&
                       (cx+cw)>event->motion.x_root && (cy+ch)>event->motion.y_root)
                        break;
                    }
                    children = children->next;
                }

                if(children && (*item)!=current)
                {
                    if(*item)
                        gtk_widget_set_state(*item, GTK_STATE_NORMAL);
                    *item=current;
                    gtk_widget_set_state(current, GTK_STATE_PRELIGHT);
                }
            }
        }
    }
    else if(GDK_LEAVE_NOTIFY==event->type)
    {
        GtkWidget **item=lookupMenubarHash(widget, FALSE);
        if(item)
        {
            if(*item && GTK_IS_MENU_ITEM(*item))
            {
                GtkMenuItem *mi=GTK_MENU_ITEM((*item));
                if(GTK_STATE_PRELIGHT==(*item)->state &&
                    mi->submenu && (!GTK_WIDGET_MAPPED (mi->submenu) ||
                                    GTK_MENU (mi->submenu)->tearoff_active))
                    gtk_widget_set_state(*item, GTK_STATE_NORMAL);
            }
            *item=0;
        }
    }

    return FALSE;
}

static gboolean menubarDeleteEvent(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    if(lookupMenubarHash(widget, FALSE))
        g_hash_table_remove(menubarHashTable, widget);
    return FALSE;
}

static gboolean windowEvent(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    if(GDK_FOCUS_CHANGE==event->type)
        if(lookupMenubarHash((GtkWidget *)user_data, FALSE)) /* Ensure widget is still valid! */
            gtk_widget_queue_draw((GtkWidget *)user_data);

    return FALSE;
}

static gpointer pixbufCacheDupKey(QtCPixKey *key)
{
    QtCPixKey *n=g_malloc(sizeof(QtCPixKey));

    n->col=key->col;
    n->pix=key->pix;
    n->shade=key->shade;
    return n;
}

static void pixbufCacheDestKey(QtCPixKey *key)
{
    g_free(key);
}

static guint pixbufCacheHashKey(gconstpointer k)
{
    QtCPixKey *key=(QtCPixKey *)k;
    /* FIXME compiler warning below! */
    int       hash=(key->pix<<24)+((key->col.red>>8)<<16) + ((key->col.green>>8)<<8) + (key->col.blue>>8);

    return g_int_hash(&hash);
}

static gboolean pixbufCacheKeyEqual(gconstpointer k1, gconstpointer k2)
{
    QtCPixKey *a=(QtCPixKey *)k1,
              *b=(QtCPixKey *)k2;

    return a->pix==b->pix && a->col.red==b->col.red && a->col.green==b->col.green &&
           a->col.blue==b->col.blue;
}

#ifdef QTC_GTK2_MENU_STRIPE_HACK_MENU
#ifdef __SUNPRO_C
#pragma align 4 (my_pixbuf)
#endif
#ifdef __GNUC__
static const guint8 blank16x16[] __attribute__ ((__aligned__ (4))) =
#else
static const guint8 blank16x16[] =
#endif
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (15) */
  "\0\0\0'"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\377\0\0\0\0\377\0\0\0\0\202\0\0\0\0"};
#endif

static GdkPixbuf * pixbufCacheValueNew(QtCPixKey *key)
{
    GdkPixbuf *res=NULL;

    switch(key->pix)
    {
        case PIX_RADIO_BORDER:
            res=gdk_pixbuf_new_from_inline(-1, radio_frame, TRUE, NULL);
            break;
        case PIX_RADIO_LIGHT:
            res=gdk_pixbuf_new_from_inline(-1, radio_light, TRUE, NULL);
            break;
        case PIX_RADIO_ON:
            res=gdk_pixbuf_new_from_inline(-1, radio_on, TRUE, NULL);
            break;
        case PIX_CHECK:
            res=gdk_pixbuf_new_from_inline(-1, opts.xCheck ? check_x_on :check_on, TRUE, NULL);
            break;
        case PIX_SLIDER:
            res=gdk_pixbuf_new_from_inline(-1, slider, TRUE, NULL);
            break;
        case PIX_SLIDER_LIGHT:
            res=gdk_pixbuf_new_from_inline(-1, slider_light, TRUE, NULL);
            break;
        case PIX_SLIDER_V:
            res=gdk_pixbuf_new_from_inline(-1, slider_v, TRUE, NULL);
            break;
        case PIX_SLIDER_LIGHT_V:
            res=gdk_pixbuf_new_from_inline(-1, slider_light_v, TRUE, NULL);
            break;
#ifdef QTC_GTK2_MENU_STRIPE_HACK_MENU
        case PIX_BLANK:
            return gdk_pixbuf_new_from_inline(-1, blank16x16, TRUE, NULL);
#endif
    }

    adjustPix(gdk_pixbuf_get_pixels(res), gdk_pixbuf_get_n_channels(res), gdk_pixbuf_get_width(res),
              gdk_pixbuf_get_height(res), gdk_pixbuf_get_rowstride(res),
              key->col.red>>8, key->col.green>>8, key->col.blue>>8, key->shade);
    return res;
}

GdkPixbuf * getPixbuf(GdkColor *widgetColor, EPixmap p, double shade)
{
    QtCPixKey key;

    key.col=*widgetColor;
    key.pix=p;
    key.shade=shade;

    if(!pixbufCache)
        pixbufCache = g_cache_new((GCacheNewFunc)pixbufCacheValueNew,
                                  (GCacheDestroyFunc)gdk_pixbuf_unref,
                                  (GCacheDupFunc)pixbufCacheDupKey,
                                  (GCacheDestroyFunc)pixbufCacheDestKey,
                                  pixbufCacheHashKey, g_direct_hash, pixbufCacheKeyEqual);

    return g_cache_insert(pixbufCache, &key);
}

static gboolean sanitizeSize(GdkWindow *window, gint *width, gint *height)
{
    gboolean set_bg = FALSE;

    if((-1==*width) && (-1==*height))
    {
        set_bg = GDK_IS_WINDOW(window);
        gdk_window_get_size(window, width, height);
    }
    else if(-1==*width)
        gdk_window_get_size(window, width, NULL);
    else if(-1==*height)
        gdk_window_get_size(window, NULL, height);

  return set_bg;
}

static void createCustomGradient(cairo_pattern_t *pt, GdkColor *base, CustomGradient *grad,
                                 gboolean rev)
{
    int i=0;

    for(i=0; i<grad->numGrad; ++i)
    {
        GdkColor col;
        shade(base, &col, grad->grad[i].val);
        cairo_pattern_add_color_stop_rgb(pt, rev ? 1.0-grad->grad[i].pos : grad->grad[i].pos,
                                         QTC_CAIRO_COL(col));
    }
}

static void drawBevelGradient(cairo_t *cr, GtkStyle *style, GdkRectangle *area,
                              GdkRegion *region, int x, int y, int width, int height, GdkColor *base,
                              double shadeTop, double shadeBot, gboolean horiz, gboolean increase,
                              gboolean sel, EAppearance bevApp, EWidget w)
{
    EAppearance app=APPEARANCE_BEVELLED!=bevApp || WIDGET_BUTTON(w) || WIDGET_LISTVIEW_HEADER==w
                        ? bevApp
                        : APPEARANCE_GRADIENT;

    if(IS_FLAT(bevApp))
        drawAreaColor(cr, area, region, base, x, y, width, height);
    else
    {
        cairo_pattern_t *pt=cairo_pattern_create_linear(x, y, horiz ? x : x+width-1, horiz ? y+height-1 : y);
        gboolean        tab=WIDGET_TAB_TOP==w || WIDGET_TAB_BOT==w,
                       selected=tab ? false : sel;

        if(WIDGET_TAB_TOP==w)
        {
            shadeBot=1.0;
            if(sel && APPEARANCE_INVERTED==app)
                shadeTop=0.8;
        }
        else if(WIDGET_TAB_BOT==w)
        {
            if(sel && APPEARANCE_INVERTED==app)
                shadeBot=1.2;
            else
                shadeBot-=fabs(shadeTop-shadeBot);
            shadeTop=1.0;
        }

        setCairoClipping(cr, area, region);

        if(!selected && (IS_GLASS(app) || APPEARANCE_SPLIT_GRADIENT==app))
        {
            if(WIDGET_TAB_BOT==w)
            {
                double t=shadeTop;
                shadeTop=shadeBot;
                shadeBot=t;
            }

            {  /* C variable scoping */
            double  shadeTopA=WIDGET_TAB_BOT==w
                                ? 1.0
                                : APPEARANCE_SPLIT_GRADIENT==app
                                    ? shadeTop
                                    : shadeTop*SHADE_GLASS_TOP_A(app, w),
                    shadeTopB=WIDGET_TAB_BOT==w
                                ? 1.0
                                : APPEARANCE_SPLIT_GRADIENT==app
                                    ? shadeTop-((shadeTop-shadeBot)*SPLIT_GRADIENT_FACTOR)
                                    : shadeTop*SHADE_GLASS_TOP_B(app, w),
                    shadeBotA=WIDGET_TAB_TOP==w
                                ? 1.0
                                : APPEARANCE_SPLIT_GRADIENT==app
                                    ? shadeBot+((shadeTop-shadeBot)*SPLIT_GRADIENT_FACTOR)
                                    : shadeBot*SHADE_GLASS_BOT_A(app),
                    shadeBotB=WIDGET_TAB_TOP==w
                                ? 1.0
                                : APPEARANCE_SPLIT_GRADIENT==app
                                    ? shadeBot
                                    : shadeBot*SHADE_GLASS_BOT_B(app);
            GdkColor topA,
                     topB,
                     botA,
                     botB;

            topA.pixel=botA.pixel=topB.pixel=botB.pixel=0;
            shade(base, &topA, shadeTopA);
            shade(base, &topB, shadeTopB);
            shade(base, &botA, shadeBotA);
            shade(base, &botB, shadeBotB);

            if(increase)
            {
                cairo_pattern_add_color_stop_rgb(pt, 0.0, QTC_CAIRO_COL(topA));
                cairo_pattern_add_color_stop_rgb(pt, 0.49999999999999999, QTC_CAIRO_COL(topB));
                cairo_pattern_add_color_stop_rgb(pt, 0.5, QTC_CAIRO_COL(botA));
                cairo_pattern_add_color_stop_rgb(pt, 1.0, QTC_CAIRO_COL(botB));
            }
            else
            {
                cairo_pattern_add_color_stop_rgb(pt, 0.0, QTC_CAIRO_COL(topB));
                cairo_pattern_add_color_stop_rgb(pt, 0.49999999999999999, QTC_CAIRO_COL(topA));
                cairo_pattern_add_color_stop_rgb(pt, 0.5, QTC_CAIRO_COL(botB));
                cairo_pattern_add_color_stop_rgb(pt, 1.0, QTC_CAIRO_COL(botA));
            }
            }
        }
        else if(!selected && APPEARANCE_BEVELLED==app &&
                ((horiz ? height : width) > (((WIDGET_BUTTON(w) ? 2 : 1)*BEVEL_BORDER(w))+4)))
        {
            if(WIDGET_LISTVIEW_HEADER==w)
            {
                GdkColor bot;

                bot.pixel=0;
                shade(base, &bot, SHADE_BEVEL_BOT(w));
                cairo_pattern_add_color_stop_rgb(pt, 0.0, QTC_CAIRO_COL(*base));
                cairo_pattern_add_color_stop_rgb(pt, 1.0-(BEVEL_BORDER(w)/((double)(horiz ? height : width))), QTC_CAIRO_COL(*base));
                cairo_pattern_add_color_stop_rgb(pt, 1.0, QTC_CAIRO_COL(bot));
            }
            else
            {
                GdkColor bot,
                         midTop,
                         midBot,
                         top;
                double   borderSize=BEVEL_BORDER(w)/((double)(horiz ? height : width));

                bot.pixel=midTop.pixel=midBot.pixel=top.pixel=0;
                shade(base, &top, SHADE_BEVEL_TOP);
                shade(base, &midTop, SHADE_BEVEL_MID_TOP);
                shade(base, &midBot, SHADE_BEVEL_MID_BOT);
                shade(base, &bot, SHADE_BEVEL_BOT(w));

                cairo_pattern_add_color_stop_rgb(pt, 0.0, QTC_CAIRO_COL(top));
                cairo_pattern_add_color_stop_rgb(pt, borderSize, QTC_CAIRO_COL(midTop));
                cairo_pattern_add_color_stop_rgb(pt, 1.05-borderSize, QTC_CAIRO_COL(midBot));
                cairo_pattern_add_color_stop_rgb(pt, 1.0, QTC_CAIRO_COL(bot));
            }
        }
        else if(!selected && IS_CUSTOM(app))
        {
            CustomGradient *grad=opts.customGradient[app-APPEARANCE_CUSTOM1];

            if(grad)
            {
                drawAreaColor(cr, area, region, base, x, y, width, height);
                createCustomGradient(pt, base, grad, WIDGET_TAB_BOT==w);
            }
            else
            {
                cairo_pattern_add_color_stop_rgb(pt, 0, QTC_CAIRO_COL(*base));
                cairo_pattern_add_color_stop_rgb(pt, 1.00, QTC_CAIRO_COL(*base));
            }
        }
        else
        {
            if(selected && WIDGET_TROUGH!=w && !tab && WIDGET_SLIDER_TROUGH!=w &&
               NULL!=opts.customGradient[APPEARANCE_SUNKEN])
            {
                drawAreaColor(cr, area, region, base, x, y, width, height);
                createCustomGradient(pt, base, opts.customGradient[APPEARANCE_SUNKEN], FALSE);
            }
            else
            {
                GdkColor top,
                         bot,
                         tabBaseCol,
                         *baseTopCol=base,
                         *t,
                         *b;
                gboolean inc=selected || APPEARANCE_INVERTED!=app ? increase : !increase;

                top.pixel=bot.pixel=tabBaseCol.pixel=0;

                if(opts.colorSelTab && sel && tab)
                {
                    generateMidColor(base, &(qtcPalette.menuitem[0]), &tabBaseCol, QTC_COLOR_SEL_TAB_FACTOR);
                    baseTopCol=&tabBaseCol;
                }

                if(equal(1.0, shadeTop))
                    t=baseTopCol;
                else
                {
                    shade(baseTopCol, &top, shadeTop);
                    t=&top;
                }
                if(equal(1.0, shadeBot))
                    b=base;
                else
                {
                    shade(base, &bot, shadeBot);
                    b=&bot;
                }

                if(WIDGET_TAB_BOT==w)
                    inc=!inc;

                if(inc)
                {
                    cairo_pattern_add_color_stop_rgb(pt, 0.0, QTC_CAIRO_COL(*t));
                    cairo_pattern_add_color_stop_rgb(pt, 1.0, QTC_CAIRO_COL(*b));
                }
                else
                {
                    cairo_pattern_add_color_stop_rgb(pt, 0.0, QTC_CAIRO_COL(*b));
                    cairo_pattern_add_color_stop_rgb(pt, 1.0, QTC_CAIRO_COL(*t));
                }
            }
        }

        cairo_set_source(cr, pt);
        cairo_rectangle(cr, x, y, width, height);
        cairo_fill(cr);
        cairo_pattern_destroy (pt);
        unsetCairoClipping(cr);
    }
}

typedef enum
{
    DF_DRAW_INSIDE     = 0x001,
    DF_BLEND           = 0x002,
    DF_DO_CORNERS      = 0x004,
    DF_SUNKEN          = 0x008,
    DF_DO_BORDER       = 0x010,
    DF_VERT            = 0x020
} EDrawFlags;

#define drawBorder(a, b, c, d, e, f, g, h, i, j, k, l, m, n) \
realDrawBorder(a, b, c, d, e, f, g, h, i, j, k, l, m, n, QT_STD_BORDER)

static void createTLPath(cairo_t *cr, double xd, double yd, double width, double height,
                         double radius, int round)
{
    if (round&CORNER_BL)
        cairo_arc(cr, xd+radius, yd+height-radius, radius, M_PI * 0.75, M_PI);
    else
        cairo_move_to(cr, xd, yd+height);

    if (round&CORNER_TL)
        cairo_arc(cr, xd+radius, yd+radius, radius, M_PI, M_PI * 1.5);
    else
        cairo_line_to(cr, xd, yd);

    if (round&CORNER_TR)
        cairo_arc(cr, xd+width-radius, yd+radius, radius, M_PI * 1.5, M_PI * 1.75);
    else
        cairo_line_to(cr, xd+width, yd);
}

static void createBRPath(cairo_t *cr, double xd, double yd, double width, double height,
                         double radius, int round)
{
    if (round&CORNER_TR)
        cairo_arc(cr, xd+width-radius, yd+radius, radius, M_PI * 1.75, 0);
    else
        cairo_move_to(cr, xd+width, yd);

    if (round&CORNER_BR)
        cairo_arc(cr, xd+width-radius, yd+height-radius, radius, 0, M_PI * 0.5);
    else
        cairo_line_to(cr, xd+width, yd+height);

    if (round&CORNER_BL)
        cairo_arc(cr, xd+radius, yd+height-radius, radius, M_PI * 0.5, M_PI * 0.75);
    else
        cairo_line_to(cr, xd, yd+height);
}

static void createPath(cairo_t *cr, double xd, double yd, double width, double height,
                       double radius, int round)
{
    if (round&CORNER_TL)
        cairo_move_to(cr, xd+radius, yd);
    else
        cairo_move_to(cr, xd, yd);

    if (round&CORNER_TR)
        cairo_arc(cr, xd+width-radius, yd+radius, radius, M_PI * 1.5, M_PI * 2);
    else
        cairo_line_to(cr, xd+width, yd);

    if (round&CORNER_BR)
        cairo_arc(cr, xd+width-radius, yd+height-radius, radius, 0, M_PI * 0.5);
    else
        cairo_line_to(cr, xd+width, yd+height);

    if (round&CORNER_BL)
        cairo_arc(cr, xd+radius, yd+height-radius, radius, M_PI * 0.5, M_PI);
    else
        cairo_line_to(cr, xd, yd+height);

    if (round&CORNER_TL)
        cairo_arc(cr, xd+radius, yd+radius, radius, M_PI, M_PI * 1.5);
    else
        cairo_line_to(cr, xd, yd);
}

static void realDrawBorder(cairo_t *cr, GtkStyle *style, GtkStateType state, GdkRectangle *area,
                           GdkRegion *region, gint x, gint y, gint width, gint height,
                           GdkColor *c_colors, int round,
                           EBorder borderProfile, EWidget widget, int flags, int borderVal)
{
    if(ROUND_NONE==opts.round)
        round=ROUNDED_NONE;

    {
    double       radius=getRadius(opts.round, width, height, widget, RADIUS_EXTERNAL),
                 xd=x+0.5,
                 yd=y+0.5;
    EAppearance  app=widgetApp(widget, &opts);
    gboolean     enabled=GTK_STATE_INSENSITIVE!=state,
                 useText=GTK_STATE_INSENSITIVE!=state && WIDGET_DEF_BUTTON==widget && IND_FONT_COLOR==opts.defBtnIndicator && enabled;
    int          useBorderVal=!enabled && WIDGET_BUTTON(widget) ? QT_DISABLED_BORDER : borderVal;
    GdkColor     *colors=c_colors ? c_colors : qtcPalette.background,
                 *border_col= useText ? &style->text[GTK_STATE_NORMAL] : &colors[useBorderVal];

    setCairoClipping(cr, area, region);

    width--;
    height--;

    switch(borderProfile)
    {
        case BORDER_FLAT:
            break;
        case BORDER_RAISED:
        case BORDER_SUNKEN:
        {
            double radiusi=getRadius(opts.round, width, height, widget, RADIUS_INTERNAL),
                   xdi=xd+1,
                   ydi=yd+1;
            int    widthi=width-2,
                   heighti=height-2;

            if((GTK_STATE_INSENSITIVE!=state || BORDER_SUNKEN==borderProfile) &&
               (BORDER_RAISED==borderProfile || APPEARANCE_FLAT!=app))
            {
                GdkColor *col=col=&colors[BORDER_RAISED==borderProfile ? 0 : QT_FRAME_DARK_SHADOW];
                if(flags&DF_BLEND)
                    cairo_set_source_rgba(cr, QTC_CAIRO_COL(*col), QTC_BORDER_BLEND_ALPHA);
                else
                    cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col));
            }
            else
                cairo_set_source_rgb(cr, QTC_CAIRO_COL(style->bg[state]));

            createTLPath(cr, xdi, ydi, widthi, heighti, radiusi, round);
            cairo_stroke(cr);
            if(WIDGET_CHECKBOX!=widget)
            {
                if(GTK_STATE_INSENSITIVE!=state && (BORDER_SUNKEN==borderProfile || APPEARANCE_FLAT!=app))
                {
                    GdkColor *col=col=&colors[BORDER_RAISED==borderProfile ? QT_FRAME_DARK_SHADOW : 0];
                    if(flags&DF_BLEND)
                        cairo_set_source_rgba(cr, QTC_CAIRO_COL(*col), QTC_BORDER_BLEND_ALPHA);
                    else
                        cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col));
                }
                else
                    cairo_set_source_rgb(cr, QTC_CAIRO_COL(style->bg[state]));

                createBRPath(cr, xdi, ydi, widthi, heighti, radiusi, round);
                cairo_stroke(cr);
            }
        }
    }

    cairo_set_source_rgb(cr, QTC_CAIRO_COL(*border_col));
    createPath(cr, xd, yd, width, height, radius, round);
    cairo_stroke(cr);
    unsetCairoClipping(cr);
    }
}

static void drawGlow(cairo_t *cr, GdkRectangle *area, GdkRegion *region,
                     int x, int y, int w, int h, EWidget widget)
{
    double   xd=x+0.5,
             yd=y+0.5,
             radius=getRadius(opts.round, w, h, WIDGET_STD_BUTTON, RADIUS_ETCH);
    GdkColor *col=&qtcPalette.mouseover[WIDGET_DEF_BUTTON==widget && IND_GLOW==opts.defBtnIndicator ? QTC_GLOW_DEFBTN : QTC_GLOW_MO];

    setCairoClipping(cr, area, region);
    cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col));
    createPath(cr, xd, yd, w-1, h-1, radius, ROUNDED_ALL);
    cairo_stroke(cr);
    unsetCairoClipping(cr);
}

static void drawEtch(cairo_t *cr, GdkRectangle *area, GdkRegion *region,
                     int x, int y, int w, int h, gboolean raised)
{
    double xd=x+0.5,
           yd=y+0.5,
           radius=getRadius(opts.round, w, h, WIDGET_STD_BUTTON, RADIUS_ETCH);

    setCairoClipping(cr, area, region);

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, QTC_ETCH_TOP_ALPHA);
    if(!raised)
    {
        createTLPath(cr, xd, yd, w-1, h-1, radius, ROUNDED_ALL);
        cairo_stroke(cr);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, QTC_ETCH_BOTTOM_ALPHA);
    }

    createBRPath(cr, xd, yd, w-1, h-1, radius, ROUNDED_ALL);
    cairo_stroke(cr);
    unsetCairoClipping(cr);
}

static void drawLightBevel(cairo_t *cr, GtkStyle *style, GdkWindow *window, GtkStateType state,
                           GdkRectangle *area, GdkRegion *region, gint x, gint y, gint width,
                           gint height, GdkColor *base, GdkColor *colors, int round, EWidget widget,
                           EBorder borderProfile, int flags)
{
    EAppearance app=widgetApp(widget, &opts);
    gboolean    sunken=flags&DF_SUNKEN,
                doColouredMouseOver=opts.coloredMouseOver && qtcPalette.mouseover &&
                                  WIDGET_UNCOLOURED_MO_BUTTON!=widget &&
                                  GTK_STATE_PRELIGHT==state &&
                                  (!IS_SLIDER(widget) || (WIDGET_SB_SLIDER==widget && opts.coloredMouseOver)) &&
                                  (IS_TOGGLE_BUTTON(widget) || !sunken),
                plastikMouseOver=doColouredMouseOver && (MO_PLASTIK==opts.coloredMouseOver || WIDGET_SB_SLIDER==widget),
                colouredMouseOver=doColouredMouseOver && WIDGET_SB_SLIDER!=widget &&
                                       (MO_COLORED==opts.coloredMouseOver ||
                                              (MO_GLOW==opts.coloredMouseOver &&
                                              ((WIDGET_COMBO!=widget && !ETCH_WIDGET(widget))))),
                lightBorder=QTC_DRAW_LIGHT_BORDER(sunken, widget, app),
                bevelledButton=WIDGET_BUTTON(widget) && APPEARANCE_BEVELLED==app,
                doEtch=flags&DF_DO_BORDER && (ETCH_WIDGET(widget) || WIDGET_COMBO_BUTTON==widget) && QTC_DO_EFFECT,
                horiz=!(flags&DF_VERT);

    setCairoClipping(cr, area, region);

    if((WIDGET_COMBO || WIDGET_COMBO_BUTTON) && doEtch)
        if(ROUNDED_RIGHT==round)
        {
            x--;
            width++;
        }
        else if(ROUNDED_LEFT==round)
            width++;

    if(doEtch)
    {
        y++;
        x++;
        height-=2;
        width-=2;
    }

    {

    double      xd=x+0.5,
                yd=y+0.5,
                bx=x, by=y, bw=width, bh=height;

    if(!colouredMouseOver && lightBorder)
    {
        if(IS_CUSTOM(app) || (WIDGET_PROGRESSBAR==widget && (!IS_GLASS(app) || opts.fillProgress)))
            by+=2,  bx+=2,  bw-=4,  bh-=4;
        else if(!horiz)
            by+=2,  bx++,  bw-=3,  bh-=4;
        else
            bx+=2,  by++,  bh-=3,  bw-=4;
    }
    else if(colouredMouseOver || (!IS_GLASS(app) && (!sunken || flags&DF_DRAW_INSIDE)))
    {
        int      dark=bevelledButton ? 2 : 4;
        GdkColor *col1=colouredMouseOver ? &qtcPalette.mouseover[QTC_MO_STD_LIGHT(widget, sunken)]
                                         : &colors[sunken ? dark : 0];
        /* Top and left */
        cairo_new_path(cr);
        cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col1));
        cairo_move_to(cr, xd + 1, yd + 1);
        cairo_line_to(cr, xd + width - 2, yd + 1);
        cairo_move_to(cr, xd + 1, yd + 1);
        cairo_line_to(cr, xd + 1, yd + height - 2);
        cairo_stroke(cr);

        if(colouredMouseOver || bevelledButton || APPEARANCE_RAISED==app)
        {
            GdkColor *col2=colouredMouseOver ? &qtcPalette.mouseover[QTC_MO_STD_DARK(widget)]
                                             : &colors[sunken ? 0 : dark];

            if(EFFECT_NONE!=opts.buttonEffect && WIDGET_SPIN_UP==widget && horiz)
            {
                height--; bh-=4;
            }

            /* Right and bottom edge */
            cairo_new_path(cr);
            cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col2));
            cairo_move_to(cr, xd + 1, yd + height - 2);
            cairo_line_to(cr, xd + width - 2, yd + height - 2);
            cairo_move_to(cr, xd + width - 2, yd + 1);
            cairo_line_to(cr, xd + width - 2, yd + height - 2);
            cairo_stroke(cr);
            bx+=2; by+=2; bh-=4; bw-=4;
        }
        else
            bx+=2, by+=2, bh-=3, bw-=3;
    }
    else
        bx++, by++, bh-=2, bw-=2;

    if(!colouredMouseOver && lightBorder)
    {
        GdkColor *col=&colors[APPEARANCE_DULL_GLASS==app ? 1 : 0];

        cairo_new_path(cr);
        cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col));
        cairo_rectangle(cr, xd+1, yd+1, width-3, height-3);
        cairo_stroke(cr);
    }

    if(bw>0 && bh>0)
    {
        drawBevelGradient(cr, style, area, region, bx, by, bw, bh, base,
                          getWidgetShade(widget, TRUE, sunken, app), getWidgetShade(widget, FALSE, sunken, app),
                          horiz, !sunken, sunken, app,
                          widget);

        if(plastikMouseOver)
        {
            if(WIDGET_SB_SLIDER==widget)
            {
                int len=QTC_SB_SLIDER_MO_LEN(horiz ? width : height),
                    so=lightBorder ? QTC_SLIDER_MO_BORDER : 1,
                    eo=len+so,
                    col=QTC_SLIDER_MO_SHADE;

                if(horiz)
                {
                    drawBevelGradient(cr, style, area, region, x+so, y, len, height, &qtcPalette.mouseover[col],
                                      getWidgetShade(widget, TRUE, sunken, app), getWidgetShade(widget, FALSE, sunken, app),
                                      horiz, !sunken, sunken, app,
                                      widget);
                    drawBevelGradient(cr, style, area, region, x+width-eo, y, len, height, &qtcPalette.mouseover[col],
                                      getWidgetShade(widget, TRUE, sunken, app), getWidgetShade(widget, FALSE, sunken, app),
                                      horiz, !sunken, sunken, app,
                                      widget);
                }
                else
                {
                    drawBevelGradient(cr, style, area, region, x, y+so, width, len, &qtcPalette.mouseover[col],
                                      getWidgetShade(widget, TRUE, sunken, app), getWidgetShade(widget, FALSE, sunken, app),
                                      horiz, !sunken, sunken, app,
                                      widget);
                    drawBevelGradient(cr, style, area, region, x, y+height-eo, width, len, &qtcPalette.mouseover[col],
                                      getWidgetShade(widget, TRUE, sunken, app), getWidgetShade(widget, FALSE, sunken, app),
                                      horiz, !sunken, sunken, app,
                                      widget);
                }
            }
            else
            {
                int      mh=height;
                GdkColor *col=&qtcPalette.mouseover[QTC_MO_PLASTIK_DARK(widget)];
                bool     horizontal=(horiz && !(WIDGET_SB_BUTTON==widget || WIDGET_SB_SLIDER==widget))||
                                    (!horiz && (WIDGET_SB_BUTTON==widget || WIDGET_SB_SLIDER==widget)),
                         thin=WIDGET_SB_BUTTON==widget || WIDGET_SPIN_UP==widget || WIDGET_SPIN_DOWN==widget ||
                              ((horiz ? height : width)<16);

                if(EFFECT_NONE!=opts.buttonEffect && WIDGET_SPIN_UP==widget && horiz)
                    mh--;
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col));
                if(horizontal)
                {
                    cairo_move_to(cr, x+1, yd+1);
                    cairo_line_to(cr, x+width-1, yd+1);
                    cairo_move_to(cr, x+1, yd+mh-2);
                    cairo_line_to(cr, x+width-1, yd+mh-2);
                }
                else
                {
                    cairo_move_to(cr, xd+1, y+1);
                    cairo_line_to(cr, xd+1, y+mh-1);
                    cairo_move_to(cr, xd+width-2, y+1);
                    cairo_line_to(cr, xd+width-2, y+mh-1);
                }
                cairo_stroke(cr);
                if(!thin)
                {
                    col=&qtcPalette.mouseover[QTC_MO_PLASTIK_LIGHT(widget)];
                    cairo_new_path(cr);
                    cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col));
                    if(horizontal)
                    {
                        cairo_move_to(cr, x+1, yd+2);
                        cairo_line_to(cr, x+width-1, yd+2);
                        cairo_move_to(cr, x+1, yd+mh-3);
                        cairo_line_to(cr, x+width-1, yd+mh-3);
                    }
                    else
                    {
                        cairo_move_to(cr, xd+2, y+1);
                        cairo_line_to(cr, xd+2, y+mh-1);
                        cairo_move_to(cr, xd+width-3, y+1);
                        cairo_line_to(cr, xd+width-3, y+mh-1);
                    }
                    cairo_stroke(cr);
                }
            }
        }
    }

    if(doEtch) //  && WIDGET_DEF_BUTTON!=widget)
        if((WIDGET_OTHER!=widget && WIDGET_SLIDER_TROUGH!=widget && WIDGET_COMBO_BUTTON!=widget &&
            MO_GLOW==opts.coloredMouseOver && GTK_STATE_PRELIGHT==state) ||
           (WIDGET_DEF_BUTTON==widget && IND_GLOW==opts.defBtnIndicator))
            drawGlow(cr, area, region, x-(WIDGET_COMBO==widget ? (ROUNDED_RIGHT==round ? 3 : 1) : 1), y-1,
                                       width+(WIDGET_COMBO==widget ? (ROUNDED_RIGHT==round ? 4 : 5) : 2), height+2,
                     WIDGET_DEF_BUTTON==widget && GTK_STATE_PRELIGHT==state ? WIDGET_STD_BUTTON : widget);
        else
            drawEtch(cr, area, region, x-(WIDGET_COMBO_BUTTON==widget ? (ROUNDED_RIGHT==round ? 3 : 1) : 1), y-1,
                                       width+(WIDGET_COMBO_BUTTON==widget ? (ROUNDED_RIGHT==round ? 4 : 5) : 2), height+2,
                    EFFECT_SHADOW==opts.buttonEffect && WIDGET_COMBO_BUTTON!=widget && WIDGET_BUTTON(widget) && !sunken);

    if(flags&DF_DO_BORDER && width>2 && height>2)
        if(doEtch && (WIDGET_OTHER!=widget && WIDGET_SLIDER_TROUGH!=widget && WIDGET_COMBO_BUTTON!=widget &&
                      MO_GLOW==opts.coloredMouseOver && GTK_STATE_PRELIGHT==state) ||
                     (WIDGET_DEF_BUTTON==widget && IND_GLOW==opts.defBtnIndicator))
            drawBorder(cr, style, state, area, region, x, y, width, height, qtcPalette.mouseover,
                       round, borderProfile, widget, flags);
        else
            drawBorder(cr, style, state, area, region, x, y, width, height, colors,
                       round, borderProfile, widget, flags);
    }

    unsetCairoClipping(cr);
}

static void drawLines(cairo_t *cr, double rx, double ry, int rwidth, int rheight, gboolean horiz,
                      int n_lines, int offset, GdkColor *cols, GdkRectangle *area, int dark, int etchedDisp,
                      gboolean light)
{
    if(horiz)
        ry+=0.5,  rwidth+=1;
    else
        rx+=0.5,  rheight+=1;

    {
    int      space =(n_lines*2)+(etchedDisp || !light ? (n_lines-1) : 0),
             step = etchedDisp || !light ? 3 : 2,
             i;
    double   x = (horiz ? rx : rx+((rwidth-space)>>1)),
             y = (horiz ? ry+((rheight-space)>>1) : ry),
             x2 = rx + rwidth-1,
             y2 = ry + rheight-1;
    GdkColor *col1 = &cols[dark],
             *col2 = &cols[0];

    setCairoClipping(cr, area, NULL);
    cairo_set_source_rgb(cr,QTC_CAIRO_COL(*col1));

    if(horiz)
    {
        for(i=0; i<space; i+=step)
        {
            cairo_move_to(cr, x+offset, y+i);
            cairo_line_to(cr, x2-(offset+etchedDisp), y+i);
        }
        cairo_stroke(cr);

        if(light)
        {
            cairo_set_source_rgb(cr,QTC_CAIRO_COL(*col2));
            for(i=1; i<space; i+=step)
            {
                cairo_move_to(cr, x+offset+etchedDisp, y+i);
                cairo_line_to(cr, x2-offset, y+i);
            }
            cairo_stroke(cr);
        }
    }
    else
    {
        for(i=0; i<space; i+=step)
        {
            cairo_move_to(cr, x+i, y+offset);
            cairo_line_to(cr, x+i, y2-(offset+etchedDisp));
        }
        cairo_stroke(cr);
        if(light)
        {
            cairo_set_source_rgb(cr,QTC_CAIRO_COL(*col2));
            for(i=1; i<space; i+=step)
            {
                cairo_move_to(cr, x+i, y+offset+etchedDisp);
                cairo_line_to(cr, x+i, y2-offset);
            }
            cairo_stroke(cr);
        }
    }
    unsetCairoClipping(cr);
    }
}

static void drawDots(cairo_t *cr, int rx, int ry, int rwidth, int rheight, gboolean horiz,
                     int n_lines, int offset, GdkColor *cols, GdkRectangle *area, int startOffset,
                     int dark)
{
    int      space =(n_lines*2)+(n_lines-1),
             x = horiz ? rx : rx+((rwidth-space)>>1),
             y = horiz ? ry+((rheight-space)>>1) : ry,
             i, j,
             numDots=(horiz ? (rwidth-(2*offset))/3 : (rheight-(2*offset))/3)+1;
    GdkColor *col1 = &cols[dark],
             *col2 = &cols[0];

    setCairoClipping(cr, area, NULL);
    if(horiz)
    {
        if(startOffset && y+startOffset>0)
            y+=startOffset;

        cairo_new_path(cr);
        cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col1));
        for(i=0; i<space; i+=3)
            for(j=0; j<numDots; j++)
                cairo_rectangle(cr, x+offset+(3*j), y+i, 1, 1);
        cairo_fill(cr);

        cairo_new_path(cr);
        cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col2));
        for(i=1; i<space; i+=3)
            for(j=0; j<numDots; j++)
                cairo_rectangle(cr, x+offset+1+(3*j), y+i, 1, 1);
        cairo_fill(cr);
    }
    else
    {
        if(startOffset && x+startOffset>0)
            x+=startOffset;

        cairo_new_path(cr);
        cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col1));
        for(i=0; i<space; i+=3)
            for(j=0; j<numDots; j++)
                cairo_rectangle(cr, x+i, y+offset+(3*j), 1, 1);
        cairo_fill(cr);

        cairo_new_path(cr);
        cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col2));
        for(i=1; i<space; i+=3)
            for(j=0; j<numDots; j++)
                cairo_rectangle(cr, x+i, y+offset+1+(3*j), 1, 1);
        cairo_fill(cr);
    }
    unsetCairoClipping(cr);
}

void getEntryParentBgCol(const GtkWidget *widget, GdkColor *color)
{
    const GtkWidget *parent;

    if (!widget)
    {
        color->red=color->green=color->blue = 65535;
        return;
    }

    parent = widget->parent;

    while (parent && GTK_WIDGET_NO_WINDOW(parent))
        parent = parent->parent;

    if (!parent)
        parent = widget;

    *color = parent->style->bg[GTK_WIDGET_STATE(parent)];
}

static void drawEntryField(cairo_t *cr, GtkStyle *style, GtkStateType state,
                           GtkWidget *widget, GdkRectangle *area, gint x, gint y, gint width,
                           gint height, int round, gboolean isCombo)
{
    gboolean enabled=!(GTK_STATE_INSENSITIVE==state || (widget && !GTK_WIDGET_IS_SENSITIVE(widget))),
             highlight=enabled && widget && GTK_WIDGET_HAS_FOCUS(widget) && GTK_APP_JAVA!=qtSettings.app,
             doEtch=QTC_DO_EFFECT;
    GdkColor *colors=highlight ? qtcPalette.menuitem : qtcPalette.button;

    if(ROUND_NONE!=opts.round)
    {
        GdkColor parentBgCol;

        getEntryParentBgCol(widget, &parentBgCol);
        setCairoClipping(cr, area, NULL);
        cairo_set_source_rgb(cr, QTC_CAIRO_COL(parentBgCol));
        cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
        if(doEtch)
            cairo_rectangle(cr, x+1.5, y+1.5, width-2, height-3);
        cairo_set_line_width(cr, 1);
        cairo_stroke(cr);
        unsetCairoClipping(cr);
    }

    if(doEtch)
        y++,  x++, height-=2,  width-=2;

#ifdef QTC_DEBUG
printf("Draw entry_field %d %d %d %d %d %d ", state, x, y, width, height, round);
debugDisplayWidget(widget, 3);
#endif
    if(ROUNDED_ALL!=round)
    {
        if(highlight)
        {
            if(doEtch)
                if(ROUNDED_RIGHT==round)  /* RtoL */
                    x--;
                else
                    width++;
        }
        else
            if(ROUNDED_RIGHT==round)  /* RtoL */
                x-=2;
            else
                width+=2;
    }

/*
    if(GTK_APP_OPEN_OFFICE!=qtSettings.app)
        drawAreaColor(cr, area, NULL, enabled ? &style->base[state] : &style->bg[GTK_STATE_INSENSITIVE], x+1, y+1, width-2, height-2);
*/

    drawBorder(cr, style, GTK_WIDGET_IS_SENSITIVE(widget) ? state : GTK_STATE_INSENSITIVE, area, NULL, x, y, width, height,
               colors, round, BORDER_SUNKEN, WIDGET_OTHER, DF_DO_CORNERS|DF_BLEND);
    if(doEtch)
    {
        GdkRectangle rect;
        GdkRegion    *region=NULL;

        y--;
        height+=2;
        x--;
        width+=2;

        rect.x=x; rect.y=y; rect.width=width; rect.height=height;
        region=gdk_region_rectangle(&rect);

        if(!(round&CORNER_TR) && !(round&CORNER_BR))
            width+=4;
        if(!(round&CORNER_TL) && !(round&CORNER_BL))
            x-=4;

        drawEtch(cr, region ? NULL : area, region, x, y, width, height, FALSE);
        gdk_region_destroy(region);
    }

    if(GTK_IS_ENTRY(widget) && !gtk_entry_get_visibility(GTK_ENTRY(widget)))
        gtk_entry_set_invisible_char(GTK_ENTRY(widget), opts.passwordChar);
}

#define QTC_GIMP_MAIN   "GimpToolbox"      /* Main GIMP toolbox */
#define QTC_GIMP_WINDOW "GimpDisplayShell" /* Image window */

static GtkWidget * getParentWindow(GtkWidget *widget)
{
    GtkWidget *top=NULL;
    GList     *topWindows,
              *node;

    if(GTK_IS_DIALOG(widget) || GTK_APP_GIMP!=qtSettings.app)
        for(topWindows=node=gtk_window_list_toplevels(); node; node = node->next)
        {
            GtkWidget *w=node->data;

            if(w && GTK_IS_WIDGET(w) && w->window && w!=widget &&
               gtk_window_has_toplevel_focus(GTK_WINDOW(w)) &&
               gtk_window_is_active(GTK_WINDOW(w)))
            {
                top=w;
                break;
            }
        }

    if(!top && GTK_APP_GIMP==qtSettings.app)
    {
        for(topWindows=node=gtk_window_list_toplevels(); node; node = node->next)
        {
            GtkWidget *w=node->data;

            if(w && GTK_IS_WIDGET(w) && 0==strcmp(gtk_type_name(GTK_WIDGET_TYPE(w)), QTC_GIMP_MAIN))
            {
                top=w;
                break;
            }
        }
    }

    return top;
}

static void dialogMapEvent(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *top=getParentWindow(widget);

    if(top)
    {
        GTK_WINDOW(widget)->transient_parent=GTK_WINDOW(top);
        gdk_window_set_transient_for(widget->window, top->window);
        /*gtk_window_set_skip_taskbar_hint(GTK_WINDOW(widget), TRUE);
        gtk_window_set_skip_pager_hint(GTK_WINDOW(widget), TRUE); */
    }
}

static bool haveAlternareListViewCol()
{
    return 0!=qtSettings.colors[PAL_ACTIVE][COLOR_LV].red || 0!=qtSettings.colors[PAL_ACTIVE][COLOR_LV].green ||
           0!=qtSettings.colors[PAL_ACTIVE][COLOR_LV].blue;
}

static void gtkDrawFlatBox(GtkStyle *style, GdkWindow *window, GtkStateType state,
                           GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget,
                           const gchar *detail, gint x, gint y, gint width, gint height)
{
    QTC_CAIRO_BEGIN

#ifdef QTC_DEBUG
printf("Draw flat box %d %d %d %d %d %d %s  ", state, shadow_type, x, y, width, height, detail ? detail : "NULL");
debugDisplayWidget(widget, 3);
#endif

#define QTC_MODAL_HACK_NAME  "--kgtk-modal-dialog-hack--"
#define QTC_MENU_HACK_NAME   "--kgtk-menu-hack--"
#ifdef QTC_REORDER_GTK_DIALOG_BUTTONS
#define QTC_BUTTON_HACK_NAME "--kgtk-button-hack--"

#if GTK_CHECK_VERSION(2, 6, 0)
    if(!opts.gtkButtonOrder && GTK_IS_WINDOW(widget) && detail && 0==strcmp(detail, "base"))
    {
        GtkWidget *topLevel=gtk_widget_get_toplevel(widget);

        if(topLevel && GTK_IS_DIALOG(topLevel) && (!topLevel->name || 0==strcmp(topLevel->name, QTC_MODAL_HACK_NAME)))
            {
                gtk_dialog_set_alternative_button_order(GTK_DIALOG(topLevel), GTK_RESPONSE_HELP,
                                                        GTK_RESPONSE_OK, GTK_RESPONSE_YES, GTK_RESPONSE_ACCEPT, GTK_RESPONSE_APPLY,
                                                        GTK_RESPONSE_REJECT, GTK_RESPONSE_CLOSE, GTK_RESPONSE_NO, GTK_RESPONSE_CANCEL, -1);

                if(!topLevel->name)
                    gtk_widget_set_name(topLevel, QTC_BUTTON_HACK_NAME);
                else
                    gtk_widget_set_name(topLevel, QTC_BUTTON_HACK_NAME QTC_MODAL_HACK_NAME);
        }
    }
#endif
#endif
    if(widget && opts.fixParentlessDialogs &&
        GTK_IS_WINDOW(widget) && detail && 0==strcmp(detail, "base"))
    {
        GtkWidget *topLevel=gtk_widget_get_toplevel(widget);

        if(topLevel && GTK_WIDGET_TOPLEVEL(topLevel))
        {
            const gchar *typename=gtk_type_name(GTK_WIDGET_TYPE(topLevel));

            if(GTK_APP_GIMP_PLUGIN==qtSettings.app)
            {
                /* CPD should really try to find active GIMP window... */
                gtk_window_set_skip_taskbar_hint(GTK_WINDOW(topLevel), TRUE);
                gtk_window_set_skip_pager_hint(GTK_WINDOW(topLevel), TRUE);
                gtk_window_set_keep_above(GTK_WINDOW(topLevel), TRUE);
            }
            else if((GTK_WINDOW(topLevel)->modal || GTK_IS_DIALOG(topLevel) || /*GTK_APP_GAIM==qtSettings.app ||*/
                    (GTK_APP_GIMP==qtSettings.app &&
                     strcmp(typename, QTC_GIMP_WINDOW) &&
                     strcmp(typename, QTC_GIMP_MAIN) ) ) &&
#if 0
                   (!topLevel->name || !strstr(topLevel->name, QTC_MODAL_HACK_NAME)) &&
#else
                   (!topLevel->name || strcmp(topLevel->name, QTC_MODAL_HACK_NAME)) &&
#endif
                   NULL==gtk_window_get_transient_for(GTK_WINDOW(topLevel)))
            {
                /* Give the widget a name so that we dont keep on performing this function... */
                if(!topLevel->name)
                    gtk_widget_set_name(topLevel, QTC_MODAL_HACK_NAME);
#if 0
                else if(0==strcmp(topLevel->name, QTC_BUTTON_HACK_NAME))
                    gtk_widget_set_name(topLevel, QTC_BUTTON_HACK_NAME QTC_MODAL_HACK_NAME);
#endif
                /*
                  For non-modal dialogs we set the transient hint when the "map" event is received, this has the
                  effect that the dialog is placed where it would've been without this hack, but it does not get
                  a taskbar entry... This "fixes" gimp and its multitude of dialogs...
                */
                if(!GTK_WINDOW(topLevel)->modal)
                    g_signal_connect(G_OBJECT(topLevel), "map", G_CALLBACK(dialogMapEvent), topLevel);
                else
                {
                    GtkWidget *top=getParentWindow(topLevel);

                    if(top)
                        GTK_WINDOW(topLevel)->transient_parent=GTK_WINDOW(top);
                }
            }
        }
    }

    if(APPEARANCE_FLAT!=opts.selectionAppearance && GTK_STATE_SELECTED==state && GTK_IS_TREE_VIEW(widget))
        drawBevelGradient(cr, style, area, NULL, x, y, width, height,
                          &style->base[GTK_WIDGET_HAS_FOCUS(widget) ? GTK_STATE_SELECTED : GTK_STATE_ACTIVE],
                          getWidgetShade(WIDGET_SELECTION, TRUE, FALSE, opts.selectionAppearance),
                          getWidgetShade(WIDGET_SELECTION, FALSE, FALSE, opts.selectionAppearance),
                          TRUE, TRUE, FALSE, opts.selectionAppearance, WIDGET_SELECTION);
    else if( ( GTK_STATE_PRELIGHT==state && (detail && (0==strcmp(detail, QTC_PANED) || 0==strcmp(detail, "expander") ||
                                                  (opts.crHighlight && 0==strcmp(detail, "checkbutton")))) ) )
        drawAreaMod(cr, style, GTK_STATE_PRELIGHT, area, NULL, opts.highlightFactor, x, y, width, height);
    else if (haveAlternareListViewCol() && GTK_STATE_SELECTED!=state &&
             GTK_IS_TREE_VIEW(widget) && gtk_tree_view_get_rules_hint(GTK_TREE_VIEW(widget)) && DETAILHAS("cell_even"))
        drawAreaColor(cr, area, NULL, getCellCol(&style->base[GTK_STATE_NORMAL], detail), x, y, width, height);
    else if (haveAlternareListViewCol() && GTK_STATE_SELECTED!=state &&
             GTK_IS_TREE_VIEW(widget) && gtk_tree_view_get_rules_hint(GTK_TREE_VIEW(widget)) && DETAILHAS("cell_odd"))
        drawAreaColor(cr, area, NULL, getCellCol(&qtSettings.colors[PAL_ACTIVE][COLOR_LV], detail),
                      x, y, width, height);
    else if(!(GTK_APP_JAVA==qtSettings.app && widget && GTK_IS_LABEL(widget)))
    {
        parent_class->draw_flat_box(style, window, state, shadow_type, area, widget, detail, x, y,
                                    width, height);

        /* For SWT (e.g. eclipse) apps. For some reason these only seem to allow a ythickness of at max 2 - but
           for etching we need 3. So we fake this by drawing the 3rd lines here...*/

        if(QTC_DO_EFFECT && GTK_STATE_INSENSITIVE!=state && DETAIL("entry_bg") &&
           isSwtComboBoxEntry(widget) && GTK_WIDGET_HAS_FOCUS(widget))
        {
            drawHLine(cr, QTC_CAIRO_COL(qtcPalette.menuitem[QT_FRAME_DARK_SHADOW]), 1.0, x, y, width);
            drawHLine(cr, QTC_CAIRO_COL(qtcPalette.menuitem[0]), 1.0, x, y+height-1, width);
        }
    }
    QTC_CAIRO_END
}

static void gtkDrawHandle(GtkStyle *style, GdkWindow *window, GtkStateType state,
                          GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget,
                          const gchar *detail, gint x, gint y, gint width,
                          gint height, GtkOrientation orientation)
{
    gboolean paf=WIDGET_TYPE_NAME("PanelAppletFrame");
    QTC_CAIRO_BEGIN

    FN_CHECK

#ifdef QTC_DEBUG
printf("Draw handle %d %d %d %d %s  ", state, shadow_type, width, height, detail ? detail : "NULL");
debugDisplayWidget(widget, 3);
#endif

    sanitizeSize(window, &width, &height);
    gtk_style_apply_default_background(style, window, widget && !GTK_WIDGET_NO_WINDOW(widget), state,
                                       area, x, y, width, height);

    if (DETAIL("dockitem") || paf)
    {
        if(GTK_ORIENTATION_HORIZONTAL==orientation)
            orientation=GTK_ORIENTATION_VERTICAL;
        else
            orientation=GTK_ORIENTATION_HORIZONTAL;
    }
    if(detail && (!strcmp(detail, "paned") || !strcmp(detail+1, "paned")))
    {
        GdkColor *cols=opts.coloredMouseOver && GTK_STATE_PRELIGHT==state
                        ? qtcPalette.mouseover
                        : qtcPalette.background;

        gtkDrawFlatBox(style, window, state, shadow_type, area, widget, QTC_PANED, x, y, width,
                      height);

        switch(opts.splitters)
        {
            case LINE_DOTS:
            default:
                drawDots(cr, x, y, width, height, height>width, NUM_SPLITTER_DASHES, 1,
                         cols, area, 0, 5);
                break;
            case LINE_SUNKEN:
                drawLines(cr, x, y, width, height, height>width, NUM_SPLITTER_DASHES, 1,
                          cols, area, 3, 1, TRUE);
            case LINE_FLAT:
                drawLines(cr, x, y, width, height, height>width, NUM_SPLITTER_DASHES, 3,
                          cols, area, 3, 0, FALSE);
                break;
            case LINE_DASHES:
                drawLines(cr, x, y, width, height, height>width, NUM_SPLITTER_DASHES, 1,
                          cols, area, 3, 0, TRUE);
        }
    }
    /* Note: I'm not sure why the 'widget && GTK_IS_HANDLE_BOX(widget)' is in the following 'if' - its been there for a while.
             But this breaks the toolbar handles for Java Swing apps. I'm leaving it in for non Java apps, as there must've been
             a reason for it.... */
    else if((DETAIL("handlebox") && (GTK_APP_JAVA==qtSettings.app || (widget && GTK_IS_HANDLE_BOX(widget)))) ||
            DETAIL("dockitem") || paf)
    {
#ifdef QTC_MOUSEOVER_HANDLES
        int *handleHash=NULL;

        if(opts.coloredMouseOver && !isMozilla())
        {
            handleHash=lookupToolbarHandleHash(widget, FALSE);

            if(!handleHash)
            {
                lookupToolbarHandleHash(widget, TRUE); /* Create hash entry... */
                gtk_widget_add_events(widget, GDK_LEAVE_NOTIFY_MASK|GDK_POINTER_MOTION_MASK);
                g_signal_connect(G_OBJECT(widget), "unrealize", G_CALLBACK(toolbarHandleDeleteEvent),
                                 widget);
                g_signal_connect(G_OBJECT(widget), "event", G_CALLBACK(toolbarHandleEvent), widget);
            }
        }
#endif
        if(paf)  /* The paf here is expected to be on the gnome panel */
            if(height<width)
                y++;
            else
                x++;
        else
            gtkDrawBox(style, window, state, shadow_type, area, widget, "handlebox", x, y, width,
                       height);

        GdkColor *cols=
#ifdef QTC_MOUSEOVER_HANDLES
                    opts.coloredMouseOver && handleHash && *handleHash
                        ? qtcPalette.mouseover :
#endif

                        qtcPalette.background;

        switch(opts.handles)
        {
            case LINE_DOTS:
                drawDots(cr, x, y, width, height, height<width, 2, 5, cols, area, 2, 5);
                break;
            case LINE_DASHES:
                if(height>width)
                    drawLines(cr, x+3, y, 3, height, TRUE, (height-8)/2, 0,
                              cols, area, 5, 0, TRUE);
                else
                    drawLines(cr, x, y+3, width, 3, FALSE, (width-8)/2, 0,
                              cols, area, 5, 0, TRUE);
                break;
            case LINE_FLAT:
                drawLines(cr, x, y, width, height, height<width, 2, 4, cols,
                          area, 4, 0, FALSE);
                break;
            default:
                drawLines(cr, x, y, width, height, height<width, 2, 4, cols,
                          area, 3, 1, TRUE);
        }
    }
    QTC_CAIRO_END
}

static void drawArrowPolygon(GdkWindow *window, GdkGC *gc, GdkRectangle *area, GdkPoint *points, int npoints, gboolean fill)
{
    if(area)
        gdk_gc_set_clip_rectangle(gc, area);

    gdk_draw_polygon(window, gc, FALSE, points, npoints);
    if(fill)
        gdk_draw_polygon(window, gc, TRUE, points, npoints);

    if(area)
        gdk_gc_set_clip_rectangle(gc, NULL);

/*
    The following produces slightly blurred looking arrows - hence I just use Gdk direct.
    int i;
    setCairoClipping(cr, area, NULL);
    cairo_new_path(cr);
    cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col));
    for(i=0; i<npoints; ++i)
        cairo_line_to(cr, points[i].x+0.5, points[i].y+0.5);
    cairo_line_to(cr, points[0].x+0.5, points[0].y+0.5);
    cairo_stroke_preserve(cr);
    cairo_fill(cr);
    unsetCairoClipping(cr);
*/
}

static void drawArrow(GdkWindow *window, GdkGC *gc, GdkRectangle *area, GtkArrowType arrow_type,
                      gint x, gint y, gboolean small, gboolean fill)
{
    if(small)
        switch(arrow_type)
        {
            case GTK_ARROW_UP:
            {
                GdkPoint a[]={{x+2,y},  {x,y-2},  {x-2,y},   {x-2,y+1}, {x-1,y}, {x+1,y}, {x+2,y+1}};
                drawArrowPolygon(window, gc, area, a, opts.vArrows ? 7 : 3, fill);
                break;
            }
            case GTK_ARROW_DOWN:
            {
                GdkPoint a[]={{x+2,y},  {x,y+2},  {x-2,y},   {x-2,y-1}, {x-1,y}, {x+1,y}, {x+2,y-1}};
                drawArrowPolygon(window, gc, area, a, opts.vArrows ? 7 : 3, fill);
                break;
            }
            case GTK_ARROW_RIGHT:
            {
                GdkPoint a[]={{x,y-2},  {x+2,y},  {x,y+2},   {x-1,y+2}, {x,y+1}, {x,y-1}, {x-1,y-2}};
                drawArrowPolygon(window, gc, area, a, opts.vArrows ? 7 : 3, fill);
                break;
            }
            case GTK_ARROW_LEFT:
            {
                GdkPoint a[]={{x,y-2},  {x-2,y},  {x,y+2},   {x+1,y+2}, {x,y+1}, {x,y-1}, {x+1,y-2}};
                drawArrowPolygon(window, gc, area, a, opts.vArrows ? 7 : 3, fill);
                break;
            }
            default:
                return;
        }
    else /* Large arrows... */
        switch(arrow_type)
        {
            case GTK_ARROW_UP:
            {
                GdkPoint a[]={{x+3,y+1},  {x,y-2},  {x-3,y+1},    {x-2, y+2},  {x,y},  {x+2,y+2}};
                drawArrowPolygon(window, gc, area, a, opts.vArrows ? 6 : 3, fill);
                break;
            }
            case GTK_ARROW_DOWN:
            {
                GdkPoint a[]={{x+3,y-1},  {x,y+2},  {x-3,y-1},   {x-2,y-2},  {x,y}, {x+2,y-2}};
                drawArrowPolygon(window, gc, area, a, opts.vArrows ? 6 : 3, fill);
                break;
            }
            case GTK_ARROW_RIGHT:
            {
                GdkPoint a[]={{x-1,y-3},  {x+2,y},  {x-1,y+3},   {x-2,y+2}, {x,y}, {x-2,y-2}};
                drawArrowPolygon(window, gc, area, a, opts.vArrows ? 6 : 3, fill);
                break;
            }
            case GTK_ARROW_LEFT:
            {
                GdkPoint a[]={{x+1,y-3},  {x-2,y},  {x+1,y+3},   {x+2,y+2}, {x,y}, {x+2,y-2}};
                drawArrowPolygon(window, gc, area, a, opts.vArrows ? 6 : 3, fill);
                break;
            }
            default:
                return;
        }
}

static void gtkDrawArrow(GtkStyle *style, GdkWindow *window, GtkStateType state,
                         GtkShadowType shadow, GdkRectangle *area, GtkWidget *widget,
                         const gchar *detail, GtkArrowType arrow_type,
                         gboolean fill, gint x, gint y, gint width, gint height)
{
//    QTC_CAIRO_BEGIN

#ifdef QTC_DEBUG
printf("Draw arrow %d %d %d %d %d %d %d %s  ", state, shadow, arrow_type, x, y, width, height, detail ? detail : "NULL");
debugDisplayWidget(widget, 3);
#endif
    if(DETAIL("arrow"))
    {
        if(isOnCombo(widget, 0) && !isOnComboEntry(widget, 0))
        {
            x++;

            drawArrow(window, style->text_gc[QTC_ARROW_STATE(state)], area,  GTK_ARROW_UP,
                      x+(width>>1), y+(height>>1)-(LARGE_ARR_HEIGHT-1), FALSE, TRUE);
            drawArrow(window, style->text_gc[QTC_ARROW_STATE(state)], area,  GTK_ARROW_DOWN,
                      x+(width>>1), y+(height>>1)+(LARGE_ARR_HEIGHT-1), FALSE, TRUE);
        }
        else
            drawArrow(window, style->text_gc[QTC_ARROW_STATE(state)], area,  arrow_type,
                      x+(width>>1), y+(height>>1), FALSE, TRUE);
    }
    else
    {
        int      isSpinButton = DETAIL("spinbutton"),
                 a_width=LARGE_ARR_WIDTH,
                 a_height=LARGE_ARR_HEIGHT;
        gboolean sbar=detail && ( 0==strcmp(detail, "hscrollbar") || 0==strcmp(detail, "vscrollbar") ||
                                  0==strcmp(detail, "stepper"));
        int      stepper=sbar ? getStepper(widget, x, y, 15, 15) : QTC_STEPPER_NONE;

/*
#if GTK_CHECK_VERSION(2, 10, 0)
        if(sbar && GTK_STATE_INSENSITIVE==state)
            state=GTK_STATE_NORMAL;
#else
*/
        if(GTK_IS_RANGE(widget) && sbar)
            setState(widget, &state, NULL);
/*
#endif
*/
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
        if(isSpinButton)
        {
            if(GTK_ARROW_UP==arrow_type)
                y++;
            a_height = SMALL_ARR_HEIGHT;
            a_width = SMALL_ARR_WIDTH;
        }
        else if(GTK_ARROW_LEFT==arrow_type || GTK_ARROW_RIGHT==arrow_type || DETAIL("menuitem"))
        {
            a_width = LARGE_ARR_HEIGHT;
            a_height = LARGE_ARR_WIDTH;
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

        if(GTK_STATE_ACTIVE==state && (sbar || isSpinButton))
        {
            x++;
            y++;
        }

        if(sbar)
            switch(stepper)
            {
                case QTC_STEPPER_B:
                    if(GTK_ARROW_RIGHT==arrow_type)
                        x--;
                    else
                        y--;
                    break;
                case QTC_STEPPER_C:
                    if(GTK_ARROW_LEFT==arrow_type)
                        x++;
                    else
                        y++;
                default:
                    break;
            }

        if(isSpinButton && !QTC_DO_EFFECT)
            if(GTK_ARROW_UP==arrow_type)
                y--;
            else
                y++;

        drawArrow(window, style->text_gc[QTC_IS_MENU_ITEM(widget) && GTK_STATE_PRELIGHT==state
                           ? GTK_STATE_SELECTED : QTC_ARROW_STATE(state)],
                  area, arrow_type, x, y, isSpinButton, TRUE);
    }
    //QTC_CAIRO_END
}

static void drawBox(GtkStyle *style, GdkWindow *window, GtkStateType state,
                    GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget,
                    const gchar *detail, gint x, gint y, gint width,
                    gint height, gboolean btn_down)
{
    gboolean sbar=detail && ( 0==strcmp(detail, "hscrollbar") || 0==strcmp(detail, "vscrollbar") ||
                              0==strcmp(detail, "stepper"));

    if(GTK_IS_RANGE(widget) && sbar)
        setState(widget, &state, &btn_down);
    {

    gboolean pbar=DETAIL("bar") && GTK_IS_PROGRESS_BAR(widget),
             qtc_paned=!pbar && IS_QTC_PANED,
             slider=!qtc_paned && (DETAIL("slider") || DETAIL("qtc-slider")),
             hscale=!slider && DETAIL("hscale"),
             vscale=!hscale && DETAIL("vscale"),
             menubar=!vscale && DETAIL("menubar"),
             menuitem=!menubar && DETAIL("menuitem"),
             button=!menuitem && DETAIL("button"),
             togglebutton=!button && DETAIL("togglebutton"),
             optionmenu=!togglebutton && DETAIL("optionmenu"),
             stepper=!optionmenu && DETAIL("stepper"),
             checkbox=!stepper && DETAIL(QTC_CHECKBOX),
             vscrollbar=!checkbox && DETAIL("vscrollbar"),
             hscrollbar=!vscrollbar && DETAIL("hscrollbar"),
             rev=reverseLayout(widget) || (widget && reverseLayout(widget->parent)),
             activeWindow=TRUE;
    GdkColor new_cols[TOTAL_SHADES+1],
             *btn_colors;
    int      bgnd=getFill(state, btn_down/*, DETAIL(QTC_CHECKBOX)*/),
             round=getRound(detail, widget, x, y, width, height, rev);
    gboolean lvh=isListViewHeader(widget),
             sunken=btn_down ||(GTK_IS_BUTTON(widget) && GTK_BUTTON(widget)->depressed) ||
                    GTK_STATE_ACTIVE==state || (2==bgnd || 3==bgnd);

    QTC_CAIRO_BEGIN

    if(button && GTK_IS_TOGGLE_BUTTON(widget))
    {
        button=FALSE;
        togglebutton=TRUE;
    }
#ifdef QTC_DEBUG
printf("Draw box %d %d %d %d %d %d %d %s  ", btn_down, state, shadow_type, x, y, width, height,
       detail ? detail : "NULL");
debugDisplayWidget(widget, 3);
#endif

    sanitizeSize(window, &width, &height);

    if(useButtonColor(detail))
    {
        if(slider|hscale|vscale|sbar && GTK_STATE_INSENSITIVE==state)
            btn_colors=qtcPalette.background;
        else if(QT_CUSTOM_COLOR_BUTTON(style))
        {
            shadeColors(&(style->bg[state]), new_cols);
            btn_colors=new_cols;
        }
        else
            QTC_SET_BTN_COLS(slider, hscale|vscale, lvh)
    }

    g_return_if_fail(style != NULL);
    g_return_if_fail(window != NULL);

    if((-1==width) &&(-1==height))
        gdk_window_get_size(window, &width, &height);
    else if(-1==width)
        gdk_window_get_size(window, &width, NULL);
    else if(-1==height)
        gdk_window_get_size(window, NULL, &height);

    if(menubar && !isMozilla() && GTK_APP_JAVA!=qtSettings.app &&
       (opts.menubarMouseOver || opts.shadeMenubarOnlyWhenActive))
    {
        GtkWidget **mbHash=lookupMenubarHash(widget, FALSE);
        GtkWindow *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(widget));

        if(!mbHash)
        {
            lookupMenubarHash(widget, TRUE); /* Create hash entry... */
            if(opts.menubarMouseOver)
            {
                gtk_widget_add_events(widget, GDK_LEAVE_NOTIFY_MASK|GDK_POINTER_MOTION_MASK);
                g_signal_connect(G_OBJECT(widget), "unrealize", G_CALLBACK(menubarDeleteEvent), widget);
                g_signal_connect(G_OBJECT(widget), "event", G_CALLBACK(menubarEvent), widget);
            }

            if(opts.shadeMenubarOnlyWhenActive && topLevel && GTK_IS_WINDOW(topLevel))
                g_signal_connect(G_OBJECT(topLevel), "event", G_CALLBACK(windowEvent), widget);
        }

        if(topLevel && GTK_IS_WINDOW(topLevel) && opts.shadeMenubarOnlyWhenActive)
            activeWindow=gtk_window_has_toplevel_focus(GTK_WINDOW(topLevel));
    }

    if(detail && (0==strcmp(detail, "spinbutton_up") || 0==strcmp(detail, "spinbutton_down")))
    {
        EWidget wid=0==strcmp(detail, "spinbutton_up")
                        ? WIDGET_SPIN_UP : WIDGET_SPIN_DOWN;

        if(WIDGET_SPIN_UP==wid)
        {
            if(QTC_DO_EFFECT)
            {
                drawEtch(cr, area, NULL, x-2, y, width+2, height*2, FALSE);
                y++;
                width--;
            }
            height++;
        }
        else if (QTC_DO_EFFECT)
        {
            drawEtch(cr, area, NULL, x-2, y-height, width+2, height*2, FALSE);
            height--;
            width--;
        }

        drawBgnd(cr, &btn_colors[bgnd], widget, area, x+1, y+1, width-2, height-2);
        drawLightBevel(cr, style, window, state, area, NULL, x, y, width, height, &btn_colors[bgnd],
                       btn_colors, round, wid, BORDER_FLAT,
                       DF_DO_CORNERS|DF_DO_BORDER|
                       (sunken ? DF_SUNKEN : 0));
    }
    else if(DETAIL("spinbutton"))
        gtk_style_apply_default_background(style, window, widget && !GTK_WIDGET_NO_WINDOW(widget),
                                           GTK_STATE_INSENSITIVE==state 
                                               ? GTK_STATE_INSENSITIVE 
                                               : GTK_STATE_NORMAL, 
                                           area, x, y, width, height);
    else if(detail &&( button || togglebutton || optionmenu || checkbox || sbar || hscale || vscale ||
                       stepper || slider || qtc_paned))
    {
        gboolean combo=0==strcmp(detail, "optionmenu") || isOnCombo(widget, 0),
                 combo_entry=combo && isOnComboEntry(widget, 0),
                 horiz_tbar,
                 tbar_button=isButtonOnToolbar(widget, &horiz_tbar),
                 handle_button=!tbar_button && isButtonOnHandlebox(widget, &horiz_tbar);

//        drawBgnd(cr, &btn_colors[bgnd], widget, area, x, y, width, height); // CPD removed as it messes up toolbars and firefox3

        if(combo && !sunken && isActiveCombo(widget))
        {
            sunken=TRUE;
            bgnd=4;
        }

        if(!qtc_paned)
        {
            gboolean horiz=(tbar_button || handle_button) && IS_GLASS(opts.appearance) &&
                           IS_GLASS(opts.toolbarAppearance)
                               ? horiz_tbar
                               : (slider && width<height) || vscrollbar || vscale
                                   ? FALSE
                                   : TRUE,
                     defBtn=GTK_STATE_INSENSITIVE!=state && (button || togglebutton) && GTK_WIDGET_HAS_DEFAULT(widget);

            if(lvh)
            {
                drawBevelGradient(cr, style, area, NULL, x, y, width, height, &btn_colors[bgnd],
                                  sunken ? SHADE_BEVEL_GRAD_SEL_LIGHT
                                         : SHADE_BEVEL_GRAD_LIGHT,
                                  sunken ? SHADE_BEVEL_GRAD_SEL_DARK
                                         : SHADE_BEVEL_GRAD_DARK, horiz, !sunken, sunken,
                                  opts.lvAppearance, WIDGET_LISTVIEW_HEADER);

                if(GTK_STATE_PRELIGHT==state && opts.coloredMouseOver)
                {
                    drawHLine(cr, QTC_CAIRO_COL(qtcPalette.mouseover[ORIGINAL_SHADE]), 1.0, x, y+height-2, width);
                    drawHLine(cr, QTC_CAIRO_COL(qtcPalette.mouseover[QT_STD_BORDER]), 1.0, x, y+height-1, width);
                }
                else
                {
                    if(APPEARANCE_RAISED==opts.lvAppearance)
                        drawHLine(cr, QTC_CAIRO_COL(qtcPalette.background[4]), 1.0, x, y+height-2, width);
                    drawHLine(cr, QTC_CAIRO_COL(qtcPalette.background[QT_STD_BORDER]), 1.0, x, y+height-1, width);
                }

                if(x>3 && height>10)
                {
                    drawVLine(cr, QTC_CAIRO_COL(qtcPalette.background[QT_STD_BORDER]), 1.0, x, y+4, height-8);
                    drawVLine(cr, QTC_CAIRO_COL(qtcPalette.background[0]), 1.0, x+1, y+4, height-8);
                }
            }
            else
            {
                EWidget  widgetType=isComboBoxButton(widget)
                                    ? WIDGET_COMBO_BUTTON
                                    : slider
                                        ? WIDGET_SB_SLIDER
                                        : hscale||vscale
                                        ? WIDGET_SLIDER
                                            : lvh
                                                ? WIDGET_LISTVIEW_HEADER
                                                : combo || optionmenu
                                                    ? WIDGET_STD_BUTTON
                                                    : tbar_button
                                                        ?
#ifdef QTC_DONT_COLOUR_MOUSEOVER_TBAR_BUTTONS
                                                            WIDGET_UNCOLOURED_MO_BUTTON
#else
                                                            WIDGET_STD_BUTTON
#endif
                                                        : togglebutton
                                                            ? WIDGET_TOGGLE_BUTTON
                                                            : checkbox
                                                                ? WIDGET_CHECKBOX
                                                                    : button
                                                                        ? defBtn
                                                                            ? WIDGET_DEF_BUTTON
                                                                            : WIDGET_STD_BUTTON
                                                                        : stepper || sbar
                                                                            ? WIDGET_SB_BUTTON
                                                                            : WIDGET_OTHER;

#ifdef QTC_DONT_COLOUR_MOUSEOVER_TBAR_BUTTONS
                /* Try and guess if this button is a toolbar button... */
                if(WIDGET_STD_BUTTON==widgetType && isMozillaWidget(widget) && GTK_IS_BUTTON(widget) && DETAIL("button") &&
                   ((width>22 && width<50 && height>30) || height>32))
                    widgetType=WIDGET_UNCOLOURED_MO_BUTTON;
#endif

                /* For some reason SWT combo's dont un-prelight when activated! So dont pre-light at all! */
                if(GTK_APP_JAVA_SWT==qtSettings.app && WIDGET_STD_BUTTON==widgetType && GTK_STATE_PRELIGHT==state && WIDGET_COMBO==widgetType)
                {
                    state=GTK_STATE_NORMAL;
                    bgnd=getFill(state, btn_down);
                }
                else if(WIDGET_SB_BUTTON==widgetType && GTK_APP_MOZILLA!=qtSettings.app)
                    switch(getStepper(widget, x, y, width, height))
                    {
                        case QTC_STEPPER_B:
                            if(horiz)
                                x--, width++;
                            else
                                y--, height++;
                            break;
                        case QTC_STEPPER_C:
                            if(horiz)
                                width++;
                            else
                                height++;
                        default:
                            break;
                    }

                if(GTK_APP_MOZILLA!=qtSettings.app && slider && SCROLLBAR_NONE==opts.scrollbarType)
                {
                    GdkRectangle troughArea;

                    if(area)
                        troughArea=*area;
                    else
                    {
                        troughArea.x=x;
                        troughArea.y=y;
                        troughArea.height=height;
                        troughArea.width=width;
                    }

                    if(horiz)
                    {
                        if(x<=widget->allocation.x)
                        {
                            troughArea.x++;
                            troughArea.width--;
                        }
                        if((x+width)>=(widget->allocation.x+widget->allocation.width-1))
                            troughArea.width--;
                    }
                    else
                    {
                        if(y<=widget->allocation.y)
                        {
                            troughArea.y++;
                            troughArea.height--;
                        }
                        if((y+height)>=(widget->allocation.y+widget->allocation.height-1))
                            troughArea.height--;
                    }
                    drawBox(style, window, state, shadow_type, &troughArea, widget, "trough",
                            horiz ? x-2 : x, horiz ? y : y-2, horiz ? width+4 : width,
                            horiz ? height : height+4, FALSE);
                }

#ifdef QTC_INCREASE_SB_SLIDER
                if(slider && widget && GTK_IS_RANGE(widget))
                {
                    GtkAdjustment *adj = GTK_RANGE(widget)->adjustment;
                    gboolean      horizontal = GTK_RANGE(widget)->orientation != GTK_ORIENTATION_HORIZONTAL;

                    if(adj->value <= adj->lower &&
                        (GTK_RANGE(widget)->has_stepper_a || GTK_RANGE(widget)->has_stepper_b))
                    {
                        if (horizontal)
                            y--, height++;
                        else
                            x--, width++;
                    }
                    if(adj->value >= adj->upper - adj->page_size &&
                        (GTK_RANGE(widget)->has_stepper_c || GTK_RANGE(widget)->has_stepper_d))
                    {
                        if (horizontal)
                            height++;
                        else
                            width++;
                    }
                }
#endif

                if(defBtn && IND_TINT==opts.defBtnIndicator)
                    btn_colors=qtcPalette.defbtn;

                drawLightBevel(cr, style, window, state, area, NULL, x, y, width, height,
                               &btn_colors[bgnd], btn_colors, round, widgetType,
                               BORDER_FLAT, (sunken ? DF_SUNKEN : 0)|
                                            (lvh ? 0 : DF_DO_BORDER)|
                                            (horiz ? 0 : DF_VERT));
            }

            if(defBtn)
                if(IND_CORNER==opts.defBtnIndicator)
                {
                    int      offset=sunken ? 5 : 4,
                             etchOffset=QTC_DO_EFFECT ? 1 : 0;
                    GdkColor *col=&qtcPalette.mouseover[GTK_STATE_ACTIVE==state ? 0 : 4];

                    cairo_new_path(cr);
                    cairo_set_source_rgb(cr, QTC_CAIRO_COL(*col));
                    cairo_move_to(cr, x+offset+etchOffset, y+offset+etchOffset);
                    cairo_line_to(cr, x+offset+6+etchOffset, y+offset+etchOffset);
                    cairo_line_to(cr, x+offset+etchOffset, y+offset+6+etchOffset);
                    cairo_fill(cr);
                }
                else if(IND_COLORED==opts.defBtnIndicator && (COLORED_BORDER_SIZE>2))
                {
                    GdkColor  *cols=qtcPalette.mouseover && GTK_STATE_PRELIGHT==state
                                        ? qtcPalette.mouseover : qtcPalette.defbtn;
                    int       o=QTC_DO_EFFECT ? 1 : 0; // offset needed because of etch
                    GdkPoint  outer[4]={ {x, y}, {x+width, y}, {x+width, y+height},
                                         {x, y+height} },
                              inner[4]={ {x+COLORED_BORDER_SIZE,       y+(COLORED_BORDER_SIZE+o) },
                                         {x+width-(COLORED_BORDER_SIZE+(o*2)), y+(COLORED_BORDER_SIZE+o)},
                                         {x+width-(COLORED_BORDER_SIZE+(o*2)), y+height-(COLORED_BORDER_SIZE+o)},
                                         {x+COLORED_BORDER_SIZE,       y+height-(COLORED_BORDER_SIZE+o)} };
                    GdkRegion *outer_region=gdk_region_polygon(outer, 4, GDK_EVEN_ODD_RULE),
                              *inner_region=gdk_region_polygon(inner, 4, GDK_EVEN_ODD_RULE);

                    gdk_region_xor(inner_region, outer_region);

                    drawLightBevel(cr, style, window, state, NULL, inner_region, x, y, width, height,
                                   &cols[QTC_MO_DEF_BTN], cols, round, WIDGET_DEF_BUTTON,
                                   BORDER_FLAT, /*(draw_inside ? DF_DRAW_INSIDE : 0) |*/
                                   DF_DO_CORNERS|(sunken ? DF_SUNKEN : 0)|
                                   DF_DO_BORDER|(horiz ? 0 : DF_VERT));

                    gdk_region_destroy(inner_region);
                    gdk_region_destroy(outer_region);
                }
        }

        if(opts.comboSplitter)
        {
            if(optionmenu)
            {
                GtkRequisition indicator_size;
                GtkBorder      indicator_spacing;
                int            cx=x, cy=y, cheight=height, cwidth=width,
                               ind_width=0,
                               darkLine=QT_BORDER(GTK_STATE_INSENSITIVE!=state);

                optionMenuGetProps(widget, &indicator_size, &indicator_spacing);

                ind_width=indicator_size.width+indicator_spacing.left+indicator_spacing.right;

                if(QTC_DO_EFFECT)
                    cx--;

        #if (GTK_MAJOR_VERSION>1) && (GTK_MINOR_VERSION<2)
                cy++;
                cheight-=2;
        #endif
                cy+=3;
                cheight-=6;

                if(sunken)
                    cx++, cy++, cheight--;

                drawVLine(cr, QTC_CAIRO_COL(btn_colors[darkLine]), 1.0,
                        cx + (rev ? ind_width+QT_STYLE->xthickness
                                    : (cwidth - ind_width - QT_STYLE->xthickness)),
                        cy + QT_STYLE->ythickness-1, cheight-3);

                if(!sunken)
                    drawVLine(cr, QTC_CAIRO_COL(btn_colors[0]), 1.0,
                            cx + (rev ? ind_width+QT_STYLE->xthickness
                                        : (cwidth - ind_width - QT_STYLE->xthickness))+1,
                            cy + QT_STYLE->ythickness-1, cheight-3);
            }
            else if((button || togglebutton) && (combo || combo_entry))
            {
                int vx=x+(width - (1 + (combo_entry ? 24 : 20))),
                    vwidth=width-(vx-x),
                    darkLine=QT_BORDER(GTK_STATE_INSENSITIVE!=state);

                if(rev)
                {
                    vx=x+LARGE_ARR_WIDTH;
                    if(combo_entry)
                        vx+=2;
                }

                if(QTC_DO_EFFECT)
                    vx-=2;

                if(!combo_entry)
                {
                    drawVLine(cr, QTC_CAIRO_COL(btn_colors[darkLine]), 1.0,
                            vx+(rev ? LARGE_ARR_WIDTH+4 : 0), y+4, height-8);

                    if(!sunken)
                        drawVLine(cr, QTC_CAIRO_COL(btn_colors[0]), 1.0,
                                vx+(rev ? LARGE_ARR_WIDTH+4 : 0)+1, y+4, height-8);
                }
            }
        }
    }
    else if(detail && (0==strcmp(detail, "buttondefault") ||
                       0==strcmp(detail, "togglebuttondefault")))
    {
    }
    else if(widget && DETAIL("trough"))
    {
        gboolean pbar=GTK_IS_PROGRESS_BAR(widget),
                 scale=!pbar && GTK_IS_SCALE(widget);
        int      border=QT_BORDER(GTK_STATE_INSENSITIVE!=state || !scale);
        GdkColor *bgndcol=&qtcPalette.background[2];
        gboolean horiz=GTK_IS_RANGE(widget)
                        ? GTK_ORIENTATION_HORIZONTAL==GTK_RANGE(widget)->orientation
                        : width>height;

        if(scale)
        {
            GtkAdjustment *adjustment = gtk_range_get_adjustment(GTK_RANGE(widget));
            int           used_x=x, used_y=y, used_h=0, used_w=0,
                          pos=(int)(((double)(horiz ? width : height) /
                                     (adjustment->upper - adjustment->lower))  *
                                 (adjustment->value - adjustment->lower));
            gboolean      inverted=gtk_range_get_inverted(GTK_RANGE(widget)),
                          doEtch=QTC_DO_EFFECT;
            int           troughSize=SLIDER_TROUGH_SIZE+(doEtch ? 2 : 0);

            if(horiz && rev)
                inverted=!inverted;

            gtk_style_apply_default_background(style, window, widget && !GTK_WIDGET_NO_WINDOW(widget),
                                               GTK_STATE_INSENSITIVE==state
                                                   ? GTK_STATE_INSENSITIVE 
                                                   : GTK_STATE_NORMAL, 
                                               area, x, y, width, height);

            if(horiz)
            {
                y +=(height - troughSize)>>1;
                height = troughSize;
                used_y=y;
                used_h=height;
            }
            else
            {
                x +=(width - troughSize)>>1;
                width = troughSize;
                used_x=x;
                used_w=width;
            }

            if(GTK_STATE_INSENSITIVE==state)
                bgndcol=&qtcPalette.background[ORIGINAL_SHADE];
            drawLightBevel(cr, style, window, state, area, NULL, x, y, width, height,
                           bgndcol, qtcPalette.background,
                           ROUNDED_ALL, WIDGET_SLIDER_TROUGH,
                           BORDER_FLAT, DF_DO_CORNERS|DF_SUNKEN|DF_DO_BORDER|
                           (horiz ? 0 : DF_VERT));

            if(opts.fillSlider && adjustment->upper!=adjustment->lower && state!=GTK_STATE_INSENSITIVE)
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
                    GdkColor *usedcol=qtcPalette.slider
                                        ? &qtcPalette.slider[ORIGINAL_SHADE]
                                        : qtcPalette.mouseover
                                            ? &qtcPalette.mouseover[ORIGINAL_SHADE]
                                            : &qtcPalette.menuitem[1];

                    drawLightBevel(cr, style, window, state, area, NULL, used_x, used_y, used_w, used_h,
                                   usedcol, qtcPalette.background,
                                   ROUNDED_ALL, WIDGET_SLIDER_TROUGH,
                                   BORDER_FLAT, DF_DO_CORNERS|DF_SUNKEN|DF_DO_BORDER|
                                   (horiz ? 0 : DF_VERT));
                }
            }
        }
        else if(pbar)
        {
            gboolean doEtch=QTC_DO_EFFECT;

            drawAreaColor(cr, area, NULL, &qtcPalette.background[ORIGINAL_SHADE], x, y, width, height);

            if(doEtch)
                x++, y++, width-=2, height-=2;

            if(opts.gradientPbGroove)
                drawBevelGradient(cr, style, area, NULL, x+1, y+1, width-2, height-2,
                                  GTK_STATE_INSENSITIVE==state ? &(qtcPalette.background[ORIGINAL_SHADE])
                                                               : &(style->base[state]),
                                  getWidgetShade(WIDGET_TROUGH, TRUE, FALSE, opts.progressAppearance),
                                  getWidgetShade(WIDGET_TROUGH, FALSE, FALSE, opts.progressAppearance),
                                  horiz, FALSE, FALSE, APPEARANCE_GRADIENT, WIDGET_TROUGH);
            else if(GTK_STATE_INSENSITIVE!=state)
                drawAreaColor(cr, area, NULL, &style->base[state], x+2, y+2, width-4, height-4);

            drawBorder(cr, widget && widget->parent ? widget->parent->style : style,
                       state, area, NULL, x, y, width, height,
                       NULL, ROUNDED_ALL, opts.gradientPbGroove ? BORDER_FLAT : BORDER_SUNKEN, WIDGET_OTHER,
                       DF_BLEND|DF_DO_CORNERS);

            if(doEtch)
                 drawEtch(cr, area, NULL, x-1, y-1, width+2, height+2, FALSE);
        }
        else /* Scrollbars... */
        {
            int sbarRound=ROUNDED_ALL;

            switch(opts.scrollbarType)
            {
                default:
                    break;
                case SCROLLBAR_NEXT:
                    sbarRound=horiz ? ROUNDED_LEFT : ROUNDED_TOP;
                    break;
                case SCROLLBAR_PLATINUM:
                    sbarRound=horiz ? ROUNDED_RIGHT : ROUNDED_BOTTOM;
                    break;
#ifdef QTC_SIMPLE_SCROLLBARS
                case SCROLLBAR_NONE:
                    sbarRound=ROUNDED_NONE;
#endif
            }

            drawLightBevel(cr, style, window, state, area, NULL, x, y, width, height,
                           &qtcPalette.background[2], qtcPalette.background, sbarRound, WIDGET_TROUGH,
                           BORDER_FLAT, DF_DO_CORNERS|DF_SUNKEN|DF_DO_BORDER|
                           (horiz ? 0 : DF_VERT));
        }
    }
    else if(widget && ( (detail && ( menubar || 0==strcmp(detail, "toolbar") ||
                                     0==strcmp(detail,"dockitem") ||
                                     0==strcmp(detail,"dockitem_bin") ||
                                     0==strcmp(detail, "handlebox") ||
                                     0==strcmp(detail,"handlebox_bin") ) )
                        || WIDGET_TYPE_NAME("PanelAppletFrame")))
    {
        if(GTK_SHADOW_NONE!=shadow_type)
        {
            GdkColor    bgnd=activeWindow && menubar && USE_SHADED_MENU_BAR_COLORS
                                    ? qtcPalette.menubar[ORIGINAL_SHADE]
                                    : style->bg[state];
            GdkColor    *col=activeWindow && menubar && USE_SHADED_MENU_BAR_COLORS
                                ? &qtcPalette.menubar[ORIGINAL_SHADE]
                                : &style->bg[state];
            EAppearance app=menubar ? opts.menubarAppearance : opts.toolbarAppearance;

            /* Toolbars and menus */
            if(!IS_FLAT(app))
            {
                if(activeWindow && menubar && SHADE_DARKEN==opts.shadeMenubars)
                    shade(&bgnd, &bgnd, MENUBAR_DARK_FACTOR);

                drawBevelGradient(cr, style, area, NULL, x, y, width,
                                height, &bgnd,
                                IS_GLASS(app)
                                    ? SHADE_BEVEL_GRAD_LIGHT
                                    : SHADE_MENU_LIGHT,
                                IS_GLASS(app)
                                    ? SHADE_BEVEL_GRAD_DARK
                                    : SHADE_MENU_DARK,
                                menubar
                                    ? TRUE
                                    : DETAIL("handlebox")
                                            ? width<height
                                            : width>height,
                                TRUE, FALSE, app, WIDGET_OTHER);
            }
            else if(activeWindow && menubar && SHADE_DARKEN==opts.shadeMenubars)
                drawAreaMod(cr, style, GTK_STATE_NORMAL, area, NULL, MENUBAR_DARK_FACTOR, x, y,
                            width, height);
            else
                drawAreaColor(cr, area, NULL, col, x, y, width, height);

            if(TB_NONE!=opts.toolbarBorders)
            {
                gboolean top=FALSE,
                         bottom=FALSE,
                         left=FALSE,
                         right=FALSE,
                         all=TB_LIGHT_ALL==opts.toolbarBorders || TB_DARK_ALL==opts.toolbarBorders;
                int      border=TB_DARK==opts.toolbarBorders || TB_DARK_ALL==opts.toolbarBorders ? 3 : 4;
                GdkColor *cols=activeWindow && menubar && USE_SHADED_MENU_BAR_COLORS
                                ? qtcPalette.menubar : qtcPalette.background;

                if(menubar)
                {
                    if(all)
                        top=bottom=left=right=TRUE;
                    else
                        bottom=TRUE;
                }
                else if(0==strcmp(detail,"toolbar")) /*  && (GTK_IS_TOOLBAR(widget) ||
                    WIDGET_TYPE_NAME("BonoboUIToolbar"))) */
                {
                    if(GTK_IS_TOOLBAR(widget))
                    {
                        if(all)
                            top=bottom=left=right=TRUE;
                        else if(GTK_ORIENTATION_HORIZONTAL==GTK_TOOLBAR(widget)->orientation)
                            top=bottom=TRUE;
                        else
                            left=right=TRUE;
                    }
                    else
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

                if(top)
                    drawHLine(cr, QTC_CAIRO_COL(cols[0]), 1.0, x, y, width);
                if(left)
                    drawVLine(cr, QTC_CAIRO_COL(cols[0]), 1.0, x, y, height);
                if(bottom)
                    drawHLine(cr, QTC_CAIRO_COL(cols[border]), 1.0, x, y + height - 1, width);
                if(right)
                    drawVLine(cr, QTC_CAIRO_COL(cols[border]), 1.0, x+width-1, y, height);
            }
        }
    }
    else if(widget && (menuitem || pbar))
    {
        GdkRegion  *region=NULL;
        GtkMenuBar *mb=menuitem ? isMenubar(widget, 0) : NULL;
        gboolean   active_mb=isMozilla() || (mb ? GTK_MENU_SHELL(mb)->active : FALSE),
                   horizPbar=isHorizontalProgressbar(widget);
        int        animShift=-PROGRESS_CHUNK_WIDTH;

#ifdef QTC_GTK2_MENU_STRIPE_HACK_MENU /* This hack doesnt work! not all items are gtkImageMenuItems's
         -> and if tey are they're drawn first incorrectly :-( */
        if(!mb && menuitem && GTK_IS_IMAGE_MENU_ITEM(widget) &&
           (0L==gtk_image_menu_item_get_image(GTK_IMAGE_MENU_ITEM(widget)) ||
            (GTK_IS_IMAGE(gtk_image_menu_item_get_image(GTK_IMAGE_MENU_ITEM(widget))) &&
             GTK_IMAGE_EMPTY==gtk_image_get_storage_type(GTK_IMAGE(gtk_image_menu_item_get_image(GTK_IMAGE_MENU_ITEM(widget)))))))
        {
            // Give it a blank icon - so that menuStripe looks ok, plus this matched KDE style!
            if(0L==gtk_image_menu_item_get_image(GTK_IMAGE_MENU_ITEM(widget)))
                gtk_image_menu_item_set_image(GTK_IS_IMAGE_MENU_ITEM(widget),
                                              gtk_image_new_from_pixbuf(getPixbuf(qtcPalette.check_radio, PIX_BLANK, 1.0)));
            else
                gtk_image_set_from_pixbuf(GTK_IMAGE(gtk_image_menu_item_get_image(GTK_IMAGE_MENU_ITEM(widget))),
                                          getPixbuf(qtcPalette.check_radio, PIX_BLANK, 1.0));
        }
#endif

        if(pbar && opts.fillProgress)
            x--, y--, width+=2, height+=2;

        if(pbar && STRIPE_NONE!=opts.stripedProgress)
        {
            GdkRectangle rect={x, y, width-2, height-2};
            int          stripeOffset;

            if(opts.animatedProgress && QTC_IS_PROGRESS_BAR(widget))
            {
                if(!GTK_PROGRESS(widget)->activity_mode)
                    qtc_animation_progressbar_add((gpointer)widget);

                animShift+=((int)(qtc_animation_elapsed(widget)*PROGRESS_CHUNK_WIDTH))%(PROGRESS_CHUNK_WIDTH*2);
            }

            constrainRect(&rect, area);
            region=gdk_region_rectangle(&rect);

            switch(opts.stripedProgress)
            {
                default:
                case STRIPE_PLAIN:
                    if(horizPbar)
                        for(stripeOffset=0; stripeOffset<(width+PROGRESS_CHUNK_WIDTH); stripeOffset+=(PROGRESS_CHUNK_WIDTH*2))
                        {
                            GdkRectangle inner_rect={x+stripeOffset+animShift, y+1, PROGRESS_CHUNK_WIDTH, height-2};

                            constrainRect(&inner_rect, area);
                            if(inner_rect.width>0 && inner_rect.height>0)
                            {
                                GdkRegion *inner_region=gdk_region_rectangle(&inner_rect);

                                gdk_region_xor(region, inner_region);
                                gdk_region_destroy(inner_region);
                            }
                        }
                    else
                        for(stripeOffset=0; stripeOffset<(height+PROGRESS_CHUNK_WIDTH); stripeOffset+=(PROGRESS_CHUNK_WIDTH*2))
                        {
                            GdkRectangle inner_rect={x+1, y+stripeOffset+animShift, width-2, PROGRESS_CHUNK_WIDTH};

                            /*constrainRect(&inner_rect, area);*/
                            if(inner_rect.width>0 && inner_rect.height>0)
                            {
                                GdkRegion *inner_region=gdk_region_rectangle(&inner_rect);

                                gdk_region_xor(region, inner_region);
                                gdk_region_destroy(inner_region);
                            }
                        }
                    break;
                case STRIPE_DIAGONAL:
                    if(horizPbar)
                        for(stripeOffset=0; stripeOffset<(width+height+2); stripeOffset+=(PROGRESS_CHUNK_WIDTH*2))
                        {
                            GdkPoint  a[4]={ {x+stripeOffset+animShift,                               y},
                                             {x+stripeOffset+animShift+PROGRESS_CHUNK_WIDTH,          y},
                                             {(x+stripeOffset+animShift+PROGRESS_CHUNK_WIDTH)-height, y+height-1},
                                             {(x+stripeOffset+animShift)-height,                      y+height-1}};
                            GdkRegion *inner_region=gdk_region_polygon(a, 4, GDK_EVEN_ODD_RULE);

                            gdk_region_xor(region, inner_region);
                            gdk_region_destroy(inner_region);
                        }
                    else
                        for(stripeOffset=0; stripeOffset<(height+width+2); stripeOffset+=(PROGRESS_CHUNK_WIDTH*2))
                        {
                            GdkPoint  a[4]={{x,         y+stripeOffset+animShift},
                                            {x+width-1, (y+stripeOffset+animShift)-width},
                                            {x+width-1, (y+stripeOffset+animShift+PROGRESS_CHUNK_WIDTH)-width},
                                            {x,         y+stripeOffset+animShift+PROGRESS_CHUNK_WIDTH}};
                            GdkRegion *inner_region=gdk_region_polygon(a, 4, GDK_EVEN_ODD_RULE);

                            gdk_region_xor(region, inner_region);
                            gdk_region_destroy(inner_region);
                        }
                    if(area)
                    {
                        GdkRegion *outer_region=gdk_region_rectangle(area);

                        gdk_region_intersect(region, outer_region);
                        gdk_region_destroy(outer_region);
                    }
            }
        }

        /* The following 'if' is just a hack for a menubar item problem with pidgin. Sometime, a 12pix width
           empty menubar item is drawn on the right - and doesnt disappear! */
        if(!mb || width>12)
        {
            gboolean grayItem=!opts.colorMenubarMouseOver && mb && !active_mb && GTK_APP_OPEN_OFFICE!=qtSettings.app;
            GdkColor *itemCols=grayItem ? qtcPalette.background : qtcPalette.menuitem;
            GdkColor *bgnd=qtcPalette.menubar && mb && !isMozilla() && GTK_APP_JAVA!=qtSettings.app
                            ? &qtcPalette.menubar[ORIGINAL_SHADE] : NULL;
            int      round=pbar ? progressbarRound(widget, rev)
                                : mb
                                    ? active_mb && opts.roundMbTopOnly
                                        ? ROUNDED_TOP
                                        : ROUNDED_ALL
                                    : ROUNDED_ALL,
                     new_state=GTK_STATE_PRELIGHT==state ? GTK_STATE_NORMAL : state;
            gboolean border=pbar || menuitem || mb,
                     stdColors=!mb || SHADE_BLEND_SELECTED!=opts.shadeMenubars,
                     horiz=horizPbar || menuitem;

            if(!pbar && !border)
                x--, y--, width+=2, height+=2;

            if(!opts.borderMenuitems && (mb || menuitem))
                drawBevelGradient(cr, style, area, region, x, y, width, height,
                                    &itemCols[ORIGINAL_SHADE],
                                    getWidgetShade(WIDGET_MENU_ITEM, TRUE, FALSE, opts.menuitemAppearance),
                                    getWidgetShade(WIDGET_MENU_ITEM, FALSE, FALSE, opts.menuitemAppearance),
                                    TRUE, TRUE, FALSE, opts.menuitemAppearance, WIDGET_MENU_ITEM);
            else if(stdColors)
            {
                if(pbar && (horizPbar ? width : height)<3)
                {
                    if((!pbar || !opts.fillProgress) && border && stdColors)
                        drawAreaColor(cr, area, NULL, &itemCols[ORIGINAL_SHADE], x, y, width, height);
                }
                else
                    drawLightBevel(cr, style, window, new_state, area, NULL, x, y,
                                width, height, &itemCols[ORIGINAL_SHADE],
                                itemCols, round, pbar ? WIDGET_PROGRESSBAR : WIDGET_MENU_ITEM, BORDER_FLAT,
                                DF_DRAW_INSIDE|(horiz ? 0 : DF_VERT)|
                                ((!pbar || !opts.fillProgress) && border && stdColors ? DF_DO_BORDER : 0)|
                                (activeWindow && USE_SHADED_MENU_BAR_COLORS ? 0 : DF_DO_CORNERS));
            }
            else
            {
                if(width>2 && height>2)
                    drawBevelGradient(cr, style, area, region, x+1, y+1, width-2, height-2,
                                    &itemCols[ORIGINAL_SHADE],
                                    getWidgetShade(WIDGET_MENU_ITEM, TRUE, FALSE, opts.menuitemAppearance),
                                    getWidgetShade(WIDGET_MENU_ITEM, FALSE, FALSE, opts.menuitemAppearance),
                                    TRUE, TRUE, FALSE, opts.menuitemAppearance, WIDGET_MENU_ITEM);

                realDrawBorder(cr, style, state, area, NULL, x, y, width, height,
                               itemCols, round, BORDER_FLAT,
                               WIDGET_OTHER, 0, 0);
            }
            if(pbar && opts.stripedProgress && width>4 && height>4)
                drawLightBevel(cr, style, window, new_state, NULL, region, x, y,
                            width, height, &qtcPalette.menuitem[1],
                            qtcPalette.menuitem, round, WIDGET_PROGRESSBAR, BORDER_FLAT,
                            DF_DRAW_INSIDE|(opts.fillProgress ? 0 : DF_DO_BORDER)|(horiz ? 0 : DF_VERT)|
                            (activeWindow && USE_SHADED_MENU_BAR_COLORS ? 0 : DF_DO_CORNERS));

            if(!opts.fillProgress && pbar && QTC_ROUNDED && ROUNDED_ALL!=round && width>4 && height>4)
            {
                /*if(!isMozilla())
                {
                    x--; y--; width+=2; height+=2;
                }*/
                cairo_new_path(cr);
                cairo_set_source_rgba(cr, QTC_CAIRO_COL(qtcPalette.background[ORIGINAL_SHADE]), 0.75);
                if(!(round&CORNER_TL))
                    cairo_rectangle(cr, x, y, 1, 1);
                if(!(round&CORNER_TR))
                    cairo_rectangle(cr, x+width-1, y, 1, 1);
                if(!(round&CORNER_BR))
                    cairo_rectangle(cr, x+width-1, y+height-1, 1, 1);
                if(!(round&CORNER_BL))
                    cairo_rectangle(cr, x, y+height-1, 1, 1);
                cairo_fill(cr);
            }
        }

        if(region)
            gdk_region_destroy(region);
    }
    else if(DETAIL("menu"))
    {
        if(opts.lighterPopupMenuBgnd)
            drawAreaModColor(cr, area, NULL, &qtcPalette.background[ORIGINAL_SHADE], POPUPMENU_LIGHT_FACTOR, x, y, width, height);
        else
        {
            drawHLine(cr, QTC_CAIRO_COL(qtcPalette.background[0]), 1.0, x+1, y+1, width-2);
            drawVLine(cr, QTC_CAIRO_COL(qtcPalette.background[0]), 1.0, x+1, y+1, height-2);
            drawHLine(cr, QTC_CAIRO_COL(qtcPalette.background[QT_FRAME_DARK_SHADOW]), 1.0, x+1, y+height-2, width-2);
            drawVLine(cr, QTC_CAIRO_COL(qtcPalette.background[QT_FRAME_DARK_SHADOW]), 1.0, x+width-2, y+1, height-2);
        }

#ifdef QTC_GTK2_MENU_STRIPE
        if(opts.menuStripe && !isComboMenu(widget))
            drawBevelGradient(cr, style, area, NULL, x+2, y+2, isMozilla() ? 18 : 22, height-4,
                              &qtcPalette.background[QTC_MENU_STRIPE_SHADE],
                              getWidgetShade(WIDGET_OTHER, TRUE, FALSE, opts.menuStripeAppearance),
                              getWidgetShade(WIDGET_OTHER, FALSE, FALSE, opts.menuStripeAppearance),
                              FALSE, TRUE, FALSE, opts.menuStripeAppearance, WIDGET_OTHER);
#endif
        cairo_new_path(cr);
        cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[QT_STD_BORDER]));
        cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
        cairo_stroke(cr);
    }
    else if(detail &&(!strcmp(detail, "paned") || !strcmp(detail+1, "paned")))
    {
        GtkOrientation orientation = GTK_ORIENTATION_HORIZONTAL;

        if(*detail == 'h')
            orientation = GTK_ORIENTATION_VERTICAL;

        gtkDrawHandle(style, window, state, shadow_type, area, widget, detail, x, y, width, height,
                      orientation);
    }
    else
    {
        int dark=APPEARANCE_FLAT==opts.appearance ? ORIGINAL_SHADE : 4;

/* CPD: TODO When is this used??? */
        drawAreaColor(cr, area, NULL, &style->bg[state], x, y, width-1, height-1);
        cairo_new_path(cr);
        cairo_rectangle(cr, x, y, width-1, height-1);
        cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[QT_STD_BORDER]));
        cairo_stroke(cr);
        drawHLine(cr, QTC_CAIRO_COL(qtcPalette.background[0]), 1.0, x+1, y+1, width-3);
        drawVLine(cr, QTC_CAIRO_COL(qtcPalette.background[0]), 1.0, x+1, y+1, height-3);
        drawHLine(cr, QTC_CAIRO_COL(qtcPalette.background[dark]), 1.0, x+1, y+height-2, width-3);
        drawVLine(cr, QTC_CAIRO_COL(qtcPalette.background[dark]), 1.0, x+1, y+height-2, height-3);
    }

    QTC_CAIRO_END
    }   /* C-Scpoing... */
}

static void gtkDrawBox(GtkStyle *style, GdkWindow *window, GtkStateType state,
                       GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget,
                       const gchar *detail, gint x, gint y, gint width, gint height)
{
    QTC_CAIRO_BEGIN
    drawBox(style, window, state, shadow_type, area, widget, detail, x, y, width, height,
            GTK_STATE_ACTIVE==state || (GTK_IS_BUTTON(widget) && GTK_BUTTON(widget)->depressed));
    QTC_CAIRO_END
}

static void gtkDrawShadow(GtkStyle *style, GdkWindow *window, GtkStateType state,
                          GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget,
                          const gchar *detail, gint x, gint y, gint width, gint height)
{
    QTC_CAIRO_BEGIN

    if(isComboList(widget))
    {
        /* if appears-as-list is set, then make border more KDE like */
        cairo_new_path(cr);
        cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
        cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[QT_STD_BORDER]));
        cairo_stroke(cr);
        cairo_new_path(cr);
        cairo_rectangle(cr, x+1.5, y+1.5, width-3, height-3);
        cairo_set_source_rgb(cr, QTC_CAIRO_COL(style->base[state]));
        cairo_stroke(cr);
    }
#if 0
    else if(isComboFrame(widget))
    {
    }
    else
#endif
    else if(DETAIL("entry") || DETAIL("text"))
    {
        gboolean combo=isComboBoxEntry(widget);

        drawEntryField(cr, style, state, widget, area, x, y, width, height,
                       combo || isSpinButton(widget)
                           ? reverseLayout(widget) || (combo && widget && reverseLayout(widget->parent))
                                ? ROUNDED_RIGHT
                                : ROUNDED_LEFT
                           : ROUNDED_ALL,
                       combo);
    }
    else
    {
        gboolean frame=!detail || 0==strcmp(detail, "frame"),
                 profiledFrame=DETAIL("scrolled_window"),
                 viewport=!profiledFrame && detail && NULL!=strstr(detail, "viewport"),
                 statusBar=isMozilla() || GTK_APP_JAVA==qtSettings.app
                            ? frame : isStatusBarFrame(widget);

    #ifdef QTC_DEBUG
        printf("Draw shadow %d %d %d %d %d %d %s  ", state, shadow_type, x, y, width, height,
               detail ? detail : "NULL");
       debugDisplayWidget(widget, 3);
    #endif

        sanitizeSize(window, &width, &height);

        if(!statusBar && (frame || profiledFrame || viewport) && QTC_ROUNDED)
        {
            if(GTK_SHADOW_NONE!=shadow_type &&
               (!frame || opts.drawStatusBarFrames || (!isMozilla() && GTK_APP_JAVA!=qtSettings.app)))
            {
                if(viewport)
                {
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
                    cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[ORIGINAL_SHADE]));
                    cairo_stroke(cr);
                }
                drawBorder(cr, style, state, area, NULL, x, y, width, height,
                           NULL, ROUNDED_ALL, profiledFrame ? BORDER_SUNKEN : BORDER_FLAT,
                           WIDGET_OTHER, DF_BLEND|(viewport ? 0 : DF_DO_CORNERS));
            }
        }
        else if(!statusBar || opts.drawStatusBarFrames)
        {
            int c1=0,
                c2=0;

            switch(shadow_type)
            {
                case GTK_SHADOW_NONE:
                    return;
                case GTK_SHADOW_IN:
                case GTK_SHADOW_ETCHED_IN:
                    c1 = 0;
                    c2 = QT_STD_BORDER;
                    break;
                case GTK_SHADOW_OUT:
                case GTK_SHADOW_ETCHED_OUT:
                    c1 = QT_STD_BORDER;
                    c2 = 0;
                    break;
            }

            switch(shadow_type)
            {
                case GTK_SHADOW_NONE:
                    break;
                case GTK_SHADOW_IN:
                case GTK_SHADOW_OUT:
                    if(frame || !QTC_ROUNDED)
                    {
                        drawHLine(cr, QTC_CAIRO_COL(qtcPalette.background[c2]), 1.0, x, y, width-1);
                        drawVLine(cr, QTC_CAIRO_COL(qtcPalette.background[c2]), 1.0, x, y, height-1);
                        if(APPEARANCE_FLAT!=opts.appearance)
                        {
                            drawHLine(cr, QTC_CAIRO_COL(qtcPalette.background[c1]), 1.0, x, y+height-1, width-1);
                            drawVLine(cr, QTC_CAIRO_COL(qtcPalette.background[c1]), 1.0, x+width-1, y, height-1);
                        }
                    }
                    break;
                case GTK_SHADOW_ETCHED_IN:
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+1.5, y+1.5, width-2, height-2);
                    cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[c1]));
                    cairo_stroke(cr);
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+0.5, y+0.5, width-2, height-2);
                    cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[c2]));
                    cairo_stroke(cr);
                    break;
                case GTK_SHADOW_ETCHED_OUT:
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+1.5, y+1.5, width-2, height-2);
                    cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[c2]));
                    cairo_stroke(cr);
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+0.5, y+0.5, width-2, height-2);
                    cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[c1]));
                    cairo_stroke(cr);
                    break;
            }
        }
    }
    QTC_CAIRO_END
}

static void drawBoxGap(cairo_t *cr, GtkStyle *style, GdkWindow *window, GtkShadowType shadow_type,
                       GtkStateType state, GtkWidget *widget, GdkRectangle *area, gint x, gint y,
                       gint width, gint height, GtkPositionType gap_side, gint gap_x,
                       gint gap_width, EBorder borderProfile, gboolean isTab)
{
    FN_CHECK

/*
    gtk_style_apply_default_background(style, window, widget && !GTK_WIDGET_NO_WINDOW(widget), state,
                                       area, x, y, width, height);
*/
    sanitizeSize(window, &width, &height);

    if(GTK_SHADOW_NONE!=shadow_type)
    {
        int round=ROUNDED_ALL;

        if(gap_x<=0)
            switch(gap_side)
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

        drawBorder(cr, widget && widget->parent ? widget->parent->style : style, state,
                   area, NULL, x, y, width, height, NULL, round,
                   borderProfile, WIDGET_OTHER, (isTab ? 0 : DF_BLEND)|DF_DO_CORNERS);
    }

    if(gap_width>0)
    {
        cairo_new_path(cr);
        switch(gap_side)
        {
            case GTK_POS_TOP:
                cairo_rectangle(cr, (x+gap_x)-0.5, y, gap_width, 2);
                break;
            case GTK_POS_BOTTOM:
                cairo_rectangle(cr, (x+gap_x)-0.5, y+height-2, gap_width, 2);
                break;
            case GTK_POS_LEFT:
                cairo_rectangle(cr, x, (y+gap_x)-0.5, 2, gap_width);
                break;
            case GTK_POS_RIGHT:
                cairo_rectangle(cr, x+width-2, (y+gap_x)-0.5, 2, gap_width);
                break;
        }

        cairo_set_source_rgb(cr, QTC_CAIRO_COL(style->bg[state]));
        cairo_fill(cr);
    }
}

static void gtkDrawCheck(GtkStyle *style, GdkWindow *window, GtkStateType state,
                         GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget,
                         const gchar *detail, gint x, gint y, gint width, gint height)
{
    if(GTK_STATE_PRELIGHT==state && (GTK_APP_MOZILLA==qtSettings.app || GTK_APP_JAVA==qtSettings.app))
        state=GTK_STATE_NORMAL;
    {

    gboolean mnu=DETAIL("check"),
             list=!mnu && isList(widget),
             on=GTK_SHADOW_IN==shadow_type,
             tri=GTK_SHADOW_ETCHED_IN==shadow_type,
             doEtch=QTC_DO_EFFECT;
    GdkColor new_colors[TOTAL_SHADES+1],
             *btn_colors;
    int      ind_state=(list || ( !mnu && GTK_STATE_INSENSITIVE==state ) ) ? state : GTK_STATE_NORMAL,
             checkSpace=doEtch && /*!list && */!mnu ? QTC_CHECK_SIZE+2 : QTC_CHECK_SIZE;

    QTC_CAIRO_BEGIN

    if(QT_CUSTOM_COLOR_BUTTON(style))
    {
        shadeColors(&(style->bg[state]), new_colors);
        btn_colors=new_colors;
    }
    else
        btn_colors=qtcPalette.button;

    x+=(width-checkSpace)>>1;
    y+=(height-checkSpace)>>1;

    if(doEtch)
        x++, y++;

    if(mnu)
    {
        y+=2;
        if(isMozilla() || GTK_APP_JAVA==qtSettings.app)
            x+=2;
        else
            x-=2;
    }

#ifdef QTC_DEBUG
printf("Draw check %d %d %d %d %d %d %d %s  ", state, shadow_type, x, y, width, height, mnu, detail ? detail : "NULL");
debugDisplayWidget(widget, 3);
#endif

    if(mnu && GTK_APP_OPEN_OFFICE==qtSettings.app)
        x+=2, y-=2;

    if(!mnu || qtSettings.qt4)
    {
        gboolean coloredMouseOver=GTK_STATE_PRELIGHT==state && opts.coloredMouseOver,
                 glow=doEtch && GTK_STATE_PRELIGHT==state && MO_GLOW==opts.coloredMouseOver;
        GdkColor *colors=coloredMouseOver
                    ? qtcPalette.mouseover
                    : btn_colors;
        GdkColor *bgndCol=/*!isList(widget) && */GTK_STATE_INSENSITIVE==state
                            ? &style->bg[GTK_STATE_NORMAL]
                            : !mnu && GTK_STATE_PRELIGHT==state && !coloredMouseOver && !opts.crHighlight
                                ? &btn_colors[QTC_CR_MO_FILL]
                                : &style->base[GTK_STATE_NORMAL];

        if(mnu)
            x--, y--, width=QTC_CHECK_SIZE, height=QTC_CHECK_SIZE;
        /*else if(list)
            y++, width=QTC_CHECK_SIZE, height=QTC_CHECK_SIZE;*/

        if(IS_FLAT(opts.appearance))
            drawAreaColor(cr, area, NULL, bgndCol, x+1, y+1, QTC_CHECK_SIZE-2, QTC_CHECK_SIZE-2);
        else
            drawBevelGradient(cr, style, area, NULL, x+1, y+1, QTC_CHECK_SIZE-2, QTC_CHECK_SIZE-2,
                              bgndCol,
                              getWidgetShade(WIDGET_TROUGH, TRUE, FALSE, opts.appearance),
                              getWidgetShade(WIDGET_TROUGH, FALSE, FALSE, opts.appearance),
                              TRUE, FALSE, FALSE, APPEARANCE_GRADIENT, WIDGET_TROUGH);

        if(coloredMouseOver && !glow)
        {
            cairo_new_path(cr);
            cairo_set_source_rgb(cr, QTC_CAIRO_COL(colors[QTC_CR_MO_FILL]));
            cairo_rectangle(cr, x+1.5, y+1.5, QTC_CHECK_SIZE-3, QTC_CHECK_SIZE-3);
            //cairo_rectangle(cr, x+2.5, y+2.5, QTC_CHECK_SIZE-5, QTC_CHECK_SIZE-5);
            cairo_stroke(cr);
        }
        else
        {
            GdkColor mid;

            generateMidColor(GTK_STATE_INSENSITIVE==state ? &style->bg[GTK_STATE_NORMAL] : &style->base[GTK_STATE_NORMAL], &(colors[3]), &mid, 1.0);

            cairo_new_path(cr);
            cairo_set_source_rgb(cr, QTC_CAIRO_COL(mid));
            cairo_move_to(cr, x+1.5, y+QTC_CHECK_SIZE-1.5);
            cairo_line_to(cr, x+1.5, y+1.5);
            cairo_line_to(cr, x+QTC_CHECK_SIZE-1.5, y+1.5);
            cairo_stroke(cr);
        }

        if(doEtch && (!list || glow) && !mnu)
        {
            if(glow)
                drawGlow(cr, area, NULL, x-1, y-1, QTC_CHECK_SIZE+2, QTC_CHECK_SIZE+2, WIDGET_STD_BUTTON);
            else
                drawEtch(cr, area, NULL, x-1, y-1, QTC_CHECK_SIZE+2, QTC_CHECK_SIZE+2, FALSE);
//             cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, QTC_ETCH_TOP_ALPHA);
//             cairo_move_to(cr, x-0.5, y+QTC_CHECK_SIZE);
//             cairo_line_to(cr, x-0.5, y);
//             cairo_move_to(cr, x, y-0.5);
//             cairo_line_to(cr, x+QTC_CHECK_SIZE, y-0.5);
//             cairo_stroke(cr);
//             cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, QTC_ETCH_BOTTOM_ALPHA);
//             cairo_move_to(cr, x, y+QTC_CHECK_SIZE+0.5);
//             cairo_line_to(cr, x+QTC_CHECK_SIZE, y+QTC_CHECK_SIZE+0.5);
//             cairo_move_to(cr, x+QTC_CHECK_SIZE+0.5, y+QTC_CHECK_SIZE);
//             cairo_line_to(cr, x+QTC_CHECK_SIZE+0.5, y);
//             cairo_stroke(cr);
        }

        drawBorder(cr, style, state, area, NULL, x, y, QTC_CHECK_SIZE, QTC_CHECK_SIZE,
                   colors, ROUNDED_ALL, BORDER_FLAT, WIDGET_CHECKBOX,
                   (list || mnu ? 0 : DF_DO_CORNERS));
    }

    if(on)
    {
        GdkPixbuf *pix=getPixbuf(GTK_STATE_INSENSITIVE==state /* || (list && (GTK_STATE_SELECTED==state || GTK_STATE_ACTIVE==state)) */
                                    ? &style->text[ind_state]
                                    : GTK_STATE_PRELIGHT==state && !qtSettings.qt4 && mnu
                                        ? &style->text[GTK_STATE_PRELIGHT]
                                        : qtcPalette.check_radio, PIX_CHECK, 1.0);
        int       pw=gdk_pixbuf_get_width(pix),
                  ph=gdk_pixbuf_get_height(pix),
                  dx=(x+(QTC_CHECK_SIZE/2))-(pw/2),
                  dy=(y+(QTC_CHECK_SIZE/2))-(ph/2);

        gdk_cairo_set_source_pixbuf(cr, pix, dx, dy);
        cairo_paint(cr);
    }
    else if (tri)
    {
        int tx=x+(QTC_CHECK_SIZE/2),
            ty=y+(QTC_CHECK_SIZE/2);

        drawHLine(cr, QTC_CAIRO_COL(style->text[ind_state]), 1.0, tx-3, ty, 6);
        drawHLine(cr, QTC_CAIRO_COL(style->text[ind_state]), 1.0, tx-3, ty+1, 6);
    }

    QTC_CAIRO_END
    }
}

static void gtkDrawOption(GtkStyle *style, GdkWindow *window, GtkStateType state,
                          GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget,
                          const gchar *detail, gint x, gint y, gint width, gint height)
{
    if(GTK_STATE_PRELIGHT==state && (GTK_APP_MOZILLA==qtSettings.app || GTK_APP_JAVA==qtSettings.app))
        state=GTK_STATE_NORMAL;
    {

    gboolean mnu=DETAIL("option"),
             list=!mnu && isList(widget);

    QTC_CAIRO_BEGIN

    if(!qtSettings.qt4 && mnu)
        gtkDrawCheck(style, window, state, shadow_type, area, widget, "check", x, y, width, height);
    else
    {
        gboolean on=GTK_SHADOW_IN==shadow_type,
                 tri=GTK_SHADOW_ETCHED_IN==shadow_type,
                 set=on||tri,
                 doEtch=QTC_DO_EFFECT,
                 glow=doEtch && GTK_STATE_PRELIGHT==state && MO_GLOW==opts.coloredMouseOver;
        int      ind_state=GTK_STATE_INSENSITIVE==state ? state : GTK_STATE_NORMAL;

        x+=((width-QTC_RADIO_SIZE)>>1)+1;  /* +1 solves clipping on prnting dialog */
        y+=((height-QTC_RADIO_SIZE)>>1)+1;

        if(doEtch)
            x++, y++;

        /* For some reason, radios dont look aligned properly - most noticeable with menuStripe set */
        if(!isMozilla())
            x-=3;

        {
            GdkColor  new_colors[TOTAL_SHADES+1],
                      *btn_colors;
            gboolean  coloredMouseOver=GTK_STATE_PRELIGHT==state && opts.coloredMouseOver;
            int       bgnd=0;

            GdkPoint  clip[8]= {{x,    y+8},     {x,    y+4},     {x+4, y},      {x+8, y},
                               { x+12, y+4},     {x+12, y+8},     {x+8, y+12},   {x+4, y+12} };

            GdkRegion *region=gdk_region_polygon(clip, 8, GDK_EVEN_ODD_RULE);

            if(QT_CUSTOM_COLOR_BUTTON(style))
            {
                shadeColors(&(style->bg[state]), new_colors);
                btn_colors=new_colors;
            }
            else
                btn_colors=qtcPalette.button;

            { /* C-scoping */
            GdkColor *colors=coloredMouseOver
                        ? qtcPalette.mouseover
                        : btn_colors;
            GdkColor *bgndCol=GTK_STATE_INSENSITIVE==state
                                ? &style->bg[GTK_STATE_NORMAL]
                                : !mnu && GTK_STATE_PRELIGHT==state && !coloredMouseOver && !opts.crHighlight
                                    ? &colors[QTC_CR_MO_FILL]
                                    : &style->base[GTK_STATE_NORMAL];

            bgnd=getFill(state, set/*, TRUE*/);

            if(IS_FLAT(opts.appearance))
                drawAreaColor(cr, NULL, region, bgndCol, x+1, y+1, QTC_RADIO_SIZE-2, QTC_RADIO_SIZE-2);
            else
                drawBevelGradient(cr, style, NULL, region, x+1, y+1, QTC_RADIO_SIZE-2, QTC_RADIO_SIZE-2,
                                  bgndCol,
                                  getWidgetShade(WIDGET_TROUGH, TRUE, FALSE, opts.appearance),
                                  getWidgetShade(WIDGET_TROUGH, FALSE, FALSE, opts.appearance),
                                  TRUE, FALSE, FALSE, APPEARANCE_GRADIENT, WIDGET_TROUGH);

            gdk_region_destroy(region);

            if(doEtch && !mnu && coloredMouseOver && !glow)
            {
                double radius=(QTC_RADIO_SIZE-2)/2.0;

                cairo_set_source_rgb(cr, QTC_CAIRO_COL(colors[QTC_CR_MO_FILL]));
                cairo_arc (cr, x+radius + 1, y+radius + 1, radius, 0, 2*M_PI);
                cairo_stroke(cr);
                radius--;
                cairo_arc (cr, x+radius + 2, y+radius + 2, radius, 0, 2*M_PI);
                cairo_stroke(cr);
            }

            if(doEtch && !mnu && (!list || glow))
            {
                double radius=(QTC_RADIO_SIZE+1)/2.0;

                if(glow)
                    cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.mouseover[QTC_GLOW_MO]));
                else
                    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, QTC_ETCH_RADIO_TOP_ALPHA);
                cairo_arc (cr, x+radius - 0.5, y+radius - 0.5, radius, 0.75*M_PI, 1.75*M_PI);
                cairo_stroke(cr);
                if(!glow)
                    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, QTC_ETCH_RADIO_BOTTOM_ALPHA);
                cairo_arc (cr, x+radius - 0.5, y+radius - 0.5, radius, 1.75*M_PI, 0.75*M_PI);
                cairo_stroke(cr);
            }

            {
                GdkPixbuf *border=getPixbuf(&colors[coloredMouseOver ? 4 : QT_BORDER(GTK_STATE_INSENSITIVE!=state)], PIX_RADIO_BORDER,
                                                 0.8);

                gdk_cairo_set_source_pixbuf(cr, border, x, y);
                cairo_paint(cr);

                /*if(!on)*/
                if(!coloredMouseOver)
                {
                    GdkPixbuf *light=getPixbuf(&btn_colors[coloredMouseOver ? 3 : 4], PIX_RADIO_LIGHT, 1.0);

                    gdk_cairo_set_source_pixbuf(cr, light, x, y);
                    cairo_paint(cr);
                }
            }

            } /* C-scoping */
        }

        if(on)
        {
            GdkPixbuf *pix=getPixbuf(GTK_STATE_INSENSITIVE==state /* || (list && (GTK_STATE_SELECTED==state || GTK_STATE_ACTIVE==state))*/
                                        ? &style->text[ind_state]
                                        : qtcPalette.check_radio, PIX_RADIO_ON, 1.0);

            gdk_cairo_set_source_pixbuf(cr, pix, x, y);
            cairo_paint(cr);
        }
        else if(tri)
        {
            int tx=x+(QTC_RADIO_SIZE/2),
                ty=y+(QTC_RADIO_SIZE/2);

            drawHLine(cr, QTC_CAIRO_COL(style->text[ind_state]), 1.0, tx-3, ty, 6);
            drawHLine(cr, QTC_CAIRO_COL(style->text[ind_state]), 1.0, tx-3, ty+1, 6);
        }
    }
    QTC_CAIRO_END
    }
}

static void gtkDrawLayout(GtkStyle *style, GdkWindow *window, GtkStateType state, gboolean use_text,
                          GdkRectangle *area, GtkWidget *widget, const gchar *detail, gint x, gint y,
                          PangoLayout *layout)
{
    if(GTK_IS_PROGRESS(widget) || GTK_IS_PROGRESS_BAR(widget) || DETAIL("progressbar"))
        parent_class->draw_layout(style, window, state, use_text, area, widget, detail, x, y, layout);
    else
    {
        QtCurveStyle *qtcurveStyle = (QtCurveStyle *)style;
        gboolean     isMenuItem=QTC_IS_MENU_ITEM(widget);
        GtkMenuBar   *mb=isMenuItem ? isMenubar(widget, 0) : NULL;
        gboolean     activeMb=mb ? GTK_MENU_SHELL(mb)->active : FALSE,
                     selectedText=isMenuItem &&
                                  (opts.colorMenubarMouseOver
                                      ? GTK_STATE_PRELIGHT==state
                                      : ((!mb || activeMb) && GTK_STATE_PRELIGHT==state)),
                     but=FALSE,
                     def_but=FALSE,
                     swap_gc=FALSE;
        GdkRectangle area2;

        if(!opts.colorMenubarMouseOver && mb && !activeMb && GTK_STATE_PRELIGHT==state)
            state=GTK_STATE_NORMAL;

#if GTK_CHECK_VERSION(2, 10, 0) && !GTK_CHECK_VERSION(2, 10, 11)
        GtkNotebook *nb=mb || isMenuItem || !GTK_IS_LABEL(widget) ||
                       !widget->parent || !GTK_IS_NOTEBOOK(widget->parent) ? NULL : GTK_NOTEBOOK(widget->parent);
#endif
        GdkGC    *prevGcs[4];
        gboolean activeWindow=TRUE;
        int      i=0;

#ifdef QTC_DEBUG
        printf("Draw layout %s %d %d %d %s  ", pango_layout_get_text(layout), state, use_text,
                QTC_IS_MENU_ITEM(widget), detail ? detail : "NULL");
        debugDisplayWidget(widget, 3);
#endif

#ifndef QTC_READ_INACTIVE_PAL /* If we reead the inactive palette, then there is no need for the following... */
        /* The following fixes the text in list views... if not used, when an item is selected it
           gets the selected text color - but when the window changes focus it gets the normal
           text color! */
         if(DETAIL("cellrenderertext") && GTK_STATE_ACTIVE==state)
             state=GTK_STATE_SELECTED;
#endif

        if(opts.shadeMenubarOnlyWhenActive)
        {
            GtkWindow *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(widget));

            if(topLevel && GTK_IS_WINDOW(topLevel))
                activeWindow=gtk_window_has_toplevel_focus(GTK_WINDOW(topLevel));
        }

        if(!isMenuItem && GTK_STATE_PRELIGHT==state)
            state=GTK_STATE_NORMAL;

        but=isOnButton(widget, 0, &def_but);

        if(but && GTK_STATE_INSENSITIVE!=state)
        {
            use_text=TRUE;
            swap_gc=TRUE;
            for(i=0; i<4; ++i)
            {
                prevGcs[i]=style->text_gc[i];
                style->text_gc[i]=qtcurveStyle->button_text_gc;
            }
        }
        else if(isMenuItem)
        {
            if(mb && activeWindow)
            {
                if(opts.customMenuTextColor && qtcurveStyle->menutext_gc[0])
                {
                    for(i=0; i<4; ++i)
                        prevGcs[i]=style->text_gc[i];
                    swap_gc=TRUE;
                    style->text_gc[GTK_STATE_NORMAL]=qtcurveStyle->menutext_gc[0];
                    style->text_gc[GTK_STATE_ACTIVE]=qtcurveStyle->menutext_gc[1];
                    style->text_gc[GTK_STATE_PRELIGHT]=qtcurveStyle->menutext_gc[0];
                    style->text_gc[GTK_STATE_SELECTED]=qtcurveStyle->menutext_gc[1];
                    style->text_gc[GTK_STATE_INSENSITIVE]=qtcurveStyle->menutext_gc[0];
                    use_text=TRUE;
                }
                else if (SHADE_BLEND_SELECTED==opts.shadeMenubars ||
                         (SHADE_CUSTOM==opts.shadeMenubars &&
                          TOO_DARK(qtcPalette.menubar[ORIGINAL_SHADE])))
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
                    active=GTK_STATE_NORMAL==tabLabel->state;
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

        if(GTK_IS_LABEL(widget) && GTK_IS_FRAME(widget->parent) && !isOnStatusBar(widget, 0))
        {

            int diff=widget->allocation.x-widget->parent->allocation.x;
            if(opts.framelessGroupBoxes)
                x-=QTC_MAX(0, QTC_MIN(diff, 6));
            else
                x+=5;
            if(area)
            {
                area2=*area;
                if(opts.framelessGroupBoxes)
                    area2.x-=QTC_MAX(0, QTC_MIN(diff, 6));
                else
                    area2.x+=5;
                area=&area2;
            }
        }

        parent_class->draw_layout(style, window, selectedText ? GTK_STATE_SELECTED : state,
                                  use_text || selectedText, area, widget, detail, x, y, layout);
        if(opts.embolden && def_but)
            parent_class->draw_layout(style, window, selectedText ? GTK_STATE_SELECTED : state,
                                      use_text || selectedText,
                                      area, widget, detail, x+1, y, layout);

        if(swap_gc)
            for(i=0; i<4; ++i)
                style->text_gc[i]=prevGcs[i];
    }
}

static void gtkDrawTab(GtkStyle *style, GdkWindow *window, GtkStateType state,
                       GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget,
                       const gchar *detail, gint x, gint y, gint width, gint height)
{
    //if(QTC_DO_EFFECT)
    //    x--;

    if(isActiveCombo(widget))
        x++, y++;

    x=reverseLayout(widget) || (widget && reverseLayout(widget->parent))
                ? x+1
                : x+(width>>1);

    //QTC_CAIRO_BEGIN
    drawArrow(window, style->text_gc[QTC_ARROW_STATE(state)], NULL, GTK_ARROW_UP, x,
              y+(height>>1)-(LARGE_ARR_HEIGHT-1), FALSE, TRUE);
    drawArrow(window, style->text_gc[QTC_ARROW_STATE(state)], NULL, GTK_ARROW_DOWN, x,
              y+(height>>1)+(LARGE_ARR_HEIGHT-1), FALSE, TRUE);
    //QTC_CAIRO_END
}

static void gtkDrawBoxGap(GtkStyle *style, GdkWindow *window, GtkStateType state,
                          GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget,
                          const gchar *detail, gint x, gint y, gint width,
                          gint height, GtkPositionType gap_side, gint gap_x, gint gap_width)
{
    GdkColor *col1 = &qtcPalette.background[0],
             *col2 = &qtcPalette.background[APPEARANCE_FLAT==opts.appearance ? ORIGINAL_SHADE : 4],
             *outer = &qtcPalette.background[QT_STD_BORDER];
    gboolean rev = reverseLayout(widget);
    int      rightPos=(width -(gap_x + gap_width));

    QTC_CAIRO_BEGIN
    FN_CHECK

    drawBoxGap(cr, style, window, GTK_SHADOW_OUT, state, widget, area, x, y,
               width, height, gap_side, gap_x, gap_width, BORDER_RAISED, TRUE);

    sanitizeSize(window, &width, &height);

    switch(gap_side)
    {
        case GTK_POS_TOP:
            if(gap_x > 0)
            {
                drawHLine(cr, QTC_CAIRO_COL(*col1), 1.0, x+gap_x-1, y+1, 3);
                drawHLine(cr, QTC_CAIRO_COL(*col1), 1.0, x+gap_x-1, y, 3);
                drawHLine(cr, QTC_CAIRO_COL(*outer), 1.0, x+gap_x-1, y, 2);
            }
            else
                drawVLine(cr, QTC_CAIRO_COL(*col1), 1.0, x+1, y, 2);

            if(rightPos >= 0)
            {
                drawHLine(cr, QTC_CAIRO_COL(*col1), 1.0, x+gap_x+gap_width-2, y+1, 3);
                drawVLine(cr, QTC_CAIRO_COL(*col2), 1.0, x+gap_x+gap_width-2, y, rightPos ? 1 : 0);
                drawHLine(cr, QTC_CAIRO_COL(*outer), 1.0, x+gap_x+gap_width-1, y, 2);
            }
            drawVLine(cr, QTC_CAIRO_COL(*outer), 1.0, rev ? x+width-1 : x, y, 3);
            break;
        case GTK_POS_BOTTOM:
            if(gap_x > 0)
            {
                drawHLine(cr, QTC_CAIRO_COL(*col1), 1.0, x+gap_x-1, y+height-1, 2);
                drawHLine(cr, QTC_CAIRO_COL(*col2), 1.0, x+gap_x-1, y+height-2, 2);
                drawHLine(cr, QTC_CAIRO_COL(*outer), 1.0, x+gap_x-1, y+height-1, 2);
            }
            else
                drawVLine(cr, QTC_CAIRO_COL(*col1), 1.0, x+1, y+height-1, 2);

            if(rightPos >= 0)
            {
                drawHLine(cr, QTC_CAIRO_COL(*col2), 1.0, x+gap_x+gap_width-2, y+height-2, 3);
                drawVLine(cr, QTC_CAIRO_COL(*col2), 1.0, x+gap_x+gap_width-2, y+height-1, rightPos ? 1 : 0);
                drawHLine(cr, QTC_CAIRO_COL(*outer), 1.0, x+gap_x+gap_width-1, y+height-1, 2);
            }
            drawVLine(cr, QTC_CAIRO_COL(*outer), 1.0, rev ? x+width-1 : x, y+height-3, 3);
            break;
        case GTK_POS_LEFT:
            if(gap_x>0)
            {
                drawVLine(cr, QTC_CAIRO_COL(*col1), 1.0, x+1, y+gap_x-1, 3);
                drawVLine(cr, QTC_CAIRO_COL(*col1), 1.0, x, y+gap_x-1, 3);
                drawVLine(cr, QTC_CAIRO_COL(*outer), 1.0, x, y+gap_x-1, 2);
            }
            else
                drawHLine(cr, QTC_CAIRO_COL(*col1), 1.0, x, y+1, 2);

            if((height-(gap_x + gap_width)) > 0)
            {
                drawVLine(cr, QTC_CAIRO_COL(*col1), 1.0, x+1, y+gap_x+gap_width-2, 3);
                drawVLine(cr, QTC_CAIRO_COL(*col2), 1.0, x, y+gap_x+gap_width-2, 1);
                drawVLine(cr, QTC_CAIRO_COL(*outer), 1.0, x, y+gap_x+gap_width-1, 2);
            }
            drawHLine(cr, QTC_CAIRO_COL(*outer), 1.0, x, y, 3);
            break;
        case GTK_POS_RIGHT:
            if(gap_x>0)
            {
                drawVLine(cr, QTC_CAIRO_COL(*col2), 1.0, x+width-2, y+gap_x-1, 2);
                drawVLine(cr, QTC_CAIRO_COL(*outer), 1.0, x+width-1, y+gap_x-1, 2);
            }
            else
                drawHLine(cr, QTC_CAIRO_COL(*col1), 1.0, x+width-2, y+1, 3);

            if((height-(gap_x + gap_width)) > 0)
            {
                drawHLine(cr, QTC_CAIRO_COL(*col2), 1.0, x+width-2, y+gap_x+gap_width-2, 3);
                drawVLine(cr, QTC_CAIRO_COL(*col2), 1.0, x+width-2, y+gap_x+gap_width-1, 2);
                drawVLine(cr, QTC_CAIRO_COL(*outer), 1.0, x+width-1, y+gap_x+gap_width-1, 2);
            }
            drawHLine(cr, QTC_CAIRO_COL(*outer), 1.0, x+width-3, y, 3);

            break;
    }

    QTC_CAIRO_END
}

static void fillTab(cairo_t *cr, GtkStyle *style, GdkWindow *window, GdkRectangle *area, GtkStateType state,
                    GdkColor *col, int x, int y, int width, int height, gboolean horiz,
                    gboolean increase, EWidget tab, gboolean grad)
{
    if(GTK_STATE_NORMAL==state && APPEARANCE_INVERTED==opts.appearance)
        drawAreaColor(cr, area, NULL, &style->bg[GTK_STATE_NORMAL], x, y, width, height);
    else
    {
        gboolean    selected=GTK_STATE_NORMAL==state;
        EAppearance app=selected ? QTC_SEL_TAB_APP : QTC_NORM_TAB_APP;

        if(grad && !IS_FLAT(app))
        {
            double s1=/*WIDGET_TAB_TOP==tab || (selected && opts.colorSelTab)
                          ? */SHADE_TAB_SEL_LIGHT
                          /*: SHADE_BOTTOM_TAB_SEL_DARK*/,
                   s2=/*WIDGET_TAB_TOP==tab || (selected && opts.colorSelTab)
                          ? */SHADE_TAB_SEL_DARK
                            /*: SHADE_BOTTOM_TAB_SEL_LIGHT*/;

            drawBevelGradient(cr, style, area, NULL, x, y, width, height,
                              col, s1, s2, horiz, increase, GTK_STATE_NORMAL==state, app, tab);
        }
        else
            drawAreaColor(cr, area, NULL, col, x, y, width, height);
    }
}

static gboolean isMozillaTab(GtkWidget *widget)
{
    return isMozillaWidget(widget) && GTK_IS_NOTEBOOK(widget);
}

static void gtkDrawExtension(GtkStyle *style, GdkWindow *window, GtkStateType state,
                             GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget,
                             const gchar *detail, gint x, gint y, gint width,
                             gint height, GtkPositionType gap_side)
{
#ifdef QTC_DEBUG
printf("Draw extension %d %d %d %d %d %d %d %s  ", state, shadow_type, gap_side, x, y, width, height, detail ? detail :
"NULL");
debugDisplayWidget(widget, 3);
#endif

    if (DETAIL ("tab"))
    {
        GtkNotebook *notebook=GTK_IS_NOTEBOOK(widget) ? GTK_NOTEBOOK(widget) : NULL;
        gboolean    highlightingEnabled=notebook && (opts.highlightFactor>1.0 || opts.coloredMouseOver);
        QtCTab      *highlightTab=highlightingEnabled ? lookupTabHash(widget, FALSE) : NULL;
        gboolean    highlight=FALSE;
        int         dark=APPEARANCE_FLAT==opts.appearance ? ORIGINAL_SHADE : QT_FRAME_DARK_SHADOW,
                    moOffset=ROUNDED_NONE==opts.round ? 1 : opts.round;
        gboolean    firstTab=notebook ? FALSE : TRUE,
                    lastTab=notebook ? FALSE : TRUE,
                    vertical=GTK_POS_LEFT==gap_side || GTK_POS_RIGHT==gap_side,
                    active=GTK_STATE_NORMAL==state, /* Normal -> active tab? */
                    rev=(GTK_POS_TOP==gap_side || GTK_POS_BOTTOM==gap_side) &&
                        reverseLayout(widget->parent),
                    mozTab=isMozillaTab(widget);
        GdkColor    *col=active
                            ? &(style->bg[GTK_STATE_NORMAL]) : &(qtcPalette.background[2]),
                    *selCol1=&qtcPalette.menuitem[0],
                    *selCol2=&qtcPalette.menuitem[IS_FLAT(opts.appearance) ? 0 : 3];
        GdkRectangle clipArea;

        FN_CHECK

        /* Hacky fix for tabs in Thunderbird */

        if(mozTab && area && area->x<(x-10))
            return;

        QTC_CAIRO_BEGIN

        /* f'in mozilla apps dont really use Gtk widgets - they just paint to a pixmap. So, no way of knowing
        the position of a tab! The 'best' look seems to be to round both corners. Not nice, but... */
        if(mozTab || GTK_APP_JAVA==qtSettings.app)
            firstTab=lastTab=TRUE;
        else if(notebook)
        {
            /* Borrowed from Qt engine... */
            int       num_children=g_list_length(notebook->children),
                      i, sdiff = 10000, pos = -1,
                      first_shown=-1,
                      last_shown=-1;
            GtkWidget *p=widget;

            for (i = 0; i < num_children; i++ )
            {
                GtkWidget *page=gtk_notebook_get_nth_page(notebook, i),
                        *tab_label=gtk_notebook_get_tab_label(notebook, page);

                int diff=tab_label ? (vertical ? tab_label->allocation.y-y : tab_label->allocation.x-x)
                                : -1;

                if ((diff > 0) && (diff < sdiff))
                {
                    sdiff = diff;
                    pos=i;
                    p=tab_label;
                }

                if(GTK_WIDGET_VISIBLE(page))
                {
                    if(i>last_shown)
                        last_shown=i;
                    if(-1==first_shown)
                        first_shown=i;
                }
            }

            if(0==pos || first_shown==pos)
                firstTab=TRUE;
            else if(pos==(num_children-1) || (pos==last_shown))
                lastTab=TRUE;

            if(highlightTab && pos==highlightTab->id && !active)
            {
                highlight=TRUE;
                col=&(qtcPalette.background[SHADE_2_HIGHLIGHT]);
            }
        }

        if(rev)
        {
            gboolean oldLast=lastTab;

            lastTab=firstTab;
            firstTab=oldLast;
        }

        if(!mozTab && GTK_APP_JAVA!=qtSettings.app && !highlightTab && highlightingEnabled)
        {
            lookupTabHash(widget, TRUE); /* Create hash entry... */
            gtk_widget_add_events(widget, GDK_LEAVE_NOTIFY_MASK|GDK_POINTER_MOTION_MASK);
            g_signal_connect(G_OBJECT(widget), "unrealize", G_CALLBACK(tabDeleteEvent), widget);
            g_signal_connect(G_OBJECT(widget), "event", G_CALLBACK(tabEvent), widget);
        }

/*
        gtk_style_apply_default_background(style, window, widget && !GTK_WIDGET_NO_WINDOW(widget),
                                           GTK_STATE_NORMAL, area, x, y, width, height);
*/
        sanitizeSize(window, &width, &height);

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

        switch(gap_side)
        {
            case GTK_POS_TOP:  /* => tabs are on bottom !!! */
    #if GTK_CHECK_VERSION(2, 10, 0) && !GTK_CHECK_VERSION(2, 10, 11)
                if(!active)
                    height-=2;
    #endif
                fillTab(cr, style, window, area, state, col, x, y, width, height-1, TRUE,
                        FALSE, WIDGET_TAB_BOT, NULL!=notebook);

                drawBorder(cr, style, state, area, NULL, x, y-4, width, height+4,
                           qtcPalette.background,
                           active || (firstTab && lastTab)
                                    ? ROUNDED_BOTTOM
                                    : firstTab
                                        ? ROUNDED_BOTTOMLEFT
                                        : lastTab
                                            ? ROUNDED_BOTTOMRIGHT
                                            : ROUNDED_NONE,
                           active && !opts.colorSelTab ? BORDER_RAISED : BORDER_FLAT, WIDGET_OTHER, 0);

                if(notebook && opts.highlightTab && active)
                {
                    GdkColor mid;

                    generateMidColor(col, &(qtcPalette.menuitem[0]), &mid, 1.0);
                    drawHLine(cr, QTC_CAIRO_COL(mid), 1.0, x+1, y+height-3, width-2);
                    drawHLine(cr, QTC_CAIRO_COL(*selCol1), 1.0, x+1, y+height-2, width-2);

                    clipArea.y=y+height-3;
                    clipArea.height=3;
                    realDrawBorder(cr, style, state, &clipArea, NULL, x, y, width, height,
                                   qtcPalette.menuitem, ROUNDED_BOTTOM,
                                   BORDER_FLAT, WIDGET_OTHER, 0, 3);
                }

                if(notebook && opts.coloredMouseOver && highlight)
                {
                    int sx=x+(firstTab ? moOffset : 1),
                        sw=width-((lastTab ? moOffset : 1)+1);

                    drawHLine(cr, QTC_CAIRO_COL(qtcPalette.mouseover[ORIGINAL_SHADE]), 1.0, sx, y+height-2, sw);
                    drawHLine(cr, QTC_CAIRO_COL(qtcPalette.mouseover[QT_STD_BORDER]), 1.0, sx, y+height-1, sw);
                }

                break;
            case GTK_POS_BOTTOM: /* => tabs are on top !!! */
    #if GTK_CHECK_VERSION(2, 10, 0) && !GTK_CHECK_VERSION(2, 10, 11)
                if(!active)
                {
                    y+=2;
                    height-=2;
                }
    #endif
                fillTab(cr, style, window, area, state, col, x, y+1, width, height-1, TRUE,
                        TRUE, WIDGET_TAB_TOP, NULL!=notebook);

                drawBorder(cr, style, state, area, NULL, x, y, width, height+4,
                           qtcPalette.background,
                           active || (firstTab && lastTab)
                                    ? ROUNDED_TOP
                                    : firstTab
                                        ? ROUNDED_TOPLEFT
                                        : lastTab
                                            ? ROUNDED_TOPRIGHT
                                            : ROUNDED_NONE,
                           active && !opts.colorSelTab ? BORDER_RAISED : BORDER_FLAT, WIDGET_OTHER, 0);

                if(notebook && opts.highlightTab && active)
                {
                    GdkColor mid;

                    generateMidColor(col, &(qtcPalette.menuitem[0]), &mid, 1.0);
                    drawHLine(cr, QTC_CAIRO_COL(mid), 1.0, x+1, y+2, width-2);
                    drawHLine(cr, QTC_CAIRO_COL(*selCol1), 1.0, x+1, y+1, width-2);

                    clipArea.y=y;
                    clipArea.height=3;
                    realDrawBorder(cr, style, state, &clipArea, NULL, x, y, width, height,
                                   qtcPalette.menuitem, ROUNDED_TOP,
                                   BORDER_FLAT, WIDGET_OTHER, 0, 3);
                }

                if(notebook && opts.coloredMouseOver && highlight)
                {
                    int sx=x+(firstTab ? moOffset : 1),
                        sw=width-((lastTab ? moOffset : 1)+1);

                    drawHLine(cr, QTC_CAIRO_COL(qtcPalette.mouseover[ORIGINAL_SHADE]), 1.0, sx, y+1, sw);
                    drawHLine(cr, QTC_CAIRO_COL(qtcPalette.mouseover[QT_STD_BORDER]), 1.0, sx, y, sw);
                }
                break;
            case GTK_POS_LEFT: /* => tabs are on right !!! */
    #if GTK_CHECK_VERSION(2, 10, 0) && !GTK_CHECK_VERSION(2, 10, 11)
                if(!active)
                    width-=2;
    #endif
                fillTab(cr, style, window, area, state, col, x, y, width-1, height, FALSE,
                        FALSE, WIDGET_TAB_BOT, NULL!=notebook);

                drawBorder(cr, style, state, area, NULL, x-4, y, width+4, height,
                           qtcPalette.background,
                           active || (firstTab && lastTab)
                                    ? ROUNDED_RIGHT
                                    : firstTab
                                        ? ROUNDED_TOPRIGHT
                                        : lastTab
                                            ? ROUNDED_BOTTOMRIGHT
                                            : ROUNDED_NONE,
                           active && !opts.colorSelTab ? BORDER_RAISED : BORDER_FLAT, WIDGET_OTHER, 0);

                if(notebook && opts.highlightTab && active)
                {
                    GdkColor mid;

                    generateMidColor(col, &(qtcPalette.menuitem[0]), &mid, 1.0);
                    drawVLine(cr, QTC_CAIRO_COL(mid), 1.0, x+width-3, y+1, height-2);
                    drawVLine(cr, QTC_CAIRO_COL(*selCol1), 1.0, x+width-2, y+1, height-2);

                    clipArea.x=x+width-3;
                    clipArea.width=3;
                    realDrawBorder(cr, style, state, &clipArea, NULL, x, y, width, height,
                                   qtcPalette.menuitem, ROUNDED_RIGHT,
                                   BORDER_FLAT, WIDGET_OTHER, 0, 3);
                }

                if(notebook && opts.coloredMouseOver && highlight)
                {
                    int sy=y+(firstTab ? moOffset : 1),
                        sh=height-((lastTab ? moOffset : 1)+1);

                    drawVLine(cr, QTC_CAIRO_COL(qtcPalette.mouseover[ORIGINAL_SHADE]), 1.0, x+width-2, sy, sh);
                    drawVLine(cr, QTC_CAIRO_COL(qtcPalette.mouseover[QT_STD_BORDER]), 1.0, x+width-1, sy, sh);
                }
                break;
            case GTK_POS_RIGHT: /* => tabs are on left !!! */
    #if GTK_CHECK_VERSION(2, 10, 0) && !GTK_CHECK_VERSION(2, 10, 11)
                if(!active)
                {
                    x+=2;
                    width-=2;
                }
    #endif
                fillTab(cr, style, window, area, state, col, x+1, y, width-1, height,
                        FALSE, TRUE, WIDGET_TAB_TOP, NULL!=notebook);

                drawBorder(cr, style, state, area, NULL, x, y, width+4, height,
                           qtcPalette.background,
                           active || (firstTab && lastTab)
                                    ? ROUNDED_LEFT
                                    : firstTab
                                        ? ROUNDED_TOPLEFT
                                        : lastTab
                                            ? ROUNDED_BOTTOMLEFT
                                            : ROUNDED_NONE,
                           active && !opts.colorSelTab ? BORDER_RAISED : BORDER_FLAT, WIDGET_OTHER, 0);

                if(notebook && opts.highlightTab && active)
                {
                    GdkColor mid;

                    generateMidColor(col, &(qtcPalette.menuitem[0]), &mid, 1.0);
                    drawVLine(cr, QTC_CAIRO_COL(mid), 1.0, x+2, y+1, height-2);
                    drawVLine(cr, QTC_CAIRO_COL(*selCol1), 1.0, x+1, y+1, height-2);

                    clipArea.x=x;
                    clipArea.width=3;
                    realDrawBorder(cr, style, state, &clipArea, NULL, x, y, width, height,
                                   qtcPalette.menuitem, ROUNDED_LEFT,
                                   BORDER_FLAT, WIDGET_OTHER, 0, 3);
                }

                if(notebook && opts.coloredMouseOver && highlight)
                {
                    int sy=y+(firstTab ? moOffset : 1),
                        sh=height-((lastTab ? moOffset : 1)+1);

                    drawVLine(cr, QTC_CAIRO_COL(qtcPalette.mouseover[ORIGINAL_SHADE]), 1.0, x+1, sy, sh);
                    drawVLine(cr, QTC_CAIRO_COL(qtcPalette.mouseover[QT_STD_BORDER]), 1.0, x, sy, sh);
                }
                break;
        }
        QTC_CAIRO_END
    }
    else
        parent_class->draw_extension(style, window, state, shadow_type, area, widget, detail, x, y, width, height, gap_side);
}

static void gtkDrawSlider(GtkStyle *style, GdkWindow *window, GtkStateType state,
                          GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget,
                          const gchar *detail, gint x, gint y, gint width, gint height,
                          GtkOrientation orientation)
{
    gboolean scrollbar=DETAIL("slider"),
             scale=DETAIL("hscale") || DETAIL("vscale");
    GdkColor new_colors[TOTAL_SHADES+1],
             *btn_colors;
    int      min=MIN_SLIDER_SIZE(opts.sliderThumbs);

    QTC_CAIRO_BEGIN

    if(GTK_IS_RANGE(widget) && scrollbar)
        setState(widget, &state, NULL);

    if(useButtonColor(detail))
    {
        if(scrollbar|scale && GTK_STATE_INSENSITIVE==state)
            btn_colors=qtcPalette.background;
        else if(QT_CUSTOM_COLOR_BUTTON(style))
        {
            shadeColors(&(style->bg[state]), new_colors);
            btn_colors=new_colors;
        }
        else
            QTC_SET_BTN_COLS(scrollbar, scale, FALSE)
    }

    FN_CHECK
    sanitizeSize(window, &width, &height);

    if(scrollbar || !(SLIDER_TRIANGULAR==opts.sliderStyle || (SLIDER_ROUND==opts.sliderStyle && ROUND_FULL==opts.round)))
    {
        gtk_paint_box(style, window, state, shadow_type, area, widget,
                      !scrollbar && SLIDER_PLAIN==opts.sliderStyle ? "qtc-slider" : "slider", x, y, width, height);

       /* Orientation is always vertical with Mozilla, why? Anyway this hack should be OK - as we only draw
          dashes when slider is larger than 'min' pixels... */
        orientation=width<height ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;
        if(LINE_NONE!=opts.sliderThumbs &&
           (scale || ((GTK_ORIENTATION_HORIZONTAL==orientation && width>=min) || height>=min)))
        {
            if(LINE_SUNKEN!=opts.sliderThumbs)
                if(GTK_ORIENTATION_HORIZONTAL==orientation)
                    x++;
                else
                    y++;

            switch(opts.sliderThumbs)
            {
                case LINE_FLAT:
                    drawLines(cr, x, y, width, height,
                              GTK_ORIENTATION_HORIZONTAL!=orientation, 3, 5, btn_colors, area, 5, 0, FALSE);
                    break;
                case LINE_SUNKEN:
                    drawLines(cr, x, y, width, height,
                              GTK_ORIENTATION_HORIZONTAL!=orientation, 4, 3, btn_colors, area, 3, 1, TRUE);
                    break;
                default:
                case LINE_DOTS:
                    drawDots(cr, x, y, width, height,
                             GTK_ORIENTATION_HORIZONTAL!=orientation, scale ? 3 : 5, scale ? 4 : 2, btn_colors, area, 0, 5);
            }
        }
    }
    else
    {
        gboolean    coloredMouseOver=GTK_STATE_PRELIGHT==state && opts.coloredMouseOver,
                    horiz=SLIDER_TRIANGULAR==opts.sliderStyle ? height>width : width>height;
        int         bgnd=getFill(state, FALSE),
                    xo=horiz ? 8 : 0,
                    yo=horiz ? 0 : 8,
                    size=SLIDER_TRIANGULAR==opts.sliderStyle ? 15 : 13,
                    light=APPEARANCE_DULL_GLASS==opts.sliderAppearance ? 1 : 0;
        GdkColor    *colors=coloredMouseOver
                       ? qtcPalette.mouseover
                       : btn_colors;
        GdkRegion    *region=NULL;
        GdkPoint     clip[8];
        GtkArrowType direction=horiz ? GTK_ARROW_DOWN : GTK_ARROW_RIGHT;
        gboolean     drawLight=(MO_GLOW!=opts.coloredMouseOver && MO_PLASTIK!=opts.coloredMouseOver) || !coloredMouseOver ||
                                       (SLIDER_ROUND==opts.sliderStyle &&
                                       (SHADE_BLEND_SELECTED==opts.shadeSliders || SHADE_SELECTED==opts.shadeSliders));

        if(SLIDER_TRIANGULAR==opts.sliderStyle)
        {
            switch(direction)
            {
                case GTK_ARROW_UP:
                default:
                case GTK_ARROW_DOWN:
                    y+=2;
                    {
                        GdkPoint pts[]={{x, y+2}, {x+2, y}, {x+8, y}, {x+10, y+2}, {x+10, y+9}, {x+5, y+14}, {x, y+9}};
                        region=gdk_region_polygon(pts, 7, GDK_EVEN_ODD_RULE);
                    }
                    break;
                case GTK_ARROW_RIGHT:
                case GTK_ARROW_LEFT:
                    x+=2;
                    {
                        GdkPoint pts[]={{x+2, y}, {x, y+2}, {x, y+8}, {x+2, y+10}, {x+9, y+10}, {x+14, y+5}, {x+9, y}};
                        region=gdk_region_polygon(pts, 7, GDK_EVEN_ODD_RULE);
                    }
            }
        }
        else
        {
            GdkPoint  clip[8]= {{x,       y+8+yo},  {x,       y+4},     {x+4,    y},        {x+8+xo, y},
                               { x+12+xo, y+4},     {x+12+xo, y+8+yo},  {x+8+xo, y+12+yo},  {x+4, y+12+yo} };

            region=gdk_region_polygon(clip, 8, GDK_EVEN_ODD_RULE);
        }

        if(IS_FLAT(opts.sliderAppearance))
        {
            drawAreaColor(cr, NULL, region, &colors[bgnd], x+1, y+1, width-2, height-2);

            if(opts.coloredMouseOver && coloredMouseOver)
            {
                int col=QTC_SLIDER_MO_SHADE,
                    len=QTC_SLIDER_MO_LEN;

                if(horiz)
                {
                    drawAreaColor(cr, NULL, region, &qtcPalette.mouseover[col], x+1, y+1, len, size-2);
                    drawAreaColor(cr, NULL, region, &qtcPalette.mouseover[col], x+width-(1+len), y+1, len, size-2);
                }
                else
                {
                    drawAreaColor(cr, NULL, region, &qtcPalette.mouseover[col], x+1, y+1, size-2, len);
                    drawAreaColor(cr, NULL, region, &qtcPalette.mouseover[col], x+1, y+height-(1+len), size-2, len);
                }
            }
        }
        else
        {
            drawBevelGradient(cr, style, NULL, region, x, y, horiz ? width-1 : size, horiz ? size : height-1, &colors[bgnd],
                              SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK,
                              horiz, TRUE, FALSE, opts.sliderAppearance, WIDGET_OTHER);

            if(opts.coloredMouseOver && coloredMouseOver)
            {
                int col=QTC_SLIDER_MO_SHADE,
                    len=QTC_SLIDER_MO_LEN;

                if(horiz)
                {
                    drawBevelGradient(cr, style, NULL, region, x+1, y+1, len, size-2, &qtcPalette.mouseover[col],
                                      SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK,
                                      horiz, TRUE, FALSE, opts.sliderAppearance, WIDGET_OTHER);
                    drawBevelGradient(cr, style, NULL, region, x+width-(1+len), y+1, len, size-2, &qtcPalette.mouseover[col],
                                      SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK,
                                      horiz, TRUE, FALSE, opts.sliderAppearance, WIDGET_OTHER);
                }
                else
                {
                    drawBevelGradient(cr, style, NULL, region, x+1, y+1, size-2, len, &qtcPalette.mouseover[col],
                                      SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK,
                                      horiz, TRUE, FALSE, opts.sliderAppearance, WIDGET_OTHER);
                    drawBevelGradient(cr, style, NULL, region, x+1, y+height-(1+len), size-2, len, &qtcPalette.mouseover[col],
                                      SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK,
                                      horiz, TRUE, FALSE, opts.sliderAppearance, WIDGET_OTHER);
                }
            }
        }

        gdk_region_destroy(region);

        if(SLIDER_TRIANGULAR==opts.sliderStyle)
        {
            double xd=x+0.5,
                   yd=y+0.5,
                   radius=2.5;

            cairo_new_path(cr);
            cairo_set_source_rgb(cr, QTC_CAIRO_COL(colors[QT_STD_BORDER]));
            switch(direction)
            {
                case GTK_ARROW_UP:
                default:
                case GTK_ARROW_DOWN:
                    cairo_move_to(cr, xd+radius, yd);
                    cairo_arc(cr, xd+10-radius, yd+radius, radius, M_PI * 1.5, M_PI * 2);
                    cairo_line_to(cr, xd+10, yd+9);
                    cairo_line_to(cr, xd+5, yd+14);
                    cairo_line_to(cr, xd, yd+9);
                    cairo_arc(cr, xd+radius, yd+radius, radius, M_PI, M_PI * 1.5);
                    cairo_stroke(cr);
                    if(drawLight)
                    {
                        drawVLine(cr, QTC_CAIRO_COL(colors[light]), 1.0, xd+1, yd+2, 7);
                        drawHLine(cr, QTC_CAIRO_COL(colors[light]), 1.0, xd+2, yd+1, 6);
                    }
                    break;
                case GTK_ARROW_RIGHT:
                case GTK_ARROW_LEFT:
                    cairo_move_to(cr, xd, yd+10-radius);
                    cairo_arc(cr, xd+radius, yd+radius, radius, M_PI, M_PI * 1.5);
                    cairo_line_to(cr, xd+9, yd);
                    cairo_line_to(cr, xd+14, yd+5);
                    cairo_line_to(cr, xd+9, yd+10);
                    cairo_arc(cr, xd+radius, yd+10-radius, radius, M_PI * 0.5, M_PI);
                    cairo_stroke(cr);
                    if(drawLight)
                    {
                        drawHLine(cr, QTC_CAIRO_COL(colors[light]), 1.0, xd+2, yd+1, 7);
                        drawVLine(cr, QTC_CAIRO_COL(colors[light]), 1.0, xd+1, yd+2, 6);
                    }
            }
        }
        else
        {
            GdkPixbuf *border=getPixbuf(&colors[coloredMouseOver ? 4 : QT_BORDER(GTK_STATE_INSENSITIVE!=state)],
                                        horiz ? PIX_SLIDER : PIX_SLIDER_V, 0.8);

            gdk_cairo_set_source_pixbuf(cr, border, x, y);
            cairo_paint(cr);

            if(drawLight)
            {
                GdkPixbuf *light=getPixbuf(&colors[0], horiz ? PIX_SLIDER_LIGHT : PIX_SLIDER_LIGHT_V, 1.0);

                gdk_cairo_set_source_pixbuf(cr, light, x, y);
                cairo_paint(cr);
            }
        }
    }

    QTC_CAIRO_END
}

static void gtkDrawShadowGap(GtkStyle *style, GdkWindow *window, GtkStateType state,
                             GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget,
                             const gchar *detail, gint x, gint y, gint width,
                             gint height, GtkPositionType gap_side, gint gap_x, gint gap_width)
{
    if(GTK_IS_FRAME(widget) && (NULL!=gtk_frame_get_label(GTK_FRAME(widget)) || NULL!=gtk_frame_get_label_widget(GTK_FRAME(widget))))
        if(opts.framelessGroupBoxes)
            return;
        else if(gap_x<5)
            gap_x+=5, gap_width+=2;

    QTC_CAIRO_BEGIN
    drawBoxGap(cr, style, window, shadow_type, state, widget, area, x, y,
               width, height, gap_side, gap_x, gap_width, BORDER_FLAT, FALSE);
    QTC_CAIRO_END
}

static void gtkDrawHLine(GtkStyle *style, GdkWindow *window, GtkStateType state, GdkRectangle *area,
                         GtkWidget *widget, const gchar *detail, gint x1, gint x2, gint y)
{
    gboolean tbar=DETAIL("toolbar");
    int      light=0,
             dark=tbar ? (LINE_FLAT==opts.toolbarSeparators ? 4 : 3) : 5;

    FN_CHECK
    QTC_CAIRO_BEGIN

#ifdef QTC_DEBUG
printf("Draw hline %d %d %d %d %s  ", state, x1, x2, y, detail ? detail : "NULL");
debugDisplayWidget(widget, 3);
#endif

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
                drawHLine(cr, QTC_CAIRO_COL(qtcPalette.background[dark]), 1.0, x1<x2 ? x1 : x2, y, abs(x2-x1));
                if(LINE_SUNKEN==opts.toolbarSeparators)
                    drawHLine(cr, QTC_CAIRO_COL(qtcPalette.background[light]), 1.0, x1<x2 ? x1 : x2, y+1, abs(x2-x1));
            }
        }
    }
    else if(DETAIL("label"))
    {
        if(state == GTK_STATE_INSENSITIVE)
            drawHLine(cr, QTC_CAIRO_COL(qtcPalette.background[light]), 1.0, (x1<x2 ? x1 : x2)+1, y+1, abs(x2-x1));
        drawHLine(cr, QTC_CAIRO_COL(style->text[state]), 1.0, x1<x2 ? x1 : x2, y, abs(x2-x1));
    }
    else if(DETAIL("menuitem"))
        drawHLine(cr, QTC_CAIRO_COL(qtcPalette.background[QTC_MENU_SEP_SHADE]), 1.0, x1<x2 ? x1 : x2, y, abs(x2-x1));
    else
        drawHLine(cr, QTC_CAIRO_COL(qtcPalette.background[dark]), 1.0, x1<x2 ? x1 : x2, y, abs(x2-x1));

    QTC_CAIRO_END
}

static void gtkDrawVLine(GtkStyle *style, GdkWindow *window, GtkStateType state, GdkRectangle *area,
                         GtkWidget *widget, const gchar *detail, gint y1, gint y2, gint x)
{
    FN_CHECK
    QTC_CAIRO_BEGIN

#ifdef QTC_DEBUG
printf("Draw vline %d %d %d %d %s  ", state, x, y1, y2, detail ? detail : "NULL");
debugDisplayWidget(widget, 3);
#endif

    if(!(DETAIL("vseparator") && isOnCombo(widget, 0))) /* CPD: Combo handled in drawBox */
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
                    drawDots(cr, x, y1, 2, y2-y1, TRUE, (((y2-y1)/3.0)+0.5), 0,
                             qtcPalette.background, area, 0, 5);
                    break;
                case LINE_NONE:
                    break;
                case LINE_FLAT:
                case LINE_SUNKEN:
                {
                    drawVLine(cr, QTC_CAIRO_COL(qtcPalette.background[dark]), 1.0, x, y1<y2 ? y1 : y2, abs(y2-y1));
                    if(LINE_SUNKEN==opts.toolbarSeparators)
                        drawVLine(cr, QTC_CAIRO_COL(qtcPalette.background[light]), 1.0, x+1, y1<y2 ? y1 : y2, abs(y2-y1));
                }
            }
        }
        else
            drawVLine(cr, QTC_CAIRO_COL(qtcPalette.background[dark]), 1.0, x, y1<y2 ? y1 : y2, abs(y2-y1));
    }
    QTC_CAIRO_END
}

static void gtkDrawFocus(GtkStyle *style, GdkWindow *window, GtkStateType state,
                         GdkRectangle *area, GtkWidget *widget, const gchar *detail,
                         gint x, gint y, gint width, gint height)
{
    if(GTK_IS_EDITABLE(widget))
        return;

    sanitizeSize(window, &width, &height);

#ifdef QTC_DEBUG
    printf("Draw focus %d %d %d %d %d %s \n", state, x, y, width, height, detail ? detail : "NULL");
    debugDisplayWidget(widget, 3);
#endif

    {
    gboolean doEtch=QTC_DO_EFFECT;

    if(opts.comboSplitter && isComboBox(widget))
    {
/*
        x++;
        y++;
        height-=2;
*/
        width+=2; /* Remove if re-add the above */

        if(widget && reverseLayout(widget->parent))
            x+=20;
        width-=22;

        if(isGimpCombo(widget))
            x+=2, y+=2, width-=4, height-=4;
    }
    else if(GTK_IS_OPTION_MENU(widget))
    {
        if(!opts.comboSplitter && widget && widget->allocation.width>width)
            width=widget->allocation.width-(doEtch ? 8 : 4);

        x++, y++, width-=2, height-=2;
    }

    if(isComboBoxEntryButton(widget))
    {
        if(doEtch)
            x++, y+=2, width-=3, height-=4;
        else
            x++, y++, width-=2, height-=2;
    }
    else if(GTK_IS_BUTTON(widget) && !GTK_IS_RADIO_BUTTON(widget) && !GTK_IS_CHECK_BUTTON(widget))
    {
        y--, height+=2;
        if(doEtch)
            x--, width+=2;
    }

#ifndef QTC_PLAIN_FOCUS_ONLY
    if(opts.stdFocus)
#endif
        parent_class->draw_focus(style, window, state, area, widget, detail, x, y, width, height);
#ifndef QTC_PLAIN_FOCUS_ONLY
    else
    {
        QTC_CAIRO_BEGIN

        if(isList(widget) || width<3 || height < 3)
        {
            height--;
            cairo_new_path(cr);
            cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[QT_FOCUS]));
            cairo_rectangle(cr, x+0.5, y+0.5, width, height);
            cairo_stroke(cr);
        }
        else
            realDrawBorder(cr, style, state, area, NULL, x, y, width, height,
                           qtcPalette.background, ROUNDED_ALL, BORDER_FLAT,
                           WIDGET_FOCUS, 0, QT_FOCUS);
        QTC_CAIRO_END
    }
#endif
    }
}

static void gtkDrawResizeGrip(GtkStyle *style, GdkWindow *window, GtkStateType state,
                              GdkRectangle *area, GtkWidget *widget, const gchar *detail,
                              GdkWindowEdge edge, gint x, gint y, gint width, gint height)
{
    int dark=QT_BORDER(GTK_STATE_INSENSITIVE!=state);

    FN_CHECK
    QTC_CAIRO_BEGIN

    switch(edge)
    {
        case GDK_WINDOW_EDGE_NORTH_WEST:
            /* make it square */
            if(width < height)
                height = width;
            else if(height < width)
                width = height;
            break;
        case GDK_WINDOW_EDGE_NORTH:
            if(width < height)
                height = width;
            break;
        case GDK_WINDOW_EDGE_NORTH_EAST:
            /* make it square, aligning to top right */
            if(width < height)
                height = width;
            else if(height < width)
            {
                x +=(width - height);
                width = height;
            }
            break;
        case GDK_WINDOW_EDGE_WEST:
            if(height < width)
                width = height;
            break;
        case GDK_WINDOW_EDGE_EAST:
            /* aligning to right */
            if(height < width)
            {
                x +=(width - height);
                width = height;
            }
            break;
        case GDK_WINDOW_EDGE_SOUTH_WEST:
            /* make it square, aligning to bottom left */
            if(width < height)
            {
                y +=(height - width);
                height = width;
            }
            else if(height < width)
                width = height;
            break;
        case GDK_WINDOW_EDGE_SOUTH:
            /* align to bottom */
            if(width < height)
            {
                y +=(height - width);
                height = width;
            }
            break;
        case GDK_WINDOW_EDGE_SOUTH_EAST:
            /* make it square, aligning to bottom right */
            if(width < height)
            {
                y +=(height - width);
                height = width;
            }
            else if(height < width)
            {
                x +=(width - height);
                width = height;
            }
            break;
        default:
            g_assert_not_reached();
    }

    /* Clear background */
    gtk_style_apply_default_background(style, window, FALSE, state, area, x, y, width, height);

    switch(edge)
    {
        case GDK_WINDOW_EDGE_WEST:
        case GDK_WINDOW_EDGE_EAST:
        {
            gint xi = x;

            while(xi < x + width)
            {
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[0]));
                cairo_move_to(cr, xi, y);
                cairo_line_to(cr, xi, y + height);
                cairo_stroke(cr);
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[dark]));
                xi++;
                cairo_move_to(cr, xi, y);
                cairo_line_to(cr, xi, y + height);
                cairo_stroke(cr);
                xi += 2;
            }
            break;
        }
        case GDK_WINDOW_EDGE_NORTH:
        case GDK_WINDOW_EDGE_SOUTH:
        {
            gint yi = y;

            while(yi < y + height)
            {
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[0]));
                cairo_move_to(cr, x, yi);
                cairo_line_to(cr, x + width, yi);
                cairo_stroke(cr);
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[dark]));
                yi++;
                cairo_move_to(cr, x, yi);
                cairo_line_to(cr, x + width, yi);
                cairo_stroke(cr);
                yi+= 2;
            }
            break;
        }
        case GDK_WINDOW_EDGE_NORTH_WEST:
        {
            gint xi = x + width,
                 yi = y + height;

            while(xi > x + 3)
            {
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[0]));
                cairo_move_to(cr, xi, y);
                cairo_line_to(cr, x, yi);
                cairo_stroke(cr);
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[dark]));
                --xi; --yi;
                cairo_move_to(cr, xi, y);
                cairo_line_to(cr, x, yi);
                cairo_stroke(cr);
                xi -= 3; yi -= 3;
            }
            break;
        }
        case GDK_WINDOW_EDGE_NORTH_EAST:
        {
            gint xi = x,
                 yi = y + height;

            while(xi <(x + width - 3))
            {
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[0]));
                cairo_move_to(cr, xi, y);
                cairo_line_to(cr, x + width, yi);
                cairo_stroke(cr);
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[dark]));
                ++xi; --yi;
                cairo_move_to(cr, xi, y);
                cairo_line_to(cr, x + width, yi);
                cairo_stroke(cr);
                xi += 3; yi -= 3;
            }
            break;
        }
        case GDK_WINDOW_EDGE_SOUTH_WEST:
        {
            gint xi = x + width,
            yi = y;

            while(xi > x + 3)
            {
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[0]));
                cairo_move_to(cr, x, yi);
                cairo_line_to(cr, xi, y + height);
                cairo_stroke(cr);
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[dark]));
                --xi; ++yi;
                cairo_move_to(cr, x, yi);
                cairo_line_to(cr, xi, y + height);
                cairo_stroke(cr);
                xi -= 3; yi += 3;
            }
            break;
        }
        case GDK_WINDOW_EDGE_SOUTH_EAST:
        {
            gint xi = x,
                 yi = y;

            while(xi <(x + width - 3))
            {
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[0]));
                cairo_move_to(cr, xi, y + height);
                cairo_line_to(cr, x + width, yi);
                cairo_stroke(cr);
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, QTC_CAIRO_COL(qtcPalette.background[dark]));
                xi++; yi++;
                cairo_move_to(cr, xi, y + height);
                cairo_line_to(cr, x + width, yi);
                cairo_stroke(cr);
                xi += 3; yi+=3;
            }
            break;
        }
        default:
            g_assert_not_reached();
    }

    QTC_CAIRO_END
}

static void gtkDrawExpander(GtkStyle *style, GdkWindow *window, GtkStateType state,
                            GdkRectangle *area, GtkWidget *widget, const gchar *detail,
                            gint x, gint y, GtkExpanderStyle expander_style)
{
#ifdef QTC_DEBUG
printf("Draw expander %d %s  ", state, detail ? detail : "NULL");
debugDisplayWidget(widget, 5);
#endif
    gboolean isExpander=widget && GTK_IS_EXPANDER(widget),
             fill=!isExpander || opts.coloredMouseOver || GTK_STATE_PRELIGHT!=state;
    GdkGC    *gc=isExpander && opts.coloredMouseOver && GTK_STATE_PRELIGHT==state
                    ? style->base_gc[GTK_STATE_PRELIGHT]
                    : style->text_gc[QTC_ARROW_STATE(state)];

    x-=QTC_LV_SIZE>>1;
    y-=QTC_LV_SIZE>>1;

    //QTC_CAIRO_BEGIN
    if(GTK_EXPANDER_COLLAPSED==expander_style)
        drawArrow(window, gc, area, reverseLayout(widget) ? GTK_ARROW_LEFT : GTK_ARROW_RIGHT,
                  x+(LARGE_ARR_WIDTH>>1), y+LARGE_ARR_HEIGHT, FALSE, fill);
    else
        drawArrow(window, gc, area, GTK_ARROW_DOWN, x+(LARGE_ARR_WIDTH>>1), y+LARGE_ARR_HEIGHT, FALSE, fill);
    //QTC_CAIRO_END
}

static void styleRealize(GtkStyle *style)
{
    QtCurveStyle *qtcurveStyle = (QtCurveStyle *)style;

    parent_class->realize(style);

    qtcurveStyle->button_text_gc=realizeColors(style, &qtSettings.colors[PAL_ACTIVE][COLOR_BUTTON_TEXT]);
    if(opts.customMenuTextColor)
    {
        qtcurveStyle->menutext_gc[0]=realizeColors(style, &opts.customMenuNormTextColor);
        qtcurveStyle->menutext_gc[1]=realizeColors(style, &opts.customMenuSelTextColor);
    }
    else
        qtcurveStyle->menutext_gc[0]=NULL;
}

static void styleUnrealize(GtkStyle *style)
{
    QtCurveStyle *qtcurveStyle = (QtCurveStyle *)style;

    parent_class->unrealize(style);
    gtk_gc_release(qtcurveStyle->button_text_gc);
    if(opts.customMenuTextColor)
    {
        gtk_gc_release(qtcurveStyle->menutext_gc[0]);
        gtk_gc_release(qtcurveStyle->menutext_gc[1]);
        qtcurveStyle->menutext_gc[0]=qtcurveStyle->menutext_gc[1]=NULL;
    }
}

static void generateColors()
{
    shadeColors(&qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW], qtcPalette.background);
    shadeColors(&qtSettings.colors[PAL_ACTIVE][COLOR_BUTTON], qtcPalette.button);
    shadeColors(&qtSettings.colors[PAL_ACTIVE][COLOR_SELECTED], qtcPalette.menuitem);

    if(SHADE_CUSTOM==opts.shadeMenubars)
        shadeColors(&opts.customMenubarsColor, qtcPalette.menubar);
    else if(SHADE_BLEND_SELECTED==opts.shadeMenubars)
    {
        GdkColor color;

        if(IS_GLASS(opts.appearance))
            shade(&qtcPalette.menuitem[ORIGINAL_SHADE], &color, MENUBAR_GLASS_SELECTED_DARK_FACTOR);
        else
            color=qtcPalette.menuitem[ORIGINAL_SHADE];

        shadeColors(&color, qtcPalette.menubar);
    }

    switch(opts.shadeSliders)
    {
        case SHADE_SELECTED:
            qtcPalette.slider=qtcPalette.menuitem;
            break;
        case SHADE_CUSTOM:
            qtcPalette.slider=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
            shadeColors(&opts.customSlidersColor, qtcPalette.slider);
            break;
        case SHADE_BLEND_SELECTED:
        {
            GdkColor mid;

            generateMidColor(&qtcPalette.menuitem[ORIGINAL_SHADE],
                            &qtcPalette.button[ORIGINAL_SHADE], &mid, 1.0);
            qtcPalette.slider=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
            shadeColors(&mid, qtcPalette.slider);
        }
        default:
            break;
    }

    if(IND_TINT==opts.defBtnIndicator)
    {
        GdkColor col;

        tintColor(&qtcPalette.button[ORIGINAL_SHADE],
                  &qtcPalette.menuitem[ORIGINAL_SHADE], &col, 0.2);
        qtcPalette.defbtn=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
        shadeColors(&col, qtcPalette.defbtn);
    }
    else if(IND_COLORED==opts.defBtnIndicator)
    {
        if(SHADE_BLEND_SELECTED==opts.shadeSliders)
            qtcPalette.defbtn=qtcPalette.slider;
        else
        {
            GdkColor mid;

            generateMidColor(&qtcPalette.menuitem[ORIGINAL_SHADE],
                             &qtcPalette.button[ORIGINAL_SHADE], &mid, 1.0);
            qtcPalette.defbtn=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
            shadeColors(&mid, qtcPalette.defbtn);
        }
    }

    if(opts.coloredMouseOver || IND_CORNER==opts.defBtnIndicator || IND_GLOW==opts.defBtnIndicator)
    {
        if(IND_COLORED==opts.defBtnIndicator)
            qtcPalette.mouseover=qtcPalette.defbtn;
        else if(SHADE_BLEND_SELECTED==opts.shadeSliders)
            qtcPalette.mouseover=qtcPalette.slider;
        else
        {
            GdkColor mid;

            generateMidColor(&qtcPalette.menuitem[ORIGINAL_SHADE],
                             &qtcPalette.button[ORIGINAL_SHADE], &mid, 1.0);
            qtcPalette.mouseover=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
            shadeColors(&mid, qtcPalette.mouseover);
        }
    }

    switch(opts.shadeCheckRadio)
    {
        default:
            qtcPalette.check_radio=&qtSettings.colors[PAL_ACTIVE][COLOR_BUTTON_TEXT];
            break;
        case SHADE_BLEND_SELECTED:
        case SHADE_SELECTED:
            qtcPalette.check_radio=&qtSettings.colors[PAL_ACTIVE][COLOR_SELECTED];
            break;
        case SHADE_CUSTOM:
            qtcPalette.check_radio=&opts.customCheckRadioColor;
    }
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
}

static GtkRcStyleClass *parent_rc_class;
GType qtcurve_type_rc_style = 0;

static guint qtcurve_rc_style_parse(GtkRcStyle *rc_style, GtkSettings *settings, GScanner *scanner)
{
    static GQuark scope_id = 0;
    guint old_scope,
          token;

    /* Set up a new scope in this scanner. */
    if(!scope_id)
        scope_id = g_quark_from_string("qtcurve_theme_engine");

    /* If we bail out due to errors, we *don't* reset the scope, so the error messaging code can make
       sense of our tokens. */
    old_scope = g_scanner_set_scope(scanner, scope_id);
    qtSetFont(rc_style);

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
    QtCurveRcStyle *dest_w,
                   *src_w;

    parent_rc_class->merge(dest, src);

    if(!QTCURVE_IS_RC_STYLE(src))
        return;

    src_w = QTCURVE_RC_STYLE(src);
    dest_w = QTCURVE_RC_STYLE(dest);
}

/* Create an empty style suitable to this RC style */
static GtkStyle * qtcurve_rc_style_create_style(GtkRcStyle *rc_style)
{
    GtkStyle *style=g_object_new(QTCURVE_TYPE_STYLE, NULL);

    qtSetColors(style, rc_style, &opts);

    return style;
}

GType qtcurve_type_style = 0;

static void qtcurve_style_init(QtCurveStyle *style)
{
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

    qtcurve_type_style = g_type_module_register_type(module, GTK_TYPE_STYLE, "QtCurveStyle",
                                                     &object_info, 0);
}

static void qtcurve_rc_style_init(QtCurveRcStyle *qtcurve_rc)
{
    if(qtInit(&opts))
        generateColors();
#ifdef QTC_ADD_EVENT_FILTER
    qtcAddEventFilter();
#endif
}

static void qtcurve_rc_style_finalize(GObject *object)
{
    qtc_animation_cleanup();
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

    qtcurve_type_rc_style = g_type_module_register_type(module, GTK_TYPE_RC_STYLE, "QtCurveRcStyle",
                                                        &object_info, 0);
}

G_MODULE_EXPORT void theme_init(GTypeModule *module)
{
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

/* The following function will be called by GTK+ when the module is loaded and checks to see if we are
   compatible with the version of GTK+ that loads us. */
G_MODULE_EXPORT const gchar * g_module_check_init(GModule *module);

const gchar* g_module_check_init(GModule *module)
{
    return gtk_check_version(GTK_MAJOR_VERSION, GTK_MINOR_VERSION,
                             GTK_MICRO_VERSION - GTK_INTERFACE_AGE);
}
