#ifndef QTC_TAB_H_
#define QTC_TAB_H_

extern gboolean qtcTabCurrentHoveredIndex(GtkWidget *widget);
extern void qtcTabSetup(GtkWidget *widget);
extern void qtcTabUpdateRect(GtkWidget *widget, int tabIndex, int x, int y, int width, int height);

#endif
