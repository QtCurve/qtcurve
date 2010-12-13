#ifndef QTC_TREE_VIEW_H_
#define QTC_TREE_VIEW_H_

extern void qtcTreeViewGetCell(GtkTreeView *treeView, GtkTreePath **path, GtkTreeViewColumn **column, int x, int y, int width, int height);
extern void qtcTreeViewSetup(GtkWidget *widget);
extern gboolean qtcTreeViewIsCellHovered(GtkWidget *widget, GtkTreePath *path, GtkTreeViewColumn *column);

#endif
