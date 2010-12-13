#ifndef QTC_SCROLLED_WINDOW_H_
#define QTC_SCROLLED_WINDOW_H_

extern void qtcScrolledWindowRegisterChild(GtkWidget *child);
extern void qtcScrolledWindowSetup(GtkWidget *widget);
extern gboolean qtcScrolledWindowHasFocus(GtkWidget *widget);
extern gboolean qtcScrolledWindowHovered(GtkWidget *widget);

#endif
