/*
  QtCurve (C) Craig Drummond, 2003 - 2007 Craig.Drummond@lycos.co.uk

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

#include "config.h"
#include "common.h"
#define CONFIG_READ
#include "config_file.c"
#include <gtk/gtk.h>
#include <time.h>
#include <gdk/gdkcolor.h>
#include <gtk/gtkenums.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <stdio.h>

#define QTC_READ_INACTIVE_PAL /* Control whether QtCurve should read the inactive palette as well.. */
#define QTC_DEFAULT_TO_KDE3   /* Should we default to KDE3, or KDE4 settings when not running under KDE? */

#define toQtColor(col) \
    col>>8
/*    ((int)((((double)col)/256.0)+0.5))*/

#define toGtkColor(col) \
    col<<8

/*
#define QTC_DEBUG
*/

static void generateMidColor(GdkColor *a, GdkColor *b, GdkColor *mid, double factor);

static int strncmp_i(const char *s1, const char *s2, int num)
{
    char c1,
         c2;
    int  i;

    for(i=0; -1==num || i<num; ++i)
    {
        c1=*s1++;
        c2=*s2++;
        if(!c1 || !c2)
            break;
        if(isupper(c1))
            c1=tolower (c1);
        if(isupper(c2))
            c2=tolower (c2);
        if(c1!=c2)
            break;
    }
    return (int)c2-(int)c1;
}

#define strcmp_i(A, B) strncmp_i(A, B, -1)

struct QtIcons
{
    int smlTbSize,
        tbSize,
        dndSize,
        btnSize,
        mnuSize,
        dlgSize;
};

enum QtColorRoles
{
    COLOR_BACKGROUND,
    COLOR_BUTTON,
    COLOR_SELECTED,
    COLOR_WINDOW,

    COLOR_FOREGROUND,
    COLOR_MID,
    COLOR_TEXT,
    COLOR_TEXT_SELECTED,
    COLOR_BUTTON_TEXT,
    COLOR_LV,
    COLOR_NONE,
    COLOR_NUMCOLORS=COLOR_NONE  /* NONE does not count! */
};

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
    GTK_APP_EVOLUTION
    /*GTK_APP_GAIM*/
} EGtkApp;

enum QtPallete
{
    PAL_ACTIVE,
    PAL_INACTIVE
#ifndef QTC_READ_INACTIVE_PAL
        = PAL_ACTIVE
#endif
    ,

    PAL_NUMPALS
};

struct QtData
{
    GdkColor        colors[PAL_NUMPALS][COLOR_NUMCOLORS],
                    inactiveSelectCol;
    char            *font,
                    *icons,
                    *boldfont,
                    *styleName;
    GtkToolbarStyle toolbarStyle;
    struct QtIcons  iconSizes;
    gboolean        buttonIcons,
                    shadeSortedList;
    EGtkApp         app;
    gboolean        qt4;
};

#include <gmodule.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

#define DEFAULT_KDE_ACT_PAL \
"active=#000000^e#dddfe4^e#ffffff^e#ffffff^e#555555^e#c7c7c7^e#000000^e#ffffff^e#000000^e#ffffff^e#efefef^e#000000^e#678db2^e#ffffff^e#0000ee^e#52188b^e"

#ifdef QTC_READ_INACTIVE_PAL
#define DEFAULT_KDE_INACT_PAL \
"inactive=#000000^e#dddfe4^e#ffffff^e#ffffff^e#555555^e#c7c7c7^e#000000^e#ffffff^e#000000^e#ffffff^e#efefef^e#000000^e#678db2^e#ffffff^e#0000ee^e#52188b^e"
#endif

#define DEFAULT_KDE_FONT      "Sans Serif"
#define DEFAULT_KDE_FONT_SIZE 10.0
#define MAX_LINE_LEN 1024

struct QtData qtSettings;

static gboolean isMozilla()
{
    return GTK_APP_MOZILLA==qtSettings.app || GTK_APP_NEW_MOZILLA==qtSettings.app;
}

static gboolean useQt3Settings()
{
    static const char *vers=NULL;

    if(!vers)
        vers=getenv("KDE_SESSION_VERSION");

#ifdef QTC_DEFAULT_TO_KDE3
    return !vers || atoi(vers)<4;
#else
    return vers && atoi(vers)<4);
#endif
}

static const char * defaultIcons()
{
    return qtSettings.qt4 ? "oxygen" : "crystalsvg";
}

enum
{
    SECT_NONE,
    SECT_PALETTE,
    SECT_GENERAL,
    SECT_KDE,
    SECT_ICONS,
    SECT_TOOLBAR_STYLE,
    SECT_MAIN_TOOLBAR_ICONS,
    SECT_SMALL_ICONS,
    SECT_QT
};

/*
  Qt uses the following predefined weights: 
    Light    = 25,
    Normal   = 50,
    DemiBold = 63,
    Bold     = 75,
    Black    = 87

  ...need to categorize mid way, i.e. 0->37 = light, 38->56 = normal, etc...
*/

enum
{
    WEIGHT_NORMAL   = 38,   /* Less than this = light */
    WEIGHT_DEMIBOLD = 57,
    WEIGHT_BOLD     = 69,
    WEIGHT_BLACK    = 81
};

static const char * weightStr(int w)
{
    if(w<WEIGHT_NORMAL)
        return "light";
    else if(w<WEIGHT_DEMIBOLD)
        return "";
    else if(w<WEIGHT_BOLD)
        return "demibold";
    else if(w<WEIGHT_BLACK)
        return "bold";
    else
        return "black";
}

static const char * italicStr(int i)
{
    return i ? "Italic" : "";
}

enum
{
    RD_ACT_PALETTE       = 0x0001,
    RD_INACT_PALETTE     = 0x0002,
    RD_FONT              = 0x0004,
    RD_CONTRAST          = 0x0008,
    RD_ICONS             = 0x0010,
    RD_TOOLBAR_STYLE     = 0x0020,
    RD_TOOLBAR_ICON_SIZE = 0x0040,
    RD_BUTTON_ICONS      = 0x0080,
    RD_SMALL_ICON_SIZE   = 0x0100,
    RD_LIST_COLOR        = 0x0200,
    RD_STYLE             = 0x0400,
    RD_LIST_SHADE        = 0x0800
};

static char * getKdeHome()
{
/* TODO: Call kde-config ! */
    static char *kdeHome=NULL;

    if(!kdeHome)
    {
        char *env=getenv(getuid() ? "KDEHOME" : "KDEROOTHOME");

        if(env)
            kdeHome=env;
        else
        {
            static char kdeHomeStr[QTC_MAX_FILENAME_LEN+1];

            const char *home=getHome();

            if(home && strlen(home)<(QTC_MAX_FILENAME_LEN-5))
            {
                sprintf(kdeHomeStr, "%s/.kde", home);
                kdeHome=kdeHomeStr;
            }
        }
    }

    return kdeHome;
}

static char * themeFileSub(const char *prefix, const char *name, char **tmpStr, const char *sub)
{
    *tmpStr=realloc(*tmpStr, strlen(prefix)+1+strlen(sub)+1+strlen(name)+strlen(QTC_THEME_SUFFIX)+1);

    if(*tmpStr)
    {
        struct stat st;

        sprintf(*tmpStr, "%s/%s/%s%s", prefix, sub, name, QTC_THEME_SUFFIX);

        if(0==stat(*tmpStr, &st))
            return *tmpStr;
    }

    return NULL;
}

static char * themeFile(const char *prefix, const char *name, char **tmpStr)
{
    char *f=themeFileSub(prefix, name, tmpStr, QTC_THEME_DIR);

    if(!f)
        f=themeFileSub(prefix, name, tmpStr, QTC_THEME_DIR4);

    return f;
}

static void parseQtColors(char *line, int p)
{
    int  n=-1;
    char *l=strtok(line, "#");

    while(l)
    {
        if(8==strlen(l))
            switch(n)
            {
                case 0:
                    setRgb(&qtSettings.colors[p][COLOR_FOREGROUND], l);
                    break;
                case 1:
                    setRgb(&qtSettings.colors[p][COLOR_BUTTON], l);
                    break;
                case 5:
                    setRgb(&qtSettings.colors[p][COLOR_MID], l);
                    break;
                case 6:
                    setRgb(&qtSettings.colors[p][COLOR_TEXT], l);
                    break;
                case 8:
                    setRgb(&qtSettings.colors[p][COLOR_BUTTON_TEXT], l);
                    break;
                case 9:
                    setRgb(&qtSettings.colors[p][COLOR_BACKGROUND], l);
                    break;
                case 10:
                    setRgb(&qtSettings.colors[p][COLOR_WINDOW], l);
                    break;
                case 12:
                    setRgb(&qtSettings.colors[p][COLOR_SELECTED], l);
                    break;
                case 13:
                    setRgb(&qtSettings.colors[p][COLOR_TEXT_SELECTED], l);
                    break;
                default:
                    break;
            }
        else
            if(n>-1)
                break;

        n++;
        if(n>13)
            break;
        l=strtok(NULL, "#");
    }
}

static int readRc(const char *rc, int rd, Options *opts, gboolean absolute, gboolean setDefaultFont, gboolean qt4)
{
    const char *home=absolute ? NULL : getHome();
    int        found=0,
               weight=WEIGHT_NORMAL,
               italic=0,
               fixedW=0;
    float      size=DEFAULT_KDE_FONT_SIZE;
    const char *family=DEFAULT_KDE_FONT,
               *foundry="*";
    char       line[QTC_MAX_INPUT_LINE_LEN+1],
               fontLine[QTC_MAX_INPUT_LINE_LEN+1];

    if(absolute || NULL!=home)
    {
        char     fname[256];
        FILE     *f;

        if(absolute)
            strcpy(fname, rc);
        else
            sprintf(fname, "%s/%s", home, rc);

        f=fopen(fname, "r");

        if(f)
        {
            int section=SECT_NONE;

            while(found!=rd && NULL!=fgets(line, QTC_MAX_INPUT_LINE_LEN, f))
                if(line[0]=='[')
                {
                    if(qt4)
                        if(0==strncmp_i(line, "[Qt]", 4))
                            section=SECT_QT;
                        else
                            section=SECT_NONE;
                    else
                        if(0==strncmp_i(line, "[Palette]", 9))
                            section=SECT_PALETTE;
                        else if(0==strncmp_i(line, "[General]", 9))
                            section=SECT_GENERAL;
                        else if(0==strncmp_i(line, "[KDE]", 5))
                            section=SECT_KDE;
                        else if(opts->mapKdeIcons && 0==strncmp_i(line, "[Icons]", 7))
                            section=SECT_ICONS;
                        else if(0==strncmp_i(line, "[Toolbar style]", 15))
                            section=SECT_TOOLBAR_STYLE;
                        else if(opts->mapKdeIcons && 0==strncmp_i(line, "[MainToolbarIcons]", 18))
                            section=SECT_MAIN_TOOLBAR_ICONS;
                        else if(opts->mapKdeIcons && 0==strncmp_i(line, "[SmallIcons]", 12))
                            section=SECT_SMALL_ICONS;
                        else
                            section=SECT_NONE;
                }
                else if (SECT_ICONS==section && rd&RD_ICONS && !(found&RD_ICONS) &&
                         0==strncmp_i(line, "Theme=", 6))
                {
                    char *eq=strstr(line, "=");

                    if(eq && ++eq)
                    {
                        unsigned int len=strlen(eq);

                        if(qtSettings.icons)
                        {
                            free(qtSettings.icons);
                            qtSettings.icons=NULL;
                        }

                        qtSettings.icons=(char *)malloc(len+1);
                        strcpy(qtSettings.icons, eq);
                        if('\n'==qtSettings.icons[len-1])
                            qtSettings.icons[len-1]='\0';
                    }

                    found|=RD_ICONS;
                }
                else if (SECT_SMALL_ICONS==section && rd&RD_SMALL_ICON_SIZE && !(found&RD_SMALL_ICON_SIZE) &&
                         0==strncmp_i(line, "Size=", 5))
                {
                    char *eq=strstr(line, "=");

                    if(eq && ++eq)
                    {
                        int size=atoi(eq);

                        qtSettings.iconSizes.smlTbSize=size;
                        qtSettings.iconSizes.btnSize=size;
                        qtSettings.iconSizes.mnuSize=size;
                        found|=RD_SMALL_ICON_SIZE;
                    }
                }
                else if (SECT_TOOLBAR_STYLE==section && rd&RD_TOOLBAR_STYLE &&
                         !(found&RD_TOOLBAR_STYLE) && 0==strncmp_i(line, "IconText=", 9))
                {
                    char *eq=strstr(line, "=");

                    if(eq && ++eq)
                    {
                        if(0==strncmp_i(eq, "IconOnly", 8))
                            qtSettings.toolbarStyle=GTK_TOOLBAR_ICONS;
                        else if(0==strncmp_i(eq, "TextOnly", 8))
                            qtSettings.toolbarStyle=GTK_TOOLBAR_TEXT;
                        else if(0==strncmp_i(eq, "IconTextRight", 13))
                            qtSettings.toolbarStyle=GTK_TOOLBAR_BOTH_HORIZ;
                        else if(0==strncmp_i(eq, "IconTextBottom", 14))
                            qtSettings.toolbarStyle=GTK_TOOLBAR_BOTH;
                        found|=RD_TOOLBAR_STYLE;
                    }
                }
                else if (SECT_MAIN_TOOLBAR_ICONS==section && rd&RD_TOOLBAR_ICON_SIZE &&
                         !(found&RD_TOOLBAR_ICON_SIZE) && 0==strncmp_i(line, "Size=", 5))
                {
                    char *eq=strstr(line, "=");

                    if(eq && ++eq)
                    {
                        qtSettings.iconSizes.tbSize=atoi(eq);
                        found|=RD_TOOLBAR_ICON_SIZE;
                    }
                }
                else if (SECT_KDE==section && rd&RD_BUTTON_ICONS && !(found&RD_BUTTON_ICONS) &&
                         0==strncmp_i(line, "ShowIconsOnPushButtons=", 23))
                {
                    char *eq=strstr(line, "=");

                    if(eq && ++eq)
                    {
                        qtSettings.buttonIcons=0==strncmp_i(eq, "true", 4);
                        found|=RD_BUTTON_ICONS;
                    }
                }
                else if(rd&RD_CONTRAST && !(found&RD_CONTRAST) &&
                         ( (!qt4 && SECT_KDE==section && 0==strncmp_i(line, "contrast=", 9)) ||
                           ( qt4 && SECT_QT==section  && 0==strncmp_i(line, "KDE\\contrast=", 13))))
                {
                    char *l=strchr(line, '=');
                    l++;
                    sscanf(l, "%i", &(opts->contrast));
                    if(opts->contrast>10 || opts->contrast<0)
                        opts->contrast=7;
                    found|=RD_CONTRAST;
                }
                else if(SECT_GENERAL==section && rd&RD_LIST_COLOR && !(found&RD_LIST_COLOR) &&
                        0==strncmp_i(line, "alternateBackground=", 20))
                {
                    sscanf(&line[20], "%d,%d,%d\n", &qtSettings.colors[PAL_ACTIVE][COLOR_LV].red,
                                                    &qtSettings.colors[PAL_ACTIVE][COLOR_LV].green,
                                                    &qtSettings.colors[PAL_ACTIVE][COLOR_LV].blue);
                    qtSettings.colors[PAL_ACTIVE][COLOR_LV].red=toGtkColor(qtSettings.colors[PAL_ACTIVE][COLOR_LV].red);
                    qtSettings.colors[PAL_ACTIVE][COLOR_LV].green=toGtkColor(qtSettings.colors[PAL_ACTIVE][COLOR_LV].green);
                    qtSettings.colors[PAL_ACTIVE][COLOR_LV].blue=toGtkColor(qtSettings.colors[PAL_ACTIVE][COLOR_LV].blue);

                    found|=RD_LIST_COLOR;
                }
                else if(SECT_GENERAL==section && rd&RD_LIST_SHADE && !(found&RD_LIST_SHADE) &&
                        0==strncmp_i(line, "shadeSortColumn=", 16))
                {
                    char *eq=strstr(line, "=");

                    if(eq && ++eq)
                    {
                        qtSettings.shadeSortedList=0==strncmp_i(eq, "true", 4);
                        found|=RD_LIST_SHADE;
                    }
                }
                else if(( (!qt4 && SECT_PALETTE==section) || (qt4 && SECT_QT==section)) && rd&RD_ACT_PALETTE && !(found&RD_ACT_PALETTE) &&
                        (qt4 ? 0==strncmp_i(line, "Palette\\active=", 15) : 0==strncmp_i(line, "active=", 7)))
                {
                    parseQtColors(line, PAL_ACTIVE);
                    found|=RD_ACT_PALETTE;
                }
#ifdef QTC_READ_INACTIVE_PAL
                else if(( (!qt4 && SECT_PALETTE==section) || (qt4 && SECT_QT==section)) && rd&RD_INACT_PALETTE && !(found&RD_INACT_PALETTE) &&
                        (qt4 ? 0==strncmp_i(line, "Palette\\inactive=", 17) : 0==strncmp_i(line, "inactive=", 9)))
                {
                    parseQtColors(line, PAL_INACTIVE);
                    found|=RD_INACT_PALETTE;
                }
#endif
                else if (((!qt4 && SECT_GENERAL==section) || (qt4 && SECT_QT==section)) &&
                         rd&RD_STYLE && !(found&RD_STYLE)&& 0==strncmp_i(line, "style=", 6))

                {
                    int len=strlen(line);
                    qtSettings.styleName=realloc(qtSettings.styleName, strlen(&line[6])+1);
                    if('\n'==line[len-1])
                        line[len-1]='\0';
                    strcpy(qtSettings.styleName, &line[6]);
                    found|=RD_STYLE;
                }
                else if (( !qt4 && SECT_GENERAL==section && rd&RD_FONT && !(found&RD_FONT) &&
                            0==strncmp_i(line, "font=", 5)) ||
                         ( qt4 && SECT_QT==section && rd&RD_FONT && !(found&RD_FONT) &&
                            0==strncmp_i(line, "font=\"", 6)) )
                {
                    int   n=-1,
                          rc_weight=WEIGHT_NORMAL,
                          rc_italic=0,
                          rc_fixedW=0;
                    float rc_size=12.0;
                    char  *l,
                          *rc_family=NULL,
                          *rc_foundry=NULL;

                    if(qt4) /* Convert Qt4's font= syntax to Qt3 style... */
                    {
                        int len=strlen(line),
                            i;

                        strcpy(fontLine, "font=");
                        for(i=6; i<QTC_MAX_INPUT_LINE_LEN && i<len; ++i)
                            if('\"'==line[i])
                            {
                                fontLine[i-1]='\0';
                                break;
                            }
                            else
                                fontLine[i-1]=line[i];
                    }
                    else
                        memcpy(fontLine, line, QTC_MAX_INPUT_LINE_LEN+1);
                    l=strtok(fontLine, "=");
                    found|=RD_FONT;

                    while(l)
                    {
                        switch(n)
                        {
                            case 0:  /* Family - and foundry(maybe!) (ignore X11 and XFT) */
                            {
                                char *ob=NULL,
                                     *cb=NULL;

                                if(NULL!=(ob=strchr(l, '[')) && ob!=l && NULL!=(cb=strchr(l, ']')))
                                {
                                    ob[-1]='\0';
                                    *cb='\0';
                                    rc_foundry=&(ob[1]);

                                    if(0==strcmp_i(rc_foundry, "xft") ||
                                       0==strcmp_i(rc_foundry, "x11"))
                                        rc_foundry=NULL;
                                }
                                else  /* Sometimes .kderc has "adobe-helvetica" */
                                {
                                    char *dash=NULL;

                                    if(NULL!=(dash=strchr(l, '-')))
                                    {
                                        rc_foundry=l;
                                        *dash='\0';
                                        l=++dash;
                                    }
                                }

                                rc_family=l;
                                break;
                            }
                            case 1:  /* Point size */
                                sscanf(l, "%f", &rc_size);
                                break;
                            case 4:  /* Weight */
                                sscanf(l, "%d", &rc_weight);
                                break;
                            case 5:  /* Slant */
                                sscanf(l, "%d", &rc_italic);
                                break;
                            case 8:  /* Spacing */
                                sscanf(l, "%d", &rc_fixedW);
                                break;
                            default:
                                break;
                        }

                        n++;
                        if(n>8 && NULL!=family)
                        {
                            weight=rc_weight;
                            italic=rc_italic;
                            fixedW=rc_fixedW;
                            size=rc_size;
                            family=rc_family;
                            foundry=rc_foundry;
                            break;
                        }
                        l=strtok(NULL, ",");
                    }
                }
                else if(found==rd)
                    break;

            fclose(f);
        }
    }

    if(rd&RD_ACT_PALETTE && !(found&RD_ACT_PALETTE))
    {
        strncpy(line, DEFAULT_KDE_ACT_PAL, QTC_MAX_INPUT_LINE_LEN);
        line[QTC_MAX_INPUT_LINE_LEN]='\0';
        parseQtColors(line, PAL_ACTIVE);
    }

#ifdef QTC_READ_INACTIVE_PAL
    if(rd&RD_INACT_PALETTE && !(found&RD_INACT_PALETTE))
    {
        strncpy(line, DEFAULT_KDE_INACT_PAL, QTC_MAX_INPUT_LINE_LEN);
        line[QTC_MAX_INPUT_LINE_LEN]='\0';
        parseQtColors(line, PAL_INACTIVE);
    }
#endif

    if(rd&RD_FONT && (found&RD_FONT || (!qtSettings.font && setDefaultFont)))  /* No need to check if read in */
    {
        if(qtSettings.font)
        {
            free(qtSettings.font);
            qtSettings.font=NULL;
        }
        if(qtSettings.boldfont)
        {
            free(qtSettings.boldfont);
            qtSettings.boldfont=NULL;
        }

        qtSettings.font=(char *)malloc(
            strlen(family) + 1 +
            strlen(weightStr(weight)) + 1 +
            strlen(italicStr(italic)) + 1 +
            20+  /* point size */ +1);

        sprintf(qtSettings.font, "%s %s %s %d",
                family,
                weightStr(weight),
                italicStr(italic),
                (int)size);

        /* Qt uses a bold font for progressbars, try to mimic this... */
        if(weight>=WEIGHT_NORMAL && weight<WEIGHT_DEMIBOLD)
        {
            qtSettings.boldfont=(char *)malloc(
                strlen(family) + 1 +
                strlen(weightStr(WEIGHT_BOLD)) + 1 +
                strlen(italicStr(italic)) + 1 +
                20+  /* point size */ +1);

            sprintf(qtSettings.boldfont, "%s %s %s %d",
                    family,
                    weightStr(WEIGHT_BOLD),
                    italicStr(italic),
                    (int)size);
        }
#ifdef QTC_DEBUG
printf("REQUEST FONT: %s\n", qtSettings.font);
#endif
    }

    if(rd&RD_ICONS && !qtSettings.icons)
    {
        qtSettings.icons=(char *)malloc(strlen(defaultIcons())+1);
        strcpy(qtSettings.icons, defaultIcons());
    }
    if(rd&RD_TOOLBAR_STYLE && !(found&RD_TOOLBAR_STYLE))
        qtSettings.toolbarStyle=GTK_TOOLBAR_ICONS;
    if(rd&RD_BUTTON_ICONS && !(found&RD_BUTTON_ICONS))
        qtSettings.buttonIcons=TRUE;
    if(rd&RD_LIST_SHADE && !(found&RD_LIST_SHADE))
        qtSettings.shadeSortedList=TRUE;
    return found;
}

static int qt_refs=0;

#define MAX_APP_NAME_LEN 32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>

static const char * kdeGlobals()
{
    static char kg[QTC_MAX_FILENAME_LEN+1]={'\0'};

    char *kdehome=getKdeHome();

    if(kdehome && strlen(kdehome)<(QTC_MAX_FILENAME_LEN-strlen("/share/config/kdeglobals")))
        sprintf(kg, "%s/share/config/kdeglobals", kdehome);

    return kg;
}

#define HICOLOR_ICONS           "hicolor"
#define HICOLOR_LEN             7
#define ICON_FOLDER             "/share/icons/"
#define ICON_FOLDER_SLEN        13
#define DEFAULT_ICON_PREFIX     "/usr/share/icons"
#define DEFAULT_ICON_PREFIX_LEN 16

static const char * kdeIconsPrefix()
{
    return !qtSettings.qt4 && KDE3_ICONS_PREFIX && strlen(KDE3_ICONS_PREFIX)>2
            ? KDE3_ICONS_PREFIX
            : qtSettings.qt4 && KDE4_ICONS_PREFIX && strlen(KDE4_ICONS_PREFIX)>2
                ? KDE4_ICONS_PREFIX
                : DEFAULT_ICON_PREFIX;
}

static char * getIconPath()
{

    static char *path=NULL;
    char        *kdeHome=getKdeHome();
    const char  *kdePrefix=kdeIconsPrefix(),
                *defIcons=defaultIcons();
    gboolean    nonDefIcons=qtSettings.icons && strcmp(qtSettings.icons, defIcons);
    int         len=strlen("pixmap_path \""),
                kdeHomeLen=kdeHome ? strlen(kdeHome) : 0,
                kdeIconPrefixLen=strlen(kdePrefix),
                iconLen=qtSettings.icons ? strlen(qtSettings.icons) : 0,
                defIconsLen=strlen(defIcons);
    gboolean    addDefaultPrefix=strcmp(kdePrefix, DEFAULT_ICON_PREFIX);

    if(nonDefIcons)
    {
        if(kdeHome)
        {
            len+=kdeHomeLen;
            len+=ICON_FOLDER_SLEN;
            len+=iconLen;
            len++;
        }
        if(kdeIconPrefixLen)
        {
            len+=kdeIconPrefixLen;
            len++;
            len+=iconLen;
            len++;
        }
        if(addDefaultPrefix)
        {
            len+=DEFAULT_ICON_PREFIX_LEN;
            len++;
            len+=iconLen;
            len++;
        }
    }

    if(kdeHome)
    {
        len+=kdeHomeLen;
        len+=ICON_FOLDER_SLEN;
        len+=defIconsLen;
        len++;
    }
    if(kdeIconPrefixLen)
    {
        len+=kdeIconPrefixLen;
        len++;
        len+=defIconsLen;
        len++;
    }
    if(addDefaultPrefix)
    {
        len+=DEFAULT_ICON_PREFIX_LEN;
        len++;
        len+=defIconsLen;
        len++;
    }
    if(kdeHome)
    {
        len+=kdeHomeLen;
        len+=ICON_FOLDER_SLEN;
        len+=HICOLOR_LEN;
        len++;
    }
    if(kdeIconPrefixLen)
    {
        len+=kdeIconPrefixLen;
        len++;
        len+=HICOLOR_LEN;
        len++;
    }
    if(addDefaultPrefix)
    {
        len+=DEFAULT_ICON_PREFIX_LEN;
        len++;
        len+=HICOLOR_LEN;
        len++;
    }
    len++;

    if(path && len!=(strlen(path)+1))
        free(path);

    if(!path)
        path=(char *)malloc(len+1);

    strcpy(path, "pixmap_path \"");

    if(nonDefIcons)
    {
        if(kdeHome)
        {
            strcat(path, kdeHome);
            strcat(path, ICON_FOLDER);
            strcat(path, qtSettings.icons);
            strcat(path, ":");
        }
        if(kdeIconPrefixLen)
        {
            strcat(path, kdePrefix);
            strcat(path, "/");
            strcat(path, qtSettings.icons);
            strcat(path, ":");
        }
        if(addDefaultPrefix)
        {
            strcat(path, DEFAULT_ICON_PREFIX);
            strcat(path, "/");
            strcat(path, qtSettings.icons);
            strcat(path, ":");
        }
    }

    if(kdeHome)
    {
        strcat(path, kdeHome);
        strcat(path, ICON_FOLDER);
        strcat(path, defIcons);
        strcat(path, ":");
    }

    if(kdeIconPrefixLen)
    {
        strcat(path, kdePrefix);
        strcat(path, "/");
        strcat(path, defIcons);
        strcat(path, ":");
    }

    if(addDefaultPrefix)
    {
        strcat(path, DEFAULT_ICON_PREFIX);
        strcat(path, "/");
        strcat(path, defIcons);
        strcat(path, ":");
    }

    if(kdeHome)
    {
        strcat(path, kdeHome);
        strcat(path, ICON_FOLDER);
        strcat(path, HICOLOR_ICONS);
        strcat(path, ":");
    }

    if(kdeIconPrefixLen)
    {
        strcat(path, kdePrefix);
        strcat(path, "/");
        strcat(path, HICOLOR_ICONS);
        strcat(path, ":");
    }

    if(addDefaultPrefix)
    {
        strcat(path, DEFAULT_ICON_PREFIX);
        strcat(path, "/");
        strcat(path, HICOLOR_ICONS);
    }

    strcat(path, "\"");

    return path;
}

#define GIMP_PLUGIN "gimpplugin"

static char * getAppNameFromPid(int pid)
{
/* CPD: There must be an easier way than this? */
    static char app_name[MAX_APP_NAME_LEN+1]="";

    int  procFile=-1;
    char cmdline[MAX_LINE_LEN+1];

    sprintf(cmdline, "/proc/%d/cmdline", pid);

    if(-1!=(procFile=open(cmdline, O_RDONLY)))
    {
        if(read(procFile, cmdline, MAX_LINE_LEN)>2)
        {
            int      len=strlen(cmdline),
                     pos=0;
            gboolean found_slash=FALSE;

            for(pos=len-1; pos>=0 && cmdline[pos] && !found_slash; --pos)
                if('/'==cmdline[pos])
                {
                    pos++;
                    found_slash=TRUE;
                }

            if(!found_slash)
                pos=0;  /* Perhaps no / */
            if(pos>=0)
            {
                if(NULL!=strstr(cmdline, "gimp/2.0/plug-ins"))
                    strcpy(app_name, GIMP_PLUGIN);
                else
                {
                    strncpy(app_name, &cmdline[pos ? pos+1 : 0], MAX_APP_NAME_LEN);
                    app_name[MAX_APP_NAME_LEN]='\0';
                }
            }
        }
        close(procFile);
    }

    return app_name;
}

static char * getAppName()
{
    char *name=getAppNameFromPid(getpid());

    if(0==strcmp(name, "perl") || 0==strcmp(name, "python"))
    {
        name=getAppNameFromPid(getppid());
        if(!name)
            return "scriptedapp";
        else if(name==strstr(name, "gimp"))
            return GIMP_PLUGIN;
    }

    return name;
}

#ifdef QTC_MODIFY_MOZILLA

#define MAX_CSS_HOME     256
#define CSS_DEFAULT      ".default"
#define CSS_DEFAULT_ALT  "default."
#define USER_CHROME_DIR  "/chrome"
#define USER_CHROME_FILE "userChrome.css"
#define USER_CHROME_CSS  USER_CHROME_DIR"/"USER_CHROME_FILE
#define MAX_DEFAULT_NAME 16+strlen(CSS_DEFAULT)+strlen(USER_CHROME_CSS)

#define QTC_GUARD_STR "Added by QtCurve -- do not remove"
#define MENU_TEXT_STR "menubar > menu { color: HighlightText !important; } menubar > menu[_moz-menuactive=\"true\"] "\
                      "{ background-color : HighlightText !important; color: HighlightText !important; } "\
                      "/* "QTC_GUARD_STR" */\n"

#define CSS_FILE_STR  "@import url(\"file://"QTC_MOZILLA_DIR"/QtCurve.css\"); /* "QTC_GUARD_STR" */\n"

static void processUserChromeCss(char *file, gboolean add_css, gboolean add_menu_colors)
{
    FILE        *f=fopen(file, "r");
    char        *contents=NULL;
    gboolean    remove_css=FALSE,
                remove_menu_colors=FALSE;
    struct stat st;
    size_t      file_size=0,
                new_size=0;

    if(f)
    {
        if(0==fstat(fileno(f), &st))
        {
            file_size = st.st_size;
            new_size=file_size+strlen(MENU_TEXT_STR)+strlen(CSS_FILE_STR)+3;
            contents=(char *)malloc(new_size);

            if(contents)
            {
                char  *line=NULL;
                size_t len=0;

                contents[0]='\0';
                while(-1!=getline(&line, &len, f))
                {
                    gboolean write_line=TRUE;

                    if(0==strcmp(line, CSS_FILE_STR))
                    {
                        if (add_css)
                            add_css=FALSE;
                        else
                        {
                            remove_css=TRUE;
                            write_line=FALSE;
                        }
                    }
                    else
                    {
                        if(0==strcmp(line, MENU_TEXT_STR))
                        {
                            if (add_menu_colors)
                                add_menu_colors=FALSE;
                            else
                            {
                                remove_menu_colors=TRUE;
                                write_line=FALSE;
                            }
                        }
                    }
                    if(write_line)
                        strcat(contents, line);
                }
                if (line)
                     free(line);
            }
        }

        fclose(f);
    }

    if(!contents || add_css || add_menu_colors)
    {
        if(!contents)
        {
            new_size=strlen(MENU_TEXT_STR)+strlen(CSS_FILE_STR)+3;

            contents=(char *)malloc(new_size);
            if(contents)
                contents[0]='\0';
        }

        if(contents)
        {
            if(add_css)  /* CSS needs to be on 1st line */
            {
                char *css_contents=(char *)malloc(new_size);

                if(css_contents)
                {
                    css_contents[0]='\0';
                    strcat(css_contents, CSS_FILE_STR);
                    strcat(css_contents, contents);
                    free(contents);
                    contents=css_contents;
                }
            }
            if(add_menu_colors)  /* This can be on last line */
            {
                int len=strlen(contents);

                if(len && contents[len-1]!='\n')
                    strcat(contents, "\n");
                strcat(contents, MENU_TEXT_STR);
            }
        }
    }

    if(contents && (add_css || remove_css || add_menu_colors || remove_menu_colors))
    {
        f=fopen(file, "w");

        if(f)
        {
            fputs(contents, f);
            fclose(f);
        }
        free(contents);
    }
}

static void processMozillaApp(gboolean add_css, gboolean add_menu_colors, char *app, gboolean under_moz)
{
    const char *home=getHome();

    if(home && (strlen(home)+strlen(app)+10+MAX_DEFAULT_NAME)<MAX_CSS_HOME)     /* 10 for .mozilla/<app>/ */
    {
        char cssHome[MAX_CSS_HOME+1];
        DIR  *dir=NULL;

        sprintf(cssHome, under_moz ? "%s/.mozilla/%s/" : "%s/.%s/", home, app);

        if((dir=opendir(cssHome)))
        {
             struct dirent *dir_ent=NULL;

             for (dir_ent=readdir(dir); dir_ent; dir_ent=readdir(dir))
             {
                 char *str=NULL;

                 if(((str=strstr(dir_ent->d_name, CSS_DEFAULT)) && str>=dir_ent->d_name &&
                    '\0'==str[strlen(CSS_DEFAULT)]) ||
                    ((str=strstr(dir_ent->d_name, CSS_DEFAULT_ALT)) && str==dir_ent->d_name &&
                    '\0'!=str[strlen(CSS_DEFAULT_ALT)]))
                 {
                     char        sub[MAX_CSS_HOME];
                     struct stat statbuf;

#ifdef QTC_MODIFY_MOZILLA_USER_JS
                     /* Add custom user.js file */
                     sprintf(sub, "%s%s/user.js", cssHome, dir_ent->d_name);
                     if(-1==lstat(sub, &statbuf))
                     {
                         FILE *in=NULL;

                         sprintf(sub, QTC_MOZILLA_DIR"/%s-user.js", app);

                         if((in=fopen(sub, "r")))
                         {
                             FILE *out=fopen(sub, "w");

                             if(out)
                             {
                                 char ch;

                                 while((ch=fgetc(in))!=EOF)
                                     fputc(ch, out);
                                 fclose(out);
                             }
                             fclose(in);
                         }
                     }
#endif

                     /* Now do userChrome.css */
                     sprintf(sub, "%s%s%s/", cssHome, dir_ent->d_name, USER_CHROME_DIR);

                     if(-1!=lstat(sub, &statbuf) && S_ISDIR(statbuf.st_mode))
                     {
                         strcat(sub, USER_CHROME_FILE);
                         processUserChromeCss(sub, add_css, add_menu_colors);
                     }
                 }
             }

             closedir(dir);
        }
    }
}
#endif

#ifdef QTC_ADD_EVENT_FILTER
/* ... Taken from Qt engine ... */

#include <gdk/gdkx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
static Atom kipcCommAtom;
static Atom desktopWindowAtom;
static Atom knwmCommAtom;

static void qtExit();
static GdkFilterReturn qtGdkEventFilter(GdkXEvent *xevent, GdkEvent *gevent, gpointer data)
{
    XEvent *event =(XEvent*) xevent;

    if ( (ClientMessage==event->type && kipcCommAtom==event->xclient.message_type) ||
         (PropertyNotify==event->type && knwmCommAtom==event->xclient.message_type) )
    {
        qt_refs=1;
        qtExit();
        gtk_rc_reset_styles(gtk_settings_get_default());

        return GDK_FILTER_REMOVE;
    }

    return GDK_FILTER_CONTINUE;
}

static void qtcAddEventFilter() /* GdkWindow *widget) */
{
    static int done=0;

    if(!done)
    {
        /* Create a new window, and set the KDE_DESKTOP_WINDOW property on it
           This window will then receive events from KDE when the style changes */

        GtkWidget *top=gtk_window_new(GTK_WINDOW_TOPLEVEL);

        if(top && GTK_IS_WIDGET(top))
        {
            long data = 1;

            gtk_window_iconify(GTK_WINDOW(top));
            gtk_window_set_decorated(GTK_WINDOW(top), FALSE);
            gtk_window_set_keep_below(GTK_WINDOW(top), TRUE);
            gtk_window_set_opacity(GTK_WINDOW(top), 100);
            gtk_window_set_type_hint(GTK_WINDOW(top), GDK_WINDOW_TYPE_HINT_DOCK);
            gtk_widget_show(top);
            gtk_window_set_skip_taskbar_hint(GTK_WINDOW(top), TRUE);
            gtk_window_set_skip_pager_hint(GTK_WINDOW(top), TRUE);

            /* Get KDE related atoms from the X server */
            kipcCommAtom = XInternAtom(gdk_x11_get_default_xdisplay(), "KIPC_COMM_ATOM", FALSE);
            knwmCommAtom = XInternAtom(gdk_x11_get_default_xdisplay(), "_KDE_NET_WM_FRAME_STRUT", FALSE);
            desktopWindowAtom = XInternAtom(gdk_x11_get_default_xdisplay(), "KDE_DESKTOP_WINDOW", FALSE);

            XChangeProperty(gdk_x11_get_default_xdisplay(), GDK_WINDOW_XID(GTK_WIDGET(top)->window),
                            desktopWindowAtom, desktopWindowAtom, 32, PropModeReplace,
                            (unsigned char *)&data, 1);
            /* This filter will intercept those events */
            gdk_window_add_filter(GTK_WIDGET(top)->window, qtGdkEventFilter, 0);
            gtk_widget_hide(top);
            done=1;
        }
    }
}

/* ... */

#endif

static void getGtk2CfgFile(char **tmpStr, const char *xdg, const char *f)
{
    *tmpStr=(char *)realloc(*tmpStr, strlen(xdg)+1+strlen(f)+1);
    sprintf(*tmpStr, "%s/%s", xdg, f);
}

static gboolean checkFileVersion(const char *fname, const char *versionStr, int versionStrLen)
{
    FILE     *f=fopen(fname, "r");
    gboolean diff=TRUE;

    if(f)
    {
        if(0!=access(fname, W_OK)) /* If file is not writeable, then just pretend no difference */
            diff=FALSE;
        else
        {
            static const int constVLen=32;

            char line[constVLen+1];
            int  numChars=QTC_MIN(constVLen, versionStrLen-1);

            diff=NULL==fgets(line, numChars+1, f) || memcmp(versionStr, line, numChars);
        }
        fclose(f);
    }

    return !diff;
}

static gboolean isMozApp(const char *app, const char *check)
{
    if(0==strcmp(app, check))
        return TRUE;
    else if(app==strstr(app, check))
    {
        int app_len=strlen(app),
            check_len=strlen(check);

        if(check_len+4 == app_len && 0==strcmp(&app[check_len], "-bin"))
            return TRUE;

        /* OK check for xulrunner-1.9 */
        {
        double dummy;
        if(app_len>(check_len+1) && 1==sscanf(&app[check_len+1], "%f", &dummy))
            return TRUE;
        }
    }

    return FALSE;
}

static gboolean qtInit(Options *opts)
{
    if(0==qt_refs++)
    {
        static int lastRead=0;

        int now=time(NULL);

        qtSettings.app=GTK_APP_UNKNOWN;

        if(abs(now-lastRead)>1)
        {
            char        *app=NULL,
                        *path=NULL,
                        *tmpStr=NULL,
                        *rcFile=NULL;
            GtkSettings *settings=NULL;
            const char  *xdg=xdgConfigFolder();

            qtSettings.icons=NULL;
            qtSettings.boldfont=NULL;
            qtSettings.font=NULL;
            qtSettings.iconSizes.smlTbSize=16;
            qtSettings.iconSizes.tbSize=22;
            qtSettings.iconSizes.dndSize=32;
            qtSettings.iconSizes.btnSize=16;
            qtSettings.iconSizes.mnuSize=16;
            qtSettings.iconSizes.dlgSize=32;
            qtSettings.colors[PAL_ACTIVE][COLOR_LV].red=qtSettings.colors[PAL_ACTIVE][COLOR_LV].green=qtSettings.colors[PAL_ACTIVE][COLOR_LV].blue=0;
            qtSettings.styleName=NULL;

            lastRead=now;

            defaultSettings(opts);

            if(useQt3Settings())
            {
                qtSettings.qt4=FALSE;
                readRc("/etc/qt/qtrc", RD_ACT_PALETTE|(opts->inactiveHighlight ? 0 : RD_INACT_PALETTE)|RD_FONT|RD_CONTRAST|RD_STYLE,
                       opts, TRUE, FALSE, FALSE);
                readRc("/etc/qt3/qtrc", RD_ACT_PALETTE|(opts->inactiveHighlight ? 0 : RD_INACT_PALETTE)|RD_FONT|RD_CONTRAST|RD_STYLE,
                       opts, TRUE, FALSE, FALSE);
                readRc(".qt/qtrc", RD_ACT_PALETTE|(opts->inactiveHighlight ? 0 : RD_INACT_PALETTE)|RD_FONT|RD_CONTRAST|RD_STYLE,
                       opts, FALSE, TRUE, FALSE);
            }
            else
            {
                #define QT4_CFG_FILE "Trolltech.conf"

                char *confFile=(char *)malloc(strlen(xdg)+strlen(QT4_CFG_FILE)+2);

                readRc("/etc/xdg/"QT4_CFG_FILE, RD_ACT_PALETTE|(opts->inactiveHighlight ? 0 : RD_INACT_PALETTE)|RD_FONT|RD_CONTRAST|RD_STYLE,
                       opts, TRUE, FALSE, TRUE);
                sprintf(confFile, "%s/"QT4_CFG_FILE, xdg);
                readRc(confFile, RD_ACT_PALETTE|(opts->inactiveHighlight ? 0 : RD_INACT_PALETTE)|RD_FONT|RD_CONTRAST|RD_STYLE,
                       opts, TRUE, TRUE, TRUE);
                free(confFile);
                qtSettings.qt4=TRUE;
            }

            /* Only for testing - allows me to simulate Qt's -style parameter. e.g start Gtk2 app as follows:

                QTC_STYLE=qtc_klearlooks gtk-demo
            */
            {
                const char *env=getenv("QTC_STYLE");

                if(env)
                {
                    qtSettings.styleName=realloc(qtSettings.styleName, strlen(env));
                    strcpy(qtSettings.styleName, env);
                }
            }

            /* Is the user using a non-default QtCurve style? */
            if(qtSettings.styleName && qtSettings.styleName==strstr(qtSettings.styleName, QTC_THEME_PREFIX))
            {
#ifdef QTC_DEBUG
                printf("Look for themerc file for %s\n", qtSettings.styleName);
#endif
                rcFile=themeFile(getKdeHome(), qtSettings.styleName, &tmpStr);

                if(!rcFile)
                {
                    rcFile=themeFile(KDE_PREFIX(qtSettings.qt4 ? 4 : 3), qtSettings.styleName, &tmpStr);
                    if(!rcFile)
                        rcFile=themeFile(KDE_PREFIX(qtSettings.qt4 ? 3 : 4), qtSettings.styleName, &tmpStr);
                }
            }

            readConfig(rcFile, opts, opts);

            if(opts->inactiveHighlight)
                generateMidColor(&(qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW]),
                                &(qtSettings.colors[PAL_ACTIVE][COLOR_SELECTED]),
                                &qtSettings.inactiveSelectCol, INACTIVE_HIGHLIGHT_FACTOR);
            else
                qtSettings.inactiveSelectCol=qtSettings.colors[PAL_INACTIVE][COLOR_SELECTED];

            /* Check if we're firefox... */
            if((app=getAppName()))
            {
                gboolean firefox=isMozApp(app, "firefox") || isMozApp(app, "iceweasel") ||
                                 isMozApp(app, "swiftfox") || isMozApp(app, "xulrunner"),
                         thunderbird=!firefox && isMozApp(app, "thunderbird"),
                         mozThunderbird=!thunderbird && !firefox && isMozApp(app, "mozilla-thunderbird");

                if(firefox || thunderbird || mozThunderbird)
                {
#ifdef QTC_MODIFY_MOZILLA
                    GdkColor *menu_col=SHADE_CUSTOM==opts->shadeMenubars
                                        ? &opts->customMenubarsColor
                                        : &qtSettings.colors[PAL_ACTIVE][COLOR_SELECTED];
                    gboolean add_menu_colors=FALSE;

                    if(SHADE_BLEND_SELECTED==opts->shadeMenubars || (SHADE_CUSTOM==opts->shadeMenubars &&
                                                               TOO_DARK(*menu_col) ))
                        add_menu_colors=TRUE;

                    if(firefox)
                        processMozillaApp(!opts->gtkButtonOrder, add_menu_colors, "firefox", TRUE);
                    else if(thunderbird)
                        processMozillaApp(!opts->gtkButtonOrder, add_menu_colors, "thunderbird", FALSE);
                    else if(mozThunderbird)
                        processMozillaApp(!opts->gtkButtonOrder, add_menu_colors, "mozilla-thunderbird", FALSE);
#endif
                    qtSettings.app= (firefox && opts->newFirefox) ||
                                    ((thunderbird || mozThunderbird) && opts->newThunderbird) ||
                                    NULL!=getenv("QTC_NEW_MOZILLA")
                                        ? GTK_APP_NEW_MOZILLA
                                        : GTK_APP_MOZILLA;
                }
                else if(0==strcmp(app, "soffice.bin"))
                    qtSettings.app=GTK_APP_OPEN_OFFICE;
                else if(0==strcmp(app, "vmplayer"))
                    qtSettings.app=GTK_APP_VMPLAYER;
                else if(0==strcmp(app, GIMP_PLUGIN))
                    qtSettings.app=GTK_APP_GIMP_PLUGIN;
                else if(app==strstr(app, "gimp"))
                    qtSettings.app=GTK_APP_GIMP;
                else if(0==strcmp(app, "java"))
                    qtSettings.app=GTK_APP_JAVA;
                else if(0==strcmp(app, "evolution"))
                    qtSettings.app=GTK_APP_EVOLUTION;
                /*else if(app==strstr(app, "gaim"))
                    qtSettings.app=GTK_APP_GAIM;*/
            }

            /* Eclipse sets a application name, so if this is set then we're not a Swing java app */
            if(GTK_APP_JAVA==qtSettings.app && g_get_application_name() && 0!=strcmp(g_get_application_name(), "<unknown>"))
                qtSettings.app=GTK_APP_JAVA_SWT;

            /*if(isMozilla() || GTK_APP_JAVA==qtSettings.app)*/
            if(GTK_APP_JAVA!=qtSettings.app)
            {
                /* KDE's "apply colors to non-KDE apps" messes up firefox, (and progress bar text) so need to fix this! */
                /* ...and inactive highlight!!! */
                static const int constFileVersion=2;
                static const int constVersionLen=1+2+(6*3)+1+1;

                FILE     *f=NULL;
                char     version[constVersionLen];
                GdkColor inactiveHighlightTextCol=opts->inactiveHighlight
                                            ? qtSettings.colors[PAL_ACTIVE][COLOR_TEXT]
                                            : qtSettings.colors[PAL_INACTIVE][COLOR_TEXT_SELECTED];

                sprintf(version, "#%02d%02X%02X%02X%02X%02X%02X%02X%02X%02X%01X",
                                    constFileVersion,
                                    toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].red),
                                    toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].green),
                                    toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].blue),

                                    toQtColor(qtSettings.inactiveSelectCol.red),
                                    toQtColor(qtSettings.inactiveSelectCol.green),
                                    toQtColor(qtSettings.inactiveSelectCol.blue),

                                    toQtColor(inactiveHighlightTextCol.red),
                                    toQtColor(inactiveHighlightTextCol.green),
                                    toQtColor(inactiveHighlightTextCol.blue),

                                    opts->inactiveHighlight);

                getGtk2CfgFile(&tmpStr, xdg, "qtcurve.gtk-colors");

                if(!checkFileVersion(tmpStr, version, constVersionLen) && (f=fopen(tmpStr, "w")))
                {
                    fprintf(f, "%s\n"
                                "# Fix for KDE's \"apply colors to non-KDE"
                                " apps\" setting\n"
                                "style \"QtCTxtFix\" "
                                "{fg[ACTIVE]=\"#%02X%02X%02X\""
                                " fg[PRELIGHT]=\"#%02X%02X%02X\"}"
                                "class \"*MenuItem\" style \"QtCTxtFix\" "
                                "widget_class \"*.*ProgressBar\" style \"QtCTxtFix\"",
                                version,
                                toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].red),
                                toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].green),
                                toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].blue),

                                toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].red),
                                toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].green),
                                toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].blue));

                    if(opts->inactiveHighlight)
                        fprintf(f, "style \"QtCHlFix\" "
                                    "{base[ACTIVE]=\"#%02X%02X%02X\""
                                    " text[ACTIVE]=\"#%02X%02X%02X\"}"
                                    "class \"*\" style \"QtCHlFix\"",

                                    toQtColor(qtSettings.inactiveSelectCol.red),
                                    toQtColor(qtSettings.inactiveSelectCol.green),
                                    toQtColor(qtSettings.inactiveSelectCol.blue),

                                    toQtColor(inactiveHighlightTextCol.red),
                                    toQtColor(inactiveHighlightTextCol.green),
                                    toQtColor(inactiveHighlightTextCol.blue));
                    fclose(f);
                }

                /* Now get gtk to read this file *after* its other gtkrc files - this
                    allows us to undo the KDE settings! */
                gtk_rc_add_default_file(tmpStr);
            }
            if(GTK_APP_VMPLAYER==qtSettings.app)
                opts->shadeMenubars=SHADE_NONE;

            readRc(kdeGlobals(),
                   (opts->mapKdeIcons ? RD_ICONS|RD_SMALL_ICON_SIZE : 0)|RD_TOOLBAR_STYLE|RD_TOOLBAR_ICON_SIZE|RD_BUTTON_ICONS|RD_LIST_COLOR|RD_LIST_SHADE,
                    opts, TRUE, FALSE, FALSE);

            /* Tear off menu items dont seem to draw they're background, and the default background
               is drawn :-(  Fix/hack this by making that background the correct color */
            if(opts->lighterPopupMenuBgnd)
            {
                static const char *format="style \"QtCLMnu\" "
                                          "{bg[NORMAL]=\"#%02X%02X%02X\"} "
                                          "class \"GtkMenu\" style \"QtCLMnu\"";
                tmpStr=(char *)realloc(tmpStr, strlen(format)+32);

                if(tmpStr)
                {
                    GdkColor col;

                    shade(&qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW], &col, POPUPMENU_LIGHT_FACTOR);
                    sprintf(tmpStr, format, toQtColor(col.red), toQtColor(col.green), toQtColor(col.blue));
                    gtk_rc_parse_string(tmpStr);
                }
            }

            if(opts->mapKdeIcons && (path=getIconPath()))
            {
                int  versionLen=1+strlen(VERSION)+1+2+(6*2)+1;  /* '#' VERSION ' '<kde version> <..nums above..>\0 */
                char *version=(char *)malloc(versionLen);

                getGtk2CfgFile(&tmpStr, xdg, "qtcurve.gtk-icons");
                sprintf(version, "#%s %02X%02X%02X%02X%02X%02X%02X%",
                                 VERSION, 
                                 qtSettings.qt4 ? 4 : 3,
                                 qtSettings.iconSizes.smlTbSize,
                                 qtSettings.iconSizes.tbSize,
                                 qtSettings.iconSizes.dndSize,
                                 qtSettings.iconSizes.btnSize,
                                 qtSettings.iconSizes.mnuSize,
                                 qtSettings.iconSizes.dlgSize);

                if(!checkFileVersion(tmpStr, version, versionLen))
                {
                    static const char *constCmdStrFmt="perl "GTK_THEME_DIR"/map_kde_icons.pl "GTK_THEME_DIR"/icons%d %s %d %d %d %d %d %d %d %s > %s";

                    const char *kdeprefix=kdeIconsPrefix();
                    char       *cmdStr=(char *)malloc(strlen(constCmdStrFmt)+strlen(VERSION)
                                                      +2+(4*6)+2+
                                                      (kdeprefix ? strlen(kdeprefix) : DEFAULT_ICON_PREFIX_LEN)+strlen(tmpStr)+1);

                    sprintf(cmdStr, constCmdStrFmt,
                                    qtSettings.qt4 ? 4 : 3,
                                    kdeprefix ? kdeprefix : DEFAULT_ICON_PREFIX,
                                    qtSettings.qt4 ? 4 : 3,
                                    qtSettings.iconSizes.smlTbSize,
                                    qtSettings.iconSizes.tbSize,
                                    qtSettings.iconSizes.dndSize,
                                    qtSettings.iconSizes.btnSize,
                                    qtSettings.iconSizes.mnuSize,
                                    qtSettings.iconSizes.dlgSize,
                                    VERSION,
                                    tmpStr);

                    system(cmdStr);
                    free(cmdStr);
                }
                free(version);
                gtk_rc_add_default_file(tmpStr);
                gtk_rc_parse_string(path);
            }

            if((settings=gtk_settings_get_default()))
            {
                GtkSettingsValue svalue;

                if(qtSettings.font)
                    g_object_set(settings, "gtk-font-name", qtSettings.font, 0);

                svalue.origin="KDE-Settings";
                svalue.value.g_type=G_TYPE_INVALID;
                g_value_init(&svalue.value, G_TYPE_LONG);
                g_value_set_long(&svalue.value, qtSettings.toolbarStyle);
                gtk_settings_set_property_value(settings, "gtk-toolbar-style", &svalue);
#ifdef QTC_DEBUG
                printf("gtk-toolbar-style %d\n", qtSettings.toolbarStyle);
#endif
                if(NULL==gtk_check_version(2, 4, 0)) /* The following settings only apply for GTK>=2.4.0 */
                {
                    g_value_set_long(&svalue.value, qtSettings.buttonIcons);
#ifdef QTC_DEBUG
                    printf("gtk-button-images %d\n", qtSettings.buttonIcons);
#endif
                    gtk_settings_set_property_value(settings, "gtk-button-images", &svalue);

#if 0
                    if(opts->drawStatusBarFrames)
                        gtk_rc_parse_string("style \"QtCStBar\""
                                            "{ GtkStatusbar::shadow-type = 1 }" /*GtkStatusbar::has-resize-grip = FALSE }" */
                                            "class \"GtkStatusbar\" style"
                                            " \"QtCStBar\"");
                    else
                        gtk_rc_parse_string("style \"QtCSBar\""
                                            "{ GtkStatusbar::shadow-type = 0 }" /*GtkStatusbar::has-resize-grip = FALSE }" */
                                            "class \"GtkStatusbar\" style"
                                            " \"QtCSBar\"");
#endif
                }

                /* The following settings only apply for GTK>=2.6.0 */
                if(!opts->gtkButtonOrder && NULL==gtk_check_version(2, 6, 0))
                    g_object_set(settings, "gtk-alternative-button-order", TRUE, NULL);
            }

            if(SLIDER_TRIANGULAR==opts->sliderStyle)
                gtk_rc_parse_string("style \"QtCSldr\" {GtkScale::slider_length = 11 GtkScale::slider_width = 18} class \"*\" style \"QtCSldr\"");

            if(qtSettings.boldfont)
            {
                static const char *constBoldPrefix="style \"QtCBFnt\"{font_name=\"";
                static const char *constBoldSuffix="\"} class \"GtkProgress\" style \"QtCBFnt\" ";

                if(opts->framelessGroupBoxes)
                {
                    static const char *constStdPrefix="style \"QtCFnt\"{font_name=\"";
                    static const char *constStdSuffix="\"} ";
                    static const char *constGrpBoxBoldSuffix="widget_class \"*Frame.GtkLabel\" style \"QtCBFnt\" "
                                                             "widget_class \"*Statusbar.*Frame.GtkLabel\" style \"QtCFnt\"";
                    tmpStr=(char *)realloc(tmpStr, strlen(constStdPrefix)+strlen(qtSettings.font)+strlen(constStdSuffix)+
                                                   strlen(constBoldPrefix)+strlen(qtSettings.boldfont)+strlen(constBoldSuffix)+
                                                   strlen(constGrpBoxBoldSuffix)+1);

                    sprintf(tmpStr, "%s%s%s%s%s%s%s",
                                    constStdPrefix, qtSettings.font, constStdSuffix,
                                    constBoldPrefix, qtSettings.boldfont, constBoldSuffix,
                                    constGrpBoxBoldSuffix);
                }
                else
                {
                    tmpStr=(char *)realloc(tmpStr, strlen(constBoldPrefix)+strlen(qtSettings.boldfont)+strlen(constBoldSuffix)+1);
                    sprintf(tmpStr, "%s%s%s", constBoldPrefix, qtSettings.boldfont, constBoldSuffix);
                }

                gtk_rc_parse_string(tmpStr);
            }

            if(opts->thinnerMenuItems)
                gtk_rc_parse_string("style \"QtCMi\" {xthickness = 1 ythickness = 2 } "
                                    "class \"*MenuItem\" style \"QtCMi\"");

            /* Set password character... */
/*
            {
                static const char *constPasswdStrFormat="style \"QtCPasswd\" { GtkEntry::invisible-char='$' } class \"*\" style \"QtCPasswd\"";

                tmpStr=(char *)realloc(tmpStr, strlen(constPasswdStrFormat)+16);
                sprintf(tmpStr, constPasswdStrFormat, opts->passwordChar);
                gtk_rc_parse_string(tmpStr);
            }
*/
            /* For some reason Firefox 3beta4 goes mad if GtkComboBoxEntry::appears-as-list = 1 !!!! */
            if(isMozilla())
                gtk_rc_parse_string("style \"QtcMz\" { GtkComboBoxEntry::appears-as-list = 0 } class \"*\" style \"QtcMz\"");

            if(GTK_APP_MOZILLA==qtSettings.app || GTK_APP_JAVA==qtSettings.app)
                opts->scrollbarType=SCROLLBAR_WINDOWS;
            else
            {
                static const char *constSbStrFormat="style \"QtCSBt\" "
                                                    "{ GtkScrollbar::has-backward-stepper=%d "
                                                      "GtkScrollbar::has-forward-stepper=%d "
                                                      "GtkScrollbar::has-secondary-backward-stepper=%d "
                                                      "GtkScrollbar::has-secondary-forward-stepper=%d } "
                                                    "class \"*\" style \"QtCSBt\"";
                tmpStr=(char *)realloc(tmpStr, strlen(constSbStrFormat)+1);

                if(GTK_APP_OPEN_OFFICE==qtSettings.app)
                {
                    if(SCROLLBAR_NEXT==opts->scrollbarType)
                        opts->scrollbarType=SCROLLBAR_KDE;
                    else if(SCROLLBAR_NONE==opts->scrollbarType)
                        opts->scrollbarType=SCROLLBAR_WINDOWS;
                }

                switch(opts->scrollbarType)
                {
                    default:
                    case SCROLLBAR_KDE:
                        sprintf(tmpStr, constSbStrFormat, 1, 1, 1, 0);
                        break;
                    case SCROLLBAR_WINDOWS:
                        sprintf(tmpStr, constSbStrFormat, 1, 1, 0, 0);
                        break;
                    case SCROLLBAR_PLATINUM:
                        sprintf(tmpStr, constSbStrFormat, 0, 1, 1, 0);
                        break;
                    case SCROLLBAR_NEXT:
                        sprintf(tmpStr, constSbStrFormat, 1, 0, 0, 1);
                        break;
                    case SCROLLBAR_NONE:
                        sprintf(tmpStr, constSbStrFormat, 0, 0, 0, 0);
                        break;
                }

                gtk_rc_parse_string(tmpStr);
            }

            /* Set cursor colours... */
            { /* C-Scope */
                static const char *constStrFormat="style \"QtCCrsr\" "
                                                    "{ GtkWidget::cursor-color=\"#%02X%02X%02X\" "
                                                      "GtkWidget::secondary-cursor-color=\"#%02X%02X%02X\" } "
                                                    "class \"*\" style \"QtCCrsr\"";
                tmpStr=(char *)realloc(tmpStr, strlen(constStrFormat)+1);

                sprintf(tmpStr, constStrFormat, qtSettings.colors[PAL_ACTIVE][COLOR_TEXT].red>>8,
                                                qtSettings.colors[PAL_ACTIVE][COLOR_TEXT].green>>8,
                                                qtSettings.colors[PAL_ACTIVE][COLOR_TEXT].blue>>8,
                                                qtSettings.colors[PAL_ACTIVE][COLOR_TEXT].red>>8,
                                                qtSettings.colors[PAL_ACTIVE][COLOR_TEXT].green>>8,
                                                qtSettings.colors[PAL_ACTIVE][COLOR_TEXT].blue>>8);
                gtk_rc_parse_string(tmpStr);
            } /* C-Scope */

            if(!opts->gtkScrollViews && NULL!=gtk_check_version(2, 12, 0))
                opts->gtkScrollViews=true;

            { /* C-Scope */
            bool doEffect=ROUND_FULL==opts->round && EFFECT_NONE!=opts->buttonEffect;
            int  thickness=2;

            if(doEffect)
                gtk_rc_parse_string("style \"QtcEtch\" "
                                    "{ xthickness = 3 ythickness = 3 } "
                                    "style \"QtcEtchI\" "
                                    "{ GtkCheckButton::indicator_size = 15 } "
                                    "class \"*GtkRange\" style \"QtcEtch\" "
                                    "class \"*GtkSpinButton\" style \"QtcEtch\" "
                                    "class \"*GtkEntry\" style  \"QtcEtch\" "
                                    "widget_class \"*Toolbar*Entry\" style \"QtcEtch\" "
                                    "class \"*Button\" style \"QtcEtch\""
                                    "class \"*GtkOptionMenu\" style \"QtcEtch\""
                                    /*"class \"*GtkWidget\" style \"QtcEtchI\""*/);

            if(!opts->gtkScrollViews)
                gtk_rc_parse_string("style \"QtcSV\""
                                    " { GtkScrolledWindow::scrollbar-spacing = 0 "
                                      " GtkScrolledWindow::scrollbars-within-bevel = 1 } "
                                    "class \"*GtkWidget\" style \"QtcSV\"");

            /* Scrolled windows */
            if(opts->squareScrollViews)
                thickness=opts->gtkScrollViews ? 1 : 2;
            else if(opts->sunkenScrollViews)
                thickness=3;

            { /* C-Scope */
                static const char *constStrFormat="style \"QtcSVt\" "
                                                    "{ xthickness = %d ythickness = %d } "
                                                      "class \"*GtkScrolledWindow\" style \"QtcSVt\"";

                tmpStr=(char *)realloc(tmpStr, strlen(constStrFormat)+1);
                sprintf(tmpStr, constStrFormat, thickness, thickness);
                gtk_rc_parse_string(tmpStr);
            } /* C-Scope */

            { /* C-Scope */
                static const char *constStrFormat="style \"QtcPbar\" "
                                                    "{ xthickness = %d ythickness = %d } "
                                                      "widget_class \"*GtkProgressBar\" style \"QtcPbar\"";
                int pthickness=opts->fillProgress
                                ? doEffect
                                    ? 2
                                    : 1
                                : doEffect
                                    ? 3
                                    : 2;

                tmpStr=(char *)realloc(tmpStr, strlen(constStrFormat)+1);
                sprintf(tmpStr, constStrFormat, pthickness, pthickness);
                gtk_rc_parse_string(tmpStr);
            } /* C-Scope */
            } /* C-Scope 'doEffect' */

            if(opts->lighterPopupMenuBgnd && !opts->borderMenuitems)
                gtk_rc_parse_string("style \"QtCM\" { xthickness=1 ythickness=1 }\n"
                                    "class \"*GtkMenu\" style \"QtCM\"");

            if(tmpStr)
                free(tmpStr);
        }
        return TRUE;
    }
    return FALSE;
}

static void qtExit()
{
    qt_refs--;

    if(0==qt_refs)
    {
        if(qtSettings.font)
            free(qtSettings.font);
        qtSettings.font=NULL;
        if(qtSettings.icons)
            free(qtSettings.icons);
        qtSettings.icons=NULL;
        if(qtSettings.boldfont)
            free(qtSettings.boldfont);
        qtSettings.boldfont=NULL;
    }
}

#define SET_COLOR_PAL(st, rc, itm, ITEM, state, QTP_COL, PAL) \
    st->itm[state]=rc->color_flags[state]&ITEM ? rc->itm[state] : qtSettings.colors[PAL][QTP_COL];

#define SET_COLOR(st, rc, itm, ITEM, state, QTP_COL) \
    SET_COLOR_PAL(st, rc, itm, ITEM, state, QTP_COL, PAL_ACTIVE)

static void qtSetColors(GtkStyle *style, GtkRcStyle *rc_style, Options *opts)
{
    SET_COLOR(style, rc_style, bg, GTK_RC_BG, GTK_STATE_NORMAL, COLOR_WINDOW)
    SET_COLOR(style, rc_style, bg, GTK_RC_BG, GTK_STATE_SELECTED, COLOR_SELECTED)
    SET_COLOR(style, rc_style, bg, GTK_RC_BG, GTK_STATE_INSENSITIVE, COLOR_WINDOW)
    SET_COLOR(style, rc_style, bg, GTK_RC_BG, GTK_STATE_ACTIVE, COLOR_MID)
    SET_COLOR(style, rc_style, bg, GTK_RC_BG, GTK_STATE_PRELIGHT, COLOR_WINDOW)

    SET_COLOR(style, rc_style, base, GTK_RC_BASE, GTK_STATE_NORMAL, COLOR_BACKGROUND)
    SET_COLOR(style, rc_style, base, GTK_RC_BASE, GTK_STATE_SELECTED, COLOR_SELECTED)
    SET_COLOR(style, rc_style, base, GTK_RC_BASE, GTK_STATE_INSENSITIVE, COLOR_WINDOW)
    /*SET_COLOR(style, rc_style, base, GTK_RC_BASE, GTK_STATE_ACTIVE, COLOR_SELECTED)*/
    style->base[GTK_STATE_ACTIVE]=qtSettings.inactiveSelectCol;
    SET_COLOR(style, rc_style, base, GTK_RC_BASE, GTK_STATE_PRELIGHT, COLOR_SELECTED)

    SET_COLOR(style, rc_style, text, GTK_RC_TEXT, GTK_STATE_NORMAL, COLOR_TEXT)
    SET_COLOR(style, rc_style, text, GTK_RC_TEXT, GTK_STATE_SELECTED, COLOR_TEXT_SELECTED)
    SET_COLOR(style, rc_style, text, GTK_RC_TEXT, GTK_STATE_INSENSITIVE, COLOR_MID)
    /*SET_COLOR(style, rc_style, text, GTK_RC_TEXT, GTK_STATE_ACTIVE, COLOR_TEXT_SELECTED)*/

    if(opts->inactiveHighlight)
        SET_COLOR(style, rc_style, text, GTK_RC_TEXT, GTK_STATE_ACTIVE, COLOR_TEXT)
    else
        SET_COLOR_PAL(style, rc_style, text, GTK_RC_TEXT, GTK_STATE_ACTIVE, COLOR_TEXT_SELECTED, PAL_INACTIVE)

    SET_COLOR(style, rc_style, text, GTK_RC_TEXT, GTK_STATE_PRELIGHT, COLOR_TEXT)

    SET_COLOR(style, rc_style, fg, GTK_RC_FG, GTK_STATE_NORMAL, COLOR_FOREGROUND)
    SET_COLOR(style, rc_style, fg, GTK_RC_FG, GTK_STATE_SELECTED, COLOR_TEXT_SELECTED)
    SET_COLOR(style, rc_style, fg, GTK_RC_FG, GTK_STATE_INSENSITIVE, COLOR_MID)
    SET_COLOR(style, rc_style, fg, GTK_RC_FG, GTK_STATE_ACTIVE, COLOR_FOREGROUND)
    SET_COLOR(style, rc_style, fg, GTK_RC_FG, GTK_STATE_PRELIGHT, COLOR_FOREGROUND)
}

static void qtSetFont(GtkRcStyle *rc_style)
{
    if(qtSettings.font)
    {
        if (rc_style->font_desc)
            pango_font_description_free (rc_style->font_desc);

        rc_style->font_desc = pango_font_description_from_string (qtSettings.font);
    }
}
