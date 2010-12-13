#ifndef QTC_WINDOW_H_
#define QTC_WINDOW_H_

extern gboolean qtcWindowIsActive(GtkWidget *widget);
extern gboolean qtcWindowSetup(GtkWidget *widget, int opacity);
extern GtkWidget * qtcWindowGetMenuBar(GtkWidget *parent, int level);
extern gboolean qtcWindowSetStatusBarProp(GtkWidget *w);
extern GtkWidget * qtcWindowGetStatusBar(GtkWidget *parent, int level);
extern void qtcWindowStatusBarDBus(GtkWidget *widget, gboolean state);

#endif
