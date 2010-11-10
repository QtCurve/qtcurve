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

#include <gmodule.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "compatability.h"
#define COMMON_FUNCTIONS
#include "qtcurve.h"

#define MO_ARROW(MENU, COL) (!MENU && MO_NONE!=opts.coloredMouseOver && GTK_STATE_PRELIGHT==state \
                                    ? &qtcPalette.mouseover[ARROW_MO_SHADE] : (COL))

#define SBAR_BTN_SIZE opts.sliderWidth

static struct
{
    GdkColor background[TOTAL_SHADES+1],
             button[2][TOTAL_SHADES+1],
             *slider,
             *defbtn,
             *mouseover,
             *combobtn,
             *selectedcr,
             *sortedlv,
             *sidebar,
             *progress,
             *wborder[2],
             mdi_text[2],
             menubar[TOTAL_SHADES+1],
             highlight[TOTAL_SHADES+1],
             focus[TOTAL_SHADES+1],
             menu[TOTAL_SHADES+1],
             *check_radio;
} qtcPalette;

static Options opts;

#include "qt_settings.c"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include "animation.c"
#include "menu.c"
#include "tab.c"
#include "widgetmap.c"
#include "window.c"
#include "entry.c"
#if !GTK_CHECK_VERSION(2, 90, 0)
#include "wmmove.c"
#endif
#include "pixmaps.h"
#include "config.h"
#include <cairo.h>

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
    GtkShadowType shadow_type;
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

#define M_PI 3.14159265358979323846
#define CAIRO_COL(A) (A).red/65535.0, (A).green/65535.0, (A).blue/65535.0

#if GTK_CHECK_VERSION(2, 90, 0)
    #define CAIRO_BEGIN \
        { \
        GdkRectangle *area=NULL; \
        cairo_set_line_width(cr, 1.0);

    #define CAIRO_END }
    #define FN_CHECK g_return_if_fail(GTK_IS_STYLE(style));
#else
    #define CAIRO_BEGIN \
        if(GDK_IS_DRAWABLE(window)) \
        { \
        cairo_t *cr=(cairo_t*)gdk_cairo_create(window); \
        setCairoClipping(cr, area); \
        cairo_set_line_width(cr, 1.0);

    #define CAIRO_END \
        cairo_destroy(cr); \
        }
    #define FN_CHECK g_return_if_fail(GTK_IS_STYLE(style)); g_return_if_fail(window != NULL);
#endif

#define STYLE style
#define WIDGET_TYPE_NAME(xx) (widget && !strcmp(g_type_name (G_TYPE_FROM_INSTANCE(widget)), (xx)))

#ifndef GTK_IS_COMBO_BOX_ENTRY
#define GTK_IS_COMBO_BOX_ENTRY(x) 0
#endif
#ifndef GTK_IS_COMBO_BOX
#define GTK_IS_COMBO_BOX(x) 0
#endif

#if GTK_CHECK_VERSION(2, 90, 0)
    #define QTC_IS_COMBO(X)    GTK_IS_COMBO_BOX_TEXT(X)
    #define QTC_COMBO_ENTRY(X) GTK_IS_COMBO_BOX_TEXT(X)
#else
    #define QTC_IS_COMBO(X)    GTK_IS_COMBO(X)
    #define QTC_COMBO_ENTRY(X) GTK_IS_COMBO_BOX_ENTRY(X)
#endif

#define SLIDER_TROUGH_SIZE 5

static void gtkDrawBox(GtkStyle *style, WINDOW_PARAM GtkStateType state, GtkShadowType shadow_type, AREA_PARAM
                       GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height);

static void dumpChildren(GtkWidget *widget, int level)
{
    if(level<5)
    {
        GList *children = gtk_container_get_children(GTK_CONTAINER(widget)),
              *child    = children;

        for(; child; child=child->next)
        {
            GtkWidget *boxChild=(GtkWidget *)child->data;

            printf(":[%d]%s:", level, g_type_name(qtcWidgetType(boxChild)));
            if(GTK_IS_BOX(boxChild))
                dumpChildren(boxChild, ++level);
        }
        if(children)
            g_list_free(children);
    }
}

static void debugDisplayWidget(GtkWidget *widget, int level)
{
    if(level>=0)
    {
        printf("%s(%s)[%x] ", widget ? g_type_name(qtcWidgetType(widget)) : "NULL",
               widget && qtcWidgetName(widget) ? qtcWidgetName(widget) : "NULL", (int)widget);
        /*if(widget)
            printf("[%d, %dx%d : %d,%d , %0X] ", qtcWidgetGetState(widget), widget->allocation.x,
                   widget->allocation.y,
                   widget->allocation.width, widget->allocation.height, qtcWidgetGetWindow(widget));*/
        if(widget && qtcWidgetGetParent(widget))
            debugDisplayWidget(qtcWidgetGetParent(widget), --level);
        else
            printf("\n");
    }
    else
        printf("\n");
}

typedef struct
{
    GdkColor col;
    EPixmap  pix;
    double   shade;
} QtCPixKey;

static GtkStyleClass  *parent_class=NULL;
static GtkRequisition defaultOptionIndicatorSize    = { 6, 13 };
static GtkBorder      defaultOptionIndicatorSpacing = { 7, 5, 1, 1 };
static GCache         *pixbufCache                  = NULL;

#define DETAIL(xx) ((detail) &&(!strcmp(xx, detail)))
#define DETAILHAS(xx) ((detail) && (strstr(detail, xx)))

#define QTC_PANED "paned-qtc"
#define QTC_CHECKBOX "checkbox-qtc"
#define IS_QTC_PANED DETAIL(QTC_PANED)

#if GTK_CHECK_VERSION(2, 90, 0)
static cairo_rectangle_int_t createRect(int x, int y, int w, int h)
{
    cairo_rectangle_int_t r={x, y, w, h};
    return r;
}

static QtcRegion * windowMask(int x, int y, int w, int h, gboolean full)
{
    cairo_rectangle_int_t rects[4];
    int                   numRects=4;

    if(full)
    {
        rects[0]=createRect(x + 4, y + 0, w-4*2, h-0*2);
        rects[1]=createRect(x + 0, y + 4, w-0*2, h-4*2);
        rects[2]=createRect(x + 2, y + 1, w-2*2, h-1*2);
        rects[3]=createRect(x + 1, y + 2, w-1*2, h-2*2);
    }
    else
    {
        rects[0]=createRect(x+1, y+1, w-2, h-2);
        rects[1]=createRect(x, y+2, w, h-4);
        rects[2]=createRect(x+2, y, w-4, h);
        numRects=3;
    }

    return cairo_region_create_rectangles(rects, numRects);
}
#endif

static void clipToRegion(cairo_t *cr, QtcRegion *region)
{
    cairo_new_path(cr);
    {
#if GTK_CHECK_VERSION(2, 90, 0)
        int numRects=cairo_region_num_rectangles(region);

        while(numRects--)
        {
            cairo_rectangle_int_t rect;
            cairo_region_get_rectangle(region, numRects, &rect);
            cairo_rectangle(cr, rect.x, rect.y, rect.width, rect.height);
        }
#else
        int          numRects;
        GdkRectangle *rects;

        gdk_region_get_rectangles(region, &rects, &numRects);

        while(numRects--)
        {
            GdkRectangle *rect=&(rects[numRects]);
            cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
        }
        g_free(rects);
#endif
    }
    cairo_clip(cr);
}

void setCairoClippingRegion(cairo_t *cr, QtcRegion *region)
{
    cairo_save(cr);
    if(region)
        clipToRegion(cr, region);
    cairo_new_path(cr);
}

void setCairoClipping(cairo_t *cr, GdkRectangle *area)
{
    cairo_save(cr);
    if(area)
    {
        cairo_rectangle(cr, area->x, area->y, area->width, area->height);
        cairo_clip(cr);
    }
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

static GdkColor * menuColors(gboolean active)
{
    return SHADE_WINDOW_BORDER==opts.shadeMenubars
            ? qtcPalette.wborder[active ? 1 : 0]
            : SHADE_NONE==opts.shadeMenubars || (opts.shadeMenubarOnlyWhenActive && !active)
                ? qtcPalette.background
                : qtcPalette.menubar;
}

static EBorder shadowToBorder(GtkShadowType shadow)
{
    switch(shadow)
    {
        case GTK_SHADOW_NONE:
            return BORDER_FLAT;
        case GTK_SHADOW_IN:
        case GTK_SHADOW_ETCHED_IN:
            return BORDER_SUNKEN;
        case GTK_SHADOW_OUT:
        case GTK_SHADOW_ETCHED_OUT:
            return BORDER_RAISED;
    }
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
                       (detail[1] && &detail[1]==strstr(detail, "scrollbar")) ||
                       0==strcmp(detail, "stepper") ||
                       0==strcmp(detail, QTC_PANED) );
}

#define QT_CUSTOM_COLOR_BUTTON(style) \
    (style && \
    !(COL_EQ(qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW].red,(style->bg[GTK_STATE_NORMAL].red)) && \
      COL_EQ(qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW].green,(style->bg[GTK_STATE_NORMAL].green)) && \
      COL_EQ(qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW].blue,(style->bg[GTK_STATE_NORMAL].blue))))

static void shadeColors(GdkColor *base, GdkColor *vals)
{
    SHADES

    int      i;
    gboolean useCustom=USE_CUSTOM_SHADES(opts);
    double   hl=TO_FACTOR(opts.highlightFactor);

    for(i=0; i<NUM_STD_SHADES; ++i)
        shade(&opts, base, &vals[i], useCustom ? opts.customShades[i] : SHADE(opts.contrast, i));
    shade(&opts, base, &vals[SHADE_ORIG_HIGHLIGHT], hl);
    shade(&opts, &vals[4], &vals[SHADE_4_HIGHLIGHT], hl);
    shade(&opts, &vals[2], &vals[SHADE_2_HIGHLIGHT], hl);
    vals[ORIGINAL_SHADE]=*base;
}

static gboolean isSortColumn(GtkWidget *button)
{
#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
    GtkWidget *parent=NULL;
    if(button && (parent=qtcWidgetGetParent(button)) && GTK_IS_TREE_VIEW(parent))
    {
        GtkWidget *sort=NULL;;
        GList     *columns=gtk_tree_view_get_columns(GTK_TREE_VIEW(parent)),
                  *column;

        for (column = columns; column && !sort && sort!=button; column=g_list_next(column))
            if(GTK_IS_TREE_VIEW_COLUMN(column->data))
            {
                GtkTreeViewColumn *c=GTK_TREE_VIEW_COLUMN(column->data);

                if(gtk_tree_view_column_get_sort_indicator(c))
                    sort=c->button;
            }

        if(columns)
            g_list_free(columns);
        return sort==button;
    }
#endif
    return FALSE;
};

#define SET_BTN_COLS(SCROLLBAR, SCALE, LISTVIEW, STATE) \
{ \
    if(SCROLLBAR || SCALE) \
        btn_colors=GTK_STATE_INSENSITIVE==STATE \
                    ? qtcPalette.background \
                    : SHADE_NONE!=opts.shadeSliders && qtcPalette.slider && \
                      (!opts.colorSliderMouseOver || GTK_STATE_PRELIGHT==STATE) \
                        ? qtcPalette.slider \
                        : qtcPalette.button[PAL_ACTIVE]; \
    else if(LISTVIEW) \
    { \
        if(GTK_STATE_INSENSITIVE!=state && qtcPalette.sortedlv && isSortColumn(widget)) \
            btn_colors=qtcPalette.sortedlv;  \
        else if(opts.lvButton) \
            btn_colors=qtcPalette.button[GTK_STATE_INSENSITIVE==STATE ? PAL_DISABLED : PAL_ACTIVE]; \
        else \
            btn_colors=qtcPalette.background; \
    } \
    else \
        btn_colors=qtcPalette.button[GTK_STATE_INSENSITIVE==STATE ? PAL_DISABLED : PAL_ACTIVE]; \
}

static GdkColor * getCellCol(GdkColor *std, const gchar *detail)
{
    static GdkColor shaded;

    if(!qtSettings.shadeSortedList || !strstr(detail, "_sorted"))
        return std;

    shaded=*std;

    if(IS_BLACK(shaded))
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

#define ARROW_STATE(state) (GTK_STATE_INSENSITIVE==state ? state : GTK_STATE_NORMAL)
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
                *horiz=GTK_ORIENTATION_HORIZONTAL==qtcToolbarGetOrientation(widget);
            return TRUE;
        }
        else if(level<4)
            return isOnToolbar(qtcWidgetGetParent(widget), horiz, ++level);
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
            return isOnHandlebox(qtcWidgetGetParent(widget), horiz, ++level);
    }

    return FALSE;
}

static gboolean isButtonOnToolbar(GtkWidget *widget, gboolean *horiz)
{
    GtkWidget *parent=NULL;
    return (widget && (parent=qtcWidgetGetParent(widget)) && GTK_IS_BUTTON(widget))
               ? isOnToolbar(parent, horiz, 0)
               : FALSE;
}

static gboolean isButtonOnHandlebox(GtkWidget *widget, gboolean *horiz)
{
    GtkWidget *parent=NULL;
    return (widget && (parent=qtcWidgetGetParent(widget)) && GTK_IS_BUTTON(widget))
               ? isOnHandlebox(parent, horiz, 0)
               : FALSE;
}

static gboolean isOnStatusBar(GtkWidget *widget, int level)
{
    GtkWidget *parent=qtcWidgetGetParent(widget);
    if(parent)
        if(GTK_IS_STATUSBAR(parent))
            return TRUE;
        else if(level<4)
            return isOnStatusBar(parent, ++level);

    return FALSE;
}

static gboolean isList(GtkWidget *widget)
{
    return widget &&
           (GTK_IS_TREE_VIEW(widget) ||
#if !GTK_CHECK_VERSION(2, 90, 0)
            GTK_IS_CLIST(widget) ||
            GTK_IS_LIST(widget) ||
            
#ifdef GTK_ENABLE_BROKEN
            GTK_IS_TREE(widget) ||
#endif
            GTK_IS_CTREE(widget) ||
#endif
            0==strcmp(g_type_name(qtcWidgetType(widget)), "GtkSCTree"));
}

static gboolean isListViewHeader(GtkWidget *widget)
{
    GtkWidget *parent=NULL;
    return widget && GTK_IS_BUTTON(widget) && (parent=qtcWidgetGetParent(widget)) &&
           (isList(parent) ||
            (GTK_APP_GIMP==qtSettings.app && GTK_IS_BOX(parent) &&
             (parent=qtcWidgetGetParent(parent)) && GTK_IS_EVENT_BOX(parent) &&
             (parent=qtcWidgetGetParent(parent)) && 0==strcmp(g_type_name(qtcWidgetType(parent)), "GimpThumbBox")));
}

static gboolean isEvolutionListViewHeader(GtkWidget *widget, const gchar *detail)
{
    GtkWidget *parent=NULL;
    return GTK_APP_EVOLUTION==qtSettings.app && widget && DETAIL("button") &&
           0==strcmp(g_type_name(qtcWidgetType(widget)), "ECanvas") &&
           (parent=qtcWidgetGetParent(widget)) && (parent=qtcWidgetGetParent(parent)) &&
           GTK_IS_SCROLLED_WINDOW(parent);
}

static gboolean isOnListViewHeader(GtkWidget *w, int level)
{
    if(w)
    {
        if(isListViewHeader(w))
            return TRUE;
        else if(level<4)
            return isOnListViewHeader(qtcWidgetGetParent(w), ++level);
    }
    return FALSE;
}

static gboolean isPathButton(GtkWidget *widget)
{
    return widget && qtcWidgetGetParent(widget) && GTK_IS_BUTTON(widget) &&
           0==strcmp(g_type_name(qtcWidgetType(qtcWidgetGetParent(widget))), "GtkPathBar");
}

// static gboolean isTabButton(GtkWidget *widget)
// {
//     return widget && GTK_IS_BUTTON(widget) && qtcWidgetGetParent(widget) &&
//            (GTK_IS_NOTEBOOK(qtcWidgetGetParent(widget)) ||
//             (qtcWidgetGetParent(widget)->parent && GTK_IS_BOX(qtcWidgetGetParent(widget)) && GTK_IS_NOTEBOOK(qtcWidgetGetParent(widget)->parent)));
// }

static GtkWidget * getComboEntry(GtkWidget *widget)
{
    GList     *children = gtk_container_get_children(GTK_CONTAINER(widget)),
              *child    = children;
    GtkWidget *rv       = NULL;

    for(; child && !rv; child=child->next)
    {
        GtkWidget *boxChild=(GtkWidget *)child->data;

        if(GTK_IS_ENTRY(boxChild))
            rv=(GtkWidget *)boxChild;
    }

    if(children)
        g_list_free(children);
    return rv;
}

static GtkWidget * getComboButton(GtkWidget *widget)
{
    GList     *children = gtk_container_get_children(GTK_CONTAINER(widget)),
              *child    = children;
    GtkWidget *rv       = NULL;

    for(; child && !rv; child=child->next)
    {
        GtkWidget *boxChild=(GtkWidget *)child->data;

        if(GTK_IS_BUTTON(boxChild))
            rv=(GtkWidget *)boxChild;
    }

    if(children)
        g_list_free(children);
    return rv;
}
                                  
static gboolean isSideBarBtn(GtkWidget *widget)
{
    GtkWidget *parent=NULL;
    return widget && (parent=qtcWidgetGetParent(widget)) &&
           (0==strcmp(g_type_name(qtcWidgetType(parent)), "GdlDockBar") ||
            (0==strcmp(g_type_name(qtcWidgetType(parent)), "GdlSwitcher")/* &&
             qtcWidgetGetParent(parent) &&
             0==strcmp(g_type_name(qtcWidgetType(parent)), "GdlDockNotebook")*/) );
}

static gboolean isComboBoxButton(GtkWidget *widget)
{
    GtkWidget *parent=NULL;
    return widget && GTK_IS_BUTTON(widget) && (parent=qtcWidgetGetParent(widget)) &&
           (QTC_COMBO_ENTRY(parent) || QTC_IS_COMBO(parent));
}

static gboolean isComboBox(GtkWidget *widget)
{
    GtkWidget *parent=NULL;
    return widget && GTK_IS_BUTTON(widget) && (parent=qtcWidgetGetParent(widget)) &&
           !QTC_COMBO_ENTRY(parent) && (GTK_IS_COMBO_BOX(parent) || QTC_IS_COMBO(parent));
}

static gboolean isComboBoxEntry(GtkWidget *widget)
{
    GtkWidget *parent=NULL;
    return widget && GTK_IS_ENTRY(widget) && (parent=qtcWidgetGetParent(widget)) &&
           (QTC_COMBO_ENTRY(parent) || QTC_IS_COMBO(parent));
}

static gboolean isComboBoxEntryButton(GtkWidget *widget)
{
    GtkWidget *parent=NULL;
    return widget && (parent=qtcWidgetGetParent(widget)) && GTK_IS_TOGGLE_BUTTON(widget) && QTC_COMBO_ENTRY(parent);
}

/*
static gboolean isSwtComboBoxEntry(GtkWidget *widget)
{
    return GTK_APP_JAVA_SWT==qtSettings.app &&
           isComboBoxEntry(widget) &&
           qtcWidgetGetParent(widget)->parent && 0==strcmp(g_type_name(qtcWidgetType(qtcWidgetGetParent(widget)->parent)), "SwtFixed");
}
*/

static gboolean isGimpCombo(GtkWidget *widget)
{
    GtkWidget *parent=NULL;
    return GTK_APP_GIMP==qtSettings.app &&
           widget && (parent=qtcWidgetGetParent(widget)) && GTK_IS_TOGGLE_BUTTON(widget) &&
           0==strcmp(g_type_name(qtcWidgetType(parent)), "GimpEnumComboBox");
}

static gboolean isOnComboEntry(GtkWidget *w, int level)
{
    if(w)
    {
        if(QTC_COMBO_ENTRY(w))
            return TRUE;
        else if(level<4)
            return isOnComboEntry(qtcWidgetGetParent(w), ++level);
    }
    return FALSE;
}

static gboolean isOnComboBox(GtkWidget *w, int level)
{
    if(w)
    {
        if(GTK_IS_COMBO_BOX(w))
            return TRUE;
        else if(level<4)
            return isOnComboBox(qtcWidgetGetParent(w), ++level);
    }
    return FALSE;
}

static gboolean isOnCombo(GtkWidget *w, int level)
{
    if(w)
    {
        if(QTC_IS_COMBO(w))
            return TRUE;
        else if(level<4)
            return isOnCombo(qtcWidgetGetParent(w), ++level);
    }
}

static gboolean isOnOptionMenu(GtkWidget *w, int level)
{
#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
    if(w)
    {
        if(GTK_IS_OPTION_MENU(w))
            return TRUE;
        else if(level<4)
            return isOnOptionMenu(qtcWidgetGetParent(w), ++level);
    }
#endif
    return FALSE;
}

static gboolean isActiveCombo(GtkWidget *widget)
{
#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
    if(GTK_IS_OPTION_MENU(widget))
    {
        GtkWidget *menu=gtk_option_menu_get_menu(GTK_OPTION_MENU(widget));
        if(menu && qtcWidgetVisible(menu) && qtcWidgetRealized(menu))
            return TRUE;
    }
#endif
    return FALSE;
}

static gboolean isSpinButton(GtkWidget *widget)
{
    return widget && GTK_IS_SPIN_BUTTON(widget);
}

static gboolean isStatusBarFrame(GtkWidget *widget)
{
    GtkWidget *parent=NULL;
    return widget && (parent=qtcWidgetGetParent(widget)) && GTK_IS_FRAME(widget) &&
           (GTK_IS_STATUSBAR(parent) || ((parent=qtcWidgetGetParent(parent)) && GTK_IS_STATUSBAR(parent)));
}

static GtkMenuBar * isMenubar(GtkWidget *w, int level)
{
    if(w)
    {
        if(GTK_IS_MENU_BAR(w))
            return (GtkMenuBar*)w;
        else if(level<3)
            return isMenubar(qtcWidgetGetParent(w), level++);
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
            return isMenuitem(qtcWidgetGetParent(w), level++);
    }

    return FALSE;
}

#define IS_MENU_ITEM(WIDGET) isMenuitem(WIDGET, 0)

static gboolean isMenuWindow(GtkWidget *w)
{
    GtkWidget *def=qtcWindowDefaultWidget(w);
    
    return def && GTK_IS_MENU(def);
}

#define IS_GROUP_BOX(W) ((W) && GTK_IS_FRAME((W)) && (NULL!=gtk_frame_get_label(GTK_FRAME((W))) || \
                                                      NULL!=gtk_frame_get_label_widget(GTK_FRAME((W)))))

static gboolean isInGroupBox(GtkWidget *w, int level)
{
    if(w)
    {
        if(IS_GROUP_BOX(w))
            return TRUE;
        else if(level<5)
            return isInGroupBox(qtcWidgetGetParent(w), level++);
    }

    return FALSE;
}

static gboolean isOnButton(GtkWidget *w, int level, gboolean *def)
{
    if(w)
    {
        if((GTK_IS_BUTTON(w)
#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
            || GTK_IS_OPTION_MENU(w)
#endif
            ) && (!(GTK_IS_RADIO_BUTTON(w) || GTK_IS_CHECK_BUTTON(w))))
        {
            if(def)
                *def=qtcWidgetHasDefault(w);
            return TRUE;
        }
        else if(level<3)
            return isOnButton(qtcWidgetGetParent(w), level++, def);
    }

    return FALSE;
}

static void optionMenuGetProps(GtkWidget *widget, GtkRequisition *indicator_size, GtkBorder *indicator_spacing)
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
    STEPPER_A,
    STEPPER_B,
    STEPPER_C,
    STEPPER_D,
    STEPPER_NONE
} EStepper;

#if GTK_CHECK_VERSION(2, 90, 0)
static EStepper getStepper(const char *detail)
#else
static EStepper getStepper(GtkWidget *widget, int x, int y, int width, int height)
#endif
{
#if GTK_CHECK_VERSION(2, 90, 0)
    if(detail && detail[1])
    {
        if(0==strcmp(&detail[11], "end_inner"))
            return STEPPER_C;
        else if(strstr(&detail[11], "start_inner"))
            return STEPPER_B;
        else if(0==strcmp(&detail[11], "end"))
            return STEPPER_D;
        else if(strstr(&detail[11], "start"))
            return STEPPER_A;
    }
#else
    if(widget && GTK_IS_RANGE(widget))
    {
        GdkRectangle   tmp;
        GdkRectangle   check_rectangle,
                       stepper;
        GtkOrientation orientation=qtcRangeGetOrientation(widget);
        GtkAllocation  alloc=qtcWidgetGetAllocation(widget);

        stepper.x=x;
        stepper.y=y;
        stepper.width=width;
        stepper.height=height;
        check_rectangle.x      = alloc.x;
        check_rectangle.y      = alloc.y;
        check_rectangle.width  = stepper.width;
        check_rectangle.height = stepper.height;

        if (-1==alloc.x && -1==alloc.y)
            return STEPPER_NONE;

        if (gdk_rectangle_intersect(&stepper, &check_rectangle, &tmp))
            return STEPPER_A;

        if (GTK_ORIENTATION_HORIZONTAL==orientation)
            check_rectangle.x = alloc.x + stepper.width;
        else
            check_rectangle.y = alloc.y + stepper.height;

        if (gdk_rectangle_intersect(&stepper, &check_rectangle, &tmp))
            return STEPPER_B;

        if (GTK_ORIENTATION_HORIZONTAL==orientation)
            check_rectangle.x = alloc.x + alloc.width - (stepper.width * 2);
        else
            check_rectangle.y = alloc.y + alloc.height - (stepper.height * 2);

        if (gdk_rectangle_intersect(&stepper, &check_rectangle, &tmp))
            return STEPPER_C;

        if (GTK_ORIENTATION_HORIZONTAL==orientation)
            check_rectangle.x = alloc.x + alloc.width - stepper.width;
        else
            check_rectangle.y = alloc.y + alloc.height - stepper.height;

        if (gdk_rectangle_intersect(&stepper, &check_rectangle, &tmp))
            return STEPPER_D;
    }
#endif
    return STEPPER_NONE;
}

static int getFillReal(GtkStateType state, gboolean set, gboolean darker)
{
    return GTK_STATE_INSENSITIVE==state
               ? (darker ? 2 : ORIGINAL_SHADE)
               : GTK_STATE_PRELIGHT==state
                   ? set /*&& allow_mouse_over_set*/
                       ? (darker ? 3 : SHADE_4_HIGHLIGHT)
                       : (darker ? SHADE_2_HIGHLIGHT : SHADE_ORIG_HIGHLIGHT)
                   : set || GTK_STATE_ACTIVE==state
                       ? (darker ? 5 : 4)
                       : (darker ? 2 : ORIGINAL_SHADE);
}

#define getFill(state, set) getFillReal(state, set, FALSE)

static gboolean isSbarDetail(const char *detail)
{
    return detail && (
#if GTK_CHECK_VERSION(2, 90, 0)
                      (detail[1] && &detail[1]==strstr(detail, "scrollbar")) ||
#endif
                      0==strcmp(detail, "hscrollbar") || 0==strcmp(detail, "vscrollbar") ||
                      0==strcmp(detail, "stepper"));
}

static int getRound(const char *detail, GtkWidget *widget, int x, int y, int width, int height, gboolean rev)
{
    if(detail)
    {
        if(0==strcmp(detail, "slider"))
            return
#ifndef SIMPLE_SCROLLBARS
                    !(opts.square&SQUARE_SB_SLIDER) && (SCROLLBAR_NONE==opts.scrollbarType || opts.flatSbarButtons) 
                        ? ROUNDED_ALL :
#endif
                    ROUNDED_NONE;
        else if(0==strcmp(detail, "qtc-slider"))
            return opts.square&SQUARE_SLIDER && (SLIDER_PLAIN==opts.sliderStyle || SLIDER_PLAIN_ROTATED==opts.sliderStyle)
                ? ROUNDED_NONE : ROUNDED_ALL;
        else if(0==strcmp(detail, "splitter") || 0==strcmp(detail, "optionmenu")  ||
                0==strcmp(detail, "togglebutton") || 0==strcmp(detail, "hscale") ||
                0==strcmp(detail, "vscale") || 0==strcmp(detail, QTC_CHECKBOX)
                /* || 0==strcmp(detail, "paned") || 0==strcmp(detail, QTC_PANED)*/ )
            return ROUNDED_ALL;
        else if(0==strcmp(detail, "spinbutton_up"))
            return rev ? ROUNDED_TOPLEFT : ROUNDED_TOPRIGHT;
        else if(0==strcmp(detail, "spinbutton_down"))
            return rev ? ROUNDED_BOTTOMLEFT : ROUNDED_BOTTOMRIGHT;
        else if(isSbarDetail(detail))
        {

            switch(getStepper(
#if GTK_CHECK_VERSION(2, 90, 0)
                                detail
#else
                                widget, x, y, width, height
#endif
                              ))
            {
                case STEPPER_A:
                    return 'h'==detail[0] ? ROUNDED_LEFT : ROUNDED_TOP;
                case STEPPER_D:
                    return 'v'==detail[0] ? ROUNDED_BOTTOM : ROUNDED_RIGHT;
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

#if GTK_CHECK_VERSION(2, 90, 0)
    return GTK_ORIENTATION_HORIZONTAL==qtcGetWidgetOrientation(widget);
#else
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
#endif
}

static gboolean isComboBoxPopupWindow(GtkWidget *widget, int level)
{
    if(widget)
    {
        if(qtcWidgetName(widget) && GTK_IS_WINDOW(widget) &&
           0==strcmp(qtcWidgetName(widget), "gtk-combobox-popup-window"))
            return TRUE;
        else if(level<4)
            return isComboBoxPopupWindow(qtcWidgetGetParent(widget), ++level);
    }
    return FALSE;
}

static gboolean isComboBoxList(GtkWidget *widget)
{
    GtkWidget *parent=NULL;
    return widget && (parent=qtcWidgetGetParent(widget)) && /*GTK_IS_FRAME(widget) && */isComboBoxPopupWindow(parent, 0);
}

static gboolean isComboPopupWindow(GtkWidget *widget, int level)
{
    if(widget)
    {
        if(qtcWidgetName(widget) && GTK_IS_WINDOW(widget) &&
            0==strcmp(qtcWidgetName(widget), "gtk-combo-popup-window"))
            return TRUE;
        else if(level<4)
            return isComboPopupWindow(qtcWidgetGetParent(widget), ++level);
    }
    return FALSE;
}

static gboolean isComboList(GtkWidget *widget)
{
    GtkWidget *parent=NULL;
    return widget && (parent=qtcWidgetGetParent(widget)) && isComboPopupWindow(parent, 0);
}

static gboolean isComboMenu(GtkWidget *widget)
{
    if(widget && qtcWidgetName(widget) && GTK_IS_MENU(widget) && 0==strcmp(qtcWidgetName(widget), "gtk-combobox-popup-menu"))
        return TRUE;
    else
    {
        GtkWidget *top        = gtk_widget_get_toplevel(widget),
                  *topChild   = top ? qtcBinGetChild(GTK_BIN(top)) : NULL,
                  *transChild = NULL;
        GtkWindow *trans      = NULL;

        return topChild && (isComboBoxPopupWindow(topChild, 0) ||
                       //GTK_IS_DIALOG(top) || /* Dialogs should not have menus! */
                       (GTK_IS_WINDOW(top) && (trans=qtcWindowTransientFor(GTK_WINDOW(top))) &&
                        (transChild=qtcBinGetChild(GTK_BIN(trans))) && isComboMenu(transChild)));
    }
}

#if 0
static gboolean isComboFrame(GtkWidget *widget)
{
    return !QTC_COMBO_ENTRY(widget) && GTK_IS_FRAME(widget) && qtcWidgetGetParent(widget) && GTK_IS_COMBO_BOX(qtcWidgetGetParent(widget));
}
#endif

static gboolean isFixedWidget(GtkWidget *widget)
{
    GtkWidget *parent=NULL;
    return widget && (parent=qtcWidgetGetParent(widget)) && GTK_IS_FIXED(parent) &&
           (parent=qtcWidgetGetParent(parent)) && GTK_IS_WINDOW(parent);
}

static gboolean isGimpDockable(GtkWidget *widget)
{
    if(GTK_APP_GIMP==qtSettings.app)
    {
        GtkWidget *wid=widget;
        while(wid)
        {
            if(0==strcmp(g_type_name(qtcWidgetType(wid)), "GimpDockable") ||
               0==strcmp(g_type_name(qtcWidgetType(wid)), "GimpToolbox"))
                return TRUE;
            wid=qtcWidgetGetParent(wid);
        }
    }
    return FALSE;
}

#define isMozillaWidget(widget) (isMozilla() && isFixedWidget(widget))

#define drawAreaColor(cr, area, col, x, y, width, height) \
        drawAreaColorAlpha(cr, area, col, x, y, width, height, 1.0)

static void drawAreaColorAlpha(cairo_t *cr, GdkRectangle *area, GdkColor *col, gint x, gint y, gint width, gint height, double alpha)
{
    setCairoClipping(cr, area);
    cairo_rectangle(cr, x, y, width, height);
    cairo_set_source_rgba(cr, CAIRO_COL(*col), alpha);
    cairo_fill(cr);
    unsetCairoClipping(cr);
}

static GdkColor * getParentBgCol(GtkWidget *widget)
{
    if(GTK_IS_SCROLLBAR(widget))
        widget=qtcWidgetGetParent(widget);

    if(widget)
    {
        widget=qtcWidgetGetParent(widget);
        while(widget && GTK_IS_BOX(widget))
            widget=qtcWidgetGetParent(widget);
    }

    GtkStyle *style=widget ? qtcWidgetGetStyle(widget) : NULL;
    return style
               ? &(style->bg[qtcWidgetGetState(widget)])
               : NULL;
}

static int getOpacity(GtkWidget *widget)
{
    if(opts.bgndOpacity==opts.dlgOpacity)
        return opts.bgndOpacity;

    if(opts.bgndOpacity!=100 || opts.dlgOpacity!=100)
    {
        if(!widget)
            return opts.bgndOpacity;
        else
        {
            GtkWidget *top=gtk_widget_get_toplevel(widget);
            return top && GTK_IS_DIALOG(top) ? opts.dlgOpacity : opts.bgndOpacity;
        }
    }
    return 100;
}

static gboolean eqRect(GdkRectangle *a, GdkRectangle *b)
{
    return a->x==b->x && a->y==b->y && a->width==b->width && a->height==b->height;
}

static void setLowerEtchCol(cairo_t *cr, GtkWidget *widget)
{
    if(USE_CUSTOM_ALPHAS(opts))
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, opts.customAlphas[ALPHA_ETCH_LIGHT]);
    else if(IS_FLAT_BGND(opts.bgndAppearance) && (!widget || !g_object_get_data(G_OBJECT (widget), "transparent-bg-hint")))
    {
        GdkColor *parentBg=getParentBgCol(widget);

        if(parentBg)
        {
            GdkColor col;

            shade(&opts, parentBg, &col, 1.06);
            cairo_set_source_rgb(cr, CAIRO_COL(col));
        }
        else
            cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.1); // 0.25);
    }
    else
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.1); // 0.4);
}

static void drawBgnd(cairo_t *cr, GdkColor *col, GtkWidget *widget, GdkRectangle *area, int x, int y, int width, int height)
{
    GdkColor *parent_col=getParentBgCol(widget),
             *bgnd_col=parent_col ? parent_col : col;

    drawAreaColor(cr, area, parent_col ? parent_col : col, x, y, width, height);
}

static GdkColor shadeColor(GdkColor *orig, double mod)
{
    if(!equal(mod, 0.0))
    {
        GdkColor modified;
        shade(&opts, orig, &modified, mod);
        return modified;
    }
    return *orig;
}

static void drawAreaModColor(cairo_t *cr, GdkRectangle *area, GdkColor *orig, double mod, gint x, gint y, gint width, gint height)
{
    GdkColor modified=shadeColor(orig, mod);

    drawAreaColor(cr, area, &modified, x, y, width, height);
}

#define drawAreaMod(/*cairo_t *       */  cr, \
                    /* GtkStyle *     */  style, \
                    /* GtkStateType   */  state, \
                    /* GdkRectangle * */  area, \
                    /* double         */  mod, \
                    /* gint           */  x, \
                    /* gint           */  y, \
                    /* gint           */  width, \
                    /* gint           */  height) \
    drawAreaModColor(cr, area, &style->bg[state], mod, x, y, width, height)

static void constrainRect(QtcRect *rect, QtcRect *con)
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

static gboolean windowEvent(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    if(GDK_FOCUS_CHANGE==event->type)
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

static GdkPixbuf * pixbufCacheValueNew(QtCPixKey *key)
{
    GdkPixbuf *res=NULL;

    switch(key->pix)
    {
        case PIX_CHECK:
            res=gdk_pixbuf_new_from_inline(-1, opts.xCheck ? check_x_on :check_on, TRUE, NULL);
            break;
        case PIX_BLANK:
            return gdk_pixbuf_new_from_inline(-1, blank16x16, TRUE, NULL);
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

static void adjustToolbarButtons(GtkWidget *widget, int *x, int *y, int *width, int *height, int *round, gboolean horiz)
{
    GtkToolbar  *toolbar=NULL;
    GtkToolItem *toolitem=NULL;
    GtkWidget   *w=widget;
    int         i;
    
    for(i=0; i<5 && w && (NULL==toolbar || NULL==toolitem); ++i)
    {
        if(GTK_IS_TOOLBAR(w))
            toolbar=GTK_TOOLBAR(w);
        else if(GTK_IS_TOOL_ITEM(w))
            toolitem=GTK_TOOL_ITEM(w);
        w=qtcWidgetGetParent(w);
    }

    if(toolbar && toolitem)
    {
        int num=gtk_toolbar_get_n_items(toolbar);

        if(num>1)
        {
            int index=gtk_toolbar_get_item_index(toolbar, toolitem);
            GtkToolItem *prev=index ? gtk_toolbar_get_nth_item(toolbar, index-1) : NULL,
                        *next=index<(num-1) ? gtk_toolbar_get_nth_item(toolbar, index+1) : NULL;
            GtkWidget   *parent=NULL;
            gboolean    roundLeft=!prev || !GTK_IS_TOOL_BUTTON(prev),
                        roundRight=!next || !GTK_IS_TOOL_BUTTON(next),
                        isMenuButton=widget && GTK_IS_BUTTON(widget) &&
                                     (parent=qtcWidgetGetParent(widget)) && GTK_IS_BOX(parent) &&
                                     (parent=qtcWidgetGetParent(parent)) && GTK_IS_MENU_TOOL_BUTTON(parent),
                        isArrowButton=isMenuButton && GTK_IS_TOGGLE_BUTTON(widget);
            int         *pos=horiz ? x : y,
                        *size=horiz ? width : height;

            if(isArrowButton)
            {
                if(roundLeft && roundRight)
                    *round=horiz ? ROUNDED_RIGHT : ROUNDED_BOTTOM, *pos-=4, *size+=4;
                else if(roundLeft)
                    *round=ROUNDED_NONE, *pos-=4, *size+=8;
                else if(roundRight)
                    *round=horiz ? ROUNDED_RIGHT : ROUNDED_BOTTOM, *pos-=4, *size+=4;
                else
                    *round=ROUNDED_NONE, *pos-=4, *size+=8;
            }
            else if(isMenuButton)
            {
                if(roundLeft && roundRight)
                    *round=horiz ? ROUNDED_LEFT : ROUNDED_TOP, *size+=4;
                else if(roundLeft)
                    *round=horiz ? ROUNDED_LEFT : ROUNDED_TOP, *size+=4;
                else if(roundRight)
                    *round=ROUNDED_NONE, *pos-=4, *size+=8;
                else
                    *round=ROUNDED_NONE, *pos-=4, *size+=8;
            }
            else if(roundLeft && roundRight)
                ;
            else if(roundLeft)
                *round=horiz ? ROUNDED_LEFT : ROUNDED_TOP, *size+=4;
            else if(roundRight)
                *round=horiz ? ROUNDED_RIGHT : ROUNDED_BOTTOM, *pos-=4, *size+=4;
            else
                *round=ROUNDED_NONE, *pos-=4, *size+=8;
        }
    }
}

#if GTK_CHECK_VERSION(2, 90, 0)
#define sanitizeSize(A, B, C) sanitizeSizeReal(widget, B, C)
static void sanitizeSizeReal(GtkWidget *widget, gint *width, gint *height)
{
    if(-1==*width || -1==*height)
    {
        GdkWindow *window=gtk_widget_get_window(widget);

        if(window)
        {
            if((-1==*width) && (-1==*height))
                gdk_window_get_size(window, width, height);
            else if(-1==*width)
                gdk_window_get_size(window, width, NULL);
            else if(-1==*height)
                gdk_window_get_size(window, NULL, height);
        }
    }
}
#else
static void sanitizeSize(GdkWindow *window, gint *width, gint *height)
{
    if((-1==*width) && (-1==*height))
        gdk_window_get_size(window, width, height);
    else if(-1==*width)
        gdk_window_get_size(window, width, NULL);
    else if(-1==*height)
        gdk_window_get_size(window, NULL, height);
}
#endif

#define drawBevelGradient(cr, style, area, x, y, width, height, base, horiz, sel, bevApp, w) \
        drawBevelGradientAlpha(cr, style, area, x, y, width, height, base, horiz, sel, bevApp, w, 1.0)
static void drawBevelGradientAlpha(cairo_t *cr, GtkStyle *style, GdkRectangle *area, int x, int y,
                                   int width, int height, GdkColor *base, gboolean horiz, gboolean sel,
                                   EAppearance bevApp, EWidget w, double alpha)
{
    EAppearance app=APPEARANCE_BEVELLED!=bevApp || WIDGET_BUTTON(w) || WIDGET_LISTVIEW_HEADER==w
                        ? bevApp
                        : APPEARANCE_GRADIENT;

    if(IS_FLAT(bevApp))
    {
        if((WIDGET_TAB_TOP!=w && WIDGET_TAB_BOT!=w) || !CUSTOM_BGND || opts.tabBgnd || !sel)
            drawAreaColorAlpha(cr, area, base, x, y, width, height, alpha);
    }
    else
    {
        cairo_pattern_t *pt=cairo_pattern_create_linear(x, y, horiz ? x : x+width-1, horiz ? y+height-1 : y);
        gboolean        topTab=WIDGET_TAB_TOP==w,
                        botTab=WIDGET_TAB_BOT==w,
                        selected=(topTab || botTab) ? false : sel;
        EAppearance     app=selected
                                ? opts.sunkenAppearance
                                : WIDGET_LISTVIEW_HEADER==w && APPEARANCE_BEVELLED==bevApp
                                    ? APPEARANCE_LV_BEVELLED
                                    : APPEARANCE_BEVELLED!=bevApp || WIDGET_BUTTON(w) || WIDGET_LISTVIEW_HEADER==w
                                        ? bevApp
                                        : APPEARANCE_GRADIENT;
        const Gradient  *grad=getGradient(app, &opts);
        int             i=0;

        setCairoClipping(cr, area);

        for(i=0; i<grad->numStops; ++i)
        {
            GdkColor col;
            
            if(/*sel && */(topTab || botTab) && i==grad->numStops-1)
            {
                if(sel /*&& CUSTOM_BGND*/ && 0==opts.tabBgnd && !isMozilla())
                    alpha=0.0;
                col=*base;
            }
            else
            {
                double val=botTab && opts.invertBotTab ? INVERT_SHADE(grad->stops[i].val) : grad->stops[i].val;
                shade(&opts, base, &col, botTab && opts.invertBotTab ? MAX(val, 0.9) : val);
            }

#ifdef QTC_CAIRO_1_10_HACK
            if(0==i || i==(grad->numStops-1))
                cairo_pattern_add_color_stop_rgba(pt, botTab ? 1.0-grad->stops[i].pos : grad->stops[i].pos,
                                                  CAIRO_COL(col), alpha*grad->stops[i].alpha);
#endif
            cairo_pattern_add_color_stop_rgba(pt, botTab ? 1.0-grad->stops[i].pos : grad->stops[i].pos,
                                              CAIRO_COL(col), alpha*grad->stops[i].alpha);
        }

        if(APPEARANCE_AGUA==app && !(topTab || botTab) && (horiz ? height : width)>AGUA_MAX)
        {
            GdkColor col;
            double pos=AGUA_MAX/((horiz ? height : width)*2.0);
            shade(&opts, base, &col, AGUA_MID_SHADE);
            cairo_pattern_add_color_stop_rgba(pt, pos, CAIRO_COL(col), alpha); /* *grad->stops[i].alpha); */
#ifdef QTC_CAIRO_1_10_HACK
            cairo_pattern_add_color_stop_rgba(pt, pos, CAIRO_COL(col), alpha); /* *grad->stops[i].alpha); */
            cairo_pattern_add_color_stop_rgba(pt, 1.0-pos, CAIRO_COL(col), alpha); /* *grad->stops[i].alpha); */
#endif
            cairo_pattern_add_color_stop_rgba(pt, 1.0-pos, CAIRO_COL(col), alpha); /* *grad->stops[i].alpha); */
        }

        cairo_set_source(cr, pt);
        cairo_rectangle(cr, x, y, width, height);
        cairo_fill(cr);
        cairo_pattern_destroy(pt);
        unsetCairoClipping(cr);
    }
}

#if 0
static gboolean drawBackgroundPng(const char *png)
{
    int             size=512;
    cairo_surface_t *surface=cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 512, 512);
    gboolean        rv=FALSE;
    
    if(surface)
    {
        cairo_t *cr=cairo_create(surface);
        
        if(cr)
        {
            drawBevelGradientAlpha(cr, NULL, NULL, 0, 0, size, size, &(qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW]),
                                   GT_HORIZ==opts.bgndGrad, FALSE, opts.bgndAppearance, WIDGET_OTHER, 1.0);
            rv=CAIRO_STATUS_SUCCESS==cairo_surface_write_to_png(surface, png);
            cairo_destroy(cr);
        }
    }
    
    return rv;
}
#endif

typedef enum
{
    DF_DRAW_INSIDE     = 0x001,
    DF_BLEND           = 0x002,
    DF_DO_CORNERS      = 0x004,
    DF_SUNKEN          = 0x008,
    DF_DO_BORDER       = 0x010,
    DF_VERT            = 0x020,
    DF_HIDE_EFFECT     = 0x040
} EDrawFlags;

#define drawBorder(a, b, c, d, e, f, g, h, i, j, k, l, m) \
realDrawBorder(a, b, c, d, e, f, g, h, i, j, k, l, m, STD_BORDER)

static void plotPoints(cairo_t *cr, GdkPoint *pts, int count)
{
    int i;
    cairo_move_to(cr, pts[0].x+0.5, pts[0].y+0.5);
    for(i=1; i<count; ++i)
        cairo_line_to(cr, pts[i].x+0.5, pts[i].y+0.5);
}

static void createTLPath(cairo_t *cr, double xd, double yd, double width, double height, double radius, int round)
{
    gboolean rounded=radius>0.0;

    if (rounded && round&CORNER_BL)
        cairo_arc(cr, xd+radius, yd+height-radius, radius, M_PI * 0.75, M_PI);
    else
        cairo_move_to(cr, xd, yd+height);

    if (rounded && round&CORNER_TL)
        cairo_arc(cr, xd+radius, yd+radius, radius, M_PI, M_PI * 1.5);
    else
        cairo_line_to(cr, xd, yd);

    if (rounded && round&CORNER_TR)
        cairo_arc(cr, xd+width-radius, yd+radius, radius, M_PI * 1.5, M_PI * 1.75);
    else
        cairo_line_to(cr, xd+width, yd);
}

static void createBRPath(cairo_t *cr, double xd, double yd, double width, double height,
                         double radius, int round)
{
    gboolean rounded=radius>0.0;

    if (rounded && round&CORNER_TR)
        cairo_arc(cr, xd+width-radius, yd+radius, radius, M_PI * 1.75, 0);
    else
        cairo_move_to(cr, xd+width, yd);

    if (rounded && round&CORNER_BR)
        cairo_arc(cr, xd+width-radius, yd+height-radius, radius, 0, M_PI * 0.5);
    else
        cairo_line_to(cr, xd+width, yd+height);

    if (rounded && round&CORNER_BL)
        cairo_arc(cr, xd+radius, yd+height-radius, radius, M_PI * 0.5, M_PI * 0.75);
    else
        cairo_line_to(cr, xd, yd+height);
}

static void createPath(cairo_t *cr, double xd, double yd, double width, double height,
                       double radius, int round)
{
    gboolean rounded=radius>0.0;

    if (rounded && round&CORNER_TL)
        cairo_move_to(cr, xd+radius, yd);
    else
        cairo_move_to(cr, xd, yd);

    if (rounded && round&CORNER_TR)
        cairo_arc(cr, xd+width-radius, yd+radius, radius, M_PI * 1.5, M_PI * 2);
    else
        cairo_line_to(cr, xd+width, yd);

    if (rounded && round&CORNER_BR)
        cairo_arc(cr, xd+width-radius, yd+height-radius, radius, 0, M_PI * 0.5);
    else
        cairo_line_to(cr, xd+width, yd+height);

    if (rounded && round&CORNER_BL)
        cairo_arc(cr, xd+radius, yd+height-radius, radius, M_PI * 0.5, M_PI);
    else
        cairo_line_to(cr, xd, yd+height);

    if (rounded && round&CORNER_TL)
        cairo_arc(cr, xd+radius, yd+radius, radius, M_PI, M_PI * 1.5);
    else
        cairo_line_to(cr, xd, yd);
}

static void realDrawBorder(cairo_t *cr, GtkStyle *style, GtkStateType state, GdkRectangle *area,
                           gint x, gint y, gint width, gint height, GdkColor *c_colors, int round, EBorder borderProfile,
                           EWidget widget, int flags, int borderVal)
{
    if(ROUND_NONE==opts.round && WIDGET_RADIO_BUTTON!=widget)
        round=ROUNDED_NONE;

    {
    double       radius=getRadius(&opts, width, height, widget, RADIUS_EXTERNAL),
                 xd=x+0.5,
                 yd=y+0.5;
    EAppearance  app=widgetApp(widget, &opts);
    gboolean     enabled=GTK_STATE_INSENSITIVE!=state,
                 useText=GTK_STATE_INSENSITIVE!=state && WIDGET_DEF_BUTTON==widget && IND_FONT_COLOR==opts.defBtnIndicator && enabled,
                 hasFocus=enabled && qtcPalette.focus && c_colors==qtcPalette.focus, /* CPD USED TO INDICATE FOCUS! */
                 hasMouseOver=enabled && qtcPalette.mouseover && c_colors==qtcPalette.mouseover && ENTRY_MO;
    GdkColor     *colors=c_colors ? c_colors : qtcPalette.background;
    int          useBorderVal=!enabled && WIDGET_BUTTON(widget)
                                ? DISABLED_BORDER
                                : qtcPalette.mouseover==colors && IS_SLIDER(widget)
                                    ? SLIDER_MO_BORDER_VAL
                                    : borderVal;
    GdkColor     *border_col= useText ? &style->text[GTK_STATE_NORMAL] : &colors[useBorderVal];

    width--;
    height--;

    setCairoClipping(cr, area);

    if(WIDGET_TAB_BOT==widget || WIDGET_TAB_TOP==widget)
        colors=qtcPalette.background;

    if(!(opts.thin&THIN_FRAMES))
        switch(borderProfile)
        {
            case BORDER_FLAT:
                break;
            case BORDER_RAISED:
            case BORDER_SUNKEN:
            case BORDER_LIGHT:
            {
                double radiusi=getRadius(&opts, width-2, height-2, widget, RADIUS_INTERNAL),
                    xdi=xd+1,
                    ydi=yd+1,
                    alpha=(hasMouseOver || hasFocus) && (WIDGET_ENTRY==widget || WIDGET_SPIN==widget || WIDGET_COMBO_BUTTON==widget)
                                ? ENTRY_INNER_ALPHA : BORDER_BLEND_ALPHA(widget);
                int    widthi=width-2,
                    heighti=height-2;

                if((GTK_STATE_INSENSITIVE!=state || BORDER_SUNKEN==borderProfile) /*&&
                (BORDER_RAISED==borderProfile || BORDER_LIGHT==borderProfile || APPEARANCE_FLAT!=app)*/)
                {
                    GdkColor *col=&colors[BORDER_RAISED==borderProfile || BORDER_LIGHT==borderProfile
                                                ? 0 : FRAME_DARK_SHADOW];
                    if(flags&DF_BLEND)
                    {
                        if(WIDGET_SPIN==widget || WIDGET_COMBO_BUTTON==widget || WIDGET_SCROLLVIEW==widget)
                        {
                            cairo_set_source_rgb(cr, CAIRO_COL(style->base[state]));
                            createTLPath(cr, xdi, ydi, widthi, heighti, radiusi, round);
                            cairo_stroke(cr);
                        }

                        cairo_set_source_rgba(cr, CAIRO_COL(*col), alpha);
                    }
                    else
                        cairo_set_source_rgb(cr, CAIRO_COL(*col));
                }
                else
                    cairo_set_source_rgb(cr, CAIRO_COL(style->bg[state]));
                
                createTLPath(cr, xdi, ydi, widthi, heighti, radiusi, round);
                cairo_stroke(cr);
                if(WIDGET_CHECKBOX!=widget)
                {
                    if(!hasFocus && !hasMouseOver && BORDER_LIGHT!=borderProfile)
                        if(WIDGET_SCROLLVIEW==widget)
                        {
                            /* Because of list view headers, need to draw dark line on right! */
                            cairo_save(cr);
                            cairo_set_source_rgb(cr, CAIRO_COL(style->base[state]));
                            createBRPath(cr, xdi, ydi, widthi, heighti, radiusi, round);
                            cairo_stroke(cr);
                            cairo_restore(cr);
                        }
                        else if(WIDGET_SCROLLVIEW==widget || WIDGET_ENTRY==widget)
                            cairo_set_source_rgb(cr, CAIRO_COL(style->base[state]));
                        else if(GTK_STATE_INSENSITIVE!=state && (BORDER_SUNKEN==borderProfile || /*APPEARANCE_FLAT!=app ||*/
                                                                WIDGET_TAB_TOP==widget || WIDGET_TAB_BOT==widget))
                        {
                            GdkColor *col=&colors[BORDER_RAISED==borderProfile ? FRAME_DARK_SHADOW : 0];
                            if(flags&DF_BLEND)
                                cairo_set_source_rgba(cr, CAIRO_COL(*col), BORDER_SUNKEN==borderProfile ? 0.0 : alpha);
                            else
                                cairo_set_source_rgb(cr, CAIRO_COL(*col));
                        }
                        else
                            cairo_set_source_rgb(cr, CAIRO_COL(style->bg[state]));

                    createBRPath(cr, xdi, ydi, widthi, heighti, radiusi, round);
                    cairo_stroke(cr);
                }
            }
        }

    if(BORDER_SUNKEN==borderProfile &&
       (WIDGET_FRAME==widget || ((WIDGET_ENTRY==widget || WIDGET_SCROLLVIEW==widget) &&
                                 !opts.etchEntry && !hasFocus && !hasMouseOver)))
    {
        cairo_set_source_rgba(cr, CAIRO_COL(*border_col), /*enabled ? */1.0/* : LOWER_BORDER_ALPHA*/);
        createTLPath(cr, xd, yd, width, height, radius, round);
        cairo_stroke(cr);
        cairo_set_source_rgba(cr, CAIRO_COL(*border_col), LOWER_BORDER_ALPHA);
        createBRPath(cr, xd, yd, width, height, radius, round);
        cairo_stroke(cr);
    }
    else
    {
        cairo_set_source_rgb(cr, CAIRO_COL(*border_col));
        createPath(cr, xd, yd, width, height, radius, round);
        cairo_stroke(cr);
    }
    unsetCairoClipping(cr);
    }
}

#define drawGlow(cr, area, x, y, w, h, round, widget) \
        drawGlowReal(cr, area, x, y, w, h, round, widget, NULL)
static void drawGlowReal(cairo_t *cr, GdkRectangle *area, int x, int y, int w, int h, int round, EWidget widget, const GdkColor *colors)
{
    if(qtcPalette.mouseover || qtcPalette.defbtn || colors)
    {
        double   xd=x+0.5,
                 yd=y+0.5,
                 radius=getRadius(&opts, w, h, widget, RADIUS_ETCH);
        gboolean def=WIDGET_DEF_BUTTON==widget && IND_GLOW==opts.defBtnIndicator,
                 defShade=def && (!qtcPalette.defbtn ||
                                  (qtcPalette.mouseover &&
                                   EQUAL_COLOR(qtcPalette.defbtn[ORIGINAL_SHADE], qtcPalette.mouseover[ORIGINAL_SHADE])));
        const GdkColor *col=colors
                        ? &colors[GLOW_MO]
                        : (def && qtcPalette.defbtn) || !qtcPalette.mouseover
                            ? &qtcPalette.defbtn[GLOW_DEFBTN] : &qtcPalette.mouseover[GLOW_MO];

        setCairoClipping(cr, area);
        cairo_set_source_rgba(cr, CAIRO_COL(*col), GLOW_ALPHA(defShade));
        createPath(cr, xd, yd, w-1, h-1, radius, round);
        cairo_stroke(cr);
        unsetCairoClipping(cr);
    }
}

static void drawEtch(cairo_t *cr, GdkRectangle *area, GtkWidget *widget, int x, int y, int w, int h, gboolean raised,
                     int round, EWidget wid)
{
    double       xd=x+0.5,
                 yd=y+0.5,
                 radius=getRadius(&opts, w, h, wid, RADIUS_ETCH);
    GdkRectangle *a=area,
                 b;

    if(WIDGET_COMBO_BUTTON==wid && GTK_APP_OPEN_OFFICE==qtSettings.app && widget && isFixedWidget(qtcWidgetGetParent(widget)))
    {
        b.x=x+2; b.y=y; b.width=w-4; b.height=h;
        a=&b;
    }
        
    setCairoClipping(cr, a);

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, USE_CUSTOM_ALPHAS(opts) ? opts.customAlphas[ALPHA_ETCH_DARK] : ETCH_TOP_ALPHA);
    if(!raised && WIDGET_SLIDER!=wid)
    {
        createTLPath(cr, xd, yd, w-1, h-1, radius, round);
        cairo_stroke(cr);
        if(WIDGET_SLIDER_TROUGH==wid && opts.thinSbarGroove && widget && GTK_IS_SCROLLBAR(widget))
            cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, USE_CUSTOM_ALPHAS(opts) ? opts.customAlphas[ALPHA_ETCH_LIGHT] : ETCH_BOTTOM_ALPHA);
        else
            setLowerEtchCol(cr, widget);
    }

    createBRPath(cr, xd, yd, w-1, h-1, radius, round);
    cairo_stroke(cr);
    unsetCairoClipping(cr);
}

static void clipPathRadius(cairo_t *cr, double x, double y, int w, int h, double radius, int round)
{
    cairo_new_path(cr);
    cairo_save(cr);
    createPath(cr, x, y, w, h, radius, round);
    cairo_clip(cr);
}

static void clipPath(cairo_t *cr, int x, int y, int w, int h, EWidget widget, int rad, int round)
{
    clipPathRadius(cr, x+0.5, y+0.5, w-1, h-1, getRadius(&opts, w, h, widget, rad), round);
}

static void addStripes(cairo_t *cr, int x, int y, int w, int h, bool horizontal)
{
    int endx=horizontal ? STRIPE_WIDTH :0,
        endy=horizontal ? 0 : STRIPE_WIDTH;

    cairo_pattern_t *pat=cairo_pattern_create_linear(x, y, x+endx, y+endy);

    cairo_pattern_add_color_stop_rgba(pat, 0.0, 1.0, 1.0, 1.0, 0.0);
#ifdef QTC_CAIRO_1_10_HACK    
    cairo_pattern_add_color_stop_rgba(pat, 0.0, 1.0, 1.0, 1.0, 0.0);
    cairo_pattern_add_color_stop_rgba(pat, 1.0, 1.0, 1.0, 1.0, 0.15);
#endif
    cairo_pattern_add_color_stop_rgba(pat, 1.0, 1.0, 1.0, 1.0, 0.15);
    cairo_pattern_set_extend(pat, CAIRO_EXTEND_REFLECT);
    cairo_set_source(cr, pat);
    cairo_rectangle(cr, x, y, w, h);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);
}

static void drawLightBevel(cairo_t *cr, GtkStyle *style, GtkStateType state, GdkRectangle *area, gint x, gint y,
                           gint width, gint height, GdkColor *base, GdkColor *colors, int round, EWidget widget,
                           EBorder borderProfile, int flags, GtkWidget *wid)
{
    EAppearance app=widgetApp(widget, &opts);
    gboolean    sunken=flags&DF_SUNKEN,
                doColouredMouseOver=opts.coloredMouseOver && qtcPalette.mouseover &&
                                  WIDGET_SPIN!=widget && WIDGET_SPIN_DOWN!=widget && WIDGET_SPIN_UP!=widget &&
                                  WIDGET_COMBO_BUTTON!=widget && WIDGET_SB_BUTTON!=widget &&
                                  (!SLIDER(widget) || !opts.colorSliderMouseOver) &&
                                  WIDGET_UNCOLOURED_MO_BUTTON!=widget &&
                                  GTK_STATE_PRELIGHT==state &&
                                  (!sunken || IS_TOGGLE_BUTTON(widget) || (WIDGET_TOOLBAR_BUTTON==widget && opts.coloredTbarMo)),
                plastikMouseOver=doColouredMouseOver && MO_PLASTIK==opts.coloredMouseOver,
                colouredMouseOver=doColouredMouseOver &&
                                  (MO_COLORED==opts.coloredMouseOver || MO_COLORED_THICK==opts.coloredMouseOver),
                flatWidget=WIDGET_PROGRESSBAR==widget && !opts.borderProgress,
                lightBorder=!flatWidget && DRAW_LIGHT_BORDER(sunken, widget, app),
                draw3dfull=!flatWidget && !lightBorder && DRAW_3D_FULL_BORDER(sunken, app),
                draw3d=!flatWidget && (draw3dfull || (!lightBorder && DRAW_3D_BORDER(sunken, app))),
                drawShine=DRAW_SHINE(sunken, app),
                bevelledButton=WIDGET_BUTTON(widget) && APPEARANCE_BEVELLED==app,
                doEtch=flags&DF_DO_BORDER && (ETCH_WIDGET(widget) || (WIDGET_COMBO_BUTTON==widget && opts.etchEntry)) && DO_EFFECT,
                glowFocus=doEtch && USE_GLOW_FOCUS(GTK_STATE_PRELIGHT==state) && wid && qtcWidgetHasFocus(wid) &&
                          GTK_STATE_INSENSITIVE!=state && !isComboBoxEntryButton(wid) &&
                          ((WIDGET_RADIO_BUTTON!=widget && WIDGET_CHECKBOX!=widget) || GTK_STATE_ACTIVE!=state),
                glowFocusSunkenToggle=sunken && (glowFocus || (doColouredMouseOver && MO_GLOW==opts.coloredMouseOver)) &&
                                      wid && GTK_IS_TOGGLE_BUTTON(wid),
                horiz=!(flags&DF_VERT);
    int         xe=x, ye=y, we=width, he=height, origWidth=width, origHeight=height;
    double      xd=x+0.5, yd=y+0.5;

    if(CIRCULAR_SLIDER(widget))
        horiz=true;

    if(WIDGET_TROUGH==widget && !opts.borderSbarGroove && flags&DF_DO_BORDER)
        flags-=DF_DO_BORDER;

    if(WIDGET_COMBO_BUTTON==widget && doEtch)
        if(ROUNDED_RIGHT==round)
            x--, xd-=1, width++;
        else if(ROUNDED_LEFT==round)
            width++;

    if(doEtch)
        xd+=1, x++, yd+=1, y++, width-=2, height-=2, xe=x, ye=y, we=width, he=height;

    if(width>0 && height>0)
    {
        if(!(flags&DF_DO_BORDER))
            clipPathRadius(cr, x, y, width, height, getRadius(&opts, width, height, widget, RADIUS_EXTERNAL), round);
        else
            clipPath(cr, x, y, width, height, widget, RADIUS_EXTERNAL, round);

        drawBevelGradient(cr, style, area, x, y, width, height, base, horiz, sunken && !IS_TROUGH(widget), app, widget);

        if(plastikMouseOver)
        {
            if(SLIDER(widget))
            {
                int len=SB_SLIDER_MO_LEN(horiz ? width : height),
                    so=lightBorder ? SLIDER_MO_PLASTIK_BORDER : 1,
                    eo=len+so,
                    col=SLIDER_MO_SHADE;

                if(horiz)
                {
                    drawBevelGradient(cr, style, area, x+so, y, len, height, &qtcPalette.mouseover[col],
                                      horiz, sunken, app, widget);
                    drawBevelGradient(cr, style, area, x+width-eo, y, len, height, &qtcPalette.mouseover[col],
                                      horiz, sunken, app, widget);
                }
                else
                {
                    drawBevelGradient(cr, style, area, x, y+so, width, len, &qtcPalette.mouseover[col],
                                      horiz, sunken, app, widget);
                    drawBevelGradient(cr, style, area, x, y+height-eo, width, len, &qtcPalette.mouseover[col],
                                      horiz, sunken, app, widget);
                }
            }
            else
            {
                int      mh=height;
                GdkColor *col=&qtcPalette.mouseover[MO_PLASTIK_DARK(widget)];
                bool     horizontal=(horiz && !(WIDGET_SB_BUTTON==widget || SLIDER(widget)))||
                                    (!horiz && (WIDGET_SB_BUTTON==widget || SLIDER(widget))),
                         thin=WIDGET_SB_BUTTON==widget || WIDGET_SPIN_UP==widget || WIDGET_SPIN_DOWN==widget ||
                              ((horiz ? height : width)<16);

                if(EFFECT_NONE!=opts.buttonEffect && WIDGET_SPIN_UP==widget && horiz)
                    mh--;
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, CAIRO_COL(*col));
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
                    col=&qtcPalette.mouseover[MO_PLASTIK_LIGHT(widget)];
                    cairo_new_path(cr);
                    cairo_set_source_rgb(cr, CAIRO_COL(*col));
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
        
        if(drawShine)
        {
            gboolean mo=GTK_STATE_PRELIGHT==state && opts.highlightFactor;
            int      xa=x, ya=y, wa=width, ha=height;

            if(WIDGET_RADIO_BUTTON==widget || CIRCULAR_SLIDER(widget))
            {
                double          topSize=(ha*0.4),
                                topWidthAdjust=3.5,
                                topGradRectX=xa+topWidthAdjust,
                                topGradRectY=ya,
                                topGradRectW=wa-(topWidthAdjust*2)-1,
                                topGradRectH=topSize-1;
                cairo_pattern_t *pt=cairo_pattern_create_linear(topGradRectX, topGradRectY, topGradRectX, topGradRectY+topGradRectH);

                clipPathRadius(cr, topGradRectX+0.5, topGradRectY+0.5, topGradRectW, topGradRectH, topGradRectW/2.0, ROUNDED_ALL);

                cairo_pattern_add_color_stop_rgba(pt, 0.0, 1.0, 1.0, 1.0, mo ? (opts.highlightFactor>0 ? 0.8 : 0.7) : 0.75);
#ifdef QTC_CAIRO_1_10_HACK
                cairo_pattern_add_color_stop_rgba(pt, 0.0, 1.0, 1.0, 1.0, mo ? (opts.highlightFactor>0 ? 0.8 : 0.7) : 0.75);
                cairo_pattern_add_color_stop_rgba(pt, 1.0, 1.0, 1.0, 1.0, /*mo ? (opts.highlightFactor>0 ? 0.3 : 0.1) :*/ 0.2);
#endif
                cairo_pattern_add_color_stop_rgba(pt, 1.0, 1.0, 1.0, 1.0, /*mo ? (opts.highlightFactor>0 ? 0.3 : 0.1) :*/ 0.2);

                cairo_set_source(cr, pt);
                cairo_rectangle(cr, topGradRectX, topGradRectY, topGradRectW, topGradRectH);
                cairo_fill(cr);
                cairo_pattern_destroy(pt);
                cairo_restore(cr);
            }
            else
            {
                double          size=(MIN((horiz ? ha : wa)/2.0, 16)),
                                rad=size/2.0;
                cairo_pattern_t *pt=NULL;
                int             mod=4;

                if(horiz)
                {
                    if(!(ROUNDED_LEFT&round))
                        xa-=8, wa+=8;
                    if(!(ROUNDED_RIGHT&round))
                        wa+=8;
                }
                else
                {
                    if(!(ROUNDED_TOP&round))
                        ya-=8, ha+=8;
                    if(!(ROUNDED_BOTTOM&round))
                        ha+=8;
                }
                pt=cairo_pattern_create_linear(xa, ya, xa+(horiz ? 0.0 : size), ya+(horiz ? size : 0.0));

                if(getWidgetRound(&opts, origWidth, origHeight, widget)<ROUND_MAX ||
                (!IS_MAX_ROUND_WIDGET(widget) && !IS_SLIDER(widget)))
                {
                    rad/=2.0;
                    mod=mod>>1;
                }

                if(horiz)
                    clipPathRadius(cr, xa+mod+0.5, ya+0.5, wa-(mod*2)-1, size-1, rad, round);
                else
                    clipPathRadius(cr, xa+0.5, ya+mod+0.5, size-1, ha-(mod*2)-1, rad, round);

                cairo_pattern_add_color_stop_rgba(pt, 0.0, 1.0, 1.0, 1.0, mo ? (opts.highlightFactor>0 ? 0.95 : 0.85) : 0.9);
#ifdef QTC_CAIRO_1_10_HACK
                cairo_pattern_add_color_stop_rgba(pt, 0.0, 1.0, 1.0, 1.0, mo ? (opts.highlightFactor>0 ? 0.95 : 0.85) : 0.9);
                cairo_pattern_add_color_stop_rgba(pt, 1.0, 1.0, 1.0, 1.0, mo ? (opts.highlightFactor>0 ? 0.3 : 0.1) : 0.2);
#endif
                cairo_pattern_add_color_stop_rgba(pt, 1.0, 1.0, 1.0, 1.0, mo ? (opts.highlightFactor>0 ? 0.3 : 0.1) : 0.2);

                cairo_set_source(cr, pt);
                cairo_rectangle(cr, xa, ya, horiz ? wa : size, horiz ? size : ha);
                cairo_fill(cr);
                cairo_pattern_destroy(pt);
                cairo_restore(cr);
            }
        }
        
        cairo_restore(cr);
    }
    xd+=1, x++, yd+=1, y++, width-=2, height-=2;

    if(plastikMouseOver) // && !sunken)
    {
        bool         thin=WIDGET_SB_BUTTON==widget || WIDGET_SPIN==widget || ((horiz ? height : width)<16),
                     horizontal=SLIDER(widget) ? !horiz
                                                   : (horiz && WIDGET_SB_BUTTON!=widget) ||
                                                      (!horiz && WIDGET_SB_BUTTON==widget);
        int          len=SLIDER(widget) ? SB_SLIDER_MO_LEN(horiz ? width : height) : (thin ? 1 : 2);
        GdkRectangle rect;
        if(horizontal)
            rect.x=x, rect.y=y+len, rect.width=width, rect.height=height-(len*2);
        else
            rect.x=x+len, rect.y=y, rect.width=width-(len*2), rect.height=height;
        setCairoClipping(cr, &rect);
    }
    else
        setCairoClipping(cr, area);

    if(!colouredMouseOver && lightBorder)
    {
        GdkColor *col=&colors[LIGHT_BORDER(app)];

        cairo_new_path(cr);
        cairo_set_source_rgb(cr, CAIRO_COL(*col));
        createPath(cr, xd, yd, width-1, height-1, getRadius(&opts, width, height, widget, RADIUS_INTERNAL), round);
        cairo_stroke(cr);
    }
    else if(colouredMouseOver || (!sunken && (draw3d || flags&DF_DRAW_INSIDE)))
    {
        int      dark=/*bevelledButton ? */2/* : 4*/;
        GdkColor *col1=colouredMouseOver ? &qtcPalette.mouseover[MO_STD_LIGHT(widget, sunken)]
                                         : &colors[sunken ? dark : 0];

        cairo_new_path(cr);
        cairo_set_source_rgb(cr, CAIRO_COL(*col1));
        createTLPath(cr, xd, yd, width-1, height-1, getRadius(&opts, width, height, widget, RADIUS_INTERNAL), round);
        cairo_stroke(cr);
        if(colouredMouseOver || bevelledButton || draw3dfull)
        {
            GdkColor *col2=colouredMouseOver ? &qtcPalette.mouseover[MO_STD_DARK(widget)]
                                             : &colors[sunken ? 0 : dark];

            cairo_new_path(cr);
            cairo_set_source_rgb(cr, CAIRO_COL(*col2));
            createBRPath(cr, xd, yd, width-1, height-1, getRadius(&opts, width, height, widget, RADIUS_INTERNAL), round);
            cairo_stroke(cr);
        }
    }

    unsetCairoClipping(cr);

    if((doEtch || glowFocus) && !(flags&DF_HIDE_EFFECT))
        if((!sunken || glowFocusSunkenToggle) && GTK_STATE_INSENSITIVE!=state && !(opts.thin&THIN_FRAMES) &&
            ((WIDGET_OTHER!=widget && WIDGET_SLIDER_TROUGH!=widget && WIDGET_COMBO_BUTTON!=widget &&
             MO_GLOW==opts.coloredMouseOver && GTK_STATE_PRELIGHT==state) ||
             glowFocus ||
            (WIDGET_DEF_BUTTON==widget && IND_GLOW==opts.defBtnIndicator)))
            drawGlowReal(cr, area, xe-1, ye-1, we+2, he+2, round,
                         WIDGET_DEF_BUTTON==widget && GTK_STATE_PRELIGHT==state ? WIDGET_STD_BUTTON : widget,
                         glowFocus ? qtcPalette.focus : NULL);
        else
            drawEtch(cr, area, wid, xe-(WIDGET_COMBO_BUTTON==widget ? (ROUNDED_RIGHT==round ? 3 : 1) : 1), ye-1,
                                    we+(WIDGET_COMBO_BUTTON==widget ? (ROUNDED_RIGHT==round ? 4 : 5) : 2), he+2,
                    EFFECT_SHADOW==opts.buttonEffect && WIDGET_COMBO_BUTTON!=widget && WIDGET_BUTTON(widget) && !sunken,
                    round, widget);

    xd-=1, x--, yd-=1, y--, width+=2, height+=2;
    if(flags&DF_DO_BORDER && width>2 && height>2)
    {
        GdkColor *borderCols=glowFocus && (!sunken || glowFocusSunkenToggle)
                            ? qtcPalette.focus
                            : (WIDGET_COMBO==widget || WIDGET_COMBO_BUTTON==widget) && colors==qtcPalette.combobtn
                                ? GTK_STATE_PRELIGHT==state && MO_GLOW==opts.coloredMouseOver && !sunken
                                    ? qtcPalette.mouseover
                                    : qtcPalette.button[PAL_ACTIVE]
                                : colors;
                            
        cairo_new_path(cr);
        /* Yuck! this is a mess!!!! */
// Copied from KDE4 version...
        if(!sunken && GTK_STATE_INSENSITIVE!=state && !glowFocus &&
            ( ( ( (doEtch && WIDGET_OTHER!=widget && WIDGET_SLIDER_TROUGH!=widget) || SLIDER(widget) || WIDGET_COMBO==widget || WIDGET_MENU_BUTTON==widget ) &&
                 (MO_GLOW==opts.coloredMouseOver/* || MO_COLORED==opts.colorMenubarMouseOver*/) && GTK_STATE_PRELIGHT==state) ||
               (doEtch && WIDGET_DEF_BUTTON==widget && IND_GLOW==opts.defBtnIndicator)))

// Previous Gtk2...        
//         if(!sunken && (doEtch || SLIDER(widget)) &&
//             ( (WIDGET_OTHER!=widget && WIDGET_SLIDER_TROUGH!=widget && WIDGET_COMBO_BUTTON!=widget &&
//                 MO_GLOW==opts.coloredMouseOver && GTK_STATE_PRELIGHT==state) ||
//               (WIDGET_DEF_BUTTON==widget && IND_GLOW==opts.defBtnIndicator)))
            drawBorder(cr, style, state, area, x, y, width, height,
                       WIDGET_DEF_BUTTON==widget && IND_GLOW==opts.defBtnIndicator &&
                       (GTK_STATE_PRELIGHT!=state || !qtcPalette.mouseover)
                            ? qtcPalette.defbtn : qtcPalette.mouseover,
                       round, borderProfile, widget, flags);
        else
            drawBorder(cr, style, state, area, x, y, width, height,
                       colouredMouseOver && MO_COLORED_THICK==opts.coloredMouseOver ? qtcPalette.mouseover : borderCols,
                       round, borderProfile, widget, flags);
    }

    if(WIDGET_SB_SLIDER==widget && opts.stripedSbar)
    {
        clipPathRadius(cr, x, y, width, height, getRadius(&opts, width, height, WIDGET_SB_SLIDER, RADIUS_INTERNAL), round);
        addStripes(cr, x, y, width, height, horiz);
        cairo_restore(cr);
    }
}

#define drawFadedLine(cr, x, y, width, height, col, area, gap, fadeStart, fadeEnd, horiz) \
        drawFadedLineReal(cr, x, y, width, height, col, area, gap, fadeStart, fadeEnd, horiz, 1.0)
                          
static void drawFadedLineReal(cairo_t *cr, int x, int y, int width, int height, GdkColor *col, GdkRectangle *area, GdkRectangle *gap,
                              gboolean fadeStart, gboolean fadeEnd, gboolean horiz, double alpha)
{
    double          rx=x+0.5,
                    ry=y+0.5;
    cairo_pattern_t *pt=cairo_pattern_create_linear(rx, ry, horiz ? rx+(width-1) : rx+1, horiz ? ry+1 : ry+(height-1));

    if(gap)
    {
        QtcRect   r={x, y, width, height};
        QtcRegion *region=qtcRegionRect(area ? area : &r),
                  *inner=qtcRegionRect(gap);

        qtcRegionXor(region, inner);
        setCairoClippingRegion(cr, region);
        qtcRegionDestroy(inner);
        qtcRegionDestroy(region);
    }
    else
        setCairoClipping(cr, area);
    cairo_pattern_add_color_stop_rgba(pt, 0, CAIRO_COL(*col), fadeStart && opts.fadeLines ? 0.0 : alpha);
    cairo_pattern_add_color_stop_rgba(pt, FADE_SIZE, CAIRO_COL(*col), alpha);
#ifdef QTC_CAIRO_1_10_HACK
    cairo_pattern_add_color_stop_rgba(pt, FADE_SIZE, CAIRO_COL(*col), alpha);
    cairo_pattern_add_color_stop_rgba(pt, 1.0-FADE_SIZE, CAIRO_COL(*col), alpha);
#endif
    cairo_pattern_add_color_stop_rgba(pt, 1.0-FADE_SIZE, CAIRO_COL(*col), alpha);
    cairo_pattern_add_color_stop_rgba(pt, 1, CAIRO_COL(*col), fadeEnd && opts.fadeLines ? 0.0 : alpha);
    cairo_set_source(cr, pt);
    if(horiz)
    {
        cairo_move_to(cr, x, ry);
        cairo_line_to(cr, x+(width-1), ry);
    }
    else
    {
        cairo_move_to(cr, rx, y);
        cairo_line_to(cr, rx, y+(height-1));
    }
    cairo_stroke(cr);
    cairo_pattern_destroy(pt);
    unsetCairoClipping(cr);
}

static void drawHighlight(cairo_t *cr, int x, int y, int width, int height, GdkRectangle *area, gboolean horiz, gboolean inc)
{
    drawFadedLineReal(cr, x, y, width, height, &qtcPalette.mouseover[ORIGINAL_SHADE], area, NULL, true, true, horiz, inc ? 0.5 : 1.0);
    drawFadedLineReal(cr, x+(horiz ? 0 : 1), y+(horiz ? 1 : 0), width, height, &qtcPalette.mouseover[ORIGINAL_SHADE],
                      area, NULL, true, true, horiz, inc ? 1.0 : 0.5);
}

static void setLineCol(cairo_t *cr, cairo_pattern_t *pt, GdkColor *col)
{
    if(pt)
    {
        cairo_pattern_add_color_stop_rgba(pt, 0, CAIRO_COL(*col), 0.0);
        cairo_pattern_add_color_stop_rgba(pt, 0.4, CAIRO_COL(*col), 1.0);
#ifdef QTC_CAIRO_1_10_HACK
        cairo_pattern_add_color_stop_rgba(pt, 0.4, CAIRO_COL(*col), 1.0);
        cairo_pattern_add_color_stop_rgba(pt, 0.6, CAIRO_COL(*col), 1.0);
#endif
        cairo_pattern_add_color_stop_rgba(pt, 0.6, CAIRO_COL(*col), 1.0);
        cairo_pattern_add_color_stop_rgba(pt, 1, CAIRO_COL(*col), 0.0);
        cairo_set_source(cr, pt);
    }
    else
        cairo_set_source_rgb(cr,CAIRO_COL(*col));
}

static void drawLines(cairo_t *cr, double rx, double ry, int rwidth, int rheight, gboolean horiz,
                      int nLines, int offset, GdkColor *cols, GdkRectangle *area, int dark, ELine type)
{
    if(horiz)
        ry+=0.5,  rwidth+=1;
    else
        rx+=0.5,  rheight+=1;

    {
    int             space = (nLines*2)+(LINE_DASHES!=type ? (nLines-1) : 0),
                    step = LINE_DASHES!=type ? 3 : 2,
                    i,
                    etchedDisp = LINE_SUNKEN==type ? 1 : 0;
    double          x = (horiz ? rx : rx+((rwidth-space)>>1)),
                    y = (horiz ? ry+((rheight-space)>>1) : ry),
                    x2 = rx + rwidth-1,
                    y2 = ry + rheight-1;
    GdkColor        *col1 = &cols[dark],
                    *col2 = &cols[0];
    cairo_pattern_t *pt1=(opts.fadeLines && (horiz ? rwidth : rheight)>(16+etchedDisp))
                          ? cairo_pattern_create_linear(rx, ry, horiz ? x2 : rx+1, horiz ? ry+1 : y2)
                          : NULL,
                    *pt2=(pt1 && LINE_FLAT!=type)
                          ? cairo_pattern_create_linear(rx, ry, horiz ? x2 : rx+1, horiz ? ry+1 : y2)
                          : NULL;

    setCairoClipping(cr, area);
    setLineCol(cr, pt1, col1);

    if(horiz)
    {
        for(i=0; i<space; i+=step)
        {
            cairo_move_to(cr, x+offset, y+i);
            cairo_line_to(cr, x2-offset, y+i);
        }
        cairo_stroke(cr);

        if(LINE_FLAT!=type)
        {
            setLineCol(cr, pt2, col2);
            x+=etchedDisp;
            x2+=etchedDisp;
            for(i=1; i<space; i+=step)
            {
                cairo_move_to(cr, x+offset, y+i);
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
            cairo_line_to(cr, x+i, y2-offset);
        }
        cairo_stroke(cr);
        if(LINE_FLAT!=type)
        {
            setLineCol(cr, pt2, col2);
            y+=etchedDisp;
            y2+=etchedDisp;
            for(i=1; i<space; i+=step)
            {
                cairo_move_to(cr, x+i, y+offset);
                cairo_line_to(cr, x+i, y2-offset);
            }
            cairo_stroke(cr);
        }
    }
    if(pt1)
        cairo_pattern_destroy(pt1);
    if(pt2)
        cairo_pattern_destroy(pt2);
    unsetCairoClipping(cr);
    }
}


static void drawDot(cairo_t *cr, int x, int y, int w, int h, GdkColor *cols)
{
    double          dx=(x+((w-5)>>1))/*+0.5*/,
                    dy=(y+((h-5)>>1))/*+0.5*/;
    cairo_pattern_t *p1=cairo_pattern_create_linear(dx, dy, dx+4, dy+4),
                    *p2=cairo_pattern_create_linear(dx+2, dy+2, dx+4, dx+4);

    cairo_pattern_add_color_stop_rgba(p1, 0.0, CAIRO_COL(cols[STD_BORDER]), 1.0);
#ifdef QTC_CAIRO_1_10_HACK
    cairo_pattern_add_color_stop_rgba(p1, 0.0, CAIRO_COL(cols[STD_BORDER]), 1.0);
    cairo_pattern_add_color_stop_rgba(p1, 1.0, CAIRO_COL(cols[STD_BORDER]), 0.4);
#endif
    cairo_pattern_add_color_stop_rgba(p1, 1.0, CAIRO_COL(cols[STD_BORDER]), 0.4);

    cairo_pattern_add_color_stop_rgba(p2, 1.0, 1.0, 1.0, 1.0, 0.9);
#ifdef QTC_CAIRO_1_10_HACK
    cairo_pattern_add_color_stop_rgba(p2, 1.0, 1.0, 1.0, 1.0, 0.9);
    cairo_pattern_add_color_stop_rgba(p2, 0.0, 1.0, 1.0, 1.0, 0.7);
#endif
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

static void drawDots(cairo_t *cr, int rx, int ry, int rwidth, int rheight, gboolean horiz,
                     int nLines, int offset, GdkColor *cols, GdkRectangle *area, int startOffset,
                     int dark)
{
    int      space =(nLines*2)+(nLines-1),
             x = horiz ? rx : rx+((rwidth-space)>>1),
             y = horiz ? ry+((rheight-space)>>1) : ry,
             i, j,
             numDots=(horiz ? (rwidth-(2*offset))/3 : (rheight-(2*offset))/3)+1;
    GdkColor *col1 = &cols[dark],
             *col2 = &cols[0];

    setCairoClipping(cr, area);
    if(horiz)
    {
        if(startOffset && y+startOffset>0)
            y+=startOffset;

        cairo_new_path(cr);
        cairo_set_source_rgb(cr, CAIRO_COL(*col1));
        for(i=0; i<space; i+=3)
            for(j=0; j<numDots; j++)
                cairo_rectangle(cr, x+offset+(3*j), y+i, 1, 1);
        cairo_fill(cr);

        cairo_new_path(cr);
        cairo_set_source_rgb(cr, CAIRO_COL(*col2));
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
        cairo_set_source_rgb(cr, CAIRO_COL(*col1));
        for(i=0; i<space; i+=3)
            for(j=0; j<numDots; j++)
                cairo_rectangle(cr, x+i, y+offset+(3*j), 1, 1);
        cairo_fill(cr);

        cairo_new_path(cr);
        cairo_set_source_rgb(cr, CAIRO_COL(*col2));
        for(i=1; i<space; i+=3)
            for(j=0; j<numDots; j++)
                cairo_rectangle(cr, x+i, y+offset+1+(3*j), 1, 1);
        cairo_fill(cr);
    }
    unsetCairoClipping(cr);
}

static void getEntryParentBgCol(GtkWidget *widget, GdkColor *color)
{
    GtkWidget *parent=NULL;
    GtkStyle  *style=NULL;

    if (!widget)
    {
        color->red=color->green=color->blue = 65535;
        return;
    }

    parent = qtcWidgetGetParent(widget);

    while (parent && (qtcWidgetNoWindow(parent)))
    {
        GtkStyle *style=NULL;
        if(opts.tabBgnd && GTK_IS_NOTEBOOK(parent) && (style=qtcWidgetGetStyle(parent)))
        {
            shade(&opts, &(style->bg[GTK_STATE_NORMAL]), color, TO_FACTOR(opts.tabBgnd));
            return;
        }
        parent = qtcWidgetGetParent(parent);
    }

    if (!parent)
        parent = widget;

    style=qtcWidgetGetStyle(parent);

    if(style)
        *color = style->bg[qtcWidgetState(parent)];
}

static void drawEntryCorners(cairo_t *cr, GdkRectangle *area, int round, int x, int y, int width, int height,
                             double r, double g, double b, double a)
{
    setCairoClipping(cr, area);
    cairo_set_source_rgba(cr, r, g, b, a);
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
    unsetCairoClipping(cr);
}

static void drawBgndRing(cairo_t *cr, int x, int y, int size, int size2, gboolean isWindow)
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

static void drawBgndRings(cairo_t *cr, gint x, gint y, gint width, gint height, gboolean isWindow)
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
            loadBgndImage(img);
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

static void drawBgndImage(cairo_t *cr, GtkStyle *style, GdkRectangle *area, gint x, gint y, gint w, gint h, GdkColor *col,
                          gboolean isWindow, double alpha)
{
    GdkPixbuf *pix=isWindow ? opts.bgndPixmap.img : opts.menuBgndPixmap.img;//getPixbuf(getCheckRadioCol(style, ind_state, mnu), PIX_CHECK, 1.0);
//     int       pw=gdk_pixbuf_get_width(pix),
//               ph=gdk_pixbuf_get_height(pix),
//               dx=(x+(opts.crSize/2))-(pw/2),
//               dy=(y+(opts.crSize/2))-(ph/2);

    gdk_cairo_set_source_pixbuf(cr, pix, 0, 0);
    cairo_pattern_set_extend(cairo_get_source(cr), CAIRO_EXTEND_REPEAT);
    cairo_rectangle(cr, x, y, w, h);
    cairo_fill(cr);
}

#define STRIPE_OUTER(A, B, PART) (B.PART=((3*A.PART+B.PART)/4))
static void drawStripedBgnd(cairo_t *cr, GtkStyle *style, GdkRectangle *area, gint x, gint y, gint w, gint h, GdkColor *col,
                            gboolean isWindow, double alpha)
{
    GdkColor col2;

    shade(&opts, col, &col2, BGND_STRIPE_SHADE);

    cairo_pattern_t *pat=cairo_pattern_create_linear(x, y, x, y+4);
    cairo_pattern_add_color_stop_rgba(pat, 0.0, CAIRO_COL(*col), alpha);
    cairo_pattern_add_color_stop_rgba(pat, 0.25-0.0001, CAIRO_COL(*col), alpha);
#ifdef QTC_CAIRO_1_10_HACK
    cairo_pattern_add_color_stop_rgba(pat, 0.25-0.0001, CAIRO_COL(*col), alpha);
    cairo_pattern_add_color_stop_rgba(pat, 0.5, CAIRO_COL(col2), alpha);
#endif
    cairo_pattern_add_color_stop_rgba(pat, 0.5, CAIRO_COL(col2), alpha);
    cairo_pattern_add_color_stop_rgba(pat, 0.75-0.0001, CAIRO_COL(col2), alpha);
    col2.red=(3*col->red+col2.red)/4;
    col2.green=(3*col->green+col2.green)/4;
    col2.blue=(3*col->blue+col2.blue)/4;
    cairo_pattern_add_color_stop_rgba(pat, 0.25, CAIRO_COL(col2), alpha);
    cairo_pattern_add_color_stop_rgba(pat, 0.5-0.0001, CAIRO_COL(col2), alpha);
#ifdef QTC_CAIRO_1_10_HACK
    cairo_pattern_add_color_stop_rgba(pat, 0.5-0.0001, CAIRO_COL(col2), alpha);
    cairo_pattern_add_color_stop_rgba(pat, 0.75, CAIRO_COL(col2), alpha);
#endif
    cairo_pattern_add_color_stop_rgba(pat, 0.75, CAIRO_COL(col2), alpha);
    cairo_pattern_add_color_stop_rgba(pat, 1.0, CAIRO_COL(col2), alpha);

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

        shade(&opts, col, &col2, BGND_STRIPE_SHADE);

        if(!(*img)
            *img=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, constSize, constSize);
        ci=cairo_create(crImg);
    }
    cairo_set_source_surface(cr, *img, width-imgWidth, y+1);
    cairo_paint(cr);
*/
}

static gboolean compositingActive(GtkWidget *widget)
{
    GdkScreen *screen=widget ? gtk_widget_get_screen(widget) : gdk_screen_get_default();

    return screen && gdk_screen_is_composited(screen);
}

static gboolean isRgbaWidget(GtkWidget *widget)
{
    if (widget)
    {
        GdkVisual *visual = gtk_widget_get_visual(widget);
#if GTK_CHECK_VERSION(2, 90, 0)
        guint32  redMask,
                 greenMask,
                 blueMask;

        gdk_visual_get_red_pixel_details(visual, &redMask, NULL, NULL);
        gdk_visual_get_green_pixel_details(visual, &greenMask, NULL, NULL);
        gdk_visual_get_blue_pixel_details(visual, &blueMask, NULL, NULL);

        return 32==gdk_visual_get_depth(visual) && (0xff0000==redMask && 0x00ff00==greenMask && 0x0000ff==blueMask);
#else
        return 32==visual->depth && (0xff0000==visual->red_mask && 0x00ff00==visual->green_mask && 0x0000ff==visual->blue_mask);
#endif
    }
    return FALSE;
}

#define BLUR_BEHIND_OBJECT "QTC_BLUR_BEHIND"
static void enableBlurBehind(GtkWidget *w, gboolean enable)
{
    GtkWindow  *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(w));

    if(topLevel)
    {
        GdkDisplay *display=gtk_widget_get_display(GTK_WIDGET(topLevel));

        if(display)
        {
            int oldValue=(int)g_object_get_data(G_OBJECT(w), BLUR_BEHIND_OBJECT);

            if(0==oldValue || (enable && 1!=oldValue) || (!enable && 2!=oldValue))
            {
                Atom atom = gdk_x11_get_xatom_by_name_for_display(display, "_KDE_NET_WM_BLUR_BEHIND_REGION");
                int  value=enable ? 1 : 2;

                g_object_set_data(G_OBJECT(w), MENU_SIZE_ATOM, (gpointer)value);
                if (enable)
                    XChangeProperty(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XID(qtcWidgetGetWindow(GTK_WIDGET(topLevel))), atom,
                                    XA_CARDINAL, 32, PropModeReplace, 0, 0);
                else
                    XDeleteProperty(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XID(qtcWidgetGetWindow(GTK_WIDGET(topLevel))), atom);
            }
        }
    }
}

static gboolean drawWindowBgnd(cairo_t *cr, GtkStyle *style, GdkRectangle *area, GtkWidget *widget, gint x, gint y, gint width,
                               gint height)
{
    GtkWidget *parent=NULL;
    if(widget && (parent=qtcWidgetGetParent(widget)) && isOnHandlebox(parent, NULL, 0))
        return TRUE;

    if(!isFixedWidget(widget) && !isGimpDockable(widget))
    {
        GtkWidget   *window=widget;
        int         xpos=0,
                    ypos=0;
        const gchar *wName=NULL;

        if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d  ", __FUNCTION__, x, y, width, height), debugDisplayWidget(widget, 20);

        while(window && !GTK_IS_WINDOW(window))
            window=qtcWidgetGetParent(window);

        if(0==y && window && window!=widget)
            gtk_widget_translate_coordinates(widget, window, x, y, &xpos, &ypos);

        if(window && (!(wName=qtcWidgetName(window)) || strcmp(wName, "gtk-tooltip")))
        {
            GdkRectangle  clip;
            int           opacity=!window || !GTK_IS_DIALOG(window) ? opts.bgndOpacity : opts.dlgOpacity,
                          xmod=0, ymod=0, wmod=0, hmod=0;
            double        alpha=1.0;
            gboolean      useAlpha=opacity<100 && isRgbaWidget(window) && compositingActive(window),
                          flatBgnd=IS_FLAT_BGND(opts.bgndAppearance);
            GtkAllocation alloc=qtcWidgetGetAllocation(window);

            if(!flatBgnd || BGND_IMG_ON_BORDER)
            {
                WindowBorders borders=qtcGetWindowBorderSize(FALSE);
                xmod=-borders.sides;
                ymod=-borders.titleHeight;
                wmod=2*borders.sides;
                hmod=borders.titleHeight+borders.bottom;
            }

            clip.x=x, clip.y=-ypos, clip.width=width, clip.height=alloc.height;
            setCairoClipping(cr, &clip);

            if(useAlpha)
            {
                alpha=opacity/100.0;
                cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            }

            if(flatBgnd)
            {
                GdkColor *parent_col=getParentBgCol(widget);

                if(parent_col)
                    drawAreaColorAlpha(cr, area, parent_col, x, -ypos, width, alloc.height, alpha);
                else if(useAlpha)
                    drawAreaColorAlpha(cr, area, &style->bg[GTK_STATE_NORMAL] , x, -ypos, width, alloc.height, alpha);
            }
            else if(APPEARANCE_STRIPED==opts.bgndAppearance)
                drawStripedBgnd(cr, style, area,  x+xmod, -ypos+ymod, width+wmod, alloc.height+hmod, &style->bg[GTK_STATE_NORMAL], TRUE, alpha);
            else if(APPEARANCE_FILE==opts.bgndAppearance)
            {
                cairo_save(cr);
                cairo_translate(cr, xmod, ymod);
                drawBgndImage(cr, style, area,  x, -ypos, width+wmod, alloc.height+hmod, &style->bg[GTK_STATE_NORMAL], TRUE, alpha);
                cairo_restore(cr);
            }
            else
            {
                if(GT_HORIZ==opts.bgndGrad)
                    drawBevelGradientAlpha(cr, style, area, x+xmod, -ypos+ymod, width+wmod, alloc.height+hmod,
                                           &style->bg[GTK_STATE_NORMAL], TRUE, FALSE, opts.bgndAppearance, WIDGET_OTHER, alpha);
                else
                    drawBevelGradientAlpha(cr, style, area, -xpos+xmod, y+ymod,alloc.width+wmod, height+hmod,
                                           &style->bg[GTK_STATE_NORMAL], FALSE, FALSE, opts.bgndAppearance, WIDGET_OTHER, alpha);

                if(GT_HORIZ==opts.bgndGrad && GB_SHINE==getGradient(opts.bgndAppearance, &opts)->border)
                {
                    int             wwidth=alloc.width+wmod,
                                    wheight=alloc.height+hmod,
                                    size=MIN(BGND_SHINE_SIZE, MIN(wheight*2, wwidth));
                    double          alpha=shineAlpha(&style->bg[GTK_STATE_NORMAL]);
                    cairo_pattern_t *pat=NULL;

                    size/=BGND_SHINE_STEPS;
                    size*=BGND_SHINE_STEPS;
                    pat=cairo_pattern_create_radial(x+xmod+(wwidth/2.0), ypos+ymod, 0, x+xmod+(wwidth/2.0), ypos+ymod, size/2.0);
                    cairo_pattern_add_color_stop_rgba(pat, 0, 1, 1, 1, alpha);
                    cairo_pattern_add_color_stop_rgba(pat, 0.5, 1, 1, 1, alpha*0.625);
#ifdef QTC_CAIRO_1_10_HACK
                    cairo_pattern_add_color_stop_rgba(pat, 0.5, 1, 1, 1, alpha*0.625);
                    cairo_pattern_add_color_stop_rgba(pat, 0.75, 1, 1, 1, alpha*0.175);
#endif
                    cairo_pattern_add_color_stop_rgba(pat, 0.75, 1, 1, 1, alpha*0.175);
                    cairo_pattern_add_color_stop_rgba(pat, 1.0, 1, 1, 1, 0.0);
                    cairo_set_source(cr, pat);
                    cairo_rectangle(cr, x+xmod+((wwidth-size)/2.0), ypos+ymod, size, size);
                    cairo_fill(cr);
                    cairo_pattern_destroy(pat);
                }
            }
            if(useAlpha)
                cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

            if(BGND_IMG_ON_BORDER)
            {
                if(IMG_FILE==opts.bgndImage.type)
                    wmod++, hmod++;
                else
                    wmod=(wmod/2)+1;
            }
            else
                wmod=0, hmod=0, xmod=0, ymod=0;

            drawBgndRings(cr, -xpos+xmod, -ypos+ymod, alloc.width+wmod-1, alloc.height+hmod-1, TRUE);
            unsetCairoClipping(cr);
            return TRUE;
        }
    }
    return FALSE;
}

static void drawEntryField(cairo_t *cr, GtkStyle *style, GtkStateType state,GtkWidget *widget, GdkRectangle *area, gint x, gint y,
                           gint width, gint height, int round, EWidget w)
{
    gboolean enabled=!(GTK_STATE_INSENSITIVE==state || (widget && !qtcWidgetIsSensitive(widget))),
             highlightReal=enabled && widget && qtcWidgetHasFocus(widget) && GTK_APP_JAVA!=qtSettings.app && qtcPalette.focus,
             mouseOver=ENTRY_MO && enabled && (GTK_STATE_PRELIGHT==state || (lastMoEntry && widget==lastMoEntry) ) &&
                       qtcPalette.mouseover && GTK_APP_JAVA!=qtSettings.app,
             highlight=highlightReal || mouseOver,
             doEtch=DO_EFFECT && opts.etchEntry;
    GdkColor *colors=mouseOver
                        ? qtcPalette.mouseover
                        : highlightReal
                            ? qtcPalette.focus
                            : qtcPalette.background;
    int      origHeight=height;

    if(GTK_APP_JAVA!=qtSettings.app)
        qtcEntrySetup(widget);

    if((doEtch || ROUND_NONE!=opts.round) && (!widget || !g_object_get_data(G_OBJECT (widget), "transparent-bg-hint")))
        if(!IS_FLAT_BGND(opts.bgndAppearance) && widget && drawWindowBgnd(cr, style, area, widget, x, y, width, height))
            ;
        else
        {
            GdkColor parentBgCol;
            getEntryParentBgCol(widget, &parentBgCol);
            drawEntryCorners(cr, area, round, x, y, width, height, CAIRO_COL(parentBgCol), 1.0);
        }

    if(0!=opts.gbFactor && (FRAME_SHADED==opts.groupBox || FRAME_FADED==opts.groupBox) && isInGroupBox(widget, 0))
    {
        double col=opts.gbFactor<0 ? 0.0 : 1.0;
        drawEntryCorners(cr, area, round, x, y, width, height, col, col, col, TO_ALPHA(opts.gbFactor));
    }
    
    if(doEtch)
        y++,  x++, height-=2,  width-=2;

    if(DEBUG_ALL==qtSettings.debug)  printf(DEBUG_PREFIX "%s %d %d %d %d %d %d ", __FUNCTION__, state, x, y, width, height, round),
                                     debugDisplayWidget(widget, 3);

    if(ROUNDED_ALL!=round)
    {
        if(WIDGET_SPIN==w || WIDGET_COMBO_BUTTON==w)
            x--, width++;
        else if(highlight)
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

    if(GTK_APP_OPEN_OFFICE!=qtSettings.app)
    {
        if(opts.round>ROUND_FULL)
            clipPath(cr, x+1, y+1, width-2, height-2, WIDGET_ENTRY, RADIUS_INTERNAL, ROUNDED_ALL);

#if GTK_CHECK_VERSION(2, 90, 0)
        drawAreaColor(cr, area, enabled
                                    ? &style->base[GTK_STATE_NORMAL]
                                    : &style->bg[GTK_STATE_INSENSITIVE], x+1, y+1, width-2, height-2);
#else
        drawAreaColor(cr, area, enabled
                                    ? &style->base[WIDGET_COMBO_BUTTON==w || GTK_STATE_PRELIGHT==state ? GTK_STATE_NORMAL : state]
                                    : &style->bg[GTK_STATE_INSENSITIVE], x+1, y+1, width-2, height-2);
#endif
        if(opts.round>ROUND_FULL)
            unsetCairoClipping(cr);
    }

    {
    int xo=x, yo=y, widtho=width, heighto=height;

    if(doEtch)
    {
        QtcRect   rect;
        QtcRegion *region=NULL;

        y--;
        height+=2;
        x--;
        width+=2;

        rect.x=x; rect.y=y; rect.width=width; rect.height=height;
        region=qtcRegionRect(&rect);

        if(!(WIDGET_SPIN==w && opts.unifySpin) && !(WIDGET_COMBO_BUTTON==w && opts.unifyCombo))
        {
            if(!(round&CORNER_TR) && !(round&CORNER_BR))
                width+=4;
            if(!(round&CORNER_TL) && !(round&CORNER_BL))
                x-=4;
        }

        drawEtch(cr, area, widget, x, y, width, height, FALSE, round, WIDGET_ENTRY);
        qtcRegionDestroy(region);
    }

    drawBorder(cr, style, !widget || qtcWidgetIsSensitive(widget) ? state : GTK_STATE_INSENSITIVE, area, xo, yo, widtho, heighto,
               colors, round, BORDER_SUNKEN, WIDGET_ENTRY, DF_DO_CORNERS|DF_BLEND);
    }
               
    if(GTK_IS_ENTRY(widget) && !gtk_entry_get_visibility(GTK_ENTRY(widget)))
        gtk_entry_set_invisible_char(GTK_ENTRY(widget), opts.passwordChar);
}

static void setProgressStripeClipping(cairo_t *cr, AREA_PARAM int x, int y, int width, int height, int animShift, gboolean horiz)
{
    int stripeOffset;

    switch(opts.stripedProgress)
    {
        default:
        case STRIPE_PLAIN:
        {
            QtcRect   rect={x, y, width-2, height-2};
            QtcRegion *region=NULL;

#if !GTK_CHECK_VERSION(2, 90, 0)
            constrainRect(&rect, area);
#endif
            region=qtcRegionRect(&rect);
            if(horiz)
                for(stripeOffset=0; stripeOffset<(width+PROGRESS_CHUNK_WIDTH); stripeOffset+=(PROGRESS_CHUNK_WIDTH*2))
                {
                    QtcRect innerRect={x+stripeOffset+animShift, y+1, PROGRESS_CHUNK_WIDTH, height-2};

#if !GTK_CHECK_VERSION(2, 90, 0)
                    constrainRect(&innerRect, area);
#endif
                    if(innerRect.width>0 && innerRect.height>0)
                    {
                        QtcRegion *innerRegion=qtcRegionRect(&innerRect);

                        qtcRegionXor(region, innerRegion);
                        qtcRegionDestroy(innerRegion);
                    }
                }
            else
                for(stripeOffset=0; stripeOffset<(height+PROGRESS_CHUNK_WIDTH); stripeOffset+=(PROGRESS_CHUNK_WIDTH*2))
                {
                    QtcRect innerRect={x+1, y+stripeOffset+animShift, width-2, PROGRESS_CHUNK_WIDTH};

                    /*constrainRect(&innerRect, area);*/
                    if(innerRect.width>0 && innerRect.height>0)
                    {
                        QtcRegion *innerRegion=qtcRegionRect(&innerRect);

                        qtcRegionXor(region, innerRegion);
                        qtcRegionDestroy(innerRegion);
                    }
                }
            setCairoClippingRegion(cr, region);
            qtcRegionDestroy(region);
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

static void drawProgress(cairo_t *cr, GtkStyle *style, GtkStateType state, GtkWidget *widget, GdkRectangle *area, int x, int y, int width, int height,
                         gboolean rev, gboolean isEntryProg)
{
#if GTK_CHECK_VERSION(2, 90, 0)
    gboolean                  revProg=widget && gtk_progress_bar_get_inverted(GTK_PROGRESS_BAR(widget));
#else
    GtkProgressBarOrientation orientation=widget && GTK_IS_PROGRESS_BAR(widget)
                                    ? gtk_progress_bar_get_orientation(GTK_PROGRESS_BAR(widget))
                                    : GTK_PROGRESS_LEFT_TO_RIGHT;
    gboolean                  revProg=GTK_PROGRESS_RIGHT_TO_LEFT==orientation || GTK_PROGRESS_BOTTOM_TO_TOP==orientation;
#endif
    gboolean                  horiz=isHorizontalProgressbar(widget);
    int                       wid=isEntryProg ? WIDGET_ENTRY_PROGRESSBAR : WIDGET_PROGRESSBAR,
                              animShift=revProg ? 0 : -PROGRESS_CHUNK_WIDTH,
                              xo=x, yo=y, wo=width, ho=height;

    if(opts.fillProgress)
        x--, y--, width+=2, height+=2, xo=x, yo=y, wo=width, ho=height;

    if(STRIPE_NONE!=opts.stripedProgress && opts.animatedProgress && (isEntryProg || IS_PROGRESS_BAR(widget)))
    {
#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
        if(isEntryProg || !GTK_PROGRESS(widget)->activity_mode)
#endif
            qtc_animation_progressbar_add((gpointer)widget, isEntryProg);

        animShift+=(revProg ? -1 : 1)*
                    (((int)(qtc_animation_elapsed(widget)*PROGRESS_CHUNK_WIDTH))%(PROGRESS_CHUNK_WIDTH*2));
    }

    {
        gboolean grayItem=GTK_STATE_INSENSITIVE==state && ECOLOR_BACKGROUND!=opts.progressGrooveColor;
        GdkColor *itemCols=grayItem
                            ? qtcPalette.background
                            : qtcPalette.progress
                                ? qtcPalette.progress
                                : qtcPalette.highlight;
        int      new_state=GTK_STATE_PRELIGHT==state ? GTK_STATE_NORMAL : state,
                 fillVal=grayItem ? 4 : ORIGINAL_SHADE,
                 borderVal=0;

        x++, y++, width-=2, height-=2;

        if(opts.borderProgress && opts.round>ROUND_SLIGHT && (horiz ? width : height)<4)
            clipPath(cr, x, y, width, height, wid, RADIUS_EXTERNAL, ROUNDED_ALL);

        if((horiz ? width : height)>1)
            drawLightBevel(cr, style, new_state, area, x, y, width, height, &itemCols[fillVal],
                           itemCols, ROUNDED_ALL, wid, BORDER_FLAT, (horiz ? 0 : DF_VERT)|DF_DO_CORNERS, widget);

        if(opts.stripedProgress && width>4 && height>4)
            if(STRIPE_FADE==opts.stripedProgress)
            {
                int posMod=opts.animatedProgress ? STRIPE_WIDTH-animShift : 0,
                    sizeMod=opts.animatedProgress ? (STRIPE_WIDTH*2) : 0;
                addStripes(cr, x-(horiz ? posMod : 0), y-(horiz ? 0 : posMod),
                                 width+(horiz ? sizeMod : 0), height+(horiz ? 0 : sizeMod), horiz);
            }
            else
            {
                setProgressStripeClipping(cr, AREA_PARAM_VAL xo, yo, wo, ho, animShift, horiz);
                drawLightBevel(cr, style, new_state, NULL, x, y, width, height, &itemCols[1],
                               qtcPalette.highlight, ROUNDED_ALL, wid, BORDER_FLAT,
                               (opts.fillProgress || !opts.borderProgress ? 0 : DF_DO_BORDER)|
                               (horiz ? 0 : DF_VERT)|DF_DO_CORNERS, widget);
                unsetCairoClipping(cr);
            }

        if(opts.glowProgress && (horiz ? width : height)>3)
        {
            int             offset=opts.borderProgress ? 1 : 0;
            cairo_pattern_t *pat=cairo_pattern_create_linear(x+offset, y+offset,
                                                             horiz ? x+width-offset : x+offset,
                                                             horiz ? y+offset : y+height-offset);
            gboolean        inverted=FALSE;
            
            if(GLOW_MIDDLE!=opts.glowProgress && widget && GTK_IS_PROGRESS_BAR(widget))
                if(isHorizontalProgressbar)
                {
                    if(revProg)
                        inverted=!rev;
                    else
                        inverted=rev;
                }
                else
                    inverted=revProg;

            cairo_pattern_add_color_stop_rgba(pat, 0.0, 1.0, 1.0, 1.0,
                                              (inverted ? GLOW_END : GLOW_START)==opts.glowProgress ? GLOW_PROG_ALPHA : 0.0);
#ifdef QTC_CAIRO_1_10_HACK
            cairo_pattern_add_color_stop_rgba(pat, 0.0, 1.0, 1.0, 1.0,
                                              (inverted ? GLOW_END : GLOW_START)==opts.glowProgress ? GLOW_PROG_ALPHA : 0.0);
#endif
            if(GLOW_MIDDLE==opts.glowProgress)
                cairo_pattern_add_color_stop_rgba(pat, 0.5, 1.0, 1.0, 1.0, GLOW_PROG_ALPHA);
#ifdef QTC_CAIRO_1_10_HACK
            cairo_pattern_add_color_stop_rgba(pat, 1.0, 1.0, 1.0, 1.0,
                                              (inverted ? GLOW_START : GLOW_END)==opts.glowProgress ? GLOW_PROG_ALPHA : 0.0);
#endif
            cairo_pattern_add_color_stop_rgba(pat, 1.0, 1.0, 1.0, 1.0,
                                              (inverted ? GLOW_START : GLOW_END)==opts.glowProgress ? GLOW_PROG_ALPHA : 0.0);
            cairo_set_source(cr, pat);
            cairo_rectangle(cr, x+offset, y+offset, width-(2*offset), height-(2*offset));
            cairo_fill(cr);
            cairo_pattern_destroy(pat);
        }

        if(width>2 && height>2 && opts.borderProgress)
            realDrawBorder(cr, style, state, area, x, y, width, height,
                           itemCols, ROUNDED_ALL, BORDER_FLAT, wid, 0, PBAR_BORDER);
        
        if(!opts.borderProgress)
            if(horiz)
            {
                drawHLine(cr, CAIRO_COL(itemCols[PBAR_BORDER]), 1.0, x, y, width);
                drawHLine(cr, CAIRO_COL(itemCols[PBAR_BORDER]), 1.0, x, y+height-1, width);
            }
            else
            {
                drawVLine(cr, CAIRO_COL(itemCols[PBAR_BORDER]), 1.0, x, y, height);
                drawVLine(cr, CAIRO_COL(itemCols[PBAR_BORDER]), 1.0, x+width-1, y, height);
            }
    }
}

#ifdef QTC_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT
#define GIMP_MAIN   "GimpToolbox"      /* Main GIMP toolbox */
#define GIMP_WINDOW "GimpDisplayShell" /* Image window */

static GtkWidget * getParentWindow(GtkWidget *widget)
{
    GtkWidget *top=NULL;
    GList     *topWindows,
              *node;

    if(GTK_IS_DIALOG(widget) || GTK_APP_GIMP!=qtSettings.app)
        for(topWindows=node=gtk_window_list_toplevels(); node; node = node->next)
        {
            GtkWidget *w=node->data;

            if(w && GTK_IS_WIDGET(w) && qtcWidgetGetWindow(w) && w!=widget && qtcWindowIsActive(w))
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

            if(w && GTK_IS_WIDGET(w) && 0==strcmp(g_type_name(qtcWidgetType(w)), GIMP_MAIN))
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
        gdk_window_set_transient_for(qtcWidgetGetWindow(widget), qtcWidgetGetWindow(top));
        /*gtk_window_set_skip_taskbar_hint(GTK_WINDOW(widget), TRUE);
        gtk_window_set_skip_pager_hint(GTK_WINDOW(widget), TRUE); */
    }
}
#endif

static void drawSelectionGradient(cairo_t *cr, GtkStyle *style, GtkStateType state, GdkRectangle *area, GtkWidget *widget,
                                  int x, int y, int width, int height, int round, gboolean isLvSelection,
                                  double alpha, GdkColor *col, gboolean horiz)
{
    if((!isLvSelection || !(opts.square&SQUARE_LISTVIEW_SELECTION)) && ROUND_NONE!=opts.round)
        clipPathRadius(cr, x, y, width, height, getRadius(&opts, width, height, WIDGET_SELECTION, RADIUS_SELECTION), round);
    drawBevelGradientAlpha(cr, style, area, x, y, width, height, col,
                           horiz, FALSE, opts.selectionAppearance, WIDGET_SELECTION, alpha);
    if((!isLvSelection || !(opts.square&SQUARE_LISTVIEW_SELECTION)) && ROUND_NONE!=opts.round)
        cairo_restore(cr);
}

static void drawSelection(cairo_t *cr, GtkStyle *style, GtkStateType state, GdkRectangle *area, GtkWidget *widget,
                          int x, int y, int width, int height, int round, gboolean isLvSelection)
{
    gboolean hasFocus=qtcWidgetHasFocus(widget);
    double   alpha=(GTK_STATE_PRELIGHT==state ? 0.20 : 1.0)*(hasFocus || !qtSettings.inactiveChangeSelectionColor ? 1.0 : INACTIVE_SEL_ALPHA);
    GdkColor *col=&style->base[hasFocus ? GTK_STATE_SELECTED : GTK_STATE_ACTIVE];

    drawSelectionGradient(cr, style, state, area, widget, x, y, width, height, round, isLvSelection, alpha, col, TRUE);

    if(opts.borderSelection && (!isLvSelection || !(opts.square&SQUARE_LISTVIEW_SELECTION)))
    {
        double   xd=x+0.5,
                 yd=y+0.5,
                 alpha=GTK_STATE_PRELIGHT==state ? 0.20 : 1.0;
        int      xo=x, widtho=width;
    
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
        cairo_set_source_rgba(cr, CAIRO_COL(*col), alpha);
        createPath(cr, xd, yd, width-1, height-1, getRadius(&opts, widtho, height, WIDGET_OTHER, RADIUS_SELECTION), round);
        cairo_stroke(cr);
        cairo_restore(cr);
    }
}

static void gtkDrawSlider(GtkStyle *style, WINDOW_PARAM GtkStateType state, GtkShadowType shadow_type, AREA_PARAM
                          GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height, GtkOrientation orientation);

static void qtcLogHandler(const gchar *domain, GLogLevelFlags level, const gchar *msg, gpointer data)
{
}

static void gtkDrawFlatBox(GtkStyle *style, WINDOW_PARAM GtkStateType state, GtkShadowType shadow_type, AREA_PARAM
                           GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height)
{
    CAIRO_BEGIN

    gboolean isMenuOrToolTipWindow=widget && GTK_IS_WINDOW(widget) &&
                                   ((qtcWidgetName(widget) && 0==strcmp(qtcWidgetName(widget), "gtk-tooltip")) ||
                                    isMenuWindow(widget));

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %s  ", __FUNCTION__, state, shadow_type, x, y, width, height, detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 3);

    sanitizeSize(window, &width, &height);

#define MODAL_HACK  "QTC_MODAL_HACK_SET"
#define BUTTON_HACK "QTC_BUTTON_ORDER_HACK_SET"

#if GTK_CHECK_VERSION(2, 6, 0)
    if(!opts.gtkButtonOrder && opts.reorderGtkButtons && GTK_IS_WINDOW(widget) && detail && 0==strcmp(detail, "base"))
    {
        GtkWidget *topLevel=gtk_widget_get_toplevel(widget);

        if(topLevel && GTK_IS_DIALOG(topLevel) && !g_object_get_data(G_OBJECT(topLevel), BUTTON_HACK))
        {
            // gtk_dialog_set_alternative_button_order will cause errors to be logged, but dont want these
            // so register ur own error handler, and then unregister afterwards...
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
#ifdef QTC_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT
    if(widget && opts.fixParentlessDialogs && !isMenuOrToolTipWindow && GTK_IS_WINDOW(widget) && detail && 0==strcmp(detail, "base"))
    {
        GtkWidget *topLevel=gtk_widget_get_toplevel(widget);

        if(topLevel && qtcWidgetTopLevel(topLevel))
        {
            const gchar *typename=g_type_name(qtcWidgetType(topLevel));

            if(GTK_APP_GIMP_PLUGIN==qtSettings.app)
            {
                /* CPD should really try to find active GIMP window... */
                gtk_window_set_skip_taskbar_hint(GTK_WINDOW(topLevel), TRUE);
                gtk_window_set_skip_pager_hint(GTK_WINDOW(topLevel), TRUE);
                gtk_window_set_keep_above(GTK_WINDOW(topLevel), TRUE);
            }
            else if((GTK_WINDOW(topLevel)->modal || GTK_IS_DIALOG(topLevel) || /*GTK_APP_GAIM==qtSettings.app ||*/
                    (GTK_APP_GIMP==qtSettings.app &&
                     strcmp(typename, GIMP_WINDOW) &&
                     strcmp(typename, GIMP_MAIN) ) ) &&
                   !g_object_get_data(G_OBJECT(topLevel), MODAL_HACK) &&
                   NULL==gtk_window_get_transient_for(GTK_WINDOW(topLevel)))
            {
                g_object_set_data(G_OBJECT(topLevel), MODAL_HACK, (gpointer)1);

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
#endif

    if(widget && ((100!=opts.bgndOpacity && GTK_IS_WINDOW(widget)) || (100!=opts.dlgOpacity && GTK_IS_DIALOG(widget))) &&
       !isFixedWidget(widget) && isRgbaWidget(widget))
        enableBlurBehind(widget, TRUE);
    
    if ((opts.menubarHiding || opts.statusbarHiding || BLEND_TITLEBAR || opts.windowBorder&WINDOW_BORDER_USE_MENUBAR_COLOR_FOR_TITLEBAR) &&
        widget && GTK_IS_WINDOW(widget) && !isFixedWidget(widget) && !isGimpDockable(widget) && !isMenuOrToolTipWindow)
    {
        if(qtcWindowSetup(widget, GTK_IS_DIALOG(widget) ? opts.dlgOpacity : opts.bgndOpacity))
        {
            GtkWidget *menuBar=qtcWindowGetMenuBar(widget, 0);
            GtkWidget *statusBar=opts.statusbarHiding ? qtcWindowGetStatusBar(widget, 0) : NULL;

            if(menuBar)
            {
                bool          hiddenMenubar=opts.menubarHiding ? qtcMenuBarHidden(qtSettings.appName) : FALSE;
                GtkAllocation alloc=qtcWidgetGetAllocation(menuBar);

                if(hiddenMenubar)
                    gtk_widget_hide(menuBar);

                if(BLEND_TITLEBAR || opts.menubarHiding&HIDE_KWIN || opts.windowBorder&WINDOW_BORDER_USE_MENUBAR_COLOR_FOR_TITLEBAR)
                    qtcEmitMenuSize(menuBar, hiddenMenubar ? 0 : alloc.height);
                    
                if(opts.menubarHiding&HIDE_KWIN)
                    qtcWindowMenuBarDBus(widget, hiddenMenubar ? 0 : alloc.height);
            }
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

    if(CUSTOM_BGND && widget && GTK_IS_WINDOW(widget) && !isMenuOrToolTipWindow &&
       drawWindowBgnd(cr, style, area, widget, x, y, width, height))
        qtcWindowSetup(widget, GTK_IS_DIALOG(widget) ? opts.dlgOpacity : opts.bgndOpacity);
    else if(widget && GTK_IS_TREE_VIEW(widget))
    {
        gboolean combo=isComboBoxPopupWindow(widget, 0);
        int      round=!combo && detail && GTK_STATE_SELECTED==state && ROUNDED
                            ? 0!=strstr(detail, "_start")
                                ? ROUNDED_LEFT
                                : 0!=strstr(detail, "_end")
                                    ? ROUNDED_RIGHT
                                    : 0!=strstr(detail, "_middle")
                                        ? ROUNDED_NONE
                                        : ROUNDED_ALL
                            : ROUNDED_NONE;

#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
        if(opts.lvLines)
        {
            QtCurveStyle *qtcurveStyle = (QtCurveStyle *)style;
            GtkStyle     *style        = qtcWidgetGetStyle(widget);

            if(style)
            {
                // GtkTreeView copies the black GC! But dont want black lines, so hack around this...
                GdkGC *black=style->black_gc;
                style->black_gc=qtcurveStyle->lv_lines_gc;
                gtk_tree_view_set_enable_tree_lines(GTK_TREE_VIEW(widget), TRUE);
                style->black_gc=black; // Restore!
            }
        }
#endif
    /*
        int px, py;
        gtk_widget_get_pointer(widget, &px, &py);
        if(px>=x && px<(x+width) && py>y && py<(y+height))
            state=GTK_STATE_PRELIGHT;
    */

        if(GTK_STATE_SELECTED!=state || ROUNDED_NONE!=round)
            drawAreaColor(cr, area,
                          getCellCol(haveAlternareListViewCol() &&
                          (opts.forceAlternateLvCols || gtk_tree_view_get_rules_hint(GTK_TREE_VIEW(widget))) &&
                          DETAILHAS("cell_odd")
                            ? &qtSettings.colors[PAL_ACTIVE][COLOR_LV]
                            : &style->base[GTK_STATE_NORMAL], detail),
                          x, y, width, height);

        if(GTK_STATE_SELECTED==state)
            if(combo)
                drawAreaColor(cr, area, &style->base[widget && qtcWidgetHasFocus(widget) ? GTK_STATE_SELECTED : GTK_STATE_ACTIVE],
                              x, y, width, height);
            else
                drawSelection(cr, style, state, area, widget, x, y, width, height, round, TRUE);
    }
    else if(detail && opts.splitterHighlight && 0==strcmp(detail, QTC_PANED))
    {
        if(GTK_STATE_PRELIGHT==state && opts.splitterHighlight)
        {
            GdkColor col=shadeColor(&style->bg[state], TO_FACTOR(opts.splitterHighlight));
            drawSelectionGradient(cr, style, state, area, widget, x, y, width, height, ROUNDED_ALL, FALSE, 1.0, &col,
                              width>height);
        }
    }
    else if(detail && 0==strcmp(detail, "checkbutton"))
    {
        if(GTK_STATE_PRELIGHT==state && opts.crHighlight && width>(opts.crSize*2))
        {
            GdkColor col=shadeColor(&style->bg[state], TO_FACTOR(opts.crHighlight));
            drawSelectionGradient(cr, style, state, area, widget, x, y, width, height, ROUNDED_ALL, FALSE, 1.0, &col, TRUE);
        }
    }
    else if(detail && 0==strcmp(detail, "expander"))
    {
        if(GTK_STATE_PRELIGHT==state && opts.expanderHighlight)
        {
            GdkColor col=shadeColor(&style->bg[state], TO_FACTOR(opts.expanderHighlight));
            drawSelectionGradient(cr, style, state, area, widget, x, y, width, height, ROUNDED_ALL, FALSE, 1.0, &col, TRUE);
        }
    }
    else if(DETAIL("tooltip"))
    {
        GdkColor *col=&qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP];
        
#if GTK_CHECK_VERSION(2,9,0)
        double   radius=0;
        gboolean nonGtk=isMozilla() || GTK_APP_OPEN_OFFICE==qtSettings.app || GTK_APP_JAVA==qtSettings.app,
                 rounded=!nonGtk && widget && !(opts.square&SQUARE_TOOLTIPS);

        if(!nonGtk && GTK_IS_WINDOW(widget))
            gtk_window_set_opacity(GTK_WINDOW(widget), 0.875);

        if(rounded)
        {
            int size=((width&0xFFFF)<<16)+(height&0xFFFF),
                old=(int)g_object_get_data(G_OBJECT(widget), "QTC_WIDGET_MASK");

            radius=MENU_AND_TOOLTIP_RADIUS; // getRadius(&opts, width, height, WIDGET_SELECTION, RADIUS_SELECTION);

            if(size!=old)
            {
#if GTK_CHECK_VERSION(2, 90, 0)
                QtcRegion *mask=windowMask(0, 0, width, height, opts.round>ROUND_SLIGHT);

                gtk_widget_shape_combine_region(widget, NULL);
                gtk_widget_shape_combine_region(widget, mask);
                qtcRegionDestroy(mask);
#else
                GdkBitmap *mask=gdk_pixmap_new(NULL, width, height, 1);
                cairo_t   *crMask = gdk_cairo_create((GdkDrawable *) mask);

                cairo_rectangle(crMask, 0, 0, width, height);
                cairo_set_source_rgba(crMask, 1, 1, 1, 0);
                cairo_set_operator(crMask, CAIRO_OPERATOR_SOURCE);
                cairo_paint(crMask);
                cairo_new_path(crMask);
                createPath(crMask, 0, 0, width, height, radius, ROUNDED_ALL);
                cairo_set_source_rgba(crMask, 1, 0, 0, 1);
                cairo_fill(crMask);

                gtk_widget_shape_combine_mask(widget, NULL, 0, 0);
                gtk_widget_shape_combine_mask(widget, mask, 0, 0);
                cairo_destroy(crMask);
#endif
                g_object_set_data(G_OBJECT(widget), "QTC_WIDGET_MASK", (gpointer)size);
                /* Setting the window type to 'popup menu' seems to re-eanble kwin shadows! */
                if(qtcWidgetGetWindow(widget))
                    gdk_window_set_type_hint(qtcWidgetGetWindow(widget), GDK_WINDOW_TYPE_HINT_POPUP_MENU);
            }

            clipPathRadius(cr, x, y, width, height, radius, ROUNDED_ALL);
        }
#endif
        drawBevelGradient(cr, style, area, x, y, width, height, col, true, FALSE, opts.tooltipAppearance, WIDGET_OTHER);
#if GTK_CHECK_VERSION(2,9,0)
        if(!rounded)
#endif
        if(IS_FLAT(opts.tooltipAppearance))
        {
            cairo_new_path(cr);
            /*if(IS_FLAT(opts.tooltipAppearance))*/
                cairo_set_source_rgb(cr, CAIRO_COL(qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP_TEXT]));
            /*else
                cairo_set_source_rgba(cr, 0, 0, 0, 0.25);*/
            cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
            cairo_stroke(cr);
        }
    }
    else if(DETAIL("icon_view_item"))
        drawSelection(cr, style, state, area, widget, x, y, width, height, ROUNDED_ALL, FALSE);
    else if(!(GTK_APP_JAVA==qtSettings.app && widget && GTK_IS_LABEL(widget)))
    {
        if(GTK_STATE_PRELIGHT==state && !opts.crHighlight && 0==strcmp(detail, "checkbutton"))
            ;
        else
            parent_class->draw_flat_box(style, WINDOW_PARAM_VAL
                                        GTK_STATE_INSENSITIVE==state && DETAIL(QTC_PANED) ? GTK_STATE_NORMAL : state,
                                        shadow_type, AREA_PARAM_VAL widget, detail, x, y, width, height);

        /* For SWT (e.g. eclipse) apps. For some reason these only seem to allow a ythickness of at max 2 - but
           for etching we need 3. So we fake this by drawing the 3rd lines here...*/

/*
        if(DO_EFFECT && GTK_STATE_INSENSITIVE!=state && DETAIL("entry_bg") &&
           isSwtComboBoxEntry(widget) && qtcWidgetHasFocus(widget))
        {
            drawHLine(cr, CAIRO_COL(qtcPalette.highlight[FRAME_DARK_SHADOW]), 1.0, x, y, width);
            drawHLine(cr, CAIRO_COL(qtcPalette.highlight[0]), 1.0, x, y+height-1, width);
        }
*/
    }
    CAIRO_END
}

static void gtkDrawHandle(GtkStyle *style, WINDOW_PARAM GtkStateType state, GtkShadowType shadow_type, AREA_PARAM
                          GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height, GtkOrientation orientation)
{
    gboolean paf=WIDGET_TYPE_NAME("PanelAppletFrame");
    CAIRO_BEGIN

    FN_CHECK

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %s  ", __FUNCTION__, state, shadow_type, width, height, detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 3);

    sanitizeSize(window, &width, &height);
    if(IS_FLAT_BGND(opts.bgndAppearance) || !(widget && drawWindowBgnd(cr, style, area, widget, x, y, width, height)))
    {
//         gtk_style_apply_default_background(style, window, widget && !qtcWidgetNoWindow(widget), state,
//                                            area, x, y, width, height);
        if(widget && IMG_NONE!=opts.bgndImage.type)
            drawWindowBgnd(cr, style, area, widget, x, y, width, height);
    }

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

        gtkDrawFlatBox(style, WINDOW_PARAM_VAL state, shadow_type, AREA_PARAM_VAL widget, QTC_PANED, x, y, width, height);

        switch(opts.splitters)
        {
            case LINE_1DOT:
                drawDot(cr, x, y, width, height, cols);
                break;
            case LINE_NONE:
                break;
            case LINE_DOTS:
            default:
                drawDots(cr, x, y, width, height, height>width, NUM_SPLITTER_DASHES, 1,
                         cols, area, 0, 5);
                break;
            case LINE_FLAT:
            case LINE_SUNKEN:
            case LINE_DASHES:
                drawLines(cr, x, y, width, height, height>width, NUM_SPLITTER_DASHES, 2,
                          cols, area, 3, opts.splitters);
        }
    }
    /* Note: I'm not sure why the 'widget && GTK_IS_HANDLE_BOX(widget)' is in the following 'if' - its been there for a while.
             But this breaks the toolbar handles for Java Swing apps. I'm leaving it in for non Java apps, as there must've been
             a reason for it.... */
    else if((DETAIL("handlebox") && (GTK_APP_JAVA==qtSettings.app || (widget && GTK_IS_HANDLE_BOX(widget)))) ||
            DETAIL("dockitem") || paf)
    {
        if(widget && GTK_STATE_INSENSITIVE!=state)
            state=qtcWidgetState(widget);

        if(paf)  /* The paf here is expected to be on the gnome panel */
            if(height<width)
                y++;
            else
                x++;
        else
            gtkDrawBox(style, WINDOW_PARAM_VAL state, shadow_type, AREA_PARAM_VAL widget, "handlebox", x, y, width, height);

        switch(opts.handles)
        {
            case LINE_1DOT:
                drawDot(cr, x, y, width, height, qtcPalette.background);
                break;
            case LINE_NONE:
                break;
            case LINE_DOTS:
                drawDots(cr, x, y, width, height, height<width, 2, 5, qtcPalette.background, AREA_PARAM_VAL_L, 2, 5);
                break;
            case LINE_DASHES:
                if(height>width)
                    drawLines(cr, x+3, y, 3, height, TRUE, (height-8)/2, 0, qtcPalette.background, AREA_PARAM_VAL_L, 5, opts.handles);
                else
                    drawLines(cr, x, y+3, width, 3, FALSE, (width-8)/2, 0, qtcPalette.background, AREA_PARAM_VAL_L, 5, opts.handles);
                break;
            case LINE_FLAT:
                drawLines(cr, x, y, width, height, height<width, 2, 4, qtcPalette.background, AREA_PARAM_VAL_L, 4, opts.handles);
                break;
            default:
                drawLines(cr, x, y, width, height, height<width, 2, 4, qtcPalette.background, AREA_PARAM_VAL_L, 3, opts.handles);
        }
    }
    CAIRO_END
}

static void drawPolygon(WINDOW_PARAM GtkStyle *style, GdkColor *col, GdkRectangle *area, GdkPoint *points, int npoints,
                        gboolean fill)
{
#if (defined QTC_USE_CAIRO_FOR_ARROWS) || GTK_CHECK_VERSION(2, 90, 0)
    CAIRO_BEGIN
    int               i;
    cairo_antialias_t aa=cairo_get_antialias(cr);
    setCairoClipping(cr, area);
    cairo_set_source_rgb(cr, CAIRO_COL(*col));
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
    cairo_move_to(cr, points[0].x+0.5, points[0].y+0.5);
    for(i=0; i<npoints; ++i)
        cairo_line_to(cr, points[i].x+0.5, points[i].y+0.5);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    if(fill)
        cairo_fill(cr);
    cairo_set_antialias(cr, aa);
    unsetCairoClipping(cr);
    CAIRO_END
#else
    QtCurveStyle *qtcurveStyle = (QtCurveStyle *)style;

    if(!qtcurveStyle->arrow_gc)
    {
        qtcurveStyle->arrow_gc=gdk_gc_new(window);
        g_object_ref(qtcurveStyle->arrow_gc);
    }

    gdk_rgb_find_color(style->colormap, col);
    gdk_gc_set_foreground(qtcurveStyle->arrow_gc, col);

    if(area)
        gdk_gc_set_clip_rectangle(qtcurveStyle->arrow_gc, area);

    gdk_draw_polygon(window, qtcurveStyle->arrow_gc, FALSE, points, npoints);
    if(fill)
        gdk_draw_polygon(window, qtcurveStyle->arrow_gc, TRUE, points, npoints);

    if(area)
        gdk_gc_set_clip_rectangle(qtcurveStyle->arrow_gc, NULL);
#endif
}

static void drawArrow(WINDOW_PARAM GtkStyle *style, GdkColor *col, GdkRectangle *area, GtkArrowType arrow_type,
                      gint x, gint y, gboolean small, gboolean fill)
{
    if(small)
        switch(arrow_type)
        {
            case GTK_ARROW_UP:
            {
                GdkPoint a[]={{x+2,y},  {x,y-2},  {x-2,y},   {x-2,y+1}, {x,y-1}, {x+2,y+1}};
                drawPolygon(WINDOW_PARAM_VAL style, col, area, a, opts.vArrows ? 6 : 3, fill);
                break;
            }
            case GTK_ARROW_DOWN:
            {
                GdkPoint a[]={{x+2,y},  {x,y+2},  {x-2,y},   {x-2,y-1}, {x,y+1}, {x+2,y-1}};
                drawPolygon(WINDOW_PARAM_VAL style, col, area, a, opts.vArrows ? 6 : 3, fill);
                break;
            }
            case GTK_ARROW_RIGHT:
            {
                GdkPoint a[]={{x,y-2},  {x+2,y},  {x,y+2},   {x-1,y+2}, {x+1,y}, {x-1,y-2}};
                drawPolygon(WINDOW_PARAM_VAL style, col, area, a, opts.vArrows ? 6 : 3, fill);
                break;
            }
            case GTK_ARROW_LEFT:
            {
                GdkPoint a[]={{x,y-2},  {x-2,y},  {x,y+2},   {x+1,y+2}, {x-1,y}, {x+1,y-2}};
                drawPolygon(WINDOW_PARAM_VAL style, col, area, a, opts.vArrows ? 6 : 3, fill);
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
                GdkPoint a[]={{x+3,y+1},  {x,y-2},  {x-3,y+1},    {x-3, y+2},  {x-2, y+2}, {x,y},  {x+2, y+2}, {x+3,y+2}};
                drawPolygon(WINDOW_PARAM_VAL style, col, area, a, opts.vArrows ? 8 : 3, fill);
                break;
            }
            case GTK_ARROW_DOWN:
            {
                GdkPoint a[]={{x+3,y-1},  {x,y+2},  {x-3,y-1},   {x-3,y-2},  {x-2, y-2}, {x,y}, {x+2, y-2}, {x+3,y-2}};
                drawPolygon(WINDOW_PARAM_VAL style, col, area, a, opts.vArrows ? 8 : 3, fill);
                break;
            }
            case GTK_ARROW_RIGHT:
            {
                GdkPoint a[]={{x-1,y+3},  {x+2,y},  {x-1,y-3},   {x-2,y-3}, {x-2, y-2},  {x,y}, {x-2, y+2},  {x-2,y+3}};
                drawPolygon(WINDOW_PARAM_VAL style, col, area, a, opts.vArrows ? 8 : 3, fill);
                break;
            }
            case GTK_ARROW_LEFT:
            {
                GdkPoint a[]={{x+1,y-3},  {x-2,y},  {x+1,y+3},   {x+2,y+3}, {x+2, y+2},  {x,y}, {x+2, y-2},  {x+2,y-3}};
                drawPolygon(WINDOW_PARAM_VAL style, col, area, a, opts.vArrows ? 8 : 3, fill);
                break;
            }
            default:
                return;
        }
}

static void gtkDrawArrow(GtkStyle *style, WINDOW_PARAM GtkStateType state, GtkShadowType shadow, AREA_PARAM
                         GtkWidget *widget, const gchar *detail, GtkArrowType arrow_type, gboolean fill, gint x, gint y,
                         gint width, gint height)
{
    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %d %s  ", __FUNCTION__, state, shadow, arrow_type, x, y, width, height,
                                           detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 3);

    if(DETAIL("arrow"))
    {
        gboolean onComboEntry=isOnComboEntry(widget, 0);

        if(isOnComboBox(widget, 0) && !onComboEntry)
        {
            GdkColor *arrowColor=MO_ARROW(false, &qtSettings.colors[GTK_STATE_INSENSITIVE==state
                                                                            ? PAL_DISABLED : PAL_ACTIVE]
                                                                       [COLOR_BUTTON_TEXT]);
            //gboolean moz=isMozilla() && widget && qtcWidgetGetParent(widget) && qtcWidgetGetParent(widget)->parent && qtcWidgetGetParent(widget)->parent->parent &&
            //             isFixedWidget(qtcWidgetGetParent(widget)->parent->parent);
            x++;

#if !GTK_CHECK_VERSION(2, 90, 0)
            // NOTE: Dont do this for moz - as looks odd fir widgets in HTML pages - arrow is shifted too much :-(
            if(!DO_EFFECT) // || moz)
                x+=2;
#endif

            if(opts.doubleGtkComboArrow)
            {
                int pad=opts.vArrows ? 0 : 1;
                drawArrow(WINDOW_PARAM_VAL style, arrowColor, AREA_PARAM_VAL_L,  GTK_ARROW_UP,
                          x+(width>>1), y+(height>>1)-(LARGE_ARR_HEIGHT-pad), FALSE, TRUE);
                drawArrow(WINDOW_PARAM_VAL style, arrowColor, AREA_PARAM_VAL_L,  GTK_ARROW_DOWN,
                          x+(width>>1), y+(height>>1)+(LARGE_ARR_HEIGHT-pad), FALSE, TRUE);
            }
            else
                drawArrow(WINDOW_PARAM_VAL style, arrowColor, AREA_PARAM_VAL_L,  GTK_ARROW_DOWN, x+(width>>1), y+(height>>1), FALSE, TRUE);
        }
        else
        {
            GdkColor *col=onComboEntry || isOnCombo(widget, 0) || isOnListViewHeader(widget, 0) ||
                                                  isOnButton(widget, 0, 0L)
                                            ? &qtSettings.colors[GTK_STATE_INSENSITIVE==state ? PAL_DISABLED : PAL_ACTIVE]
                                                                [COLOR_BUTTON_TEXT]
                                            : &style->text[ARROW_STATE(state)];
            if(onComboEntry && GTK_STATE_ACTIVE==state && opts.unifyCombo)
                x--, y--;
            drawArrow(WINDOW_PARAM_VAL style, MO_ARROW(false, col), AREA_PARAM_VAL_L,  arrow_type, x+(width>>1), y+(height>>1), FALSE, TRUE);
        }
    }
    else
    {
        int      isSpinButton = DETAIL("spinbutton"),
                 isMenuItem = DETAIL("menuitem"),
                 a_width=LARGE_ARR_WIDTH,
                 a_height=LARGE_ARR_HEIGHT;
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
        if(isSpinButton)
        {
//             if(GTK_ARROW_UP==arrow_type)
//                 y++;
            a_height = SMALL_ARR_HEIGHT;
            a_width = SMALL_ARR_WIDTH;
        }
        else if(GTK_ARROW_LEFT==arrow_type || GTK_ARROW_RIGHT==arrow_type || DETAIL("menuitem"))
        {
            a_width = LARGE_ARR_HEIGHT;
            a_height = LARGE_ARR_WIDTH;

            if(isMozilla() && opts.vArrows && a_height && height<a_height)
                smallArrows=true;
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

        if(sbar)
            switch(stepper)
            {
                case STEPPER_B:
                    if(opts.flatSbarButtons || !opts.vArrows)
                        if(GTK_ARROW_RIGHT==arrow_type)
                            x--;
                        else
                            y--;
                    break;
                case STEPPER_C:
                    if(opts.flatSbarButtons || !opts.vArrows)
                        if(GTK_ARROW_LEFT==arrow_type)
                            x++;
                        else
                            y++;
                default:
                    break;
            }

        if(isSpinButton && isFixedWidget(widget) && (isMozilla() || GTK_APP_OPEN_OFFICE==qtSettings.app))
            x--;

        if(isSpinButton && !DO_EFFECT)
            y+=(GTK_ARROW_UP==arrow_type ? -1 : 1);

        if(opts.unifySpin && isSpinButton && !opts.vArrows && GTK_ARROW_DOWN==arrow_type)
            y--;
 
        if(GTK_STATE_ACTIVE==state && (sbar  || isSpinButton) && MO_GLOW==opts.coloredMouseOver)
            state=GTK_STATE_PRELIGHT;

        if(isMenuItem && GTK_ARROW_RIGHT==arrow_type && !isMozilla() && GTK_APP_OPEN_OFFICE!=qtSettings.app)
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

        drawArrow(WINDOW_PARAM_VAL style, MO_ARROW(isMenuItem, col), AREA_PARAM_VAL_L, arrow_type, x, y, smallArrows, TRUE);
        }
    }
}

static void drawBox(GtkStyle *style, WINDOW_PARAM GtkStateType state, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget,
                    const gchar *detail, gint x, gint y, gint width, gint height, gboolean btn_down)
{
    gboolean sbar=isSbarDetail(detail),
             pbar=DETAIL("bar"), //  && GTK_IS_PROGRESS_BAR(widget),
             qtc_paned=!pbar && IS_QTC_PANED,
             qtcSlider=!qtc_paned && DETAIL("qtc-slider"),
             slider=qtcSlider || (!qtc_paned && DETAIL("slider")),
             hscale=!slider && DETAIL("hscale"),
             vscale=!hscale && DETAIL("vscale"),
             menubar=!vscale && DETAIL("menubar"),
             menuitem=!menubar && DETAIL("menuitem"),
             button=!menuitem && DETAIL("button"),
             togglebutton=!button && DETAIL("togglebutton"),
             optionmenu=!togglebutton && DETAIL("optionmenu"),
             stepper=!optionmenu && DETAIL("stepper"),
             checkbox=!stepper && DETAIL(QTC_CHECKBOX),
             vscrollbar=!checkbox && detail && detail==strstr(detail, "vscrollbar"),
             hscrollbar=!vscrollbar && detail && detail==strstr(detail, "hscrollbar"),
             spinUp=!hscrollbar && DETAIL("spinbutton_up"),
             spinDown=!spinUp && DETAIL("spinbutton_down"),
             menuScroll=detail && NULL!=strstr(detail, "menu_scroll_arrow_"),
             rev=reverseLayout(widget) || (widget && reverseLayout(qtcWidgetGetParent(widget))),
             activeWindow=TRUE;
    GdkColor new_cols[TOTAL_SHADES+1],
             *btn_colors;
    int      bgnd=getFill(state, btn_down/*, DETAIL(QTC_CHECKBOX)*/),
             round=getRound(detail, widget, x, y, width, height, rev);
    gboolean lvh=isListViewHeader(widget) || isEvolutionListViewHeader(widget, detail),
             sunken=btn_down || (GTK_IS_BUTTON(widget) && qtcButtonIsDepressed(widget)) ||
                    GTK_STATE_ACTIVE==state || (2==bgnd || 3==bgnd);

    if(button && GTK_IS_TOGGLE_BUTTON(widget))
    {
        button=FALSE;
        togglebutton=TRUE;
    }

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %d %s  ", __FUNCTION__, btn_down, state, shadow_type, x, y, width, height,
                                           detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 3);

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
            SET_BTN_COLS(slider, hscale|vscale, lvh, state)
    }

    g_return_if_fail(style != NULL);

    if(menubar && !isMozilla() && GTK_APP_JAVA!=qtSettings.app && opts.shadeMenubarOnlyWhenActive)
    {
        GtkWindow *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(widget));
                
        if(topLevel && GTK_IS_WINDOW(topLevel))
        {
            #define SHADE_ACTIVE_MB_HACK_SET "QTC_SHADE_ACTIVE_MB_HACK_SET"
            if (!g_object_get_data(G_OBJECT(topLevel), SHADE_ACTIVE_MB_HACK_SET))
            {
                g_object_set_data(G_OBJECT(topLevel), SHADE_ACTIVE_MB_HACK_SET, (gpointer)1);
                g_signal_connect(G_OBJECT(topLevel), "event", G_CALLBACK(windowEvent), widget);
            }
            activeWindow=qtcWindowIsActive(GTK_WIDGET(topLevel));
        }
    }

    if (opts.menubarMouseOver && GE_IS_MENU_SHELL(widget) && !isMozilla())
        qtcMenuShellSetup(widget);

    CAIRO_BEGIN
    if(spinUp || spinDown)
    {
        if(!opts.unifySpin && (!opts.unifySpinBtns || sunken/* || GTK_STATE_PRELIGHT==state*/))
        {
            EWidget      wid=spinUp ? WIDGET_SPIN_UP : WIDGET_SPIN_DOWN;
            GdkRectangle *a=area,
                         b,
                         unified;
            gboolean     ooOrMoz=GTK_APP_OPEN_OFFICE==qtSettings.app || isMozilla();

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

            drawBgnd(cr, &btn_colors[bgnd], widget, area, x+1, y+1, width-2, height-2);
            drawLightBevel(cr, style, state, area, x, y, width, height-(WIDGET_SPIN_UP==wid && DO_EFFECT ? 1 : 0), &btn_colors[bgnd],
                           btn_colors, round, wid, BORDER_FLAT, DF_DO_CORNERS|DF_DO_BORDER|(sunken ? DF_SUNKEN : 0), widget);
        }
    }
    else if(DETAIL("spinbutton"))
    {
        if(IS_FLAT_BGND(opts.bgndAppearance) || !(widget && drawWindowBgnd(cr, style, area, widget, x, y, width, height)))
        {
            qtcStyleApplyDefBgnd(widget && !qtcWidgetNoWindow(widget),
                                 GTK_STATE_INSENSITIVE==state ? GTK_STATE_INSENSITIVE : GTK_STATE_NORMAL);
            if(widget && IMG_NONE!=opts.bgndImage.type)
                drawWindowBgnd(cr, style, area, widget, x, y, width, height);
        }

        if(opts.unifySpin)
        {
            gboolean rev=reverseLayout(widget) || (widget && reverseLayout(qtcWidgetGetParent(widget))),
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
            drawEntryField(cr, style, state, widget, area, x, y, width, height, rev ? ROUNDED_LEFT : ROUNDED_RIGHT, WIDGET_SPIN);
#if !GTK_CHECK_VERSION(2, 90, 0)
            if(moz)
#endif
                unsetCairoClipping(cr);
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
            drawLightBevel(cr, style, state, area, x, y+offset, width-offset, height-(2*offset), &btn_colors[bgnd],
                           btn_colors, ROUNDED_RIGHT, WIDGET_SPIN, BORDER_FLAT,
                           DF_DO_CORNERS|DF_DO_BORDER|(sunken ? DF_SUNKEN : 0), widget);
            drawFadedLine(cr, x+2, y+(height>>1), width-(offset+4), 1, &btn_colors[STD_BORDER], area, NULL, TRUE, TRUE, TRUE);
        }

    }
    else if(!opts.stdSidebarButtons && (button || togglebutton) && isSideBarBtn(widget))
    {     
        if(GTK_STATE_PRELIGHT==state || sunken || GTK_STATE_ACTIVE==state)
        {
            gboolean horiz=width>height;
            GdkColor *cols=GTK_STATE_ACTIVE==state ? qtcPalette.sidebar : qtcPalette.background;
            drawLightBevel(cr, style, state, area, x, y, width, height, &cols[bgnd], cols, ROUNDED_NONE, WIDGET_MENU_ITEM,
                           BORDER_FLAT, (horiz ? 0 : DF_VERT)|(sunken ? DF_SUNKEN : 0), widget);

            if(opts.coloredMouseOver && GTK_STATE_PRELIGHT==state)
            {
                GdkColor *col=&qtcPalette.mouseover[1];

                if(horiz || MO_PLASTIK!=opts.coloredMouseOver)
                {
                    cairo_new_path(cr);
                    cairo_set_source_rgb(cr, CAIRO_COL(*col));
                    cairo_move_to(cr, x, y+0.5);
                    cairo_line_to(cr, x+width-1, y+0.5);
                    cairo_move_to(cr, x+1, y+1.5);
                    cairo_line_to(cr, x+width-2, y+1.5);
                    cairo_stroke(cr);
                }

                if(!horiz || MO_PLASTIK!=opts.coloredMouseOver)
                {
                    cairo_new_path(cr);
                    cairo_set_source_rgb(cr, CAIRO_COL(*col));
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
                    cairo_set_source_rgb(cr, CAIRO_COL(*col));
                    cairo_move_to(cr, x, y+height-1.5);
                    cairo_line_to(cr, x+width-1, y+height-1.5);
                    cairo_move_to(cr, x+1, y+height-2.5);
                    cairo_line_to(cr, x+width-2, y+height-2.5);
                    cairo_stroke(cr);
                }

                if(!horiz || MO_PLASTIK!=opts.coloredMouseOver)
                {
                    cairo_new_path(cr);
                    cairo_set_source_rgb(cr, CAIRO_COL(*col));
                    cairo_move_to(cr, x+width-1.5, y);
                    cairo_line_to(cr, x+width-1.5, y+height-1);
                    cairo_move_to(cr, x+width-2.5, y+1);
                    cairo_line_to(cr, x+width-2.5, y+height-2);
                    cairo_stroke(cr);
                }
            }
        }
    }
    else if(detail &&( button || togglebutton || optionmenu || checkbox || sbar || hscale || vscale ||
                       stepper || slider || qtc_paned))
    {
        gboolean combo=0==strcmp(detail, "optionmenu") || isOnComboBox(widget, 0),
                 combo_entry=combo && isOnComboEntry(widget, 0),
                 horiz_tbar,
                 tbar_button=isButtonOnToolbar(widget, &horiz_tbar),
                 handle_button=!tbar_button && isButtonOnHandlebox(widget, &horiz_tbar);
        int      xAdjust=0, yAdjust=0, wAdjust=0, hAdjust=0;

//        drawBgnd(cr, &btn_colors[bgnd], widget, area, x, y, width, height); // CPD removed as it messes up toolbars and firefox3

        if(combo && !sunken && isActiveCombo(widget))
        {
            sunken=TRUE;
            bgnd=4;
        }

        if(tbar_button && TBTN_JOINED==opts.tbarBtns)
        {
            adjustToolbarButtons(widget, &xAdjust, &yAdjust, &wAdjust, &hAdjust, &round, horiz_tbar);
            x+=xAdjust, y+=yAdjust, width+=wAdjust, height+=hAdjust;
        }

        if(!qtc_paned)
        {
            gboolean horiz=(tbar_button || handle_button) && IS_GLASS(opts.appearance) &&
                           IS_GLASS(opts.toolbarAppearance)
                               ? horiz_tbar
                               : (slider && width<height) || vscrollbar || vscale
                                   ? FALSE
                                   : TRUE,
                     defBtn=GTK_STATE_INSENSITIVE!=state && (button || togglebutton) && widget && qtcWidgetHasDefault(widget);

            if(lvh)
            {
                drawBevelGradient(cr, style, area, x, y, width, height, &btn_colors[bgnd],
                                  horiz, sunken, opts.lvAppearance, WIDGET_LISTVIEW_HEADER);

                if(APPEARANCE_RAISED==opts.lvAppearance)
                    drawHLine(cr, CAIRO_COL(qtcPalette.background[4]), 1.0, x, y+height-2, width);
                drawHLine(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]), 1.0, x, y+height-1, width);
                    
                if(GTK_STATE_PRELIGHT==state && opts.coloredMouseOver)
                    drawHighlight(cr, x, y+height-2, width, 2, area, true, true);

#if GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
                drawFadedLine(cr, x+width-2, y+4, 1, height-8, &btn_colors[STD_BORDER], area, NULL, TRUE, TRUE, FALSE);
                drawFadedLine(cr, x+width-1, y+4, 1, height-8, &btn_colors[0], area, NULL, TRUE, TRUE, FALSE);
#else
                if(x>3 && height>10)
                {
                    drawFadedLine(cr, x, y+4, 1, height-8, &btn_colors[STD_BORDER], area, NULL, TRUE, TRUE, FALSE);
                    drawFadedLine(cr, x+1, y+4, 1, height-8, &btn_colors[0], area, NULL, TRUE, TRUE, FALSE);
                }
#endif
            }
            else if(isPathButton(widget))
            {
                if(GTK_STATE_PRELIGHT==state)
                    drawSelection(cr, style, state, area, widget, x, y, width, height, ROUNDED_ALL, FALSE);

                if(GTK_IS_TOGGLE_BUTTON(widget))
                {                       
                    drawArrow(WINDOW_PARAM_VAL style, &qtcPalette.background[5], area, GTK_ARROW_RIGHT,
                              x+width-((LARGE_ARR_WIDTH>>1)+4), y+((height-(LARGE_ARR_HEIGHT>>1))>>1)+1, FALSE, TRUE);
//                     cairo_new_path(cr);
//                     cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.background[5]));
//                     cairo_move_to(cr, x+width-0.5, y+9.5);
//                     cairo_line_to(cr, x+width-3.5, y+height-9.5);
//                     cairo_stroke(cr);
                }
            }
// Re-enable this to stop the button background being drawn for tab widget icons.
//             else if (isTabButton(widget))
//             {
//                 ;
//             }
            else
            {
                /* Yuck this is a horrible mess!!!!! */
                gboolean glowFocus=widget && qtcWidgetHasFocus(widget) && MO_GLOW==opts.coloredMouseOver && FULL_FOCUS;
                EWidget  widgetType=isComboBoxButton(widget)
                                    ? WIDGET_COMBO_BUTTON
                                    : slider
                                        ? qtcSlider ? WIDGET_SLIDER : WIDGET_SB_SLIDER
                                        : hscale||vscale
                                        ? WIDGET_SLIDER
                                            : lvh
                                                ? WIDGET_LISTVIEW_HEADER
                                                : combo || optionmenu
                                                    ? WIDGET_COMBO
                                                    : tbar_button
                                                        ? (opts.coloredTbarMo ? WIDGET_TOOLBAR_BUTTON : WIDGET_UNCOLOURED_MO_BUTTON)
                                                        : togglebutton
                                                            ? (glowFocus && !sunken ? WIDGET_DEF_BUTTON : WIDGET_TOGGLE_BUTTON)
                                                            : checkbox
                                                                ? WIDGET_CHECKBOX
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
                    GTK_IS_BUTTON(widget) && DETAIL("button") && ((width>22 && width<56 && height>30) || height>32))
                    widgetType=opts.coloredTbarMo ? WIDGET_TOOLBAR_BUTTON : WIDGET_UNCOLOURED_MO_BUTTON;

                if(ROUND_MAX==opts.round &&
                    ( (WIDGET_TOGGLE_BUTTON==widgetType && height>(opts.crSize+8) && width<(height+10)) ||
                      (GTK_APP_GIMP==qtSettings.app && WIDGET_STD_BUTTON==widgetType && WIDGET_TYPE_NAME("GimpViewableButton")) ||
                      (opts.stdSidebarButtons && WIDGET_STD_BUTTON==widgetType && widget && isSideBarBtn(widget)) ) )
                    widgetType=WIDGET_TOOLBAR_BUTTON;

                /* For some reason SWT combo's dont un-prelight when activated! So dont pre-light at all! */
/*
                if(GTK_APP_JAVA_SWT==qtSettings.app && WIDGET_STD_BUTTON==widgetType && GTK_STATE_PRELIGHT==state && WIDGET_COMBO==widgetType)
                {
                    state=GTK_STATE_NORMAL;
                    bgnd=getFill(state, btn_down);
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
                    GtkAllocation alloc         = qtcWidgetGetAllocation(widget);
                    gboolean      horizontal    = qtcRangeGetOrientation(widget) != GTK_ORIENTATION_HORIZONTAL;
                    int           sbarTroughLen = (horizontal ? alloc.height : alloc.width)-
                                                  ( (qtcRangeHasStepperA(widget) ? opts.sliderWidth : 0)+
                                                    (qtcRangeHasStepperB(widget) ? opts.sliderWidth : 0)+
                                                    (qtcRangeHasStepperC(widget) ? opts.sliderWidth : 0)+
                                                    (qtcRangeHasStepperD(widget) ? opts.sliderWidth : 0)),
                                  sliderLen     = (horizontal ? height : width);

                    if(sbarTroughLen==sliderLen)
                    {
                        state=GTK_STATE_INSENSITIVE;
                        btn_colors=qtcPalette.background;
                        bgnd=getFill(state, FALSE);
                    }
                }
#ifdef INCREASE_SB_SLIDER
                if(slider && widget && GTK_IS_RANGE(widget) && !opts.flatSbarButtons && SCROLLBAR_NONE!=opts.scrollbarType
                   /*&& !(GTK_STATE_PRELIGHT==state && MO_GLOW==opts.coloredMouseOver)*/)
                {
                    GtkAdjustment *adj       = qtcRangeGetAdjustment(GTK_RANGE(widget));
                    gboolean      horizontal = GTK_ORIENTATION_HORIZONTAL==qtcRangeGetOrientation(widget),
#if GTK_CHECK_VERSION(2, 90, 0)
                                  hasStartStepper = SCROLLBAR_PLATINUM!=opts.scrollbarType,
                                  hasEndStepper   = SCROLLBAR_NEXT!=opts.scrollbarType,
#else
                                  hasStartStepper = qtcRangeHasStepperA(widget) || qtcRangeHasStepperB(widget),
                                  hasEndStepper   = qtcRangeHasStepperC(widget) || qtcRangeHasStepperD(widget),
#endif
                                  atEnd      = FALSE;
                    double        value      = qtcAdjustmentGetValue(adj);

                    if(hasStartStepper && value <= qtcAdjustmentGetLower(adj))
                    {
                        if (horizontal)
                            x--, width++;
                        else
                            y--, height++;

                        atEnd=TRUE;
                    }
                    if(hasEndStepper && value >= qtcAdjustmentGetUpper(adj) - qtcAdjustmentGetPageSize(adj))
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
                if(opts.unifyCombo && WIDGET_COMBO_BUTTON==widgetType)
                {
                    GtkWidget    *parent=widget ? qtcWidgetGetParent(widget) : NULL;
                    GtkWidget    *entry=parent ? getComboEntry(parent) : NULL;
                    GtkStateType entryState=entry ? qtcWidgetGetState(entry) : GTK_STATE_NORMAL;
                    gboolean     rev=FALSE,
                                 mozToolbar=isMozilla() && parent &&
                                            GTK_IS_TOGGLE_BUTTON(widget) &&
                                            QTC_COMBO_ENTRY(parent) &&
                                            (parent=qtcWidgetGetParent(parent)) && GTK_IS_FIXED(parent) &&
                                            (parent=qtcWidgetGetParent(parent)) && GTK_IS_WINDOW(parent) &&
                                            0==strcmp(qtcWidgetName(parent), "MozillaGtkWidget");

                    if(!entry && widget && qtcWidgetGetParent(widget))
                        entry=getMappedWidget(qtcWidgetGetParent(widget), 1);

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
                    drawEntryField(cr, style, state, entry, area, x, y, width, height, rev ? ROUNDED_LEFT : ROUNDED_RIGHT, WIDGET_COMBO_BUTTON);
                    // Get entry to redraw by setting its state...
                    // ...cant do a queue redraw, as then entry does for the button, else we get stuck in a loop!
                    if(!mozToolbar && widget && entry && entryState!=qtcWidgetGetState(widget) && GTK_STATE_INSENSITIVE!=entryState &&
                       GTK_STATE_INSENSITIVE!=state)
                        gtk_widget_set_state(entry, state);
                }
                else if(opts.flatSbarButtons && WIDGET_SB_BUTTON==widgetType)
                {
#if !GTK_CHECK_VERSION(2, 90, 0)
                   /* if(opts.gtkScrollViews && IS_FLAT(opts.sbarBgndAppearance) && 0!=opts.tabBgnd && widget && qtcWidgetGetParent(widget) && qtcWidgetGetParent(widget)->parent &&
                       GTK_IS_SCROLLED_WINDOW(qtcWidgetGetParent(widget)) && GTK_IS_NOTEBOOK(qtcWidgetGetParent(widget)->parent))
                        drawAreaModColor(cr, area, &qtcPalette.background[ORIGINAL_SHADE], TO_FACTOR(opts.tabBgnd), xo, yo, wo, ho);
                    else*/ if(IS_FLAT_BGND(opts.bgndAppearance) || !(opts.gtkScrollViews && IS_FLAT(opts.sbarBgndAppearance) &&
                                                              widget && drawWindowBgnd(cr, style, area, widget, xo, yo, wo, ho)))
                    {
                        if(!IS_FLAT(opts.sbarBgndAppearance) && SCROLLBAR_NONE!=opts.scrollbarType)
                            drawBevelGradient(cr, style, area, xo, yo, wo, ho, &qtcPalette.background[ORIGINAL_SHADE],
                                              horiz, FALSE, opts.sbarBgndAppearance, WIDGET_SB_BGND);
//                          else
//                              drawBgnd(cr, &qtcPalette.background[ORIGINAL_SHADE], widget, area, xo, yo, wo, ho);
                    }
#endif
                }
                else
                {
                    GdkColor *cols=defBtn && (IND_TINT==opts.defBtnIndicator || IND_COLORED==opts.defBtnIndicator || IND_SELECTED==opts.defBtnIndicator)
                                    ? qtcPalette.defbtn
                                    : WIDGET_COMBO_BUTTON==widgetType && qtcPalette.combobtn && GTK_STATE_INSENSITIVE!=state
                                        ? qtcPalette.combobtn
                                        : btn_colors;
                    int      bg=(WIDGET_COMBO_BUTTON==widgetType &&
                                  (SHADE_DARKEN==opts.comboBtn ||
                                      (SHADE_NONE!=opts.comboBtn && GTK_STATE_INSENSITIVE==state))) ||
                                (WIDGET_SB_SLIDER==widgetType && SHADE_DARKEN==opts.shadeSliders) ||
                                (defBtn && IND_DARKEN==opts.defBtnIndicator)
                                    ? getFillReal(state, btn_down, true) : bgnd;

                    drawLightBevel(cr, style, state, area, x, y, width, height, &cols[bg], cols, round, widgetType,
                                   BORDER_FLAT, (sunken ? DF_SUNKEN : 0)|(lvh ? 0 : DF_DO_BORDER)|(horiz ? 0 : DF_VERT), widget);

                    if(tbar_button && TBTN_JOINED==opts.tbarBtns)
                    {
                        const int constSpace=4;
                        int xo=x-xAdjust, yo=y-yAdjust, wo=width-wAdjust, ho=height-hAdjust;

                        if(xAdjust)
                            drawFadedLine(cr, xo, yo+constSpace, 1, ho-(2*constSpace), &btn_colors[0], area, NULL, TRUE, TRUE, FALSE);
                        if(yAdjust)
                            drawFadedLine(cr, xo+constSpace, yo, wo-(2*constSpace), 1, &btn_colors[0], area, NULL, TRUE, TRUE, TRUE);
                        if(wAdjust)
                            drawFadedLine(cr, xo+wo-1, yo+constSpace, 1, ho-(2*constSpace), &btn_colors[STD_BORDER], area, NULL, TRUE, TRUE, FALSE);
                        if(hAdjust)
                            drawFadedLine(cr, xo+constSpace, yo+ho-1, wo-(2*constSpace), 1, &btn_colors[STD_BORDER], area, NULL, TRUE, TRUE, TRUE);
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
                    gtkDrawSlider(lastSlider.style, lastSlider.cr, lastSlider.state, lastSlider.shadow_type, lastSlider.widget,
                                  lastSlider.detail, lastSlider.x, lastSlider.y, lastSlider.width, lastSlider.height, lastSlider.orientation);
#else
                    gtkDrawSlider(lastSlider.style, lastSlider.window, lastSlider.state, lastSlider.shadow_type, NULL, lastSlider.widget,
                                  lastSlider.detail, lastSlider.x, lastSlider.y, lastSlider.width, lastSlider.height, lastSlider.orientation);
#endif
                    lastSlider.widget=NULL;
                }
#endif
            }

            if(defBtn)
                if(IND_CORNER==opts.defBtnIndicator)
                {
                    int      offset=sunken ? 5 : 4,
                             etchOffset=DO_EFFECT ? 1 : 0;
                    GdkColor *cols=qtcPalette.focus ? qtcPalette.focus : qtcPalette.highlight,
                             *col=&cols[GTK_STATE_ACTIVE==state ? 0 : 4];

                    cairo_new_path(cr);
                    cairo_set_source_rgb(cr, CAIRO_COL(*col));
                    cairo_move_to(cr, x+offset+etchOffset, y+offset+etchOffset);
                    cairo_line_to(cr, x+offset+6+etchOffset, y+offset+etchOffset);
                    cairo_line_to(cr, x+offset+etchOffset, y+offset+6+etchOffset);
                    cairo_fill(cr);
                }
                else if(IND_COLORED==opts.defBtnIndicator && (COLORED_BORDER_SIZE>2))
                {
                    int o=COLORED_BORDER_SIZE+(DO_EFFECT ? 1 : 0); // offset needed because of etch

                    drawBevelGradient(cr, style, area, x+o, y+o, width-(2*o), height-(2*o),
                                      &btn_colors[bgnd],
                                      TRUE, GTK_STATE_ACTIVE==state, opts.appearance, WIDGET_STD_BUTTON);
                }
        }

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
                    GdkColor     *cols=qtcPalette.combobtn && GTK_STATE_INSENSITIVE!=state ? qtcPalette.combobtn : btn_colors;
                    int          bg=SHADE_DARKEN==opts.comboBtn || (GTK_STATE_INSENSITIVE==state && SHADE_NONE!=opts.comboBtn)
                                        ? getFillReal(state, btn_down, true) : bgnd;

                    btn.x=cx + (rev ? ind_width+STYLE->xthickness
                                    : (cwidth - ind_width - STYLE->xthickness)+1),
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
                        unsetCairoClipping(cr);
                }
                else if(opts.comboSplitter)
                {
                    if(sunken)
                        cx++, cy++, cheight--;

                    drawFadedLine(cr, cx + (rev ? ind_width+STYLE->xthickness : (cwidth - ind_width - STYLE->xthickness)),
                                      cy + STYLE->ythickness-1, 1, cheight-3,
                                  &btn_colors[darkLine], area, NULL, TRUE, TRUE, FALSE);

                    if(!sunken)
                        drawFadedLine(cr, cx + 1 + (rev ? ind_width+STYLE->xthickness : (cwidth - ind_width - STYLE->xthickness)),
                                          cy + STYLE->ythickness-1, 1, cheight-3,
                                      &btn_colors[0], area, NULL, TRUE, TRUE, FALSE);
                }
            }
            else if((button || togglebutton) && (combo || combo_entry))
            {
                int vx=x+(width - (1 + (combo_entry ? 24 : 20))),
                    vwidth=width-(vx-x),
                    darkLine=BORDER_VAL(GTK_STATE_INSENSITIVE!=state);

                if(rev)
                {
                    vx=x+LARGE_ARR_WIDTH;
                    if(combo_entry)
                        vx+=2;
                }

                if(DO_EFFECT)
                    vx-=2;

                if(!combo_entry)
                {
                    if(SHADE_NONE!=opts.comboBtn)
                    {
                        GdkRectangle btn;
                        GdkColor     *cols=qtcPalette.combobtn && GTK_STATE_INSENSITIVE!=state
                                        ? qtcPalette.combobtn : btn_colors;
                        int          bg=SHADE_DARKEN==opts.comboBtn ||
                                            (GTK_STATE_INSENSITIVE==state && SHADE_NONE!=opts.comboBtn)
                                        ? getFillReal(state, btn_down, true) : bgnd;
                                        
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
                            unsetCairoClipping(cr);
                    }
                    else if(opts.comboSplitter)
                    {
                        drawFadedLine(cr, vx+(rev ? LARGE_ARR_WIDTH+4 : 0), y+4, 1, height-8,
                                      &btn_colors[darkLine], area, NULL, TRUE, TRUE, FALSE);

                        if(!sunken)
                            drawFadedLine(cr, vx+1+(rev ? LARGE_ARR_WIDTH+4 : 0), y+4, 1, height-8,
                                          &btn_colors[0], area, NULL, TRUE, TRUE, FALSE);
                    }
                }
            }
        }
    }
    else if(detail && (0==strcmp(detail, "buttondefault") ||
                       0==strcmp(detail, "togglebuttondefault")))
    {
    }
    else if(widget && detail && (0==strcmp(detail, "trough") || detail==strstr(detail, "trough-")))
    {
        gboolean list=isList(widget),
                 pbar=list || GTK_IS_PROGRESS_BAR(widget),
                 scale=!pbar && GTK_IS_SCALE(widget);
        int      border=BORDER_VAL(GTK_STATE_INSENSITIVE!=state || !scale);
        GdkColor *bgndcols=qtcPalette.background,
                 *bgndcol=&bgndcols[2];
        gboolean horiz=GTK_IS_RANGE(widget)
                        ? GTK_ORIENTATION_HORIZONTAL==qtcRangeGetOrientation(widget)
                        : width>height;

        if(scale)
        {
            GtkAdjustment *adjustment = gtk_range_get_adjustment(GTK_RANGE(widget));
            double        upper=qtcAdjustmentGetUpper(adjustment),
                          lower=qtcAdjustmentGetLower(adjustment),
                          value=qtcAdjustmentGetValue(adjustment);
            int           used_x=x, used_y=y, used_h=0, used_w=0,
                          pos=(int)(((double)(horiz ? width : height) /
                                    (upper - lower))  *
                                    (value - lower));
            gboolean      inverted=gtk_range_get_inverted(GTK_RANGE(widget)),
                          doEtch=DO_EFFECT;
            int           troughSize=SLIDER_TROUGH_SIZE+(doEtch ? 2 : 0);
            GdkColor      *usedcols=opts.fillSlider && upper!=lower && state!=GTK_STATE_INSENSITIVE
                                    ? qtcPalette.slider
                                        ? qtcPalette.slider
                                        : qtcPalette.highlight
                                    : qtcPalette.background;
            EWidget       wid=WIDGET_SLIDER_TROUGH;

            if(horiz && rev)
                inverted=!inverted;

//             if((!widget || !g_object_get_data(G_OBJECT (widget), "transparent-bg-hint")) &&
//                 IS_FLAT_BGND(opts.bgndAppearance) || !(widget && drawWindowBgnd(cr, style, area, widget, x, y, width, height)))
//             {
//                 gtk_style_apply_default_background(style, window, widget && !qtcWidgetNoWindow(widget),
//                                                    GTK_STATE_INSENSITIVE==state
//                                                         ? GTK_STATE_INSENSITIVE
//                                                         : GTK_STATE_NORMAL,
//                                                    area, x, y, width, height);
//                 if(widget && IMG_NONE!=opts.bgndImage.type)
//                     drawWindowBgnd(cr, style, area, widget, x, y, width, height);
//             }

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
                bgndcol=&bgndcols[ORIGINAL_SHADE];
            else if (0==strcmp(detail, "trough-lower") && opts.fillSlider)
            {
                bgndcols=usedcols;
                bgndcol=&usedcols[ORIGINAL_SHADE];
                wid=WIDGET_FILLED_SLIDER_TROUGH;
            }
            drawLightBevel(cr, style, state, area, x, y, width, height,
                           bgndcol, bgndcols, opts.square&SQUARE_SLIDER ? ROUNDED_NONE : ROUNDED_ALL, wid,
                           BORDER_FLAT, DF_DO_CORNERS|DF_SUNKEN|DF_DO_BORDER|(horiz ? 0 : DF_VERT), widget);

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
                                   BORDER_FLAT, DF_DO_CORNERS|DF_SUNKEN|DF_DO_BORDER|(horiz ? 0 : DF_VERT), widget);
                }
            }
        }
        else if(pbar)
        {
            gboolean doEtch=!list && DO_EFFECT;
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

            if(!list && (IS_FLAT_BGND(opts.bgndAppearance) || !(widget &&
                                                           drawWindowBgnd(cr, style, area, widget, x, y, width, height)))
                && (!widget || !g_object_get_data(G_OBJECT (widget), "transparent-bg-hint")))
                drawAreaColor(cr, area, &qtcPalette.background[ORIGINAL_SHADE], x, y, width, height);

            if(doEtch && opts.borderProgress)
                x++, y++, width-=2, height-=2;

            /*clipPath(cr, x, y, width, height, WIDGET_PBAR_TROUGH, RADIUS_INTERNAL, ROUNDED_ALL);*/
            drawBevelGradient(cr, style, area, x+offset, y+offset, width-(2*offset), height-(2*offset), col,
                              horiz, FALSE, opts.progressGrooveAppearance, WIDGET_PBAR_TROUGH);
            /*unsetCairoClipping(cr);*/

            if(doEtch && opts.borderProgress)
                 drawEtch(cr, area, widget, x-1, y-1, width+2, height+2, FALSE, ROUNDED_ALL, WIDGET_PBAR_TROUGH);

            if(opts.borderProgress)
            {
                GtkWidget *parent=widget ? qtcWidgetGetParent(widget) : NULL;
                GtkStyle  *style=widget ? qtcWidgetGetStyle(parent ? parent : widget) : NULL;
                drawBorder(cr, style, state, area, x, y, width, height, NULL, ROUNDED_ALL,
                           IS_FLAT(opts.progressGrooveAppearance) && ECOLOR_DARK!=opts.progressGrooveColor
                                ? BORDER_SUNKEN : BORDER_FLAT,
                           WIDGET_PBAR_TROUGH, DF_BLEND|DF_DO_CORNERS);
            }
            else if(!opts.borderProgress)
                if(horiz)
                {
                    drawHLine(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]), 1.0, x, y, width);
                    drawHLine(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]), 1.0, x, y+height-1, width);
                }
                else
                {
                    drawVLine(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]), 1.0, x, y, height);
                    drawVLine(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]), 1.0, x+width-1, y, height);
                }
        }
        else /* Scrollbars... */
        {
            int      sbarRound=ROUNDED_ALL,
                     xo=x, yo=y, wo=width, ho=height;
            gboolean drawBg=opts.flatSbarButtons/* && !IS_FLAT(opts.sbarBgndAppearance) && SCROLLBAR_NONE!=opts.scrollbarType*/,
                     thinner=opts.thinSbarGroove && (SCROLLBAR_NONE==opts.scrollbarType || opts.flatSbarButtons);

            if(opts.flatSbarButtons)
            {
#if GTK_CHECK_VERSION(2, 90, 0)
                gboolean lower=detail && strstr(detail, "-lower");
                sbarRound=lower
                            ? horiz
                                ? ROUNDED_LEFT
                                : ROUNDED_TOP
                            : horiz
                                ? ROUNDED_RIGHT
                                : ROUNDED_BOTTOM;

                switch(opts.scrollbarType)
                {
                    case SCROLLBAR_KDE:
                        if(lower)
                        {
                            if(horiz)
                                x+=SBAR_BTN_SIZE;
                            else
                                y+=SBAR_BTN_SIZE;
                        }
                        else
                        {
                            if(horiz)
                                width-=SBAR_BTN_SIZE*2;
                            else
                                height-=SBAR_BTN_SIZE*2;
                        }
                        break;
                    case SCROLLBAR_WINDOWS:
                        if(lower)
                        {
                            if(horiz)
                                x+=SBAR_BTN_SIZE;
                            else
                                y+=SBAR_BTN_SIZE;
                        }
                        else
                        {
                            if(horiz)
                                width-=SBAR_BTN_SIZE;
                            else
                                height-=SBAR_BTN_SIZE;
                        }
                        break;
                    case SCROLLBAR_NEXT:
                        if(lower)
                        {
                            if(horiz)
                                x+=SBAR_BTN_SIZE*2;
                            else
                                y+=SBAR_BTN_SIZE*2;
                        }
                        break;
                    case SCROLLBAR_PLATINUM:
                        if(!lower)
                        {
                            if(horiz)
                                width-=SBAR_BTN_SIZE*2;
                            else
                                height-=SBAR_BTN_SIZE*2;
                        }
                        break;
                }
#else
                switch(opts.scrollbarType)
                {
                    case SCROLLBAR_KDE:
                        if(horiz)
                            x+=SBAR_BTN_SIZE, width-=SBAR_BTN_SIZE*3;
                        else
                            y+=SBAR_BTN_SIZE, height-=SBAR_BTN_SIZE*3;
                        break;
                    case SCROLLBAR_WINDOWS:
                        if(horiz)
                            x+=SBAR_BTN_SIZE, width-=SBAR_BTN_SIZE*2;
                        else
                            y+=SBAR_BTN_SIZE, height-=SBAR_BTN_SIZE*2;
                        break;
                    case SCROLLBAR_NEXT:
                        if(horiz)
                            x+=SBAR_BTN_SIZE*2, width-=SBAR_BTN_SIZE*2;
                        else
                            y+=SBAR_BTN_SIZE*2, height-=SBAR_BTN_SIZE*2;
                        break;
                    case SCROLLBAR_PLATINUM:
                        if(horiz)
                            width-=SBAR_BTN_SIZE*2;
                        else
                            height-=SBAR_BTN_SIZE*2;
                        break;
                }
#endif
            }
            else
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
    #ifdef SIMPLE_SCROLLBARS
                    case SCROLLBAR_NONE:
                        sbarRound=ROUNDED_NONE;
                        break;
    #endif
                }

            if(opts.square&SQUARE_SB_SLIDER)
                sbarRound=ROUNDED_NONE;

#if !GTK_CHECK_VERSION(2, 90, 0)
            if(drawBg)
            {
                GtkWidget *parent=NULL;
                if(opts.gtkScrollViews && IS_FLAT(opts.sbarBgndAppearance) && 0!=opts.tabBgnd && widget &&
                  (parent=qtcWidgetGetParent(widget)) && GTK_IS_SCROLLED_WINDOW(parent) &&
                  (parent=qtcWidgetGetParent(parent)) && GTK_IS_NOTEBOOK(parent))
                    drawAreaModColor(cr, area, &qtcPalette.background[ORIGINAL_SHADE], TO_FACTOR(opts.tabBgnd), xo, yo, wo, ho);
                else if(!CUSTOM_BGND || !(opts.gtkScrollViews && IS_FLAT(opts.sbarBgndAppearance) &&
                                                          widget && drawWindowBgnd(cr, style, area, widget, xo, yo, wo, ho)))
                    drawBevelGradient(cr, style, area, xo, yo, wo, ho,
                                      &qtcPalette.background[ORIGINAL_SHADE],
                                      horiz, FALSE, opts.sbarBgndAppearance, WIDGET_SB_BGND);
            }
#endif

            if(isMozilla())
            {
                if(!drawBg)
                {
                    GdkColor *parent_col=getParentBgCol(widget),
                             *bgnd_col=parent_col ? parent_col : &qtcPalette.background[ORIGINAL_SHADE];

                    setCairoClipping(cr, area);

                    drawAreaColor(cr, area, &qtcPalette.background[ORIGINAL_SHADE], x, y, width, height);
                    if(horiz)
                    {
                        if(ROUNDED_LEFT==sbarRound || ROUNDED_ALL==sbarRound)
                            drawVLine(cr, CAIRO_COL(*bgnd_col), 1.0, x, y, height);
                        if(ROUNDED_RIGHT==sbarRound || ROUNDED_ALL==sbarRound)
                            drawVLine(cr, CAIRO_COL(*bgnd_col), 1.0, x+width-1, y, height);
                    }
                    else
                    {
                        if(ROUNDED_TOP==sbarRound || ROUNDED_ALL==sbarRound)
                            drawHLine(cr, CAIRO_COL(*bgnd_col), 1.0, x, y, width);
                        if(ROUNDED_BOTTOM==sbarRound || ROUNDED_ALL==sbarRound)
                            drawHLine(cr, CAIRO_COL(*bgnd_col), 1.0, x, y+height-1, width);
                    }
                    unsetCairoClipping(cr);
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
                //drawAreaColor(cr, area, &qtcPalette.background[ORIGINAL_SHADE], x, y, width, height);
                drawBgnd(cr, &qtcPalette.background[ORIGINAL_SHADE], widget, area, x, y, width, height);

            drawLightBevel(cr, style, state, area,
                           thinner && !horiz ? x+THIN_SBAR_MOD : x,
                           thinner && horiz  ? y+THIN_SBAR_MOD : y,
                           thinner && !horiz ? width-(THIN_SBAR_MOD*2) : width,
                           thinner && horiz  ? height-(THIN_SBAR_MOD*2) : height,
                           &qtcPalette.background[2], qtcPalette.background, sbarRound,
                           thinner ? WIDGET_SLIDER_TROUGH : WIDGET_TROUGH,
                           BORDER_FLAT, DF_DO_CORNERS|DF_SUNKEN|DF_DO_BORDER|
                           (horiz ? 0 : DF_VERT), widget);
        }
    }
    else if(DETAIL("entry-progress"))
    {
        int adjust=(opts.fillProgress ? 4 : 3)-(opts.etchEntry ? 1 : 0);
        drawProgress(cr, style, state, widget, area, x-adjust, y-adjust, width+adjust, height+(2*adjust), rev, TRUE);
    }
    else if(detail && (0==strcmp(detail,"dockitem") || 0==strcmp(detail,"dockitem_bin")))
    {
        if(CUSTOM_BGND && widget)
            drawWindowBgnd(cr, style, area, widget, x, y, width, height);
    }
    else if(widget && ( (detail && ( menubar || 0==strcmp(detail, "toolbar") ||
                                     0==strcmp(detail, "handlebox") ||
                                     0==strcmp(detail,"handlebox_bin") ) )
                        || WIDGET_TYPE_NAME("PanelAppletFrame")))
    {
        //if(GTK_SHADOW_NONE!=shadow_type)
        {
            GdkColor    *col=menubar && (GTK_STATE_INSENSITIVE!=state || SHADE_NONE!=opts.shadeMenubars)
                                ? &menuColors(activeWindow)[ORIGINAL_SHADE]
                                : &style->bg[state];
            EAppearance app=menubar ? opts.menubarAppearance : opts.toolbarAppearance;
            int         menuBarAdjust=0,
                        opacity=getOpacity(widget);
            double      alpha=opacity!=100 ? (opacity/100.00) : 1.0;
            gboolean    drawGradient=GTK_SHADOW_NONE!=shadow_type && !IS_FLAT(app),
                        fillBackground=menubar && SHADE_NONE!=opts.shadeMenubars;

#if !GTK_CHECK_VERSION(2, 90, 0)
//             if(menubar)
            if(!isMozilla())
                qtcWMMoveSetup(widget);
#endif

            if(menubar && BLEND_TITLEBAR)
            {
                menuBarAdjust=qtcGetWindowBorderSize(FALSE).titleHeight;
                if(widget)
                {
                    if(qtcEmitMenuSize(widget, height) &&
                      (opts.menubarHiding || opts.windowBorder&WINDOW_BORDER_USE_MENUBAR_COLOR_FOR_TITLEBAR))
                        qtcWindowMenuBarDBus(widget, height);
                }
            }

            if(widget && (opacity!=100 || (CUSTOM_BGND && !drawGradient && !fillBackground)))
                drawWindowBgnd(cr, style, area, widget, x, y, width, height);
            
            if(drawGradient)
            {
                drawBevelGradientAlpha(cr, style, area, x, y-menuBarAdjust, width, height+menuBarAdjust, col,
                                menubar
                                    ? TRUE
                                    : DETAIL("handlebox")
                                            ? width<height
                                            : width>height,
                                FALSE, MODIFY_AGUA(app), WIDGET_OTHER, alpha);
            }
            else if(fillBackground)
            {
                drawAreaColorAlpha(cr, area, col, x, y, width, height, alpha);
            }

            if(GTK_SHADOW_NONE!=shadow_type && TB_NONE!=opts.toolbarBorders)
            {
                gboolean top=FALSE,
                         bottom=FALSE,
                         left=FALSE,
                         right=FALSE,
                         all=TB_LIGHT_ALL==opts.toolbarBorders || TB_DARK_ALL==opts.toolbarBorders;
                int      border=TB_DARK==opts.toolbarBorders || TB_DARK_ALL==opts.toolbarBorders ? 3 : 4;
                GdkColor *cols=activeWindow && menubar && (GTK_STATE_INSENSITIVE!=state || SHADE_NONE!=opts.shadeMenubars)
                                ? menuColors(activeWindow) : qtcPalette.background;

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
                        else if(GTK_ORIENTATION_HORIZONTAL==qtcToolbarGetOrientation(widget))
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
                    drawHLine(cr, CAIRO_COL(cols[0]), 1.0, x, y, width);
                if(left)
                    drawVLine(cr, CAIRO_COL(cols[0]), 1.0, x, y, height);
                if(bottom)
                    drawHLine(cr, CAIRO_COL(cols[border]), 1.0, x, y + height - 1, width);
                if(right)
                    drawVLine(cr, CAIRO_COL(cols[border]), 1.0, x+width-1, y, height);
            }
        }
    }
    else if(widget && pbar)
        drawProgress(cr, style, state, widget, area, x, y, width, height, rev, FALSE);
    else if(widget && menuitem)
    {
        GtkMenuBar *mb=menuitem ? isMenubar(widget, 0) : NULL;
#if GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
        gboolean   active_mb=TRUE;
#else
        gboolean   active_mb=isMozilla() || (mb ? GTK_MENU_SHELL(mb)->active : FALSE);

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
                              (!opts.useHighlightForMenu && (mb || menuitem));
            GdkColor *itemCols=grayItem ? qtcPalette.background : qtcPalette.highlight;
            GdkColor *bgnd=qtcPalette.menubar && mb && !isMozilla() && GTK_APP_JAVA!=qtSettings.app
                            ? &qtcPalette.menubar[ORIGINAL_SHADE] : NULL;
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
               (opts.borderMenuitems || !IS_FLAT(opts.menuitemAppearance)))
                fillVal=ORIGINAL_SHADE;

            if(mb && !opts.roundMbTopOnly && !(opts.square&SQUARE_POPUP_MENUS))
                x++, y++, width-=2, height-=2;

            if(!mb && menuitem && APPEARANCE_FADE==opts.menuitemAppearance)
            {
                gboolean        reverse=FALSE; /* TODO !!! */
                cairo_pattern_t *pt=NULL;
                double          fadePercent=0.0;

                if(ROUNDED)
                {
                    x++, y++, width-=2, height-=2;
                    clipPathRadius(cr, x, y, width, height, 4, reverse ? ROUNDED_RIGHT : ROUNDED_LEFT);
                }

                fadePercent=((double)MENUITEM_FADE_SIZE)/(double)width;
                pt=cairo_pattern_create_linear(x, y, x+width-1, y);

                cairo_pattern_add_color_stop_rgba(pt, 0, CAIRO_COL(itemCols[fillVal]), reverse ? 0.0 : 1.0);
#ifdef QTC_CAIRO_1_10_HACK
                cairo_pattern_add_color_stop_rgba(pt, 0, CAIRO_COL(itemCols[fillVal]), reverse ? 0.0 : 1.0);
#endif
                cairo_pattern_add_color_stop_rgba(pt, reverse ? fadePercent : 1.0-fadePercent, CAIRO_COL(itemCols[fillVal]), 1.0);
#ifdef QTC_CAIRO_1_10_HACK  
                cairo_pattern_add_color_stop_rgba(pt, 1.00, CAIRO_COL(itemCols[fillVal]), reverse ? 1.0 : 0.0);
#endif
                cairo_pattern_add_color_stop_rgba(pt, 1.00, CAIRO_COL(itemCols[fillVal]), reverse ? 1.0 : 0.0);
                cairo_set_source(cr, pt);
                cairo_rectangle(cr, x, y, width, height);
                cairo_fill(cr);
                cairo_pattern_destroy(pt);
                if(ROUNDED)
                    unsetCairoClipping(cr);
            }
            else if(!opts.borderMenuitems && !mb && menuitem)
            {
                /*For now dont round combos - getting weird effects with shadow/clipping :-( */
                gboolean roundedMenu=(!widget || !isComboMenu(qtcWidgetGetParent(widget))) && !(opts.square&SQUARE_POPUP_MENUS);

                if(roundedMenu)
                    clipPathRadius(cr, x, y, width, height, MENU_AND_TOOLTIP_RADIUS-1.0, round);
                drawBevelGradient(cr, style, area, x, y, width, height, &itemCols[fillVal],
                                  TRUE, FALSE, opts.menuitemAppearance, WIDGET_MENU_ITEM);
                if(roundedMenu)
                    unsetCairoClipping(cr);
            }
            else if(stdColors && opts.borderMenuitems)
            {
                drawLightBevel(cr, style, new_state, area, x, y, width, height, &itemCols[fillVal],
                                itemCols, round, WIDGET_MENU_ITEM, BORDER_FLAT, DF_DRAW_INSIDE|
                                (stdColors ? DF_DO_BORDER : 0)|(activeWindow && USE_SHADED_MENU_BAR_COLORS ? 0 : DF_DO_CORNERS), widget);
            }
            else
            {
                if(width>2 && height>2)
                    drawBevelGradient(cr, style, area, x+1, y+1, width-2, height-2, &itemCols[fillVal],
                                      TRUE, FALSE, opts.menuitemAppearance, WIDGET_MENU_ITEM);

                realDrawBorder(cr, style, state, area, x, y, width, height,
                               itemCols, round, BORDER_FLAT, WIDGET_MENU_ITEM, 0, borderVal);
            }
        }
    }
    else if(DETAIL("menu"))
    {
        gboolean comboMenu=isComboMenu(widget),
                 roundedMenu=/*!comboMenu &&*/ !(opts.square&SQUARE_POPUP_MENUS);
        double   radius=0.0;

        /*For now dont round combos - getting weird effects with shadow/clipping :-( */
        if(roundedMenu && !comboMenu)
        {
            int      size=((width&0xFFFF)<<16)+(height&0xFFFF),
                     old=(int)g_object_get_data(G_OBJECT(widget), "QTC_WIDGET_MASK");

            radius=MENU_AND_TOOLTIP_RADIUS; // getRadius(&opts, width, height, WIDGET_SELECTION, RADIUS_SELECTION);
            if(size!=old)
            {
#if GTK_CHECK_VERSION(2, 90, 0)
                QtcRegion *mask=windowMask(0, 0, width, height, opts.round>ROUND_SLIGHT);

                gtk_widget_shape_combine_region(widget, NULL);
                gtk_widget_shape_combine_region(widget, mask);
                qtcRegionDestroy(mask);
#else
                GdkBitmap *mask=gdk_pixmap_new(NULL, width, height, 1);
                cairo_t   *crMask = gdk_cairo_create((GdkDrawable *) mask);

                cairo_rectangle(crMask, 0, 0, width, height);
                cairo_set_source_rgba(crMask, 1, 1, 1, 0);
                cairo_set_operator(crMask, CAIRO_OPERATOR_SOURCE);
                cairo_paint(crMask);
                cairo_new_path(crMask);
                createPath(crMask, 0, 0, width, height, radius-0.25, ROUNDED_ALL);
                cairo_set_source_rgba(crMask, 1, 0, 0, 1);
                cairo_fill(crMask);

                gdk_window_shape_combine_mask(gtk_widget_get_parent_window(widget), NULL, 0, 0);
                gdk_window_shape_combine_mask(gtk_widget_get_parent_window(widget), mask, 0, 0);
                cairo_destroy(crMask);
#endif
                g_object_set_data(G_OBJECT(widget), "QTC_WIDGET_MASK", (gpointer)size);
            }
            clipPath(cr, x, y, width, height, WIDGET_OTHER, radius+1, ROUNDED_ALL);
        }

        if(widget && /*!comboMenu && */100!=opts.menuBgndOpacity && !isFixedWidget(widget) && isRgbaWidget(widget))
            enableBlurBehind(widget, TRUE);

        if(/*!comboMenu && */!IS_FLAT_BGND(opts.menuBgndAppearance))
        {
            double   alpha=1.0;
            gboolean useAlpha=opts.menuBgndOpacity<100 && isRgbaWidget(widget) && compositingActive(widget);
            
            if(useAlpha)
            {
                alpha=opts.menuBgndOpacity/100.0;
                cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            }
            if(APPEARANCE_STRIPED==opts.menuBgndAppearance)
                drawStripedBgnd(cr, style, area, x, y, width, height, &qtcPalette.menu[ORIGINAL_SHADE], FALSE, alpha);
            else if(APPEARANCE_FILE==opts.bgndAppearance)
                drawBgndImage(cr, style, area, x, y, width, height, &qtcPalette.menu[ORIGINAL_SHADE], FALSE, alpha);
            else
                drawBevelGradientAlpha(cr, style, area, x, y, width, height,
                                       &qtcPalette.menu[ORIGINAL_SHADE], GT_HORIZ==opts.menuBgndGrad, FALSE, opts.menuBgndAppearance,
                                       WIDGET_OTHER, alpha);
            if(useAlpha)
                cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        }
        else if(opts.shadePopupMenu || USE_LIGHTER_POPUP_MENU)
        {
            double   alpha=1.0;
            gboolean useAlpha=/*!comboMenu  && */opts.menuBgndOpacity<100 && isRgbaWidget(widget) && compositingActive(widget);

            if(useAlpha)
            {
                alpha=opts.menuBgndOpacity/100.0;
                cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            }
            drawAreaColorAlpha(cr, area, &qtcPalette.menu[ORIGINAL_SHADE], x, y, width, height, alpha);
            if(useAlpha)
                cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        }

        if(/*!comboMenu && */IMG_NONE!=opts.menuBgndImage.type)
            drawBgndRings(cr, x, y, width, height, FALSE);

        if(opts.menuStripe && !comboMenu)
        {
            gboolean mozOo=GTK_APP_OPEN_OFFICE==qtSettings.app || isMozilla();
            int stripeWidth=mozOo ? 22 : 21;

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
                                GTK_IMAGE_EMPTY==gtk_image_get_storage_type(GTK_IMAGE(
                                        gtk_image_menu_item_get_image(item)))))
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

            drawBevelGradient(cr, style, area, x+1, y+1, stripeWidth+1, height-2,
                              &opts.customMenuStripeColor, FALSE, FALSE, opts.menuStripeAppearance, WIDGET_OTHER);
        }

        if(opts.popupBorder)
        {
            GdkColor        *cols=USE_LIGHTER_POPUP_MENU || opts.shadePopupMenu
                                ? qtcPalette.menu
                                : qtcPalette.background;
            EGradientBorder border=getGradient(opts.menuBgndAppearance, &opts)->border;

            if(roundedMenu && !comboMenu)
                unsetCairoClipping(cr);
            cairo_new_path(cr);
            cairo_set_source_rgb(cr, CAIRO_COL(cols[STD_BORDER]));
             /*For now dont round combos - getting weird effects with shadow/clipping :-( */
            if(roundedMenu && !comboMenu)
                createPath(cr, x+0.5, y+0.5, width-1, height-1, radius-1, ROUNDED_ALL);
            else
                cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
            cairo_stroke(cr);
            if(USE_BORDER(border) && APPEARANCE_FLAT!=opts.menuBgndAppearance)
            {
                if(roundedMenu)
                {
                    if(GB_3D!=border)
                    {
                        cairo_new_path(cr);
                        cairo_set_source_rgb(cr, CAIRO_COL(cols[0]));
                        createTLPath(cr, x+1.5, y+1.5, width-3, height-3, radius-2, ROUNDED_ALL);
                        cairo_stroke(cr);
                    }
                    cairo_new_path(cr);
                    cairo_set_source_rgb(cr, CAIRO_COL(cols[GB_LIGHT==border ? 0 : FRAME_DARK_SHADOW]));
                    createBRPath(cr, x+1.5, y+1.5, width-3, height-3, radius-2, ROUNDED_ALL);
                    cairo_stroke(cr);
                }
                else
                {
                    if(GB_3D!=border)
                    {
                        drawHLine(cr, CAIRO_COL(cols[0]), 1.0, x+1, y+1, width-2);
                        drawVLine(cr, CAIRO_COL(cols[0]), 1.0, x+1, y+1, height-2);
                    }
                    drawHLine(cr, CAIRO_COL(cols[GB_LIGHT==border ? 0 : FRAME_DARK_SHADOW]), 1.0, x+1, y+height-2, width-2);
                    drawVLine(cr, CAIRO_COL(cols[GB_LIGHT==border ? 0 : FRAME_DARK_SHADOW]), 1.0, x+width-2, y+1, height-2);
                }
            }
        }
    }
    else if(detail &&(!strcmp(detail, "paned") || !strcmp(detail+1, "paned")))
    {
        GtkOrientation orientation = GTK_ORIENTATION_HORIZONTAL;

        if(*detail == 'h')
            orientation = GTK_ORIENTATION_VERTICAL;

        gtkDrawHandle(style, WINDOW_PARAM_VAL state, shadow_type, AREA_PARAM_VAL widget, detail, x, y, width, height,
                      orientation);
    }
    else if(detail && 0==strcmp(detail+1, "ruler"))
    {
        drawBevelGradient(cr, style, area, x, y, width, height, &qtcPalette.background[ORIGINAL_SHADE],
                          'h'==detail[0], FALSE, opts.lvAppearance, WIDGET_LISTVIEW_HEADER);

//        if(IS_FLAT_BGND(opts.bgndAppearance) || !widget || !drawWindowBgnd(cr, style, area, widget, x, y, width, height))
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

        if(offset && (GTK_APP_OPEN_OFFICE==qtSettings.app || isMozilla()))
            offset+=2;

        if(isMenuItem && (USE_LIGHTER_POPUP_MENU || opts.shadePopupMenu))
            cols=SHADE_WINDOW_BORDER==opts.shadeMenubars
                    ? qtcPalette.wborder[0]
                    : qtcPalette.menu
                        ? qtcPalette.menu
                        : qtcPalette.background;

        drawFadedLine(cr, x+1+offset, y+(height>>1), width-(1+offset), 1, &cols[isMenuItem ? MENU_SEP_SHADE : STD_BORDER], area, NULL,
                      TRUE, TRUE, TRUE);
    }
    else if(DETAIL("vseparator"))
        drawFadedLine(cr, x+(width>>1), y, 1, height, &qtcPalette.background[STD_BORDER], area, NULL, TRUE, TRUE, FALSE);
    else
    {
        EWidget wt=!detail && GTK_IS_TREE_VIEW(widget) ? WIDGET_PBAR_TROUGH : WIDGET_FRAME;
        clipPath(cr, x+1, y+1, width-2, height-2, WIDGET_OTHER, RADIUS_INTERNAL, round);
        if(IS_FLAT_BGND(opts.bgndAppearance) || !widget || !drawWindowBgnd(cr, style, area, widget, x+1, y+1, width-2, height-2))
        {
            drawAreaColor(cr, area, &style->bg[state], x+1, y+1, width-2, height-2);
            if(widget && IMG_NONE!=opts.bgndImage.type)
                drawWindowBgnd(cr, style, area, widget, x, y, width, height);
        }
        unsetCairoClipping(cr);
        drawBorder(cr, style, state, area, x, y, width, height,
                   NULL, menuScroll || opts.square&SQUARE_FRAME ? ROUNDED_NONE : ROUNDED_ALL, shadowToBorder(shadow_type), wt, STD_BORDER);
    }
    CAIRO_END
}

static void gtkDrawBox(GtkStyle *style, WINDOW_PARAM GtkStateType state, GtkShadowType shadow_type, AREA_PARAM
                       GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height)
{
    sanitizeSize(window, &width, &height);
    drawBox(style, WINDOW_PARAM_VAL state, shadow_type, AREA_PARAM_VAL_L, widget, detail, x, y, width, height,
            GTK_STATE_ACTIVE==state || (GTK_IS_BUTTON(widget) && qtcButtonIsDepressed(widget)));
}

static void gtkDrawShadow(GtkStyle *style, WINDOW_PARAM GtkStateType state,
                          GtkShadowType shadow_type, AREA_PARAM GtkWidget *widget,
                          const gchar *detail, gint x, gint y, gint width, gint height)
{
    sanitizeSize(window, &width, &height);

    CAIRO_BEGIN

    if(isComboBoxList(widget))
    {
        drawAreaColor(cr, area, &style->base[state], x, y, width, height);
        if(opts.popupBorder)
        {
            cairo_new_path(cr);
            cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
            cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]));
            cairo_stroke(cr);
        }
    }
    else if(isComboList(widget))
    {
        drawAreaColor(cr, area, &style->base[state], x, y, width, height);
        if(opts.popupBorder && !DETAIL("viewport"))
        {
            cairo_new_path(cr);
            cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
            cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]));
            cairo_stroke(cr);
        }
    }
#if 0
    else if(isComboFrame(widget))
    {
    }
    else
#endif
    else if(DETAIL("entry") || DETAIL("text"))
    {
        GtkWidget *parent=widget ? qtcWidgetGetParent(widget) : NULL;
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
            GtkStateType savedState=state;
        
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
                    btn=getMappedWidget(parent, 0);
                if(btn && GTK_STATE_PRELIGHT==qtcWidgetGetState(btn))
                    state=GTK_STATE_PRELIGHT, qtcWidgetSetState(widget, GTK_STATE_PRELIGHT);
            }

#if GTK_CHECK_VERSION(2, 90, 0)
            if(!opts.unifySpin && isSpin)
                width-=16;
#endif
            drawEntryField(cr, style, state, widget, area, x, y, width, height,
                        combo || isSpin
                            ? rev
                                    ? ROUNDED_RIGHT
                                    : ROUNDED_LEFT
                            : ROUNDED_ALL,
                        WIDGET_ENTRY);
            if(combo && opts.unifyCombo && parent)
            {
                if(btn && GTK_STATE_INSENSITIVE!=qtcWidgetGetState(widget))
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
                 statusBar=isMozilla() || GTK_APP_JAVA==qtSettings.app
                            ? frame : isStatusBarFrame(widget);

        if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %s  ", __FUNCTION__, state, shadow_type, x, y, width, height,
                                               detail ? detail : "NULL"),
                                        debugDisplayWidget(widget, 3);

        if(!statusBar && !drawSquare && (frame || scrolledWindow || viewport/* || drawSquare*/)) // && ROUNDED)
        {
            if(GTK_SHADOW_NONE!=shadow_type &&
               (!frame || opts.drawStatusBarFrames || (!isMozilla() && GTK_APP_JAVA!=qtSettings.app)))
            {
                GtkWidget *parent=widget ? qtcWidgetGetParent(widget) : NULL;
                gboolean doBorder=!viewport && !drawSquare,
                         windowFrame=parent && !isFixedWidget(widget) && GTK_IS_FRAME(widget) && GTK_IS_WINDOW(parent);

                if(windowFrame)
                {
                    GtkAllocation wAlloc=qtcWidgetGetAllocation(widget),
                                  pAlloc=qtcWidgetGetAllocation(parent);
                    windowFrame=eqRect(&wAlloc, &pAlloc);
                }

//                 if(!drawSquare && widget && qtcWidgetGetParent(widget) && !isFixedWidget(widget) &&
//                    GTK_IS_FRAME(widget) && GTK_IS_WINDOW(qtcWidgetGetParent(widget)))
//                     drawSquare=true;

                if(scrolledWindow)
                {
                    /* See code in qt_settings.c as to isMozill part */
                    if((opts.square&SQUARE_SCROLLVIEW) || isMozillaWidget(widget))
                    {
                        /* Flat style...
                        drawBorder(cr, style, state, area, x, y, width, height,
                                   NULL, ROUNDED_NONE, BORDER_FLAT, WIDGET_SCROLLVIEW, 0);
                        */
                        /* 3d... */
                        cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]));
                        createTLPath(cr, x+0.5, y+0.5, width-1, height-1, 0.0, ROUNDED_NONE);
                        cairo_stroke(cr);
                        if(!opts.gtkScrollViews)
                            cairo_set_source_rgba(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]), LOWER_BORDER_ALPHA);
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
                        cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]));
                    else
                        cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.background[ORIGINAL_SHADE]));
                    cairo_stroke(cr);
                }
                else if(doBorder)
                    drawBorder(cr, style, state, area, x, y, width, height,
                               NULL, ROUNDED_ALL, scrolledWindow ? BORDER_SUNKEN : BORDER_FLAT,
                               scrolledWindow ? WIDGET_SCROLLVIEW : WIDGET_FRAME, DF_BLEND|(viewport ? 0 : DF_DO_CORNERS));
            }
        }
        else if(!statusBar || opts.drawStatusBarFrames)
        {
            int c1=0,
                c2=0;

            switch(shadow_type)
            {
                case GTK_SHADOW_NONE:
                    if(statusBar)
                        shadow_type=GTK_SHADOW_IN;
                    else
                        break;
                case GTK_SHADOW_IN:
                case GTK_SHADOW_ETCHED_IN:
                    c1 = 0;
                    c2 = STD_BORDER;
                    break;
                case GTK_SHADOW_OUT:
                case GTK_SHADOW_ETCHED_OUT:
                    c1 = STD_BORDER;
                    c2 = 0;
                    break;
            }

            switch(shadow_type)
            {
                case GTK_SHADOW_NONE:
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+0.5, y+0.5, width-1, height-1);
                    cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]));
                    cairo_stroke(cr);
                    break;
                case GTK_SHADOW_IN:
                case GTK_SHADOW_OUT:
                    //if(drawSquare || frame || !ROUNDED)
                    {
                        double c2Alpha=GTK_SHADOW_IN==shadow_type ? 1.0 : LOWER_BORDER_ALPHA,
                               c1Alpha=GTK_SHADOW_OUT==shadow_type ? 1.0 : LOWER_BORDER_ALPHA;
                        drawHLine(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]), c2Alpha, x, y, width);
                        drawVLine(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]), c2Alpha, x, y, height);
                        if(APPEARANCE_FLAT!=opts.appearance)
                        {
                            drawHLine(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]), c1Alpha, x, y+height-1, width);
                            drawVLine(cr, CAIRO_COL(qtcPalette.background[STD_BORDER]), c1Alpha, x+width-1, y, height);
                        }
                    }
                    break;
                case GTK_SHADOW_ETCHED_IN:
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+1.5, y+1.5, width-2, height-2);
                    cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.background[c1]));
                    cairo_stroke(cr);
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+0.5, y+0.5, width-2, height-2);
                    cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.background[c2]));
                    cairo_stroke(cr);
                    break;
                case GTK_SHADOW_ETCHED_OUT:
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+1.5, y+1.5, width-2, height-2);
                    cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.background[c2]));
                    cairo_stroke(cr);
                    cairo_new_path(cr);
                    cairo_rectangle(cr, x+0.5, y+0.5, width-2, height-2);
                    cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.background[c1]));
                    cairo_stroke(cr);
                    break;
            }
        }
    }
    CAIRO_END
}

static void setGapClip(cairo_t *cr, GdkRectangle *area, GtkPositionType gap_side, int gap_x, int gap_width, int x, int y, int width,
                       int height, gboolean isTab)
{
    if(gap_width>0)
    {
        GdkRectangle gapRect;
        int          adjust=isTab ? (gap_x>1 ? 1 : 2) : 0;

        switch(gap_side)
        {
            case GTK_POS_TOP:
                gapRect.x=x+gap_x+adjust, gapRect.y=y, gapRect.width=gap_width-(2*adjust), gapRect.height=2;
                if(GTK_APP_JAVA==qtSettings.app && isTab)
                    gapRect.width-=3;
                break;
            case GTK_POS_BOTTOM:
                gapRect.x=x+gap_x+adjust, gapRect.y=y+height-2, gapRect.width=gap_width-(2*adjust), gapRect.height=2;
                break;
            case GTK_POS_LEFT:
                gapRect.x=x, gapRect.y=y+gap_x+adjust, gapRect.width=2, gapRect.height=gap_width-(2*adjust);
                break;
            case GTK_POS_RIGHT:
                gapRect.x=x+width-2, gapRect.y=y+gap_x+adjust, gapRect.width=2, gapRect.height=gap_width-(2*adjust);
                break;
        }

        QtcRect   r={x, y, width, height};
        QtcRegion *region=qtcRegionRect(area ? area : &r),
                  *inner=qtcRegionRect(&gapRect);

        qtcRegionXor(region, inner);
        setCairoClippingRegion(cr, region);
        qtcRegionDestroy(inner);
        qtcRegionDestroy(region);
    }
}

static void drawBoxGap(cairo_t *cr, GtkStyle *style, GtkShadowType shadow_type, GtkStateType state,
                       GtkWidget *widget, GdkRectangle *area, gint x, gint y, gint width, gint height, GtkPositionType gap_side,
                       gint gap_x, gint gap_width, EBorder borderProfile, gboolean isTab)
{
    g_return_if_fail(GTK_IS_STYLE(style));

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %d %d %d ", __FUNCTION__, shadow_type, state, x, y, width, height, gap_x,
                                           gap_width, isTab),
                                    debugDisplayWidget(widget, 3);

    if(isTab && 0!=opts.tabBgnd)
    {
        clipPath(cr, x-1, y-1, width+2, height+2, WIDGET_TAB_FRAME, RADIUS_EXTERNAL, ROUNDED_ALL);
        drawAreaMod(cr, style, state, area, TO_FACTOR(opts.tabBgnd), x, y, width, height);
        unsetCairoClipping(cr);
    }

    if(TAB_MO_GLOW==opts.tabMouseOver && gap_width>4 && isMozillaWidget(widget))
        gap_width-=2;
        
    if(GTK_SHADOW_NONE!=shadow_type)
    {
        int       round=((!isTab && opts.square&SQUARE_FRAME) || (isTab && opts.square&SQUARE_TAB_FRAME)) ? ROUNDED_NONE : ROUNDED_ALL;
        GtkWidget *parent=widget ? qtcWidgetGetParent(widget) : NULL;
        
        if(!(opts.square&SQUARE_TAB_FRAME) && gap_x<=0)
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

        setGapClip(cr, area, gap_side, gap_x, gap_width, x, y, width, height, isTab);
        drawBorder(cr, qtcWidgetGetStyle(parent ? parent : widget), state, area, x, y, width, height, NULL, round,
                   borderProfile, isTab ? WIDGET_TAB_FRAME : WIDGET_FRAME, (isTab ? 0 : DF_BLEND)|DF_DO_CORNERS);
        if(gap_width>0)
            unsetCairoClipping(cr);
    }
}

static GdkColor * getCheckRadioCol(GtkStyle *style, GtkStateType state, gboolean mnu)
{
    return !qtSettings.qt4 && mnu
                ? &style->text[state]
                : GTK_STATE_INSENSITIVE==state
                    ? &qtSettings.colors[PAL_DISABLED][opts.crButton ? COLOR_BUTTON_TEXT : COLOR_TEXT]
                    : qtcPalette.check_radio;
}

static void gtkDrawCheck(GtkStyle *style, WINDOW_PARAM GtkStateType state,
                         GtkShadowType shadow_type, AREA_PARAM GtkWidget *widget,
                         const gchar *detail, gint x, gint y, gint width, gint height)
{
    if(GTK_STATE_PRELIGHT==state && (GTK_APP_MOZILLA==qtSettings.app || GTK_APP_JAVA==qtSettings.app))
        state=GTK_STATE_NORMAL;
    {

    gboolean mnu=DETAIL("check"),
             list=!mnu && isList(widget),
             on=GTK_SHADOW_IN==shadow_type,
             tri=GTK_SHADOW_ETCHED_IN==shadow_type,
             doEtch=DO_EFFECT;
    GdkColor new_colors[TOTAL_SHADES+1],
             *btn_colors;
    int      ind_state=(list || ( !mnu && GTK_STATE_INSENSITIVE==state ) ) ? state : GTK_STATE_NORMAL,
             checkSpace=doEtch ? opts.crSize+2 : opts.crSize;

    CAIRO_BEGIN

    if(opts.crColor && GTK_STATE_INSENSITIVE!=state && (on || tri))
        btn_colors=qtcPalette.selectedcr;
    else if(!mnu && QT_CUSTOM_COLOR_BUTTON(style))
    {
        shadeColors(&(style->bg[state]), new_colors);
        btn_colors=new_colors;
    }
    else
        btn_colors=qtcPalette.button[GTK_STATE_INSENSITIVE==state ? PAL_DISABLED : PAL_ACTIVE];

    x+=(width-checkSpace)>>1;
    y+=(height-checkSpace)>>1;

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %d %s  ", __FUNCTION__, state, shadow_type, x, y, width, height, mnu,
                                           detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 3);

    if((mnu && GTK_STATE_PRELIGHT==state) ||
       (list && GTK_STATE_ACTIVE==state))
        state=GTK_STATE_NORMAL;

    if(mnu && isMozilla())
        x-=2;

    if(!mnu || qtSettings.qt4)
    {
        if(opts.crButton)
        {
            drawLightBevel(cr, style, state, area, x, y, checkSpace, checkSpace, &btn_colors[getFill(state, false)],
                           btn_colors, ROUNDED_ALL, WIDGET_CHECKBOX, BORDER_FLAT,
                           DF_DO_BORDER|(GTK_STATE_ACTIVE==state ? DF_SUNKEN : 0), list ? NULL : widget);
            if(doEtch)
                x++, y++;
        }
        else
        {
            gboolean    coloredMouseOver=GTK_STATE_PRELIGHT==state && opts.coloredMouseOver,
                        glow=doEtch && GTK_STATE_PRELIGHT==state && MO_GLOW==opts.coloredMouseOver,
                        lightBorder=DRAW_LIGHT_BORDER(FALSE, WIDGET_TROUGH, APPEARANCE_INVERTED),
                        draw3dFull=!lightBorder && DRAW_3D_FULL_BORDER(FALSE, APPEARANCE_INVERTED),
                        draw3d=draw3dFull || (!lightBorder && DRAW_3D_BORDER(FALSE, APPEARANCE_INVERTED));
            GdkColor    *colors=coloredMouseOver
                            ? qtcPalette.mouseover
                            : btn_colors,
                        *bgndCol=/*!isList(widget) && */GTK_STATE_INSENSITIVE==state || GTK_STATE_ACTIVE==state
                                    ? &style->bg[GTK_STATE_NORMAL]
                                    : !mnu && GTK_STATE_PRELIGHT==state && !coloredMouseOver && !opts.crHighlight
                                        ? &btn_colors[CR_MO_FILL]
                                        : &style->base[GTK_STATE_NORMAL];

            if(doEtch)
                x++, y++;

            drawBevelGradient(cr, style, area, x+1, y+1, opts.crSize-2, opts.crSize-2, bgndCol,
                              TRUE, FALSE, APPEARANCE_INVERTED, WIDGET_TROUGH);

            if(coloredMouseOver && !glow)
            {
                cairo_new_path(cr);
                cairo_set_source_rgb(cr, CAIRO_COL(colors[CR_MO_FILL]));
                cairo_rectangle(cr, x+1.5, y+1.5, opts.crSize-3, opts.crSize-3);
                //cairo_rectangle(cr, x+2.5, y+2.5, opts.crSize-5, opts.crSize-5);
                cairo_stroke(cr);
            }
            else
            {
                cairo_new_path(cr);
                if(lightBorder)
                {
                    cairo_set_source_rgb(cr, CAIRO_COL(btn_colors[LIGHT_BORDER(APPEARANCE_INVERTED)]));
                    cairo_rectangle(cr, x+1.5, y+1.5, opts.crSize-3, opts.crSize-3);
                }
                else
                {
                    GdkColor mid=midColor(GTK_STATE_INSENSITIVE==state
                                            ? &style->bg[GTK_STATE_NORMAL] : &style->base[GTK_STATE_NORMAL], &(colors[3]));

                    cairo_set_source_rgb(cr, CAIRO_COL(mid));
                    cairo_move_to(cr, x+1.5, y+opts.crSize-1.5);
                    cairo_line_to(cr, x+1.5, y+1.5);
                    cairo_line_to(cr, x+opts.crSize-1.5, y+1.5);
                }
                cairo_stroke(cr);
            }

            if(doEtch && (!list || glow) && !mnu)
            {
                if(glow && !(opts.thin&THIN_FRAMES))
                    drawGlow(cr, area, x-1, y-1, opts.crSize+2, opts.crSize+2, ROUNDED_ALL, WIDGET_CHECKBOX);
                else
                    drawEtch(cr, area, widget, x-1, y-1, opts.crSize+2, opts.crSize+2,
                            false, ROUNDED_ALL, WIDGET_CHECKBOX);
            }

            drawBorder(cr, style, state, area, x, y, opts.crSize, opts.crSize, colors, ROUNDED_ALL, BORDER_FLAT, WIDGET_CHECKBOX,
                       (list || mnu ? 0 : DF_DO_CORNERS));
        }
    }

    if(on)
    {
        GdkPixbuf *pix=getPixbuf(getCheckRadioCol(style, ind_state, mnu), PIX_CHECK, 1.0);
        int       pw=gdk_pixbuf_get_width(pix),
                  ph=gdk_pixbuf_get_height(pix),
                  dx=(x+(opts.crSize/2))-(pw/2),
                  dy=(y+(opts.crSize/2))-(ph/2);

        gdk_cairo_set_source_pixbuf(cr, pix, dx, dy);
        cairo_paint(cr);
    }
    else if (tri)
    {
        int      ty=y+(opts.crSize/2);
        GdkColor *col=getCheckRadioCol(style, ind_state, mnu);

        drawHLine(cr, CAIRO_COL(*col), 1.0, x+3, ty, opts.crSize-6);
        drawHLine(cr, CAIRO_COL(*col), 1.0, x+3, ty+1, opts.crSize-6);
    }

    CAIRO_END
    }
}

static void gtkDrawOption(GtkStyle *style, WINDOW_PARAM GtkStateType state, GtkShadowType shadow_type, AREA_PARAM
                          GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height)
{
    if(GTK_STATE_PRELIGHT==state && (GTK_APP_MOZILLA==qtSettings.app || GTK_APP_JAVA==qtSettings.app))
        state=GTK_STATE_NORMAL;
    {

    gboolean mnu=DETAIL("option"),
             list=!mnu && isList(widget);

    CAIRO_BEGIN

    if((mnu && GTK_STATE_PRELIGHT==state) ||
       (list && GTK_STATE_ACTIVE==state))
        state=GTK_STATE_NORMAL;

    if(!qtSettings.qt4 && mnu)
        gtkDrawCheck(style, WINDOW_PARAM_VAL state, shadow_type, AREA_PARAM_VAL widget, "check", x, y, width, height);
    else
    {
        gboolean  on=GTK_SHADOW_IN==shadow_type,
                  tri=GTK_SHADOW_ETCHED_IN==shadow_type,
                  set=on||tri,
                  doEtch=DO_EFFECT;
        int       ind_state=GTK_STATE_INSENSITIVE==state ? state : GTK_STATE_NORMAL,
                  optSpace=doEtch ? opts.crSize+2 : opts.crSize;
        GdkColor  new_colors[TOTAL_SHADES+1],
                  *btn_colors;

        x+=((width-optSpace)>>1);
        y+=((height-optSpace)>>1);
    
        if(opts.crColor && GTK_STATE_INSENSITIVE!=state && (on || tri))
            btn_colors=qtcPalette.selectedcr;
        else if(!mnu && QT_CUSTOM_COLOR_BUTTON(style))
        {
            shadeColors(&(style->bg[state]), new_colors);
            btn_colors=new_colors;
        }
        else
            btn_colors=qtcPalette.button[GTK_STATE_INSENSITIVE==state ? PAL_DISABLED : PAL_ACTIVE];

        if(opts.crButton)
        {
            drawLightBevel(cr, style, state, area, x, y, optSpace, optSpace, &btn_colors[getFill(state, false)],
                           btn_colors, ROUNDED_ALL, WIDGET_RADIO_BUTTON, BORDER_FLAT,
                           DF_DO_BORDER|(GTK_STATE_ACTIVE==state ? DF_SUNKEN : 0), list ? NULL : widget);
            if(doEtch)
                x++, y++;
        }
        else
        {
            bool     glow=doEtch && GTK_STATE_PRELIGHT==state && MO_GLOW==opts.coloredMouseOver;
            gboolean lightBorder=DRAW_LIGHT_BORDER(FALSE, WIDGET_TROUGH, APPEARANCE_INVERTED),
                     draw3d=!lightBorder &&
                            (DRAW_3D_BORDER(FALSE, APPEARANCE_INVERTED) || DRAW_3D_FULL_BORDER(FALSE, APPEARANCE_INVERTED)),
                     coloredMouseOver=GTK_STATE_PRELIGHT==state && opts.coloredMouseOver,
                     doneShadow=false;
            int      bgnd=0;
            GdkColor *colors=coloredMouseOver
                        ? qtcPalette.mouseover
                        : btn_colors,
                     *bgndCol=GTK_STATE_INSENSITIVE==state || GTK_STATE_ACTIVE==state
                        ? &style->bg[GTK_STATE_NORMAL]
                        : !mnu && GTK_STATE_PRELIGHT==state && !coloredMouseOver && !opts.crHighlight
                            ? &colors[CR_MO_FILL]
                            : &style->base[GTK_STATE_NORMAL];
            double   radius=(opts.crSize+1)/2.0;

            bgnd=getFill(state, set/*, TRUE*/);

            if(doEtch)
                x++, y++;

            clipPath(cr, x, y, opts.crSize, opts.crSize, WIDGET_RADIO_BUTTON, RADIUS_EXTERNAL, ROUNDED_ALL);
            drawBevelGradient(cr, style, NULL, x+1, y+1, opts.crSize-2, opts.crSize-2, bgndCol,
                                TRUE, FALSE, APPEARANCE_INVERTED, WIDGET_TROUGH);
            cairo_restore(cr);

            if(!mnu && coloredMouseOver && !glow)
            {
                double radius=(opts.crSize-2)/2.0;

                cairo_set_source_rgb(cr, CAIRO_COL(colors[CR_MO_FILL]));
                cairo_arc(cr, x+radius + 1, y+radius + 1, radius, 0, 2*M_PI);
                cairo_stroke(cr);
                radius--;
                cairo_arc(cr, x+radius + 2, y+radius + 2, radius, 0, 2*M_PI);
                cairo_stroke(cr);
            }

            if(!doneShadow && doEtch && !mnu && (!list || glow))
            {
                double   radius=(opts.crSize+1)/2.0;

                if(glow)
                    cairo_set_source_rgb(cr, CAIRO_COL(qtcPalette.mouseover[GLOW_MO]));
                else
                    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, ETCH_RADIO_TOP_ALPHA);

                if(EFFECT_NONE!=opts.buttonEffect || glow)
                {
                    cairo_arc(cr, x+radius - 0.5, y+radius - 0.5, radius, 0.75*M_PI, 1.75*M_PI);
                    cairo_stroke(cr);
                    if(!glow)
                        setLowerEtchCol(cr, widget);
                }
                cairo_arc(cr, x+radius - 0.5, y+radius - 0.5, radius, 1.75*M_PI, 0.75*M_PI);
                cairo_stroke(cr);
            }

            cairo_set_source_rgb(cr, CAIRO_COL(colors[coloredMouseOver ? 4 : BORDER_VAL(GTK_STATE_INSENSITIVE!=state)]));
            radius=(opts.crSize-0.5)/2.0;
            cairo_arc(cr, x+0.25+radius, y+0.25+radius, radius, 0, 2*M_PI);
            cairo_stroke(cr);
            if(!coloredMouseOver)
            {
                radius=(opts.crSize-1)/2.0;
                cairo_set_source_rgb(cr, CAIRO_COL(btn_colors[coloredMouseOver ? 3 : 4]));
                cairo_arc(cr, x+0.75+radius, y+0.75+radius, radius,
                            lightBorder ? 0 : 0.75*M_PI,
                            lightBorder ? 2*M_PI : 1.75*M_PI);
                cairo_stroke(cr);
            }
        }

        if(on)
        {
            GdkColor *col=getCheckRadioCol(style, ind_state, mnu);
            double   radius=opts.smallRadio ? 2.5 : 3.5,
                     offset=(opts.crSize/2.0)-radius;

            cairo_set_source_rgb(cr, CAIRO_COL(*col));
            cairo_arc(cr, x+offset+radius, y+offset+radius, radius, 0, 2*M_PI);
            cairo_fill(cr);
        }
        else if(tri)
        {
            int ty=y+(opts.crSize/2);

            GdkColor *col=getCheckRadioCol(style, ind_state, mnu);

            drawHLine(cr, CAIRO_COL(*col), 1.0, x+3, ty, opts.crSize-6);
            drawHLine(cr, CAIRO_COL(*col), 1.0, x+3, ty+1, opts.crSize-6);
        }
    }
    CAIRO_END
    }
}

static void ge_cairo_transform_for_layout(cairo_t *cr, PangoLayout *layout, int x, int y)
{
    const PangoMatrix *matrix;

    matrix = pango_context_get_matrix(pango_layout_get_context(layout));
    if (matrix)
    {
        cairo_matrix_t cairo_matrix;
        PangoRectangle rect;

        cairo_matrix_init(&cairo_matrix, matrix->xx, matrix->yx, matrix->xy, matrix->yy, matrix->x0, matrix->y0);
        pango_layout_get_extents(layout, NULL, &rect);
        pango_matrix_transform_rectangle(matrix, &rect);
        pango_extents_to_pixels(&rect, NULL);

        cairo_matrix.x0 += x - rect.x;
        cairo_matrix.y0 += y - rect.y;

        cairo_set_matrix(cr, &cairo_matrix);
    }
    else
        cairo_translate(cr, x, y);
}

static void qtcDrawLayout(GtkStyle *style, WINDOW_PARAM GtkStateType state, gboolean use_text, AREA_PARAM gint x, gint y, PangoLayout *layout)
{
    CAIRO_BEGIN
    gdk_cairo_set_source_color(cr, use_text || GTK_STATE_INSENSITIVE==state ? &style->text[state] : &style->fg[state]);
    ge_cairo_transform_for_layout(cr, layout, x, y);
    pango_cairo_show_layout(cr, layout);
    CAIRO_END
}

#define NUM_GCS 5

static void gtkDrawLayout(GtkStyle *style, WINDOW_PARAM GtkStateType state, gboolean use_text, AREA_PARAM GtkWidget *widget,
                          const gchar *detail, gint x, gint y, PangoLayout *layout)
{
    if(IS_PROGRESS)
        qtcDrawLayout(style, WINDOW_PARAM_VAL state, use_text, AREA_PARAM_VAL x, y, layout);
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
        gboolean     selectedText=opts.useHighlightForMenu && isMenuItem &&
                                  (opts.colorMenubarMouseOver
                                      ? GTK_STATE_PRELIGHT==state
                                      : ((!mb || activeMb) && GTK_STATE_PRELIGHT==state)),
                     def_but=FALSE,
                     but=isOnButton(widget, 0, &def_but),
                     swapColors=FALSE;
        GdkRectangle area2;
        GtkWidget    *parent=widget ? qtcWidgetGetParent(widget) : NULL;

        if(!opts.colorMenubarMouseOver && mb && !activeMb && GTK_STATE_PRELIGHT==state)
            state=GTK_STATE_NORMAL;

#if GTK_CHECK_VERSION(2, 10, 0) && !GTK_CHECK_VERSION(2, 10, 11)
        GtkNotebook *nb=mb || isMenuItem || !GTK_IS_LABEL(widget) || !parent || !GTK_IS_NOTEBOOK(parent) ? NULL : GTK_NOTEBOOK(parent);
#endif
        GdkColor      prevColors[NUM_GCS];
        gboolean      activeWindow=TRUE;
        int           i=0;

        if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %s %d %d %d %d %d %s  ", __FUNCTION__, pango_layout_get_text(layout), x, y, state, use_text,
                                               IS_MENU_ITEM(widget), detail ? detail : "NULL"),
                                        debugDisplayWidget(widget, 3);

        if(DETAIL("cellrenderertext") && widget && GTK_STATE_INSENSITIVE==qtcWidgetState(widget))
             state=GTK_STATE_INSENSITIVE;
             
#ifndef READ_INACTIVE_PAL /* If we reead the inactive palette, then there is no need for the following... */
        /* The following fixes the text in list views... if not used, when an item is selected it
           gets the selected text color - but when the window changes focus it gets the normal
           text color! */
         if(DETAIL("cellrenderertext") && GTK_STATE_ACTIVE==state)
             state=GTK_STATE_SELECTED;
#endif
            
        if(opts.shadeMenubarOnlyWhenActive)
            activeWindow=qtcWindowIsActive(gtk_widget_get_toplevel(widget));

        if(!isMenuItem && GTK_STATE_PRELIGHT==state)
            state=GTK_STATE_NORMAL;

        if(!use_text && parent && GTK_IS_LABEL(widget) && isOnOptionMenu(parent, 0))
            use_text=TRUE;

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
            if(/*opts.shadePopupMenu || */(mb && (activeWindow || SHADE_WINDOW_BORDER==opts.shadeMenubars)))
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
                    active=GTK_STATE_NORMAL==qtcWidgetGetState(tabLabel);
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

#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
        if(parent && GTK_IS_LABEL(widget) && GTK_IS_FRAME(parent) && !isOnStatusBar(widget, 0))
        {
            int diff=qtcWidgetGetAllocation(widget).x-qtcWidgetGetAllocation(parent).x;

            if(NO_FRAME(opts.groupBox))
                x-=MAX(0, MIN(diff, 6));
            else if(opts.gbLabel&GB_LBL_OUTSIDE)
                x-=MAX(0, MIN(diff, 2));
            else
                x+=5;
            if(area)
            {
                area2=*area;
                if(NO_FRAME(opts.groupBox))
                    area2.x-=MAX(0, MIN(diff, 6));
                else if(opts.gbLabel&GB_LBL_OUTSIDE)
                    area2.x-=MAX(0, MIN(diff, 2));
                else
                    area2.x+=5;
                area=&area2;
            }
        }
#endif

        if(!opts.useHighlightForMenu && (isMenuItem || mb) && GTK_STATE_INSENSITIVE!=state)
            state=GTK_STATE_NORMAL;

        qtcDrawLayout(style, WINDOW_PARAM_VAL selectedText ? GTK_STATE_SELECTED : state, use_text || selectedText, AREA_PARAM_VAL
                      x, y, layout);

        if(opts.embolden && def_but)
            qtcDrawLayout(style, WINDOW_PARAM_VAL selectedText ? GTK_STATE_SELECTED : state, use_text || selectedText, AREA_PARAM_VAL
                          x+1, y, layout);

        if(swapColors)
            for(i=0; i<5; ++i)
                style->text[i]=prevColors[i];
    }
}

static GdkPixbuf * scaleOrRef(GdkPixbuf *src, int width, int height)
{
    if (width == gdk_pixbuf_get_width(src) && height == gdk_pixbuf_get_height(src))
        return g_object_ref (src);
    else
        return gdk_pixbuf_scale_simple(src, width, height, GDK_INTERP_BILINEAR);
}

static GdkPixbuf * setTransparency(const GdkPixbuf *pixbuf, gdouble alpha_percent)
{
    GdkPixbuf *target;

    g_return_val_if_fail(pixbuf != NULL, NULL);
    g_return_val_if_fail(GDK_IS_PIXBUF (pixbuf), NULL);

    /* Returns a copy of pixbuf with it's non-completely-transparent pixels to
        have an alpha level "alpha_percent" of their original value. */

    target = gdk_pixbuf_add_alpha (pixbuf, FALSE, 0, 0, 0);

    if (alpha_percent == 1.0)
        return target;
    else
    {
        guint  width = gdk_pixbuf_get_width(target),
               height = gdk_pixbuf_get_height(target),
               rowstride = gdk_pixbuf_get_rowstride(target);
        guchar *data = gdk_pixbuf_get_pixels(target),
               *current;
        int    x, y;

        for (y = 0; y < height; y++)
            for (x = 0; x < width; x++)
            {
                /* The "4" is the number of chars per pixel, in this case, RGBA,
                   the 3 means "skip to the alpha" */
                current = data + (y * rowstride) + (x * 4) + 3;
                *(current) = (guchar) (*(current) * alpha_percent);
            }
    }

    return target;
}

static GdkPixbuf * gtkRenderIcon(GtkStyle *style, const GtkIconSource *source, GtkTextDirection direction,
                                 GtkStateType state, GtkIconSize size, GtkWidget *widget, const char *detail)
{
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
        settings = gtk_settings_get_for_screen(screen);
    }
#if GTK_CHECK_VERSION(2, 90, 0)
    else if (style->visual)
    {
        screen = gdk_visual_get_screen(style->visual);
        settings = gtk_settings_get_for_screen(screen);
    }
#else
    else if(style->colormap)
    {
        screen = gdk_colormap_get_screen(style->colormap);
        settings = gtk_settings_get_for_screen(screen);
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

static void gtkDrawTab(GtkStyle *style, WINDOW_PARAM GtkStateType state, GtkShadowType shadow_type, AREA_PARAM
                       GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height)
{
    QtCurveStyle *qtcurveStyle = (QtCurveStyle *)style;
    GdkColor     *arrowColor=MO_ARROW(false, &qtSettings.colors[GTK_STATE_INSENSITIVE==state
                                                                            ? PAL_DISABLED : PAL_ACTIVE]
                                                                   [COLOR_BUTTON_TEXT]);
    //if(DO_EFFECT)
    //    x--;
    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %s  ", __FUNCTION__, state, shadow_type, detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 5);

    if(isActiveCombo(widget))
        x++, y++;

    x=reverseLayout(widget) || ((widget=qtcWidgetGetParent(widget)) && reverseLayout(widget))
                ? x+1
                : x+(width>>1);

    if(opts.doubleGtkComboArrow)
    {
        int pad=opts.vArrows ? 0 : 1;
        drawArrow(WINDOW_PARAM_VAL style, arrowColor, AREA_PARAM_VAL_L,  GTK_ARROW_UP, x, y+(height>>1)-(LARGE_ARR_HEIGHT-pad), FALSE, TRUE);
        drawArrow(WINDOW_PARAM_VAL style, arrowColor, AREA_PARAM_VAL_L,  GTK_ARROW_DOWN, x, y+(height>>1)+(LARGE_ARR_HEIGHT-pad), FALSE, TRUE);
    }
    else
        drawArrow(WINDOW_PARAM_VAL style, arrowColor, AREA_PARAM_VAL_L,  GTK_ARROW_DOWN, x, y+(height>>1), FALSE, TRUE);
}

static void gtkDrawBoxGap(GtkStyle *style, WINDOW_PARAM GtkStateType state, GtkShadowType shadow_type, AREA_PARAM
                          GtkWidget *widget, const gchar *detail, gint x, gint y, gint width,  gint height, GtkPositionType gap_side,
                          gint gap_x, gint gap_width)
{
    GdkColor *col1 = &qtcPalette.background[0],
             *col2 = &qtcPalette.background[opts.borderTab ? 0 : (APPEARANCE_FLAT==opts.appearance
                                                                    ? ORIGINAL_SHADE : FRAME_DARK_SHADOW)],
             *outer = &qtcPalette.background[STD_BORDER];
    gboolean rev = reverseLayout(widget),
             thin = opts.thin&THIN_FRAMES;
    int      rightPos=(width -(gap_x + gap_width));

    CAIRO_BEGIN
    FN_CHECK

    if(thin && 0==gap_x)
        gap_x--, gap_width+=2;

    sanitizeSize(window, &width, &height);
    drawBoxGap(cr, style, GTK_SHADOW_OUT, state, widget, area, x, y,
               width, height, gap_side, gap_x, gap_width, opts.borderTab ? BORDER_LIGHT : BORDER_RAISED, TRUE);

    switch(gap_side)
    {
        case GTK_POS_TOP:
            if(gap_x > 0)
            {
                if(!thin)
                {
                    drawHLine(cr, CAIRO_COL(*col1), 1.0, x+gap_x-1, y+1, 3);
                    drawHLine(cr, CAIRO_COL(*col1), 1.0, x+gap_x-1, y, 3);
                }
                drawHLine(cr, CAIRO_COL(*outer), 1.0, x+gap_x-1, y, 2);
            }
            else if(!thin)
                drawVLine(cr, CAIRO_COL(*col1), 1.0, x+1, y, 2);

            if(rightPos >= 0)
            {
                if(!thin)
                {
                    drawHLine(cr, CAIRO_COL(*col1), 1.0, x+gap_x+gap_width-2, y+1, 3);
                    drawVLine(cr, CAIRO_COL(*col2), 1.0, x+gap_x+gap_width-2, y, rightPos ? 1 : 0);
                }
                drawHLine(cr, CAIRO_COL(*outer), 1.0, x+gap_x+gap_width-1, y, 2);
            }
            if(!(opts.square&SQUARE_TAB_FRAME) && opts.round>ROUND_SLIGHT)
                if(gap_x>0 && TAB_MO_GLOW==opts.tabMouseOver)
                    drawVLine(cr, CAIRO_COL(*outer), 1.0, rev ? x+width-2 : x+1, y, 2);
                else
                {
                    drawVLine(cr, CAIRO_COL(*outer), 1.0, rev ? x+width-1 : x, y, 3);
                    if(gap_x>0 && !thin)
                        drawHLine(cr, CAIRO_COL(qtcPalette.background[2]), 1.0, x+1, y, 1);
                }
            break;
        case GTK_POS_BOTTOM:
            if(gap_x > 0)
            {
                if(!thin)
                {
                    drawHLine(cr, CAIRO_COL(*col1), 1.0, x+gap_x-1, y+height-1, 2);
                    drawHLine(cr, CAIRO_COL(*col2), 1.0, x+gap_x-1, y+height-2, 2);
                }
                drawHLine(cr, CAIRO_COL(*outer), 1.0, x+gap_x-1, y+height-1, 2);
            }
            else if(!thin)
                drawVLine(cr, CAIRO_COL(*col1), 1.0, x+1, y+height-1, 2);

            if(rightPos >= 0)
            {
                if(!thin)
                {
                    drawHLine(cr, CAIRO_COL(*col2), 1.0, x+gap_x+gap_width-2, y+height-2, 3);
                    drawVLine(cr, CAIRO_COL(*col2), 1.0, x+gap_x+gap_width-2, y+height-1, rightPos ? 1 : 0);
                }
                drawHLine(cr, CAIRO_COL(*outer), 1.0, x+gap_x+gap_width-1, y+height-1, 2);
            }
            if(!(opts.square&SQUARE_TAB_FRAME) && opts.round>ROUND_SLIGHT)
                if(gap_x>0 && TAB_MO_GLOW==opts.tabMouseOver)
                    drawVLine(cr, CAIRO_COL(*outer), 1.0, rev ? x+width-2 : x+1, y+height-2, 2);
                else
                    drawVLine(cr, CAIRO_COL(*outer), 1.0, rev ? x+width-1 : x, y+height-3, 3);
            break;
        case GTK_POS_LEFT:
            if(gap_x>0)
            {
                if(!thin)
                {
                    drawVLine(cr, CAIRO_COL(*col1), 1.0, x+1, y+gap_x-1, 3);
                    drawVLine(cr, CAIRO_COL(*col1), 1.0, x, y+gap_x-1, 3);
                }
                drawVLine(cr, CAIRO_COL(*outer), 1.0, x, y+gap_x-1, 2);
            }
            else if(!thin)
                drawHLine(cr, CAIRO_COL(*col1), 1.0, x, y+1, 2);

            if((height-(gap_x + gap_width)) > 0)
            {
                if(!thin)
                {
                    drawVLine(cr, CAIRO_COL(*col1), 1.0, x+1, y+gap_x+gap_width-2, 3);
                    drawVLine(cr, CAIRO_COL(*col2), 1.0, x, y+gap_x+gap_width-2, 1);
                }
                drawVLine(cr, CAIRO_COL(*outer), 1.0, x, y+gap_x+gap_width-1, 2);
            }
            if(!(opts.square&SQUARE_TAB_FRAME) && opts.round>ROUND_SLIGHT)
                if(gap_x>0 && TAB_MO_GLOW==opts.tabMouseOver)
                    drawHLine(cr, CAIRO_COL(*outer), 1.0, x, y+1, 2);
                else
                {
                    drawHLine(cr, CAIRO_COL(*outer), 1.0, x, y, 3);
                    if(gap_x>0 && !thin)
                        drawHLine(cr, CAIRO_COL(qtcPalette.background[2]), 1.0, x, y+1, 1);
                }
            break;
        case GTK_POS_RIGHT:
            if(gap_x>0)
            {
                if(!thin)
                    drawVLine(cr, CAIRO_COL(*col2), 1.0, x+width-2, y+gap_x-1, 2);
                drawVLine(cr, CAIRO_COL(*outer), 1.0, x+width-1, y+gap_x-1, 2);
            }
            else if(!thin)
                drawHLine(cr, CAIRO_COL(*col1), 1.0, x+width-2, y+1, 3);

            if((height-(gap_x + gap_width)) > 0)
            {
                if(!thin)
                {
                    drawHLine(cr, CAIRO_COL(*col2), 1.0, x+width-2, y+gap_x+gap_width-2, 3);
                    drawVLine(cr, CAIRO_COL(*col2), 1.0, x+width-2, y+gap_x+gap_width-1, 2);
                }
                drawVLine(cr, CAIRO_COL(*outer), 1.0, x+width-1, y+gap_x+gap_width-1, 2);
            }
            if(!(opts.square&SQUARE_TAB_FRAME) && opts.round>ROUND_SLIGHT)
                if(gap_x>0 && TAB_MO_GLOW==opts.tabMouseOver)
                    drawHLine(cr, CAIRO_COL(*outer), 1.0, x+width-2, y+1, 2);
                else
                    drawHLine(cr, CAIRO_COL(*outer), 1.0, x+width-3, y, 3);
            break;
    }

    CAIRO_END
}

static void fillTab(cairo_t *cr, GtkStyle *style, GtkWidget *widget, GdkRectangle *area, GtkStateType state,
                    GdkColor *col, int x, int y, int width, int height, gboolean horiz, EWidget tab, gboolean grad)
{
    gboolean selected=GTK_STATE_NORMAL==state,
             flatBgnd=!CUSTOM_BGND || 0!=opts.tabBgnd;

//     if(selected && !flatBgnd)
//         drawWindowBgnd(cr, style, area, widget, x, y, width, height);

    GdkColor *c=col,
             b;
    double   alpha=1.0;

    if(selected && 0!=opts.tabBgnd)
    {
        shade(&opts, col, &b, TO_FACTOR(opts.tabBgnd));
        c=&b;
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
            drawAreaColorAlpha(cr, area, &style->bg[GTK_STATE_NORMAL], x, y, width, height, alpha);
    }
    else if(grad)
        drawBevelGradientAlpha(cr, style, area, x, y, width, height,
                               c, horiz, selected, selected ? SEL_TAB_APP : NORM_TAB_APP, tab, alpha);
    else if(!selected || flatBgnd)
        drawAreaColorAlpha(cr, area, c, x, y, width, height, alpha);
}

static void colorTab(cairo_t *cr, int x, int y, int width, int height, int round, EWidget tab, gboolean horiz)
{
    cairo_pattern_t *pt=cairo_pattern_create_linear(x, y, horiz ? x : x+width-1, horiz ? y+height-1 : y);

    clipPath(cr, x, y, width, height, tab, RADIUS_EXTERNAL, round);
    cairo_pattern_add_color_stop_rgba(pt, 0, CAIRO_COL(qtcPalette.highlight[ORIGINAL_SHADE]),
                                      WIDGET_TAB_TOP==tab ? (TO_ALPHA(opts.colorSelTab)) : 0.0);
#ifdef QTC_CAIRO_1_10_HACK
    cairo_pattern_add_color_stop_rgba(pt, 0, CAIRO_COL(qtcPalette.highlight[ORIGINAL_SHADE]),
                                      WIDGET_TAB_TOP==tab ? (TO_ALPHA(opts.colorSelTab)) : 0.0);
    cairo_pattern_add_color_stop_rgba(pt, 1, CAIRO_COL(qtcPalette.highlight[ORIGINAL_SHADE]),
                                      WIDGET_TAB_TOP==tab ? 0.0 : (TO_ALPHA(opts.colorSelTab)));
#endif
    cairo_pattern_add_color_stop_rgba(pt, 1, CAIRO_COL(qtcPalette.highlight[ORIGINAL_SHADE]),
                                      WIDGET_TAB_TOP==tab ? 0.0 : (TO_ALPHA(opts.colorSelTab)));
    cairo_set_source(cr, pt);
    cairo_rectangle(cr, x, y, width, height);
    cairo_fill(cr);
    cairo_pattern_destroy(pt);
    unsetCairoClipping(cr);
}
                    
static gboolean isMozillaTab(GtkWidget *widget)
{
    return /*isMozillaWidget(widget) */ isFixedWidget(widget) && GTK_IS_NOTEBOOK(widget);
}

static void gtkDrawExtension(GtkStyle *style, WINDOW_PARAM GtkStateType state, GtkShadowType shadow_type, AREA_PARAM
                             GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height, GtkPositionType gap_side)
{
    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %d %s  ", __FUNCTION__, state, shadow_type, gap_side, x, y, width, height,
                                           detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 3);

    sanitizeSize(window, &width, &height);

    if (DETAIL ("tab"))
    {
        GtkNotebook *notebook=GTK_IS_NOTEBOOK(widget) ? GTK_NOTEBOOK(widget) : NULL;
        gboolean    highlightingEnabled=notebook && (opts.highlightFactor || opts.coloredMouseOver);
        QtCTab      *highlightTab=highlightingEnabled ? lookupTabHash(widget, FALSE) : NULL;
        gboolean    highlight=FALSE;
        int         dark=APPEARANCE_FLAT==opts.appearance ? ORIGINAL_SHADE : FRAME_DARK_SHADOW,
                    moOffset=ROUNDED_NONE==opts.round || TAB_MO_TOP!=opts.tabMouseOver ? 1 : opts.round;
        GtkWidget   *parent=widget ? qtcWidgetGetParent(widget) : NULL;
        gboolean    firstTab=notebook ? FALSE : TRUE,
                    lastTab=notebook ? FALSE : TRUE,
                    vertical=GTK_POS_LEFT==gap_side || GTK_POS_RIGHT==gap_side,
                    active=GTK_STATE_NORMAL==state, /* Normal -> active tab? */
                    rev=(GTK_POS_TOP==gap_side || GTK_POS_BOTTOM==gap_side) && parent && reverseLayout(parent),
                    mozTab=isMozillaTab(widget),
                    glowMo=!active && notebook && opts.coloredMouseOver && TAB_MO_GLOW==opts.tabMouseOver,
                    drawOuterGlow=glowMo && !(opts.thin&THIN_FRAMES);
        int         mod=active ? 1 : 0,
                    highlightOffset=opts.highlightTab && opts.round>ROUND_SLIGHT ? 2 : 1,
                    highlightBorder=(opts.round>ROUND_FULL ? 4 : 3),
                    sizeAdjust=(!active || mozTab) && TAB_MO_GLOW==opts.tabMouseOver ? 1 : 0;
        GdkColor    *col=active
                            ? &(style->bg[GTK_STATE_NORMAL]) : &(qtcPalette.background[2]),
                    *selCol1=&qtcPalette.highlight[0],
                    *selCol2=&qtcPalette.highlight[IS_FLAT(opts.appearance) ? 0 : 3];
        GdkRectangle clipArea;
        EBorder     borderProfile=active || opts.borderInactiveTab
                                    ? opts.borderTab
                                        ? BORDER_LIGHT
                                        : BORDER_RAISED
                                    : BORDER_FLAT;

        FN_CHECK

#if !GTK_CHECK_VERSION(2, 90, 0)
        /* Hacky fix for tabs in Thunderbird */
        if(mozTab && area && area->x<(x-10))
            return;
#endif

        CAIRO_BEGIN

        /* f'in mozilla apps dont really use Gtk widgets - they just paint to a pixmap. So, no way of knowing
        the position of a tab! The 'best' look seems to be to round both corners. Not nice, but... */
        if(mozTab || GTK_APP_JAVA==qtSettings.app)
            firstTab=lastTab=TRUE;
        else if(notebook)
        {
            /* Borrowed from Qt engine... */
            GList     *children=gtk_container_get_children(GTK_CONTAINER(widget));
            int       num_children=children ? g_list_length(children) : 0,
                      i, sdiff = 10000, pos = -1,
                      first_shown=-1,
                      last_shown=-1;
            GtkWidget *p=widget;

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
                    pos=i;
                    p=tab_label;
                }

                if(page && qtcWidgetVisible(page))
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
            qtcTabSetup(widget);

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

        switch(gap_side)
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
    #if GTK_CHECK_VERSION(2, 10, 0) && !GTK_CHECK_VERSION(2, 10, 11)
                if(!active)
                    height-=2;
    #endif
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

                if(notebook && opts.highlightTab && active)
                {
                    drawHLine(cr, CAIRO_COL(*selCol1), 0.5, x+1, y+height-3, width-2);
                    drawHLine(cr, CAIRO_COL(*selCol1), 1.0, x+highlightOffset, y+height-2, width-(2*highlightOffset));

                    clipArea.y=y+height-highlightBorder;
                    clipArea.height=highlightBorder;
                    realDrawBorder(cr, style, state, &clipArea, x, y, width, height, qtcPalette.highlight, ROUNDED_BOTTOM,
                                   BORDER_FLAT, WIDGET_OTHER, 0, 3);
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
    #if GTK_CHECK_VERSION(2, 10, 0) && !GTK_CHECK_VERSION(2, 10, 11)
                if(!active)
                {
                    y+=2;
                    height-=2;
                }
    #endif
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
                    drawHLine(cr, CAIRO_COL(*selCol1), 0.5, x+1, y+2, width-2);
                    drawHLine(cr, CAIRO_COL(*selCol1), 1.0, x+highlightOffset, y+1, width-(2*highlightOffset));

                    clipArea.y=y;
                    clipArea.height=highlightBorder;
                    realDrawBorder(cr, style, state, &clipArea, x, y, width, height,
                                   qtcPalette.highlight, ROUNDED_TOP, BORDER_FLAT, WIDGET_OTHER, 0, 3);
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
    #if GTK_CHECK_VERSION(2, 10, 0) && !GTK_CHECK_VERSION(2, 10, 11)
                if(!active)
                    width-=2;
    #endif
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

                if(notebook && opts.highlightTab && active)
                {
                    drawVLine(cr, CAIRO_COL(*selCol1), 0.5, x+width-3, y+1, height-2);
                    drawVLine(cr, CAIRO_COL(*selCol1), 1.0, x+width-2, y+highlightOffset, height-(2*highlightOffset));

                    clipArea.x=x+width-highlightBorder;
                    clipArea.width=highlightBorder;
                    realDrawBorder(cr, style, state, &clipArea, x, y, width, height, qtcPalette.highlight, ROUNDED_RIGHT,
                                   BORDER_FLAT, WIDGET_OTHER, 0, 3);
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
    #if GTK_CHECK_VERSION(2, 10, 0) && !GTK_CHECK_VERSION(2, 10, 11)
                if(!active)
                {
                    x+=2;
                    width-=2;
                }
    #endif
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
                if(notebook && opts.highlightTab && active)
                {
                    drawVLine(cr, CAIRO_COL(*selCol1), 0.5, x+2, y+1, height-2);
                    drawVLine(cr, CAIRO_COL(*selCol1), 1.0, x+1, y+highlightOffset, height-(2*highlightOffset));

                    clipArea.x=x;
                    clipArea.width=highlightBorder;
                    realDrawBorder(cr, style, state, &clipArea, x, y, width, height, qtcPalette.highlight, ROUNDED_LEFT,
                                   BORDER_FLAT, WIDGET_OTHER, 0, 3);
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
        CAIRO_END
    }
    else
        parent_class->draw_extension(style, WINDOW_PARAM_VAL state, shadow_type, AREA_PARAM_VAL widget, detail, x, y, width, height, gap_side);
}

static void gtkDrawSlider(GtkStyle *style, WINDOW_PARAM GtkStateType state, GtkShadowType shadow_type, AREA_PARAM
                          GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height, GtkOrientation orientation)
{
    gboolean scrollbar=DETAIL("slider"),
             scale=DETAIL("hscale") || DETAIL("vscale");
    GdkColor new_colors[TOTAL_SHADES+1],
             *btn_colors;
    int      min=MIN_SLIDER_SIZE(opts.sliderThumbs);

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %d %s  ", __FUNCTION__, state, shadow_type, x, y, width, height,
                                           detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 3);

    CAIRO_BEGIN

#ifdef INCREASE_SB_SLIDER
    lastSlider.widget=NULL;
#endif
    /* Fix Java swing sliders looking pressed */
    if(!scrollbar && GTK_STATE_ACTIVE==state)
        state=GTK_STATE_PRELIGHT;

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
            SET_BTN_COLS(scrollbar, scale, FALSE, state)
    }

    FN_CHECK
    sanitizeSize(window, &width, &height);

    if(scrollbar || SLIDER_TRIANGULAR!=opts.sliderStyle)
    {
#ifdef INCREASE_SB_SLIDER
        if(!opts.flatSbarButtons && SHADE_NONE!=opts.shadeSliders && SCROLLBAR_NONE!=opts.scrollbarType && !isMozilla())
        {
            lastSlider.style=style;
#if GTK_CHECK_VERSION(2, 90, 0)
            lastSlider.cr=cr;
#else
            lastSlider.window=window;
#endif
            lastSlider.state=state;
            lastSlider.shadow_type=shadow_type;
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
                
        drawBox(style, WINDOW_PARAM_VAL state, shadow_type, area, widget, !scrollbar ? "qtc-slider" : "slider", x, y, width, height, FALSE);

       /* Orientation is always vertical with Mozilla, why? Anyway this hack should be OK - as we only draw
          dashes when slider is larger than 'min' pixels... */
        orientation=width<height ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;
        if(LINE_NONE!=opts.sliderThumbs && (scrollbar || SLIDER_CIRCULAR!=opts.sliderStyle) &&
           (scale || ((GTK_ORIENTATION_HORIZONTAL==orientation && width>=min) || height>=min)))
        {
            GdkColor *markers=/*opts.coloredMouseOver && GTK_STATE_PRELIGHT==state
                                ? qtcPalette.mouseover
                                : */btn_colors;
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
    {
        gboolean     coloredMouseOver=GTK_STATE_PRELIGHT==state && opts.coloredMouseOver &&
                                      !opts.colorSliderMouseOver,
                     horiz=SLIDER_TRIANGULAR==opts.sliderStyle ? (height>width || DETAIL("hscale")) : width>height;
        int          bgnd=getFillReal(state, FALSE, SHADE_DARKEN==opts.shadeSliders),
                     xo=horiz ? 8 : 0,
                     yo=horiz ? 0 : 8,
                     size=SLIDER_TRIANGULAR==opts.sliderStyle ? 15 : 13,
                     light=APPEARANCE_DULL_GLASS==opts.sliderAppearance ? 1 : 0;
        GdkColor     *colors=btn_colors,
                     *borderCols=GTK_STATE_PRELIGHT==state && (MO_GLOW==opts.coloredMouseOver ||
                                                               MO_COLORED==opts.coloredMouseOver)
                                    ? qtcPalette.mouseover : btn_colors;
        GdkPoint     clip[8];
        GtkArrowType direction=horiz ? GTK_ARROW_DOWN : GTK_ARROW_RIGHT;
        gboolean     drawLight=MO_PLASTIK!=opts.coloredMouseOver || !coloredMouseOver;
        int          borderVal=qtcPalette.mouseover==borderCols ? SLIDER_MO_BORDER_VAL : BORDER_VAL(GTK_STATE_INSENSITIVE==state);

        if(MO_GLOW==opts.coloredMouseOver && DO_EFFECT)
            x++, y++, xo++, yo++;

        cairo_new_path(cr);
        cairo_save(cr);
        
        switch(direction)
        {
            case GTK_ARROW_UP:
            default:
            case GTK_ARROW_DOWN:
                y+=2;
                {
                    GdkPoint pts[]={{x, y}, {x, y+2}, {x+2, y}, {x+8, y}, {x+10, y+2}, {x+10, y+9}, {x+5, y+14}, {x, y+9}};
                    plotPoints(cr, pts, 8);
                }
                break;
            case GTK_ARROW_RIGHT:
            case GTK_ARROW_LEFT:
                x+=2;
                {
                    GdkPoint pts[]={{x, y}, {x+2, y}, {x, y+2}, {x, y+8}, {x+2, y+10}, {x+9, y+10}, {x+14, y+5}, {x+9, y}};
                    plotPoints(cr, pts, 8);
                }
        }

        cairo_clip(cr);

        if(IS_FLAT(opts.sliderAppearance))
        {
            drawAreaColor(cr, NULL, &colors[bgnd], x+1, y+1, width-2, height-2);

            if(MO_PLASTIK==opts.coloredMouseOver && coloredMouseOver)
            {
                int col=SLIDER_MO_SHADE,
                    len=SLIDER_MO_LEN;

                if(horiz)
                {
                    drawAreaColor(cr, NULL, &qtcPalette.mouseover[col], x+1, y+1, len, size-2);
                    drawAreaColor(cr, NULL, &qtcPalette.mouseover[col], x+width-(1+len), y+1, len, size-2);
                }
                else
                {
                    drawAreaColor(cr, NULL, &qtcPalette.mouseover[col], x+1, y+1, size-2, len);
                    drawAreaColor(cr, NULL, &qtcPalette.mouseover[col], x+1, y+height-(1+len), size-2, len);
                }
            }
        }
        else
        {
            drawBevelGradient(cr, style, NULL, x, y, horiz ? width-1 : size, horiz ? size : height-1, &colors[bgnd],
                              horiz, FALSE, MODIFY_AGUA(opts.sliderAppearance), WIDGET_OTHER);

            if(MO_PLASTIK==opts.coloredMouseOver && coloredMouseOver)
            {
                int col=SLIDER_MO_SHADE,
                    len=SLIDER_MO_LEN;

                if(horiz)
                {
                    drawBevelGradient(cr, style, NULL, x+1, y+1, len, size-2, &qtcPalette.mouseover[col],
                                      horiz, FALSE, MODIFY_AGUA(opts.sliderAppearance), WIDGET_OTHER);
                    drawBevelGradient(cr, style, NULL, x+width-(1+len), y+1, len, size-2, &qtcPalette.mouseover[col],
                                      horiz, FALSE, MODIFY_AGUA(opts.sliderAppearance), WIDGET_OTHER);
                }
                else
                {
                    drawBevelGradient(cr, style, NULL, x+1, y+1, size-2, len, &qtcPalette.mouseover[col],
                                      horiz, FALSE, MODIFY_AGUA(opts.sliderAppearance), WIDGET_OTHER);
                    drawBevelGradient(cr, style, NULL, x+1, y+height-(1+len), size-2, len, &qtcPalette.mouseover[col],
                                      horiz, FALSE, MODIFY_AGUA(opts.sliderAppearance), WIDGET_OTHER);
                }
            }
        }

        unsetCairoClipping(cr);

        { /* C-Scope */
        double   xd=x+0.5,
                 yd=y+0.5,
                 radius=2.5,
                 xdg=xd-1,
                 ydg=yd-1,
                 radiusg=radius+1;
        gboolean glowMo=MO_GLOW==opts.coloredMouseOver && coloredMouseOver && DO_EFFECT;
    
        cairo_new_path(cr);
        if(glowMo)
            cairo_set_source_rgba(cr, CAIRO_COL(borderCols[GLOW_MO]), GLOW_ALPHA(FALSE));
        else
            cairo_set_source_rgb(cr, CAIRO_COL(borderCols[borderVal]));
        switch(direction)
        {
            case GTK_ARROW_UP:
            default:
            case GTK_ARROW_DOWN:
                if(glowMo)
                {
                    cairo_move_to(cr, xdg+radiusg, ydg);
                    cairo_arc(cr, xdg+12-radiusg, ydg+radiusg, radiusg, M_PI * 1.5, M_PI * 2);
                    cairo_line_to(cr, xdg+12, ydg+10.5);
                    cairo_line_to(cr, xdg+6, ydg+16.5);
                    cairo_line_to(cr, xdg, ydg+10.5);
                    cairo_arc(cr, xdg+radiusg, ydg+radiusg, radiusg, M_PI, M_PI * 1.5);
                    cairo_stroke(cr);
                    cairo_set_source_rgb(cr, CAIRO_COL(borderCols[borderVal]));
                }
                cairo_move_to(cr, xd+radius, yd);
                cairo_arc(cr, xd+10-radius, yd+radius, radius, M_PI * 1.5, M_PI * 2);
                cairo_line_to(cr, xd+10, yd+9);
                cairo_line_to(cr, xd+5, yd+14);
                cairo_line_to(cr, xd, yd+9);
                cairo_arc(cr, xd+radius, yd+radius, radius, M_PI, M_PI * 1.5);
                cairo_stroke(cr);
                if(drawLight)
                {
                    drawVLine(cr, CAIRO_COL(colors[light]), 1.0, xd+1, yd+2, 7);
                    drawHLine(cr, CAIRO_COL(colors[light]), 1.0, xd+2, yd+1, 6);
                }
                break;
            case GTK_ARROW_RIGHT:
            case GTK_ARROW_LEFT:
                if(glowMo)
                {
                    cairo_move_to(cr, xdg, ydg+12-radiusg);
                    cairo_arc(cr, xdg+radiusg, ydg+radiusg, radiusg, M_PI, M_PI * 1.5);
                    cairo_line_to(cr, xdg+10.5, ydg);
                    cairo_line_to(cr, xdg+16.5, ydg+6);
                    cairo_line_to(cr, xdg+10.5, ydg+12);
                    cairo_arc(cr, xdg+radiusg, ydg+12-radiusg, radiusg, M_PI * 0.5, M_PI);
                    cairo_stroke(cr);
                    cairo_set_source_rgb(cr, CAIRO_COL(borderCols[borderVal]));
                }
                cairo_move_to(cr, xd, yd+10-radius);
                cairo_arc(cr, xd+radius, yd+radius, radius, M_PI, M_PI * 1.5);
                cairo_line_to(cr, xd+9, yd);
                cairo_line_to(cr, xd+14, yd+5);
                cairo_line_to(cr, xd+9, yd+10);
                cairo_arc(cr, xd+radius, yd+10-radius, radius, M_PI * 0.5, M_PI);
                cairo_stroke(cr);
                if(drawLight)
                {
                    drawHLine(cr, CAIRO_COL(colors[light]), 1.0, xd+2, yd+1, 7);
                    drawVLine(cr, CAIRO_COL(colors[light]), 1.0, xd+1, yd+2, 6);
                }
        }
        } /* C-Scope */
    }

    CAIRO_END
}

static void gtkDrawShadowGap(GtkStyle *style, WINDOW_PARAM GtkStateType state, GtkShadowType shadow_type, AREA_PARAM
                             GtkWidget *widget, const gchar *detail, gint x, gint y, gint width, gint height, GtkPositionType gap_side,
                             gint gap_x, gint gap_width)
{
    CAIRO_BEGIN

    gboolean drawFrame=TRUE,
             isGroupBox=IS_GROUP_BOX(widget);

    if(isGroupBox)
    {
        if(gap_x<5)
            gap_x+=5, gap_width+=2;

        switch(opts.groupBox)
        {
            case FRAME_LINE:
            {
                GdkRectangle gap={x, y, gap_width, 1};
                drawFadedLine(cr, x, y, width, 1, &qtcPalette.background[STD_BORDER], area, &gap, FALSE, TRUE, TRUE);
                drawFrame=FALSE;
                break;
            }
            case FRAME_NONE:
                drawFrame=FALSE;
                return;
            case FRAME_SHADED:
            case FRAME_FADED:
                if(opts.gbLabel&(GB_LBL_INSIDE|GB_LBL_OUTSIDE) && widget && GTK_IS_FRAME(widget))
                {
                    GtkFrame       *frame=GTK_FRAME(widget);
                    GtkRequisition child_requisition;
                    gint           height_extra;
                    GtkStyle       *style=frame ? qtcWidgetGetStyle(GTK_WIDGET(frame)) : NULL;
                    GtkWidget      *label=frame ? qtcFrameGetLabel(frame) : NULL;

                    if(style && label)
                    {
                        gtk_widget_get_child_requisition(label, &child_requisition);
                        height_extra = (MAX(0, child_requisition.height - style->ythickness)
                                        - qtcFrameGetLabelYAlign(frame) * child_requisition.height) + 2;

                        if(opts.gbLabel&GB_LBL_INSIDE)
                            y-=height_extra, height+=height_extra, gap_width=0;
                        else if(opts.gbLabel&GB_LBL_OUTSIDE)
                            y+=height_extra, height-=height_extra, gap_width=0;
                    }
                }
                if(GTK_SHADOW_NONE!=shadow_type)
                {
                    int             round=opts.square&SQUARE_FRAME ? ROUNDED_NONE : ROUNDED_ALL;
                    double          col=opts.gbFactor<0 ? 0.0 : 1.0,
                                    radius=ROUNDED_ALL==round ? getRadius(&opts, width, height, WIDGET_FRAME, RADIUS_EXTERNAL) : 0.0;
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
#ifdef QTC_CAIRO_1_10_HACK
                            cairo_pattern_add_color_stop_rgba(pt, 0, col, col, col, TO_ALPHA(opts.gbFactor));
                            cairo_pattern_add_color_stop_rgba(pt, 1, col, col, col, 0);
#endif
                            cairo_pattern_add_color_stop_rgba(pt, 1, col, col, col, 0);
                            cairo_set_source(cr, pt);
                        }
                        cairo_fill(cr);
                        cairo_restore(cr);
                        if(pt)
                            cairo_pattern_destroy(pt);
                    }
                    if(FRAME_FADED==opts.groupBox)
                    {
                        pt=cairo_pattern_create_linear(x, y, x, y+height-1);
                        cairo_pattern_add_color_stop_rgba(pt, 0, CAIRO_COL(qtcPalette.background[STD_BORDER]), 1.0);
#ifdef QTC_CAIRO_1_10_HACK
                        cairo_pattern_add_color_stop_rgba(pt, 0, CAIRO_COL(qtcPalette.background[STD_BORDER]), 1.0);
                        cairo_pattern_add_color_stop_rgba(pt, 1, CAIRO_COL(qtcPalette.background[STD_BORDER]), 0);
#endif
                        cairo_pattern_add_color_stop_rgba(pt, 1, CAIRO_COL(qtcPalette.background[STD_BORDER]), 0);
                        setGapClip(cr, area, gap_side, gap_x, gap_width, x, y, width, height, FALSE);
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
        sanitizeSize(window, &width, &height);
        drawBoxGap(cr, style, shadow_type, state, widget, area, x, y, width, height, gap_side, gap_x, gap_width,
                   isGroupBox && FRAME_SHADED==opts.groupBox && GTK_SHADOW_NONE!=shadow_type
                    ? /*opts.gbFactor<0
                        ?*/ BORDER_SUNKEN
                        /*: BORDER_RAISED*/
                    : shadowToBorder(shadow_type),
                   FALSE);
    }
    CAIRO_END
}

static void gtkDrawHLine(GtkStyle *style, WINDOW_PARAM GtkStateType state, AREA_PARAM GtkWidget *widget,
                         const gchar *detail, gint x1, gint x2, gint y)
{
    gboolean tbar=DETAIL("toolbar");
    int      light=0,
             dark=tbar ? (LINE_FLAT==opts.toolbarSeparators ? 4 : 3) : 5;

    FN_CHECK
    CAIRO_BEGIN

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %s  ", __FUNCTION__, state, x1, x2, y, detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 3);

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

        if(offset && (GTK_APP_OPEN_OFFICE==qtSettings.app || isMozilla()))
            offset+=2;

        if(USE_LIGHTER_POPUP_MENU || opts.shadePopupMenu)
            cols=SHADE_WINDOW_BORDER==opts.shadeMenubars
                    ? qtcPalette.wborder[0]
                    : qtcPalette.menu
                        ? qtcPalette.menu
                        : qtcPalette.background;

        if(offset && (GTK_APP_OPEN_OFFICE==qtSettings.app || isMozilla()))
            offset+=2;

        //drawHLine(cr, CAIRO_COL(qtcPalette.background[MENU_SEP_SHADE]), 1.0, x1<x2 ? x1 : x2, y, abs(x2-x1));
        drawFadedLine(cr, offset+(x1<x2 ? x1 : x2), y+1, abs(x2-x1)-offset, 1, &cols[MENU_SEP_SHADE], area, NULL, true, true, true);
    }
    else
        //drawHLine(cr, CAIRO_COL(qtcPalette.background[dark]), 1.0, x1<x2 ? x1 : x2, y, abs(x2-x1));
        drawFadedLine(cr, x1<x2 ? x1 : x2, y, abs(x2-x1), 1, &qtcPalette.background[dark],  area, NULL, true, true, true);

    CAIRO_END
}

static void gtkDrawVLine(GtkStyle *style, WINDOW_PARAM GtkStateType state, AREA_PARAM GtkWidget *widget,
                         const gchar *detail, gint y1, gint y2, gint x)
{
    FN_CHECK
    CAIRO_BEGIN

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %s  ", __FUNCTION__, state, x, y1, y2, detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 3);

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
                    drawDots(cr, x, y1, 2, y2-y1, TRUE, (((y2-y1)/3.0)+0.5), 0,
                             qtcPalette.background, area, 0, 5);
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

static void gtkDrawFocus(GtkStyle *style, WINDOW_PARAM GtkStateType state, AREA_PARAM GtkWidget *widget, const gchar *detail,
                         gint x, gint y, gint width, gint height)
{
    if(GTK_IS_EDITABLE(widget))
        return;

    sanitizeSize(window, &width, &height);

    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %d %d %d %d %s ", __FUNCTION__, state, x, y, width, height, detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 3);

    {
    GtkWidget *parent=widget ? qtcWidgetGetParent(widget) : NULL;
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
#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
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
            toolbarBtn=GTK_APP_GIMP==qtSettings.app && (NULL==(text=qtcButtonGetLabelText(GTK_BUTTON(widget))) ||
                                                        '\0'==text[0]);

            if(!toolbarBtn && FOCUS_GLOW==opts.focus && !isMozilla())
                return;

            if(toolbarBtn)
            {
                if(GTK_APP_GIMP==qtSettings.app && FOCUS_GLOW==opts.focus && toolbarBtn)
                    x-=2, width+=4, y-=1, height+=2;
            }
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
        parent_class->draw_focus(style, WINDOW_PARAM_VAL state, AREA_PARAM_VAL widget, detail, x, y, width, height);
    else
    {
        gboolean drawRounded=ROUNDED;
        GdkColor *col=view && GTK_STATE_SELECTED==state
                           ? &style->text[state]
                           : &qtcPalette.focus[FOCUS_SHADE(GTK_STATE_SELECTED==state)];

        CAIRO_BEGIN

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
                    else
                        x-=3, y-=3, width+=6, height+=6;
                }

                if(FOCUS_FILLED==opts.focus)
                {
                    if(drawRounded)
                        createPath(cr, x+0.5, y+0.5, width-1, height-1, getRadius(&opts, width, height, WIDGET_OTHER,
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
                            : getRadius(&opts, width, height, WIDGET_OTHER,
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

static void gtkDrawResizeGrip(GtkStyle *style, WINDOW_PARAM GtkStateType state, AREA_PARAM GtkWidget *widget,
                              const gchar *detail, GdkWindowEdge edge, gint x, gint y, gint width, gint height)
{
    FN_CHECK
    CAIRO_BEGIN

    int size=SIZE_GRIP_SIZE-2;

    /* Clear background */
    if(IS_FLAT_BGND(opts.bgndAppearance) || !(widget && drawWindowBgnd(cr, style, area, widget, x, y, width, height)))
    {
//         gtk_style_apply_default_background(style, window, FALSE, state, area, x, y, width, height);
        if(widget && IMG_NONE!=opts.bgndImage.type)
            drawWindowBgnd(cr, style, area, widget, x, y, width, height);
    }

    switch(edge)
    {
        case GDK_WINDOW_EDGE_SOUTH_EAST:
        // Adjust Firefox's resize grip so that it can be completely covered by QtCurve's KWin resize grip.
        if(isMozilla())
            x++, y++;
        {
            GdkPoint a[]={{ x+width,       (y+height)-size},
                          { x+width,        y+height},
                          {(x+width)-size,  y+height}};
            drawPolygon(WINDOW_PARAM_VAL style, &qtcPalette.background[2], area, a, 3, TRUE);
            break;
        }
        case GDK_WINDOW_EDGE_SOUTH_WEST:
        {
            GdkPoint a[]={{(x+width)-size, (y+height)-size},
                          { x+width,        y+height},
                          {(x+width)-size,  y+height}};
            drawPolygon(WINDOW_PARAM_VAL style, &qtcPalette.background[2], area, a, 3, TRUE);
            break;
        }
        case GDK_WINDOW_EDGE_NORTH_EAST:
            // TODO!!
        case GDK_WINDOW_EDGE_NORTH_WEST:
            // TODO!!
        default:
            parent_class->draw_resize_grip(style, WINDOW_PARAM_VAL state, AREA_PARAM_VAL widget, detail, edge, x, y, width, height);
    }

    CAIRO_END
}

static void gtkDrawExpander(GtkStyle *style, WINDOW_PARAM GtkStateType state, AREA_PARAM GtkWidget *widget,
                            const gchar *detail, gint x, gint y, GtkExpanderStyle expander_style)
{
    if(DEBUG_ALL==qtSettings.debug) printf(DEBUG_PREFIX "%s %d %s  ", __FUNCTION__, state, detail ? detail : "NULL"),
                                    debugDisplayWidget(widget, 5);

    gboolean isExpander=widget && GTK_IS_EXPANDER(widget),
             fill=!isExpander || opts.coloredMouseOver || GTK_STATE_PRELIGHT!=state;
    GdkColor *col=isExpander && opts.coloredMouseOver && GTK_STATE_PRELIGHT==state
                    ? &qtcPalette.mouseover[ARROW_MO_SHADE]
                    : &style->text[ARROW_STATE(state)];

    x-=LV_SIZE>>1;
    y-=LV_SIZE>>1;

    if(GTK_EXPANDER_COLLAPSED==expander_style)
        drawArrow(WINDOW_PARAM_VAL style, col, AREA_PARAM_VAL_L, reverseLayout(widget) ? GTK_ARROW_LEFT : GTK_ARROW_RIGHT,
                  x+(LARGE_ARR_WIDTH>>1), y+LARGE_ARR_HEIGHT, FALSE, fill);
    else
        drawArrow(WINDOW_PARAM_VAL style, col, AREA_PARAM_VAL_L, GTK_ARROW_DOWN, x+(LARGE_ARR_WIDTH>>1), y+LARGE_ARR_HEIGHT, FALSE, fill);
}

#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
static GdkGC * realizeColors(GtkStyle *style, GdkColor *color)
{
    GdkGCValues gc_values;

    gdk_colormap_alloc_color(style->colormap, color, FALSE, TRUE);
    gc_values.foreground = *color;

    return gtk_gc_get(style->depth, style->colormap, &gc_values, GDK_GC_FOREGROUND);
}
#endif

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

#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
    qtcurveStyle->lv_lines_gc=opts.lvLines ? realizeColors(style, &qtSettings.colors[PAL_ACTIVE][COLOR_MID]) : NULL;
#ifndef QTC_USE_CAIRO_FOR_ARROWS
    qtcurveStyle->arrow_gc=NULL;
#endif
#endif
}

static void styleUnrealize(GtkStyle *style)
{
    QtCurveStyle *qtcurveStyle = (QtCurveStyle *)style;

    parent_class->unrealize(style);

#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
    if(opts.lvLines)
    {
        gtk_gc_release(qtcurveStyle->lv_lines_gc);
        qtcurveStyle->lv_lines_gc=NULL;
    }

#ifndef QTC_USE_CAIRO_FOR_ARROWS
    if(qtcurveStyle->arrow_gc)
    {
        g_object_unref(qtcurveStyle->arrow_gc);
        qtcurveStyle->arrow_gc=NULL;
    }
#endif
#endif
}

static void generateColors()
{
    shadeColors(&qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW], qtcPalette.background);
    shadeColors(&qtSettings.colors[PAL_ACTIVE][COLOR_BUTTON], qtcPalette.button[PAL_ACTIVE]);
    shadeColors(&qtSettings.colors[PAL_DISABLED][COLOR_BUTTON], qtcPalette.button[PAL_DISABLED]);
    shadeColors(&qtSettings.colors[PAL_ACTIVE][COLOR_SELECTED], qtcPalette.highlight);

    shadeColors(&qtSettings.colors[PAL_ACTIVE][COLOR_FOCUS], qtcPalette.focus);

    switch(opts.shadeMenubars)
    {
        case SHADE_WINDOW_BORDER:
            qtcPalette.wborder[0]=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
            qtcPalette.wborder[1]=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
            shadeColors(&qtSettings.colors[PAL_INACTIVE][COLOR_WINDOW_BORDER], qtcPalette.wborder[0]);
            shadeColors(&qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW_BORDER], qtcPalette.wborder[1]);
            break;
        case SHADE_NONE:
            memcpy(qtcPalette.menubar, qtcPalette.background, sizeof(GdkColor)*(TOTAL_SHADES+1));
            break;
        case SHADE_BLEND_SELECTED:
        {
            GdkColor mid=midColor(&qtcPalette.highlight[ORIGINAL_SHADE], &qtcPalette.background[ORIGINAL_SHADE]);
            shadeColors(&mid, qtcPalette.menubar);
            break;
        }    
        case SHADE_SELECTED:
        {
            GdkColor color;

            if(IS_GLASS(opts.appearance))
                shade(&opts, &qtcPalette.highlight[ORIGINAL_SHADE], &color, MENUBAR_GLASS_SELECTED_DARK_FACTOR);
            else
                color=qtcPalette.highlight[ORIGINAL_SHADE];

            shadeColors(&color, qtcPalette.menubar);
            break;
        }
        case SHADE_CUSTOM:
            shadeColors(&opts.customMenubarsColor, qtcPalette.menubar);
            break;
        case SHADE_DARKEN:
        {
            GdkColor color;

            shade(&opts, &qtcPalette.background[ORIGINAL_SHADE], &color, MENUBAR_DARK_FACTOR);
            shadeColors(&color, qtcPalette.menubar);
            break;
        }
    }

    switch(opts.shadeSliders)
    {
        case SHADE_SELECTED:
            qtcPalette.slider=qtcPalette.highlight;
            break;
        case SHADE_CUSTOM:
            qtcPalette.slider=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
            shadeColors(&opts.customSlidersColor, qtcPalette.slider);
            break;
        case SHADE_BLEND_SELECTED:
        {
            GdkColor mid=midColor(&qtcPalette.highlight[ORIGINAL_SHADE],
                                  &qtcPalette.button[PAL_ACTIVE][ORIGINAL_SHADE]);

            qtcPalette.slider=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
            shadeColors(&mid, qtcPalette.slider);
        }
        default:
            break;
    }

    qtcPalette.combobtn=NULL;
    switch(opts.comboBtn)
    {
        case SHADE_SELECTED:
            qtcPalette.combobtn=qtcPalette.highlight;
            break;
        case SHADE_CUSTOM:
            if(SHADE_CUSTOM==opts.shadeSliders && EQUAL_COLOR(opts.customSlidersColor, opts.customComboBtnColor))
                qtcPalette.combobtn=qtcPalette.slider;
            else
            {
                qtcPalette.combobtn=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
                shadeColors(&opts.customComboBtnColor, qtcPalette.combobtn);
            }
            break;
        case SHADE_BLEND_SELECTED:
            if(SHADE_BLEND_SELECTED==opts.shadeSliders)
                qtcPalette.combobtn=qtcPalette.slider;
            else
            {
                GdkColor mid=midColor(&qtcPalette.highlight[ORIGINAL_SHADE],
                                      &qtcPalette.button[PAL_ACTIVE][ORIGINAL_SHADE]);

                qtcPalette.combobtn=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
                shadeColors(&mid, qtcPalette.combobtn);
            }
        default:
            break;
    }

    qtcPalette.sortedlv=NULL;
    switch(opts.sortedLv)
    {
        case SHADE_DARKEN:
        {
            GdkColor color;

            qtcPalette.sortedlv=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
            shade(&opts, opts.lvButton ? &qtcPalette.button[PAL_ACTIVE][ORIGINAL_SHADE]
                                       : &qtcPalette.background[ORIGINAL_SHADE], &color, LV_HEADER_DARK_FACTOR);
            shadeColors(&color, qtcPalette.sortedlv);
            break;
        }
        case SHADE_SELECTED:
            qtcPalette.sortedlv=qtcPalette.highlight;
            break;
        case SHADE_CUSTOM:
            if(SHADE_CUSTOM==opts.shadeSliders && EQUAL_COLOR(opts.customSlidersColor, opts.customSortedLvColor))
                qtcPalette.sortedlv=qtcPalette.slider;
            else if(SHADE_CUSTOM==opts.comboBtn && EQUAL_COLOR(opts.customComboBtnColor, opts.customSortedLvColor))
                qtcPalette.sortedlv=qtcPalette.combobtn;
            else
            {
                qtcPalette.sortedlv=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
                shadeColors(&opts.customSortedLvColor, qtcPalette.sortedlv);
            }
            break;
        case SHADE_BLEND_SELECTED:
            if(SHADE_BLEND_SELECTED==opts.shadeSliders)
                qtcPalette.sortedlv=qtcPalette.slider;
            else if(SHADE_BLEND_SELECTED==opts.comboBtn)
                qtcPalette.sortedlv=qtcPalette.combobtn;
            else
            {
                GdkColor mid=midColor(&qtcPalette.highlight[ORIGINAL_SHADE],
                                      opts.lvButton ? &qtcPalette.button[PAL_ACTIVE][ORIGINAL_SHADE]
                                                    : &qtcPalette.background[ORIGINAL_SHADE]);

                qtcPalette.sortedlv=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
                shadeColors(&mid, qtcPalette.sortedlv);
            }
        default:
            break;
    }

    switch(opts.defBtnIndicator)
    {
        case IND_TINT:
        {
            GdkColor col=tint(&qtcPalette.button[PAL_ACTIVE][ORIGINAL_SHADE],
                            &qtcPalette.highlight[ORIGINAL_SHADE], DEF_BNT_TINT);
            qtcPalette.defbtn=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
            shadeColors(&col, qtcPalette.defbtn);
            break;
        }
        case IND_GLOW:
        case IND_SELECTED:
            qtcPalette.defbtn=qtcPalette.highlight;
            break;
        default:
            break;
        case IND_COLORED:
            if(SHADE_BLEND_SELECTED==opts.shadeSliders)
                qtcPalette.defbtn=qtcPalette.slider;
            else
            {
                GdkColor mid=midColor(&qtcPalette.highlight[ORIGINAL_SHADE],
                                      &qtcPalette.button[PAL_ACTIVE][ORIGINAL_SHADE]);

                qtcPalette.defbtn=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
                shadeColors(&mid, qtcPalette.defbtn);
            }
    }

    if(opts.coloredMouseOver)
    {
        qtcPalette.mouseover=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
        shadeColors(&qtSettings.colors[PAL_ACTIVE][COLOR_HOVER], qtcPalette.mouseover);
    }

    switch(opts.shadeCheckRadio)
    {
        default:
            qtcPalette.check_radio=&qtSettings.colors[PAL_ACTIVE][opts.crButton ? COLOR_BUTTON_TEXT : COLOR_TEXT];
            break;
        case SHADE_BLEND_SELECTED:
        case SHADE_SELECTED:
            qtcPalette.check_radio=&qtSettings.colors[PAL_ACTIVE][COLOR_SELECTED];
            break;
        case SHADE_CUSTOM:
            qtcPalette.check_radio=&opts.customCheckRadioColor;
    }

    if(opts.shadePopupMenu)
        memcpy(qtcPalette.menu,
               SHADE_WINDOW_BORDER==opts.shadeMenubars
                ? qtcPalette.wborder[0]
                : qtcPalette.menubar, sizeof(GdkColor)*(TOTAL_SHADES+1));
    else
    {
        GdkColor color;
        if(opts.lighterPopupMenuBgnd)
            shade(&opts, &qtcPalette.background[ORIGINAL_SHADE], &color, TO_FACTOR(opts.lighterPopupMenuBgnd));
        else
            color=qtcPalette.background[ORIGINAL_SHADE];
        shadeColors(&color, qtcPalette.menu);
    }

    /* Tear off menu items dont seem to draw they're background, and the default background
        is drawn :-(  Fix/hack this by making that background the correct color */
    if(USE_LIGHTER_POPUP_MENU || opts.shadePopupMenu)
    {
        static const char *format="style \""RC_SETTING"Mnu\" { "
                                    "bg[NORMAL]=\"#%02X%02X%02X\" "
                                    "fg[NORMAL]=\"#%02X%02X%02X\" "
                                    "text[INSENSITIVE]=\"#%02X%02X%02X\" "
                                    "} class \"GtkMenu\" style \""RC_SETTING"Mnu\" "
                                    "widget_class \"*Menu.*Label\" style \""RC_SETTING"Mnu\""
                                    " style  \""RC_SETTING"CView\" = \""RC_SETTING"Mnu\" { text[NORMAL]=\"#%02X%02X%02X\" } "
                                    " widget_class \"*<GtkMenuItem>*<GtkCellView>\" style \""RC_SETTING"CView\"";
        char *str=(char *)malloc(strlen(format)+24+1);

        if(str)
        {
            GdkColor *col=&qtcPalette.menu[ORIGINAL_SHADE];
            GdkColor text=opts.shadePopupMenu
                            ? SHADE_WINDOW_BORDER==opts.shadeMenubars
                                ? qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW_BORDER_TEXT]
                                : opts.customMenuTextColor
                                    ? opts.customMenuNormTextColor
                                    : SHADE_BLEND_SELECTED==opts.shadeMenubars || SHADE_SELECTED==opts.shadeMenubars ||
                                    (SHADE_CUSTOM==opts.shadeMenubars && TOO_DARK(qtcPalette.menubar[ORIGINAL_SHADE]))
                                    ? qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED]
                                    : qtSettings.colors[PAL_ACTIVE][COLOR_TEXT]
                            : qtSettings.colors[PAL_ACTIVE][COLOR_TEXT],
                     mid=opts.shadePopupMenu ? midColor(col, &text) : qtSettings.colors[PAL_DISABLED][COLOR_TEXT];
            sprintf(str, format, toQtColor(col->red), toQtColor(col->green), toQtColor(col->blue),
                                 toQtColor(text.red), toQtColor(text.green), toQtColor(text.blue),
                                 toQtColor(mid.red),  toQtColor(mid.green),  toQtColor(mid.blue),
                                 toQtColor(text.red), toQtColor(text.green), toQtColor(text.blue));
            gtk_rc_parse_string(str);
            free(str);
        }
    }
            
    switch(opts.menuStripe)
    {
        default:
        case SHADE_NONE:
            opts.customMenuStripeColor=qtcPalette.background[ORIGINAL_SHADE];
            break;
        case SHADE_DARKEN:
            opts.customMenuStripeColor=USE_LIGHTER_POPUP_MENU || opts.shadePopupMenu
                ? qtcPalette.menu[ORIGINAL_SHADE]
                : qtcPalette.background[MENU_STRIPE_SHADE];
            break;
        case SHADE_CUSTOM:
            break;
        case SHADE_BLEND_SELECTED:
            opts.customMenuStripeColor=midColor(&qtcPalette.highlight[ORIGINAL_SHADE],
                                                opts.lighterPopupMenuBgnd || opts.shadePopupMenu
                                                    ? &qtcPalette.menu[ORIGINAL_SHADE]
                                                    : &qtcPalette.background[ORIGINAL_SHADE]);
            break;
        case SHADE_SELECTED:
            opts.customMenuStripeColor=qtcPalette.highlight[MENU_STRIPE_SHADE];
    }

    qtcPalette.selectedcr=NULL;

    switch(opts.crColor)
    {
        case SHADE_DARKEN:
        {
            GdkColor color;

            qtcPalette.selectedcr=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
            shade(&opts, &qtcPalette.button[PAL_ACTIVE][ORIGINAL_SHADE], &color, LV_HEADER_DARK_FACTOR);
            shadeColors(&color, qtcPalette.selectedcr);
            break;
        }
        default:
        case SHADE_NONE:
            qtcPalette.selectedcr=qtcPalette.button[PAL_ACTIVE];
            break;
        case SHADE_SELECTED:
            qtcPalette.selectedcr=qtcPalette.highlight;
            break;
        case SHADE_CUSTOM:
            if(SHADE_CUSTOM==opts.shadeSliders && EQUAL_COLOR(opts.customSlidersColor, opts.customCrBgndColor))
                qtcPalette.selectedcr=qtcPalette.slider;
            else if(SHADE_CUSTOM==opts.comboBtn && EQUAL_COLOR(opts.customComboBtnColor, opts.customCrBgndColor))
                qtcPalette.selectedcr=qtcPalette.combobtn;
            else if(SHADE_CUSTOM==opts.sortedLv && EQUAL_COLOR(opts.customSortedLvColor, opts.customCrBgndColor))
                qtcPalette.selectedcr=qtcPalette.sortedlv;
            else
            {
                qtcPalette.selectedcr=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
                shadeColors(&opts.customCrBgndColor, qtcPalette.selectedcr);
            }
            break;
        case SHADE_BLEND_SELECTED:
            if(SHADE_BLEND_SELECTED==opts.shadeSliders)
                qtcPalette.selectedcr=qtcPalette.slider;
            else if(SHADE_BLEND_SELECTED==opts.comboBtn)
                qtcPalette.selectedcr=qtcPalette.combobtn;
            else if(SHADE_BLEND_SELECTED==opts.sortedLv)
                qtcPalette.selectedcr=qtcPalette.sortedlv;
            else
            {
                GdkColor mid=midColor(&qtcPalette.highlight[ORIGINAL_SHADE], &qtcPalette.button[PAL_ACTIVE][ORIGINAL_SHADE]);

                qtcPalette.selectedcr=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
                shadeColors(&mid, qtcPalette.selectedcr);
            }
    }
        
    qtcPalette.sidebar=NULL;
    if(!opts.stdSidebarButtons)
        if(SHADE_BLEND_SELECTED==opts.shadeSliders)
             qtcPalette.sidebar=qtcPalette.slider;
        else if(IND_COLORED==opts.defBtnIndicator)
            qtcPalette.sidebar=qtcPalette.defbtn;
        else
        {
            GdkColor mid=midColor(&qtcPalette.highlight[ORIGINAL_SHADE], &qtcPalette.button[PAL_ACTIVE][ORIGINAL_SHADE]);

            qtcPalette.sidebar=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
            shadeColors(&mid, qtcPalette.sidebar);
        }

    qtcPalette.progress=NULL;
    switch(opts.progressColor)
    {
        case SHADE_NONE:
            qtcPalette.progress=qtcPalette.background;
        default:
            /* Not set! */
            break;
        case SHADE_CUSTOM:
            if(SHADE_CUSTOM==opts.shadeSliders && EQUAL_COLOR(opts.customSlidersColor, opts.customProgressColor))
                qtcPalette.progress=qtcPalette.slider;
            else if(SHADE_CUSTOM==opts.comboBtn && EQUAL_COLOR(opts.customComboBtnColor, opts.customProgressColor))
                qtcPalette.progress=qtcPalette.combobtn;
            else if(SHADE_CUSTOM==opts.sortedLv && EQUAL_COLOR(opts.customSortedLvColor, opts.customProgressColor))
                qtcPalette.progress=qtcPalette.sortedlv;
            else if(SHADE_CUSTOM==opts.crColor && EQUAL_COLOR(opts.customCrBgndColor, opts.customProgressColor))
                qtcPalette.progress=qtcPalette.selectedcr;
            else
            {
                qtcPalette.progress=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
                shadeColors(&opts.customProgressColor, qtcPalette.progress);
            }
            break;
        case SHADE_BLEND_SELECTED:
            if(SHADE_BLEND_SELECTED==opts.shadeSliders)
                qtcPalette.progress=qtcPalette.slider;
            else if(SHADE_BLEND_SELECTED==opts.comboBtn)
                qtcPalette.progress=qtcPalette.combobtn;
            else if(SHADE_BLEND_SELECTED==opts.sortedLv)
                qtcPalette.progress=qtcPalette.sortedlv;
            else if(SHADE_BLEND_SELECTED==opts.crColor)
                qtcPalette.progress=qtcPalette.selectedcr;
            else
            {
                GdkColor mid=midColor(&qtcPalette.highlight[ORIGINAL_SHADE], &qtcPalette.background[ORIGINAL_SHADE]);

                qtcPalette.progress=(GdkColor *)malloc(sizeof(GdkColor)*(TOTAL_SHADES+1));
                shadeColors(&mid, qtcPalette.progress);
            }
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
    style_class->render_icon = gtkRenderIcon;
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

    if(isQtCNoteBook)
        shade(&opts, &qtcPalette.background[ORIGINAL_SHADE], &src->bg[GTK_STATE_NORMAL], TO_FACTOR(opts.tabBgnd));

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

    qtSetColors(style, rc_style);
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

    qtcurve_type_style = g_type_module_register_type(module, GTK_TYPE_STYLE, "QtCurveStyle", &object_info, 0);
}

static void qtcurve_rc_style_init(QtCurveRcStyle *qtcurve_rc)
{
#ifdef INCREASE_SB_SLIDER
    lastSlider.widget=NULL;
#endif
    if(qtInit())
    {
        generateColors();
#if !GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
        if(opts.dlgOpacity<100 || opts.bgndOpacity<100 || opts.menuBgndOpacity<100)
        {
            GdkColormap *colormap = gdk_screen_get_rgba_colormap(gdk_screen_get_default());

            if (colormap)
                gtk_widget_set_default_colormap(colormap);
        }
#endif
    }
}

static void qtcurve_rc_style_finalize(GObject *object)
{
    qtc_animation_cleanup();
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
    return gtk_check_version(GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION - GTK_INTERFACE_AGE);
}
