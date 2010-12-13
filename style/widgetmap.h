#ifndef QTC_WIDGET_MAP_H_
#define QTC_WIDGET_MAP_H_

extern void qtcWidgetMapSetup(GtkWidget *from, GtkWidget *to, int map);
extern GtkWidget * qtcWidgetMapGetWidget(GtkWidget *widget, int map);

#endif
