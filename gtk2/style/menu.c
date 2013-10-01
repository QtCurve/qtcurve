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

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include "compatability.h"
#include <qtcurve-utils/gtkutils.h>
#include <common/common.h>

gboolean
qtcMenuEmitSize(GtkWidget *w, unsigned int size)
{
    if (w) {
        unsigned int oldSize =
            GPOINTER_TO_INT(g_object_get_data(G_OBJECT(w), MENU_SIZE_ATOM));

        if (oldSize != size) {
            GtkWidget *topLevel = gtk_widget_get_toplevel(w);
            GdkDisplay *display = gtk_widget_get_display(topLevel);

            if (0xFFFF == size)
                size = 0;
            g_object_set_data(G_OBJECT(w), MENU_SIZE_ATOM,
                              GINT_TO_POINTER(size));
            unsigned short ssize = size;
            XChangeProperty(gdk_x11_display_get_xdisplay(display),
                            GDK_WINDOW_XID(qtcWidgetGetWindow(topLevel)),
                            gdk_x11_get_xatom_by_name_for_display(display,
                                                                  MENU_SIZE_ATOM),
                            XA_CARDINAL, 16, PropModeReplace,
                            (unsigned char*)&ssize, 1);
            return TRUE;
        }
    }
    return FALSE;
}

gboolean
objectIsA(const GObject *object, const gchar *type_name)
{
    if (object) {
        GType tmp = g_type_from_name(type_name);
        if(tmp) {
            return g_type_check_instance_is_a((GTypeInstance*)object, tmp);
        }
    }

    return FALSE;
}

/* #if !GTK_CHECK_VERSION(2, 90, 0) /\* Gtk3:TODO !!! *\/ */
#define EXTEND_MENUBAR_ITEM_HACK
/* #endif */

#ifdef EXTEND_MENUBAR_ITEM_HACK
static const int constMenuAdjust = 2;

static gboolean
menuIsSelectable(GtkWidget *menu)
{
    return !((!qtcBinGetChild(GTK_BIN(menu)) &&
              G_OBJECT_TYPE(menu) == GTK_TYPE_MENU_ITEM) ||
             GTK_IS_SEPARATOR_MENU_ITEM(menu) ||
             !qtcWidgetIsSensitive(menu) ||
             !qtcWidgetVisible(menu));
}

static gboolean
qtcMenuShellButtonPress(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    QTC_UNUSED(data);
    if (GTK_IS_MENU_BAR(widget)) {
        // QtCurve's menubars have a 2 pixel border ->
        // but want the left/top to be 'active'...
        int nx, ny;
        gdk_window_get_origin(qtcWidgetGetWindow(widget), &nx, &ny);
        if ((event->x_root-nx) <= 2.0 || (event->y_root - ny) <= 2.0) {
            GtkMenuShell *menuShell = GTK_MENU_SHELL(widget);
            GList *children =
                gtk_container_get_children(GTK_CONTAINER(menuShell));
            GList *child = children;
            gboolean rv = FALSE;

            if((event->x_root - nx) <= 2.0)
                event->x_root += 2.0;
            if((event->y_root - ny) <= 2.0)
                event->y_root += 2.0;

            while (child && !rv) {
                GtkWidget *item = child->data;
                GtkAllocation alloc = qtcWidgetGetAllocation(item);

                int cx = alloc.x + nx;
                int cy = alloc.y + ny;
                int cw = alloc.width;
                int ch = alloc.height;

                if (cx <= event->x_root && cy <= event->y_root &&
                   (cx + cw) > event->x_root && (cy + ch) > event->y_root) {
                    if (menuIsSelectable(item)) {
                        if (GDK_BUTTON_PRESS == event->type) {
                            if (item != menuShell->active_menu_item) {
                                menuShell->active = FALSE;
                                gtk_menu_shell_select_item(menuShell, item);
                                menuShell->active = TRUE;
                            } else {
                                menuShell->active = TRUE;
                                gtk_menu_shell_deselect(menuShell);
                                menuShell->active = FALSE;
                            }
                        }
                        rv = TRUE;
                    }

                    break;
                }
                child = child->next;
            }

            if(children)
                g_list_free(children);
            return rv;
        }
    }

    return FALSE;
}
#endif

static void
qtcMenuShellCleanup(GtkWidget *widget)
{
    if (GTK_IS_MENU_BAR(widget)) {
        GObject *obj = G_OBJECT(widget);
        qtcDisconnectFromData(obj, "QTC_MENU_SHELL_MOTION_ID");
        qtcDisconnectFromData(obj, "QTC_MENU_SHELL_LEAVE_ID");
        qtcDisconnectFromData(obj, "QTC_MENU_SHELL_DESTROY_ID");
        qtcDisconnectFromData(obj, "QTC_MENU_SHELL_STYLE_SET_ID");
#ifdef EXTEND_MENUBAR_ITEM_HACK
        qtcDisconnectFromData(obj, "QTC_MENU_SHELL_BUTTON_PRESS_ID");
        qtcDisconnectFromData(obj, "QTC_MENU_SHELL_BUTTON_RELEASE_ID");
#endif
        g_object_steal_data(G_OBJECT(widget), "QTC_MENU_SHELL_HACK_SET");
    }
}

static gboolean
qtcMenuShellStyleSet(GtkWidget *widget, GtkStyle *prev_style, gpointer data)
{
    QTC_UNUSED(data);
    QTC_UNUSED(prev_style);
    qtcMenuShellCleanup(widget);
    return FALSE;
}

static gboolean
qtcMenuShellDestroy(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    QTC_UNUSED(data);
    QTC_UNUSED(event);
    qtcMenuShellCleanup(widget);
    return FALSE;
}

static gboolean
qtcMenuShellMotion(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
    QTC_UNUSED(data);
    QTC_UNUSED(event);
    if (GTK_IS_MENU_SHELL(widget)) {
        gint pointer_x, pointer_y;
        GdkModifierType pointer_mask;

        gdk_window_get_pointer(qtcWidgetGetWindow(widget), &pointer_x,
                               &pointer_y, &pointer_mask);

        if (GTK_IS_CONTAINER(widget)) {
            GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
            GList *child;

            for (child = g_list_first(children);child;
                 child = g_list_next(child)) {
                if((child->data) && GTK_IS_WIDGET(child->data) &&
                   (qtcWidgetGetState(GTK_WIDGET(child->data)) !=
                    GTK_STATE_INSENSITIVE)) {
                    GtkAllocation alloc =
                        qtcWidgetGetAllocation(GTK_WIDGET(child->data));

                    if ((pointer_x >= alloc.x) && (pointer_y >= alloc.y) &&
                        (pointer_x <(alloc.x + alloc.width)) &&
                        (pointer_y <(alloc.y + alloc.height))) {
                        gtk_widget_set_state(GTK_WIDGET(child->data),
                                             GTK_STATE_PRELIGHT);
                    } else {
                        gtk_widget_set_state(GTK_WIDGET(child->data),
                                             GTK_STATE_NORMAL);
                    }
                }
            }

            if(children) {
                g_list_free(children);
            }
        }
    }

    return FALSE;
}

static gboolean
qtcMenuShellLeave(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
    QTC_UNUSED(data);
    QTC_UNUSED(event);
    if (GTK_IS_MENU_SHELL(widget) && GTK_IS_CONTAINER(widget)) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
        GList *child = NULL;

        for (child = g_list_first(children);child;child = g_list_next(child)) {
            if((child->data) && GTK_IS_MENU_ITEM(child->data) &&
               (qtcWidgetGetState(GTK_WIDGET(child->data)) !=
                GTK_STATE_INSENSITIVE)) {
                GtkWidget *submenu =
                    qtcMenuItemGetSubMenu(GTK_MENU_ITEM(child->data));
                GtkWidget *topLevel =
                    submenu ? qtcMenuGetTopLevel(submenu) : NULL;

                if (submenu &&
                    ((!GTK_IS_MENU(submenu)) ||
                     (!(qtcWidgetRealized(submenu) &&
                        qtcWidgetVisible(submenu) &&
                        qtcWidgetRealized(topLevel) &&
                        qtcWidgetVisible(topLevel))))) {
                    gtk_widget_set_state(GTK_WIDGET(child->data),
                                         GTK_STATE_NORMAL);
                }
            }
        }

        if(children) {
            g_list_free(children);
        }
    }

    return FALSE;
}

void
qtcMenuShellSetup(GtkWidget *widget)
{
    GObject *obj = G_OBJECT(widget);
    if (GTK_IS_MENU_BAR(widget) &&
        !g_object_get_data(obj, "QTC_MENU_SHELL_HACK_SET")) {
        g_object_set_data(obj, "QTC_MENU_SHELL_HACK_SET",
                          GINT_TO_POINTER(1));
        qtcConnectToData(obj, "QTC_MENU_SHELL_MOTION_ID", "motion-notify-event",
                         qtcMenuShellMotion, NULL);
        qtcConnectToData(obj, "QTC_MENU_SHELL_LEAVE_ID", "leave-notify-event",
                         qtcMenuShellLeave, NULL);
        qtcConnectToData(obj, "QTC_MENU_SHELL_DESTROY_ID", "destroy-event",
                         qtcMenuShellDestroy, NULL);
        qtcConnectToData(obj, "QTC_MENU_SHELL_STYLE_SET_ID", "style-set",
                         qtcMenuShellStyleSet, NULL);
#ifdef EXTEND_MENUBAR_ITEM_HACK
        qtcConnectToData(obj, "QTC_MENU_SHELL_BUTTON_PRESS_ID",
                         "button-press-event",
                         qtcMenuShellButtonPress, NULL);
        qtcConnectToData(obj, "QTC_MENU_SHELL_BUTTON_RELEASE_ID",
                         "button-release-event",
                         qtcMenuShellButtonPress, NULL);
#endif
    }
}
