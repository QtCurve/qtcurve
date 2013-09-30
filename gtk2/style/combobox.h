#ifndef __QTC_COMBOBOX_H__
#define __QTC_COMBOBOX_H__

#include <gtk/gtk.h>

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

gboolean qtcComboBoxIsFocusChanged(GtkWidget *widget);
gboolean qtcComboBoxIsHovered(GtkWidget *widget);
gboolean qtcComboBoxHasFocus(GtkWidget *widget, GtkWidget *mapped);
void qtcComboBoxSetup(GtkWidget *frame, GtkWidget *combo);
gboolean qtcComboHasFrame(GtkWidget *widget);

#endif
