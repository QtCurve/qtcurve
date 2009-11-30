/*
  QtCurve (C) Craig Drummond, 2003 - 2009 ee11cd@googlemail.com

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
#define QTC_FILE               "stylerc"
#define QTC_OLD_FILE           "qtcurvestylerc"
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
        if(0==memcmp(str, "darken", 6))
            return IND_DARKEN;
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
        if(0==memcmp(str, "1dot", 5))
            return LINE_1DOT;
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
        if(0==memcmp(str, "thickcolored", 12))
            return MO_COLORED_THICK;
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
        if(0==memcmp(str, "dullglass", 9))
            return APPEARANCE_DULL_GLASS;
        if(0==memcmp(str, "glass", 5) || 0==memcmp(str, "shinyglass", 10))
            return APPEARANCE_SHINY_GLASS;
        if(0==memcmp(str, "agua", 4))
#if defined __cplusplus && !defined QTC_CONFIG_DIALOG  && defined QT_VERSION && QT_VERSION < 0x040000
            return APPEARANCE_AGUA_MOD;
#else
            return APPEARANCE_AGUA;
#endif
        if(0==memcmp(str, "soft", 4))
            return APPEARANCE_SOFT_GRADIENT;
        if(0==memcmp(str, "gradient", 8) || 0==memcmp(str, "lightgradient", 13))
            return APPEARANCE_GRADIENT;
        if(0==memcmp(str, "harsh", 5))
            return APPEARANCE_HARSH_GRADIENT;
        if(0==memcmp(str, "inverted", 8))
            return APPEARANCE_INVERTED;
        if(0==memcmp(str, "darkinverted", 12))
            return APPEARANCE_DARK_INVERTED;
        if(0==memcmp(str, "splitgradient", 13))
            return APPEARANCE_SPLIT_GRADIENT;
        if(0==memcmp(str, "bevelled", 8))
            return APPEARANCE_BEVELLED;
        if(allowFade && 0==memcmp(str, "fade", 4))
            return APPEARANCE_FADE;

        if(0==memcmp(str, "customgradient", 14) && strlen(str)>14)
        {
            int i=atoi(&str[14]);

            i--;
            if(i>=0 && i<QTC_NUM_CUSTOM_GRAD)
                return (EAppearance)(APPEARANCE_CUSTOM1+i);
        }
    }
    return def;
}

static EShade toShade(const char *str, bool allowDarken, EShade def, bool menuShade, color *col)
{
    if(str)
    {
        /* true/false is from 0.25... */
        if((!menuShade && 0==memcmp(str, "true", 4)) || 0==memcmp(str, "selected", 8))
            return SHADE_BLEND_SELECTED;
        if(0==memcmp(str, "origselected", 12))
            return SHADE_SELECTED;
        if(allowDarken && (0==memcmp(str, "darken", 6) || (menuShade && 0==memcmp(str, "true", 4))))
            return SHADE_DARKEN;
        if(0==memcmp(str, "custom", 6))
            return SHADE_CUSTOM;
        if('#'==str[0] && col)
        {
            setRgb(col, str);
            return SHADE_CUSTOM;
        }
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
        if(0==memcmp(str, "max", 3))
            return ROUND_MAX;
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
        if(0==memcmp(str, "hcy", 3))
            return SHADING_HCY;
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
        if(0==memcmp(str, "rect", 4) || 0==memcmp(str, "highlight", 9))
            return FOCUS_RECTANGLE;
        if(0==memcmp(str, "filled", 6))
            return FOCUS_FILLED;
        if(0==memcmp(str, "full", 4))
            return FOCUS_FULL;
        if(0==memcmp(str, "line", 4))
            return FOCUS_LINE;
    }

    return def;
}

static ETabMo toTabMo(const char *str, ETabMo def)
{
    if(str)
    {
        if(0==memcmp(str, "top", 3))
            return TAB_MO_TOP;
        if(0==memcmp(str, "bot", 3))
            return TAB_MO_BOTTOM;
        if(0==memcmp(str, "glow", 4))
            return TAB_MO_GLOW;
    }

    return def;
}

static EGradType toGradType(const char *str, EGradType def)
{
    if(str)
    {
        if(0==memcmp(str, "horiz", 5))
            return GT_HORIZ;
        if(0==memcmp(str, "vert", 4))
            return GT_VERT;
    }
    return def;
}

static ELvLines toLvLines(const char *str, ELvLines def)
{
    if(str)
    {
        if(0==memcmp(str, "true", 4) || 0==memcmp(str, "new", 3))
            return LV_NEW;
        if(0==memcmp(str, "old", 3))
            return LV_OLD;
        if(0==memcmp(str, "false", 5) || 0==memcmp(str, "none", 4))
            return LV_NONE;
    }
    return def;
}

static EGradientBorder toGradientBorder(const char *str)
{
    if(str)
    {
        if(0==memcmp(str, "light", 5) || 0==memcmp(str, "true", 4))
            return GB_LIGHT;
        if(0==memcmp(str, "none", 4))
            return GB_NONE;
        if(0==memcmp(str, "3dfull", 6))
            return GB_3D_FULL;
        if(0==memcmp(str, "3d", 2) || 0==memcmp(str, "false", 5))
            return GB_3D;

    }
    return GB_3D;
}

#ifdef __cplusplus
static EAlign toAlign(const char *str, EAlign def)
{
    if(str)
    {
        if(0==memcmp(str, "left", 4))
            return ALIGN_LEFT;
        if(0==memcmp(str, "center-full", 11))
            return ALIGN_FULL_CENTER;
        if(0==memcmp(str, "center", 6))
            return ALIGN_CENTER;
        if(0==memcmp(str, "right", 5))
            return ALIGN_RIGHT;
    }
    return def;
}
#endif

#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000))
static ETitleBarIcon toTitlebarIcon(const char *str, ETitleBarIcon def)
{
    if(str)
    {
        if(0==memcmp(str, "none", 4))
            return TITLEBAR_ICON_NONE;
        if(0==memcmp(str, "menu", 4))
            return TITLEBAR_ICON_MENU_BUTTON;
        if(0==memcmp(str, "title", 5))
            return TITLEBAR_ICON_NEXT_TO_TITLE;
    }
    return def;
}
#endif

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

#ifdef __cplusplus

#ifdef QTC_QT_ONLY
#if QT_VERSION < 0x040000
#include <qdir.h>
#include <qfile.h>
#endif
// Take from KStandardDirs::makeDir
bool makeDir(const QString& dir, int mode)
{
    // we want an absolute path
    if (QDir::isRelativePath(dir))
        return false;

#ifdef Q_WS_WIN
    return QDir().mkpath(dir);
#else
    QString target = dir;
    uint len = target.length();

    // append trailing slash if missing
    if (dir.at(len - 1) != '/')
        target += '/';

    QString base;
    uint i = 1;

    while( i < len )
    {
        struct stat st;
#if QT_VERSION >= 0x040000
        int pos = target.indexOf('/', i);
#else
        int pos = target.find('/', i);
#endif
        base += target.mid(i - 1, pos - i + 1);
        QByteArray baseEncoded = QFile::encodeName(base);
        // bail out if we encountered a problem
        if (stat(baseEncoded, &st) != 0)
        {
            // Directory does not exist....
            // Or maybe a dangling symlink ?
            if (lstat(baseEncoded, &st) == 0)
                (void)unlink(baseEncoded); // try removing

            if (mkdir(baseEncoded, static_cast<mode_t>(mode)) != 0) {
#if QT_VERSION >= 0x040000
                baseEncoded.prepend( "trying to create local folder " );
                perror(baseEncoded.constData());
#else
                perror("trying to create QtCurve config folder ");
#endif
                return false; // Couldn't create it :-(
            }
        }
        i = pos + 1;
    }
    return true;
#endif
}

#else
#include <kstandarddirs.h>
#endif
#endif

static const char *qtcConfDir()
{
    static char *cfgDir=NULL;

    if(!cfgDir)
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

            cfgDir=(char *)malloc(strlen(home)+18);
            sprintf(cfgDir, "%s/.config/qtcurve/", home);
        }
        else
        {
            cfgDir=(char *)malloc(strlen(env)+10);
            sprintf(cfgDir, "%s/qtcurve/", env);
        }

//#if defined CONFIG_WRITE || !defined __cplusplus
        {
        struct stat info;

        if(0!=lstat(cfgDir, &info))
        {
#ifdef __cplusplus
#ifdef QTC_QT_ONLY
            makeDir(cfgDir, 0755);
#else
            KStandardDirs::makeDir(cfgDir, 0755);
#endif
#else
            g_mkdir_with_parents(cfgDir, 0755);
#endif
        }
        }
//#endif
    }

    return cfgDir;
}

#if (!defined QT_VERSION || QT_VERSION >= 0x040000) && !defined QTC_CONFIG_DIALOG

#define QTC_MENU_FILE_PREFIX "menubar-"

#ifdef __cplusplus
static bool qtcMenuBarHidden(const QString &app)
{
    return QFile::exists(QFile::decodeName(qtcConfDir())+QTC_MENU_FILE_PREFIX+app);
}

static void qtcSetMenuBarHidden(const QString &app, bool hidden)
{
    if(!hidden)
        QFile::remove(QFile::decodeName(qtcConfDir())+QTC_MENU_FILE_PREFIX+app);
    else
        QFile(QFile::decodeName(qtcConfDir())+QTC_MENU_FILE_PREFIX+app).open(QIODevice::WriteOnly);
}
#else
static bool qtcFileExists(const char *name)
{
    struct stat info;

    return 0==lstat(name, &info) && S_ISREG(info.st_mode);
}

static char * qtcGetMenuBarFileName(const char *app)
{
    char *filename=NULL;

    if(!filename)
    {
        filename=(char *)malloc(strlen(qtcConfDir())+strlen(QTC_MENU_FILE_PREFIX)+strlen(app)+1);
        sprintf(filename, "%s"QTC_MENU_FILE_PREFIX"%s", qtcConfDir(), app);
    }

    return filename;
}

static bool qtcMenuBarHidden(const char *app)
{
    return qtcFileExists(qtcGetMenuBarFileName(app));
}

static void qtcSetMenuBarHidden(const char *app, bool hidden)
{
    if(!hidden)
        unlink(qtcGetMenuBarFileName(app));
    else
    {
        FILE *f=fopen(qtcGetMenuBarFileName(app), "w");

        if(f)
            fclose(f);
    }
}
#endif
#endif

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
#define QTC_LATIN1(A) A.toLatin1().constData()
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
            opts->ENTRY=def->ENTRY; \
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
            opts->ENTRY=def->ENTRY; \
    }
#endif

#define QTC_CFG_READ_NUM(ENTRY) \
    opts->ENTRY=readNumEntry(cfg, #ENTRY, def->ENTRY);

#define QTC_CFG_READ_BOOL(ENTRY) \
    opts->ENTRY=readBoolEntry(cfg, #ENTRY, def->ENTRY);

#define QTC_CFG_READ_ROUND(ENTRY) \
    opts->ENTRY=toRound(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_INT(ENTRY) \
    opts->ENTRY=readNumEntry(cfg, #ENTRY, def->ENTRY);

#define QTC_CFG_READ_INT_BOOL(ENTRY, DEF) \
    if(readBoolEntry(cfg, #ENTRY, false)) \
        opts->ENTRY=DEF; \
    else \
        opts->ENTRY=readNumEntry(cfg, #ENTRY, def->ENTRY);
    
#define QTC_CFG_READ_TB_BORDER(ENTRY) \
    opts->ENTRY=toTBarBorder(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_MOUSE_OVER(ENTRY) \
    opts->ENTRY=toMouseOver(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_APPEARANCE(ENTRY, ALLOW_FADE) \
    opts->ENTRY=toAppearance(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY, ALLOW_FADE);

/*
#define QTC_CFG_READ_APPEARANCE(ENTRY) \
    opts->ENTRY=toAppearance(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);
*/

#define QTC_CFG_READ_STRIPE(ENTRY) \
    opts->ENTRY=toStripe(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_SLIDER(ENTRY) \
    opts->ENTRY=toSlider(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_DEF_BTN(ENTRY) \
    opts->ENTRY=toInd(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_LINE(ENTRY) \
    opts->ENTRY=toLine(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_SHADE(ENTRY, AD, MENU_STRIPE, COL) \
    opts->ENTRY=toShade(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), AD, def->ENTRY, MENU_STRIPE, COL);

#define QTC_CFG_READ_SCROLLBAR(ENTRY) \
    opts->ENTRY=toScrollbar(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_EFFECT(ENTRY) \
    opts->ENTRY=toEffect(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_SHADING(ENTRY) \
    opts->ENTRY=toShading(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_ECOLOR(ENTRY) \
    opts->ENTRY=toEColor(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_FOCUS(ENTRY) \
    opts->ENTRY=toFocus(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_TAB_MO(ENTRY) \
    opts->ENTRY=toTabMo(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_GRAD_TYPE(ENTRY) \
    opts->ENTRY=toGradType(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_LV_LINES(ENTRY) \
    opts->ENTRY=toLvLines(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#ifdef __cplusplus
#define QTC_CFG_READ_ALIGN(ENTRY) \
    opts->ENTRY=toAlign(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);
#endif

#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000))
#define QTC_CFG_READ_TB_ICON(ENTRY) \
    opts->ENTRY=toTitlebarIcon(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);
#endif
    
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

#ifndef __cplusplus
static void copyGradients(Options *src, Options *dest)
{
    if(src && dest && src!=dest)
    {
        int i;

        for(i=0; i<QTC_NUM_CUSTOM_GRAD; ++i)
            if(src->customGradient[i] && src->customGradient[i]->numStops>0)
            {
                dest->customGradient[i]=malloc(sizeof(Gradient));
                dest->customGradient[i]->numStops=src->customGradient[i]->numStops;
                dest->customGradient[i]->stops=malloc(sizeof(GradientStop) * dest->customGradient[i]->numStops*2);
                memcpy(dest->customGradient[i]->stops, src->customGradient[i]->stops,
                        sizeof(GradientStop) * dest->customGradient[i]->numStops*2);
                dest->customGradient[i]->border=src->customGradient[i]->border;
            }
            else
                dest->customGradient[i]=NULL;
    }
}

static void copyOpts(Options *src, Options *dest)
{
    if(src && dest && src!=dest)
    {
        memcpy(dest, src, sizeof(Options));
        memcpy(dest->customShades, src->customShades, sizeof(double)*NUM_STD_SHADES);
        copyGradients(src, dest);
    }
}
#endif

#ifdef __cplusplus
static bool readConfig(const QString &file, Options *opts, Options *defOpts=0L)
#else
static bool readConfig(const char *file, Options *opts, Options *defOpts)
#endif
{
#ifdef __cplusplus
    if(file.isEmpty())
    {
        const char *cfgDir=qtcConfDir();

        if(cfgDir)
        {
            QString filename(QFile::decodeName(cfgDir)+QTC_FILE);

            if(!QFile::exists(filename))
                filename=QFile::decodeName(cfgDir)+"../"QTC_OLD_FILE;
            return readConfig(filename, opts, defOpts);
        }
    }
#else
    if(!file)
    {
        const char *cfgDir=qtcConfDir();

        if(cfgDir)
        {
            char *filename=(char *)malloc(strlen(cfgDir)+strlen(QTC_OLD_FILE)+4);
            bool rv=false;

            sprintf(filename, "%s"QTC_FILE, cfgDir);
            if(!qtcFileExists(filename))
                sprintf(filename, "%s../"QTC_OLD_FILE, cfgDir);
            rv=readConfig(filename, opts, defOpts);
            free(filename);
            return rv;
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
#ifdef __cplusplus
            Options newOpts;

            if(defOpts)
                newOpts=*defOpts;
            else
                defaultSettings(&newOpts);

            Options *def=&newOpts;

            if(opts!=def)
                opts->customGradient=def->customGradient;

#if !defined QTC_CONFIG_DIALOG && defined QT_VERSION && (QT_VERSION >= 0x040000)
            if(opts!=def)
            {
                opts->menubarApps=QSet<QString>::fromList(readStringEntry(cfg, "menubarApps").split(','));
                opts->menubarApps << "kcalc" << "amarok" << "vlc" << "smplayer" << "arora";
            }
#endif

#else
            Options newOpts;
            Options *def=&newOpts;

            if(defOpts)
                copyOpts(defOpts, &newOpts);
            else
                defaultSettings(&newOpts);
            if(opts!=def)
                copyGradients(def, opts);
#endif
            /* Check if the config file expects old default values... */
            if(version<QTC_MAKE_VERSION(1, 0))
            {
                def->roundAllTabs=false;
                def->smallRadio=false;
                def->splitters=LINE_FLAT;
                def->handles=LINE_SUNKEN;
                def->crHighlight=0;
#ifdef __cplusplus
                def->dwtAppearance=APPEARANCE_FLAT;
#if defined QT_VERSION && (QT_VERSION >= 0x040000)
                def->dwtSettings=0;
#endif
                def->inactiveTitlebarAppearance=APPEARANCE_CUSTOM2;
#endif
            }
            if(version<QTC_MAKE_VERSION(0, 67))
                def->doubleGtkComboArrow=false;
            if(version<QTC_MAKE_VERSION(0, 66))
            {
                def->menuStripeAppearance=APPEARANCE_GRADIENT;
                def->etchEntry=true;
                def->gtkScrollViews=false;
                def->thinSbarGroove=false;
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000))
                def->titlebarButtons=QTC_TITLEBAR_BUTTON_HOVER_FRAME;
                def->titlebarIcon=TITLEBAR_ICON_MENU_BUTTON;
#endif
            }
            if(version<QTC_MAKE_VERSION(0, 65))
            {
                def->tabMouseOver=TAB_MO_BOTTOM;
                def->activeTabAppearance=APPEARANCE_FLAT;
                def->unifySpin=false;
                def->unifyCombo=false;
                def->borderTab=false;
                def->thinnerBtns=false;
            }
            if(version<QTC_MAKE_VERSION(0, 63))
            {
                def->tabMouseOver=TAB_MO_TOP;
                def->sliderStyle=SLIDER_TRIANGULAR;
#ifdef __cplusplus
                def->titlebarAlignment=ALIGN_LEFT;
#endif
            }
            if(version<QTC_MAKE_VERSION(0, 62))
            {
#ifdef __cplusplus
                def->titlebarAppearance=APPEARANCE_GRADIENT;
                def->inactiveTitlebarAppearance=APPEARANCE_GRADIENT;
#endif
                def->round=ROUND_FULL;
                def->appearance=APPEARANCE_DULL_GLASS;
                def->sliderAppearance=APPEARANCE_DULL_GLASS;
                def->menuitemAppearance=APPEARANCE_DULL_GLASS;
                def->useHighlightForMenu=true;
                def->tabAppearance=APPEARANCE_GRADIENT;
                def->highlightFactor=5;
                def->toolbarSeparators=LINE_NONE;
                def->menubarAppearance=APPEARANCE_SOFT_GRADIENT;
                def->crButton=false;
                def->customShades[0]=0;
                def->stripedProgress=STRIPE_DIAGONAL;
                def->sunkenAppearance=APPEARANCE_INVERTED;
                def->focus=FOCUS_FILLED;
            }
            if(version<QTC_MAKE_VERSION(0, 61))
            {
                def->coloredMouseOver=MO_PLASTIK;
                def->buttonEffect=EFFECT_NONE;
                def->defBtnIndicator=IND_TINT;
                def->vArrows=false;
                def->toolbarAppearance=APPEARANCE_GRADIENT;
                def->focus=FOCUS_STANDARD;
                def->selectionAppearance=APPEARANCE_FLAT;
                def->flatSbarButtons=false;
                def->comboSplitter=true;
                def->handles=LINE_DOTS;
                def->lighterPopupMenuBgnd=15;
                def->activeTabAppearance=APPEARANCE_GRADIENT;
                def->groupBoxLine=false;
                def->shadeSliders=SHADE_BLEND_SELECTED;
                def->progressGrooveColor=ECOLOR_BASE;
                def->shadeMenubars=SHADE_DARKEN;
                opts->highlightTab=true;
            }

            if(opts!=def)
            {
                opts->customShades[0]=0;
                if(QTC_USE_CUSTOM_SHADES(*def))
                    memcpy(opts->customShades, def->customShades, sizeof(double)*NUM_STD_SHADES);
            }

            QTC_CFG_READ_NUM(passwordChar)
            QTC_CFG_READ_ROUND(round)
            QTC_CFG_READ_INT(highlightFactor)
            QTC_CFG_READ_INT(menuDelay)
            QTC_CFG_READ_INT(sliderWidth)
            QTC_CFG_READ_INT_BOOL(lighterPopupMenuBgnd, def->lighterPopupMenuBgnd)
            QTC_CFG_READ_INT(tabBgnd)
            QTC_CFG_READ_TB_BORDER(toolbarBorders)
            QTC_CFG_READ_APPEARANCE(appearance, false)
            QTC_CFG_READ_APPEARANCE(bgndAppearance, false)
            QTC_CFG_READ_GRAD_TYPE(bgndGrad)
            QTC_CFG_READ_GRAD_TYPE(menuBgndGrad)
            QTC_CFG_READ_APPEARANCE(menuBgndAppearance, false)
            QTC_CFG_READ_BOOL(fixParentlessDialogs)
            QTC_CFG_READ_STRIPE(stripedProgress)
            QTC_CFG_READ_SLIDER(sliderStyle)
            QTC_CFG_READ_BOOL(animatedProgress)
            QTC_CFG_READ_BOOL(embolden)
            QTC_CFG_READ_DEF_BTN(defBtnIndicator)
            QTC_CFG_READ_LINE(sliderThumbs)
            QTC_CFG_READ_LINE(handles)
            QTC_CFG_READ_BOOL(highlightTab)
            QTC_CFG_READ_INT_BOOL(colorSelTab, DEF_COLOR_SEL_TAB_FACTOR)
            QTC_CFG_READ_BOOL(roundAllTabs)
            QTC_CFG_READ_TAB_MO(tabMouseOver)
            QTC_CFG_READ_SHADE(shadeSliders, true, false, &opts->customSlidersColor)
            QTC_CFG_READ_SHADE(shadeMenubars, true, false, &opts->customMenubarsColor)
            QTC_CFG_READ_SHADE(shadeCheckRadio, false, false, &opts->customCheckRadioColor)
            QTC_CFG_READ_SHADE(sortedLv, true, false, &opts->customSortedLvColor)
            QTC_CFG_READ_APPEARANCE(menubarAppearance, false)
            QTC_CFG_READ_APPEARANCE(menuitemAppearance, true)
            QTC_CFG_READ_APPEARANCE(toolbarAppearance, false)
            QTC_CFG_READ_APPEARANCE(selectionAppearance, false)
#ifdef __cplusplus
            QTC_CFG_READ_APPEARANCE(dwtAppearance, false)
#endif
            QTC_CFG_READ_LINE(toolbarSeparators)
            QTC_CFG_READ_LINE(splitters)
            QTC_CFG_READ_BOOL(customMenuTextColor)
            QTC_CFG_READ_MOUSE_OVER(coloredMouseOver)
            QTC_CFG_READ_BOOL(menubarMouseOver)
            QTC_CFG_READ_BOOL(useHighlightForMenu)
            QTC_CFG_READ_BOOL(shadeMenubarOnlyWhenActive)
            QTC_CFG_READ_BOOL(thinnerMenuItems)
            QTC_CFG_READ_BOOL(thinnerBtns)
            if(version<QTC_MAKE_VERSION(0, 63))
            {
                if(QTC_IS_BLACK(opts->customSlidersColor))
                    QTC_CFG_READ_COLOR(customSlidersColor)
                if(QTC_IS_BLACK(opts->customMenubarsColor))
                    QTC_CFG_READ_COLOR(customMenubarsColor)
                if(QTC_IS_BLACK(opts->customCheckRadioColor))
                    QTC_CFG_READ_COLOR(customCheckRadioColor)
            }
            QTC_CFG_READ_COLOR(customMenuSelTextColor)
            QTC_CFG_READ_COLOR(customMenuNormTextColor)
            QTC_CFG_READ_SCROLLBAR(scrollbarType)
            QTC_CFG_READ_EFFECT(buttonEffect)
            QTC_CFG_READ_APPEARANCE(lvAppearance, false)
            QTC_CFG_READ_APPEARANCE(tabAppearance, false)
            QTC_CFG_READ_APPEARANCE(activeTabAppearance, false)
            QTC_CFG_READ_APPEARANCE(sliderAppearance, false)
            QTC_CFG_READ_APPEARANCE(progressAppearance, false)
            QTC_CFG_READ_APPEARANCE(progressGrooveAppearance, false)
            QTC_CFG_READ_APPEARANCE(grooveAppearance, false)
            QTC_CFG_READ_APPEARANCE(sunkenAppearance, false)
            QTC_CFG_READ_APPEARANCE(sbarBgndAppearance, false)

            if(version<QTC_MAKE_VERSION(0, 63))
                opts->sliderFill=IS_FLAT(opts->appearance) ? opts->grooveAppearance : APPEARANCE_GRADIENT;
            else
            {
                QTC_CFG_READ_APPEARANCE(sliderFill, false)
            }
            QTC_CFG_READ_ECOLOR(progressGrooveColor)
            QTC_CFG_READ_FOCUS(focus)
            QTC_CFG_READ_BOOL(lvButton)
            QTC_CFG_READ_LV_LINES(lvLines)
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
            QTC_CFG_READ_BOOL(bgndRings)
#endif
            QTC_CFG_READ_BOOL(colorMenubarMouseOver)
            QTC_CFG_READ_INT_BOOL(crHighlight, opts->highlightFactor)
            QTC_CFG_READ_BOOL(crButton)
            QTC_CFG_READ_BOOL(crColor)
            QTC_CFG_READ_BOOL(smallRadio)
            QTC_CFG_READ_BOOL(fillProgress)
            QTC_CFG_READ_BOOL(comboSplitter)
            QTC_CFG_READ_BOOL(squareScrollViews)
            QTC_CFG_READ_BOOL(highlightScrollViews)
            QTC_CFG_READ_BOOL(etchEntry)
            QTC_CFG_READ_INT_BOOL(splitterHighlight, opts->highlightFactor)
            QTC_CFG_READ_BOOL(flatSbarButtons)
            QTC_CFG_READ_BOOL(popupBorder)
            QTC_CFG_READ_BOOL(unifySpinBtns)
            QTC_CFG_READ_BOOL(unifySpin)
            QTC_CFG_READ_BOOL(unifyCombo)
            QTC_CFG_READ_BOOL(borderTab)
            QTC_CFG_READ_BOOL(borderInactiveTab)
            QTC_CFG_READ_BOOL(thinSbarGroove)
            QTC_CFG_READ_BOOL(colorSliderMouseOver)
            QTC_CFG_READ_BOOL(menuIcons)
            QTC_CFG_READ_BOOL(forceAlternateLvCols)
            QTC_CFG_READ_BOOL(squareLvSelection)
            QTC_CFG_READ_BOOL(invertBotTab)
            QTC_CFG_READ_BOOL(menubarHiding)
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000))
            QTC_CFG_READ_BOOL(stdBtnSizes)
            QTC_CFG_READ_BOOL(titlebarBorder)
            QTC_CFG_READ_INT(titlebarButtons)
            QTC_CFG_READ_TB_ICON(titlebarIcon)
#endif
#if defined QT_VERSION && (QT_VERSION >= 0x040000)
            QTC_CFG_READ_BOOL(xbar)
            QTC_CFG_READ_INT(dwtSettings)
#endif
            QTC_CFG_READ_SHADE(menuStripe, true, true, &opts->customMenuStripeColor)
            QTC_CFG_READ_APPEARANCE(menuStripeAppearance, false)
            if(version<QTC_MAKE_VERSION(0, 63) && QTC_IS_BLACK(opts->customMenuStripeColor))
                QTC_CFG_READ_COLOR(customMenuStripeColor)
            QTC_CFG_READ_SHADE(comboBtn, true, false, &opts->customComboBtnColor);
            QTC_CFG_READ_BOOL(gtkScrollViews)
            QTC_CFG_READ_BOOL(doubleGtkComboArrow)
            QTC_CFG_READ_BOOL(stdSidebarButtons)
#ifdef __cplusplus
            QTC_CFG_READ_ALIGN(titlebarAlignment)
            QTC_CFG_READ_EFFECT(titlebarEffect)
            QTC_CFG_READ_BOOL(gtkComboMenus)
            QTC_CFG_READ_BOOL(colorTitlebarOnly)
/*
#else
            QTC_CFG_READ_BOOL(setDialogButtonOrder)
*/
#endif
#if !defined __cplusplus || defined QTC_CONFIG_DIALOG
            QTC_CFG_READ_INT(expanderHighlight)
            QTC_CFG_READ_BOOL(mapKdeIcons)
            
            if(SHADE_NONE==opts->menuStripe)
                opts->gtkMenuStripe=false;
            else
            {
                QTC_CFG_READ_BOOL(gtkMenuStripe)
            }
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
#if defined QT_VERSION && (QT_VERSION >= 0x040000)
            if(opts->xbar && opts->menubarHiding)
                opts->xbar=false;
#endif
#endif
            QTC_CFG_READ_SHADING(shading);

#ifdef __cplusplus
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000))
            if(opts->titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR)
            {
#if (defined QT_VERSION && (QT_VERSION >= 0x040000))
                QStringList cols(readStringEntry(cfg, "titlebarButtonColors").split(',', QString::SkipEmptyParts));
#else
                QStringList cols(QStringList::split(',', readStringEntry(cfg, "titlebarButtonColors")));
#endif
                if(NUM_TITLEBAR_BUTTONS==cols.count())
                {
                    QStringList::ConstIterator it(cols.begin()),
                                               end(cols.end());
                    TBCols                     cols;

                    for(int i=0; it!=end; ++it, ++i)
                    {
                        QColor col;
                        setRgb(&col, QTC_LATIN1((*it)));
                        cols[(ETitleBarButtons)i]=col;
                    }
                    opts->titlebarButtonColors=cols;
                }
                else
                    opts->titlebarButtons&=~QTC_TITLEBAR_BUTTON_COLOR;
            }
#endif

#if (defined QT_VERSION && (QT_VERSION >= 0x040000))
            QStringList shades(readStringEntry(cfg, "customShades").split(',', QString::SkipEmptyParts));
#else
            QStringList shades(QStringList::split(',', readStringEntry(cfg, "customShades")));
#endif
            bool ok(NUM_STD_SHADES==shades.size());

            if(ok)
            {
                QStringList::ConstIterator it(shades.begin());

                for(i=0; i<NUM_STD_SHADES && ok; ++i, ++it)
                    opts->customShades[i]=(*it).toDouble(&ok);
            }

            if(!ok && shades.size())
                opts->customShades[0]=0;

            for(i=APPEARANCE_CUSTOM1; i<(APPEARANCE_CUSTOM1+QTC_NUM_CUSTOM_GRAD); ++i)
            {
                QString gradKey;

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
                    Gradient                   grad;
                    int                        j;

                    grad.border=toGradientBorder(QTC_LATIN1((*it)));

                    for(++it, j=0; it!=end && ok; ++it, ++j)
                    {
                        double pos=(*it).toDouble(&ok),
                               val=ok ? (*(++it)).toDouble(&ok) : 0.0;

                        if(ok)
                            ok=(pos>=0 && pos<=1.0) &&
                                (val>=0.0 && val<=2.0);

                        if(ok)
                            grad.stops.insert(GradientStop(pos, val));
                    }

                    if(ok)
                    {
                        opts->customGradient[(EAppearance)i]=grad;
                        opts->customGradient[(EAppearance)i].stops=grad.stops.fix();
                    }
                }
            }
#else
            {
            char *str=readStringEntry(cfg, "customShades");

            if(str)
            {
                int  j,
                     comma=0;
                bool ok=true;

                for(j=0; str[j]; ++j)
                    if(','==str[j])
                        comma++;

                ok=(NUM_STD_SHADES-1)==comma;
                if(ok)
                {

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
                }

                if(!ok)
                    opts->customShades[0]=0;
            }
            }

            for(i=0; i<QTC_NUM_CUSTOM_GRAD; ++i)
            {
                char gradKey[18];
                char *str;

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
                        if(opts->customGradient[i]->stops)
                            free(opts->customGradient[i]->stops);
                        free(opts->customGradient[i]);
                        opts->customGradient[i]=0L;
                    }

                    if(comma>=4 && 0==comma%2)
                    {
                        char *c=strchr(str, ',');

                        if(c)
                        {
                            EGradientBorder border=toGradientBorder(str);
                            bool            ok=true;

                            *c='\0';

                            opts->customGradient[i]=malloc(sizeof(Gradient));
                            opts->customGradient[i]->numStops=comma/2;
                            opts->customGradient[i]->stops=malloc(sizeof(GradientStop) * opts->customGradient[i]->numStops);
                            opts->customGradient[i]->border=border;
                            str=c+1;
                            for(j=0; j<comma && str && ok; j+=2)
                            {
                                c=strchr(str, ',');

                                if(c)
                                {
                                    *c='\0';
                                    opts->customGradient[i]->stops[j/2].pos=g_ascii_strtod(str, NULL);
                                    str=c+1;
                                    c=str ? strchr(str, ',') : 0L;

                                    if(c || str)
                                    {
                                        if(c)
                                            *c='\0';
                                        opts->customGradient[i]->stops[j/2].val=g_ascii_strtod(str, NULL);
                                        str=c ? c+1 : c;
                                    }
                                    else
                                        ok=false;
                                }
                                else
                                    ok=false;
                            }

                            if(ok)
                            {
                                int addStart=0,
                                    addEnd=0;
                                if(opts->customGradient[i]->stops[0].pos>0.001)
                                    addStart=1;
                                if(opts->customGradient[i]->stops[opts->customGradient[i]->numStops-1].pos<0.999)
                                    addEnd=1;
                                if(addStart || addEnd)
                                {
                                    int          newSize=opts->customGradient[i]->numStops+addStart+addEnd;
                                    GradientStop *stops=malloc(sizeof(GradientStop) * newSize*2);

                                    if(addStart)
                                    {
                                        stops[0].pos=0.0;
                                        stops[0].val=1.0;
                                    }
                                    memcpy(&stops[1], opts->customGradient[i]->stops, sizeof(GradientStop) * opts->customGradient[i]->numStops*2);
                                    if(addEnd)
                                    {
                                        stops[opts->customGradient[i]->numStops+1].pos=1.0;
                                        stops[opts->customGradient[i]->numStops+1].val=1.0;
                                    }
                                    opts->customGradient[i]->numStops=newSize;
                                    free(opts->customGradient[i]->stops);
                                    opts->customGradient[i]->stops=stops;
                                }
                            }
                            else
                            {
                                free(opts->customGradient[i]->stops);
                                free(opts->customGradient[i]);
                                opts->customGradient[i]=0L;
                            }
                        }
                    }
                }
            }
#endif

            /* **Must** check appearance first, as the rest will default to this */
            checkAppearance(&opts->appearance, opts);
            checkAppearance(&opts->bgndAppearance, opts);
            checkAppearance(&opts->menuBgndAppearance, opts);
            checkAppearance(&opts->menubarAppearance, opts);
            checkAppearance(&opts->menuitemAppearance, opts);
            checkAppearance(&opts->toolbarAppearance, opts);
            checkAppearance(&opts->lvAppearance, opts);
            checkAppearance(&opts->tabAppearance, opts);
            checkAppearance(&opts->activeTabAppearance, opts);
            checkAppearance(&opts->sliderAppearance, opts);
            checkAppearance(&opts->selectionAppearance, opts);
#ifdef __cplusplus
            checkAppearance(&opts->titlebarAppearance, opts);
            checkAppearance(&opts->inactiveTitlebarAppearance, opts);
            checkAppearance(&opts->titlebarButtonAppearance, opts);
            checkAppearance(&opts->selectionAppearance, opts);
            checkAppearance(&opts->dwtAppearance, opts);
#endif
            checkAppearance(&opts->menuStripeAppearance, opts);
            checkAppearance(&opts->progressAppearance, opts);
            checkAppearance(&opts->progressGrooveAppearance, opts);
            checkAppearance(&opts->grooveAppearance, opts);
            checkAppearance(&opts->sunkenAppearance, opts);
            checkAppearance(&opts->sbarBgndAppearance, opts);
            checkAppearance(&opts->sliderFill, opts);
#ifndef __cplusplus
            releaseConfig(cfg);
#endif
            if(SHADE_BLEND_SELECTED==opts->shadeCheckRadio)
                opts->shadeCheckRadio=SHADE_SELECTED;

            checkColor(&opts->shadeMenubars, &opts->customMenubarsColor);
            checkColor(&opts->shadeSliders, &opts->customSlidersColor);
            checkColor(&opts->shadeCheckRadio, &opts->customCheckRadioColor);
            checkColor(&opts->menuStripe, &opts->customMenuStripeColor);
            checkColor(&opts->comboBtn, &opts->customComboBtnColor);
            checkColor(&opts->sortedLv, &opts->customSortedLvColor);
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

            if(APPEARANCE_RAISED==opts->selectionAppearance)
                opts->selectionAppearance=APPEARANCE_FLAT;
            else if(APPEARANCE_BEVELLED==opts->selectionAppearance)
                opts->selectionAppearance=APPEARANCE_GRADIENT;

            if(APPEARANCE_RAISED==opts->menuStripeAppearance)
                opts->menuStripeAppearance=APPEARANCE_FLAT;
            else if(APPEARANCE_BEVELLED==opts->menuStripeAppearance)
                opts->menuStripeAppearance=APPEARANCE_GRADIENT;

            if(opts->highlightFactor<MIN_HIGHLIGHT_FACTOR || opts->highlightFactor>MAX_HIGHLIGHT_FACTOR)
                opts->highlightFactor=DEFAULT_HIGHLIGHT_FACTOR;

            if(opts->crHighlight<MIN_HIGHLIGHT_FACTOR || opts->crHighlight>MAX_HIGHLIGHT_FACTOR)
                opts->crHighlight=DEFAULT_CR_HIGHLIGHT_FACTOR;

            if(opts->splitterHighlight<MIN_HIGHLIGHT_FACTOR || opts->splitterHighlight>MAX_HIGHLIGHT_FACTOR)
                opts->splitterHighlight=DEFAULT_SPLITTER_HIGHLIGHT_FACTOR;

#if !defined __cplusplus || defined QTC_CONFIG_DIALOG
            if(opts->expanderHighlight<MIN_HIGHLIGHT_FACTOR || opts->expanderHighlight>MAX_HIGHLIGHT_FACTOR)
                opts->expanderHighlight=DEFAULT_EXPANDER_HIGHLIGHT_FACTOR;
#endif

            if(opts->menuDelay<MIN_MENU_DELAY || opts->menuDelay>MAX_MENU_DELAY)
                opts->menuDelay=DEFAULT_MENU_DELAY;

            if(0==opts->sliderWidth%2)
                opts->sliderWidth++;

            if(opts->sliderWidth<MIN_SLIDER_WIDTH || opts->sliderWidth>MAX_SLIDER_WIDTH)
                opts->sliderWidth=DEFAULT_SLIDER_WIDTH;

            if(opts->sliderWidth<DEFAULT_SLIDER_WIDTH)
                opts->sliderThumbs=LINE_NONE;

            if(opts->lighterPopupMenuBgnd<MIN_LIGHTER_POPUP_MENU || opts->lighterPopupMenuBgnd>MAX_LIGHTER_POPUP_MENU)
                opts->lighterPopupMenuBgnd=DEF_POPUPMENU_LIGHT_FACTOR;

            if(opts->tabBgnd<MIN_TAB_BGND || opts->tabBgnd>MAX_TAB_BGND)
                opts->tabBgnd=DEF_TAB_BGND;

            if(opts->animatedProgress && !opts->stripedProgress)
                opts->animatedProgress=false;

/*
??
            if(SHADE_CUSTOM==opts->shadeMenubars || SHADE_BLEND_SELECTED==opts->shadeMenubars || !opts->borderMenuitems)
                opts->colorMenubarMouseOver=true;
*/

#if defined __cplusplus && defined QT_VERSION && QT_VERSION < 0x040000 && !defined QTC_CONFIG_DIALOG
            if(opts->round>ROUND_FULL)
                opts->round=ROUND_FULL;

            if(MO_GLOW==opts->coloredMouseOver && (EFFECT_NONE==opts->buttonEffect || opts->round<ROUND_FULL))
                opts->coloredMouseOver=MO_COLORED;

            if(IND_GLOW==opts->defBtnIndicator && (EFFECT_NONE==opts->buttonEffect || opts->round<ROUND_FULL))
                opts->defBtnIndicator=IND_TINT;
#endif
#ifndef QTC_CONFIG_DIALOG
            if(opts->round>ROUND_EXTRA)
                opts->focus=FOCUS_LINE;

            if(EFFECT_NONE==opts->buttonEffect)
                opts->etchEntry=false;

            if(opts->squareScrollViews)
                opts->highlightScrollViews=false;

            if(!opts->framelessGroupBoxes)
                opts->groupBoxLine=false;
#endif
#ifndef __cplusplus
            if(!defOpts)
            {
                int i;

                for(i=0; i<QTC_NUM_CUSTOM_GRAD; ++i)
                    if(def->customGradient[i])
                        free(def->customGradient[i]);
            }
#endif

#ifndef QTC_CONFIG_DIALOG
            opts->bgndAppearance=MODIFY_AGUA(opts->bgndAppearance);
            opts->selectionAppearance=MODIFY_AGUA(opts->selectionAppearance);
            opts->lvAppearance=MODIFY_AGUA_X(opts->lvAppearance, APPEARANCE_LV_AGUA);
            opts->sbarBgndAppearance=MODIFY_AGUA(opts->sbarBgndAppearance);
            opts->progressGrooveAppearance=MODIFY_AGUA(opts->progressGrooveAppearance);
            opts->menuBgndAppearance=MODIFY_AGUA(opts->menuBgndAppearance);
            opts->menuStripeAppearance=MODIFY_AGUA(opts->menuStripeAppearance);
            opts->grooveAppearance=MODIFY_AGUA(opts->grooveAppearance);
            opts->progressAppearance=MODIFY_AGUA(opts->progressAppearance);
            opts->sliderFill=MODIFY_AGUA(opts->sliderFill);
            opts->tabAppearance=MODIFY_AGUA(opts->tabAppearance);
            opts->activeTabAppearance=MODIFY_AGUA(opts->activeTabAppearance);
            opts->menuitemAppearance=MODIFY_AGUA(opts->menuitemAppearance);
#ifdef __cplusplus
            opts->titlebarAppearance=MODIFY_AGUA(opts->titlebarAppearance);
            opts->inactiveTitlebarAppearance=MODIFY_AGUA(opts->inactiveTitlebarAppearance);

#if defined QT_VERSION && QT_VERSION >= 0x040000
            if(!(opts->titlebarButtons&QTC_TITLEBAR_BUTTON_ROUND))
#endif
                opts->titlebarButtonAppearance=MODIFY_AGUA(opts->titlebarButtonAppearance);
            opts->dwtAppearance=MODIFY_AGUA(opts->dwtAppearance);
#endif
            if(APPEARANCE_FLAT==opts->tabAppearance)
                opts->tabAppearance=APPEARANCE_RAISED;

            if(EFFECT_NONE==opts->buttonEffect && MO_GLOW==opts->coloredMouseOver)
                opts->coloredMouseOver=MO_COLORED_THICK;

            if(EFFECT_NONE==opts->buttonEffect)
                opts->etchEntry=false;
            if(opts->colorSliderMouseOver &&
               (SHADE_NONE==opts->shadeSliders || SHADE_DARKEN==opts->shadeSliders))
                opts->colorSliderMouseOver=false;
#endif

            if(LINE_1DOT==opts->toolbarSeparators)
                opts->toolbarSeparators=LINE_DOTS;

            return true;
        }
        else
        {
#ifdef __cplusplus
            if(defOpts)
                *opts=*defOpts;
            else
                defaultSettings(opts);
#else
            if(defOpts)
                copyOpts(defOpts, opts);
            else
                defaultSettings(opts);
#endif
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

    for(i=0; i<QTC_NUM_CUSTOM_GRAD; ++i)
        opts->customGradient[i]=0L;
#else
    // Setup titlebar gradients...
    setupGradient(&(opts->customGradient[APPEARANCE_CUSTOM1]), GB_3D,3,0.0,1.2,0.5,1.0,1.0,1.0);
    setupGradient(&(opts->customGradient[APPEARANCE_CUSTOM2]), GB_3D,3,0.0,0.9,0.5,1.0,1.0,1.0);
#endif
    opts->customShades[0]=1.16;
    opts->customShades[1]=1.07;
    opts->customShades[2]=0.9;
    opts->customShades[3]=0.78;
    opts->customShades[4]=0.84;
    opts->customShades[5]=0.75;
    opts->contrast=7;
    opts->passwordChar=0x25CF;
    opts->highlightFactor=DEFAULT_HIGHLIGHT_FACTOR;
    opts->crHighlight=DEFAULT_CR_HIGHLIGHT_FACTOR;
    opts->splitterHighlight=DEFAULT_SPLITTER_HIGHLIGHT_FACTOR;
    opts->menuDelay=DEFAULT_MENU_DELAY;
    opts->sliderWidth=DEFAULT_SLIDER_WIDTH;
    opts->selectionAppearance=APPEARANCE_HARSH_GRADIENT;
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000)) || !defined __cplusplus
    opts->round=ROUND_EXTRA;
    opts->fadeLines=true;
    opts->bgndRings=false;
    opts->gtkButtonOrder=false;
#else
    opts->round=ROUND_FULL;
#endif
#ifdef __cplusplus
    opts->dwtAppearance=APPEARANCE_CUSTOM1;
#endif
    opts->lighterPopupMenuBgnd=DEF_POPUPMENU_LIGHT_FACTOR;
    opts->tabBgnd=DEF_TAB_BGND;
    opts->animatedProgress=false;
    opts->stripedProgress=STRIPE_NONE;
    opts->sliderStyle=SLIDER_PLAIN;
    opts->highlightTab=false;
    opts->colorSelTab=0;
    opts->roundAllTabs=true;
    opts->tabMouseOver=TAB_MO_GLOW;
    opts->embolden=false;
    opts->bgndGrad=GT_HORIZ;
    opts->menuBgndGrad=GT_HORIZ;
    opts->appearance=APPEARANCE_SOFT_GRADIENT;
    opts->bgndAppearance=APPEARANCE_FLAT;
    opts->menuBgndAppearance=APPEARANCE_FLAT;
    opts->lvAppearance=APPEARANCE_BEVELLED;
    opts->tabAppearance=APPEARANCE_SOFT_GRADIENT;
    opts->activeTabAppearance=APPEARANCE_SOFT_GRADIENT;
    opts->sliderAppearance=APPEARANCE_SOFT_GRADIENT;
    opts->menubarAppearance=APPEARANCE_FLAT;
    opts->menuitemAppearance=APPEARANCE_FADE;
    opts->toolbarAppearance=APPEARANCE_FLAT;
    opts->progressAppearance=APPEARANCE_DULL_GLASS;
    opts->progressGrooveAppearance=APPEARANCE_INVERTED;
    opts->progressGrooveColor=ECOLOR_DARK;
    opts->grooveAppearance=APPEARANCE_INVERTED;
    opts->sunkenAppearance=APPEARANCE_SOFT_GRADIENT;
    opts->sbarBgndAppearance=APPEARANCE_FLAT;
    opts->sliderFill=APPEARANCE_GRADIENT;
    opts->defBtnIndicator=IND_GLOW;
    opts->sliderThumbs=LINE_FLAT;
    opts->handles=LINE_1DOT;
    opts->shadeSliders=SHADE_NONE;
    opts->shadeMenubars=SHADE_NONE;
    opts->shadeCheckRadio=SHADE_NONE;
    opts->sortedLv=SHADE_NONE;
    opts->toolbarBorders=TB_NONE;
    opts->toolbarSeparators=LINE_SUNKEN;
    opts->splitters=LINE_1DOT;
    opts->fixParentlessDialogs=false;
    opts->customMenuTextColor=false;
    opts->coloredMouseOver=MO_GLOW;
    opts->menubarMouseOver=true;
    opts->useHighlightForMenu=false;
    opts->shadeMenubarOnlyWhenActive=false;
    opts->thinnerMenuItems=false;
    opts->thinnerBtns=true;
    opts->scrollbarType=SCROLLBAR_KDE;
    opts->buttonEffect=EFFECT_SHADOW;
    opts->focus=FOCUS_LINE;
    opts->lvButton=false;
    opts->lvLines=LV_NONE;
    opts->drawStatusBarFrames=false;
    opts->fillSlider=true;
    opts->roundMbTopOnly=true;
    opts->borderMenuitems=false;
    opts->darkerBorders=false;
    opts->vArrows=true;
    opts->xCheck=false;
    opts->framelessGroupBoxes=true;
    opts->groupBoxLine=true;
    opts->colorMenubarMouseOver=true;
    opts->crButton=true;
    opts->crColor=false;
    opts->smallRadio=true;
    opts->fillProgress=true;
    opts->comboSplitter=false;
    opts->squareScrollViews=false;
    opts->highlightScrollViews=false;
    opts->etchEntry=false;
    opts->flatSbarButtons=true;
    opts->popupBorder=true;
    opts->unifySpinBtns=false;
    opts->unifySpin=true;
    opts->unifyCombo=true;
    opts->borderTab=true;
    opts->borderInactiveTab=false;
    opts->thinSbarGroove=true;
    opts->colorSliderMouseOver=false;
    opts->menuIcons=true;
    opts->forceAlternateLvCols=false;
    opts->squareLvSelection=false;
    opts->invertBotTab=true;
    opts->menubarHiding=false;
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000))
    opts->stdBtnSizes=false;
    opts->titlebarBorder=true;
    opts->titlebarButtons=QTC_TITLEBAR_BUTTON_ROUND|QTC_TITLEBAR_BUTTON_HOVER_SYMBOL;
    opts->titlebarIcon=TITLEBAR_ICON_NEXT_TO_TITLE;
#endif
#if defined QT_VERSION && (QT_VERSION >= 0x040000)
    opts->xbar=false;
    opts->dwtSettings=QTC_DWT_BUTTONS_AS_PER_TITLEBAR|QTC_DWT_ROUND_TOP_ONLY;
#endif
    opts->menuStripe=SHADE_NONE;
    opts->menuStripeAppearance=APPEARANCE_DARK_INVERTED;
    opts->shading=SHADING_HSL;
    opts->gtkScrollViews=true;
    opts->comboBtn=SHADE_NONE;
    opts->doubleGtkComboArrow=true;
    opts->stdSidebarButtons=false;
#ifdef __cplusplus
    opts->gtkComboMenus=false;
    opts->colorTitlebarOnly=false;
    opts->customMenubarsColor.setRgb(0, 0, 0);
    opts->customSlidersColor.setRgb(0, 0, 0);
    opts->customMenuNormTextColor.setRgb(0, 0, 0);
    opts->customMenuSelTextColor.setRgb(0, 0, 0);
    opts->customCheckRadioColor.setRgb(0, 0, 0);
    opts->customComboBtnColor.setRgb(0, 0, 0);
    opts->customMenuStripeColor.setRgb(0, 0, 0);
    opts->titlebarAlignment=ALIGN_FULL_CENTER;
    opts->titlebarEffect=EFFECT_SHADOW;
#else
/*
    opts->setDialogButtonOrder=false;
*/
    opts->customMenubarsColor.red=opts->customMenubarsColor.green=opts->customMenubarsColor.blue=0;
    opts->customSlidersColor.red=opts->customSlidersColor.green=opts->customSlidersColor.blue=0;
    opts->customMenuNormTextColor.red=opts->customMenuNormTextColor.green=opts->customMenuNormTextColor.blue=0;
    opts->customMenuSelTextColor.red=opts->customMenuSelTextColor.green=opts->customMenuSelTextColor.blue=0;
    opts->customCheckRadioColor.red=opts->customCheckRadioColor.green=opts->customCheckRadioColor.blue=0;
    opts->customComboBtnColor.red=opts->customCheckRadioColor.green=opts->customCheckRadioColor.blue=0;
    opts->customMenuStripeColor.red=opts->customMenuStripeColor.green=opts->customMenuStripeColor.blue=0;
#endif

#if !defined __cplusplus || defined QTC_CONFIG_DIALOG
    opts->mapKdeIcons=true;
    opts->gtkMenuStripe=false;
    opts->expanderHighlight=DEFAULT_EXPANDER_HIGHLIGHT_FACTOR;
#endif
#ifdef __cplusplus
    opts->titlebarAppearance=APPEARANCE_CUSTOM1;
    opts->inactiveTitlebarAppearance=APPEARANCE_CUSTOM1;
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
        opts->focus=FOCUS_FULL;
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
        case IND_DARKEN:
            return "darken";
        default:
            return "colored";
    }
}

static const char *toStr(ELine ind, bool dashes)
{
    switch(ind)
    {
        case LINE_1DOT:
            return "1dot";
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
        case MO_COLORED_THICK:
            return "thickcolored";
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
        case APPEARANCE_DULL_GLASS:
            return "dullglass";
        case APPEARANCE_SHINY_GLASS:
            return "shinyglass";
        case APPEARANCE_AGUA:
            return "agua";
        case APPEARANCE_SOFT_GRADIENT:
            return "soft";
        case APPEARANCE_GRADIENT:
            return "gradient";
        case APPEARANCE_HARSH_GRADIENT:
            return "harsh";
        case APPEARANCE_INVERTED:
            return "inverted";
        case APPEARANCE_DARK_INVERTED:
            return "darkinverted";
        case APPEARANCE_SPLIT_GRADIENT:
            return "splitgradient";
        case APPEARANCE_BEVELLED:
            return "bevelled";
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

static QString toStr(const QColor &col)
{
    QString colorStr;

    colorStr.sprintf("#%02X%02X%02X", col.red(), col.green(), col.blue());
    return colorStr;
}

static QString toStr(EShade exp, const QColor &col)
{
    switch(exp)
    {
        default:
        case SHADE_NONE:
            return "none";
        case SHADE_BLEND_SELECTED:
            return "selected";
        case SHADE_CUSTOM:
            return toStr(col);
        case SHADE_SELECTED:
            return "origselected";
        case SHADE_DARKEN:
            return "darken";
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
        case ROUND_MAX:
            return "max";
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
        case SHADING_HCY:
            return "hcy";
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
        case FOCUS_RECTANGLE:
            return "rect";
        case FOCUS_FILLED:
            return "filled";
        case FOCUS_FULL:
            return "full";
        case FOCUS_LINE:
            return "line";
    }
}

static const char *toStr(ETabMo f)
{
    switch(f)
    {
        default:
        case TAB_MO_BOTTOM:
            return "bot";
        case TAB_MO_TOP:
            return "top";
        case TAB_MO_GLOW:
            return "glow";
    }
}

static const char *toStr(EGradientBorder g)
{
    switch(g)
    {
        case GB_NONE:
            return "none";
        case GB_LIGHT:
            return "light";
        case GB_3D_FULL:
            return "3dfull";
        default:
        case GB_3D:
            return "3d";
    }
}

static const char *toStr(EAlign ind)
{
    switch(ind)
    {
        default:
        case ALIGN_LEFT:
            return "left";
        case ALIGN_CENTER:
            return "center";
        case ALIGN_FULL_CENTER:
            return "center-full";
        case ALIGN_RIGHT:
            return "right";
    }
}

static const char * toStr(ETitleBarIcon icn)
{
    switch(icn)
    {
        case TITLEBAR_ICON_NONE:
            return "none";
        default:
        case TITLEBAR_ICON_MENU_BUTTON:
            return "menu";
        case TITLEBAR_ICON_NEXT_TO_TITLE:
            return "title";
    }
}

static const char * toStr(EGradType gt)
{
    switch(gt)
    {
        case GT_VERT:
            return "vert";
        default:
        case GT_HORIZ:
            return "horiz";
    }
}

static const char * toStr(ELvLines lv)
{
    switch(lv)
    {
        case LV_NEW:
            return "new";
        case LV_OLD:
            return "old";
        default:
        case LV_NONE:
            return "none";
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

#define CFG_WRITE_ENTRY_B(ENTRY, B) \
    if (!exportingStyle && def.ENTRY==opts.ENTRY) \
        CFG.deleteEntry(#ENTRY); \
    else \
        CFG.writeEntry(#ENTRY, toStr(opts.ENTRY, B));

#define CFG_WRITE_ENTRY_NUM(ENTRY) \
    if (!exportingStyle && def.ENTRY==opts.ENTRY) \
        CFG.deleteEntry(#ENTRY); \
    else \
        CFG.writeEntry(#ENTRY, opts.ENTRY);

#define CFG_WRITE_SHADE_ENTRY(ENTRY, COL) \
    if (!exportingStyle && def.ENTRY==opts.ENTRY) \
        CFG.deleteEntry(#ENTRY); \
    else \
        CFG.writeEntry(#ENTRY, toStr(opts.ENTRY, opts.COL));

bool static writeConfig(KConfig *cfg, const Options &opts, const Options &def, bool exportingStyle=false)
{
    if(!cfg)
    {
        const char *cfgDir=qtcConfDir();

        if(cfgDir)
        {
#if QT_VERSION >= 0x040000
            KConfig defCfg(QFile::decodeName(cfgDir)+QTC_FILE, KConfig::SimpleConfig);
#else
            KConfig defCfg(QFile::decodeName(cfgDir)+QTC_FILE, false, false);
#endif

            if(writeConfig(&defCfg, opts, def, exportingStyle))
            {
                const char *oldFiles[]={ QTC_OLD_FILE, "qtcurve.gtk-icons", 0};

                for(int i=0; oldFiles[i]; ++i)
                {
                    QString oldFileName(QFile::decodeName(cfgDir)+QString("../")+oldFiles[i]);

                    if(QFile::exists(oldFileName))
                        QFile::remove(oldFileName);
                }
            }
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
        CFG_WRITE_ENTRY_NUM(menuDelay)
        CFG_WRITE_ENTRY_NUM(sliderWidth)
        CFG_WRITE_ENTRY(toolbarBorders)
        CFG_WRITE_ENTRY(appearance)
        CFG_WRITE_ENTRY(bgndAppearance)
        CFG_WRITE_ENTRY(bgndGrad)
        CFG_WRITE_ENTRY(menuBgndGrad)
        CFG_WRITE_ENTRY(menuBgndAppearance)
        CFG_WRITE_ENTRY(fixParentlessDialogs)
        CFG_WRITE_ENTRY(stripedProgress)
        CFG_WRITE_ENTRY(sliderStyle)
        CFG_WRITE_ENTRY(animatedProgress)
        CFG_WRITE_ENTRY_NUM(lighterPopupMenuBgnd)
        CFG_WRITE_ENTRY_NUM(tabBgnd)
        CFG_WRITE_ENTRY(embolden)
        CFG_WRITE_ENTRY(defBtnIndicator)
        CFG_WRITE_ENTRY_B(sliderThumbs, false)
        CFG_WRITE_ENTRY_B(handles, true)
        CFG_WRITE_ENTRY(highlightTab)
        CFG_WRITE_ENTRY_NUM(colorSelTab)
        CFG_WRITE_ENTRY(roundAllTabs)
        CFG_WRITE_ENTRY(tabMouseOver)
        CFG_WRITE_ENTRY(menubarAppearance)
        CFG_WRITE_ENTRY(menuitemAppearance)
        CFG_WRITE_ENTRY(toolbarAppearance)
        CFG_WRITE_ENTRY(selectionAppearance)
#ifdef __cplusplus
        CFG_WRITE_ENTRY(dwtAppearance)
        CFG_WRITE_ENTRY(titlebarEffect)
#endif
        CFG_WRITE_ENTRY(menuStripeAppearance)
        CFG_WRITE_ENTRY_B(toolbarSeparators, false)
        CFG_WRITE_ENTRY_B(splitters, true)
        CFG_WRITE_ENTRY(customMenuTextColor)
        CFG_WRITE_ENTRY(coloredMouseOver)
        CFG_WRITE_ENTRY(menubarMouseOver)
        CFG_WRITE_ENTRY(useHighlightForMenu)
        CFG_WRITE_ENTRY(shadeMenubarOnlyWhenActive)
        CFG_WRITE_ENTRY(thinnerMenuItems)
        CFG_WRITE_ENTRY(thinnerBtns)
        CFG_WRITE_SHADE_ENTRY(shadeSliders, customSlidersColor)
        CFG_WRITE_SHADE_ENTRY(shadeMenubars, customMenubarsColor)
        CFG_WRITE_SHADE_ENTRY(sortedLv, customSortedLvColor)
        CFG_WRITE_ENTRY(customMenuSelTextColor)
        CFG_WRITE_ENTRY(customMenuNormTextColor)
        CFG_WRITE_SHADE_ENTRY(shadeCheckRadio, customCheckRadioColor)
        CFG_WRITE_ENTRY(scrollbarType)
        CFG_WRITE_ENTRY(buttonEffect)
        CFG_WRITE_ENTRY(lvAppearance)
        CFG_WRITE_ENTRY(tabAppearance)
        CFG_WRITE_ENTRY(activeTabAppearance)
        CFG_WRITE_ENTRY(sliderAppearance)
        CFG_WRITE_ENTRY(progressAppearance)
        CFG_WRITE_ENTRY(progressGrooveAppearance)
        CFG_WRITE_ENTRY(grooveAppearance)
        CFG_WRITE_ENTRY(sunkenAppearance)
        CFG_WRITE_ENTRY(sbarBgndAppearance)
        CFG_WRITE_ENTRY(sliderFill)
        CFG_WRITE_ENTRY(progressGrooveColor)
        CFG_WRITE_ENTRY(focus)
        CFG_WRITE_ENTRY(lvButton)
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
        CFG_WRITE_ENTRY(bgndRings)
        CFG_WRITE_ENTRY(colorMenubarMouseOver)
        CFG_WRITE_ENTRY_NUM(crHighlight)
        CFG_WRITE_ENTRY(crButton)
        CFG_WRITE_ENTRY(crColor)
        CFG_WRITE_ENTRY(smallRadio)
        CFG_WRITE_ENTRY(fillProgress)
        CFG_WRITE_ENTRY(comboSplitter)
        CFG_WRITE_ENTRY(squareScrollViews)
        CFG_WRITE_ENTRY(highlightScrollViews)
        CFG_WRITE_ENTRY(etchEntry)
        CFG_WRITE_ENTRY_NUM(splitterHighlight)
        CFG_WRITE_ENTRY_NUM(expanderHighlight)
        CFG_WRITE_ENTRY(flatSbarButtons)
        CFG_WRITE_ENTRY(popupBorder)
        CFG_WRITE_ENTRY(unifySpinBtns)
        CFG_WRITE_ENTRY(unifySpin)
        CFG_WRITE_ENTRY(unifyCombo)
        CFG_WRITE_ENTRY(borderTab)
        CFG_WRITE_ENTRY(borderInactiveTab)
        CFG_WRITE_ENTRY(thinSbarGroove)
        CFG_WRITE_ENTRY(colorSliderMouseOver)
        CFG_WRITE_ENTRY(menuIcons)
        CFG_WRITE_ENTRY(forceAlternateLvCols)
        CFG_WRITE_ENTRY(squareLvSelection)
        CFG_WRITE_ENTRY(invertBotTab)
        CFG_WRITE_ENTRY(menubarHiding)
#if defined QT_VERSION && (QT_VERSION >= 0x040000)
        CFG_WRITE_ENTRY(xbar)
        CFG_WRITE_ENTRY_NUM(dwtSettings)
#endif
#if defined QTC_CONFIG_DIALOG || (defined QT_VERSION && (QT_VERSION >= 0x040000))
        CFG_WRITE_ENTRY(stdBtnSizes)
        CFG_WRITE_ENTRY(titlebarBorder)
        CFG_WRITE_ENTRY_NUM(titlebarButtons)
        CFG_WRITE_ENTRY(titlebarIcon)

        if(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR && NUM_TITLEBAR_BUTTONS==opts.titlebarButtonColors.size())
        {
            QString     val;
#if QT_VERSION >= 0x040000
            QTextStream str(&val);
#else
            QTextStream str(&val, IO_WriteOnly);
#endif
            for(int i=0; i<NUM_TITLEBAR_BUTTONS; ++i)
            {
                TBCols::const_iterator c(opts.titlebarButtonColors.find((ETitleBarButtons)i));

                if(c!=opts.titlebarButtonColors.end())
                {
                    if(i)
                        str << ',';
                    str << toStr((*c).second);
                }
            }
            CFG.writeEntry("titlebarButtonColors", val);
        }
        else
            CFG.deleteEntry("titlebarButtonColors");
#endif
        CFG_WRITE_SHADE_ENTRY(menuStripe, customMenuStripeColor)
        CFG_WRITE_SHADE_ENTRY(comboBtn, customComboBtnColor)
        CFG_WRITE_ENTRY(stdSidebarButtons)
        CFG_WRITE_ENTRY(titlebarAppearance)
        CFG_WRITE_ENTRY(inactiveTitlebarAppearance)
        CFG_WRITE_ENTRY(titlebarButtonAppearance)
        CFG_WRITE_ENTRY(gtkScrollViews)
        CFG_WRITE_ENTRY(gtkComboMenus)
        CFG_WRITE_ENTRY(doubleGtkComboArrow)
        CFG_WRITE_ENTRY(colorTitlebarOnly)
        CFG_WRITE_ENTRY(gtkButtonOrder)
        CFG_WRITE_ENTRY(mapKdeIcons)
        CFG_WRITE_ENTRY(gtkMenuStripe)
        CFG_WRITE_ENTRY(shading)
        CFG_WRITE_ENTRY(titlebarAlignment)

        for(int i=APPEARANCE_CUSTOM1; i<(APPEARANCE_CUSTOM1+QTC_NUM_CUSTOM_GRAD); ++i)
        {
            GradientCont::const_iterator cg(opts.customGradient.find((EAppearance)i));
            QString                      gradKey;

            gradKey.sprintf("customgradient%d", (i-APPEARANCE_CUSTOM1)+1);

            if(cg==opts.customGradient.end())
                CFG.deleteEntry(gradKey);
            else
            {
                GradientCont::const_iterator d;

                if(exportingStyle || (d=def.customGradient.find((EAppearance)i))==def.customGradient.end() || !((*d)==(*cg)))
                {
                    QString     gradVal;
#if QT_VERSION >= 0x040000
                    QTextStream str(&gradVal);
#else
                    QTextStream str(&gradVal, IO_WriteOnly);
#endif

                    str << toStr((*cg).second.border);

                    GradientStopCont                 stops((*cg).second.stops.fix());
                    GradientStopCont::const_iterator it(stops.begin()),
                                                     end(stops.end());

                    for(; it!=end; ++it)
                        str << ',' << (*it).pos << ',' << (*it).val;
                    CFG.writeEntry(gradKey, gradVal);
                }
                else
                    CFG.deleteEntry(gradKey);
            }
        }

        if(opts.customShades[0]==0 ||
           exportingStyle ||
           opts.customShades[0]!=def.customShades[0] ||
           opts.customShades[1]!=def.customShades[1] ||
           opts.customShades[2]!=def.customShades[2] ||
           opts.customShades[3]!=def.customShades[3] ||
           opts.customShades[4]!=def.customShades[4] ||
           opts.customShades[5]!=def.customShades[5])
        {
            QString     shadeVal;
#if QT_VERSION >= 0x040000
            QTextStream str(&shadeVal);
#else
            QTextStream str(&shadeVal, IO_WriteOnly);
#endif
            if(0==opts.customShades[0])
                 str << 0;
            else
                for(int i=0; i<NUM_STD_SHADES; ++i)
                    if(0==i)
                        str << opts.customShades[i];
                    else
                        str << ',' << opts.customShades[i];
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
