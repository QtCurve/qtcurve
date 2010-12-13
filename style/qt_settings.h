#ifndef QT_SETTINGS_H_
#define QT_SETTINGS_H_

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
    GTK_APP_PIDGIN
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
    char            *fonts[FONT_NUM_TOTAL],
                    *icons,
#ifdef QTC_STYLE_SUPPORT
                    *styleName,
#endif
                    *appName;
    GtkToolbarStyle toolbarStyle;
    QtIcons         iconSizes;
    gboolean        buttonIcons,
                    shadeSortedList;
    EGtkApp         app;
    gboolean        qt4,
                    inactiveChangeSelectionColor,
                    useAlpha;
    int             startDragDist,
                    startDragTime;
    QtcDebug        debug;
#ifdef FIX_FIREFOX_LOCATION_BAR
    gboolean        isBrowser;
    float           fontSize;
#endif
} QtData;

extern QtData qtSettings;

extern gboolean qtSettingsInit();
extern void qtSettingsSetColors(GtkStyle *style, GtkRcStyle *rc_style);
extern char * getAppName();
extern bool haveAlternateListViewCol();
extern gboolean isMozilla();

#endif
