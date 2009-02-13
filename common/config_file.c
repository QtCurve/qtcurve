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

#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/types.h>

#define QTC_MAKE_VERSION(a, b) (((a) << 16) | ((b) << 8))

#define QTC_MAX_FILENAME_LEN   1024
#define QTC_MAX_INPUT_LINE_LEN 256
#define QTC_FILE               "qtcurvestylerc"
#define QTC_VERSION_KEY        "version"

#ifdef CONFIG_READ
static int c2h(char ch)
{
    return (ch>='0' && ch<='9') ? ch-'0' :
           (ch>='a' && ch<='f') ? 10+(ch-'a') :
           (ch>='A' && ch<='F') ? 10+(ch-'A') :
           0;
}

#define ATOH(str) ((c2h(*str)<<4)+c2h(*(str+1)))

static void setRgb(color *col, const char *str)
{
    if(str && strlen(str)>6)
    {
        int offset='#'==str[0] ? 1 : 0;
#ifdef __cplusplus
        col->setRgb(ATOH(&str[offset]), ATOH(&str[offset+2]), ATOH(&str[offset+4]));
#else
        col->red=ATOH(&str[offset])<<8;
        col->green=ATOH(&str[offset+2])<<8;
        col->blue=ATOH(&str[offset+4])<<8;
        col->pixel=0;
#endif
    }
    else
#ifdef __cplusplus
        col->setRgb(0, 0, 0);
#else
        col->red=col->green=col->blue=col->pixel=0;
#endif
}

static EDefBtnIndicator toInd(const char *str, EDefBtnIndicator def)
{
    if(str)
    {
        if(0==memcmp(str, "fontcolor", 9) || 0==memcmp(str, "border", 6))
            return IND_FONT_COLOR;
        if(0==memcmp(str, "none", 4))
            return IND_NONE;
        if(0==memcmp(str, "corner", 6))
            return IND_CORNER;
        if(0==memcmp(str, "colored", 7))
            return IND_COLORED;
        if(0==memcmp(str, "tint", 4))
            return IND_TINT;
        if(0==memcmp(str, "glow", 4))
            return IND_GLOW;
    }

    return def;
}

static ELine toLine(const char *str, ELine def)
{
    if(str)
    {
        if(0==memcmp(str, "dashes", 6))
            return LINE_DASHES;
        if(0==memcmp(str, "none", 4))
            return LINE_NONE;
        if(0==memcmp(str, "sunken", 6))
            return LINE_SUNKEN;
        if(0==memcmp(str, "dots", 4))
            return LINE_DOTS;
        if(0==memcmp(str, "flat", 4))
            return LINE_FLAT;
    }
    return def;
}

static ETBarBorder toTBarBorder(const char *str, ETBarBorder def)
{
    if(str)
    {
        if(0==memcmp(str, "dark", 4))
            return 0==memcmp(&str[4], "-all", 4) ? TB_DARK_ALL : TB_DARK;
        if(0==memcmp(str, "none", 4))
            return TB_NONE;
        if(0==memcmp(str, "light", 5))
            return 0==memcmp(&str[5], "-all", 4) ? TB_LIGHT_ALL : TB_LIGHT;
    }
    return def;
}

static EMouseOver toMouseOver(const char *str, EMouseOver def)
{
    if(str)
    {
        if(0==memcmp(str, "true", 4) || 0==memcmp(str, "colored", 7))
            return MO_COLORED;
        if(0==memcmp(str, "plastik", 7))
            return MO_PLASTIK;
        if(0==memcmp(str, "glow", 4))
            return MO_GLOW;
        if(0==memcmp(str, "false", 4) || 0==memcmp(str, "none", 4))
            return MO_NONE;
    }
    return def;
}

static EAppearance toAppearance(const char *str, EAppearance def, bool allowFade)
{
    if(str)
    {
        if(0==memcmp(str, "flat", 4))
            return APPEARANCE_FLAT;
        if(0==memcmp(str, "raised", 6))
            return APPEARANCE_RAISED;
        if(0==memcmp(str, "gradient", 8) || 0==memcmp(str, "lightgradient", 13))
            return APPEARANCE_GRADIENT;
        if(0==memcmp(str, "splitgradient", 13))
            return APPEARANCE_SPLIT_GRADIENT;
        if(0==memcmp(str, "glass", 5) || 0==memcmp(str, "shinyglass", 10))
            return APPEARANCE_SHINY_GLASS;
        if(0==memcmp(str, "dullglass", 9))
            return APPEARANCE_DULL_GLASS;
        if(0==memcmp(str, "inverted", 8))
            return APPEARANCE_INVERTED;
        if(0==memcmp(str, "bevelled", 8))
            return APPEARANCE_BEVELLED;
        if(allowFade && 0==memcmp(str, "fade", 4))
            return APPEARANCE_FADE;

        if(0==memcmp(str, "customgradient", 14) && strlen(str)>14)
        {
            int i=atoi(&str[14]);

            if(i>=1 && i<(QTC_NUM_CUSTOM_GRAD+1))
                return (EAppearance)(APPEARANCE_CUSTOM1+(i-1));
        }
    }
    return def;
}

static EShade toShade(const char *str, bool allowDarken, EShade def)
{
    if(str)
    {
        /* true/false is from 0.25... */
        if(0==memcmp(str, "true", 4) || 0==memcmp(str, "selected", 8))
            return SHADE_BLEND_SELECTED;
        if(0==memcmp(str, "origselected", 12))
            return SHADE_SELECTED;
        if(allowDarken && 0==memcmp(str, "darken", 6))
            return SHADE_DARKEN;
        if(0==memcmp(str, "custom", 6))
            return SHADE_CUSTOM;
        if(0==memcmp(str, "none", 4))
            return SHADE_NONE;
    }

    return def;
}

/* Prior to 0.42 round was a bool - so need to read 'false' as 'none' */
static ERound toRound(const char *str, ERound def)
{
    if(str)
    {
        if(0==memcmp(str, "none", 4) || 0==memcmp(str, "false", 5))
            return ROUND_NONE;
        if(0==memcmp(str, "slight", 6))
            return ROUND_SLIGHT;
        if(0==memcmp(str, "full", 4))
            return ROUND_FULL;
        if(0==memcmp(str, "extra", 5))
            return ROUND_EXTRA;
    }

    return def;
}

static EScrollbar toScrollbar(const char *str, EScrollbar def)
{
    if(str)
    {
        if(0==memcmp(str, "kde", 3))
            return SCROLLBAR_KDE;
        if(0==memcmp(str, "windows", 7))
            return SCROLLBAR_WINDOWS;
        if(0==memcmp(str, "platinum", 8))
            return SCROLLBAR_PLATINUM;
        if(0==memcmp(str, "next", 4))
            return SCROLLBAR_NEXT;
        if(0==memcmp(str, "none", 4))
            return SCROLLBAR_NONE;
    }

    return def;
}

static EEffect toEffect(const char *str, EEffect def)
{
    if(str)
    {
        if(0==memcmp(str, "none", 4))
            return EFFECT_NONE;
        if(0==memcmp(str, "shadow", 6))
            return EFFECT_SHADOW;
        if(0==memcmp(str, "etch", 4))
            return EFFECT_ETCH;
    }

    return def;
}

static EShading toShading(const char *str, EShading def)
{
    if(str)
    {
        if(0==memcmp(str, "simple", 6))
            return SHADING_SIMPLE;
        if(0==memcmp(str, "hsl", 3))
            return SHADING_HSL;
        if(0==memcmp(str, "hsv", 3))
            return SHADING_HSV;
    }

    return def;
}

static EStripe toStripe(const char *str, EStripe def)
{
    if(str)
    {
        if(0==memcmp(str, "plain", 5) || 0==memcmp(str, "true", 4))
            return STRIPE_PLAIN;
        if(0==memcmp(str, "none", 4) || 0==memcmp(str, "false", 5))
            return STRIPE_NONE;
        if(0==memcmp(str, "diagonal", 8))
            return STRIPE_DIAGONAL;
    }

    return def;
}

static ESliderStyle toSlider(const char *str, ESliderStyle def)
{
    if(str)
    {
        if(0==memcmp(str, "round", 5))
            return SLIDER_ROUND;
        if(0==memcmp(str, "plain", 5))
            return SLIDER_PLAIN;
        if(0==memcmp(str, "r-round", 7))
            return SLIDER_ROUND_ROTATED;
        if(0==memcmp(str, "r-plain", 7))
            return SLIDER_PLAIN_ROTATED;
        if(0==memcmp(str, "triangular", 10))
            return SLIDER_TRIANGULAR;
    }

    return def;
}

static EColor toEColor(const char *str, EColor def)
{
    if(str)
    {
        if(0==memcmp(str, "base", 4))
            return ECOLOR_BASE;
        if(0==memcmp(str, "dark", 4))
            return ECOLOR_DARK;
        if(0==memcmp(str, "background", 10))
            return ECOLOR_BACKGROUND;
    }

    return def;
}

static EFocus toFocus(const char *str, EFocus def)
{
    if(str)
    {
        if(0==memcmp(str, "standard", 8))
            return FOCUS_STANDARD;
        if(0==memcmp(str, "highlight", 9))
            return FOCUS_HIGHLIGHT;
        if(0==memcmp(str, "background", 10))
            return FOCUS_BACKGROUND;
        if(0==memcmp(str, "filled", 6))
            return FOCUS_FILLED;
    }

    return def;
}

#endif

#ifdef CONFIG_WRITE
#include <kstandarddirs.h>
#endif

static const char * getHome()
{
    static const char *home=NULL;

    if(!home)
    {
        struct passwd *p=getpwuid(getuid());

        if(p)
            home=p->pw_dir;
        else
        {
            char *env=getenv("HOME");

            if(env)
                home=env;
        }

        if(!home)
            home="/tmp";
    }

    return home;
}

static const char *xdgConfigFolder()
{
    static char xdgDir[QTC_MAX_FILENAME_LEN]={'\0'};

    if(!xdgDir[0])
    {
        static const char *home=NULL;

#if 0
        char *env=getenv("XDG_CONFIG_HOME");

        /*
            Check the setting of XDG_CONFIG_HOME
            For some reason, sudo leaves the env vars set to those of the
            caller - so XDG_CONFIG_HOME would point to the users setting, and
            not roots.

            Therefore, check that home is first part of XDG_CONFIG_HOME
        */

        if(env && 0==getuid())
        {
            if(!home)
                home=getHome();
            if(home && home!=strstr(env, home))
                env=NULL;
        }
#else
        /*
           Hmm... for 'root' dont bother to check env var, just set to ~/.config
           - as problems would arise if "sudo kcmshell style", and then
           "sudo su" / "kcmshell style". The 1st would write to ~/.config, but
           if root has a XDG_ set then that would be used on the second :-(
        */
        char *env=0==getuid() ? NULL : getenv("XDG_CONFIG_HOME");
#endif

        if(!env)
        {
            if(!home)
                home=getHome();

            sprintf(xdgDir, "%s/.config", home);
        }
        else
            strcpy(xdgDir, env);

#if defined CONFIG_WRITE || !defined __cplusplus
        {
        struct stat info;

        if(0!=lstat(xdgDir, &info))
        {
#ifdef __cplusplus
            KStandardDirs::makeDir(xdgDir, 0755);
#else
            g_mkdir_with_parents(xdgDir, 0755);
#endif
        }
        }
#endif
    }

    return xdgDir;
}

#ifdef CONFIG_READ

#ifdef __cplusplus
#define QTC_IS_BLACK(A) (0==(A).red() && 0==(A).green() && 0==(A).blue())
#else
#define QTC_IS_BLACK(A) (0==(A).red && 0==(A).green && 0==(A).blue)
#endif

static void checkColor(EShade *s, color *c)
{
    if(SHADE_CUSTOM==*s && QTC_IS_BLACK(*c))
        *s=SHADE_NONE;
}

#ifdef __cplusplus

#if QT_VERSION >= 0x040000
#include <QMap>
#include <QFile>
#include <QTextStream>
#define QTC_LATIN1(A) A.toLatin1()
#else
#define QTC_LATIN1(A) A.latin1()

#include <qmap.h>
#include <qfile.h>
#include <qtextstream.h>
#endif

class QtCConfig
{
    public:

    QtCConfig(const QString &filename);

    bool            ok() const { return values.count()>0; }
    const QString & readEntry(const QString &key, const QString &def=QString::null);

    private:

    QMap<QString, QString> values;
};

QtCConfig::QtCConfig(const QString &filename)
{
    QFile f(filename);

#if QT_VERSION >= 0x040000
    if(f.open(QIODevice::ReadOnly))
#else
    if(f.open(IO_ReadOnly))
#endif
    {
        QTextStream stream(&f);
        QString     line;

        while(!stream.atEnd())
        {
            line = stream.readLine();
#if QT_VERSION >= 0x040000
            int pos=line.indexOf('=');
#else
            int pos=line.find('=');
#endif
            if(-1!=pos)
                values[line.left(pos)]=line.mid(pos+1);
        }
        f.close();
    }
}

inline const QString & QtCConfig::readEntry(const QString &key, const QString &def)
{
    return values.contains(key) ? values[key] : def;
}

inline QString readStringEntry(QtCConfig &cfg, const QString &key)
{
    return cfg.readEntry(key);
}

static int readNumEntry(QtCConfig &cfg, const QString &key, int def)
{
    const QString &val(readStringEntry(cfg, key));

    return val.isEmpty() ? def : val.toInt();
}

#if QT_VERSION >= 0x040000
#define QTC_LATIN1(A) A.toLatin1().constData()
#else
#define QTC_LATIN1(A) A.latin1()
#endif

static int readVersionEntry(QtCConfig &cfg, const QString &key)
{
    const QString &val(readStringEntry(cfg, key));
    int           major, minor;

    return !val.isEmpty() && 2==sscanf(QTC_LATIN1(val), "%d.%d", &major, &minor)
            ? QTC_MAKE_VERSION(major, minor)
            : 0;
}

static bool readBoolEntry(QtCConfig &cfg, const QString &key, bool def)
{
    const QString &val(readStringEntry(cfg, key));

    return val.isEmpty() ? def : (val=="true" ? true : false);
}

#define QTC_CFG_READ_COLOR(ENTRY) \
    { \
        QString sVal(cfg.readEntry(#ENTRY)); \
        if(sVal.isEmpty()) \
            opts->ENTRY=def.ENTRY; \
        else \
            setRgb(&(opts->ENTRY), QTC_LATIN1(sVal)); \
    }

#else

static char * lookupCfgHash(GHashTable **cfg, char *key, char *val)
{
    char *rv=NULL;

    if(!*cfg)
        *cfg=g_hash_table_new(g_str_hash, g_str_equal);
    else
        rv=(char *)g_hash_table_lookup(*cfg, key);

    if(!rv && val)
    {
        g_hash_table_insert(*cfg, g_strdup(key), g_strdup(val));
        rv=(char *)g_hash_table_lookup(*cfg, key);
    }

    return rv;
}

static GHashTable * loadConfig(const char *filename)
{
    FILE       *f=fopen(filename, "r");
    GHashTable *cfg=NULL;

    if(f)
    {
        char line[QTC_MAX_INPUT_LINE_LEN];

        while(NULL!=fgets(line, QTC_MAX_INPUT_LINE_LEN-1, f))
        {
            char *eq=strchr(line, '=');
            int  pos=eq ? eq-line : -1;

            if(pos>0)
            {
                char *endl=strchr(line, '\n');

                if(endl)
                    *endl='\0';

                line[pos]='\0';

                lookupCfgHash(&cfg, line, &line[pos+1]);
            }
        }

        fclose(f);
    }

    return cfg;
}

static void releaseConfig(GHashTable *cfg)
{
    g_hash_table_destroy(cfg);
}

static char * readStringEntry(GHashTable *cfg, char *key)
{
    return lookupCfgHash(&cfg, key, NULL);
}

static int readNumEntry(GHashTable *cfg, char *key, int def)
{
    char *str=readStringEntry(cfg, key);

    return str ? atoi(str) : def;
}

static int readVersionEntry(GHashTable *cfg, char *key)
{
    char *str=readStringEntry(cfg, key);
    int  major, minor;

    return str && 2==sscanf(str, "%d.%d", &major, &minor)
            ? QTC_MAKE_VERSION(major, minor)
            : 0;
}

static gboolean readBoolEntry(GHashTable *cfg, char *key, gboolean def)
{
    char *str=readStringEntry(cfg, key);

    return str ? (0==memcmp(str, "true", 4) ? true : false) : def;
}

#define QTC_LATIN1(A) A

#define QTC_CFG_READ_COLOR(ENTRY) \
    { \
        const char *str=readStringEntry(cfg, #ENTRY); \
    \
        if(str) \
            setRgb(&(opts->ENTRY), str); \
        else \
            opts->ENTRY=def.ENTRY; \
    }
#endif

#define QTC_CFG_READ_NUM(ENTRY) \
    opts->ENTRY=readNumEntry(cfg, #ENTRY, def.ENTRY);

#define QTC_CFG_READ_BOOL(ENTRY) \
    opts->ENTRY=readBoolEntry(cfg, #ENTRY, def.ENTRY);

#define QTC_CFG_READ_ROUND(ENTRY) \
    opts->ENTRY=toRound(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def.ENTRY);

#define QTC_CFG_READ_INT(ENTRY) \
    opts->ENTRY=readNumEntry(cfg, #ENTRY, def.ENTRY);

#define QTC_CFG_READ_INT_BOOL(ENTRY) \
    if(readBoolEntry(cfg, #ENTRY, false)) \
        opts->ENTRY=def.ENTRY; \
    else \
        opts->ENTRY=readNumEntry(cfg, #ENTRY, def.ENTRY);
    
#define QTC_CFG_READ_TB_BORDER(ENTRY) \
    opts->ENTRY=toTBarBorder(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def.ENTRY);

#define QTC_CFG_READ_MOUSE_OVER(ENTRY) \
    opts->ENTRY=toMouseOver(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def.ENTRY);

#define QTC_CFG_READ_APPEARANCE(ENTRY, ALLOW_FADE) \
    opts->ENTRY=toAppearance(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def.ENTRY, ALLOW_FADE);

/*
#define QTC_CFG_READ_APPEARANCE(ENTRY) \
    opts->ENTRY=toAppearance(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def.ENTRY);
*/

#define QTC_CFG_READ_STRIPE(ENTRY) \
    opts->ENTRY=toStripe(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def.ENTRY);

#define QTC_CFG_READ_SLIDER(ENTRY) \
    opts->ENTRY=toSlider(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def.ENTRY);

#define QTC_CFG_READ_DEF_BTN(ENTRY) \
    opts->ENTRY=toInd(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def.ENTRY);

#define QTC_CFG_READ_LINE(ENTRY) \
    opts->ENTRY=toLine(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def.ENTRY);

#define QTC_CFG_READ_SHADE(ENTRY, AD) \
    opts->ENTRY=toShade(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), AD, def.ENTRY);

#define QTC_CFG_READ_SCROLLBAR(ENTRY) \
    opts->ENTRY=toScrollbar(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def.ENTRY);

#define QTC_CFG_READ_EFFECT(ENTRY) \
    opts->ENTRY=toEffect(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def.ENTRY);

#ifdef QTC_CONFIG_DIALOG
#define QTC_CFG_READ_SHADING(ENTRY, UNUSED) \
    opts->ENTRY=toShading(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def.ENTRY);
#else
#define QTC_CFG_READ_SHADING(ENTRY, DEF) \
    ENTRY=toShading(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), DEF);
#endif

#define QTC_CFG_READ_ECOLOR(ENTRY) \
    opts->ENTRY=toEColor(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def.ENTRY);

#define QTC_CFG_READ_FOCUS(ENTRY) \
    opts->ENTRY=toFocus(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def.ENTRY);

static void checkAppearance(EAppearance *ap, Options *opts)
{
    if(*ap>=APPEARANCE_CUSTOM1 && *ap<(APPEARANCE_CUSTOM1+QTC_NUM_CUSTOM_GRAD))
    {
#ifdef __cplusplus
        if(opts->customGradient.end()==opts->customGradient.find(*ap))
#else
        if(!opts->customGradient[*ap-APPEARANCE_CUSTOM1])
#endif
            if(ap==&opts->appearance)
                *ap=APPEARANCE_FLAT;
            else
                *ap=opts->appearance;
    }
}

static void defaultSettings(Options *opts);

#ifdef __cplusplus
static bool readConfig(const QString &file, Options *opts, Options *defOpts=0L)
#else
static bool readConfig(const char *file, Options *opts, Options *defOpts)
#endif
{
#ifdef __cplusplus
    if(file.isEmpty())
    {
        const char *xdg=xdgConfigFolder();

        if(xdg)
        {
            QString filename(xdg);

            filename+="/"QTC_FILE;
            return readConfig(filename, opts, defOpts);
        }
    }
#else
    if(!file)
    {
        const char *xdg=xdgConfigFolder();

        if(xdg)
        {
            char filename[QTC_MAX_FILENAME_LEN];

            sprintf(filename, "%s/"QTC_FILE, xdg);
            return readConfig(filename, opts, defOpts);
        }
    }
#endif
    else
    {
#ifdef __cplusplus
        QtCConfig cfg(file);

        if(cfg.ok())
        {
#else
        GHashTable *cfg=loadConfig(file);

        if(cfg)
        {
#endif
            int     i,
                    version=readVersionEntry(cfg, QTC_VERSION_KEY);
            Options def;

            /* If we were supplied a default set of values (from config dialog), then
               take a copy of these settings - as we will change things if version numbers
               are different... */
            if(defOpts)
                def=*defOpts;
            else
                defaultSettings(&def);

            /* Check if the config file expects old default values... */
            if(version<QTC_MAKE_VERSION(0, 61))
            {
                def.coloredMouseOver=MO_PLASTIK;
                def.buttonEffect=EFFECT_NONE;
                def.defBtnIndicator=IND_TINT;
                def.vArrows=false;
                def.toolbarAppearance=APPEARANCE_GRADIENT;
                def.focus=FOCUS_STANDARD;
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
                def.selectionAppearance=APPEARANCE_FLAT;
#endif
                def.flatSbarButtons=false;
                def.comboSplitter=true;
                def.handles=LINE_DOTS;
                def.lighterPopupMenuBgnd=15;
                def.activeTabAppearance=APPEARANCE_GRADIENT;
                def.groupBoxLine=false;
                def.shadeSliders=SHADE_BLEND_SELECTED;
            }

            QTC_CFG_READ_NUM(passwordChar)
            QTC_CFG_READ_ROUND(round)
            QTC_CFG_READ_INT(highlightFactor)
            QTC_CFG_READ_INT_BOOL(lighterPopupMenuBgnd)
            QTC_CFG_READ_TB_BORDER(toolbarBorders)
            QTC_CFG_READ_APPEARANCE(appearance, false)
            QTC_CFG_READ_BOOL(fixParentlessDialogs)
            QTC_CFG_READ_STRIPE(stripedProgress)
            QTC_CFG_READ_SLIDER(sliderStyle)
            QTC_CFG_READ_BOOL(animatedProgress)
            QTC_CFG_READ_BOOL(embolden)
            QTC_CFG_READ_DEF_BTN(defBtnIndicator)
            QTC_CFG_READ_LINE(sliderThumbs)
            QTC_CFG_READ_LINE(handles)
            QTC_CFG_READ_BOOL(highlightTab)
            QTC_CFG_READ_BOOL(colorSelTab)
            QTC_CFG_READ_SHADE(shadeSliders, false)
            QTC_CFG_READ_SHADE(shadeMenubars, true)
            QTC_CFG_READ_SHADE(shadeCheckRadio, false)
            QTC_CFG_READ_APPEARANCE(menubarAppearance, false)
            QTC_CFG_READ_APPEARANCE(menuitemAppearance, true)
            QTC_CFG_READ_APPEARANCE(toolbarAppearance, false)
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
            QTC_CFG_READ_APPEARANCE(selectionAppearance, false)
#endif
            QTC_CFG_READ_LINE(toolbarSeparators)
            QTC_CFG_READ_LINE(splitters)
            QTC_CFG_READ_BOOL(customMenuTextColor)
            QTC_CFG_READ_MOUSE_OVER(coloredMouseOver)
            QTC_CFG_READ_BOOL(menubarMouseOver)
            QTC_CFG_READ_BOOL(useHighlightForMenu)
            QTC_CFG_READ_BOOL(shadeMenubarOnlyWhenActive)
            QTC_CFG_READ_BOOL(thinnerMenuItems)
            QTC_CFG_READ_COLOR(customSlidersColor)
            QTC_CFG_READ_COLOR(customMenubarsColor)
            QTC_CFG_READ_COLOR(customMenuSelTextColor)
            QTC_CFG_READ_COLOR(customMenuNormTextColor)
            QTC_CFG_READ_COLOR(customCheckRadioColor)
            QTC_CFG_READ_SCROLLBAR(scrollbarType)
            QTC_CFG_READ_EFFECT(buttonEffect)
            QTC_CFG_READ_APPEARANCE(lvAppearance, false)
            QTC_CFG_READ_APPEARANCE(tabAppearance, false)
            QTC_CFG_READ_APPEARANCE(activeTabAppearance, false)
            QTC_CFG_READ_APPEARANCE(sliderAppearance, false)
            QTC_CFG_READ_APPEARANCE(progressAppearance, false)
            QTC_CFG_READ_APPEARANCE(progressGrooveAppearance, false)
            QTC_CFG_READ_ECOLOR(progressGrooveColor)
            QTC_CFG_READ_FOCUS(focus)
            QTC_CFG_READ_BOOL(lvLines)
            QTC_CFG_READ_BOOL(drawStatusBarFrames)
            QTC_CFG_READ_BOOL(fillSlider)
            QTC_CFG_READ_BOOL(roundMbTopOnly)
            QTC_CFG_READ_BOOL(borderMenuitems)
            QTC_CFG_READ_BOOL(darkerBorders)
            QTC_CFG_READ_BOOL(vArrows)
            QTC_CFG_READ_BOOL(xCheck)
            QTC_CFG_READ_BOOL(framelessGroupBoxes)
            QTC_CFG_READ_BOOL(groupBoxLine)
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
            QTC_CFG_READ_BOOL(fadeLines)
#endif
            QTC_CFG_READ_BOOL(inactiveHighlight)
            QTC_CFG_READ_BOOL(colorMenubarMouseOver)
            QTC_CFG_READ_BOOL(crHighlight)
            QTC_CFG_READ_BOOL(crButton)
            QTC_CFG_READ_BOOL(fillProgress)
            QTC_CFG_READ_BOOL(comboSplitter)
            QTC_CFG_READ_BOOL(squareScrollViews)
            QTC_CFG_READ_BOOL(highlightScrollViews)
            QTC_CFG_READ_BOOL(sunkenScrollViews)
            QTC_CFG_READ_BOOL(flatSbarButtons)
#if defined __cplusplus || defined QTC_GTK2_MENU_STRIPE
            QTC_CFG_READ_BOOL(menuStripe)
            QTC_CFG_READ_APPEARANCE(menuStripeAppearance, false)
#endif
            QTC_CFG_READ_BOOL(gtkScrollViews)
#ifdef __cplusplus
            QTC_CFG_READ_BOOL(stdSidebarButtons)
            QTC_CFG_READ_BOOL(gtkComboMenus)
            QTC_CFG_READ_BOOL(colorTitlebarOnly)
/*
#else
            QTC_CFG_READ_BOOL(setDialogButtonOrder)
*/
#endif
#if !defined __cplusplus || defined QTC_CONFIG_DIALOG
            QTC_CFG_READ_BOOL(mapKdeIcons)
#endif
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
            QTC_CFG_READ_BOOL(gtkButtonOrder)
#endif
#ifdef __cplusplus
            QTC_CFG_READ_APPEARANCE(titlebarAppearance, false)
            QTC_CFG_READ_APPEARANCE(inactiveTitlebarAppearance, false)
            QTC_CFG_READ_APPEARANCE(titlebarButtonAppearance, false)

            if(APPEARANCE_BEVELLED==opts->titlebarAppearance)
                opts->titlebarAppearance=APPEARANCE_GRADIENT;
            else if(APPEARANCE_RAISED==opts->titlebarAppearance)
                opts->titlebarAppearance=APPEARANCE_FLAT;

            if(APPEARANCE_BEVELLED==opts->inactiveTitlebarAppearance)
                opts->inactiveTitlebarAppearance=APPEARANCE_GRADIENT;
            else if(APPEARANCE_RAISED==opts->inactiveTitlebarAppearance)
                opts->inactiveTitlebarAppearance=APPEARANCE_FLAT;
#endif
            QTC_CFG_READ_SHADING(shading, shading);

#ifdef __cplusplus
#if (defined QT_VERSION && (QT_VERSION >= 0x040000))
            QStringList shades(readStringEntry(cfg, "customShades").split(',', QString::SkipEmptyParts));
#else
            QStringList shades(QStringList::split(',', readStringEntry(cfg, "customShades")));
#endif

            if(NUM_STD_SHADES==shades.size())
            {
                QStringList::ConstIterator it(shades.begin());
                bool                       ok(true);

                opts->customShades.resize(NUM_STD_SHADES);
                for(i=0; i<NUM_STD_SHADES && ok; ++i, ++it)
                    opts->customShades[i]=(*it).toDouble(&ok);

                if(!ok)
                    opts->customShades.clear();
            }

            for(i=APPEARANCE_CUSTOM1; i<(APPEARANCE_CUSTOM1+QTC_NUM_CUSTOM_GRAD+1); ++i)
            {
                QString gradKey;

                if(i==QTC_NUM_CUSTOM_GRAD)
                    gradKey="sunkengradient";
                else
                    gradKey.sprintf("customgradient%d", (i-APPEARANCE_CUSTOM1)+1);

#if (defined QT_VERSION && (QT_VERSION >= 0x040000))
                QStringList vals(readStringEntry(cfg, gradKey).split(',', QString::SkipEmptyParts));
#else
                QStringList vals(QStringList::split(',', readStringEntry(cfg, gradKey)));
#endif

                if(vals.size())
                    opts->customGradient.erase((EAppearance)i);

                if(vals.size()>=5 && vals.size()%2) // Need odd number...
                {
                    QStringList::ConstIterator it(vals.begin()),
                                               end(vals.end());
                    bool                       ok(true);
                    CustomGradient             grad;

                    if((*it)=="true")
                        grad.lightBorder=true;
                    else if((*it)=="false")
                        grad.lightBorder=false;
                    else
                        ok=false;

                    if(ok)
                    {
                        int j;

                        for(++it, j=0; it!=end && ok; ++it, ++j)
                        {
                            double pos=(*it).toDouble(&ok),
                                   val=ok ? (*(++it)).toDouble(&ok) : 0.0;

                            if(ok)
                                ok=(pos>=0 && pos<=1.0) &&
                                   (val>=0.0 && val<=2.0);

                            if(ok)
                                grad.grad.insert(Gradient(pos, val));
                        }
                    }

                    if(ok)
                        opts->customGradient[(EAppearance)i]=grad;
                }
            }
#else
            {
            char *str=readStringEntry(cfg, "customShades");

            if(str)
            {
                int j,
                    comma=0;

                for(j=0; str[j]; ++j)
                    if(','==str[j])
                        comma++;

                if((NUM_STD_SHADES-1)==comma)
                {
                    bool ok=true;

                    opts->customShades=malloc(sizeof(double)*NUM_STD_SHADES);

                    for(j=0; j<comma+1 && str && ok; ++j)
                    {
                        char *c=strchr(str, ',');

                        if(c || (str && NUM_STD_SHADES-1==comma))
                        {
                            if(c)
                                *c='\0';
                            opts->customShades[j]=g_ascii_strtod(str, NULL);
                            str=c+1;
                        }
                        else
                            ok=false;
                    }

                    if(!ok)
                    {
                        free(opts->customShades);
                        opts->customShades=0L;
                    }
                }
            }
            }

            for(i=0; i<QTC_NUM_CUSTOM_GRAD+1; ++i)
            {
                char gradKey[18];
                char *str;

                if(i==QTC_NUM_CUSTOM_GRAD)
                    sprintf(gradKey, "sunkengradient");
                else
                    sprintf(gradKey, "customgradient%d", i+1);
                if((str=readStringEntry(cfg, gradKey)))
                {
                    int j,
                        comma=0;

                    for(j=0; str[j]; ++j)
                        if(','==str[j])
                            comma++;

                    if(comma && opts->customGradient[i])
                    {
                        if(opts->customGradient[i]->grad)
                            free(opts->customGradient[i]->grad);
                        free(opts->customGradient[i]);
                        opts->customGradient[i]=0L;
                    }

                    if(comma>=4 && 0==comma%2)
                    {
                        char *c=strchr(str, ',');

                        if(c)
                        {
                            bool lb=false,
                                 ok=true;

                            *c='\0';
                            if(0==memcmp(str, "true", 4))
                                lb=true;
                            else if(0==memcmp(str, "false", 4))
                                lb=false;
                            else
                                ok=false;

                            if(ok)
                            {
                                opts->customGradient[i]=malloc(sizeof(CustomGradient));
                                opts->customGradient[i]->numGrad=comma/2;
                                opts->customGradient[i]->grad=malloc(sizeof(Gradient) * opts->customGradient[i]->numGrad);
                                opts->customGradient[i]->lightBorder=lb;

                                str=c+1;
                                for(j=0; j<comma && str && ok; j+=2)
                                {
                                    c=strchr(str, ',');

                                    if(c)
                                    {
                                        *c='\0';
                                        opts->customGradient[i]->grad[j/2].pos=g_ascii_strtod(str, NULL);
                                        str=c+1;
                                        c=str ? strchr(str, ',') : 0L;

                                        if(c || str)
                                        {
                                            if(c)
                                                *c='\0';
                                            opts->customGradient[i]->grad[j/2].val=g_ascii_strtod(str, NULL);
                                            str=c ? c+1 : c;
                                        }
                                        else
                                            ok=false;
                                    }
                                    else
                                        ok=false;
                                }

                                if(!ok)
                                {
                                    free(opts->customGradient[i]->grad);
                                    free(opts->customGradient[i]);
                                    opts->customGradient[i]=0L;
                                }
                            }
                        }
                    }
                }
            }
#endif

            /* **Must** check appearance first, as the rest will default to this */
            checkAppearance(&opts->appearance, opts);
            checkAppearance(&opts->menubarAppearance, opts);
            checkAppearance(&opts->menuitemAppearance, opts);
            checkAppearance(&opts->toolbarAppearance, opts);
            checkAppearance(&opts->lvAppearance, opts);
            checkAppearance(&opts->tabAppearance, opts);
            checkAppearance(&opts->activeTabAppearance, opts);
            checkAppearance(&opts->sliderAppearance, opts);
#ifdef __cplusplus
            checkAppearance(&opts->titlebarAppearance, opts);
            checkAppearance(&opts->inactiveTitlebarAppearance, opts);
            checkAppearance(&opts->titlebarButtonAppearance, opts);
#endif
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
            checkAppearance(&opts->selectionAppearance, opts);
#endif
#if defined __cplusplus || defined QTC_GTK2_MENU_STRIPE
            checkAppearance(&opts->menuStripeAppearance, opts);
#endif
            checkAppearance(&opts->progressAppearance, opts);
            checkAppearance(&opts->progressGrooveAppearance, opts);

#ifndef __cplusplus
            releaseConfig(cfg);
#endif
            if(SHADE_SELECTED==opts->shadeCheckRadio)
                opts->shadeCheckRadio=SHADE_BLEND_SELECTED;

            checkColor(&opts->shadeMenubars, &opts->customMenubarsColor);
            checkColor(&opts->shadeSliders, &opts->customSlidersColor);
            checkColor(&opts->shadeCheckRadio, &opts->customCheckRadioColor);

            if(APPEARANCE_BEVELLED==opts->toolbarAppearance)
                opts->toolbarAppearance=APPEARANCE_GRADIENT;
            else if(APPEARANCE_RAISED==opts->toolbarAppearance)
                opts->toolbarAppearance=APPEARANCE_FLAT;

            if(APPEARANCE_BEVELLED==opts->menubarAppearance)
                opts->menubarAppearance=APPEARANCE_GRADIENT;
            else if(APPEARANCE_RAISED==opts->menubarAppearance)
                opts->menubarAppearance=APPEARANCE_FLAT;

            if(APPEARANCE_BEVELLED==opts->sliderAppearance)
                opts->sliderAppearance=APPEARANCE_GRADIENT;

            if(APPEARANCE_BEVELLED==opts->tabAppearance)
                opts->tabAppearance=APPEARANCE_GRADIENT;

            if(APPEARANCE_BEVELLED==opts->activeTabAppearance)
                opts->activeTabAppearance=APPEARANCE_GRADIENT;

#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
            if(APPEARANCE_RAISED==opts->selectionAppearance)
                opts->selectionAppearance=APPEARANCE_FLAT;
            else if(APPEARANCE_BEVELLED==opts->selectionAppearance)
                opts->selectionAppearance=APPEARANCE_GRADIENT;
#endif
#if defined __cplusplus || defined QTC_GTK2_MENU_STRIPE
            if(APPEARANCE_RAISED==opts->menuStripeAppearance)
                opts->menuStripeAppearance=APPEARANCE_FLAT;
            else if(APPEARANCE_BEVELLED==opts->menuStripeAppearance)
                opts->menuStripeAppearance=APPEARANCE_GRADIENT;
#endif
            if(opts->highlightFactor<MIN_HIGHLIGHT_FACTOR || opts->highlightFactor>MAX_HIGHLIGHT_FACTOR)
                opts->highlightFactor=DEFAULT_HIGHLIGHT_FACTOR;

            if(opts->lighterPopupMenuBgnd>MAX_LIGHTER_POPUP_MENU)
                opts->lighterPopupMenuBgnd=DEF_POPUPMENU_LIGHT_FACTOR;

            if(opts->animatedProgress && !opts->stripedProgress)
                opts->animatedProgress=false;

            if(opts->colorSelTab && APPEARANCE_GRADIENT!=opts->activeTabAppearance &&
                                    APPEARANCE_INVERTED!=opts->activeTabAppearance)
                opts->colorSelTab=false;

/*
??
            if(SHADE_CUSTOM==opts->shadeMenubars || SHADE_BLEND_SELECTED==opts->shadeMenubars || !opts->borderMenuitems)
                opts->colorMenubarMouseOver=true;
*/

            if(MO_GLOW==opts->coloredMouseOver && (EFFECT_NONE==opts->buttonEffect || opts->round<ROUND_FULL))
                opts->coloredMouseOver=MO_COLORED;

            if(IND_GLOW==opts->defBtnIndicator && (EFFECT_NONE==opts->buttonEffect || opts->round<ROUND_FULL))
                opts->defBtnIndicator=IND_TINT;

            if(opts->squareScrollViews || EFFECT_NONE==opts->buttonEffect)
                opts->sunkenScrollViews=false;

            if(opts->squareScrollViews)
                opts->highlightScrollViews=false;

            if(!opts->framelessGroupBoxes)
                opts->groupBoxLine=false;
            return true;
        }
    }

    return false;
}

static bool fileExists(const char *path)
{
    struct stat info;

    return 0==lstat(path, &info) && (info.st_mode&S_IFMT)==S_IFREG;
}

static const char * getSystemConfigFile()
{
    static const char * constFiles[]={ "/etc/qt4/"QTC_FILE, "/etc/qt3/"QTC_FILE, "/etc/qt/"QTC_FILE, "/etc/"QTC_FILE, NULL };

    int i;

    for(i=0; constFiles[i]; ++i)
        if(fileExists(constFiles[i]))
            return constFiles[i];
    return NULL;
}

static void defaultSettings(Options *opts)
{
    /* Set hard-coded defaults... */
#ifndef __cplusplus
    int i;

    for(i=0; i<QTC_NUM_CUSTOM_GRAD+1; ++i)
        opts->customGradient[i]=0L;

    opts->customShades=0L;
#endif

    opts->contrast=7;
    opts->passwordChar=0x25CF;
    opts->highlightFactor=DEFAULT_HIGHLIGHT_FACTOR;
    opts->round=ROUND_FULL;
    opts->lighterPopupMenuBgnd=DEF_POPUPMENU_LIGHT_FACTOR;
    opts->animatedProgress=false;
    opts->stripedProgress=STRIPE_DIAGONAL;
    opts->sliderStyle=SLIDER_TRIANGULAR;
    opts->highlightTab=true;
    opts->colorSelTab=false;
    opts->embolden=false;
    opts->appearance=APPEARANCE_DULL_GLASS;
    opts->lvAppearance=APPEARANCE_BEVELLED;
    opts->tabAppearance=APPEARANCE_GRADIENT;
    opts->activeTabAppearance=APPEARANCE_FLAT;
    opts->sliderAppearance=APPEARANCE_DULL_GLASS;
    opts->menubarAppearance=APPEARANCE_GRADIENT;
    opts->menuitemAppearance=APPEARANCE_DULL_GLASS;
    opts->toolbarAppearance=APPEARANCE_FLAT;
    opts->progressAppearance=APPEARANCE_DULL_GLASS;
    opts->progressGrooveAppearance=APPEARANCE_INVERTED;
    opts->progressGrooveColor=ECOLOR_BASE;
    opts->defBtnIndicator=IND_GLOW;
    opts->sliderThumbs=LINE_FLAT;
    opts->handles=LINE_SUNKEN;
    opts->shadeSliders=SHADE_NONE;
    opts->shadeMenubars=SHADE_DARKEN;
    opts->shadeCheckRadio=SHADE_NONE;
    opts->toolbarBorders=TB_NONE;
    opts->toolbarSeparators=LINE_NONE;
    opts->splitters=LINE_FLAT;
    opts->fixParentlessDialogs=false;
    opts->customMenuTextColor=false;
    opts->coloredMouseOver=MO_GLOW;
    opts->menubarMouseOver=true;
    opts->useHighlightForMenu=true;
    opts->shadeMenubarOnlyWhenActive=false;
    opts->thinnerMenuItems=false;
    opts->scrollbarType=SCROLLBAR_KDE;
    opts->buttonEffect=EFFECT_SHADOW;
    opts->focus=FOCUS_FILLED;
    opts->lvLines=false;
    opts->drawStatusBarFrames=false;
    opts->fillSlider=true;
    opts->roundMbTopOnly=true;
    opts->borderMenuitems=false;
    opts->darkerBorders=false;
    opts->vArrows=true;
    opts->xCheck=false;
    opts->framelessGroupBoxes=true;
    opts->groupBoxLine=true;
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
    opts->fadeLines=true;
#endif
    opts->colorMenubarMouseOver=true;
    opts->inactiveHighlight=false;
    opts->crHighlight=false;
    opts->crButton=false;
    opts->fillProgress=true;
    opts->comboSplitter=false;
    opts->squareScrollViews=false;
    opts->highlightScrollViews=false;
    opts->sunkenScrollViews=false;
    opts->flatSbarButtons=true;
#if defined __cplusplus || defined QTC_GTK2_MENU_STRIPE
    opts->menuStripe=false;
    opts->menuStripeAppearance=APPEARANCE_GRADIENT;
#endif
#ifdef QTC_CONFIG_DIALOG
    opts->shading=SHADING_HSL;
#endif

#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
    opts->selectionAppearance=APPEARANCE_GRADIENT;
#endif

#ifdef __cplusplus
    opts->stdSidebarButtons=false;
    opts->gtkScrollViews=false;
    opts->gtkComboMenus=false;
    opts->colorTitlebarOnly=false;
    opts->customMenubarsColor.setRgb(0, 0, 0);
    opts->customSlidersColor.setRgb(0, 0, 0);
    opts->customMenuNormTextColor.setRgb(0, 0, 0);
    opts->customMenuSelTextColor.setRgb(0, 0, 0);
    opts->customCheckRadioColor.setRgb(0, 0, 0);
#else
/*
    opts->setDialogButtonOrder=false;
*/
    opts->customMenubarsColor.red=opts->customMenubarsColor.green=opts->customMenubarsColor.blue=0;
    opts->customSlidersColor.red=opts->customSlidersColor.green=opts->customSlidersColor.blue=0;
    opts->customMenuNormTextColor.red=opts->customMenuNormTextColor.green=opts->customMenuNormTextColor.blue=0;
    opts->customMenuSelTextColor.red=opts->customMenuSelTextColor.green=opts->customMenuSelTextColor.blue=0;
    opts->customCheckRadioColor.red=opts->customCheckRadioColor.green=opts->customCheckRadioColor.blue=0;
#endif

#if !defined __cplusplus || defined QTC_CONFIG_DIALOG
    opts->mapKdeIcons=true;
#endif
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
    opts->gtkButtonOrder=false;
#endif
#ifndef __cplusplus
#endif
#ifdef __cplusplus
    opts->titlebarAppearance=APPEARANCE_GRADIENT;
    opts->inactiveTitlebarAppearance=APPEARANCE_GRADIENT;
    opts->titlebarButtonAppearance=APPEARANCE_GRADIENT;
#endif
    /* Read system config file... */
    {
    static const char * systemFilename=NULL;

    if(!systemFilename)
        systemFilename=getSystemConfigFile();

    if(systemFilename)
        readConfig(systemFilename, opts, opts);
    }

#if !defined QTC_CONFIG_DIALOG && defined QT_VERSION && (QT_VERSION < 0x040000)
    if(FOCUS_FILLED==opts->focus)
        opts->focus=FOCUS_HIGHLIGHT;
#endif
}

#endif

#ifdef CONFIG_WRITE
static const char *toStr(EDefBtnIndicator ind)
{
    switch(ind)
    {
        case IND_NONE:
            return "none";
        case IND_FONT_COLOR:
            return "fontcolor";
        case IND_CORNER:
            return "corner";
        case IND_TINT:
            return "tint";
        case IND_GLOW:
            return "glow";
        default:
            return "colored";
    }
}

static const char *toStr(ELine ind, bool dashes)
{
    switch(ind)
    {
        case LINE_DOTS:
            return "dots";
        case LINE_DASHES:
            return dashes ? "dashes" : "none";
        case LINE_NONE:
            return "none";
        case LINE_FLAT:
            return "flat";
        default:
            return "sunken";
    }
}

static const char *toStr(ETBarBorder ind)
{
    switch(ind)
    {
        case TB_DARK:
            return "dark";
        case TB_DARK_ALL:
            return "dark-all";
        case TB_LIGHT_ALL:
            return "light-all";
        case TB_NONE:
            return "none";
        default:
            return "light";
    }
}

static const char *toStr(EMouseOver mo)
{
    switch(mo)
    {
        case MO_COLORED:
            return "colored";
        case MO_NONE:
            return "none";
        case MO_GLOW:
            return "glow";
        default:
            return "plastik";
    }
}

static QString toStr(EAppearance exp)
{
    switch(exp)
    {
        case APPEARANCE_FLAT:
            return "flat";
        case APPEARANCE_RAISED:
            return "raised";
        case APPEARANCE_GRADIENT:
            return "gradient";
        case APPEARANCE_SPLIT_GRADIENT:
            return "splitgradient";
        case APPEARANCE_DULL_GLASS:
            return "dullglass";
        case APPEARANCE_BEVELLED:
            return "bevelled";
        case APPEARANCE_INVERTED:
            return "inverted";
        case APPEARANCE_SHINY_GLASS:
            return "shinyglass";
        case APPEARANCE_FADE:
            return "fade";
        default:
        {
            QString app;

            app.sprintf("customgradient%d", (exp-APPEARANCE_CUSTOM1)+1);
            return app;
        }
    }
}

static const char *toStr(EShade exp, bool dark, bool convertBlendSelToSel)
{
    switch(exp)
    {
        default:
        case SHADE_NONE:
            return "none";
        case SHADE_BLEND_SELECTED:
            return dark || !convertBlendSelToSel ? "selected" : "origselected";
        case SHADE_CUSTOM:
            return "custom";
        /* case SHADE_SELECTED */
        case SHADE_DARKEN:
            return dark ? "darken" : "origselected";
    }
}

static const char *toStr(ERound exp)
{
    switch(exp)
    {
        case ROUND_NONE:
            return "none";
        case ROUND_SLIGHT:
            return "slight";
        case ROUND_EXTRA:
            return "extra";
        default:
        case ROUND_FULL:
            return "full";
    }
}

static const char *toStr(EScrollbar sb)
{
    switch(sb)
    {
        case SCROLLBAR_KDE:
            return "kde";
        default:
        case SCROLLBAR_WINDOWS:
            return "windows";
        case SCROLLBAR_PLATINUM:
            return "platinum";
        case SCROLLBAR_NEXT:
            return "next";
        case SCROLLBAR_NONE:
            return "none";
    }
}

static const char *toStr(EEffect e)
{
    switch(e)
    {
        case EFFECT_NONE:
            return "none";
        default:
        case EFFECT_SHADOW:
            return "shadow";
        case EFFECT_ETCH:
            return "etch";
    }
}

inline const char * toStr(bool b) { return b ? "true" : "false"; }

static QString toStr(const QColor &col)
{
    QString colorStr;

    colorStr.sprintf("#%02X%02X%02X", col.red(), col.green(), col.blue());
    return colorStr;
}

static const char *toStr(EShading s)
{
    switch(s)
    {
        case SHADING_SIMPLE:
            return "simple";
        default:
        case SHADING_HSL:
            return "hsl";
        case SHADING_HSV:
            return "hsv";
    }
}

static const char *toStr(EStripe s)
{
    switch(s)
    {
        default:
        case STRIPE_PLAIN:
            return "plain";
        case STRIPE_NONE:
            return "none";
        case STRIPE_DIAGONAL:
            return "diagonal";
    }
}

static const char *toStr(ESliderStyle s)
{
    switch(s)
    {
        case SLIDER_PLAIN:
            return "plain";
        case SLIDER_TRIANGULAR:
            return "triangular";
        case SLIDER_ROUND_ROTATED:
            return "r-round";
        case SLIDER_PLAIN_ROTATED:
            return "r-plain";
        default:
        case SLIDER_ROUND:
            return "round";
    }
}

static const char *toStr(EColor s)
{
    switch(s)
    {
        case ECOLOR_BACKGROUND:
            return "background";
        case ECOLOR_DARK:
            return "dark";
        default:
        case ECOLOR_BASE:
            return "base";
    }
}

static const char *toStr(EFocus f)
{
    switch(f)
    {
        default:
        case FOCUS_STANDARD:
            return "standard";
        case FOCUS_HIGHLIGHT:
            return "highlight";
        case FOCUS_BACKGROUND:
            return "background";
        case FOCUS_FILLED:
            return "filled";
    }
}

#if QT_VERSION >= 0x040000
#include <QTextStream>
#define CFG config
#else
#define CFG (*cfg)
#endif

#define CFG_WRITE_ENTRY(ENTRY) \
    if (!exportingStyle && def.ENTRY==opts.ENTRY) \
        CFG.deleteEntry(#ENTRY); \
    else \
        CFG.writeEntry(#ENTRY, toStr(opts.ENTRY));

#define CFG_WRITE_ENTRY_FORCE(ENTRY) \
        CFG.writeEntry(#ENTRY, toStr(opts.ENTRY));

#define CFG_WRITE_ENTRY_B(ENTRY, B) \
    if (!exportingStyle && def.ENTRY==opts.ENTRY) \
        CFG.deleteEntry(#ENTRY); \
    else \
        CFG.writeEntry(#ENTRY, toStr(opts.ENTRY, B));

#define CFG_WRITE_ENTRY_SHADE(ENTRY, DARK, CONVERT_SHADE) \
    if (!exportingStyle && def.ENTRY==opts.ENTRY) \
        CFG.deleteEntry(#ENTRY); \
    else \
        CFG.writeEntry(#ENTRY, toStr(opts.ENTRY, DARK, CONVERT_SHADE));

#define CFG_WRITE_ENTRY_NUM(ENTRY) \
    if (!exportingStyle && def.ENTRY==opts.ENTRY) \
        CFG.deleteEntry(#ENTRY); \
    else \
        CFG.writeEntry(#ENTRY, opts.ENTRY);

bool static writeConfig(KConfig *cfg, const Options &opts, const Options &def, bool exportingStyle=false)
{
    if(!cfg)
    {
        const char *xdg=xdgConfigFolder();

        if(xdg)
        {
            char filename[QTC_MAX_FILENAME_LEN];

            sprintf(filename, "%s/"QTC_FILE, xdg);

#if QT_VERSION >= 0x040000
            KConfig defCfg(filename, KConfig::SimpleConfig);
#else
            KConfig defCfg(filename, false, false);
#endif

            return writeConfig(&defCfg, opts, def, exportingStyle);
        }
    }
    else
    {
#if QT_VERSION >= 0x040000
        KConfigGroup config(cfg, QTC_GROUP);
#else
        cfg->setGroup(QTC_GROUP);
#endif
        CFG.writeEntry(QTC_VERSION_KEY, VERSION);
        CFG_WRITE_ENTRY_NUM(passwordChar)
        CFG_WRITE_ENTRY(round)
        CFG_WRITE_ENTRY_NUM(highlightFactor)
        CFG_WRITE_ENTRY(toolbarBorders)
        CFG_WRITE_ENTRY_FORCE(appearance)
        CFG_WRITE_ENTRY(fixParentlessDialogs)
        CFG_WRITE_ENTRY(stripedProgress)
        CFG_WRITE_ENTRY(sliderStyle)
        CFG_WRITE_ENTRY(animatedProgress)
        CFG_WRITE_ENTRY_NUM(lighterPopupMenuBgnd)
        CFG_WRITE_ENTRY(embolden)
        CFG_WRITE_ENTRY(defBtnIndicator)
        CFG_WRITE_ENTRY_B(sliderThumbs, false)
        CFG_WRITE_ENTRY_B(handles, true)
        CFG_WRITE_ENTRY(highlightTab)
        CFG_WRITE_ENTRY(colorSelTab)
        CFG_WRITE_ENTRY_SHADE(shadeSliders, false, false)
        CFG_WRITE_ENTRY_SHADE(shadeMenubars, true, false)
        CFG_WRITE_ENTRY_SHADE(shadeCheckRadio, false, true)
        CFG_WRITE_ENTRY_FORCE(menubarAppearance)
        CFG_WRITE_ENTRY_FORCE(menuitemAppearance)
        CFG_WRITE_ENTRY_FORCE(toolbarAppearance)
        CFG_WRITE_ENTRY_FORCE(selectionAppearance)
        CFG_WRITE_ENTRY_FORCE(menuStripeAppearance)
        CFG_WRITE_ENTRY_B(toolbarSeparators, false)
        CFG_WRITE_ENTRY_B(splitters, true)
        CFG_WRITE_ENTRY(customMenuTextColor)
        CFG_WRITE_ENTRY(coloredMouseOver)
        CFG_WRITE_ENTRY(menubarMouseOver)
        CFG_WRITE_ENTRY(useHighlightForMenu)
        CFG_WRITE_ENTRY(shadeMenubarOnlyWhenActive)
        CFG_WRITE_ENTRY(thinnerMenuItems)
        CFG_WRITE_ENTRY(customSlidersColor)
        CFG_WRITE_ENTRY(customMenubarsColor)
        CFG_WRITE_ENTRY(customMenuSelTextColor)
        CFG_WRITE_ENTRY(customMenuNormTextColor)
        CFG_WRITE_ENTRY(customCheckRadioColor)
        CFG_WRITE_ENTRY(scrollbarType)
        CFG_WRITE_ENTRY(buttonEffect)
        CFG_WRITE_ENTRY_FORCE(lvAppearance)
        CFG_WRITE_ENTRY_FORCE(tabAppearance)
        CFG_WRITE_ENTRY_FORCE(activeTabAppearance)
        CFG_WRITE_ENTRY_FORCE(sliderAppearance)
        CFG_WRITE_ENTRY_FORCE(progressAppearance)
        CFG_WRITE_ENTRY_FORCE(progressGrooveAppearance)
        CFG_WRITE_ENTRY(progressGrooveColor)
        CFG_WRITE_ENTRY(focus)
        CFG_WRITE_ENTRY(lvLines)
        CFG_WRITE_ENTRY(drawStatusBarFrames)
        CFG_WRITE_ENTRY(fillSlider)
        CFG_WRITE_ENTRY(roundMbTopOnly)
        CFG_WRITE_ENTRY(borderMenuitems)
        CFG_WRITE_ENTRY(darkerBorders)
        CFG_WRITE_ENTRY(vArrows)
        CFG_WRITE_ENTRY(xCheck)
        CFG_WRITE_ENTRY(framelessGroupBoxes)
        CFG_WRITE_ENTRY(groupBoxLine)
        CFG_WRITE_ENTRY(fadeLines)
        CFG_WRITE_ENTRY(inactiveHighlight)
        CFG_WRITE_ENTRY(crHighlight)
        CFG_WRITE_ENTRY(crButton)
        CFG_WRITE_ENTRY(fillProgress)
        CFG_WRITE_ENTRY(comboSplitter)
        CFG_WRITE_ENTRY(squareScrollViews)
        CFG_WRITE_ENTRY(highlightScrollViews)
        CFG_WRITE_ENTRY(sunkenScrollViews)
        CFG_WRITE_ENTRY(flatSbarButtons)
        CFG_WRITE_ENTRY(menuStripe)
        CFG_WRITE_ENTRY(stdSidebarButtons)
        CFG_WRITE_ENTRY_FORCE(titlebarAppearance)
        CFG_WRITE_ENTRY_FORCE(inactiveTitlebarAppearance)
        CFG_WRITE_ENTRY_FORCE(titlebarButtonAppearance)

        CFG_WRITE_ENTRY(gtkScrollViews)
        CFG_WRITE_ENTRY(gtkComboMenus)
        CFG_WRITE_ENTRY(colorTitlebarOnly)
        CFG_WRITE_ENTRY(gtkButtonOrder)
        CFG_WRITE_ENTRY(mapKdeIcons)
        CFG_WRITE_ENTRY(shading)

        for(int i=APPEARANCE_CUSTOM1; i<(APPEARANCE_CUSTOM1+QTC_NUM_CUSTOM_GRAD+1); ++i)
        {
            CustomGradientCont::const_iterator cg(opts.customGradient.find((EAppearance)i));
            QString                            gradKey;

            if(i==(APPEARANCE_CUSTOM1+QTC_NUM_CUSTOM_GRAD))
                gradKey="sunkengradient";
            else
                gradKey.sprintf("customgradient%d", (i-APPEARANCE_CUSTOM1)+1);

            if(cg==opts.customGradient.end())
                CFG.deleteEntry(gradKey);
            else
            {
                QString     gradVal;
#if QT_VERSION >= 0x040000
                QTextStream str(&gradVal);
#else
                QTextStream str(&gradVal, IO_WriteOnly);
#endif

                str << (const char *)((*cg).second.lightBorder ? "true" : "false");

                GradientCont::const_iterator it((*cg).second.grad.begin()),
                                             end((*cg).second.grad.end());

                for(; it!=end; ++it)
                    str << ',' << (*it).pos << ',' << (*it).val;
                CFG.writeEntry(gradKey, gradVal);
            }
        }

        if(NUM_STD_SHADES==opts.customShades.size())
        {
            QString     shadeVal;
#if QT_VERSION >= 0x040000
            QTextStream str(&shadeVal);
#else
            QTextStream str(&shadeVal, IO_WriteOnly);
#endif

            ShadesCont::const_iterator it(opts.customShades.begin()),
                                       end(opts.customShades.end());

            for(int i=0; it!=end; ++it, ++i)
                if(0==i)
                    str << *it;
                else
                    str << ',' << *it;
            CFG.writeEntry("customShades", shadeVal);
        }
        else
            CFG.deleteEntry("customShades");

        cfg->sync();
        return true;
    }
    return false;
}
#endif
