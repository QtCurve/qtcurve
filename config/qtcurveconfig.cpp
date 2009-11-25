/*
  QtCurve (C) Craig Drummond, 2007-2009 ee11cd@googlemail.com

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

#include "qtcurveconfig.h"
#ifdef QTC_STYLE_SUPPORT
#include "exportthemedialog.h"
#endif
#include "ui_stylepreview.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QFrame>
#include <QTabWidget>
#include <QFileInfo>
#include <QBoxLayout>
#include <QGridLayout>
#include <QTreeWidget>
#include <QPainter>
#include <QSettings>
#include <QtDBus>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QStyleFactory>
#include <QCloseEvent>
#include <QRegExp>
#include <QRegExpValidator>
#include <KGuiItem>
#include <KInputDialog>
#include <KDE/KLocale>
#include <KDE/KColorButton>
#include <KDE/KConfig>
#include <KDE/KFileDialog>
#include <KDE/KMessageBox>
#include <KDE/KPushButton>
#include <KDE/KCharSelect>
#include <KDE/KDialog>
#include <KDE/KIntNumInput>
#include <KDE/KTemporaryFile>
#include <KDE/KXmlGuiWindow>
#include <KDE/KStandardAction>
#include <KDE/KStatusBar>
#include <KDE/KAboutData>
#include <KDE/KComponentData>
#include <KDE/KActionCollection>
#include <KDE/KToolBar>
#include <unistd.h>
#include "config.h"
#define CONFIG_READ
#define CONFIG_WRITE
#include "config_file.c"

#define QTC_EXTENSION ".qtcurve"

extern "C"
{
    KDE_EXPORT QObject * allocate_kstyle_config(QWidget* parent)
    {
        KGlobal::locale()->insertCatalog("qtcurve");

        return new QtCurveConfig(parent);
    }
}

static void setStyleRecursive(QWidget* w, QStyle* s)
{
    w->setStyle(s);

    const QObjectList children = w->children();

    foreach (QObject* child, children)
    {
        if (child->isWidgetType())
            setStyleRecursive((QWidget *) child, s);
    }
}

static const KStandardAction::StandardAction standardAction[] =
{
    KStandardAction::New, KStandardAction::Open, KStandardAction::OpenRecent, KStandardAction::Save, KStandardAction::SaveAs, KStandardAction::Revert, KStandardAction::Close, KStandardAction::Quit,
    KStandardAction::Cut, KStandardAction::Copy, KStandardAction::Paste,
    KStandardAction::ActionNone
};

class CStylePreview : public KXmlGuiWindow, public Ui::StylePreview
{
    public:

    CStylePreview(QWidget *parent = 0)
        : KXmlGuiWindow(parent)
    {
        aboutData = new KAboutData("qtcurve", 0, ki18n("QtCurve"), VERSION,
                                   ki18n("Unified widet style."),
                                   KAboutData::License_GPL,
                                   ki18n("(C) Craig Drummond, 2003-2009"),
                                   KLocalizedString());
        aboutData->setProgramIconName("preferences-desktop-theme");
        componentData = new KComponentData(aboutData);

        QWidget *main=new QWidget(this);
        setupUi(main);
        setCentralWidget(main);
        setComponentData(*componentData);
        for (uint i = 0; standardAction[i] != KStandardAction::ActionNone; ++i)
            actionCollection()->addAction(standardAction[i]);
        createGUI();
        statusBar()->setSizeGripEnabled(true);
        toolBar()->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        setCaption(i18n("Preview Window"));
    }

    ~CStylePreview()
    {
        delete componentData;
        delete aboutData;
    }

    void closeEvent(QCloseEvent *e)
    {
        e->ignore();
    }

    QSize sizeHint() const
    {
        return QSize(500, 260);
    }

    private:

    KAboutData     *aboutData;
    KComponentData *componentData;
};

class CWorkspace : public QMdiArea
{
    public:

    CWorkspace(QWidget *parent) : QMdiArea(parent)
    {
         setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    QSize sizeHint() const
    {
        return QSize(200, 200);
    }

    void paintEvent(QPaintEvent *)
    {
        QPainter p(viewport());
        p.fillRect(rect(), palette().color(backgroundRole()).dark(110));
    }
};

class CharSelectDialog : public KDialog
{
    public:

    CharSelectDialog(QWidget *parent, int v)
        : KDialog(parent)
    {
        setCaption(i18n("Select Password Character"));
        setModal(true);
        setButtons(KDialog::Ok|KDialog::Cancel);
        enableButtonOk(true);
        enableButtonCancel(true);

        QFrame *page = new QFrame(this);
        setMainWidget(page);

        QBoxLayout *layout=new QBoxLayout(QBoxLayout::TopToBottom, page);
        layout->setMargin(0);
        layout->setSpacing(KDialog::spacingHint());

        itsSelector=new KCharSelect(page);
        itsSelector->setCurrentChar(QChar(v));
        layout->addWidget(itsSelector);
    }

    int currentChar() const { return itsSelector->currentChar().unicode(); }

    private:

    KCharSelect *itsSelector;
};

class CGradItem : public QTreeWidgetItem
{
    public:

    CGradItem(QTreeWidget *p, const QStringList &vals)
        : QTreeWidgetItem(p, vals)
    {
        setFlags(flags()|Qt::ItemIsEditable);
    }

    virtual ~CGradItem() { }

    bool operator<(const QTreeWidgetItem &i) const
    {
        return text(0).toDouble()<i.text(0).toDouble() ||
               (equal(text(0).toDouble(), i.text(0).toDouble()) &&
               text(1).toDouble()<i.text(1).toDouble());
    }
};

static QStringList toList(const QString &str)
{
    QStringList lst;
    lst.append(str);
    return lst;
}

class CStackItem : public QTreeWidgetItem
{
    public:

    CStackItem(QTreeWidget *p, const QString &text, int s)
        : QTreeWidgetItem(p, toList(text)),
          stackId(s)
    {
        if(0==s)
        {
            QFont fnt(font(0));

            fnt.setBold(true);
            setFont(0, fnt);
        }
        setTextAlignment(0, Qt::AlignRight);
    }

    bool operator<(const QTreeWidgetItem &o) const
    {
        return stackId<((CStackItem &)o).stackId;
    }

    int stack() { return stackId; }

    private:

    int stackId;
};

CGradientPreview::CGradientPreview(QtCurveConfig *c, QWidget *p)
                : QWidget(p),
                  cfg(c)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
}

QSize CGradientPreview::sizeHint() const
{
    return QSize(64, 64);
}

QSize CGradientPreview::minimumSizeHint() const
{
    return sizeHint();
}

void CGradientPreview::paintEvent(QPaintEvent *)
{
    QRect    r(rect());
    QPainter p(this);

    if(stops.size())
    {
        QLinearGradient                  grad(r.topLeft(), r.bottomLeft());
        GradientStopCont                 st(stops.fix());
        GradientStopCont::const_iterator it(st.begin()),
                                         end(st.end());

        for(; it!=end; ++it)
        {
            QColor col;
            Options opts;
            opts.shading=cfg->currentShading();
            shade(&opts, color, &col, (*it).val);
            grad.setColorAt((*it).pos, col);
        }
        p.fillRect(r, QBrush(grad));
    }
    else
        p.fillRect(r, color);
    p.end();
}

void CGradientPreview::setGrad(const GradientStopCont &s)
{
    stops=s;
    repaint();
}

void CGradientPreview::setColor(const QColor &col)
{
    if(col!=color)
    {
        color=col;
        repaint();
    }
}

static QString readEnvPath(const char *env)
{
   const char *path=getenv(env);

   return path ? QFile::decodeName(path) : QString::null;
}
static QString kdeHome(bool kde3)
{
    static QString kdeHome[2];

    // Execute kde-config to ascertain users KDEHOME
    if(kdeHome[kde3 ? 0 : 1].isEmpty())
    {
        FILE *fpipe;

        if (fpipe = (FILE*)popen(kde3 ? "kde-config --localprefix" : "kde4-config --localprefix", "r"))
        {
            char line[1024];

            while(fgets(line, sizeof line, fpipe))
            {
                kdeHome[kde3 ? 0 : 1]=QFile::decodeName(line).replace("\n", "");
                break;
            }
            pclose(fpipe);
        }
    }

    // Try env vars...
    if(kdeHome[kde3 ? 0 : 1].isEmpty())
    {
        kdeHome[kde3 ? 0 : 1]=readEnvPath(getuid() ? "KDEHOME" : "KDEROOTHOME");
        if (kdeHome[kde3 ? 0 : 1].isEmpty())
        {
            QDir    homeDir(QDir::homePath());
            QString kdeConfDir("/.kde");
            if (!kde3 && homeDir.exists(".kde4"))
                kdeConfDir = QString("/.kde4");
            kdeHome[kde3 ? 0 : 1] = QDir::homePath() + kdeConfDir;
        }
    }
    return kdeHome[kde3 ? 0 : 1];
}

static int toInt(const QString &str)
{
    return str.length()>1 ? str[0].unicode() : 0;
}

enum ShadeWidget
{
    SW_MENUBAR,
    SW_SLIDER,
    SW_CHECK_RADIO,
    SW_MENU_STRIPE,
    SW_COMBO,
    SW_LV_HEADER
};

static void insertShadeEntries(QComboBox *combo, ShadeWidget sw)
{
    switch(sw)
    {
        case SW_MENUBAR:
            combo->insertItem(SHADE_NONE, i18n("Background"));
            break;
        case SW_COMBO:
        case SW_SLIDER:
            combo->insertItem(SHADE_NONE, i18n("Button"));
            break;
        case SW_CHECK_RADIO:
            combo->insertItem(SHADE_NONE, i18n("Text"));
            break;
        case SW_LV_HEADER:
        case SW_MENU_STRIPE:
            combo->insertItem(SHADE_NONE, i18n("None"));
            break;
    }

    combo->insertItem(SHADE_CUSTOM, i18n("Custom:"));
    combo->insertItem(SHADE_SELECTED, i18n("Selected background"));
    if(SW_CHECK_RADIO!=sw) // For check/radio, we dont blend, and dont allow darken
    {
        combo->insertItem(SHADE_BLEND_SELECTED, i18n("Blended selected background"));
        combo->insertItem(SHADE_DARKEN, SW_MENU_STRIPE==sw ? i18n("Menu background") : i18n("Darken"));
    }
}

static void insertAppearanceEntries(QComboBox *combo, bool split=true, bool bev=true, bool fade=false)
{
    for(int i=APPEARANCE_CUSTOM1; i<(APPEARANCE_CUSTOM1+QTC_NUM_CUSTOM_GRAD); ++i)
        combo->insertItem(i, i18n("Custom gradient %1", (i-APPEARANCE_CUSTOM1)+1));

    combo->insertItem(APPEARANCE_FLAT, i18n("Flat"));
    combo->insertItem(APPEARANCE_RAISED, i18n("Raised"));
    combo->insertItem(APPEARANCE_DULL_GLASS, i18n("Dull glass"));
    combo->insertItem(APPEARANCE_SHINY_GLASS, i18n("Shiny glass"));
    combo->insertItem(APPEARANCE_AGUA, i18n("Agua"));
    combo->insertItem(APPEARANCE_SOFT_GRADIENT, i18n("Soft gradient"));
    combo->insertItem(APPEARANCE_GRADIENT, i18n("Standard gradient"));
    combo->insertItem(APPEARANCE_HARSH_GRADIENT, i18n("Harsh gradient"));
    combo->insertItem(APPEARANCE_INVERTED, i18n("Inverted gradient"));
    combo->insertItem(APPEARANCE_DARK_INVERTED, i18n("Dark inverted gradient"));
    if(split)
    {
        combo->insertItem(APPEARANCE_SPLIT_GRADIENT, i18n("Split gradient"));
        if(bev)
        {
            combo->insertItem(APPEARANCE_BEVELLED, i18n("Bevelled"));
            if(fade)
                combo->insertItem(APPEARANCE_FADE, i18n("Fade out (popup menuitems)"));
        }
    }
}

static void insertLineEntries(QComboBox *combo,  bool singleDot, bool dashes)
{
    combo->insertItem(LINE_NONE, i18n("None"));
    combo->insertItem(LINE_SUNKEN, i18n("Sunken lines"));
    combo->insertItem(LINE_FLAT, i18n("Flat lines"));
    combo->insertItem(LINE_DOTS, i18n("Dots"));
    if(singleDot)
    {
        combo->insertItem(LINE_1DOT, i18n("Single dot"));
        if(dashes)
            combo->insertItem(LINE_DASHES, i18n("Dashes"));
    }
}

static void insertDefBtnEntries(QComboBox *combo)
{
    combo->insertItem(IND_CORNER, i18n("Corner indicator"));
    combo->insertItem(IND_FONT_COLOR, i18n("Font color thin border"));
    combo->insertItem(IND_COLORED, i18n("Selected background thick border"));
    combo->insertItem(IND_TINT, i18n("Selected background tinting"));
    combo->insertItem(IND_GLOW, i18n("A slight glow"));
    combo->insertItem(IND_DARKEN, i18n("Darken"));
    combo->insertItem(IND_NONE, i18n("No indicator"));
}

static void insertScrollbarEntries(QComboBox *combo)
{
    combo->insertItem(SCROLLBAR_KDE, i18n("KDE"));
    combo->insertItem(SCROLLBAR_WINDOWS, i18n("MS Windows"));
    combo->insertItem(SCROLLBAR_PLATINUM, i18n("Platinum"));
    combo->insertItem(SCROLLBAR_NEXT, i18n("NeXT"));
    combo->insertItem(SCROLLBAR_NONE, i18n("No buttons"));
}

static void insertRoundEntries(QComboBox *combo)
{
    combo->insertItem(ROUND_NONE, i18n("Square"));
    combo->insertItem(ROUND_SLIGHT, i18n("Slightly rounded"));
    combo->insertItem(ROUND_FULL, i18n("Fully rounded"));
    combo->insertItem(ROUND_EXTRA, i18n("Extra rounded (KDE4 & Gtk2)"));
    combo->insertItem(ROUND_MAX, i18n("Max rounded (KDE4 & Gtk2)"));
}

static void insertMouseOverEntries(QComboBox *combo)
{
    combo->insertItem(MO_NONE, i18n("No coloration"));
    combo->insertItem(MO_COLORED, i18n("Color border"));
    combo->insertItem(MO_COLORED_THICK, i18n("Thick color border"));
    combo->insertItem(MO_PLASTIK, i18n("Plastik style"));
    combo->insertItem(MO_GLOW, i18n("Glow"));
}

static void insertToolbarBorderEntries(QComboBox *combo)
{
    combo->insertItem(TB_NONE, i18n("None"));
    combo->insertItem(TB_LIGHT, i18n("Light"));
    combo->insertItem(TB_DARK, i18n("Dark"));
    combo->insertItem(TB_LIGHT_ALL, i18n("Light (all sides)"));
    combo->insertItem(TB_DARK_ALL, i18n("Dark (all sides)"));
}

static void insertEffectEntries(QComboBox *combo)
{
    combo->insertItem(EFFECT_NONE, i18n("Plain"));
    combo->insertItem(EFFECT_ETCH, i18n("Etched"));
    combo->insertItem(EFFECT_SHADOW, i18n("Shadowed"));
}

static void insertShadingEntries(QComboBox *combo)
{
    combo->insertItem(SHADING_SIMPLE, i18n("Simple"));
    combo->insertItem(SHADING_HSL, i18n("Use HSL color space"));
    combo->insertItem(SHADING_HSV, i18n("Use HSV color space"));
    combo->insertItem(SHADING_HCY, i18n("Use HCY color space"));
}

static void insertStripeEntries(QComboBox *combo)
{
    combo->insertItem(STRIPE_NONE, i18n("Plain"));
    combo->insertItem(STRIPE_PLAIN, i18n("Striped"));
    combo->insertItem(STRIPE_DIAGONAL, i18n("Diagonal stripes"));
}

static void insertSliderStyleEntries(QComboBox *combo)
{
    combo->insertItem(SLIDER_PLAIN, i18n("Plain"));
    combo->insertItem(SLIDER_ROUND, i18n("Round"));
    combo->insertItem(SLIDER_PLAIN_ROTATED, i18n("Plain - rotated"));
    combo->insertItem(SLIDER_ROUND_ROTATED, i18n("Round - rotated"));
    combo->insertItem(SLIDER_TRIANGULAR, i18n("Triangular"));
}

static void insertEColorEntries(QComboBox *combo)
{
    combo->insertItem(ECOLOR_BASE, i18n("Base color"));
    combo->insertItem(ECOLOR_BACKGROUND, i18n("Background color"));
    combo->insertItem(ECOLOR_DARK, i18n("Darkened background color"));
}

static void insertFocusEntries(QComboBox *combo)
{
    combo->insertItem(FOCUS_STANDARD, i18n("Standard (dotted)"));
    combo->insertItem(FOCUS_RECTANGLE, i18n("Highlight color"));
    combo->insertItem(FOCUS_FULL, i18n("Highlight color (full size)"));
    combo->insertItem(FOCUS_FILLED, i18n("Highlight color, and fill (Gtk2 & KDE4 only)"));
    combo->insertItem(FOCUS_LINE, i18n("Line drawn with highlight color"));
}

static void insertGradBorderEntries(QComboBox *combo)
{
    combo->insertItem(GB_NONE, i18n("No border"));
    combo->insertItem(GB_LIGHT, i18n("Light border"));
    combo->insertItem(GB_3D, i18n("3D border (light only)"));
    combo->insertItem(GB_3D_FULL, i18n("3D border (dark and light)"));
}

static void insertAlignEntries(QComboBox *combo)
{
    combo->insertItem(ALIGN_LEFT, i18n("Left"));
    combo->insertItem(ALIGN_CENTER, i18n("Center (between controls)"));
    combo->insertItem(ALIGN_FULL_CENTER, i18n("Center (full width)"));
    combo->insertItem(ALIGN_RIGHT, i18n("Right"));
}

enum ETitleBarButtonColoration
{
    TITLE_BTN_COL_BACKGROUND,
    TITLE_BTN_COL_BUTTON,
    TITLE_BTN_COL_CUSTOM
};

static void insertTitlebarIconEntries(QComboBox *combo)
{
    combo->insertItem(TITLEBAR_ICON_NONE, i18n("Do not show"));
    combo->insertItem(TITLEBAR_ICON_MENU_BUTTON, i18n("Place on menu button"));
    combo->insertItem(TITLEBAR_ICON_NEXT_TO_TITLE, i18n("Place next to title"));
}

static void insertTabMoEntries(QComboBox *combo)
{
    combo->insertItem(TAB_MO_TOP, i18n("Highlight on top"));
    combo->insertItem(TAB_MO_BOTTOM, i18n("Highlight on bottom"));
    combo->insertItem(TAB_MO_GLOW, i18n("Add a slight glow"));
}

static void insertGradTypeEntries(QComboBox *combo)
{
    combo->insertItem(GT_HORIZ, i18n("Top to bottom"));
    combo->insertItem(GT_VERT, i18n("Left to right"));
}

static void insertLvLinesEntries(QComboBox *combo)
{
    combo->insertItem(LV_NONE, i18n("None"));
    combo->insertItem(LV_NEW, i18n("New style (KDE and Gtk2 similar)"));
    combo->insertItem(LV_OLD, i18n("Old style (KDE and Gtk2 different)"));
}

QtCurveConfig::QtCurveConfig(QWidget *parent)
             : QWidget(parent),
               widgetStyle(NULL),
#ifdef QTC_STYLE_SUPPORT
               exportDialog(NULL),
#endif
               gradPreview(NULL)
{
    setupUi(this);
    titleLabel->setText("QtCurve " VERSION " - (C) Craig Drummond, 2003-2009");
    insertShadeEntries(shadeSliders, SW_SLIDER);
    insertShadeEntries(shadeMenubars, SW_MENUBAR);
    insertShadeEntries(shadeCheckRadio, SW_CHECK_RADIO);
    insertShadeEntries(menuStripe, SW_MENU_STRIPE);
    insertShadeEntries(comboBtn, SW_COMBO);
    insertShadeEntries(sortedLv, SW_LV_HEADER);
    insertAppearanceEntries(appearance);
    insertAppearanceEntries(menubarAppearance);
    insertAppearanceEntries(toolbarAppearance);
    insertAppearanceEntries(lvAppearance);
    insertAppearanceEntries(sliderAppearance);
    insertAppearanceEntries(tabAppearance, false);
    insertAppearanceEntries(activeTabAppearance, false);
    insertAppearanceEntries(progressAppearance);
    insertAppearanceEntries(progressGrooveAppearance);
    insertAppearanceEntries(grooveAppearance);
    insertAppearanceEntries(sunkenAppearance);
    insertAppearanceEntries(menuitemAppearance, true, true, true);
    insertAppearanceEntries(menuBgndAppearance);
    insertAppearanceEntries(titlebarAppearance, true, false);
    insertAppearanceEntries(inactiveTitlebarAppearance, true, false);
    insertAppearanceEntries(titlebarButtonAppearance);
    insertAppearanceEntries(selectionAppearance, true, false);
    insertAppearanceEntries(menuStripeAppearance, true, false);
    insertAppearanceEntries(sbarBgndAppearance);
    insertAppearanceEntries(sliderFill);
    insertAppearanceEntries(bgndAppearance);
    insertAppearanceEntries(dwtAppearance);
    insertLineEntries(handles, true, true);
    insertLineEntries(sliderThumbs, true, false);
    insertLineEntries(toolbarSeparators, false, false);
    insertLineEntries(splitters, true, true);
    insertDefBtnEntries(defBtnIndicator);
    insertScrollbarEntries(scrollbarType);
    insertRoundEntries(round);
    insertMouseOverEntries(coloredMouseOver);
    insertToolbarBorderEntries(toolbarBorders);
    insertEffectEntries(buttonEffect);
    insertShadingEntries(shading);
    insertStripeEntries(stripedProgress);
    insertSliderStyleEntries(sliderStyle);
    insertEColorEntries(progressGrooveColor);
    insertFocusEntries(focus);
    insertGradBorderEntries(gradBorder);
    insertAlignEntries(titlebarAlignment);
    insertEffectEntries(titlebarEffect);
    insertTitlebarIconEntries(titlebarIcon);
    insertTabMoEntries(tabMouseOver);
    insertGradTypeEntries(bgndGrad);
    insertGradTypeEntries(menuBgndGrad);
    insertLvLinesEntries(lvLines);

    highlightFactor->setRange(MIN_HIGHLIGHT_FACTOR, MAX_HIGHLIGHT_FACTOR);
    highlightFactor->setValue(DEFAULT_HIGHLIGHT_FACTOR);

    crHighlight->setRange(MIN_HIGHLIGHT_FACTOR, MAX_HIGHLIGHT_FACTOR);
    crHighlight->setValue(DEFAULT_CR_HIGHLIGHT_FACTOR);

    splitterHighlight->setRange(MIN_HIGHLIGHT_FACTOR, MAX_HIGHLIGHT_FACTOR);
    splitterHighlight->setValue(DEFAULT_SPLITTER_HIGHLIGHT_FACTOR);

    lighterPopupMenuBgnd->setRange(MIN_LIGHTER_POPUP_MENU, MAX_LIGHTER_POPUP_MENU);
    lighterPopupMenuBgnd->setValue(DEF_POPUPMENU_LIGHT_FACTOR);

    expanderHighlight->setRange(MIN_HIGHLIGHT_FACTOR, MAX_HIGHLIGHT_FACTOR);
    expanderHighlight->setValue(DEFAULT_EXPANDER_HIGHLIGHT_FACTOR);
   
    menuDelay->setRange(MIN_MENU_DELAY, MAX_MENU_DELAY);
    menuDelay->setValue(DEFAULT_MENU_DELAY);

    sliderWidth->setRange(MIN_SLIDER_WIDTH, MAX_SLIDER_WIDTH, 2);
    sliderWidth->setValue(DEFAULT_SLIDER_WIDTH);
    sliderWidth->setSuffix(i18n(" pixels"));

    tabBgnd->setRange(MIN_TAB_BGND, MAX_TAB_BGND);
    tabBgnd->setValue(DEF_TAB_BGND);

    colorSelTab->setRange(MIN_COLOR_SEL_TAB_FACTOR, MAX_COLOR_SEL_TAB_FACTOR);
    colorSelTab->setValue(DEF_COLOR_SEL_TAB_FACTOR);

    connect(lighterPopupMenuBgnd, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(tabBgnd, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(menuDelay, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(sliderWidth, SIGNAL(valueChanged(int)), SLOT(sliderWidthChanged()));
    connect(menuStripe, SIGNAL(currentIndexChanged(int)), SLOT(menuStripeChanged()));
    connect(customMenuStripeColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(menuStripeAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(gtkMenuStripe, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(bgndGrad, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(menuBgndGrad, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(round, SIGNAL(currentIndexChanged(int)), SLOT(roundChanged()));
    connect(toolbarBorders, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(sliderThumbs, SIGNAL(currentIndexChanged(int)), SLOT(sliderThumbChanged()));
    connect(handles, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(appearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(customMenuTextColor, SIGNAL(toggled(bool)), SLOT(customMenuTextColorChanged()));
    connect(stripedProgress, SIGNAL(currentIndexChanged(int)), SLOT(stripedProgressChanged()));
    connect(animatedProgress, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(embolden, SIGNAL(toggled(bool)), SLOT(emboldenToggled()));
    connect(defBtnIndicator, SIGNAL(currentIndexChanged(int)), SLOT(defBtnIndicatorChanged()));
    connect(highlightTab, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(menubarAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(toolbarAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(lvAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(sliderAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(tabAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(activeTabAppearance, SIGNAL(currentIndexChanged(int)), SLOT(activeTabAppearanceChanged()));
    connect(toolbarSeparators, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(splitters, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(fixParentlessDialogs, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(fillSlider, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(sliderStyle, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(roundMbTopOnly, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(menubarHiding, SIGNAL(toggled(bool)), SLOT(menubarHidingChanged()));
    connect(fillProgress, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(darkerBorders, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(comboSplitter, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(comboBtn, SIGNAL(currentIndexChanged(int)), SLOT(comboBtnChanged()));
    connect(sortedLv, SIGNAL(currentIndexChanged(int)), SLOT(sortedLvChanged()));
    connect(customComboBtnColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(customSortedLvColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(unifySpinBtns, SIGNAL(toggled(bool)), SLOT(unifySpinBtnsToggled()));
    connect(unifySpin, SIGNAL(toggled(bool)), SLOT(unifySpinToggled()));
    connect(unifyCombo, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(vArrows, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(xCheck, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(crHighlight, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(expanderHighlight, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(crButton, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(colorSelTab, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(roundAllTabs, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(borderTab, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(borderInactiveTab, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(invertBotTab, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(doubleGtkComboArrow, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(tabMouseOver, SIGNAL(currentIndexChanged(int)), SLOT(tabMoChanged()));
    connect(stdSidebarButtons, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(borderMenuitems, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(popupBorder, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(progressAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(progressGrooveAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(grooveAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(sunkenAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(progressGrooveColor, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(menuitemAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(menuBgndAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(titlebarAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(inactiveTitlebarAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(titlebarButtonAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(colorTitlebarOnly, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(selectionAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(shadeCheckRadio, SIGNAL(currentIndexChanged(int)), SLOT(shadeCheckRadioChanged()));
    connect(customCheckRadioColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(focus, SIGNAL(currentIndexChanged(int)), SLOT(focusChanged()));
    connect(lvLines, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(lvButton, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(drawStatusBarFrames, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(buttonEffect, SIGNAL(currentIndexChanged(int)), SLOT(buttonEffectChanged()));
    connect(coloredMouseOver, SIGNAL(currentIndexChanged(int)), SLOT(coloredMouseOverChanged()));
    connect(menubarMouseOver, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(shadeMenubarOnlyWhenActive, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(thinnerMenuItems, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(thinnerBtns, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(customSlidersColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(customMenubarsColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(customMenuSelTextColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(customMenuNormTextColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(shadeSliders, SIGNAL(currentIndexChanged(int)), SLOT(shadeSlidersChanged()));
    connect(shadeMenubars, SIGNAL(currentIndexChanged(int)), SLOT(shadeMenubarsChanged()));
    connect(highlightFactor, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(scrollbarType, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(shading, SIGNAL(currentIndexChanged(int)), SLOT(shadingChanged()));
    connect(gtkScrollViews, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(squareScrollViews, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(highlightScrollViews, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(etchEntry, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(flatSbarButtons, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(thinSbarGroove, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(colorSliderMouseOver, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(titlebarBorder, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(sbarBgndAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(sliderFill, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(bgndAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(dwtAppearance, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(dwtBtnAsPerTitleBar, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(dwtColAsPerTitleBar, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(dwtFontAsPerTitleBar, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(dwtTextAsPerTitleBar, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(dwtEffectAsPerTitleBar, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(dwtRoundTopOnly, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(xbar, SIGNAL(toggled(bool)), SLOT(xbarChanged()));
    connect(crColor, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(smallRadio, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(splitterHighlight, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(gtkComboMenus, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(gtkButtonOrder, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(mapKdeIcons, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(passwordChar, SIGNAL(clicked()), SLOT(passwordCharClicked()));
    connect(framelessGroupBoxes, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(colorMenubarMouseOver, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(useHighlightForMenu, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(groupBoxLine, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(fadeLines, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(menuIcons, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(stdBtnSizes, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(forceAlternateLvCols, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(squareLvSelection, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(titlebarAlignment, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(titlebarEffect, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(titlebarIcon, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));

    connect(titlebarButtons_button, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(titlebarButtons_custom, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(titlebarButtons_noFrame, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(titlebarButtons_round, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(titlebarButtons_hoverFrame, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(titlebarButtons_hoverSymbol, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(titlebarButtons_hoverSymbolFull, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(titlebarButtons_colorOnMouseOver, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(titlebarButtons_colorSymbolsOnly, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(titlebarButtons_colorInactive, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(titlebarButtons_colorClose, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(titlebarButtons_colorMin, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(titlebarButtons_colorMax, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(titlebarButtons_colorKeepAbove, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(titlebarButtons_colorKeepBelow, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(titlebarButtons_colorHelp, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(titlebarButtons_colorMenu, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(titlebarButtons_colorShade, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(titlebarButtons_colorAllDesktops, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));

    Options currentStyle,
            defaultStyle;

    defaultSettings(&defaultStyle);
    if(!readConfig(NULL, &currentStyle, &defaultStyle))
        currentStyle=defaultStyle;

    previewStyle=currentStyle;
    setupShadesTab();
    setWidgetOptions(currentStyle);

    setupGradientsTab();
    setupStack();
    setupPresets(currentStyle, defaultStyle);
    setupPreview();
}

QtCurveConfig::~QtCurveConfig()
{
    // Remove QTCURVE_PREVIEW_CONFIG setting, so that main kcmstyle preview does not revert to
    // default settings!
    qputenv(QTCURVE_PREVIEW_CONFIG, "");
    previewFrame->hide();
    previewFrame->setParent(0);
    // When KMainWindow closes it dereferences KGlobal. When KGlobal's refs get to 0 it quits!
    // ...runnin kcmshell4 style does not seem to increase ref count of KGlobal. So if we allowed
    // KGlobal to quit, kcmshell4 would exit immediately after QtCurve's config dialog was closed :-(
    KGlobal::setAllowQuit(false);
    delete previewFrame;
    KGlobal::setAllowQuit(true);
}

QSize QtCurveConfig::sizeHint() const
{
    return QSize(700, 500);
}

void QtCurveConfig::save()
{
    Options opts=presets[currentText].opts;

    setOptions(opts);
    writeConfig(NULL, opts, presets[defaultText].opts);

    // This is only read by KDE3...
    KConfig      k3globals(kdeHome(true)+"/share/config/kdeglobals", KConfig::CascadeConfig);
    KConfigGroup kde(&k3globals, "KDE");

    if(opts.gtkButtonOrder)
        kde.writeEntry("ButtonLayout", 2);
    else
        kde.deleteEntry("ButtonLayout");

    // If using QtCurve window decoration, get this to update...
    KConfig      kwin("kwinrc", KConfig::CascadeConfig);
    KConfigGroup style(&kwin, "Style");

    if(style.readEntry("PluginLib", QString())=="kwin3_qtcurve")
        QDBusConnection::sessionBus().send(QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig"));

    // Remove QTCURVE_PREVIEW_CONFIG setting, so that main kcmstyle preview does not revert to
    // default settings!
    qputenv(QTCURVE_PREVIEW_CONFIG, "");
}

void QtCurveConfig::defaults()
{
    int index=-1;

    for(int i=0; i<presetsCombo->count() && -1==index; ++i)
        if(presetsCombo->itemText(i)==defaultText)
            index=i;

    presetsCombo->setCurrentIndex(index);
    setPreset();
}

void QtCurveConfig::emboldenToggled()
{
    if(!embolden->isChecked() && IND_NONE==defBtnIndicator->currentIndex())
        defBtnIndicator->setCurrentIndex(IND_TINT);
    updateChanged();
}

void QtCurveConfig::defBtnIndicatorChanged()
{
    if(IND_NONE==defBtnIndicator->currentIndex() && !embolden->isChecked())
        embolden->setChecked(true);
    else if(IND_GLOW==defBtnIndicator->currentIndex() && EFFECT_NONE==buttonEffect->currentIndex())
        buttonEffect->setCurrentIndex(EFFECT_SHADOW);

    if(IND_COLORED==defBtnIndicator->currentIndex() && round->currentIndex()>ROUND_FULL)
        round->setCurrentIndex(ROUND_FULL);

    updateChanged();
}

void QtCurveConfig::buttonEffectChanged()
{
    if(EFFECT_NONE==buttonEffect->currentIndex())
    {
        if(IND_GLOW==defBtnIndicator->currentIndex())
            defBtnIndicator->setCurrentIndex(IND_TINT);
        if(MO_GLOW==coloredMouseOver->currentIndex())
            coloredMouseOver->setCurrentIndex(MO_PLASTIK);
    }

    updateChanged();
}

void QtCurveConfig::coloredMouseOverChanged()
{
    if(MO_GLOW==coloredMouseOver->currentIndex() &&
       EFFECT_NONE==buttonEffect->currentIndex())
        buttonEffect->setCurrentIndex(EFFECT_SHADOW);

    updateChanged();
}

void QtCurveConfig::shadeSlidersChanged()
{
    customSlidersColor->setEnabled(SHADE_CUSTOM==shadeSliders->currentIndex());
    updateChanged();
    if(gradPreview)
        gradPreview->repaint();
}

void QtCurveConfig::shadeMenubarsChanged()
{
    customMenubarsColor->setEnabled(SHADE_CUSTOM==shadeMenubars->currentIndex());
    updateChanged();
}

void QtCurveConfig::shadeCheckRadioChanged()
{
    customCheckRadioColor->setEnabled(SHADE_CUSTOM==shadeCheckRadio->currentIndex());
    updateChanged();
}

void QtCurveConfig::customMenuTextColorChanged()
{
    customMenuNormTextColor->setEnabled(customMenuTextColor->isChecked());
    customMenuSelTextColor->setEnabled(customMenuTextColor->isChecked());
    updateChanged();
}

void QtCurveConfig::menuStripeChanged()
{
    customMenuStripeColor->setEnabled(SHADE_CUSTOM==menuStripe->currentIndex());
    menuStripeAppearance->setEnabled(SHADE_NONE!=menuStripe->currentIndex());
    gtkMenuStripe->setEnabled(SHADE_NONE!=menuStripe->currentIndex());
    if(SHADE_NONE==menuStripe->currentIndex())
        gtkMenuStripe->setChecked(false);
    updateChanged();
}

void QtCurveConfig::comboBtnChanged()
{
    customComboBtnColor->setEnabled(SHADE_CUSTOM==comboBtn->currentIndex());
    updateChanged();
}

void QtCurveConfig::sortedLvChanged()
{
    customSortedLvColor->setEnabled(SHADE_CUSTOM==sortedLv->currentIndex());
    updateChanged();
}

void QtCurveConfig::stripedProgressChanged()
{
    animatedProgress->setEnabled(STRIPE_NONE!=stripedProgress->currentIndex());
    if(animatedProgress->isChecked() && STRIPE_NONE==stripedProgress->currentIndex())
        animatedProgress->setChecked(false);
    updateChanged();
}

void QtCurveConfig::activeTabAppearanceChanged()
{
    int  current(activeTabAppearance->currentIndex());
    bool disableCol(APPEARANCE_FLAT==current && APPEARANCE_RAISED==current);

    if(colorSelTab->value() && disableCol)
        colorSelTab->setValue(MIN_COLOR_SEL_TAB_FACTOR);
    colorSelTab->setEnabled(!disableCol);
    updateChanged();
}

void QtCurveConfig::tabMoChanged()
{
    if(TAB_MO_GLOW==tabMouseOver->currentIndex())
        roundAllTabs->setChecked(true);
    roundAllTabs->setEnabled(TAB_MO_GLOW!=tabMouseOver->currentIndex());
    roundAllTabs_false->setEnabled(TAB_MO_GLOW!=tabMouseOver->currentIndex());
    updateChanged();
}

void QtCurveConfig::shadingChanged()
{
    updateChanged();
    if(gradPreview)
        gradPreview->repaint();
}

void QtCurveConfig::passwordCharClicked()
{
    int              cur(toInt(passwordChar->text()));
    CharSelectDialog dlg(this, cur);

    if(QDialog::Accepted==dlg.exec() && dlg.currentChar()!=cur)
    {
        setPasswordChar(dlg.currentChar());
        updateChanged();
    }
}

void QtCurveConfig::unifySpinBtnsToggled()
{
    if(unifySpinBtns->isChecked())
        unifySpin->setChecked(false);
    unifySpin->setDisabled(unifySpinBtns->isChecked());
    updateChanged();
}

void QtCurveConfig::unifySpinToggled()
{
    if(unifySpin->isChecked())
        unifySpinBtns->setChecked(false);
    unifySpinBtns->setDisabled(unifySpin->isChecked());
    updateChanged();
}

void QtCurveConfig::sliderThumbChanged()
{
    if(LINE_NONE!=sliderThumbs->currentIndex() && sliderWidth->value()<DEFAULT_SLIDER_WIDTH)
        sliderWidth->setValue(DEFAULT_SLIDER_WIDTH);
    updateChanged();
}

void QtCurveConfig::sliderWidthChanged()
{
    if(0==sliderWidth->value()%2)
        sliderWidth->setValue(sliderWidth->value()+1);

    if(LINE_NONE!=sliderThumbs->currentIndex() && sliderWidth->value()<DEFAULT_SLIDER_WIDTH)
        sliderThumbs->setCurrentIndex(LINE_NONE);
    updateChanged();
}

void QtCurveConfig::menubarHidingChanged()
{
    if(menubarHiding->isChecked())
        xbar->setChecked(false);
    updateChanged();
}

void QtCurveConfig::xbarChanged()
{
    if(xbar->isChecked())
        menubarHiding->setChecked(false);
    updateChanged();
}
    
void QtCurveConfig::setupStack()
{
    int i=0;
    CStackItem *first=new CStackItem(stackList, i18n("Presets and Preview"), i++);
    new CStackItem(stackList, i18n("General"), i++);
    new CStackItem(stackList, i18n("Group Boxes"), i++);
    new CStackItem(stackList, i18n("Combos"), i++);
    new CStackItem(stackList, i18n("Spin Buttons"), i++);
    new CStackItem(stackList, i18n("Splitters"), i++);
    new CStackItem(stackList, i18n("Sliders and Scrollbars"), i++);
    new CStackItem(stackList, i18n("Progressbars"), i++);
    new CStackItem(stackList, i18n("Default Button"),i++);
    new CStackItem(stackList, i18n("Mouse-over"), i++);
    new CStackItem(stackList, i18n("Listviews"), i++);
    new CStackItem(stackList, i18n("Scrollviews"), i++);
    new CStackItem(stackList, i18n("Tabs"), i++);
    new CStackItem(stackList, i18n("Checks and Radios"), i++);
    new CStackItem(stackList, i18n("Windows"), i++);
    new CStackItem(stackList, i18n("Window buttons"), i++);
    new CStackItem(stackList, i18n("Window button colors"), i++);
    new CStackItem(stackList, i18n("Menubars"), i++);
    new CStackItem(stackList, i18n("Popup menus"), i++);
    new CStackItem(stackList, i18n("Toolbars"), i++);
    new CStackItem(stackList, i18n("Dock windows"), i++);
    new CStackItem(stackList, i18n("Advanced Settings"), i++);
    new CStackItem(stackList, i18n("Legacy"), i++);
    new CStackItem(stackList, i18n("Custom Gradients"), i++);
    new CStackItem(stackList, i18n("Custom Shades"), i++);

    stackList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    stackList->setSelectionMode(QAbstractItemView::SingleSelection);
    first->setSelected(true);
    connect(stackList, SIGNAL(itemSelectionChanged()), SLOT(changeStack()));
}

void QtCurveConfig::setupPresets(const Options &currentStyle, const Options &defaultStyle)
{
    QStringList files(KGlobal::dirs()->findAllResources("data", "QtCurve/*"QTC_EXTENSION, KStandardDirs::NoDuplicates));

    files.sort();

    QStringList::Iterator it(files.begin()),
                          end(files.end());

    saveButton->setGuiItem(KGuiItem(i18n("Save"), "document-save"));
    deleteButton->setGuiItem(KGuiItem(i18n("Delete"), "edit-delete"));
    importButton->setGuiItem(KGuiItem(i18n("Import..."), "document-import"));
    exportButton->setGuiItem(KGuiItem(i18n("Export..."), "document-export"));
    
    deleteButton->setEnabled(false);

    currentText=i18n("(Current)");
    defaultText=i18n("(Default)");
    presets[currentText]=Preset(currentStyle);
    presets[defaultText]=Preset(defaultStyle);
    for(; it!=end; ++it)
    {
        QString name(QFileInfo(*it).fileName().remove(QTC_EXTENSION).replace('_', ' '));

        if(!name.isEmpty() && name!=currentText && name!=defaultText)
        {
            presetsCombo->insertItem(0, name);
            presets[name]=Preset(*it);
        }
    }

    presetsCombo->insertItem(0, currentText);
    presetsCombo->insertItem(0, defaultText);
    presetsCombo->model()->sort(0);
    connect(presetsCombo, SIGNAL(currentIndexChanged(int)), SLOT(setPreset()));
    connect(saveButton, SIGNAL(clicked(bool)), SLOT(savePreset()));
    connect(deleteButton, SIGNAL(clicked(bool)), SLOT(deletePreset()));
    connect(importButton, SIGNAL(clicked(bool)), SLOT(importPreset()));
    connect(exportButton, SIGNAL(clicked(bool)), SLOT(exportPreset()));
    
    int index=-1;
 
    for(int i=0; i<presetsCombo->count() && -1==index; ++i)
        if(presetsCombo->itemText(i)==currentText)
            index=i;

    presetsCombo->setCurrentIndex(index);
    setPreset();
}

void QtCurveConfig::setupPreview()
{
    QVBoxLayout *layout = new QVBoxLayout(previewFrame);
    CWorkspace  *workSpace = new CWorkspace(previewFrame);

    layout->setMargin(0);
    layout->addWidget(workSpace);

    CStylePreview *stylePreview = new CStylePreview;
    QMdiSubWindow *mdiWindow = workSpace->addSubWindow(stylePreview, Qt::Window);
    mdiWindow->move(4, 4);
    mdiWindow->show();
    updatePreview();
}

void QtCurveConfig::changeStack()
{
    CStackItem *item=(CStackItem *)(stackList->currentItem());

    if(item && !item->isSelected())
        item->setSelected(true);

    if(item)
    {
        if(0==item->stack() && settingsChanged(previewStyle))
            updatePreview();
        stack->setCurrentIndex(item->stack());
    }
}

void QtCurveConfig::gradChanged(int i)
{
    GradientCont::const_iterator it(customGradient.find((EAppearance)i));

    gradStops->clear();

    if(it!=customGradient.end())
    {
        gradPreview->setGrad((*it).second.stops);
        gradBorder->setCurrentIndex((*it).second.border);

        GradientStopCont::const_iterator git((*it).second.stops.begin()),
                                         gend((*it).second.stops.end());

        for(; git!=gend; ++git)
        {
            QStringList details;

            details << QString().setNum((*git).pos*100.0)
                    << QString().setNum((*git).val*100.0);

            new CGradItem(gradStops, details);
        }

        gradStops->sortItems(0, Qt::AscendingOrder);
    }
    else
    {
        gradPreview->setGrad(GradientStopCont());
        gradBorder->setCurrentIndex(GB_3D);
    }

    gradBorder->setEnabled(QTC_NUM_CUSTOM_GRAD!=i);
}

static double prev=0.0;

void QtCurveConfig::editItem(QTreeWidgetItem *i, int col)
{
    bool   ok;
    prev=i->text(col).toDouble(&ok);
    if(!ok)
        prev=0.0;

    gradStops->editItem(i, col);
}

void QtCurveConfig::itemChanged(QTreeWidgetItem *i, int col)
{
    bool   ok;
    double val=i->text(col).toDouble(&ok)/100.0;

    if(ok && equal(val, prev))
        return;

    if(!ok || (0==col && (val<0.0 || val>1.0)) || (1==col && (val<0.0 || val>2.0)))
        i->setText(col, QString().setNum(prev));
    else
    {
        double other=i->text(col ? 0 : 1).toDouble(&ok)/100.0;

        GradientCont::iterator it=customGradient.find((EAppearance)gradCombo->currentIndex());

        if(it!=customGradient.end())
        {
            (*it).second.stops.erase(GradientStop(col ? other : prev, col ? prev : other));
            (*it).second.stops.insert(GradientStop(col ? other : val, col ? val : other));
            gradPreview->setGrad((*it).second.stops);
            i->setText(col, QString().setNum(val*100.0));
            emit changed(true);
        }
    }
}

void QtCurveConfig::addGradStop()
{
    bool added(false);

    GradientCont::iterator cg=customGradient.find((EAppearance)gradCombo->currentIndex());

    if(cg==customGradient.end())
    {
        Gradient cust;

        cust.border=(EGradientBorder)gradBorder->currentIndex();
        cust.stops.insert(GradientStop(stopPosition->value()/100.0, stopValue->value()/100.0));
        customGradient[(EAppearance)gradCombo->currentIndex()]=cust;
        added=true;
        gradChanged(gradCombo->currentIndex());
        emit changed(true);
    }
    else
    {
        GradientStopCont::const_iterator it((*cg).second.stops.begin()),
                                         end((*cg).second.stops.end());
        double                           pos(stopPosition->value()/100.0),
                                         val(stopValue->value()/100.0);

        for(; it!=end; ++it)
            if(equal(pos, (*it).pos))
                if(equal(val, (*it).val))
                    return;
                else
                {
                    (*cg).second.stops.erase(it);
                    break;
                }

        unsigned int b4=(*cg).second.stops.size();
        (*cg).second.stops.insert(GradientStop(pos, val));
        if((*cg).second.stops.size()!=b4)
        {
            gradPreview->setGrad((*cg).second.stops);

            QStringList details;

            details << QString().setNum(pos*100.0)
                    << QString().setNum(val*100.0);

            QTreeWidgetItem *i=new CGradItem(gradStops, details);

            gradStops->setCurrentItem(i);
            gradStops->sortItems(0, Qt::AscendingOrder);
        }
    }
}

void QtCurveConfig::removeGradStop()
{
    QTreeWidgetItem *cur=gradStops->currentItem();

    if(cur)
    {
        QTreeWidgetItem *next=gradStops->itemBelow(cur);

        if(!next)
            next=gradStops->itemAbove(cur);

        GradientCont::iterator it=customGradient.find((EAppearance)gradCombo->currentIndex());

        if(it!=customGradient.end())
        {
            bool   ok;
            double pos=cur->text(0).toDouble(&ok)/100.0,
                   val=cur->text(1).toDouble(&ok)/100.0;

            (*it).second.stops.erase(GradientStop(pos, val));
            gradPreview->setGrad((*it).second.stops);
            emit changed(true);

            delete cur;
            if(next)
                gradStops->setCurrentItem(next);
        }
    }
}

void QtCurveConfig::updateGradStop()
{
    QTreeWidgetItem *i=gradStops->selectedItems().size() ? *(gradStops->selectedItems().begin()) : 0L;

    GradientCont::iterator cg=customGradient.find((EAppearance)gradCombo->currentIndex());

    if(i)
    {
        double curPos=i->text(0).toDouble()/100.0,
               curVal=i->text(1).toDouble()/100.0,
               newPos(stopPosition->value()/100.0),
               newVal(stopValue->value()/100.0);

        if(!equal(newPos, curPos) || !equal(newVal, curVal))
        {
            (*cg).second.stops.erase(GradientStop(curPos, curVal));
            (*cg).second.stops.insert(GradientStop(newPos, newVal));

            i->setText(0, QString().setNum(stopPosition->value()));
            i->setText(1, QString().setNum(stopValue->value()));
            gradPreview->setGrad((*cg).second.stops);
            emit changed(true);
        }
    }
    else
        addGradStop();
}

void QtCurveConfig::stopSelected()
{
    QTreeWidgetItem *i=gradStops->selectedItems().size() ? *(gradStops->selectedItems().begin()) : 0L;

    removeButton->setEnabled(i);
    updateButton->setEnabled(i);

    if(i)
    {
        stopPosition->setValue(i->text(0).toInt());
        stopValue->setValue(i->text(1).toInt());
    }
    else
    {
        stopPosition->setValue(0);
        stopValue->setValue(0);
    }
}

void QtCurveConfig::exportKDE3()
{
    if(KMessageBox::Yes==KMessageBox::questionYesNo(this, i18n("Export your current KDE4 color palette, and font, so "
                                                               "that they can be used by KDE3 applications?")))
    {
        QString      kde3Home(kdeHome(true));
        KConfig      k3globals(kde3Home+"/share/config/kdeglobals", KConfig::NoGlobals);
        KConfigGroup general(&k3globals, "General");
        KConfigGroup wm(&k3globals, "WM");

        general.writeEntry("alternateBackground", palette().color(QPalette::Active, QPalette::AlternateBase));
        general.writeEntry("background", palette().color(QPalette::Active, QPalette::Window));
        general.writeEntry("buttonBackground", palette().color(QPalette::Active, QPalette::Button));
        general.writeEntry("buttonForeground", palette().color(QPalette::Active, QPalette::ButtonText));
        general.writeEntry("foreground", palette().color(QPalette::Active, QPalette::WindowText));
        general.writeEntry("selectBackground", palette().color(QPalette::Active, QPalette::Highlight));
        general.writeEntry("selectForeground", palette().color(QPalette::Active, QPalette::HighlightedText));
        general.writeEntry("windowBackground", palette().color(QPalette::Active, QPalette::Base));
        general.writeEntry("windowForeground", palette().color(QPalette::Active, QPalette::Text));
        general.writeEntry("linkColor", palette().color(QPalette::Active, QPalette::Link));
        general.writeEntry("visitedLinkColor", palette().color(QPalette::Active, QPalette::LinkVisited));

        if(kdeHome(false)!=kde3Home)
        {
            KConfigGroup k4General(KGlobal::config(), "General");
            KConfigGroup k4wm(KGlobal::config(), "WM");
            
            // Mainly for K3B...
            wm.writeEntry("activeBackground", k4wm.readEntry("activeBackground",
                                                             palette().color(QPalette::Active, QPalette::Window)));
            wm.writeEntry("activeForeground", k4wm.readEntry("activeForeground",
                                                             palette().color(QPalette::Active, QPalette::WindowText)));
            wm.writeEntry("inactiveBackground", k4wm.readEntry("inactiveBackground",
                                                               palette().color(QPalette::Inactive, QPalette::Window)));
            wm.writeEntry("inactiveForeground", k4wm.readEntry("inactiveForeground",
                                                               palette().color(QPalette::Inactive, QPalette::WindowText)));
            // Font settings...
            general.writeEntry("font", k4General.readEntry("font", font()));
            general.writeEntry("fixed", k4General.readEntry("fixed", font()));
            general.writeEntry("desktopFont", k4General.readEntry("desktopFont", font()));
            general.writeEntry("taskbarFont", k4General.readEntry("taskbarFont", font()));
            general.writeEntry("toolBarFont", k4General.readEntry("toolBarFont", font()));
        }
    }
}

void QtCurveConfig::exportQt()
{
    if(KMessageBox::Yes==KMessageBox::questionYesNo(this, i18n("Export your current KDE4 color palette, and font, so "
                                                               "that they can be used by pure-Qt3 applications?")))
    {
        KConfig        cfg(QDir::homePath()+"/.qt/qtrc", KConfig::NoGlobals);
        KConfigGroup   gen(&cfg, "General");
        KConfigGroup   pal(&cfg, "Palette");
        KConfigGroup   kde(&cfg, "KDE");
        const QPalette &p=palette();
        int            i;
        QStringList    act,
                       inact,
                       dis;
        QString        sep("^e");

        QPalette::ColorRole roles[]={QPalette::Foreground,
                                     QPalette::Button,
                                     QPalette::Light,
                                     QPalette::Midlight,
                                     QPalette::Dark,
                                     QPalette::Mid,
                                     QPalette::Text,
                                     QPalette::BrightText,
                                     QPalette::ButtonText,
                                     QPalette::Base,
                                     QPalette::Background,
                                     QPalette::Shadow,
                                     QPalette::Highlight,
                                     QPalette::HighlightedText,
                                     QPalette::Link,
                                     QPalette::LinkVisited,
                                     QPalette::NColorRoles
                                    };
        
        for (i = 0; roles[i] != QPalette::NColorRoles; i++)
        {
            act << p.color(QPalette::Active, roles[i]).name();
            inact << p.color(QPalette::Inactive, roles[i]).name();
            dis << p.color(QPalette::Disabled, roles[i]).name();
        }

        KConfigGroup k4General(KGlobal::config(), "General");
        gen.writeEntry("font", k4General.readEntry("font", font()));
        gen.writeEntry("font", font());
        pal.writeEntry("active", act.join(sep));
        pal.writeEntry("disabled", dis.join(sep));
        pal.writeEntry("inactive", inact.join(sep));
        kde.writeEntry("contrast", QSettings(QLatin1String("Trolltech")).value("/Qt/KDE/contrast", 7).toInt());
    }
}

void QtCurveConfig::updatePreview()
{
    KTemporaryFile tempFile;

    if(tempFile.open())
    {
        KConfig cfg(tempFile.fileName(), KConfig::NoGlobals);
        bool    rv(true);

        if(rv)
        {
            setOptions(previewStyle);
            rv=writeConfig(&cfg, previewStyle, presets[defaultText].opts, true);
        }

        if(rv)
        {
            qputenv(QTCURVE_PREVIEW_CONFIG, QFile::encodeName(tempFile.fileName()));
            QStyle *style = QStyleFactory::create("qtcurve");
            if (!style)
            {
                tempFile.close();
                return;
            }
            setStyleRecursive(previewFrame, style);
            delete widgetStyle;
            widgetStyle = style;
        }
        tempFile.close();
    }
}

void QtCurveConfig::setupGradientsTab()
{
    for(int i=APPEARANCE_CUSTOM1; i<(APPEARANCE_CUSTOM1+QTC_NUM_CUSTOM_GRAD); ++i)
        gradCombo->insertItem(i-APPEARANCE_CUSTOM1, i18n("Custom gradient %1", (i-APPEARANCE_CUSTOM1)+1));

    gradCombo->setCurrentIndex(APPEARANCE_CUSTOM1);

    gradPreview=new CGradientPreview(this, previewWidgetContainer);
    QBoxLayout *layout=new QBoxLayout(QBoxLayout::TopToBottom, previewWidgetContainer);
    layout->addWidget(gradPreview);
    layout->setMargin(0);
    layout->setSpacing(0);
    QColor col(palette().color(QPalette::Active, QPalette::Button));
    previewColor->setColor(col);
    gradPreview->setColor(col);
    gradChanged(APPEARANCE_CUSTOM1);
    addButton->setGuiItem(KGuiItem(i18n("Add"), "list-add"));
    removeButton->setGuiItem(KGuiItem(i18n("Remove"), "list-remove"));
    updateButton->setGuiItem(KGuiItem(i18n("Update"), "dialog-ok"));

    stopPosition->setRange(0, 100, 5);
    stopValue->setRange(0, 200, 5);
    removeButton->setEnabled(false);
    updateButton->setEnabled(false);
    connect(gradCombo, SIGNAL(currentIndexChanged(int)), SLOT(gradChanged(int)));
    connect(previewColor, SIGNAL(changed(const QColor &)), gradPreview, SLOT(setColor(const QColor &)));
    connect(gradStops, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(editItem(QTreeWidgetItem *, int)));
    connect(gradStops, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(itemChanged(QTreeWidgetItem *, int)));
    connect(addButton, SIGNAL(clicked(bool)), SLOT(addGradStop()));
    connect(removeButton, SIGNAL(clicked(bool)), SLOT(removeGradStop()));
    connect(updateButton, SIGNAL(clicked(bool)), SLOT(updateGradStop()));
    connect(gradStops, SIGNAL(itemSelectionChanged()), SLOT(stopSelected()));
}

void QtCurveConfig::setupShadesTab()
{
    int shade(0);

    setupShade(shade0, shade++);
    setupShade(shade1, shade++);
    setupShade(shade2, shade++);
    setupShade(shade3, shade++);
    setupShade(shade4, shade++);
    setupShade(shade5, shade++);
    connect(customShading, SIGNAL(toggled(bool)), SLOT(updateChanged()));
}

void QtCurveConfig::setupShade(KDoubleNumInput *w, int shade)
{
    w->setRange(0.0, 2.0, 0.05, false);
    connect(w, SIGNAL(valueChanged(double)), SLOT(updateChanged()));
    shadeVals[shade]=w;
}

void QtCurveConfig::populateShades(const Options &opts)
{
    QTC_SHADES
    int contrast=QSettings(QLatin1String("Trolltech")).value("/Qt/KDE/contrast", 7).toInt();

    if(contrast<0 || contrast>10)
        contrast=7;

    customShading->setChecked(QTC_USE_CUSTOM_SHADES(opts));

    for(int i=0; i<NUM_STD_SHADES; ++i)
        shadeVals[i]->setValue(QTC_USE_CUSTOM_SHADES(opts)
                                  ? opts.customShades[i]
                                  : shades[SHADING_SIMPLE==shading->currentIndex()
                                            ? 1 : 0]
                                          [contrast]
                                          [i]);
}

bool QtCurveConfig::diffShades(const Options &opts)
{
    if( (!QTC_USE_CUSTOM_SHADES(opts) && customShading->isChecked()) ||
        (QTC_USE_CUSTOM_SHADES(opts) && !customShading->isChecked()) )
        return true;

    if(customShading->isChecked())
    {
        for(int i=0; i<NUM_STD_SHADES; ++i)
            if(!equal(shadeVals[i]->value(), opts.customShades[i]))
                return true;
    }

    return false;
}

void QtCurveConfig::setPasswordChar(int ch)
{
    QString     str;
    QTextStream s(&str);

    s.setIntegerBase(16);
    s << QChar(ch) << " (" << ch << ')';
    passwordChar->setText(str);
}

void QtCurveConfig::updateChanged()
{
    if (settingsChanged())
        emit changed(true);
}

void QtCurveConfig::focusChanged()
{
    if(ROUND_MAX==round->currentIndex() && FOCUS_LINE!=focus->currentIndex())
        round->setCurrentIndex(ROUND_EXTRA);
    updateChanged();
}

void QtCurveConfig::roundChanged()
{
    if(ROUND_MAX==round->currentIndex() && FOCUS_LINE!=focus->currentIndex())
        focus->setCurrentIndex(FOCUS_LINE);

    if(round->currentIndex()>ROUND_FULL && IND_COLORED==defBtnIndicator->currentIndex())
        defBtnIndicator->setCurrentIndex(IND_TINT);
    updateChanged();
}

void QtCurveConfig::setPreset()
{
    Preset &p(presets[presetsCombo->currentText()]);

    if(!p.loaded)
        readConfig(p.fileName, &p.opts, &presets[defaultText].opts);

    setWidgetOptions(p.opts);
    if (settingsChanged(previewStyle))
        updatePreview();
    if (settingsChanged())
        emit changed(true);

    deleteButton->setEnabled(currentText!=presetsCombo->currentText() &&
                             defaultText!=presetsCombo->currentText() &&
                             0==presets[presetsCombo->currentText()].fileName.indexOf(QDir::homePath()));
}

void QtCurveConfig::savePreset()
{
    QString name=getPresetName(i18n("Save Preset"), i18n("Please enter a name for the preset:"),
                               currentText==presetsCombo->currentText() || defaultText==presetsCombo->currentText()
                                ? i18n("New preset")
                                : 0==presets[presetsCombo->currentText()].fileName.indexOf(QDir::homePath())
                                    ? presetsCombo->currentText()
                                    : i18n("%1 New", presetsCombo->currentText()));

    if(!name.isEmpty() && !savePreset(name))
        KMessageBox::error(this, i18n("Sorry, failed to save preset"));
}

bool QtCurveConfig::savePreset(const QString &name)
{
    QString dir(KGlobal::dirs()->saveLocation("data", "QtCurve/", KStandardDirs::NoDuplicates));

    KConfig cfg(dir+name+QTC_EXTENSION, KConfig::NoGlobals);
    Options opts;

    setOptions(opts);
    if(writeConfig(&cfg, opts, presets[defaultText].opts, true))
    {
        QMap<QString, Preset>::iterator it(presets.find(name)),
                                        end(presets.end());

        presets[name]=Preset(opts, dir+name+QTC_EXTENSION);
        if(it==end)
        {
            presetsCombo->insertItem(0, name);
            presetsCombo->model()->sort(0);
            int index=-1;

            for(int i=0; i<presetsCombo->count() && -1==index; ++i)
                if(presetsCombo->itemText(i)==name)
                    index=i;

            presetsCombo->setCurrentIndex(index);
            setPreset();
        }

        return true;
    }

    return false;
}

QString QtCurveConfig::getPresetName(const QString &cap, QString label, QString def, QString name)
{
    QRegExp          exp("\\w+[^\\0042\\0044\\0045\\0046\\0047\\0052\\0057\\0077\\0137\\0140]*");
    QRegExpValidator validator(exp, this);
 
    while(true)
    {
        if(name.isEmpty())
            name=KInputDialog::getText(cap, label, def, 0, this, &validator);

        if(!name.isEmpty())
        {
            name=name.replace('\"', ' ')
                     .replace('$', ' ')
                     .replace('%', ' ')
                     .replace('&', ' ')
                     .replace('\'', ' ')
                     .replace('*', ' ')
                     .replace('/', ' ')
                     .replace('?', ' ')
                     .replace('_', ' ')
                     .replace('`', ' ')
                     .simplified();

            if(name==currentText || name==defaultText)
            {
                label=i18n("<p>You cannot use the name \"%1\".</p>"
                           "<p>Please enter a different name:<p>", name);
                def=i18n("%1 New", name);
                name=QString();
            }
            else
            {
                QMap<QString, Preset>::iterator it(presets.find(name)),
                                                end(presets.end());

                if(it!=end)
                {
                    if(0!=(*it).fileName.indexOf(QDir::homePath()))
                    {
                        label=i18n("<p>A system defined preset named\"%1\" aleady exists.</p>"
                                   "<p>Please enter a new name:<p>", name);
                        def=i18n("%1 New", name);
                        name=QString();
                    }
                    else
                        if(name==presetsCombo->currentText() ||
                            KMessageBox::Yes==KMessageBox::warningYesNo(this, i18n("<p>A preset named \"%1\" "
                                                                                   "aleady exists.</p>"
                                                                                   "<p>Do you wish to overwrite this?<p>",
                                                                                   name)))
                            return name;
                        else
                        {
                            label=i18n("<p>Please enter a new name:<p>");
                            def=i18n("%1 New", name);
                            name=QString();
                        }
                }
                else
                    return name;
            }
        }
        else
            return QString();
    }

    return QString();
}

void QtCurveConfig::deletePreset()
{
    if(KMessageBox::Yes==KMessageBox::warningYesNo(this, i18n("<p>Are you sure you wish to delete:</p><p><b>%1</b></p>",
                                                              presetsCombo->currentText())))
        if(QFile::remove(presets[presetsCombo->currentText()].fileName))
        {
            presets.remove(presetsCombo->currentText());
            presetsCombo->removeItem(presetsCombo->currentIndex());
        }
        else
            KMessageBox::error(this, i18n("<p>Sorry, failed to remove the preset file:</p><p><i>%1</i></p>",
                                          presets[presetsCombo->currentText()].fileName));
}
    
void QtCurveConfig::importPreset()
{
    QString file(KFileDialog::getOpenFileName(KUrl(),
                                              i18n("*"QTC_EXTENSION"|QtCurve Settings Files\n"
                                                   QTC_THEME_PREFIX"*"QTC_THEME_SUFFIX"|QtCurve KDE Theme Files"), this));

    if(!file.isEmpty())
    {
        QString fileName(QFileInfo(file).fileName()),
                name(fileName.remove(QTC_EXTENSION).replace('_', ' '));
        Options opts;

        if(name.isEmpty())
            KMessageBox::error(this, i18n("<p>Sorry, failed to load file.</p><p><i>Empty preset name?</i></p>"));
        else if(name==currentText || name==defaultText)
            KMessageBox::error(this, i18n("<p>Sorry, failed to load file.</p><p><i>Cannot have a preset named "
                                          "\"%1\"</i></p>", name));
        else if (readConfig(file, &opts, &presets[defaultText].opts))
        {
            name=getPresetName(i18n("Import Preset"), QString(), name, name);
            if(!name.isEmpty())
            {
                setWidgetOptions(opts);
                savePreset(name);
            }
        }
        else
            KMessageBox::error(this, i18n("Sorry, failed to load file."));
    }
}

void QtCurveConfig::exportPreset()
{
#ifdef QTC_STYLE_SUPPORT
    switch(KMessageBox::questionYesNoCancel(this, i18n("<p>In which format would you like to export the QtCurve "
                                                       "settings?<ul><li><i>QtCurve settings file</i> -"
                                                       " a file to be imported via this config dialog.</li>"
                                                       "<li><i>Standalone theme</i> - a style that user\'s can "
                                                       " select from the KDE style panel.</li></ul></p>")),
                                            i18n("QtCurve Settings File"), i18n("Standalone Theme"))
    {
        case KMessageBox::No:
            exportTheme();
        case KMessageBox::Cancel:
            return;
        case KMessageBox::Yes:
            break;
    }
#endif

    QString file(KFileDialog::getSaveFileName(KUrl(), i18n("*"QTC_EXTENSION"|QtCurve Settings Files"), this));

    if(!file.isEmpty())
    {
        KConfig cfg(file, KConfig::NoGlobals);
        bool    rv(true);

        if(rv)
        {
            Options opts;

            setOptions(opts);
            rv=writeConfig(&cfg, opts, presets[defaultText].opts, true);
        }

        if(!rv)
            KMessageBox::error(this, i18n("Could not write to file:\n%1").arg(file));
    }
}

void QtCurveConfig::exportTheme()
{
#ifdef QTC_STYLE_SUPPORT
    if(!exportDialog)
        exportDialog=new CExportThemeDialog(this);

    Options opts;

    setOptions(opts);
    exportDialog->run(opts);
#endif
}

int QtCurveConfig::getTitleBarButtonFlags()
{
    int titlebarButtons=0;

    if(titlebarButtons_button->isChecked())
        titlebarButtons+=QTC_TITLEBAR_BUTTON_STD_COLOR;
    if(titlebarButtons_custom->isChecked())
        titlebarButtons+=QTC_TITLEBAR_BUTTON_COLOR;
    if(titlebarButtons_noFrame->isChecked())
        titlebarButtons+=QTC_TITLEBAR_BUTTON_NO_FRAME;
    if(titlebarButtons_round->isChecked())
        titlebarButtons+=QTC_TITLEBAR_BUTTON_ROUND;
    if(titlebarButtons_hoverFrame->isChecked())
        titlebarButtons+=QTC_TITLEBAR_BUTTON_HOVER_FRAME;
    if(titlebarButtons_hoverSymbol->isChecked())
        titlebarButtons+=QTC_TITLEBAR_BUTTON_HOVER_SYMBOL;
    if(titlebarButtons_hoverSymbolFull->isChecked())
        titlebarButtons+=QTC_TITLEBAR_BUTTON_HOVER_SYMBOL_FULL;
    if(titlebarButtons_colorOnMouseOver->isChecked())
        titlebarButtons+=QTC_TITLEBAR_BUTTON_COLOR_MOUSE_OVER;
    if(titlebarButtons_colorInactive->isChecked())
        titlebarButtons+=QTC_TITLEBAR_BUTTON_COLOR_INACTIVE;
    if(titlebarButtons_colorSymbolsOnly->isChecked())
        titlebarButtons+=QTC_TITLEBAR_BUTTON_COLOR_SYMBOL;
    return titlebarButtons;
}

void QtCurveConfig::setOptions(Options &opts)
{
    opts.round=(ERound)round->currentIndex();
    opts.toolbarBorders=(ETBarBorder)toolbarBorders->currentIndex();
    opts.appearance=(EAppearance)appearance->currentIndex();
    opts.focus=(EFocus)focus->currentIndex();
    opts.lvLines=(ELvLines)lvLines->currentIndex();
    opts.lvButton=lvButton->isChecked();
    opts.drawStatusBarFrames=drawStatusBarFrames->isChecked();
    opts.buttonEffect=(EEffect)buttonEffect->currentIndex();
    opts.coloredMouseOver=(EMouseOver)coloredMouseOver->currentIndex();
    opts.menubarMouseOver=menubarMouseOver->isChecked();
    opts.shadeMenubarOnlyWhenActive=shadeMenubarOnlyWhenActive->isChecked();
    opts.thinnerMenuItems=thinnerMenuItems->isChecked();
    opts.thinnerBtns=thinnerBtns->isChecked();
    opts.fixParentlessDialogs=fixParentlessDialogs->isChecked();
    opts.animatedProgress=animatedProgress->isChecked();
    opts.stripedProgress=(EStripe)stripedProgress->currentIndex();
    opts.lighterPopupMenuBgnd=lighterPopupMenuBgnd->value();
    opts.tabBgnd=tabBgnd->value();
    opts.menuDelay=menuDelay->value();
    opts.sliderWidth=sliderWidth->value();
    opts.menuStripe=(EShade)menuStripe->currentIndex();
    opts.customMenuStripeColor=customMenuStripeColor->color();
    opts.menuStripeAppearance=(EAppearance)menuStripeAppearance->currentIndex();
    opts.gtkMenuStripe=gtkMenuStripe->isChecked();
    opts.bgndGrad=(EGradType)bgndGrad->currentIndex();
    opts.menuBgndGrad=(EGradType)menuBgndGrad->currentIndex();
    opts.embolden=embolden->isChecked();
    opts.scrollbarType=(EScrollbar)scrollbarType->currentIndex();
    opts.defBtnIndicator=(EDefBtnIndicator)defBtnIndicator->currentIndex();
    opts.sliderThumbs=(ELine)sliderThumbs->currentIndex();
    opts.handles=(ELine)handles->currentIndex();
    opts.highlightTab=highlightTab->isChecked();
    opts.shadeSliders=(EShade)shadeSliders->currentIndex();
    opts.shadeMenubars=(EShade)shadeMenubars->currentIndex();
    opts.menubarAppearance=(EAppearance)menubarAppearance->currentIndex();
    opts.toolbarAppearance=(EAppearance)toolbarAppearance->currentIndex();
    opts.lvAppearance=(EAppearance)lvAppearance->currentIndex();
    opts.sliderAppearance=(EAppearance)sliderAppearance->currentIndex();
    opts.tabAppearance=(EAppearance)tabAppearance->currentIndex();
    opts.activeTabAppearance=(EAppearance)activeTabAppearance->currentIndex();
    opts.toolbarSeparators=(ELine)toolbarSeparators->currentIndex();
    opts.splitters=(ELine)splitters->currentIndex();
    opts.customSlidersColor=customSlidersColor->color();
    opts.customMenubarsColor=customMenubarsColor->color();
    opts.highlightFactor=highlightFactor->value();
    opts.customMenuNormTextColor=customMenuNormTextColor->color();
    opts.customMenuSelTextColor=customMenuSelTextColor->color();
    opts.customMenuTextColor=customMenuTextColor->isChecked();
    opts.fillSlider=fillSlider->isChecked();
    opts.sliderStyle=(ESliderStyle)sliderStyle->currentIndex();
    opts.roundMbTopOnly=roundMbTopOnly->isChecked();
    opts.menubarHiding=menubarHiding->isChecked();
    opts.fillProgress=fillProgress->isChecked();
    opts.darkerBorders=darkerBorders->isChecked();
    opts.comboSplitter=comboSplitter->isChecked();
    opts.comboBtn=(EShade)comboBtn->currentIndex();
    opts.customComboBtnColor=customComboBtnColor->color();
    opts.sortedLv=(EShade)sortedLv->currentIndex();
    opts.customSortedLvColor=customSortedLvColor->color();
    opts.unifySpinBtns=unifySpinBtns->isChecked();
    opts.unifySpin=unifySpin->isChecked();
    opts.unifyCombo=unifyCombo->isChecked();
    opts.vArrows=vArrows->isChecked();
    opts.xCheck=xCheck->isChecked();
    opts.crHighlight=crHighlight->value();
    opts.expanderHighlight=expanderHighlight->value();
    opts.crButton=crButton->isChecked();
    opts.colorSelTab=colorSelTab->value();
    opts.roundAllTabs=roundAllTabs->isChecked();
    opts.borderTab=borderTab->isChecked();
    opts.borderInactiveTab=borderInactiveTab->isChecked();
    opts.invertBotTab=invertBotTab->isChecked();
    opts.doubleGtkComboArrow=doubleGtkComboArrow->isChecked();
    opts.tabMouseOver=(ETabMo)tabMouseOver->currentIndex();
    opts.stdSidebarButtons=stdSidebarButtons->isChecked();
    opts.borderMenuitems=borderMenuitems->isChecked();
    opts.popupBorder=popupBorder->isChecked();
    opts.progressAppearance=(EAppearance)progressAppearance->currentIndex();
    opts.progressGrooveAppearance=(EAppearance)progressGrooveAppearance->currentIndex();
    opts.grooveAppearance=(EAppearance)grooveAppearance->currentIndex();
    opts.sunkenAppearance=(EAppearance)sunkenAppearance->currentIndex();
    opts.progressGrooveColor=(EColor)progressGrooveColor->currentIndex();
    opts.menuitemAppearance=(EAppearance)menuitemAppearance->currentIndex();
    opts.menuBgndAppearance=(EAppearance)menuBgndAppearance->currentIndex();
    opts.titlebarAppearance=(EAppearance)titlebarAppearance->currentIndex();
    opts.inactiveTitlebarAppearance=(EAppearance)inactiveTitlebarAppearance->currentIndex();
    opts.titlebarButtonAppearance=(EAppearance)titlebarButtonAppearance->currentIndex();
    opts.colorTitlebarOnly=colorTitlebarOnly->isChecked();
    opts.selectionAppearance=(EAppearance)selectionAppearance->currentIndex();
    opts.shadeCheckRadio=(EShade)shadeCheckRadio->currentIndex();
    opts.customCheckRadioColor=customCheckRadioColor->color();
    opts.shading=(EShading)shading->currentIndex();
    opts.gtkScrollViews=gtkScrollViews->isChecked();
    opts.highlightScrollViews=highlightScrollViews->isChecked();
    opts.squareScrollViews=squareScrollViews->isChecked();
    opts.etchEntry=etchEntry->isChecked();
    opts.flatSbarButtons=flatSbarButtons->isChecked();
    opts.thinSbarGroove=thinSbarGroove->isChecked();
    opts.colorSliderMouseOver=colorSliderMouseOver->isChecked();
    opts.titlebarBorder=titlebarBorder->isChecked();
    opts.sbarBgndAppearance=(EAppearance)sbarBgndAppearance->currentIndex();
    opts.sliderFill=(EAppearance)sliderFill->currentIndex();
    opts.bgndAppearance=(EAppearance)bgndAppearance->currentIndex();
    opts.dwtAppearance=(EAppearance)dwtAppearance->currentIndex();
    opts.xbar=xbar->isChecked();
    opts.crColor=crColor->isChecked();
    opts.smallRadio=smallRadio->isChecked();
    opts.splitterHighlight=splitterHighlight->value();
    opts.gtkComboMenus=gtkComboMenus->isChecked();
    opts.gtkButtonOrder=gtkButtonOrder->isChecked();
    opts.mapKdeIcons=mapKdeIcons->isChecked();
    opts.passwordChar=toInt(passwordChar->text());
    opts.framelessGroupBoxes=framelessGroupBoxes->isChecked();
    opts.customGradient=customGradient;
    opts.colorMenubarMouseOver=colorMenubarMouseOver->isChecked();
    opts.useHighlightForMenu=useHighlightForMenu->isChecked();
    opts.groupBoxLine=groupBoxLine->isChecked();
    opts.fadeLines=fadeLines->isChecked();
    opts.menuIcons=menuIcons->isChecked();
    opts.stdBtnSizes=stdBtnSizes->isChecked();
    opts.forceAlternateLvCols=forceAlternateLvCols->isChecked();
    opts.squareLvSelection=squareLvSelection->isChecked();
    opts.titlebarAlignment=(EAlign)titlebarAlignment->currentIndex();
    opts.titlebarEffect=(EEffect)titlebarEffect->currentIndex();
    opts.titlebarIcon=(ETitleBarIcon)titlebarIcon->currentIndex();
    opts.dwtSettings=getDwtSettingsFlags();

    if(customShading->isChecked())
    {
        for(int i=0; i<NUM_STD_SHADES; ++i)
            opts.customShades[i]=shadeVals[i]->value();
    }
    else
        opts.customShades[0]=0;

    opts.titlebarButtons=getTitleBarButtonFlags();
    if(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR)
    {
        opts.titlebarButtonColors[TITLEBAR_CLOSE]=titlebarButtons_colorClose->color();
        opts.titlebarButtonColors[TITLEBAR_MIN]=titlebarButtons_colorMin->color();
        opts.titlebarButtonColors[TITLEBAR_MAX]=titlebarButtons_colorMax->color();
        opts.titlebarButtonColors[TITLEBAR_KEEP_ABOVE]=titlebarButtons_colorKeepAbove->color();
        opts.titlebarButtonColors[TITLEBAR_KEEP_BELOW]=titlebarButtons_colorKeepBelow->color();
        opts.titlebarButtonColors[TITLEBAR_HELP]=titlebarButtons_colorHelp->color();
        opts.titlebarButtonColors[TITLEBAR_MENU]=titlebarButtons_colorMenu->color();
        opts.titlebarButtonColors[TITLEBAR_SHADE]=titlebarButtons_colorShade->color();
        opts.titlebarButtonColors[TITLEBAR_ALL_DESKTOPS]=titlebarButtons_colorAllDesktops->color();
    }
    else
        opts.titlebarButtonColors.clear();
}

static QColor getColor(const TBCols &cols, ETitleBarButtons btn)
{
    TBCols::const_iterator it=cols.find(btn);

    return cols.end()==it ? Qt::black : (*it).second;
}

void QtCurveConfig::setWidgetOptions(const Options &opts)
{
    round->setCurrentIndex(opts.round);
    scrollbarType->setCurrentIndex(opts.scrollbarType);
    lighterPopupMenuBgnd->setValue(opts.lighterPopupMenuBgnd);
    tabBgnd->setValue(opts.tabBgnd);
    menuDelay->setValue(opts.menuDelay);
    sliderWidth->setValue(opts.sliderWidth);
    menuStripe->setCurrentIndex(opts.menuStripe);
    customMenuStripeColor->setColor(opts.customMenuStripeColor);
    menuStripeAppearance->setCurrentIndex(opts.menuStripeAppearance);
    gtkMenuStripe->setChecked(opts.gtkMenuStripe);
    bgndGrad->setCurrentIndex(opts.bgndGrad);
    menuBgndGrad->setCurrentIndex(opts.menuBgndGrad);
    toolbarBorders->setCurrentIndex(opts.toolbarBorders);
    sliderThumbs->setCurrentIndex(opts.sliderThumbs);
    handles->setCurrentIndex(opts.handles);
    appearance->setCurrentIndex(opts.appearance);
    focus->setCurrentIndex(opts.focus);
    lvLines->setCurrentIndex(opts.lvLines);
    lvButton->setChecked(opts.lvButton);
    drawStatusBarFrames->setChecked(opts.drawStatusBarFrames);
    buttonEffect->setCurrentIndex(opts.buttonEffect);
    coloredMouseOver->setCurrentIndex(opts.coloredMouseOver);
    menubarMouseOver->setChecked(opts.menubarMouseOver);
    shadeMenubarOnlyWhenActive->setChecked(opts.shadeMenubarOnlyWhenActive);
    thinnerMenuItems->setChecked(opts.thinnerMenuItems);
    thinnerBtns->setChecked(opts.thinnerBtns);
    fixParentlessDialogs->setChecked(opts.fixParentlessDialogs);
    animatedProgress->setChecked(opts.animatedProgress);
    stripedProgress->setCurrentIndex(opts.stripedProgress);
    embolden->setChecked(opts.embolden);
    defBtnIndicator->setCurrentIndex(opts.defBtnIndicator);
    highlightTab->setChecked(opts.highlightTab);
    menubarAppearance->setCurrentIndex(opts.menubarAppearance);
    toolbarAppearance->setCurrentIndex(opts.toolbarAppearance);
    lvAppearance->setCurrentIndex(opts.lvAppearance);
    sliderAppearance->setCurrentIndex(opts.sliderAppearance);
    tabAppearance->setCurrentIndex(opts.tabAppearance);
    activeTabAppearance->setCurrentIndex(opts.activeTabAppearance);
    toolbarSeparators->setCurrentIndex(opts.toolbarSeparators);
    splitters->setCurrentIndex(opts.splitters);
    shadeSliders->setCurrentIndex(opts.shadeSliders);
    shadeMenubars->setCurrentIndex(opts.shadeMenubars);
    highlightFactor->setValue(opts.highlightFactor);
    customSlidersColor->setColor(opts.customSlidersColor);
    customMenubarsColor->setColor(opts.customMenubarsColor);
    customMenuNormTextColor->setColor(opts.customMenuNormTextColor);
    customMenuSelTextColor->setColor(opts.customMenuSelTextColor);
    customMenuTextColor->setChecked(opts.customMenuTextColor);

    customSlidersColor->setEnabled(SHADE_CUSTOM==opts.shadeSliders);
    customMenubarsColor->setEnabled(SHADE_CUSTOM==opts.shadeMenubars);
    customMenuNormTextColor->setEnabled(customMenuTextColor->isChecked());
    customMenuSelTextColor->setEnabled(customMenuTextColor->isChecked());
    customCheckRadioColor->setEnabled(SHADE_CUSTOM==opts.shadeCheckRadio);
    customMenuStripeColor->setEnabled(SHADE_CUSTOM==opts.menuStripe);
    menuStripeAppearance->setEnabled(SHADE_NONE!=opts.menuStripe);

    animatedProgress->setEnabled(STRIPE_NONE!=stripedProgress->currentIndex());

    fillSlider->setChecked(opts.fillSlider);
    sliderStyle->setCurrentIndex(opts.sliderStyle);
    roundMbTopOnly->setChecked(opts.roundMbTopOnly);
    menubarHiding->setChecked(opts.menubarHiding);
    fillProgress->setChecked(opts.fillProgress);
    darkerBorders->setChecked(opts.darkerBorders);
    comboSplitter->setChecked(opts.comboSplitter);
    comboBtn->setCurrentIndex(opts.comboBtn);
    customComboBtnColor->setColor(opts.customComboBtnColor);
    sortedLv->setCurrentIndex(opts.sortedLv);
    customSortedLvColor->setColor(opts.customSortedLvColor);
    unifySpinBtns->setChecked(opts.unifySpinBtns);
    unifySpin->setChecked(opts.unifySpin);
    unifyCombo->setChecked(opts.unifyCombo);
    vArrows->setChecked(opts.vArrows);
    xCheck->setChecked(opts.xCheck);
    xCheck_false->setChecked(!opts.xCheck);
    crHighlight->setValue(opts.crHighlight);
    expanderHighlight->setValue(opts.expanderHighlight);
    crButton->setChecked(opts.crButton);
    colorSelTab->setValue(opts.colorSelTab);
    roundAllTabs->setChecked(opts.roundAllTabs);
    roundAllTabs_false->setChecked(!opts.roundAllTabs);
    borderTab->setChecked(opts.borderTab);
    borderInactiveTab->setChecked(opts.borderInactiveTab);
    invertBotTab->setChecked(opts.invertBotTab);
    doubleGtkComboArrow->setChecked(opts.doubleGtkComboArrow);
    borderTab_false->setChecked(!opts.borderTab);
    borderInactiveTab_false->setChecked(!opts.borderInactiveTab);
    invertBotTab_false->setChecked(!opts.invertBotTab);
    tabMouseOver->setCurrentIndex(opts.tabMouseOver);
    stdSidebarButtons->setChecked(opts.stdSidebarButtons);
    borderMenuitems->setChecked(opts.borderMenuitems);
    popupBorder->setChecked(opts.popupBorder);
    progressAppearance->setCurrentIndex(opts.progressAppearance);
    progressGrooveAppearance->setCurrentIndex(opts.progressGrooveAppearance);
    grooveAppearance->setCurrentIndex(opts.grooveAppearance);
    sunkenAppearance->setCurrentIndex(opts.sunkenAppearance);
    progressGrooveColor->setCurrentIndex(opts.progressGrooveColor);
    menuitemAppearance->setCurrentIndex(opts.menuitemAppearance);
    menuBgndAppearance->setCurrentIndex(opts.menuBgndAppearance);
    titlebarAppearance->setCurrentIndex(opts.titlebarAppearance);
    inactiveTitlebarAppearance->setCurrentIndex(opts.inactiveTitlebarAppearance);
    titlebarButtonAppearance->setCurrentIndex(opts.titlebarButtonAppearance);
    colorTitlebarOnly->setChecked(opts.colorTitlebarOnly);
    colorTitlebarOnly_false->setChecked(!opts.colorTitlebarOnly);
    selectionAppearance->setCurrentIndex(opts.selectionAppearance);
    shadeCheckRadio->setCurrentIndex(opts.shadeCheckRadio);
    customCheckRadioColor->setColor(opts.customCheckRadioColor);
    colorMenubarMouseOver->setChecked(opts.colorMenubarMouseOver);
    useHighlightForMenu->setChecked(opts.useHighlightForMenu);
    groupBoxLine->setChecked(opts.groupBoxLine);
    fadeLines->setChecked(opts.fadeLines);
    menuIcons->setChecked(opts.menuIcons);
    stdBtnSizes->setChecked(opts.stdBtnSizes);
    forceAlternateLvCols->setChecked(opts.forceAlternateLvCols);
    squareLvSelection->setChecked(opts.squareLvSelection);
    titlebarAlignment->setCurrentIndex(opts.titlebarAlignment);
    titlebarEffect->setCurrentIndex(opts.titlebarEffect);
    titlebarIcon->setCurrentIndex(opts.titlebarIcon);

    shading->setCurrentIndex(opts.shading);
    gtkScrollViews->setChecked(opts.gtkScrollViews);
    highlightScrollViews->setChecked(opts.highlightScrollViews);
    squareScrollViews->setChecked(opts.squareScrollViews);
    etchEntry->setChecked(opts.etchEntry);
    flatSbarButtons->setChecked(opts.flatSbarButtons);
    thinSbarGroove->setChecked(opts.thinSbarGroove);
    colorSliderMouseOver->setChecked(opts.colorSliderMouseOver);
    titlebarBorder->setChecked(opts.titlebarBorder);
    sbarBgndAppearance->setCurrentIndex(opts.sbarBgndAppearance);
    sliderFill->setCurrentIndex(opts.sliderFill);
    bgndAppearance->setCurrentIndex(opts.bgndAppearance);
    dwtAppearance->setCurrentIndex(opts.dwtAppearance);
    dwtBtnAsPerTitleBar->setChecked(opts.dwtSettings&QTC_DWT_BUTTONS_AS_PER_TITLEBAR);
    dwtColAsPerTitleBar->setChecked(opts.dwtSettings&QTC_DWT_COLOR_AS_PER_TITLEBAR);
    dwtFontAsPerTitleBar->setChecked(opts.dwtSettings&QTC_DWT_FONT_AS_PER_TITLEBAR);
    dwtTextAsPerTitleBar->setChecked(opts.dwtSettings&QTC_DWT_TEXT_ALIGN_AS_PER_TITLEBAR);
    dwtEffectAsPerTitleBar->setChecked(opts.dwtSettings&QTC_DWT_EFFECT_AS_PER_TITLEBAR);
    dwtRoundTopOnly->setChecked(opts.dwtSettings&QTC_DWT_ROUND_TOP_ONLY);
    xbar->setChecked(opts.xbar);
    crColor->setChecked(opts.crColor);
    smallRadio->setChecked(opts.smallRadio);
    smallRadio_false->setChecked(!opts.smallRadio);
    splitterHighlight->setValue(opts.splitterHighlight);
    gtkComboMenus->setChecked(opts.gtkComboMenus);
    gtkButtonOrder->setChecked(opts.gtkButtonOrder);
    mapKdeIcons->setChecked(opts.mapKdeIcons);
    setPasswordChar(opts.passwordChar);
    framelessGroupBoxes->setChecked(opts.framelessGroupBoxes);
    customGradient=opts.customGradient;
    gradCombo->setCurrentIndex(APPEARANCE_CUSTOM1);

    if(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR)
    {
        titlebarButtons_colorClose->setColor(getColor(opts.titlebarButtonColors, TITLEBAR_CLOSE));
        titlebarButtons_colorMin->setColor(getColor(opts.titlebarButtonColors, TITLEBAR_MIN));
        titlebarButtons_colorMax->setColor(getColor(opts.titlebarButtonColors, TITLEBAR_MAX));
        titlebarButtons_colorKeepAbove->setColor(getColor(opts.titlebarButtonColors, TITLEBAR_KEEP_ABOVE));
        titlebarButtons_colorKeepBelow->setColor(getColor(opts.titlebarButtonColors, TITLEBAR_KEEP_BELOW));
        titlebarButtons_colorHelp->setColor(getColor(opts.titlebarButtonColors, TITLEBAR_HELP));
        titlebarButtons_colorMenu->setColor(getColor(opts.titlebarButtonColors, TITLEBAR_MENU));
        titlebarButtons_colorShade->setColor(getColor(opts.titlebarButtonColors, TITLEBAR_SHADE));
        titlebarButtons_colorAllDesktops->setColor(getColor(opts.titlebarButtonColors, TITLEBAR_ALL_DESKTOPS));
    }
    else
    {
        QColor col(palette().color(QPalette::Active, QPalette::Button));
    
        titlebarButtons_colorClose->setColor(col);
        titlebarButtons_colorMin->setColor(col);
        titlebarButtons_colorMax->setColor(col);
        titlebarButtons_colorKeepAbove->setColor(col);
        titlebarButtons_colorKeepBelow->setColor(col);
        titlebarButtons_colorHelp->setColor(col);
        titlebarButtons_colorMenu->setColor(col);
        titlebarButtons_colorShade->setColor(col);
        titlebarButtons_colorAllDesktops->setColor(col);
    }

    titlebarButtons_button->setChecked(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_STD_COLOR);
    titlebarButtons_custom->setChecked(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR);
    titlebarButtons_noFrame->setChecked(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_NO_FRAME);
    titlebarButtons_round->setChecked(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_ROUND);
    titlebarButtons_hoverFrame->setChecked(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_HOVER_FRAME);
    titlebarButtons_hoverSymbol->setChecked(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_HOVER_SYMBOL);
    titlebarButtons_hoverSymbolFull->setChecked(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_HOVER_SYMBOL_FULL);
    titlebarButtons_colorOnMouseOver->setChecked(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR_MOUSE_OVER);
    titlebarButtons_colorInactive->setChecked(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR_INACTIVE);
    titlebarButtons_colorSymbolsOnly->setChecked(opts.titlebarButtons&QTC_TITLEBAR_BUTTON_COLOR_SYMBOL);

    populateShades(opts);
}

int QtCurveConfig::getDwtSettingsFlags()
{
    int dwt(0);

    if(dwtBtnAsPerTitleBar->isChecked())
        dwt|=QTC_DWT_BUTTONS_AS_PER_TITLEBAR;
    if(dwtColAsPerTitleBar->isChecked())
        dwt|=QTC_DWT_COLOR_AS_PER_TITLEBAR;
    if(dwtFontAsPerTitleBar->isChecked())
        dwt|=QTC_DWT_FONT_AS_PER_TITLEBAR;
    if(dwtTextAsPerTitleBar->isChecked())
        dwt|=QTC_DWT_TEXT_ALIGN_AS_PER_TITLEBAR;
    if(dwtEffectAsPerTitleBar->isChecked())
        dwt|=QTC_DWT_EFFECT_AS_PER_TITLEBAR;
    if(dwtRoundTopOnly->isChecked())
        dwt|=QTC_DWT_ROUND_TOP_ONLY;
    return dwt;
}

bool QtCurveConfig::diffTitleBarButtonColors(const Options &opts)
{
    return titlebarButtons_custom->isChecked() &&
           ( titlebarButtons_colorClose->color()!=getColor(opts.titlebarButtonColors, TITLEBAR_CLOSE) ||
             titlebarButtons_colorMin->color()!=getColor(opts.titlebarButtonColors, TITLEBAR_MIN) ||
             titlebarButtons_colorMax->color()!=getColor(opts.titlebarButtonColors, TITLEBAR_MAX) ||
             titlebarButtons_colorKeepAbove->color()!=getColor(opts.titlebarButtonColors, TITLEBAR_KEEP_ABOVE) ||
             titlebarButtons_colorKeepBelow->color()!=getColor(opts.titlebarButtonColors, TITLEBAR_KEEP_BELOW) ||
             titlebarButtons_colorHelp->color()!=getColor(opts.titlebarButtonColors, TITLEBAR_HELP) ||
             titlebarButtons_colorMenu->color()!=getColor(opts.titlebarButtonColors, TITLEBAR_MENU) ||
             titlebarButtons_colorShade->color()!=getColor(opts.titlebarButtonColors, TITLEBAR_SHADE) ||
             titlebarButtons_colorAllDesktops->color()!=getColor(opts.titlebarButtonColors, TITLEBAR_ALL_DESKTOPS));
}

bool QtCurveConfig::settingsChanged(const Options &opts)
{
    return round->currentIndex()!=opts.round ||
         toolbarBorders->currentIndex()!=opts.toolbarBorders ||
         appearance->currentIndex()!=(int)opts.appearance ||
         focus->currentIndex()!=(int)opts.focus ||
         lvLines->currentIndex()!=(int)opts.lvLines ||
         lvButton->isChecked()!=opts.lvButton ||
         drawStatusBarFrames->isChecked()!=opts.drawStatusBarFrames ||
         buttonEffect->currentIndex()!=(EEffect)opts.buttonEffect ||
         coloredMouseOver->currentIndex()!=(int)opts.coloredMouseOver ||
         menubarMouseOver->isChecked()!=opts.menubarMouseOver ||
         shadeMenubarOnlyWhenActive->isChecked()!=opts.shadeMenubarOnlyWhenActive ||
         thinnerMenuItems->isChecked()!=opts.thinnerMenuItems ||
         thinnerBtns->isChecked()!=opts.thinnerBtns ||
         fixParentlessDialogs->isChecked()!=opts.fixParentlessDialogs ||
         animatedProgress->isChecked()!=opts.animatedProgress ||
         stripedProgress->currentIndex()!=opts.stripedProgress ||
         lighterPopupMenuBgnd->value()!=opts.lighterPopupMenuBgnd ||
         tabBgnd->value()!=opts.tabBgnd ||
         menuDelay->value()!=opts.menuDelay ||
         sliderWidth->value()!=opts.sliderWidth ||
         menuStripe->currentIndex()!=opts.menuStripe ||
         menuStripeAppearance->currentIndex()!=opts.menuStripeAppearance ||
         gtkMenuStripe->isChecked()!=opts.gtkMenuStripe ||
         bgndGrad->currentIndex()!=opts.bgndGrad ||
         menuBgndGrad->currentIndex()!=opts.menuBgndGrad ||
         embolden->isChecked()!=opts.embolden ||
         fillSlider->isChecked()!=opts.fillSlider ||
         sliderStyle->currentIndex()!=opts.sliderStyle ||
         roundMbTopOnly->isChecked()!=opts.roundMbTopOnly ||
         menubarHiding->isChecked()!=opts.menubarHiding ||
         fillProgress->isChecked()!=opts.fillProgress ||
         darkerBorders->isChecked()!=opts.darkerBorders ||
         comboSplitter->isChecked()!=opts.comboSplitter ||
         comboBtn->currentIndex()!=(int)opts.comboBtn ||
         sortedLv->currentIndex()!=(int)opts.sortedLv ||
         unifySpinBtns->isChecked()!=opts.unifySpinBtns ||
         unifySpin->isChecked()!=opts.unifySpin ||
         unifyCombo->isChecked()!=opts.unifyCombo ||
         vArrows->isChecked()!=opts.vArrows ||
         xCheck->isChecked()!=opts.xCheck ||
         crHighlight->value()!=opts.crHighlight ||
         expanderHighlight->value()!=opts.expanderHighlight ||
         crButton->isChecked()!=opts.crButton ||
         colorSelTab->value()!=opts.colorSelTab ||
         roundAllTabs->isChecked()!=opts.roundAllTabs ||
         borderTab->isChecked()!=opts.borderTab ||
         borderInactiveTab->isChecked()!=opts.borderInactiveTab ||
         invertBotTab->isChecked()!=opts.invertBotTab ||
         doubleGtkComboArrow->isChecked()!=opts.doubleGtkComboArrow ||
         tabMouseOver->currentIndex()!=opts.tabMouseOver ||
         stdSidebarButtons->isChecked()!=opts.stdSidebarButtons ||
         borderMenuitems->isChecked()!=opts.borderMenuitems ||
         popupBorder->isChecked()!=opts.popupBorder ||
         defBtnIndicator->currentIndex()!=(int)opts.defBtnIndicator ||
         sliderThumbs->currentIndex()!=(int)opts.sliderThumbs ||
         handles->currentIndex()!=(int)opts.handles ||
         scrollbarType->currentIndex()!=(int)opts.scrollbarType ||
         highlightTab->isChecked()!=opts.highlightTab ||
         shadeSliders->currentIndex()!=(int)opts.shadeSliders ||
         shadeMenubars->currentIndex()!=(int)opts.shadeMenubars ||
         shadeCheckRadio->currentIndex()!=(int)opts.shadeCheckRadio ||
         menubarAppearance->currentIndex()!=opts.menubarAppearance ||
         toolbarAppearance->currentIndex()!=opts.toolbarAppearance ||
         lvAppearance->currentIndex()!=opts.lvAppearance ||
         sliderAppearance->currentIndex()!=opts.sliderAppearance ||
         tabAppearance->currentIndex()!=opts.tabAppearance ||
         activeTabAppearance->currentIndex()!=opts.activeTabAppearance ||
         progressAppearance->currentIndex()!=opts.progressAppearance ||
         progressGrooveAppearance->currentIndex()!=opts.progressGrooveAppearance ||
         grooveAppearance->currentIndex()!=opts.grooveAppearance ||
         sunkenAppearance->currentIndex()!=opts.sunkenAppearance ||
         progressGrooveColor->currentIndex()!=opts.progressGrooveColor ||
         menuitemAppearance->currentIndex()!=opts.menuitemAppearance ||
         menuBgndAppearance->currentIndex()!=opts.menuBgndAppearance ||
         titlebarAppearance->currentIndex()!=opts.titlebarAppearance ||
         inactiveTitlebarAppearance->currentIndex()!=opts.inactiveTitlebarAppearance ||
         titlebarButtonAppearance->currentIndex()!=opts.titlebarButtonAppearance ||
         colorTitlebarOnly->isChecked()!=opts.colorTitlebarOnly ||
         selectionAppearance->currentIndex()!=opts.selectionAppearance ||
         toolbarSeparators->currentIndex()!=opts.toolbarSeparators ||
         splitters->currentIndex()!=opts.splitters ||
         colorMenubarMouseOver->isChecked()!=opts.colorMenubarMouseOver ||
         useHighlightForMenu->isChecked()!=opts.useHighlightForMenu ||
         groupBoxLine->isChecked()!=opts.groupBoxLine ||
         fadeLines->isChecked()!=opts.fadeLines ||
         menuIcons->isChecked()!=opts.menuIcons ||
         stdBtnSizes->isChecked()!=opts.stdBtnSizes ||
         forceAlternateLvCols->isChecked()!=opts.forceAlternateLvCols ||
         squareLvSelection->isChecked()!=opts.squareLvSelection ||
         titlebarAlignment->currentIndex()!=opts.titlebarAlignment ||
         titlebarEffect->currentIndex()!=opts.titlebarEffect ||
         titlebarIcon->currentIndex()!=opts.titlebarIcon ||

         shading->currentIndex()!=(int)opts.shading ||
         gtkScrollViews->isChecked()!=opts.gtkScrollViews ||
         highlightScrollViews->isChecked()!=opts.highlightScrollViews ||
         squareScrollViews->isChecked()!=opts.squareScrollViews ||
         etchEntry->isChecked()!=opts.etchEntry ||
         flatSbarButtons->isChecked()!=opts.flatSbarButtons ||
         thinSbarGroove->isChecked()!=opts.thinSbarGroove ||
         colorSliderMouseOver->isChecked()!=opts.colorSliderMouseOver ||
         titlebarBorder->isChecked()!=opts.titlebarBorder ||
         sbarBgndAppearance->currentIndex()!=opts.sbarBgndAppearance ||
         sliderFill->currentIndex()!=opts.sliderFill ||
         bgndAppearance->currentIndex()!=opts.bgndAppearance ||
         dwtAppearance->currentIndex()!=opts.dwtAppearance ||
         xbar->isChecked()!=opts.xbar ||
         crColor->isChecked()!=opts.crColor ||
         smallRadio->isChecked()!=opts.smallRadio ||
         splitterHighlight->value()!=opts.splitterHighlight ||
         gtkComboMenus->isChecked()!=opts.gtkComboMenus ||
         gtkButtonOrder->isChecked()!=opts.gtkButtonOrder ||
         mapKdeIcons->isChecked()!=opts.mapKdeIcons ||
         framelessGroupBoxes->isChecked()!=opts.framelessGroupBoxes ||

         toInt(passwordChar->text())!=opts.passwordChar ||
         highlightFactor->value()!=opts.highlightFactor ||
         getTitleBarButtonFlags()!=opts.titlebarButtons ||

         getDwtSettingsFlags()!=opts.dwtSettings ||

         diffTitleBarButtonColors(opts) ||
         
         customMenuTextColor->isChecked()!=opts.customMenuTextColor ||
         (SHADE_CUSTOM==opts.shadeSliders &&
               customSlidersColor->color()!=opts.customSlidersColor) ||
         (SHADE_CUSTOM==opts.shadeMenubars &&
               customMenubarsColor->color()!=opts.customMenubarsColor) ||
         (SHADE_CUSTOM==opts.shadeCheckRadio &&
               customCheckRadioColor->color()!=opts.customCheckRadioColor) ||
         (customMenuTextColor->isChecked() &&
               customMenuNormTextColor->color()!=opts.customMenuNormTextColor) ||
         (customMenuTextColor->isChecked() &&
               customMenuSelTextColor->color()!=opts.customMenuSelTextColor) ||
         (SHADE_CUSTOM==opts.menuStripe &&
               customMenuStripeColor->color()!=opts.customMenuStripeColor) ||
         (SHADE_CUSTOM==opts.comboBtn &&
               customComboBtnColor->color()!=opts.customComboBtnColor) ||
         (SHADE_CUSTOM==opts.sortedLv &&
               customSortedLvColor->color()!=opts.customSortedLvColor) ||

         customGradient!=opts.customGradient ||

         diffShades(opts);
}

#include "qtcurveconfig.moc"
