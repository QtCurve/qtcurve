/*****************************************************************************
 *   Copyright 2003 - 2010 Craig Drummond <craig.p.drummond@gmail.com>       *
 *   Copyright 2013 - 2014 Yichao Yu <yyc1992@gmail.com>                     *
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

#include <qtcurve-utils/gtkutils.h>

typedef struct {
    int id;
    int numRects;
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

        rv = qtcNew(QtCTab);
        rv->numRects = gtk_notebook_get_n_pages(GTK_NOTEBOOK(hash));
        rv->rects = qtcNew(GdkRectangle, rv->numRects);
        rv->id = -1;

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
    if (qtcTabHashTable) {
        QtCTab *tab = GTK_IS_NOTEBOOK(hash) ? qtcTabLookupHash(hash, FALSE) : NULL;

        if (tab)
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

static void
qtcTabCleanup(GtkWidget *widget)
{
    if (widget) {
        GObject *obj = G_OBJECT(widget);
        qtcDisconnectFromData(obj, "QTC_TAB_MOTION_ID");
        qtcDisconnectFromData(obj, "QTC_TAB_LEAVE_ID");
        qtcDisconnectFromData(obj, "QTC_TAB_PAGE_ADDED_ID");
        qtcDisconnectFromData(obj, "QTC_TAB_DESTROY_ID");
        qtcDisconnectFromData(obj, "QTC_TAB_UNREALIZE_ID");
        qtcDisconnectFromData(obj, "QTC_TAB_STYLE_SET_ID");
        g_object_steal_data(obj, "QTC_TAB_HACK_SET");
        qtcTabRemoveHash(widget);
    }
}

static gboolean
qtcTabStyleSet(GtkWidget *widget, GtkStyle *prev_style, void *data)
{
    QTC_UNUSED(prev_style);
    QTC_UNUSED(data);
    qtcTabCleanup(widget);
    return FALSE;
}

static gboolean
qtcTabDestroy(GtkWidget *widget, GdkEvent *event, void *data)
{
    QTC_UNUSED(event);
    QTC_UNUSED(data);
    qtcTabCleanup(widget);
    return FALSE;
}

static void qtcSetHoveredTab(QtCTab *tab, GtkWidget *widget, int index)
{
    if (index!=tab->id) {
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

static gboolean
qtcTabMotion(GtkWidget *widget, GdkEventMotion *event, void *data)
{
    QTC_UNUSED(event);
    QTC_UNUSED(data);
    QtCTab *tab =
        GTK_IS_NOTEBOOK(widget) ? qtcTabLookupHash(widget, FALSE) : NULL;

    if(tab)
    {
        int px, py, t;
        gdk_window_get_pointer(gtk_widget_get_window(widget), &px, &py, NULL);

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

static gboolean
qtcTabLeave(GtkWidget *widget, GdkEventCrossing *event, void *data)
{
    QTC_UNUSED(event);
    QTC_UNUSED(data);
    QtCTab *prevTab =
        GTK_IS_NOTEBOOK(widget) ? qtcTabLookupHash(widget, FALSE) : NULL;

    if (prevTab && prevTab->id >= 0) {
        prevTab->id = -1;
        gtk_widget_queue_draw(widget);
    }
    return FALSE;
}

static void
qtcTabUnRegisterChild(GtkWidget *widget)
{
    GObject *obj;
    if (widget && (obj = G_OBJECT(widget)) &&
        g_object_get_data(obj, "QTC_TAB_HACK_CHILD_SET")) {
        qtcDisconnectFromData(obj, "QTC_TAB_C_ENTER_ID");
        qtcDisconnectFromData(obj, "QTC_TAB_C_LEAVE_ID");
        qtcDisconnectFromData(obj, "QTC_TAB_C_DESTROY_ID");
        qtcDisconnectFromData(obj, "QTC_TAB_C_STYLE_SET_ID");
        if (GTK_IS_CONTAINER(widget))
            qtcDisconnectFromData(obj, "QTC_TAB_C_ADD_ID");
        g_object_steal_data(obj, "QTC_TAB_HACK_CHILD_SET");
    }
}

static void qtcTabUpdateChildren(GtkWidget *widget);

static gboolean qtcTabChildMotion(GtkWidget *widget, GdkEventMotion *event, void *user_data)
{
    qtcTabMotion((GtkWidget*)user_data, event, widget);
    return FALSE;
}

static gboolean
qtcTabChildDestroy(GtkWidget *widget, GdkEventCrossing *event, void *data)
{
    QTC_UNUSED(event);
    QTC_UNUSED(data);
    qtcTabUnRegisterChild(widget);
    return FALSE;
}

static gboolean
qtcTabChildStyleSet(GtkWidget *widget, GdkEventCrossing *event, void *data)
{
    QTC_UNUSED(event);
    QTC_UNUSED(data);
    qtcTabUnRegisterChild(widget);
    return FALSE;
}

static gboolean
qtcTabChildAdd(GtkWidget *widget, GdkEventCrossing *event, void *data)
{
    QTC_UNUSED(widget);
    QTC_UNUSED(event);
    qtcTabUpdateChildren((GtkWidget*)data);
    return FALSE;
}

static void
qtcTabRegisterChild(GtkWidget *notebook, GtkWidget *widget)
{
    GObject *obj;
    if(widget && (obj = G_OBJECT(widget)) &&
       !g_object_get_data(obj, "QTC_TAB_HACK_CHILD_SET")) {
        qtcConnectToData(obj, "QTC_TAB_C_ENTER_ID", "enter-notify-event",
                         qtcTabChildMotion, notebook);
        qtcConnectToData(obj, "QTC_TAB_C_LEAVE_ID", "leave-notify-event",
                         qtcTabChildMotion, notebook);
        qtcConnectToData(obj, "QTC_TAB_C_DESTROY_ID", "destroy",
                         qtcTabChildDestroy, notebook);
        qtcConnectToData(obj, "QTC_TAB_C_STYLE_SET_ID", "style-set",
                         qtcTabChildStyleSet, notebook);
        if (GTK_IS_CONTAINER(widget)) {
            GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
            qtcConnectToData(obj, "QTC_TAB_C_ADD_ID", "add",
                             qtcTabChildAdd, notebook);
            for (GList *child = g_list_first(children);child;
                 child = g_list_next(child)) {
                qtcTabRegisterChild(notebook, GTK_WIDGET(child->data));
            }
            if (children) {
                g_list_free(children);
            }
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

static gboolean
qtcTabPageAdded(GtkWidget *widget, GdkEventCrossing *event, void *data)
{
    QTC_UNUSED(event);
    QTC_UNUSED(data);
    qtcTabUpdateChildren(widget);
    return FALSE;
}

void
qtcTabSetup(GtkWidget *widget)
{
    GObject *obj;
    if (widget && (obj = G_OBJECT(widget)) &&
        !g_object_get_data(obj, "QTC_TAB_HACK_SET")) {
        qtcTabLookupHash(widget, TRUE);
        qtcConnectToData(obj, "QTC_TAB_MOTION_ID", "motion-notify-event",
                         qtcTabMotion, NULL);
        qtcConnectToData(obj, "QTC_TAB_LEAVE_ID", "leave-notify-event",
                         qtcTabLeave, NULL);
        qtcConnectToData(obj, "QTC_TAB_PAGE_ADDED_ID", "page-added",
                         qtcTabPageAdded, NULL);
        qtcConnectToData(obj, "QTC_TAB_DESTROY_ID", "destroy-event",
                         qtcTabDestroy, NULL);
        qtcConnectToData(obj, "QTC_TAB_UNREALIZE_ID", "unrealize",
                         qtcTabDestroy, NULL);
        qtcConnectToData(obj, "QTC_TAB_STYLE_SET_ID", "style-set",
                         qtcTabStyleSet, NULL);
        g_object_set_data(obj, "QTC_TAB_HACK_SET", (void*)1);
        qtcTabUpdateChildren(widget);
    }
}

gboolean qtcTabIsLabel(GtkNotebook *notebook, GtkWidget *widget)
{
    int numPages = gtk_notebook_get_n_pages(notebook);
    for (int i = 0;i < numPages;++i) {
        if (gtk_notebook_get_tab_label(
                notebook, gtk_notebook_get_nth_page(notebook, i)) == widget) {
            return TRUE;
        }
    }
    return FALSE;
}

GdkRectangle qtcTabGetTabbarRect(GtkNotebook *notebook)
{
    GdkRectangle  rect, empty;
    GtkAllocation pageAllocation;
    unsigned int         borderWidth;
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
    pageAllocation = qtcWidgetGetAllocation(page);
    switch (gtk_notebook_get_tab_pos(notebook)) {
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
    if (gtk_notebook_get_show_tabs(notebook)) {
        int numPages = gtk_notebook_get_n_pages(notebook);
        for(int i = 0;i < numPages;i++) {
            GtkWidget *label = gtk_notebook_get_tab_label(
                notebook, gtk_notebook_get_nth_page(notebook, i));
#if GTK_CHECK_VERSION(2, 20, 0)
            if (label && !gtk_widget_get_mapped(label)) {
                return TRUE;
            }
#else
            if (label && !GTK_WIDGET_MAPPED(label)) {
                return TRUE;
            }
#endif
        }
    }
    return FALSE;
}
