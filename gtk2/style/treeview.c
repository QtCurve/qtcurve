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
    GtkTreePath       *path;
    GtkTreeViewColumn *column;
    gboolean          fullWidth;
} QtCTreeView;

static GHashTable *qtcTreeViewTable=NULL;

static QtCTreeView * qtcTreeViewLookupHash(void *hash, gboolean create)
{
    QtCTreeView *rv=NULL;

    if(!qtcTreeViewTable)
        qtcTreeViewTable=g_hash_table_new(g_direct_hash, g_direct_equal);

    rv=(QtCTreeView *)g_hash_table_lookup(qtcTreeViewTable, hash);

    if(!rv && create)
    {
        rv = qtcNew(QtCTreeView);
        rv->path=NULL;
        rv->column=NULL;
        rv->fullWidth=FALSE;
        g_hash_table_insert(qtcTreeViewTable, hash, rv);
        rv=g_hash_table_lookup(qtcTreeViewTable, hash);
    }

    return rv;
}

static void qtcTreeViewRemoveFromHash(void *hash)
{
    if(qtcTreeViewTable)
    {
        QtCTreeView *tv=qtcTreeViewLookupHash(hash, FALSE);
        if(tv)
        {
            if(tv->path)
                gtk_tree_path_free(tv->path);
            g_hash_table_remove(qtcTreeViewTable, hash);
        }
    }
}

void qtcTreeViewGetCell(GtkTreeView *treeView, GtkTreePath **path, GtkTreeViewColumn **column, int x, int y, int width, int height)
{
    int      pos;
    GdkPoint points[4]={ {x+1, y+1}, {x+1, y+height-1}, {x+width-1, y+1}, {x+width, y+height-1} };

    for(pos=0; pos<4 && !(*path); ++pos)
        gtk_tree_view_get_path_at_pos(treeView, points[pos].x, points[pos].y, path, column, 0L, 0L);
}

static void
qtcTreeViewCleanup(GtkWidget *widget)
{
    GObject *obj;
    if (widget && (obj = G_OBJECT(widget)) &&
        g_object_get_data(obj, "QTC_TREE_VIEW_SET")) {
        qtcTreeViewRemoveFromHash(widget);
        qtcDisconnectFromData(obj, "QTC_TREE_VIEW_DESTROY_ID");
        qtcDisconnectFromData(obj, "QTC_TREE_VIEW_UNREALIZE_ID");
        qtcDisconnectFromData(obj, "QTC_TREE_VIEW_STYLE_SET_ID");
        qtcDisconnectFromData(obj, "QTC_TREE_VIEW_MOTION_ID");
        qtcDisconnectFromData(obj, "QTC_TREE_VIEW_LEAVE_ID");
        g_object_steal_data(obj, "QTC_TREE_VIEW_SET");
    }
}

static gboolean
qtcTreeViewStyleSet(GtkWidget *widget, GtkStyle *prev_style, void *data)
{
    QTC_UNUSED(prev_style);
    QTC_UNUSED(data);
    qtcTreeViewCleanup(widget);
    return FALSE;
}

static gboolean
qtcTreeViewDestroy(GtkWidget *widget, GdkEvent *event, void *data)
{
    QTC_UNUSED(event);
    QTC_UNUSED(data);
    qtcTreeViewCleanup(widget);
    return FALSE;
}

static gboolean qtcTreeViewSamePath(GtkTreePath *a, GtkTreePath *b)
{
    return a ? (b && !gtk_tree_path_compare(a, b)) : !b;
}

static void qtcTreeViewUpdatePosition(GtkWidget *widget, int x, int y)
{
    if(GTK_IS_TREE_VIEW(widget))
    {
        QtCTreeView *tv=qtcTreeViewLookupHash(widget, FALSE);
        if(tv)
        {
            GtkTreeView       *treeView=GTK_TREE_VIEW(widget);
            GtkTreePath       *path=NULL;
            GtkTreeViewColumn *column=NULL;

            gtk_tree_view_get_path_at_pos(treeView, x, y, &path, &column, 0L, 0L);

            if(!qtcTreeViewSamePath(tv->path, path))
            {
                // prepare update area
                // get old rectangle
                GdkRectangle  oldRect={0, 0, -1, -1 },
                              newRect={0, 0, -1, -1 },
                              updateRect;
                GtkAllocation alloc=qtcWidgetGetAllocation(widget);

                if(tv->path && tv->column)
                    gtk_tree_view_get_background_area(treeView, tv->path, tv->column, &oldRect);
                if(tv->fullWidth)
                    oldRect.x = 0, oldRect.width = alloc.width;

                // get new rectangle and update position
                if(path && column)
                    gtk_tree_view_get_background_area(treeView, path, column, &newRect);
                if(path && column && tv->fullWidth)
                    newRect.x = 0, newRect.width = alloc.width;

                // take the union of both rectangles
                if(oldRect.width > 0 && oldRect.height > 0)
                {
                    if(newRect.width > 0 && newRect.height > 0)
                        gdk_rectangle_union(&oldRect, &newRect, &updateRect);
                    else
                        updateRect = oldRect;
                }
                else
                    updateRect = newRect;

                // store new cell info
                if(tv->path)
                    gtk_tree_path_free(tv->path);
                tv->path=path ? gtk_tree_path_copy(path) : NULL;
                tv->column=column;

                // convert to widget coordinates and schedule redraw
                gtk_tree_view_convert_bin_window_to_widget_coords(treeView, updateRect.x, updateRect.y, &updateRect.x, &updateRect.y);
                gtk_widget_queue_draw_area(widget, updateRect.x, updateRect.y, updateRect.width, updateRect.height);
            }

            if(path)
                gtk_tree_path_free(path);
        }
    }
}

gboolean qtcTreeViewIsCellHovered(GtkWidget *widget, GtkTreePath *path, GtkTreeViewColumn *column)
{
    QtCTreeView *tv=qtcTreeViewLookupHash(widget, FALSE);
    return tv && (tv->fullWidth || tv->column==column) && qtcTreeViewSamePath(path, tv->path);
}

static gboolean
qtcTreeViewMotion(GtkWidget *widget, GdkEventMotion *event, void *data)
{
    QTC_UNUSED(data);
    if (event && event->window && GTK_IS_TREE_VIEW(widget) &&
        gtk_tree_view_get_bin_window(GTK_TREE_VIEW(widget)) == event->window) {
        qtcTreeViewUpdatePosition(widget, event->x, event->y);
    }
    return FALSE;
}

static gboolean
qtcTreeViewLeave(GtkWidget *widget, GdkEventMotion *event, void *data)
{
    QTC_UNUSED(event);
    QTC_UNUSED(data);
    if (GTK_IS_TREE_VIEW(widget)) {
        QtCTreeView *tv=qtcTreeViewLookupHash(widget, FALSE);
        if(tv)
        {
            GtkTreeView   *treeView=GTK_TREE_VIEW(widget);
            GdkRectangle  rect={0, 0, -1, -1 };
            GtkAllocation alloc = qtcWidgetGetAllocation(widget);

            if(tv->path && tv->column)
                gtk_tree_view_get_background_area(treeView, tv->path, tv->column, &rect);
            if(tv->fullWidth)
                rect.x = 0, rect.width = alloc.width;

            if(tv->path)
                gtk_tree_path_free(tv->path);
            tv->path=NULL;
            tv->column=NULL;

            gtk_tree_view_convert_bin_window_to_widget_coords(treeView, rect.x, rect.y, &rect.x, &rect.y);
            gtk_widget_queue_draw_area(widget, rect.x, rect.y, rect.width, rect.height);
        }
    }
    return FALSE;
}

void
qtcTreeViewSetup(GtkWidget *widget)
{
    GObject *obj;
    if (widget && (obj = G_OBJECT(widget)) &&
        !g_object_get_data(obj, "QTC_TREE_VIEW_SET")) {
        QtCTreeView *tv = qtcTreeViewLookupHash(widget, TRUE);
        GtkTreeView *treeView = GTK_TREE_VIEW(widget);
        GtkWidget *parent = gtk_widget_get_parent(widget);

        if (tv) {
            int x, y;
#if GTK_CHECK_VERSION(2, 90, 0) /* Gtk3:TODO !!! */
            tv->fullWidth = TRUE;
#else
            gtk_widget_style_get(widget, "row_ending_details",
                                 &tv->fullWidth, NULL);
#endif
            gdk_window_get_pointer(gtk_widget_get_window(widget), &x, &y, 0L);
            gtk_tree_view_convert_widget_to_bin_window_coords(treeView, x, y,
                                                              &x, &y);
            qtcTreeViewUpdatePosition(widget, x, y);
            g_object_set_data(obj, "QTC_TREE_VIEW_SET", (void*)1);
            qtcConnectToData(obj, "QTC_TREE_VIEW_DESTROY_ID", "destroy-event",
                             qtcTreeViewDestroy, NULL);
            qtcConnectToData(obj, "QTC_TREE_VIEW_UNREALIZE_ID", "unrealize",
                             qtcTreeViewDestroy, NULL);
            qtcConnectToData(obj, "QTC_TREE_VIEW_STYLE_SET_ID", "style-set",
                             qtcTreeViewStyleSet, NULL);
            qtcConnectToData(obj, "QTC_TREE_VIEW_MOTION_ID",
                             "motion-notify-event", qtcTreeViewMotion, NULL);
            qtcConnectToData(obj, "QTC_TREE_VIEW_LEAVE_ID",
                             "leave-notify-event", qtcTreeViewLeave, NULL);
        }

        if (!gtk_tree_view_get_show_expanders(treeView))
            gtk_tree_view_set_show_expanders(treeView, TRUE);
        if (gtk_tree_view_get_enable_tree_lines(treeView))
            gtk_tree_view_set_enable_tree_lines(treeView, FALSE);

        if (GTK_IS_SCROLLED_WINDOW(parent) &&
            GTK_SHADOW_IN !=
            gtk_scrolled_window_get_shadow_type(GTK_SCROLLED_WINDOW(parent))) {
            gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(parent),
                                                GTK_SHADOW_IN);
        }
    }
}

gboolean qtcTreeViewCellIsLeftOfExpanderColumn(GtkTreeView *treeView, GtkTreeViewColumn *column)
{
    // check expander column
    GtkTreeViewColumn *expanderColumn=gtk_tree_view_get_expander_column(treeView);

    if(!expanderColumn || column == expanderColumn)
        return FALSE;
    else
    {
        gboolean found=FALSE,
                 isLeft=FALSE;

        // get all columns
        GList *columns=gtk_tree_view_get_columns(treeView),
              *child;

        for(child = g_list_first(columns); child; child = g_list_next(child))
        {
            if(!GTK_IS_TREE_VIEW_COLUMN(child->data))
                continue;
            else
            {
                GtkTreeViewColumn *childCol=GTK_TREE_VIEW_COLUMN(child->data);
                if(childCol == expanderColumn)
                {
                    if(found)
                        isLeft = TRUE;
                }
                else if(found)
                    break;
                else if(column == childCol)
                    found = TRUE;
            }
        }

        if(columns)
            g_list_free(columns);
        return isLeft;
    }
}
