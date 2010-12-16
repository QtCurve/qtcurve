/*
  QtCurve(C) Craig Drummond, 2003 - 2010 craig.p.drummond@gmail.com

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
#include <stdlib.h>
#include "compatability.h"

typedef struct
{
    int          id,
                 numRects;
    GdkRectangle *rects;
} QtCTab;

static GHashTable *qtcTabHashTable = NULL;

static QtCTab * qtcTabLookupHash(void *hash, gboolean create)
{
    QtCTab *rv=NULL;

    if(!qtcTabHashTable)
        qtcTabHashTable=g_hash_table_new(g_direct_hash, g_direct_equal);

    rv=(QtCTab *)g_hash_table_lookup(qtcTabHashTable, hash);

    if(!rv && create)
    {
        int p;
            
        rv=(QtCTab *)malloc(sizeof(QtCTab));
        rv->numRects=gtk_notebook_get_n_pages(GTK_NOTEBOOK(hash));
        rv->rects=(GdkRectangle *)malloc(sizeof(GdkRectangle) * rv->numRects);
        rv->id=-1;
        
        for(p=0; p<rv->numRects; ++p)
        {
            rv->rects[p].x=rv->rects[p].y=0;
            rv->rects[p].width=rv->rects[p].height=-1;
        }
        g_hash_table_insert(qtcTabHashTable, hash, rv);
        rv=g_hash_table_lookup(qtcTabHashTable, hash);
    }

    return rv;
}

static void qtcTabRemoveHash(void *hash)
{
    if(qtcTabHashTable)
    {
        QtCTab *tab=GTK_IS_NOTEBOOK(hash) ? qtcTabLookupHash(hash, FALSE) : NULL;
        
        if(tab)
            free(tab->rects);
        g_hash_table_remove(qtcTabHashTable, hash);
    }
}

gboolean qtcTabCurrentHoveredIndex(GtkWidget *widget)
{
    QtCTab *tab=GTK_IS_NOTEBOOK(widget) ? qtcTabLookupHash(widget, FALSE) : NULL;

    return tab ? tab->id : -1;
}

void qtcTabUpdateRect(GtkWidget *widget, int tabIndex, int x, int y, int width, int height)
{
    QtCTab *tab=GTK_IS_NOTEBOOK(widget) ? qtcTabLookupHash(widget, FALSE) : NULL;

    if(tab && tabIndex>=0)
    {
        if(tabIndex>=tab->numRects)
        {
            int p;
            
            tab->rects=realloc(tab->rects, sizeof(GdkRectangle)*(tabIndex+8));
            for(p=tab->numRects; p<tabIndex+8; ++p)
            {
                tab->rects[p].x=tab->rects[p].y=0;
                tab->rects[p].width=tab->rects[p].height=-1;
            }
            tab->numRects=tabIndex+8;
        }
        tab->rects[tabIndex].x=x, tab->rects[tabIndex].y=y, tab->rects[tabIndex].width=width, tab->rects[tabIndex].height=height;
    }
}

static void qtcTabCleanup(GtkWidget *widget)
{
    if(widget)
    { 
        g_signal_handler_disconnect(G_OBJECT(widget),(gint)g_object_steal_data(G_OBJECT(widget), "QTC_TAB_MOTION_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),(gint)g_object_steal_data(G_OBJECT(widget), "QTC_TAB_LEAVE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),(gint)g_object_steal_data(G_OBJECT(widget), "QTC_TAB_PAGE_ADDED_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),(gint)g_object_steal_data(G_OBJECT(widget), "QTC_TAB_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),(gint)g_object_steal_data(G_OBJECT(widget), "QTC_TAB_UNREALIZE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),(gint)g_object_steal_data(G_OBJECT(widget), "QTC_TAB_STYLE_SET_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_TAB_HACK_SET");
        qtcTabRemoveHash(widget);
    }
}

static gboolean qtcTabStyleSet(GtkWidget *widget, GtkStyle *previous_style, gpointer user_data)
{
    qtcTabCleanup(widget);
    return FALSE;
}

static gboolean qtcTabDestroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    qtcTabCleanup(widget);
    return FALSE;
}

static void qtcSetHoveredTab(QtCTab *tab, GtkWidget *widget, int index)
{
    if(index!=tab->id)
    {
        GdkRectangle updateRect;
        int          p;
        
        updateRect.x=updateRect.y=0;
        updateRect.width=updateRect.height=-1;

        tab->id=index;

        for(p=0; p<tab->numRects; ++p)
            gdk_rectangle_union(&(tab->rects[p]), &updateRect, &updateRect);

        gtk_widget_queue_draw_area(widget, updateRect.x-4, updateRect.y-4, updateRect.width+8, updateRect.height+8);
    }
}

static gboolean qtcTabMotion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    QtCTab *tab=GTK_IS_NOTEBOOK(widget) ? qtcTabLookupHash(widget, FALSE) : NULL;

    if(tab)
    {
        int px, py, t;
        gdk_window_get_pointer(qtcWidgetGetWindow(widget), &px, &py, NULL);

        for(t=0; t < tab->numRects; t++)
        {
            if(tab->rects[t].x<=px && tab->rects[t].y<=py &&(tab->rects[t].x+tab->rects[t].width)>px &&(tab->rects[t].y+tab->rects[t].height)>py)
            {
                qtcSetHoveredTab(tab, widget, t);
                return FALSE;
            } 
        }

        qtcSetHoveredTab(tab, widget, -1);
    }

    return FALSE;
}

static gboolean qtcTabLeave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    QtCTab *prevTab=GTK_IS_NOTEBOOK(widget) ? qtcTabLookupHash(widget, FALSE) : NULL;

    if(prevTab && prevTab->id>=0)
    {
        prevTab->id=-1;
        gtk_widget_queue_draw(widget);
    }
 
    return FALSE;
}

static void qtcTabUnRegisterChild(GtkWidget *widget)
{
    if(widget && g_object_get_data(G_OBJECT(widget), "QTC_TAB_HACK_CHILD_SET"))
    {
        g_signal_handler_disconnect(G_OBJECT(widget),(gint)g_object_steal_data(G_OBJECT(widget), "QTC_TAB_C_ENTER_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),(gint)g_object_steal_data(G_OBJECT(widget), "QTC_TAB_C_LEAVE_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),(gint)g_object_steal_data(G_OBJECT(widget), "QTC_TAB_C_DESTROY_ID"));
        g_signal_handler_disconnect(G_OBJECT(widget),(gint)g_object_steal_data(G_OBJECT(widget), "QTC_TAB_C_STYLE_SET_ID"));
        if(GTK_IS_CONTAINER(widget))
            g_signal_handler_disconnect(G_OBJECT(widget),(gint)g_object_steal_data(G_OBJECT(widget), "QTC_TAB_C_ADD_ID"));
        g_object_steal_data(G_OBJECT(widget), "QTC_TAB_HACK_CHILD_SET");
    }
}

static void qtcTabUpdateChildren(GtkWidget *widget);

static gboolean qtcTabChildMotion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    qtcTabMotion((GtkWidget *)user_data, event, widget);
    return FALSE;
}

static gboolean qtcTabChildDestroy(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    qtcTabUnRegisterChild(widget);
    return FALSE;
}

static gboolean qtcTabChildStyleSet(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    qtcTabUnRegisterChild(widget);
    return FALSE;
}

static gboolean qtcTabChildAdd(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    qtcTabUpdateChildren((GtkWidget *)user_data);
    return FALSE;
}

static void qtcTabRegisterChild(GtkWidget *notebook, GtkWidget *widget)
{
    if(widget && !g_object_get_data(G_OBJECT(widget), "QTC_TAB_HACK_CHILD_SET"))
    {
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_C_ENTER_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "enter-notify-event", G_CALLBACK(qtcTabChildMotion), notebook));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_C_LEAVE_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(qtcTabChildMotion), notebook));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_C_DESTROY_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "destroy", G_CALLBACK(qtcTabChildDestroy), notebook));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_C_STYLE_SET_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "style-set", G_CALLBACK(qtcTabChildStyleSet), notebook));
        if(GTK_IS_CONTAINER(widget))
        {
            GList *children=gtk_container_get_children(GTK_CONTAINER(widget)),
                  *child;

            g_object_set_data(G_OBJECT(widget), "QTC_TAB_C_ADD_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "add", G_CALLBACK(qtcTabChildAdd), notebook));
            for(child = g_list_first(children); child; child = g_list_next(child))
                qtcTabRegisterChild(notebook, GTK_WIDGET(child->data));

            if(children)
                g_list_free(children);
        }
    }
}

static void qtcTabUpdateChildren(GtkWidget *widget)
{
    if(widget && GTK_IS_NOTEBOOK(widget))
    {
        GtkNotebook *notebook=GTK_NOTEBOOK(widget);
        int i,
            numPages=gtk_notebook_get_n_pages(notebook);

        for(i = 0; i<numPages ; ++i)
        {
            qtcTabRegisterChild(widget, gtk_notebook_get_tab_label(notebook, gtk_notebook_get_nth_page(notebook, i)));
        }
    }
}

static gboolean qtcTabPageAdded(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    qtcTabUpdateChildren(widget);
    return FALSE;
}

void qtcTabSetup(GtkWidget *widget)
{
    if(widget && !g_object_get_data(G_OBJECT(widget), "QTC_TAB_HACK_SET"))
    {
        qtcTabLookupHash(widget, TRUE);

        g_object_set_data(G_OBJECT(widget), "QTC_TAB_MOTION_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(qtcTabMotion), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_LEAVE_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(qtcTabLeave), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_PAGE_ADDED_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "page-added", G_CALLBACK(qtcTabPageAdded), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_DESTROY_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "destroy-event", G_CALLBACK(qtcTabDestroy), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_UNREALIZE_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "unrealize", G_CALLBACK(qtcTabDestroy), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_STYLE_SET_ID",
                         (gpointer)g_signal_connect(G_OBJECT(widget), "style-set", G_CALLBACK(qtcTabStyleSet), NULL));
        g_object_set_data(G_OBJECT(widget), "QTC_TAB_HACK_SET",(gpointer)1);
        qtcTabUpdateChildren(widget);
    }  
}

gboolean qtcTabIsLabel(GtkNotebook *notebook, GtkWidget *widget)
{
    int i,
        numPages=gtk_notebook_get_n_pages(notebook);

    for(i=0; i < numPages; ++i)
        if(widget==gtk_notebook_get_tab_label(notebook, gtk_notebook_get_nth_page(notebook, i)))
            return TRUE;
    return FALSE;
}

GdkRectangle qtcTabGetTabbarRect(GtkNotebook *notebook)
{
    GdkRectangle  rect, empty;
    GtkAllocation pageAllocation;
    guint         borderWidth;
    int           pageIndex;
    GtkWidget     *page;

    rect.x=rect.y=0;
    rect.width=rect.height=-1;
    empty=rect;

    // check tab visibility
    if(!(gtk_notebook_get_show_tabs(notebook) && gtk_container_get_children(GTK_CONTAINER(notebook))))
        return empty;
    
    // get full rect
    rect=qtcWidgetGetAllocation(GTK_WIDGET(notebook));

    // adjust to account for borderwidth
    borderWidth=gtk_container_get_border_width(GTK_CONTAINER(notebook));

    rect.x += borderWidth;
    rect.y += borderWidth;
    rect.height -= 2*borderWidth;
    rect.width -= 2*borderWidth;

    // get current page
    pageIndex=gtk_notebook_get_current_page(notebook);

    if(!(pageIndex >= 0 && pageIndex < gtk_notebook_get_n_pages(notebook)))
        return empty;
    page=gtk_notebook_get_nth_page(notebook, pageIndex);
    if(!page)
        return empty;

    // removes page allocated size from rect, based on tabwidget orientation
    pageAllocation=qtcWidgetGetAllocation(page);
    switch(gtk_notebook_get_tab_pos(notebook))
    {
        case GTK_POS_BOTTOM:
            rect.y += pageAllocation.height;
            rect.height -= pageAllocation.height;
            break;
        case GTK_POS_TOP:
            rect.height -= pageAllocation.height;
            break;
        case GTK_POS_RIGHT:
            rect.x += pageAllocation.width;
            rect.width -= pageAllocation.width;
            break;
        case GTK_POS_LEFT:
            rect.width -= pageAllocation.width;
            break;
    }

    return rect;
}

gboolean qtcTabHasVisibleArrows(GtkNotebook *notebook)
{
    int i;

    if(gtk_notebook_get_show_tabs(notebook))
    {
        int i,
            numPages=gtk_notebook_get_n_pages(notebook);

        for(i = 0; i<numPages; ++i)
        {
            GtkWidget *label=gtk_notebook_get_tab_label(notebook, gtk_notebook_get_nth_page(notebook, i));
            
            if(label && !gtk_widget_get_mapped(label))
                return TRUE;
        }
    }

    return FALSE;
}
