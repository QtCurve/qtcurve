#ifndef QTC_COMBOBOX_H_
#define QTC_COMBOBOX_H_

extern gboolean qtcComboBoxIsFocusChanged(GtkWidget *widget);
extern gboolean qtcComboBoxIsHovered(GtkWidget *widget);
extern gboolean qtcComboBoxHasFocus(GtkWidget *widget, GtkWidget *mapped);
extern void qtcComboBoxSetup(GtkWidget *frame, GtkWidget *combo);

#endif
