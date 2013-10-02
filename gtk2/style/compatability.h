#ifndef __QTC_COMPATABILITY_H__
#define __QTC_COMPATABILITY_H__

#include <gtk/gtk.h>

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

#if GTK_CHECK_VERSION(2, 90, 0)

static inline gboolean
qtcIsProgressBar(GtkWidget *w)
{
    if (GTK_IS_PROGRESS_BAR(w)) {
        GtkAllocation alloc;
        gtk_widget_get_allocation(w, &alloc);
        return -1 != alloc.x && -1 != alloc.y;
    }
    return FALSE;
}

static inline GtkOrientation
qtcGetWidgetOrientation(GtkWidget *w)
{
    if (GTK_IS_ORIENTABLE(w)) {
        return gtk_orientable_get_orientation(GTK_ORIENTABLE(w));
    } else {
        GtkAllocation alloc;
        gtk_widget_get_allocation(w, &alloc);
        return alloc.width > alloc.height;
    }
}

#define qtcButtonIsDepressed(W)         (GTK_SHADOW_IN==shadow_type)
#define qtcToolbarGetOrientation(W)     qtcGetWidgetOrientation(W)
#define qtcRangeGetOrientation(W)       qtcGetWidgetOrientation(W)
#define qtcRangeHasStepperA(W)          (SCROLLBAR_NONE!=opts.scrollbarType && SCROLLBAR_PLATINUM!=opts.scrollbarType)
#define qtcRangeHasStepperB(W)          (SCROLLBAR_NEXT==opts.scrollbarType)
#define qtcRangeHasStepperC(W)          (SCROLLBAR_KDE==opts.scrollbarType || SCROLLBAR_PLATINUM==opts.scrollbarType)
#define qtcRangeHasStepperD(W)          (SCROLLBAR_NONE!=opts.scrollbarType && SCROLLBAR_NEXT!=opts.scrollbarType)
#define IS_PROGRESS_BAR(W)              qtcIsProgressBar(W)
#define IS_PROGRESS                     (GTK_IS_PROGRESS_BAR(widget) || DETAIL("progressbar"))

#define qtcStyleApplyDefBgnd(SBG, ST)   gtk_style_apply_default_background(style, cr, gtk_widget_get_window(widget), ST, x, y, width, height)

#define QtcRegion                       cairo_region_t
#define qtcRegionRect                   cairo_region_create_rectangle
#define qtcRegionXor                    cairo_region_xor
#define qtcRegionDestroy                cairo_region_destroy

#define QtcRect                         cairo_rectangle_int_t

#define QTC_KEY_m GDK_KEY_m
#define QTC_KEY_M GDK_KEY_M
#define QTC_KEY_s GDK_KEY_s
#define QTC_KEY_S GDK_KEY_S

#else

#define qtcButtonIsDepressed(W)         (GTK_BUTTON((W))->depressed)
#define qtcToolbarGetOrientation(W)     (GTK_TOOLBAR(W)->orientation)
#define qtcRangeGetOrientation(W)       (GTK_RANGE(W)->orientation)
#define qtcRangeHasStepperA(W)          (GTK_RANGE(W)->has_stepper_a)
#define qtcRangeHasStepperB(W)          (GTK_RANGE(W)->has_stepper_b)
#define qtcRangeHasStepperC(W)          (GTK_RANGE(W)->has_stepper_c)
#define qtcRangeHasStepperD(W)          (GTK_RANGE(W)->has_stepper_d)
#define IS_PROGRESS_BAR(W)              (GTK_IS_PROGRESS_BAR((W)) && (W)->allocation.x != -1 && (W)->allocation.y != -1)
#define IS_PROGRESS                     (GTK_IS_PROGRESS(widget) || GTK_IS_PROGRESS_BAR(widget) || DETAIL("progressbar"))

#define qtcStyleApplyDefBgnd(SBG, ST)   gtk_style_apply_default_background(style, window, SBG, ST, area, x, y, width, height)

#define QtcRegion                       GdkRegion
#define qtcRegionRect                   gdk_region_rectangle
#define qtcRegionXor                    gdk_region_xor
#define qtcRegionDestroy                gdk_region_destroy

#define QtcRect                         GdkRectangle

#define QTC_KEY_m                       GDK_m
#define QTC_KEY_M                       GDK_M
#define QTC_KEY_s                       GDK_s
#define QTC_KEY_S                       GDK_S

#endif

#endif
