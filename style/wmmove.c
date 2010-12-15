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
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include "compatability.h"
#include "common.h"
#include "helpers.h"
#include "qt_settings.h"
#include "menu.h"

extern Options opts;

static int       qtcWMMoveLastX=-1;
static int       qtcWMMoveLastY=-1;
static int       qtcWMMoveTimer=0;
static GtkWidget *qtcWMMoveDragWidget=NULL;

static void qtcWMMoveStopTimer()
{
    if(qtcWMMoveTimer)
        g_source_remove(qtcWMMoveTimer);
    qtcWMMoveTimer=0;
}

static void qtcWMMoveReset()
{
    qtcWMMoveLastX=-1;
    qtcWMMoveLastY=-1;
    qtcWMMoveDragWidget=NULL;
    qtcWMMoveStopTimer();
}

static void qtcWMMoveStore(GtkWidget *widget, GdkEventButton *event)
{
    qtcWMMoveLastX=event ? event->x_root : -1;
    qtcWMMoveLastY=event ? event->y_root : -1;
    qtcWMMoveDragWidget=widget;
}

static gboolean qtcWMMoveButtonRelease(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

static void qtcWMMoveTrigger(GtkWidget *w, int x, int y)
{
    XEvent     xev;
    GtkWindow  *topLevel=GTK_WINDOW(gtk_widget_get_toplevel(w));
    GdkWindow  *window=gtk_widget_get_window(GTK_WIDGET(topLevel));
    GdkDisplay *display=gtk_widget_get_display(GTK_WIDGET(topLevel));
    GdkWindow  *root=gdk_screen_get_root_window(gtk_window_get_screen(topLevel));

    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = gdk_x11_get_xatom_by_name_for_display(display, "_NET_WM_MOVERESIZE");
    xev.xclient.display = GDK_DISPLAY_XDISPLAY(display);
    xev.xclient.window = GDK_WINDOW_XID(window);
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = x;
    xev.xclient.data.l[1] = y;
    xev.xclient.data.l[2] = 8; // NET::Move
    xev.xclient.data.l[3] = Button1;
    xev.xclient.data.l[4] = 0;
    XUngrabPointer(GDK_DISPLAY_XDISPLAY(display), CurrentTime);

    XSendEvent(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XID(root), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
    /* force a release as some widgets miss it... */
    qtcWMMoveButtonRelease(w, NULL, NULL);
}

static gboolean qtcWMMoveWithinWidget(GtkWidget *widget, GdkEventButton *event)
{
    GdkWindow *window = gtk_widget_get_parent_window(widget);

    if(window)
    {
        GtkAllocation alloc=qtcWidgetGetAllocation(widget);
        int           nx=0,
                      ny=0,
                      adjust=0; /* TODO !!! */

        // Need to get absolute co-ordinates...
        alloc.x-=adjust;
        alloc.y-=adjust;
        alloc.width+=adjust;
        alloc.height+=adjust;
        gdk_window_get_origin(gtk_widget_get_parent_window(widget), &nx, &ny);
        alloc.x+=nx;
        alloc.y+=ny;

        return alloc.x<=event->x_root && alloc.y<=event->y_root &&
               (alloc.x+alloc.width)>event->x_root &&(alloc.y+alloc.height)>event->y_root;
    }
    return TRUE;
}

static gboolean isBlackListed(GObject *object)
{
    static const char *widgets[]={ "GtkPizza", "GladeDesignLayout", "MetaFrames" , 0 };
    
    int i;
    
    for(i=0; widgets[i]; ++i)
        if(objectIsA(object, widgets[i]))
            return TRUE;
    return FALSE;
}

static gboolean qtcWMMoveChildrenUseEvent(GtkWidget *widget, GdkEventButton *event, gboolean inNoteBook)
{
    // accept, by default
    gboolean usable = TRUE;

    // get children and check
    GList *children = gtk_container_get_children(GTK_CONTAINER(widget)),
          *child;

    for(child = g_list_first(children); child && usable; child = g_list_next(child))
    {
        // cast child to GtkWidget
        if(GTK_IS_WIDGET(child->data))
        {
            GtkWidget* childWidget=GTK_WIDGET(child->data);

            // check widget state and type
            if(GTK_STATE_PRELIGHT==qtcWidgetGetState(childWidget))
            {
                // if widget is prelight, we don't need to check where event happen,
                // any prelight widget indicate we can't do a move
                usable = FALSE;
            }
            else if(GTK_IS_NOTEBOOK( childWidget ) )
            {
                inNoteBook = TRUE;
            }
            else if(event && qtcWMMoveWithinWidget(childWidget, event))
            {
                GdkWindow *window = gtk_widget_get_window(childWidget);
                if(window && gdk_window_is_visible(window))
                {
                    // TODO: one could probably check here whether widget is enabled or not,
                    // and accept if widget is disabled.
                    if(isBlackListed(G_OBJECT(childWidget)))
                    {
                        usable = FALSE;
                    }
                    else if(GTK_IS_BUTTON(childWidget) && qtcWidgetGetState(childWidget)!=GTK_STATE_INSENSITIVE)
                    {
                        usable = false;
                    }
                    else if(gtk_widget_get_events(childWidget)&(GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK))
                    {
                        // widget listening to press event
                        usable = FALSE;
                    }
                    else if(GTK_IS_MENU_ITEM(childWidget))
                    {
                        // deal with menu item, GtkMenuItem only listen to
                        // GDK_BUTTON_PRESS_MASK when state == GTK_STATE_PRELIGHT
                        // so previous check are invalids :(
                        usable = FALSE;
                    }
                    else if(GTK_IS_SCROLLED_WINDOW(childWidget) && (!inNoteBook || gtk_widget_is_focus( childWidget)))
                    {
                        // Scrolled do not send release events
                        // to parents so not usable
                        usable = FALSE;
                    }
                }
            }

            // if child is a container and event has been accepted so far, also check it, recursively
            if(usable && GTK_IS_CONTAINER(childWidget))
                usable = qtcWMMoveChildrenUseEvent(childWidget, event, inNoteBook);
        }
    }

    if(children)
        g_list_free(children);

    return usable;
}

static gboolean qtcWMMoveUseEvent(GtkWidget *widget, GdkEventButton *event)
{
    if(!GTK_IS_CONTAINER(widget))
        return TRUE;
    
    // if widget is a notebook, accept if there is no hovered tab
    if(GTK_IS_NOTEBOOK(widget))
    {
        GtkWidget *parent;
        if(opts.windowDrag>WM_DRAG_MENU_AND_TOOLBAR && GTK_APP_PIDGIN==qtSettings.app &&
            (parent=qtcWidgetGetParent(widget)) && GTK_IS_BOX(parent) &&
            (parent=qtcWidgetGetParent(parent)) && GTK_IS_WINDOW(parent))
            return FALSE;
        return -1==qtcTabCurrentHoveredIndex(widget);
    }

    return qtcWMMoveChildrenUseEvent(widget, event, FALSE);
}

static gboolean qtcWWMoveStartDelayedDrag(gpointer data)
{
    if(qtcWMMoveDragWidget)
        qtcWMMoveTrigger(qtcWMMoveDragWidget, qtcWMMoveLastX, qtcWMMoveLastY);
}

static gboolean qtcWMMoveIsWindowDragWidget(GtkWidget *widget, GdkEventButton *event)
{
    if(opts.windowDrag && (!event || (qtcWMMoveWithinWidget(widget, event) && qtcWMMoveUseEvent(widget, event))))
    {
        qtcWMMoveStore(widget, event);
        // Start timer
        qtcWMMoveStopTimer();
        qtcWMMoveTimer=g_timeout_add(qtSettings.startDragTime, (GSourceFunc)qtcWWMoveStartDelayedDrag, NULL);

        return TRUE;
    }
    return FALSE;
}

static gboolean qtcWMMoveButtonPress(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    if (GDK_BUTTON_PRESS==event->type && 1==event->button && qtcWMMoveIsWindowDragWidget(widget, event))
    {
        qtcWMMoveDragWidget=widget;
        return TRUE;
    }

    return FALSE;
}

static gboolean qtcWMMoveDragEnd(GtkWidget *widget)
{
    if (widget==qtcWMMoveDragWidget)
    {
        gtk_grab_remove(widget);
        gdk_pointer_ungrab(CurrentTime);
        qtcWMMoveReset();
        return TRUE;
    }

    return FALSE;
}

static gboolean qtcWMMoveButtonRelease(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    return qtcWMMoveDragEnd(widget);
}

static void qtcWMMoveCleanup(GtkWidget *widget)
{
    if (g_object_get_data(G_OBJECT(widget), "QTC_WM_MOVE_HACK_SET"))
    {
        if(widget==qtcWMMoveDragWidget)
            qtcWMMoveReset();
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_MOTION_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_LEAVE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_STYLE_SET_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_BUTTON_PRESS_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),
                                    (gint)g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_BUTTON_RELEASE_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_WM_MOVE_HACK_SET");
    }
}

static gboolean qtcWMMoveStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcWMMoveCleanup(widget);
    return FALSE;
}
 
static gboolean qtcWMMoveDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcWMMoveCleanup(widget);
    return FALSE;
}

static gboolean qtcWMMoveMotion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    if (qtcWMMoveDragWidget==widget)
    {
        // check displacement with respect to drag start
        const int distance=abs(qtcWMMoveLastX - event->x_root) + abs(qtcWMMoveLastY - event->y_root);

        if(distance > 0)
            qtcWMMoveStopTimer();

        if(distance < qtSettings.startDragDist)
            return FALSE;
        qtcWMMoveTrigger(widget, event->x_root, event->y_root);
        return TRUE;
    }

    return FALSE;
}

static gboolean qtcWMMoveLeave(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    return qtcWMMoveDragEnd(widget);
}

void qtcWMMoveSetup(GtkWidget *widget)
{
    if(widget && GTK_IS_WINDOW(widget) && !gtk_window_get_decorated(GTK_WINDOW(widget)))
        return;

    if (widget && !isFakeGtk() && !g_object_get_data(G_OBJECT(widget), "QTC_WM_MOVE_HACK_SET"))
    {
        gtk_widget_add_events(widget, GDK_BUTTON_RELEASE_MASK|GDK_BUTTON_PRESS_MASK|GDK_LEAVE_NOTIFY_MASK|GDK_BUTTON1_MOTION_MASK);
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_HACK_SET", (gpointer)1);
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_MOTION_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(qtcWMMoveMotion), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_LEAVE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(qtcWMMoveLeave), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_DESTROY_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event", G_CALLBACK(qtcWMMoveDestroy), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_STYLE_SET_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "style-set", G_CALLBACK(qtcWMMoveStyleSet), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_BUTTON_PRESS_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "button-press-event", G_CALLBACK(qtcWMMoveButtonPress), widget));
        g_object_set_data(G_OBJECT(widget), "QTC_WM_MOVE_BUTTON_RELEASE_ID",
                          (gpointer)g_signal_connect(G_OBJECT(widget), "button-release-event", G_CALLBACK(qtcWMMoveButtonRelease), widget));
    }  
}
