/*
  QtCurve (C) Craig Drummond, 2007 - 2009 craig_p_drummond@yahoo.co.uk

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

static bool usePixmapCache=true;

#include <QtGui>
#include <QtDBus/QtDBus>
#include <QX11Info>
#define QTC_COMMON_FUNCTIONS
#include "qtcurve.h"
#include "pixmaps.h"
#define CONFIG_READ
#include "config_file.c"

// WebKit seems to just use the values from ::pixelMetric to get button sizes. So, in pixelMetric we add some extra padding to PM_ButtonMargin
// if we're max rounding - this gives a nicer border. However, dont want this on real buttons - so in sizeFromContents we remove this padding
// in CT_PushButton and CT_ComboBox
#define QTC_MAX_ROUND_BTN_PAD (ROUND_MAX==opts.round ? 3 : 0)

#ifdef QTC_XBAR_SUPPORT
#include "macmenu.h"
#endif

// TODO! REMOVE THIS WHEN KDE'S ICON SETTINGS ACTUALLY WORK!!!
#define QTC_FIX_DISABLED_ICONS

#define QTC_MO_ARROW_X(MO, COL) (state&State_Enabled \
                                    ? (MO_GLOW==opts.coloredMouseOver && MO \
                                        ? itsMouseOverCols[QT_STD_BORDER] \
                                        : palette.color(COL)) \
                                    : palette.color(QPalette::Disabled, COL))
#define QTC_MO_ARROW(COL)       QTC_MO_ARROW_X(state&State_MouseOver, COL)

static bool useQt3Settings()
{
    static const char *full = getenv("KDE_FULL_SESSION");
    static const char *vers = full ? getenv("KDE_SESSION_VERSION") : 0;
    static bool       use   = full && (!vers || atoi(vers)<4);

    return use;
}

#ifndef QTC_QT_ONLY
#include <KDE/KApplication>
#include <KDE/KAboutData>
#include <KDE/KGlobalSettings>
#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KIconLoader>
#include <KDE/KIcon>
#include <KDE/KColorScheme>
#include <KDE/KStandardDirs>
#include <KDE/KComponentData>
#include <KDE/KTitleWidget>
#include <KDE/KTabBar>

#if !defined QTC_DISABLE_KDEFILEDIALOG_CALLS && !KDE_IS_VERSION(4, 1, 0)
static int theInstanceCount=0;

// KDE4.1 does this functionality for us!
#include <KDE/KFileDialog>
#include <KDE/KDirSelectDialog>
#include <KDE/KGlobal>

typedef QString (*_qt_filedialog_existing_directory_hook)(QWidget *parent, const QString &caption, const QString &dir, QFileDialog::Options options);
extern _qt_filedialog_existing_directory_hook qt_filedialog_existing_directory_hook;

typedef QString (*_qt_filedialog_open_filename_hook)(QWidget * parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options);
extern _qt_filedialog_open_filename_hook qt_filedialog_open_filename_hook;

typedef QStringList (*_qt_filedialog_open_filenames_hook)(QWidget * parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options);
extern _qt_filedialog_open_filenames_hook qt_filedialog_open_filenames_hook;

typedef QString (*_qt_filedialog_save_filename_hook)(QWidget * parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options);
extern _qt_filedialog_save_filename_hook qt_filedialog_save_filename_hook;

static QString qt2KdeFilter(const QString &f)
{
    QString               filter;
    QTextStream           str(&filter, QIODevice::WriteOnly);
    QStringList           list(f.split(";;"));
    QStringList::Iterator it(list.begin()),
                          end(list.end());
    bool                  first=true;

    for(; it!=end; ++it)
    {
        int ob=(*it).lastIndexOf('('),
            cb=(*it).lastIndexOf(')');

        if(-1!=cb && ob<cb)
        {
            if(first)
                first=false;
            else
                str << '\n';
            str << (*it).mid(ob+1, (cb-ob)-1) << '|' << (*it).mid(0, ob);
        }
    }

    return filter;
}

static void kde2QtFilter(const QString &orig, const QString &kde, QString *sel)
{
    if(sel)
    {
        QStringList           list(orig.split(";;"));
        QStringList::Iterator it(list.begin()),
                              end(list.end());
        int                   pos;

        for(; it!=end; ++it)
            if(-1!=(pos=(*it).indexOf(kde)) && pos>0 &&
               ('('==(*it)[pos-1] || ' '==(*it)[pos-1]) &&
               (*it).length()>=kde.length()+pos &&
               (')'==(*it)[pos+kde.length()] || ' '==(*it)[pos+kde.length()]))
            {
                *sel=*it;
                return;
            }
    }
}

static QString getExistingDirectory(QWidget *parent, const QString &caption, const QString &dir, QFileDialog::Options)
{
    KUrl url(KDirSelectDialog::selectDirectory(KUrl(dir), true, parent, caption));

    if(url.isLocalFile())
        return url.pathOrUrl();
    else
        return QString();
}

static QString getOpenFileName(QWidget *parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter,
                               QFileDialog::Options)
{
    KFileDialog dlg(KUrl(dir), qt2KdeFilter(filter), parent);

    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setMode(KFile::File|KFile::LocalOnly);
    dlg.setCaption(caption);
    dlg.exec();

    QString rv(dlg.selectedFile());

    if(!rv.isEmpty())
        kde2QtFilter(filter, dlg.currentFilter(), selectedFilter);

    return rv;
}

static QStringList getOpenFileNames(QWidget *parent, const QString &caption, const QString &dir, const QString &filter,
                                    QString *selectedFilter, QFileDialog::Options)
{
    KFileDialog dlg(KUrl(dir), qt2KdeFilter(filter), parent);

    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setMode(KFile::Files|KFile::LocalOnly);
    dlg.setCaption(caption);
    dlg.exec();

    QStringList rv(dlg.selectedFiles());

    if(rv.count())
        kde2QtFilter(filter, dlg.currentFilter(), selectedFilter);

    return rv;
}

static QString getSaveFileName(QWidget *parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter,
                               QFileDialog::Options)
{
    KFileDialog dlg(KUrl(dir), qt2KdeFilter(filter), parent);

    dlg.setOperationMode(KFileDialog::Saving);
    dlg.setMode(KFile::File|KFile::LocalOnly);
    dlg.setCaption(caption);
    dlg.exec();

    QString rv(dlg.selectedFile());

    if(!rv.isEmpty())
        kde2QtFilter(filter, dlg.currentFilter(), selectedFilter);

    return rv;
}

static void setFileDialogs()
{
    if(1==++theInstanceCount)
    {
        if(!qt_filedialog_existing_directory_hook)
            qt_filedialog_existing_directory_hook=&getExistingDirectory;
        if(!qt_filedialog_open_filename_hook)
            qt_filedialog_open_filename_hook=&getOpenFileName;
        if(!qt_filedialog_open_filenames_hook)
            qt_filedialog_open_filenames_hook=&getOpenFileNames;
        if(!qt_filedialog_save_filename_hook)
            qt_filedialog_save_filename_hook=&getSaveFileName;
    }
}

static void unsetFileDialogs()
{
    if(0==--theInstanceCount)
    {
        if(qt_filedialog_existing_directory_hook==&getExistingDirectory)
            qt_filedialog_existing_directory_hook=0;
        if(qt_filedialog_open_filename_hook==&getOpenFileName)
            qt_filedialog_open_filename_hook=0;
        if(qt_filedialog_open_filenames_hook==&getOpenFileNames)
            qt_filedialog_open_filenames_hook=0;
        if(qt_filedialog_save_filename_hook==&getSaveFileName)
            qt_filedialog_save_filename_hook=0;
    }
}

#endif
#endif

#if defined QTC_FIX_DISABLED_ICONS && !defined QTC_QT_ONLY
#include <KDE/KIconEffect>
QPixmap getIconPixmap(const QIcon &icon, const QSize &size, QIcon::Mode mode, QIcon::State)
{
    QPixmap pix=icon.pixmap(size, QIcon::Normal);

    if(QIcon::Disabled==mode)
    {
        QImage img=pix.toImage();
        KIconEffect::toGray(img, 1.0);
        KIconEffect::semiTransparent(img);
        pix=QPixmap::fromImage(img);
    }

    return pix;
}

#else
inline QPixmap getIconPixmap(const QIcon &icon, const QSize &size, QIcon::Mode mode, QIcon::State state=QIcon::Off)
{
    return icon.pixmap(size, mode, state);
}
#endif
inline QPixmap getIconPixmap(const QIcon &icon, int size, QIcon::Mode mode, QIcon::State state=QIcon::Off)
{
    return getIconPixmap(icon, QSize(size, size), mode, state);
}

inline QPixmap getIconPixmap(const QIcon &icon, int size, int flags, QIcon::State state=QIcon::Off)
{
    return getIconPixmap(icon, QSize(size, size), flags&QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled, state);
}

inline QPixmap getIconPixmap(const QIcon &icon, const QSize &size, int flags, QIcon::State state=QIcon::Off)
{
    return getIconPixmap(icon, size, flags&QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled, state);
}

static void drawTbArrow(const QStyle *style, const QStyleOptionToolButton *toolbutton,
                        const QRect &rect, QPainter *painter, const QWidget *widget = 0)
{
    QStyle::PrimitiveElement pe;
    switch (toolbutton->arrowType)
    {
        case Qt::LeftArrow:
            pe = QStyle::PE_IndicatorArrowLeft;
            break;
        case Qt::RightArrow:
            pe = QStyle::PE_IndicatorArrowRight;
            break;
        case Qt::UpArrow:
            pe = QStyle::PE_IndicatorArrowUp;
            break;
        case Qt::DownArrow:
            pe = QStyle::PE_IndicatorArrowDown;
            break;
        default:
            return;
    }

    QStyleOption arrowOpt;
    arrowOpt.rect = rect;
    arrowOpt.palette = toolbutton->palette;
    arrowOpt.state = toolbutton->state;
    style->drawPrimitive(pe, &arrowOpt, painter, widget);
}

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

#define WINDOWTITLE_SPACER    0x10000000
#define QTC_STATE_REVERSE     (QStyle::StateFlag)0x10000000
#define QTC_STATE_MENU        (QStyle::StateFlag)0x20000000
#define QTC_STATE_KWIN_BUTTON (QStyle::StateFlag)0x40000000
#define QTC_STATE_TBAR_BUTTON (QStyle::StateFlag)0x80000000
// #define QTC_NO_BGND_BUTTON    (QStyle::StateFlag)0x80000000

#define M_PI 3.14159265358979323846

static const int constMenuPixmapWidth=22;

static enum
{
    APP_SKIP_TASKBAR,
    APP_KPRINTER,
    APP_KDIALOG,
    APP_KDIALOGD,
    APP_PLASMA,
    APP_KRUNNER,
    APP_KWIN,
    APP_SYSTEMSETTINGS,
    APP_SKYPE,
    APP_KONQUEROR,
    APP_KONTACT,
    APP_ARORA,
    APP_KMIX,
#ifdef QTC_XBAR_SUPPORT
    APP_QTDESIGNER,
#endif
    APP_OTHER
} theThemedApp=APP_OTHER;

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

static QWidget * getActiveWindow(QWidget *widget)
{
    QWidget *activeWindow=QApplication::activeWindow();

//    if(!activeWindow)
//    {
//        QWidgetList::ConstIterator it(QApplication::topLevelWidgets().begin()),
//                                    end(QApplication::topLevelWidgets().end());
//
//        for(; it!=end; ++it)
//            if((*it)->isVisible() && *it!=widget->window())
//                if(!activeWindow)
//                    activeWindow=*it;
//                else
//                {
//                    activeWindow=0L;
//                    break;
//                }
//    }

    return activeWindow && activeWindow!=widget ? activeWindow : 0L;
}

// static bool inStackWidget(const QWidget *w)
// {
//     while(w)
//     {
//         if(::qobject_cast<const QStackedWidget *>(w))
//             return true;
//         w=w->parentWidget();
//     }
//     
//     return false;
// }

static const QWidget * getToolBar(const QWidget *w/*, bool checkQ3*/)
{
    return w
            ? qobject_cast<const QToolBar *>(w) // || (checkQ3 && w->inherits("Q3ToolBar"))
                ? w
                : getToolBar(w->parentWidget()/*, checkQ3*/)
            : 0L;
}

//
// OK, Etching looks cr*p on plasma widgets, and khtml...
// CPD:TODO WebKit?
static QSet<const QWidget *> theNoEtchWidgets;

static bool isA(const QObject *w, const char *type)
{
    return w && (0==strcmp(w->metaObject()->className(), type) ||
                (w->parent() && 0==strcmp(w->parent()->metaObject()->className(), type)));
}

static bool isInQAbstractItemView(const QObject *w)
{
    int level=8;

    while(w && --level>0)
    {
        if(qobject_cast<const QAbstractItemView *>(w))
            return true;
        if(qobject_cast<const QDialog *>(w)/* || qobject_cast<const QMainWindow *>(w)*/)
            return false;
        w=w->parent();
    }

    return false;
}

static bool isKontactPreviewPane(const QWidget *widget)
{
    return APP_KONTACT==theThemedApp && widget && widget->parentWidget() && widget->parentWidget()->parentWidget() &&
           widget->inherits("KHBox") && ::qobject_cast<const QSplitter *>(widget->parentWidget()) &&
           widget->parentWidget()->parentWidget()->inherits("KMReaderWin");
}

static bool isKateView(const QWidget *widget)
{
    return widget && widget->parentWidget() &&
           ::qobject_cast<const QFrame *>(widget) && widget->parentWidget()->inherits("KateView");
}

static bool isNoEtchWidget(const QWidget *widget)
{
    if(APP_KRUNNER==theThemedApp)
        return true;

    if(APP_PLASMA==theThemedApp)
    {
        const QWidget *top=widget->window();

        return !top || (!qobject_cast<const QDialog *>(top) && !qobject_cast<const QMainWindow *>(top));
    }

    if(widget && widget->inherits("QWebView"))
        return true;

    // KHTML:  widget -> QWidget       -> QWidget    -> KHTMLView
    const QObject *w=widget && widget->parent() && widget->parent()->parent()
                        ? widget->parent()->parent()->parent() : 0L;

    return (w && isA(w, "KHTMLView")) || (widget && isInQAbstractItemView(widget->parentWidget()));
}

static QWidget * scrollViewFrame(QWidget *widget)
{
    QWidget *w=widget;

    for(int i=0; i<10 && w; ++i, w=w->parentWidget())
    {
//     printf("Look at %s (%s) [%d / %d]\n", w->metaObject()->className(),
//            w->metaObject()->superClass() ? w->metaObject()->superClass()->className() : "<>",
//            qobject_cast<QFrame *>(w) ? ((QFrame *)w)->frameWidth() : -1,
//            qobject_cast<QTabWidget *>(w) ? 1 : 0);
        if( (qobject_cast<QFrame *>(w) && ((QFrame *)w)->frameWidth()>0) ||
            qobject_cast<QTabWidget *>(w))
            return w;
    }
    return 0L;
}

// from windows style
static const int windowsItemFrame    =  2; // menu item frame width
static const int windowsItemHMargin  =  3; // menu item hor text margin
static const int windowsItemVMargin  =  2; // menu item ver text margin
static const int windowsRightBorder  = 15; // right border on windows
static const int windowsArrowHMargin =  6; // arrow horizontal margin
static const int constProgressBarFps = 20;
static const int constTabPad         = 6;

#define QTC_SB_SUB2 ((QStyle::SubControl)(QStyle::SC_ScrollBarGroove << 1))

static QString kdeHome()
{
// #if defined QTC_QT_ONLY
    static QString kdeHomePath;
    if (kdeHomePath.isEmpty())
    {
        kdeHomePath = QString::fromLocal8Bit(qgetenv("KDEHOME"));
        if (kdeHomePath.isEmpty())
        {
            QDir    homeDir(QDir::homePath());
            QString kdeConfDir(QLatin1String("/.kde"));
            if (!useQt3Settings() && homeDir.exists(QLatin1String(".kde4")))
                kdeConfDir = QLatin1String("/.kde4");
            kdeHomePath = QDir::homePath() + kdeConfDir;
        }
    }
    return kdeHomePath;
// #else
//     return KGlobal::dirs()->localkdedir();
// #endif
}

static void getStyles(const QString &dir, const char *sub, QSet<QString> &styles)
{
    QDir d(dir+sub);

    if(d.exists())
    {
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
                styles.insert(style);
        }
    }
}

static void getStyles(const QString &dir, QSet<QString> &styles)
{
    getStyles(dir, QTC_THEME_DIR, styles);
    getStyles(dir, QTC_THEME_DIR4, styles);
}

static QString themeFile(const QString &dir, const QString &n, const char *sub)
{
    QString name(dir+sub+n+QTC_THEME_SUFFIX);

    return QFile(name).exists() ? name : QString();
}

static QString themeFile(const QString &dir, const QString &n, bool kde3=false)
{
    QString name(themeFile(dir, n, kde3 ? QTC_THEME_DIR : QTC_THEME_DIR4));

    if(name.isEmpty())
        name=themeFile(dir, n, kde3 ? QTC_THEME_DIR4 : QTC_THEME_DIR);
    return name;
}

class QtCurveStylePlugin : public QStylePlugin
{
    public:

    QtCurveStylePlugin(QObject *parent=0) : QStylePlugin( parent ) {}
    ~QtCurveStylePlugin() {}

    QStringList keys() const
    {
        QSet<QString> styles;
        styles.insert("QtCurve");

        getStyles(kdeHome(), styles);
        getStyles(KDE_PREFIX(useQt3Settings() ? 3 : 4), styles);
        getStyles(KDE_PREFIX(useQt3Settings() ? 4 : 3), styles);

        return styles.toList();
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

static inline void drawAaPoint(QPainter *p, int x, int y)
{
    p->drawPoint(QPointF(x+0.5, y+0.5));
}

static inline void drawAaRect(QPainter *p, const QRect &r)
{
    p->drawRect(QRectF(r.x()+0.5, r.y()+0.5, r.width()-1, r.height()-1));
}

static void drawDots(QPainter *p, const QRect &r, bool horiz, int nLines, int offset,
                     const QColor *cols, int startOffset, int dark)
{
    int space((nLines*2)+(nLines-1)),
        x(horiz ? r.x() : r.x()+((r.width()-space)>>1)),
        y(horiz ? r.y()+((r.height()-space)>>1) : r.y()),
        i, j,
        numDots((horiz ? (r.width()-(2*offset))/3 : (r.height()-(2*offset))/3)+1);

    p->setRenderHint(QPainter::Antialiasing, true);
    if(horiz)
    {
        if(startOffset && y+startOffset>0)
            y+=startOffset;

        p->setPen(cols[dark]);
        for(i=0; i<space; i+=3)
            for(j=0; j<numDots; j++)
                drawAaPoint(p, x+offset+(3*j), y+i);

        p->setPen(cols[0]);
        for(i=1; i<space; i+=3)
            for(j=0; j<numDots; j++)
                drawAaPoint(p, x+offset+1+(3*j), y+i);
    }
    else
    {
        if(startOffset && x+startOffset>0)
            x+=startOffset;

        p->setPen(cols[dark]);
        for(i=0; i<space; i+=3)
            for(j=0; j<numDots; j++)
                drawAaPoint(p, x+i, y+offset+(3*j));

        p->setPen(cols[0]);
        for(i=1; i<space; i+=3)
            for(j=0; j<numDots; j++)
                drawAaPoint(p, x+i, y+offset+1+(3*j));
    }
    p->setRenderHint(QPainter::Antialiasing, false);
}

static bool isHoriz(const QStyleOption *option, EWidget w)
{
    return WIDGET_BUTTON(w) || option->state&QStyle::State_Horizontal;
}

#define QTC_PIXMAP_DIMENSION 10

/*
Cache key:
    widgettype 2
    app        5
    size      15
    horiz      1
    alpha      8
    blue       8
    green      8
    red        8
    type       1  (0 for widget, 1 for pixmap)
    ------------
              56
*/
enum ECacheType
{
    CACHE_STD,
    CACHE_PBAR,
    CACHE_TAB_TOP,
    CACHE_TAB_BOT
};

static QtcKey createKey(qulonglong size, const QColor &color, bool horiz, int app, EWidget w)
{
    ECacheType type=WIDGET_TAB_TOP==w
                        ? CACHE_TAB_TOP
                        : WIDGET_TAB_BOT==w
                            ? CACHE_TAB_BOT
                            : WIDGET_PROGRESSBAR==w
                                ? CACHE_PBAR
                                : CACHE_STD;

    return (color.rgba()<<1)+
           (((qulonglong)(horiz ? 1 : 0))<<33)  +
           (((qulonglong)(size&0xFFFF))<<34)+
           (((qulonglong)(app&0x1F))<<50)+
           (((qulonglong)(type&0x03))<<55);
}

static QtcKey createKey(const QColor &color, EPixmap p)
{
    return 1+
           ((color.rgb()&RGB_MASK)<<1)+
           (((qulonglong)(p&0x1F))<<33)+
           (((qulonglong)1)<<38);
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

static const QPushButton * getButton(const QWidget *w, const QPainter *p)
{
    const QWidget *widget=w ? w : (p && p->device() ? dynamic_cast<const QWidget *>(p->device()) : 0L);
    return widget ? ::qobject_cast<const QPushButton *>(widget) : 0L;
}

inline bool isMultiTabBarTab(const QPushButton *button)
{
    return button && button->isFlat() && button->inherits("KMultiTabBarTab");
}

QtCurveStyle::QtCurveStyle(const QString &name)
            : itsSliderCols(0L),
              itsDefBtnCols(0L),
              itsComboBtnCols(0L),
              itsSortedLvColors(0L),
              itsSidebarButtonsCols(0L),
              itsActiveMdiColors(0L),
              itsMdiColors(0L),
              itsPixmapCache(150000),
              itsActive(true),
              itsSbWidget(0L),
              itsProgressBarAnimateTimer(0),
              itsAnimateStep(0),
              itsPos(-1, -1),
              itsHoverWidget(0L),
              itsQtVersion(VER_UNKNOWN)
{
#if !defined QTC_QT_ONLY
    if(KGlobal::hasMainComponent())
        itsComponentData=KGlobal::mainComponent();
    else
    {
        //printf("Creating KComponentData\n");

        QString name(QApplication::applicationName());

        if(name.isEmpty())
            name=qAppName();

        if(name.isEmpty())
            name="QtApp";

        itsComponentData=KComponentData(name.toLatin1(), name.toLatin1(), KComponentData::SkipMainComponentRegistration);
    }
#if !defined QTC_DISABLE_KDEFILEDIALOG_CALLS && !KDE_IS_VERSION(4, 1, 0)
    setFileDialogs();
#endif
    QDBusConnection::sessionBus().connect(QString(), "/KGlobalSettings", "org.kde.KGlobalSettings",
                                          "notifyChange", this, SLOT(kdeGlobalSettingsChange(int, int)));
#endif

    QString rcFile;

    if(!name.isEmpty())
    {
        rcFile=themeFile(kdeHome(), name);

        if(rcFile.isEmpty())
        {
            rcFile=themeFile(KDE_PREFIX(useQt3Settings() ? 3 : 4), name, useQt3Settings());
            if(rcFile.isEmpty())
                rcFile=themeFile(KDE_PREFIX(useQt3Settings() ? 4 : 3), name, !useQt3Settings());
        }
    }

    readConfig(rcFile, &opts);
    opts.contrast=QSettings(QLatin1String("Trolltech")).value("/Qt/KDE/contrast", 7).toInt();
    if(opts.contrast<0 || opts.contrast>10)
        opts.contrast=7;

    shadeColors(QApplication::palette().color(QPalette::Active, QPalette::Highlight), itsHighlightCols);
    shadeColors(QApplication::palette().color(QPalette::Active, QPalette::Background), itsBackgroundCols);
    shadeColors(QApplication::palette().color(QPalette::Active, QPalette::Button), itsButtonCols);

    // Set defaults for Hover and Focus, these will be changed when KDE4 palette is applied...
    shadeColors(QApplication::palette().color(QPalette::Active, QPalette::Highlight), itsFocusCols);
    shadeColors(QApplication::palette().color(QPalette::Active, QPalette::Highlight), itsMouseOverCols);

    switch(opts.shadeSliders)
    {
        default:
        case SHADE_DARKEN:
        case SHADE_NONE:
            break;
        case SHADE_SELECTED:
            itsSliderCols=itsHighlightCols;
            break;
        case SHADE_BLEND_SELECTED:
        case SHADE_CUSTOM:
            if(!itsSliderCols)
                itsSliderCols=new QColor [TOTAL_SHADES+1];
            shadeColors(SHADE_BLEND_SELECTED==opts.shadeSliders
                            ? midColor(itsHighlightCols[ORIGINAL_SHADE],
                                       itsButtonCols[ORIGINAL_SHADE])
                            : opts.customSlidersColor,
                        itsSliderCols);
    }

    switch(opts.defBtnIndicator)
    {
        case IND_GLOW:
            itsDefBtnCols=itsFocusCols;
            break;
        case IND_TINT:
            itsDefBtnCols=new QColor [TOTAL_SHADES+1];
            shadeColors(tint(itsButtonCols[ORIGINAL_SHADE],
                            itsHighlightCols[ORIGINAL_SHADE], QTC_DEF_BNT_TINT), itsDefBtnCols);
            break;
        default:
            break;
        case IND_COLORED:
            if(SHADE_BLEND_SELECTED==opts.shadeSliders)
                itsDefBtnCols=itsSliderCols;
            else
            {
                itsDefBtnCols=new QColor [TOTAL_SHADES+1];
                shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE],
                                    itsButtonCols[ORIGINAL_SHADE]), itsDefBtnCols);
            }
    }

    switch(opts.comboBtn)
    {
        default:
        case SHADE_DARKEN:
        case SHADE_NONE:
            break;
        case SHADE_SELECTED:
            itsComboBtnCols=itsHighlightCols;
            break;
        case SHADE_BLEND_SELECTED:
            if(opts.shadeSliders==SHADE_BLEND_SELECTED)
            {
                itsComboBtnCols=itsSliderCols;
                break;
            }
        case SHADE_CUSTOM:
            if(opts.shadeSliders==SHADE_CUSTOM && opts.customSlidersColor==opts.customComboBtnColor)
            {
                itsComboBtnCols=itsSliderCols;
                break;
            }
            if(!itsComboBtnCols)
                itsComboBtnCols=new QColor [TOTAL_SHADES+1];
            shadeColors(SHADE_BLEND_SELECTED==opts.comboBtn
                            ? midColor(itsHighlightCols[ORIGINAL_SHADE],
                                       itsButtonCols[ORIGINAL_SHADE])
                            : opts.customComboBtnColor,
                        itsComboBtnCols);
    }

    switch(opts.sortedLv)
    {
        case SHADE_DARKEN:
            if(!itsSortedLvColors)
                itsSortedLvColors=new QColor [TOTAL_SHADES+1];
            shadeColors(shade(opts.lvButton ? itsButtonCols[ORIGINAL_SHADE] : itsBackgroundCols[ORIGINAL_SHADE], LV_HEADER_DARK_FACTOR), itsSortedLvColors);
            break;
        default:
        case SHADE_NONE:
            break;
        case SHADE_SELECTED:
            itsSortedLvColors=itsHighlightCols;
            break;
        case SHADE_BLEND_SELECTED:
            if(opts.shadeSliders==SHADE_BLEND_SELECTED)
            {
                itsSortedLvColors=itsSliderCols;
                break;
            }
            else if(SHADE_BLEND_SELECTED==opts.comboBtn)
            {
                itsSortedLvColors=itsComboBtnCols;
                break;
            }
        case SHADE_CUSTOM:
            if(opts.shadeSliders==SHADE_CUSTOM && opts.customSlidersColor==opts.customSortedLvColor)
            {
                itsSortedLvColors=itsSliderCols;
                break;
            }
            if(opts.comboBtn==SHADE_CUSTOM && opts.customComboBtnColor==opts.customSortedLvColor)
            {
                itsSortedLvColors=itsComboBtnCols;
                break;
            }
            if(!itsSortedLvColors)
                itsSortedLvColors=new QColor [TOTAL_SHADES+1];
            shadeColors(SHADE_BLEND_SELECTED==opts.comboBtn
                            ? midColor(itsHighlightCols[ORIGINAL_SHADE],
                                       (opts.lvButton ? itsButtonCols[ORIGINAL_SHADE] : itsBackgroundCols[ORIGINAL_SHADE]))
                            : opts.customSortedLvColor,
                        itsSortedLvColors);
    }

    setMenuColors(QApplication::palette().color(QPalette::Active, QPalette::Background));

    if(USE_LIGHTER_POPUP_MENU)
        itsLighterPopupMenuBgndCol=shade(itsBackgroundCols[ORIGINAL_SHADE],
                                         QTC_TO_FACTOR(opts.lighterPopupMenuBgnd));

    switch(opts.shadeCheckRadio)
    {
        default:
            itsCheckRadioCol=QApplication::palette().color(QPalette::Active, opts.crButton
                                                                                ? QPalette::ButtonText : QPalette::Text);
            break;
        case SHADE_BLEND_SELECTED:
        case SHADE_SELECTED:
            itsCheckRadioCol=QApplication::palette().color(QPalette::Active, QPalette::Highlight);
            break;
        case SHADE_CUSTOM:
            itsCheckRadioCol=opts.customCheckRadioColor;
    }

    if(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR && NUM_TITLEBAR_BUTTONS==opts.titlebarButtonColors.size())
        for(int i=0; i<NUM_TITLEBAR_BUTTONS; ++i)
        {
            QColor *cols=new QColor [TOTAL_SHADES+1];
            shadeColors(opts.titlebarButtonColors[(ETitleBarButtons)i], cols);
            itsTitleBarButtonsCols[i]=cols;
        }
    else
        opts.titlebarButtons&=~QTC_TITLEBAR_BUTTON_COLOR;

#if !defined QTC_QT_ONLY
    QMetaObject::invokeMethod(this, "setupKde4", Qt::QueuedConnection);
#endif
}

QtCurveStyle::~QtCurveStyle()
{
    killTimer(itsProgressBarAnimateTimer);
#if !defined QTC_QT_ONLY && !defined QTC_DISABLE_KDEFILEDIALOG_CALLS
#if !KDE_IS_VERSION(4, 1, 0)
    unsetFileDialogs();
#endif
#endif

    if(itsSidebarButtonsCols &&
       itsSidebarButtonsCols!=itsSliderCols &&
       itsSidebarButtonsCols!=itsDefBtnCols)
        delete [] itsSidebarButtonsCols;
    if(itsActiveMdiColors && itsActiveMdiColors!=itsHighlightCols)
        delete [] itsActiveMdiColors;
    if(itsMdiColors && itsMdiColors!=itsBackgroundCols)
        delete [] itsMdiColors;
    if(itsDefBtnCols && itsDefBtnCols!=itsSliderCols && itsDefBtnCols!=itsFocusCols)
        delete [] itsDefBtnCols;
    if(itsSliderCols && itsSliderCols!=itsHighlightCols)
        delete [] itsSliderCols;
    if(itsComboBtnCols && itsComboBtnCols!=itsHighlightCols && itsComboBtnCols!=itsSliderCols)
        delete [] itsComboBtnCols;
    if(itsSortedLvColors && itsSortedLvColors!=itsHighlightCols && itsSortedLvColors!=itsSliderCols &&
       itsSortedLvColors!=itsComboBtnCols)
        delete [] itsSortedLvColors;
    if(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR)
        for(int i=0; i<NUM_TITLEBAR_BUTTONS; ++i)
            delete [] itsTitleBarButtonsCols[i];
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
        if("kwin"==appName)
            theThemedApp=APP_KWIN;
        else if("systemsettings"==appName)
            theThemedApp=APP_SYSTEMSETTINGS;
        else if("plasma"==appName || appName.startsWith("plasma-"))
            theThemedApp=APP_PLASMA;
        else if("krunner"==appName || "krunner_lock"==appName || "kscreenlocker"==appName)
            theThemedApp=APP_KRUNNER;
        else if("konqueror"==appName)
            theThemedApp=APP_KONQUEROR;
        else if("kontact"==appName)
            theThemedApp=APP_KONTACT;
        else if("skype"==appName)
            theThemedApp=APP_SKYPE;
        else if("arora"==appName)
            theThemedApp=APP_ARORA;
        else if("kmix"==appName)
            theThemedApp=APP_KMIX;
#ifdef QTC_XBAR_SUPPORT
        else if("Designer"==QCoreApplication::applicationName())
            theThemedApp=APP_QTDESIGNER;
#endif

    // Plasma does not like the 'Fix parentless dialogs' option...
    if(APP_PLASMA==theThemedApp && opts.fixParentlessDialogs)
        opts.fixParentlessDialogs=false;
}

void QtCurveStyle::polish(QPalette &palette)
{
    int  contrast(QSettings(QLatin1String("Trolltech")).value("/Qt/KDE/contrast", 7).toInt());
    bool newContrast(false);

    if(contrast<0 || contrast>10)
        contrast=7;

    if(contrast!=opts.contrast)
    {
        opts.contrast=contrast;
        newContrast=true;
    }

    bool newMenu(newContrast ||
                 itsHighlightCols[ORIGINAL_SHADE]!=palette.color(QPalette::Active, QPalette::Highlight)),
         newGray(newContrast ||
                 itsBackgroundCols[ORIGINAL_SHADE]!=palette.color(QPalette::Active, QPalette::Background)),
         newButton(newContrast ||
                   itsButtonCols[ORIGINAL_SHADE]!=palette.color(QPalette::Active, QPalette::Button)),
         newSlider(itsSliderCols && itsHighlightCols!=itsSliderCols && SHADE_BLEND_SELECTED==opts.shadeSliders &&
                   (newContrast || newButton || newMenu)),
         newDefBtn(itsDefBtnCols && /*( (IND_COLORED==opts.defBtnIndicator &&*/
                                       SHADE_BLEND_SELECTED!=opts.shadeSliders/*) ||*/ // If so, def btn == slider!
                                      /*(IND_TINT==opts.defBtnIndicator) )*/ &&
                   (newContrast || newButton || newMenu)),
         newMouseOver(newContrast || newButton || newMenu),
         newComboBtn(itsComboBtnCols && itsHighlightCols!=itsComboBtnCols && itsSliderCols!=itsComboBtnCols &&
                     SHADE_BLEND_SELECTED==opts.comboBtn &&
                     (newContrast || newButton || newMenu)),
         newSortedLv(itsSortedLvColors && ( (SHADE_BLEND_SELECTED==opts.sortedLv && itsHighlightCols!=itsSortedLvColors && itsSliderCols!=itsSortedLvColors &&
                                             itsComboBtnCols!=itsSortedLvColors) || 
                                             SHADE_DARKEN==opts.sortedLv) &&
                     (newContrast || (opts.lvButton ? newButton : newGray)));

    if(newGray)
        shadeColors(palette.color(QPalette::Active, QPalette::Background), itsBackgroundCols);

    if(newButton)
        shadeColors(palette.color(QPalette::Active, QPalette::Button), itsButtonCols);

    if(newMenu)
        shadeColors(palette.color(QPalette::Active, QPalette::Highlight), itsHighlightCols);

// Dont set these here, they will be updated in setDecorationColors()...
//     shadeColors(QApplication::palette().color(QPalette::Active, QPalette::Highlight), itsFocusCols);
//     if(opts.coloredMouseOver)
//         shadeColors(QApplication::palette().color(QPalette::Active, QPalette::Highlight), itsMouseOverCols);

    setMenuColors(palette.color(QPalette::Active, QPalette::Background));

    if(newSlider)
        shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE],
                    itsButtonCols[ORIGINAL_SHADE]), itsSliderCols);

    if(newComboBtn)
        shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE],
                    itsButtonCols[ORIGINAL_SHADE]), itsComboBtnCols);

    if(newSortedLv)
        if(SHADE_BLEND_SELECTED==opts.sortedLv)
            shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE],
                        opts.lvButton ? itsButtonCols[ORIGINAL_SHADE] : itsBackgroundCols[ORIGINAL_SHADE]), itsSortedLvColors);
        else
            shadeColors(shade(opts.lvButton ? itsButtonCols[ORIGINAL_SHADE] : itsBackgroundCols[ORIGINAL_SHADE], LV_HEADER_DARK_FACTOR), itsSortedLvColors);

    if(newDefBtn)
        if(IND_TINT==opts.defBtnIndicator)
            shadeColors(tint(itsButtonCols[ORIGINAL_SHADE],
                        itsHighlightCols[ORIGINAL_SHADE], QTC_DEF_BNT_TINT), itsDefBtnCols);
        else if(IND_GLOW!=opts.defBtnIndicator)
            shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE],
                        itsButtonCols[ORIGINAL_SHADE]), itsDefBtnCols);

    if(itsSidebarButtonsCols && SHADE_BLEND_SELECTED!=opts.shadeSliders &&
       IND_COLORED!=opts.defBtnIndicator)
        shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE],
                   itsButtonCols[ORIGINAL_SHADE]), itsSidebarButtonsCols);

    if(USE_LIGHTER_POPUP_MENU && newGray)
        itsLighterPopupMenuBgndCol=shade(itsBackgroundCols[ORIGINAL_SHADE],
                                         QTC_TO_FACTOR(opts.lighterPopupMenuBgnd));

    switch(opts.shadeCheckRadio)
    {
        default:
            itsCheckRadioCol=palette.color(QPalette::Active, opts.crButton
                                                                 ? QPalette::ButtonText : QPalette::Text);
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

    palette.setColor(QPalette::Disabled, QPalette::Base, palette.color(QPalette::Active, QPalette::Background));
    palette.setColor(QPalette::Disabled, QPalette::Background, palette.color(QPalette::Active, QPalette::Background));

    // Fix KDE4's palette...
    for(int i=QPalette::WindowText; i<QPalette::NColorRoles; ++i)
        if(i!=QPalette::Highlight && i!=QPalette::HighlightedText)
            palette.setColor(QPalette::Inactive, (QPalette::ColorRole)i, palette.color(QPalette::Active, (QPalette::ColorRole)i));

//     KConfigGroup group(KGlobal::config(), "ColorEffects:Inactive");
//     if(group.readEntry("ChangeSelectionColor", group.readEntry("Enable", false)))
//     {
//         palette.setColor(QPalette::Inactive, QPalette::Highlight,
//                           midColor(palette.color(QPalette::Active, QPalette::Window),
//                                    palette.color(QPalette::Active, QPalette::Highlight), INACTIVE_HIGHLIGHT_FACTOR));
//         palette.setColor(QPalette::Inactive, QPalette::HighlightedText, palette.color(QPalette::Active, QPalette::WindowText));
//     }

    // Force this to be re-generated!
    if(SHADE_BLEND_SELECTED==opts.menuStripe)
        opts.customMenuStripeColor=Qt::black;
}

void QtCurveStyle::polish(QWidget *widget)
{
    bool enableMouseOver(opts.highlightFactor || opts.coloredMouseOver);

#ifndef QTC_XBAR_SUPPORT
    // 'Fix' konqueror's large menubar...
    if(APP_KONQUEROR==theThemedApp && widget->parentWidget() && qobject_cast<QToolButton*>(widget) && qobject_cast<QMenuBar*>(widget->parentWidget()))
        widget->parentWidget()->setMaximumSize(32768, konqMenuBarSize((QMenuBar *)widget->parentWidget()));
#endif

    if(EFFECT_NONE!=opts.buttonEffect && isNoEtchWidget(widget))
    {
        theNoEtchWidgets.insert(static_cast<const QWidget *>(widget));
        connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(widgetDestroyed(QObject *)));
    }

    if(!IS_FLAT(opts.bgndAppearance))
    {
        switch (widget->windowFlags() & Qt::WindowType_Mask)
        {
            case Qt::Window:
            case Qt::Dialog:
                widget->installEventFilter(this);
                widget->setAttribute(Qt::WA_StyledBackground);
                break;
            case Qt::Popup: // we currently don't want that kind of gradient on menus etc
            case Qt::Tool: // this we exclude as it is used for dragging of icons etc
            default:
                break;
        }
        if(qobject_cast<QSlider *>(widget))
            widget->setBackgroundRole(QPalette::NoRole);
    }

    // Enable hover effects in all itemviews
    if (QAbstractItemView *itemView = qobject_cast<QAbstractItemView*>(widget))
    {
        itemView->viewport()->setAttribute(Qt::WA_Hover);
        if(opts.forceAlternateLvCols && !widget->inherits("KFilePlacesView"))
            itemView->setAlternatingRowColors(true);
    }

    if(APP_KONTACT==theThemedApp && qobject_cast<QToolButton *>(widget))
        ((QToolButton *)widget)->setAutoRaise(true);

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
        qobject_cast<QHeaderView *>(widget) ||
        qobject_cast<QTabBar *>(widget) ||
        qobject_cast<QAbstractScrollArea *>(widget) ||
        qobject_cast<QTextEdit *>(widget) ||
        qobject_cast<QLineEdit *>(widget) ||
//        qobject_cast<QDockWidget *>(widget) ||
        widget->inherits("QWorkspaceTitleBar") ||
        widget->inherits("QDockSeparator") ||
        widget->inherits("QDockWidgetSeparator") ||
        widget->inherits("Q3DockWindowResizeHandle")))
        widget->setAttribute(Qt::WA_Hover, true);

    if (qobject_cast<QSplitterHandle *>(widget))
        widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
    if (qobject_cast<QScrollBar *>(widget))
    {
        if(enableMouseOver)
            widget->setAttribute(Qt::WA_Hover, true);
        widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
        if(!opts.gtkScrollViews)
            widget->installEventFilter(this);
    }
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
    else if(opts.highlightScrollViews && widget->inherits("Q3ScrollView"))
        widget->installEventFilter(this);
    else if(qobject_cast<QMenuBar *>(widget))
    {
#ifdef QTC_XBAR_SUPPORT
        if (!((APP_QTDESIGNER==theThemedApp) && widget->inherits("QDesignerMenuBar")))
            Bespin::MacMenu::manage((QMenuBar *)widget);
#endif
        if(!IS_FLAT(opts.bgndAppearance))
            widget->setBackgroundRole(QPalette::NoRole);

        widget->setAttribute(Qt::WA_Hover, true);

//         if(opts.shadeMenubarOnlyWhenActive && SHADE_NONE!=opts.shadeMenubars)
            widget->installEventFilter(this);

        if(opts.customMenuTextColor || SHADE_BLEND_SELECTED==opts.shadeMenubars ||
           SHADE_SELECTED==opts.shadeMenubars ||
           (SHADE_CUSTOM==opts.shadeMenubars && TOO_DARK(itsMenubarCols[ORIGINAL_SHADE])))
        {
            QPalette pal(widget->palette());

            pal.setBrush(QPalette::Active, QPalette::Foreground, opts.customMenuTextColor
                                                   ? opts.customMenuNormTextColor
                                                   : pal.highlightedText().color());

            if(!opts.shadeMenubarOnlyWhenActive)
                pal.setBrush(QPalette::Inactive, QPalette::Foreground, opts.customMenuTextColor
                                                   ? opts.customMenuNormTextColor
                                                   : pal.highlightedText().color());

            widget->setPalette(pal);
        }
    }
    else if(qobject_cast<QLabel*>(widget))
        widget->installEventFilter(this);
    else if(/*!opts.gtkScrollViews && */qobject_cast<QAbstractScrollArea *>(widget))
    {
        if(!opts.gtkScrollViews && (((QFrame *)widget)->frameWidth()>0))
            widget->installEventFilter(this);
        if(APP_KONTACT==theThemedApp && widget->parentWidget())
        {
//         printf("Look for scrollview frame for %s\n", widget->metaObject()->className());
            QWidget *frame=scrollViewFrame(widget->parentWidget());

            if(frame)
            {
                frame->installEventFilter(this);
                itsSViewContainers[frame].insert(widget);
                connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(widgetDestroyed(QObject *)));
                connect(frame, SIGNAL(destroyed(QObject *)), this, SLOT(widgetDestroyed(QObject *)));
            }
//             else
//             printf("Not found :-(\n");
        }
    }
    else if(qobject_cast<QDialog*>(widget) && widget->inherits("QPrintPropertiesDialog") &&
            widget->parentWidget() && widget->parentWidget()->topLevelWidget() &&
            widget->topLevelWidget() && widget->topLevelWidget()->windowTitle().isEmpty() &&
            !widget->parentWidget()->topLevelWidget()->windowTitle().isEmpty())
    {
        widget->topLevelWidget()->setWindowTitle(widget->parentWidget()->topLevelWidget()->windowTitle());
    }
    else if(widget->inherits("QWhatsThat"))
    {
        QPalette pal(widget->palette());
        QColor   shadow(pal.shadow().color());

        shadow.setAlpha(32);
        pal.setColor(QPalette::Shadow, shadow);
        widget->setPalette(pal);
        widget->setMask(QRegion(widget->rect().adjusted(0, 0, -6, -6))+
                        QRegion(widget->rect().adjusted(6, 6, 0, 0)));
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
             //widget->installEventFilter(this);
        }
        else if(qobject_cast<QDialog *>(widget) && widget->windowFlags()&Qt::WindowType_Mask &&
                (!widget->parentWidget()) /*|| widget->parentWidget()->isHidden())*/)
        {
            QWidget *activeWindow=getActiveWindow(widget);

            if(activeWindow)
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
            //else if (QFrame::HLine==frame->frameShape() || QFrame::VLine==frame->frameShape())
                 widget->installEventFilter(this);

#ifdef QTC_QT_ONLY
            if(widget->parent() && widget->parent()->inherits("KTitleWidget"))
#else
            if(widget->parent() && qobject_cast<KTitleWidget *>(widget->parent()))
#endif
            {
                if(IS_FLAT(opts.bgndAppearance))
                    frame->setBackgroundRole(QPalette::Window);
                else
                    frame->setAutoFillBackground(false);

                QLayout *layout(frame->layout());

                if(layout)
                    layout->setMargin(0);
            }

            QWidget *p=0L;

            if(opts.gtkComboMenus && widget->parentWidget() && (p=widget->parentWidget()->parentWidget()) &&
               qobject_cast<QComboBox *>(p) && !((QComboBox *)(p))->isEditable())
            {
                QPalette pal(widget->palette());

                pal.setBrush(QPalette::Active, QPalette::Base, USE_LIGHTER_POPUP_MENU ? itsLighterPopupMenuBgndCol : itsBackgroundCols[ORIGINAL_SHADE]);
                widget->setPalette(pal);
            }
        }

    if(!IS_FLAT(opts.menuBgndAppearance))
        widget->installEventFilter(this);
    else if(USE_LIGHTER_POPUP_MENU && qobject_cast<QMenu *>(widget))
    {
        QPalette pal(widget->palette());

        pal.setBrush(QPalette::Active, QPalette::Window, itsLighterPopupMenuBgndCol);
        widget->setPalette(pal);
    }

    //bool onToolBar(widget && widget->parent() && 0L!=getToolBar(widget->parentWidget(), true));
    bool parentIsToolbar(widget && widget->parent() && (qobject_cast<QToolBar *>(widget->parent()) || widget->parent()->inherits("Q3ToolBar")));

    if (qobject_cast<QMenuBar *>(widget) ||
        widget->inherits("Q3ToolBar") ||
        qobject_cast<QToolBar *>(widget) ||
        parentIsToolbar)
        widget->setBackgroundRole(QPalette::Window);

    if(!IS_FLAT(opts.toolbarAppearance) && parentIsToolbar)
        widget->setAutoFillBackground(false);

    if(APP_SYSTEMSETTINGS==theThemedApp &&
       widget && widget->parentWidget() && widget->parentWidget()->parentWidget() &&
       qobject_cast<QFrame *>(widget) && QFrame::NoFrame!=((QFrame *)widget)->frameShape() &&
       qobject_cast<QFrame *>(widget->parentWidget()) &&
       qobject_cast<QTabWidget *>(widget->parentWidget()->parentWidget()))
        ((QFrame *)widget)->setFrameShape(QFrame::NoFrame);

    if (QLayout *layout = widget->layout())
    {
        // explicitely check public layout classes, QMainWindowLayout doesn't work here
        if (qobject_cast<QBoxLayout *>(layout)
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
             || qobject_cast<QFormLayout *>(layout)
#endif
             || qobject_cast<QGridLayout *>(layout)
             || qobject_cast<QStackedLayout *>(layout))
                polishLayout(layout);
    }
}

#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
static QFontMetrics styledFontMetrics(const QStyleOption *option, const QWidget *widget)
{
    if (option) {
        return option->fontMetrics;
    } else if (widget) {
        return widget->fontMetrics();
    }
    return qApp->fontMetrics();
}

static int fontHeight(const QStyleOption *option, const QWidget *widget)
{
    return styledFontMetrics(option, widget).height();
}


// Taken from skulpture 0.2.3
void QtCurveStyle::polishFormLayout(QFormLayout *layout)
{
    int widgetSize=-1;

    if (layout->labelAlignment() & Qt::AlignVCenter)
        return;

    int addedHeight = -1;
    for (int row = 0; row < layout->rowCount(); ++row)
    {
        QLayoutItem *labelItem = layout->itemAt(row, QFormLayout::LabelRole);
        if (!labelItem)
            continue;

        QLayoutItem *fieldItem = layout->itemAt(row, QFormLayout::FieldRole);
        if (!fieldItem)
            continue;

        QWidget *label = labelItem->widget();
        if (!label)
            continue;

        int labelHeight;
        if (addedHeight < 0)
        {
#if 0
            // fixed value in Qt
            static const int verticalMargin = 1;
            QStyleOptionFrame option;
            option.initFrom(label);
            option.lineWidth = label->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &option, label);
            option.midLineWidth = 0;
            option.rect = QRect(0, 0, 10, fontHeight(option, label) + 2 * verticalMargin);
            // label should be aligned centered to LineEdit, so use its size
            addedHeight = label->style()->sizeFromContents(QStyle::CT_LineEdit, &option, option.rect.size(), label).height() - fontHeight(option, height);
#else
            addedHeight = 4 + 2 * widgetSize;
#endif
        }
        if (qobject_cast<QLabel *>(label))
            labelHeight = label->sizeHint().height() + addedHeight;
        else if (qobject_cast<QCheckBox *>(label))
            labelHeight = label->sizeHint().height();
        else
            continue;

        int fieldHeight = fieldItem->sizeHint().height();
        // work around KIntNumInput::sizeHint() bug
        if (fieldItem->widget() && fieldItem->widget()->inherits("KIntNumInput"))
        {
            fieldHeight -= 2;
            fieldItem->widget()->setMaximumHeight(fieldHeight);
        }
        /* for large fields, we don't center */
        if (fieldHeight <= 2 * fontHeight(0, label) + addedHeight)
        {
            if (fieldHeight > labelHeight)
                labelHeight = fieldHeight;
        }
//         else if (verticalTextShift(label->fontMetrics()) & 1)
//                 labelHeight += 1;
        if (qobject_cast<QCheckBox *>(label))
            label->setMinimumHeight(labelHeight);
        else
        {
            // QFormLayout determines label size as height * 5 / 4, so revert that
            label->setMinimumHeight((labelHeight * 4 + 4) / 5);
        }
    }
}

void QtCurveStyle::polishLayout(QLayout *layout)
{
//    if (forceSpacingAndMargins) {
// #if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
//         if (QFormLayout *formLayout = qobject_cast<QFormLayout *>(layout)) {
//             if (formLayout->spacing() >= 2) {
//                 formLayout->setSpacing(-1);
//             }
//         } else
// #endif
//         if (QGridLayout *gridLayout = qobject_cast<QGridLayout *>(layout)) {
//             if (gridLayout->spacing() >= 2) {
//                 gridLayout->setSpacing(-1);
//             }
//         } else if (QBoxLayout *boxLayout = qobject_cast<QBoxLayout *>(layout)) {
//             if (boxLayout->spacing() >= 2) {
//                 boxLayout->setSpacing(-1);
//             }
//         } else {
//             if (layout->spacing() >= 2) {
//                 layout->setSpacing(-1);
//             }
//         }
//         if (layout->margin() >= 4) {
//             layout->setMargin(-1);
//         }
//     }
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
    if (QFormLayout *formLayout = qobject_cast<QFormLayout *>(layout))
        polishFormLayout(formLayout);
#endif
    // recurse into layouts
    for (int i = 0; i < layout->count(); ++i)
        if (QLayout *l = layout->itemAt(i)->layout())
            polishLayout(l);

}
#endif

void QtCurveStyle::unpolish(QWidget *widget)
{
    if(EFFECT_NONE!=opts.buttonEffect && theNoEtchWidgets.contains(widget))
    {
        theNoEtchWidgets.remove(static_cast<const QWidget *>(widget));
        disconnect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(widgetDestroyed(QObject *)));
    }

    if(!IS_FLAT(opts.bgndAppearance))
    {
        switch (widget->windowFlags() & Qt::WindowType_Mask)
        {
            case Qt::Window:
            case Qt::Dialog:
                widget->removeEventFilter(this);
                widget->setAttribute(Qt::WA_StyledBackground, false);
                break;
            case Qt::Popup: // we currently don't want that kind of gradient on menus etc
            case Qt::Tool: // this we exclude as it is used for dragging of icons etc
            default:
                break;
        }

        if(qobject_cast<QSlider *>(widget))
            widget->setBackgroundRole(QPalette::Window);
    }

    if(qobject_cast<QPushButton *>(widget) ||
       qobject_cast<QComboBox *>(widget) ||
       qobject_cast<QAbstractSpinBox *>(widget) ||
       qobject_cast<QCheckBox *>(widget) ||
       qobject_cast<QGroupBox *>(widget) ||
       qobject_cast<QRadioButton *>(widget) ||
       qobject_cast<QSplitterHandle *>(widget) ||
       qobject_cast<QSlider *>(widget) ||
       qobject_cast<QHeaderView *>(widget) ||
       qobject_cast<QTabBar *>(widget) ||
       qobject_cast<QAbstractScrollArea *>(widget) ||
       qobject_cast<QTextEdit *>(widget) ||
       qobject_cast<QLineEdit *>(widget) ||
//       qobject_cast<QDockWidget *>(widget) ||
       widget->inherits("QWorkspaceTitleBar") ||
       widget->inherits("QDockSeparator") ||
       widget->inherits("QDockWidgetSeparator") ||
       widget->inherits("Q3DockWindowResizeHandle"))
        widget->setAttribute(Qt::WA_Hover, false);
    if (qobject_cast<QScrollBar *>(widget))
    {
        widget->setAttribute(Qt::WA_Hover, false);
        if(QTC_ROUNDED && !opts.flatSbarButtons)
            widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
        if(!opts.gtkScrollViews)
            widget->removeEventFilter(this);
    }
    else if (qobject_cast<QProgressBar *>(widget))
    {
        widget->removeEventFilter(this);
        itsProgressBars.remove((QProgressBar *)widget);
    }
    else if (widget->inherits("Q3Header"))
    {
        widget->setMouseTracking(false);
        widget->removeEventFilter(this);
    }
    else if(opts.highlightScrollViews && widget->inherits("Q3ScrollView"))
        widget->removeEventFilter(this);
    else if(qobject_cast<QMenuBar *>(widget))
    {
#ifdef QTC_XBAR_SUPPORT
        Bespin::MacMenu::release((QMenuBar *)widget);
#endif
        widget->setAttribute(Qt::WA_Hover, false);

        if(!IS_FLAT(opts.bgndAppearance))
            widget->setBackgroundRole(QPalette::Background);

//         if(opts.shadeMenubarOnlyWhenActive && SHADE_NONE!=opts.shadeMenubars)
            widget->removeEventFilter(this);

        if(opts.customMenuTextColor || SHADE_BLEND_SELECTED==opts.shadeMenubars ||
           SHADE_SELECTED==opts.shadeMenubars ||
           (SHADE_CUSTOM==opts.shadeMenubars &&TOO_DARK(itsMenubarCols[ORIGINAL_SHADE])))
            widget->setPalette(QApplication::palette());
    }
    else if(qobject_cast<QLabel*>(widget))
        widget->removeEventFilter(this);
    else if(/*!opts.gtkScrollViews && */qobject_cast<QAbstractScrollArea *>(widget))
    {
        if(!opts.gtkScrollViews && (((QFrame *)widget)->frameWidth()>0))
            widget->removeEventFilter(this);
        if(APP_KONTACT==theThemedApp && widget->parentWidget())
        {
            QWidget *frame=scrollViewFrame(widget->parentWidget());

            if(frame)
            {
                if(itsSViewContainers.contains(frame))
                {
                    itsSViewContainers[frame].remove(widget);
                    if(0==itsSViewContainers[frame].count())
                    {
                        frame->removeEventFilter(this);
                        itsSViewContainers.remove(frame);
                        disconnect(frame, SIGNAL(destroyed(QObject *)), this, SLOT(widgetDestroyed(QObject *)));
                    }
                }
            }
        }
    }
    else if(opts.fixParentlessDialogs && qobject_cast<QDialog *>(widget))
        widget->removeEventFilter(this);
    if (!widget->isWindow())
        if (QFrame *frame = qobject_cast<QFrame *>(widget))
        {
//             if (QFrame::HLine==frame->frameShape() || QFrame::VLine==frame->frameShape())
                 widget->removeEventFilter(this);

#ifdef QTC_QT_ONLY
            if(widget->parent() && widget->parent()->inherits("KTitleWidget"))
#else
            if(widget->parent() && qobject_cast<KTitleWidget *>(widget->parent()))
#endif
            {
                if(IS_FLAT(opts.bgndAppearance))
                    frame->setBackgroundRole(QPalette::Base);
                else
                    frame->setAutoFillBackground(true);

                QLayout *layout(frame->layout());

                if(layout)
                    layout->setMargin(6);
            }
        }

    if(!IS_FLAT(opts.menuBgndAppearance))
        widget->removeEventFilter(this);

    if (qobject_cast<QMenuBar *>(widget) ||
        widget->inherits("Q3ToolBar") ||
        qobject_cast<QToolBar *>(widget) ||
        (widget && qobject_cast<QToolBar *>(widget->parent())))
        widget->setBackgroundRole(QPalette::Button);
}

//
// QtCurve's menu's have a 2 pixel border all around - but want the top, and left edges to
// active the nearest menu item. Therefore, when we get a mouse event in that region then
// adjsut its position...
static bool updateMenuBarEvent(QMouseEvent *event, QMenuBar *menu)
{
    struct HackEvent : public QMouseEvent
    {
        bool adjust()
        {
            if(p.x()<2 || p.y()<2)
            {
                p=QPoint(p.x()<2 ? p.x()+2 : p.x(), p.y()<2 ? p.y()+2 : p.y());
                g=QPoint(p.x()<2 ? g.x()+2 : g.x(), p.y()<2 ? g.y()+2 : g.y());
                return true;
            }
            return false;
        }
    };

    struct HackedMenu : public QMenuBar
    {
        void send(QMouseEvent *ev)
        {
            event(ev);
        }
    };

    if(((HackEvent *)event)->adjust())
    {
        ((HackedMenu *)menu)->send(event);
        return true;
    }
    return false;
}

bool QtCurveStyle::eventFilter(QObject *object, QEvent *event)
{
    bool isSViewCont=APP_KONTACT==theThemedApp && itsSViewContainers.contains((QWidget*)object);

    if(::qobject_cast<QMenuBar *>(object) && dynamic_cast<QMouseEvent *>(event))
    {
        if(updateMenuBarEvent((QMouseEvent *)event, (QMenuBar*)object))
            return true;
    }

    if((!opts.gtkScrollViews &&  ::qobject_cast<QAbstractScrollArea *>(object)) || isSViewCont)
    {
        QPoint pos;
        switch(event->type())
        {
            case QEvent::MouseMove:
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
                pos=((QMouseEvent *)event)->pos();
                break;
            case QEvent::Wheel:
                pos=((QWheelEvent *)event)->pos();
            default:
                break;
        }

        if(!pos.isNull())
        {
            QAbstractScrollArea *area=0L;
            QPoint              mapped(pos);

            if(isSViewCont)
            {
                QSet<QWidget *>::ConstIterator it(itsSViewContainers[(QWidget *)object].begin()),
                                               end(itsSViewContainers[(QWidget *)object].end());

                for(; it!=end && !area; ++it)
                    if((*it)->isVisible())
                    {
                        mapped=(*it)->mapFrom((QWidget *)object, pos);
                        if((*it)->rect().adjusted(0, 0, 4, 4).contains(mapped))
                            area=(QAbstractScrollArea *)(*it);
                    }
            }
            else
                area=(QAbstractScrollArea *)object;

            if(area)
            {
                QScrollBar *sbars[2]={area->verticalScrollBar(), area->horizontalScrollBar() };

                for(int i=0; i<2; ++i)
                    if(sbars[i])
                    {
                        QRect r(i ? 0 : area->rect().right()-3,   i ? area->rect().bottom()-3 : 0,
                                sbars[i]->rect().width(), sbars[i]->rect().height());

                        if(r.contains(pos) ||
                            (sbars[i]==itsSViewSBar &&
                             (QEvent::MouseMove==event->type() ||
                              QEvent::MouseButtonRelease==event->type())))
                        {
                            if(QEvent::Wheel!=event->type())
                            {
                                struct HackEvent : public QMouseEvent
                                {
                                    void set(const QPoint &mapped, bool vert)
                                    {
                                        p=QPoint(vert ? 0 : mapped.x(), vert ? mapped.y() : 0);
                                        g=QPoint(g.x()+(vert ? 0 : -3), g.y()+(vert ? -3 : 0));
                                    }
                                };

                                ((HackEvent *)event)->set(mapped, 0==i);
                            }
                            sbars[i]->event(event);
                            if(QEvent::MouseButtonPress==event->type())
                                itsSViewSBar=sbars[i];
                            else if(QEvent::MouseButtonRelease==event->type())
                                itsSViewSBar=0L;
                            return true;
                        }
                    }
            }
        }
    }

    if(!IS_FLAT(opts.bgndAppearance) && QEvent::Paint==event->type())
    {
        QWidget *widget=qobject_cast<QWidget *>(object);

        if(widget && widget->isWindow() && widget->isVisible() &&
           widget->testAttribute(Qt::WA_StyledBackground) && !widget->testAttribute(Qt::WA_NoSystemBackground))
            drawWindowBackground(widget);
    }

    switch(event->type())
    {
        case QEvent::Paint:
        {
            if(!IS_FLAT(opts.menuBgndAppearance) && qobject_cast<QMenu*>(object))
            {
                QWidget *widget=(QWidget*)object;
                QPainter painter(widget);

                drawBevelGradientReal(USE_LIGHTER_POPUP_MENU ? itsLighterPopupMenuBgndCol : itsBackgroundCols[ORIGINAL_SHADE],
                                      &painter, widget->rect(), GT_HORIZ==opts.menuBgndGrad, false,
                                      opts.menuBgndAppearance, WIDGET_OTHER);
            }
            else
            {
                QFrame *frame = qobject_cast<QFrame*>(object);

                if (frame)
                {
                    if(QFrame::HLine==frame->frameShape() || QFrame::VLine==frame->frameShape())
                    {
                        QPainter painter(frame);
                        QRect    r(QFrame::HLine==frame->frameShape()
                                    ? QRect(frame->rect().x(), frame->rect().y()+ (frame->rect().height()/2), frame->rect().width(), 1)
                                    : QRect(frame->rect().x()+(frame->rect().width()/2),  frame->rect().y(), 1, frame->rect().height()));

                        drawFadedLine(&painter, r, backgroundColors(frame->palette().window().color())[QT_STD_BORDER], true, true, QFrame::HLine==frame->frameShape());
                        return true;
                    }
                    else
                        return false;
                }
                else if(itsClickedLabel==object && qobject_cast<QLabel*>(object) && ((QLabel *)object)->buddy() && ((QLabel *)object)->buddy()->isEnabled())
                {
                    // paint focus rect
                    QLabel                *lbl = (QLabel *)object;
                    QPainter              painter(lbl);
                    QStyleOptionFocusRect opts;

                    opts.palette = lbl->palette();
                    opts.rect    = QRect(0, 0, lbl->width(), lbl->height());
                    drawPrimitive(PE_FrameFocusRect, &opts, &painter, lbl);
                }
            }
            break;
        }
        case QEvent::MouseButtonPress:
            if(qobject_cast<QLabel*>(object) && ((QLabel *)object)->buddy() && dynamic_cast<QMouseEvent*>(event))
            {
                QLabel      *lbl = (QLabel *)object;
                QMouseEvent *mev = (QMouseEvent *)event;

                if (lbl->rect().contains(mev->pos()))
                {
                    itsClickedLabel=lbl;
                    lbl->repaint();
                }
            }
            break;
        case QEvent::MouseButtonRelease:
            if(qobject_cast<QLabel*>(object) && ((QLabel *)object)->buddy() && dynamic_cast<QMouseEvent*>(event))
            {
                QLabel      *lbl = (QLabel *)object;
                QMouseEvent *mev = (QMouseEvent *)event;

                if(itsClickedLabel)
                {
                    itsClickedLabel=0;
                    lbl->update();
                }

                // set focus to the buddy...
                if (lbl->rect().contains(mev->pos()))
                    ((QLabel *)object)->buddy()->setFocus(Qt::ShortcutFocusReason);
            }
            break;
        case QEvent::StyleChange:
        case QEvent::Show:
        {
            QProgressBar *bar = qobject_cast<QProgressBar *>(object);

            if(bar)
            {
                itsProgressBars.insert(bar);
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
            if(itsHoverWidget && object==itsHoverWidget)
            {
                itsPos.setX(-1);
                itsPos.setY(-1);
                itsHoverWidget=0L;
            }

            // The Destroy event is sent from ~QWidget, which happens after
            // ~QProgressBar - therefore, we can't cast to a QProgressBar.
            // So we have to check on object.
            if(object && !itsProgressBars.isEmpty())
            {
                itsProgressBars.remove(reinterpret_cast<QProgressBar*>(object));
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
                    itsHoverWidget=0L;
            }
            break;
        case QEvent::Leave:
            if(itsHoverWidget && object==itsHoverWidget)
            {
                itsPos.setX(-1);
                itsPos.setY(-1);
                itsHoverWidget=0L;
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
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            if(opts.highlightScrollViews && object->isWidgetType() && object->inherits("Q3ScrollView"))
                ((QWidget *)object)->repaint();
            break;
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
                    QWidget *activeWindow=getActiveWindow((QWidget *)object);

                    if(activeWindow)
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
            if ((opts.animatedProgress && 0==itsAnimateStep%2 && bar->value()!=bar->minimum() && bar->value()!=bar->maximum()) ||
                (0==bar->minimum() && 0==bar->maximum()))
                bar->update();
    }

    event->ignore();
}

int QtCurveStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch(metric)
    {
#ifndef QTC_QT_ONLY
        case PM_ButtonIconSize:
        case PM_SmallIconSize:
        {
            KConfigGroup g(KGlobal::config(), "SmallIcons");
            return g.readEntry("Size", 16);
        }
        case PM_ToolBarIconSize:
        {
            KConfigGroup g(KGlobal::config(), "MainToolbarIcons");
            return g.readEntry("Size", 22);
        }
#endif
#if QT_VERSION >= 0x040500
        case PM_SubMenuOverlap:
            return -2;
        case PM_ScrollView_ScrollBarSpacing:
#else
        case PM_TextCursorWidth+3:
#endif
            return opts.etchEntry ? 2 : 3;
        case PM_MenuPanelWidth:
            return opts.popupBorder ? pixelMetric(PM_DefaultFrameWidth, option, widget) : 0;
        case PM_SizeGripSize:
            return SIZE_GRIP_SIZE;
        case PM_TabBarScrollButtonWidth:
            return 18;
        case PM_HeaderMargin:
            return 3;
        case PM_DefaultChildMargin:
            return 6;
        case PM_DefaultTopLevelMargin:
            return 9;
        case PM_LayoutHorizontalSpacing:
        case PM_LayoutVerticalSpacing:
            return -1; // use layoutSpacingImplementation
        case PM_DefaultLayoutSpacing:
            return 6;
        case PM_LayoutLeftMargin:
        case PM_LayoutTopMargin:
        case PM_LayoutRightMargin:
        case PM_LayoutBottomMargin:
            return pixelMetric((option && (option->state&QStyle::State_Window)) || (widget && widget->isWindow())
                                ? PM_DefaultTopLevelMargin
                                : PM_DefaultChildMargin, option, widget);
        case PM_MenuBarItemSpacing:
            return 0;
        case PM_FocusFrameVMargin:
        case PM_FocusFrameHMargin:
            return 2;
        case PM_MenuBarVMargin:
        case PM_MenuBarHMargin:
#ifdef QTC_XBAR_SUPPORT
            if(widget && 0==widget->size().height())
                return 0;
#endif
            return 2;
        case PM_MenuHMargin:
        case PM_MenuVMargin:
            return 0;
        case PM_MenuButtonIndicator:
            return QTC_DO_EFFECT ? 16 : 15;
        case PM_ButtonMargin:
            return (QTC_DO_EFFECT
                    ? opts.thinnerBtns ? 4 : 6
                    : opts.thinnerBtns ? 2 : 4)+QTC_MAX_ROUND_BTN_PAD;
        case PM_TabBarTabShiftVertical:
#ifdef QTC_STYLE_QTABBAR
            if(widget && widget->parentWidget() && !qobject_cast<const QTabWidget *>(widget->parentWidget()))
                return option && option->state & State_Selected ? 0 : -2;
#endif
#if QT_VERSION < 0x040500
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option))
            {
                 if(qtVersion()<VER_45 && (QTabBar::RoundedSouth==tab->shape || QTabBar::TriangularSouth==tab->shape))
                    return -2;
            }
#endif
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
            if ((!opts.popupBorder || opts.gtkComboMenus) && widget && widget->inherits("QComboBoxPrivateContainer"))
                return opts.gtkComboMenus ? 1 : 0;

            if ((!opts.gtkScrollViews || opts.squareScrollViews) && isKateView(widget))
                return opts.squareScrollViews ? 1 : 0;

            if (opts.squareScrollViews && widget &&
                (::qobject_cast<const QAbstractScrollArea *>(widget) || isKontactPreviewPane(widget)))
                return opts.gtkScrollViews ? 1 : 2;

            if ((USE_LIGHTER_POPUP_MENU || !IS_FLAT(opts.menuBgndAppearance)) && !opts.borderMenuitems &&
                qobject_cast<const QMenu *>(widget))
                return 1;

            if(QTC_DO_EFFECT && opts.etchEntry &&
                (!widget || // !isFormWidget(widget) &&
                ::qobject_cast<const QLineEdit *>(widget) ||
                ::qobject_cast<const QAbstractScrollArea*>(widget) ||
                widget->inherits("Q3ScrollView") /*||
                     isKontactPreviewPane(widget)*/))
                return 3;
            else
                return 2;
        case PM_SpinBoxFrameWidth:
            return QTC_DO_EFFECT ? 3 : 2;
        case PM_IndicatorWidth:
        case PM_IndicatorHeight:
            return QTC_DO_EFFECT
                        ? QTC_CHECK_SIZE+2 : QTC_CHECK_SIZE;
        case PM_ExclusiveIndicatorWidth:
        case PM_ExclusiveIndicatorHeight:
            return QTC_DO_EFFECT
                        ? QTC_RADIO_SIZE+2 : QTC_RADIO_SIZE;
        case PM_TabBarTabOverlap:
            return TAB_MO_GLOW==opts.tabMouseOver ? 0 : 1;
        case PM_ProgressBarChunkWidth:
            return 4;
//         case PM_DockWindowSeparatorExtent:
//             return 4;
//         case PM_DockWindowHandleExtent:
//             return 10;
        case PM_SplitterWidth:
            return /*LINE_1DOT==opts.splitters ? 7 : */6;
        case PM_ScrollBarSliderMin:
            return opts.sliderWidth+1;
        case PM_SliderThickness:
            return SLIDER_TRIANGULAR==opts.sliderStyle ? 19 : (QTC_SLIDER_SIZE+(QTC_ROTATED_SLIDER ? 11 : 6));
        case PM_SliderControlThickness:
            return SLIDER_TRIANGULAR==opts.sliderStyle ? 11 : (QTC_SLIDER_SIZE+(QTC_ROTATED_SLIDER ? 6 : -2));
         case PM_SliderTickmarkOffset:
             return SLIDER_TRIANGULAR==opts.sliderStyle ? 5 : 4;
        case PM_SliderSpaceAvailable:
            if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
            {
                int size(SLIDER_TRIANGULAR==opts.sliderStyle ? 17 : (QTC_SLIDER_SIZE+(QTC_ROTATED_SLIDER ? 6 : -2)));

                if (slider->tickPosition & QSlider::TicksBelow)
                    ++size;
                if (slider->tickPosition & QSlider::TicksAbove)
                    ++size;
                return size;
            }
            return QTC_BASE_STYLE::pixelMetric(metric, option, widget);
        case PM_SliderLength:
            return SLIDER_TRIANGULAR==opts.sliderStyle ? 11 : (QTC_SLIDER_SIZE+(QTC_ROTATED_SLIDER ? -2 : 6));
        case PM_ScrollBarExtent:
            return opts.sliderWidth;
        case PM_MaximumDragDistance:
            return -1;
        case PM_TabBarTabHSpace:
            return 14;
        case PM_TabBarTabVSpace:
            return opts.highlightTab ? 10 : 8;
        case PM_TitleBarHeight:
            return qMax(widget ? widget->fontMetrics().lineSpacing()
                               : option ? option->fontMetrics.lineSpacing()
                                        : 0, 24);
        case QtC_Round:
            return (int)opts.round;
        case QtC_TitleBarColorTopOnly:
            return opts.colorTitlebarOnly;
        case QtC_TitleBarButtonAppearance:
            return (int)opts.titlebarButtonAppearance;
        case QtC_TitleAlignment:
            switch(opts.titlebarAlignment)
            {
                default:
                case ALIGN_LEFT:
                    return Qt::AlignLeft;
                case ALIGN_CENTER:
                    return Qt::AlignHCenter|Qt::AlignVCenter;
                case ALIGN_FULL_CENTER:
                    return Qt::AlignHCenter;
                case ALIGN_RIGHT:
                    return Qt::AlignRight;
            }
        case QtC_TitleBarButtons:
            return opts.titlebarButtons;
        case QtC_TitleBarIcon:
            return opts.titlebarIcon;
        case QtC_TitleBarIconColor:
            return buttonColors(option)[ORIGINAL_SHADE].rgb();
// The following is a somewhat hackyish fix for konqueror's show close button on tab setting...
// ...its hackish in the way that I'm assuming when KTabBar is positioning the close button and it
// asks for these options, it only passes in a QStyleOption  not a QStyleOptionTab
//.........
        case PM_TabBarBaseHeight:
#ifdef QTC_QT_ONLY
            if(widget && widget->inherits("KTabBar") && !qstyleoption_cast<const QStyleOptionTab *>(option))
#else
            if(widget && qobject_cast<const KTabBar*>(widget) && !qstyleoption_cast<const QStyleOptionTab *>(option))
#endif
                return 10;
            return QTC_BASE_STYLE::pixelMetric(metric, option, widget);
        case PM_TabBarBaseOverlap:
#ifdef QTC_QT_ONLY
            if(widget && widget->inherits("KTabBar") && !qstyleoption_cast<const QStyleOptionTab *>(option))
#else
            if(widget && qobject_cast<const KTabBar*>(widget) && !qstyleoption_cast<const QStyleOptionTab *>(option))
#endif
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
        case SH_RubberBand_Mask:
        {
            const QStyleOptionRubberBand *opt = qstyleoption_cast<const QStyleOptionRubberBand *>(option);
            if (!opt)
                return true;
            if (QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask*>(returnData))
            {
                mask->region = option->rect;
                mask->region -= option->rect.adjusted(1,1,-1,-1);
            }
            return true;
        }
        case SH_Menu_SubMenuPopupDelay:
            return opts.menuDelay;
        case SH_ToolButton_PopupDelay:
            return 250;
        case SH_Menu_AllowActiveAndDisabled:
            return false;
        case SH_ComboBox_PopupFrameStyle:
            return QFrame::StyledPanel|QFrame::Plain;
        case SH_TabBar_Alignment:
            return Qt::AlignLeft;
        case SH_Header_ArrowAlignment:
            return Qt::AlignLeft;
        case SH_MessageBox_CenterButtons:
            return false;
        case SH_ProgressDialog_CenterCancelButton:
            return false;
        case SH_PrintDialog_RightAlignButtons:
            return true;
        case SH_DitherDisabledText:
            return false;
        case SH_EtchDisabledText:
            return false;
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
                if (titleBar && (titleBar->titleBarState&Qt::WindowMinimized))
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
//        case SH_ItemView_ArrowKeysNavigateIntoChildren:
//            return false;
//         case SH_ItemView_ChangeHighlightOnFocus: // gray out selected items when losing focus.
//             return true;
        case SH_ItemView_ShowDecorationSelected:
            return false; // Controls whether the highlighting of listview/treeview items highlights whole line.
        case SH_ToolBox_SelectedPageTitleBold:
        case SH_ScrollBar_MiddleClickAbsolutePosition:
            return true;
        case SH_MainWindow_SpaceBelowMenuBar:
#ifdef QTC_XBAR_SUPPORT
            if (const QMenuBar *menubar = qobject_cast<const QMenuBar*>(widget))
                if (0==menubar->height() && !menubar->actions().isEmpty())
                {   // we trick menubars if we use macmenus - hehehe...
                    // NOTICE the final result NEEDS to be > "0" (i.e. "1") to avoid side effects...
                    return -menubar->actionGeometry(menubar->actions().first()).height() + 1;
                }
#endif

            return 0;
        case SH_SpinControls_DisableOnBounds:
            return true;
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
        case SH_MenuBar_MouseTracking:
            // Always return 1, as setting to 0 dissables the effect when a menu is shown.
            return 1; // opts.menubarMouseOver ? 1 : 0;
        case SH_ScrollView_FrameOnlyAroundContents:
            return widget && widget->isWindow()
                    ? false
                    : opts.gtkScrollViews && (!widget || !widget->inherits("QComboBoxListView"));
        case SH_ComboBox_Popup:
            if(opts.gtkComboMenus)
            {
                if (widget && widget->inherits("Q3ComboBox"))
                    return 0;
                if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(option))
                    return !cmb->editable;
            }
            return 0;
#if QT_VERSION >= 0x040400
        // per HIG, align the contents in a form layout to the left
        case SH_FormLayoutFormAlignment:
            return Qt::AlignLeft | Qt::AlignTop;
        // per HIG, align the labels in a form layout to the right
        case SH_FormLayoutLabelAlignment:
            return Qt::AlignRight;
        case SH_FormLayoutFieldGrowthPolicy:
            return QFormLayout::ExpandingFieldsGrow;
        case SH_FormLayoutWrapPolicy:
            return QFormLayout::DontWrapRows;
#endif
#if !defined QTC_QT_ONLY
        case SH_DialogButtonBox_ButtonsHaveIcons:
            return KGlobalSettings::showIconsOnPushButtons();
        case SH_ItemView_ActivateItemOnSingleClick:
            return KGlobalSettings::singleClick();
#endif
        case SH_MenuBar_AltKeyNavigation:
            return false;
        default:
#if !defined QTC_QT_ONLY
            // Tell the calling app that we can handle certain custom widgets...
            if(hint>=SH_CustomBase && widget)
                if("CE_CapacityBar"==widget->objectName())
                    return CE_QtC_KCapacityBar;
#endif
            return QTC_BASE_STYLE::styleHint(hint, option, widget, returnData);
   }
}

QPalette QtCurveStyle::standardPalette() const
{
#if defined QTC_QT_ONLY
    return QTC_BASE_STYLE::standardPalette();
#else
    return KGlobalSettings::createApplicationPalette(KSharedConfig::openConfig(itsComponentData));
#endif
}

QIcon QtCurveStyle::standardIconImplementation(StandardPixmap pix, const QStyleOption *option, const QWidget *widget) const
{
#if !defined QTC_QT_ONLY
    switch(pix)
    {
//         case SP_TitleBarMenuButton:
//         case SP_TitleBarMinButton:
//         case SP_TitleBarMaxButton:
//         case SP_TitleBarCloseButton:
//         case SP_TitleBarNormalButton:
//         case SP_TitleBarShadeButton:
//         case SP_TitleBarUnshadeButton:
//         case SP_TitleBarContextHelpButton:
//         case SP_DockWidgetCloseButton:
        case SP_MessageBoxInformation:
            return KIcon("dialog-information");
        case SP_MessageBoxWarning:
            return KIcon("dialog-warning");
        case SP_MessageBoxCritical:
            return KIcon("dialog-error");
        case SP_MessageBoxQuestion:
            return KIcon("dialog-information");
        case SP_DesktopIcon:
            return KIcon("user-desktop");
        case SP_TrashIcon:
            return KIcon("user-trash");
        case SP_ComputerIcon:
            return KIcon("computer");
        case SP_DriveFDIcon:
            return KIcon("media-floppy");
        case SP_DriveHDIcon:
            return KIcon("drive-harddisk");
        case SP_DriveCDIcon:
        case SP_DriveDVDIcon:
            return KIcon("media-optical");
        case SP_DriveNetIcon:
            return KIcon("network-server");
        case SP_DirOpenIcon:
            return KIcon("document-open");
        case SP_DirIcon:
        case SP_DirClosedIcon:
            return KIcon("folder");
//         case SP_DirLinkIcon:
        case SP_FileIcon:
            return KIcon("application-x-zerosize");
//         case SP_FileLinkIcon:
//         case SP_ToolBarHorizontalExtensionButton:
//         case SP_ToolBarVerticalExtensionButton:
        case SP_FileDialogStart:
            return KIcon(Qt::RightToLeft==QApplication::layoutDirection()
                                                    ? "go-edn" : "go-first");
        case SP_FileDialogEnd:
            return KIcon(Qt::RightToLeft==QApplication::layoutDirection()
                                                    ? "go-first" : "go-end");
        case SP_FileDialogToParent:
            return KIcon("go-up");
        case SP_FileDialogNewFolder:
            return KIcon("folder-new");
        case SP_FileDialogDetailedView:
            return KIcon("view-list-details");
//         case SP_FileDialogInfoView:
//             return KIcon("dialog-ok");
//         case SP_FileDialogContentsView:
//             return KIcon("dialog-ok");
        case SP_FileDialogListView:
            return KIcon("view-list-icons");
        case SP_FileDialogBack:
            return KIcon(Qt::RightToLeft==QApplication::layoutDirection()
                                                    ? "go-next" : "go-previous");
        case SP_DialogOkButton:
            return KIcon("dialog-ok");
        case SP_DialogCancelButton:
            return KIcon("dialog-cancel");
        case SP_DialogHelpButton:
            return KIcon("help-contents");
        case SP_DialogOpenButton:
            return KIcon("document-open");
        case SP_DialogSaveButton:
            return KIcon("document-save");
        case SP_DialogCloseButton:
            return KIcon("dialog-close");
        case SP_DialogApplyButton:
            return KIcon("dialog-ok-apply");
        case SP_DialogResetButton:
            return KIcon("document-revert");
//         case SP_DialogDiscardButton:
//              return KIcon("dialog-cancel");
        case SP_DialogYesButton:
            return KIcon("dialog-ok");
        case SP_DialogNoButton:
            return KIcon("dialog-cancel");
        case SP_ArrowUp:
            return KIcon("arrow-up");
        case SP_ArrowDown:
            return KIcon("arrow-down");
        case SP_ArrowLeft:
            return KIcon("arrow-left");
        case SP_ArrowRight:
            return KIcon("arrow-right");
        case SP_ArrowBack:
            return KIcon(Qt::RightToLeft==QApplication::layoutDirection()
                                                    ? "go-next" : "go-previous");
        case SP_ArrowForward:
            return KIcon(Qt::RightToLeft==QApplication::layoutDirection()
                                                    ? "go-previous"
                                                    : "go-next");
        case SP_DirHomeIcon:
            return KIcon("user-home");
//         case SP_CommandLink:
//         case SP_VistaShield:
        case SP_BrowserReload:
            return KIcon("view-refresh");
        case SP_BrowserStop:
            return KIcon("process-stop");
        case SP_MediaPlay:
            return KIcon("media-playback-start");
        case SP_MediaStop:
            return KIcon("media-playback-stop");
        case SP_MediaPause:
            return KIcon("media-playback-pause");
        case SP_MediaSkipForward:
            return KIcon("media-skip-forward");
        case SP_MediaSkipBackward:
            return KIcon("media-skip-backward");
        case SP_MediaSeekForward:
            return KIcon("media-seek-forward");
        case SP_MediaSeekBackward:
            return KIcon("media-seek-backward");
        case SP_MediaVolume:
            return KIcon("player-volume");
        case SP_MediaVolumeMuted:
            return KIcon("player-volume-muted");
        default:
            break;
    }
#endif
    return QTC_BASE_STYLE::standardIconImplementation(pix, option, widget);
}

int QtCurveStyle::layoutSpacingImplementation(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
                                              Qt::Orientation orientation, const QStyleOption *option,
                                              const QWidget *widget) const
{
    Q_UNUSED(control1); Q_UNUSED(control2); Q_UNUSED(orientation);

    return pixelMetric(PM_DefaultLayoutSpacing, option, widget);
}

void QtCurveStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                                 const QWidget *widget) const
{
    QRect               r(option->rect);
    const QFlags<State> &state(option->state);
    const QPalette      &palette(option->palette);
    bool                reverse(Qt::RightToLeft==option->direction);

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

                if(LV_OLD==opts.lvLines)
                {
                    beforeH=ar.x();
                    beforeV=ar.y();
                    afterH=ar.x()+QTC_LV_SIZE+4;
                    afterV=ar.y()+QTC_LV_SIZE+4;

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

                drawArrow(painter, ar, state&State_Open
                                                ? PE_IndicatorArrowDown
                                                : reverse
                                                    ? PE_IndicatorArrowLeft
                                                    : PE_IndicatorArrowRight, palette.text().color());
            }

            const int constStep=LV_OLD==opts.lvLines
                                    ? 0
                                    : widget && qobject_cast<const QTreeView *>(widget)
                                        ? ((QTreeView *)widget)->indentation() : 20;

            if(opts.lvLines && (LV_OLD==opts.lvLines || (r.x()>=constStep && constStep>0)))
            {
                painter->setPen(palette.mid().color());
                if (state&State_Item)
                    if (reverse)
                        painter->drawLine(r.left(), middleV, afterH, middleV);
                    else
                    {
                        if(LV_NEW==opts.lvLines)
                        {
                            if(state&State_Children)
                                painter->drawLine(middleH-constStep, middleV, r.right()-constStep, middleV);
                            else
                                drawFadedLine(painter, QRect(middleH-constStep, middleV, r.right()-(middleH-constStep), middleV), palette.mid().color(),
                                              false, true, true);
                        }
                        else
                            painter->drawLine(afterH, middleV, r.right(), middleV);
                    }
                if (state&State_Sibling && afterV<r.bottom())
                    painter->drawLine(middleH-constStep, afterV, middleH-constStep, r.bottom());
                if (state & (State_Open | State_Children | State_Item | State_Sibling) && (LV_NEW==opts.lvLines || beforeV>r.y()))
                    painter->drawLine(middleH-constStep, r.y(), middleH-constStep, beforeV);
            }
            painter->restore();
            break;
        }
        case PE_IndicatorViewItemCheck:
        {
            QStyleOption opt(*option);

            opt.state &= ~State_MouseOver;
            opt.state |= QTC_STATE_MENU;
            drawPrimitive(PE_IndicatorCheckBox, &opt, painter, widget);
            break;
        }
        case PE_IndicatorHeaderArrow:
            if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option))
                drawArrow(painter, r,
                          header->sortIndicator & QStyleOptionHeader::SortUp ? PE_IndicatorArrowUp : PE_IndicatorArrowDown,
                          QTC_MO_ARROW(QPalette::ButtonText));
            break;
        case PE_IndicatorArrowUp:
        case PE_IndicatorArrowDown:
        case PE_IndicatorArrowLeft:
        case PE_IndicatorArrowRight:
            if(state&(State_Sunken|State_On) &&
               !(widget && ( (opts.unifySpin && qobject_cast<const QSpinBox *>(widget)) ||
                             (opts.unifyCombo && qobject_cast<const QComboBox *>(widget) && ((const QComboBox *)widget)->isEditable()))))
                r.adjust(1, 1, 1, 1);
            drawArrow(painter, r, element, QTC_MO_ARROW(QPalette::Text));
            break;
        case PE_IndicatorSpinMinus:
        case PE_IndicatorSpinPlus:
        case PE_IndicatorSpinUp:
        case PE_IndicatorSpinDown:
        {
            QRect        sr(r);
            const QColor *use(buttonColors(option)),
                         col(QTC_MO_ARROW(QPalette::ButtonText));
            bool         down(PE_IndicatorSpinDown==element || PE_IndicatorSpinMinus==element);

            if((!opts.unifySpinBtns || state&State_Sunken) && !opts.unifySpin)
                drawLightBevel(painter, sr, option, widget, down
                                                    ? reverse
                                                            ? ROUNDED_BOTTOMLEFT
                                                            : ROUNDED_BOTTOMRIGHT
                                                        : reverse
                                                            ? ROUNDED_TOPLEFT
                                                            : ROUNDED_TOPRIGHT,
                               getFill(option, use), use, true, WIDGET_SPIN);

            if(PE_IndicatorSpinUp==element || PE_IndicatorSpinDown==element)
            {
                sr.setY(sr.y()+(down ? -2 : 1));

                if(opts.unifySpin)
                {
                    sr.adjust(reverse ? 1 : -1, 0, reverse ? 1 : -1, 0);
                    if(!opts.vArrows)
                        sr.setY(sr.y()+(down ? -2 : 2));
                }
                else if(state&State_Sunken)
                    sr.adjust(1, 1, 1, 1);

                drawArrow(painter, sr, PE_IndicatorSpinUp==element ? PE_IndicatorArrowUp : PE_IndicatorArrowDown,
                          col, !opts.unifySpin);
            }
            else
            {
                int    l(qMin(r.width()-6, r.height()-6));
                QPoint c(r.x()+(r.width()/2), r.y()+(r.height()/2));

                l/=2;
                if(l%2 != 0)
                    --l;

                if(state&State_Sunken && !opts.unifySpin)
                    c+=QPoint(1, 1);

                painter->setPen(col);
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
                        drawFadedLine(painter, QRect(x, r.y()+QTC_TOOLBAR_SEP_GAP, 1, r.height()-(QTC_TOOLBAR_SEP_GAP*2)),
                                      itsBackgroundCols[LINE_SUNKEN==opts.toolbarSeparators ? 3 : 4], true, true, false);

                        if(LINE_SUNKEN==opts.toolbarSeparators)
                            drawFadedLine(painter, QRect(x+1, r.y()+6, 1, r.height()-12),
                                          itsBackgroundCols[0], true, true, false);
                    }
                    else
                    {
                        int y(r.y()+((r.height()-2) / 2));

                        drawFadedLine(painter, QRect(r.x()+QTC_TOOLBAR_SEP_GAP, y, r.width()-(QTC_TOOLBAR_SEP_GAP*2), 1),
                                      itsBackgroundCols[LINE_SUNKEN==opts.toolbarSeparators ? 3 : 4], true, true, true);
                        if(LINE_SUNKEN==opts.toolbarSeparators)
                            drawFadedLine(painter, QRect(r.x()+QTC_TOOLBAR_SEP_GAP, y+1, r.width()-(QTC_TOOLBAR_SEP_GAP*2), 1),
                                          itsBackgroundCols[0], true, true, true);
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
            if(opts.framelessGroupBoxes && !opts.groupBoxLine)
                break;
            if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(option))
            {
                QStyleOptionFrameV2 frameV2(*frame);
                if (frameV2.features & QStyleOptionFrameV2::Flat || opts.groupBoxLine)
                    drawFadedLine(painter, QRect(r.x(), r.y(), r.width(), 1),
                                  backgroundColors(option)[QT_STD_BORDER], reverse, !reverse, true);
                else
                {
                    frameV2.state &= ~(State_Sunken | State_HasFocus);
                    drawPrimitive(PE_Frame, &frameV2, painter, widget);
                }
            }
            break;
        case PE_Frame:
#ifdef QTC_QT_ONLY
            if(widget && widget->parent() && widget->parent()->inherits("KTitleWidget"))
                break;
#else
            if(widget && widget->parent() && qobject_cast<const KTitleWidget *>(widget->parent()))
                break;
#endif
            else if(widget && widget->parent() && qobject_cast<const QComboBox *>(widget->parent()))
            {
                if(opts.gtkComboMenus && !((QComboBox *)(widget->parent()))->isEditable())
                    drawPrimitive(PE_FrameMenu, option, painter, widget);
                else
                {
                    const QColor *use(APP_KRUNNER==theThemedApp ? itsBackgroundCols : backgroundColors(option));

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
                bool sv(::qobject_cast<const QAbstractScrollArea *>(widget) ||
                        (widget && widget->inherits("Q3ScrollView")) ||
                        (opts.squareScrollViews && (isKateView(widget) || isKontactPreviewPane(widget)))),
                     squareSv(sv && (opts.squareScrollViews || (widget && widget->isWindow())));

                if(sv && (opts.etchEntry || squareSv))
                {
                    if(squareSv)
                    {
                        QColor col(backgroundColors(option)[QT_STD_BORDER]);

                        if(APP_ARORA==theThemedApp)
                            painter->fillRect(r, palette.brush(QPalette::Base));
                        painter->setPen(col);

                        // Flat style...
                        //drawRect(painter, r);
                        // 3d style...
                        painter->drawLine(r.x(), r.y(), r.x(), r.y()+r.height()-1);
                        painter->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                        col.setAlphaF(QT_LOWER_BORDER_ALPHA);
                        painter->setPen(col);
                        painter->drawLine(r.x()+r.width()-1, r.y()+1, r.x()+r.width()-1, r.y()+r.height()-1);
                        painter->drawLine(r.x()+1, r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                    }
                    else
                    {
                        const QStyleOptionFrame *fo = qstyleoption_cast<const QStyleOptionFrame *>(option);

                        // For some reason, in KPackageKit, the KTextBrower when polished is not in the scrollview,
                        // but is when painted. So check here if it should not be etched.
                        // Also, see not in getLowerEtchCol()
                        if(QTC_DO_EFFECT && widget && widget->parentWidget() && !theNoEtchWidgets.contains(widget) &&
                           isInQAbstractItemView(widget->parentWidget()))
                            theNoEtchWidgets.insert(widget);

                        // If we are set to have sunken scrollviews, then the frame width is set to 3.
                        // ...but it we are a scrollview within a scrollview, then we dont draw sunken, therefore
                        // need to draw inner border...
                        bool doEtch=QTC_DO_EFFECT && opts.etchEntry,
                             noEtchW=doEtch && theNoEtchWidgets.contains(widget);
                        if(doEtch && noEtchW)
                        {
                            painter->setPen(palette.brush(QPalette::Base).color());
                            drawRect(painter, r.adjusted(2, 2, -2, -2));
                        }

                        if(!opts.highlightScrollViews && fo)
                        {
                            QStyleOptionFrame opt(*fo);
                            opt.state&=~State_HasFocus;
                            drawEntryField(painter, r, widget, &opt, ROUNDED_ALL, false,
                                           doEtch && !noEtchW, WIDGET_SCROLLVIEW);
                        }
                        else
                            drawEntryField(painter, r, widget, option, ROUNDED_ALL, false,
                                           doEtch && !noEtchW, WIDGET_SCROLLVIEW);
                    }
                }
                else
                {
                    const QStyleOptionFrame *fo = qstyleoption_cast<const QStyleOptionFrame *>(option);

                    if (fo && fo->lineWidth>0)
                    {
                        bool         kwinTab(APP_KWIN==theThemedApp &&  widget && !widget->parentWidget() &&
                                             0==strcmp(widget->metaObject()->className(), "KWin::TabBox"));
                        QStyleOption opt(*option);

                        painter->save();

                        if(kwinTab)
                            r.adjust(-1, -1, 1, 1);

                        if(!opts.highlightScrollViews)
                            opt.state&=~State_HasFocus;

                        if(sv)
                        {
                            painter->setRenderHint(QPainter::Antialiasing, true);
                            painter->setPen(option->palette.brush(QPalette::Base).color());
                            painter->drawPath(buildPath(r.adjusted(1, 1, -1, -1), WIDGET_ENTRY, ROUNDED_ALL,
                                                        getRadius(&opts, r.width()-2, r.height()-2, WIDGET_ENTRY,
                                                                  RADIUS_INTERNAL)));
                            painter->setRenderHint(QPainter::Antialiasing, false);
                        }

                        drawBorder(painter, r, &opt,
                                   opts.round &&
                                    ( (APP_KMIX==theThemedApp &&  widget && widget->parentWidget() && qobject_cast<const QFrame *>(widget) &&
                                       0==strcmp(widget->parentWidget()->metaObject()->className(), "ViewDockAreaPopup")) ||
                                      (APP_KRUNNER==theThemedApp &&  widget && widget->parentWidget() && qobject_cast<const QFrame *>(widget) &&
                                       0==strcmp(widget->parentWidget()->metaObject()->className(), "PasswordDlg")) ||
                                      kwinTab)
                                   ? ROUND_NONE : ROUNDED_ALL, backgroundColors(option),
                                   sv ? WIDGET_SCROLLVIEW : WIDGET_FRAME, state&State_Sunken || state&State_HasFocus
                                                          ? BORDER_SUNKEN
                                                            : state&State_Raised
                                                                ? BORDER_RAISED
                                                                : BORDER_FLAT);
                        painter->restore();
                    }
                }
            }
            break;
        case PE_PanelMenuBar:
            if (widget && widget->parentWidget() && (qobject_cast<const QMainWindow *>(widget->parentWidget()) ||
                                                     widget->parentWidget()->inherits("Q3MainWindow")))
            {
                painter->save();
#ifdef QTC_XBAR_SUPPORT
                if(!widget || 0!=strcmp("QWidget", widget->metaObject()->className()))
#endif
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
                        painter->drawLine(r.x(), r.y(), r.x(), r.y()+r.height()-1);
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
                        painter->setPen(use[QTabBar::RoundedNorth==tbb->shape ? QT_STD_BORDER
                                                                              : (opts.borderTab ? 0 : QT_FRAME_DARK_SHADOW)]);
                    painter->drawLine(topLine);
                    painter->setPen(use[QTabBar::RoundedNorth==tbb->shape ? 0 : QT_STD_BORDER]);
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

            if(!USE_LIGHTER_POPUP_MENU && IS_FLAT(opts.menuBgndAppearance))
            /*
            {
                painter->setPen(itsLighterPopupMenuBgndCol);
                drawRect(painter, r.adjusted(1, 1, -1, -1));
            }
            else
            */
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
                    drawLightBevel(painter, r2, &opt, widget, ROUNDED_NONE, getFill(&opt, use), use, false, WIDGET_MENU_ITEM);
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
            bool dwt(widget && (widget->inherits("QDockWidgetTitleButton") ||
                                (widget->parentWidget() && widget->parentWidget()->inherits("KoDockWidgetTitleBar"))));

            if( ((state&State_Enabled) || !(state&State_AutoRaise)) &&
               (!widget || !dwt || (state&State_MouseOver)) )
            {
                QStyleOption opt(*option);

                if(dwt)
                    opt.state|=QTC_STATE_TBAR_BUTTON;
                drawPrimitive(PE_PanelButtonCommand, &opt, painter, widget);
            }
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
        case PE_PanelLineEdit:
            if (const QStyleOptionFrame *panel = qstyleoption_cast<const QStyleOptionFrame *>(option))
            {
                if(panel->lineWidth > 0)
                {
                    QRect r2(r.adjusted(1, 1, -1, (QTC_DO_EFFECT ? -2 : -1)));
                    painter->fillPath(buildPath(r2, WIDGET_ENTRY, ROUNDED_ALL,
                                                getRadius(&opts, r2.width(), r2.height(), WIDGET_ENTRY, RADIUS_INTERNAL)),
                                      palette.brush(QPalette::Base));
                    drawPrimitive(PE_FrameLineEdit, option, painter, widget);
                }
                else
                    painter->fillRect(r.adjusted(2, 2, -2, -2), palette.brush(QPalette::Base));
            }
            break;
        case PE_FrameLineEdit:
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

                    if(QTC_DO_EFFECT && APP_ARORA==theThemedApp && widget && widget->parentWidget() && 0==strcmp(widget->metaObject()->className(), "LocationBar"))
                    {
                        const QToolBar *tb=(const QToolBar *)getToolBar(widget->parentWidget()/*, false*/);

                        if(tb)
                        {
                            QRect r2(r);

                            struct TB : public QToolBar
                            {
                                void initOpt(QStyleOptionToolBar *opt) { initStyleOption(opt); }
                            };

                            QStyleOptionToolBar opt;

                            ((TB *)tb)->initOpt(&opt);
                            bool horiz=Qt::NoToolBarArea==opt.toolBarArea || Qt::BottomToolBarArea==opt.toolBarArea || Qt::TopToolBarArea==opt.toolBarArea;

//                             if(horiz) // Should be!!!
                            {
                                painter->save();

                                // Only need to adjust coords if toolbar has a gradient...
                                if(!IS_FLAT(opts.toolbarAppearance))
                                {
                                    r2.setY(-widget->mapTo((QWidget *)tb, QPoint(r.x(), r.y())).y());
                                    r2.setHeight(tb->rect().height());
                                }
                                painter->setClipRegion(QRegion(r2).subtract(QRegion(r2.adjusted(2, 2, -2, -2))));
//                                 if(!IS_FLAT(opts.bgndAppearance))
//                                     drawWindowBackground((QWidget *)tb);
//                                     painter->fillRect(r.x(), r.y()+2, r.width(), r.height()-4,
//                                                      palette.window().color());
                                drawMenuOrToolBarBackground(painter, r2, &opt, false, horiz);
                                painter->restore();
                            }
                        }
                    }
                    painter->save();
                    drawEntryField(painter, r, widget, &opt, ROUNDED_ALL, false, QTC_DO_EFFECT);
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
        case PE_IndicatorMenuCheckMark:
        case PE_IndicatorCheckBox:
        {
            bool          menu(state&QTC_STATE_MENU),
                          sunken(!menu && (state&State_Sunken)),
                          mo(!sunken && state&State_MouseOver && state&State_Enabled),
                          doEtch(PE_IndicatorMenuCheckMark!=element && !menu
                                 && r.width()>=QTC_CHECK_SIZE+2 && r.height()>=QTC_CHECK_SIZE+2
                                 && QTC_DO_EFFECT),
                          glow(doEtch && MO_GLOW==opts.coloredMouseOver && mo);
            QRect         rect(doEtch ? r.adjusted(1, 1, -1, -1) : r);
            const QColor *bc(sunken ? 0L : borderColors(option, 0L)),
                         *btn(buttonColors(option)),
                         *use(bc ? bc : btn);
            const QColor &bgnd(opts.crButton
                                ? menu ? btn[ORIGINAL_SHADE] : getFill(option, btn, true)
                                : state&State_Enabled && !sunken
                                    ? MO_NONE==opts.coloredMouseOver && !opts.crHighlight && mo
                                        ? use[QTC_CR_MO_FILL]
                                        : palette.base().color()
                                    : palette.background().color());
            EWidget      wid=opts.crButton ? WIDGET_STD_BUTTON : WIDGET_TROUGH;
            EAppearance  app=opts.crButton ? opts.appearance : APPEARANCE_INVERTED;
            bool         drawSunken=opts.crButton ? sunken : false,
                         lightBorder=QTC_DRAW_LIGHT_BORDER(drawSunken, wid, app),
                         draw3dFull=!lightBorder && QTC_DRAW_3D_FULL_BORDER(drawSunken, app),
                         draw3d=draw3dFull || (!lightBorder && QTC_DRAW_3D_BORDER(drawSunken, app)),
                         drawLight=opts.crButton && !drawSunken && (lightBorder || draw3d),
                         drawDark=drawLight && draw3dFull && !lightBorder;

            painter->save();
            if(IS_FLAT(opts.appearance))
                painter->fillRect(rect.adjusted(1, 1, -1, -1), bgnd);
            else
                drawBevelGradient(bgnd, painter, rect.adjusted(1, 1, -1, -1), true,
                                  drawSunken, MODIFY_AGUA(app), WIDGET_TROUGH);

            if(MO_NONE!=opts.coloredMouseOver && !glow && mo)
            {
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(use[QTC_CR_MO_FILL]);
                drawAaRect(painter, rect.adjusted(1, 1, -1, -1));
//                 drawAaRect(painter, rect.adjusted(2, 2, -2, -2));
                painter->setRenderHint(QPainter::Antialiasing, false);
            }
            else if(!opts.crButton || drawLight)
            {
                painter->setPen(drawLight ? btn[QTC_LIGHT_BORDER(app)]
                                          : midColor(state&State_Enabled ? palette.base().color()
                                                                         : palette.background().color(), use[3]));
                if(lightBorder)
                    drawRect(painter, rect.adjusted(1, 1, -1, -1));
                else
                {
                    painter->drawLine(rect.x()+1, rect.y()+1, rect.x()+1, rect.y()+rect.height()-2);
                    painter->drawLine(rect.x()+1, rect.y()+1, rect.x()+rect.width()-2, rect.y()+1);

                    if(drawDark)
                    {
                        painter->setPen(btn[2]);
                        painter->drawLine(rect.x()+rect.width()-2, rect.y()+1,
                                          rect.x()+rect.width()-2, rect.y()+rect.height()-2);
                        painter->drawLine(rect.x()+1, rect.y()+rect.height()-2,
                                          rect.x()+rect.width()-2, rect.y()+rect.height()-2);
                    }
                }
            }

            if(doEtch)
                if(glow)
                    drawGlow(painter, r, WIDGET_CHECKBOX);
                else
                    drawEtch(painter, r, widget, WIDGET_CHECKBOX,
                             opts.crButton && EFFECT_SHADOW==opts.buttonEffect ? !sunken : false);

            drawBorder(painter, rect, option, ROUNDED_ALL, use, WIDGET_CHECKBOX);

            if(state&State_On)
            {
                QPixmap *pix(getPixmap(checkRadioCol(option), PIX_CHECK, 1.0));

                painter->drawPixmap(r.center().x()-(pix->width()/2), r.center().y()-(pix->height()/2),
                                    *pix);
            }
            else if (state&State_NoChange)    // tri-state
            {
                int x(r.center().x()), y(r.center().y());

                painter->setPen(checkRadioCol(option));
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
            bool        menu(state&QTC_STATE_MENU),
                        sunken(!menu && (state&State_Sunken)),
                        mo(!sunken && state&State_MouseOver && state&State_Enabled),
                        doEtch(!menu
                               && r.width()>=QTC_RADIO_SIZE+2 && r.height()>=QTC_RADIO_SIZE+2
                               && QTC_DO_EFFECT),
                        glow(doEtch && MO_GLOW==opts.coloredMouseOver && mo),
                        coloredMo(MO_NONE!=opts.coloredMouseOver && !glow && mo && !sunken);
            QRect       rect(doEtch ? r.adjusted(1, 1, -1, -1) : r);
            int         x(rect.x()), y(rect.y());
            QPolygon    clipRegion;
            EWidget     wid=opts.crButton ? WIDGET_STD_BUTTON : WIDGET_TROUGH;
            EAppearance app=opts.crButton ? opts.appearance : APPEARANCE_INVERTED;
            bool        drawSunken=opts.crButton ? sunken : false,
                        lightBorder=QTC_DRAW_LIGHT_BORDER(drawSunken, wid, app),
                        draw3d=!lightBorder &&
                               (QTC_DRAW_3D_BORDER(drawSunken, app) || QTC_DRAW_3D_FULL_BORDER(drawSunken, app)),
                        drawLight=opts.crButton && !drawSunken && (lightBorder || draw3d),
                        doneShadow=false;

            clipRegion.setPoints(8,  x+1,  y+8,   x+1,  y+4,   x+4, y+1,    x+8, y+1,
                                     x+12, y+4,   x+12, y+8,   x+8, y+12,   x+4, y+12);

            const QColor *bc(sunken ? 0L : borderColors(option, 0L)),
                         *btn(buttonColors(option)),
                         *use(bc ? bc : btn);
            const QColor &bgnd(opts.crButton
                                ? menu ? btn[ORIGINAL_SHADE] : getFill(option, btn, true)
                                : state&State_Enabled && !sunken
                                    ? MO_NONE==opts.coloredMouseOver && !opts.crHighlight && mo
                                        ? use[QTC_CR_MO_FILL]
                                        : palette.base().color()
                                    : palette.background().color());

            painter->save();

            if(doEtch && !glow && opts.crButton && !drawSunken && EFFECT_SHADOW==opts.buttonEffect)
            {
                QColor col(Qt::black);

                col.setAlphaF(QTC_ETCH_RADIO_TOP_ALPHA);
                doneShadow=true;
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setBrush(Qt::NoBrush);
                painter->setPen(col);
                painter->drawArc(QRectF(r.x()+1.5, r.y()+1.5, QTC_RADIO_SIZE, QTC_RADIO_SIZE), 0, 360*16);
                painter->setRenderHint(QPainter::Antialiasing, false);
            }

            painter->setClipRegion(QRegion(clipRegion));
            if(IS_FLAT(opts.appearance))
                painter->fillRect(rect, bgnd);
            else
                drawBevelGradient(bgnd, painter, rect.adjusted(1, 1, -1, -1), true, drawSunken, MODIFY_AGUA(app), wid);
            if(coloredMo)
            {
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setBrush(Qt::NoBrush);
                painter->setPen(use[QTC_CR_MO_FILL]);
                painter->drawArc(QRectF(x+1, y+1, QTC_RADIO_SIZE-2, QTC_RADIO_SIZE-2), 0, 360*16);
                painter->drawArc(QRectF(x+2, y+2, QTC_RADIO_SIZE-4, QTC_RADIO_SIZE-4), 0, 360*16);
//                 painter->drawArc(QRectF(x+3, y+3, QTC_RADIO_SIZE-6, QTC_RADIO_SIZE-6), 0, 360*16);
                painter->setRenderHint(QPainter::Antialiasing, false);
            }

            painter->setClipping(false);

            if(!doneShadow && doEtch && (glow || EFFECT_ETCH==opts.buttonEffect || sunken))
            {
                QColor topCol(glow ? itsMouseOverCols[QTC_GLOW_MO] : Qt::black);

                if(!glow)
                    topCol.setAlphaF(QTC_ETCH_RADIO_TOP_ALPHA);

                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setBrush(Qt::NoBrush);
                painter->setPen(topCol);
                painter->drawArc(QRectF(r.x()+0.5, r.y()+0.5, QTC_RADIO_SIZE+1, QTC_RADIO_SIZE+1), 45*16, 180*16);
                if(!glow)
                    painter->setPen(getLowerEtchCol(widget));
                painter->drawArc(QRectF(r.x()+0.5, r.y()+0.5, QTC_RADIO_SIZE+1, QTC_RADIO_SIZE+1), 225*16, 180*16);
                painter->setRenderHint(QPainter::Antialiasing, false);
            }

            painter->drawPixmap(x, y, *getPixmap(use[QT_BORDER(state&State_Enabled)], PIX_RADIO_BORDER, 0.8));

            if(state&State_On)
                painter->drawPixmap(x, y, *getPixmap(checkRadioCol(option), PIX_RADIO_ON, 1.0));
            if(!coloredMo && (!opts.crButton || drawLight))
                painter->drawPixmap(x, y, *getPixmap(btn[drawLight ? QTC_LIGHT_BORDER(app)
                                                                   : (state&State_MouseOver ? 3 : 4)],
                                                     lightBorder ? PIX_RADIO_INNER : PIX_RADIO_LIGHT));
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
                if (!(focusFrame->state&State_KeyboardFocusChange) ||
                    (widget && widget->inherits("QComboBoxListView")))
                    return;

                QRect r2(r);

                if(widget && (::qobject_cast<const QCheckBox *>(widget) || ::qobject_cast<const QRadioButton *>(widget)) &&
                   ((QAbstractButton *)widget)->text().isEmpty() &&
                   r.height()<=widget->rect().height()-2 && r.width()<=widget->rect().width()-2 &&
                   r.x()>=1 && r.y()>=1)
                {
                    int adjust=qMin(qMin(abs(widget->rect().x()-r.x()), 2), abs(widget->rect().y()-r.y()));
                    r2.adjust(-adjust, -adjust, adjust, adjust);
                }

                if(widget && ::qobject_cast<const QGroupBox *>(widget))
                   r2.adjust(0, 2, 0, 0);

                if(FOCUS_STANDARD==opts.focus)
                {
                    QStyleOptionFocusRect opt(*focusFrame);

                    opt.rect=r2;
                    QTC_BASE_STYLE::drawPrimitive(element, &opt, painter, widget);
                }
                else
                {
                    //Figuring out in what beast we are painting...
                    bool view((widget && ((dynamic_cast<const QAbstractScrollArea*>(widget)) ||
                                        widget->inherits("Q3ScrollView"))) ||
                                         (widget && widget->parent() &&
                                            ((dynamic_cast<const QAbstractScrollArea*>(widget->parent())) ||
                                              widget->parent()->inherits("Q3ScrollView"))));
                    painter->save();
                    QColor c(view && state&State_Selected
                                  ? palette.highlightedText().color()
                                  : itsFocusCols[QT_FOCUS(state&State_Selected)]);

                    if(FOCUS_LINE==opts.focus)
                        if(!(state&State_Horizontal) && widget && qobject_cast<const QTabBar *>(widget))
                            drawFadedLine(painter, QRect(r2.x()+r2.width()-1, r2.y(), 1, r2.height()),
                                          c, true, true, false);
                        else
                            drawFadedLine(painter, QRect(r2.x(), r2.y()+r2.height()-(view ? 3 : 1), r2.width(), 1),
                                          c, true, true, true);
                    else
                    {
                        painter->setPen(c);
                        if(FOCUS_FILLED==opts.focus)
                        {
                            c.setAlphaF(QTC_FOCUS_ALPHA);
                            painter->setBrush(c);
                        }
                        if(QTC_ROUNDED)
                        {
                            painter->setRenderHint(QPainter::Antialiasing, true);
                            painter->drawPath(buildPath(r2, WIDGET_SELECTION, ROUNDED_ALL,
                                                        getRadius(&opts, r2.width(), r2.height(), WIDGET_OTHER,
                                                                  QTC_FULL_FOCUS ? RADIUS_EXTERNAL : RADIUS_SELECTION)));
                        }
                        else
                            drawRect(painter, r2);
                    }
                    painter->restore();
                }
            }
            break;
        case PE_PanelButtonBevel:
        case PE_PanelButtonCommand:
        {
            bool doEtch(QTC_DO_EFFECT);

            // This fixes the "Sign in" button at mail.lycos.co.uk
            // ...basically if KHTML gices us a fully transparent background colour, then
            // dont paint the button.
            if(0==option->palette.button().color().alpha())
            {
                if(state&State_MouseOver && state&State_Enabled && MO_GLOW==opts.coloredMouseOver && doEtch)
                    drawGlow(painter, r, WIDGET_STD_BUTTON);
                return;
            }

            const QColor *use(buttonColors(option));
            bool         isDefault(false),
                         isFlat(false),
                         isKWin(state&QtC_StateKWin),
                         isDown(state&State_Sunken || state&State_On),
                         isOnListView(!isKWin && widget && qobject_cast<const QAbstractItemView *>(widget));
            QStyleOption opt(*option);

            if(PE_PanelButtonBevel==element)
                opt.state|=State_Enabled;

            if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton*>(option))
            {
                isDefault = (button->features & QStyleOptionButton::DefaultButton) && (button->state&State_Enabled);
                isFlat = (button->features & QStyleOptionButton::Flat);
            }

            if(!(opt.state&State_Enabled))
                opt.state&=~State_MouseOver;

            isDefault=isDefault || (doEtch && QTC_FULL_FOCUS && MO_GLOW==opts.coloredMouseOver &&
                                    opt.state&State_HasFocus && opt.state&State_Enabled);
            if(isFlat && !isDown && !(opt.state&State_MouseOver))
                return;

            painter->save();

            if(isOnListView)
                opt.state|=State_Horizontal|State_Raised;

            if(isDefault && state&State_Enabled && IND_TINT==opts.defBtnIndicator)
                use=itsDefBtnCols;

            if(isKWin)
                opt.state|=QTC_STATE_KWIN_BUTTON;

            // This section fixes some drawng issues with krunner's buttons on nvidia
//             painter->setRenderHint(QPainter::Antialiasing, true);
//             painter->fillRect(doEtch ? r.adjusted(2, 2, -2, -2) : r.adjusted(1, 1, -1, -1), palette.background().color());
//             painter->setRenderHint(QPainter::Antialiasing, false);

            bool coloredDef=isDefault && state&State_Enabled && IND_COLORED==opts.defBtnIndicator;

            drawLightBevel(painter, r, &opt, widget, ROUNDED_ALL,
                           coloredDef ? itsDefBtnCols[QTC_MO_DEF_BTN]
                                      : getFill(&opt, use, false,
                                                isDefault && state&State_Enabled && IND_DARKEN==opts.defBtnIndicator),
                           coloredDef ? itsDefBtnCols : use,
                           true, isKWin
                                    ? WIDGET_MDI_WINDOW_BUTTON
                                    : isOnListView
                                        ? WIDGET_NO_ETCH_BTN
                                        : isDefault && state&State_Enabled
                                            ? WIDGET_DEF_BUTTON
                                            : state&QTC_STATE_TBAR_BUTTON
                                                ? WIDGET_TOOLBAR_BUTTON
                                                : WIDGET_STD_BUTTON);

            if (isDefault && state&State_Enabled)
                switch(opts.defBtnIndicator)
                {
                    case IND_CORNER:
                    {
                        QPainterPath path;
                        int          offset(isDown ? 5 : 4),
                                     etchOffset(doEtch ? 1 : 0);
                        double       xd(r.x()+0.5),
                                     yd(r.y()+0.5);
                        const QColor *cols(itsFocusCols ? itsFocusCols : itsHighlightCols);

                        path.moveTo(xd+offset+etchOffset, yd+offset+etchOffset);
                        path.lineTo(xd+offset+6+etchOffset, yd+offset+etchOffset);
                        path.lineTo(xd+offset+etchOffset, yd+offset+6+etchOffset);
                        path.lineTo(xd+offset+etchOffset, yd+offset+etchOffset);
                        painter->setBrush(cols[isDown ? 0 : 4]);
                        painter->setPen(cols[isDown ? 0 : 4]);
                        painter->setRenderHint(QPainter::Antialiasing, true);
                        painter->drawPath(path);
                        painter->setRenderHint(QPainter::Antialiasing, false);
                        break;
                    }
                    case IND_COLORED:
                    {
                        int   offset=COLORED_BORDER_SIZE+(doEtch ? 1 : 0);
                        QRect r2(r.adjusted(offset, offset, -offset, -offset));

                        drawBevelGradient(getFill(&opt, use), painter, r2, true,
                                          state &(State_On | State_Sunken),
                                          opts.appearance, WIDGET_STD_BUTTON);
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
            ERound oldRound=opts.round;
            if(state&QtCStateKWinNotFull && state&QtC_StateKWin)
                opts.round=ROUND_NONE;

            const QColor *borderCols(opts.colorTitlebarOnly
                                        ? backgroundColors(palette.color(QPalette::Active, QPalette::Window))
                                        : theThemedApp==APP_KWIN
                                            ? buttonColors(option)
                                            : getMdiColors(option, state&State_Active));
            QStyleOption opt(*option);
            bool         roundKWinFull(QTC_FULLLY_ROUNDED &&
                                       (APP_KWIN==theThemedApp || state&QtC_StateKWin));

            opt.state=State_Horizontal|State_Enabled|State_Raised;
            if(state&QtC_StateKWinHighlight)
                opt.state|=QtC_StateKWinHighlight;

            if(APP_KWIN!=theThemedApp && roundKWinFull) // Set clipping for preview in kcmshell...
            {
                int     x(r.x()), y(r.y()), w(r.width()), h(r.height());
                QRegion mask(x+5, y+0, w-10, h);

                mask += QRegion(x+0, y+5, 1, h-10);
                mask += QRegion(x+1, y+3, 1, h-6);
                mask += QRegion(x+2, y+2, 1, h-4);
                mask += QRegion(x+3, y+1, 2, h-2);

                mask += QRegion(x+w-1, y+5, 1, h-10);
                mask += QRegion(x+w-2, y+3, 1, h-6);
                mask += QRegion(x+w-3, y+2, 1, h-4);
                mask += QRegion(x+w-5, y+1, 2, h-2);
                painter->setClipRegion(mask);
            }

            drawBorder(painter, r, &opt, ROUNDED_BOTTOM, borderCols, WIDGET_MDI_WINDOW, opts.titlebarBorder ? BORDER_RAISED : BORDER_FLAT);
            opts.round=oldRound;

            if(roundKWinFull)
            {
                bool   kwinHighlight(state&QtC_StateKWinHighlight);
                QColor col(opts.colorTitlebarOnly
                            ? backgroundColors(option)[QT_STD_BORDER]
                            : kwinHighlight ? itsFocusCols[0] : buttonColors(option)[QT_STD_BORDER]);

                painter->setPen(col);

                if(kwinHighlight || (state&QtC_StateKWinShadows))
                {
                    painter->drawPoint(r.x()+3, r.y()+r.height()-2);
                    painter->drawPoint(r.x()+1, r.y()+r.height()-4);
                    painter->drawPoint(r.x()+r.width()-4, r.y()+r.height()-2);
                    painter->drawPoint(r.x()+r.width()-2, r.y()+r.height()-4);
                    col.setAlphaF(0.5);
                    painter->setPen(col);
                    painter->drawPoint(r.x()+2, r.y()+r.height()-3);
                    painter->drawPoint(r.x()+4, r.y()+r.height()-2);
                    painter->drawPoint(r.x()+1, r.y()+r.height()-5);
                    painter->drawPoint(r.x()+r.width()-3, r.y()+r.height()-3);
                    painter->drawPoint(r.x()+r.width()-5, r.y()+r.height()-2);
                    painter->drawPoint(r.x()+r.width()-2, r.y()+r.height()-5);
                }
                else
                {
                    painter->drawPoint(r.x()+2, r.y()+r.height()-3);
                    painter->drawPoint(r.x()+r.width()-3, r.y()+r.height()-3);
                    painter->drawLine(r.x()+1, r.y()+r.height()-5, r.x()+1, r.y()+r.height()-4);
                    painter->drawLine(r.x()+3, r.y()+r.height()-2, r.x()+4, r.y()+r.height()-2);
                    painter->drawLine(r.x()+r.width()-2, r.y()+r.height()-5, r.x()+r.width()-2, r.y()+r.height()-4);
                    painter->drawLine(r.x()+r.width()-4, r.y()+r.height()-2, r.x()+r.width()-5, r.y()+r.height()-2);
                }
            }
            break;
        }
        case PE_FrameTabWidget:
        {
            int round(ROUNDED_ALL);

            painter->save();

            if(const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option))
                if((opts.round || (!IS_FLAT(opts.bgndAppearance && 0==opts.tabBgnd))) &&
                    widget && ::qobject_cast<const QTabWidget *>(widget))
                {
                    struct QtcTabWidget : public QTabWidget
                    {
                        bool  tabsVisible()    const { return tabBar() && tabBar()->isVisible(); }
                        QRect currentTabRect() const { return tabBar()->tabRect(tabBar()->currentIndex()); }
                    };

                    const QTabWidget *tw((const QTabWidget *)widget);

                    if(tw->count()>0 && ((const QtcTabWidget *)widget)->tabsVisible())
                    {
                        if(!IS_FLAT(opts.bgndAppearance) && 0==opts.tabBgnd)
                        {
                            QRect tabRect(((const QtcTabWidget *)widget)->currentTabRect());

                            switch(tw->tabPosition())
                            {
                                case QTabWidget::South:
                                    tabRect=QRect(tabRect.x()+2, r.y()+r.height()-2, tabRect.width()-4, 4);
                                    break;
                                case QTabWidget::North:
                                    tabRect.adjust(2, 0, -2, 2);
                                    break;
                                case QTabWidget::West:
                                    tabRect.adjust(0, 2, 2, -2);
                                    break;
                                case QTabWidget::East:
                                    tabRect=QRect(r.x()+r.width()-2, tabRect.y()+2, 4, tabRect.height()-4);
                                    break;
                            }

                            painter->setClipRegion(QRegion(r).subtract(tabRect), Qt::IntersectClip);
                        }

                        if(0==tw->currentIndex())
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
                }

            QStyleOption opt(*option);
            const QColor *use=backgroundColors(option);

            opt.state|=State_Enabled;
            if(0!=opts.tabBgnd)
                painter->fillPath(buildPath(r, WIDGET_TAB_FRAME, ROUNDED_ALL,
                                            getRadius(&opts, r.width(), r.height(), WIDGET_TAB_FRAME, RADIUS_EXTERNAL)),
                                            shade(use[ORIGINAL_SHADE], QTC_TO_FACTOR(opts.tabBgnd)));
            drawBorder(painter, r, &opt, round, use, WIDGET_TAB_FRAME,
                       opts.borderTab ? BORDER_LIGHT : BORDER_RAISED, false);
            painter->restore();
            break;
        }
#if QT_VERSION >= 0x040400
        case PE_PanelItemViewItem:
        {
            const QStyleOptionViewItemV4 *v4Opt = qstyleoption_cast<const QStyleOptionViewItemV4*>(option);
            const QAbstractItemView      *view = qobject_cast<const QAbstractItemView *>(widget);
            bool                         hover = state&State_MouseOver && state&State_Enabled && (!view ||
                                                 QAbstractItemView::NoSelection!=view->selectionMode()),
                                         hasCustomBackground = v4Opt->backgroundBrush.style() != Qt::NoBrush &&
                                                               !(option->state & State_Selected),
                                         hasSolidBackground = !hasCustomBackground || Qt::SolidPattern==v4Opt->backgroundBrush.style();

            if (!hover && !(state & State_Selected) && !hasCustomBackground &&
                !(v4Opt->features & QStyleOptionViewItemV2::Alternate))
                break;

            QPalette::ColorGroup cg(state&State_Enabled
                                        ? state&State_Active
                                            ? QPalette::Normal
                                            : QPalette::Inactive
                                        : QPalette::Disabled);

            if (v4Opt && (v4Opt->features & QStyleOptionViewItemV2::Alternate))
                painter->fillRect(option->rect, option->palette.brush(cg, QPalette::AlternateBase));

            if (!hover && !(state&State_Selected) && !hasCustomBackground)
                break;

            if(hasCustomBackground)
            {
                const QPointF prevOrigin(painter->brushOrigin());

                painter->setBrushOrigin(r.topLeft());
                painter->fillRect(r, v4Opt->backgroundBrush);
                painter->setBrushOrigin(prevOrigin);
            }

            if(state&State_Selected || hover)
            {
                QColor color(hasCustomBackground && hasSolidBackground
                                ? v4Opt->backgroundBrush.color()
                                : palette.color(cg, QPalette::Highlight));

                if (hover && !hasCustomBackground)
                {
                    if (!(state & State_Selected))
                        color.setAlphaF(APP_PLASMA==theThemedApp && !widget ? 0.5 : 0.20);
                    else
                        color = color.lighter(110);
                }

                QPixmap pix(QSize(24, r.height()));
                QString key;

                key.sprintf("qtc-sel-%x-%x", pix.height(), color.rgba());
                if(!usePixmapCache || !QPixmapCache::find(key, pix))
                {
                    pix.fill(Qt::transparent);

                    QPainter pixPainter(&pix);
                    QRect    border(0, 0, pix.width(), pix.height());
                    double   radius(getRadius(&opts, r.width(), r.height(), WIDGET_OTHER, RADIUS_SELECTION));

                    pixPainter.setRenderHint(QPainter::Antialiasing, true);
                    drawBevelGradient(color, &pixPainter, border,
                                      buildPath(border, WIDGET_OTHER, ROUNDED_ALL, radius), true, false,
                                      opts.selectionAppearance, WIDGET_SELECTION, !usePixmapCache);
                    pixPainter.setBrush(Qt::NoBrush);
                    pixPainter.setPen(color);
                    pixPainter.drawPath(buildPath(border, WIDGET_SELECTION, ROUNDED_ALL, radius));
                    pixPainter.end();

                    if(usePixmapCache)
                        QPixmapCache::insert(key, pix);
                }

                bool roundedLeft  = false,
                     roundedRight = false;

                if (v4Opt)
                {
                    roundedLeft  = (QStyleOptionViewItemV4::Beginning==v4Opt->viewItemPosition);
                    roundedRight = (QStyleOptionViewItemV4::End==v4Opt->viewItemPosition);
                    if (QStyleOptionViewItemV4::OnlyOne==v4Opt->viewItemPosition ||
                        QStyleOptionViewItemV4::Invalid==v4Opt->viewItemPosition ||
                        (view && view->selectionBehavior() != QAbstractItemView::SelectRows))
                    {
                        roundedLeft=roundedRight=true;
                    }
                }

                int size(roundedLeft && roundedRight ? qMin(8, r.width()/2) : 8);

                if (!reverse ? roundedLeft : roundedRight)
                {
                    painter->drawPixmap(r.topLeft(), pix.copy(0, 0, size, r.height()));
                    r.adjust(size, 0, 0, 0);
                }
                if (!reverse ? roundedRight : roundedLeft)
                {
                    painter->drawPixmap(r.right() - size + 1, r.top(), pix.copy(24-size, 0, size, r.height()));
                    r.adjust(0, 0, -size, 0);
                }
                if (r.isValid())
                    painter->drawTiledPixmap(r, pix.copy(7, 0, 8, r.height()));
            }
            break;
        }
#endif
        default:
            QTC_BASE_STYLE::drawPrimitive(element, option, painter, widget);
            break;
    }
}

void QtCurveStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
                               const QWidget *widget) const
{
    QRect               r(option->rect);
    const QFlags<State> &state(option->state);
    const QPalette      &palette(option->palette);
    bool                reverse(Qt::RightToLeft==option->direction);

    switch(element)
    {
        case CE_QtC_KCapacityBar:
            if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option))
            {
                QStyleOptionProgressBar mod(*bar);

                if(mod.rect.height()>16 && widget->parentWidget() &&
                   (qobject_cast<const QStatusBar *>(widget->parentWidget()) ||
                    widget->parentWidget()->inherits("DolphinStatusBar")))
                {
                    int m=(mod.rect.height()-16)/2;
                    mod.rect.adjust(0, m, 0, -m);
                }
                drawControl(CE_ProgressBarGroove, &mod, painter, widget);
                if(QTC_DO_EFFECT)
                    mod.rect.adjust(1, 1, -1, -1);
                drawControl(CE_ProgressBarContents, &mod, painter, widget);
                drawControl(CE_ProgressBarLabel, &mod, painter, widget);
            }
            break;
        case CE_ToolBoxTabShape:
        {
            const QStyleOptionToolBox *tb = qstyleoption_cast<const QStyleOptionToolBox *>(option);
            if(!(tb && widget))
                break;

//             const QStyleOptionToolBoxV2 *v2 = qstyleoption_cast<const QStyleOptionToolBoxV2 *>(option);
//
//             if (v2 && QStyleOptionToolBoxV2::Beginning==v2->position)
//                 break;

            const QColor *use = backgroundColors(widget->palette().color(QPalette::Window));
            QPainterPath path;
            int          y = r.height()*15/100;

            painter->save();
            if (reverse)
            {
                path.moveTo(r.left()+52, r.top());
                path.cubicTo(QPointF(r.left()+50-8, r.top()), QPointF(r.left()+50-10, r.top()+y),
                             QPointF(r.left()+50-10, r.top()+y));
                path.lineTo(r.left()+18+9, r.bottom()-y);
                path.cubicTo(QPointF(r.left()+18+9, r.bottom()-y), QPointF(r.left()+19+6, r.bottom()-1-0.3),
                             QPointF(r.left()+19, r.bottom()-1-0.3));
            }
            else
            {
                path.moveTo(r.right()-52, r.top());
                path.cubicTo(QPointF(r.right()-50+8, r.top()), QPointF(r.right()-50+10, r.top()+y),
                             QPointF(r.right()-50+10, r.top()+y));
                path.lineTo(r.right()-18-9, r.bottom()-y);
                path.cubicTo(QPointF(r.right()-18-9, r.bottom()-y), QPointF(r.right()-19-6, r.bottom()-1-0.3),
                             QPointF(r.right()-19, r.bottom()-1-0.3));
            }

            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->translate(0, 1);
            painter->setPen(use[0]);
            painter->drawPath(path);
            painter->translate(0, -1);
            painter->setPen(use[4]);
            painter->drawPath(path);
            painter->setRenderHint(QPainter::Antialiasing, false);
            if (reverse)
            {
                painter->drawLine(r.left()+50-1, r.top(), r.right(), r.top());
                painter->drawLine(r.left()+20, r.bottom()-2, r.left(), r.bottom()-2);
                painter->setPen(use[0]);
                painter->drawLine(r.left()+50, r.top()+1, r.right(), r.top()+1);
                painter->drawLine(r.left()+20, r.bottom()-1, r.left(), r.bottom()-1);
            }
            else
            {
                painter->drawLine(r.left(), r.top(), r.right()-50+1, r.top());
                painter->drawLine(r.right()-20, r.bottom()-2, r.right(), r.bottom()-2);
                painter->setPen(use[0]);
                painter->drawLine(r.left(), r.top()+1, r.right()-50, r.top()+1);
                painter->drawLine(r.right()-20, r.bottom()-1, r.right(), r.bottom()-1);
            }
            painter->restore();
            break;
        }
        case CE_MenuScroller:
            painter->fillRect(r, USE_LIGHTER_POPUP_MENU ? itsLighterPopupMenuBgndCol
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
            if(r.width()>0 && r.height()>0)
            {
                painter->save();
                QColor c(itsHighlightCols[ORIGINAL_SHADE]);

                painter->setClipRegion(r);
                painter->setPen(c);
                c.setAlpha(50);
                painter->setBrush(c);
//                 double radius=QTC_ROUNDED ? getRadius(&opts, r.width(), r.height(), WIDGET_RUBBER_BAND, RADIUS_SELECTION) : 0.0;
//                 if(radius>0.0 && r.width()>(2*radius) && r.height()>(2*radius))
//                 {
//                     painter->setRenderHint(QPainter::Antialiasing, true);
//                     painter->drawPath(buildPath(r, WIDGET_RUBBER_BAND, ROUNDED_ALL, radius));
//                 }
//                 else
                    drawRect(painter, r);
                painter->restore();
            }
            break;
        }
        case CE_Splitter:
        {
            const QColor *use(buttonColors(option));
            const QColor *border(borderColors(option, use));

            painter->save();
            if(/*IS_FLAT(opts.bgndAppearance) || */state&State_MouseOver && state&State_Enabled)
            {
                QColor color(palette.color(QPalette::Active, QPalette::Window));

//                 if(0!=opts.tabBgnd && inStackWidget(widget))
//                     color=shade(color, QTC_TO_FACTOR(opts.tabBgnd));
                painter->fillRect(r, QColor(state&State_MouseOver && state&State_Enabled
                                                ? shade(color, QTC_TO_FACTOR(opts.highlightFactor))
                                                : color));
            }

            switch(opts.splitters)
            {
                case LINE_NONE:
                    break;
//                 case LINE_1DOT:
//                     painter->drawPixmap(r.x()+((r.width()-5)/2), r.y()+((r.height()-5)/2), *getPixmap(border[QT_STD_BORDER], PIX_DOT, 1.0));
//                     break;
                default:
                case LINE_DOTS:
                    drawDots(painter, r, state&State_Horizontal, NUM_SPLITTER_DASHES, 1, border, 0, 5);
                    break;
                case LINE_FLAT:
                case LINE_SUNKEN:
                case LINE_DASHES:
                    drawLines(painter, r, state&State_Horizontal, NUM_SPLITTER_DASHES, 3, border, 0, 3, opts.splitters);
            }
            painter->restore();
            break;
        }
        case CE_SizeGrip:
        {
            QPolygon   triangle(3);
            Qt::Corner corner;
            int        size=SIZE_GRIP_SIZE-2;

            if (const QStyleOptionSizeGrip *sgrp = qstyleoption_cast<const QStyleOptionSizeGrip *>(option))
                corner = sgrp->corner;
            else if (Qt::RightToLeft==option->direction)
                corner = Qt::BottomLeftCorner;
            else
                corner = Qt::BottomRightCorner;

            switch(corner)
            {
                case Qt::BottomLeftCorner:
                    triangle.putPoints(0, 3, 0,0, size,size, 0,size);
                    triangle.translate(r.x(), r.y()+(r.height()-(SIZE_GRIP_SIZE-1)));
                    break;
                case Qt::BottomRightCorner:
                    triangle.putPoints(0, 3, size,0, size,size, 0,size);
                    triangle.translate(r.x()+(r.width()-(SIZE_GRIP_SIZE-1)), r.y()+(r.height()-(SIZE_GRIP_SIZE-1)));
                    break;
                case Qt::TopRightCorner:
                    triangle.putPoints(0, 3, 0,0, size,0, size,size);
                    triangle.translate(r.x()+(r.width()-(SIZE_GRIP_SIZE-1)), r.y());
                    break;
                case Qt::TopLeftCorner:
                    triangle.putPoints(0, 3, 0,0, size,0, 0,size);
                    triangle.translate(r.x(), r.y());
            }
            painter->save();
            painter->setPen(itsBackgroundCols[2]);
            painter->setBrush(itsBackgroundCols[2]);
            painter->drawPolygon(triangle);
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
                        painter->drawLine(r.x(), r.y(), r.x(), r.y()+r.height()-1);
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
#if QT_VERSION >= 0x040300
                const QStyleOptionDockWidgetV2 *v2 = qstyleoption_cast<const QStyleOptionDockWidgetV2*>(dwOpt);
                bool                           verticalTitleBar(v2 == 0 ? false : v2->verticalTitleBar);
#endif
                // This section fixes the look of KOffice's dock widget titlebars...
                QRect fillRect(r);
                if(widget && widget->inherits("KoDockWidgetTitleBar"))
                    fillRect.adjust(-r.x(), -r.y(), r.x(), r.y());

                painter->save();
//     #if QT_VERSION >= 0x040300
//                 painter->fillRect(fillRect, palette.background().color().darker(105));
//     #else
//                 painter->fillRect(fillRect, palette.background().color().dark(105));
//     #endif
                painter->setRenderHint(QPainter::Antialiasing, true);
                QPen old(painter->pen());
#if QT_VERSION >= 0x040300
                if(verticalTitleBar)
                    drawFadedLine(painter, QRect(fillRect.x()+fillRect.width()/2, fillRect.y(), 1, fillRect.height()),
                                  itsBackgroundCols[QT_STD_BORDER], true, true, false);
                else
#endif
                    drawFadedLine(painter, QRect(fillRect.x(), fillRect.y()+fillRect.height()-2, fillRect.width(), 1),
                                  itsBackgroundCols[QT_STD_BORDER], true, true, true);
                painter->setRenderHint(QPainter::Antialiasing, false);
                painter->setPen(old);

                if (!dwOpt->title.isEmpty())
                {
#if QT_VERSION >= 0x040300
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
                    const int margin(4);
                    QRect titleRect(visualRect(dwOpt->direction, r, r.adjusted(margin, 0, -margin * 2 - 26, 0)));
#endif

                    drawItemText(painter, titleRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic, palette,
                                 dwOpt->state&State_Enabled,
                                 painter->fontMetrics().elidedText(dwOpt->title, Qt::ElideRight, titleRect.width(),
                                 QPalette::WindowText));
                }

                painter->restore();
            }
            break;
#if QT_VERSION >= 0x040300
        case CE_HeaderEmptyArea:
        {
            const QStyleOptionHeader *ho = qstyleoption_cast<const QStyleOptionHeader *>(option);
            bool                     horiz(ho ? Qt::Horizontal==ho->orientation : state&State_Horizontal);
            QStyleOption             opt(*option);
            const QColor             *use(opts.lvButton ? buttonColors(option) : backgroundColors(option));

            opt.state&=~State_MouseOver;
            painter->save();

            drawBevelGradient(getFill(&opt, use), painter, r, horiz,
                              false, opts.lvAppearance, WIDGET_LISTVIEW_HEADER);

            painter->setRenderHint(QPainter::Antialiasing, true);
            if(APPEARANCE_RAISED==opts.lvAppearance)
            {
                painter->setPen(use[4]);
                if(horiz)
                    drawAaLine(painter, r.x(), r.y()+r.height()-2, r.x()+r.width()-1, r.y()+r.height()-2);
                else
                    drawAaLine(painter, r.x()+r.width()-2, r.y(), r.x()+r.width()-2, r.y()+r.height()-1);
            }

            painter->setPen(use[QT_STD_BORDER]);
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
                const QColor *use(state&State_Enabled && itsSortedLvColors && QStyleOptionHeader::None!=ho->sortIndicator
                                    ? itsSortedLvColors
                                    : opts.lvButton ? buttonColors(option) : backgroundColors(option));

                painter->save();

                if(state & (State_Raised | State_Sunken))
                {
                    bool         sunken(state &(/*State_Down |*/ /*State_On | */State_Sunken)),
                                 q3Header(widget && widget->inherits("Q3Header"));
                    QStyleOption opt(*option);

                    opt.state&=~State_On;
                    if(q3Header && widget && widget->underMouse() && itsHoverWidget && r.contains(itsPos))
                        opt.state|=State_MouseOver;

                    if(-1==ho->section && !(state&State_Enabled) && widget && widget->isEnabled())
                        opt.state|=State_Enabled;

                    drawBevelGradient(getFill(&opt, use), painter, r,
                                        Qt::Horizontal==ho->orientation,
                                        sunken, opts.lvAppearance, WIDGET_LISTVIEW_HEADER);

                    painter->setRenderHint(QPainter::Antialiasing, true);
                    if(APPEARANCE_RAISED==opts.lvAppearance)
                    {
                        painter->setPen(use[4]);
                        if(Qt::Horizontal==ho->orientation)
                            drawAaLine(painter, r.x(), r.y()+r.height()-2, r.x()+r.width()-1, r.y()+r.height()-2);
                        else
                            drawAaLine(painter, r.x()+r.width()-2, r.y(), r.x()+r.width()-2, r.y()+r.height()-1);
                    }

                    if(Qt::Horizontal==ho->orientation)
                    {
                        painter->setPen(use[QT_STD_BORDER]);
                        drawAaLine(painter, r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                        if(opts.coloredMouseOver && state&State_MouseOver && state&State_Enabled)
                            drawHighlight(painter, QRect(r.x(), r.y()+r.height()-2, r.width(), 2), true, true);

                        if(q3Header ||
                           (QStyleOptionHeader::End!=ho->position && QStyleOptionHeader::OnlyOneSection!=ho->position))
                        {
                            drawFadedLine(painter, QRect(r.x()+r.width()-2, r.y()+5, 1, r.height()-10),
                                          use[QT_STD_BORDER], true, true, false);
                            drawFadedLine(painter, QRect(r.x()+r.width()-1, r.y()+5, 1, r.height()-10),
                                          use[0], true, true, false);
                        }
                    }
                    else
                    {
                        painter->setPen(use[QT_STD_BORDER]);
                        drawAaLine(painter, r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);

                        if(q3Header ||
                           (QStyleOptionHeader::End!=ho->position && QStyleOptionHeader::OnlyOneSection!=ho->position))
                        {
                            painter->setPen(use[QT_STD_BORDER]);
                            drawAaLine(painter, r.x()+5, r.y()+r.height()-2, r.x()+r.width()-6,
                                      r.y()+r.height()-2);
                            painter->setPen(use[0]);
                            drawAaLine(painter, r.x()+5, r.y()+r.height()-1, r.x()+r.width()-6,
                                      r.y()+r.height()-1);
                        }
                        if(opts.coloredMouseOver && state&State_MouseOver && state&State_Enabled)
                            drawHighlight(painter, QRect(r.x(), r.y()+r.height()-3, r.width(), 2), true, true);
                    }
                    painter->setRenderHint(QPainter::Antialiasing, false);
                }
                else
                    painter->fillRect(r, getFill(option, use));
                painter->restore();
            }
            break;
        case CE_HeaderLabel:
            if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option))
            {
//                 if (state & (State_On | State_Sunken))
//                     r.translate(pixelMetric(PM_ButtonShiftHorizontal, option, widget),
//                                 pixelMetric(PM_ButtonShiftVertical, option, widget));

                if (!header->icon.isNull())
                {
                    QPixmap pixmap(getIconPixmap(header->icon, pixelMetric(PM_SmallIconSize), header->state));
                    int     pixw(pixmap.width());
                    QRect   aligned(alignedRect(header->direction, QFlag(header->iconAlignment), pixmap.size(), r)),
                            inter(aligned.intersected(r));

                    painter->drawPixmap(inter.x(), inter.y(), pixmap, inter.x() - aligned.x(), inter.y() - aligned.y(), inter.width(), inter.height());

                    if (header->direction == Qt::LeftToRight)
                        r.setLeft(r.left() + pixw + 2);
                    else
                        r.setRight(r.right() - pixw - 2);
                }
                drawItemText(painter, r, header->textAlignment, palette, state&State_Enabled, header->text, QPalette::ButtonText);
            }
            break;
        case CE_ProgressBarGroove:
        {
            bool   doEtch(QTC_DO_EFFECT),
                   horiz(true);
            QColor col;

            if (const QStyleOptionProgressBarV2 *bar = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(option))
                horiz = Qt::Horizontal==bar->orientation;

            painter->save();

            if(doEtch)
                r.adjust(1, 1, -1, -1);

            switch(opts.progressGrooveColor)
            {
                default:
                case ECOLOR_BASE:
                    col=palette.base().color();
                    break;
                case ECOLOR_BACKGROUND:
                    col=palette.background().color();
                    break;
                case ECOLOR_DARK:
                    col=itsBackgroundCols[2];
            }

            drawBevelGradient(col, painter, r,
                              buildPath(r, WIDGET_PBAR_TROUGH, ROUNDED_ALL,
                                        getRadius(&opts, r.width(), r.height(), WIDGET_PBAR_TROUGH, RADIUS_EXTERNAL)),
                              horiz, false, opts.progressGrooveAppearance, WIDGET_PBAR_TROUGH);

            if(doEtch)
                drawEtch(painter, r.adjusted(-1, -1, 1, 1), widget, WIDGET_PBAR_TROUGH);

            drawBorder(painter, r, option, ROUNDED_ALL, backgroundColors(option), WIDGET_PBAR_TROUGH,
                       IS_FLAT(opts.progressGrooveAppearance) && ECOLOR_DARK!=opts.progressGrooveColor ? BORDER_SUNKEN : BORDER_FLAT);
            painter->restore();
            break;
        }
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

                if (!indeterminate && -1==bar->progress)
                    break;

                bool reverse = (!vertical && (bar->direction == Qt::RightToLeft)) || vertical;

                if (inverted)
                    reverse = !reverse;

                painter->save();

                if(indeterminate) //Busy indicator
                {
                    int chunkSize(PROGRESS_CHUNK_WIDTH*3.4),
                        measure(vertical ? r.height() : r.width());

                    if(chunkSize>(measure/2))
                        chunkSize=measure/2;

                    int          step(itsAnimateStep % ((measure-chunkSize) * 2));
                    QStyleOption opt(*option);

                    if (step > (measure-chunkSize))
                        step = 2 * (measure-chunkSize) - step;

                    opt.state|=State_Raised|State_Horizontal;
                    drawProgress(painter, vertical
                                                ? QRect(r.x(), r.y()+step, r.width(), chunkSize)
                                                : QRect(r.x()+step, r.y(), chunkSize, r.height()),
                                 option, ROUNDED_ALL, vertical);
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

                            if(inverted)
                                drawProgress(painter, QRect(cr.x(), cr.y(), cr.width(), height), option,
                                            height==cr.height() ?  ROUNDED_NONE : ROUNDED_BOTTOM, true);
                            else
                                drawProgress(painter, QRect(cr.x(), cr.y()+(cr.height()-height), cr.width(), height),
                                             option, height==cr.height() ? ROUNDED_NONE : ROUNDED_TOP, true);
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
                bool   flip((!vertical && (((Qt::RightToLeft==bar->direction) && !inverted) || ((Qt::LeftToRight==bar->direction) && inverted))) ||
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
                bool    down(state&(State_On|State_Sunken)),
                        active(state&State_Enabled && (down || (state&State_Selected && opts.menubarMouseOver)));
                uint    alignment(Qt::AlignCenter|Qt::TextShowMnemonic|Qt::TextDontClip|Qt::TextSingleLine);
                QPixmap pix(getIconPixmap(mbi->icon, pixelMetric(PM_SmallIconSize), mbi->state));

                if (!styleHint(SH_UnderlineShortcut, mbi, widget))
                    alignment|=Qt::TextHideMnemonic;

                painter->save();

#ifdef QTC_XBAR_SUPPORT
                if(!widget || 0!=strcmp("QWidget", widget->metaObject()->className()))
#endif
                drawMenuOrToolBarBackground(painter, mbi->menuRect, option);

                if(active)
                    drawMenuItem(painter, r, option, true, down && opts.roundMbTopOnly ? ROUNDED_TOP : ROUNDED_ALL,
                                 opts.useHighlightForMenu && (opts.colorMenubarMouseOver || down)
                                    ? itsHighlightCols : itsBackgroundCols);

                if (!pix.isNull())
                    drawItemPixmap(painter, mbi->rect, alignment, pix);
                else
                {
                    const QColor &col=state&State_Enabled
                                        ? ((opts.colorMenubarMouseOver && active) || (!opts.colorMenubarMouseOver && down))
                                            ? opts.customMenuTextColor
                                                ? opts.customMenuSelTextColor
                                                : opts.useHighlightForMenu
                                                    ? palette.highlightedText().color()
                                                    : palette.foreground().color()
                                            : palette.foreground().color()
                                         : palette.foreground().color();

// #ifdef QTC_XBAR_SUPPORT
//                     if(palette.foreground().color()==col && palette.foreground().color()!=QApplication::palette().foreground().color() &&
//                        palette.background().color()==QApplication::palette().background().color())
//                         col=QApplication::palette().foreground().color();
// #endif
                    painter->setPen(col);
                    painter->drawText(r, alignment, mbi->text);
                }
                painter->restore();
            }
            break;
        case CE_MenuItem:
            if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
            {
                bool comboMenu(qobject_cast<const QComboBox*>(widget)),
                     reverse(Qt::RightToLeft==menuItem->direction);
                int  checkcol(qMax(menuItem->maxIconWidth, 20)),
                     stripeWidth(qMax(checkcol, constMenuPixmapWidth)-2);

                painter->save();

                if (QStyleOptionMenuItem::Separator==menuItem->menuItemType)
                {
                    bool isMenu(!widget || qobject_cast<const QMenu*>(widget)),
                         doStripe(isMenu && opts.menuStripe && !comboMenu);

/*
                    if(isMenu)
                        painter->fillRect(menuItem->rect, USE_LIGHTER_POPUP_MENU ? itsLighterPopupMenuBgndCol
                                                                                 : itsBackgroundCols[ORIGINAL_SHADE]);
*/

                    if(doStripe)
                        drawBevelGradient(menuStripeCol(),
                                          painter, QRect(reverse ? r.right()-stripeWidth : r.x(), r.y(),
                                                         stripeWidth, r.height()), false,
                                          false, opts.menuStripeAppearance, WIDGET_OTHER);

                    QRect miRect(menuItem->rect.left() + 3 +
                                    (!reverse && doStripe ? stripeWidth : 0),
                                    menuItem->rect.center().y(),
                                    menuItem->rect.width() - (7 + (doStripe ? stripeWidth : 0)),
                                    1);
                    drawFadedLine(painter, miRect, itsBackgroundCols[QTC_MENU_SEP_SHADE], true, true, true);

//                     if (!menuItem->text.isEmpty())
//                     {
//                         painter->setFont(menuItem->font);
//                         drawItemText(painter, menuItem->rect.adjusted(5, 0, -5, 0), Qt::AlignLeft | Qt::AlignVCenter,
//                                      palette, state&State_Enabled, menuItem->text, QPalette::Text);
//                     }

                    painter->restore();
                    break;
                }

                bool selected(state&State_Selected),
                     checkable(QStyleOptionMenuItem::NotCheckable!=menuItem->checkType),
                     checked(menuItem->checked),
                     enabled(state&State_Enabled);

                if(opts.menuStripe && !comboMenu)
                    drawBevelGradient(menuStripeCol(), painter,
                                        QRect(reverse ? r.right()-stripeWidth : r.x(), r.y(), stripeWidth,
                                            r.height()), false,
                                        false, opts.menuStripeAppearance, WIDGET_OTHER);

                if (selected && enabled)
                    drawMenuItem(painter,
                                 comboMenu && opts.gtkComboMenus
                                    ? r
                                    : r.adjusted(0, 0, -1, 0),
                                 option, false, ROUNDED_ALL,
                                 opts.useHighlightForMenu ? itsHighlightCols : itsBackgroundCols);

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
                    QRect checkRect(r.left() + 3, r.center().y() - 6, QTC_CHECK_SIZE, QTC_CHECK_SIZE);
                    checkRect = visualRect(menuItem->direction, menuItem->rect, checkRect);
                    if (checkable)
                    {
                        if ((menuItem->checkType & QStyleOptionMenuItem::Exclusive) && menuItem->icon.isNull())
                        {
                            QStyleOptionButton button;
                            button.rect = checkRect;
                            button.state = menuItem->state|QTC_STATE_MENU;
                            if (checked)
                                button.state |= State_On;
                            button.palette = palette;
                            drawPrimitive(PE_IndicatorRadioButton, &button, painter, widget);
                        }
                        else
                        {
                            if (menuItem->icon.isNull() || !opts.menuIcons)
                            {
                                QStyleOptionButton button;
                                button.rect = checkRect;
                                button.state = menuItem->state|QTC_STATE_MENU;
                                if (checked)
                                    button.state |= State_On;
                                button.palette = palette;
                                drawPrimitive(PE_IndicatorCheckBox, &button, painter, widget);
                            }
                            else if (checked)
                            {
                                int          iconSize(qMax(menuItem->maxIconWidth, 20));
                                QRect        sunkenRect(r.left() + 1, r.top() + (r.height() - iconSize) / 2,
                                                        iconSize, iconSize);
                                QStyleOption opt(*option);

                                sunkenRect = visualRect(menuItem->direction, menuItem->rect, sunkenRect);
                                opt.state = menuItem->state;
                                opt.state|=State_Raised|State_Horizontal;
                                if (checked)
                                    opt.state |= State_On;
                                drawLightBevel(painter, sunkenRect, &opt, widget, ROUNDED_ALL, getFill(&opt, itsButtonCols), itsButtonCols);
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

                if (opts.menuIcons && !menuItem->icon.isNull())
                {
                    QIcon::Mode mode(dis ? QIcon::Disabled : QIcon::Normal);

                    if (act && !dis)
                        mode = QIcon::Active;

                    QPixmap pixmap(getIconPixmap(menuItem->icon, pixelMetric(PM_SmallIconSize), mode,
                                   checked ? QIcon::On : QIcon::Off));

                    int     pixw(pixmap.width()),
                            pixh(pixmap.height());
                    QRect   pmr(0, 0, pixw, pixh);

                    pmr.moveCenter(vCheckRect.center());
                    painter->setPen(palette.text().color());
                    if (checkable && checked)
                        painter->drawPixmap(QPoint(pmr.left() + 1, pmr.top() + 1), pixmap);
                    else
                        painter->drawPixmap(pmr.topLeft(), pixmap);
                }

                painter->setPen(dis
                                    ? palette.text().color()
                                    : selected && opts.useHighlightForMenu
                                        ? palette.highlightedText().color()
                                        : palette.foreground().color());

                int x, y, w, h,
                    tab(menuItem->tabWidth);

                menuItem->rect.getRect(&x, &y, &w, &h);

                int     xm(windowsItemFrame + checkcol + windowsItemHMargin -2),
                        xpos(menuItem->rect.x() + xm);
                QRect   textRect(xpos, y + windowsItemVMargin, w - xm - windowsRightBorder - tab + 1, h - 2 * windowsItemVMargin),
                        vTextRect = visualRect(option->direction, menuItem->rect, textRect);
                QString s(menuItem->text);

                if (!s.isEmpty())                      // draw text
                {
                    int t(s.indexOf(QLatin1Char('\t'))),
                        textFlags(Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine);

                    if (!styleHint(SH_UnderlineShortcut, menuItem, widget))
                        textFlags |= Qt::TextHideMnemonic;
                    textFlags |= Qt::AlignLeft;

                    if (t >= 0)
                    {
                        QRect vShortcutRect(visualRect(option->direction, menuItem->rect,
                                                    QRect(textRect.topRight(), QPoint(menuItem->rect.right(), textRect.bottom()))));

                        painter->drawText(vShortcutRect, textFlags, s.mid(t + 1));
                        s = s.left(t);
                    }

                    QFont font(menuItem->font);

                    if (menuItem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                        font.setBold(true);

                    painter->setFont(font);
                    painter->drawText(vTextRect, textFlags, s.left(t));
                }

                // Arrow
                if (QStyleOptionMenuItem::SubMenu==menuItem->menuItemType) // draw sub menu arrow
                {
                    int              dim((menuItem->rect.height() - 4) / 2),
                                     xpos(menuItem->rect.left() + menuItem->rect.width() - 3 - dim);
                    PrimitiveElement arrow(Qt::RightToLeft==option->direction ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight);
                    QRect            vSubMenuRect(visualRect(option->direction, menuItem->rect,
                                                             QRect(xpos, menuItem->rect.top() + menuItem->rect.height() / 2 - dim / 2, dim, dim)));

                    drawArrow(painter, vSubMenuRect, arrow,
                              opts.useHighlightForMenu && state&State_Selected
                                ? palette.highlightedText().color()
                                : palette.text().color());
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
                drawControl(CE_PushButtonBevel, btn, painter, widget);

                QStyleOptionButton subopt(*btn);

                subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
                drawControl(CE_PushButtonLabel, &subopt, painter, widget);

                if (state&State_HasFocus &&
                    !(state&State_MouseOver && QTC_FULL_FOCUS && MO_NONE!=opts.coloredMouseOver))
                {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*btn);
                    fropt.rect = subElementRect(SE_PushButtonFocusRect, btn, widget);

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
                if (btn->features & QStyleOptionButton::HasMenu)
                {
                    int   mbi(pixelMetric(PM_MenuButtonIndicator, btn, widget));
                    QRect ar(Qt::LeftToRight==btn->direction
                                ? btn->rect.right() - mbi
                                : btn->rect.x() + 6,
                             ((btn->rect.height() - mbi)/2),
                             mbi - 6, mbi);

                    if(option->state &(State_On | State_Sunken))
                        ar.adjust(1, 1, 1, 1);

                    drawArrow(painter, ar, PE_IndicatorArrowDown, QTC_MO_ARROW(QPalette::ButtonText));
                }
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
                    QPixmap      pixmap(getIconPixmap(button->icon, button->iconSize, mode, state));
                    int          labelWidth(pixmap.width()),
                                 labelHeight(pixmap.height()),
                                 iconSpacing (4);//### 4 is currently hardcoded in QPushButton::sizeHint()

                    if (!button->text.isEmpty())
                        labelWidth += (button->fontMetrics.boundingRect(option->rect, tf, button->text).width() + iconSpacing);

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

                // The following is mainly for DejaVu Sans 11...
                if(button->fontMetrics.height()==19 && r.height()==(23+(opts.thinnerBtns ? 0 : 2)))
                    r.translate(0, 1);
   
                if (button->features&QStyleOptionButton::HasMenu)
                {
                    int mbi(pixelMetric(PM_MenuButtonIndicator, button, widget));

                    if (Qt::LeftToRight==button->direction)
                        r = r.adjusted(0, 0, -mbi, 0);
                    else
                        r = r.adjusted(mbi, 0, 0, 0);

                    if(APP_SKYPE==theThemedApp)
                    {
                        // Skype seems to draw a blurry arrow in the lower right corner,
                        // ...draw over this with a nicer sharper arrow...
                        QRect ar(button->rect.x()+(button->rect.width()-(LARGE_ARR_WIDTH+3)),
                                 button->rect.y()+(button->rect.height()-(LARGE_ARR_HEIGHT+2)),
                                 LARGE_ARR_WIDTH,
                                 LARGE_ARR_HEIGHT);

                        if(option->state &(State_On | State_Sunken))
                            ar.adjust(1, 1, 1, 1);
                        drawArrow(painter, ar, PE_IndicatorArrowDown, QTC_MO_ARROW(QPalette::ButtonText));
                    }

//                     QRect              ir(button->rect);
//                     QStyleOptionButton newBtn(*button);
//
//                     newBtn.rect = QRect(Qt::LeftToRight==button->direction
//                                             ? ir.right() - mbi + 2
//                                             : ir.x() + 6,
//                                         ((ir.height() - mbi)/2) + 2,
//                                         mbi - 6, mbi);
//                     drawPrimitive(PE_IndicatorArrowDown, &newBtn, painter, widget);
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
                QRect editRect = subControlRect(CC_ComboBox, comboBox, SC_ComboBoxEditField, widget);
                bool  sunken=state & (State_On|State_Sunken);
                int   shiftH=sunken ? pixelMetric(PM_ButtonShiftHorizontal, option, widget) : 0,
                      shiftV=sunken ? pixelMetric(PM_ButtonShiftVertical, option, widget) : 0;

                painter->save();

                if (!comboBox->currentIcon.isNull())
                {
                    QPixmap pixmap = getIconPixmap(comboBox->currentIcon, comboBox->iconSize, state);
                    QRect   iconRect(editRect);

                    iconRect.setWidth(comboBox->iconSize.width() + 5);
                    if(!comboBox->editable)
                        iconRect = alignedRect(QApplication::layoutDirection(), Qt::AlignLeft|Qt::AlignVCenter,
                                               iconRect.size(), editRect);
                    if (comboBox->editable)
                        painter->fillRect(iconRect, palette.brush(QPalette::Base));

                    if (sunken)
                        iconRect.translate(shiftH, shiftV);

                    drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

                    if (reverse)
                        editRect.translate(-4 - comboBox->iconSize.width(), 0);
                    else
                        editRect.translate(comboBox->iconSize.width() + 4, 0);
                }

                if (sunken)
                    editRect.translate(shiftH, shiftV);

                int margin=comboBox->frame && widget && widget->rect().height()<(QTC_DO_EFFECT ? 22 : 20)  ? 4 : 0;
                editRect.adjust(1, -margin, -1, margin);
                painter->setClipRect(editRect);

                if (!comboBox->currentText.isEmpty() && !comboBox->editable)
                    drawItemText(painter, editRect, Qt::AlignLeft|Qt::AlignVCenter, palette,
                                 state&State_Enabled, comboBox->currentText, QPalette::ButtonText);
                painter->restore();
            }
            break;
        case CE_MenuBarEmptyArea:
            {
                painter->save();

#ifdef QTC_XBAR_SUPPORT
                if(!widget || 0!=strcmp("QWidget", widget->metaObject()->className()))
#endif
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
        case CE_TabBarTabLabel:
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option))
            {
#if QT_VERSION >= 0x040500
                QStyleOptionTabV3 tabV2(*tab);
#else
                QStyleOptionTabV2 tabV2(*tab);
#endif
                bool verticalTabs(QTabBar::RoundedEast==tabV2.shape || QTabBar::RoundedWest==tabV2.shape ||
                                  QTabBar::TriangularEast==tabV2.shape || QTabBar::TriangularWest==tabV2.shape);

                if (verticalTabs)
                {
                    painter->save();
                    int newX, newY, newRot;
                    if (QTabBar::RoundedEast==tabV2.shape || QTabBar::TriangularEast==tabV2.shape)
                    {
                        newX = r.width();
                        newY = r.y();
                        newRot = 90;
                    }
                    else
                    {
                        newX = 0;
                        newY = r.y() + r.height();
                        newRot = -90;
                    }
                    r.setRect(0, 0, r.height(), r.width());

                    QTransform m;
                    m.translate(newX, newY);
                    m.rotate(newRot);
                    painter->setTransform(m, true);
                }

                int alignment(Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic);

                if (!styleHint(SH_UnderlineShortcut, option, widget))
                    alignment |= Qt::TextHideMnemonic;

#if QT_VERSION >= 0x040500
                r = subElementRect(SE_TabBarTabText, option, widget);
#else
                r.adjust(0, 0, pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, tab, widget),
                               pixelMetric(QStyle::PM_TabBarTabShiftVertical, tab, widget));

                if (state&State_Selected)
                {
                    r.setBottom(r.bottom() - pixelMetric(QStyle::PM_TabBarTabShiftVertical, tab, widget));
                    r.setRight(r.right() - pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, tab, widget));
                }
#endif
                if (!tabV2.icon.isNull())
                {
                    QSize iconSize(tabV2.iconSize);
                    if (!iconSize.isValid())
                    {
                        int iconExtent(pixelMetric(PM_SmallIconSize));
                        iconSize = QSize(iconExtent, iconExtent);
                    }

                    QPixmap tabIcon(getIconPixmap(tabV2.icon, iconSize, state&State_Enabled));
                    QSize   tabIconSize = tabV2.icon.actualSize(iconSize, tabV2.state&State_Enabled
                                                                  ? QIcon::Normal
                                                                  : QIcon::Disabled);

                    int offset = 4,
                        left = option->rect.left();
#if QT_VERSION >= 0x040500
                    if (tabV2.leftButtonSize.isNull() || tabV2.leftButtonSize.width()<=0)
                        offset += 2;
                    else
                        left += tabV2.leftButtonSize.width() + 2;
#endif
                    QRect iconRect = QRect(left + offset, r.center().y() - tabIcon.height() / 2,
                                           tabIconSize.width(), tabIconSize.height());
                    if (!verticalTabs)
                        iconRect = visualRect(option->direction, option->rect, iconRect);
                    painter->drawPixmap(iconRect.x(), iconRect.y(), tabIcon);
#if QT_VERSION < 0x040500
                    r.adjust(reverse ? 0 : tabIconSize.width(), 0, reverse ? -tabIconSize.width() : 0, 0);
#endif
                }

                if(!tab->text.isEmpty())
                {
#if QT_VERSION < 0x040500
                    r.adjust(constTabPad, 0, -constTabPad, 0);
#endif
                    drawItemText(painter, r, alignment, tab->palette, tab->state&State_Enabled, tab->text,
                                 QPalette::WindowText);
                }

                if (verticalTabs)
                    painter->restore();

                if (tabV2.state & State_HasFocus)
                {
                    const int constOffset = 1 + pixelMetric(PM_DefaultFrameWidth);

                    int                   x1=tabV2.rect.left(),
                                          x2=tabV2.rect.right() - 1;
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*tab);
                    fropt.rect.setRect(x1 + 1 + constOffset, tabV2.rect.y() + constOffset,
                                       x2 - x1 - 2*constOffset, tabV2.rect.height() - 2*constOffset);

                    fropt.state|=State_Horizontal;
                    if(TAB_MO_BOTTOM==opts.tabMouseOver && FOCUS_LINE==opts.focus)
                        switch(tabV2.shape)
                        {
                            case QTabBar::RoundedNorth:
                            case QTabBar::TriangularNorth:
                                fropt.rect.adjust(0, 0, 0, 1);
                                break;
                            case QTabBar::RoundedEast:
                            case QTabBar::TriangularEast:
                                fropt.rect.adjust(-2, 0, -(fropt.rect.width()+1), 0);
                                fropt.state&=~State_Horizontal;
                                break;
                            case QTabBar::RoundedSouth:
                            case QTabBar::TriangularSouth:
                                fropt.rect.adjust(0, 0, 0, 1);
                                break;
                            case QTabBar::RoundedWest:
                            case QTabBar::TriangularWest:
                                fropt.rect.adjust(0, 0, 2, 0);
                                fropt.state&=~State_Horizontal;
                            default:
                                break;
                        }

                    drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
                }
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

                        drawLightBevel(painter, opt.rect, &opt, widget, ROUNDED_ALL,
                                       /*selected ? use[ORIGINAL_SHADE] :*/ getFill(&opt, use), use, true, WIDGET_STD_BUTTON);
                    }
                    break;
                }
#endif
#if QT_VERSION >= 0x040500
                QStyleOptionTabV3 tabV3(*tab);
#endif
                QRect        r2(r);
                bool         rtlHorTabs(Qt::RightToLeft==tab->direction && horiz),
                             onlyTab(QStyleOptionTab::OnlyOneTab==tab->position),
                             leftCornerWidget(tab->cornerWidgets&QStyleOptionTab::LeftCornerWidget),
                             rightCornerWidget(tab->cornerWidgets&QStyleOptionTab::RightCornerWidget),
                             firstTab((tab->position == (Qt::LeftToRight==tab->direction || !horiz ?
                                       QStyleOptionTab::Beginning : QStyleOptionTab::End)) || onlyTab),
                             lastTab((tab->position == (Qt::LeftToRight==tab->direction  || !horiz ?
                                     QStyleOptionTab::End : QStyleOptionTab::Beginning)) || onlyTab);
                int          tabBarAlignment(styleHint(SH_TabBar_Alignment, tab, widget)),
                             tabOverlap(onlyTab ? 0 : pixelMetric(PM_TabBarTabOverlap, option, widget)),
                             moOffset(ROUNDED_NONE==opts.round || TAB_MO_TOP!=opts.tabMouseOver ? 1 : opts.round),
                             highlightOffset(opts.highlightTab && opts.round>ROUND_SLIGHT ? 2 : 1),
                             highlightBorder(opts.round>ROUND_FULL ? 4 : 3),
                             sizeAdjust(!selected && TAB_MO_GLOW==opts.tabMouseOver ? 1 : 0);
                bool         leftAligned((!rtlHorTabs && Qt::AlignLeft==tabBarAlignment) ||
                                 (rtlHorTabs && Qt::AlignRight==tabBarAlignment)),
                             rightAligned((!rtlHorTabs && Qt::AlignRight==tabBarAlignment) ||
                                          (rtlHorTabs && Qt::AlignLeft==tabBarAlignment)),
                             docMode(
#if QT_VERSION >= 0x040500
                                      tabV3.documentMode
#else
                                    false
#endif
                                   ),
                             docFixLeft(!leftCornerWidget && leftAligned && firstTab && (docMode || onlyBase)),
                             fixLeft(!onlyBase && !leftCornerWidget && leftAligned && firstTab && !docMode),
                             fixRight(!onlyBase && !rightCornerWidget && rightAligned && lastTab && !docMode),
                             mouseOver(state&State_Enabled && state&State_MouseOver),
                             glowMo(!selected && mouseOver && opts.coloredMouseOver && TAB_MO_GLOW==opts.tabMouseOver);
                const QColor *use(backgroundColors(option));
                const QColor &fill(getTabFill(selected, mouseOver, use));
                double       radius=getRadius(&opts, r.width(), r.height(), WIDGET_TAB_TOP, RADIUS_EXTERNAL);
                EBorder      borderProfile(selected || opts.borderInactiveTab
                                        ? opts.borderTab
                                            ? BORDER_LIGHT
                                            : BORDER_RAISED
                                        : BORDER_FLAT);

                painter->save();
                switch(tab->shape)
                {
                    case QTabBar::RoundedNorth:
                    case QTabBar::TriangularNorth:
                    {
                        int round=selected || onlyTab || TAB_MO_GLOW==opts.tabMouseOver || opts.roundAllTabs
                                        ? ROUNDED_TOP
                                        : firstTab
                                            ? ROUNDED_TOPLEFT
                                            : lastTab
                                                ? ROUNDED_TOPRIGHT
                                                : ROUNDED_NONE;
                        if(!selected)
                            r.adjust(0, 2, 0, -2);

                        if(!firstTab)
                            r.adjust(-tabOverlap, 0, 0, 0);
                        painter->setClipPath(buildPath(r.adjusted(0, 0, 0, 4), WIDGET_TAB_TOP, round, radius));
                        fillTab(painter, r.adjusted(1+sizeAdjust, 1, -(1+sizeAdjust), 0), option, fill, true, WIDGET_TAB_TOP);
                        painter->setClipping(false);
                        // This clipping helps with plasma's tabs and nvidia
                        if(selected)
                            painter->setClipRect(r2.adjusted(-1, 0, 1, -1));
                        drawBorder(painter, r.adjusted(sizeAdjust, 0, -sizeAdjust, 4), option, round, glowMo ? itsMouseOverCols : 0L, WIDGET_TAB_TOP, borderProfile, false);
                        if(glowMo)
                            drawGlow(painter, r.adjusted(0, -1, 0, 5), WIDGET_TAB_TOP);

                        if(selected)
                        {
                            painter->setClipping(false);
                            painter->setPen(use[0]);

                            // The point drawn below is because of the clipping above...
                            if(fixLeft)
                                painter->drawPoint(r2.x()+1, r2.y()+r2.height()-1);
                            else
                                painter->drawLine(r2.left()-1, r2.bottom(), r2.left(), r2.bottom());
                            if(!fixRight)
                                painter->drawLine(r2.right()-1, r2.bottom(), r2.right(), r2.bottom());

                            if(docFixLeft)
                            {
                                QColor col(use[QT_STD_BORDER]);
                                col.setAlphaF(0.5);
                                painter->setPen(col);
                                painter->drawPoint(r2.x(), r2.y()+r2.height()-1);
                            }
                        }
                        else
                        {
                            int l(fixLeft ? r2.left()+(opts.round>ROUND_SLIGHT ? 2 : 1) : r2.left()-1),
                                r(fixRight ? r2.right()-2 : r2.right()+1);
                            painter->setPen(use[QT_STD_BORDER]);
                            painter->drawLine(l, r2.bottom()-1, r, r2.bottom()-1);
                            painter->setPen(use[0]);
                            painter->drawLine(l, r2.bottom(), r, r2.bottom());
                        }

                        if(selected)
                        {
                            if(opts.highlightTab)
                            {
                                QColor col(itsHighlightCols[0]);
                                painter->setRenderHint(QPainter::Antialiasing, true);
                                painter->setPen(col);
                                drawAaLine(painter, r.left()+highlightOffset, r.top()+1, r.right()-highlightOffset, r.top()+1);
                                col.setAlphaF(0.5);
                                painter->setPen(col);
                                drawAaLine(painter, r.left()+1, r.top()+2, r.right()-1, r.top()+2);
                                painter->setRenderHint(QPainter::Antialiasing, false);
                                painter->setClipRect(QRect(r.x(), r.y(), r.width(), highlightBorder));
                                drawBorder(painter, r, option, ROUNDED_ALL, itsHighlightCols, WIDGET_TAB_TOP, BORDER_FLAT, false, 3);
                            }

                            if(opts.colorSelTab)
                                colorTab(painter, r.adjusted(1+sizeAdjust, 1, -(1+sizeAdjust), 0), true, WIDGET_TAB_TOP, round);
                        }
                        else if(mouseOver && opts.coloredMouseOver && TAB_MO_GLOW!=opts.tabMouseOver)
                            drawHighlight(painter, QRect(r.x()+(firstTab ? moOffset : 1),
                                                         r.y()+(TAB_MO_TOP==opts.tabMouseOver ? 0 : r.height()-1),
                                                         r.width()-(firstTab || lastTab ? moOffset : 1), 2),
                                          true, TAB_MO_TOP==opts.tabMouseOver);
                        break;
                    }
                    case QTabBar::RoundedSouth:
                    case QTabBar::TriangularSouth:
                    {
                        int round=selected || onlyTab || TAB_MO_GLOW==opts.tabMouseOver || opts.roundAllTabs
                                        ? ROUNDED_BOTTOM
                                        : firstTab
                                            ? ROUNDED_BOTTOMLEFT
                                            : lastTab
                                                ? ROUNDED_BOTTOMRIGHT
                                                : ROUNDED_NONE;
                        if(!selected)
                            r.adjust(0, 2, 0, -2);
                        if(!firstTab)
                            r.adjust(-tabOverlap, 0, 0, 0);

                        painter->setClipPath(buildPath(r.adjusted(0, -4, 0, 0), WIDGET_TAB_BOT, round, radius));
                        fillTab(painter, r.adjusted(1+sizeAdjust, 0, -(1+sizeAdjust), -1), option, fill, true, WIDGET_TAB_BOT);
                        painter->setClipping(false);
                        drawBorder(painter, r.adjusted(sizeAdjust, -4, -sizeAdjust, 0), option, round, glowMo ? itsMouseOverCols : 0L, WIDGET_TAB_BOT, borderProfile, false);
                        if(glowMo)
                            drawGlow(painter, r.adjusted(0, -5, 0, 1), WIDGET_TAB_BOT);

                        if(selected)
                        {
                            painter->setPen(use[opts.borderTab ? 0 : QT_FRAME_DARK_SHADOW]);
                            if(!fixLeft)
                                painter->drawPoint(r2.left()-(TAB_MO_GLOW==opts.tabMouseOver ? 0 : 1), r2.top());
                            if(!fixRight)
                                painter->drawLine(r2.right()-(TAB_MO_GLOW==opts.tabMouseOver ? 0 : 1), r2.top(), r2.right(), r2.top());
                            if(docFixLeft)
                            {
                                QColor col(use[QT_STD_BORDER]);
                                col.setAlphaF(0.5);
                                painter->setPen(col);
                                painter->drawPoint(r2.x(), r2.y());
                            }
                        }
                        else
                        {
                            int l(fixLeft ? r2.left()+(opts.round>ROUND_SLIGHT ? 2 : 1) : r2.left()-1),
                                r(fixRight ? r2.right()-2 : r2.right());
                            painter->setPen(use[QT_STD_BORDER]);
                            painter->drawLine(l, r2.top()+1, r, r2.top()+1);
                            painter->setPen(use[opts.borderTab ? 0 : QT_FRAME_DARK_SHADOW]);
                            painter->drawLine(l, r2.top(), r, r2.top());
                        }

                        if(selected)
                        {
                            if(opts.highlightTab)
                            {
                                QColor col(itsHighlightCols[0]);
                                painter->setRenderHint(QPainter::Antialiasing, true);
                                painter->setPen(col);
                                drawAaLine(painter, r.left()+highlightOffset, r.bottom()-1, r.right()-highlightOffset, r.bottom()-1);
                                col.setAlphaF(0.5);
                                painter->setPen(col);
                                drawAaLine(painter, r.left()+1, r.bottom()-2, r.right()-1, r.bottom()-2);
                                painter->setRenderHint(QPainter::Antialiasing, false);
                                painter->setClipRect(QRect(r.x(), r.y()+r.height()-highlightBorder, r.width(), r.y()+r.height()-1));
                                drawBorder(painter, r, option, ROUNDED_ALL, itsHighlightCols, WIDGET_TAB_BOT, BORDER_FLAT, false, 3);
                            }

                            if(opts.colorSelTab)
                                colorTab(painter, r.adjusted(1+sizeAdjust, 0, -(1+sizeAdjust), -1), true, WIDGET_TAB_BOT, round);
                        }
                        else if(mouseOver && opts.coloredMouseOver && TAB_MO_GLOW!=opts.tabMouseOver)
                            drawHighlight(painter, QRect(r.x()+(firstTab ? moOffset : 1),
                                                         r.y()+(TAB_MO_TOP==opts.tabMouseOver ? r.height()-2 : -1),
                                                         r.width()-(firstTab || lastTab ? moOffset : 1), 2),
                                          true, TAB_MO_TOP!=opts.tabMouseOver);
                        break;
                    }
                    case QTabBar::RoundedWest:
                    case QTabBar::TriangularWest:
                    {
                        int round=selected || onlyTab || TAB_MO_GLOW==opts.tabMouseOver || opts.roundAllTabs
                                        ? ROUNDED_LEFT
                                        : firstTab
                                            ? ROUNDED_TOPLEFT
                                            : lastTab
                                                ? ROUNDED_BOTTOMLEFT
                                                : ROUNDED_NONE;
                        if(!selected)
                            r.adjust(2, 0, -2, 0);

                        if(!firstTab)
                            r.adjust(0, -tabOverlap, 0, 0);
                        painter->setClipPath(buildPath(r.adjusted(0, 0, 4, 0), WIDGET_TAB_TOP, round, radius));
                        fillTab(painter, r.adjusted(1, sizeAdjust, 0, -(1+sizeAdjust)), option, fill, false, WIDGET_TAB_TOP);
                        painter->setClipping(false);
                        drawBorder(painter, r.adjusted(0, sizeAdjust, 4, -sizeAdjust), option, round, glowMo ? itsMouseOverCols : 0L, WIDGET_TAB_TOP, borderProfile, false);
                        if(glowMo)
                            drawGlow(painter, r.adjusted(-1, 0, 5, 0), WIDGET_TAB_TOP);

                        if(selected)
                        {
                            painter->setPen(use[0]);
                            if(!firstTab)
                                painter->drawPoint(r2.right(), r2.top()-(TAB_MO_GLOW==opts.tabMouseOver ? 0 : 1));
                            painter->drawLine(r2.right(), r2.bottom()-1, r2.right(), r2.bottom());
                        }
                        else
                        {
                            int t(firstTab ? r2.top()+(opts.round>ROUND_SLIGHT ? 2 : 1) : r2.top()-1),
                                b(/*lastTab ? r2.bottom()-2 : */ r2.bottom()+1);

                            painter->setPen(use[QT_STD_BORDER]);
                            painter->drawLine(r2.right()-1, t, r2.right()-1, b);
                            painter->setPen(use[0]);
                            painter->drawLine(r2.right(), t, r2.right(), b);
                        }

                        if(selected)
                        {
                            if(opts.highlightTab)
                            {
                                QColor col(itsHighlightCols[0]);
                                painter->setRenderHint(QPainter::Antialiasing, true);
                                painter->setPen(col);
                                drawAaLine(painter, r.left()+1, r.top()+highlightOffset, r.left()+1, r.bottom()-highlightOffset);
                                col.setAlphaF(0.5);
                                painter->setPen(col);
                                drawAaLine(painter, r.left()+2, r.top()+1, r.left()+2, r.bottom()-1);
                                painter->setRenderHint(QPainter::Antialiasing, false);
                                painter->setClipRect(QRect(r.x(), r.y(), highlightBorder, r.height()));
                                drawBorder(painter, r, option, ROUNDED_ALL, itsHighlightCols, WIDGET_TAB_TOP, BORDER_FLAT, false, 3);
                            }

                            if(opts.colorSelTab)
                                colorTab(painter, r.adjusted(1, sizeAdjust, 0, -(1+sizeAdjust)), false, WIDGET_TAB_TOP, round);
                        }
                        else if(mouseOver && opts.coloredMouseOver && TAB_MO_GLOW!=opts.tabMouseOver)
                            drawHighlight(painter, QRect(r.x()+(TAB_MO_TOP==opts.tabMouseOver ? 0 : r.width()-1),
                                                         r.y()+(firstTab ? moOffset : 1),
                                                         2, r.height()-(firstTab || lastTab ? moOffset : 1)),
                                          false, TAB_MO_TOP==opts.tabMouseOver);
                        break;
                    }
                    case QTabBar::RoundedEast:
                    case QTabBar::TriangularEast:
                    {
                        int round=selected || onlyTab || TAB_MO_GLOW==opts.tabMouseOver || opts.roundAllTabs
                                        ? ROUNDED_RIGHT
                                        : firstTab
                                            ? ROUNDED_TOPRIGHT
                                            : lastTab
                                                ? ROUNDED_BOTTOMRIGHT
                                                : ROUNDED_NONE;
                        if(!selected)
                            r.adjust(2, 0, -2, 0);

                        if(!firstTab)
                            r.adjust(0, -tabOverlap, 0, 0);
                        painter->setClipPath(buildPath(r.adjusted(-4, 0, 0, 0), WIDGET_TAB_BOT, round, radius));
                        fillTab(painter, r.adjusted(0, sizeAdjust, -1, -(1+sizeAdjust)), option, fill, false, WIDGET_TAB_BOT);
                        painter->setClipping(false);
                        drawBorder(painter, r.adjusted(-4, sizeAdjust, 0, -sizeAdjust), option, round, glowMo ? itsMouseOverCols : 0L, WIDGET_TAB_BOT, borderProfile, false);
                        if(glowMo)
                            drawGlow(painter, r.adjusted(-5, 0, 1, 0), WIDGET_TAB_BOT);

                        if(selected)
                        {
                            painter->setPen(use[opts.borderTab ? 0 : QT_FRAME_DARK_SHADOW]);
                            if(!firstTab)
                                painter->drawPoint(r2.left(), r2.top()-(TAB_MO_GLOW==opts.tabMouseOver ? 0 : 1));
                            painter->drawLine(r2.left(), r2.bottom()-(TAB_MO_GLOW==opts.tabMouseOver ? 0 : 1), r2.left(), r2.bottom());
                        }
                        else
                        {
                            int t(firstTab ? r2.top()+(opts.round>ROUND_SLIGHT ? 2 : 1) : r2.top()-1),
                                b(/*lastTab ? r2.bottom()-2 : */ r2.bottom()+1);

                            painter->setPen(use[QT_STD_BORDER]);
                            painter->drawLine(r2.left()+1, t, r2.left()+1, b);
                            painter->setPen(use[opts.borderTab ? 0 : QT_FRAME_DARK_SHADOW]);
                            painter->drawLine(r2.left(), t, r2.left(), b);
                        }

                        if(selected)
                        {
                            if(opts.highlightTab)
                            {
                                QColor col(itsHighlightCols[0]);
                                painter->setRenderHint(QPainter::Antialiasing, true);
                                painter->setPen(col);
                                drawAaLine(painter, r.right()-1, r.top()+highlightOffset, r.right()-1, r.bottom()-highlightOffset);
                                col.setAlphaF(0.5);
                                painter->setPen(col);
                                drawAaLine(painter, r.right()-2, r.top()+1, r.right()-2, r.bottom()-1);
                                painter->setRenderHint(QPainter::Antialiasing, false);
                                painter->setClipRect(QRect(r.x()+r.width()-highlightBorder, r.y(), r.x()+r.width()-1, r.height()));
                                drawBorder(painter, r, option, ROUNDED_ALL, itsHighlightCols, WIDGET_TAB_TOP, BORDER_FLAT, false, 3);
                            }

                            if(opts.colorSelTab)
                                colorTab(painter, r.adjusted(0, sizeAdjust, -1, -(1+sizeAdjust)), false, WIDGET_TAB_BOT, round);
                        }
                        else if(mouseOver && opts.coloredMouseOver && TAB_MO_GLOW!=opts.tabMouseOver)
                            drawHighlight(painter, QRect(r.x()+(TAB_MO_TOP==opts.tabMouseOver ? r.width()-2 : -1),
                                                         r.y()+(firstTab ? moOffset : 1),
                                                         2, r.height()-(firstTab || lastTab ? moOffset : 1)),
                                          false, TAB_MO_TOP!=opts.tabMouseOver);
                        break;
                    }
                }
                painter->restore();
            }
            break;
        case CE_ScrollBarAddLine:
        case CE_ScrollBarSubLine:
        {
            QRect            br(r),
                             ar(r);
            const QColor     *use(state&State_Enabled ? itsButtonCols : itsBackgroundCols); // buttonColors(option));
            bool             reverse(option && Qt::RightToLeft==option->direction);
            PrimitiveElement pe=state&State_Horizontal
                                 ? CE_ScrollBarAddLine==element ? (reverse ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight)
                                                                : (reverse ? PE_IndicatorArrowRight : PE_IndicatorArrowLeft)
                                 : CE_ScrollBarAddLine==element ? PE_IndicatorArrowDown : PE_IndicatorArrowUp;
            int              round=PE_IndicatorArrowRight==pe ? ROUNDED_RIGHT :
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
                        if(opts.flatSbarButtons || !opts.vArrows)
                            ar.adjust(1, 0, 1, 0);
                    }
                    else if(reverse && PE_IndicatorArrowRight==pe && r.x()>3)
                    {
                        if(SCROLLBAR_PLATINUM==opts.scrollbarType)
                        {
                            round=ROUNDED_NONE;
                            br.adjust(-1, 0, 0, 0);
                            if(opts.flatSbarButtons || !opts.vArrows)
                                ar.adjust(-1, 0, -1, 0);
                        }
                        else
                        {
                            if(r.x()<pixelMetric(PM_ScrollBarExtent, option, widget)+2)
                                round=ROUNDED_NONE;
                            br.adjust(0, 0, 1, 0);
                            if(opts.flatSbarButtons || !opts.vArrows)
                                ar.adjust(1, 0, 1, 0);
                        }
                    }
                    else if(PE_IndicatorArrowUp==pe && r.y()>3)
                    {
                        round=ROUNDED_NONE;
                        br.adjust(0, 0, 0, 1);
                        if(opts.flatSbarButtons || !opts.vArrows)
                            ar.adjust(0, 1, 0, 1);
                    }
                    break;
                case SCROLLBAR_NEXT:
                    if(!reverse && PE_IndicatorArrowRight==pe)
                    {
                        round=ROUNDED_NONE;
                        br.adjust(-1, 0, 0, 0);
                        if(opts.flatSbarButtons || !opts.vArrows)
                            ar.adjust(-1, 0, 0, -1);
                    }
                    else if(reverse && PE_IndicatorArrowLeft==pe)
                    {
                        round=ROUNDED_NONE;
                        br.adjust(0, 0, 1, 0);
                        if(opts.flatSbarButtons || !opts.vArrows)
                            ar.adjust(-1, 0, 0, 1);
                    }
                    else if(PE_IndicatorArrowDown==pe)
                    {
                        round=ROUNDED_NONE;
                        br.adjust(0, -1, 0, 0);
                        if(opts.flatSbarButtons || !opts.vArrows)
                            ar.adjust(0, -1, 0, -1);
                    }
                    break;
            }

            painter->save();
            if(opts.flatSbarButtons && !IS_FLAT(opts.sbarBgndAppearance) && SCROLLBAR_NONE!=opts.scrollbarType)
                drawBevelGradientReal(palette.brush(QPalette::Background).color(), painter, r, state&State_Horizontal, false,
                                      opts.sbarBgndAppearance, WIDGET_SB_BGND);
//             else if(!(state&QTC_NO_BGND_BUTTON) && (!widget || !widget->testAttribute(Qt::WA_NoSystemBackground)))
//                 painter->fillRect(r, palette.brush(QPalette::Background));

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

            if(opts.flatSbarButtons)
                opt.state&=~(State_Sunken|State_On);
            else
                drawLightBevel(painter, br, &opt, widget, round, getFill(&opt, use), use, true, WIDGET_SB_BUTTON);

            opt.rect = ar;

            if(opt.palette.text().color()!=opt.palette.buttonText().color()) // The following fixes gwenviews scrollbars...
                opt.palette.setColor(QPalette::Text, opt.palette.buttonText().color());

            drawPrimitive(pe, &opt, painter, widget);
            painter->restore();
            break;
        }
        case CE_ScrollBarSubPage:
        case CE_ScrollBarAddPage:
        {
            const QColor *use(itsBackgroundCols); // backgroundColors(option));
            int          borderAdjust(0);

            painter->save();
#ifndef QTC_SIMPLE_SCROLLBARS
            if(QTC_ROUNDED && (SCROLLBAR_NONE==opts.scrollbarType || opts.flatSbarButtons))
                painter->fillRect(r, palette.background().color());
#endif

            switch(opts.scrollbarType)
            {
                case SCROLLBAR_KDE:
                case SCROLLBAR_WINDOWS:
                    borderAdjust=1;
                    break;
                case SCROLLBAR_PLATINUM:
                    if(CE_ScrollBarAddPage==element)
                        borderAdjust=1;
                    break;
                case SCROLLBAR_NEXT:
                    if(CE_ScrollBarSubPage==element)
                        borderAdjust=1;
                default:
                    break;
            }

            if(state&State_Horizontal)
            {
                if(IS_FLAT(opts.appearance))
                    painter->fillRect(r.x(), r.y()+1, r.width(), r.height()-2, use[2]);
                else
                    drawBevelGradient(use[2], painter, QRect(r.x(), r.y()+1, r.width(), r.height()-2),
                                      true, false, opts.grooveAppearance, WIDGET_TROUGH);

#ifndef QTC_SIMPLE_SCROLLBARS
                if(QTC_ROUNDED && (SCROLLBAR_NONE==opts.scrollbarType || opts.flatSbarButtons))
                {
                    if(CE_ScrollBarAddPage==element)
                        drawBorder(painter, r.adjusted(-5, 0, 0, 0), option, ROUNDED_RIGHT, use, WIDGET_TROUGH);
                    else
                        drawBorder(painter, r.adjusted(0, 0, 5, 0), option, ROUNDED_LEFT, use, WIDGET_TROUGH);
                }
                else
#endif
                    if(CE_ScrollBarAddPage==element)
                        drawBorder(painter, r.adjusted(-5, 0, borderAdjust, 0), option, ROUNDED_NONE, use, WIDGET_TROUGH);
                    else
                        drawBorder(painter, r.adjusted(-borderAdjust, 0, 5, 0), option, ROUNDED_NONE, use, WIDGET_TROUGH);
            }
            else
            {
                if(IS_FLAT(opts.appearance))
                    painter->fillRect(r.x()+1, r.y(), r.width()-2, r.height(), use[2]);
                else
                    drawBevelGradient(use[2], painter, QRect(r.x()+1, r.y(), r.width()-2, r.height()),
                                      false, false, opts.grooveAppearance, WIDGET_TROUGH);

#ifndef QTC_SIMPLE_SCROLLBARS
                if(QTC_ROUNDED && (SCROLLBAR_NONE==opts.scrollbarType || opts.flatSbarButtons))
                {
                    if(CE_ScrollBarAddPage==element)
                        drawBorder(painter, r.adjusted(0, -5, 0, 0), option, ROUNDED_BOTTOM, use, WIDGET_TROUGH);
                    else
                        drawBorder(painter, r.adjusted(0, 0, 0, 5), option, ROUNDED_TOP, use, WIDGET_TROUGH);
                }
                else
#endif
                    if(CE_ScrollBarAddPage==element)
                        drawBorder(painter, r.adjusted(0, -5, 0, borderAdjust), option, ROUNDED_NONE, use, WIDGET_TROUGH);
                    else
                        drawBorder(painter, r.adjusted(0, -borderAdjust, 0, 5), option, ROUNDED_NONE, use, WIDGET_TROUGH);
            }
            painter->restore();
            break;
        }
        case CE_ScrollBarSlider:
            painter->save();
            drawSbSliderHandle(painter, r, option);
            painter->restore();
            break;
#ifdef QTC_FIX_DISABLED_ICONS
        // Taken from QStyle - only required so that we can corectly set the disabled icon!!!
        case CE_ToolButtonLabel:
            if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option))
            {
                int shiftX = 0,
                    shiftY = 0;
                if (state & (State_Sunken|State_On))
                {
                    shiftX = pixelMetric(PM_ButtonShiftHorizontal, tb, widget);
                    shiftY = pixelMetric(PM_ButtonShiftVertical, tb, widget);
                }

                // Arrow type always overrules and is always shown
                bool hasArrow = tb->features & QStyleOptionToolButton::Arrow;

                if (((!hasArrow && tb->icon.isNull()) && !tb->text.isEmpty())
                    || Qt::ToolButtonTextOnly==tb->toolButtonStyle)
                {
                    int alignment = Qt::AlignCenter|Qt::TextShowMnemonic;

                    if (!styleHint(SH_UnderlineShortcut, option, widget))
                        alignment |= Qt::TextHideMnemonic;

                    r.translate(shiftX, shiftY);
                    drawItemText(painter, r, alignment, tb->palette, state&State_Enabled, tb->text, QPalette::ButtonText);
                }
                else
                {
                    QPixmap pm;
                    QSize   pmSize = tb->iconSize;
                    QRect   pr = r;

                    if (!tb->icon.isNull())
                    {
                        QIcon::State state = tb->state & State_On ? QIcon::On : QIcon::Off;
                        QIcon::Mode  mode=!(tb->state & State_Enabled)
                                            ? QIcon::Disabled
                                            : (state&State_MouseOver) && (state&State_AutoRaise)
                                                ? QIcon::Active
                                                : QIcon::Normal;
                        QSize        iconSize = tb->iconSize;

                        if (!iconSize.isValid())
                        {
                            int iconExtent = pixelMetric(PM_ToolBarIconSize);
                            iconSize = QSize(iconExtent, iconExtent);
                        }
                        else if(iconSize.width()>iconSize.height())
                            iconSize.setWidth(iconSize.height());
                        else if(iconSize.width()<iconSize.height())
                            iconSize.setHeight(iconSize.width());

                        if(iconSize.width()>tb->rect.size().width())
                            iconSize=QSize(tb->rect.size().width(), tb->rect.size().width());
                        if(iconSize.height()>tb->rect.size().height())
                            iconSize=QSize(tb->rect.size().height(), tb->rect.size().height());

                        pm=getIconPixmap(tb->icon, iconSize, mode, state);
                        pmSize = tb->icon.actualSize(iconSize, mode);
                        if(pmSize.width()<pm.width())
                            pr.setX(pr.x()+((pm.width()-pmSize.width())));
                        if(pmSize.height()<pm.height())
                            pr.setY(pr.y()+((pm.height()-pmSize.height())));
                    }

                    if (Qt::ToolButtonIconOnly!=tb->toolButtonStyle)
                    {
                        QRect tr = r;
                        int   alignment = Qt::TextShowMnemonic;

                        painter->setFont(tb->font);
                        if (!styleHint(SH_UnderlineShortcut, option, widget))
                            alignment |= Qt::TextHideMnemonic;

                        if (Qt::ToolButtonTextUnderIcon==tb->toolButtonStyle)
                        {
                            pr.setHeight(pmSize.height() + 6);

                            tr.adjust(0, pr.bottom()-3, 0, 0); // -3);
                            pr.translate(shiftX, shiftY);
                            if (hasArrow)
                                drawTbArrow(this, tb, pr, painter, widget);
                            else
                                drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
                            alignment |= Qt::AlignCenter;
                        }
                        else
                        {
                            pr.setWidth(pmSize.width() + 8);
                            tr.adjust(pr.right(), 0, 0, 0);
                            pr.translate(shiftX, shiftY);
                            if (hasArrow)
                                drawTbArrow(this, tb, pr, painter, widget);
                            else
                                drawItemPixmap(painter, QStyle::visualRect(option->direction, r, pr), Qt::AlignCenter, pm);
                            alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                        }
                        tr.translate(shiftX, shiftY);
                        drawItemText(painter, QStyle::visualRect(option->direction, r, tr), alignment, tb->palette,
                                     tb->state & State_Enabled, tb->text, QPalette::ButtonText);
                    }
                    else
                    {
                        pr.translate(shiftX, shiftY);

                        if (hasArrow)
                            drawTbArrow(this, tb, pr, painter, widget);
                        else
                        {
                            if (!(tb->subControls&SC_ToolButtonMenu) && tb->features&QStyleOptionToolButton::HasMenu)
                                pr.adjust(-LARGE_ARR_WIDTH, 0, 0, 0);
                            drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
                        }
                    }
                }
            }
            break;
        case CE_RadioButtonLabel:
        case CE_CheckBoxLabel:
            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))
            {
                uint    alignment = visualAlignment(btn->direction, Qt::AlignLeft | Qt::AlignVCenter);
                QPixmap pix;
                QRect   textRect = r;

                if (!styleHint(SH_UnderlineShortcut, btn, widget))
                    alignment |= Qt::TextHideMnemonic;

                if (!btn->icon.isNull())
                {
                    pix = getIconPixmap(btn->icon, btn->iconSize, btn->state);
                    drawItemPixmap(painter, r, alignment, pix);
                    if (reverse)
                        textRect.setRight(textRect.right() - btn->iconSize.width() - 4);
                    else
                        textRect.setLeft(textRect.left() + btn->iconSize.width() + 4);
                }
                if (!btn->text.isEmpty())
                    drawItemText(painter, textRect, alignment | Qt::TextShowMnemonic,
                                 palette, state&State_Enabled, btn->text, QPalette::WindowText);
            }
            break;
        case CE_ToolBoxTabLabel:
            if (const QStyleOptionToolBox *tb = qstyleoption_cast<const QStyleOptionToolBox *>(option))
            {
                bool    enabled = state & State_Enabled,
                        selected = state & State_Selected;
                QPixmap pm = getIconPixmap(tb->icon, pixelMetric(QStyle::PM_SmallIconSize, tb, widget) ,state);
                QRect   cr = subElementRect(QStyle::SE_ToolBoxTabContents, tb, widget);
                QRect   tr, ir;
                int     ih = 0;

                if (pm.isNull())
                {
                    tr = cr;
                    tr.adjust(4, 0, -8, 0);
                }
                else
                {
                    int iw = pm.width() + 4;
                    ih = pm.height();
                    ir = QRect(cr.left() + 4, cr.top(), iw + 2, ih);
                    tr = QRect(ir.right(), cr.top(), cr.width() - ir.right() - 4, cr.height());
                }

                if (selected && styleHint(QStyle::SH_ToolBox_SelectedPageTitleBold, tb, widget))
                {
                    QFont f(painter->font());
                    f.setBold(true);
                    painter->setFont(f);
                }

                QString txt = tb->fontMetrics.elidedText(tb->text, Qt::ElideRight, tr.width());

                if (ih)
                    painter->drawPixmap(ir.left(), (tb->rect.height() - ih) / 2, pm);

                int alignment = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic;
                if (!styleHint(QStyle::SH_UnderlineShortcut, tb, widget))
                    alignment |= Qt::TextHideMnemonic;
                drawItemText(painter, tr, alignment, tb->palette, enabled, txt, QPalette::ButtonText);

                if (!txt.isEmpty() && state&State_HasFocus)
                {
                    QStyleOptionFocusRect opt;
                    opt.rect = tr;
                    opt.palette = palette;
                    opt.state = QStyle::State_None;
                    drawPrimitive(QStyle::PE_FrameFocusRect, &opt, painter, widget);
                }
            }
            break;
#endif
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
                        painter->fillRect(highlightRect,
                                          shade(palette.background().color(), QTC_TO_FACTOR(opts.highlightFactor)));
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
    QRect               r(option->rect);
    const QFlags<State> &state(option->state);
    const QPalette      &palette(option->palette);
    bool                reverse(Qt::RightToLeft==option->direction);

    switch (control)
    {
        case CC_ToolButton:
            if (const QStyleOptionToolButton *toolbutton = qstyleoption_cast<const QStyleOptionToolButton *>(option))
            {
                if (widget)
                {
                    if(qobject_cast<QTabBar *>(widget->parentWidget()))
                    {
                        QStyleOptionToolButton btn(*toolbutton);

                        if(Qt::LeftArrow==toolbutton->arrowType || Qt::RightArrow==toolbutton->arrowType)
                            btn.rect.adjust(0, 4, 0, -4);
                        else
                            btn.rect.adjust(4, 0, -4, 0);
                        if(!(btn.state&State_Enabled))
                            btn.state&=~State_MouseOver;
                        drawPrimitive(PE_PanelButtonTool, &btn, painter, widget);
                        if(opts.vArrows)
                            switch(toolbutton->arrowType)
                            {
                                case Qt::LeftArrow:
                                    btn.rect.adjust(-1, 0, -1, 0);
                                    break;
                                case Qt::RightArrow:
                                    btn.rect.adjust(1, 0, 1, 0);
                                    break;
                                case Qt::UpArrow:
                                    btn.rect.adjust(0, -1, 0, -1);
                                    break;
                                case Qt::DownArrow:
                                    btn.rect.adjust(0, 1, 0, 1);
                                default:
                                    break;
                            }
                        drawTbArrow(this, &btn, btn.rect, painter, widget);
                        break;
                    }

                    const QToolButton *btn = qobject_cast<const QToolButton *>(widget);

                    if(btn && btn->isDown() && Qt::ToolButtonTextBesideIcon==btn->toolButtonStyle() &&
                       widget->parentWidget() && widget->parentWidget()->inherits("KMenu"))
                    {
                        if(opts.menuStripe)
                        {
                            int stripeWidth(qMax(20, constMenuPixmapWidth));

                            drawBevelGradient(menuStripeCol(),
                                              painter, QRect(reverse ? r.right()-stripeWidth : r.x(), r.y(),
                                                             stripeWidth, r.height()), false,
                                              false, opts.menuStripeAppearance, WIDGET_OTHER);
                        }

                        // For some reason the MenuTitle has a larger border on the left, so adjust the width by 1 pixel
                        // to make this look nicer.
                        //drawBorder(painter, r.adjusted(2, 2, -3, -2), option, ROUNDED_ALL, 0L, WIDGET_OTHER, BORDER_SUNKEN);
                        QStyleOptionToolButton opt(*toolbutton);
                        opt.rect = r.adjusted(2, 2, -3, -2);
                        opt.state=State_Raised|State_Enabled|State_Horizontal;
                        drawLightBevel(painter, opt.rect, &opt, widget, ROUNDED_ALL,
                                       getFill(&opt, itsBackgroundCols), itsBackgroundCols,
                                       true, WIDGET_NO_ETCH_BTN);
                        int fw = pixelMetric(PM_DefaultFrameWidth, option, widget);
                        opt.rect.adjust(fw, fw, -fw, -fw);
                        drawControl(CE_ToolButtonLabel, &opt, painter, widget);
                        break;
                    }
                }
                QRect button(subControlRect(control, toolbutton, SC_ToolButton, widget)),
                      menuarea(subControlRect(control, toolbutton, SC_ToolButtonMenu, widget));
                State bflags(toolbutton->state);
                bool  etched(QTC_DO_EFFECT);

                if (!(bflags&State_Enabled))
                    bflags &= ~(State_MouseOver/* | State_Raised*/);

                if(bflags&State_MouseOver)
                    bflags |= State_Raised;
                else if(bflags&State_AutoRaise)
                    bflags &= ~State_Raised;

                //if(state&State_AutoRaise)
                    bflags|=QTC_STATE_TBAR_BUTTON;

                State mflags(bflags);

#if QT_VERSION >= 0x040500
                if (state&State_Sunken && !(toolbutton->activeSubControls&SC_ToolButton))
                    bflags&=~State_Sunken;
#else
                // Try to detect if this is Qt 4.5...
                if(qtVersion()>=VER_45)
                {
                    if (state&State_Sunken && !(toolbutton->activeSubControls&SC_ToolButton))
                        bflags&=~State_Sunken;
                }
                else if (toolbutton->activeSubControls&SC_ToolButtonMenu && state&State_Enabled)
                    mflags |= State_Sunken;
#endif

                bool         drawMenu=mflags & (State_Sunken | State_On | State_Raised);
                QStyleOption tool(0);
                tool.palette = toolbutton->palette;

                if ( (toolbutton->subControls&SC_ToolButton && (bflags & (State_Sunken | State_On | State_Raised))) ||
                     (toolbutton->subControls&SC_ToolButtonMenu && drawMenu))
                {
                    tool.rect = toolbutton->subControls&SC_ToolButtonMenu ? button.united(menuarea) : button;
                    tool.state = bflags;

                    if(!(bflags&State_Sunken) && (mflags&State_Sunken))
                        tool.state &= ~State_MouseOver;

                    drawPrimitive(PE_PanelButtonTool, &tool, painter, widget);
                }

                if (toolbutton->subControls&SC_ToolButtonMenu)
                {
                    if(etched)
                        if(reverse)
                            menuarea.adjust(1, 1, 0, -1);
                        else
                            menuarea.adjust(0, 1, -1, -1);

                    tool.rect = menuarea;
                    tool.state = mflags|State_Horizontal;

                    if(drawMenu)
                    {
                        const QColor *use(buttonColors(option));

                        if(mflags&State_Sunken)
                            tool.state&=~State_MouseOver;

                        drawLightBevel(painter, menuarea, &tool, widget,
                                       reverse ? ROUNDED_LEFT : ROUNDED_RIGHT, getFill(&tool, use), use, true,
                                       MO_GLOW==opts.coloredMouseOver ? WIDGET_MENU_BUTTON : WIDGET_NO_ETCH_BTN);
                    }

                    if(mflags&State_Sunken)
                        tool.rect.adjust(1, 1, 1, 1);
                    drawArrow(painter, tool.rect, PE_IndicatorArrowDown,
                              QTC_MO_ARROW_X(toolbutton->activeSubControls&SC_ToolButtonMenu,
                                             QPalette::ButtonText));
                }
/*
                else if (toolbutton->features & QStyleOptionToolButton::HasMenu)
                {
                    QRect arrow(r.right()-(LARGE_ARR_WIDTH+(etched ? 3 : 2)),
                                r.bottom()-(LARGE_ARR_HEIGHT+(etched ? 4 : 3)),
                                LARGE_ARR_WIDTH, LARGE_ARR_HEIGHT);

                    if(bflags&State_Sunken)
                        arrow.adjust(1, 1, 1, 1);

                    drawArrow(painter, arrow, option, PE_IndicatorArrowDown, false, false);
                }
*/

                if (toolbutton->state & State_HasFocus)
                {
                    QStyleOptionFocusRect fr;

                    fr.QStyleOption::operator=(*toolbutton);
                    if(QTC_FULL_FOCUS)
                    {
                        if(etched && MO_GLOW==opts.coloredMouseOver)
                            fr.rect.adjust(1, 1, -1, -1);
                    }
                    else
                    {
                        if(etched)
                            fr.rect.adjust(4, 4, -4, -4);
                        else
                            fr.rect.adjust(3, 3, -3, -3);
#if QT_VERSION >= 0x040300
                        if (toolbutton->features & QStyleOptionToolButton::MenuButtonPopup)
#else
                        if (toolbutton->features & QStyleOptionToolButton::Menu)
#endif
                            fr.rect.adjust(0, 0, -(pixelMetric(QStyle::PM_MenuButtonIndicator, toolbutton, widget)-1), 0);
                    }
                    if(!(state&State_MouseOver && QTC_FULL_FOCUS && MO_NONE!=opts.coloredMouseOver))
                        drawPrimitive(PE_FrameFocusRect, &fr, painter, widget);
                }
                QStyleOptionToolButton label = *toolbutton;
                int fw = pixelMetric(PM_DefaultFrameWidth, option, widget);
                label.rect = button.adjusted(fw, fw, -fw, -fw);
                label.state = bflags;
                drawControl(CE_ToolButtonLabel, &label, painter, widget);

                if (!(toolbutton->subControls&SC_ToolButtonMenu) &&
                     (toolbutton->features&QStyleOptionToolButton::HasMenu))
                {
                    QRect arrow(r.right()-(LARGE_ARR_WIDTH+(etched ? 3 : 2)),
                                r.bottom()-(LARGE_ARR_HEIGHT+(etched ? 4 : 3)),
                                LARGE_ARR_WIDTH, LARGE_ARR_HEIGHT);

                    if(bflags&State_Sunken)
                        arrow.adjust(1, 1, 1, 1);

                    drawArrow(painter, arrow, PE_IndicatorArrowDown, QTC_MO_ARROW(QPalette::ButtonText));
                }
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
                if (lv->subControls&SC_Q3ListView)
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
                    if ((lv->activeSubControls&SC_All) && (lv->subControls&SC_Q3ListViewExpand))
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
                                    || (child.childCount > 0 && child.height > 0))
                                {

                                    QRect ar(bx-4, linebot-4, 11, 11);

                                    if(LV_OLD==opts.lvLines)
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

                                    drawArrow(painter, ar,
                                              child.state&State_Open
                                                ? PE_IndicatorArrowDown
                                                : reverse
                                                    ? PE_IndicatorArrowLeft
                                                    : PE_IndicatorArrowRight,
                                              palette.text().color());

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

                    if (opts.lvLines && (lv->subControls&SC_Q3ListViewBranch))
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
                      down(subControlRect(CC_SpinBox, option, SC_SpinBoxDown, widget)),
                      all(frame.united(up).united(down));
                bool  doFrame(spinBox->frame && frame.isValid()),
                      sunken(state&State_Sunken),
                      enabled(state&State_Enabled),
                      mouseOver(state&State_MouseOver),
                      upIsActive(SC_SpinBoxUp==spinBox->activeSubControls),
                      downIsActive(SC_SpinBoxDown==spinBox->activeSubControls),
                      doEtch(QTC_DO_EFFECT && opts.etchEntry);

                if(doEtch)
                {
                    drawEtch(painter, all, widget, WIDGET_SPIN);
                    down.adjust(reverse ? 1 : 0, 0, reverse ? 0 : -1, -1);
                    up.adjust(reverse ? 1 : 0, 1, reverse ? 0 : -1, 0);
                    frame.adjust(reverse ? 0 : 1, 1, reverse ? -1 : 0, -1);
                    all.adjust(1, 1, -1, -1);
                }

                if(opts.unifySpin)
                    drawEntryField(painter, all, widget, option, ROUNDED_ALL, true, false);
                else
                {
                    if(opts.unifySpinBtns)
                    {
                        QRect btns=up.united(down);
                        const QColor *use(buttonColors(option));
                        QStyleOption opt(*option);

                        opt.state&=~(State_Sunken|State_MouseOver);
                        opt.state|=State_Horizontal;

                        drawLightBevel(painter, btns, &opt, widget, reverse ?  ROUNDED_LEFT : ROUNDED_RIGHT,
                                    getFill(&opt, use), use, true, WIDGET_SPIN);

                        if(state&State_MouseOver && state&State_Enabled && !(state&State_Sunken))
                        {
                            opt.state|=State_MouseOver;
                            painter->save();
                            painter->setClipRect(upIsActive ? up : down);
                            drawLightBevel(painter, btns, &opt, widget, reverse ?  ROUNDED_LEFT : ROUNDED_RIGHT,
                                        getFill(&opt, use), use, true, WIDGET_SPIN);
                            painter->restore();
                        }
                        drawFadedLine(painter, down.adjusted(2, 0, -2, 0), use[QT_BORDER(state&State_Enabled)], true, true, true);
                    }
                }

                if(up.isValid())
                {
                    QStyleOption opt(*option);

                    up.setHeight(up.height()+1);
                    opt.rect=up;
                    opt.direction=option->direction;
                    opt.state=(enabled && spinBox->stepEnabled&QAbstractSpinBox::StepUpEnabled ? State_Enabled : State_None)|
                              (upIsActive && sunken ? State_Sunken : State_Raised)|
                              (upIsActive && !sunken && mouseOver ? State_MouseOver : State_None)|State_Horizontal;;

                    drawPrimitive(QAbstractSpinBox::PlusMinus==spinBox->buttonSymbols ? PE_IndicatorSpinPlus : PE_IndicatorSpinUp,
                                  &opt, painter, widget);
                }

                if(down.isValid())
                {
                    QStyleOption opt(*option);

                    opt.rect=down;
                    opt.state=(enabled && spinBox->stepEnabled&QAbstractSpinBox::StepDownEnabled ? State_Enabled : State_None)|
                              (downIsActive && sunken ? State_Sunken : State_Raised)|
                              (downIsActive && !sunken && mouseOver ? State_MouseOver : State_None)|State_Horizontal;
                    opt.direction=option->direction;

                    drawPrimitive(QAbstractSpinBox::PlusMinus==spinBox->buttonSymbols ? PE_IndicatorSpinMinus : PE_IndicatorSpinDown,
                                  &opt, painter, widget);
                }
                if(doFrame && !opts.unifySpin)
                {
                    if(reverse)
                        frame.setX(frame.x()-1);
                    else
                        frame.setWidth(frame.width()+1);
                    drawEntryField(painter, frame, widget, option, reverse ? ROUNDED_RIGHT : ROUNDED_LEFT, true, false);
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

                if ((option->subControls&SC_SliderGroove) && groove.isValid())
                    drawSliderGroove(painter, groove, handle, slider, widget);

                if ((option->subControls&SC_SliderHandle) && handle.isValid())
                {
                    QStyleOptionSlider s(*slider);
                    if(!(s.activeSubControls & QStyle::SC_SliderHandle))
                    {
                        s.state &= ~QStyle::State_MouseOver;
                        s.state &= ~QStyle::State_Sunken;
                    }

                    drawSliderHandle(painter, handle, &s);

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

                if (option->subControls&SC_SliderTickmarks)
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

                    while (v <= slider->maximum + 1)
                    {
                        if (v == slider->maximum + 1 && interval == 1)
                            break;

                        int pos(sliderPositionFromValue(slider->minimum, slider->maximum,
                                                        qMin(v, slider->maximum), (horizontal
                                                            ? slider->rect.width()
                                                            : slider->rect.height()) - len,
                                                        slider->upsideDown) + len / 2);

                        int extra(2); // - ((v == slider->minimum || v == slider->maximum) ? 1 : 0);

                        if (horizontal)
                        {
                            if (ticksAbove)
                                painter->drawLine(QLine(pos, slider->rect.top() + extra,
                                                pos, slider->rect.top() + tickSize));
                            if (ticksBelow)
                                painter->drawLine(QLine(pos, slider->rect.bottom() - extra,
                                                pos, slider->rect.bottom() - tickSize));
                        }
                        else
                        {
                            if (ticksAbove)
                                painter->drawLine(QLine(slider->rect.left() + extra, pos,
                                                slider->rect.left() + tickSize, pos));
                            if (ticksBelow)
                                painter->drawLine(QLine(slider->rect.right() - extra, pos,
                                                slider->rect.right() - tickSize, pos));
                        }

                        // in the case where maximum is max int
                        int nextInterval = v + interval;
                        if (nextInterval < v)
                            break;
                        v = nextInterval;
                    }
                    painter->setPen(oldPen);
                }
            }
            break;
        case CC_TitleBar:
            if (const QStyleOptionTitleBar *titleBar = qstyleoption_cast<const QStyleOptionTitleBar *>(option))
            {
                painter->save();

                bool         active(state&State_Active),
                             kwin(theThemedApp==APP_KWIN || titleBar->titleBarState&QtC_StateKWin),
                             roundKWinFull(QTC_FULLLY_ROUNDED &&
                                            ((APP_KWIN==theThemedApp && !(titleBar->titleBarState&State_Raised)) ||
                                              titleBar->titleBarState&QtC_StateKWin));
                const QColor *bgndCols(kwin ? buttonColors(option) : getMdiColors(titleBar, active)),
                             *btnCols(kwin || opts.titlebarButtons&QTC_TITLEBAR_BUTTON_STD_COLOR
                                        ? buttonColors(option)
                                        : getMdiColors(titleBar, active)),
                             *titleCols(kwin || !(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_STD_COLOR)
                                            ? btnCols : getMdiColors(titleBar, active));
                QColor       textColor(theThemedApp==APP_KWIN
                                        ? option->palette.color(QPalette::WindowText)
                                        : active || opts.titlebarButtons&QTC_TITLEBAR_BUTTON_HOVER_SYMBOL
                                            ? itsActiveMdiTextColor
                                            : itsMdiTextColor),
                             shadow(shadowColor(textColor));
                QStyleOption opt(*option);
                bool         drawLine=opts.colorTitlebarOnly &&
                                        (kwin ? titleBar->titleBarState&QtCStateKWinDrawLine
                                              : ((itsActiveMdiColors &&
                                                  itsActiveMdiColors[ORIGINAL_SHADE]!=itsBackgroundCols[ORIGINAL_SHADE]) ||
                                                (itsMdiColors &&
                                                  itsMdiColors[ORIGINAL_SHADE]!=itsBackgroundCols[ORIGINAL_SHADE])) );

                opt.state=State_Horizontal|State_Enabled|State_Raised|(active ? State_Active : State_None);
                if(state&QtC_StateKWinHighlight)
                    opt.state|=QtC_StateKWinHighlight;

//                 if(APP_KWIN!=theThemedApp && roundKWinFull) // Set clipping for preview in kcmshell...
//                 {
//                     int     x(r.x()), y(r.y()), w(r.width()), h(r.height());
//                     QRegion mask(x+5, y+0, w-10, h);
//
//                     mask += QRegion(x+0, y+5, 1, h-6);
//                     mask += QRegion(x+1, y+3, 1, h-3);
//                     mask += QRegion(x+2, y+2, 1, h-2);
//                     mask += QRegion(x+3, y+1, 2, h-1);
//
//                     mask += QRegion(x+w-1, y+5, 1, h-6);
//                     mask += QRegion(x+w-2, y+3, 1, h-3);
//                     mask += QRegion(x+w-3, y+2, 1, h-2);
//                     mask += QRegion(x+w-5, y+1, 2, h-1);
//                     painter->setClipRegion(mask);
//                 }

                if(state&QtCStateKWinNoBorder)
                    drawBevelGradient(titleCols[ORIGINAL_SHADE], painter, r, true, false,
                                      widgetApp(WIDGET_MDI_WINDOW, &opts, option->state&State_Active),
                                      WIDGET_MDI_WINDOW);
                else
                {
                    drawLightBevel(painter, r, &opt, widget,
                                   titleBar->titleBarState&State_Raised
                                        ? ROUNDED_NONE
                                        : titleBar->titleBarState&State_Enabled
                                            ? ROUNDED_ALL
                                            : ROUNDED_TOP,
                                    titleCols[ORIGINAL_SHADE], titleCols, true,
                                    titleBar->titleBarState&Qt::WindowMinimized ? WIDGET_MDI_WINDOW : WIDGET_MDI_WINDOW_TITLE);

                    if(opts.titlebarBorder)
                    {
                        painter->setPen(btnCols[0]);
                        painter->drawPoint(r.x()+1, r.y()+r.height()-1);
                    }

                    if(roundKWinFull)
                    {
                        bool   kwinHighlight(state&QtC_StateKWinHighlight);
                        QColor col(kwinHighlight ? itsFocusCols[0] : btnCols[QT_STD_BORDER]);

                        painter->setPen(col);

                        if(kwinHighlight || (state&QtC_StateKWinShadows))
                        {
                            painter->drawPoint(r.x()+3, r.y()+1);
                            painter->drawPoint(r.x()+1, r.y()+3);
                            painter->drawPoint(r.x()+r.width()-4, r.y()+1);
                            painter->drawPoint(r.x()+r.width()-2, r.y()+3);
    //                         painter->drawPoint(r.x()+r.width()-1, r.y()+6);
                            col.setAlphaF(0.5);
                            painter->setPen(col);
                            painter->drawPoint(r.x()+2, r.y()+2);
                            painter->drawPoint(r.x()+4, r.y()+1);
                            painter->drawPoint(r.x()+1, r.y()+4);
                            painter->drawPoint(r.x()+r.width()-3, r.y()+2);
                            painter->drawPoint(r.x()+r.width()-5, r.y()+1);
                            painter->drawPoint(r.x()+r.width()-2, r.y()+4);
                        }
                        else
                        {
                            painter->drawLine(r.x()+1, r.y()+4, r.x()+1, r.y()+3);
                            painter->drawPoint(r.x()+2, r.y()+2);
                            painter->drawLine(r.x()+3, r.y()+1, r.x()+4, r.y()+1);
                            painter->drawLine(r.x()+r.width()-2, r.y()+4, r.x()+r.width()-2, r.y()+3);
                            painter->drawPoint(r.x()+r.width()-3, r.y()+2);
                            painter->drawLine(r.x()+r.width()-4, r.y()+1, r.x()+r.width()-5, r.y()+1);
                        }
                        if(opts.titlebarBorder && (APPEARANCE_SHINY_GLASS!=(active ? opts.titlebarAppearance : opts.inactiveTitlebarAppearance)))
                        {
                            col=btnCols[0];
                            //col.setAlphaF(0.5);
                            painter->setPen(col);
                            painter->drawLine(r.x()+2, r.y()+4, r.x()+2, r.y()+3);
                            painter->drawLine(r.x()+3, r.y()+2, r.x()+4, r.y()+2);
                            //painter->drawLine(r.x()+r.width()-3, r.y()+4, r.x()+r.width()-3, r.y()+3);
                            painter->drawLine(r.x()+r.width()-4, r.y()+2, r.x()+r.width()-5, r.y()+2);
                        }
                    }
                }

                if(drawLine)
                {
                    painter->setPen(btnCols[QT_STD_BORDER]);
                    painter->drawLine(r.left()+1, r.bottom(), r.right()-1, r.bottom());
                }

                bool    showIcon=TITLEBAR_ICON_NEXT_TO_TITLE==opts.titlebarIcon && !titleBar->icon.isNull();
                int     iconSize=showIcon ? pixelMetric(QStyle::PM_SmallIconSize) : 0,
                        iconX=r.x();
                QPixmap pixmap;

                if(showIcon)
                    pixmap=getIconPixmap(titleBar->icon, iconSize, titleBar->state);

                if(!titleBar->text.isEmpty())
                {
                    static const int constPad=4;

                    QFont         font(painter->font());
                    Qt::Alignment alignment((Qt::Alignment)pixelMetric((QStyle::PixelMetric)QtC_TitleAlignment, 0L, 0L));
                    bool          alignFull(Qt::AlignHCenter==alignment),
                                  iconRight((!reverse && alignment&Qt::AlignRight) || (reverse && alignment&Qt::AlignLeft));
                    QRect         captionRect(subControlRect(CC_TitleBar, titleBar, SC_TitleBarLabel, widget)),
                                  textRect(alignFull
                                            ? QRect(r.x(), captionRect.y(), r.width(), captionRect.height())
                                            : captionRect);

                    font.setBold(true);
                    painter->setFont(font);

                    QFontMetrics fm(painter->fontMetrics());
                    QString str(fm.elidedText(titleBar->text, Qt::ElideRight, textRect.width(), QPalette::WindowText));

                    int           textWidth=alignFull || (showIcon && alignment&Qt::AlignHCenter)
                                                ? fm.boundingRect(str).width()+(showIcon ? iconSize+constPad : 0) : 0;

                    if(alignFull)
                        if(captionRect.left()>((textRect.width()-textWidth)>>1))
                        {
                            alignment=Qt::AlignVCenter|Qt::AlignLeft;
                            textRect=captionRect;
                        }
                        else if(captionRect.right()<((textRect.width()+textWidth)>>1))
                        {
                            alignment=Qt::AlignVCenter|Qt::AlignRight;
                            textRect=captionRect;
                        }

                    if(showIcon)
                        if(alignment&Qt::AlignHCenter)
                        {
                            if(reverse)
                            {
                                iconX=((textRect.width()-textWidth)/2.0)+0.5+textWidth+iconSize;
                                textRect.setX(textRect.x()-(iconSize+constPad));
                            }
                            else
                            {
                                iconX=((textRect.width()-textWidth)/2.0)+0.5;
                                textRect.setX(iconX+iconSize+constPad);
                                alignment=Qt::AlignVCenter|Qt::AlignLeft;
                            }
                        }
                        else if((!reverse && alignment&Qt::AlignLeft) || (reverse && alignment&Qt::AlignRight))
                        {
                            iconX=textRect.x();
                            textRect.setX(textRect.x()+(iconSize+constPad));
                        }
                        else if((!reverse && alignment&Qt::AlignRight) || (reverse && alignment&Qt::AlignLeft))
                        {
                            if(iconRight)
                            {
                                iconX=textRect.x()+textRect.width()-iconSize;
                                textRect.setWidth(textRect.width()-(iconSize+constPad));
                            }
                            else
                            {
                                iconX=textRect.x()+textRect.width()-textWidth;
                                if(iconX<textRect.x())
                                    iconX=textRect.x();
                            }
                        }

                    QTextOption textOpt(alignment|Qt::AlignVCenter);
                    textOpt.setWrapMode(QTextOption::NoWrap);

                    painter->setPen(shadow);
                    painter->drawText(textRect.adjusted(1, 1, 1, 1), str, textOpt);
                    painter->setPen(textColor);
                    painter->drawText(textRect, str, textOpt);
                }

                if(showIcon && iconX>=0)
                    painter->drawPixmap(iconX, r.y()+((r.height()-iconSize)/2)+1, pixmap);

                if ((titleBar->subControls&SC_TitleBarMinButton) && (titleBar->titleBarFlags&Qt::WindowMinimizeButtonHint) &&
                    !(titleBar->titleBarState&Qt::WindowMinimized))
                    drawMdiControl(painter, titleBar, SC_TitleBarMinButton, widget, TITLEBAR_MIN, textColor, shadow, btnCols, bgndCols);

                if ((titleBar->subControls&SC_TitleBarMaxButton) && (titleBar->titleBarFlags&Qt::WindowMaximizeButtonHint) &&
                    !(titleBar->titleBarState&Qt::WindowMaximized))
                    drawMdiControl(painter, titleBar, SC_TitleBarMaxButton, widget, TITLEBAR_MAX, textColor, shadow, btnCols, bgndCols);

                if ((titleBar->subControls&SC_TitleBarCloseButton) && (titleBar->titleBarFlags&Qt::WindowSystemMenuHint))
                    drawMdiControl(painter, titleBar, SC_TitleBarCloseButton, widget, TITLEBAR_CLOSE, textColor, shadow, btnCols, bgndCols);

                if ((titleBar->subControls&SC_TitleBarNormalButton) &&
                    (((titleBar->titleBarFlags&Qt::WindowMinimizeButtonHint) &&
                    (titleBar->titleBarState&Qt::WindowMinimized)) ||
                    ((titleBar->titleBarFlags&Qt::WindowMaximizeButtonHint) &&
                    (titleBar->titleBarState&Qt::WindowMaximized))))
                    drawMdiControl(painter, titleBar, SC_TitleBarNormalButton, widget, TITLEBAR_MAX, textColor, shadow, btnCols, bgndCols);

                if (titleBar->subControls&SC_TitleBarContextHelpButton && (titleBar->titleBarFlags&Qt::WindowContextHelpButtonHint))
                    drawMdiControl(painter, titleBar, SC_TitleBarContextHelpButton, widget, TITLEBAR_HELP, textColor, shadow, btnCols, bgndCols);

                if (titleBar->subControls&SC_TitleBarShadeButton && (titleBar->titleBarFlags&Qt::WindowShadeButtonHint))
                    drawMdiControl(painter, titleBar, SC_TitleBarShadeButton, widget, TITLEBAR_SHADE, textColor, shadow, btnCols, bgndCols);

                if (titleBar->subControls&SC_TitleBarUnshadeButton && (titleBar->titleBarFlags&Qt::WindowShadeButtonHint))
                    drawMdiControl(painter, titleBar, SC_TitleBarUnshadeButton, widget, TITLEBAR_SHADE, textColor, shadow, btnCols, bgndCols);

                if ((titleBar->subControls&SC_TitleBarSysMenu) && (titleBar->titleBarFlags&Qt::WindowSystemMenuHint))
                    if(TITLEBAR_ICON_MENU_BUTTON==opts.titlebarIcon)
                    {
                        QRect rect = subControlRect(CC_TitleBar, titleBar, SC_TitleBarSysMenu, widget);
                        if (rect.isValid())
                        {
                            bool sunken((titleBar->activeSubControls&SC_TitleBarSysMenu) && (titleBar->state&State_Sunken)),
                                 hover((titleBar->activeSubControls&SC_TitleBarSysMenu) && (titleBar->state&State_MouseOver));
                            int  offset(sunken ? 1 : 0);

                            if(!(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_ROUND))
                                drawMdiButton(painter, rect, hover, sunken,
                                            coloredMdiButtons(state&State_Active, hover) ? itsTitleBarButtonsCols[TITLEBAR_MENU] : btnCols);

                            if (!titleBar->icon.isNull())
                                titleBar->icon.paint(painter, rect.adjusted(offset, offset, offset, offset));
                            else
                            {
                                QStyleOption tool(0);

                                tool.palette = palette;
                                tool.rect = rect;
                                painter->save();
                                drawItemPixmap(painter, rect.adjusted(offset, offset, offset, offset), Qt::AlignCenter, standardIcon(SP_TitleBarMenuButton, &tool, widget).pixmap(16, 16));
                                painter->restore();
                            }
                        }
                    }
                    else
                        drawMdiControl(painter, titleBar, SC_TitleBarSysMenu, widget, TITLEBAR_MENU, textColor, shadow, btnCols, bgndCols);
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
                                   atMax(maxed || scrollbar->sliderValue==scrollbar->maximum)/*,
                                   inStack(0!=opts.tabBgnd && inStackWidget(widget))*/;
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
                bool  noButtons(QTC_ROUNDED && (SCROLLBAR_NONE==opts.scrollbarType || opts.flatSbarButtons));
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

                if(opts.flatSbarButtons && !IS_FLAT(opts.sbarBgndAppearance) && SCROLLBAR_NONE!=opts.scrollbarType)
                    drawBevelGradientReal(palette.brush(QPalette::Background).color(), painter, r, horiz, false,
                                          opts.sbarBgndAppearance, WIDGET_SB_BGND);
//                 else
//                 {
//                     if(!widget || !widget->testAttribute(Qt::WA_NoSystemBackground))
//                         painter->fillRect(r, palette.brush(QPalette::Background));
// 
//                     if(opts.flatSbarButtons && APP_KRUNNER==theThemedApp)
//                         painter->fillRect(r, itsBackgroundCols[ORIGINAL_SHADE]);
// 
//                     if(inStack && 0!=opts.tabBgnd)
//                         painter->fillRect(r, shade(option->palette.background().color(), QTC_TO_FACTOR(opts.tabBgnd)));
// //                     else if(opts.gtkScrollViews && !IS_FLAT(opts.bgndAppearance))
// //                         drawWindowBackground((QWidget *)widget);
//                 }

                if(noButtons || opts.flatSbarButtons)
                {
                    int mod=QTC_THIN_SBAR_MOD;
                    // Draw complete groove here, as we want to round both ends...
                    opt.rect=subpage.united(addpage);
                    opt.state=scrollbar->state;
                    opt.state&=~(State_MouseOver|State_Sunken|State_On);

                    if(opts.thinSbarGroove && slider.isValid())
                    {
                        painter->save();
                        painter->setClipRegion(QRegion(opt.rect).subtract(slider));
                    }
                    drawLightBevel(painter, opts.thinSbarGroove
                                                ? horiz
                                                    ? opt.rect.adjusted(0, mod, 0, -mod)
                                                    : opt.rect.adjusted(mod, 0, -mod, 0)
                                                : opt.rect, &opt, widget,
    #ifndef QTC_SIMPLE_SCROLLBARS
                                   SCROLLBAR_NONE==opts.scrollbarType || opts.flatSbarButtons ? ROUNDED_ALL :
    #endif
                                   ROUNDED_NONE,
                                   itsBackgroundCols[2], itsBackgroundCols, true,
                                   opts.thinSbarGroove ? WIDGET_SLIDER_TROUGH : WIDGET_TROUGH);
                    if(opts.thinSbarGroove && slider.isValid())
                        painter->restore();
                }
                else
                {
                    if((option->subControls&SC_ScrollBarSubPage) && subpage.isValid())
                    {
                        opt.state=scrollbar->state;
                        opt.rect = subpage;
//                         if (!(scrollbar->activeSubControls&SC_ScrollBarSubPage))
                            opt.state &= ~(State_Sunken|State_MouseOver|State_On);
                        drawControl(CE_ScrollBarSubPage, &opt, painter, widget);
                    }

                    if((option->subControls&SC_ScrollBarAddPage) && addpage.isValid())
                    {
                        opt.state=scrollbar->state;
                        opt.rect = addpage;
//                         if (!(scrollbar->activeSubControls&SC_ScrollBarAddPage))
                            opt.state &= ~(State_Sunken|State_MouseOver|State_On);
                        drawControl(CE_ScrollBarAddPage, &opt, painter, widget);
                    }
                }

                if((option->subControls&SC_ScrollBarSubLine) && subline.isValid())
                {
                    opt.rect=subline;
                    opt.state=scrollbar->state/*|(inStack ? QTC_NO_BGND_BUTTON : State_None)*/;
                    if(maxed || atMin)
                        opt.state&=~State_Enabled;
                    if (!(scrollbar->activeSubControls&SC_ScrollBarSubLine) ||
                        (useThreeButtonScrollBar && itsSbWidget && itsSbWidget==widget))
                        opt.state &= ~(State_Sunken | State_MouseOver);

                    drawControl(CE_ScrollBarSubLine, &opt, painter, widget);

                    if (useThreeButtonScrollBar && subline2.isValid())
                    {
                        opt.rect=subline2;
                        opt.state=scrollbar->state/*|(inStack ? QTC_NO_BGND_BUTTON : State_None)*/;
                        if(maxed || atMin)
                            opt.state&=~State_Enabled;
                        if ((!(scrollbar->activeSubControls&SC_ScrollBarSubLine)) || (itsSbWidget && itsSbWidget!=widget))
                            opt.state &= ~(State_Sunken | State_MouseOver);

                        drawControl(CE_ScrollBarSubLine, &opt, painter, widget);
                    }
                }

                if((option->subControls&SC_ScrollBarAddLine) && addline.isValid())
                {
                    opt.rect=addline;
                    opt.state=scrollbar->state/*|(inStack ? QTC_NO_BGND_BUTTON : State_None)*/;
                    if(maxed || atMax)
                        opt.state&=~State_Enabled;
                    if (!(scrollbar->activeSubControls&SC_ScrollBarAddLine))
                        opt.state &= ~(State_Sunken | State_MouseOver);
                    drawControl(CE_ScrollBarAddLine, &opt, painter, widget);
                }

                if((option->subControls&SC_ScrollBarFirst) && first.isValid())
                {
                    opt.rect=first;
                    opt.state=scrollbar->state;
                    if (!(scrollbar->activeSubControls&SC_ScrollBarFirst))
                        opt.state &= ~(State_Sunken | State_MouseOver);
                    drawControl(CE_ScrollBarFirst, &opt, painter, widget);
                }

                if((option->subControls&SC_ScrollBarLast) && last.isValid())
                {
                    opt.rect=last;
                    opt.state=scrollbar->state;
                    if (!(scrollbar->activeSubControls&SC_ScrollBarLast))
                        opt.state &= ~(State_Sunken | State_MouseOver);
                    drawControl(CE_ScrollBarLast, &opt, painter, widget);
                }

                if(((option->subControls&SC_ScrollBarSlider) || noButtons) && slider.isValid())
                {
                    // If "SC_ScrollBarSlider" wasn't specified, then we only want to draw the portion
                    // of the slider that overlaps with the trough. So, once again set the clipping
                    // region...

                    // NO! Seeems to mess things up with Arora, su just dsiable all clipping when drawing
                    // the slider...
                    painter->setClipping(false);
//                     if(!(option->subControls&SC_ScrollBarSlider))
//                         painter->setClipRegion(QRegion(s2)+QRegion(a2));
#ifdef QTC_INCREASE_SB_SLIDER
                    /*else*/ if(!opts.flatSbarButtons)
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
                    if (!(scrollbar->activeSubControls&SC_ScrollBarSlider))
                        opt.state &= ~(State_Sunken | State_MouseOver);
                    drawControl(CE_ScrollBarSlider, &opt, painter, widget);

                    // ### perhaps this should not be able to accept focus if maxedOut?
                    if(state&State_HasFocus)
                    {
                        opt.state=scrollbar->state;
                        opt.rect=QRect(slider.x()+2, slider.y()+2, slider.width()-5, slider.height()-5);
                        drawPrimitive(PE_FrameFocusRect, &opt, painter, widget);
                    }

//                     if(!(option->subControls&SC_ScrollBarSlider))
//                         painter->setClipping(false);
                }
                painter->restore();
            }
            break;
        case CC_ComboBox:
            if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
            {
                painter->save();

                QRect        frame(subControlRect(CC_ComboBox, option, SC_ComboBoxFrame, widget)),
                             arrow(subControlRect(CC_ComboBox, option, SC_ComboBoxArrow, widget)),
                             field(subControlRect(CC_ComboBox, option, SC_ComboBoxEditField, widget));
                const QColor *use(buttonColors(option));
                bool         sunken(state&State_On), // comboBox->listBox() ? comboBox->listBox()->isShown() : false),
                             glowOverFocus(state&State_MouseOver && QTC_FULL_FOCUS &&
                                           MO_GLOW==opts.coloredMouseOver && QTC_DO_EFFECT && !sunken && !comboBox->editable &&
                                           state&State_Enabled && state&State_HasFocus),
                             doEffect(QTC_DO_EFFECT && (!comboBox->editable || opts.etchEntry));

//                 painter->fillRect(r, Qt::transparent);
                if(doEffect)
                {
                    if(!glowOverFocus && !sunken && MO_GLOW==opts.coloredMouseOver &&
                        ((QTC_FULL_FOCUS && state&State_HasFocus) || state&State_MouseOver) &&
                       state&State_Enabled && !comboBox->editable)
                        drawGlow(painter, r, QTC_FULL_FOCUS && state&State_HasFocus ? WIDGET_DEF_BUTTON : WIDGET_COMBO);
                    else
                        drawEtch(painter, r, widget, WIDGET_COMBO,
                                !comboBox->editable && EFFECT_SHADOW==opts.buttonEffect && !sunken);

                    frame.adjust(1, 1, -1, -1);
                }

//                 // This section fixes some drawng issues with krunner's combo on nvidia
//                 painter->setRenderHint(QPainter::Antialiasing, true);
//                 painter->fillRect(frame.adjusted(1, 1, -1, -1), palette.background().color());
//                 painter->setRenderHint(QPainter::Antialiasing, false);

                if(/*comboBox->frame &&*/ frame.isValid() && (!comboBox->editable || !opts.unifyCombo))
                {
                    const QColor *cols=itsComboBtnCols && comboBox->editable && state&State_Enabled ? itsComboBtnCols : use;

                    QStyleOption frameOpt(*option);

                    if (comboBox->editable && !(comboBox->activeSubControls&SC_ComboBoxArrow))
                        frameOpt.state &= ~(State_Sunken | State_MouseOver);

                    if(!sunken)
                        frameOpt.state|=State_Raised;

                    //if(opts.coloredMouseOver && frameOpt.state&State_MouseOver && comboBox->editable && !sunken)
                    //    frame.adjust(reverse ? 0 : 1, 0, reverse ? 1 : 0, 0);

                    drawLightBevel(painter, frame, &frameOpt, widget,
                                   comboBox->editable ? (reverse ? ROUNDED_LEFT : ROUNDED_RIGHT) : ROUNDED_ALL,
                                   getFill(&frameOpt, cols, false,
                                           (SHADE_DARKEN==opts.comboBtn || (SHADE_NONE!=opts.comboBtn &&
                                                                            !(state&State_Enabled))) &&
                                           comboBox->editable),
                                   cols, true, comboBox->editable ? WIDGET_COMBO_BUTTON : WIDGET_COMBO);
                }

                if(/*controls&SC_ComboBoxEditField &&*/ field.isValid())
                {
                    if(comboBox->editable)
                    {
                        if(opts.unifyCombo)
                        {
                            field=r;
                            if(doEffect)
                                field.adjust(1, 1, -1, -1);
                        }
                        //field.adjust(-1,-1, 0, 1);
//                         painter->setPen(state&State_Enabled ? palette.base().color() : palette.background().color());
//                         drawRect(painter, field);
                        // 2 for frame width
                        if(!opts.unifyCombo)
                        {
                            int pad=opts.round>ROUND_FULL ? 2 : 0;

                            field.adjust(-(2+pad),-2, (2+pad), 2);
                        }
                        drawEntryField(painter, field, widget, option, opts.unifyCombo ? ROUNDED_ALL : reverse ? ROUNDED_RIGHT : ROUNDED_LEFT, true, false);
                    }
                    else if(opts.comboSplitter && !(SHADE_DARKEN==opts.comboBtn || itsComboBtnCols))
                    {
                        drawFadedLine(painter, QRect(reverse ? arrow.right()+1 : arrow.x()-1, arrow.top()+2,
                                                     1, arrow.height()-4),
                                      use[QT_BORDER(state&State_Enabled)], true, true, false);
                        if(!sunken)
                            drawFadedLine(painter, QRect(reverse ? arrow.right()+2 : arrow.x(), arrow.top()+2,
                                                         1, arrow.height()-4),
                                          use[0], true, true, false);
                    }
                }

                if(/*controls&SC_ComboBoxArrow && */arrow.isValid())
                {
                    bool mouseOver=comboBox->editable && !(comboBox->activeSubControls&SC_ComboBoxArrow)
                                    ? false : (state&State_MouseOver ? true : false);

                    if(!comboBox->editable && (SHADE_DARKEN==opts.comboBtn || itsComboBtnCols))
                    {
                        QStyleOption frameOpt(*option);
                        QRect        btn(arrow.x(), frame.y(), arrow.width()+2, frame.height());
                        const QColor *cols=SHADE_DARKEN==opts.comboBtn || !(state&State_Enabled) ? use : itsComboBtnCols;
                        if(!sunken)
                            frameOpt.state|=State_Raised;
                        painter->save();
                        painter->setClipRect(btn);
                        drawLightBevel(painter, opts.comboSplitter
                                                    ? btn
                                                    : btn.adjusted(reverse ? 0 : -2, 0, reverse ? 2 : 0, 0),
                                       &frameOpt, widget, reverse ? ROUNDED_LEFT : ROUNDED_RIGHT,
                                       getFill(&frameOpt, cols, false,
                                               SHADE_DARKEN==opts.comboBtn || (SHADE_NONE!=opts.comboBtn &&
                                                                               !(state&State_Enabled))),
                                       cols, true, WIDGET_COMBO);
                        painter->restore();
                    }

                    if(sunken && (!comboBox->editable || !opts.unifyCombo))
                        arrow.adjust(1, 1, 1, 1);

                    QColor arrowColor(QTC_MO_ARROW_X(mouseOver, QPalette::ButtonText));
                    if(comboBox->editable || !(opts.gtkComboMenus && opts.doubleGtkComboArrow))
                        drawArrow(painter, arrow, PE_IndicatorArrowDown, arrowColor, false, false);
                    else
                    {
                        int middle=arrow.y()+(arrow.height()>>1),
                            gap=(opts.vArrows ? 2 : 1);

                        QRect ar=QRect(arrow.x(), middle-(LARGE_ARR_HEIGHT+gap), arrow.width(), LARGE_ARR_HEIGHT);
                        drawArrow(painter, ar, PE_IndicatorArrowUp, arrowColor, false, false);
                        ar=QRect(arrow.x(), middle+gap, arrow.width(), LARGE_ARR_HEIGHT);
                        drawArrow(painter, ar, PE_IndicatorArrowDown, arrowColor, false, false);
                    }
                }

                if(state&State_Enabled && state&State_HasFocus &&
                    /*state&State_KeyboardFocusChange &&*/ !comboBox->editable)
                {
                    QStyleOptionFocusRect focus;
                    bool                  listViewCombo=comboBox->frame && widget && widget->rect().height()<(QTC_DO_EFFECT ? 22 : 20);

                    if(QTC_FULL_FOCUS)
                        focus.rect=frame;
                    else if(opts.comboSplitter)
                    {
                        focus.rect=reverse
                                    ? field.adjusted(0, -1, 1, 1)
                                    : field.adjusted(-1, -1, 0, 1);

                        if(listViewCombo)
                            focus.rect.adjust(0, -2, 0, 2);
                    }
                    else if(listViewCombo)
                        focus.rect=frame.adjusted(1, 1, -1, -1);
                    else
                        focus.rect=frame.adjusted(3, 3, -3, -3);

                    // Draw glow over top of filled focus
                    if(glowOverFocus)
                        drawGlow(painter, frame.adjusted(-1, -1, 1, 1), WIDGET_COMBO);
                    else
                        drawPrimitive(PE_FrameFocusRect, &focus, painter, widget);
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
            newSize=size;
            newSize.setWidth(newSize.width()+8);

            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))
            {
                if(btn->features&QStyleOptionButton::AutoDefaultButton)
                {
                    int iconHeight=btn->icon.isNull() ? btn->iconSize.height() : 16;
                    if(size.height()<iconHeight+2)
                        newSize.setHeight(iconHeight+2);
                }

                int margin = (pixelMetric(PM_ButtonMargin, btn, widget)+
                             (pixelMetric(PM_DefaultFrameWidth, btn, widget) * 2))-QTC_MAX_ROUND_BTN_PAD;

                newSize+=QSize(margin, margin);

                if (!btn->text.isEmpty() && "..."!=btn->text && newSize.width() < 80)
                    newSize.setWidth(80);
                if (btn->features&QStyleOptionButton::HasMenu)
                    newSize+=QSize(4, 0);
                newSize.rheight() += ((1 - newSize.rheight()) & 1);
//                 if (!btn->icon.isNull() && btn->iconSize.height() > 16)
//                     newSize -= QSize(0, 2);

            }
            break;
        }
        case CT_RadioButton:
            ++newSize.rheight();
            ++newSize.rwidth();
            break;
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
            if(opts.unifySpin)
                newSize.rheight() --;
            else
                newSize.rheight() -= ((1 - newSize.rheight()) & 1);
            break;
        case CT_ToolButton:
        {
            newSize = QSize(size.width()+8, size.height()+8);
            // -- from kstyle & oxygen --
            // We want to avoid super-skiny buttons, for things like "up" when icons + text
            // For this, we would like to make width >= height.
            // However, once we get here, QToolButton may have already put in the menu area
            // (PM_MenuButtonIndicator) into the width. So we may have to take it out, fix things
            // up, and add it back in. So much for class-independent rendering...

            int menuAreaWidth(0);

            if (const QStyleOptionToolButton* tbOpt = qstyleoption_cast<const QStyleOptionToolButton*>(option))
            {
                // Make Kate/KWrite's option toolbuton have the same size as the next/prev buttons...
                if(widget && !getToolBar(widget) && !tbOpt->text.isEmpty() &&
                   tbOpt->features&QStyleOptionToolButton::MenuButtonPopup)
                {
                    QStyleOptionButton btn;

                    btn.init(widget);
                    btn.text=tbOpt->text;
                    btn.icon=tbOpt->icon;
                    btn.iconSize=tbOpt->iconSize;
                    btn.features=tbOpt->features&QStyleOptionToolButton::MenuButtonPopup
                                    ? QStyleOptionButton::HasMenu : QStyleOptionButton::None;
                    return sizeFromContents(CT_PushButton, &btn, size, widget);
                }

                if (!tbOpt->icon.isNull() && !tbOpt->text.isEmpty() && Qt::ToolButtonTextUnderIcon==tbOpt->toolButtonStyle)
                    newSize.setHeight(newSize.height()-4);

                if (tbOpt->features & QStyleOptionToolButton::MenuButtonPopup)
                    menuAreaWidth = pixelMetric(QStyle::PM_MenuButtonIndicator, option, widget);
                else if (tbOpt->features & QStyleOptionToolButton::HasMenu)
                    switch(tbOpt->toolButtonStyle)
                    {
                        case Qt::ToolButtonIconOnly:
                            newSize.setWidth(newSize.width()+LARGE_ARR_WIDTH+2);
                            break;
                        case Qt::ToolButtonTextBesideIcon:
                            newSize.setWidth(newSize.width()+3);
                            break;
                        case Qt::ToolButtonTextOnly:
                            newSize.setWidth(newSize.width()+8);
                            break;
                        case Qt::ToolButtonTextUnderIcon:
                            newSize.setWidth(newSize.width()+8);
                            break;
                    }
            }

            newSize.setWidth(newSize.width() - menuAreaWidth);
            if (newSize.width() < newSize.height())
                newSize.setWidth(newSize.height());
            newSize.setWidth(newSize.width() + menuAreaWidth);

            break;
        }
        case CT_ComboBox:
        {
            newSize=size;
            newSize.setWidth(newSize.width()+4);

// Not sure about this - makes combos the same as pushbuttons, bu sometimes they are too big?
//             int iconHeight=/*btn->icon.isNull() ? btn->iconSize.height() : */16;
//             if(size.height()<iconHeight+2)
//                 newSize.setHeight(iconHeight+2);
                    
            int margin      = (pixelMetric(PM_ButtonMargin, option, widget)+
                              (pixelMetric(PM_DefaultFrameWidth, option, widget) * 2))-QTC_MAX_ROUND_BTN_PAD,
                textMargins = 2*(pixelMetric(PM_FocusFrameHMargin) + 1),
                // QItemDelegate::sizeHint expands the textMargins two times, thus the 2*textMargins...
                other = qMax(QTC_DO_EFFECT ? 20 : 18, 2*textMargins + pixelMetric(QStyle::PM_ScrollBarExtent, option, widget));

            newSize+=QSize(margin+other, margin);
            newSize.rheight() += ((1 - newSize.rheight()) & 1);
            break;
        }
        case CT_MenuItem:
            if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
            {
                int h(newSize.height()-8); // Fix mainly for Qt4.4

                if (QStyleOptionMenuItem::Separator==mi->menuItemType)
                    h = 7;
                else
                {
                    h = qMax(h, mi->fontMetrics.height());
                    if (!mi->icon.isNull())
                        h = qMax(h, mi->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal).height());

                    if (h < 18)
                        h = 18;
                    h+=(opts.thinnerMenuItems ? 2 : 4);
                }

                newSize.setHeight(h);
                // Gtk2's icon->text spacing is 2 pixels smaller - so adjust here...
                newSize.setWidth(newSize.width()-2);
            }
            break;
        case CT_MenuBarItem:
#if QT_VERSION >= 0x040500
            if (!size.isEmpty())
                newSize=size+QSize((windowsItemHMargin * 4)+2, windowsItemVMargin+1);
#else
            if (!size.isEmpty())
                newSize=size+QSize((windowsItemHMargin * 4)+2, windowsItemVMargin+(qtVersion()<VER_45 ? 0 : 1));
#endif
            break;
        case CT_MenuBar:
            if(APP_KONQUEROR==theThemedApp && widget && qobject_cast<const QMenuBar *>(widget))
            {
                int height=konqMenuBarSize((const QMenuBar *)widget);
#ifdef QTC_XBAR_SUPPORT
                if(size.height()>height)
#endif
                newSize.setHeight(height);
            }
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
#if QT_VERSION >= 0x040500
        case SE_TabBarTabText:
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option))
            {
                QStyleOptionTabV3 tabV2(*tab);
                bool              verticalTabs=QTabBar::RoundedEast==tabV2.shape || QTabBar::RoundedWest==tabV2.shape ||
                                               QTabBar::TriangularEast==tabV2.shape || QTabBar::TriangularWest==tabV2.shape;

                rect=tabV2.rect;
                if (verticalTabs)
                    rect.setRect(0, 0, rect.height(), rect.width());
                int verticalShift = pixelMetric(QStyle::PM_TabBarTabShiftVertical, tab, widget);
                int horizontalShift = pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, tab, widget);
                if (tabV2.shape == QTabBar::RoundedSouth || tabV2.shape == QTabBar::TriangularSouth)
                    verticalShift = -verticalShift;
                rect.adjust(0, 0, horizontalShift, verticalShift);
                bool selected = tabV2.state & State_Selected;
                if (selected)
                {
                    rect.setBottom(rect.bottom() - verticalShift);
                    rect.setRight(rect.right() - horizontalShift);
                }

                // left widget
                if (tabV2.leftButtonSize.isNull())
                    rect.setLeft(rect.left()+constTabPad);
                else if(tabV2.leftButtonSize.width()>0)
                    rect.setLeft(rect.left() + constTabPad + 2 +
                                (verticalTabs ? tabV2.leftButtonSize.height() : tabV2.leftButtonSize.width()));
                else if(tabV2.icon.isNull())
                    rect.setLeft(rect.left()+constTabPad);
                else
                    rect.setLeft(rect.left() + 2);

                // icon
                if (!tabV2.icon.isNull())
                {
                    QSize iconSize = tabV2.iconSize;
                    if (!iconSize.isValid())
                    {
                        int iconExtent = pixelMetric(PM_SmallIconSize);
                        iconSize = QSize(iconExtent, iconExtent);
                    }
                    QSize tabIconSize = tabV2.icon.actualSize(iconSize,
                                                            (tabV2.state & State_Enabled) ? QIcon::Normal
                                                            : QIcon::Disabled);
                    int offset = 4;
                    if (tabV2.leftButtonSize.isNull())
                        offset += 2;

                    QRect iconRect = QRect(rect.left() + offset, rect.center().y() - tabIconSize.height() / 2,
                                           tabIconSize.width(), tabIconSize .height());
                    if (!verticalTabs)
                        iconRect = visualRect(option->direction, option->rect, iconRect);
                    rect.setLeft(rect.left() + tabIconSize.width() + offset + 2);
                }

                // right widget
                if (!tabV2.rightButtonSize.isNull() && tabV2.rightButtonSize.width()>0)
                    rect.setRight(rect.right() - constTabPad - 2 -
                                  (verticalTabs ? tabV2.rightButtonSize.height() : tabV2.rightButtonSize.width()));

                if (!verticalTabs)
                    rect = visualRect(option->direction, option->rect, rect);
                return rect;
            }
            break;
#endif
        case SE_RadioButtonIndicator:
            rect = visualRect(option->direction, option->rect,
                              QTC_BASE_STYLE::subElementRect(element, option, widget)).adjusted(0, 0, 1, 1);
            break;
        case SE_ProgressBarContents:
          return opts.fillProgress
                    ? QTC_DO_EFFECT
                        ? option->rect.adjusted(1, 1, -1, -1)
                        : option->rect
                    : QTC_DO_EFFECT
                        ? option->rect.adjusted(3, 3, -3, -3)
                        : option->rect.adjusted(2, 2, -2, -2);
        case SE_ProgressBarGroove:
        case SE_ProgressBarLabel:
            return option->rect;
#if QT_VERSION >= 0x040300
        case SE_GroupBoxLayoutItem:
            rect = option->rect;
//             if (const QStyleOptionGroupBox *groupBoxOpt = qstyleoption_cast<const QStyleOptionGroupBox *>(option))
//                 if (groupBoxOpt->subControls & (SC_GroupBoxCheckBox | SC_GroupBoxLabel))
//                     rect.setTop(rect.top() + 2);    // eat the top margin a little bit
            break;
#endif
        case SE_PushButtonFocusRect:
            if(QTC_FULL_FOCUS)
            {
                rect=subElementRect(SE_PushButtonContents, option, widget);
                if(QTC_DO_EFFECT)
                    rect.adjust(-1, -1, 1, 1);
                else
                    rect.adjust(-2, -2, 2, 2);
            }
            else
            {
                rect=QTC_BASE_STYLE::subElementRect(element, option, widget);
                if(QTC_DO_EFFECT)
                    rect.adjust(1, 1, -1, -1);
            }
            return rect;
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
                bool doEtch(opts.etchEntry && QTC_DO_EFFECT),
                     ed(comboBox->editable);
                int  x(r.x()),
                     y(r.y()),
                     w(r.width()),
                     h(r.height()),
                     margin(comboBox->frame ? 3 : 0),
                     bmarg(comboBox->frame ? 2 : 0);

                switch (subControl)
                {
                    case SC_ComboBoxFrame:
                        if(ed)
                        {
                            int btnWidth(doEtch ? 20 : 18);

                            r=QRect(x+w-btnWidth, y, btnWidth, h);
                        }
                        break;
                    case SC_ComboBoxArrow:
                        r.setRect(x + w - bmarg - (doEtch ? 17 : 16), y + bmarg, 16, h - 2*bmarg);
                        if(ed)
                            r.adjust(2, 0, 0, 0);
                        break;
                    case SC_ComboBoxEditField:
                        r.setRect(x + margin +1, y + margin + 1, w - 2 * margin - 19, h - 2 * margin -2);
                        if(doEtch)
                            r.adjust(1, 1, -1, -1);
                        if(ed)
                        {
                            int pad=opts.round>ROUND_FULL ? 2 : 0;
                            r.adjust(-2+pad, -2, 2-pad, 2);
                        }
                        break;
                    case SC_ComboBoxListBoxPopup:
                    default:
                        break;
                }
                return visualRect(comboBox->direction, comboBox->rect, r);
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
                bs.setWidth(QTC_DO_EFFECT && opts.etchEntry ? 16 : 15);
                bs=bs.expandedTo(QApplication::globalStrut());

                int y(0), x(reverse ? 0 : r.width()-bs.width());

                switch(subControl)
                {
                    case SC_SpinBoxUp:
                        return QRect(x, y, bs.width(), bs.height());
                    case SC_SpinBoxDown:
                    {
                        int extra(bs.height()*2==r.height() ? 0 : 1);
                        return QRect(x, y+bs.height(), bs.width(), bs.height()+extra);
                    }
                    case SC_SpinBoxEditField:
                    {
                        int pad=opts.round>ROUND_FULL ? 2 : 0;
                        return QRect(fw+(reverse ? bs.width() : 0)+pad, fw, (x-fw*2)-pad, r.height()-2*fw);
                    }
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
            if(0==(r.height()%2))
                r.adjust(0, 0, 1, 1);
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
    itsSbWidget=0L;
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

void QtCurveStyle::drawHighlight(QPainter *p, const QRect &r, bool horiz, bool inc) const
{
    QColor col1(itsMouseOverCols[ORIGINAL_SHADE]);

    col1.setAlphaF(0.5);
    drawFadedLine(p, r, inc ? col1 : itsMouseOverCols[ORIGINAL_SHADE], true, true, horiz);
    drawFadedLine(p, r.adjusted(horiz ? 0 : 1, horiz ? 1 : 0, 0, 0), inc ? itsMouseOverCols[ORIGINAL_SHADE] : col1, true, true, horiz);
}

void QtCurveStyle::drawFadedLine(QPainter *p, const QRect &r, const QColor &col, bool fadeStart, bool fadeEnd, bool horiz) const
{
    bool            aa(p->testRenderHint(QPainter::Antialiasing));
    QPointF         start(r.x()+(aa ? 0.5 : 0.0), r.y()+(aa ? 0.5 : 0.0)),
                    end(r.x()+(horiz ? r.width()-1 : 0)+(aa ? 0.5 : 0.0),
                        r.y()+(horiz ? 0 : r.height()-1)+(aa ? 0.5 : 0.0));

    if(opts.fadeLines && (fadeStart || fadeEnd))
    {
        QLinearGradient grad(start, end);
        QColor          fade(col);

        fade.setAlphaF(0.0);
        grad.setColorAt(0, fadeStart && opts.fadeLines ? fade : col);
        grad.setColorAt(QTC_FADE_SIZE, col);
        grad.setColorAt(1.0-QTC_FADE_SIZE, col);
        grad.setColorAt(1, fadeEnd && opts.fadeLines ? fade : col);
        p->setPen(QPen(QBrush(grad), 1));
    }
    else
        p->setPen(col);
    p->drawLine(start, end);
}

void QtCurveStyle::drawLines(QPainter *p, const QRect &r, bool horiz, int nLines, int offset,
                             const QColor *cols, int startOffset, int dark, ELine type) const
{
    int  space((nLines*2)+(LINE_DASHES!=type ? (nLines-1) : 0)),
         step(LINE_DASHES!=type ? 3 : 2),
         etchedDisp(LINE_SUNKEN==type ? 1 : 0),
         x(horiz ? r.x() : r.x()+((r.width()-space)>>1)),
         y(horiz ? r.y()+((r.height()-space)>>1) : r.y()),
         x2(r.x()+r.width()-1),
         y2(r.y()+r.height()-1),
         i;
    QPen dp(cols[dark], 1),
         lp(cols[0], 1);

    if(opts.fadeLines && (horiz ? r.width() : r.height())>16)
    {
        QLinearGradient grad(r.topLeft(), horiz ? r.topRight() : r.bottomLeft());
        QColor          fade(cols[dark]);

        fade.setAlphaF(0.0);
        grad.setColorAt(0, fade);
        grad.setColorAt(0.4, cols[dark]);
        grad.setColorAt(0.6, cols[dark]);
        grad.setColorAt(1, fade);

        dp=QPen(QBrush(grad), 1);

        if(LINE_FLAT!=type)
        {
            fade=QColor(cols[0]);

            fade.setAlphaF(0.0);
            grad.setColorAt(0, fade);
            grad.setColorAt(0.4, cols[0]);
            grad.setColorAt(0.6, cols[0]);
            grad.setColorAt(1, fade);
            lp=QPen(QBrush(grad), 1);
        }
    }

    p->setRenderHint(QPainter::Antialiasing, true);
    if(horiz)
    {
        if(startOffset && y+startOffset>0)
            y+=startOffset;

        p->setPen(dp);
        for(i=0; i<space; i+=step)
            drawAaLine(p, x+offset, y+i, x2-offset, y+i);

        if(LINE_FLAT!=type)
        {
            p->setPen(lp);
            x+=etchedDisp;
            x2+=etchedDisp;
            for(i=1; i<space; i+=step)
                drawAaLine(p, x+offset, y+i, x2-offset, y+i);
        }
    }
    else
    {
        if(startOffset && x+startOffset>0)
            x+=startOffset;

        p->setPen(dp);
        for(i=0; i<space; i+=step)
            drawAaLine(p, x+i, y+offset, x+i, y2-offset);

        if(LINE_FLAT!=type)
        {
            p->setPen(lp);
            y+=etchedDisp;
            y2+=etchedDisp;
            for(i=1; i<space; i+=step)
                drawAaLine(p, x+i, y+offset, x+i, y2-offset);
        }
    }
    p->setRenderHint(QPainter::Antialiasing, false);
}

void QtCurveStyle::drawProgressBevelGradient(QPainter *p, const QRect &origRect, const QStyleOption *option, bool horiz, EAppearance bevApp) const
{
    const QColor *use=option->state&State_Enabled || ECOLOR_BACKGROUND==opts.progressGrooveColor ? itsHighlightCols : itsBackgroundCols;
    bool    vertical(!horiz),
            inCache(true);
    QRect   r(0, 0, horiz ? PROGRESS_CHUNK_WIDTH*2 : origRect.width(),
                    horiz ? origRect.height() : PROGRESS_CHUNK_WIDTH*2);
    QtcKey  key(createKey(horiz ? r.height() : r.width(), use[ORIGINAL_SHADE], horiz, bevApp, WIDGET_PROGRESSBAR));
    QPixmap *pix(itsPixmapCache.object(key));

    if(!pix)
    {
        pix=new QPixmap(r.width(), r.height());

        QPainter pixPainter(pix);

        if(IS_FLAT(bevApp))
            pixPainter.fillRect(r, use[ORIGINAL_SHADE]);
        else
            drawBevelGradientReal(use[ORIGINAL_SHADE], &pixPainter, r, horiz, false,
                                  bevApp, WIDGET_PROGRESSBAR);

        switch(opts.stripedProgress)
        {
            default:
            case STRIPE_NONE:
                break;
            case STRIPE_PLAIN:
            {
                QRect r2(horiz
                            ? QRect(r.x(), r.y(), PROGRESS_CHUNK_WIDTH, r.height())
                            : QRect(r.x(), r.y(), r.width(), PROGRESS_CHUNK_WIDTH));

                if(IS_FLAT(bevApp))
                    pixPainter.fillRect(r2, use[1]);
                else
                    drawBevelGradientReal(use[1], &pixPainter, r2, horiz, false, bevApp,
                                          WIDGET_PROGRESSBAR);
                break;
            }
            case STRIPE_DIAGONAL:
            {
                QRegion reg;
                int     size(vertical ? origRect.width() : origRect.height());

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
                    pixPainter.fillRect(r, use[1]);
                else
                    drawBevelGradientReal(use[1], &pixPainter, r, horiz, false, bevApp, WIDGET_PROGRESSBAR);
            }
        }

        pixPainter.end();
        int cost(pix->width()*pix->height()*(pix->depth()/8));

        if(cost<itsPixmapCache.maxCost())
            itsPixmapCache.insert(key, pix, cost);
        else
            inCache=false;
    }
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

    p->save();
    p->setClipRect(origRect, Qt::IntersectClip);
    p->drawTiledPixmap(fillRect, *pix);
    p->restore();

    if(!inCache)
        delete pix;
}

void QtCurveStyle::drawBevelGradient(const QColor &base, QPainter *p, const QRect &origRect, const QPainterPath &path,
                                     bool horiz, bool sel, EAppearance bevApp, EWidget w, bool useCache) const
{
    if(origRect.width()<1 || origRect.height()<1)
        return;

    if(IS_FLAT(bevApp))
    {
        if((WIDGET_TAB_TOP!=w && WIDGET_TAB_BOT!=w) || IS_FLAT(opts.bgndAppearance) || opts.tabBgnd || !sel)
            if(path.isEmpty())
                p->fillRect(origRect, base);
            else
                p->fillPath(path, base);
    }
    else
    {
        bool        tab(WIDGET_TAB_TOP==w || WIDGET_TAB_BOT==w),
                    selected(tab ? false : sel);
        EAppearance app(selected
                            ? opts.sunkenAppearance
                            : WIDGET_LISTVIEW_HEADER==w && APPEARANCE_BEVELLED==bevApp
                                ? APPEARANCE_LV_BEVELLED
                                : APPEARANCE_BEVELLED!=bevApp || WIDGET_BUTTON(w) || WIDGET_LISTVIEW_HEADER==w ||
                                  WIDGET_TROUGH==w || WIDGET_NO_ETCH_BTN==w || WIDGET_MENU_BUTTON==w
                                    ? bevApp
                                    : APPEARANCE_GRADIENT);

        if(WIDGET_PROGRESSBAR==w || !useCache)
            drawBevelGradientReal(base, p, origRect, path, horiz, sel, app, w);
        else
        {
            QRect   r(0, 0, horiz ? QTC_PIXMAP_DIMENSION : origRect.width(),
                            horiz ? origRect.height() : QTC_PIXMAP_DIMENSION);
            QtcKey  key(createKey(horiz ? r.height() : r.width(), base, horiz, app, w));
            QPixmap *pix(itsPixmapCache.object(key));
            bool    inCache(true);

            if(!pix)
            {
                pix=new QPixmap(r.width(), r.height());
                pix->fill(Qt::transparent);

                QPainter pixPainter(pix);

                drawBevelGradientReal(base, &pixPainter, r, horiz, sel, app, w);
                pixPainter.end();

                int cost(pix->width()*pix->height()*(pix->depth()/8));

                if(cost<itsPixmapCache.maxCost())
                    itsPixmapCache.insert(key, pix, cost);
                else
                    inCache=false;
            }

            if(!path.isEmpty())
            {
                p->save();
                p->setClipPath(path, Qt::IntersectClip);
            }
            //if(path.isEmpty())
                p->drawTiledPixmap(origRect, *pix);
            //else
            //    p->fillPath(path, QBrush(*pix));  // Does nto work well :-(
            if(!path.isEmpty())
                p->restore();
            if(!inCache)
                delete pix;
        }
    }
}

void QtCurveStyle::drawBevelGradientReal(const QColor &base, QPainter *p, const QRect &r, const QPainterPath &path,
                                         bool horiz, bool sel, EAppearance app, EWidget w) const
{
    bool                             topTab(WIDGET_TAB_TOP==w),
                                     botTab(WIDGET_TAB_BOT==w);
    const Gradient                   *grad=getGradient(app, &opts);
    QLinearGradient                  g(r.topLeft(), horiz ? r.bottomLeft() : r.topRight());
    GradientStopCont::const_iterator it(grad->stops.begin()),
                                     end(grad->stops.end());
    int                              numStops(grad->stops.size());

    for(int i=0; it!=end; ++it, ++i)
    {
        QColor col;

        if(/*sel && */(topTab || botTab) && i==numStops-1)
        {
            col=base;
            if(sel && !IS_FLAT(opts.bgndAppearance) && 0==opts.tabBgnd)
                col.setAlphaF(0.0);
        }
        else
            shade(base, &col, botTab ? qMax(INVERT_SHADE((*it).val), 0.9) : (*it).val);
        g.setColorAt(botTab ? 1.0-(*it).pos : (*it).pos, col);
    }
    //p->fillRect(r, base);
    if(path.isEmpty())
        p->fillRect(r, QBrush(g));
    else
        p->fillPath(path, QBrush(g));
}

void QtCurveStyle::drawLightBevel(QPainter *p, const QRect &r, const QStyleOption *option,
                                  const QWidget *widget, int round, const QColor &fill, const QColor *custom,
                                  bool doBorder, EWidget w) const
{
    if(WIDGET_PROGRESSBAR==w || WIDGET_SB_BUTTON==w || (WIDGET_SPIN==w && !opts.unifySpin) || !usePixmapCache)
        drawLightBevelReal(p, r, option, widget, round, fill, custom, doBorder, w, true, opts.round);
    else
    {
        static const int constMaxCachePixmap = 128;

        int    endSize=0,
               middleSize=8;
        bool   horiz(isHoriz(option, w)),
               circular(WIDGET_MDI_WINDOW_BUTTON==w && (opts.titlebarButtons&QTC_TITLEBAR_BUTTON_ROUND));
        double radius=0;
        ERound realRound=getWidgetRound(&opts, r.width(), r.height(), w);

        if(!circular)
        {
            switch(realRound)
            {
                case ROUND_SLIGHT:
                case ROUND_NONE:
                    endSize=WIDGET_SB_SLIDER==w && MO_PLASTIK==opts.coloredMouseOver && option->state&State_MouseOver ? 7 : 4;
                    break;
                case ROUND_FULL:
                    endSize=WIDGET_SB_SLIDER==w && MO_PLASTIK==opts.coloredMouseOver && option->state&State_MouseOver ? 7 : 5;
                    break;
                case ROUND_EXTRA:
                    endSize=7;
                    break;
                case ROUND_MAX:
                {
                    radius=getRadius(&opts, r.width(), r.height(), w, RADIUS_ETCH);
                    endSize=WIDGET_SB_SLIDER==w
                                ? qMax((opts.sliderWidth/2)+1, (int)(radius+1.5))
                                : (int)(radius+2.5);
                    middleSize=(QTC_MIN_ROUND_MAX_WIDTH-(endSize*2))+4;
                    break;
                }
            }
        }

        int size((2*endSize)+middleSize);

        if(size>constMaxCachePixmap)
            drawLightBevelReal(p, r, option, widget, round, fill, custom, doBorder, w, true, realRound);
        else
        {
            QString key;
            bool    small(circular || (horiz ? r.width() : r.height())<(2*endSize));
            QPixmap pix(small ? QSize(r.width(), r.height()) : QSize(horiz ? size : r.width(), horiz ? r.height() : size));
            uint    state(option->state&(State_Raised|State_Sunken|State_On|State_Horizontal|State_HasFocus|State_MouseOver|
                          (WIDGET_MDI_WINDOW_BUTTON==w || WIDGET_MDI_WINDOW==w || WIDGET_MDI_WINDOW_TITLE==w ? State_Active : State_None)));

            key.sprintf("qtc-%x-%d-%x-%x-%x-%x-%x", w, (int)realRound, pix.width(), pix.height(), state, fill.rgba(), (int)(radius*100));
            if(!QPixmapCache::find(key, pix))
            {
                pix.fill(Qt::transparent);

                QPainter pixPainter(&pix);
                ERound   oldRound=opts.round;
                opts.round=realRound;
                drawLightBevelReal(&pixPainter, QRect(0, 0, pix.width(), pix.height()), option, widget, round, fill, custom,
                                   doBorder, w, false, realRound);
                opts.round=oldRound;
                pixPainter.end();

                QPixmapCache::insert(key, pix);
            }

            if(small)
                p->drawPixmap(r.topLeft(), pix);
            else if(horiz)
            {
                int middle(qMin(r.width()-(2*endSize), middleSize));
                if(middle>0)
                    p->drawTiledPixmap(r.x()+endSize, r.y(), r.width()-(2*endSize), pix.height(), pix.copy(endSize, 0, middle, pix.height()));
                p->drawPixmap(r.x(), r.y(), pix.copy(0, 0, endSize, pix.height()));
                p->drawPixmap(r.x()+r.width()-endSize, r.y(), pix.copy(pix.width()-endSize, 0, endSize, pix.height()));
            }
            else
            {
                int middle(qMin(r.height()-(2*endSize), middleSize));
                if(middle>0)
                    p->drawTiledPixmap(r.x(), r.y()+endSize, pix.width(), r.height()-(2*endSize),
                                       pix.copy(0, endSize, pix.width(), middle));
                p->drawPixmap(r.x(), r.y(), pix.copy(0, 0, pix.width(), endSize));
                p->drawPixmap(r.x(), r.y()+r.height()-endSize, pix.copy(0, pix.height()-endSize, pix.width(), endSize));
            }
        }
    }
}

void QtCurveStyle::drawLightBevelReal(QPainter *p, const QRect &rOrig, const QStyleOption *option,
                                      const QWidget *widget, int round, const QColor &fill, const QColor *custom,
                                      bool doBorder, EWidget w, bool useCache, ERound realRound) const
{
    EAppearance  app(widgetApp(w, &opts, option->state&State_Active));

    if(APPEARANCE_RAISED==app && (WIDGET_MDI_WINDOW==w || WIDGET_MDI_WINDOW_TITLE==w))
        app=APPEARANCE_FLAT;

    QRect        r(rOrig);
    bool         bevelledButton((WIDGET_BUTTON(w) || WIDGET_NO_ETCH_BTN==w || WIDGET_MENU_BUTTON==w) && APPEARANCE_BEVELLED==app),
                 sunken(option->state &(/*State_Down | */State_On | State_Sunken)),
                 flatMdi(WIDGET_MDI_WINDOW_BUTTON==w &&
                         (opts.round==ROUND_MAX || opts.titlebarButtons&QTC_TITLEBAR_BUTTON_ROUND)),
                 lightBorder(!flatMdi && WIDGET_MDI_WINDOW!=w && WIDGET_MDI_WINDOW_TITLE!=w && QTC_DRAW_LIGHT_BORDER(sunken, w, app)),
                 draw3dfull(!flatMdi && !lightBorder && WIDGET_MDI_WINDOW!=w && WIDGET_MDI_WINDOW_TITLE!=w &&
                        QTC_DRAW_3D_FULL_BORDER(sunken, app)),
                 draw3d(!flatMdi && (draw3dfull || (
                            !lightBorder && WIDGET_MDI_WINDOW!=w && WIDGET_MDI_WINDOW_TITLE!=w &&
                            QTC_DRAW_3D_BORDER(sunken, app)))),
                 doColouredMouseOver(!sunken && doBorder && option->state&State_Enabled &&
                                     WIDGET_MDI_WINDOW_BUTTON!=w &&
                                     (WIDGET_SPIN!=w || !opts.unifySpinBtns) &&
                                     !(option->state&QTC_STATE_KWIN_BUTTON) &&
#ifdef QTC_DONT_COLOUR_MOUSEOVER_TBAR_BUTTONS
                                     !(option->state&QTC_STATE_TBAR_BUTTON) &&
#endif
                                     opts.coloredMouseOver && option->state&State_MouseOver &&
                                     WIDGET_PROGRESSBAR!=w &&
                                     (/*option->state&QTC_TOGGLE_BUTTON ||*/ !sunken)),
                 plastikMouseOver(doColouredMouseOver && MO_PLASTIK==opts.coloredMouseOver),
                 colouredMouseOver(doColouredMouseOver && WIDGET_MENU_BUTTON!=w &&
                                       (MO_COLORED==opts.coloredMouseOver ||
                                        MO_COLORED_THICK==opts.coloredMouseOver ||
                                              (MO_GLOW==opts.coloredMouseOver && !QTC_DO_EFFECT))),
                 doEtch(doBorder && ETCH_WIDGET(w) && QTC_DO_EFFECT),
                 horiz(isHoriz(option, w));
    const QColor *cols(custom ? custom : itsBackgroundCols),
                 *border(colouredMouseOver ? borderColors(option, cols) : cols);

    p->save();

    if(doEtch)
        r.adjust(1, 1, -1, -1);

    p->setRenderHint(QPainter::Antialiasing, true);

    if(r.width()>0 && r.height()>0)
    {
        if(WIDGET_PROGRESSBAR==w && STRIPE_NONE!=opts.stripedProgress)
            drawProgressBevelGradient(p, r.adjusted(1, 1, -1, -1), option, horiz, app);
        else
        {
            QRect br(r.adjusted(0, 0, 0,  WIDGET_MDI_WINDOW_TITLE==w ? 1 : 0));

            drawBevelGradient(fill, p, br,
                              buildPath(br, w, round, getRadius(&opts, br.width()-2, br.height()-2, w, RADIUS_INTERNAL)),
                              horiz, sunken, app, w, useCache);

            if(!sunken)
                if(plastikMouseOver && !sunken)
                {
                    if(WIDGET_SB_SLIDER==w)
                    {
                        int len(QTC_SB_SLIDER_MO_LEN(horiz ? r.width() : r.height())+1),
                            so(lightBorder ? QTC_SLIDER_MO_BORDER : 1),
                            eo(len+so),
                            col(QTC_SLIDER_MO_SHADE);

                        if(horiz)
                        {
                            drawBevelGradient(itsMouseOverCols[col], p, QRect(r.x()+so, r.y(), len, r.height()-1),
                                              horiz, sunken, app, w, useCache);
                            drawBevelGradient(itsMouseOverCols[col], p,
                                              QRect(r.x()+r.width()-eo, r.y(), len, r.height()-1), horiz, sunken, app, w, useCache);
                        }
                        else
                        {
                            drawBevelGradient(itsMouseOverCols[col], p, QRect(r.x(), r.y()+so, r.width()-1, len),
                                              horiz, sunken, app, w, useCache);
                            drawBevelGradient(itsMouseOverCols[col], p,
                                              QRect(r.x(), r.y()+r.height()-eo, r.width()-1, len), horiz, sunken, app, w, useCache);
                        }
                    }
                    else
                    {
                        bool horizontal((horiz && WIDGET_SB_BUTTON!=w)|| (!horiz && WIDGET_SB_BUTTON==w)),
                             thin(WIDGET_SB_BUTTON==w || WIDGET_SPIN==w || ((horiz ? r.height() : r.width())<16));

                        p->setPen(itsMouseOverCols[QTC_MO_PLASTIK_DARK(w)]);
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
                    }
                }
        }

        if(APPEARANCE_AGUA==app && !sunken)
        {
            QRectF ra(r.x()+0.5, r.y()+0.5, r.width(), r.height());
            double size=(QTC_MIN((horiz ? ra.height() : ra.width())/2.0, 16)),
                   rad=size/2.0;
            int    mod=4;

            if(horiz)
            {
                if(!(ROUNDED_LEFT&round))
                    ra.adjust(-8, 0, 0, 0);
                if(!(ROUNDED_RIGHT&round))
                    ra.adjust(0, 0, 8, 0);
            }
            else
            {
                if(!(ROUNDED_TOP&round))
                    ra.adjust(0, -8, 0, 0);
                if(!(ROUNDED_BOTTOM&round))
                    ra.adjust(0, 0, 0, 8);
            }

/*            if(WIDGET_MDI_WINDOW_BUTTON==w)
            {
                rad/=1.5;
                mod/=1.25;
                ra.adjust(0, 0, 0, -4);
            }
            else */if(realRound<ROUND_MAX || (!QTC_IS_MAX_ROUND_WIDGET(w) && !IS_SLIDER(w)))
            {
                rad/=2.0;
                mod=mod>>1;
            }

            QRectF          gr(horiz ? QRectF(ra.x()+mod, ra.y(), ra.width()-(mod*2)-1, size-1)
                                     : QRectF(ra.x(), ra.y()+mod, size-1, ra.height()-(mod*2)-1));
            QLinearGradient g(gr.topLeft(), horiz ? gr.bottomLeft() : gr.topRight());
            QColor          white(Qt::white);
            bool            mo(option->state&State_MouseOver && opts.highlightFactor);

            white.setAlphaF(mo ? (opts.highlightFactor>0 ? 0.95 : 0.85) : 0.9);
            g.setColorAt(0.0, white);
            white.setAlphaF(mo ? (opts.highlightFactor>0 ? 0.3 : 0.1) : 0.2);
            g.setColorAt(1.0, white);
            p->fillPath(buildPath(gr, w, round, rad), QBrush(g));
        }
    }

    r.adjust(1, 1, -1, -1);

    if(plastikMouseOver && !sunken)
    {
        bool thin(WIDGET_SB_BUTTON==w || WIDGET_SPIN==w || ((horiz ? r.height() : r.width())<16)),
             horizontal(WIDGET_SB_SLIDER==w ? !horiz : (horiz && WIDGET_SB_BUTTON!=w)|| (!horiz && WIDGET_SB_BUTTON==w));
        int  len(WIDGET_SB_SLIDER==w ? QTC_SB_SLIDER_MO_LEN(horiz ? r.width() : r.height())+2 : (thin ? 1 : 2));

        p->save();
        if(horizontal)
            p->setClipRect(r.x(), r.y()+len, r.width(), r.height()-(len*2));
        else
            p->setClipRect(r.x()+len, r.y(), r.width()-(len*2), r.height());
    }

    if(!colouredMouseOver && lightBorder)
    {
        p->setPen(cols[APPEARANCE_DULL_GLASS==app ? 1 : 0]);
        p->drawPath(buildPath(r, w, round, getRadius(&opts, r.width(), r.height(), w, RADIUS_INTERNAL)));
    }
    else if(colouredMouseOver || (opts.titlebarBorder && (WIDGET_MDI_WINDOW==w || WIDGET_MDI_WINDOW_TITLE==w)) ||
            (draw3d && option->state&State_Raised))
    {
        QPainterPath innerTlPath,
                     innerBrPath;
        int          dark(/*bevelledButton ? */2/* : 4*/);

        buildSplitPath(r, w, round,
                       getRadius(&opts, r.width(), r.height(), w, RADIUS_INTERNAL),
                       innerTlPath, innerBrPath);

        p->setPen(border[colouredMouseOver ? QTC_MO_STD_LIGHT(w, sunken) : (sunken ? dark : 0)]);
        if(colouredMouseOver || bevelledButton || draw3dfull)
        {
            p->drawPath(innerTlPath);
            p->setPen(border[colouredMouseOver ? QTC_MO_STD_DARK(w) : (sunken ? 0 : dark)]);
            p->drawPath(innerBrPath);
        }
        else
        {
            if((WIDGET_MDI_WINDOW==w || WIDGET_MDI_WINDOW_TITLE==w) && APPEARANCE_SHINY_GLASS==app)
                drawAaLine(p, r.x(), r.y()+1, r.x(), r.y()+r.height()-(WIDGET_MDI_WINDOW_TITLE==w ? 0 : 1));
            else
                p->drawPath(innerTlPath);
        }
    }
    if(plastikMouseOver && !sunken)
        p->restore();
    p->setRenderHint(QPainter::Antialiasing, false);

    if(doEtch)
    {
        if( !sunken &&
            ((WIDGET_OTHER!=w && WIDGET_SLIDER_TROUGH!=w && MO_GLOW==opts.coloredMouseOver && option->state&State_MouseOver) ||
            (WIDGET_DEF_BUTTON==w && IND_GLOW==opts.defBtnIndicator)))
            drawGlow(p, rOrig, WIDGET_DEF_BUTTON==w && option->state&State_MouseOver ? WIDGET_STD_BUTTON : w);
        else
            drawEtch(p, rOrig, widget, w, EFFECT_SHADOW==opts.buttonEffect && WIDGET_BUTTON(w) && !sunken);
    }

    if(doBorder)
    {
        const QColor *borderCols=(WIDGET_COMBO==w || WIDGET_COMBO_BUTTON==w) && border==itsComboBtnCols
                            ? option->state&State_MouseOver && MO_GLOW==opts.coloredMouseOver && !sunken
                                ? itsMouseOverCols
                                : itsButtonCols
                            : cols;

        r.adjust(-1, -1, 1, 1);
        if(!sunken && option->state&State_Enabled &&
            ( ( ( (doEtch && WIDGET_OTHER!=w && WIDGET_SLIDER_TROUGH!=w) || WIDGET_SB_SLIDER==w || WIDGET_COMBO==w || WIDGET_MENU_BUTTON==w ) &&
                 (MO_GLOW==opts.coloredMouseOver/* || MO_COLORED==opts.colorMenubarMouseOver*/) && option->state&State_MouseOver) ||
               (doEtch && WIDGET_DEF_BUTTON==w && IND_GLOW==opts.defBtnIndicator)))
            drawBorder(p, r, option, round,
                       WIDGET_DEF_BUTTON==w && IND_GLOW==opts.defBtnIndicator && !(option->state&State_MouseOver)
                            ? itsDefBtnCols : itsMouseOverCols, w);
        else
            drawBorder(p, r, option, round,
                       colouredMouseOver && MO_COLORED_THICK==opts.coloredMouseOver ? itsMouseOverCols : borderCols, w);
    }

    p->restore();
}

void QtCurveStyle::drawGlow(QPainter *p, const QRect &r, EWidget w) const
{
    bool   def(WIDGET_DEF_BUTTON==w && IND_GLOW==opts.defBtnIndicator),
           defShade=def && (!itsDefBtnCols ||
                            (itsDefBtnCols[ORIGINAL_SHADE]==itsMouseOverCols[ORIGINAL_SHADE]));
    QColor col(def && itsDefBtnCols
                    ? itsDefBtnCols[QTC_GLOW_DEFBTN] : itsMouseOverCols[QTC_GLOW_MO]);

    col.setAlphaF(QTC_GLOW_ALPHA(defShade));
    p->setBrush(Qt::NoBrush);
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setPen(col);
    p->drawPath(buildPath(r, w, ROUNDED_ALL, getRadius(&opts, r.width(), r.height(), w, RADIUS_ETCH)));
    p->setRenderHint(QPainter::Antialiasing, false);
}

void QtCurveStyle::drawEtch(QPainter *p, const QRect &r, const QWidget *widget,  EWidget w, bool raised) const
{
    QPainterPath tl,
                 br;
    QColor       col(Qt::black);

    buildSplitPath(r, w, ROUNDED_ALL, getRadius(&opts, r.width(), r.height(), w, RADIUS_ETCH), tl, br);

    col.setAlphaF(QTC_ETCH_TOP_ALPHA);
    p->setBrush(Qt::NoBrush);
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setPen(col);

    if(!raised)
    {
        p->drawPath(tl);
        if(WIDGET_SLIDER_TROUGH==w && opts.thinSbarGroove && widget && qobject_cast<const QScrollBar *>(widget))
        {
            QColor col(Qt::white);
            col.setAlphaF(0.25);
            p->setPen(col);
        }
        else
            p->setPen(getLowerEtchCol(widget));
    }

    p->drawPath(br);
    p->setRenderHint(QPainter::Antialiasing, false);
}

void QtCurveStyle::drawWindowBackground(QWidget *widget) const
{
    QPainter      p(widget);
    const QWidget *window = widget->window();
    // get coordinates relative to the client area
    const QWidget *w = widget;
    int           y = 0;

    while (!w->isWindow())
    {
        y += w->geometry().y();
        w = w->parentWidget();
    }

    p.setClipRegion(widget->rect(), Qt::IntersectClip);

    drawBevelGradientReal(window->palette().window().color(), &p,
                          QRect(widget->rect().x(), y, widget->rect().width(), window->rect().height()),
                          GT_HORIZ==opts.bgndGrad, false, opts.bgndAppearance, WIDGET_OTHER);
}

QPainterPath QtCurveStyle::buildPath(const QRectF &r, EWidget w, int round, double radius) const
{
    QPainterPath path;

    if(WIDGET_MDI_WINDOW_BUTTON==w && opts.titlebarButtons&QTC_TITLEBAR_BUTTON_ROUND)
    {
        path.addEllipse(r);
        return path;
    }

    if(ROUND_NONE==opts.round)
        round=ROUNDED_NONE;

    double       diameter(radius*2);

    if (WIDGET_MDI_WINDOW_TITLE!=w && round&CORNER_BR)
        path.moveTo(r.x()+r.width(), r.y()+r.height()-radius);
    else
        path.moveTo(r.x()+r.width(), r.y()+r.height());

    if (round&CORNER_TR)
        path.arcTo(r.x()+r.width()-diameter, r.y(), diameter, diameter, 0, 90);
    else
        path.lineTo(r.x()+r.width(), r.y());

    if (round&CORNER_TL)
        path.arcTo(r.x(), r.y(), diameter, diameter, 90, 90);
    else
        path.lineTo(r.x(), r.y());

    if (WIDGET_MDI_WINDOW_TITLE!=w && round&CORNER_BL)
        path.arcTo(r.x(), r.y()+r.height()-diameter, diameter, diameter, 180, 90);
    else
        path.lineTo(r.x(), r.y()+r.height());

    if(WIDGET_MDI_WINDOW_TITLE!=w)
        if (round&CORNER_BR)
            path.arcTo(r.x()+r.width()-diameter, r.y()+r.height()-diameter, diameter, diameter, 270, 90);
        else
            path.lineTo(r.x()+r.width(), r.y()+r.height());

    return path;
}

QPainterPath QtCurveStyle::buildPath(const QRect &r, EWidget w, int round, double radius) const
{
    return buildPath(QRectF(r.x()+0.5, r.y()+0.5, r.width()-1, r.height()-1), w, round, radius);
}

void QtCurveStyle::buildSplitPath(const QRect &r, EWidget w, int round, double radius, QPainterPath &tl, QPainterPath &br) const
{
    bool         window(WIDGET_MDI_WINDOW==w || WIDGET_MDI_WINDOW_TITLE==w);
// #if QT_VERSION >= 0x040400
//     double       xd(window ? r.x() : (r.x()+0.5)),
//                  yd(window ? r.y() : (r.y()+0.5));
// #else
    double       xd(r.x()+0.5),
                 yd(r.y()+0.5);
// #endif
    double       diameter(radius*2);
    bool         rounded=diameter>0.0;
    int          width(r.width()-1),
                 height(r.height()-1);

    if (rounded && !window && round&CORNER_TR)
    {
        tl.arcMoveTo(xd+width-diameter, yd, diameter, diameter, 36);
        tl.arcTo(xd+width-diameter, yd, diameter, diameter, 36, 36);
    }
    else
        tl.moveTo(xd+width, yd);

    if (rounded && !window && round&CORNER_TL)
        tl.arcTo(xd, yd, diameter, diameter, 90, 90);
    else
        tl.lineTo(xd, yd);

    if (rounded && !window && round&CORNER_BL)
    {
        tl.arcTo(xd, yd+height-diameter, diameter, diameter, 180, 36);
        br.arcMoveTo(xd, yd+height-diameter, diameter, diameter, 180+36);
        br.arcTo(xd, yd+height-diameter, diameter, diameter, 180+36, 54);
    }
    else
    {
        tl.lineTo(xd, yd+height);
        br.moveTo(xd, yd+height);
    }

    if (rounded && !window && round&CORNER_BR)
        br.arcTo(xd+width-diameter, yd+height-diameter, diameter, diameter, 270, 90);
    else
        br.lineTo(xd+width, yd+height);

    if (rounded && !window && round&CORNER_TR)
        br.arcTo(xd+width-diameter, yd, diameter, diameter, 0, 54);
    else
        br.lineTo(xd+width, yd);
}

void QtCurveStyle::drawBorder(QPainter *p, const QRect &r, const QStyleOption *option,
                              int round, const QColor *custom, EWidget w,
                              EBorder borderProfile, bool doBlend, int borderVal) const
{
    if(ROUND_NONE==opts.round)
        round=ROUNDED_NONE;

    State        state(option->state);
    bool         enabled(state&State_Enabled),
                 entry(WIDGET_ENTRY==w || (WIDGET_SCROLLVIEW==w && opts.highlightScrollViews)),
                 hasFocus(enabled && entry && state&State_HasFocus),
                 hasMouseOver(enabled && WIDGET_ENTRY==w && state&State_MouseOver),
                 window(WIDGET_MDI_WINDOW==w || WIDGET_MDI_WINDOW_TITLE==w);
    const QColor *cols(enabled && hasMouseOver && opts.coloredMouseOver && entry
                        ? itsMouseOverCols
                        : enabled && hasFocus && itsFocusCols && entry
                              ? itsFocusCols
                              : custom
                                ? custom
                                : APP_KRUNNER==theThemedApp ? itsBackgroundCols : backgroundColors(option));
    QColor       border(WIDGET_DEF_BUTTON==w && IND_FONT_COLOR==opts.defBtnIndicator && enabled
                          ? option->palette.buttonText().color()
                          : cols[WIDGET_PROGRESSBAR==w
                                    ? QT_PBAR_BORDER
                                    : !enabled && (WIDGET_BUTTON(w) || WIDGET_SLIDER_TROUGH==w)
                                        ? QT_DISABLED_BORDER
                                            : itsMouseOverCols==cols && IS_SLIDER(w)
                                                ? QT_SLIDER_MO_BORDER
                                                : borderVal]);

//     if(!window)
        p->setRenderHint(QPainter::Antialiasing, true);

    p->setBrush(Qt::NoBrush);

    if(!window || opts.titlebarBorder)
    {
        if(WIDGET_TAB_BOT==w || WIDGET_TAB_TOP==w)
            cols=itsBackgroundCols;

        switch(borderProfile)
        {
            case BORDER_FLAT:
                break;
            case BORDER_RAISED:
            case BORDER_SUNKEN:
            case BORDER_LIGHT:
            {
                EAppearance  app(widgetApp(w, &opts));
                int          dark=window ? ORIGINAL_SHADE : QT_FRAME_DARK_SHADOW;

                if(APPEARANCE_FLAT==app && window)
                    app=APPEARANCE_RAISED;

                QColor       tl(cols[BORDER_RAISED==borderProfile || BORDER_LIGHT==borderProfile ? 0 : dark]),
                             br(cols[BORDER_RAISED==borderProfile ? dark : 0]);
                QPainterPath topPath,
                             botPath;

                if( ((hasMouseOver || hasFocus) && WIDGET_ENTRY==w) ||
                    (hasFocus && WIDGET_SCROLLVIEW==w))
                {
                    tl.setAlphaF(QTC_ENTRY_INNER_ALPHA);
                    br.setAlphaF(QTC_ENTRY_INNER_ALPHA);
                }
                else if(doBlend && !window)
                {
                    tl.setAlphaF(QTC_BORDER_BLEND_ALPHA);
                    br.setAlphaF(BORDER_SUNKEN ? 0.0 : QTC_BORDER_BLEND_ALPHA);
                }

    //             if(window)
    //                 tl.setAlphaF(0.5);

                QRect inner(r.adjusted(1, 1, -1, -1));

                buildSplitPath(inner, w, round, getRadius(&opts, inner.width(), inner.height(), w, RADIUS_INTERNAL),
                               topPath, botPath);

                p->setPen((enabled || BORDER_SUNKEN==borderProfile) &&
                          (BORDER_RAISED==borderProfile || hasFocus || APPEARANCE_FLAT!=app)
                                ? tl
                                : option->palette.background().color());
                p->drawPath(topPath);
                if(!hasFocus && !hasMouseOver && BORDER_LIGHT!=borderProfile)
                    p->setPen(WIDGET_SCROLLVIEW==w && !hasFocus
                                ? option->palette.background().color()
                                : WIDGET_ENTRY==w && !hasFocus
                                    ? option->palette.base().color()
                                    : enabled && (BORDER_SUNKEN==borderProfile || hasFocus || APPEARANCE_FLAT!=app ||
                                      WIDGET_TAB_TOP==w || WIDGET_TAB_BOT==w)
                                        ? br
                                        : option->palette.background().color());
                p->drawPath(botPath);
            }
        }
    }

    if(BORDER_SUNKEN==borderProfile &&
       (WIDGET_FRAME==w || ((WIDGET_ENTRY==w || WIDGET_SCROLLVIEW==w) && !opts.etchEntry && !hasFocus && !hasMouseOver)))
    {
        QPainterPath topPath,
                     botPath;
        QColor       col(border);

        col.setAlphaF(QT_LOWER_BORDER_ALPHA);
        buildSplitPath(r, w, round, getRadius(&opts, r.width(), r.height(), w, RADIUS_EXTERNAL), topPath, botPath);
        p->setPen(enabled ? border : col);
        p->drawPath(topPath);
        if(enabled)
            p->setPen(col);
        p->drawPath(botPath);
        if(enabled)
            p->setPen(border);
    }
    else
    {
        p->setPen(window && state&QtC_StateKWinHighlight ? itsFocusCols[0] : border);
        p->drawPath(buildPath(r, w, round, getRadius(&opts, r.width(), r.height(), w, RADIUS_EXTERNAL)));
    }

//     if(!window)
        p->setRenderHint(QPainter::Antialiasing, false);

    if(QTC_FULLLY_ROUNDED && window)
    {
        if(WIDGET_MDI_WINDOW==w)
        {
            p->drawPoint(r.x(), r.y()+r.height()-2);
            p->drawPoint(r.x()+1, r.y()+r.height()-1);
            p->drawPoint(r.x()+r.width()-2, r.y()+r.height()-1);
            p->drawPoint(r.x()+r.width()-1, r.y()+r.height()-2);
//             p->setPen(cols[ORIGINAL_SHADE]);
//             p->drawPoint(r.x()+1, r.y()+r.height()-2);
        }

        if(WIDGET_MDI_WINDOW_TITLE==w)
        {
            p->drawPoint(r.x()+2, r.y());
            p->drawPoint(r.x(), r.y()+2);
            p->drawPoint(r.x()+r.width()-1, r.y()+2);
            p->drawPoint(r.x()+r.width()-3, r.y());
//             p->setPen(cols[0]);
//             p->drawPoint(r.x()+1, r.y()+2);
//             p->drawPoint(r.x()+2, r.y()+1);
        }
    }
}

void QtCurveStyle::drawMdiControl(QPainter *p, const QStyleOptionTitleBar *titleBar, SubControl sc, const QWidget *widget,
                                  ETitleBarButtons btn, const QColor &textColor, const QColor &shadow,
                                  const QColor *btnCols, const QColor *bgndCols) const
{
    QRect rect(subControlRect(CC_TitleBar, titleBar, sc, widget));

    if (rect.isValid())
    {
        bool sunken((titleBar->activeSubControls&sc) && (titleBar->state&State_Sunken)),
             hover((titleBar->activeSubControls&sc) && (titleBar->state&State_MouseOver)),
             colored=coloredMdiButtons(titleBar->state&State_Active, hover),
             useBtnCols(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_STD_COLOR &&
                         (hover ||
                          !(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR_MOUSE_OVER) ||
                          opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR));

        drawMdiButton(p, rect, hover, sunken,
                      colored && !(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR_SYMBOL)
                        ? itsTitleBarButtonsCols[btn] : (useBtnCols ? btnCols : bgndCols));
        drawMdiIcon(p, colored && opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR_SYMBOL
                        ? itsTitleBarButtonsCols[btn][ORIGINAL_SHADE]
                        : (SC_TitleBarCloseButton==sc && !(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR) && (hover || sunken) ? CLOSE_COLOR : textColor),
                    shadow, rect, hover, sunken, sc);
    }
}

void QtCurveStyle::drawMdiButton(QPainter *painter, const QRect &r, bool hover, bool sunken, const QColor *cols) const
{
    if(!(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_NO_FRAME) &&
       (hover || sunken || !(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_HOVER_FRAME)))
    {
        QStyleOption opt;

        opt.rect=r; // .adjusted(1, 1, -1, -1);
        if(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_ROUND)
            opt.rect.adjust(1, 1, -1, -1);
        opt.state=State_Enabled|State_Horizontal|State_Raised;
        if(hover)
            opt.state|=State_MouseOver;
        if(sunken)
            opt.state|=State_Sunken;

        drawLightBevel(painter, opt.rect, &opt, 0L, ROUNDED_ALL, getFill(&opt, cols), cols, true,
                       WIDGET_MDI_WINDOW_BUTTON);
    }
}

void QtCurveStyle::drawMdiIcon(QPainter *painter, const QColor &color, const QColor &shadow, const QRect &r,
                               bool hover, bool sunken, SubControl button) const
{
    if(!sunken) // && hover && !(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_HOVER_SYMBOL) && !customCol)
        drawWindowIcon(painter, shadow, r.adjusted(1, 1, 1, 1), sunken, button);

    QColor col(color);

    if(!sunken && !hover && opts.titlebarButtons&QTC_TITLEBAR_BUTTON_HOVER_SYMBOL)
        col.setAlphaF(HOVER_BUTTON_ALPHA);

    drawWindowIcon(painter, col, r, sunken, button);
}

void QtCurveStyle::drawWindowIcon(QPainter *painter, const QColor &color, const QRect &r, bool sunken, SubControl button) const
{
    static const int constIconSize=9;

    QRect rect(r);

    int diff=(rect.height()-constIconSize)/2;
    rect.adjust(diff, diff, -diff, -diff);
    rect.setHeight(constIconSize);
    rect.setWidth(constIconSize);

    if(sunken)
        rect.adjust(1, 1, 1, 1);

    painter->setPen(color);

    switch(button)
    {
        case SC_TitleBarMinButton:
            drawRect(painter, QRect(rect.left(), rect.bottom()-1, rect.width(), 2));
            break;
        case SC_TitleBarMaxButton:
            drawRect(painter, rect);
            painter->drawLine(rect.left() + 1, rect.top() + 1,  rect.right() - 1, rect.top() + 1);
            break;
        case SC_TitleBarCloseButton:
        {
            static unsigned char xbm[] = { 0x83, 0x01, 0xc7, 0x01, 0xee, 0x00, 0x7c, 0x00, 0x38, 0x00, 0x7c, 0x00,
                                           0xee, 0x00, 0xc7, 0x01, 0x83, 0x01 };
            static QBitmap bitmap=QBitmap::fromData(QSize(constIconSize, constIconSize), xbm);
            painter->drawPixmap(rect.x()+(rect.width()-bitmap.width())/2, rect.y()+(rect.height()-bitmap.height())/2, bitmap);
            break;
        }
        case SC_TitleBarNormalButton:
        {
            static unsigned char xbm[] = {0xfc, 0x01, 0xfc, 0x01, 0x04, 0x01, 0x7f, 0x01, 0x7f, 0x01, 0xc1, 0x01,
                                          0x41, 0x00, 0x41, 0x00, 0x7f, 0x00 };
            static QBitmap bitmap=QBitmap::fromData(QSize(constIconSize, constIconSize), xbm);
            painter->drawPixmap(rect.x()+(rect.width()-bitmap.width())/2, rect.y()+(rect.height()-bitmap.height())/2, bitmap);
            break;
        }
        case SC_TitleBarShadeButton:
            drawArrow(painter, rect, PE_IndicatorArrowUp, color, false, true);
            break;
        case SC_TitleBarUnshadeButton:
            drawArrow(painter, rect, PE_IndicatorArrowDown, color, false, true);
            break;
        case SC_TitleBarSysMenu:
            for(int i=1; i<=constIconSize; i+=3)
                painter->drawLine(rect.left() + 1, rect.top() + i,  rect.right() - 1, rect.top() + i);
        default:
            break;
    }
}

void QtCurveStyle::drawEntryField(QPainter *p, const QRect &rx,  const QWidget *widget, const QStyleOption *option,
                                  int round, bool fill, bool doEtch, EWidget w) const
{
    QRect r(rx);

    if(doEtch && opts.etchEntry)
        r.adjust(1, 1, -1, -1);

    if(fill)
        p->fillPath(buildPath(r.adjusted(1, 1, -1, -1), WIDGET_ENTRY, round,
                              getRadius(&opts, r.width()-2, r.height()-2, WIDGET_ENTRY, RADIUS_INTERNAL)),
                    option->palette.brush(QPalette::Base));
    else
    {
        p->setRenderHint(QPainter::Antialiasing, true);
        p->setPen(option->palette.brush(QPalette::Base).color());
        p->drawPath(buildPath(r.adjusted(1, 1, -1, -1), WIDGET_ENTRY, round,
                              getRadius(&opts, r.width()-2, r.height()-2, WIDGET_ENTRY, RADIUS_INTERNAL)));
        p->setRenderHint(QPainter::Antialiasing, false);
    }

    if(doEtch && opts.etchEntry)
        drawEtch(p, rx, widget, WIDGET_ENTRY, false);

    drawBorder(p, r, option, round, 0L, w, BORDER_SUNKEN);
}

void QtCurveStyle::drawMenuItem(QPainter *p, const QRect &r, const QStyleOption *option, bool mbi, int round, const QColor *cols) const
{
    int fill=opts.useHighlightForMenu && (!mbi || itsHighlightCols==cols) ? ORIGINAL_SHADE : 4,
        border=opts.borderMenuitems ? 0 : fill;

    if(itsHighlightCols!=cols && mbi && !(option->state&(State_On|State_Sunken)) &&
       !opts.colorMenubarMouseOver && (opts.borderMenuitems || !IS_FLAT(opts.menuitemAppearance)))
        fill=ORIGINAL_SHADE;

    if(!mbi && APPEARANCE_FADE==opts.menuitemAppearance)
    {
        bool  reverse=Qt::RightToLeft==option->direction;
        int   roundOffet=QTC_ROUNDED ? 1 : 0;
        QRect main(r.adjusted(reverse ? 1+MENUITEM_FADE_SIZE : roundOffet+1, roundOffet+1,
                              reverse ? -(roundOffet+1) : -(1+MENUITEM_FADE_SIZE), -(roundOffet+1))),
              fade(reverse ? r.x()+1 : r.width()-MENUITEM_FADE_SIZE, r.y()+1, MENUITEM_FADE_SIZE, r.height()-2);

        p->fillRect(main, cols[fill]);
        if(QTC_ROUNDED)
        {
            QStyleOption opt(*option);

            opt.state|=State_Horizontal|State_Raised;
            opt.state&=~(State_Sunken|State_On);

            drawBorder(p, main.adjusted(-1, -1, 1, 1), &opt, reverse ? ROUNDED_RIGHT : ROUNDED_LEFT,
                       cols, WIDGET_MENU_ITEM, BORDER_FLAT, false, fill);
        }

        QLinearGradient grad(fade.topLeft(), fade.topRight());

        grad.setColorAt(0, reverse ? option->palette.background().color() : cols[fill]);
        grad.setColorAt(1, reverse ? cols[fill] : option->palette.background().color());
        p->fillRect(fade, QBrush(grad));
    }
    else if(mbi || opts.borderMenuitems)
    {
        bool stdColor(!mbi || (SHADE_BLEND_SELECTED!=opts.shadeMenubars && SHADE_SELECTED!=opts.shadeMenubars));

        QStyleOption opt(*option);

        opt.state|=State_Horizontal|State_Raised;
        opt.state&=~(State_Sunken|State_On);

        if(stdColor && opts.borderMenuitems)
            drawLightBevel(p, r, &opt, 0L, round, cols[fill], cols, stdColor, WIDGET_MENU_ITEM);
        else
        {
            QRect fr(r);

            fr.adjust(1, 1, -1, -1);

            if(fr.width()>0 && fr.height()>0)
                drawBevelGradient(cols[fill], p, fr, true,
                                  false, opts.menuitemAppearance, WIDGET_MENU_ITEM);
            drawBorder(p, r, &opt, round, cols, WIDGET_MENU_ITEM, BORDER_FLAT, false, border);
        }
    }
    else
        drawBevelGradient(cols[fill], p, r, true,
                          false, opts.menuitemAppearance, WIDGET_MENU_ITEM);

}

void QtCurveStyle::drawProgress(QPainter *p, const QRect &r, const QStyleOption *option, int round, bool vertical, bool reverse) const
{
    QStyleOption opt(*option);
    QRect        rx(r);

    opt.state|=State_Raised;

    if(vertical)
        opt.state&=~State_Horizontal;
    else
        opt.state|=State_Horizontal;

    if(reverse)
        opt.state|=QTC_STATE_REVERSE;
    else
        opt.state&=~QTC_STATE_REVERSE;

    if((vertical ? r.height() : r.width())<1)
        return;

    if(vertical && r.height()<3)
        rx.setHeight(3);

    if(!vertical && rx.width()<3)
        rx.setWidth(3);

    int          length(vertical ? rx.height() : rx.width());
    const QColor *use=option->state&State_Enabled || ECOLOR_BACKGROUND==opts.progressGrooveColor
                    ? itsHighlightCols : itsBackgroundCols;

    drawLightBevel(p, rx, &opt, 0L, opts.fillProgress ? ROUNDED_ALL : round, use[ORIGINAL_SHADE], use, true,
                   WIDGET_PROGRESSBAR);

    if(!opts.fillProgress && QTC_ROUNDED && length>2 && ROUNDED_ALL!=round)
    {
        bool drawFull(length > 3);

        p->setPen(midColor(option->palette.background().color(), itsHighlightCols[QT_PBAR_BORDER]));
        if(!(round&CORNER_TL) || !drawFull)
            p->drawPoint(rx.x(), rx.y());
        if(!(round&CORNER_BL) || !drawFull)
            p->drawPoint(rx.x(), rx.y()+rx.height()-1);
        if(!(round&CORNER_TR) || !drawFull)
            p->drawPoint(rx.x()+rx.width()-1, rx.y());
        if(!(round&CORNER_BR) || !drawFull)
            p->drawPoint(rx.x()+rx.width()-1, rx.y()+rx.height()-1);
    }
}

void QtCurveStyle::drawArrow(QPainter *p, const QRect &r, PrimitiveElement pe, QColor col, bool small, bool mdi) const
{
    QPolygon     a;
    QPainterPath path;

    if(small)
        switch(pe)
        {
            case PE_IndicatorArrowUp:
                a.setPoints(opts.vArrows ? 6 : 3,  2,0,  0,-2,  -2,0,   -2,1, 0,-1, 2,1);
                break;
            case PE_IndicatorArrowDown:
                a.setPoints(opts.vArrows ? 6 : 3,  2,0,  0,2,  -2,0,   -2,-1, 0,1, 2,-1);
                break;
            case PE_IndicatorArrowRight:
                a.setPoints(opts.vArrows ? 6 : 3,  0,-2,  2,0,  0,2,   -1,2, 1,0, -1,-2);
                break;
            case PE_IndicatorArrowLeft:
                a.setPoints(opts.vArrows ? 6 : 3,  0,-2,  -2,0,  0,2,   1,2, -1,0, 1,-2);
                break;
            default:
                return;
        }
    else // Large arrows...
        switch(pe)
        {
            case PE_IndicatorArrowUp:
                a.setPoints(opts.vArrows ? 8 : 3,  3,1,  0,-2,  -3,1,    -3,2,  -2,2, 0,0,  2,2, 3,2);
                break;
            case PE_IndicatorArrowDown:
                a.setPoints(opts.vArrows ? 8 : 3,  3,-1,  0,2,  -3,-1,   -3,-2,  -2,-2, 0,0, 2,-2, 3,-2);
                break;
            case PE_IndicatorArrowRight:
                a.setPoints(opts.vArrows ? 8 : 3,  -1,-3,  2,0,  -1,3,   -2,3, -2,2, 0,0, -2,-2, -2,-3);
                break;
            case PE_IndicatorArrowLeft:
                a.setPoints(opts.vArrows ? 8 : 3,  1,-3,  -2,0,  1,3,    2,3, 2,2, 0,0, 2,-2, 2,-3);
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
    p->save();
    if(!mdi)
        col.setAlpha(255);
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setPen(col);
    p->setBrush(col);
    p->fillPath(path, col);
    p->setRenderHint(QPainter::Antialiasing, false);
    p->drawPolygon(a);
    p->restore();
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

    drawLightBevel(p, r, &opt, 0L, slider
#ifndef QTC_SIMPLE_SCROLLBARS
                   || SCROLLBAR_NONE==opts.scrollbarType || opts.flatSbarButtons
#endif
                    ? ROUNDED_ALL : ROUNDED_NONE,
                   getFill(&opt, use, false, SHADE_DARKEN==opts.shadeSliders), use, true, WIDGET_SB_SLIDER);

    const QColor *markers(/*opts.coloredMouseOver && opt.state&State_MouseOver
                              ? itsMouseOverCols
                              : */use);

    if(opt.state&State_Horizontal)
        r.setX(r.x()+1);
    else
        r.setY(r.y()+1);

    if(LINE_NONE!=opts.sliderThumbs && (slider || ((opt.state&State_Horizontal && r.width()>=min)|| r.height()>=min)))
        switch(opts.sliderThumbs)
        {
//             case LINE_1DOT:
//                 p->drawPixmap(r.x()+((r.width()-5)/2), r.y()+((r.height()-5)/2), *getPixmap(markers[QT_STD_BORDER], PIX_DOT, 1.0));
//                 break;
            case LINE_FLAT:
                drawLines(p, r, !(opt.state&State_Horizontal), 3, 5, markers, 0, 5, opts.sliderThumbs);
                break;
            case LINE_SUNKEN:
                drawLines(p, r, !(opt.state&State_Horizontal), 4, 3, markers, 0, 3, opts.sliderThumbs);
                break;
            case LINE_DOTS:
            default:
                drawDots(p, r, !(opt.state&State_Horizontal), slider ? 3 : 5, slider ? 5 : 2, markers, 0, 5);
        }
}

void QtCurveStyle::drawSliderHandle(QPainter *p, const QRect &r, const QStyleOptionSlider *option) const
{
    bool horiz(SLIDER_TRIANGULAR==opts.sliderStyle ? r.height()>r.width() : r.width()>r.height());

    if(SLIDER_TRIANGULAR==opts.sliderStyle ||
       ((SLIDER_ROUND==opts.sliderStyle || SLIDER_ROUND_ROTATED==opts.sliderStyle) && QTC_FULLLY_ROUNDED))
    {
        QStyleOption opt(*option);

        if(r.width()>r.height())
            opt.state|=State_Horizontal;
        opt.state&=~(State_Sunken|State_On);
        if(!(option->activeSubControls&SC_SliderHandle) || !(opt.state&State_Enabled))
            opt.state&=~State_MouseOver;

        opt.state|=State_Raised;

        const QColor     *use(sliderColors(&opt)),
                         *border(opt.state&State_MouseOver && (MO_GLOW==opts.coloredMouseOver ||
                                                               MO_COLORED==opts.coloredMouseOver)
                                    ? itsMouseOverCols : use);
        const QColor     &fill(getFill(&opt, use, false, SHADE_DARKEN==opts.shadeSliders));
        int              x(r.x()),
                         y(r.y()),
                         xo(horiz ? 8 : 0),
                         yo(horiz ? 0 : 8);
        PrimitiveElement direction(horiz ? PE_IndicatorArrowDown : PE_IndicatorArrowRight);
        QPolygon         clipRegion;
        bool             drawLight(MO_PLASTIK!=opts.coloredMouseOver || !(opt.state&State_MouseOver) ||
                                   (SLIDER_ROUND==opts.sliderStyle &&
                                   (SHADE_BLEND_SELECTED==opts.shadeSliders || SHADE_SELECTED==opts.shadeSliders)));
        int              size(SLIDER_TRIANGULAR==opts.sliderStyle ? 15 : 13),
                         borderVal(itsMouseOverCols==border ? QT_SLIDER_MO_BORDER : QT_BORDER(opt.state&State_Enabled));

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
            drawBevelGradient(fill, p, QRect(x, y, horiz ? r.width()-1 : size, horiz ? size : r.height()-1),
                              horiz, false, MODIFY_AGUA(opts.sliderAppearance));

            if(MO_PLASTIK==opts.coloredMouseOver && opt.state&State_MouseOver)
            {
                int col(QTC_SLIDER_MO_SHADE),
                    len(QTC_SLIDER_MO_LEN);

                if(horiz)
                {
                    drawBevelGradient(itsMouseOverCols[col], p, QRect(x+1, y+1, len, size-2),
                                      horiz, false, MODIFY_AGUA(opts.sliderAppearance));
                    drawBevelGradient(itsMouseOverCols[col], p,  QRect(x+r.width()-(1+len), y+1, len, size-2),
                                      horiz, false, MODIFY_AGUA(opts.sliderAppearance));
                }
                else
                {
                    drawBevelGradient(itsMouseOverCols[col], p, QRect(x+1, y+1, size-2, len),
                                      horiz, false, MODIFY_AGUA(opts.sliderAppearance));
                    drawBevelGradient(itsMouseOverCols[col], p,QRect(x+1, y+r.height()-(1+len), size-2, len),
                                      horiz, false, MODIFY_AGUA(opts.sliderAppearance));
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

            p->setPen(border[borderVal]);
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
                          *getPixmap(border[borderVal], horiz ? PIX_SLIDER : PIX_SLIDER_V, 0.8));

            if(drawLight)
                p->drawPixmap(x, y, *getPixmap(use[0], horiz ? PIX_SLIDER_LIGHT : PIX_SLIDER_LIGHT_V));
        }
        p->restore();
    }
    else
    {
//         QRect sr(r);
//
//         if(horiz)
//             sr.adjust(0, 1, 0, 0);
//         else
//             sr.adjust(1, 0, 0, 0);

        QStyleOption opt(*option);

        if(QTC_ROTATED_SLIDER)
            opt.state^=State_Horizontal;

        drawSbSliderHandle(p, r, &opt, true);
   }
}

void QtCurveStyle::drawSliderGroove(QPainter *p, const QRect &groove, const QRect &handle,
                                    const QStyleOptionSlider *slider, const QWidget *widget) const
{
    bool               horiz(Qt::Horizontal==slider->orientation);
    QRect              grv(groove);
    QStyleOptionSlider opt(*slider);

//     if (horiz)
//         grv.adjust(0, 0, -1, 0);
//     else
//         grv.adjust(0, 0, 0, -1);

    opt.state&=~(State_HasFocus|State_On|State_Sunken|State_MouseOver);

    if(horiz)
    {
        int dh=(grv.height()-5)>>1;
        grv.adjust(0, dh, 0, -dh);
        opt.state|=State_Horizontal;

        if(QTC_DO_EFFECT)
            grv.adjust(0, -1, 0, 1);
    }
    else
    {
        int dw=(grv.width()-5)>>1;
        grv.adjust(dw, 0, -dw, 0);
        opt.state&=~State_Horizontal;

        if(QTC_DO_EFFECT)
            grv.adjust(-1, 0, 1, 0);
    }

    if(grv.height()>0 && grv.width()>0)
    {
        drawLightBevel(p, grv, &opt, widget, ROUNDED_ALL, itsBackgroundCols[slider->state&State_Enabled ? 2 : ORIGINAL_SHADE],
                       itsBackgroundCols, true, WIDGET_SLIDER_TROUGH);

        if(opts.fillSlider && slider->maximum!=slider->minimum && slider->state&State_Enabled)
        {
            const QColor *usedCols=itsSliderCols ? itsSliderCols : itsHighlightCols;

            if (horiz)
                if (slider->upsideDown)
                    grv=QRect(handle.right()-4, grv.top(), (grv.right()-handle.right())+4, grv.height());
                else
                    grv=QRect(grv.left(), grv.top(), handle.left()+4, grv.height());
            else
                if (slider->upsideDown)
                    grv=QRect(grv.left(), handle.bottom()-4, grv.width(), (grv.height() - handle.bottom())+4);
                else
                    grv=QRect(grv.left(), grv.top(), grv.width(), (handle.top() - grv.top())+4);

            if(grv.height()>0 && grv.width()>0)
                drawLightBevel(p, grv, &opt, widget, ROUNDED_ALL, usedCols[ORIGINAL_SHADE], usedCols, true, WIDGET_FILLED_SLIDER_TROUGH);
        }
    }
}

void QtCurveStyle::drawMenuOrToolBarBackground(QPainter *p, const QRect &r, const QStyleOption *option, bool menu, bool horiz) const
{
    if(IS_FLAT(opts.bgndAppearance) || !IS_FLAT(menu ? opts.menubarAppearance : opts.toolbarAppearance))
        drawBevelGradient(menu && itsActive && (option->state&State_Enabled || SHADE_NONE!=opts.shadeMenubars)
                            ? itsMenubarCols[ORIGINAL_SHADE]
                            : option->palette.background().color(),
                          p, r, horiz, false, menu ? opts.menubarAppearance : opts.toolbarAppearance);
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
        case LINE_NONE:
            break;
//         case LINE_1DOT:
//              p->drawPixmap(r.x()+((r.width()-5)/2), r.y()+((r.height()-5)/2), *getPixmap(border[QT_STD_BORDER], PIX_DOT, 1.0));
//             break;
        case LINE_DOTS:
            drawDots(p, r, !(option->state&State_Horizontal), 2,
                     tb ? 5 : 3, border, tb ? -2 : 0, 5);
            break;
        case LINE_DASHES:
            if(option->state&State_Horizontal)
            {
                QRect r1(r.x()+(tb ? 2 : (r.width()-6)/2), r.y(), 3, r.height());

                drawLines(p, r1, true, (r.height()-8)/2,
                          tb ? 0 : (r.width()-5)/2, border, 0, 5, handles);
            }
            else
            {
                QRect r1(r.x(), r.y()+(tb ? 2 : (r.height()-6)/2), r.width(), 3);

                drawLines(p, r1, false, (r.width()-8)/2,
                          tb ? 0 : (r.height()-5)/2, border, 0, 5, handles);
            }
            break;
        case LINE_FLAT:
            drawLines(p, r, !(option->state&State_Horizontal), 2,
                      tb ? 4 : 2, border, tb ? -2 : 0, 4, handles);
            break;
        default:
            drawLines(p, r, !(option->state&State_Horizontal), 2,
                      tb ? 4 : 2, border, tb ? -2 : 0, 3, handles);
    }
}

void QtCurveStyle::fillTab(QPainter *p, const QRect &r, const QStyleOption *option, const QColor &fill, bool horiz, EWidget tab) const
{
    bool   invertedSel=option->state&State_Selected && APPEARANCE_INVERTED==opts.appearance;
    QColor col(invertedSel ? option->palette.background().color() : fill);

    if(opts.tabBgnd)
        col=shade(col, QTC_TO_FACTOR(opts.tabBgnd));

    if(invertedSel)
        p->fillRect(r, col);
    else
    {
        bool        selected(option->state&State_Selected);
        EAppearance app(selected ? QTC_SEL_TAB_APP : QTC_NORM_TAB_APP);

        drawBevelGradient(col, p, r, horiz, selected, app, tab);
    }
}

void QtCurveStyle::colorTab(QPainter *p, const QRect &r, bool horiz, EWidget tab, int round) const
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    QLinearGradient grad(r.topLeft(), horiz ? r.bottomLeft() : r.topRight());
    QColor          start(itsHighlightCols[ORIGINAL_SHADE]),
                    end(itsHighlightCols[ORIGINAL_SHADE]);

    start.setAlphaF(QTC_COLOR_SEL_TAB_FACTOR);
    end.setAlphaF(0.0);
    grad.setColorAt(0, WIDGET_TAB_TOP==tab ? start : end);
    grad.setColorAt(1, WIDGET_TAB_TOP==tab ? end : start);
    p->fillPath(buildPath(r, tab, round, getRadius(&opts, r.width(), r.height(), tab, RADIUS_EXTERNAL)), grad);
    p->restore();
}

void QtCurveStyle::shadeColors(const QColor &base, QColor *vals) const
{
    QTC_SHADES

    bool   useCustom(QTC_USE_CUSTOM_SHADES(opts));
    double hl=QTC_TO_FACTOR(opts.highlightFactor);

    for(int i=0; i<NUM_STD_SHADES; ++i)
        shade(base, &vals[i], useCustom ? opts.customShades[i] : QTC_SHADE(opts.contrast, i));
    shade(base, &vals[SHADE_ORIG_HIGHLIGHT], hl);
    shade(vals[4], &vals[SHADE_4_HIGHLIGHT], hl);
    shade(vals[2], &vals[SHADE_2_HIGHLIGHT], hl);
    vals[ORIGINAL_SHADE]=base;
}

const QColor * QtCurveStyle::buttonColors(const QStyleOption *option) const
{
    if(option && option->version>=QTC_TBAR_VERSION_HACK &&
       option->version<QTC_TBAR_VERSION_HACK+NUM_TITLEBAR_BUTTONS &&
       coloredMdiButtons(option->state&State_Active, option->state&(State_MouseOver|State_Sunken)))
        return itsTitleBarButtonsCols[option->version-QTC_TBAR_VERSION_HACK];

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
                ? SHADE_NONE!=opts.shadeSliders && itsSliderCols &&
                  (!opts.colorSliderMouseOver || option->state&State_MouseOver)
                        ? itsSliderCols
                        : itsButtonCols //buttonColors(option)
                : itsBackgroundCols;
}

const QColor * QtCurveStyle::backgroundColors(const QColor &col) const
{
    if(col.alpha()!=0 && col!=itsBackgroundCols[ORIGINAL_SHADE])
    {
        shadeColors(col, itsColoredBackgroundCols);
        return itsColoredBackgroundCols;
    }

    return itsBackgroundCols;
}

const QColor * QtCurveStyle::borderColors(const QStyleOption *option, const QColor *use) const
{
    return opts.coloredMouseOver && option && option->state&State_MouseOver && option->state&State_Enabled
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
            shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE], itsButtonCols[ORIGINAL_SHADE]),
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
        case SHADE_BLEND_SELECTED:
            shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE], itsBackgroundCols[ORIGINAL_SHADE]), itsMenubarCols);
            break;
        case SHADE_SELECTED:
            shadeColors(IS_GLASS(opts.appearance)
                            ? shade(itsHighlightCols[ORIGINAL_SHADE], MENUBAR_GLASS_SELECTED_DARK_FACTOR)
                            : itsHighlightCols[ORIGINAL_SHADE],
                        itsMenubarCols);
            break;
        case SHADE_CUSTOM:
            shadeColors(opts.customMenubarsColor, itsMenubarCols);
            break;
        case SHADE_DARKEN:
            shadeColors(shade(bgnd, MENUBAR_DARK_FACTOR), itsMenubarCols);
    }
}

bool QtCurveStyle::coloredMdiButtons(bool active, bool mouseOver) const
{
    return opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR &&
            (active
                ? (mouseOver || !(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR_MOUSE_OVER))
                : ( (opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR_MOUSE_OVER && mouseOver) ||
                    (!(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR_MOUSE_OVER) &&
                       opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR_INACTIVE)) );
}

const QColor * QtCurveStyle::getMdiColors(const QStyleOption *option, bool active) const
{
    if(!itsActiveMdiColors)
    {
        itsActiveMdiTextColor=option->palette.highlightedText().color();
        itsMdiTextColor=option->palette.text().color();

#if !defined QTC_QT_ONLY
        QColor col=KGlobalSettings::activeTitleColor();

        if(col!=itsHighlightCols[ORIGINAL_SHADE])
        {
            itsActiveMdiColors=new QColor [TOTAL_SHADES+1];
            shadeColors(col, itsActiveMdiColors);
        }

        col=KGlobalSettings::inactiveTitleColor();
        if(col!=itsButtonCols[ORIGINAL_SHADE])
        {
            itsMdiColors=new QColor [TOTAL_SHADES+1];
            shadeColors(col, itsMdiColors);
        }

        itsActiveMdiTextColor=KGlobalSettings::activeTextColor();
        itsMdiTextColor=KGlobalSettings::inactiveTextColor();
#endif

        if(!itsActiveMdiColors)
            itsActiveMdiColors=(QColor *)itsHighlightCols;
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

#if !defined QTC_QT_ONLY
        KConfig      cfg("kwinrc");
        KConfigGroup grp(&cfg, "Style");

        if(grp.readEntry("CustomButtonPositions", false))
        {
            QString val=grp.readEntry("ButtonsOnLeft");
            if(!val.isEmpty())
            {
                itsMdiButtons[0].clear();
                parseWindowLine(val, itsMdiButtons[0]);
            }

            val=grp.readEntry("ButtonsOnRight");
            if(!val.isEmpty())
            {
                itsMdiButtons[1].clear();
                parseWindowLine(val, itsMdiButtons[1]);
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
#endif
    }
}

const QColor & QtCurveStyle::getFill(const QStyleOption *option, const QColor *use, bool cr, bool darker) const
{
    return !option || !(option->state&State_Enabled)
               ? use[darker ? 2 : ORIGINAL_SHADE]
               : option->state&State_Sunken  // State_Down ????
                   ? use[darker ? 5 : 4]
                   : option->state&State_MouseOver
                         ? !cr && option->state&State_On
                               ? use[darker ? 3 : SHADE_4_HIGHLIGHT]
                               : use[darker ? SHADE_2_HIGHLIGHT : SHADE_ORIG_HIGHLIGHT]
                         : !cr && option->state&State_On
                               ? use[darker ? 5 : 4]
                               : use[darker ? 2 : ORIGINAL_SHADE];
}

static QImage rotateImage(const QImage &img, double angle=90.0)
{
    QMatrix matrix;
    matrix.translate(img.width()/2, img.height()/2);
    matrix.rotate(angle);

    QRect newRect(matrix.mapRect(QRect(0, 0, img.width(), img.height())));

    return img.transformed(QMatrix(matrix.m11(), matrix.m12(), matrix.m21(), matrix.m22(),
                                   matrix.dx() - newRect.left(), matrix.dy() - newRect.top()));
}

QPixmap * QtCurveStyle::getPixmap(const QColor col, EPixmap p, double shade) const
{
    QtcKey  key(createKey(col, p));
    QPixmap *pix=itsPixmapCache.object(key);

    if(!pix)
    {
/*
    Can't get Gtk2 version to look nice, so give up for now!
        if(PIX_DOT==p)
        {
            pix=new QPixmap(5, 5);
            pix->fill(Qt::transparent);

            QColor          c(col);
            QPainter        p(pix);
            QLinearGradient g1(0, 0, 5, 5),
                            g2(0, 0, 3, 3);

            g1.setColorAt(0.0, c);
            c.setAlphaF(0.4);
            g1.setColorAt(1.0, c);
            c=Qt::white;
            c.setAlphaF(1.0);
            g2.setColorAt(0.0, c);
            c.setAlphaF(0.8);
            g2.setColorAt(1.0, c);
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setPen(Qt::NoPen);
            p.setBrush(g1);
            p.drawEllipse(0, 0, 5, 5);
            p.setBrush(g2);
            p.drawEllipse(2, 2, 3, 3);
            p.end();
        }
        else
*/
        {
            pix=new QPixmap();

            QImage img;

            switch(p)
            {
                case PIX_RADIO_BORDER:
                    img.loadFromData(radio_frame_png_data, radio_frame_png_len);
                    break;
                case PIX_RADIO_INNER:
                    img.loadFromData(radio_inner_png_data, radio_inner_png_len);
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
                    img.loadFromData(slider_png_data, slider_png_len);
                    img=rotateImage(img);
                    break;
                case PIX_SLIDER_LIGHT_V:
                    img.loadFromData(slider_light_png_data, slider_light_png_len);
                    img=rotateImage(img).mirrored(true, false);
                default:
                    break;
            }

            if (img.depth()<32)
                img=img.convertToFormat(QImage::Format_ARGB32);

            adjustPix(img.bits(), 4, img.width(), img.height(), img.bytesPerLine(), col.red(),
                    col.green(), col.blue(), shade);
            *pix=QPixmap::fromImage(img);
        }
        itsPixmapCache.insert(key, pix, pix->depth()/8);
    }

    return pix;
}

int QtCurveStyle::konqMenuBarSize(const QMenuBar *menu) const
{
    const QFontMetrics   fm(menu->fontMetrics());
    QSize                sz(100, fm.height());

    QStyleOptionMenuItem opt;
    opt.fontMetrics = fm;
    opt.state = QStyle::State_Enabled;
    opt.menuRect = menu->rect();
    opt.text = "File";
    sz = sizeFromContents(QStyle::CT_MenuBarItem, &opt, sz, menu);
    return sz.height()+6;
}

QtCurveStyle::Version QtCurveStyle::qtVersion() const
{
    if(VER_UNKNOWN==itsQtVersion)
    {
        int major, minor, patch;

        // Try to detect if this is Qt 4.5...
        if(3==sscanf(qVersion(), "%d.%d.%d", &major, &minor, &patch) && 4==major)
            if(minor<5)
                itsQtVersion=VER_4x;
            else
                itsQtVersion=VER_45;
    }

    return itsQtVersion;
}

const QColor & QtCurveStyle::getTabFill(bool current, bool highlight, const QColor *use) const
{
    return current
            ? use[ORIGINAL_SHADE]
            : highlight
                ? use[SHADE_2_HIGHLIGHT]
                : use[2];
}

const QColor & QtCurveStyle::menuStripeCol() const
{
    switch(opts.menuStripe)
    {
        default:
        case SHADE_NONE:
            return itsBackgroundCols[ORIGINAL_SHADE];
        case SHADE_CUSTOM:
            return opts.customMenuStripeColor;
        case SHADE_BLEND_SELECTED:
            // Hack! Use opts.customMenuStripeColor to store this setting!
            if(QTC_IS_BLACK(opts.customMenuStripeColor))
                opts.customMenuStripeColor=midColor(itsHighlightCols[ORIGINAL_SHADE],
                                opts.lighterPopupMenuBgnd<0
                                    ? itsLighterPopupMenuBgndCol
                                    : itsBackgroundCols[ORIGINAL_SHADE]);
            return opts.customMenuStripeColor;
        case SHADE_SELECTED:
            return itsHighlightCols[QTC_MENU_STRIPE_SHADE];
        case SHADE_DARKEN:
            return USE_LIGHTER_POPUP_MENU
                ? itsLighterPopupMenuBgndCol
                : itsBackgroundCols[QTC_MENU_STRIPE_SHADE];
    }
}

const QColor & QtCurveStyle::checkRadioCol(const QStyleOption *opt) const
{
    return opt->state&State_Enabled
            ? itsCheckRadioCol
            : opts.crButton
                ? opt->palette.buttonText().color()
                : opt->palette.text().color();
}

QColor QtCurveStyle::shade(const QColor &a, float k) const
{
    QColor mod;

    ::shade(&opts, a, &mod, k);
    return mod;
}

void QtCurveStyle::shade(const color &ca, color *cb, double k) const
{
    ::shade(&opts, ca, cb, k);
}

QColor QtCurveStyle::getLowerEtchCol(const QWidget *widget) const
{
    if(IS_FLAT(opts.bgndAppearance))
    {
        bool doEtch=widget && widget->parentWidget() && !theNoEtchWidgets.contains(widget);
// CPD: Don't really want to check here for every widget, when (so far) on problem seems to be in
// KPackageKit, and thats with its KTextBrowser - so just check when we draw scrollviews...
//     if(doEtch && isInQAbstractItemView(widget->parentWidget()))
//     {
//         doEtch=false;
//         theNoEtchWidgets.insert(widget);
//     }

        if(doEtch)
        {
            QColor bgnd(widget->parentWidget()->palette().color(widget->parentWidget()->backgroundRole()));

            if(bgnd.alpha()>0)
                return shade(bgnd, 1.06);
        }
    }

    QColor col(Qt::white);
    col.setAlphaF(IS_FLAT(opts.bgndAppearance) ? 0.25 : 0.4);

    return col;
}

void QtCurveStyle::widgetDestroyed(QObject *o)
{
    QWidget *w=static_cast<const QWidget *>(o);
    theNoEtchWidgets.remove(w);
    if(APP_KONTACT==theThemedApp)
    {
        itsSViewContainers.remove(w);
        QMap<QWidget *, QSet<QWidget *> >::Iterator it(itsSViewContainers.begin()),
                                                    end(itsSViewContainers.end());
        QSet<QWidget *>                             rem;

        for(; it!=end; ++it)
        {
            (*it).remove(w);
            if((*it).isEmpty())
                rem.insert(it.key());
        }

        QSet<QWidget *>::ConstIterator r(rem.begin()),
                                       remEnd(rem.end());

        for(; r!=remEnd; ++r)
            itsSViewContainers.remove(*r);
    }
}

void QtCurveStyle::setupKde4()
{
#if !defined QTC_QT_ONLY
    if(kapp)
        setDecorationColors();
    else
    {
        applyKdeSettings(true);
        applyKdeSettings(false);
    }
#endif
}

#if !defined QTC_QT_ONLY
void QtCurveStyle::setDecorationColors()
{
    KColorScheme kcs(QPalette::Active);
    if(opts.coloredMouseOver)
        shadeColors(kcs.decoration(KColorScheme::HoverColor).color(), itsMouseOverCols);
    shadeColors(kcs.decoration(KColorScheme::FocusColor).color(), itsFocusCols);
}

void QtCurveStyle::applyKdeSettings(bool pal)
{
    if(pal)
    {
        if(!kapp)
            QApplication::setPalette(standardPalette());
        setDecorationColors();
    }
    else
    {
        KConfigGroup g(KGlobal::config(), "General");
        QFont        mnu=g.readEntry("menuFont", QApplication::font());

        QApplication::setFont(g.readEntry("font", QApplication::font()));
        QApplication::setFont(mnu, "QMenuBar");
        QApplication::setFont(mnu, "QMenu");
        QApplication::setFont(mnu, "KPopupTitle");
// Don't set toolbar font - messes things up with Arora...
//         QApplication::setFont(KGlobalSettings::toolBarFont(), "QToolBar");
    }
}
#endif

void QtCurveStyle::kdeGlobalSettingsChange(int type, int)
{
#if defined QTC_QT_ONLY
    Q_UNUSED(type)
#else
    switch(type)
    {
        case KGlobalSettings::PaletteChanged:
            KGlobal::config()->reparseConfiguration();
            applyKdeSettings(true);
            if(usePixmapCache)
                QPixmapCache::clear();
            break;
        case KGlobalSettings::FontChanged:
            KGlobal::config()->reparseConfiguration();
            applyKdeSettings(false);
            break;
    }
#endif
}
