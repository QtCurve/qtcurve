#ifndef __QT_SETTINGS_H__
#define __QT_SETTINGS_H__

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

#include <gtk/gtk.h>

#define READ_INACTIVE_PAL /* Control whether QtCurve should read the inactive palette as well.. */

#define DEBUG_PREFIX "QtCurve: "
#define RC_SETTING "QtC__"

#define COL_EQ(A, B)(abs(A-B)<(3<<8))

#define EQUAL_COLOR(A, B) \
   (COL_EQ(A.red, B.red) && COL_EQ(A.green, B.green) && COL_EQ(A.blue, B.blue))

#define toQtColor(col) \
    ((col&0xFF00)>>8)
/*    ((int)((((double)col)/256.0)+0.5))*/

#define toGtkColor(col) \
    ((col<<8)+col)

typedef struct
{
    int smlTbSize,
        tbSize,
        dndSize,
        btnSize,
        mnuSize,
        dlgSize;
} QtIcons;

typedef enum
{
    COLOR_BACKGROUND,
    COLOR_BUTTON,
    COLOR_SELECTED,
    COLOR_WINDOW,

    COLOR_MID,
    COLOR_TEXT,
    COLOR_TEXT_SELECTED,
    COLOR_LV,

    COLOR_TOOLTIP,

    COLOR_BUTTON_TEXT,
    COLOR_WINDOW_TEXT,
    COLOR_TOOLTIP_TEXT,

    COLOR_FOCUS,    /* KDE4 */
    COLOR_HOVER,    /* KDE4 */
    COLOR_WINDOW_BORDER,
    COLOR_WINDOW_BORDER_TEXT,

    COLOR_NUMCOLORS,
    COLOR_NUMCOLORS_STD = COLOR_NUMCOLORS-2 /* Remove Window border colors */
} QtColorRoles;

typedef enum
{
    GTK_APP_UNKNOWN,
    GTK_APP_MOZILLA,
    GTK_APP_NEW_MOZILLA, /* For firefox3 */
    GTK_APP_OPEN_OFFICE,
    GTK_APP_VMPLAYER,
    GTK_APP_GIMP,
    GTK_APP_GIMP_PLUGIN,
    GTK_APP_JAVA,
    GTK_APP_JAVA_SWT,
    GTK_APP_EVOLUTION,
    GTK_APP_FLASH_PLUGIN,
    GTK_APP_GHB
    /*GTK_APP_GAIM*/
} EGtkApp;

typedef enum
{
    PAL_ACTIVE,
    PAL_DISABLED,
    PAL_INACTIVE
#ifndef READ_INACTIVE_PAL
        = PAL_ACTIVE
#endif
    ,

    PAL_NUMPALS
} QtPallete;

typedef enum
{
    FONT_GENERAL,
    FONT_MENU,
    FONT_TOOLBAR,

    FONT_NUM_STD,

    FONT_BOLD = FONT_NUM_STD,

    FONT_NUM_TOTAL
} QtFont;

typedef enum
{
    DEBUG_NONE,
    DEBUG_SETTINGS,
    DEBUG_ALL,
} QtcDebug;

typedef struct
{
    GdkColor        colors[PAL_NUMPALS][COLOR_NUMCOLORS]; /*,
                    inactiveSelectCol;*/
    char *fonts[FONT_NUM_TOTAL];
    char *icons;
#ifdef QTC_GTK2_STYLE_SUPPORT
    char *styleName;
#endif
    const char *appName;
    GtkToolbarStyle toolbarStyle;
    QtIcons         iconSizes;
    gboolean        buttonIcons,
                    shadeSortedList;
    EGtkApp         app;
    gboolean        qt4,
                    inactiveChangeSelectionColor,
                    useAlpha;
    int             //startDragDist,
                    startDragTime;
    QtcDebug        debug;
} QtData;

typedef struct
{
    GdkColor background[TOTAL_SHADES+1],
             button[2][TOTAL_SHADES+1],
             *slider,
             *defbtn,
             *mouseover,
             *combobtn,
             *selectedcr,
             *sortedlv,
             *sidebar,
             *progress,
             *wborder[2],
             mdi_text[2],
             menubar[TOTAL_SHADES+1],
             highlight[TOTAL_SHADES+1],
             focus[TOTAL_SHADES+1],
             menu[TOTAL_SHADES+1],
             *check_radio;
} QtCPalette;

extern QtCPalette qtcPalette;
extern Options    opts;
extern QtData     qtSettings;

gboolean qtSettingsInit();
void qtSettingsSetColors(GtkStyle *style, GtkRcStyle *rc_style);
const char *getAppName();
bool runCommand(const char* cmd, char** result);

#endif
