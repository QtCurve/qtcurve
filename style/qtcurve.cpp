/*
  QtCurve (C) Craig Drummond, 2007-2008 Craig.Drummond@lycos.co.uk

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

#include <QtGui>
#include <QX11Info>
#define QTC_COMMON_FUNCTIONS
#include "qtcurve.h"
#include "pixmaps.h"
#define CONFIG_READ
#include "config_file.c"

// The tabs used in multi-dock widgets, and KDE's properties dialog, look odd,
// as the QTabBar is not a child of a QTabWidget! the QTC_STYLE_QTABBAR controls
// whether we should style this differently.
// CPD:TODO Check if this is OK with KDE's properties dialog, this looks broken
//          in KDE4 betas, etc. But this may be changed/fixed when released!
//#define QTC_STYLE_QTABBAR

//KDE Properties dialog: QTabBar::KDEPrivate::KPageTabbedView
//  --fixed in KDE4.0
//Dolphin's views:       QTabBar::DolphinMainWindow
//Konsole:               KTabBar::QWidget

#define WINDOWTITLE_SPACER 0x10000000
#define QTC_STATE_REVERSE  State_Mini

#define M_PI 3.14159265358979323846

static const int constMenuPixmapWidth=22;

static enum
{
    APP_SKIP_TASKBAR,
    APP_KPRINTER,
    APP_KDIALOG,
    APP_KDIALOGD,
    APP_PLASMA,
    APP_KWIN,
    APP_OTHER
} theThemedApp=APP_OTHER;

static QSet<const QWidget *> theNoEtchWidgets;

int static toHint(int sc)
{
    switch(sc)
    {
        case QStyle::SC_TitleBarSysMenu:
            return Qt::WindowSystemMenuHint;
        case QStyle::SC_TitleBarMinButton:
            return Qt::WindowMinimizeButtonHint;
        case QStyle::SC_TitleBarMaxButton:
            return Qt::WindowMaximizeButtonHint;
        case QStyle::SC_TitleBarCloseButton:
            return 0;
        case QStyle::SC_TitleBarNormalButton:
            return 0;
        case QStyle::SC_TitleBarShadeButton:
        case QStyle::SC_TitleBarUnshadeButton:
            return Qt::WindowShadeButtonHint;
        case QStyle::SC_TitleBarContextHelpButton:
            return Qt::WindowContextHelpButtonHint;
        default:
            return 0;
    }
}

//
// OK, Etching looks cr*p on plasma widgets, and khtml...
// CPD:TODO WebKit?
class CEtchCheck
{
    public:

    CEtchCheck(const QWidget *widget)
    {
        itsPreviousStatus=theirStatus;

        if(theirStatus)
            theirStatus=!(theNoEtchWidgets.contains(widget));
    }

    ~CEtchCheck()
    {
        theirStatus=itsPreviousStatus;
    }

    static void disable() { theirStatus=false; }
    static bool canEtch() { return theirStatus; }

    static bool isA(const QObject *w, const char *type)
    {
        return w && (0==strcmp(w->metaObject()->className(), type) ||
                     (w->parent() && 0==strcmp(w->parent()->metaObject()->className(), type)));
    }

    static bool isNoEtchWidget(const QWidget *widget)
    {
        // Plasma: widget -> ControlWidget -> ControlBox -> RootWidget
        // KHTML:  widget -> QWidget       -> QWidget    -> KHTMLView
        const QObject *w=widget && widget->parent() && widget->parent()->parent()
                            ? widget->parent()->parent()->parent() : NULL;

        return w && (isA(w, "KHTMLView") || (APP_PLASMA==theThemedApp && isA(w, "RootWidget")));
    }

    private:

    bool        itsPreviousStatus;
    static bool theirStatus;
};

bool CEtchCheck::theirStatus=true;

// from windows style
static const int windowsItemFrame    =  2; // menu item frame width
static const int windowsItemHMargin  =  3; // menu item hor text margin
static const int windowsItemVMargin  =  2; // menu item ver text margin
static const int windowsRightBorder  = 15; // right border on windows

static const int constProgressBarFps = 20;

#define QTC_SB_SUB2 ((QStyle::SubControl)(QStyle::SC_ScrollBarGroove << 1))

static QString readEnvPath(const char *env)
{
   QByteArray path=getenv(env);

   return path.isEmpty() ? QString::null : QFile::decodeName(path);
}

static QString kdeHome()
{
    QString env(readEnvPath(getuid() ? "KDEHOME" : "KDEROOTHOME"));

    return env.isEmpty()
                ? QDir::homePath()+"/.kde"
                : env;
}

static void getStyles(const QString &dir, QStringList &styles)
{
    QDir        d(dir+QTC_THEME_DIR);
    QStringList filters;

    filters << QString(QTC_THEME_PREFIX"*"QTC_THEME_SUFFIX);
    d.setNameFilters(filters);

    QStringList                entries(d.entryList());
    QStringList::ConstIterator it(entries.begin()),
                               end(entries.end());

    for(; it!=end; ++it)
    {
        QString style((*it).left((*it).lastIndexOf(QTC_THEME_SUFFIX)));

        if(!styles.contains(style))
            styles.append(style);
    }
}

static QString themeFile(const QString &dir, const QString &n)
{
    QString name(dir+QTC_THEME_DIR+n+QTC_THEME_SUFFIX);

    return QFile(name).exists() ? name : QString();
}

class QtCurveStylePlugin : public QStylePlugin
{
    public:

    QtCurveStylePlugin(QObject *parent=0) : QStylePlugin( parent ) {}
    ~QtCurveStylePlugin() {}

    QStringList keys() const
    {
        QStringList list;
        list << "QtCurve";

        getStyles(kdeHome(), list);
        getStyles(KDE_PREFIX(3), list);
        getStyles(KDE_PREFIX(4), list);

        return list;
    }

    QStyle * create(const QString &key)
    {
        return "qtcurve"==key.toLower()
                    ? new QtCurveStyle
                    : 0==key.indexOf(QTC_THEME_PREFIX)
                        ? new QtCurveStyle(key)
                        : 0;
    }
};

Q_EXPORT_PLUGIN2(QtCurveStyle, QtCurveStylePlugin)

inline int numButtons(EScrollbar type)
{
    switch(type)
    {
        default:
        case SCROLLBAR_KDE:
            return 3;
            break;
        case SCROLLBAR_WINDOWS:
        case SCROLLBAR_PLATINUM:
        case SCROLLBAR_NEXT:
            return 2;
            break;
        case SCROLLBAR_NONE:
            return 0;
    }
}

static inline void drawRect(QPainter *p, const QRect &r)
{
    p->drawRect(r.x(), r.y(), r.width()-1, r.height()-1);
}

static inline void drawAaLine(QPainter *p, int x1, int y1, int x2, int y2)
{
    p->drawLine(QLineF(x1+0.5, y1+0.5, x2+0.5, y2+0.5));
}

static void drawLines(QPainter *p, const QRect &r, bool horiz, int nLines, int offset,
                      const QColor *cols, int startOffset, int dark, int etchedDisp=1,
                      bool light=true)
{
    int space((nLines*2)+(etchedDisp || !light ? (nLines-1) : 0)),
        step(etchedDisp || !light ? 3 : 2),
        x(horiz ? r.x(): r.x()+((r.width()-space)>>1)),
        y(horiz ? r.y()+((r.height()-space)>>1): r.y()),
        x2(r.x()+r.width()-1),
        y2(r.y()+r.height()-1),
        i;

    if(horiz)
    {
        if(startOffset && y+startOffset>0)
            y+=startOffset;

        p->setPen(cols[dark]);
        for(i=0; i<space; i+=step)
            p->drawLine(x+offset, y+i, x2-(offset+etchedDisp), y+i);

        if(light)
        {
            p->setPen(cols[0]);
            for(i=1; i<space; i+=step)
                p->drawLine(x+offset+etchedDisp, y+i, x2-offset, y+i);
        }
    }
    else
    {
        if(startOffset && x+startOffset>0)
            x+=startOffset;

        p->setPen(cols[dark]);
        for(i=0; i<space; i+=step)
            p->drawLine(x+i, y+offset, x+i, y2-(offset+etchedDisp));

        if(light)
        {
            p->setPen(cols[0]);
            for(i=1; i<space; i+=step)
                p->drawLine(x+i, y+offset+etchedDisp, x+i, y2-offset);
        }
    }
}

static void drawDots(QPainter *p, const QRect &r, bool horiz, int nLines, int offset,
                     const QColor *cols, int startOffset, int dark)
{
    int space((nLines*2)+(nLines-1)),
        x(horiz ? r.x() : r.x()+((r.width()-space)>>1)),
        y(horiz ? r.y()+((r.height()-space)>>1) : r.y()),
        i, j,
        numDots((horiz ? (r.width()-(2*offset))/3 : (r.height()-(2*offset))/3)+1);

    if(horiz)
    {
        if(startOffset && y+startOffset>0)
            y+=startOffset;

        p->setPen(cols[dark]);
        for(i=0; i<space; i+=3)
            for(j=0; j<numDots; j++)
                p->drawPoint(x+offset+(3*j), y+i);

        p->setPen(cols[0]);
        for(i=1; i<space; i+=3)
            for(j=0; j<numDots; j++)
                p->drawPoint(x+offset+1+(3*j), y+i);
    }
    else
    {
        if(startOffset && x+startOffset>0)
            x+=startOffset;

        p->setPen(cols[dark]);
        for(i=0; i<space; i+=3)
            for(j=0; j<numDots; j++)
                p->drawPoint(x+i, y+offset+(3*j));

        p->setPen(cols[0]);
        for(i=1; i<space; i+=3)
            for(j=0; j<numDots; j++)
                p->drawPoint(x+i, y+offset+1+(3*j));
    }
}

inline QColor midColor(const QColor &a, const QColor &b, double factor=1.0)
{
    return QColor((a.red()+limit(b.red()*factor))>>1, 
                  (a.green()+limit(b.green()*factor))>>1, 
                  (a.blue()+limit(b.blue()*factor))>>1);
}

inline QColor tint(const QColor &a, const QColor &b, double factor=0.2)
{
    return QColor((int)((a.red()+(factor*b.red()))/(1+factor)),
                  (int)((a.green()+(factor*b.green()))/(1+factor)),
                  (int)((a.blue()+(factor*b.blue()))/(1+factor)));
}

static QColor shade(const QColor &a, float k)
{
    QColor mod;

    shade(a, &mod, k);
    return mod;
}

static bool isHoriz(const QStyleOption *option, EWidget w)
{
    return WIDGET_BUTTON(w) || option->state&QStyle::State_Horizontal;
}

enum ECacheAppearance
{
    CACHE_APPEARANCE_SELECTED=APPEARANCE_BEVELLED+1
};

inline int app2App(EAppearance app, bool sel)
{
    return sel ? (int)CACHE_APPEARANCE_SELECTED : (int)app;
}

#define QTC_PIXMAP_DIMENSION 10

/*
Cache key:
    type       1  (0 for widget, 1 for pixmap)
    w          5
    top        7
    bot        7
    size      15
    app        3
    horiz      1
    increase   1
    blue       8
    green      8
    red        8
    ------------
              64
*/
#ifdef QTC_INT_CACHE_KEY
static qulonglong double2int(double d) { return (qulonglong)(d*100); }
#else
static int double2int(double d) { return (int)(d*100); }
#endif

static QtcKey createKey(qulonglong size, QRgb color, bool horiz, bool increase=false, int app=0,
                        EWidget w=WIDGET_OTHER, double shadeTop=0.0, double shadeBot=0.0)
{
    if(WIDGET_DEF_BUTTON==w && !IS_GLASS(app)) // Glass uses different shading for def button...
        w=WIDGET_STD_BUTTON;

#ifdef QTC_INT_CACHE_KEY
    return (color&RGB_MASK)+
           ((horiz ? 1 : 0)<<24)+((increase ? 1 : 0)<<25)+((((qulonglong)app)&0x7)<<26)+
           ((size&0x7FFF)<<29)+((double2int(shadeTop)&0x7F)<<44)+((double2int(shadeBot)&0x7F)<<51)+
           ((((qulonglong)w)&0x1F)<<58)+0;
#else
    QString key;

    QTextStream(&key) << size << color << horiz << increase << app << (int)w
                      << ' ' << double2int(shadeTop) << ' ' << double2int(shadeBot);

    return key;
#endif
}

static QtcKey createKey(QRgb color, EPixmap p)
{
#ifdef QTC_INT_CACHE_KEY
    return (((qulonglong)0xFFFFFFFF)<<32)|((p&0xFF)<<24)|(color&RGB_MASK);
#else
    QString key;

    QTextStream(&key) << 'P' << color << p;

    return key;
#endif
}

static void readPal(QString &line, QPalette::ColorGroup grp, QPalette &pal)
{
    QStringList cols=line.mid(line.indexOf("=#")+1).split("^e", QString::SkipEmptyParts);

    if(16==cols.count())
    {
        QStringList::ConstIterator it(cols.begin()),
                                   end(cols.end());

        for(int i=0; it!=end; ++it, ++i)
        {
            QColor col;

            setRgb(&col, (*it).toLatin1().constData());
            pal.setColor(grp, (QPalette::ColorRole)i, col);
        }
    }

#if QT_VERSION >= 0x040300
    pal.setBrush(grp, QPalette::AlternateBase, pal.color(grp, QPalette::Base).darker(110));
#else
    pal.setBrush(grp, QPalette::AlternateBase, pal.color(grp, QPalette::Base).dark(110));
#endif
}

static void setRgb(QColor *col, const QStringList &rgb)
{
    if(3==rgb.size())
        *col=QColor(rgb[0].toInt(), rgb[1].toInt(), rgb[2].toInt());
}

static void parseWindowLine(const QString &line, QList<int> &data)
{
    int len(line.length());

    for(int i=0; i<len; ++i)
        switch(line[i].toLatin1())
        {
            case 'M':
                data.append(QStyle::SC_TitleBarSysMenu);
                break;
            case '_':
                data.append(WINDOWTITLE_SPACER);
                break;
            case 'H':
                data.append(QStyle::SC_TitleBarContextHelpButton);
                break;
            case 'L':
                data.append(QStyle::SC_TitleBarShadeButton);
                break;
            case 'I':
                data.append(QStyle::SC_TitleBarMinButton);
                break;
            case 'A':
                data.append(QStyle::SC_TitleBarMaxButton);
                break;
            case 'X':
                data.append(QStyle::SC_TitleBarCloseButton);
            default:
                break;
        }
}

static bool readQt3(QFile &f, QPalette &pal, QFont &font, int *contrast)
{
    enum ESect
    {
        SECT_NONE,
        SECT_GEN,
        SECT_PAL,
        SECT_KDE
    } sect=SECT_NONE;

    bool gotPal(false),
         gotFont(false),
         gotContrast(false);

    if(f.open(QIODevice::ReadOnly))
    {
        QTextStream in(&f);

        while (!in.atEnd())
        {
            QString line(in.readLine());

            if(SECT_PAL==sect)
            {
                gotPal=true;
                if(0==line.indexOf("active=#", Qt::CaseInsensitive))
                    readPal(line, QPalette::Active, pal);
                else if(0==line.indexOf("disabled=#", Qt::CaseInsensitive))
                    readPal(line, QPalette::Disabled, pal);
                else if(0==line.indexOf("inactive=#", Qt::CaseInsensitive))
                    readPal(line, QPalette::Inactive, pal);
                else if (0==line.indexOf('['))
                    sect=SECT_NONE;
            }
            else if(SECT_GEN==sect)
            {
                if(0==line.indexOf("font=", Qt::CaseInsensitive))
                    gotFont=font.fromString(line.mid(5));
                else if (0==line.indexOf('['))
                    sect=SECT_NONE;
            }
            else if(SECT_KDE==sect)
            {
                if(0==line.indexOf("contrast=", Qt::CaseInsensitive))
                {
                    *contrast=line.mid(9).toInt();
                    gotContrast=true;
                }
                else if (0==line.indexOf('['))
                    sect=SECT_NONE;
            }

            if(SECT_NONE==sect)
            {
                if(0==line.indexOf("[Palette]", Qt::CaseInsensitive))
                    sect=SECT_PAL;
                else if(0==line.indexOf("[General]", Qt::CaseInsensitive))
                    sect=SECT_GEN;
                else if(contrast && 0==line.indexOf("[KDE]", Qt::CaseInsensitive))
                    sect=SECT_KDE;
                if(gotPal && gotFont && (!contrast || gotContrast))
                    break;
            }
        }
        f.close();
    }

    return gotPal && gotFont && (!contrast || gotContrast);
}

static bool useQt3Settings()
{
    static const char *full=getenv("KDE_FULL_SESSION");
    static const char *vers=full ? getenv("KDE_SESSION_VERSION") : 0;

    return full && (!vers || atoi(vers)<4);
}

static bool readQt3(QPalette &pal, QFont &font, int *contrast)
{
    if(useQt3Settings())
    {
        QFile file(QDir::homePath()+QLatin1String("/.qt/qtrc"));

        if(!file.exists() || !readQt3(file, pal, font, contrast))
        {
            file.setFileName("/etc/qt3/qtrc");
            if(!file.exists() || !readQt3(file, pal,font, contrast))
            {
                file.setFileName("/etc/qt/qtrc");
                if(!file.exists() || !readQt3(file, pal, font, contrast))
                    return false;
            }
        }
        return true;
    }
    return false;
}

static const QPushButton * getButton(const QWidget *w, const QPainter *p)
{
    const QWidget *widget=w ? w : (p && p->device() ? dynamic_cast<const QWidget *>(p->device()) : 0L);
    return widget ? ::qobject_cast<const QPushButton *>(widget) : 0L;
}

inline bool isMultiTabBarTab(const QPushButton *button)
{
    return button && button->isFlat() && button->inherits("KMultiTabBarTab");
}

#define QTC_SKIP_TASKBAR  (APP_SKIP_TASKBAR==theThemedApp || APP_KPRINTER==theThemedApp || APP_KDIALOG==theThemedApp)
#define QTC_CAN_DO_EFFECT (QTC_DO_EFFECT && CEtchCheck::canEtch())

QtCurveStyle::QtCurveStyle(const QString &name)
            : itsSliderCols(NULL),
              itsDefBtnCols(NULL),
              itsMouseOverCols(NULL),
              itsSidebarButtonsCols(NULL),
              itsActiveMdiColors(NULL),
              itsMdiColors(NULL),
              itsPixmapCache(150000),
              itsActive(true),
              itsSbWidget(NULL),
              itsProgressBarAnimateTimer(0),
              itsAnimateStep(0),
              itsPos(-1, -1),
              itsHoverWidget(NULL)
{
    QString rcFile;

    defaultSettings(&opts);
    if(!name.isEmpty())
    {
        rcFile=themeFile(kdeHome(), name);

        if(rcFile.isEmpty())
        {
            rcFile=themeFile(KDE_PREFIX(useQt3Settings() ? 3 : 4), name);
            if(rcFile.isEmpty())
                rcFile=themeFile(KDE_PREFIX(useQt3Settings() ? 4 : 3), name);
        }
    }

    readConfig(rcFile, &opts, &opts);
    opts.contrast=QSettings(QLatin1String("Trolltech")).value("/Qt/KDE/contrast", 7).toInt();
    if(opts.contrast<0 || opts.contrast>10)
        opts.contrast=7;

    if(EFFECT_NONE==opts.buttonEffect)
        CEtchCheck::disable();

    shadeColors(QApplication::palette().color(QPalette::Active, QPalette::Highlight), itsMenuitemCols);
    shadeColors(QApplication::palette().color(QPalette::Active, QPalette::Background), itsBackgroundCols);
    shadeColors(QApplication::palette().color(QPalette::Active, QPalette::Button), itsButtonCols);

    if(SHADE_SELECTED==opts.shadeSliders)
        itsSliderCols=itsMenuitemCols;
    else if(SHADE_NONE!=opts.shadeSliders)
    {
        itsSliderCols=new QColor [TOTAL_SHADES+1];
        shadeColors(SHADE_BLEND_SELECTED==opts.shadeSliders
                        ? midColor(itsMenuitemCols[ORIGINAL_SHADE],
                                   itsButtonCols[ORIGINAL_SHADE])
                        : opts.customSlidersColor,
                    itsSliderCols);
    }

    if(IND_TINT==opts.defBtnIndicator)
    {
        itsDefBtnCols=new QColor [TOTAL_SHADES+1];
        shadeColors(tint(itsButtonCols[ORIGINAL_SHADE],
                         itsMenuitemCols[ORIGINAL_SHADE]), itsDefBtnCols);
    }
    else/* if(IND_COLORED==opts.defBtnIndicator)*/
    {
        if(SHADE_BLEND_SELECTED==opts.shadeSliders)
            itsDefBtnCols=itsSliderCols;
        else
        {
            itsDefBtnCols=new QColor [TOTAL_SHADES+1];
            shadeColors(midColor(itsMenuitemCols[ORIGINAL_SHADE],
                                 itsButtonCols[ORIGINAL_SHADE]), itsDefBtnCols);
        }
    }

    if(opts.coloredMouseOver || IND_CORNER==opts.defBtnIndicator)
        if(itsDefBtnCols && IND_TINT!=opts.defBtnIndicator)
            itsMouseOverCols=itsDefBtnCols;
        else
        {
            itsMouseOverCols=new QColor [TOTAL_SHADES+1];
            shadeColors(midColor(itsMenuitemCols[ORIGINAL_SHADE],
                                 itsButtonCols[ORIGINAL_SHADE]), itsMouseOverCols);
        }

    setMenuColors(QApplication::palette().color(QPalette::Active, QPalette::Background));

    if(opts.lighterPopupMenuBgnd)
        itsLighterPopupMenuBgndCol=shade(itsBackgroundCols[ORIGINAL_SHADE],
                                         POPUPMENU_LIGHT_FACTOR);

    switch(opts.shadeCheckRadio)
    {
        default:
            itsCheckRadioCol=QApplication::palette().color(QPalette::Active, QPalette::Text);
            break;
        case SHADE_BLEND_SELECTED:
        case SHADE_SELECTED:
            itsCheckRadioCol=QApplication::palette().color(QPalette::Active, QPalette::Highlight);
            break;
        case SHADE_CUSTOM:
            itsCheckRadioCol=opts.customCheckRadioColor;
    }
}

QtCurveStyle::~QtCurveStyle()
{
    if(itsSidebarButtonsCols &&
       itsSidebarButtonsCols!=itsSliderCols &&
       itsSidebarButtonsCols!=itsDefBtnCols)
        delete [] itsSidebarButtonsCols;
    if(itsActiveMdiColors && itsActiveMdiColors!=itsMenuitemCols)
        delete [] itsActiveMdiColors;
    if(itsMdiColors && itsMdiColors!=itsBackgroundCols)
        delete [] itsMdiColors;
    if(itsMouseOverCols && itsMouseOverCols!=itsDefBtnCols &&
       itsMouseOverCols!=itsSliderCols)
        delete [] itsMouseOverCols;
    if(itsDefBtnCols && itsDefBtnCols!=itsSliderCols)
        delete [] itsDefBtnCols;
    if(itsSliderCols && itsSliderCols!=itsMenuitemCols)
        delete [] itsSliderCols;
}

static QString getFile(const QString &f)
{
    QString d(f);

    int slashPos(d.lastIndexOf('/'));

    if(slashPos!=-1)
        d.remove(0, slashPos+1);

    return d;
}

void QtCurveStyle::polish(QApplication *app)
{
    QString appName(getFile(app->argv()[0]));

    if(opts.fixParentlessDialogs)
    {
        if ("kdefilepicker"==appName)
            theThemedApp=APP_SKIP_TASKBAR;
        else if ("kprinter"==appName)
            theThemedApp=APP_KPRINTER;
        else if ("kdialog"==appName)
            theThemedApp=APP_KDIALOG;
        else if ("kdialogd"==appName)
            theThemedApp=APP_KDIALOGD;
    }
    else
        theThemedApp=APP_OTHER;

    if(APP_OTHER==theThemedApp)
        if("plasma"==appName || "krunner"==appName)
            theThemedApp=APP_PLASMA;
        else if("kwin"==appName)
            theThemedApp=APP_KWIN;

    QPalette pal(app->palette());

    polish(pal);
    app->setPalette(pal);
}

void QtCurveStyle::polish(QPalette &palette)
{
    int      contrast(7);
    bool     newContrast(false);
    QPalette pal;
    QFont    font;

    // Set palette from Qt3 settings.
    if(readQt3(pal, font, &contrast))
    {
        palette=pal;
        QApplication::setFont(font);
    }
    else
        contrast=QSettings(QLatin1String("Trolltech")).value("/Qt/KDE/contrast", 7).toInt();

    if(contrast<0 || contrast>10)
        contrast=7;

    if(contrast!=opts.contrast)
    {
        opts.contrast=contrast;
        newContrast=true;
    }

    bool newMenu(newContrast ||
                 itsMenuitemCols[ORIGINAL_SHADE]!=palette.color(QPalette::Active, QPalette::Highlight)),
         newGray(newContrast ||
                 itsBackgroundCols[ORIGINAL_SHADE]!=palette.color(QPalette::Active, QPalette::Background)),
         newButton(newContrast ||
                   itsButtonCols[ORIGINAL_SHADE]!=palette.color(QPalette::Active, QPalette::Button)),
         newSlider(itsSliderCols && SHADE_BLEND_SELECTED==opts.shadeSliders &&
                   (newContrast || newButton || newMenu)),
         newDefBtn(itsDefBtnCols && ( (IND_COLORED==opts.defBtnIndicator &&
                                       SHADE_BLEND_SELECTED!=opts.shadeSliders) ||
                                      (IND_TINT==opts.defBtnIndicator) ) &&
                   (newContrast || newButton || newMenu)),
         newMouseOver(itsMouseOverCols && itsMouseOverCols!=itsDefBtnCols &&
                      itsMouseOverCols!=itsSliderCols &&
                     (newContrast || newButton || newMenu));

    if(newGray)
        shadeColors(palette.color(QPalette::Active, QPalette::Background), itsBackgroundCols);

    if(newButton)
        shadeColors(palette.color(QPalette::Active, QPalette::Button), itsButtonCols);

    if(newMenu)
        shadeColors(palette.color(QPalette::Active, QPalette::Highlight), itsMenuitemCols);

    setMenuColors(palette.color(QPalette::Active, QPalette::Background));

    if(newSlider)
        shadeColors(midColor(itsMenuitemCols[ORIGINAL_SHADE],
                    itsButtonCols[ORIGINAL_SHADE]), itsSliderCols);

    if(newDefBtn)
        if(IND_TINT==opts.defBtnIndicator)
            shadeColors(tint(itsButtonCols[ORIGINAL_SHADE],
                        itsMenuitemCols[ORIGINAL_SHADE]), itsDefBtnCols);
        else
            shadeColors(midColor(itsMenuitemCols[ORIGINAL_SHADE],
                        itsButtonCols[ORIGINAL_SHADE]), itsDefBtnCols);

    if(newMouseOver)
        shadeColors(midColor(itsMenuitemCols[ORIGINAL_SHADE],
                    itsButtonCols[ORIGINAL_SHADE]), itsMouseOverCols);

    if(itsSidebarButtonsCols && SHADE_BLEND_SELECTED!=opts.shadeSliders &&
       IND_COLORED!=opts.defBtnIndicator)
        shadeColors(midColor(itsMenuitemCols[ORIGINAL_SHADE],
                   itsButtonCols[ORIGINAL_SHADE]), itsSidebarButtonsCols);

    if(opts.lighterPopupMenuBgnd && newGray)
        itsLighterPopupMenuBgndCol=shade(itsBackgroundCols[ORIGINAL_SHADE],
                                         POPUPMENU_LIGHT_FACTOR);

    switch(opts.shadeCheckRadio)
    {
        default:
            itsCheckRadioCol=palette.color(QPalette::Active, QPalette::Text);
            break;
        case SHADE_BLEND_SELECTED:
        case SHADE_SELECTED:
            itsCheckRadioCol=palette.color(QPalette::Active, QPalette::Highlight);
            break;
        case SHADE_CUSTOM:
             itsCheckRadioCol=opts.customCheckRadioColor;
    }

    palette.setColor(QPalette::Active, QPalette::Light, itsBackgroundCols[0]);
    palette.setColor(QPalette::Active, QPalette::Dark, itsBackgroundCols[QT_STD_BORDER]);
    palette.setColor(QPalette::Inactive, QPalette::Light, itsBackgroundCols[0]);
    palette.setColor(QPalette::Inactive, QPalette::Dark, itsBackgroundCols[QT_STD_BORDER]);
    palette.setColor(QPalette::Inactive, QPalette::WindowText, palette.color(QPalette::Active, QPalette::WindowText));
    palette.setColor(QPalette::Disabled, QPalette::Light, itsBackgroundCols[0]);
    palette.setColor(QPalette::Disabled, QPalette::Dark, itsBackgroundCols[QT_STD_BORDER]);

    // Fix KDE4's palette...
    for(int i=QPalette::WindowText; i<QPalette::NColorRoles; ++i)
        if(i!=QPalette::Highlight && i!=QPalette::HighlightedText)
            palette.setColor(QPalette::Inactive, (QPalette::ColorRole)i, palette.color(QPalette::Active, (QPalette::ColorRole)i));

    if(opts.inactiveHighlight)
    {
        palette.setColor(QPalette::Inactive, QPalette::Highlight,
                          midColor(palette.color(QPalette::Active, QPalette::Window),
                                   palette.color(QPalette::Active, QPalette::Highlight), INACTIVE_HIGHLIGHT_FACTOR));
        palette.setColor(QPalette::Inactive, QPalette::HighlightedText, palette.color(QPalette::Active, QPalette::WindowText));
    }
}

void QtCurveStyle::polish(QWidget *widget)
{
    bool enableMouseOver(!equal(opts.highlightFactor, 1.0) || opts.coloredMouseOver);

    if(EFFECT_NONE!=opts.buttonEffect && CEtchCheck::isNoEtchWidget(widget))
    {
        theNoEtchWidgets.insert(static_cast<const QWidget *>(widget));
        connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(widgetDestroyed(QObject *)));
    }

    if(enableMouseOver &&
       (qobject_cast<QPushButton *>(widget) ||
        qobject_cast<QAbstractButton*>(widget) ||
        qobject_cast<QComboBox *>(widget) ||
        qobject_cast<QAbstractSpinBox *>(widget) ||
        qobject_cast<QCheckBox *>(widget) ||
        qobject_cast<QGroupBox *>(widget) ||
        qobject_cast<QRadioButton *>(widget) ||
        qobject_cast<QSplitterHandle *>(widget) ||
        qobject_cast<QSlider *>(widget) ||
        qobject_cast<QScrollBar *>(widget) ||
        qobject_cast<QHeaderView *>(widget) ||
        qobject_cast<QTabBar *>(widget) ||
//        qobject_cast<QDockWidget *>(widget) ||
        widget->inherits("QWorkspaceTitleBar") ||
        widget->inherits("QDockSeparator") ||
        widget->inherits("QDockWidgetSeparator") ||
        widget->inherits("Q3DockWindowResizeHandle")))
        widget->setAttribute(Qt::WA_Hover, true);
    else if (qobject_cast<QProgressBar *>(widget))
    {
        if(widget->palette().color(QPalette::Inactive, QPalette::HighlightedText)!=widget->palette().color(QPalette::Active, QPalette::HighlightedText))
        {
            QPalette pal(widget->palette());
            pal.setColor(QPalette::Inactive, QPalette::HighlightedText, pal.color(QPalette::Active, QPalette::HighlightedText));
            widget->setPalette(pal);
        }
        widget->installEventFilter(this);
    }
    else if (widget->inherits("Q3Header"))
    {
        widget->setMouseTracking(true);
        widget->installEventFilter(this);
    }
#ifdef QTC_HIGHLIGHT_SCROLVIEWS
    else if(widget->inherits("Q3ScrollView"))
        widget->installEventFilter(this);
#endif
    else if(qobject_cast<QMenuBar *>(widget))
    {
        widget->setAttribute(Qt::WA_Hover, true);

        if(opts.shadeMenubarOnlyWhenActive && SHADE_NONE!=opts.shadeMenubars)
            widget->installEventFilter(this);
    }
    else if(opts.fixParentlessDialogs)
        if(APP_KPRINTER==theThemedApp || APP_KDIALOG==theThemedApp || APP_KDIALOGD==theThemedApp)
        {
            QString cap(widget->windowTitle());
            int     index=-1;

            // Remove horrible "Open - KDialog" titles...
            if( cap.length() &&
                ( (APP_KPRINTER==theThemedApp && (-1!=(index=cap.indexOf(" - KPrinter"))) &&
                    (index+11)==(int)cap.length()) ||
                  (APP_KDIALOG==theThemedApp && (-1!=(index=cap.indexOf(" - KDialog"))) &&
                    (index+10)==(int)cap.length()) ||
                  (APP_KDIALOGD==theThemedApp && (-1!=(index=cap.indexOf(" - KDialog Daemon"))) &&
                    (index+17)==(int)cap.length())) )
                widget->QWidget::setWindowTitle(cap.left(index));
             widget->installEventFilter(this);
        }
        else if(qobject_cast<QDialog *>(widget) && widget->windowFlags()&Qt::WindowType_Mask &&
                (!widget->parentWidget()) /*|| widget->parentWidget()->isHidden())*/)
        {
            QWidget *activeWindow=qApp->activeWindow();

            if(activeWindow && activeWindow!=widget)
            {
                itsReparentedDialogs[widget]=widget->parentWidget();
                widget->setParent(activeWindow, widget->windowFlags());
            }
            widget->installEventFilter(this);
        }

    if (!widget->isWindow())
        if (QFrame *frame = qobject_cast<QFrame *>(widget))
        {
            // kill ugly frames...
            if (QFrame::Box==frame->frameShape() || QFrame::Panel==frame->frameShape() || QFrame::WinPanel==frame->frameShape())
                frame->setFrameShape(QFrame::StyledPanel);
            else if (QFrame::HLine==frame->frameShape() || QFrame::VLine==frame->frameShape())
                 widget->installEventFilter(this);

            if(widget->parent() && widget->parent()->inherits("KTitleWidget"))
            {
                frame->setBackgroundRole(QPalette::Background);

                QLayout *layout(frame->layout());

                if(layout)
                    layout->setMargin(0);
            }

            QWidget *p=NULL;

            if(opts.gtkComboMenus && widget->parentWidget() && (p=widget->parentWidget()->parentWidget()) &&
               qobject_cast<QComboBox *>(p) && !((QComboBox *)(p))->isEditable())
            {
                QPalette pal(widget->palette());

                pal.setBrush(QPalette::Active, QPalette::Base, opts.lighterPopupMenuBgnd ? itsLighterPopupMenuBgndCol : itsBackgroundCols[ORIGINAL_SHADE]);
                widget->setPalette(pal);
            }
        }

    bool onToolBar(widget && widget->parent() && (qobject_cast<QToolBar *>(widget->parent()) || widget->parent()->inherits("Q3ToolBar")));

    if (qobject_cast<QMenuBar *>(widget) ||
        widget->inherits("Q3ToolBar") ||
        qobject_cast<QToolBar *>(widget) ||
        onToolBar)
        widget->setBackgroundRole(QPalette::Window);

    if(!IS_FLAT(opts.toolbarAppearance) && onToolBar)
        widget->setAutoFillBackground(false);
}

void QtCurveStyle::unpolish(QWidget *widget)
{
    if(EFFECT_NONE!=opts.buttonEffect && theNoEtchWidgets.contains(widget))
    {
        theNoEtchWidgets.remove(static_cast<const QWidget *>(widget));
        disconnect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(widgetDestroyed(QObject *)));
    }

    if(qobject_cast<QPushButton *>(widget) ||
       qobject_cast<QComboBox *>(widget) ||
       qobject_cast<QAbstractSpinBox *>(widget) ||
       qobject_cast<QCheckBox *>(widget) ||
       qobject_cast<QGroupBox *>(widget) ||
       qobject_cast<QRadioButton *>(widget) ||
       qobject_cast<QSplitterHandle *>(widget) ||
       qobject_cast<QSlider *>(widget) ||
       qobject_cast<QScrollBar *>(widget) ||
       qobject_cast<QHeaderView *>(widget) ||
       qobject_cast<QTabBar *>(widget) ||
//       qobject_cast<QDockWidget *>(widget) ||
       widget->inherits("QWorkspaceTitleBar") ||
       widget->inherits("QDockSeparator") ||
       widget->inherits("QDockWidgetSeparator") ||
       widget->inherits("Q3DockWindowResizeHandle"))
        widget->setAttribute(Qt::WA_Hover, false);
    else if (qobject_cast<QProgressBar *>(widget))
        widget->removeEventFilter(this);
    else if (widget->inherits("Q3Header"))
    {
        widget->setMouseTracking(false);
        widget->removeEventFilter(this);
    }
#ifdef QTC_HIGHLIGHT_SCROLVIEWS
    else if(widget->inherits("Q3ScrollView"))
        widget->removeEventFilter(this);
#endif
    else if(qobject_cast<QMenuBar *>(widget))
    {
        widget->setAttribute(Qt::WA_Hover, false);

        if(opts.shadeMenubarOnlyWhenActive && SHADE_NONE!=opts.shadeMenubars)
            widget->removeEventFilter(this);
    }
    else if(opts.fixParentlessDialogs && qobject_cast<QDialog *>(widget))
        widget->removeEventFilter(this);
    if (!widget->isWindow())
        if (QFrame *frame = qobject_cast<QFrame *>(widget))
        {
            if (QFrame::HLine==frame->frameShape() || QFrame::VLine==frame->frameShape())
                 widget->removeEventFilter(this);

            if(widget->parent() && widget->parent()->inherits("KTitleWidget"))
            {
                frame->setBackgroundRole(QPalette::Base);

                QLayout *layout(frame->layout());

                if(layout)
                    layout->setMargin(6);
            }
        }

   if (qobject_cast<QMenuBar *>(widget) ||
        widget->inherits("Q3ToolBar") ||
        qobject_cast<QToolBar *>(widget) ||
        (widget && qobject_cast<QToolBar *>(widget->parent())))
        widget->setBackgroundRole(QPalette::Button);
}

bool QtCurveStyle::eventFilter(QObject *object, QEvent *event)
{
    switch(event->type())
    {
        case QEvent::Paint:
        {
            QFrame *frame = qobject_cast<QFrame*>(object);

            if (frame && (QFrame::HLine==frame->frameShape() || QFrame::VLine==frame->frameShape()))
            {
                QPainter painter(frame);
                QPoint   p1, p2;
                QRect    r(frame->rect());

                if (QFrame::HLine==frame->frameShape())
                {
                    p1 = QPoint(r.x(), r.height() / 2);
                    p2 = QPoint(r.x() + r.width(), p1.y());
                }
                else
                {
                    p1 = QPoint(r.x()+r.width() / 2, 0);
                    p2 = QPoint(p1.x(), r.height());
                }

                if (QFrame::Plain==frame->frameShadow())
                {
                    painter.setPen(QPen(frame->palette().dark().color(), frame->lineWidth()));
                    painter.drawLine(p1, p2);
                }
                else
                    qDrawShadeLine(&painter, p1, p2, frame->palette(), QFrame::Sunken==frame->frameShadow(), frame->lineWidth(), frame->midLineWidth());
                return true;
            }
            break;
        }
        case QEvent::Show:
        {
            QProgressBar *bar = qobject_cast<QProgressBar *>(object);

            if(bar)
            {
                itsProgressBars.append(bar);
                if (1==itsProgressBars.size())
                {
                    itsTimer.start();
                    itsProgressBarAnimateTimer = startTimer(1000 / constProgressBarFps);
                }
            }
//             else if (QFrame *frame = qobject_cast<QFrame *>(object) &&
//                     (QFrame::Box==frame->frameShape() || QFrame::Panel==frame->frameShape() || QFrame::WinPanel==frame->frameShape()))
//             {
//                 // This catches the case where the frame is created, and then its style set...
//                     frame->setFrameShape(QFrame::StyledPanel);
//             }
            break;
        }
        case QEvent::Destroy:
        case QEvent::Hide:
        {
            QProgressBar *bar = qobject_cast<QProgressBar *>(object);

            if(itsHoverWidget && object==itsHoverWidget)
            {
                itsPos.setX(-1);
                itsPos.setY(-1);
                itsHoverWidget=NULL;
            }

            if(bar && !itsProgressBars.isEmpty())
            {
                itsProgressBars.removeAll(bar);
                if (itsProgressBars.isEmpty())
                {
                    killTimer(itsProgressBarAnimateTimer);
                    itsProgressBarAnimateTimer = 0;
                }
            }

            if(opts.fixParentlessDialogs &&
               qobject_cast<QDialog *>(object) &&
               itsReparentedDialogs.contains((QWidget*)object))
            {
                QWidget *widget=(QWidget*)object;

                // OK, reset back to its original parent..
                if(widget->windowFlags()&Qt::WindowType_Mask)
                {
                    widget->removeEventFilter(this);
                    widget->setParent(itsReparentedDialogs[widget]);
                    widget->installEventFilter(this);
                }
                itsReparentedDialogs.remove(widget);
            }
            break;
        }
        case QEvent::Enter:
            if(object->isWidgetType() && object->inherits("Q3Header"))
            {
                itsHoverWidget=(QWidget *)object;

                if(itsHoverWidget && !itsHoverWidget->isEnabled())
                    itsHoverWidget=NULL;
            }
            break;
        case QEvent::Leave:
            if(itsHoverWidget && object==itsHoverWidget)
            {
                itsPos.setX(-1);
                itsPos.setY(-1);
                itsHoverWidget=NULL;
                ((QWidget *)object)->repaint();
            }
            break;
        case QEvent::MouseMove:  // Only occurs for widgets with mouse tracking enabled
        {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);

            if(me && itsHoverWidget && object->isWidgetType() && object->inherits("Q3Header"))
            {
                if(!me->pos().isNull() && me->pos()!=itsPos)
                    itsHoverWidget->repaint();
                itsPos=me->pos();
            }
            break;
        }
#ifdef QTC_HIGHLIGHT_SCROLVIEWS
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            if(object->isWidgetType() && object->inherits("Q3ScrollView"))
                ((QWidget *)object)->repaint();
            break;
#endif
        case QEvent::WindowActivate:
            if(opts.shadeMenubarOnlyWhenActive && SHADE_NONE!=opts.shadeMenubars && qobject_cast<QMenuBar *>(object))
            {
                itsActive=true;
                ((QWidget *)object)->repaint();
                return false;
            }
            break;
        case QEvent::WindowDeactivate:
            if(opts.shadeMenubarOnlyWhenActive && SHADE_NONE!=opts.shadeMenubars && qobject_cast<QMenuBar *>(object))
            {
                itsActive=false;
                ((QWidget *)object)->repaint();
                return false;
            }
            break;
        case 70: // QEvent::ChildInserted - QT3_SUPPORT
            if(opts.fixParentlessDialogs && qobject_cast<QDialog *>(object))
            {
                QDialog *dlg=(QDialog *)object;

                // The parent->isHidden is needed for KWord. It's insert picture file dialog is a
                // child of the insert picture dialog - but the file dialog is shown *before* the
                // picture dialog!
                if(dlg && dlg->windowFlags()&Qt::WindowType_Mask && (!dlg->parentWidget() || dlg->parentWidget()->isHidden()))
                {
                    QWidget *activeWindow=qApp->activeWindow();

                    if(activeWindow && activeWindow!=dlg)
                    {
                        dlg->removeEventFilter(this);
                        dlg->setParent(activeWindow, dlg->windowFlags());
                        dlg->installEventFilter(this);
                        itsReparentedDialogs[(QWidget *)dlg]=dlg->parentWidget();
                        return false;
                    }
                }
            }
        default:
            break;
    }

    return QTC_BASE_STYLE::eventFilter(object, event);
}

void QtCurveStyle::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == itsProgressBarAnimateTimer)
    {
        itsAnimateStep = itsTimer.elapsed() / (1000 / constProgressBarFps);
        foreach (QProgressBar *bar, itsProgressBars)
            if ((opts.animatedProgress && 0==itsAnimateStep%2) || (0==bar->minimum() && 0==bar->maximum()))
                bar->update();
    }

    event->ignore();
}

int QtCurveStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch(metric)
    {
        case PM_MenuBarVMargin:
        case PM_MenuBarHMargin:
            return 3;
        case PM_MenuButtonIndicator:
            return 15;
        case PM_ButtonMargin:
            return 3;
        case PM_TabBarTabShiftVertical:
#ifdef QTC_STYLE_QTABBAR
            if(widget && widget->parentWidget() && !qobject_cast<const QTabWidget *>(widget->parentWidget()))
                return option && option->state & State_Selected ? 0 : -2;
#endif
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option))
            {
                 if(QTabBar::RoundedSouth==tab->shape || QTabBar::TriangularSouth==tab->shape)
                    return -2;
            }
            return 2;
        case PM_TabBarTabShiftHorizontal:
#ifdef QTC_STYLE_QTABBAR
            if(widget && widget->parentWidget() && !qobject_cast<const QTabWidget *>(widget->parentWidget()))
                return option && option->state & State_Selected ? 0 : -1;
#endif
            return 0;
        case PM_ButtonShiftHorizontal:
            // return Qt::RightToLeft==QApplication::layoutDirection() ? -1 : 1;
        case PM_ButtonShiftVertical:
            return 1;
        case PM_ButtonDefaultIndicator:
            return 0;
        case PM_DefaultFrameWidth:
//             if (widget && widget->inherits("QComboBoxPrivateContainer"))
//                 return 1;
//            if (qobject_cast<const QMenu *>(widget))
//                return 1;
//             if(QTC_CAN_DO_EFFECT && widget && // !isFormWidget(widget) &&
//                (::qobject_cast<const QLineEdit *>(widget) || // ::qobject_cast<const QDateTimeEditBase*>(widget) ||
//                 ::qobject_cast<const QTextEdit*>(widget)))
//                 return 3;
//             else
                return 2;
        case PM_SpinBoxFrameWidth:
            return QTC_CAN_DO_EFFECT ? 3 : 2;
        case PM_IndicatorWidth:
        case PM_IndicatorHeight:
            return QTC_CHECK_SIZE;
        case PM_ExclusiveIndicatorWidth:
        case PM_ExclusiveIndicatorHeight:
            return QTC_RADIO_SIZE;
        case PM_TabBarTabOverlap:
            return 1;
        case PM_ProgressBarChunkWidth:
            return 4;
//         case PM_DockWindowSeparatorExtent:
//             return 4;
//         case PM_DockWindowHandleExtent:
//             return 10;
        case PM_SplitterWidth:
            return 6;
        case PM_ScrollBarSliderMin:
            return 16;
        case PM_SliderThickness:
            return 21;
        case PM_SliderControlThickness:
            return SLIDER_TRIANGULAR==opts.sliderStyle ? 11 : 13;
         case PM_SliderTickmarkOffset:
             return SLIDER_TRIANGULAR==opts.sliderStyle ? 5 : 4;
        case PM_SliderSpaceAvailable:
            if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
            {
                int size(SLIDER_TRIANGULAR==opts.sliderStyle ? 17 : 13);

                if (slider->tickPosition & QSlider::TicksBelow)
                    ++size;
                if (slider->tickPosition & QSlider::TicksAbove)
                    ++size;
                return size;
            }
            return QTC_BASE_STYLE::pixelMetric(metric, option, widget);
        case PM_SliderLength:
            return SLIDER_TRIANGULAR==opts.sliderStyle ? 11 : 21;
        case PM_ScrollBarExtent:
            return /*APP_KPRESENTER==theThemedApp ||
                   ((APP_KONQUEROR==theThemedApp || APP_KONTACT==theThemedApp) && (!widget || isFormWidget(widget)))
                        ? 16 : */ 15;
        case PM_MaximumDragDistance:
            return -1;
        case PM_TabBarTabHSpace:
            return 20;
        case PM_TabBarTabVSpace:
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option))
                if (!tab->icon.isNull())
                    return opts.highlightTab ? 12 : 10;
            return opts.highlightTab ? 10 : 8;
        case PM_TitleBarHeight:
            return qMax(widget ? widget->fontMetrics().lineSpacing()
                               : option ? option->fontMetrics.lineSpacing()
                                        : 0, 24);
        case QtC_Round:
            return (int)opts.round;
        case QtC_TitleBarAppearance:
            return (int)opts.titlebarAppearance;

// The following is a somewhat hackyish fix for konqueror's show close button on tab setting...
// ...its hackish in the way that I'm assuming when KTabBar is positioning the close button and it
// asks for these options, it only passes in a QStyleOption  not a QStyleOptionTab
//.........
        case PM_TabBarBaseHeight:
            if(widget && widget->inherits("KTabBar") && !qstyleoption_cast<const QStyleOptionTab *>(option))
                return 10;
            return QTC_BASE_STYLE::pixelMetric(metric, option, widget);
        case PM_TabBarBaseOverlap:
            if(widget && widget->inherits("KTabBar") && !qstyleoption_cast<const QStyleOptionTab *>(option))
                return 0;
            // Fall through!
//.........
        default:
            return QTC_BASE_STYLE::pixelMetric(metric, option, widget);
    }
}

int QtCurveStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                            QStyleHintReturn *returnData) const
{
    switch (hint)
    {
          //case SH_ItemView_ChangeHighlightOnFocus: // Gray out selected items when losing focus.
          //  return true;
        case SH_TabBar_Alignment:
            return Qt::AlignLeft;
        case SH_Header_ArrowAlignment:
            return Qt::AlignLeft;
        case SH_ProgressDialog_CenterCancelButton:
            return false;
        case SH_PrintDialog_RightAlignButtons:
            return true;
        case SH_DitherDisabledText:
            return false;
        case SH_EtchDisabledText:
            return true;
        case SH_WindowFrame_Mask:
            if (QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask *>(returnData))
            {
                QRect r(option->rect);

                mask->region = r;
                mask->region -= QRect(r.left(), r.top(), 2, 1);
                mask->region -= QRect(r.right() - 1, r.top(), 2, 1);
                mask->region -= QRect(r.left(), r.top() + 1, 1, 1);
                mask->region -= QRect(r.right(), r.top() + 1, 1, 1);

                const QStyleOptionTitleBar *titleBar = qstyleoption_cast<const QStyleOptionTitleBar *>(option);
                if (titleBar && (titleBar->titleBarState & Qt::WindowMinimized))
                {
                    mask->region -= QRect(r.left(), r.bottom(), 2, 1);
                    mask->region -= QRect(r.right() - 1, r.bottom(), 2, 1);
                    mask->region -= QRect(r.left(), r.bottom() - 1, 1, 1);
                    mask->region -= QRect(r.right(), r.bottom() - 1, 1, 1);
                }
                else
                {
                    mask->region -= QRect(r.bottomLeft(), QSize(1, 1));
                    mask->region -= QRect(r.bottomRight(), QSize(1, 1));
                }
            }
            return 1;
        case SH_TitleBar_NoBorder:
            return 1;
        case SH_TitleBar_AutoRaise:
            return 1;
        case SH_ItemView_ShowDecorationSelected:
            return false; // Controls whether the highlighting of listview/treeview items highlights whole line.
        case SH_ToolBox_SelectedPageTitleBold:
        case SH_ScrollBar_MiddleClickAbsolutePosition:
            return true;
        case SH_MainWindow_SpaceBelowMenuBar:
            return 0;
        case SH_DialogButtonLayout:
            return opts.gtkButtonOrder ? QDialogButtonBox::GnomeLayout : QDialogButtonBox::KdeLayout;
        case SH_MessageBox_TextInteractionFlags:
            return Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse;
        case SH_LineEdit_PasswordCharacter:
            if(opts.passwordChar)
            {
                int                chars[4]={opts.passwordChar, 0x25CF, 0x2022, 0};
                const QFontMetrics &fm(option ? option->fontMetrics
                                        : (widget ? widget->fontMetrics() : QFontMetrics(QFont())));
                for(int i=0; chars[i]; ++i)
                    if (fm.inFont(QChar(chars[i])))
                        return chars[i];
                return '*';
            }
            else
                return '\0';
//        case SH_ItemView_ArrowKeysNavigateIntoChildren:
//            return false;
        case SH_MenuBar_MouseTracking:
            return opts.menubarMouseOver ? 1 : 0;
        case SH_ScrollView_FrameOnlyAroundContents:
            return opts.gtkScrollViews && (!widget || !widget->inherits("QComboBoxListView"));
        case SH_ComboBox_Popup:
            if(opts.gtkComboMenus)
            {
                if (widget && widget->inherits("Q3ComboBox"))
                    return 0;
                if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(option))
                    return !cmb->editable;
            }
            return 0;
        default:
            return QTC_BASE_STYLE::styleHint(hint, option, widget, returnData);
   }
}

QPalette QtCurveStyle::standardPalette() const
{
    QPalette palette;
    QFont    font;

    if(!readQt3(palette, font, NULL))
    {
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, QColor(QRgb(0xff808080)));
        palette.setBrush(QPalette::Disabled, QPalette::Button, QColor(QRgb(0xffdddfe4)));
        palette.setBrush(QPalette::Disabled, QPalette::Light, QColor(QRgb(0xffffffff)));
        palette.setBrush(QPalette::Disabled, QPalette::Midlight, QColor(QRgb(0xffffffff)));
        palette.setBrush(QPalette::Disabled, QPalette::Dark, QColor(QRgb(0xff555555)));
        palette.setBrush(QPalette::Disabled, QPalette::Mid, QColor(QRgb(0xffc7c7c7)));
        palette.setBrush(QPalette::Disabled, QPalette::Text, QColor(QRgb(0xffc7c7c7)));
        palette.setBrush(QPalette::Disabled, QPalette::BrightText, QColor(QRgb(0xffffffff)));
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, QColor(QRgb(0xff808080)));
        palette.setBrush(QPalette::Disabled, QPalette::Base, QColor(QRgb(0xffefefef)));
#if QT_VERSION >= 0x040300
        palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, palette.color(QPalette::Disabled, QPalette::Base).darker(110));
#else
        palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, palette.color(QPalette::Disabled, QPalette::Base).dark(110));
#endif
        palette.setBrush(QPalette::Disabled, QPalette::Window, QColor(QRgb(0xffefefef)));
        palette.setBrush(QPalette::Disabled, QPalette::Shadow, QColor(QRgb(0xff000000)));
        palette.setBrush(QPalette::Disabled, QPalette::Highlight, QColor(QRgb(0xff567594)));
        palette.setBrush(QPalette::Disabled, QPalette::HighlightedText, QColor(QRgb(0xffffffff)));
        palette.setBrush(QPalette::Disabled, QPalette::Link, QColor(QRgb(0xff0000ee)));
        palette.setBrush(QPalette::Disabled, QPalette::LinkVisited, QColor(QRgb(0xff52188b)));
        palette.setBrush(QPalette::Active, QPalette::WindowText, QColor(QRgb(0xff000000)));
        palette.setBrush(QPalette::Active, QPalette::Button, QColor(QRgb(0xffdddfe4)));
        palette.setBrush(QPalette::Active, QPalette::Light, QColor(QRgb(0xffffffff)));
        palette.setBrush(QPalette::Active, QPalette::Midlight, QColor(QRgb(0xffffffff)));
        palette.setBrush(QPalette::Active, QPalette::Dark, QColor(QRgb(0xff555555)));
        palette.setBrush(QPalette::Active, QPalette::Mid, QColor(QRgb(0xffc7c7c7)));
        palette.setBrush(QPalette::Active, QPalette::Text, QColor(QRgb(0xff000000)));
        palette.setBrush(QPalette::Active, QPalette::BrightText, QColor(QRgb(0xffffffff)));
        palette.setBrush(QPalette::Active, QPalette::ButtonText, QColor(QRgb(0xff000000)));
        palette.setBrush(QPalette::Active, QPalette::Base, QColor(QRgb(0xffffffff)));
#if QT_VERSION >= 0x040300
        palette.setBrush(QPalette::Active, QPalette::AlternateBase, palette.color(QPalette::Active, QPalette::Base).darker(110));
#else
        palette.setBrush(QPalette::Active, QPalette::AlternateBase, palette.color(QPalette::Active, QPalette::Base).dark(110));
#endif
        palette.setBrush(QPalette::Active, QPalette::Window, QColor(QRgb(0xffefefef)));
        palette.setBrush(QPalette::Active, QPalette::Shadow, QColor(QRgb(0xff000000)));
        palette.setBrush(QPalette::Active, QPalette::Highlight, QColor(QRgb(0xff678db2)));
        palette.setBrush(QPalette::Active, QPalette::HighlightedText, QColor(QRgb(0xffffffff)));
        palette.setBrush(QPalette::Active, QPalette::Link, QColor(QRgb(0xff0000ee)));
        palette.setBrush(QPalette::Active, QPalette::LinkVisited, QColor(QRgb(0xff52188b)));
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, QColor(QRgb(0xff000000)));
        palette.setBrush(QPalette::Inactive, QPalette::Button, QColor(QRgb(0xffdddfe4)));
        palette.setBrush(QPalette::Inactive, QPalette::Light, QColor(QRgb(0xffffffff)));
        palette.setBrush(QPalette::Inactive, QPalette::Midlight, QColor(QRgb(0xffffffff)));
        palette.setBrush(QPalette::Inactive, QPalette::Dark, QColor(QRgb(0xff555555)));
        palette.setBrush(QPalette::Inactive, QPalette::Mid, QColor(QRgb(0xffc7c7c7)));
        palette.setBrush(QPalette::Inactive, QPalette::Text, QColor(QRgb(0xff000000)));
        palette.setBrush(QPalette::Inactive, QPalette::BrightText, QColor(QRgb(0xffffffff)));
        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, QColor(QRgb(0xff000000)));
        palette.setBrush(QPalette::Inactive, QPalette::Base, QColor(QRgb(0xffffffff)));
#if QT_VERSION >= 0x040300
        palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, palette.color(QPalette::Inactive, QPalette::Base).darker(110));
#else
        palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, palette.color(QPalette::Inactive, QPalette::Base).dark(110));
#endif
        palette.setBrush(QPalette::Inactive, QPalette::Window, QColor(QRgb(0xffefefef)));
        palette.setBrush(QPalette::Inactive, QPalette::Shadow, QColor(QRgb(0xff000000)));
        palette.setBrush(QPalette::Inactive, QPalette::Highlight, QColor(QRgb(0xff678db2)));
        palette.setBrush(QPalette::Inactive, QPalette::HighlightedText, QColor(QRgb(0xffffffff)));
        palette.setBrush(QPalette::Inactive, QPalette::Link, QColor(QRgb(0xff0000ee)));
        palette.setBrush(QPalette::Inactive, QPalette::LinkVisited, QColor(QRgb(0xff52188b)));
    }
    return palette;
}

QPixmap QtCurveStyle::standardPixmap(StandardPixmap pix, const QStyleOption *opttion, const QWidget *widget) const
{
    return QTC_BASE_STYLE::standardPixmap(pix, opttion, widget);
}

void QtCurveStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                                 const QWidget *widget) const
{
    QRect                 r(option->rect);
    const QFlags<State> & state(option->state);
    const QPalette &      palette(option->palette);
    bool                  reverse(Qt::RightToLeft==option->direction);

    switch (element)
    {
        case PE_IndicatorBranch:
        {
            int middleH((r.x() + r.width() / 2)-1),
                middleV(r.y() + r.height() / 2),
                beforeH(middleH),
                beforeV(middleV),
                afterH(middleH),
                afterV(middleV);

            painter->save();

            if (state&State_Children)
            {
                QRect ar(r.x()+((r.width()-(QTC_LV_SIZE+4))>>1), r.y()+((r.height()-(QTC_LV_SIZE+4))>>1), QTC_LV_SIZE+4,
                         QTC_LV_SIZE+4);

                beforeH=ar.x();
                beforeV=ar.y();
                afterH=ar.x()+QTC_LV_SIZE+4;
                afterV=ar.y()+QTC_LV_SIZE+4;

                if(opts.lvLines)
                {
                    int lo(QTC_ROUNDED ? 2 : 0);

                    painter->setPen(palette.mid().color());
                    painter->drawLine(ar.x()+lo, ar.y(), (ar.x()+ar.width()-1)-lo, ar.y());
                    painter->drawLine(ar.x()+lo, ar.y()+ar.height()-1, (ar.x()+ar.width()-1)-lo,
                                      ar.y()+ar.height()-1);
                    painter->drawLine(ar.x(), ar.y()+lo, ar.x(), (ar.y()+ar.height()-1)-lo);
                    painter->drawLine(ar.x()+ar.width()-1, ar.y()+lo, ar.x()+ar.width()-1,
                                      (ar.y()+ar.height()-1)-lo);

                    if(QTC_ROUNDED)
                    {
                        painter->drawPoint(ar.x()+1, ar.y()+1);
                        painter->drawPoint(ar.x()+1, ar.y()+ar.height()-2);
                        painter->drawPoint(ar.x()+ar.width()-2, ar.y()+1);
                        painter->drawPoint(ar.x()+ar.width()-2, ar.y()+ar.height()-2);

                        QColor col(palette.mid().color());

                        col.setAlphaF(0.5);
                        painter->setPen(col);
                        painter->drawLine(ar.x()+1, ar.y()+1, ar.x()+2, ar.y());
                        painter->drawLine(ar.x()+ar.width()-2, ar.y(), ar.x()+ar.width()-1, ar.y()+1);
                        painter->drawLine(ar.x()+1, ar.y()+ar.height()-2, ar.x()+2, ar.y()+ar.height()-1);
                        painter->drawLine(ar.x()+ar.width()-2, ar.y()+ar.height()-1, ar.x()+ar.width()-1,
                                          ar.y()+ar.height()-2);
                    }
                }

                QStyleOption opt(*option);

                opt.state|=State_Enabled;

                drawArrow(painter, ar, &opt, state&State_Open
                                                ? PE_IndicatorArrowDown
                                                : reverse
                                                    ? PE_IndicatorArrowLeft
                                                    : PE_IndicatorArrowRight);
            }

            if(opts.lvLines)
            {
                painter->setPen(palette.mid().color());
                if (state&State_Item)
                    if (reverse)
                        painter->drawLine(r.left(), middleV, afterH, middleV);
                    else
                        painter->drawLine(afterH, middleV, r.right(), middleV);
                if (state&State_Sibling && afterV<r.bottom())
                    painter->drawLine(middleH, afterV, middleH, r.bottom());
                if (state & (State_Open | State_Children | State_Item | State_Sibling) && beforeV>r.y())
                    painter->drawLine(middleH, r.y(), middleH, beforeV);
            }
            painter->restore();
            break;
        }
        case PE_IndicatorViewItemCheck:
        {
//             QStyleOptionButton button;
// 
//             button.QStyleOption::operator=(*option);
//             button.state &= ~State_MouseOver;
            drawPrimitive(PE_IndicatorCheckBox, /*&button*/option, painter, widget);
            break;
        }
        case PE_IndicatorHeaderArrow:
            if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option))
                drawArrow(painter, r, option, header->sortIndicator & QStyleOptionHeader::SortUp ? PE_IndicatorArrowUp : PE_IndicatorArrowDown);
            break;
        case PE_IndicatorArrowUp:
        case PE_IndicatorArrowDown:
        case PE_IndicatorArrowLeft:
        case PE_IndicatorArrowRight:
            if(state&(State_Sunken|State_On))
                r.adjust(1, 1, 1, 1);
            drawArrow(painter, r, option, element);
            break;
        case PE_IndicatorSpinMinus:
        case PE_IndicatorSpinPlus:
        case PE_IndicatorSpinUp:
        case PE_IndicatorSpinDown:
        {
            QRect        sr(r);
            const QColor *use(buttonColors(option));
            bool         down(PE_IndicatorSpinDown==element || PE_IndicatorSpinMinus==element),
                         doEtch(QTC_CAN_DO_EFFECT);

            if(doEtch)
                if(down)
                    sr.adjust(0, 0, 0, -1);
                else
                    sr.adjust(0, 1, 0, 0);

            drawLightBevel(painter, sr, option, down
                                                  ? reverse
                                                        ? ROUNDED_BOTTOMLEFT
                                                        : ROUNDED_BOTTOMRIGHT
                                                    : reverse
                                                        ? ROUNDED_TOPLEFT
                                                        : ROUNDED_TOPRIGHT,
                           getFill(option, use), use, true, WIDGET_SPIN);

            if(doEtch)
            {
                QRect er(r);
                painter->setClipRegion(er);
                if(reverse)
                    er.adjust(0, 0, 2, 0);
                else
                    er.adjust(-2, 0, 0, 0);
                drawEtch(painter, er, /*option, */!down, down);
                painter->setClipping(false);
            }

            if(PE_IndicatorSpinUp==element || PE_IndicatorSpinDown==element)
            {
                sr.setY(sr.y()+(down ? -2 : 1));

                if(state&State_Sunken)
                    sr.adjust(1, 1, 1, 1);

                drawArrow(painter, sr, option, PE_IndicatorSpinUp==element ? PE_IndicatorArrowUp : PE_IndicatorArrowDown, true);
            }
            else
            {
                int    l(qMin(r.width()-6, r.height()-6));
                QPoint c(r.x()+(r.width()/2), r.y()+(r.height()/2));

                l/=2;
                if(l%2 != 0)
                    --l;

                if(state&State_Sunken)
                    c+=QPoint(1, 1);

                painter->setPen(palette.buttonText().color());
                painter->drawLine(c.x()-l, c.y(), c.x()+l, c.y());
                if(!down)
                    painter->drawLine(c.x(), c.y()-l, c.x(), c.y()+l);
            }
            break;
        }
        case PE_IndicatorToolBarSeparator:
        {
            painter->save();
            switch(opts.toolbarSeparators)
            {
                case LINE_NONE:
                    break;
                case LINE_FLAT:
                case LINE_SUNKEN:
                    if(r.width()<r.height())
                    {
                        int x(r.x()+((r.width()-2) / 2));

                        painter->setPen(itsBackgroundCols[LINE_SUNKEN==opts.toolbarSeparators ? 3 : 4]);
                        painter->drawLine(x, r.y()+6, x, r.y()+r.height()-7);
                        if(LINE_SUNKEN==opts.toolbarSeparators)
                        {
                            painter->setPen(itsBackgroundCols[0]);
                            painter->drawLine(x+1, r.y()+6, x+1, r.y()+r.height()-7);
                        }
                    }
                    else
                    {
                        int y(r.y()+((r.height()-2) / 2));

                        painter->setPen(itsBackgroundCols[LINE_SUNKEN==opts.toolbarSeparators ? 3 : 4]);
                        painter->drawLine(r.x()+6, y, r.x()+r.width()-7, y);
                        if(LINE_SUNKEN==opts.toolbarSeparators)
                        {
                            painter->setPen(itsBackgroundCols[0]);
                            painter->drawLine(r.x()+6, y+1, r.x()+r.width()-7, y+1);
                        }
                    }
                    break;
                default:
                case LINE_DOTS:
                    drawDots(painter, r, !(state&State_Horizontal), 1, 5, itsBackgroundCols, 0, 5);
            }
            painter->restore();
            break;
        }
        case PE_FrameGroupBox:
            if(opts.framelessGroupBoxes)
                break;
            if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(option))
            {
                QStyleOptionFrameV2 frameV2(*frame);
                if (frameV2.features & QStyleOptionFrameV2::Flat)
                {
                    QPen oldPen = painter->pen();
                    painter->setPen(itsBackgroundCols[QT_STD_BORDER]);
                    painter->drawLine(frameV2.rect.topLeft(), frameV2.rect.topRight());
                    painter->setPen(oldPen);
                }
                else
                {
                    frameV2.state &= ~(State_Sunken | State_HasFocus);
                    drawPrimitive(PE_Frame, &frameV2, painter, widget);
                }
            }
            break;
        case PE_Frame:
            if(widget && widget->parent() && widget->parent()->inherits("KTitleWidget"))
            {
                static const int constLineHeight=1;  // Match with SystemSettings, etc.

                QLinearGradient grad(0, 0, r.width(), 1);
                const QColor    *use(backgroundColors(option));

                grad.setColorAt(0, use[QT_STD_BORDER]); // palette.text().color());
                grad.setColorAt(1, palette.background().color());
                painter->fillRect(QRect(r.x(), (r.y()+r.height())-(constLineHeight+1), r.width(), constLineHeight), QBrush(grad));
            }
            else if(widget && widget->parent() && qobject_cast<const QComboBox *>(widget->parent()))
            {
                if(opts.gtkComboMenus && !((QComboBox *)(widget->parent()))->isEditable())
                    drawPrimitive(PE_FrameMenu, option, painter, widget);
                else
                {
                    const QColor *use(backgroundColors(option));

                    painter->save();
                    painter->setPen(use[QT_STD_BORDER]);
                    drawRect(painter, r);
                    painter->setPen(palette.base().color());
                    drawRect(painter, r.adjusted(1, 1, -1, -1));
                    painter->restore();
                }
            }
            else
            {
                const QStyleOptionFrame *fo = qstyleoption_cast<const QStyleOptionFrame *>(option);

                if (fo && fo->lineWidth>0)
                {
                    painter->save();

                    QStyleOption opt(*option);
#ifndef QTC_HIGHLIGHT_SCROLVIEWS
                    opt.state&=~State_HasFocus;
#endif

                    drawBorder(painter, r, &opt, ROUNDED_ALL, backgroundColors(option),
                               WIDGET_FRAME, state&State_Sunken || state&State_HasFocus
                                                ? BORDER_SUNKEN
                                                : state&State_Raised
                                                    ? BORDER_RAISED
                                                    : BORDER_FLAT);
                    painter->restore();
                }
            }
            break;
        case PE_PanelMenuBar:
            if (widget && qobject_cast<const QMainWindow *>(widget->parentWidget())
                || (widget->parentWidget() && widget->parentWidget()->inherits("Q3MainWindow")))
            {
                painter->save();
                drawMenuOrToolBarBackground(painter, r, option);
                if(TB_NONE!=opts.toolbarBorders)
                {
                    const QColor *use=itsActive
                                        ? itsMenubarCols
                                        : backgroundColors(option);
                    bool         dark(TB_DARK==opts.toolbarBorders || TB_DARK_ALL==opts.toolbarBorders);

                    if(TB_DARK_ALL==opts.toolbarBorders || TB_LIGHT_ALL==opts.toolbarBorders)
                    {
                        painter->setPen(use[0]);
                        painter->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                        painter->drawLine(r.x(), r.y(), r.x(), r.y()+r.width()-1);
                        painter->setPen(use[dark ? 3 : 4]);
                        painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                        painter->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
                    }
                    else
                    {
                        painter->setPen(use[dark ? 3 : 4]);
                        painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                    }
                }
                painter->restore();
            }
            break;
        case PE_FrameTabBarBase:
            if (const QStyleOptionTabBarBase *tbb = qstyleoption_cast<const QStyleOptionTabBarBase *>(option))
                if(tbb->shape != QTabBar::RoundedNorth && tbb->shape != QTabBar::RoundedWest &&
                   tbb->shape != QTabBar::RoundedSouth && tbb->shape != QTabBar::RoundedEast)
                    QTC_BASE_STYLE::drawPrimitive(element, option, painter, widget);
                else
                {
#ifdef QTC_STYLE_QTABBAR
                    // Is this tabbar part of a tab widget?
                    if(widget && widget->parentWidget() && !qobject_cast<const QTabWidget *>(widget->parentWidget()))
                    {
#if 0
                        QRect r2(tbb->tabBarRect);

                        if(QTabBar::RoundedSouth==tbb->shape)
                            r2.adjust(0, 0, 0, -4);
                        else
                            r2.adjust(0, 4, 0, 0);
                        painter->setClipRect(r2);

                        drawBorder(painter, tbb->tabBarRect, option, ROUNDED_ALL, backgroundColors(option), WIDGET_OTHER, BORDER_RAISED);

                        painter->setClipping(false);
#endif
                        break;
                    }
#endif
                    const QColor *use(backgroundColors(option));
                    QRegion      region(tbb->rect);
                    QLine        topLine(tbb->rect.bottomLeft() - QPoint(0, 1), tbb->rect.bottomRight() - QPoint(0, 1)),
                                 bottomLine(tbb->rect.bottomLeft(), tbb->rect.bottomRight());

                    region -= tbb->tabBarRect;

                    painter->save();
                    painter->setClipRegion(region);
                    if(QTabBar::RoundedSouth==tbb->shape && APPEARANCE_FLAT==opts.appearance)
                        painter->setPen(palette.background().color());
                    else
                        painter->setPen(use[QTabBar::RoundedNorth==tbb->shape ? 5 : QT_FRAME_DARK_SHADOW]);
                    painter->drawLine(topLine);
                    painter->setPen(use[QTabBar::RoundedNorth==tbb->shape ? 0 : 5]);
                    painter->drawLine(bottomLine);
                    painter->restore();
                }
            break;
        case PE_FrameStatusBar:
            if(!opts.drawStatusBarFrames)
                break;
        case PE_FrameMenu:
        {
            const QColor *use(backgroundColors(option));

            painter->save();
            painter->setPen(use[QT_STD_BORDER]);
            drawRect(painter, r);

            if(opts.lighterPopupMenuBgnd)
            {
                painter->setPen(itsLighterPopupMenuBgndCol);
                drawRect(painter, r.adjusted(1, 1, -1, -1));
            }
            else
            {
                painter->setPen(use[0]);
                painter->drawLine(r.x()+1, r.y()+1, r.x()+r.width()-2,  r.y()+1);
                painter->drawLine(r.x()+1, r.y()+1, r.x()+1,  r.y()+r.height()-2);
                painter->setPen(use[QT_FRAME_DARK_SHADOW]);
                painter->drawLine(r.x()+1, r.y()+r.height()-2, r.x()+r.width()-2,  r.y()+r.height()-2);
                painter->drawLine(r.x()+r.width()-2, r.y()+1, r.x()+r.width()-2,  r.y()+r.height()-2);
            }
            painter->restore();
            break;
        }
        case PE_FrameDockWidget:
        {
            const QColor *use(backgroundColors(option));

            painter->save();
            painter->setPen(use[0]);
            painter->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
            painter->drawLine(r.x(), r.y(), r.x(), r.y()+r.height()-1);
            painter->setPen(use[APPEARANCE_FLAT==opts.appearance ? ORIGINAL_SHADE : QT_STD_BORDER]);
            painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
            painter->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
            painter->restore();
            break;
        }
        case PE_PanelButtonTool:
            if(!opts.stdSidebarButtons && isMultiTabBarTab(getButton(widget, painter)))
            {
                QRect        r2(r);
                QStyleOption opt(*option);

                if(r2.height()>r2.width() || (r2.height()<r2.width() && r2.width()<=32))
                    opt.state&=~State_Horizontal;
                else
                    opt.state|=State_Horizontal;

                const QColor *use(opt.state&State_On ? getSidebarButtons() : buttonColors(option));
                bool         horiz(opt.state&State_Horizontal);

                painter->save();
                if(opt.state&State_On || opt.state&State_MouseOver)
                {
                    r2.adjust(-1, -1, 1, 1);
                    drawLightBevel(painter, r2, &opt, ROUNDED_NONE, getFill(&opt, use), use, false, WIDGET_MENU_ITEM);
                }
                else
                    painter->fillRect(r2, palette.background().color());

                if(opt.state&State_MouseOver && opts.coloredMouseOver)
                {
                    r2=r;
                    if(MO_PLASTIK==opts.coloredMouseOver)
                        if(horiz)
                            r2.adjust(0, 1, 0, -1);
                        else
                            r2.adjust(1, 0, -1, 0);
                    else
                        r2.adjust(1, 1, -1, -1);

                    painter->setPen(itsMouseOverCols[opt.state&State_On ? 0 : 1]);

                    if(horiz || MO_PLASTIK!=opts.coloredMouseOver)
                    {
                        painter->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                        painter->drawLine(r2.x(), r2.y(), r2.x()+r2.width()-1, r2.y());
                    }

                    if(!horiz || MO_PLASTIK!=opts.coloredMouseOver)
                    {
                        painter->drawLine(r.x(), r.y(), r.x(), r.y()+r.height()-1);
                        painter->drawLine(r2.x(), r2.y(), r2.x(), r2.y()+r2.height()-1);
                        if(MO_PLASTIK!=opts.coloredMouseOver)
                            painter->setPen(itsMouseOverCols[opt.state&State_On ? 1 : 2]);
                    }

                    if(horiz || MO_PLASTIK!=opts.coloredMouseOver)
                    {
                        painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                        painter->drawLine(r2.x(), r2.y()+r2.height()-1, r2.x()+r2.width()-1,
                                          r2.y()+r2.height()-1);
                    }

                    if(!horiz || MO_PLASTIK!=opts.coloredMouseOver)
                    {
                        painter->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
                        painter->drawLine(r2.x()+r2.width()-1, r2.y(), r2.x()+r2.width()-1,
                                        r2.y()+r2.height()-1);
                    }
                }

                painter->restore();
                break;
            }
        case PE_IndicatorButtonDropDown: // This should never be called, but just in case - draw as a normal toolbutton...
        {
            bool dwt(widget && widget->inherits("QDockWidgetTitleButton"));

            if( ((state&State_Enabled) || !(state&State_AutoRaise)) &&
               (!widget || !dwt || (state&State_MouseOver)) )
                drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
            break;
        }
        case PE_IndicatorDockWidgetResizeHandle:
        {
            QStyleOption dockWidgetHandle = *option;
            bool horizontal = state&State_Horizontal;
            if (horizontal)
                dockWidgetHandle.state &= ~State_Horizontal;
            else
                dockWidgetHandle.state |= State_Horizontal;
            drawControl(CE_Splitter, &dockWidgetHandle, painter, widget);
            break;
        }
        case PE_FrameLineEdit:
        case PE_PanelLineEdit:
            if (const QStyleOptionFrame *lineEdit = qstyleoption_cast<const QStyleOptionFrame *>(option))
            {
                if (lineEdit->lineWidth>0 &&
                    !(widget &&
                     (qobject_cast<const QComboBox *>(widget->parentWidget()) ||
                      qobject_cast<const QAbstractSpinBox *>(widget->parentWidget()))))
                {
                    QStyleOptionFrame opt(*lineEdit);

                    if(opt.state&State_Enabled && state&State_ReadOnly)
                        opt.state^=State_Enabled;

                    painter->save();

                    CEtchCheck check(widget);
                    drawEntryField(painter, r, &opt, ROUNDED_ALL);
                    painter->restore();
                }
            }
            break;
        case PE_Q3CheckListIndicator:
            if (const QStyleOptionQ3ListView *lv = qstyleoption_cast<const QStyleOptionQ3ListView *>(option))
            {
                if(lv->items.isEmpty())
                    break;

                QStyleOptionQ3ListViewItem item(lv->items.at(0));
                int                        x(lv->rect.x()),
                                           w(lv->rect.width()),
                                           marg(lv->itemMargin);

                if (state & State_Selected && !lv->rootIsDecorated && !(item.features & QStyleOptionQ3ListViewItem::ParentControl))
                    painter->fillRect(0, 0, x + marg + w + 4, item.height, palette.brush(QPalette::Highlight));
            }

            r.setX(r.x()+((r.width()-QTC_CHECK_SIZE)/2)-1);
            r.setY(r.y()+((r.height()-QTC_CHECK_SIZE)/2)-1);
            r.setWidth(QTC_CHECK_SIZE);
            r.setHeight(QTC_CHECK_SIZE);
        case PE_IndicatorCheckBox:
        {
            const QColor *bc(borderColors(option, NULL)),
                         *btn(buttonColors(option)),
                         *use(bc ? bc : btn);
            const QColor &bgnd(state&State_Enabled
                                ? state&State_MouseOver
                                    ? use[QTC_CR_MO_FILL]
                                    : palette.base().color()
                                : palette.background().color());

            painter->save();
            if(IS_FLAT(opts.appearance))
                painter->fillRect(QRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2), bgnd);
            else
                drawBevelGradient(bgnd, false, painter, QRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2), true,
                                  getWidgetShade(WIDGET_TROUGH, true, false, APPEARANCE_GRADIENT),
                                  getWidgetShade(WIDGET_TROUGH, false, false, APPEARANCE_GRADIENT),
                                  false, APPEARANCE_GRADIENT, WIDGET_TROUGH);
            painter->setPen(midColor(state&State_Enabled ? palette.base().color() : palette.background().color(), use[3]));
            painter->drawLine(r.x()+1, r.y()+1, r.x()+1, r.y()+r.height()-2);
            painter->drawLine(r.x()+1, r.y()+1, r.x()+r.width()-2, r.y()+1);

            drawBorder(painter, r, option, ROUNDED_ALL, use, WIDGET_CHECKBOX);

            if(state&State_On)
            {
                QPixmap *pix(getPixmap(state&State_Enabled
                                        ? /*state&State_Selected
                                            ? palette.highlightedText().color()
                                            : */ itsCheckRadioCol
                                        : palette.mid().color(),
                                    PIX_CHECK, 1.0));

                painter->drawPixmap(r.center().x()-(pix->width()/2), r.center().y()-(pix->height()/2),
                                    *pix);
            }
            else if (state&State_NoChange)    // tri-state
            {
                int x(r.center().x()), y(r.center().y());

                painter->setPen(state&State_Enabled
                                ? /*state&State_Selected
                                    ? palette.highlightedText().color()
                                    : */ itsCheckRadioCol
                                : palette.mid().color());
                painter->drawLine(x-3, y, x+3, y);
                painter->drawLine(x-3, y+1, x+3, y+1);
            }

            painter->restore();
            break;
        }
        case PE_Q3CheckListExclusiveIndicator:
            if (const QStyleOptionQ3ListView *lv = qstyleoption_cast<const QStyleOptionQ3ListView *>(option))
            {
                if(lv->items.isEmpty())
                    break;

                QStyleOptionQ3ListViewItem item(lv->items.at(0));
                int                        x(lv->rect.x()),
                                           w(lv->rect.width()),
                                           marg(lv->itemMargin);

                if (state & State_Selected && !lv->rootIsDecorated && !(item.features & QStyleOptionQ3ListViewItem::ParentControl))
                    painter->fillRect(0, 0, x + marg + w + 4, item.height, palette.brush(QPalette::Highlight));
            }

            r.setX(r.x()+((r.width()-QTC_RADIO_SIZE)/2)-1);
            r.setY(r.y()+((r.height()-QTC_RADIO_SIZE)/2)-1);
            r.setWidth(QTC_RADIO_SIZE);
            r.setHeight(QTC_RADIO_SIZE);
        case PE_IndicatorRadioButton:
        {
            int      x(r.x()), y(r.y());
            QPolygon clipRegion;

            clipRegion.setPoints(8,  x,    y+8,     x,    y+4,     x+4, y,      x+8, y,
                                     x+12, y+4,     x+12, y+8,     x+8, y+12,   x+4, y+12);

            const QColor *bc(borderColors(option, NULL)),
                            *btn(buttonColors(option)),
                            *use(bc ? bc : btn);
            const QColor &bgnd(state&State_Enabled
                                ? state&State_MouseOver
                                    ? use[QTC_CR_MO_FILL]
                                    : palette.base().color()
                                : palette.background().color());

            painter->save();
            painter->setClipRegion(QRegion(clipRegion));
            if(IS_FLAT(opts.appearance))
                painter->fillRect(QRect(x+1, y+1, r.width()-2, r.height()-2), bgnd);
            else
                drawBevelGradient(bgnd, false, painter, QRect(x+1, y+1, r.width()-2, r.height()-2), true,
                                  getWidgetShade(WIDGET_TROUGH, true, false, APPEARANCE_GRADIENT),
                                  getWidgetShade(WIDGET_TROUGH, false, false, APPEARANCE_GRADIENT),
                                  false, APPEARANCE_GRADIENT, WIDGET_TROUGH);
            painter->setClipping(false);

/*
            painter->setRenderHint(QPainter::Antialiasing, true);
            QPainterPath path;
            path.moveTo(12, 12);
            path.arcTo(0.5, 6.5, 12, 12, 0, 360);
            painter->drawPath(path);
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->setPen(use[opts.coloredMouseOver && state&State_MouseOver ? 4 : QT_BORDER(state&State_Enabled)]);
            painter->drawPath(path);
*/
            painter->drawPixmap(x, y, *getPixmap(use[opts.coloredMouseOver && state&State_MouseOver ? 4 : QT_BORDER(state&State_Enabled)],
                                PIX_RADIO_BORDER, 0.8));
            if(state&State_On)
                painter->drawPixmap(x, y, *getPixmap(state&State_Enabled
                                                        ? /*state&State_Selected
                                                            ? palette.highlightedText().color()
                                                            :*/ itsCheckRadioCol
                                                        : palette.mid().color(), PIX_RADIO_ON, 1.0));
            painter->drawPixmap(x, y, *getPixmap(btn[state&State_MouseOver ? 3 : 4], PIX_RADIO_LIGHT));
            painter->restore();
            break;
        }
        case PE_IndicatorToolBarHandle:
            painter->save();
            drawHandleMarkers(painter, r, option, true, opts.handles);
            painter->restore();
            break;
        case PE_FrameFocusRect:
            if (const QStyleOptionFocusRect *focusFrame = qstyleoption_cast<const QStyleOptionFocusRect *>(option))
            {
                 if (!(focusFrame->state&State_KeyboardFocusChange))
                     return;

#ifndef QTC_PLAIN_FOCUS_ONLY
                if(opts.stdFocus)
#endif
                    QTC_BASE_STYLE::drawPrimitive(element, option, painter, widget);
#ifndef QTC_PLAIN_FOCUS_ONLY
                else
                {
                    //Figuring out in what beast we are painting...
                    const QColor *use(backgroundColors(option));

                    if(r.width()<4 || r.height()<4 ||
                       (widget && ((dynamic_cast<const QAbstractScrollArea*>(widget)) || widget->inherits("Q3ScrollView"))) ||
                       (widget && widget->parent() && ((dynamic_cast<const QAbstractScrollArea*>(widget->parent())) ||
                                                       widget->parent()->inherits("Q3ScrollView"))))
                    {
                        painter->setPen(use[QT_FOCUS]);
                        drawRect(painter, r);
                    }
                    else
                        drawBorder(painter, r, option, ROUNDED_ALL, use, WIDGET_OTHER, BORDER_FLAT, QT_FOCUS);
                }
#endif
            }
            break;
        case PE_PanelButtonBevel:
        case PE_PanelButtonCommand:
        {
            const QColor *use(buttonColors(option));
            bool         isDefault(false),
                         isFlat(false),
                         isKWin(state&QtC_StateKWin),
                         isDown(state&State_Sunken || state&State_On),
                         isOnListView(!isKWin && widget && qobject_cast<const QTreeView *>(widget));
            QStyleOption opt(*option);
            CEtchCheck   check(widget);

            if(PE_PanelButtonBevel==element)
                opt.state|=State_Enabled;

            if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton*>(option))
            {
                isDefault = (button->features & QStyleOptionButton::DefaultButton) && (button->state&State_Enabled);
                isFlat = (button->features & QStyleOptionButton::Flat);
            }

            if(isFlat && !isDown && !(state&State_MouseOver))
                return;

            painter->save();

            if(isOnListView)
                opt.state|=State_Horizontal|State_Raised;

            if(isDefault && state&State_Enabled && IND_TINT==opts.defBtnIndicator)
                use=itsDefBtnCols;

            // This section fixes some drawng issues with krunner's buttons on nvidia
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->fillRect(r.adjusted(1, 1, -1, -1), palette.background().color());
            painter->setRenderHint(QPainter::Antialiasing, false);

            drawLightBevel(painter, r, &opt, ROUNDED_ALL, getFill(&opt, use), use,
                           true, isKWin
                                    ? WIDGET_MDI_WINDOW_BUTTON
                                    : isOnListView
                                        ? WIDGET_NO_ETCH_BTN
                                        : isDefault && state&State_Enabled && IND_FONT_COLOR==opts.defBtnIndicator
                                            ? WIDGET_DEF_BUTTON
                                            : WIDGET_STD_BUTTON);

            if (isDefault && state&State_Enabled)
                switch(opts.defBtnIndicator)
                {
                    case IND_CORNER:
                    {
                        QPainterPath path;
                        int          offset(isDown ? 5 : 4),
                                     etchOffset(QTC_CAN_DO_EFFECT ? 1 : 0);
                        double       xd(r.x()+0.5),
                                     yd(r.y()+0.5);

                        path.moveTo(xd+offset, yd+offset+etchOffset);
                        path.lineTo(xd+offset+6, yd+offset+etchOffset);
                        path.lineTo(xd+offset, yd+offset+6+etchOffset);
                        path.lineTo(xd+offset, yd+offset+etchOffset);
                        painter->setBrush(itsMouseOverCols[isDown ? 0 : 4]);
                        painter->setPen(itsMouseOverCols[isDown ? 0 : 4]);
                        painter->setRenderHint(QPainter::Antialiasing, true);
                        painter->drawPath(path);
                        painter->setRenderHint(QPainter::Antialiasing, false);
                        break;
                    }
                    case IND_COLORED:
                    {
                        const QColor *cols=itsMouseOverCols && opt.state&State_MouseOver ? itsMouseOverCols : itsDefBtnCols;
                        QRegion      outer(r);
                        QRect        r2(r);

                        if(QTC_CAN_DO_EFFECT)
                            r2.adjust(0, 1, 0, -1);

                        r2.adjust(COLORED_BORDER_SIZE, COLORED_BORDER_SIZE, -COLORED_BORDER_SIZE,
                                    -COLORED_BORDER_SIZE);

                        QRegion inner(r2);

                        painter->setClipRegion(outer.subtract(inner));

                        drawLightBevel(painter, r, option, ROUNDED_ALL, cols[QTC_MO_DEF_BTN],
                                       cols, true, WIDGET_DEF_BUTTON);

                        painter->setClipping(false);
                    }
                    default:
                        break;
                }
            painter->restore();
            break;
        }
        case PE_FrameDefaultButton:
            break;
        case PE_FrameWindow:
        {
            const QColor *borderCols(theThemedApp==APP_KWIN
                                        ? buttonColors(option)
                                        : getMdiColors(option, state&State_Active));
            QStyleOption opt(*option);

            opt.state=State_Horizontal|State_Enabled|State_Raised;
            drawBorder(painter, r, &opt, ROUNDED_BOTTOM, borderCols, WIDGET_MDI_WINDOW, BORDER_RAISED);
            break;
        }
        case PE_FrameTabWidget:
        {
            int round(ROUNDED_ALL);

            if(opts.round && widget && ::qobject_cast<const QTabWidget *>(widget))
            {
                const QTabWidget *tw((const QTabWidget *)widget);

                if(0==tw->currentIndex())
                    if(const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option))
                    {
                        bool reverse(Qt::RightToLeft==twf->direction);

                        switch(tw->tabPosition())
                        {
                            case QTabWidget::North:
                                if(reverse && twf->rightCornerWidgetSize.isEmpty())
                                    round-=CORNER_TR;
                                else if(!reverse && twf->leftCornerWidgetSize.isEmpty())
                                    round-=CORNER_TL;
                                break;
                            case QTabWidget::South:
                                if(reverse && twf->rightCornerWidgetSize.isEmpty())
                                    round-=CORNER_BR;
                                else if(!reverse && twf->leftCornerWidgetSize.isEmpty())
                                    round-=CORNER_BL;
                                break;
                            case QTabWidget::West:
                                round-=CORNER_TL;
                                break;
                            case QTabWidget::East:
                                round-=CORNER_TR;
                                break;
                        }
                    }
            }
            painter->save();
            drawBorder(painter, r, option, round, backgroundColors(option), WIDGET_OTHER, BORDER_RAISED, false);
            painter->restore();
            break;
        }
        default:
            QTC_BASE_STYLE::drawPrimitive(element, option, painter, widget);
            break;
    }
}

void QtCurveStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
                               const QWidget *widget) const
{
    QRect                 r(option->rect);
    const QFlags<State> & state(option->state);
    const QPalette &      palette(option->palette);

    switch(element)
    {
        case CE_MenuScroller:
            painter->fillRect(r, opts.lighterPopupMenuBgnd ? itsLighterPopupMenuBgndCol
                                                           : itsBackgroundCols[ORIGINAL_SHADE]);
                //QStyleOption arrowOpt = *opt;
                //arrowOpt.state |= State_Enabled;

            painter->setPen(itsBackgroundCols[QT_STD_BORDER]);
            drawRect(painter, r);
            drawPrimitive(((state&State_DownArrow) ? PE_IndicatorArrowDown : PE_IndicatorArrowUp),
                           option, painter, widget);
            break;
        case CE_RubberBand: // Rubber band used in such things as iconview.
        {
            painter->save();
            QColor c(itsMenuitemCols[ORIGINAL_SHADE]);
            painter->setPen(c);
            c.setAlpha(50);
            painter->setBrush(c);
            drawRect(painter, r);
            painter->restore();
            break;
        }
        case CE_Splitter:
        {
            const QColor *use(buttonColors(option));
            const QColor *border(borderColors(option, use));

            painter->save();
            painter->fillRect(r, QColor(state&State_MouseOver
                                      ? shade(palette.background().color(), opts.highlightFactor)
                                      : palette.background().color()));
            switch(opts.splitters)
            {
                default:
                case LINE_DOTS:
                    drawDots(painter, r, state&State_Horizontal, NUM_SPLITTER_DASHES, 1, border, 0, 5);
                    break;
                case LINE_SUNKEN:
                    drawLines(painter, r, state&State_Horizontal, NUM_SPLITTER_DASHES, 1, border, 0, 3);
                    break;
                case LINE_FLAT:
                    drawLines(painter, r, state&State_Horizontal, NUM_SPLITTER_DASHES, 3, border, 0, 3, 0, false);
                    break;
                case LINE_DASHES:
                    drawLines(painter, r, state&State_Horizontal, NUM_SPLITTER_DASHES, 1, border, 0, 3, 0);
            }
            painter->restore();
            break;
        }
        case CE_SizeGrip:
        {
            int x, y, w, h;
            r.getRect(&x, &y, &w, &h);

            int sw(qMin(h, w));

            painter->save();
            if (h > w)
                painter->translate(0, h - w);
            else
                painter->translate(w - h, 0);

            int sx(x),
                sy(y),
                s(4);

            if (option->direction == Qt::RightToLeft)
            {
                sx = x + sw;
                for (int i = 0; i < 4; ++i) {
                    painter->setPen(QPen(itsBackgroundCols[0], 1));
                    painter->drawLine(x, sy - 1 , sx + 1, sw);
                    painter->setPen(QPen(itsBackgroundCols[QT_STD_BORDER], 1));
                    painter->drawLine(x, sy, sx, sw);
                    sx -= s;
                    sy += s;
                }
            }
            else
                for (int i = 0; i < 4; ++i)
                {
                    painter->setPen(QPen(itsBackgroundCols[0], 1));
                    painter->drawLine(sx - 1, sw, sw, sy - 1);
                    painter->setPen(QPen(itsBackgroundCols[QT_STD_BORDER], 1));
                    painter->drawLine(sx, sw, sw, sy);
                    sx += s;
                    sy += s;
                }
            painter->restore();
            break;
        }
        case CE_ToolBar:
            if (const QStyleOptionToolBar *toolbar = qstyleoption_cast<const QStyleOptionToolBar *>(option))
            {
                painter->save();
                drawMenuOrToolBarBackground(painter, r, option, false, Qt::NoToolBarArea==toolbar->toolBarArea ||
                                                                       Qt::BottomToolBarArea==toolbar->toolBarArea ||
                                                                       Qt::TopToolBarArea==toolbar->toolBarArea);
                if(TB_NONE!=opts.toolbarBorders && (!widget->parent() || qobject_cast<QMainWindow *>(widget->parent())))
                {
                    const QColor *use=/*PE_PanelMenuBar==pe && itsActive
                                        ? itsMenubarCols
                                        : */ backgroundColors(option);
                    bool         dark(TB_DARK==opts.toolbarBorders || TB_DARK_ALL==opts.toolbarBorders);

                    if(TB_DARK_ALL==opts.toolbarBorders || TB_LIGHT_ALL==opts.toolbarBorders)
                    {
                        painter->setPen(use[0]);
                        painter->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                        painter->drawLine(r.x(), r.y(), r.x(), r.y()+r.width()-1);
                        painter->setPen(use[dark ? 3 : 4]);
                        painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                        painter->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
                    }
                    else
                    {
                        bool paintH(true),
                             paintV(true);

                        switch (toolbar->toolBarArea)
                        {
                            case Qt::BottomToolBarArea:
                            case Qt::TopToolBarArea:
                                paintV=false;
                                break;
                            case Qt::RightToolBarArea:
                            case Qt::LeftToolBarArea:
                                paintH=false;
                            default:
                                break;
                        }

                        painter->setPen(use[0]);
                        if(paintH)
                            painter->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                        if(paintV)
                            painter->drawLine(r.x(), r.y(), r.x(), r.y()+r.height()-1);
                        painter->setPen(use[dark ? 3 : 4]);
                        if(paintH)
                            painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                        if(paintV)
                            painter->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
                    }
                }
                painter->restore();
            }
            break;
        case CE_DockWidgetTitle:
            if (const QStyleOptionDockWidget *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget *>(option))
            {
                painter->save();

#if QT_VERSION >= 0x040300
                const QStyleOptionDockWidgetV2 *v2 = qstyleoption_cast<const QStyleOptionDockWidgetV2*>(dwOpt);
                bool verticalTitleBar(v2 == 0 ? false : v2->verticalTitleBar);

                QRect titleRect(subElementRect(SE_DockWidgetTitleBarText, option, widget));

                if (verticalTitleBar)
                {
                    QRect rVert(r);
                    QSize s(rVert.size());

                    s.transpose();
                    rVert.setSize(s);

                    titleRect = QRect(rVert.left() + r.bottom() - titleRect.bottom(),
                                      rVert.top() + titleRect.left() - r.left(),
                                      titleRect.height(), titleRect.width());

                    painter->translate(rVert.left(), rVert.top() + rVert.width());
                    painter->rotate(-90);
                    painter->translate(-rVert.left(), -rVert.top());
                }
#else
/*                bool  verticalTitleBar(false);*/
                const int margin(4);
                QRect titleRect(visualRect(dwOpt->direction, r, r.adjusted(margin, 0, -margin * 2 - 26, 0)));
#endif
                QRect handleRect(titleRect);

//                if(state&State_MouseOver)
                {
    /*
                    if(IS_FLAT(opts.appearance))
    */
    #if QT_VERSION >= 0x040300
                        painter->fillRect(r, palette.background().color().darker(105));
    #else
                        painter->fillRect(r, palette.background().color().dark(105));
    #endif
    /*
                    else
                    {
                        const QColor *use(backgroundColors(option));

                        drawBevelGradient(use[ORIGINAL_SHADE], true, painter, r, true,
                                        getWidgetShade(WIDGET_STD_BUTTON, true, false, opts.appearance),
                                        getWidgetShade(WIDGET_STD_BUTTON, false, false, opts.appearance),
                                        false, opts.appearance, WIDGET_STD_BUTTON);
                    }
    */
                }
                if (!dwOpt->title.isEmpty())
                {
//                     int textWidth=option->fontMetrics.width(dwOpt->title);
// 
//                     if(verticalTitleBar)
//                         handleRect.adjust(0, textWidth+margin, 0, -margin);
//                     else
//                         handleRect.adjust(reverse ? margin : textWidth+margin, 0, reversde ? -(textWidth+margin) : -margin, 0);

                    drawItemText(painter, titleRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic, palette,
                                 dwOpt->state&State_Enabled,
                                 painter->fontMetrics().elidedText(dwOpt->title, Qt::ElideRight, titleRect.width(),
                                 QPalette::WindowText));
                }

//                 if(handleRect.isValid())
//                     drawHandleMarkers(painter, handleRect, option, false, opts.handles);
                painter->restore();
            }
            break;
#if QT_VERSION >= 0x040300
        case CE_HeaderEmptyArea:
        {
            const QStyleOptionHeader *ho = qstyleoption_cast<const QStyleOptionHeader *>(option);
            bool horiz(ho ? Qt::Horizontal==ho->orientation : state&State_Horizontal);
            QStyleOption opt(*option);

            opt.state&=~State_MouseOver;
            painter->save();

            drawBevelGradient(getFill(&opt, itsBackgroundCols), true, painter, r, horiz,
                              SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK,
                              false, opts.lvAppearance, WIDGET_LISTVIEW_HEADER);

            painter->setRenderHint(QPainter::Antialiasing, true);
            if(APPEARANCE_RAISED==opts.lvAppearance)
            {
                painter->setPen(itsBackgroundCols[4]);
                if(horiz)
                    drawAaLine(painter, r.x(), r.y()+r.height()-2, r.x()+r.width()-1, r.y()+r.height()-2);
                else
                    drawAaLine(painter, r.x()+r.width()-2, r.y(), r.x()+r.width()-2, r.y()+r.height()-1);
            }

            painter->setPen(itsBackgroundCols[QT_STD_BORDER]);
            if(horiz)
                drawAaLine(painter, r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
            else
                drawAaLine(painter, r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->restore();
            break;
        }
#endif
        case CE_HeaderSection:
            if (const QStyleOptionHeader *ho = qstyleoption_cast<const QStyleOptionHeader *>(option))
            {
                painter->save();

                if(state & (State_Raised | State_Sunken))
                {
                    bool         sunken(state &(/*State_Down |*/ State_On | State_Sunken)),
                                 q3Header(widget && widget->inherits("Q3Header"));
                    QStyleOption opt(*option);

                    if(q3Header && widget && widget->underMouse() && itsHoverWidget && r.contains(itsPos))
                        opt.state|=State_MouseOver;

                    if(-1==ho->section && !(state&State_Enabled) && widget && widget->isEnabled())
                        opt.state|=State_Enabled;

                    drawBevelGradient(getFill(&opt, itsBackgroundCols), !sunken, painter, r,
                                        Qt::Horizontal==ho->orientation,
                                        sunken ? SHADE_BEVEL_GRAD_SEL_LIGHT : SHADE_BEVEL_GRAD_LIGHT,
                                        sunken ? SHADE_BEVEL_GRAD_SEL_DARK : SHADE_BEVEL_GRAD_DARK,
                                        sunken, opts.lvAppearance, WIDGET_LISTVIEW_HEADER);

                    painter->setRenderHint(QPainter::Antialiasing, true);
                    if(APPEARANCE_RAISED==opts.lvAppearance)
                    {
                        painter->setPen(itsBackgroundCols[4]);
                        if(Qt::Horizontal==ho->orientation)
                            drawAaLine(painter, r.x(), r.y()+r.height()-2, r.x()+r.width()-1, r.y()+r.height()-2);
                        else
                            drawAaLine(painter, r.x()+r.width()-2, r.y(), r.x()+r.width()-2, r.y()+r.height()-1);
                    }

                    const QColor *border(borderColors(&opt, NULL));

                    if(Qt::Horizontal==ho->orientation)
                    {
                        if(border)
                        {
                            painter->setPen(border[ORIGINAL_SHADE]);
                            drawAaLine(painter, r.x(), r.y()+r.height()-2, r.x()+r.width()-1,
                                      r.y()+r.height()-2);
                            painter->setPen(border[QT_STD_BORDER]);
                        }
                        else
                            painter->setPen(itsBackgroundCols[QT_STD_BORDER]);
                        drawAaLine(painter, r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);

                        if(q3Header ||
                           (QStyleOptionHeader::End!=ho->position && QStyleOptionHeader::OnlyOneSection!=ho->position))
                        {
                            painter->setPen(itsBackgroundCols[QT_STD_BORDER]);
                            drawAaLine(painter, r.x()+r.width()-2, r.y()+5, r.x()+r.width()-2, r.y()+r.height()-6);
                            painter->setPen(itsBackgroundCols[0]);
                            drawAaLine(painter, r.x()+r.width()-1, r.y()+5, r.x()+r.width()-1, r.y()+r.height()-6);
                        }
                    }
                    else
                    {
                        if(border)
                        {
                            painter->setPen(border[ORIGINAL_SHADE]);
                            drawAaLine(painter, r.x()+r.width()-2, r.y(), r.x()+r.width()-2, r.y()+r.height()-1);
                            painter->setPen(border[QT_STD_BORDER]);
                        }
                        else
                            painter->setPen(itsBackgroundCols[QT_STD_BORDER]);
                        drawAaLine(painter, r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);

                        if(q3Header ||
                           (QStyleOptionHeader::End!=ho->position && QStyleOptionHeader::OnlyOneSection!=ho->position))
                        {
                            painter->setPen(itsBackgroundCols[QT_STD_BORDER]);
                            drawAaLine(painter, r.x()+5, r.y()+r.height()-2, r.x()+r.width()-6,
                                      r.y()+r.height()-2);
                            painter->setPen(itsBackgroundCols[0]);
                            drawAaLine(painter, r.x()+5, r.y()+r.height()-1, r.x()+r.width()-6,
                                      r.y()+r.height()-1);
                        }
                    }
                    painter->setRenderHint(QPainter::Antialiasing, false);
                }
                else
                    painter->fillRect(r, getFill(option, itsBackgroundCols));
                painter->restore();
            }
            break;
//         case CE_HeaderLabel:
//             if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option))
//             {
//                 if (state & (State_On | State_Sunken))
//                     r.translate(pixelMetric(PM_ButtonShiftHorizontal, option, widget),
//                                 pixelMetric(PM_ButtonShiftVertical, option, widget));
// 
//                 if (!header->icon.isNull())
//                 {
//                     QPixmap pixmap(header->icon.pixmap(pixelMetric(PM_SmallIconSize), (header->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled));
//                     int     pixw(pixmap.width());
//                     QRect   aligned(alignedRect(header->direction, QFlag(header->iconAlignment), pixmap.size(), r)),
//                             inter(aligned.intersected(r));
// 
//                     painter->drawPixmap(inter.x(), inter.y(), pixmap, inter.x() - aligned.x(), inter.y() - aligned.y(), inter.width(), inter.height());
// 
//                     if (header->direction == Qt::LeftToRight)
//                         r.setLeft(r.left() + pixw + 2);
//                     else
//                         r.setRight(r.right() - pixw - 2);
//                 }
//                 drawItemText(painter, r, header->textAlignment, palette, state&State_Enabled, header->text, QPalette::ButtonText);
//             }
//             break;
        case CE_ProgressBarGroove:
            painter->save();
            if(opts.gradientPbGroove)
            {
                bool horiz(true);

                if (const QStyleOptionProgressBarV2 *bar = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(option))
                    horiz = Qt::Horizontal==bar->orientation;

                drawBevelGradient(state&State_Enabled ? palette.base().color() : palette.background().color(), false,
                                  painter, r.adjusted(1, 1, -1, -1), horiz,
                                  getWidgetShade(WIDGET_TROUGH, true, false, opts.progressAppearance),
                                  getWidgetShade(WIDGET_TROUGH, false, false, opts.progressAppearance),
                                  false, APPEARANCE_GRADIENT, WIDGET_TROUGH);
            }
            else
                painter->fillRect(r.adjusted(1, 1, -1, -1), state&State_Enabled ? palette.base().color() : palette.background().color());
            drawBorder(painter, r, option, ROUNDED_ALL, backgroundColors(option), WIDGET_OTHER, BORDER_SUNKEN);
            painter->restore();
            break;
        case CE_ProgressBarContents:
            if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option))
            {
                bool vertical(false),
                     inverted(false),
                     indeterminate(0==bar->minimum && 0==bar->maximum);

                // Get extra style options if version 2
                if (const QStyleOptionProgressBarV2 *bar2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(option))
                {
                    vertical = Qt::Vertical==bar2->orientation;
                    inverted = bar2->invertedAppearance;
                }

                if (!indeterminate && bar->progress == -1)
                    break;

                bool reverse = (!vertical && (bar->direction == Qt::RightToLeft)) || vertical;

                if (inverted)
                    reverse = !reverse;

                painter->save();

                if(indeterminate) //Busy indicator
                {
                    int          measure(vertical ? r.height() : r.width()),
                                 step(itsAnimateStep % ((measure-PROGRESS_CHUNK_WIDTH) * 2));
                    QStyleOption opt(*option);

                    if (step > (measure-PROGRESS_CHUNK_WIDTH))
                        step = 2 * (measure-PROGRESS_CHUNK_WIDTH) - step;

                    opt.state|=State_Raised|State_Horizontal;

                    if(vertical)
                        opt.state^=State_Horizontal;

                    drawLightBevel(painter, vertical
                                                ? QRect(r.x(), r.y()+step, r.width(), PROGRESS_CHUNK_WIDTH)
                                                : QRect(r.x()+step, r.y(), PROGRESS_CHUNK_WIDTH, r.height()),
                                   &opt, ROUNDED_ALL,
                                   itsMenuitemCols[ORIGINAL_SHADE], itsMenuitemCols, true, WIDGET_PROGRESSBAR);
                }
                else
                {
                    QRect cr(r);

                    if(cr.isValid() && bar->progress>0)
                    {
                        double pg(((double)bar->progress) / (bar->maximum-bar->minimum));

                        if(vertical)
                        {
                            int height(qMin(cr.height(), (int)(pg * cr.height())));

/*                            if(reverse || inverted)*/
                                drawProgress(painter, QRect(cr.x(), cr.y()+(cr.height()-height), cr.width(), height),
                                             option, height==cr.height() ? ROUNDED_NONE : ROUNDED_TOP, true, true);
//                             else
//                                 drawProgress(painter, QRect(cr.x(), cr.y(), cr.width(), height), option,
//                                             height==cr.height() ?  ROUNDED_NONE : ROUNDED_TOP);
                        }
                        else
                        {
                            int width(qMin(cr.width(), (int)(pg * cr.width())));

                            if(reverse || inverted)
                                drawProgress(painter, QRect(cr.x()+(cr.width()-width), cr.y(), width, cr.height()),
                                             option, width==cr.width() ? ROUNDED_NONE : ROUNDED_LEFT, false, true);
                            else
                                drawProgress(painter, QRect(cr.x(), cr.y(), width, cr.height()), option,
                                             width==cr.width() ? ROUNDED_NONE : ROUNDED_RIGHT);
                        }
                    }
                }
                painter->restore();
            }
            break;
        case CE_ProgressBarLabel:
            if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option))
            {
                // The busy indicator doesn't draw a label
                if (0==bar->minimum && 0==bar->maximum)
                    return;

                bool vertical(false),
                     inverted(false),
                     bottomToTop(false);

                // Get extra style options if version 2
                if (const QStyleOptionProgressBarV2 *bar2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(option))
                {
                    vertical = (bar2->orientation == Qt::Vertical);
                    inverted = bar2->invertedAppearance;
                    bottomToTop = bar2->bottomToTop;
                }

#if QT_VERSION < 0x040300
                if(vertical)
                    return;
#endif

                painter->save();

                QRect leftRect;
                QFont font;

                font.setBold(true);
                painter->setFont(font);
                painter->setPen(palette.text().color());

#if QT_VERSION >= 0x040300
                if (vertical)
                {
                    r = QRect(r.left(), r.top(), r.height(), r.width()); // flip width and height

                    QTransform m;
                    if (bottomToTop)
                    {
                        m.translate(0.0, r.width());
                        m.rotate(-90);
                    }
                    else
                    {
                        m.translate(r.height(), 0.0);
                        m.rotate(90);
                    }
                    painter->setTransform(m);
                }
#endif

                double vc6Workaround(((bar->progress - qint64(bar->minimum)) / double(qint64(bar->maximum) - qint64(bar->minimum))) * r.width());
                int    progressIndicatorPos=(int)vc6Workaround;

                bool flip((!vertical && (((Qt::RightToLeft==bar->direction) && !inverted) || ((Qt::LeftToRight==bar->direction) && inverted))) ||
                          (vertical && ((!inverted && !bottomToTop) || (inverted && bottomToTop))));

                if (flip)
                {
                    int indicatorPos(r.width() - progressIndicatorPos);

                    if (indicatorPos >= 0 && indicatorPos <= r.width())
                    {
                        painter->setPen(palette.base().color());
                        leftRect = QRect(r.left(), r.top(), indicatorPos, r.height());
                    }
                    else if (indicatorPos > r.width())
                        painter->setPen(palette.text().color());
                    else
                        painter->setPen(palette.highlightedText().color());
                }
                else
                {
                    if (progressIndicatorPos >= 0 && progressIndicatorPos <= r.width())
                        leftRect = QRect(r.left(), r.top(), progressIndicatorPos, r.height());
                    else if (progressIndicatorPos > r.width())
                        painter->setPen(palette.highlightedText().color());
                    else
                        painter->setPen(palette.text().color());
                }

                painter->drawText(r, bar->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
                if (!leftRect.isNull())
                {
                    painter->setPen(flip ? palette.text().color() : palette.highlightedText().color());
                    painter->setClipRect(leftRect, Qt::IntersectClip);
                    painter->drawText(r, bar->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
                }

                painter->restore();
            }
            break;
        case CE_MenuBarItem:
            if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
            {
                bool down(state&(State_On|state&State_Sunken)),
                     active(state&State_Enabled && (down || (state&State_Selected && opts.menubarMouseOver)));
                uint alignment(Qt::AlignCenter|Qt::TextShowMnemonic|Qt::TextDontClip|Qt::TextSingleLine);

                if (!styleHint(SH_UnderlineShortcut, mbi, widget))
                    alignment|=Qt::TextHideMnemonic;

                QPixmap pix(mbi->icon.pixmap(pixelMetric(PM_SmallIconSize), (mbi->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled));

                painter->save();

                drawMenuOrToolBarBackground(painter, mbi->menuRect, option);

                if(active)
                    drawMenuItem(painter, r, option, true, down && opts.roundMbTopOnly ? ROUNDED_TOP : ROUNDED_ALL,
                                opts.colorMenubarMouseOver || down ? itsMenuitemCols : itsBackgroundCols);

                if (!pix.isNull())
                    drawItemPixmap(painter, mbi->rect, alignment, pix);
                else
                {
                    const QColor &col=state&State_Enabled
                                        ? (opts.colorMenubarMouseOver && active) || (!opts.colorMenubarMouseOver && down)
                                            ? opts.customMenuTextColor
                                                ? opts.customMenuSelTextColor
                                                : palette.highlightedText().color()
                                            : itsActive
                                                ? opts.customMenuTextColor
                                                    ? opts.customMenuNormTextColor
                                                    : SHADE_BLEND_SELECTED==opts.shadeMenubars ||
                                                        (SHADE_CUSTOM==opts.shadeMenubars &&
                                                        TOO_DARK(itsMenubarCols[ORIGINAL_SHADE]))
                                                        ? palette.highlightedText().color()
                                                        : palette.foreground().color()
                                                : palette.foreground().color()
                                        : palette.foreground().color();

                    painter->setPen(col);
                    painter->drawText(r, alignment, mbi->text);
                }
                painter->restore();
            }
            break;
        case CE_MenuItem:
            if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
            {
                bool comboMenu(qobject_cast<const QComboBox*>(widget));
                int  checkcol(qMax(menuItem->maxIconWidth, 20));

                painter->save();

                if (QStyleOptionMenuItem::Separator==menuItem->menuItemType)
                {
                    painter->fillRect(menuItem->rect, opts.lighterPopupMenuBgnd ? itsLighterPopupMenuBgndCol
                                                                                : itsBackgroundCols[ORIGINAL_SHADE]);

                    if(opts.menuStripe && !comboMenu)
                        drawBevelGradient(itsBackgroundCols[opts.lighterPopupMenuBgnd ? ORIGINAL_SHADE : 3], true, painter,
                                        QRect(r.x(), r.y(), qMax(checkcol, constMenuPixmapWidth), r.height()), false,
                                        getWidgetShade(WIDGET_OTHER, true, false, opts.menuStripeAppearance),
                                        getWidgetShade(WIDGET_OTHER, false, false, opts.menuStripeAppearance),
                                        false, opts.menuStripeAppearance, WIDGET_OTHER);

                    int w = 0;
                    if (!menuItem->text.isEmpty())
                    {
                        painter->setFont(menuItem->font);
                        drawItemText(painter, menuItem->rect.adjusted(5, 0, -5, 0), Qt::AlignLeft | Qt::AlignVCenter,
                                     palette, state&State_Enabled, menuItem->text, QPalette::Text);
                        w = menuItem->fontMetrics.width(menuItem->text) + 5;
                    }

                    bool reverse = menuItem->direction == Qt::RightToLeft;

                    painter->setPen(itsBackgroundCols[QT_STD_BORDER]);
                    painter->drawLine(menuItem->rect.left() + 4 + (reverse ? 0 : w), menuItem->rect.center().y(),
                                    menuItem->rect.right() - 4 - (reverse ? w : 0), menuItem->rect.center().y());

//                     painter->setPen(itsBackgroundCols[0]);
//                     painter->drawLine(menuItem->rect.left() + 4 + (reverse ? 0 : w), menuItem->rect.center().y()+1,
//                                     menuItem->rect.right() - 4 - (reverse ? w : 0), menuItem->rect.center().y()+1);
                    painter->restore();
                    break;
                }

                bool selected(state&State_Selected),
                     checkable(QStyleOptionMenuItem::NotCheckable!=menuItem->checkType),
                     checked(menuItem->checked),
                     enabled(state&State_Enabled);

                if (selected && enabled)
                    drawMenuItem(painter, r.adjusted(0, 0, -1, 0), option, false, ROUNDED_ALL, itsMenuitemCols);
                else
                {
                    painter->fillRect(menuItem->rect, opts.lighterPopupMenuBgnd ? itsLighterPopupMenuBgndCol
                                                                                : itsBackgroundCols[ORIGINAL_SHADE]);
                    if(opts.menuStripe && !comboMenu)
                        drawBevelGradient(itsBackgroundCols[opts.lighterPopupMenuBgnd ? ORIGINAL_SHADE : 3], true, painter,
                                        QRect(r.x(), r.y(), qMax(checkcol, constMenuPixmapWidth), r.height()), false,
                                        getWidgetShade(WIDGET_OTHER, true, false, opts.menuStripeAppearance),
                                        getWidgetShade(WIDGET_OTHER, false, false, opts.menuStripeAppearance),
                                        false, opts.menuStripeAppearance, WIDGET_OTHER);
                }

                if(comboMenu)
                {
                    if (menuItem->icon.isNull())
                        checkcol = 0;
                    else
                        checkcol = menuItem->maxIconWidth;
                }
                else
                {
                    // Check
                    QRect checkRect(r.left() + 4, r.center().y() - 6, 13, 13);
                    checkRect = visualRect(menuItem->direction, menuItem->rect, checkRect);
                    if (checkable)
                    {
                        if ((menuItem->checkType & QStyleOptionMenuItem::Exclusive) && menuItem->icon.isNull())
                        {
                            QStyleOptionButton button;
                            button.rect = checkRect;
                            button.state = menuItem->state;
                            if (checked)
                                button.state |= State_On;
                            button.palette = palette;
                            drawPrimitive(PE_IndicatorRadioButton, &button, painter, widget);
                        }
                        else
                        {
                            if (menuItem->icon.isNull())
                            {
                                QStyleOptionButton button;
                                button.rect = checkRect;
                                button.state = menuItem->state;
                                if (checked)
                                    button.state |= State_On;
                                button.palette = palette;
                                drawPrimitive(PE_IndicatorCheckBox, &button, painter, widget);
                            }
                            else if (checked)
                            {
                                int          iconSize(qMax(menuItem->maxIconWidth, 20));
                                QRect        sunkenRect(r.left() + 1, r.top() + (r.height() - iconSize) / 2 + 1,
                                                        iconSize, iconSize);
                                QStyleOption opt(*option);

                                sunkenRect = visualRect(menuItem->direction, menuItem->rect, sunkenRect);
                                opt.state = menuItem->state;
                                opt.state|=State_Raised;
                                if (checked)
                                    opt.state |= State_On;
                                drawLightBevel(painter, sunkenRect, &opt, ROUNDED_ALL, getFill(&opt, itsButtonCols), itsButtonCols);
                            }
                        }
                    }
                }

                // Text and icon, ripped from windows style
                bool  dis(!(state&State_Enabled)),
                      act(state&State_Selected);
                QRect vCheckRect(visualRect(option->direction, menuItem->rect,
                                            QRect(menuItem->rect.x(), menuItem->rect.y(),
                                                  checkcol, menuItem->rect.height())));

                if (!menuItem->icon.isNull())
                {
                    QIcon::Mode mode(dis ? QIcon::Disabled : QIcon::Normal);

                    if (act && !dis)
                        mode = QIcon::Active;

                    QPixmap pixmap(checked ? menuItem->icon.pixmap(pixelMetric(PM_SmallIconSize), mode, QIcon::On)
                                           : menuItem->icon.pixmap(pixelMetric(PM_SmallIconSize), mode));

                    int   pixw(pixmap.width()),
                          pixh(pixmap.height());
                    QRect pmr(0, 0, pixw, pixh);

                    pmr.moveCenter(vCheckRect.center());
                    painter->setPen(palette.text().color());
                    if (checkable && checked)
                        painter->drawPixmap(QPoint(pmr.left() + 1, pmr.top() + 1), pixmap);
                    else
                        painter->drawPixmap(pmr.topLeft(), pixmap);
                }

                painter->setPen(selected ? palette.highlightedText().color() : palette.foreground().color());

                int    x, y, w, h,
                       tab(menuItem->tabWidth);
                QColor discol;

                menuItem->rect.getRect(&x, &y, &w, &h);

                if (dis)
                {
                    discol = palette.text().color();
                    painter->setPen(discol);
                }

                int     xm(windowsItemFrame + checkcol + windowsItemHMargin),
                        xpos(menuItem->rect.x() + xm);
                QRect   textRect(xpos, y + windowsItemVMargin, w - xm - windowsRightBorder - tab + 1, h - 2 * windowsItemVMargin),
                        vTextRect = visualRect(option->direction, menuItem->rect, textRect);
                QString s(menuItem->text);

                if (!s.isEmpty())                      // draw text
                {
                    int t(s.indexOf(QLatin1Char('\t'))),
                        textFlags(Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine);

                    painter->save();

                    if (!styleHint(SH_UnderlineShortcut, menuItem, widget))
                        textFlags |= Qt::TextHideMnemonic;
                    textFlags |= Qt::AlignLeft;

                    if (t >= 0)
                    {
                        QRect vShortcutRect(visualRect(option->direction, menuItem->rect,
                                                    QRect(textRect.topRight(), QPoint(menuItem->rect.right(), textRect.bottom()))));

                        if (dis && !act)
                        {
                            painter->setPen(palette.light().color());
                            painter->drawText(vShortcutRect.adjusted(1, 1, 1, 1), textFlags, s.mid(t + 1));
                            painter->setPen(discol);
                        }
                        painter->drawText(vShortcutRect, textFlags, s.mid(t + 1));
                        s = s.left(t);
                    }

                    QFont font(menuItem->font);

                    if (menuItem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                        font.setBold(true);

                    painter->setFont(font);
                    if (dis && !act)
                    {
                        painter->setPen(palette.light().color());
                        painter->drawText(vTextRect.adjusted(1, 1, 1, 1), textFlags, s.left(t));
                        painter->setPen(discol);
                    }
                    painter->drawText(vTextRect, textFlags, s.left(t));
                    painter->restore();
                }

                // Arrow
                if (QStyleOptionMenuItem::SubMenu==menuItem->menuItemType) // draw sub menu arrow
                {
                    int              dim((menuItem->rect.height() - 4) / 2),
                                     xpos(menuItem->rect.left() + menuItem->rect.width() - 6 - 2 - dim);
                    PrimitiveElement arrow(Qt::RightToLeft==option->direction ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight);
                    QRect            vSubMenuRect(visualRect(option->direction, menuItem->rect,
                                                             QRect(xpos, menuItem->rect.top() + menuItem->rect.height() / 2 - dim / 2, dim, dim)));

                    drawArrow(painter, vSubMenuRect, option, arrow, false, true);
                }

                painter->restore();
            }
            break;
        case CE_MenuHMargin:
        case CE_MenuVMargin:
        case CE_MenuEmptyArea:
            break;
        case CE_PushButton:
            if(const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))
            {
                CEtchCheck check(widget);

                drawControl(CE_PushButtonBevel, btn, painter, widget);

                QStyleOptionButton subopt(*btn);

                subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
                drawControl(CE_PushButtonLabel, &subopt, painter, widget);

                if (btn->state & State_HasFocus)
                {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*btn);
                    fropt.rect = subElementRect(SE_PushButtonFocusRect, btn, widget);

                    if(QTC_CAN_DO_EFFECT)
                        fropt.rect.adjust(0, 1, 0, -1);
                    drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
                }
            }
            break;
        case CE_PushButtonBevel:
            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))
            {
                int dbi(pixelMetric(PM_ButtonDefaultIndicator, btn, widget));

                if (btn->features & QStyleOptionButton::DefaultButton)
                    drawPrimitive(PE_FrameDefaultButton, option, painter, widget);
                if (btn->features & QStyleOptionButton::AutoDefaultButton)
                    r.setCoords(r.left() + dbi, r.top() + dbi, r.right() - dbi, r.bottom() - dbi);
                if ( !(btn->features & (QStyleOptionButton::Flat
#if QT_VERSION >= 0x040300
                       |QStyleOptionButton::CommandLinkButton
#endif
                       )) ||
                    state&(State_Sunken | State_On | State_MouseOver))
                {
                    QStyleOptionButton tmpBtn(*btn);

                    tmpBtn.rect = r;
                    drawPrimitive(PE_PanelButtonCommand, &tmpBtn, painter, widget);
                }
//                 if (btn->features & QStyleOptionButton::HasMenu)
//                 {
//                     int                mbi(pixelMetric(PM_MenuButtonIndicator, btn, widget));
//                     QRect              ir(btn->rect);
//                     QStyleOptionButton newBtn(*btn);
// 
//                     newBtn.rect = QRect(ir.right() - mbi + 2, ir.height()/2 - mbi/2 + 3, mbi - 6, mbi - 6);
//                     drawPrimitive(PE_IndicatorArrowDown, &newBtn, painter, widget);
//                 }
            }
            break;
        case CE_PushButtonLabel:
            if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option))
            {
                uint tf(Qt::AlignVCenter | Qt::TextShowMnemonic);

                if (!styleHint(SH_UnderlineShortcut, button, widget))
                    tf |= Qt::TextHideMnemonic;

                if (!button->icon.isNull())
                {
                    //Center both icon and text
                    QIcon::Mode mode(button->state&State_Enabled ? QIcon::Normal : QIcon::Disabled);

                    if (QIcon::Normal==mode && button->state&State_HasFocus)
                        mode = QIcon::Active;

                    QIcon::State state(button->state&State_On ? QIcon::On : QIcon::Off);
                    QPixmap      pixmap(button->icon.pixmap(button->iconSize, mode, state));
                    int          labelWidth(pixmap.width()),
                                 labelHeight(pixmap.height()),
                                 iconSpacing (4),//### 4 is currently hardcoded in QPushButton::sizeHint()
                                 textWidth(button->fontMetrics.boundingRect(option->rect, tf, button->text).width());

                    if (!button->text.isEmpty())
                        labelWidth += (textWidth + iconSpacing);

                    QRect iconRect(r.x() + (r.width() - labelWidth) / 2,
                                r.y() + (r.height() - labelHeight) / 2,
                                pixmap.width(), pixmap.height());

                    iconRect = visualRect(button->direction, r, iconRect);

                    tf |= Qt::AlignLeft; //left align, we adjust the text-rect instead

                    if (Qt::RightToLeft==button->direction)
                        r.setRight(iconRect.left() - iconSpacing);
                    else
                        r.setLeft(iconRect.left() + iconRect.width() + iconSpacing);

                    if (button->state & (State_On|State_Sunken))
                        iconRect.translate(pixelMetric(PM_ButtonShiftHorizontal, option, widget),
                                        pixelMetric(PM_ButtonShiftVertical, option, widget));
                    painter->drawPixmap(iconRect, pixmap);
                }
                else
                    tf |= Qt::AlignHCenter;

                if (button->state & (State_On|State_Sunken))
                    r.translate(pixelMetric(PM_ButtonShiftHorizontal, option, widget),
                                pixelMetric(PM_ButtonShiftVertical, option, widget));

                //this tweak ensures the font is perfectly centered on small sizes
                //but slightly downward to make it more gnomeish if not
                if (button->fontMetrics.height() > 14)
                    r.translate(0, 1);

                if (button->features&QStyleOptionButton::HasMenu)
                {
                    int indicatorSize(pixelMetric(PM_MenuButtonIndicator, button, widget));

                    if (Qt::LeftToRight==button->direction)
                        r = r.adjusted(0, 0, -indicatorSize, 0);
                    else
                        r = r.adjusted(indicatorSize, 0, 0, 0);

                    QRect              ir(button->rect);
                    QStyleOptionButton newBtn(*button);

                    newBtn.rect = QRect(Qt::LeftToRight==button->direction
                                            ? ir.right() - indicatorSize + 2
                                            : ir.x() + 6,
                                        ir.height()/2 - indicatorSize/2 + 3,
                                        indicatorSize - 6, indicatorSize - 6);
                    drawPrimitive(PE_IndicatorArrowDown, &newBtn, painter, widget);
                }

                int num(opts.embolden && button->features&QStyleOptionButton::DefaultButton ? 2 : 1);

                for(int i=0; i<num; ++i)
                    drawItemText(painter, r.adjusted(i, 0, i, 0), tf, button->palette, (button->state&State_Enabled),
                                button->text, QPalette::ButtonText);
            }
            break;
        case CE_ComboBoxLabel:
            if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
            {
                if (!comboBox->editable)
                {
                    QStyleOptionComboBox opt(*comboBox);

                    painter->save();
                    painter->setPen(QPen(palette.buttonText(), 0));
                    if (state & (State_On | State_Sunken))
                        opt.rect.translate(pixelMetric(PM_ButtonShiftHorizontal, option, widget),
                                           pixelMetric(PM_ButtonShiftVertical, option, widget));
                    QTC_BASE_STYLE::drawControl(element, &opt, painter, widget);
                    painter->restore();
                }
                else if(!comboBox->currentIcon.isNull())
                {
                    QRect editRect = subControlRect(CC_ComboBox, comboBox, SC_ComboBoxEditField, widget);
                    if (comboBox->direction == Qt::RightToLeft)
                        editRect.adjust(0, 2, -2, -2);
                    else
                        editRect.adjust(2, 2, 0, -2);
                    painter->save();
                    painter->setClipRect(editRect);
                    if (!comboBox->currentIcon.isNull())
                    {
                        QPixmap pixmap(comboBox->currentIcon.pixmap(comboBox->iconSize, state&State_Enabled ? QIcon::Normal : QIcon::Disabled));
                        QRect   iconRect(editRect);
                        iconRect.setWidth(comboBox->iconSize.width() + 5);
                        iconRect = alignedRect(QApplication::layoutDirection(), Qt::AlignLeft | Qt::AlignVCenter,
                                            iconRect.size(), editRect);
                        painter->fillRect(iconRect, palette.brush(QPalette::Base));
                        drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
                    }
                    painter->restore();
                }
                else
                    QTC_BASE_STYLE::drawControl(element, option, painter, widget);
            }
            break;
        case CE_MenuBarEmptyArea:
            {
                painter->save();

                drawMenuOrToolBarBackground(painter, r, option);
                if (TB_NONE!=opts.toolbarBorders && widget && widget->parentWidget() &&
                    (qobject_cast<const QMainWindow *>(widget->parentWidget()) || widget->parentWidget()->inherits("Q3MainWindow")))
                {
                    const QColor *use=itsActive
                                        ? itsMenubarCols
                                        : backgroundColors(option);

                    bool         dark(TB_DARK==opts.toolbarBorders || TB_DARK_ALL==opts.toolbarBorders);

                    if(TB_DARK_ALL==opts.toolbarBorders || TB_LIGHT_ALL==opts.toolbarBorders)
                    {
                        painter->setPen(use[0]);
                        painter->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                        painter->drawLine(r.x(), r.y(), r.x(), r.y()+r.width()-1);
                        painter->setPen(use[dark ? 3 : 4]);
                        painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                        painter->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
                    }
                    else
                    {
                        painter->setPen(use[dark ? 3 : 4]);
                        painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                    }
                }
                painter->restore();
            }
            break;
        case CE_TabBarTabShape:
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option))
            {
                bool onlyBase(widget && widget->parentWidget()
                                ? qobject_cast<const QTabWidget *>(widget->parentWidget()) ? false : true
                                : false),
                     selected(state&State_Selected),
                     horiz(QTabBar::RoundedNorth==tab->shape || QTabBar::RoundedSouth==tab->shape);

#ifdef QTC_STYLE_QTABBAR
                if(onlyBase)
                {
                    if(selected || state&State_MouseOver)
                    {
                        QStyleOption opt(*option);
                        const QColor *use(buttonColors(option));

                        if(selected)
                            opt.state|=State_On;
                        if(horiz)
                            opt.state|=State_Horizontal;
                        else
                            opt.state^=State_Horizontal;

//                         drawBorder(painter, r, option, ROUNDED_ALL, state&State_MouseOver && opts.coloredMouseOver
//                                                                         ? itsMouseOverCols
//                                                                         : use, WIDGET_OTHER,
//                                    selected ? BORDER_SUNKEN : BORDER_RAISED);

                        drawLightBevel(painter, opt.rect, &opt, ROUNDED_ALL,
                                       /*selected ? use[ORIGINAL_SHADE] :*/ getFill(&opt, use), use, true, WIDGET_STD_BUTTON);
                    }
                    break;
                }
#endif

                QRect r2(r);
                bool rtlHorTabs(Qt::RightToLeft==tab->direction && horiz),
                     onlyTab(QStyleOptionTab::OnlyOneTab==tab->position),
                     leftCornerWidget(tab->cornerWidgets&QStyleOptionTab::LeftCornerWidget),
                     rightCornerWidget(tab->cornerWidgets&QStyleOptionTab::RightCornerWidget),
                     firstTab((tab->position == (Qt::LeftToRight==tab->direction || !horiz ?
                                  QStyleOptionTab::Beginning : QStyleOptionTab::End)) || onlyTab),
                     lastTab((tab->position == (Qt::LeftToRight==tab->direction  || !horiz ?
                                  QStyleOptionTab::End : QStyleOptionTab::Beginning)) || onlyTab);
                int  tabBarAlignment(styleHint(SH_TabBar_Alignment, tab, widget)),
                     tabOverlap(onlyTab ? 0 : pixelMetric(PM_TabBarTabOverlap, option, widget));
                bool leftAligned((!rtlHorTabs && Qt::AlignLeft==tabBarAlignment) ||
                                 (rtlHorTabs && Qt::AlignRight==tabBarAlignment)),
                     rightAligned((!rtlHorTabs && Qt::AlignRight==tabBarAlignment) ||
                                   (rtlHorTabs && Qt::AlignLeft==tabBarAlignment)),
                     fixLeft(!onlyBase && !leftCornerWidget && leftAligned && firstTab),
                     fixRight(!onlyBase && !rightCornerWidget && rightAligned && lastTab);
                const QColor &fill(getTabFill(selected, state&State_MouseOver, itsBackgroundCols));

                painter->save();
                switch(tab->shape)
                {
                    case QTabBar::RoundedNorth:
                    case QTabBar::TriangularNorth:
                        if(!selected)
                            r.adjust(0, 2, 0, -2);

                        if(!firstTab)
                            r.adjust(-tabOverlap, 0, 0, 0);
                        fillTab(painter, r.adjusted(1, 0, -1, 0), option, fill, true, true, WIDGET_TAB_TOP);

                        // This slipping helps with plasma's tabs and nvidia
                        if(selected)
                            painter->setClipRect(r2.adjusted(0, 0, 0, -1));
                        drawBorder(painter, r.adjusted(0, 0, 0, 4), option,
                                    selected || onlyTab
                                        ? ROUNDED_TOP
                                        : firstTab
                                            ? ROUNDED_TOPLEFT
                                            : lastTab
                                                ? ROUNDED_TOPRIGHT
                                                : ROUNDED_NONE,
                                    NULL, WIDGET_TAB_TOP, selected && !opts.colorSelTab ? BORDER_RAISED : BORDER_FLAT, false);

                        if(selected)
                        {
                            painter->setClipping(false);
                            painter->setPen(itsBackgroundCols[0]);

                            // The point drawn below is because of the clipping above...
                            if(fixLeft)
                                painter->drawPoint(r2.x()+1, r2.y()+r2.height()-1);
                            else
                                painter->drawLine(r2.left()-1, r2.bottom(), r2.left(), r2.bottom());
                            if(!fixRight)
                                painter->drawLine(r2.right()-1, r2.bottom(), r2.right(), r2.bottom());
                        }
                        else
                        {
                            int l(fixLeft ? r2.left()+2 : r2.left()-1),
                                r(fixRight ? r2.right()-2 : r2.right()+1);
                            painter->setPen(itsBackgroundCols[QT_STD_BORDER]);
                            painter->drawLine(l, r2.bottom()-1, r, r2.bottom()-1);
                            painter->setPen(itsBackgroundCols[0]);
                            painter->drawLine(l, r2.bottom(), r, r2.bottom());
                        }

                        if(selected)
                        {
                            if(opts.highlightTab)
                            {
                                painter->setRenderHint(QPainter::Antialiasing, true);
                                painter->setPen(itsMenuitemCols[0]);
                                drawAaLine(painter, r.left()+1, r.top()+1, r.right()-1, r.top()+1);
                                painter->setPen(midColor(fill, itsMenuitemCols[0], IS_FLAT(opts.tabAppearance) ? 1.0 : 1.2));
                                drawAaLine(painter, r.left()+1, r.top()+2, r.right()-1, r.top()+2);
                                painter->setRenderHint(QPainter::Antialiasing, false);

                                painter->setClipRect(QRect(r.x(), r.y(), r.width(), 3));
                                drawBorder(painter, r, option, ROUNDED_ALL, itsMenuitemCols, WIDGET_TAB_TOP, BORDER_FLAT, false, 3);
                            }
                        }
                        else if(state&State_MouseOver && opts.coloredMouseOver)
                        {
                            painter->setRenderHint(QPainter::Antialiasing, true);
                            painter->setPen(itsMouseOverCols[ORIGINAL_SHADE]);
                            drawAaLine(painter, r.x()+(firstTab ? opts.round : 1), r.y()+1,
                                              r.x()+r.width()-((lastTab ? opts.round : 0)+1), r.y()+1);
                            painter->setPen(itsMouseOverCols[QT_STD_BORDER]);
                            drawAaLine(painter, r.x()+(firstTab ? opts.round : 1), r.y(),
                                              r.x()+r.width()-((lastTab ? opts.round : 0)+1), r.y());
                            painter->setRenderHint(QPainter::Antialiasing, false);
                        }
                        break;
                    case QTabBar::RoundedSouth:
                    case QTabBar::TriangularSouth:
                        if(!selected)
                            r.adjust(0, 2, 0, -2);
                        if(!firstTab)
                            r.adjust(-tabOverlap, 0, 0, 0);

                        fillTab(painter, r.adjusted(1, 0, -1, 0), option, fill, true, false, WIDGET_TAB_BOT);

                        drawBorder(painter, r.adjusted(0, -4, 0, 0), option,
                                    selected || onlyTab
                                        ? ROUNDED_BOTTOM
                                        : firstTab
                                            ? ROUNDED_BOTTOMLEFT
                                            : lastTab
                                                ? ROUNDED_BOTTOMRIGHT
                                                : ROUNDED_NONE,
                                    NULL, WIDGET_TAB_BOT, selected && !opts.colorSelTab ? BORDER_RAISED : BORDER_FLAT, false);

                        if(selected)
                        {
                            painter->setPen(itsBackgroundCols[QT_FRAME_DARK_SHADOW]);
                            if(!fixLeft)
                                painter->drawPoint(r2.left()-1, r2.top());
                            if(!fixRight)
                                painter->drawLine(r2.right()-1, r2.top(), r2.right(), r2.top());
                        }
                        else
                        {
                            int l(fixLeft ? r2.left()+2 : r2.left()-1),
                                r(fixRight ? r2.right()-2 : r2.right());
                            painter->setPen(itsBackgroundCols[QT_STD_BORDER]);
                            painter->drawLine(l, r2.top()+1, r, r2.top()+1);
                            painter->setPen(itsBackgroundCols[QT_FRAME_DARK_SHADOW]);
                            painter->drawLine(l, r2.top(), r, r2.top());
                        }

                        if(selected)
                        {
                            if(opts.highlightTab)
                            {
                                painter->setRenderHint(QPainter::Antialiasing, true);
                                painter->setPen(itsMenuitemCols[0]);
                                drawAaLine(painter, r.left()+1, r.bottom()-1, r.right()-1, r.bottom()-1);
                                painter->setPen(midColor(fill, itsMenuitemCols[0]));
                                drawAaLine(painter, r.left()+1, r.bottom()-2, r.right()-1, r.bottom()-2);
                                painter->setRenderHint(QPainter::Antialiasing, false);

                                painter->setClipRect(QRect(r.x(), r.y()+r.height()-3, r.width(), r.y()+r.height()-1));
                                drawBorder(painter, r, option, ROUNDED_ALL, itsMenuitemCols, WIDGET_TAB_BOT, BORDER_FLAT, false, 3);
                            }
                        }
                        else if(state&State_MouseOver && opts.coloredMouseOver)
                        {
                            painter->setRenderHint(QPainter::Antialiasing, true);
                            painter->setPen(itsMouseOverCols[ORIGINAL_SHADE]);
                            drawAaLine(painter, r.x()+(firstTab ? opts.round : 1), r.bottom()-1,
                                              r.x()+r.width()-((lastTab ? opts.round : 0)+1), r.bottom()-1);
                            painter->setPen(itsMouseOverCols[QT_STD_BORDER]);
                            drawAaLine(painter, r.x()+(firstTab ? opts.round : 1), r.bottom(),
                                              r.x()+r.width()-((lastTab ? opts.round : 0)+1), r.bottom());
                            painter->setRenderHint(QPainter::Antialiasing, false);
                        }
                        break;
                    case QTabBar::RoundedWest:
                    case QTabBar::TriangularWest:
                        if(!selected)
                            r.adjust(2, 0, -2, 0);

                        if(!firstTab)
                            r.adjust(0, -tabOverlap, 0, 0);
                        fillTab(painter, r.adjusted(0, 1, 0, -1), option, fill, false, true, WIDGET_TAB_TOP);

                        drawBorder(painter, r.adjusted(0, 0, 4, 0), option,
                                    selected || onlyTab
                                        ? ROUNDED_LEFT
                                        : firstTab
                                            ? ROUNDED_TOPLEFT
                                            : lastTab
                                                ? ROUNDED_BOTTOMLEFT
                                                : ROUNDED_NONE,
                                    NULL, WIDGET_TAB_TOP, selected && !opts.colorSelTab ? BORDER_RAISED : BORDER_FLAT, false);

                        if(selected)
                        {
                            painter->setPen(itsBackgroundCols[0]);
                            if(!firstTab)
                                painter->drawPoint(r2.right(), r2.top()-1);
                            painter->drawLine(r2.right(), r2.bottom()-1, r2.right(), r2.bottom());
                        }
                        else
                        {
                            int t(firstTab ? r2.top()+2 : r2.top()-1),
                                b(/*lastTab ? r2.bottom()-2 : */ r2.bottom()+1);

                            painter->setPen(itsBackgroundCols[QT_STD_BORDER]);
                            painter->drawLine(r2.right()-1, t, r2.right()-1, b);
                            painter->setPen(itsBackgroundCols[0]);
                            painter->drawLine(r2.right(), t, r2.right(), b);
                        }

                        if(selected)
                        {
                            if(opts.highlightTab)
                            {
                                painter->setRenderHint(QPainter::Antialiasing, true);
                                painter->setPen(itsMenuitemCols[0]);
                                drawAaLine(painter, r.left()+1, r.top()+1, r.left()+1, r.bottom()-1);
                                painter->setPen(midColor(fill, itsMenuitemCols[0], IS_FLAT(opts.tabAppearance) ? 1.0 : 1.2));
                                drawAaLine(painter, r.left()+2, r.top()+1, r.left()+2, r.bottom()-1);
                                painter->setRenderHint(QPainter::Antialiasing, false);

                                painter->setClipRect(QRect(r.x(), r.y(), 3, r.height()));
                                drawBorder(painter, r, option, ROUNDED_ALL, itsMenuitemCols, WIDGET_TAB_TOP, BORDER_FLAT, false, 3);
                            }
                        }
                        else if(state&State_MouseOver && opts.coloredMouseOver)
                        {
                            painter->setRenderHint(QPainter::Antialiasing, true);
                            painter->setPen(itsMouseOverCols[ORIGINAL_SHADE]);
                            drawAaLine(painter, r.x()+1, r.y()+(firstTab ? opts.round : 1),
                                              r.x()+1, r.y()+r.height()-((lastTab ? opts.round : 0)+1));
                            painter->setPen(itsMouseOverCols[QT_STD_BORDER]);
                            drawAaLine(painter, r.x(), r.y()+(firstTab ? opts.round : 1),
                                              r.x(), r.y()+r.height()-((lastTab ? opts.round : 0)+1));
                            painter->setRenderHint(QPainter::Antialiasing, false);
                        }
                        break;
                    case QTabBar::RoundedEast:
                    case QTabBar::TriangularEast:
                        if(!selected)
                            r.adjust(2, 0, -2, 0);

                        if(!firstTab)
                            r.adjust(0, -tabOverlap, 0, 0);
                        fillTab(painter, r.adjusted(0, 1, 0, -1), option, fill, false, false, WIDGET_TAB_BOT);

                        drawBorder(painter, r.adjusted(-4, 0, 0, 0), option,
                                    selected || onlyTab
                                        ? ROUNDED_RIGHT
                                        : firstTab
                                            ? ROUNDED_TOPRIGHT
                                            : lastTab
                                                ? ROUNDED_BOTTOMRIGHT
                                                : ROUNDED_NONE,
                                    NULL, WIDGET_TAB_BOT, selected && !opts.colorSelTab ? BORDER_RAISED : BORDER_FLAT, false);

                        if(selected)
                        {
                            painter->setPen(itsBackgroundCols[QT_FRAME_DARK_SHADOW]);
                            if(!firstTab)
                                painter->drawPoint(r2.left(), r2.top()-1);
                            painter->drawLine(r2.left(), r2.bottom()-1, r2.left(), r2.bottom());
                        }
                        else
                        {
                            int t(firstTab ? r2.top()+2 : r2.top()-1),
                                b(/*lastTab ? r2.bottom()-2 : */ r2.bottom()+1);

                            painter->setPen(itsBackgroundCols[QT_STD_BORDER]);
                            painter->drawLine(r2.left()+1, t, r2.left()+1, b);
                            painter->setPen(itsBackgroundCols[QT_FRAME_DARK_SHADOW]);
                            painter->drawLine(r2.left(), t, r2.left(), b);
                        }

                        if(selected)
                        {
                            if(opts.highlightTab)
                            {
                                painter->setRenderHint(QPainter::Antialiasing, true);
                                painter->setPen(itsMenuitemCols[0]);
                                drawAaLine(painter, r.right()-1, r.top()+1, r.right()-1, r.bottom()-1);
                                painter->setPen(midColor(fill, itsMenuitemCols[0]));
                                drawAaLine(painter, r.right()-2, r.top()+1, r.right()-2, r.bottom()-1);
                                painter->setRenderHint(QPainter::Antialiasing, false);

                                painter->setClipRect(QRect(r.x()+r.width()-3, r.y(), r.x()+r.width()-1, r.height()));
                                drawBorder(painter, r, option, ROUNDED_ALL, itsMenuitemCols, WIDGET_TAB_TOP, BORDER_FLAT, false, 3);
                            }
                        }
                        else if(state&State_MouseOver && opts.coloredMouseOver)
                        {
                            painter->setRenderHint(QPainter::Antialiasing, true);
                            painter->setPen(itsMouseOverCols[ORIGINAL_SHADE]);
                            drawAaLine(painter, r.right()-1, r.y()+(firstTab ? opts.round : 1),
                                              r.right()-1, r.y()+r.height()-((lastTab ? opts.round : 0)+1));
                            painter->setPen(itsMouseOverCols[QT_STD_BORDER]);
                            drawAaLine(painter, r.right(), r.y()+(firstTab ? opts.round : 1),
                                              r.right(), r.y()+r.height()-((lastTab ? opts.round : 0)+1));
                            painter->setRenderHint(QPainter::Antialiasing, false);
                        }
                        break;
                }
                painter->restore();
            }
            break;
        case CE_ScrollBarAddLine:
        case CE_ScrollBarSubLine:
        {
            QRect        br(r),
                         ar(r);
            const QColor *use(itsButtonCols); // buttonColors(option));
            bool         reverse(option && Qt::RightToLeft==option->direction);
            PrimitiveElement pe=state&State_Horizontal
                   ? CE_ScrollBarAddLine==element ? (reverse ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight)
                                                  : (reverse ? PE_IndicatorArrowRight : PE_IndicatorArrowLeft)
                   : CE_ScrollBarAddLine==element ? PE_IndicatorArrowDown : PE_IndicatorArrowUp;

            int round=PE_IndicatorArrowRight==pe ? ROUNDED_RIGHT :
                      PE_IndicatorArrowLeft==pe ? ROUNDED_LEFT :
                      PE_IndicatorArrowDown==pe ? ROUNDED_BOTTOM :
                      PE_IndicatorArrowUp==pe ? ROUNDED_TOP : ROUNDED_NONE;

            switch(opts.scrollbarType)
            {
                default:
                case SCROLLBAR_WINDOWS:
                    break;
                case SCROLLBAR_KDE:
                case SCROLLBAR_PLATINUM:
                    if(!reverse && PE_IndicatorArrowLeft==pe && r.x()>3)
                    {
                        round=ROUNDED_NONE;
                        br.adjust(0, 0, 1, 0);
                        ar.adjust(1, 0, 1, 0);
                    }
                    else if(reverse && PE_IndicatorArrowRight==pe && r.x()>3)
                    {
                        if(SCROLLBAR_PLATINUM==opts.scrollbarType)
                        {
                            round=ROUNDED_NONE;
                            br.adjust(-1, 0, 0, 0);
                            ar.adjust(-1, 0, -1, 0);
                        }
                        else
                        {
                            if(r.x()<pixelMetric(PM_ScrollBarExtent, option, widget)+2)
                                round=ROUNDED_NONE;
                            br.adjust(0, 0, 1, 0);
                            ar.adjust(1, 0, 1, 0);
                        }
                    }
                    else if(PE_IndicatorArrowUp==pe && r.y()>3)
                    {
                        round=ROUNDED_NONE;
                        br.adjust(0, 0, 0, 1);
                        ar.adjust(0, 1, 0, 1);
                    }
                    break;
                case SCROLLBAR_NEXT:
                    if(!reverse && PE_IndicatorArrowRight==pe)
                    {
                        round=ROUNDED_NONE;
                        br.adjust(-1, 0, 0, 0);
                        ar.adjust(-1, 0, 0, -1);
                    }
                    else if(reverse && PE_IndicatorArrowLeft==pe)
                    {
                        round=ROUNDED_NONE;
                        br.adjust(0, 0, 1, 0);
                        ar.adjust(-1, 0, 0, 1);
                    }
                    else if(PE_IndicatorArrowDown==pe)
                    {
                        round=ROUNDED_NONE;
                        br.adjust(0, -1, 0, 0);
                        ar.adjust(0, -1, 0, -1);
                    }
                    break;
            }

            painter->save();
            painter->fillRect(r, palette.background());

            QStyleOption opt(*option);

            opt.state|=State_Raised;

            if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
            {
                if((CE_ScrollBarSubLine==element && slider->sliderValue==slider->minimum) ||
                   (CE_ScrollBarAddLine==element && slider->sliderValue==slider->maximum))
                    opt.state&=~(State_MouseOver|State_Sunken|State_On);

                if(slider->minimum==slider->maximum && opt.state&State_Enabled)
                    opt.state^=State_Enabled;
            }

            drawLightBevel(painter, br, &opt, round, getFill(&opt, use), use, true, WIDGET_SB_BUTTON);

            opt.rect = ar;
            // The following fixes gwenviews scrollbars...
            if(opt.palette.text().color()!=opt.palette.buttonText().color())
                opt.palette.setColor(QPalette::Text, opt.palette.buttonText().color());
            drawPrimitive(pe, &opt, painter, widget);
            painter->restore();
            break;
        }
        case CE_ScrollBarSubPage:
        case CE_ScrollBarAddPage:
        {
            const QColor *use(backgroundColors(option));

            painter->save();
            if(state&State_Horizontal)
            {
                if(IS_FLAT(opts.appearance))
                    painter->fillRect(r.x(), r.y()+1, r.width(), r.height()-2, use[2]);
                else
                    drawBevelGradient(use[2], true, painter, QRect(r.x(), r.y()+1, r.width(), r.height()-2),
                                      true, SHADE_SBAR_DARK, SHADE_SBAR_LIGHT, false,
                                      APPEARANCE_GRADIENT, WIDGET_OTHER);

#ifndef QTC_SIMPLE_SCROLLBARS
                if(QTC_ROUNDED && SCROLLBAR_NONE==opts.scrollbarType)
                {
                    painter->fillRect(r, palette.background().color());
                    if(CE_ScrollBarAddPage==element)
                        drawBorder(painter, r.adjusted(-5, 0, 0, 0), option, ROUNDED_RIGHT, use);
                    else
                        drawBorder(painter, r.adjusted(0, 0, 5, 0), option, ROUNDED_LEFT, use);
                }
                else
#endif
                {
                    painter->setPen(use[QT_STD_BORDER]);
                    painter->drawLine(r.left(), r.top(), r.right(), r.top());
                    painter->drawLine(r.left(), r.bottom(), r.right(), r.bottom());

                    if(CE_ScrollBarAddPage==element && SCROLLBAR_NEXT==opts.scrollbarType)
                        painter->drawLine(r.right(), r.top(), r.right(), r.bottom());
                    else if(CE_ScrollBarSubPage==element && SCROLLBAR_PLATINUM==opts.scrollbarType)
                        painter->drawLine(r.left(), r.top(), r.left(), r.bottom());
                }
            }
            else
            {
                if(IS_FLAT(opts.appearance))
                    painter->fillRect(r.x()+1, r.y(), r.width()-2, r.height(), use[2]);
                else
                    drawBevelGradient(use[2], true, painter, QRect(r.x()+1, r.y(), r.width()-2, r.height()),
                                      false, SHADE_SBAR_DARK, SHADE_SBAR_LIGHT, false,
                                      APPEARANCE_GRADIENT, WIDGET_OTHER);

#ifndef QTC_SIMPLE_SCROLLBARS
                if(QTC_ROUNDED && SCROLLBAR_NONE==opts.scrollbarType)
                {
                    painter->fillRect(r, palette.background().color());
                    if(CE_ScrollBarAddPage==element)
                        drawBorder(painter, r.adjusted(0, -5, 0, 0), option, ROUNDED_BOTTOM, use);
                    else
                        drawBorder(painter, r.adjusted(0, 0, 0, 5), option, ROUNDED_TOP, use);
                }
                else
#endif
                {

                    painter->setPen(use[QT_STD_BORDER]);
                    painter->drawLine(r.left(), r.top(), r.left(), r.bottom());
                    painter->drawLine(r.right(), r.top(), r.right(), r.bottom());

                    if(CE_ScrollBarAddPage==element && (SCROLLBAR_NEXT==opts.scrollbarType || SCROLLBAR_NONE==opts.scrollbarType))
                        painter->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
                    else if(CE_ScrollBarSubPage==element && (SCROLLBAR_PLATINUM==opts.scrollbarType || SCROLLBAR_NONE==opts.scrollbarType))
                        painter->drawLine(r.left(), r.top(), r.right(), r.top());
                }
            }
            painter->restore();
            break;
        }
        case CE_ScrollBarSlider:
            painter->save();
            drawSbSliderHandle(painter, r, option);
            painter->restore();
            break;
        case CE_RadioButton:
        case CE_CheckBox:
            if (opts.crHighlight)
                if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option))
                {
                    QStyleOptionButton copy(*button);

                    copy.rect.adjust(2, 0, -2, 0);

                    if(button->state&State_MouseOver && button->state&State_Enabled)
                    {
                        QRect highlightRect(subElementRect(CE_RadioButton==element ? SE_RadioButtonFocusRect : SE_CheckBoxFocusRect,
                                                           option, widget));

                        if(Qt::RightToLeft==button->direction)
                            highlightRect.setRight(r.right());
                        else
                            highlightRect.setX(r.x());
                        painter->fillRect(highlightRect, shade(palette.background().color(), opts.highlightFactor));
                    }
                    QTC_BASE_STYLE::drawControl(element, &copy, painter, widget);
                    break;
                }
            // Fall through!
        default:
            QTC_BASE_STYLE::drawControl(element, option, painter, widget);
    }
}

void QtCurveStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    QRect                 r(option->rect);
    const QFlags<State> & state(option->state);
    const QPalette &      palette(option->palette);
    bool                  reverse(Qt::RightToLeft==option->direction);

    switch (control)
    {
        case CC_ToolButton:
            if (const QStyleOptionToolButton *toolbutton = qstyleoption_cast<const QStyleOptionToolButton *>(option))
            {
                QRect button(subControlRect(control, toolbutton, SC_ToolButton, widget)),
                      menuarea(subControlRect(control, toolbutton, SC_ToolButtonMenu, widget));
                State bflags(toolbutton->state);
                bool  etched(QTC_CAN_DO_EFFECT);

                if(bflags&State_MouseOver && !(bflags&State_Enabled))
                    bflags &= ~State_MouseOver;

                if (bflags&State_AutoRaise && !(bflags & State_MouseOver))
                    bflags &= ~State_Raised;

                if (toolbutton->activeSubControls & SC_ToolButton)
                    bflags |= State_Sunken;

                State mflags(bflags);

                if (toolbutton->activeSubControls & SC_ToolButtonMenu)
                    mflags |= State_Sunken;

                QStyleOption tool(0);
                tool.palette = toolbutton->palette;
                if (toolbutton->subControls & SC_ToolButton && (bflags & (State_Sunken | State_On | State_Raised)))
                {
                    tool.rect = toolbutton->subControls & SC_ToolButtonMenu ? button.united(menuarea) : button;
                    tool.state = bflags;

                    if(!(bflags&State_Sunken) && (mflags&State_Sunken))
                        tool.state &= ~State_MouseOver;

                    drawPrimitive(PE_PanelButtonTool, &tool, painter, widget);
                }

                if (toolbutton->subControls & SC_ToolButtonMenu)
                {
                    tool.rect = menuarea;
                    tool.state = mflags|State_Horizontal;

                    if(mflags & (State_Sunken | State_On | State_Raised))
                    {
                        const QColor *use(buttonColors(option));

                        if(mflags&State_Sunken)
                            tool.state&=~State_MouseOver;
                        drawLightBevel(painter, menuarea.adjusted(0, etched ? 1 : 0, 0, etched ? -1 : 0), &tool,
                                       reverse ? ROUNDED_LEFT : ROUNDED_RIGHT, getFill(&tool, use), use, true, WIDGET_NO_ETCH_BTN);
                    }

                    drawPrimitive(PE_IndicatorArrowDown, &tool, painter, widget);
                }

                if (toolbutton->state & State_HasFocus)
                {
                    QStyleOptionFocusRect fr;

                    fr.QStyleOption::operator=(*toolbutton);
                    fr.rect.adjust(3, etched ? 4 : 3, -3, etched ? -4 : -3);
#if QT_VERSION >= 0x040300
                    if (toolbutton->features & QStyleOptionToolButton::MenuButtonPopup)
#else
                    if (toolbutton->features & QStyleOptionToolButton::Menu)
#endif
                        fr.rect.adjust(0, 0, -(pixelMetric(QStyle::PM_MenuButtonIndicator, toolbutton, widget)-1), 0);
                    drawPrimitive(PE_FrameFocusRect, &fr, painter, widget);
                }
                QStyleOptionToolButton label = *toolbutton;
                int fw = pixelMetric(PM_DefaultFrameWidth, option, widget);
                label.rect = button.adjusted(fw, fw, -fw, -fw);
                label.state = bflags;
                drawControl(CE_ToolButtonLabel, &label, painter, widget);
            }
            break;
        case CC_GroupBox:
            if(opts.framelessGroupBoxes)
            {
                QFont font(painter->font());

                font.setBold(true);
                painter->save();
                painter->setFont(font);
            }
            QTC_BASE_STYLE::drawComplexControl(control, option, painter, widget);

            if(opts.framelessGroupBoxes)
                painter->restore();
            break;
        case CC_Q3ListView:
            if (const QStyleOptionQ3ListView *lv = qstyleoption_cast<const QStyleOptionQ3ListView *>(option))
            {
                int i;
                if (lv->subControls & SC_Q3ListView)
                    QCommonStyle::drawComplexControl(control, lv, painter, widget);
                if (lv->subControls & (SC_Q3ListViewBranch | SC_Q3ListViewExpand))
                {
                    if (lv->items.isEmpty())
                        break;

                    QStyleOptionQ3ListViewItem item(lv->items.at(0));
                    int                        y(r.y()),
                                               c,
                                               offset(0);
                    QPolygon                   lines;

                    painter->save();
                    if ((lv->activeSubControls & SC_All) && (lv->subControls & SC_Q3ListViewExpand))
                    {
                        c = 2;
                        if(opts.lvLines)
                        {
                            lines.resize(2);
                            lines[0] = QPoint(r.right(), r.top());
                            lines[1] = QPoint(r.right(), r.bottom());
                        }
                    }
                    else
                    {
                        int linetop(0),
                            linebot(0);
                        // each branch needs at most two lines, ie. four end points
                        offset = (item.itemY + item.height - y) % 2;
                        lines.resize(item.childCount * 4);
                        c = 0;

                        // skip the stuff above the exposed rectangle
                        for (i = 1; i < lv->items.size(); ++i)
                        {
                            QStyleOptionQ3ListViewItem child = lv->items.at(i);
                            if (child.height + y > 0)
                                break;
                            y += child.totalHeight;
                        }
                        int bx(r.width() / 2);

                        // paint stuff in the magical area
                        while (i < lv->items.size() && y < r.height())
                        {
                            QStyleOptionQ3ListViewItem child = lv->items.at(i);
                            if (child.features & QStyleOptionQ3ListViewItem::Visible)
                            {
                                int lh(!(item.features & QStyleOptionQ3ListViewItem::MultiLine)
                                        ? child.height
                                        : painter->fontMetrics().height() + 2 * lv->itemMargin);

                                lh = qMax(lh, QApplication::globalStrut().height());
                                if (lh % 2 > 0)
                                    ++lh;
                                linebot = y + lh / 2;
                                if (child.features & QStyleOptionQ3ListViewItem::Expandable
                                    || child.childCount > 0 && child.height > 0)
                                {
                                    // needs a box

                                    QRect ar(bx-4, linebot-4, 11, 11);

                                    if(opts.lvLines)
                                    {
                                        int lo(QTC_ROUNDED ? 2 : 0);

                                        painter->setPen(palette.mid().color());
                                        painter->drawLine(ar.x()+lo, ar.y(), (ar.x()+ar.width()-1)-lo, ar.y());
                                        painter->drawLine(ar.x()+lo, ar.y()+ar.height()-1, (ar.x()+ar.width()-1)-lo,
                                                        ar.y()+ar.height()-1);
                                        painter->drawLine(ar.x(), ar.y()+lo, ar.x(), (ar.y()+ar.height()-1)-lo);
                                        painter->drawLine(ar.x()+ar.width()-1, ar.y()+lo, ar.x()+ar.width()-1,
                                                        (ar.y()+ar.height()-1)-lo);

                                        if(QTC_ROUNDED)
                                        {
                                            painter->drawPoint(ar.x()+1, ar.y()+1);
                                            painter->drawPoint(ar.x()+1, ar.y()+ar.height()-2);
                                            painter->drawPoint(ar.x()+ar.width()-2, ar.y()+1);
                                            painter->drawPoint(ar.x()+ar.width()-2, ar.y()+ar.height()-2);

                                            QColor col(palette.mid().color());

                                            col.setAlphaF(0.5);
                                            painter->setPen(col);
                                            painter->drawLine(ar.x()+1, ar.y()+1, ar.x()+2, ar.y());
                                            painter->drawLine(ar.x()+ar.width()-2, ar.y(), ar.x()+ar.width()-1, ar.y()+1);
                                            painter->drawLine(ar.x()+1, ar.y()+ar.height()-2, ar.x()+2, ar.y()+ar.height()-1);
                                            painter->drawLine(ar.x()+ar.width()-2, ar.y()+ar.height()-1, ar.x()+ar.width()-1,
                                                            ar.y()+ar.height()-2);
                                        }
                                    }

                                    QStyleOption opt(*option);

                                    opt.state|=State_Enabled;

                                    drawArrow(painter, ar, &opt, child.state&State_Open
                                                                    ? PE_IndicatorArrowDown
                                                                    : reverse
                                                                        ? PE_IndicatorArrowLeft
                                                                        : PE_IndicatorArrowRight);

                                    if(opts.lvLines)
                                    {
                                        lines[c++] = QPoint(bx+1, linetop);
                                        lines[c++] = QPoint(bx+1, linebot - 4);
                                        lines[c++] = QPoint(bx + 6, linebot);
                                        lines[c++] = QPoint(r.width(), linebot);
                                        linetop = linebot + 6;
                                    }
                                }
                                else if(opts.lvLines)
                                {
                                    // just dotlinery
                                    lines[c++] = QPoint(bx+1, linebot -1);
                                    lines[c++] = QPoint(r.width(), linebot -1);
                                }
                                y += child.totalHeight;
                            }
                            ++i;
                        }

                        if(opts.lvLines)
                        {
                            // Expand line height to edge of rectangle if there's any
                            // visible child below
                            while (i < lv->items.size() && lv->items.at(i).height <= 0)
                                ++i;

                            if (i < lv->items.size())
                                linebot = r.height();

                            if (linetop < linebot)
                            {
                                lines[c++] = QPoint(bx+1, linetop);
                                lines[c++] = QPoint(bx+1, linebot-1);
                            }
                        }
                    }

                    if (opts.lvLines && (lv->subControls & SC_Q3ListViewBranch))
                    {
                        painter->setPen(palette.mid().color());

                        for(int line = 0; line < c; line += 2)
                            if (lines[line].y() == lines[line+1].y())
                                painter->drawLine(lines[line].x(), lines[line].y(), lines[line + 1].x(), lines[line].y());
                            else
                                painter->drawLine(lines[line].x(), lines[line].y(), lines[line].x(), lines[line + 1].y());
                    }
                    painter->restore();
                }
            }
            break;
        case CC_SpinBox:
            if (const QStyleOptionSpinBox *spinBox = qstyleoption_cast<const QStyleOptionSpinBox *>(option))
            {
                QRect frame(subControlRect(CC_SpinBox, option, SC_SpinBoxFrame, widget)),
                      up(subControlRect(CC_SpinBox, option, SC_SpinBoxUp, widget)),
                      down(subControlRect(CC_SpinBox, option, SC_SpinBoxDown, widget));;
                bool  doFrame(spinBox->frame && frame.isValid()),
                      sunken(state&State_Sunken),
                      enabled(state&State_Enabled),
                      mouseOver(state&State_MouseOver),
                      upIsActive(SC_SpinBoxUp==spinBox->activeSubControls),
                      downIsActive(SC_SpinBoxDown==spinBox->activeSubControls);

                CEtchCheck check(widget);

                if(up.isValid())
                {
                    QStyleOption opt;

                    up.setHeight(up.height()+1);
                    opt.rect=up;
                    opt.direction=option->direction;
                    opt.state|=(enabled ? State_Enabled : State_None)|(upIsActive && sunken ? State_Sunken : State_Raised)|
                              (upIsActive && !sunken && mouseOver ? State_MouseOver : State_None)|State_Horizontal;;

                    drawPrimitive(QAbstractSpinBox::PlusMinus==spinBox->buttonSymbols ? PE_IndicatorSpinPlus : PE_IndicatorSpinUp,
                                  &opt, painter, widget);
                }

                if(down.isValid())
                {
                    QStyleOption opt;

                    opt.rect=down;
                    opt.state|=(enabled ? State_Enabled : State_None)|(downIsActive && sunken ? State_Sunken : State_Raised)|
                               (downIsActive && !sunken && mouseOver ? State_MouseOver : State_None)|State_Horizontal;
                    opt.direction=option->direction;

                    drawPrimitive(QAbstractSpinBox::PlusMinus==spinBox->buttonSymbols ? PE_IndicatorSpinMinus : PE_IndicatorSpinDown,
                                  &opt, painter, widget);
                }
                if(doFrame)
                {
                    if(reverse)
                        frame.setX(frame.x()-1);
                    else
                        frame.setWidth(frame.width()+1);
                    drawEntryField(painter, frame, option, reverse ? ROUNDED_RIGHT : ROUNDED_LEFT, WIDGET_SPIN);
                }
            }
            break;
        case CC_Slider:
            if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
            {
                QRect groove(subControlRect(CC_Slider, option, SC_SliderGroove, widget)),
                      handle(subControlRect(CC_Slider, option, SC_SliderHandle, widget)),
                      ticks(subControlRect(CC_Slider, option, SC_SliderTickmarks, widget));
                bool  horizontal(slider->orientation == Qt::Horizontal),
                      ticksAbove(slider->tickPosition & QSlider::TicksAbove),
                      ticksBelow(slider->tickPosition & QSlider::TicksBelow);

                //The clickable region is 5 px wider than the visible groove for improved usability
//                 if (groove.isValid())
//                     groove = horizontal ? groove.adjusted(0, 5, 0, -5) : groove.adjusted(5, 0, -5, 0);

                if ((option->subControls & SC_SliderGroove) && groove.isValid())
                    drawSliderGroove(painter, groove, handle, slider);

                if ((option->subControls & SC_SliderHandle) && handle.isValid())
                {
                    drawSliderHandle(painter, handle, slider);

                    if (state&State_HasFocus)
                    {
                        QStyleOptionFocusRect fropt;
                        fropt.QStyleOption::operator=(*slider);
                        fropt.rect = slider->rect;

                        if(horizontal)
                            fropt.rect.adjust(0, 0, 0, -1);
                        else
                            fropt.rect.adjust(0, 0, -1, 0);

                        drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
                    }
                }

                if (option->subControls & SC_SliderTickmarks)
                {
                    QPen oldPen = painter->pen();
                    painter->setPen(backgroundColors(option)[QT_STD_BORDER]);
                    int tickSize(pixelMetric(PM_SliderTickmarkOffset, option, widget)),
                        available(pixelMetric(PM_SliderSpaceAvailable, slider, widget)),
                        interval(slider->tickInterval);
                    if (interval <= 0)
                    {
                        interval = slider->singleStep;
                        if (QStyle::sliderPositionFromValue(slider->minimum, slider->maximum, interval,
                                                            available)
                            - QStyle::sliderPositionFromValue(slider->minimum, slider->maximum,
                                                            0, available) < 3)
                            interval = slider->pageStep;
                    }
                    if (interval <= 0)
                        interval = 1;

                    int sliderLength(slider->maximum - slider->minimum + 1),
                        nticks(sliderLength / interval); // add one to get the end tickmark
                    if (sliderLength % interval > 0)
                        nticks++; // round up the number of tick marks

                    int v(slider->minimum),
                        len(pixelMetric(PM_SliderLength, slider, widget));

                    QVarLengthArray<QLine, 32> lines;
                    while (v <= slider->maximum)
                    {
                        int pos(sliderPositionFromValue(slider->minimum, slider->maximum,
                                                        v, (horizontal
                                                            ? slider->rect.width()
                                                            : slider->rect.height()) - len,
                                                        slider->upsideDown) + len / 2);

                        int extra(2); // - ((v == slider->minimum || v == slider->maximum) ? 1 : 0);

                        if (horizontal)
                        {
                            if (ticksAbove)
                                lines.append(QLine(pos, slider->rect.top() + extra,
                                                pos, slider->rect.top() + tickSize));
                            if (ticksBelow)
                                lines.append(QLine(pos, slider->rect.bottom() - extra,
                                                pos, slider->rect.bottom() - tickSize));
                        }
                        else
                        {
                            if (ticksAbove)
                                lines.append(QLine(slider->rect.left() + extra, pos,
                                                slider->rect.left() + tickSize, pos));
                            if (ticksBelow)
                                lines.append(QLine(slider->rect.right() - extra, pos,
                                                slider->rect.right() - tickSize, pos));
                        }

                        // in the case where maximum is max int
                        int nextInterval = v + interval;
                        if (nextInterval < v)
                            break;
                        v = nextInterval;
                    }
                    painter->drawLines(lines.constData(), lines.size());
                    painter->setPen(oldPen);
                }
            }
            break;
        case CC_TitleBar:
            if (const QStyleOptionTitleBar *titleBar = qstyleoption_cast<const QStyleOptionTitleBar *>(option))
            {
                painter->save();

                const int    buttonMargin(6);
                bool         active(state & State_Active),
                             roundKWinFull(ROUND_FULL==opts.round &&
                                            ((APP_KWIN==theThemedApp && !(titleBar->titleBarState&State_Raised)) ||
                                              titleBar->titleBarState&QtC_StateKWin));
                const QColor *btnCols(theThemedApp==APP_KWIN
                                        ? buttonColors(option)
                                        : getMdiColors(titleBar, active));
                QColor       textColor(theThemedApp==APP_KWIN
                                        ? option->palette.color(QPalette::WindowText)
                                        : active
                                            ? itsActiveMdiTextColor
                                            : itsMdiTextColor);
                QStyleOption opt(*option);

                opt.state=State_Horizontal|State_Enabled|State_Raised;

                if(APP_KWIN!=theThemedApp && roundKWinFull) // Set clipping for preview in kcmshell...
                {
                    int     x(r.x()), y(r.y()), w(r.width()), h(r.height());
                    QRegion mask(x+5, y+0, w-10, h);

                    mask += QRegion(x+0, y+5, 1, h-6);
                    mask += QRegion(x+1, y+3, 1, h-3);
                    mask += QRegion(x+2, y+2, 1, h-2);
                    mask += QRegion(x+3, y+1, 2, h-1);

                    mask += QRegion(x+w-1, y+5, 1, h-6);
                    mask += QRegion(x+w-2, y+3, 1, h-3);
                    mask += QRegion(x+w-3, y+2, 1, h-2);
                    mask += QRegion(x+w-5, y+1, 2, h-1);
                    painter->setClipRegion(mask);
                }

                drawLightBevel(painter, r, &opt,
                               titleBar->titleBarState&State_Raised
                                ? ROUNDED_NONE
                                : titleBar->titleBarState&State_Enabled
                                    ? ROUNDED_ALL
                                    : ROUNDED_TOP,
                               btnCols[2], btnCols, true,
                               titleBar->titleBarState&Qt::WindowMinimized ? WIDGET_MDI_WINDOW : WIDGET_MDI_WINDOW_TITLE);

                if(roundKWinFull)
                {
                    painter->setPen(btnCols[QT_STD_BORDER]);
                    painter->drawLine(r.x()+1, r.y()+4, r.x()+1, r.y()+3);
                    painter->drawPoint(r.x()+2, r.y()+2);
                    painter->drawLine(r.x()+3, r.y()+1, r.x()+4, r.y()+1);
                    painter->drawLine(r.x()+r.width()-2, r.y()+4, r.x()+r.width()-2, r.y()+3);
                    painter->drawPoint(r.x()+r.width()-3, r.y()+2);
                    painter->drawLine(r.x()+r.width()-4, r.y()+1, r.x()+r.width()-5, r.y()+1);
                    painter->setPen(btnCols[0]);
                    painter->drawLine(r.x()+2, r.y()+4, r.x()+2, r.y()+3);
                    painter->drawLine(r.x()+3, r.y()+2, r.x()+4, r.y()+2);
                    //painter->drawLine(r.x()+r.width()-3, r.y()+4, r.x()+r.width()-3, r.y()+3);
                    painter->drawLine(r.x()+r.width()-4, r.y()+2, r.x()+r.width()-5, r.y()+2);
                }

                if(!titleBar->text.isEmpty())
                {
                    QFont       font(painter->font());
                    QRect       textRect(subControlRect(CC_TitleBar, titleBar, SC_TitleBarLabel, widget));
                    QTextOption textOpt(Qt::AlignLeft | Qt::AlignVCenter);

                    font.setBold(true);
                    painter->setFont(font);
                    textOpt.setWrapMode(QTextOption::NoWrap);

                    QString str(painter->fontMetrics().elidedText(titleBar->text, Qt::ElideRight, textRect.width(), QPalette::WindowText));

                    painter->setPen(shadowColor(textColor));
                    painter->drawText(textRect.adjusted(1, 1, 1, 1), str, textOpt);
                    painter->setPen(textColor);
                    painter->drawText(textRect, str, textOpt);
                }

                // min button
                if ((titleBar->subControls & SC_TitleBarMinButton) && (titleBar->titleBarFlags & Qt::WindowMinimizeButtonHint) &&
                    !(titleBar->titleBarState& Qt::WindowMinimized))
                {
                    QRect rect(subControlRect(CC_TitleBar, titleBar, SC_TitleBarMinButton, widget));

                    if (rect.isValid())
                    {
                        bool sunken((titleBar->activeSubControls & SC_TitleBarMinButton) && (titleBar->state & State_Sunken));

                        drawMdiButton(painter, rect,
                                      (titleBar->activeSubControls & SC_TitleBarMinButton) && (titleBar->state & State_MouseOver),
                                      sunken, btnCols);
                        drawMdiIcon(painter, textColor, rect, sunken, buttonMargin, SC_TitleBarMinButton);
                    }
                }
                // max button
                if ((titleBar->subControls & SC_TitleBarMaxButton) && (titleBar->titleBarFlags & Qt::WindowMaximizeButtonHint) &&
                    !(titleBar->titleBarState & Qt::WindowMaximized))
                {
                    QRect rect(subControlRect(CC_TitleBar, titleBar, SC_TitleBarMaxButton, widget));

                    if (rect.isValid())
                    {
                        bool sunken((titleBar->activeSubControls & SC_TitleBarMaxButton) && (titleBar->state & State_Sunken));

                        drawMdiButton(painter, rect,
                                      (titleBar->activeSubControls & SC_TitleBarMaxButton) && (titleBar->state & State_MouseOver),
                                      sunken, btnCols);
                        drawMdiIcon(painter, textColor, rect, sunken, buttonMargin, SC_TitleBarMaxButton);
                    }
                }

                // close button
                if ((titleBar->subControls & SC_TitleBarCloseButton) && (titleBar->titleBarFlags & Qt::WindowSystemMenuHint))
                {
                    QRect rect(subControlRect(CC_TitleBar, titleBar, SC_TitleBarCloseButton, widget));

                    if (rect.isValid())
                    {
  
                        bool   sunken((titleBar->activeSubControls & SC_TitleBarCloseButton) && (titleBar->state & State_Sunken));
                        QColor closeCols[TOTAL_SHADES+1];

                        shadeColors(midColor(btnCols[ORIGINAL_SHADE], QColor(180,64,32)), closeCols);
                        drawMdiButton(painter, rect,
                                      (titleBar->activeSubControls & SC_TitleBarCloseButton) && (titleBar->state & State_MouseOver),
                                      sunken, closeCols);
                        drawMdiIcon(painter, textColor, rect, sunken, buttonMargin, SC_TitleBarCloseButton);
                    }
                }

                // normalize button
                if ((titleBar->subControls & SC_TitleBarNormalButton) &&
                    (((titleBar->titleBarFlags & Qt::WindowMinimizeButtonHint) &&
                    (titleBar->titleBarState & Qt::WindowMinimized)) ||
                    ((titleBar->titleBarFlags & Qt::WindowMaximizeButtonHint) &&
                    (titleBar->titleBarState & Qt::WindowMaximized))))
                {
                    QRect rect(subControlRect(CC_TitleBar, titleBar, SC_TitleBarNormalButton, widget));

                    if (rect.isValid())
                    {
                        bool sunken((titleBar->activeSubControls & SC_TitleBarNormalButton) && (titleBar->state & State_Sunken));

                        QRect normalButtonIconRect(rect.adjusted(buttonMargin, buttonMargin, -buttonMargin, -buttonMargin));
                        drawMdiButton(painter, rect,
                                      (titleBar->activeSubControls & SC_TitleBarNormalButton) && (titleBar->state & State_MouseOver),
                                      sunken, btnCols);
                        drawMdiIcon(painter, textColor, rect, sunken, buttonMargin, SC_TitleBarNormalButton);
                    }
                }

                // context help button
                if (titleBar->subControls & SC_TitleBarContextHelpButton && (titleBar->titleBarFlags & Qt::WindowContextHelpButtonHint))
                {
                    QRect rect(subControlRect(CC_TitleBar, titleBar, SC_TitleBarContextHelpButton, widget));

                    if (rect.isValid())
                    {
                        bool sunken((titleBar->activeSubControls & SC_TitleBarContextHelpButton) && (titleBar->state & State_Sunken));

                        drawMdiButton(painter, rect,
                                      (titleBar->activeSubControls & SC_TitleBarContextHelpButton) && (titleBar->state & State_MouseOver),
                                      sunken, btnCols);

                        QColor blend;
                        QImage image(standardPixmap(SP_TitleBarContextHelpButton, option, widget).toImage());
                        QColor alpha(textColor);

                        alpha.setAlpha(128);
                        image.setColor(1, textColor.rgba());
                        image.setColor(2, alpha.rgba());
                        painter->setRenderHint(QPainter::SmoothPixmapTransform);
                        painter->drawImage(rect.adjusted(4, 4, -4, -4), image);
                    }
                }

                // shade button
                if (titleBar->subControls & SC_TitleBarShadeButton && (titleBar->titleBarFlags & Qt::WindowShadeButtonHint))
                {
                    QRect rect(subControlRect(CC_TitleBar, titleBar, SC_TitleBarShadeButton, widget));

                    if (rect.isValid())
                    {
                        bool sunken((titleBar->activeSubControls & SC_TitleBarShadeButton) && (titleBar->state & State_Sunken));

                        drawMdiButton(painter, rect,
                                      (titleBar->activeSubControls & SC_TitleBarShadeButton) && (titleBar->state & State_MouseOver),
                                      sunken, btnCols);
                        drawMdiIcon(painter, textColor, rect, sunken, buttonMargin, SC_TitleBarShadeButton);
                    }
                }

                // unshade button
                if (titleBar->subControls & SC_TitleBarUnshadeButton && (titleBar->titleBarFlags & Qt::WindowShadeButtonHint))
                {
                    QRect rect(subControlRect(CC_TitleBar, titleBar, SC_TitleBarUnshadeButton, widget));

                    if (rect.isValid())
                    {
                        bool sunken((titleBar->activeSubControls & SC_TitleBarUnshadeButton) && (titleBar->state & State_Sunken));

                        drawMdiButton(painter, rect,
                                      (titleBar->activeSubControls & SC_TitleBarUnshadeButton) && (titleBar->state & State_MouseOver),
                                      sunken, btnCols);
                        drawMdiIcon(painter, textColor, rect, sunken, buttonMargin, SC_TitleBarUnshadeButton);
                    }
                }

                if ((titleBar->subControls & SC_TitleBarSysMenu) && (titleBar->titleBarFlags & Qt::WindowSystemMenuHint))
                {
                    QRect rect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarSysMenu, widget);
                    if (rect.isValid())
                    {
                        drawMdiButton(painter, rect,
                                      (titleBar->activeSubControls & SC_TitleBarSysMenu) && (titleBar->state & State_MouseOver),
                                      (titleBar->activeSubControls & SC_TitleBarSysMenu) && (titleBar->state & State_Sunken), btnCols);

                        if (!titleBar->icon.isNull())
                            titleBar->icon.paint(painter, rect);
                        else
                        {
                            QStyleOption tool(0);

                            tool.palette = palette;
                            tool.rect = rect;
                            painter->save();
                            drawItemPixmap(painter, rect, Qt::AlignCenter, standardIcon(SP_TitleBarMenuButton, &tool, widget).pixmap(16, 16));
                            painter->restore();
                        }
                    }
                }
                painter->restore();
            }
            break;
        case CC_ScrollBar:
            if (const QStyleOptionSlider *scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(option))
            {
                bool               useThreeButtonScrollBar(SCROLLBAR_KDE==opts.scrollbarType),
                                   horiz(Qt::Horizontal==scrollbar->orientation),
                                   maxed(scrollbar->minimum == scrollbar->maximum),
                                   atMin(maxed || scrollbar->sliderValue==scrollbar->minimum),
                                   atMax(maxed || scrollbar->sliderValue==scrollbar->maximum);
                State              sState((horiz ? State_Horizontal : State_None) |
                                          (maxed ? State_None : State_Enabled));
                QRect              subline(subControlRect(control, option, SC_ScrollBarSubLine, widget)),
                                   addline(subControlRect(control, option, SC_ScrollBarAddLine, widget)),
                                   subpage(subControlRect(control, option, SC_ScrollBarSubPage, widget)),
                                   addpage(subControlRect(control, option, SC_ScrollBarAddPage, widget)),
                                   slider(subControlRect(control, option, SC_ScrollBarSlider, widget)),
                                   first(subControlRect(control, option, SC_ScrollBarFirst, widget)),
                                   last(subControlRect(control, option, SC_ScrollBarLast, widget)),
                                   subline2(addline),
                                   sbRect(scrollbar->rect);
                QStyleOptionSlider opt(*scrollbar);

                if(reverse && horiz)
                {
                    bool tmp(atMin);

                    atMin=atMax;
                    atMax=tmp;
                }

                if (useThreeButtonScrollBar)
                {
                    int sbextent(pixelMetric(PM_ScrollBarExtent, scrollbar, widget));

                    if(horiz && reverse)
                        subline2=QRect((r.x()+r.width()-1)-sbextent, r.y(), sbextent, sbextent);
                    else if (horiz)
                        subline2.translate(-addline.width(), 0);
                    else
                        subline2.translate(0, -addline.height());

                    if (horiz)
                        subline.setWidth(sbextent);
                    else
                        subline.setHeight(sbextent);
                }

                // Draw trough...
                const QColor *trough(itsBackgroundCols); // backgroundColors(option));
                bool  noButtons(SCROLLBAR_NONE==opts.scrollbarType);
                QRect s2(subpage), a2(addpage);

#ifndef QTC_SIMPLE_SCROLLBARS
                if(noButtons)
                {
                    // Increase clipping to allow trough to "bleed" into slider corners...
                    a2.adjust(-3, -3, 3, 3);
                    s2.adjust(-3, -3, 3, 3);
                }
#endif

                painter->save();
#ifndef QTC_SIMPLE_SCROLLBARS
                painter->fillRect(sbRect, palette.background().color());
#endif
                painter->setClipRegion(QRegion(s2)+QRegion(addpage));

                opt.rect=sbRect;
                opt.state=scrollbar->state|State_On;
                opt.state&=~State_MouseOver;

                drawLightBevel(painter, sbRect, &opt,
#ifndef QTC_SIMPLE_SCROLLBARS
                            SCROLLBAR_NONE==opts.scrollbarType ? ROUNDED_ALL :
#endif
                            ROUNDED_NONE,
                            trough[2], trough, true, WIDGET_TROUGH);
                painter->setClipping(false);

                if((option->subControls&SC_ScrollBarSubLine) && subline.isValid())
                {
                    opt.rect=subline;
                    opt.state=scrollbar->state;
                    if (!(scrollbar->activeSubControls & SC_ScrollBarSubLine) ||
                        (useThreeButtonScrollBar && itsSbWidget && itsSbWidget==widget))
                        opt.state &= ~(State_Sunken | State_MouseOver);

                    drawControl(CE_ScrollBarSubLine, &opt, painter, widget);

                    if (useThreeButtonScrollBar && subline2.isValid())
                    {
                        opt.rect=subline2;
                        opt.state=scrollbar->state;
                        if ((!(scrollbar->activeSubControls & SC_ScrollBarSubLine)) || (itsSbWidget && itsSbWidget!=widget))
                            opt.state &= ~(State_Sunken | State_MouseOver);

                        drawControl(CE_ScrollBarSubLine, &opt, painter, widget);
                    }
                }

                if((option->subControls&SC_ScrollBarAddLine) && addline.isValid())
                {
                    opt.rect=addline;
                    opt.state=scrollbar->state;
                    if (!(scrollbar->activeSubControls & SC_ScrollBarAddLine))
                        opt.state &= ~(State_Sunken | State_MouseOver);
                    drawControl(CE_ScrollBarAddLine, &opt, painter, widget);
                }

                if((option->subControls&SC_ScrollBarFirst) && first.isValid())
                {
                    opt.rect=first;
                    opt.state=scrollbar->state;
                    if (!(scrollbar->activeSubControls & SC_ScrollBarFirst))
                        opt.state &= ~(State_Sunken | State_MouseOver);
                    drawControl(CE_ScrollBarFirst, &opt, painter, widget);
                }

                if((option->subControls&SC_ScrollBarLast) && last.isValid())
                {
                    opt.rect=last;
                    opt.state=scrollbar->state;
                    if (!(scrollbar->activeSubControls & SC_ScrollBarLast))
                        opt.state &= ~(State_Sunken | State_MouseOver);
                    drawControl(CE_ScrollBarLast, &opt, painter, widget);
                }

                if(((option->subControls&SC_ScrollBarSlider) || noButtons) && slider.isValid())
                {
                    // If "SC_ScrollBarSlider" wasn't specified, then we only want to draw the portion
                    // of the slider that overlaps with the trough. So, once again set the clipping
                    // region...
                    if(!(option->subControls&SC_ScrollBarSlider))
                        painter->setClipRegion(QRegion(s2)+QRegion(addpage));
#ifdef QTC_INCREASE_SB_SLIDER
                    else
                    {
                        if(atMax)
                            switch(opts.scrollbarType)
                            {
                                case SCROLLBAR_KDE:
                                case SCROLLBAR_WINDOWS:
                                case SCROLLBAR_PLATINUM:
                                    if(horiz)
                                        slider.adjust(0, 0, 1, 0);
                                    else
                                        slider.adjust(0, 0, 0, 1);
                                default:
                                    break;
                            }
                        if(atMin)
                            switch(opts.scrollbarType)
                            {
                                case SCROLLBAR_KDE:
                                case SCROLLBAR_WINDOWS:
                                case SCROLLBAR_NEXT:
                                    if(horiz)
                                        slider.adjust(-1, 0, 0, 0);
                                    else
                                        slider.adjust(0, -1, 0, 0);
                                default:
                                    break;
                            }
                    }
#endif

                    opt.rect=slider;
                    opt.state=scrollbar->state;
                    if (!(scrollbar->activeSubControls & SC_ScrollBarSlider))
                        opt.state &= ~(State_Sunken | State_MouseOver);
                    drawControl(CE_ScrollBarSlider, &opt, painter, widget);

                    // ### perhaps this should not be able to accept focus if maxedOut?
                    if(state&State_HasFocus)
                    {
                        opt.state=scrollbar->state;
                        opt.rect=QRect(slider.x()+2, slider.y()+2, slider.width()-5, slider.height()-5);
                        drawPrimitive(PE_FrameFocusRect, &opt, painter, widget);
                    }

#if !defined QTC_SIMPLE_SCROLLBARS
                    if(noButtons && (!atMin || !atMax))
                    {
                        painter->setPen(backgroundColors(option)[QT_STD_BORDER]);

                        if(horiz)
                        {
                            if(!atMin)
                            {
                                painter->drawLine(slider.x(), slider.y(),
                                                slider.x()+1, slider.y());
                                painter->drawLine(slider.x(), slider.y()+slider.height()-1,
                                                slider.x()+1, slider.y()+slider.height()-1);
                            }
                            if(!atMax)
                            {
                                painter->drawLine(slider.x()+slider.width()-1, slider.y(),
                                                slider.x()+slider.width()-2, slider.y());
                                painter->drawLine(slider.x()+slider.width()-1,
                                                slider.y()+slider.height()-1,
                                                slider.x()+slider.width()-2,
                                                slider.y()+slider.height()-1);
                            }
                        }
                        else
                        {
                            if(!atMin)
                            {
                                painter->drawLine(slider.x(), slider.y(),
                                                slider.x(), slider.y()+1);
                                painter->drawLine(slider.x()+slider.width()-1, slider.y(),
                                                slider.x()+slider.width()-1, slider.y()+1);
                            }
                            if(!atMax)
                            {
                                painter->drawLine(slider.x(), slider.y()+slider.height()-1,
                                                slider.x(), slider.y()+slider.height()-2);
                                painter->drawLine(slider.x()+slider.width()-1,
                                                slider.y()+slider.height()-1,
                                                slider.x()+slider.width()-1,
                                                slider.y()+slider.height()-2);
                            }
                        }
                    }
#endif
                    if(!(option->subControls&SC_ScrollBarSlider))
                        painter->setClipping(false);
                }
                painter->restore();
            }
            break;
        case CC_ComboBox:
            if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
            {
                painter->save();

                QRect           frame(subControlRect(CC_ComboBox, option, SC_ComboBoxFrame, widget)),
                                arrow(subControlRect(CC_ComboBox, option, SC_ComboBoxArrow, widget)),
                                field(subControlRect(CC_ComboBox, option, SC_ComboBoxEditField, widget));
                const QColor    *use(buttonColors(option));
                bool            sunken(state&State_On); // comboBox->listBox() ? comboBox->listBox()->isShown() : false),
                CEtchCheck      check(widget);

//                 if(sunken)
//                 {
//                     fillFlags|=State_Sunken;
//                     if(fillFlags&State_MouseOver)
//                         fillFlags-=State_MouseOver;
//                 }

//                 if(comboBox->editable)
//                 {
//                     painter->setPen(cg.background());
//                     drawRect(painter, r);
//                 }

                // This section fixes some drawng issues with krunner's combo on nvidia
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->fillRect(frame.adjusted(1, 1, -1, -1), palette.background().color());
                painter->setRenderHint(QPainter::Antialiasing, false);

                if(/*comboBox->frame &&*/ frame.isValid())
                {
                    QStyleOption frameOpt(*option);


                    if (comboBox->editable && !(comboBox->activeSubControls & SC_ComboBoxArrow))
                        frameOpt.state &= ~(State_Sunken | State_MouseOver);

                    if(!sunken)
                        frameOpt.state|=State_Raised;

                    if(opts.coloredMouseOver && frameOpt.state&State_MouseOver && comboBox->editable && !sunken)
                        frame.adjust(reverse ? 0 : 1, 0, reverse ? 1 : 0, 0);

                    drawLightBevel(painter, frame, &frameOpt,
                                   comboBox->editable ? (reverse ? ROUNDED_LEFT : ROUNDED_RIGHT) : ROUNDED_ALL,
                                   getFill(&frameOpt, use), use, true, WIDGET_STD_BUTTON);
                }

                if(/*controls&SC_ComboBoxArrow && */arrow.isValid())
                {
                    if(reverse)
                        arrow.adjust(-1, 0, 0, 0);
                    else
                        arrow.adjust(1, 0, 0, 0);

                    if(sunken)
                        arrow.adjust(1, 1, 1, 1);

                    if(comboBox->editable || !opts.gtkComboMenus)
                        drawArrow(painter, arrow, option, PE_IndicatorArrowDown);
                    else
                    {
                        QRect ar=QRect(arrow.x(), arrow.y()+(arrow.height()>>1)-(LARGE_ARR_HEIGHT+1), arrow.width(), LARGE_ARR_HEIGHT);
                        drawArrow(painter, ar, option, PE_IndicatorArrowUp);
                        ar=QRect(arrow.x(), arrow.y()+(arrow.height()>>1)+(LARGE_ARR_HEIGHT-3), arrow.width(), LARGE_ARR_HEIGHT);
                        drawArrow(painter, ar, option, PE_IndicatorArrowDown);
                    }
                }

                if(/*controls&SC_ComboBoxEditField &&*/ field.isValid())
                {
                    if(comboBox->editable)
                    {
                        field.adjust(-1,-1, 0, 1);
                        painter->setPen(state&State_Enabled ? palette.base().color() : palette.background().color());
                        drawRect(painter, field);
                        field.adjust(-2,-2, 2, 2);
                        drawEntryField(painter, field, option, reverse ? ROUNDED_RIGHT : ROUNDED_LEFT, WIDGET_STD_BUTTON);
                    }
                    else 
                    {
                        field.adjust(1, sunken ? 2 : 1, sunken ? 2 : 1, -1);
                        if(reverse)
                            field.adjust(4, 0, 4, 0);
                        painter->setPen(use[QT_BORDER(state&State_Enabled)]);
                        painter->drawLine(reverse ? field.left()-3 : field.right(), field.top(),
                                          reverse ? field.left()-3 : field.right(), field.bottom());
                        if(!sunken)
                        {
                            painter->setPen(use[0]);
                            painter->drawLine(reverse ? field.left()-2 : field.right()+1, field.top(),
                                              reverse ? field.left()-2 : field.right()+1, field.bottom());
                        }
                    }

                    if((state&State_Enabled) && (state&State_HasFocus) && (state&State_KeyboardFocusChange) && !comboBox->editable)
                    {
                        QStyleOptionFocusRect focus;

                        focus.rect = subControlRect(CC_ComboBox, option, SC_ComboBoxEditField, widget);

                        if(reverse)
                            focus.rect.adjust(3, 0, 0, 0);
                        else
                            focus.rect.adjust(0, 0, -2, 0);

                        if(QTC_CAN_DO_EFFECT)
                            focus.rect.adjust(0, 1, 0, -1);
                        drawPrimitive(PE_FrameFocusRect, &focus, painter, widget);
                    }
                }

                painter->restore();
            }
            break;
        default:
            QTC_BASE_STYLE::drawComplexControl(control, option, painter, widget);
            break;
    }
}

void QtCurveStyle::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text,
                                QPalette::ColorRole textRole) const
{
    if(QPalette::ButtonText==textRole)
    {
        const QPushButton *button=getButton(0L, painter);

        if(button && isMultiTabBarTab(button) && button->isChecked())
            textRole=QPalette::HighlightedText;
    }

    QTC_BASE_STYLE::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
}

#if 0 // Not sure about this...
void QtCurveStyle::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    QWidget *widget=dynamic_cast<QWidget *>(painter->device());

    if(widget && widget->parentWidget() && widget->inherits("QDockWidgetTitleButton") && !widget->parentWidget()->underMouse())
        return;

    QTC_BASE_STYLE::drawItemPixmap(painter, rect, alignment, pixmap);
}
#endif

QSize QtCurveStyle::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    QSize newSize(QTC_BASE_STYLE::sizeFromContents(type, option, size, widget));

    switch (type)
    {
        case CT_PushButton:
        {
            newSize.setWidth(newSize.width()+4);

            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))
            {
                const int constMinH(QTC_CAN_DO_EFFECT ? 29 : 27);

                if (!btn->text.isEmpty() && "..."!=btn->text && size.width() < 80 && newSize.width()<size.width())
                    newSize.setWidth(80);
                if (!btn->icon.isNull() && btn->iconSize.height() > 16)
                    newSize -= QSize(0, 2);
                if(!btn->text.isEmpty() && size.height() < constMinH)
                    newSize.setHeight(constMinH);
            }
            break;
        }
        case CT_RadioButton:
            ++newSize.rheight();
            ++newSize.rwidth();
            break;
#if 0
        case CT_Slider:
            if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
            {
                int tickSize = pixelMetric(PM_SliderTickmarkOffset, option, widget);
                if (slider->tickPosition & QSlider::TicksBelow) {
                    if (slider->orientation == Qt::Horizontal)
                        newSize.rheight() += tickSize;
                    else
                        newSize.rwidth() += tickSize;
                }
                if (slider->tickPosition & QSlider::TicksAbove) {
                    if (slider->orientation == Qt::Horizontal)
                        newSize.rheight() += tickSize;
                    else
                        newSize.rwidth() += tickSize;
                }
            }
            break;
#endif
        case CT_ScrollBar:
            if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option))
            {
                int scrollBarExtent(pixelMetric(PM_ScrollBarExtent, option, widget)),
                    scrollBarSliderMinimum(pixelMetric(PM_ScrollBarSliderMin, option, widget));

                if (scrollBar->orientation == Qt::Horizontal)
                    newSize = QSize(scrollBarExtent * numButtons(opts.scrollbarType) + scrollBarSliderMinimum, scrollBarExtent);
                else
                    newSize = QSize(scrollBarExtent, scrollBarExtent * numButtons(opts.scrollbarType) + scrollBarSliderMinimum);
            }
            break;
        case CT_SpinBox:
            //newSize.setHeight(sizeFromContents(CT_LineEdit, option, size, widget).height());
            newSize.rheight() -= (1 - newSize.rheight() & 1);
            break;
        case CT_ToolButton:
        {
            newSize.rheight() += 3;
            newSize.rwidth() += 3;

            // -- from kstyle & oxygen --
            // We want to avoid super-skiny buttons, for things like "up" when icons + text
            // For this, we would like to make width >= height.
            // However, once we get here, QToolButton may have already put in the menu area
            // (PM_MenuButtonIndicator) into the width. So we may have to take it out, fix things
            // up, and add it back in. So much for class-independent rendering...

            int menuAreaWidth(0);

            if (const QStyleOptionToolButton* tbOpt = qstyleoption_cast<const QStyleOptionToolButton*>(option))
            {
                if ((!tbOpt->icon.isNull()) && (!tbOpt->text.isEmpty()) && Qt::ToolButtonTextUnderIcon==tbOpt->toolButtonStyle)
                    newSize.setHeight(newSize.height()-5);

                if (tbOpt->features & QStyleOptionToolButton::MenuButtonPopup)
                    menuAreaWidth = pixelMetric(QStyle::PM_MenuButtonIndicator, option, widget);
            }

            newSize.setWidth(newSize.width() - menuAreaWidth);
            if (newSize.width() < newSize.height())
                newSize.setWidth(newSize.height());
            newSize.setWidth(newSize.width() + menuAreaWidth);

            break;
        }
        case CT_ComboBox:
        {
            const int constMinH(QTC_CAN_DO_EFFECT ? 26 : 24);

            newSize = sizeFromContents(CT_PushButton, option, size, widget);
            newSize.rwidth() += 30; // Make room for drop-down indicator
            newSize.rheight() += 4;

            if(size.height() < constMinH)
                newSize.setHeight(constMinH);
            break;
        }
        case CT_MenuItem:
            if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
            {
                int h(size.height());

                if (QStyleOptionMenuItem::Separator==mi->menuItemType)
                    h = mi->text.isEmpty() ? (opts.thinnerMenuItems ? 6 : 8) : mi->fontMetrics.lineSpacing();
                else
                {
                    h = qMax(h, mi->fontMetrics.height());
                    if (!mi->icon.isNull())
                        h = qMax(h, mi->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal).height());

                    if (h < 16)
                        h = 16;
                    h+=(opts.thinnerMenuItems ? 2 : 4);
                }

                newSize.setHeight(h);
            }
            break;
        case CT_MenuBarItem:
            newSize.setHeight(newSize.height() - 1);
            newSize.setWidth(newSize.width() + 1);
            break;
        default:
            break;
    }

    return newSize;
}

QRect QtCurveStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    QRect rect;
    switch (element)
    {
        case SE_RadioButtonIndicator:
            rect = visualRect(option->direction, option->rect,
                              QTC_BASE_STYLE::subElementRect(element, option, widget)).adjusted(0, 0, 1, 1);
            break;
        case SE_ProgressBarContents:
            return option->rect.adjusted(2, 2, -2, -2);
        case SE_ProgressBarGroove:
        case SE_ProgressBarLabel:
            return option->rect;
#if QT_VERSION >= 0x040300
        case SE_GroupBoxLayoutItem:
            rect = option->rect;
            if (const QStyleOptionGroupBox *groupBoxOpt = qstyleoption_cast<const QStyleOptionGroupBox *>(option))
                if (groupBoxOpt->subControls & (SC_GroupBoxCheckBox | SC_GroupBoxLabel))
                    rect.setTop(rect.top() + 2);    // eat the top margin a little bit
            break;
#endif
        default:
            return QTC_BASE_STYLE::subElementRect(element, option, widget);
    }

    return visualRect(option->direction, option->rect, rect);
}

QRect QtCurveStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                                   SubControl subControl, const QWidget *widget) const
{
    QRect r(option->rect);
    bool  reverse(Qt::RightToLeft==option->direction);

    switch (control)
    {
        case CC_ComboBox:
            if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
            {
                QRect r(QTC_BASE_STYLE::subControlRect(control, option, subControl, widget));

                if(SC_ComboBoxFrame==subControl)
                {
                    if(comboBox->editable)
                        if(reverse)
                            r=QRect(r.x(), r.y(), 19, r.height());
                        else
                            r=QRect((r.x()+r.width()-1)-18, r.y(), 19, r.height());
                }
                else if (reverse && SC_ComboBoxEditField==subControl)
                    r.adjust(2, 0, 0, 0);
                return r;
            }
            break;
        case CC_SpinBox:
            if (const QStyleOptionSpinBox *spinbox = qstyleoption_cast<const QStyleOptionSpinBox *>(option))
            {
                int   fw(spinbox->frame ? pixelMetric(PM_SpinBoxFrameWidth, spinbox, widget) : 0);
                QSize bs;

                bs.setHeight(r.height()>>1);
                if(bs.height()< 8)
                    bs.setHeight(8);
                bs.setWidth(15);
                bs=bs.expandedTo(QApplication::globalStrut());

                int extra(bs.height()*2==r.height() ? 0 : 1),
                    y(0), x(reverse ? 0 : r.width()-bs.width()),
                    rx(x-fw*2);

                switch(subControl)
                {
                    case SC_SpinBoxUp:
                        return QRect(x, y, bs.width(), bs.height());
                    case SC_SpinBoxDown:
                        return QRect(x, y+bs.height(), bs.width(), bs.height()+extra);
                    case SC_SpinBoxEditField:
                        return QRect(fw+(reverse ? bs.width() : 0), fw, rx, r.height()-2*fw);
                    case SC_SpinBoxFrame:
                        return reverse
                                ? QRect(r.x()+bs.width(), r.y(),
                                        r.width()-bs.width()-1, r.height())
                                : QRect(r.x(), r.y(),
                                        r.width()-bs.width(), r.height());
                    default:
                        break; // Remove compiler warnings...
                }
            }
            break;
        case CC_ScrollBar:
            if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option))
            {
                // Taken from kstyle.cpp (KDE 3) , and modified so as to allow for no scrollbar butttons...
                bool  threeButtonScrollBar(SCROLLBAR_KDE==opts.scrollbarType),
                      platinumScrollBar(SCROLLBAR_PLATINUM==opts.scrollbarType),
                      nextScrollBar(SCROLLBAR_NEXT==opts.scrollbarType),
                      noButtons(SCROLLBAR_NONE==opts.scrollbarType);
                QRect ret;
                bool  horizontal(Qt::Horizontal==scrollBar->orientation);
                int   sbextent(pixelMetric(PM_ScrollBarExtent, scrollBar, widget)),
                      sliderMaxLength(((scrollBar->orientation == Qt::Horizontal) ?
                                      scrollBar->rect.width() : scrollBar->rect.height()) - (sbextent * numButtons(opts.scrollbarType))),
                      sliderMinLength(pixelMetric(PM_ScrollBarSliderMin, scrollBar, widget)),
                      sliderLength;

                if (scrollBar->maximum != scrollBar->minimum)
                {
                    uint valueRange = scrollBar->maximum - scrollBar->minimum;
                    sliderLength = (scrollBar->pageStep * sliderMaxLength) / (valueRange + scrollBar->pageStep);

                    if (sliderLength < sliderMinLength || valueRange > INT_MAX / 2)
                        sliderLength = sliderMinLength;
                    if (sliderLength > sliderMaxLength)
                        sliderLength = sliderMaxLength;
                }
                else
                    sliderLength = sliderMaxLength;

                int sliderstart(sliderPositionFromValue(scrollBar->minimum,
                                                        scrollBar->maximum,
                                                        scrollBar->sliderPosition,
                                                        sliderMaxLength - sliderLength,
                                                        scrollBar->upsideDown));

                switch(opts.scrollbarType)
                {
                    case SCROLLBAR_KDE:
                    case SCROLLBAR_WINDOWS:
                        sliderstart+=sbextent;
                        break;
                    case SCROLLBAR_NEXT:
                        sliderstart+=sbextent*2;
                    default:
                        break;
                }

                // Subcontrols
                switch(subControl)
                {
                    case SC_ScrollBarSubLine:
                        if(noButtons)
                            return QRect();

                        // top/left button
                        if (platinumScrollBar)
                            if (horizontal)
                                ret.setRect(scrollBar->rect.width() - 2 * sbextent, 0, sbextent, sbextent);
                            else
                                ret.setRect(0, scrollBar->rect.height() - 2 * sbextent, sbextent, sbextent);
                        else if(threeButtonScrollBar)
                            if (horizontal)
                                ret.setRect(0, 0, scrollBar->rect.width() - sbextent +1, sbextent);
                            else
                                ret.setRect(0, 0, sbextent, scrollBar->rect.height() - sbextent +1);
                        else
                            ret.setRect(0, 0, sbextent, sbextent);
                        break;
                    case QTC_SB_SUB2:
                        if(threeButtonScrollBar)
                            if (horizontal)
                                if(reverse)
                                    ret.setRect(sbextent, 0, sbextent, sbextent);
                                else
                                    ret.setRect(scrollBar->rect.width() - 2 * sbextent, 0, sbextent, sbextent);
                            else
                                ret.setRect(0, scrollBar->rect.height() - 2 * sbextent, sbextent, sbextent);
                        else
                            return QRect();
                        break;
                    case SC_ScrollBarAddLine:
                        if(noButtons)
                            return QRect();

                        // bottom/right button
                        if (nextScrollBar)
                            if (horizontal)
                                ret.setRect(sbextent, 0, sbextent, sbextent);
                            else
                                ret.setRect(0, sbextent, sbextent, sbextent);
                        else
                            if (horizontal)
                                ret.setRect(scrollBar->rect.width() - sbextent, 0, sbextent, sbextent);
                            else
                                ret.setRect(0, scrollBar->rect.height() - sbextent, sbextent, sbextent);
                        break;
                    case SC_ScrollBarSubPage:
                        // between top/left button and slider
                        if (platinumScrollBar)
                            if (horizontal)
                                ret.setRect(0, 0, sliderstart, sbextent);
                            else
                                ret.setRect(0, 0, sbextent, sliderstart);
                        else if (nextScrollBar)
                            if (horizontal)
                                ret.setRect(sbextent*2, 0, sliderstart-2*sbextent, sbextent);
                            else
                                ret.setRect(0, sbextent*2, sbextent, sliderstart-2*sbextent);
                        else
                            if (horizontal)
                                ret.setRect(noButtons ? 0 : sbextent, 0,
                                            noButtons ? sliderstart
                                                    : (sliderstart - sbextent), sbextent);
                            else
                                ret.setRect(0, noButtons ? 0 : sbextent, sbextent,
                                            noButtons ? sliderstart : (sliderstart - sbextent));
                        break;
                    case SC_ScrollBarAddPage:
                    {
                        // between bottom/right button and slider
                        int fudge;

                        if (platinumScrollBar)
                            fudge = 0;
                        else if (nextScrollBar)
                            fudge = 2*sbextent;
                        else if(noButtons)
                            fudge = 0;
                        else
                            fudge = sbextent;

                        if (horizontal)
                            ret.setRect(sliderstart + sliderLength, 0,
                                        sliderMaxLength - sliderstart - sliderLength + fudge, sbextent);
                        else
                            ret.setRect(0, sliderstart + sliderLength, sbextent,
                                        sliderMaxLength - sliderstart - sliderLength + fudge);
                        break;
                    }
                    case SC_ScrollBarGroove:
                        if(noButtons)
                        {
                            if (horizontal)
                                ret=QRect(0, 0, scrollBar->rect.width(), scrollBar->rect.height()); 
                            else
                                ret=QRect(0, 0, scrollBar->rect.width(), scrollBar->rect.height()); 
                        }
                        else
                        {
                            int multi = threeButtonScrollBar ? 3 : 2,
                                fudge;

                            if (platinumScrollBar)
                                fudge = 0;
                            else if (nextScrollBar)
                                fudge = 2*sbextent;
                            else
                                fudge = sbextent;

                            if (horizontal)
                                ret=QRect(fudge, 0, scrollBar->rect.width() - sbextent * multi, scrollBar->rect.height()); 
                            else
                                ret=QRect(0, fudge, scrollBar->rect.width(), scrollBar->rect.height() - sbextent * multi);
                        }
                        break;
                    case SC_ScrollBarSlider:
                        if (horizontal)
                            ret=QRect(sliderstart, 0, sliderLength, sbextent);
                        else
                            ret=QRect(0, sliderstart, sbextent, sliderLength);
                        break;
                    default:
                        ret = QTC_BASE_STYLE::subControlRect(control, option, subControl, widget);
                        break;
                }
                return visualRect(scrollBar->direction/*Qt::LeftToRight*/, scrollBar->rect, ret);
            }
            break;
        case CC_Slider:
            if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
                if(SLIDER_TRIANGULAR==opts.sliderStyle)
                {
                    int   tickSize(pixelMetric(PM_SliderTickmarkOffset, option, widget));
                    QRect rect(QTC_BASE_STYLE::subControlRect(control, option, subControl, widget));

                    switch (subControl)
                    {
                        case SC_SliderHandle:
                            if (slider->orientation == Qt::Horizontal)
                            {
                                rect.setWidth(11);
                                rect.setHeight(15);
                                int centerY(r.center().y() - rect.height() / 2);
                                if (slider->tickPosition & QSlider::TicksAbove)
                                    centerY += tickSize;
                                if (slider->tickPosition & QSlider::TicksBelow)
                                    centerY -= (tickSize-1);
                                rect.moveTop(centerY);
                            }
                            else
                            {
                                rect.setWidth(15);
                                rect.setHeight(11);
                                int centerX(r.center().x() - rect.width() / 2);
                                if (slider->tickPosition & QSlider::TicksAbove)
                                    centerX += tickSize;
                                if (slider->tickPosition & QSlider::TicksBelow)
                                    centerX -= (tickSize-1);
                                rect.moveLeft(centerX);
                            }
                            break;
                        case SC_SliderGroove:
                        {
                            QPoint grooveCenter(r.center());

                            if (Qt::Horizontal==slider->orientation)
                            {
                                rect.setHeight(13);
                                --grooveCenter.ry();
                                if (slider->tickPosition & QSlider::TicksAbove)
                                    grooveCenter.ry() += (tickSize+2);
                                if (slider->tickPosition & QSlider::TicksBelow)
                                    grooveCenter.ry() -= (tickSize-1);
                            }
                            else
                            {
                                rect.setWidth(13);
                                --grooveCenter.rx();
                                if (slider->tickPosition & QSlider::TicksAbove)
                                    grooveCenter.rx() += (tickSize+2);
                                if (slider->tickPosition & QSlider::TicksBelow)
                                    grooveCenter.rx() -= (tickSize-1);
                            }
                            rect.moveCenter(grooveCenter);
                            break;
                        }
                        default:
                            break;
                    }
                    return rect;
                }
                else
                {
                    int  tickOffset(slider->tickPosition&QSlider::TicksAbove ||
                                    slider->tickPosition&QSlider::TicksBelow
                                        ? pixelMetric(PM_SliderTickmarkOffset, slider, widget)
                                        : pixelMetric(PM_SliderTickmarkOffset, slider, widget)/2),
                         thickness(pixelMetric(PM_SliderControlThickness, slider, widget));
                    bool horizontal(Qt::Horizontal==slider->orientation);

                    switch (subControl)
                    {
                        case SC_SliderHandle:
                        {
                            int len(pixelMetric(PM_SliderLength, slider, widget)),
                                sliderPos(sliderPositionFromValue(slider->minimum, slider->maximum,
                                                                slider->sliderPosition,
                                                                (horizontal ? r.width()
                                                                            : r.height()) - len,
                                                                slider->upsideDown));

                            if (horizontal)
                                r.setRect(r.x() + sliderPos, r.y() + tickOffset, len, thickness);
                            else
                                r.setRect(r.x() + tickOffset, r.y() + sliderPos, thickness, len);
                            break;
                        }
                        case SC_SliderGroove:
                            if (horizontal)
                                r.setRect(r.x(), r.y() + tickOffset, r.width(), thickness);
                            else
                                r.setRect(r.x() + tickOffset, r.y(), thickness, r.height());
                            break;
                        default:
                            break;
                    }
                    return visualRect(slider->direction, r, r);
                }
            break;
        case CC_GroupBox:
            if(opts.framelessGroupBoxes && (SC_GroupBoxCheckBox==subControl || SC_GroupBoxLabel==subControl))
                if (const QStyleOptionGroupBox *groupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(option))
                {
                    QFont font(widget ? widget->font() : QApplication::font());

                    font.setBold(true);

                    QFontMetrics fontMetrics(font);
                    int          h(fontMetrics.height()),
                                 tw(fontMetrics.size(Qt::TextShowMnemonic, groupBox->text + QLatin1Char(' ')).width()),
                                 indicatorWidth(pixelMetric(PM_IndicatorWidth, option, widget)),
                                 indicatorSpace(pixelMetric(PM_CheckBoxLabelSpacing, option, widget) - 1);
                    bool         hasCheckBox(groupBox->subControls & QStyle::SC_GroupBoxCheckBox);
                    int          checkBoxSize(hasCheckBox ? (indicatorWidth + indicatorSpace) : 0);

                    r.setHeight(h);

                    // Adjusted rect for label + indicatorWidth + indicatorSpace
                    r=alignedRect(groupBox->direction, groupBox->textAlignment, QSize(tw + checkBoxSize, h), r);

                    // Adjust totalRect if checkbox is set
                    if (hasCheckBox)
                    {
                        int left = 0;

                        if (SC_GroupBoxCheckBox==subControl) // Adjust for check box
                        {
                            int indicatorHeight(pixelMetric(PM_IndicatorHeight, option, widget)),
                                top(r.top() + (fontMetrics.height() - indicatorHeight) / 2);

                            left = reverse ? (r.right() - indicatorWidth) : r.left();
                            r.setRect(left, top, indicatorWidth, indicatorHeight);
                        }
                        else // Adjust for label
                        {
                            left = reverse ? r.left() : (r.left() + checkBoxSize - 2);
                            r.setRect(left, r.top(), r.width() - checkBoxSize, r.height());
                        }
                    }
                    return r;
                }
        break;
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(option))
        {
            bool isMinimized(tb->titleBarState&Qt::WindowMinimized),
                 isMaximized(tb->titleBarState&Qt::WindowMaximized);

            if( (isMaximized && SC_TitleBarMaxButton==subControl) ||
                (isMinimized && SC_TitleBarMinButton==subControl) ||
                (isMinimized && SC_TitleBarShadeButton==subControl) ||
                (!isMinimized && SC_TitleBarUnshadeButton==subControl))
                return QRect();

            readMdiPositions();

            const int windowMargin(2);
            const int controlSize(tb->rect.height() - windowMargin *2);

            QList<int>::ConstIterator it(itsMdiButtons[0].begin()),
                                      end(itsMdiButtons[0].end());
            int                       sc(SC_TitleBarUnshadeButton==subControl
                                        ? SC_TitleBarShadeButton
                                        : SC_TitleBarNormalButton==subControl
                                            ? isMaximized
                                                ? SC_TitleBarMaxButton
                                                : SC_TitleBarMinButton
                                            : subControl),
                                      pos(0),
                                      totalLeft(0),
                                      totalRight(0);
            bool                      rhs(false),
                                      found(false);

            for(; it!=end; ++it)
                if(SC_TitleBarCloseButton==(*it) || WINDOWTITLE_SPACER==(*it) || tb->titleBarFlags&(toHint(*it)))
                {
                    totalLeft+=WINDOWTITLE_SPACER==(*it) ? controlSize/2 : controlSize;
                    if(*it==sc)
                        found=true;
                    else if(!found)
                        pos+=WINDOWTITLE_SPACER==(*it) ? controlSize/2 : controlSize;
                }

            if(!found)
            {
                pos=0;
                rhs=true;
            }

            it=itsMdiButtons[1].begin();
            end=itsMdiButtons[1].end();
            for(; it!=end; ++it)
                if(SC_TitleBarCloseButton==(*it) || WINDOWTITLE_SPACER==(*it) || tb->titleBarFlags&(toHint(*it)))
                {
                    if(WINDOWTITLE_SPACER!=(*it) || totalRight)
                        totalRight+=WINDOWTITLE_SPACER==(*it) ? controlSize/2 : controlSize;
                    if(rhs)
                        if(*it==sc)
                        {
                            pos+=controlSize;
                            found=true;
                        }
                        else if(found)
                            pos+=WINDOWTITLE_SPACER==(*it) ? controlSize/2 : controlSize;
                }

            totalLeft+=(windowMargin*(totalLeft ? 2 : 1));
            totalRight+=(windowMargin*(totalRight ? 2 : 1));

            if(SC_TitleBarLabel==subControl)
                r.adjust(totalLeft, 0, -totalRight, 0);
            else if(!found)
                return QRect();
            else if(rhs)
                r.setRect(r.right()-(pos+windowMargin),
                          r.top()+windowMargin,
                          controlSize, controlSize);
            else
                r.setRect(r.left()+windowMargin+pos, r.top()+windowMargin,
                          controlSize, controlSize);
            return visualRect(tb->direction, tb->rect, r);
        }
        default:
            break;
    }

    return QTC_BASE_STYLE::subControlRect(control, option, subControl, widget);
}

QStyle::SubControl QtCurveStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                                       const QPoint &pos, const QWidget *widget) const
{
    itsSbWidget=NULL;
    switch (control)
    {
        case CC_ScrollBar:
            if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option))
            {
                if (subControlRect(control, scrollBar, SC_ScrollBarSlider, widget).contains(pos))
                    return SC_ScrollBarSlider;

                if (subControlRect(control, scrollBar, SC_ScrollBarAddLine, widget).contains(pos))
                    return SC_ScrollBarAddLine;

                if (subControlRect(control, scrollBar, SC_ScrollBarSubPage, widget).contains(pos))
                    return SC_ScrollBarSubPage;

                if (subControlRect(control, scrollBar, SC_ScrollBarAddPage, widget).contains(pos))
                    return SC_ScrollBarAddPage;

                if (subControlRect(control, scrollBar, SC_ScrollBarSubLine, widget).contains(pos))
                {
                    if (SCROLLBAR_KDE==opts.scrollbarType && subControlRect(control, scrollBar, QTC_SB_SUB2, widget).contains(pos))
                        itsSbWidget=widget;
                    return SC_ScrollBarSubLine;
                }
            }
        default:
            break;
    }

    return QTC_BASE_STYLE::hitTestComplexControl(control, option,  pos, widget);
}

static int getStripedPbarSize(int width)
{
    int chunk2(PROGRESS_CHUNK_WIDTH*2);

    return ((width/chunk2)+((width%chunk2) ? 1 : 0))*chunk2;
}

void QtCurveStyle::drawProgressBevelGradient(QPainter *p, const QRect &origRect, const QStyleOption *option, bool horiz, double shadeTop,
                                             double shadeBot, EAppearance bevApp) const
{
    bool    vertical(!horiz);
    int     size(STRIPE_DIAGONAL==opts.stripedProgress ? getStripedPbarSize(vertical ? origRect.width() : origRect.height())
                                                       : PROGRESS_CHUNK_WIDTH*2);
    QRect   r(0, 0, horiz ? size : origRect.width(),
                    horiz ? origRect.height() : size);
    QtcKey  key(createKey(horiz ? r.height() : r.width(), itsMenuitemCols[ORIGINAL_SHADE].rgb(), horiz, true,
                          app2App(bevApp, false), WIDGET_PROGRESSBAR, shadeTop, shadeBot));
    QPixmap *pix(itsPixmapCache.object(key));

    if(!pix)
    {
        pix=new QPixmap(r.width(), r.height());

        QPainter pixPainter(pix);

        if(IS_FLAT(bevApp))
            pixPainter.fillRect(r, itsMenuitemCols[ORIGINAL_SHADE]);
        else
            drawBevelGradientReal(itsMenuitemCols[ORIGINAL_SHADE], true, &pixPainter, r, horiz, shadeTop, shadeBot, false,
                                  bevApp, WIDGET_PROGRESSBAR);

        switch(opts.stripedProgress)
        {
            default:
            case STRIPE_NONE:
                break;
            case STRIPE_PLAIN:
                drawBevelGradientReal(itsMenuitemCols[1], true, &pixPainter,
                                        horiz
                                        ? QRect(r.x(), r.y(), PROGRESS_CHUNK_WIDTH, r.height())
                                        : QRect(r.x(), r.y(), r.width(), PROGRESS_CHUNK_WIDTH),
                                        horiz, shadeTop, shadeBot, false, bevApp, WIDGET_PROGRESSBAR);
                break;
            case STRIPE_DIAGONAL:
            {
                QRegion  reg;

                for(int offset=0; offset<(size*2); offset+=(PROGRESS_CHUNK_WIDTH*2))
                {
                    QPolygon a;

                    if(vertical)
                        a.setPoints(4, r.x(),           r.y()+offset,
                                       r.x()+r.width(), (r.y()+offset)-size,
                                       r.x()+r.width(), (r.y()+offset+PROGRESS_CHUNK_WIDTH)-size,
                                       r.x(),           r.y()+offset+PROGRESS_CHUNK_WIDTH);
                    else
                        a.setPoints(4, r.x()+offset,                             r.y(),
                                       r.x()+offset+PROGRESS_CHUNK_WIDTH,        r.y(),
                                       (r.x()+offset+PROGRESS_CHUNK_WIDTH)-size, r.y()+r.height(),
                                       (r.x()+offset)-size,                      r.y()+r.height());

                    reg+=QRegion(a);
                }

                pixPainter.setClipRegion(reg);
                if(IS_FLAT(bevApp))
                    pixPainter.fillRect(r, itsMenuitemCols[1]);
                else
                    drawBevelGradientReal(itsMenuitemCols[1], true, &pixPainter, r,
                                          horiz, shadeTop, shadeBot, false, bevApp, WIDGET_PROGRESSBAR);
            }
        }

        pixPainter.end();
        itsPixmapCache.insert(key, pix, pix->width()*pix->height()*(pix->depth()/8));
    }
    p->setClipRect(origRect);
    QRect fillRect(origRect);

    if(opts.animatedProgress)
    {
        int animShift=vertical || option->state&QTC_STATE_REVERSE ? PROGRESS_CHUNK_WIDTH : -PROGRESS_CHUNK_WIDTH;

        if(vertical || option->state&QTC_STATE_REVERSE)
            animShift -= (itsAnimateStep/2) % (PROGRESS_CHUNK_WIDTH*2);
        else
            animShift += (itsAnimateStep/2) % (PROGRESS_CHUNK_WIDTH*2);

        if(horiz)
            fillRect.adjust(animShift-PROGRESS_CHUNK_WIDTH, 0, PROGRESS_CHUNK_WIDTH, 0);
        else
            fillRect.adjust(0, animShift-PROGRESS_CHUNK_WIDTH, 0, PROGRESS_CHUNK_WIDTH);
    }
    p->drawTiledPixmap(fillRect, *pix);
    p->setClipping(false);
}

void QtCurveStyle::drawBevelGradient(const QColor &base, bool increase, QPainter *p,
                                     const QRect &origRect, bool horiz, double shadeTop,
                                     double shadeBot, bool sel, EAppearance bevApp, EWidget w) const
{
    if(IS_FLAT(bevApp))
        p->fillRect(origRect, base);
    else
    {
        EAppearance app(APPEARANCE_BEVELLED!=bevApp || WIDGET_BUTTON(w) || WIDGET_LISTVIEW_HEADER==w || WIDGET_NO_ETCH_BTN==w
                            ? bevApp
                            : APPEARANCE_GRADIENT);

        if(WIDGET_PROGRESSBAR==w)
            drawBevelGradientReal(base, increase, p, origRect, horiz, shadeTop, shadeBot, sel, app, w);
        else
        {
            QRect   r(0, 0, horiz ? QTC_PIXMAP_DIMENSION : origRect.width(),
                            horiz ? origRect.height() : QTC_PIXMAP_DIMENSION);
            QtcKey  key(createKey(horiz ? r.height() : r.width(), base.rgb(), horiz, increase,
                                  app2App(app, sel), w, shadeTop, shadeBot));
            QPixmap *pix(itsPixmapCache.object(key));

            if(!pix)
            {
                pix=new QPixmap(r.width(), r.height());

                QPainter pixPainter(pix);

                drawBevelGradientReal(base, increase, &pixPainter, r, horiz, shadeTop, shadeBot, sel, app, w);
                pixPainter.end();
                itsPixmapCache.insert(key, pix, pix->width()*pix->height()*(pix->depth()/8));
            }
            p->drawTiledPixmap(origRect, *pix);
        }
    }
}

void QtCurveStyle::drawBevelGradientReal(const QColor &base, bool increase, QPainter *p,
                                         const QRect &r, bool horiz, double shadeTop,
                                         double shadeBot, bool sel, EAppearance app, EWidget w) const
{
    bool selected(opts.colorSelTab && (WIDGET_TAB_TOP==w || WIDGET_TAB_BOT==w) ? false : sel);

    if(!selected && (IS_GLASS(app) || APPEARANCE_SPLIT_GRADIENT==app))
    {
        if(WIDGET_TAB_BOT==w)
        {
            double t(shadeTop);
            shadeTop=shadeBot;
            shadeBot=t;
        }

        double shadeTopA(WIDGET_TAB_BOT==w
                            ? 1.0
                            : APPEARANCE_SPLIT_GRADIENT==app
                                ? shadeTop
                                : shadeTop*SHADE_GLASS_TOP_A(app, w)),
               shadeTopB(WIDGET_TAB_BOT==w
                            ? 1.0
                            : APPEARANCE_SPLIT_GRADIENT==app
                                ? shadeTop-((shadeTop-shadeBot)*SPLIT_GRADIENT_FACTOR)
                                : shadeTop*SHADE_GLASS_TOP_B(app, w)),
               shadeBotA(WIDGET_TAB_TOP==w
                            ? 1.0
                            : APPEARANCE_SPLIT_GRADIENT==app
                                ? shadeBot+((shadeTop-shadeBot)*SPLIT_GRADIENT_FACTOR)
                                : shadeBot*SHADE_GLASS_BOT_A(app)),
               shadeBotB(WIDGET_TAB_TOP==w
                            ? 1.0
                            : APPEARANCE_SPLIT_GRADIENT==app
                                ? shadeBot
                                : shadeBot*SHADE_GLASS_BOT_B(app));

        QColor topA, topB, botA, botB;

        shade(base, &topA, shadeTopA);
        shade(base, &topB, shadeTopB);
        shade(base, &botA, shadeBotA);
        shade(base, &botB, shadeBotB);

        QLinearGradient grad(r.topLeft(), horiz ? r.bottomLeft() : r.topRight());

        grad.setColorAt(0, increase ? topA : topB);
        grad.setColorAt(0.499999999999999999, increase ? topB : topA);
        grad.setColorAt(0.5, increase ? botA : botB);
        grad.setColorAt(1, increase ? botB : botA);
        p->fillRect(r, QBrush(grad));
    }
    else if(!selected && APPEARANCE_BEVELLED==app &&
            ((horiz ? r.height()
                    : r.width()) > (((WIDGET_BUTTON(w) ? 2 : 1)*BEVEL_BORDER(w))+4)))
    {
        if(WIDGET_LISTVIEW_HEADER==w)
        {
            QColor bot;
            QRect  r1(r), r2(r);

            if(horiz)
            {
                r2.setHeight(BEVEL_BORDER(w));
                r1.setHeight(r.height()-r2.height());
                r2.moveTop(r.y()+r1.height());
            }
            else
            {
                r2.setWidth(BEVEL_BORDER(w));
                r1.setWidth(r.width()-r2.width());
                r2.moveLeft(r.x()+r1.width());
            }
            shade(base, &bot, SHADE_BEVEL_BOT(w));
            p->fillRect(r1, base);

            QLinearGradient grad(r2.topLeft(), horiz ? r2.bottomLeft() : r2.topRight());

            grad.setColorAt(0, base);
            grad.setColorAt(1, bot);
            p->fillRect(r2, QBrush(grad));
        }
        else
        {
            QColor bot, midTop, midBot, top;

            shade(base, &top, SHADE_BEVEL_TOP);
            shade(base, &midTop, SHADE_BEVEL_MID_TOP);
            shade(base, &midBot, SHADE_BEVEL_MID_BOT);
            shade(base, &bot, SHADE_BEVEL_BOT(w));

            qreal           borderSize(BEVEL_BORDER(w)/((qreal)(horiz ? r.height() : r.width())));
            QLinearGradient grad(r.topLeft(), horiz ? r.bottomLeft() : r.topRight());

            grad.setColorAt(0, top);
            grad.setColorAt(borderSize, midTop);
            grad.setColorAt(1.0-borderSize, midBot);
            grad.setColorAt(1, bot);
            p->fillRect(r, QBrush(grad));
        }
    }
    else
    {
        QColor top,
               bot,
               baseTopCol(opts.colorSelTab && sel && (WIDGET_TAB_TOP==w || WIDGET_TAB_BOT==w)
                                ? midColor(base, itsMenuitemCols[0], QTC_COLOR_SEL_TAB_FACTOR) : base);

        if(equal(1.0, shadeTop))
            top=baseTopCol;
        else
            shade(baseTopCol, &top, shadeTop);
        if(equal(1.0, shadeBot))
            bot=base;
        else
            shade(base, &bot, shadeBot);

        bool            inc(sel || APPEARANCE_INVERTED!=app ? increase : !increase);
        QLinearGradient grad(r.topLeft(), horiz ? r.bottomLeft() : r.topRight());

        grad.setColorAt(0, inc ? top : bot);
        grad.setColorAt(1, inc ? bot : top);
        p->fillRect(r, QBrush(grad));
    }
}

void QtCurveStyle::drawLightBevel(QPainter *p, const QRect &rOrig, const QStyleOption *option,
                                  int round, const QColor &fill, const QColor *custom,
                                  bool doBorder, EWidget w) const
{
    EAppearance  app(widgetApp(w, &opts));

    if(APPEARANCE_RAISED==app && (WIDGET_MDI_WINDOW==w || WIDGET_MDI_WINDOW_TITLE==w))
        app=APPEARANCE_FLAT;

    QRect        r(rOrig),
                 br(r);
    bool         bevelledButton((WIDGET_BUTTON(w) || WIDGET_NO_ETCH_BTN==w) && APPEARANCE_BEVELLED==app),
                 sunken(option->state &(/*State_Down | */State_On | State_Sunken)),
                 lightBorder(WIDGET_MDI_WINDOW!=w && WIDGET_MDI_WINDOW_TITLE!=w && QTC_DRAW_LIGHT_BORDER(sunken, w, app)),
                 doColouredMouseOver(doBorder && option->state&State_Enabled &&
                                     opts.coloredMouseOver && option->state&State_MouseOver &&
                                     (!IS_SLIDER(w) || (WIDGET_SB_SLIDER==w && MO_PLASTIK==opts.coloredMouseOver)) &&
                                     WIDGET_PROGRESSBAR!=w &&
                                     (/*option->state&QTC_TOGGLE_BUTTON ||*/ !sunken)),
                 plastikMouseOver(doColouredMouseOver && MO_PLASTIK==opts.coloredMouseOver),
                 colouredMouseOver(doColouredMouseOver && MO_COLORED==opts.coloredMouseOver),
                 doEtch(doBorder && ETCH_WIDGET(w) && QTC_CAN_DO_EFFECT),
                 horiz(isHoriz(option, w));
    int          dark(bevelledButton ? 2 : 4),
                 c1(sunken ? dark : 0);
    const QColor *cols(custom ? custom : itsBackgroundCols),
                 *border(colouredMouseOver ? borderColors(option, cols) : cols);

    p->save();

    if(doEtch)
    {
        r.adjust(0, 1, 0, -1);
        br=r;
    }

    if(!colouredMouseOver && lightBorder)
        br.adjust(1, 1,-1,-1);
    else if(colouredMouseOver || WIDGET_MDI_WINDOW==w || WIDGET_MDI_WINDOW_TITLE==w ||
            (!IS_GLASS(app) && !sunken && option->state&State_Raised))
    {
        if(colouredMouseOver)
            p->setPen(border[QTC_MO_STD_LIGHT(w, sunken)]);
        else
            p->setPen(border[c1]);
        if(colouredMouseOver || bevelledButton || APPEARANCE_RAISED==app)
        {
            //Left & top
            p->drawLine(br.x()+1, br.y()+2, br.x()+1, br.y()+br.height()-3);
            p->drawLine(br.x()+1, br.y()+1, br.x()+br.width()-2, br.y()+1);

            if(colouredMouseOver)
                p->setPen(border[QTC_MO_STD_DARK(w)]);
            else
                p->setPen(border[sunken ? 0 : dark]);
            //Right & bottom
            p->drawLine(br.x()+br.width()-2, br.y()+1, br.x()+br.width()-2, br.y()+br.height()-3);
            p->drawLine(br.x()+1, br.y()+br.height()-2, br.x()+br.width()-2, br.y()+br.height()-2);
            br.adjust(2, 2,-2,-2);
        }
        else
        {
            //Left & top
            p->drawLine(br.x()+1, br.y()+2, br.x()+1, br.y()+br.height()-(WIDGET_MDI_WINDOW_TITLE==w ? 1 : 2));
            if((WIDGET_MDI_WINDOW==w || WIDGET_MDI_WINDOW_TITLE==w) && APPEARANCE_SHINY_GLASS==app)
                br.adjust(2, 1, -1, -1);
            else
            {
                p->drawLine(br.x()+1, br.y()+1, br.x()+br.width()-2, br.y()+1);
                br.adjust(2, 2,-1,-1);
            }
        }
    }
    else
        br.adjust(1, 1,-1,-1);

    if(!colouredMouseOver && lightBorder && br.width()>0 && br.height()>0)
    {
        QColor col(cols[APPEARANCE_DULL_GLASS==app ? 1 : 0]);

        p->setPen(col);
        br=r;
        br.adjust(1,1,-1,-1);
        drawRect(p, br);

        if(WIDGET_PROGRESSBAR==w && !IS_GLASS(app))
            br.adjust(1,1,-1,-1);
        else if(horiz)
            br.adjust(1,0,-1,-1);
        else
            br.adjust(0,1,-1,-1);
    }

    // fill
    if(br.width()>0 && br.height()>0)
    {
        if(WIDGET_PROGRESSBAR==w && STRIPE_NONE!=opts.stripedProgress)
            drawProgressBevelGradient(p, br, option, horiz, getWidgetShade(w, true, sunken, app),
                                      getWidgetShade(w, false, sunken, app), app);
        else
        {
            drawBevelGradient(fill, !sunken, p, WIDGET_MDI_WINDOW_TITLE==w ? br.adjusted(0, 0, 0, 1) : br, horiz,
                              getWidgetShade(w, true, sunken, app),
                              getWidgetShade(w, false, sunken, app), sunken, app, w);

            if(plastikMouseOver)
            {
                if(WIDGET_SB_SLIDER==w)
                {
                    int len(QTC_SB_SLIDER_MO_LEN(horiz ? r.width() : r.height())),
                        so(lightBorder ? QTC_SLIDER_MO_BORDER : 1),
                        eo(len+so),
                        col(QTC_SLIDER_MO_SHADE);

                    if(horiz)
                    {
                        drawBevelGradient(itsMouseOverCols[col], !sunken, p, QRect(r.x()+so, r.y(), len, r.height()-1), horiz,
                                         getWidgetShade(w, true, sunken, app),
                                         getWidgetShade(w, false, sunken, app), sunken, app, w);
                        drawBevelGradient(itsMouseOverCols[col], !sunken, p, QRect(r.x()+r.width()-eo, r.y(), len, r.height()-1), horiz,
                                         getWidgetShade(w, true, sunken, app),
                                         getWidgetShade(w, false, sunken, app), sunken, app, w);
                    }
                    else
                    {
                        drawBevelGradient(itsMouseOverCols[col], !sunken, p, QRect(r.x(), r.y()+so, r.width()-1, len), horiz,
                                         getWidgetShade(w, true, sunken, app),
                                         getWidgetShade(w, false, sunken, app), sunken, app, w);
                        drawBevelGradient(itsMouseOverCols[col], !sunken, p, QRect(r.x(), r.y()+r.height()-eo, r.width()-1, len), horiz,
                                         getWidgetShade(w, true, sunken, app),
                                         getWidgetShade(w, false, sunken, app), sunken, app, w);
                    }
                }
                else
                {
                    bool horizontal((horiz && WIDGET_SB_BUTTON!=w)|| (!horiz && WIDGET_SB_BUTTON==w)),
                         thin(WIDGET_SB_BUTTON==w || WIDGET_SPIN==w || ((horiz ? r.height() : r.width())<16));

                    p->setPen(itsMouseOverCols[QTC_MO_PLASTIK_DARK(w)]);
                    p->setRenderHint(QPainter::Antialiasing, true);
                    if(horizontal)
                    {
                        drawAaLine(p, r.x()+1, r.y()+1, r.x()+r.width()-2, r.y()+1);
                        drawAaLine(p, r.x()+1, r.y()+r.height()-2, r.x()+r.width()-2, r.y()+r.height()-2);
                    }
                    else
                    {
                        drawAaLine(p, r.x()+1, r.y()+1, r.x()+1, r.y()+r.height()-2);
                        drawAaLine(p, r.x()+r.width()-2, r.y()+1, r.x()+r.width()-2, r.y()+r.height()-2);
                    }
                    if(!thin)
                    {
                        p->setPen(itsMouseOverCols[QTC_MO_PLASTIK_LIGHT(w)]);
                        if(horizontal)
                        {
                            drawAaLine(p, r.x()+1, r.y()+2, r.x()+r.width()-2, r.y()+2);
                            drawAaLine(p, r.x()+1, r.y()+r.height()-3, r.x()+r.width()-2, r.y()+r.height()-3);
                        }
                        else
                        {
                            drawAaLine(p, r.x()+2, r.y()+1, r.x()+2, r.y()+r.height()-2);
                            drawAaLine(p, r.x()+r.width()-3, r.y()+1, r.x()+r.width()-3, r.y()+r.height()-2);
                        }
                    }
                    p->setRenderHint(QPainter::Antialiasing, false);
                }
            }
        }
    }

    if(doBorder)
        drawBorder(p, r, option, round, cols, w);

    if(doEtch)
        drawEtch(p, rOrig, /*option, */true, true, EFFECT_SHADOW==opts.buttonEffect && WIDGET_BUTTON(w) && !sunken);

    p->restore();
}

void QtCurveStyle::drawEtch(QPainter *p, const QRect &r, /*const QStyleOption *option, */bool top, bool bot, bool raised) const
{
    if(top && !raised)
    {
        QColor darkCol(Qt::black); // itsBackgroundCols[2]);

        darkCol.setAlphaF(QTC_ETCH_TOP_MIDDLE_ALPHA);
        p->setPen(darkCol);
        p->drawLine(r.x()+2, r.y(), r.x()+r.width()-3, r.y());
        darkCol.setAlphaF(QTC_ETCH_TOP_SIDES_ALPHA);
        p->setPen(darkCol);
        p->drawPoint(r.x()+1, r.y());
        p->drawPoint(r.x()+r.width()-2, r.y());
        p->drawPoint(r.x(), r.y()+1);
        p->drawPoint(r.x()+r.width()-1, r.y()+1);
    }
    if(bot)
    {
        QColor col(raised ? Qt::black /*itsBackgroundCols[2]*/ : Qt::white); // backgroundColors(option)[0]);

        col.setAlphaF(raised ? QTC_ETCH_TOP_MIDDLE_ALPHA : QTC_ETCH_BOTTOM_MIDDLE_ALPHA);
        p->setPen(col);
        p->drawLine(r.x()+2, r.y()+r.height()-1, r.x()+r.width()-3, r.y()+r.height()-1);

        if(!raised)
        {
            col.setAlphaF(0.3);
            p->setPen(col);
            p->drawPoint(r.x()+1, r.y()+r.height()-2);
            p->drawPoint(r.x()+r.width()-2, r.y()+r.height()-2);
        }

        col.setAlphaF(raised ? QTC_ETCH_TOP_SIDES_ALPHA : QTC_ETCH_BOTTOM_SIDES_ALPHA);
        p->setPen(col);
        p->drawPoint(r.x()+1, r.y()+r.height()-1);
        p->drawPoint(r.x()+r.width()-2, r.y()+r.height()-1);
        p->drawPoint(r.x(), r.y()+r.height()-2);
        p->drawPoint(r.x()+r.width()-1, r.y()+r.height()-2);
    }
}

void QtCurveStyle::drawBorder(QPainter *p, const QRect &r, const QStyleOption *option,
                              int round, const QColor *custom, EWidget w,
                              EBorder borderProfile, bool doBlend, int borderVal) const
{
    if(ROUND_NONE==opts.round)
        round=ROUNDED_NONE;

    EAppearance  app(widgetApp(w, &opts));
    State        state(option->state);
    bool         enabled(state&State_Enabled),
                 hasFocus(state&State_HasFocus),
                 window(WIDGET_MDI_WINDOW==w || WIDGET_MDI_WINDOW_TITLE==w);
    const QColor *cols(enabled && hasFocus && (WIDGET_FRAME==w || WIDGET_ENTRY==w)
                        ? itsMenuitemCols
                        : custom
                            ? custom
                            : WIDGET_ENTRY==w
                                ? buttonColors(option)
                                : itsBackgroundCols);
    QColor       border(WIDGET_DEF_BUTTON==w && IND_FONT_COLOR==opts.defBtnIndicator &&
                        (option->state&State_Enabled)
                          ? option->palette.buttonText().color()
                          : cols[!enabled && (WIDGET_BUTTON(w) || WIDGET_SLIDER_TROUGH==w)
                                    ? QT_DISABLED_BORDER : borderVal]);
    double       xd(r.x()+0.5),
                 yd(r.y()+0.5),
                 radius(getRadius(opts.round, r.width(), r.height(), w, false)),
                 diameter(radius*2);
    int          width(r.width()-1),
                 height(r.height()-1);

    if(APPEARANCE_FLAT==app && window)
        app=APPEARANCE_RAISED;

    if(WIDGET_MDI_WINDOW!=w && WIDGET_MDI_WINDOW_TITLE!=w)
        p->setRenderHint(QPainter::Antialiasing, true);

    switch(borderProfile)
    {
        case BORDER_FLAT:
            break;
        case BORDER_RAISED:
        case BORDER_SUNKEN:
        {
            QColor       tl(cols[BORDER_RAISED==borderProfile ? 0 : QT_FRAME_DARK_SHADOW]),
                         br(cols[BORDER_RAISED==borderProfile ? QT_FRAME_DARK_SHADOW : 0]);
            QPainterPath topPath,
                         botPath;
            double       radiusi(getRadius(opts.round, r.width(), r.height(), w, true)),
                         diameteri(radiusi*2),
                         xdi(window ? r.x()+1 : xd+1),
                         ydi(window ? r.y()+1 : yd+1);
            int          widthi(width-2),
                         heighti(height-2);

            if(doBlend && !window)
            {
                tl.setAlphaF(QTC_BORDER_BLEND_ALPHA);
                br.setAlphaF(QTC_BORDER_BLEND_ALPHA);
            }

            p->setPen((enabled || BORDER_SUNKEN==borderProfile) &&
                      (BORDER_RAISED==borderProfile || hasFocus || APPEARANCE_FLAT!=app)
                            ? tl
                            : option->palette.background().color());

            if (!window && round&CORNER_TR)
            {
                topPath.moveTo(xdi+widthi, ydi+radiusi);
                topPath.arcTo(xdi+widthi-diameteri, ydi, diameteri, diameteri, 0, 90);
            }
            else
                topPath.moveTo(xdi+widthi, ydi);

            if (!window && round&CORNER_TL)
                topPath.arcTo(xdi, ydi, diameteri, diameteri, 90, 90);
            else
                topPath.lineTo(xdi, ydi);

            topPath.lineTo(xdi, !window && round&CORNER_BL ? ydi+heighti-radiusi : ydi+heighti);
            botPath.moveTo(xdi, !window && round&CORNER_BL ? ydi+heighti-radiusi : ydi+heighti);
            if (!window && round&CORNER_BL)
                botPath.arcTo(xdi, ydi+heighti-diameteri, diameteri, diameteri, 180, 90);

            if (!window && round&CORNER_BR)
                botPath.arcTo(xdi+widthi-diameteri, ydi+heighti-diameteri, diameteri, diameteri, 270, 90);
            else
                botPath.lineTo(xdi+widthi, ydi+heighti);

            botPath.lineTo(xdi+widthi, round&CORNER_TR ? ydi+radiusi : ydi);

            p->drawPath(topPath);
            p->setPen(enabled && (BORDER_SUNKEN==borderProfile || hasFocus || APPEARANCE_FLAT!=app)
                            ? br
                            : option->palette.background().color());
            p->drawPath(botPath);
        }
    }

    QPainterPath path;

    if (WIDGET_MDI_WINDOW_TITLE!=w && round&CORNER_BR)
        path.moveTo(xd+width, yd+height-radius);
    else
        path.moveTo(xd+width, yd+height);

    if (round&CORNER_TR)
        path.arcTo(xd+width-diameter, yd, diameter, diameter, 0, 90);
    else
        path.lineTo(xd+width, yd);

    if (round&CORNER_TL)
        path.arcTo(xd, yd, diameter, diameter, 90, 90);
    else
        path.lineTo(xd, yd);

    if (WIDGET_MDI_WINDOW_TITLE!=w && round&CORNER_BL)
        path.arcTo(xd, yd+height-diameter, diameter, diameter, 180, 90);
    else
        path.lineTo(xd, yd+height);

    if(WIDGET_MDI_WINDOW_TITLE!=w)
    {
        if (round&CORNER_BR)
            path.arcTo(xd+width-diameter, yd+height-diameter, diameter, diameter, 270, 90);
        else
            path.lineTo(xd+width, yd+height);
    }

    p->setPen(border);
    p->drawPath(path);
    if(WIDGET_MDI_WINDOW!=w && WIDGET_MDI_WINDOW_TITLE!=w)
        p->setRenderHint(QPainter::Antialiasing, false);

    if(ROUND_FULL==opts.round && window)
    {
        if(WIDGET_MDI_WINDOW==w)
        {
            p->drawPoint(r.x(), r.y()+r.height()-2);
            p->drawPoint(r.x()+1, r.y()+r.height()-1);
            p->drawPoint(r.x()+r.width()-2, r.y()+r.height()-1);
            p->drawPoint(r.x()+r.width()-1, r.y()+r.height()-2);
        }

        if(WIDGET_MDI_WINDOW_TITLE==w)
        {
            p->drawPoint(r.x()+2, r.y());
            p->drawPoint(r.x(), r.y()+2);
            p->drawPoint(r.x()+r.width()-1, r.y()+2);
            p->drawPoint(r.x()+r.width()-3, r.y());
            p->setPen(cols[0]);
            p->drawPoint(r.x()+1, r.y()+2);
            p->drawPoint(r.x()+2, r.y()+1);
        }
    }
}

void QtCurveStyle::drawMdiButton(QPainter *painter, const QRect &r, bool hover, bool sunken, const QColor *cols) const
{
    if(hover || sunken)
    {
        QStyleOption opt;

        opt.rect=r; // .adjusted(1, 1, -1, -1);
        opt.state=State_Enabled|State_Horizontal|State_Raised;
        if(hover)
            opt.state|=State_MouseOver;
        if(sunken)
            opt.state|=State_Sunken;

        drawLightBevel(painter, opt.rect, &opt, ROUNDED_ALL, getFill(&opt, cols), cols, true,
                       WIDGET_MDI_WINDOW_BUTTON);
    }
}

void QtCurveStyle::drawMdiIcon(QPainter *painter, const QColor &color, const QRect &r, bool sunken, int margin, SubControl button) const
{
    if(!sunken)
        drawWindowIcon(painter, shadowColor(color), r.adjusted(1, 1, 1, 1), sunken, margin, button);
    drawWindowIcon(painter, color, r, sunken, margin, button);
}

void QtCurveStyle::drawWindowIcon(QPainter *painter, const QColor &color, const QRect &r, bool sunken, int margin, SubControl button) const
{
    QRect rect(r);

    // Icons look best at 22x22...
    if(rect.height()>22)
    {
        int diff=(rect.height()-22)/2;
        rect.adjust(diff, diff, -diff, -diff);
    }

    if(sunken)
        rect.adjust(1, 1, 1, 1);

    if(margin)
        rect.adjust(margin, margin, -margin, -margin);

    painter->setPen(color);

    switch(button)
    {
        case SC_TitleBarMinButton:
            painter->drawLine(rect.center().x() - 2, rect.center().y() + 3, rect.center().x() + 3, rect.center().y() + 3);
            painter->drawLine(rect.center().x() - 2, rect.center().y() + 4, rect.center().x() + 3, rect.center().y() + 4);
            painter->drawLine(rect.center().x() - 3, rect.center().y() + 3, rect.center().x() - 3, rect.center().y() + 4);
            painter->drawLine(rect.center().x() + 4, rect.center().y() + 3, rect.center().x() + 4, rect.center().y() + 4);
            break;
        case SC_TitleBarMaxButton:
            painter->drawRect(rect.adjusted(0, 0, -1, -1));
            painter->drawLine(rect.left() + 1, rect.top() + 1,  rect.right() - 1, rect.top() + 1);
            painter->drawPoint(rect.topLeft());
            painter->drawPoint(rect.topRight());
            painter->drawPoint(rect.bottomLeft());
            painter->drawPoint(rect.bottomRight());
            break;
        case SC_TitleBarCloseButton:
            painter->drawLine(rect.left() + 1, rect.top(), rect.right(), rect.bottom() - 1);
            painter->drawLine(rect.left(), rect.top() + 1, rect.right() - 1, rect.bottom());
            painter->drawLine(rect.right() - 1, rect.top(), rect.left(), rect.bottom() - 1);
            painter->drawLine(rect.right(), rect.top() + 1, rect.left() + 1, rect.bottom());
            painter->drawPoint(rect.topLeft());
            painter->drawPoint(rect.topRight());
            painter->drawPoint(rect.bottomLeft());
            painter->drawPoint(rect.bottomRight());
            painter->drawLine(rect.left() + 1, rect.top() + 1, rect.right() - 1, rect.bottom() - 1);
            painter->drawLine(rect.left() + 1, rect.bottom() - 1, rect.right() - 1, rect.top() + 1);
            break;
        case SC_TitleBarNormalButton:
        {
            QRect r2 = rect.adjusted(0, 3, -3, 0);

            painter->drawRect(r2.adjusted(0, 0, -1, -1));
            painter->drawLine(r2.left() + 1, r2.top() + 1, r2.right() - 1, r2.top() + 1);
            painter->drawPoint(r2.topLeft());
            painter->drawPoint(r2.topRight());
            painter->drawPoint(r2.bottomLeft());
            painter->drawPoint(r2.bottomRight());

            QRect   backWindowRect(rect.adjusted(3, 0, 0, -3));
            QRegion clipRegion(backWindowRect);

            clipRegion -= r2;
            if(sunken)
                backWindowRect.adjust(1, 1, 1, 1);
            painter->drawRect(backWindowRect.adjusted(0, 0, -1, -1));
            painter->drawLine(backWindowRect.left() + 1, backWindowRect.top() + 1,
                            backWindowRect.right() - 1, backWindowRect.top() + 1);
            painter->drawPoint(backWindowRect.topLeft());
            painter->drawPoint(backWindowRect.topRight());
            painter->drawPoint(backWindowRect.bottomLeft());
            painter->drawPoint(backWindowRect.bottomRight());
            break;
        }
        case SC_TitleBarShadeButton:
            drawArrow(painter, rect, PE_IndicatorArrowUp, color);
            break;
        case SC_TitleBarUnshadeButton:
            drawArrow(painter, rect, PE_IndicatorArrowDown, color);
        default:
            break;
    }
}

void QtCurveStyle::drawEntryField(QPainter *p, const QRect &rx, const QStyleOption *option,
                                  int round, EWidget w) const
{
    QRect r(rx);
    bool  doEtch(QTC_CAN_DO_EFFECT);

    if(doEtch)
        r.adjust(0, 1, 0, -1);

    p->fillRect(QRect(rx.x()+1, rx.y()+1, rx.x()+rx.width()-2, rx.y()+rx.height()-2),
                option->state&State_Enabled ? option->palette.base().color() : option->palette.background().color());

    drawBorder(p, r, option, round, NULL, WIDGET_ENTRY, BORDER_SUNKEN);
    if(doEtch)
    {
        r=rx;
        p->setClipRegion(r);

        if(!(round&CORNER_TR) && !(round&CORNER_BR))
            r.adjust(0, 0, 2, 0);
        if(!(round&CORNER_TL) && !(round&CORNER_BL))
            r.adjust(-2, 0, 0, 0);
        drawEtch(p, r, /*option, */true, true, EFFECT_SHADOW==opts.buttonEffect && WIDGET_BUTTON(w) &&
                 !(option->state &(/*State_Down |*/ State_On | State_Sunken)));
        p->setClipping(false);
    }
}

void QtCurveStyle::drawMenuItem(QPainter *p, const QRect &r, const QStyleOption *option, bool mbi, int round, const QColor *cols) const
{
    if(opts.borderMenuitems)
    {
        bool stdColor(!mbi || SHADE_BLEND_SELECTED!=opts.shadeMenubars);

        QStyleOption opt(*option);

        opt.state|=State_Horizontal|State_Raised;
        opt.state&=~(State_Sunken|State_On);

        if(stdColor)
            drawLightBevel(p, r, &opt, round, cols[ORIGINAL_SHADE], cols, stdColor, WIDGET_MENU_ITEM);
        else
        {
            QRect fr(r);

            fr.adjust(1, 1, -1, -1);

            if(fr.width()>0 && fr.height()>0)
                drawBevelGradient(cols[ORIGINAL_SHADE], true, p, fr, true,
                                  getWidgetShade(WIDGET_MENU_ITEM, true, false, opts.menuitemAppearance),
                                  getWidgetShade(WIDGET_MENU_ITEM, false, false, opts.menuitemAppearance),
                                  false, opts.menuitemAppearance, WIDGET_MENU_ITEM);
            drawBorder(p, r, &opt, round, cols, WIDGET_MENU_ITEM, BORDER_FLAT, false, 0);
        }
    }
    else
        drawBevelGradient(cols[ORIGINAL_SHADE], true, p, r, true,
                          getWidgetShade(WIDGET_MENU_ITEM, true, false, opts.menuitemAppearance),
                          getWidgetShade(WIDGET_MENU_ITEM, false, false, opts.menuitemAppearance),
                          false, opts.menuitemAppearance, WIDGET_MENU_ITEM);
}

void QtCurveStyle::drawProgress(QPainter *p, const QRect &r, const QStyleOption *option, int round, bool vertical, bool reverse) const
{
    QStyleOption opt(*option);

    opt.state|=State_Raised;

    if(vertical)
        opt.state&=~State_Horizontal;
    else
        opt.state|=State_Horizontal;

    if(reverse)
        opt.state|=QTC_STATE_REVERSE;
    else
        opt.state&=~QTC_STATE_REVERSE;

    if(r.width()<1)
        return;

    int  length(vertical ? r.height() : r.width());
    bool drawFull(length > 3);

    if(drawFull)
        drawLightBevel(p, r, &opt, round, itsMenuitemCols[ORIGINAL_SHADE], itsMenuitemCols, true, WIDGET_PROGRESSBAR);
    else
    {
        p->setPen(itsMenuitemCols[QT_STD_BORDER]);
        if(length>1)
        {
            p->setBrush(itsMenuitemCols[ORIGINAL_SHADE]);
            drawRect(p, r);
        }
        else if(vertical)
            p->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
        else
            p->drawLine(r.x(), r.y(), r.x(), r.y()+r.height()-1);
    }

    if(QTC_ROUNDED && length>2 && ROUNDED_ALL!=round)
    {
        p->setPen(midColor(option->palette.background().color(), itsMenuitemCols[QT_STD_BORDER]));
        if(!(round&CORNER_TL) || !drawFull)
            p->drawPoint(r.x(), r.y());
        if(!(round&CORNER_BL) || !drawFull)
            p->drawPoint(r.x(), r.y()+r.height()-1);
        if(!(round&CORNER_TR) || !drawFull)
            p->drawPoint(r.x()+r.width()-1, r.y());
        if(!(round&CORNER_BR) || !drawFull)
            p->drawPoint(r.x()+r.width()-1, r.y()+r.height()-1);
    }
}

void QtCurveStyle::drawArrow(QPainter *p, const QRect &r, PrimitiveElement pe, const QColor &col, bool small) const
{
    QPolygon     a;
    QPainterPath path;

    if(small)
        switch(pe)
        {
            case PE_IndicatorArrowUp:
                a.setPoints(opts.vArrows ? 7 : 3,  2,0,  0,-2,  -2,0,   -2,1, -1,0, 1,0, 2,1);
                break;
            case PE_IndicatorArrowDown:
                a.setPoints(opts.vArrows ? 7 : 3,  2,0,  0,2,  -2,0,   -2,-1, -1,0, 1,0, 2,-1);
                break;
            case PE_IndicatorArrowRight:
                a.setPoints(opts.vArrows ? 7 : 3,  0,-2,  2,0,  0,2,   -1,2, 0,1, 0,-1, -1,-2);
                break;
            case PE_IndicatorArrowLeft:
                a.setPoints(opts.vArrows ? 7 : 3,  0,-2,  -2,0,  0,2,   1,2, 0,1, 0,-1, 1,-2);
                break;
            default:
                return;
        }
    else // Large arrows...
        switch(pe)
        {
            case PE_IndicatorArrowUp:
                a.setPoints(opts.vArrows ? 6 : 3,  3,1,  0,-2,  -3,1,    -2, 2,  0,0,  2,2);
                break;
            case PE_IndicatorArrowDown:
                a.setPoints(opts.vArrows ? 6 : 3,  3,-1,  0,2,  -3,-1,   -2,-2,  0,0, 2,-2);
                break;
            case PE_IndicatorArrowRight:
                a.setPoints(opts.vArrows ? 6 : 3,  -1,-3,  2,0,  -1,3,   -2,2, 0,0, -2,-2);
                break;
            case PE_IndicatorArrowLeft:
                a.setPoints(opts.vArrows ? 6 : 3,  1,-3,  -2,0,  1,3,    2,2, 0,0, 2,-2);
                break;
            default:
                return;
        }

    a.translate((r.x()+(r.width()>>1)), (r.y()+(r.height()>>1)));

    path.moveTo(a[0].x()+0.5, a[0].y()+0.5);
    for(int i=1; i<a.size(); ++i)
        path.lineTo(a[i].x()+0.5, a[i].y()+0.5);
    path.lineTo(a[0].x()+0.5, a[0].y()+0.5);

    // This all looks like overkill - but seems to fix issues with plasma and nvidia
    // Just using 'aa' and drawing the arrows would be fine - but this makes them look
    // slightly blurry, and I dont like that.
    p->setRenderHint(QPainter::Antialiasing, true);
    p->fillPath(path, col);
    p->setRenderHint(QPainter::Antialiasing, false);
    p->setRenderHint(QPainter::HighQualityAntialiasing, true);
    p->strokePath(path, col);
    p->setPen(col);
    p->drawPath(path);
    p->setRenderHint(QPainter::HighQualityAntialiasing, false);
}

void QtCurveStyle::drawArrow(QPainter *p, const QRect &r, const QStyleOption *option,
                             PrimitiveElement pe, bool small, bool checkActive) const
{
    drawArrow(p, r, pe, option->state&State_Enabled
                            ? checkActive && option->state&State_Selected
                                ? option->palette.highlightedText().color()
                                : option->palette.text().color()
                            : option->palette.mid().color(), small);
}

void QtCurveStyle::drawSbSliderHandle(QPainter *p, const QRect &rOrig, const QStyleOption *option, bool slider) const
{
    QStyleOption opt(*option);
    QRect        r(rOrig);

    if(opt.state&(State_Sunken|State_On))
        opt.state|=State_MouseOver;

    if(r.width()>r.height())
        opt.state|=State_Horizontal;

    opt.state&=~(State_Sunken|State_On);
    opt.state|=State_Raised;

    if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
        if(slider->minimum==slider->maximum)
            opt.state&=~(State_MouseOver|State_Enabled);

    int          min(MIN_SLIDER_SIZE(opts.sliderThumbs));
    const QColor *use(sliderColors(&opt));

    drawLightBevel(p, r, &opt, slider
#ifndef QTC_SIMPLE_SCROLLBARS
                   || SCROLLBAR_NONE==opts.scrollbarType
#endif
                    ? ROUNDED_ALL : ROUNDED_NONE,
                   getFill(&opt, use), use, true, WIDGET_SB_SLIDER);

    const QColor *markers(/*opts.coloredMouseOver && opt.state&State_MouseOver
                              ? SHADE_NONE==shade ? itsMouseOverCols : itsBackgroundCols
                              : */use);

    if(opt.state&State_Horizontal)
        r.setX(r.x()+1);
    else
        r.setY(r.y()+1);

    if(LINE_NONE!=opts.sliderThumbs && (slider || ((opt.state&State_Horizontal && r.width()>=min)|| r.height()>=min)))
        switch(opts.sliderThumbs)
        {
            case LINE_FLAT:
                drawLines(p, r, !(opt.state&State_Horizontal), 3, 5, markers, 0, 5, 0, false);
                break;
            case LINE_SUNKEN:
                drawLines(p, r, !(opt.state&State_Horizontal), 4, 3, markers, 0, 3);
                break;
            case LINE_DOTS:
            default:
                drawDots(p, r, !(opt.state&State_Horizontal), slider ? 3 : 5, slider ? 5 : 2, markers, 0, 5);
        }
}

void QtCurveStyle::drawSliderHandle(QPainter *p, const QRect &r, const QStyleOptionSlider *option) const
{
    bool horiz(SLIDER_TRIANGULAR==opts.sliderStyle ? r.height()>r.width() : r.width()>r.height());

    if(SLIDER_TRIANGULAR==opts.sliderStyle || (SLIDER_ROUND==opts.sliderStyle && ROUND_FULL==opts.round))
    {
        QStyleOption opt(*option);

        if(r.width()>r.height())
            opt.state|=State_Horizontal;
        opt.state&=~(State_Sunken|State_On);
        if(!(option->activeSubControls&SC_SliderHandle) || !(opt.state&State_Enabled))
            opt.state&=~State_MouseOver;

        opt.state|=State_Raised;

        const QColor     *use(sliderColors(&opt));
        const QColor     &fill(getFill(&opt, use));
        int              x(r.x()),
                         y(r.y()),
                         xo(horiz ? 8 : 0),
                         yo(horiz ? 0 : 8);
        PrimitiveElement direction(horiz ? PE_IndicatorArrowDown : PE_IndicatorArrowRight);
        QPolygon         clipRegion;
        bool             drawLight(MO_PLASTIK!=opts.coloredMouseOver || !(opt.state&State_MouseOver) ||
                                   (SLIDER_ROUND==opts.sliderStyle &&
                                   (SHADE_BLEND_SELECTED==opts.shadeSliders || SHADE_SELECTED==opts.shadeSliders)));
        int              size(SLIDER_TRIANGULAR==opts.sliderStyle ? 15 : 13);

        if(SLIDER_TRIANGULAR==opts.sliderStyle)
        {
            if(option->tickPosition & QSlider::TicksBelow)
                direction=horiz ? PE_IndicatorArrowDown : PE_IndicatorArrowRight;
            else if(option->tickPosition & QSlider::TicksAbove)
                direction=horiz ? PE_IndicatorArrowUp : PE_IndicatorArrowLeft;

            switch(direction)
            {
                default:
                case PE_IndicatorArrowDown:
                    clipRegion.setPoints(7,   x, y+2,    x+2, y,   x+8, y,    x+10, y+2,   x+10, y+9,   x+5, y+14,    x, y+9);
                    break;
                case PE_IndicatorArrowUp:
                    clipRegion.setPoints(7,   x, y+12,   x+2, y+14,   x+8, y+14,   x+10, y+12,   x+10, y+5,   x+5, y,    x, y+5);
                    break;
                case PE_IndicatorArrowLeft:
                    clipRegion.setPoints(7,   x+12, y,   x+14, y+2,   x+14, y+8,   x+12, y+10,   x+5, y+10,    x, y+5,    x+5, y );
                    break;
                case PE_IndicatorArrowRight:
                    clipRegion.setPoints(7,   x+2, y,    x, y+2,   x, y+8,    x+2, y+10,   x+9, y+10,   x+14, y+5,    x+9, y);
            }
        }
        else
            clipRegion.setPoints(8, x,       y+8+yo,  x,       y+4,     x+4,    y,        x+8+xo, y,
                                 x+12+xo, y+4,     x+12+xo, y+8+yo,  x+8+xo, y+12+yo,  x+4,    y+12+yo);

        p->save();
        p->setClipRegion(QRegion(clipRegion)); // , QPainter::CoordPainter);
        if(IS_FLAT(opts.sliderAppearance))
        {
            p->fillRect(r, fill);

            if(MO_PLASTIK==opts.coloredMouseOver && opt.state&State_MouseOver)
            {
                int col(QTC_SLIDER_MO_SHADE),
                    len(QTC_SLIDER_MO_LEN);

                if(horiz)
                {
                    p->fillRect(QRect(x+1, y+1, len, size-2), itsMouseOverCols[col]);
                    p->fillRect(QRect(x+r.width()-(1+len), y+1, len, r.height()-2), itsMouseOverCols[col]);
                }
                else
                {
                    p->fillRect(QRect(x+1, y+1, size-2, len), itsMouseOverCols[col]);
                    p->fillRect(QRect(x+1, y+r.height()-(1+len), r.width()-2, len), itsMouseOverCols[col]);
                }
            }
        }
        else
        {
            drawBevelGradient(fill, true, p, QRect(x, y, horiz ? r.width()-1 : size, horiz ? size : r.height()-1),
                              horiz, SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK,
                              false, opts.sliderAppearance);

            if(MO_PLASTIK==opts.coloredMouseOver && opt.state&State_MouseOver)
            {
                int col(QTC_SLIDER_MO_SHADE),
                    len(QTC_SLIDER_MO_LEN);

                if(horiz)
                {
                    drawBevelGradient(itsMouseOverCols[col], true, p, QRect(x+1, y+1, len, size-2),
                                      horiz, SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK, false, opts.sliderAppearance);
                    drawBevelGradient(itsMouseOverCols[col], true, p,  QRect(x+r.width()-(1+len), y+1, len, size-2),
                                      horiz,SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK, false, opts.sliderAppearance);
                }
                else
                {
                    drawBevelGradient(itsMouseOverCols[col], true, p, QRect(x+1, y+1, size-2, len),
                                      horiz, SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK, false, opts.sliderAppearance);
                    drawBevelGradient(itsMouseOverCols[col], true, p,QRect(x+1, y+r.height()-(1+len), size-2, len),
                                      horiz, SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK, false, opts.sliderAppearance);
                }
            }
        }

        p->setClipping(false);

        if(SLIDER_TRIANGULAR==opts.sliderStyle)
        {
            QPainterPath path;
            double       xd(r.x()+0.5),
                         yd(r.y()+0.5),
                         radius(2.5),
                         diameter(radius*2);

            switch(direction)
            {
                default:
                case PE_IndicatorArrowDown:
                    path.moveTo(xd+10-radius, yd);
                    path.arcTo(xd, yd, diameter, diameter, 90, 90);
                    path.lineTo(xd, yd+9);
                    path.lineTo(xd+5, yd+14);
                    path.lineTo(xd+10, yd+9);
                    path.arcTo(xd+10-diameter, yd, diameter, diameter, 0, 90);
                    p->setPen(use[QT_STD_BORDER]);
                    p->setRenderHint(QPainter::Antialiasing, true);
                    p->drawPath(path);
                    p->setRenderHint(QPainter::Antialiasing, false);
                    if(drawLight)
                    {
                        p->setPen(use[APPEARANCE_DULL_GLASS==opts.sliderAppearance ? 1 : 0]);
                        p->drawLine(r.x()+1, r.y()+2, r.x()+1, r.y()+8);
                        p->drawLine(r.x()+2, r.y()+1, r.x()+7, r.y()+1);
                    }
                    break;
                case PE_IndicatorArrowUp:
                    path.moveTo(xd, yd+5);
                    path.arcTo(xd, yd+14-diameter, diameter, diameter, 180, 90);
                    path.arcTo(xd+10-diameter, yd+14-diameter, diameter, diameter, 270, 90);
                    path.lineTo(xd+10, yd+5);
                    path.lineTo(xd+5, yd);
                    path.lineTo(xd, yd+5);
                    p->setPen(use[QT_STD_BORDER]);
                    p->setRenderHint(QPainter::Antialiasing, true);
                    p->drawPath(path);
                    p->setRenderHint(QPainter::Antialiasing, false);
                    if(drawLight)
                    {
                        p->setPen(use[APPEARANCE_DULL_GLASS==opts.sliderAppearance ? 1 : 0]);
                        p->drawLine(r.x()+5, r.y()+1, r.x()+1, r.y()+5);
                        p->drawLine(r.x()+1, r.y()+5, r.x()+1, r.y()+11);
                    }
                    break;
                case PE_IndicatorArrowLeft:
                    path.moveTo(xd+5, yd+10);
                    path.arcTo(xd+14-diameter, yd+10-diameter, diameter, diameter, 270, 90);
                    path.arcTo(xd+14-diameter, yd, diameter, diameter, 0, 90);
                    path.lineTo(xd+5, yd);
                    path.lineTo(xd, yd+5);
                    path.lineTo(xd+5, yd+10);
                    p->setPen(use[QT_STD_BORDER]);
                    p->setRenderHint(QPainter::Antialiasing, true);
                    p->drawPath(path);
                    p->setRenderHint(QPainter::Antialiasing, false);
                    if(drawLight)
                    {
                        p->setPen(use[APPEARANCE_DULL_GLASS==opts.sliderAppearance ? 1 : 0]);
                        p->drawLine(r.x()+1, r.y()+5, r.x()+5, r.y()+1);
                        p->drawLine(r.x()+5, r.y()+1, r.x()+11, r.y()+1);
                    }
                    break;
                case PE_IndicatorArrowRight:
                    path.moveTo(xd+9, yd);
                    path.arcTo(xd, yd, diameter, diameter, 90, 90);
                    path.arcTo(xd, yd+diameter, diameter, diameter, 180, 90);
                    path.lineTo(xd+9, yd+10);
                    path.lineTo(xd+14, yd+5);
                    path.lineTo(xd+9, yd);
                    p->setPen(use[QT_STD_BORDER]);
                    p->setRenderHint(QPainter::Antialiasing, true);
                    p->drawPath(path);
                    p->setRenderHint(QPainter::Antialiasing, false);
                    if(drawLight)
                    {
                        p->setPen(use[APPEARANCE_DULL_GLASS==opts.sliderAppearance ? 1 : 0]);
                        p->drawLine(r.x()+2, r.y()+1, r.x()+7, r.y()+1);
                        p->drawLine(r.x()+1, r.y()+2, r.x()+1, r.y()+8);
                    }
                    break;
            }
        }
        else
        {
            p->drawPixmap(x, y,
                          *getPixmap(use[opts.coloredMouseOver && opt.state&State_MouseOver ? 4 : QT_BORDER(opt.state&State_Enabled)],
                                     horiz ? PIX_SLIDER : PIX_SLIDER_V, 0.8));

            if(drawLight)
                p->drawPixmap(x, y, *getPixmap(use[0], horiz ? PIX_SLIDER_LIGHT : PIX_SLIDER_LIGHT_V));
        }
        p->restore();
    }
    else
//     {
//         QRect sr(r);
// 
//         if(horiz)
//             sr.adjust(0, 1, 0, 0);
//         else
//             sr.adjust(1, 0, 0, 0);

        drawSbSliderHandle(p, r, option, true);
/*    }*/
}

void QtCurveStyle::drawSliderGroove(QPainter *p, const QRect &groove, const QRect &handle,
                                    const QStyleOptionSlider *slider) const
{
    bool               horiz(Qt::Horizontal==slider->orientation);
    QRect              grv(groove);
    QStyleOptionSlider opt(*slider);

//     if (horiz)
//         grv.adjust(0, 0, -1, 0);
//     else
//         grv.adjust(0, 0, 0, -1);

    opt.state&=~(State_HasFocus|State_On);
    opt.state|=State_Sunken;

    if(horiz)
    {
        int dh=(grv.height()-5)>>1;
        grv.adjust(0, dh, 0, -dh);
        opt.state|=State_Horizontal;
    }
    else
    {
        int dw=(grv.width()-5)>>1;
        grv.adjust(dw, 0, -dw, 0);
        opt.state&=~State_Horizontal;
    }

    if(grv.height()>0 && grv.width()>0)
    {
        drawLightBevel(p, grv, &opt, ROUNDED_ALL, itsBackgroundCols[slider->state&State_Enabled ? 2 : ORIGINAL_SHADE],
                       itsBackgroundCols, true, WIDGET_SLIDER_TROUGH);

        if(opts. fillSlider && slider->maximum!=slider->minimum && slider->state&State_Enabled)
        {
            const QColor &usedCol=itsSliderCols
                                   ? itsSliderCols[ORIGINAL_SHADE]
                                   : itsMouseOverCols
                                       ? itsMouseOverCols[ORIGINAL_SHADE]
                                       : itsMenuitemCols[1];

            if (horiz)
                if (slider->upsideDown)
                    grv=QRect(handle.right()-2, grv.top(), (grv.right()-handle.right())+2, grv.height());
                else
                    grv=QRect(grv.left(), grv.top(), handle.left()+2, grv.height());
            else
                if (slider->upsideDown)
                    grv=QRect(grv.left(), handle.bottom()-2, grv.width(), (grv.height() - handle.bottom())+2);
                else
                    grv=QRect(grv.left(), grv.top(), grv.width(), (handle.top() - grv.top())+2);

            if(grv.height()>0 && grv.width()>0)
                drawLightBevel(p, grv, &opt, ROUNDED_ALL, usedCol, NULL, true, WIDGET_SLIDER_TROUGH);
        }
    }
}

void QtCurveStyle::drawMenuOrToolBarBackground(QPainter *p, const QRect &r, const QStyleOption *option, bool menu, bool horiz) const
{
    EAppearance app(menu ? opts.menubarAppearance : opts.toolbarAppearance);
    QColor      color(menu && itsActive ? itsMenubarCols[ORIGINAL_SHADE] : option->palette.background().color());
    double      from(0.0), to(0.0);

    switch(app)
    {
        default:
        case APPEARANCE_GRADIENT:
            from=SHADE_MENU_LIGHT;
            to=SHADE_MENU_DARK;
            break;
        case APPEARANCE_FLAT:
        case APPEARANCE_RAISED:
            break;
        case APPEARANCE_SHINY_GLASS:
        case APPEARANCE_DULL_GLASS:
            from=SHADE_BEVEL_GRAD_LIGHT;
            to=SHADE_BEVEL_GRAD_DARK;
    }

    drawBevelGradient(color, true, p, r, horiz, from, to, false, app);
}

void QtCurveStyle::drawHandleMarkers(QPainter *p, const QRect &r, const QStyleOption *option, bool tb,
                                     ELine handles) const
{
    if(r.width()<2 || r.height()<2)
        return;

    // CPD: Mouse over of toolbar handles not working - the whole toolbar seems to be active :-(
    QStyleOption opt(*option);

    opt.state&=~State_MouseOver;

    const QColor *border(borderColors(&opt, itsBackgroundCols));

    switch(handles)
    {
        case LINE_DOTS:
            drawDots(p, r, !(option->state&State_Horizontal), 2,
                     tb ? 5 : 3, border, tb ? -2 : 0, 5);
            break;
        case LINE_DASHES:
            if(option->state&State_Horizontal)
            {
                QRect r1(r.x()+(tb ? 2 : (r.width()-6)/2), r.y(), 3, r.height());

                drawLines(p, r1, true, (r.height()-8)/2,
                          tb ? 0 : (r.width()-5)/2, border, 0, 5, 0);
            }
            else
            {
                QRect r1(r.x(), r.y()+(tb ? 2 : (r.height()-6)/2), r.width(), 3);

                drawLines(p, r1, false, (r.width()-8)/2,
                          tb ? 0 : (r.height()-5)/2, border, 0, 5, 0);
            }
            break;
        case LINE_FLAT:
            drawLines(p, r, !(option->state&State_Horizontal), 2,
                      tb ? 4 : 2, border, tb ? -2 : 0, 4, 0, false);
            break;
        default:
            drawLines(p, r, !(option->state&State_Horizontal), 2,
                      tb ? 4 : 2, border, tb ? -2 : 0, 3);
    }
}

void QtCurveStyle::fillTab(QPainter *p, const QRect &r, const QStyleOption *option, const QColor &fill, bool horiz, bool increase, EWidget tab) const
{
    if(option->state&State_Selected && APPEARANCE_INVERTED==opts.appearance)
        p->fillRect(r, option->palette.background().color());
    else
    {
        bool        selected(option->state&State_Selected);
        EAppearance app(selected ? QTC_SEL_TAB_APP : QTC_NORM_TAB_APP);

        if(!IS_FLAT(app))
        {
            double s1=WIDGET_TAB_TOP==tab || (selected && opts.colorSelTab)
                          ? SHADE_TAB_SEL_LIGHT
                          : SHADE_BOTTOM_TAB_SEL_DARK,
                   s2=WIDGET_TAB_TOP==tab || (selected && opts.colorSelTab)
                          ? SHADE_TAB_SEL_DARK
                          : SHADE_BOTTOM_TAB_SEL_LIGHT;

            drawBevelGradient(fill, increase, p, r, horiz, s1, s2, option->state&State_Selected, app, tab);
        }
        else
            p->fillRect(r, fill);
    }
}

void QtCurveStyle::shadeColors(const QColor &base, QColor *vals) const
{
    QTC_SHADES

    for(int i=0; i<NUM_STD_SHADES; ++i)
        shade(base, &vals[i], QTC_SHADE(opts.contrast, i));
    shade(base, &vals[SHADE_ORIG_HIGHLIGHT], opts.highlightFactor);
    shade(vals[4], &vals[SHADE_4_HIGHLIGHT], opts.highlightFactor);
    shade(vals[2], &vals[SHADE_2_HIGHLIGHT], opts.highlightFactor);
    vals[ORIGINAL_SHADE]=base;
}

const QColor * QtCurveStyle::buttonColors(const QStyleOption *option) const
{
    if(option && option->palette.button()!=itsButtonCols[ORIGINAL_SHADE])
    {
        shadeColors(option->palette.button().color(), itsColoredButtonCols);
        return itsColoredButtonCols;
    }

    return itsButtonCols;
}

const QColor * QtCurveStyle::sliderColors(const QStyleOption *option) const
{
    return (option && option->state&State_Enabled)
                ? SHADE_NONE!=opts.shadeSliders// && option->palette.button()==itsButtonCols[ORIGINAL_SHADE]
                        ? itsSliderCols
                        : itsButtonCols //buttonColors(option)
                : itsBackgroundCols;
}

const QColor * QtCurveStyle::backgroundColors(const QColor &col) const
{
    if(col!=itsBackgroundCols[ORIGINAL_SHADE])
    {
        shadeColors(col, itsColoredBackgroundCols);
        return itsColoredBackgroundCols;
    }

    return itsBackgroundCols;
}

const QColor * QtCurveStyle::borderColors(const QStyleOption *option, const QColor *use) const
{
    return itsMouseOverCols && opts.coloredMouseOver && option && option->state&State_MouseOver
               ? itsMouseOverCols : use;
}

const QColor * QtCurveStyle::getSidebarButtons() const
{
    if(!itsSidebarButtonsCols)
    {
        if(SHADE_BLEND_SELECTED==opts.shadeSliders)
            itsSidebarButtonsCols=itsSliderCols;
        else if(IND_COLORED==opts.defBtnIndicator)
            itsSidebarButtonsCols=itsDefBtnCols;
        else
        {
            itsSidebarButtonsCols=new QColor [TOTAL_SHADES+1];
            shadeColors(midColor(itsMenuitemCols[ORIGINAL_SHADE], itsButtonCols[ORIGINAL_SHADE]),
                        itsSidebarButtonsCols);
        }
    }

    return itsSidebarButtonsCols;
}

void QtCurveStyle::setMenuColors(const QColor &bgnd)
{
    switch(opts.shadeMenubars)
    {
        case SHADE_NONE:
            memcpy(itsMenubarCols, itsBackgroundCols, sizeof(QColor)*(TOTAL_SHADES+1));
            break;
        case SHADE_BLEND_SELECTED:  // For menubars we dont actually blend...
            shadeColors(IS_GLASS(opts.appearance)
                            ? shade(itsMenuitemCols[ORIGINAL_SHADE], MENUBAR_GLASS_SELECTED_DARK_FACTOR)
                            : itsMenuitemCols[ORIGINAL_SHADE],
                        itsMenubarCols);
            break;
        case SHADE_CUSTOM:
            shadeColors(opts.customMenubarsColor, itsMenubarCols);
            break;
        case SHADE_DARKEN:
            shadeColors(shade(bgnd, MENUBAR_DARK_FACTOR), itsMenubarCols);
    }
}

const QColor * QtCurveStyle::getMdiColors(const QStyleOption *option, bool active) const
{
    if(!itsActiveMdiColors)
    {
        itsActiveMdiTextColor=option->palette.highlightedText().color();
        itsMdiTextColor=option->palette.text().color();

        // Try to read kwin's settings...
        if(useQt3Settings())
        {
            QFile f(QDir::homePath()+QLatin1String("/.qt/qtrc"));

            if(f.open(QIODevice::ReadOnly))
            {
                QTextStream in(&f);
                bool        inPal(false);

                while (!in.atEnd())
                {
                    QString line(in.readLine());

                    if(inPal)
                    {
                        if(!itsActiveMdiColors && 0==line.indexOf("activeBackground=#", Qt::CaseInsensitive))
                        {
                            QColor col;

                            setRgb(&col, line.mid(17).toLatin1().constData());

                            if(col!=itsMenuitemCols[ORIGINAL_SHADE])
                            {
                                itsActiveMdiColors=new QColor [TOTAL_SHADES+1]; 
                                shadeColors(col, itsActiveMdiColors);
                            }
                        }
                        else if(!itsMdiColors && 0==line.indexOf("inactiveBackground=#", Qt::CaseInsensitive))
                        {
                            QColor col;

                            setRgb(&col, line.mid(19).toLatin1().constData());
                            if(col!=itsButtonCols[ORIGINAL_SHADE])
                            {
                                itsMdiColors=new QColor [TOTAL_SHADES+1];
                                shadeColors(col, itsMdiColors);
                            }
                        }
                        else if(0==line.indexOf("activeForeground=#", Qt::CaseInsensitive))
                            setRgb(&itsActiveMdiTextColor, line.mid(17).toLatin1().constData());
                        else if(0==line.indexOf("inactiveForeground=#", Qt::CaseInsensitive))
                            setRgb(&itsMdiTextColor, line.mid(19).toLatin1().constData());
                        else if (-1!=line.indexOf('['))
                            break;
                    }
                    else if(0==line.indexOf("[KWinPalette]", Qt::CaseInsensitive))
                        inPal=true;
                }
                f.close();
            }
        }
        else // KDE4
        {
            QFile f(kdeHome()+QLatin1String("/share/config/kdeglobals"));

            if(f.open(QIODevice::ReadOnly))
            {
                QTextStream in(&f);
                bool        inPal(false);

                while (!in.atEnd())
                {
                    QString line(in.readLine());

                    if(inPal)
                    {
                        if(!itsActiveMdiColors && 0==line.indexOf("activeBackground=", Qt::CaseInsensitive))
                        {
                            QColor col;

                            setRgb(&col, line.mid(17).split(QLatin1String(",")));

                            if(col!=itsMenuitemCols[ORIGINAL_SHADE])
                            {
                                itsActiveMdiColors=new QColor [TOTAL_SHADES+1];
                                shadeColors(col, itsActiveMdiColors);
                            }
                        }
                        else if(!itsMdiColors && 0==line.indexOf("inactiveBackground=", Qt::CaseInsensitive))
                        {
                            QColor col;

                            setRgb(&col, line.mid(19).split(QLatin1String(",")));
                            if(col!=itsButtonCols[ORIGINAL_SHADE])
                            {
                                itsMdiColors=new QColor [TOTAL_SHADES+1];
                                shadeColors(col, itsMdiColors);
                            }
                        }
                        else if(0==line.indexOf("activeForeground=", Qt::CaseInsensitive))
                            setRgb(&itsActiveMdiTextColor, line.mid(17).split(QLatin1String(",")));
                        else if(0==line.indexOf("inactiveForeground=", Qt::CaseInsensitive))
                            setRgb(&itsMdiTextColor, line.mid(19).split(QLatin1String(",")));
                        else if (-1!=line.indexOf('['))
                            break;
                    }
                    else if(0==line.indexOf("[WM]", Qt::CaseInsensitive))
                        inPal=true;
                }
                f.close();
            }
        }

        if(!itsActiveMdiColors)
            itsActiveMdiColors=(QColor *)itsMenuitemCols;
        if(!itsMdiColors)
            itsMdiColors=(QColor *)itsBackgroundCols;
    }

    return active ? itsActiveMdiColors : itsMdiColors;
}

void QtCurveStyle::readMdiPositions() const
{
    if(0==itsMdiButtons[0].size() && 0==itsMdiButtons[1].size())
    {
        // Set defaults...
        itsMdiButtons[0].append(SC_TitleBarSysMenu);
        itsMdiButtons[0].append(SC_TitleBarShadeButton);

        itsMdiButtons[1].append(SC_TitleBarContextHelpButton);
        itsMdiButtons[1].append(SC_TitleBarMinButton);
        itsMdiButtons[1].append(SC_TitleBarMaxButton);
        itsMdiButtons[1].append(WINDOWTITLE_SPACER);
        itsMdiButtons[1].append(SC_TitleBarCloseButton);

        // Read in KWin settings...
        QFile f(kdeHome()+QLatin1String("/share/config/kwinrc"));

        if(f.open(QIODevice::ReadOnly))
        {
            QTextStream in(&f);
            bool        inStyle(false);

            while (!in.atEnd())
            {
                QString line(in.readLine());

                if(inStyle)
                {
                    if(0==line.indexOf("ButtonsOnLeft=", Qt::CaseInsensitive))
                    {
                        itsMdiButtons[0].clear();
                        parseWindowLine(line.mid(14), itsMdiButtons[0]);
                    }
                    else if(0==line.indexOf("ButtonsOnRight=", Qt::CaseInsensitive))
                    {
                        itsMdiButtons[1].clear();
                        parseWindowLine(line.mid(15), itsMdiButtons[1]);
                    }
                    else if (-1!=line.indexOf('['))
                        break;
                }
                else if(0==line.indexOf("[Style]", Qt::CaseInsensitive))
                    inStyle=true;
            }
            f.close();
        }

        // Designer uses shade buttons, not min/max - so if we dont have shade in our kwin config. then add this
        // button near the max button...
        if(-1==itsMdiButtons[0].indexOf(SC_TitleBarShadeButton) && -1==itsMdiButtons[1].indexOf(SC_TitleBarShadeButton))
        {
            int maxPos=itsMdiButtons[0].indexOf(SC_TitleBarMaxButton);

            if(-1==maxPos) // Left doesnt have max button, assume right does and add shade there
            {
                int minPos=itsMdiButtons[1].indexOf(SC_TitleBarMinButton);
                maxPos=itsMdiButtons[1].indexOf(SC_TitleBarMaxButton);

                itsMdiButtons[1].insert(minPos<maxPos ? (minPos==-1 ? 0 : minPos)
                                                        : (maxPos==-1 ? 0 : maxPos), SC_TitleBarShadeButton);
            }
            else // Add to left button
            {
                int minPos=itsMdiButtons[0].indexOf(SC_TitleBarMinButton);

                itsMdiButtons[1].insert(minPos>maxPos ? (minPos==-1 ? 0 : minPos)
                                                      : (maxPos==-1 ? 0 : maxPos), SC_TitleBarShadeButton);
            }
        }
    }
}

const QColor & QtCurveStyle::getFill(const QStyleOption *option, const QColor *use) const
{
    return !(option->state&State_Enabled)
               ? use[ORIGINAL_SHADE]
               : option->state&State_Sunken  // State_Down ????
                   ? use[4]
                   : option->state&State_MouseOver
                         ? option->state&State_On
                               ? use[SHADE_4_HIGHLIGHT]
                               : use[SHADE_ORIG_HIGHLIGHT]
                         : option->state&State_On
                               ? use[4]
                               : use[ORIGINAL_SHADE];
}

QPixmap * QtCurveStyle::getPixmap(const QColor col, EPixmap p, double shade) const
{
    QRgb    rgb(col.rgb());
    QtcKey  key(createKey(rgb, p));
    QPixmap *pix=itsPixmapCache.object(key);

    if(!pix)
    {
        pix=new QPixmap();

        QImage img;

        switch(p)
        {
            case PIX_RADIO_BORDER:
                img.loadFromData(radio_frame_png_data, radio_frame_png_len);
                break;
            case PIX_RADIO_LIGHT:
                img.loadFromData(radio_light_png_data, radio_light_png_len);
                break;
            case PIX_RADIO_ON:
                img.loadFromData(radio_on_png_data, radio_on_png_len);
                break;
            case PIX_CHECK:
                img.loadFromData(opts.xCheck ? check_x_on_png_data : check_on_png_data, opts.xCheck ? check_x_on_png_len : check_on_png_len);
                break;
            case PIX_SLIDER:
                img.loadFromData(slider_png_data, slider_png_len);
                break;
            case PIX_SLIDER_LIGHT:
                img.loadFromData(slider_light_png_data, slider_light_png_len);
                break;
            case PIX_SLIDER_V:
                img.loadFromData(slider_v_png_data, slider_v_png_len);
                break;
            case PIX_SLIDER_LIGHT_V:
                img.loadFromData(slider_light_v_png_data, slider_light_v_png_len);
                break;
        }

        if (img.depth()<32)
            img=img.convertToFormat(QImage::Format_ARGB32);

        adjustPix(img.bits(), 4, img.width(), img.height(), img.bytesPerLine(), col.red(),
                  col.green(), col.blue(), shade);
        *pix=QPixmap::fromImage(img);
        itsPixmapCache.insert(key, pix, pix->depth()/8);
    }

    return pix;
}

const QColor & QtCurveStyle::getTabFill(bool current, bool highlight, const QColor *use) const
{
    return current
            ? use[ORIGINAL_SHADE]
            : highlight
                ? use[SHADE_2_HIGHLIGHT]
                : use[2];
}

void QtCurveStyle::widgetDestroyed(QObject *o)
{
    theNoEtchWidgets.remove(static_cast<const QWidget *>(o));
}

#include "qtcurve.moc"
