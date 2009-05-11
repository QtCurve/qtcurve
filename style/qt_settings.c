/*
  QtCurve (C) Craig Drummond, 2003 - 2009 craig_p_drummond@yahoo.co.uk

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
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>

#define QTC_READ_INACTIVE_PAL /* Control whether QtCurve should read the inactive palette as well.. */
#define QTC_RC_SETTING "QtC__"

#define toQtColor(col) \
    ((col&0xFF00)>>8)
/*    ((int)((((double)col)/256.0)+0.5))*/

#define toGtkColor(col) \
    ((col<<8)+col)

static GdkColor setGdkColor(int r, int g, int b)
{
    GdkColor col;

    col.red=toGtkColor(r);
    col.green=toGtkColor(g);
    col.blue=toGtkColor(b);
    return col;
}

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
    /*GTK_APP_JAVA_SWT,*/
    GTK_APP_EVOLUTION
    /*GTK_APP_GAIM*/
} EGtkApp;

enum QtPallete
{
    PAL_ACTIVE,
    PAL_DISABLED,
    PAL_INACTIVE
#ifndef QTC_READ_INACTIVE_PAL
        = PAL_ACTIVE
#endif
    ,

    PAL_NUMPALS
};

enum QtFont
{
    FONT_GENERAL,
    FONT_MENU,
    FONT_TOOLBAR,

    FONT_NUM_STD,

    FONT_BOLD = FONT_NUM_STD,

    FONT_NUM_TOTAL
};

struct QtData
{
    GdkColor        colors[PAL_NUMPALS][COLOR_NUMCOLORS],
                    inactiveSelectCol;
    char            *fonts[FONT_NUM_TOTAL],
                    *icons,
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

#define DEFAULT_KDE_DIS_PAL \
"disabled=#000000^e#dddfe4^e#ffffff^e#ffffff^e#555555^e#c7c7c7^e#c7c7c7^e#ffffff^e#000000^e#ffffff^e#efefef^e#000000^e#678db2^e#ffffff^e#0000ee^e#52188b^e"

#ifdef QTC_READ_INACTIVE_PAL
#define DEFAULT_KDE_INACT_PAL \
"inactive=#000000^e#dddfe4^e#ffffff^e#ffffff^e#555555^e#c7c7c7^e#000000^e#ffffff^e#000000^e#ffffff^e#efefef^e#000000^e#678db2^e#ffffff^e#0000ee^e#52188b^e"
#endif

#define DEFAULT_KDE_FONT      "Sans Serif"
#define DEFAULT_KDE_FONT_SIZE 10.0
#define MAX_LINE_LEN 1024

struct QtData qtSettings;

static bool haveAlternareListViewCol()
{
    return 0!=qtSettings.colors[PAL_ACTIVE][COLOR_LV].red || 0!=qtSettings.colors[PAL_ACTIVE][COLOR_LV].green ||
           0!=qtSettings.colors[PAL_ACTIVE][COLOR_LV].blue;
}

static gboolean isMozilla()
{
    return GTK_APP_MOZILLA==qtSettings.app || GTK_APP_NEW_MOZILLA==qtSettings.app;
}

static gboolean useQt3Settings()
{
    static int ver=0;

    if(0==ver)
    {
        const char *sessionVersion=getenv("KDE_SESSION_VERSION");

        ver=sessionVersion
                ? atoi(sessionVersion)<4
                    ? 3
                    : 4
#ifdef QTC_DEFAULT_TO_KDE3
                : 3;
#else
                : getenv("KDE_FULL_SESSION")
                    ? 3
                    : 4;
#endif
    }

    return 3==ver;
}

static const char * defaultIcons()
{
    return qtSettings.qt4 ? "oxygen" : "crystalsvg";
}

enum
{
    SECT_NONE                 =0x000001,
    SECT_PALETTE              =0x000002,
    SECT_GENERAL              =0x000004,
    SECT_KDE                  =0x000008,
    SECT_ICONS                =0x000010,
    SECT_TOOLBAR_STYLE        =0x000020,
    SECT_MAIN_TOOLBAR_ICONS   =0x000040,
    SECT_SMALL_ICONS          =0x000080,

    SECT_KDE4_COL_BUTTON      =0x000100,
    SECT_KDE4_COL_SEL         =0x000200,
    SECT_KDE4_COL_TOOLTIP     =0x000400,
    SECT_KDE4_COL_VIEW        =0x000800,
    SECT_KDE4_COL_WINDOW      =0x001000,

    SECT_KDE4_EFFECT_DISABLED =0x002000,
    SECT_KDE4_EFFECT_INACTIVE =0x004000,

    SECT_QT
};

#define ALL_KDE4_PAL_SETTINGS (SECT_KDE4_COL_BUTTON|SECT_KDE4_COL_SEL|SECT_KDE4_COL_TOOLTIP|SECT_KDE4_COL_VIEW| \
                               SECT_KDE4_COL_WINDOW|SECT_KDE4_EFFECT_DISABLED|SECT_KDE4_EFFECT_INACTIVE)
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
    RD_DIS_PALETTE       = 0x0002,
    RD_INACT_PALETTE     = 0x0004,
    RD_FONT              = 0x0008,
    RD_CONTRAST          = 0x0010,
    RD_ICONS             = 0x0020,
    RD_TOOLBAR_STYLE     = 0x0040,
    RD_TOOLBAR_ICON_SIZE = 0x0080,
    RD_BUTTON_ICONS      = 0x0100,
    RD_SMALL_ICON_SIZE   = 0x0200,
    RD_LIST_COLOR        = 0x0400,
    RD_STYLE             = 0x0800,
    RD_LIST_SHADE        = 0x1000,
    RD_KDE4_PAL          = 0x2000,

    RD_MENU_FONT         = 0x4000,
    RD_TB_FONT           = 0x8000
};

/*
 Try to determine if a mozilla app is >= v3.  Tested with:

Mozilla Firefox 3.0.8, Copyright (c) 1998 - 2009 mozilla.org
 Thunderbird 2.0.0.21, Copyright (c) 1998-2009 mozilla.org
 Thunderbird 3.0b2, Copyright (c) 1998-2009 mozilla.org
Mozilla XULRunner 1.9.0.8 - 2009032711

To get the verison number:
1. Get the command-line from /proc/<pid>/cmdline - so Linux specific
2. Append --version to this, and call the application
3. Look for the fist '.' in the returned string
4. Look at the character to the left of that, if it is a digit and >2  then
   we assume this is a new mozilla app...

...what a pain...
*/

static int getMozillaVersion(int pid)
{
    char cmdline[MAX_LINE_LEN+11];
    int  ver=0,
         procFile=-1;

    sprintf(cmdline, "/proc/%d/cmdline", pid);

    if(-1!=(procFile=open(cmdline, O_RDONLY)))
    {
        if(read(procFile, cmdline, MAX_LINE_LEN)>2)
        {
            char *version=0L;
            strcat(cmdline, " --version");
            if(g_spawn_command_line_sync(cmdline, &version, NULL, NULL, NULL))
            {
                char *dot=strchr(version, '.');

                if(dot && dot!=version && isdigit(dot[-1]))
                    ver=dot[-1]-'0';
            }
        }
        close(procFile);
    }

    return ver;
}

static char * getKdeHome()
{
    static char *kdeHome=NULL;

    if(!kdeHome)
        if(g_spawn_command_line_sync(qtSettings.qt4 ? "kde4-config --expandvars --localprefix"
                                                    : "kde-config --expandvars --localprefix", &kdeHome, NULL, NULL, NULL))
        {
            int len=strlen(kdeHome);

            if(len>1 && kdeHome[len-1]=='\n')
                kdeHome[len-1]='\0';
        }
        else
            kdeHome=0L;

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
        if(strlen(l)>=7)
            switch(n)
            {
                case 0:
                    setRgb(&qtSettings.colors[p][COLOR_WINDOW_TEXT], l);
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
        else if(n>-1)
            break;

        n++;
        if(n>13)
            break;
        l=strtok(NULL, "#");
    }
}

typedef enum
{
    BackgroundAlternate,
    BackgroundNormal,
    DecorationFocus,
    DecorationHover,
    ForegroundNormal,

    UnknownColor
} ColorType;

typedef enum  // Taken from "kcolorscheme.cpp"
{
    // Effects
    Intensity = 0,
    Color = 1,
    Contrast = 2,
    // Intensity
    IntensityNoEffect = 0,
    IntensityShade = 1,
    IntensityDarken = 2,
    IntensityLighten = 3,
    // Color
    ColorNoEffect = 0,
    ColorDesaturate = 1,
    ColorFade = 2,
    ColorTint = 3,
    // Contrast
    ContrastNoEffect = 0,
    ContrastFade = 1,
    ContrastTint = 2
} ColAdjustEffects;
    
typedef struct
{
    double           amount;
    ColAdjustEffects effect;
} ColAdjust;

typedef struct
{
    GdkColor  col;
    ColAdjust intensity,
              color,
              contrast;
    gboolean  enabled;
} ColorEffect;

typedef enum
{
    EFF_DISABLED,
    EFF_INACTIVE
} Effect;

static ColorType getColorType(const char *line)
{
    if(0==strncmp_i(line, "BackgroundAlternate=", 20))
        return BackgroundAlternate;
    if(0==strncmp_i(line, "BackgroundNormal=", 17))
        return BackgroundNormal;
    if(0==strncmp_i(line, "ForegroundNormal=", 17))
        return ForegroundNormal;
    if(0==strncmp_i(line, "DecorationFocus=", 16))
        return DecorationFocus;
    if(0==strncmp_i(line, "DecorationHover=", 16))
        return DecorationHover;
    return UnknownColor;
}

static GdkColor readColor(const char *line)
{
    char     *eq=strchr(line, '=');
    GdkColor col;

    if(eq && ++eq && *eq!='\0' && 3==sscanf(eq, "%d,%d,%d", (int *)&col.red, (int *)&col.green, (int *)&col.blue))
    {
        col.red=toGtkColor(col.red);
        col.green=toGtkColor(col.green);
        col.blue=toGtkColor(col.blue);
    }
    else
        col.red=col.blue=col.green=0;
    return col;
}

static int readInt(const char *line, int offset)
{
    return line[offset]!='\0' ? atoi(&line[offset]) : 0;
}

static double readDouble(const char *line, int offset)
{
    return line[offset]!='\0' ? g_ascii_strtod(&line[offset], NULL) : 0;
}

static gboolean readBool(const char *line, int offset)
{
    return line[offset]!='\0' ? 0==strncmp_i(&line[offset], "true", 4) : false;
}

typedef struct
{
    int   weight,
          italic,
          fixedW;
    float size;
    char  family[QTC_MAX_INPUT_LINE_LEN+1];
} QtFontDetails;

static void initFont(QtFontDetails *f, gboolean setFamily)
{
    f->weight=WEIGHT_NORMAL,
    f->italic=0;
    f->fixedW=0;
    f->size=DEFAULT_KDE_FONT_SIZE;
    if(setFamily)
        strcpy(f->family, DEFAULT_KDE_FONT);
    else
        f->family[0]='\0';
}

static void parseFontLine(const char *line, QtFontDetails *font)
{
    int           n=-1;
    char          *l,
                  fontLine[QTC_MAX_INPUT_LINE_LEN+1];
    QtFontDetails rc;

    initFont(&rc, FALSE);
    memcpy(fontLine, line, QTC_MAX_INPUT_LINE_LEN+1);
    l=strtok(fontLine, "=");

    while(l)
    {
        switch(n)
        {
            case 0:  /* Family - and foundry(maybe!) (ignore X11 and XFT) */
            {
                char *dash=NULL;

                if(NULL!=(dash=strchr(l, '-')))
                {
                    *dash='\0';
                    l=++dash;
                }

                strcpy(rc.family, l);
                break;
            }
            case 1:  /* Point size */
                sscanf(l, "%f", &rc.size);
                break;
            case 4:  /* Weight */
                sscanf(l, "%d", &rc.weight);
                break;
            case 5:  /* Slant */
                sscanf(l, "%d", &rc.italic);
                break;
            case 8:  /* Spacing */
                sscanf(l, "%d", &rc.fixedW);
                break;
            default:
                break;
        }

        n++;
        if(n>8 && '\0'!=font->family[0])
        {
            font->weight=rc.weight;
            font->italic=rc.italic;
            font->fixedW=rc.fixedW;
            font->size=rc.size;
            strcpy(font->family, rc.family);
            break;
        }
        l=strtok(NULL, ",");
    }
}

static void setFont(QtFontDetails *font, int f)
{
    if(qtSettings.fonts[f])
    {
        free(qtSettings.fonts[f]);
        qtSettings.fonts[f]=NULL;
    }
    if(FONT_GENERAL==f && qtSettings.fonts[FONT_BOLD])
    {
        free(qtSettings.fonts[FONT_BOLD]);
        qtSettings.fonts[FONT_BOLD]=NULL;
    }

    qtSettings.fonts[f]=(char *)malloc(
        strlen(font->family) + 1 +
        strlen(weightStr(font->weight)) + 1 +
        strlen(italicStr(font->italic)) + 1 +
        20+  /* point size */ +1);

    sprintf(qtSettings.fonts[f], "%s %s %s %d",
            font->family,
            weightStr(font->weight),
            italicStr(font->italic),
            (int)font->size);

    /* Qt uses a bold font for progressbars, try to mimic this... */
    if(FONT_GENERAL==f && font->weight>=WEIGHT_NORMAL && font->weight<WEIGHT_DEMIBOLD)
    {
        qtSettings.fonts[FONT_BOLD]=(char *)malloc(
            strlen(font->family) + 1 +
            strlen(weightStr(WEIGHT_BOLD)) + 1 +
            strlen(italicStr(font->italic)) + 1 +
            20+  /* point size */ +1);

        sprintf(qtSettings.fonts[FONT_BOLD], "%s %s %s %d",
                font->family,
                weightStr(WEIGHT_BOLD),
                italicStr(font->italic),
                (int)font->size);
    }
#ifdef QTC_DEBUG
    printf("REQUEST FONT: %s\n", qtSettings.fonts[f]);
#endif
}

#define QTC_MIX(a, b, bias) (a + ((b - a) * bias))
GdkColor mixColors(const GdkColor *c1, const GdkColor *c2, double bias)
{
    if (bias <= 0.0 || isnan(bias)) return *c1;
    if (bias >= 1.0) return *c2;

    {
    double   r1=c1->red/65535.0,
             g1=c1->green/65535.0,
             b1=c1->blue/65535.0,
             r2=c2->red/65535.0,
             g2=c2->green/65535.0,
             b2=c2->blue/65535.0;
    GdkColor col;

    col.red=(int)(65535.0*QTC_MIX(r1, r2, bias));
    col.green=(int)(65535.0*QTC_MIX(g1, g2, bias));
    col.blue=(int)(65535.0*QTC_MIX(b1, b2, bias));

    return col;
    }
}

static void readKdeGlobals(const char *rc, int rd, Options *opts)
{
    ColorEffect   effects[2];
    int           found=0,
                  colorsFound=0,
                  i;
    char          line[QTC_MAX_INPUT_LINE_LEN+1];
    FILE          *f=fopen(rc, "r");
    QtFontDetails fonts[FONT_NUM_STD];

    for(i=0; i<FONT_NUM_STD; ++i)
        initFont(&fonts[i], TRUE);

    if(qtSettings.qt4)
    {
        // Set defaults!
        effects[EFF_DISABLED].col.red=112;
        effects[EFF_DISABLED].col.green=111;
        effects[EFF_DISABLED].col.blue=110;
        effects[EFF_DISABLED].color.amount=0.0;
        effects[EFF_DISABLED].color.effect=ColorNoEffect;
        effects[EFF_DISABLED].contrast.amount=0.65;
        effects[EFF_DISABLED].contrast.effect=ContrastFade;
        effects[EFF_DISABLED].intensity.amount=0.1;
        effects[EFF_DISABLED].intensity.effect=IntensityDarken;
        effects[EFF_DISABLED].enabled=true;
        effects[EFF_INACTIVE].col.red=112;
        effects[EFF_INACTIVE].col.green=111;
        effects[EFF_INACTIVE].col.blue=110;
        effects[EFF_INACTIVE].color.amount=0.0;
        effects[EFF_INACTIVE].color.effect=ColorNoEffect;
        effects[EFF_INACTIVE].contrast.amount=0.0;
        effects[EFF_INACTIVE].contrast.effect=ContrastNoEffect;
        effects[EFF_INACTIVE].intensity.amount=0.0;
        effects[EFF_INACTIVE].intensity.effect=IntensityNoEffect;
        effects[EFF_INACTIVE].enabled=false;

        qtSettings.colors[PAL_ACTIVE][COLOR_BUTTON]=setGdkColor(232, 231, 230);
        qtSettings.colors[PAL_ACTIVE][COLOR_BUTTON_TEXT]=setGdkColor(20, 19, 18);
        qtSettings.colors[PAL_ACTIVE][COLOR_SELECTED]=setGdkColor(65, 139, 212);
        qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED]=setGdkColor(255, 255, 255);
        qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP]=setGdkColor(192, 218, 255);
        qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP_TEXT]=setGdkColor(20, 19, 18);
        qtSettings.colors[PAL_ACTIVE][COLOR_BACKGROUND]=setGdkColor(255, 255, 255);
        qtSettings.colors[PAL_ACTIVE][COLOR_TEXT]=setGdkColor(20, 19, 18);
        qtSettings.colors[PAL_ACTIVE][COLOR_LV]=setGdkColor(248, 247, 246);
        qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW]=setGdkColor(233, 232, 232);
        qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW_TEXT]=setGdkColor(20, 19, 18);

        qtSettings.colors[PAL_ACTIVE][COLOR_FOCUS]=
        qtSettings.colors[PAL_ACTIVE][COLOR_HOVER]=
            qtSettings.colors[PAL_ACTIVE][COLOR_SELECTED];
    }

    if(f)
    {
        int section=SECT_NONE;

        while(found!=rd && NULL!=fgets(line, QTC_MAX_INPUT_LINE_LEN, f))
            if(line[0]=='[')
            {
                if(0==strncmp_i(line, "[Icons]", 7))
                    section=SECT_ICONS;
                else if(0==strncmp_i(line, "[Toolbar style]", 15))
                    section=SECT_TOOLBAR_STYLE;
                else if(0==strncmp_i(line, "[MainToolbarIcons]", 18))
                    section=SECT_MAIN_TOOLBAR_ICONS;
                else if(0==strncmp_i(line, "[SmallIcons]", 12))
                    section=SECT_SMALL_ICONS;
                else if(qtSettings.qt4 && 0==strncmp_i(line, "[Colors:View]", 13))
                    section=SECT_KDE4_COL_VIEW;
                else if(qtSettings.qt4 && 0==strncmp_i(line, "[Colors:Button]", 15))
                    section=SECT_KDE4_COL_BUTTON;
                else if(qtSettings.qt4 && 0==strncmp_i(line, "[Colors:Selection]", 18))
                    section=SECT_KDE4_COL_SEL;
                else if(qtSettings.qt4 && 0==strncmp_i(line, "[Colors:Tooltip]", 16))
                    section=SECT_KDE4_COL_TOOLTIP;
                else if(qtSettings.qt4 && 0==strncmp_i(line, "[Colors:Window]", 15))
                    section=SECT_KDE4_COL_WINDOW;
                else if(qtSettings.qt4 && 0==strncmp_i(line, "[ColorEffects:Disabled]", 23))
                    section=SECT_KDE4_EFFECT_DISABLED;
                else if(qtSettings.qt4 && 0==strncmp_i(line, "[ColorEffects:Inactive]", 23))
                    section=SECT_KDE4_EFFECT_INACTIVE;
                else if(/*qtSettings.qt4 && */0==strncmp_i(line, "[General]", 9))
                    section=SECT_GENERAL;
                else if(qtSettings.qt4 && 0==strncmp_i(line, "[KDE]", 5))
                    section=SECT_KDE;
                else
                {
                    section=SECT_NONE;
                    if(colorsFound==ALL_KDE4_PAL_SETTINGS)
                        found|=RD_KDE4_PAL;
                    if(found==rd)
                        break;
                }
            }
            else if (SECT_ICONS==section && rd&RD_ICONS && !(found&RD_ICONS) &&
                     0==strncmp_i(line, "Theme=", 6))
            {
                char *eq=strstr(line, "=");

                if(eq && ++eq)
                {
                    unsigned int len=strlen(eq);

                    qtSettings.icons=(char *)realloc(qtSettings.icons, len+1);
                    strcpy(qtSettings.icons, eq);
                    if('\n'==qtSettings.icons[len-1])
                        qtSettings.icons[len-1]='\0';
                }
                found|=RD_ICONS;
            }
            else if (SECT_SMALL_ICONS==section && rd&RD_SMALL_ICON_SIZE && !(found&RD_SMALL_ICON_SIZE) &&
                     0==strncmp_i(line, "Size=", 5))
            {
                int size=readInt(line, 5);

                if(0!=size)
                {
                    qtSettings.iconSizes.smlTbSize=size;
                    qtSettings.iconSizes.btnSize=size;
                    qtSettings.iconSizes.mnuSize=size;
                    found|=RD_SMALL_ICON_SIZE;
                }
            }
            else if (SECT_TOOLBAR_STYLE==section && rd&RD_TOOLBAR_STYLE &&
                        !(found&RD_TOOLBAR_STYLE) &&
                        ( (!qtSettings.qt4 && 0==strncmp_i(line, "IconText=", 9)) ||
                        (qtSettings.qt4 && 0==strncmp_i(line, "ToolButtonStyle=", 16))))
            {
                char *eq=strstr(line, "=");

                if(eq && ++eq)
                {
                    if(0==strncmp_i(eq, "IconOnly", 8))
                        qtSettings.toolbarStyle=GTK_TOOLBAR_ICONS;
                    else if(0==strncmp_i(eq, "TextOnly", 8))
                        qtSettings.toolbarStyle=GTK_TOOLBAR_TEXT;
                    else if( (!qtSettings.qt4 && 0==strncmp_i(eq, "IconTextRight", 13)) ||
                                (qtSettings.qt4 && 0==strncmp_i(eq, "TextBesideIcon", 14)) )
                        qtSettings.toolbarStyle=GTK_TOOLBAR_BOTH_HORIZ;
                    else if( (!qtSettings.qt4 && 0==strncmp_i(eq, "IconTextBottom", 14)) ||
                                (qtSettings.qt4 && 0==strncmp_i(eq, "TextUnderIcon", 13)))
                        qtSettings.toolbarStyle=GTK_TOOLBAR_BOTH;
                    found|=RD_TOOLBAR_STYLE;
                }
            }
            else if (SECT_MAIN_TOOLBAR_ICONS==section && rd&RD_TOOLBAR_ICON_SIZE &&
                        !(found&RD_TOOLBAR_ICON_SIZE) && 0==strncmp_i(line, "Size=", 5))
            {
                qtSettings.iconSizes.tbSize=readInt(line, 5);
                found|=RD_TOOLBAR_ICON_SIZE;
            }
            else if (SECT_KDE==section && rd&RD_BUTTON_ICONS && !(found&RD_BUTTON_ICONS) &&
                        0==strncmp_i(line, "ShowIconsOnPushButtons=", 23))
            {
                qtSettings.buttonIcons=readBool(line, 23);
                found|=RD_BUTTON_ICONS;
            }
            else if(rd&RD_LIST_COLOR && !(found&RD_LIST_COLOR) &&
                    !qtSettings.qt4 && SECT_GENERAL==section && 0==strncmp_i(line, "alternateBackground=", 20))
            {
                qtSettings.colors[PAL_ACTIVE][COLOR_LV]=readColor(line);
                found|=RD_LIST_COLOR;
            }
            else if(qtSettings.qt4 && section>=SECT_KDE4_COL_BUTTON && section<=SECT_KDE4_COL_WINDOW &&
                    rd&RD_KDE4_PAL && !(found&RD_KDE4_PAL))
            {
                ColorType colorType=getColorType(line);

                colorsFound|=section;
                if(UnknownColor!=colorType)
                {
                    GdkColor color=readColor(line);

                    switch(section)
                    {
                        case SECT_KDE4_COL_BUTTON:
                            switch(colorType)
                            {
                                case BackgroundNormal:
                                    qtSettings.colors[PAL_ACTIVE][COLOR_BUTTON]=color;
                                    break;
                                case ForegroundNormal:
                                    qtSettings.colors[PAL_ACTIVE][COLOR_BUTTON_TEXT]=color;
                                    break;
                                case DecorationFocus:
                                    qtSettings.colors[PAL_ACTIVE][COLOR_FOCUS]=color;
                                    break;
                                case DecorationHover:
                                    qtSettings.colors[PAL_ACTIVE][COLOR_HOVER]=color;
                                    break;
                            }
                            break;
                        case SECT_KDE4_COL_SEL:
                            if(BackgroundNormal==colorType)
                                qtSettings.colors[PAL_ACTIVE][COLOR_SELECTED]=color;
                            else if(ForegroundNormal==colorType)
                                qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED]=color;
                            break;
                        case SECT_KDE4_COL_TOOLTIP:
                            if(BackgroundNormal==colorType)
                                qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP]=color;
                            else if(ForegroundNormal==colorType)
                                qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP_TEXT]=color;
                            break;
                        case SECT_KDE4_COL_VIEW:
                            if(BackgroundNormal==colorType)
                                qtSettings.colors[PAL_ACTIVE][COLOR_BACKGROUND]=color;
                            else if(ForegroundNormal==colorType)
                                qtSettings.colors[PAL_ACTIVE][COLOR_TEXT]=color;
                            else if(BackgroundAlternate==colorType)
                                qtSettings.colors[PAL_ACTIVE][COLOR_LV]=color;
                            break;
                        case SECT_KDE4_COL_WINDOW:
                            if(BackgroundNormal==colorType)
                                qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW]=color;
                            else if(ForegroundNormal==colorType)
                                qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW_TEXT]=color;
                            break;
                        default:
                            break;
                    }
                }
            }
            else if (qtSettings.qt4 && SECT_GENERAL==section && rd&RD_FONT && !(found&RD_FONT) &&
                     0==strncmp_i(line, "font=", 5))
            {
                parseFontLine(line, &fonts[FONT_GENERAL]);
                found|=RD_FONT;
            }
            else if (SECT_GENERAL==section && rd&RD_MENU_FONT && !(found&RD_MENU_FONT) &&
                     0==strncmp_i(line, "menuFont=", 9))
            {
                parseFontLine(line, &fonts[FONT_MENU]);
                found|=RD_MENU_FONT;
            }
            else if (SECT_GENERAL==section && rd&RD_TB_FONT && !(found&RD_TB_FONT) &&
                     0==strncmp_i(line, "toolBarFont=", 12))
            {
                parseFontLine(line, &fonts[FONT_TOOLBAR]);
                found|=RD_TB_FONT;
            }
            else if(qtSettings.qt4 && rd&RD_CONTRAST && !(found&RD_CONTRAST) && SECT_KDE==section &&
                    0==strncmp_i(line, "contrast=", 9))
            {
                opts->contrast=readInt(line, 9);
                if(opts->contrast>10 || opts->contrast<0)
                    opts->contrast=7;
                found|=RD_CONTRAST;
            }
            else if(qtSettings.qt4 && SECT_GENERAL==section && rd&RD_STYLE && !(found&RD_STYLE) &&
                     0==strncmp_i(line, "widgetStyle=", 12))
            {
                int len=strlen(line);
                qtSettings.styleName=realloc(qtSettings.styleName, strlen(&line[12])+1);
                if('\n'==line[len-1])
                    line[len-1]='\0';
                strcpy(qtSettings.styleName, &line[12]);
                found|=RD_STYLE;
            }
            else if(qtSettings.qt4 && (SECT_KDE4_EFFECT_DISABLED==section || SECT_KDE4_EFFECT_INACTIVE==section) &&
                    rd&RD_KDE4_PAL && !(found&RD_KDE4_PAL))
            {
                colorsFound|=section;
                Effect eff=SECT_KDE4_EFFECT_DISABLED==section ? EFF_DISABLED : EFF_INACTIVE;
                if(0==strncmp_i(line, "Color=", 6))
                    effects[eff].col=readColor(line);
                else if(0==strncmp_i(line, "ColorAmount=", 12))
                    effects[eff].color.amount=readDouble(line, 12);
                else if(0==strncmp_i(line, "ColorEffect=", 12))
                    effects[eff].color.effect=readInt(line, 12);
                else if(0==strncmp_i(line, "ContrastAmount=", 15))
                    effects[eff].contrast.amount=readDouble(line, 15);
                else if(0==strncmp_i(line, "ContrastEffect=", 15))
                    effects[eff].contrast.effect=readInt(line, 15);
                else if(0==strncmp_i(line, "IntensityAmount=", 16))
                    effects[eff].intensity.amount=readDouble(line, 16);
                else if(0==strncmp_i(line, "IntensityEffect=", 16))
                    effects[eff].intensity.effect=readInt(line, 16);
                else if(0==strncmp_i(line, "Enable=", 7))
                    effects[eff].enabled=readBool(line, 7);
            }
            else if(SECT_GENERAL==section && rd&RD_LIST_SHADE && !(found&RD_LIST_SHADE) &&
                    0==strncmp_i(line, "shadeSortColumn=", 16))
            {
                qtSettings.shadeSortedList=readBool(line, 16);
                found|=RD_LIST_SHADE;
            }
            else if(found==rd)
                break;

        fclose(f);
    }

    if(qtSettings.qt4)
    {
        int    eff=0;
        double contrast=0.1*opts->contrast,
               y;

        contrast = (1.0 > contrast ? (-1.0 < contrast ? contrast : -1.0) : 1.0);
        y = ColorUtils_luma(&qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW]);

        if(y<0.006)
            qtSettings.colors[PAL_ACTIVE][COLOR_MID]=ColorUtils_shade(&qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW], 0.01 + 0.20 * contrast, 0.0);
        else if(y>0.93)
            qtSettings.colors[PAL_ACTIVE][COLOR_MID]=ColorUtils_shade(&qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW], -0.02 - 0.20 * contrast, 0.0);
        else
        {
            double darkAmount =  (     - y       ) * (0.55 + contrast * 0.35);

            qtSettings.colors[PAL_ACTIVE][COLOR_MID]=ColorUtils_shade(&qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW], (0.35 + 0.15 * y) * darkAmount, 0.0);
        }

        for(eff=0; eff<2; ++eff)
        {
            int p=0==eff ? PAL_DISABLED : PAL_INACTIVE;
            memcpy(qtSettings.colors[p], qtSettings.colors[PAL_ACTIVE], sizeof(GdkColor) * COLOR_NUMCOLORS);
            if(effects[eff].enabled)
            {
                int col;
                for(col=0; col<COLOR_NUMCOLORS; ++col)
                {
                    switch(effects[eff].intensity.effect)
                    {
                        case IntensityShade:
                            qtSettings.colors[p][col] = ColorUtils_shade(&qtSettings.colors[p][col], effects[eff].intensity.amount, 0.0);
                            break;
                        case IntensityDarken:
                            qtSettings.colors[p][col] = ColorUtils_darken(&qtSettings.colors[p][col], effects[eff].intensity.amount, 1.0);
                            break;
                        case IntensityLighten:
                            qtSettings.colors[p][col] = ColorUtils_lighten(&qtSettings.colors[p][col], effects[eff].intensity.amount, 1.0);
                        default:
                            break;
                    }
                    switch (effects[eff].color.effect)
                    {
                        case ColorDesaturate:
                            qtSettings.colors[p][col] = ColorUtils_darken(&qtSettings.colors[p][col], 0.0, 1.0 - effects[eff].color.amount);
                            break;
                        case ColorFade:
                            qtSettings.colors[p][col] = ColorUtils_mix(&qtSettings.colors[p][col], &effects[eff].col, effects[eff].color.amount);
                            break;
                        case ColorTint:
                            qtSettings.colors[p][col] = ColorUtils_tint(&qtSettings.colors[p][col], &effects[eff].col, effects[eff].color.amount);
                        default:
                            break;
                    }

                    if(COLOR_BUTTON_TEXT==col || COLOR_TEXT==col || COLOR_WINDOW_TEXT==col || COLOR_TOOLTIP_TEXT==col)
                    {
                        int other=COLOR_BUTTON_TEXT==col
                                    ? COLOR_BUTTON
                                    : COLOR_WINDOW_TEXT==col
                                        ? COLOR_WINDOW
                                        : COLOR_TEXT==col
                                            ? COLOR_BACKGROUND
                                            : COLOR_TOOLTIP;

                        switch(effects[eff].contrast.effect)
                        {
                            case ContrastFade:
                                qtSettings.colors[p][col]=ColorUtils_mix(&qtSettings.colors[p][col],
                                                                         &qtSettings.colors[PAL_DISABLED][other],
                                                                         effects[eff].contrast.amount);
                                break;
                            case ContrastTint:
                                qtSettings.colors[p][col]=ColorUtils_tint(&qtSettings.colors[p][col],
                                                                          &qtSettings.colors[PAL_DISABLED][other],
                                                                          effects[eff].contrast.amount);
                            default:
                                break;
                        }
                    }
                }
            }
        }
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
    if(rd&RD_FONT && (found&RD_FONT || (!qtSettings.fonts[FONT_GENERAL] && qtSettings.qt4)))  /* No need to check if read in */
        setFont(&fonts[FONT_GENERAL], FONT_GENERAL);
    if(rd&RD_MENU_FONT && found&RD_MENU_FONT)
        setFont(&fonts[FONT_MENU], FONT_MENU);
    if(rd&RD_TB_FONT && found&RD_TB_FONT)
        setFont(&fonts[FONT_TOOLBAR], FONT_TOOLBAR);
}

static void readQtRc(const char *rc, int rd, Options *opts, gboolean absolute, gboolean setDefaultFont)
{
    const char    *home=absolute ? NULL : getHome();
    int           found=0;
    char          line[QTC_MAX_INPUT_LINE_LEN+1];
    QtFontDetails font;

    initFont(&font, TRUE);

    if(absolute || NULL!=home)
    {
        char fname[256];
        FILE *f;

        if(!absolute)
            sprintf(fname, "%s/%s", home, rc);

        f=fopen(absolute ? rc : fname, "r");

        if(f)
        {
            int section=SECT_NONE;

            while(found!=rd && NULL!=fgets(line, QTC_MAX_INPUT_LINE_LEN, f))
                if(line[0]=='[')
                {
                    if(0==strncmp_i(line, "[Palette]", 9))
                        section=SECT_PALETTE;
                    else if(0==strncmp_i(line, "[General]", 9))
                        section=SECT_GENERAL;
                    else if(0==strncmp_i(line, "[KDE]", 5))
                        section=SECT_KDE;
                    else
                        section=SECT_NONE;
                }
                else if(rd&RD_CONTRAST && !(found&RD_CONTRAST) && SECT_KDE==section && 0==strncmp_i(line, "contrast=", 9))
                {
                    opts->contrast=readInt(line, 9);
                    if(opts->contrast>10 || opts->contrast<0)
                        opts->contrast=7;
                    found|=RD_CONTRAST;
                }
                else if(SECT_PALETTE==section && rd&RD_ACT_PALETTE && !(found&RD_ACT_PALETTE) &&
                        0==strncmp_i(line, "active=", 7))
                {
                    parseQtColors(line, PAL_ACTIVE);
                    found|=RD_ACT_PALETTE;
                }
                else if(SECT_PALETTE==section && rd&RD_DIS_PALETTE && !(found&RD_DIS_PALETTE) &&
                        0==strncmp_i(line, "disabled=", 7))
                {
                    parseQtColors(line, PAL_DISABLED);
                    found|=RD_DIS_PALETTE;
                }
#ifdef QTC_READ_INACTIVE_PAL
                else if(SECT_PALETTE==section && rd&RD_INACT_PALETTE && !(found&RD_INACT_PALETTE) &&
                        0==strncmp_i(line, "inactive=", 9))
                {
                    parseQtColors(line, PAL_INACTIVE);
                    found|=RD_INACT_PALETTE;
                }
#endif
                else if (SECT_GENERAL==section && rd&RD_STYLE && !(found&RD_STYLE) && 0==strncmp_i(line, "style=", 6))
                {
                    int len=strlen(line);
                    qtSettings.styleName=realloc(qtSettings.styleName, strlen(&line[6])+1);
                    if('\n'==line[len-1])
                        line[len-1]='\0';
                    strcpy(qtSettings.styleName, &line[6]);
                    found|=RD_STYLE;
                }
                else if (SECT_GENERAL==section && rd&RD_FONT && !(found&RD_FONT) && 0==strncmp_i(line, "font=", 5))
                {
                    parseFontLine(line, &font);
                    found|=RD_FONT;
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

    if(rd&RD_DIS_PALETTE && !(found&RD_DIS_PALETTE))
    {
        strncpy(line, DEFAULT_KDE_DIS_PAL, QTC_MAX_INPUT_LINE_LEN);
        line[QTC_MAX_INPUT_LINE_LEN]='\0';
        parseQtColors(line, PAL_DISABLED);
    }

    if(rd&RD_ACT_PALETTE)
    {
        qtSettings.colors[PAL_ACTIVE][COLOR_FOCUS]=
        qtSettings.colors[PAL_ACTIVE][COLOR_HOVER]=
            qtSettings.colors[PAL_ACTIVE][COLOR_SELECTED];
    }
/*
    if(rd&RD_DIS_PALETTE)
    {
        qtSettings.colors[PAL_DISABLED][COLOR_FOCUS]=
        qtSettings.colors[PAL_DISABLED][COLOR_HOVER]=
            qtSettings.colors[PAL_DISABLED][COLOR_SELECTED];
    }
*/
#ifdef QTC_READ_INACTIVE_PAL
    if(rd&RD_INACT_PALETTE && !(found&RD_INACT_PALETTE))
    {
        strncpy(line, DEFAULT_KDE_INACT_PAL, QTC_MAX_INPUT_LINE_LEN);
        line[QTC_MAX_INPUT_LINE_LEN]='\0';
        parseQtColors(line, PAL_INACTIVE);
    }

/*
    if(rd&RD_INACT_PALETTE)
    {
        qtSettings.colors[PAL_INACTIVE][COLOR_FOCUS]=
        qtSettings.colors[PAL_INACTIVE][COLOR_HOVER]=
            qtSettings.colors[PAL_INACTIVE][COLOR_SELECTED];
    }
*/
#endif

    if(rd&RD_FONT && (found&RD_FONT || (!qtSettings.fonts[FONT_GENERAL] && setDefaultFont)))  /* No need to check if read in */
        setFont(&font, FONT_GENERAL);
}

static int qt_refs=0;

#define MAX_APP_NAME_LEN 32

#define KDE_CFG_DIR         "/share/config/"
#define KDEGLOBALS_FILE     KDE_CFG_DIR"kdeglobals"
#define KDEGLOBALS_SYS_FILE KDE_CFG_DIR"system.kdeglobals"

static const char * kdeGlobals()
{
    static char kg[QTC_MAX_FILENAME_LEN+1]={'\0'};

    char *kdehome=getKdeHome();

    if(kdehome && strlen(kdehome)<(QTC_MAX_FILENAME_LEN-strlen(KDEGLOBALS_FILE)))
        sprintf(kg, "%s"KDEGLOBALS_FILE, kdehome);

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
    static char *kdeIcons=NULL;

    if(!kdeIcons)
        if(g_spawn_command_line_sync(qtSettings.qt4 ? "kde4-config --expandvars --install icon"
                                                    : "kde-config --expandvars --install icon", &kdeIcons, NULL, NULL, NULL))
        {
            int len=strlen(kdeIcons);

            if(len>1 && kdeIcons[len-1]=='\n')
                kdeIcons[len-1]='\0';
        }
        else
            kdeIcons=0L;

    if(!kdeIcons)
        kdeIcons=!qtSettings.qt4 && KDE3_ICONS_PREFIX && strlen(KDE3_ICONS_PREFIX)>2
                    ? KDE3_ICONS_PREFIX
                    : qtSettings.qt4 && KDE4_ICONS_PREFIX && strlen(KDE4_ICONS_PREFIX)>2
                        ? KDE4_ICONS_PREFIX
                        : DEFAULT_ICON_PREFIX;

    return kdeIcons;
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

    {
    int plen=strlen(path);

    if(path[plen - 1] == ':')
        path[plen - 1] = '\0';
    }

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
    static char *name=0L;

    if(!name)
    {
        name=getAppNameFromPid(getpid());

        if(0==strcmp(name, "perl") || 0==strcmp(name, "python"))
        {
            name=getAppNameFromPid(getppid());
            if(!name)
                name="scriptedapp";
            else if(name==strstr(name, "gimp"))
                name=GIMP_PLUGIN;
        }
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

#define CSS_FILE_STR     "@import url(\"file://"QTC_MOZILLA_DIR"/QtCurve.css\"); /* "QTC_GUARD_STR" */\n"
#define BTN_CSS_FILE_STR "@import url(\"file://"QTC_MOZILLA_DIR"/QtCurve-KDEButtonOrder.css\"); /* "QTC_GUARD_STR" */\n"

static void processUserChromeCss(char *file, gboolean add_btn_css, gboolean add_menu_colors)
{
    FILE        *f=fopen(file, "r");
    char        *contents=NULL;
    gboolean    remove_btn_css=FALSE,
                remove_menu_colors=FALSE,
                add_css=TRUE;
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

                    if(0==strcmp(line, BTN_CSS_FILE_STR))
                    {
                        if (add_btn_css)
                            add_btn_css=FALSE;
                        else
                        {
                            remove_btn_css=TRUE;
                            write_line=FALSE;
                        }
                    }
                    else if(0==strcmp(line, CSS_FILE_STR))
                        add_css=FALSE;
                    else if(0==strcmp(line, MENU_TEXT_STR))
                    {
                        if (add_menu_colors)
                            add_menu_colors=FALSE;
                        else
                        {
                            remove_menu_colors=TRUE;
                            write_line=FALSE;
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

    if(!contents || add_btn_css || add_menu_colors || add_css)
    {
        if(!contents)
        {
            new_size=strlen(MENU_TEXT_STR)+strlen(BTN_CSS_FILE_STR)+strlen(CSS_FILE_STR)+4;

            contents=(char *)malloc(new_size);
            if(contents)
                contents[0]='\0';
        }

        if(contents)
        {
            if(add_css)
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
            if(add_btn_css)
            {
                char *css_contents=(char *)malloc(new_size);

                if(css_contents)
                {
                    css_contents[0]='\0';
                    strcat(css_contents, BTN_CSS_FILE_STR);
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

    if(contents && (add_btn_css || remove_btn_css || add_menu_colors || remove_menu_colors))
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

static void processMozillaApp(gboolean add_btn_css, gboolean add_menu_colors, char *app, gboolean under_moz)
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
                    FILE        *userJs=NULL;
                    gboolean    alterUserJs=TRUE;

                    /* Add custom user.js file */
                    sprintf(sub, "%s%s/user.js", cssHome, dir_ent->d_name);

                    if((userJs=fopen(sub, "r")))
                    {
                        char  *line=NULL;
                        size_t len=0;
                        
                        while(-1!=getline(&line, &len, userJs))
                            if(NULL!=strstr(line, "browser.preferences.instantApply"))
                            {
                                /* If instant-apply is set to true, then we cannot alter button order,
                                   as this produces sde effects (such as the preferences dialog's close
                                   button having no text! */
                                if(NULL!=strstr(line, "true"))
                                    add_btn_css=FALSE;
                                alterUserJs=FALSE;
                                break;
                            }
                        fclose(userJs);
                        if (line)
                            free(line);
                    }

                    if(alterUserJs && ((userJs=fopen(sub, "a"))))
                    {
                        fputs("\nuser_pref(\"browser.preferences.instantApply\", false);\n", userJs);
                        fclose(userJs);
                    }

                    /* Now do userChrome.css */
                    sprintf(sub, "%s%s%s/", cssHome, dir_ent->d_name, USER_CHROME_DIR);

                    if(-1!=lstat(sub, &statbuf) && S_ISDIR(statbuf.st_mode))
                    {
                        strcat(sub, USER_CHROME_FILE);
                        processUserChromeCss(sub, add_btn_css, add_menu_colors);
                    }
                }
            }

            closedir(dir);
        }
    }
}
#endif

#ifdef QTC_ADD_EVENT_FILTER____DISABLED
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
            memset(qtSettings.fonts, 0, sizeof(char *)*FONT_NUM_TOTAL);
            qtSettings.iconSizes.smlTbSize=16;
            qtSettings.iconSizes.tbSize=22;
            qtSettings.iconSizes.dndSize=32;
            qtSettings.iconSizes.btnSize=16;
            qtSettings.iconSizes.mnuSize=16;
            qtSettings.iconSizes.dlgSize=32;
            qtSettings.colors[PAL_ACTIVE][COLOR_LV]=setGdkColor(0, 0, 0);
            qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP_TEXT]=setGdkColor(0, 0, 0);
            qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP]=setGdkColor(0xFF, 0xFF, 192);
            qtSettings.styleName=NULL;

            lastRead=now;

            qtSettings.qt4=!useQt3Settings();
            if(!qtSettings.qt4)
            {
                qtSettings.qt4=FALSE;
                readQtRc("/etc/qt/qtrc", RD_ACT_PALETTE|RD_DIS_PALETTE|RD_INACT_PALETTE|RD_FONT|RD_CONTRAST|RD_STYLE,
                         opts, TRUE, FALSE);
                readQtRc("/etc/qt3/qtrc", RD_ACT_PALETTE|RD_DIS_PALETTE|RD_INACT_PALETTE|RD_FONT|RD_CONTRAST|RD_STYLE,
                         opts, TRUE, FALSE);
                readQtRc(".qt/qtrc", RD_ACT_PALETTE|RD_DIS_PALETTE|RD_INACT_PALETTE|RD_FONT|RD_CONTRAST|RD_STYLE,
                         opts, FALSE, TRUE);
            }

            {
            int        f=0;
            const char *files[]={"/etc/kderc",
                                 qtSettings.qt4 ? "/etc/kde4/kdeglobals" : "/etc/kde3/kdeglobals",
                                 qtSettings.qt4 ? "/etc/kde4rc" : "/etc/kde3rc",
                                 qtSettings.qt4 ? KDE4PREFIX KDEGLOBALS_FILE : KDE3PREFIX KDEGLOBALS_FILE,
                                 qtSettings.qt4 ? KDE4PREFIX KDEGLOBALS_SYS_FILE : KDE3PREFIX KDEGLOBALS_SYS_FILE,
                                 kdeGlobals(),
                                 0L};

            for(f=0; 0!=files[f]; ++f)
                readKdeGlobals(files[f], RD_ICONS|RD_SMALL_ICON_SIZE|RD_TOOLBAR_STYLE|RD_MENU_FONT|RD_TB_FONT|
                                         RD_TOOLBAR_ICON_SIZE|RD_BUTTON_ICONS|RD_LIST_SHADE|
                                         (qtSettings.qt4 ? RD_KDE4_PAL|RD_FONT|RD_CONTRAST|RD_STYLE : RD_LIST_COLOR),
                               opts);
            }

            /* Only for testing - allows me to simulate Qt's -style parameter. e.g start Gtk2 app as follows:

                QTC_STYLE=qtc_klearlooks gtk-demo
            */
            {
                const char *env=getenv("QTC_STYLE");

                if(env && strlen(env))
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

            readConfig(rcFile, opts, 0L);

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
                                 isMozApp(app, "swiftfox") || isMozApp(app, "xulrunner") ||
                                 isMozApp(app, "abrowser"),
                         thunderbird=!firefox && isMozApp(app, "thunderbird"),
                         mozThunderbird=!thunderbird && !firefox && isMozApp(app, "mozilla-thunderbird"),
                         seamonkey=!thunderbird && !firefox && !mozThunderbird && isMozApp(app, "seamonkey");
                int      mozVersion=0;

                if(firefox || thunderbird || mozThunderbird || seamonkey)
                {
#ifdef QTC_MODIFY_MOZILLA
                    GdkColor *menu_col=SHADE_CUSTOM==opts->shadeMenubars
                                        ? &opts->customMenubarsColor
                                        : &qtSettings.colors[PAL_ACTIVE][COLOR_SELECTED];
                    gboolean add_menu_colors=SHADE_BLEND_SELECTED==opts->shadeMenubars ||
                                             (SHADE_CUSTOM==opts->shadeMenubars && TOO_DARK(*menu_col) );

                    if(firefox)
                        processMozillaApp(!opts->gtkButtonOrder, add_menu_colors, "firefox", TRUE);
                    else if(thunderbird)
                        processMozillaApp(!opts->gtkButtonOrder, add_menu_colors, "thunderbird", FALSE);
                    else if(mozThunderbird)
                        processMozillaApp(!opts->gtkButtonOrder, add_menu_colors, "mozilla-thunderbird", FALSE);
#endif
                    qtSettings.app=
#ifndef QTC_OLD_MOZILLA
                        firefox || NULL!=getenv("QTC_NEW_MOZILLA")
                                    ? GTK_APP_NEW_MOZILLA :
#endif
                                    GTK_APP_MOZILLA;

                    if(GTK_APP_MOZILLA==qtSettings.app)
                        mozVersion=getMozillaVersion(getpid());
                    if(GTK_APP_MOZILLA==qtSettings.app && mozVersion>2)
                        qtSettings.app=GTK_APP_NEW_MOZILLA;
                    if(GTK_APP_NEW_MOZILLA!=qtSettings.app && APPEARANCE_FADE==opts->menuitemAppearance &&
                       (thunderbird || mozThunderbird || (seamonkey && mozVersion<2)))
                        opts->menuitemAppearance=APPEARANCE_GRADIENT;
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
                /*else if(0==strcmp(app, "eclipse"))
                    qtSettings.app=GTK_APP_JAVA_SWT;*/
                /*else if(app==strstr(app, "gaim"))
                    qtSettings.app=GTK_APP_GAIM;*/
            }

            /* Eclipse sets a application name, so if this is set then we're not a Swing java app */
            /*if(GTK_APP_JAVA==qtSettings.app && g_get_application_name() && 0!=strcmp(g_get_application_name(), "<unknown>"))
                qtSettings.app=GTK_APP_JAVA_SWT;*/

            /*if(isMozilla() || GTK_APP_JAVA==qtSettings.app)*/
            if(GTK_APP_JAVA!=qtSettings.app)
            {
                /* KDE's "apply colors to non-KDE apps" messes up firefox, (and progress bar text) so need to fix this! */
                /* ...and inactive highlight!!! */
                static const char *format="style \""QTC_RC_SETTING"MTxt\""
                                          " {fg[ACTIVE]=\"#%02X%02X%02X\""
                                          " fg[PRELIGHT]=\"#%02X%02X%02X\"}"
                                          " style \""QTC_RC_SETTING"PTxt\""
                                          " {fg[ACTIVE]=\"#%02X%02X%02X\""
                                          " fg[PRELIGHT]=\"#%02X%02X%02X\"}"
                                          " class \"*MenuItem\" style \""QTC_RC_SETTING"MTxt\" "
                                          " widget_class \"*.*MenuItem*\" style \""QTC_RC_SETTING"MTxt\" "
                                          " widget_class \"*.*ProgressBar\" style \""QTC_RC_SETTING"PTxt\"";
                tmpStr=(char *)realloc(tmpStr, strlen(format));

                if(tmpStr)
                {
                    GdkColor *highlightedMenuCol=opts->useHighlightForMenu
                                            ? &qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED]
                                            : &qtSettings.colors[PAL_ACTIVE][COLOR_TEXT];

                    sprintf(tmpStr, format, toQtColor(highlightedMenuCol->red),
                                            toQtColor(highlightedMenuCol->green),
                                            toQtColor(highlightedMenuCol->blue),
                                            toQtColor(highlightedMenuCol->red),
                                            toQtColor(highlightedMenuCol->green),
                                            toQtColor(highlightedMenuCol->blue),
                                            toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].red),
                                            toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].green),
                                            toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].blue),
                                            toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].red),
                                            toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].green),
                                            toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT_SELECTED].blue));
                    gtk_rc_parse_string(tmpStr);
                }
            }

            if(opts->inactiveHighlight)
            {
                static const char *format="style \""QTC_RC_SETTING"HlFix\" "
                                          "{base[ACTIVE]=\"#%02X%02X%02X\""
                                          " text[ACTIVE]=\"#%02X%02X%02X\"}"
                                          "class \"*\" style \""QTC_RC_SETTING"HlFix\"";

                tmpStr=(char *)realloc(tmpStr, strlen(format));

                if(tmpStr)
                {
                    GdkColor *inactiveHighlightTextCol=opts->inactiveHighlight
                                            ? &qtSettings.colors[PAL_ACTIVE][COLOR_TEXT]
                                            : &qtSettings.colors[PAL_INACTIVE][COLOR_TEXT_SELECTED];

                    sprintf(tmpStr, format, toQtColor(qtSettings.inactiveSelectCol.red),
                                            toQtColor(qtSettings.inactiveSelectCol.green),
                                            toQtColor(qtSettings.inactiveSelectCol.blue),

                                            toQtColor(inactiveHighlightTextCol->red),
                                            toQtColor(inactiveHighlightTextCol->green),
                                            toQtColor(inactiveHighlightTextCol->blue));
                    gtk_rc_parse_string(tmpStr);
                }
            }

            if(GTK_APP_VMPLAYER==qtSettings.app)
                opts->shadeMenubars=SHADE_NONE;

            /* Tear off menu items dont seem to draw they're background, and the default background
               is drawn :-(  Fix/hack this by making that background the correct color */
            if(opts->lighterPopupMenuBgnd>1)
            {
                static const char *format="style \""QTC_RC_SETTING"Mnu\" "
                                          "{bg[NORMAL]=\"#%02X%02X%02X\"} "
                                          "class \"GtkMenu\" style \""QTC_RC_SETTING"Mnu\"";
                tmpStr=(char *)realloc(tmpStr, strlen(format)+32);

                if(tmpStr)
                {
                    GdkColor col;

                    shade(&qtSettings.colors[PAL_ACTIVE][COLOR_WINDOW], &col, QTC_TO_FACTOR(opts->lighterPopupMenuBgnd));
                    sprintf(tmpStr, format, toQtColor(col.red), toQtColor(col.green), toQtColor(col.blue));
                    gtk_rc_parse_string(tmpStr);
                }
            }

            if(opts->mapKdeIcons && qtSettings.icons)
            {
                static const char *constFormat="gtk-icon-theme-name=\"%s\" gtk-fallback-icon-theme=\"hicolor\"";
                tmpStr=(char *)realloc(tmpStr, strlen(constFormat)+strlen(qtSettings.icons)+1);

                sprintf(tmpStr, constFormat, qtSettings.icons);
                gtk_rc_parse_string(tmpStr);
            }
            
            if(opts->mapKdeIcons && (path=getIconPath()))
            {
                const char *iconTheme=qtSettings.icons ? qtSettings.icons : "XX";
                int  versionLen=1+strlen(VERSION)+1+strlen(iconTheme)+1+2+(6*2)+1;  /* '#' VERSION ' '<kde version> <..nums above..>\0 */
                char *version=(char *)malloc(versionLen);

                getGtk2CfgFile(&tmpStr, xdg, "qtcurve.gtk-icons");
                sprintf(version, "#%s %s %02X%02X%02X%02X%02X%02X%02X",
                                 VERSION,
                                 iconTheme,
                                 qtSettings.qt4 ? 4 : 3,
                                 qtSettings.iconSizes.smlTbSize,
                                 qtSettings.iconSizes.tbSize,
                                 qtSettings.iconSizes.dndSize,
                                 qtSettings.iconSizes.btnSize,
                                 qtSettings.iconSizes.mnuSize,
                                 qtSettings.iconSizes.dlgSize);

                if(!checkFileVersion(tmpStr, version, versionLen))
                {
                    static const char *constCmdStrFmt="perl "GTK_THEME_DIR"/map_kde_icons.pl "GTK_THEME_DIR"/icons%d %s %d %d %d %d %d %d %d %s "VERSION" > %s.%d && mv %s.%d %s";

                    const char *kdeprefix=kdeIconsPrefix();
                    int        fileNameLen=strlen(tmpStr);
                    char       *cmdStr=(char *)malloc(strlen(constCmdStrFmt)
                                                      +2+(4*6)+2+
                                                      strlen(iconTheme)+
                                                      (kdeprefix ? strlen(kdeprefix) : DEFAULT_ICON_PREFIX_LEN)+(fileNameLen*3)+64+1);

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
                                    iconTheme,
                                    tmpStr,
                                    getpid(),
                                    tmpStr,
                                    getpid(),
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

                if(qtSettings.fonts[FONT_GENERAL])
                    g_object_set(settings, "gtk-font-name", qtSettings.fonts[FONT_GENERAL], NULL);

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
                        gtk_rc_parse_string("style \""QTC_RC_SETTING"StBar\""
                                            "{ GtkStatusbar::shadow-type = 1 }" /*GtkStatusbar::has-resize-grip = FALSE }" */
                                            "class \"GtkStatusbar\" style"
                                            " \""QTC_RC_SETTING"StBar\"");
                    else
                        gtk_rc_parse_string("style \""QTC_RC_SETTING"SBar\""
                                            "{ GtkStatusbar::shadow-type = 0 }" /*GtkStatusbar::has-resize-grip = FALSE }" */
                                            "class \"GtkStatusbar\" style"
                                            " \""QTC_RC_SETTING"SBar\"");
#endif
                }

                /* The following settings only apply for GTK>=2.6.0 */
                if(!opts->gtkButtonOrder && NULL==gtk_check_version(2, 6, 0))
                    g_object_set(settings, "gtk-alternative-button-order", TRUE, NULL);
            }

            if(SLIDER_TRIANGULAR==opts->sliderStyle)
                gtk_rc_parse_string("style \""QTC_RC_SETTING"Sldr\" {GtkScale::slider_length = 11 GtkScale::slider_width = 18} class \"*\" style \""QTC_RC_SETTING"Sldr\"");
            else if(SLIDER_PLAIN_ROTATED==opts->sliderStyle || SLIDER_ROUND_ROTATED==opts->sliderStyle)
                gtk_rc_parse_string("style \""QTC_RC_SETTING"Sldr\" {GtkScale::slider_length = 13 GtkScale::slider_width = 21} class \"*\" style \""QTC_RC_SETTING"Sldr\"");

            if(qtSettings.fonts[FONT_GENERAL])
            {
                static const char *constFormat="style \""QTC_RC_SETTING"Fnt\" {font_name=\"%s\"} "
                                               "widget_class \"*\" style \""QTC_RC_SETTING"Fnt\" ";
                tmpStr=(char *)realloc(tmpStr, strlen(constFormat)+strlen(qtSettings.fonts[FONT_GENERAL])+1);

                sprintf(tmpStr, constFormat, qtSettings.fonts[FONT_GENERAL]);
                gtk_rc_parse_string(tmpStr);
            }

            if(qtSettings.fonts[FONT_BOLD])
            {
                static const char *constBoldPrefix="style \""QTC_RC_SETTING"BFnt\"{font_name=\"";
                static const char *constBoldSuffix="\"} class \"GtkProgress\" style \""QTC_RC_SETTING"BFnt\" "
                                                   "widget_class \"*GtkProgress*\" style \""QTC_RC_SETTING"BFnt\" ";

                if(opts->framelessGroupBoxes)
                {
                    static const char *constStdPrefix="style \""QTC_RC_SETTING"Fnt\"{font_name=\"";
                    static const char *constStdSuffix="\"} ";
                    static const char *constGrpBoxBoldSuffix="widget_class \"*Frame.GtkLabel\" style \""QTC_RC_SETTING"BFnt\" "
                                                             "widget_class \"*Statusbar.*Frame.GtkLabel\" style \""QTC_RC_SETTING"Fnt\"";
                    tmpStr=(char *)realloc(tmpStr, strlen(constStdPrefix)+strlen(qtSettings.fonts[FONT_GENERAL])+strlen(constStdSuffix)+
                                                   strlen(constBoldPrefix)+strlen(qtSettings.fonts[FONT_BOLD])+strlen(constBoldSuffix)+
                                                   strlen(constGrpBoxBoldSuffix)+1);

                    sprintf(tmpStr, "%s%s%s%s%s%s%s",
                                    constStdPrefix, qtSettings.fonts[FONT_GENERAL], constStdSuffix,
                                    constBoldPrefix, qtSettings.fonts[FONT_BOLD], constBoldSuffix,
                                    constGrpBoxBoldSuffix);
                }
                else
                {
                    tmpStr=(char *)realloc(tmpStr, strlen(constBoldPrefix)+strlen(qtSettings.fonts[FONT_BOLD])+strlen(constBoldSuffix)+1);
                    sprintf(tmpStr, "%s%s%s", constBoldPrefix, qtSettings.fonts[FONT_BOLD], constBoldSuffix);
                }

                gtk_rc_parse_string(tmpStr);
            }

            if(qtSettings.fonts[FONT_MENU] && qtSettings.fonts[FONT_GENERAL] && strcmp(qtSettings.fonts[FONT_MENU], qtSettings.fonts[FONT_GENERAL]))
            {
                static const char *constFormat="style \""QTC_RC_SETTING"MFnt\" {font_name=\"%s\"} "
                                               "widget_class \"*.*MenuItem.*\" style \""QTC_RC_SETTING"MFnt\" ";
                tmpStr=(char *)realloc(tmpStr, strlen(constFormat)+strlen(qtSettings.fonts[FONT_MENU])+1);

                sprintf(tmpStr, constFormat, qtSettings.fonts[FONT_MENU]);
                gtk_rc_parse_string(tmpStr);
            }

            if(qtSettings.fonts[FONT_TOOLBAR] && qtSettings.fonts[FONT_GENERAL] && strcmp(qtSettings.fonts[FONT_TOOLBAR], qtSettings.fonts[FONT_GENERAL]))
            {
                static const char *constFormat="style \""QTC_RC_SETTING"TbFnt\" {font_name=\"%s\"} "
                                               "widget_class \"*.*Toolbar.*\" style \""QTC_RC_SETTING"TbFnt\" ";
                tmpStr=(char *)realloc(tmpStr, strlen(constFormat)+strlen(qtSettings.fonts[FONT_TOOLBAR])+1);

                sprintf(tmpStr, constFormat, qtSettings.fonts[FONT_TOOLBAR]);
                gtk_rc_parse_string(tmpStr);
            }

            if(opts->thinnerMenuItems)
                gtk_rc_parse_string("style \""QTC_RC_SETTING"Mi\" {xthickness = 1 ythickness = 2 } "
                                    "class \"*MenuItem\" style \""QTC_RC_SETTING"Mi\"");

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
                gtk_rc_parse_string("style \""QTC_RC_SETTING"Mz\" { GtkComboBoxEntry::appears-as-list = 0 } class \"*\" style \""QTC_RC_SETTING"Mz\"");

            if(GTK_APP_MOZILLA==qtSettings.app || GTK_APP_JAVA==qtSettings.app)
                opts->scrollbarType=SCROLLBAR_WINDOWS;
            else
            {
                static const char *constSbStrFormat="style \""QTC_RC_SETTING"SBt\" "
                                                    "{ GtkScrollbar::has-backward-stepper=%d "
                                                      "GtkScrollbar::has-forward-stepper=%d "
                                                      "GtkScrollbar::has-secondary-backward-stepper=%d "
                                                      "GtkScrollbar::has-secondary-forward-stepper=%d } "
                                                    "class \"*\" style \""QTC_RC_SETTING"SBt\"";
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
                static const char *constStrFormat="style \""QTC_RC_SETTING"Crsr\" "
                                                    "{ GtkWidget::cursor-color=\"#%02X%02X%02X\" "
                                                      "GtkWidget::secondary-cursor-color=\"#%02X%02X%02X\" } "
                                                    "class \"*\" style \""QTC_RC_SETTING"Crsr\"";
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
            bool doEffect=EFFECT_NONE!=opts->buttonEffect;
            int  thickness=2;

            if(doEffect)
                gtk_rc_parse_string("style \""QTC_RC_SETTING"Etch\" "
                                    "{ xthickness = 3 ythickness = 3 } "
//                                     "style \""QTC_RC_SETTING"EtchI\" "
//                                     "{ GtkCheckButton::indicator_size = 15 } "
                                    "class \"*GtkRange\" style \""QTC_RC_SETTING"Etch\" "
                                    "class \"*GtkSpinButton\" style \""QTC_RC_SETTING"Etch\" "
                                    "class \"*GtkEntry\" style  \""QTC_RC_SETTING"Etch\" "
                                    "widget_class \"*Toolbar*Entry\" style \""QTC_RC_SETTING"Etch\" "
                                    "class \"*Button\" style \""QTC_RC_SETTING"Etch\""
                                    "class \"*GtkOptionMenu\" style \""QTC_RC_SETTING"Etch\""
                                    /*"class \"*GtkWidget\" style \"QtcEtchI\""*/);

            if(!opts->gtkScrollViews)
                gtk_rc_parse_string("style \""QTC_RC_SETTING"SV\""
                                    " { GtkScrolledWindow::scrollbar-spacing = 0 "
                                      " GtkScrolledWindow::scrollbars-within-bevel = 1 } "
                                    "class \"*GtkWidget\" style \""QTC_RC_SETTING"SV\"");

            /* Scrolled windows */
            if(opts->squareScrollViews)
                thickness=opts->gtkScrollViews ? 1 : 2;
            else if(opts->sunkenScrollViews)
                thickness=3;

            { /* C-Scope */
                static const char *constStrFormat="style \""QTC_RC_SETTING"SVt\" "
                                                    "{ xthickness = %d ythickness = %d } "
                                                      "class \"*GtkScrolledWindow\" style \""QTC_RC_SETTING"SVt\"";

                tmpStr=(char *)realloc(tmpStr, strlen(constStrFormat)+1);
                sprintf(tmpStr, constStrFormat, thickness, thickness);
                gtk_rc_parse_string(tmpStr);
            } /* C-Scope */

            { /* C-Scope */
                static const char *constStrFormat="style \""QTC_RC_SETTING"Pbar\" "
                                                    "{ xthickness = %d ythickness = %d } "
                                                      "widget_class \"*GtkProgressBar\" style \""QTC_RC_SETTING"Pbar\"";
                int pthickness=opts->fillProgress
                                ? doEffect
                                    ? 1
                                    : 0
                                : doEffect
                                    ? 2
                                    : 1;

                tmpStr=(char *)realloc(tmpStr, strlen(constStrFormat)+1);
                sprintf(tmpStr, constStrFormat, pthickness, pthickness);
                gtk_rc_parse_string(tmpStr);
            } /* C-Scope */
            } /* C-Scope 'doEffect' */

            { /* C-Scope */
                static const char *constStrFormat="style \""QTC_RC_SETTING"TT\" "
                                                    "{ xthickness = 4 ythickness = 4 bg[NORMAL] = \"#%02X%02X%02X\" fg[NORMAL] = \"#%02X%02X%02X\"} "
                                                    "widget \"gtk-tooltips*\" style \""QTC_RC_SETTING"TT\" "
                                                    "widget \"gtk-tooltip*\" style \""QTC_RC_SETTING"TT\"";

                tmpStr=(char *)realloc(tmpStr, strlen(constStrFormat)+1);
                sprintf(tmpStr, constStrFormat,
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP].red),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP].green),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP].blue),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP_TEXT].red),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP_TEXT].green),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TOOLTIP_TEXT].blue));
                gtk_rc_parse_string(tmpStr);
            } /* C-Scope */

            if( EFFECT_NONE==opts->buttonEffect)
                gtk_rc_parse_string("style \""QTC_RC_SETTING"Cmb\" { xthickness = 4 ythickness = 2 }"
                                    "widget_class \"*.GtkCombo.GtkEntry\" style \""QTC_RC_SETTING"Cmb\"");

            if(opts->round>=ROUND_FULL && EFFECT_NONE!=opts->buttonEffect)
                gtk_rc_parse_string("style \""QTC_RC_SETTING"Swt\" { xthickness = 3 ythickness = 2 }"
                                    "widget_class \"*.SwtFixed.GtkCombo.GtkButton\" style \""QTC_RC_SETTING"Swt\""
                                    "widget_class \"*.SwtFixed.GtkCombo.GtkEntry\" style \""QTC_RC_SETTING"Swt\"");

            if(opts->lighterPopupMenuBgnd && !opts->borderMenuitems)
                gtk_rc_parse_string("style \""QTC_RC_SETTING"M\" { xthickness=1 ythickness=1 }\n"
                                    "class \"*GtkMenu\" style \""QTC_RC_SETTING"M\"");

            { /* C-Scope */
                static const char *constStrFormat="style \""QTC_RC_SETTING"Tree\" "
                                                    "{ GtkTreeView::odd-row-color = \"#%02X%02X%02X\" GtkTreeView::even-row-color = \"#%02X%02X%02X\"} "
                                                    "widget \"*GtkTreeView*\" style \""QTC_RC_SETTING"Tree\"";
                int alt=haveAlternareListViewCol() ? COLOR_LV : COLOR_BACKGROUND;

                tmpStr=(char *)realloc(tmpStr, strlen(constStrFormat)+1);
                sprintf(tmpStr, constStrFormat,
                        toQtColor(qtSettings.colors[PAL_ACTIVE][alt].red),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][alt].green),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][alt].blue),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_BACKGROUND].red),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_BACKGROUND].green),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_BACKGROUND].blue));
                gtk_rc_parse_string(tmpStr);
            } /* C-Scope */

            if(!opts->useHighlightForMenu)
            {
                static const char *constStrFormat="style \""QTC_RC_SETTING"Mnu\" "
                                                  "{ text[ACTIVE] = \"#%02X%02X%02X\" "
                                                  " text[SELECTED] = \"#%02X%02X%02X\" } "
                                                  " class \"*MenuItem\" style \""QTC_RC_SETTING"Mnu\""
                                                  " widget_class \"*MenuBar*MenuItem\" style \""QTC_RC_SETTING"Mnu\""
                                                  " widget_class \"*.GtkAccelMenuItem\" style \""QTC_RC_SETTING"Mnu\""
                                                  " widget_class \"*.GtkRadioMenuItem\" style \""QTC_RC_SETTING"Mnu\""
                                                  " widget_class \"*.GtkCheckMenuItem\" style \""QTC_RC_SETTING"Mnu\""
                                                  " widget_class \"*.GtkImageMenuItem\" style \""QTC_RC_SETTING"Mnu\"";

                tmpStr=(char *)realloc(tmpStr, strlen(constStrFormat)+1);
                sprintf(tmpStr, constStrFormat,
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT].red),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT].green),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT].blue),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT].red),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT].green),
                        toQtColor(qtSettings.colors[PAL_ACTIVE][COLOR_TEXT].blue));
                gtk_rc_parse_string(tmpStr);
            }

            /* Mozilla seems to assume that all scrolledviews are square :-(
               So, set the xthickness and ythickness to 1, and in qtcurve.c draw these as sqare */
            if(isMozilla())
                gtk_rc_parse_string("style \""QTC_RC_SETTING"SVm\""
                                    " { xthickness=1 ythickness=1 } "
                                    "widget_class \"GtkWindow.GtkFixed.GtkScrolledWindow\" style \""QTC_RC_SETTING"SVm\"");
            

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
        int i;

        for(i=0; i<FONT_NUM_TOTAL; ++i)
        {
            if(qtSettings.fonts[i])
                free(qtSettings.fonts[i]);
            qtSettings.fonts[i]=NULL;
        }

        if(qtSettings.icons)
            free(qtSettings.icons);
        qtSettings.icons=NULL;
    }
}

#define SET_COLOR_PAL(st, rc, itm, ITEM, state, QTP_COL, PAL) \
    st->itm[state]=rc->color_flags[state]&ITEM \
        ? rc->itm[state] \
        : qtSettings.colors[state==GTK_STATE_INSENSITIVE \
                                ? PAL_DISABLED \
                                : PAL ][QTP_COL];

#define SET_COLOR_PAL_ACT(st, rc, itm, ITEM, state, QTP_COL) \
    st->itm[state]=rc->color_flags[state]&ITEM \
        ? rc->itm[state] \
        : qtSettings.colors[PAL_ACTIVE][QTP_COL];

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
    SET_COLOR_PAL_ACT(style, rc_style, base, GTK_RC_BASE, GTK_STATE_INSENSITIVE, COLOR_WINDOW)
    /*SET_COLOR(style, rc_style, base, GTK_RC_BASE, GTK_STATE_ACTIVE, COLOR_SELECTED)*/
    style->base[GTK_STATE_ACTIVE]=qtSettings.inactiveSelectCol;
    SET_COLOR(style, rc_style, base, GTK_RC_BASE, GTK_STATE_PRELIGHT, COLOR_SELECTED)

    SET_COLOR(style, rc_style, text, GTK_RC_TEXT, GTK_STATE_NORMAL, COLOR_TEXT)
    SET_COLOR(style, rc_style, text, GTK_RC_TEXT, GTK_STATE_SELECTED, COLOR_TEXT_SELECTED)
    SET_COLOR(style, rc_style, text, GTK_RC_TEXT, GTK_STATE_INSENSITIVE, COLOR_TEXT)
    /*SET_COLOR(style, rc_style, text, GTK_RC_TEXT, GTK_STATE_ACTIVE, COLOR_TEXT_SELECTED)*/

    if(opts->inactiveHighlight)
        SET_COLOR(style, rc_style, text, GTK_RC_TEXT, GTK_STATE_ACTIVE, COLOR_TEXT)
    else
        SET_COLOR_PAL(style, rc_style, text, GTK_RC_TEXT, GTK_STATE_ACTIVE, COLOR_TEXT_SELECTED, PAL_INACTIVE)

    SET_COLOR(style, rc_style, text, GTK_RC_TEXT, GTK_STATE_PRELIGHT, COLOR_TEXT)

    SET_COLOR(style, rc_style, fg, GTK_RC_FG, GTK_STATE_NORMAL, COLOR_WINDOW_TEXT)
    SET_COLOR(style, rc_style, fg, GTK_RC_FG, GTK_STATE_SELECTED, COLOR_TEXT_SELECTED)
    SET_COLOR(style, rc_style, fg, GTK_RC_FG, GTK_STATE_INSENSITIVE, COLOR_MID)
    SET_COLOR(style, rc_style, fg, GTK_RC_FG, GTK_STATE_ACTIVE, COLOR_WINDOW_TEXT)
    SET_COLOR(style, rc_style, fg, GTK_RC_FG, GTK_STATE_PRELIGHT, COLOR_WINDOW_TEXT)
}
