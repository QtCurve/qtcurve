/*
  QtCurve (C) Craig Drummond, 2007 Craig.Drummond@lycos.co.uk

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
#include "exportthemedialog.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QFrame>
#include <QTabWidget>
#include <QMenu>
#include <QFileInfo>
#include <QBoxLayout>
#include <klocale.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kcharselect.h>
#include <kdialog.h>
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
        KGlobal::locale()->insertCatalog("kstyle_qtcurve_config");

        return new QtCurveConfig(parent);
    }
}

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

static int toInt(const QString &str)
{
    return str.length()>1 ? str[0].unicode() : 0;
}

static void insertShadeEntries(QComboBox *combo, bool withDarken, bool checkRadio=false)
{
    combo->insertItem(SHADE_NONE, checkRadio ? i18n("Text")
                                             : withDarken ? i18n("Background")
                                                          : i18n("Button"));
    combo->insertItem(SHADE_CUSTOM, i18n("Custom:"));

    if(checkRadio) // For check/radio, we dont blend, and dont allow darken
        combo->insertItem(SHADE_BLEND_SELECTED, i18n("Selected background"));
    else if(withDarken)
    {
         // For menubars we dont actually blend...
        combo->insertItem(SHADE_BLEND_SELECTED, i18n("Selected background"));
        combo->insertItem(SHADE_DARKEN, i18n("Darken"));
    }
    else
    {
        combo->insertItem(SHADE_BLEND_SELECTED, i18n("Blended selected background"));
        combo->insertItem(SHADE_SELECTED, i18n("Selected background"));
    }
}

static void insertAppearanceEntries(QComboBox *combo, bool all=true)
{
    combo->insertItem(APPEARANCE_FLAT, i18n("Flat"));
    combo->insertItem(APPEARANCE_RAISED, i18n("Raised"));
    combo->insertItem(APPEARANCE_DULL_GLASS, i18n("Dull glass"));
    combo->insertItem(APPEARANCE_SHINY_GLASS, i18n("Shiny glass"));
    combo->insertItem(APPEARANCE_GRADIENT, i18n("Gradient"));
    combo->insertItem(APPEARANCE_INVERTED, i18n("Inverted gradient"));
    if(all)
    {
        combo->insertItem(APPEARANCE_SPLIT_GRADIENT, i18n("Split gradient"));
        combo->insertItem(APPEARANCE_BEVELLED, i18n("Bevelled"));
    }
}

static void insertLineEntries(QComboBox *combo, bool none)
{
    combo->insertItem(LINE_SUNKEN, i18n("Sunken lines"));
    combo->insertItem(LINE_FLAT, i18n("Flat lines"));
    combo->insertItem(LINE_DOTS, i18n("Dots"));
    combo->insertItem(LINE_DASHES, none ? i18n("None") : i18n("Dashes"));
}

static void insertDefBtnEntries(QComboBox *combo)
{
    combo->insertItem(IND_CORNER, i18n("Corner indicator"));
    combo->insertItem(IND_FONT_COLOR, i18n("Font color thin border"));
    combo->insertItem(IND_COLORED, i18n("Selected background thick border"));
    combo->insertItem(IND_NONE, i18n("None"));
}

static void insertScrollbarEntries(QComboBox *combo)
{
    combo->insertItem(SCROLLBAR_KDE, i18n("KDE"));
    combo->insertItem(SCROLLBAR_WINDOWS, i18n("Windows"));
    combo->insertItem(SCROLLBAR_PLATINUM, i18n("Platinum"));
    combo->insertItem(SCROLLBAR_NEXT, i18n("Next"));
    combo->insertItem(SCROLLBAR_NONE, i18n("No buttons"));
}

static void insertRoundEntries(QComboBox *combo)
{
    combo->insertItem(ROUND_NONE, i18n("Square"));
    combo->insertItem(ROUND_SLIGHT, i18n("Slightly rounded"));
    combo->insertItem(ROUND_FULL, i18n("Fully rounded"));
}

static void insertMouseOverEntries(QComboBox *combo)
{
    combo->insertItem(MO_NONE, i18n("No coloration"));
    combo->insertItem(MO_COLORED, i18n("Color border"));
    combo->insertItem(MO_PLASTIK, i18n("Plastik style"));
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
    combo->insertItem(SLIDER_TRIANGULAR, i18n("Triangular"));
}

QtCurveConfig::QtCurveConfig(QWidget *parent)
             : QWidget(parent),
               exportDialog(NULL)
{
    setupUi(this);
    titleLabel->setText("QtCurve " VERSION " - (C) Craig Drummond, 2003-2007");
    insertShadeEntries(shadeSliders, false);
    insertShadeEntries(shadeMenubars, true);
    insertShadeEntries(shadeCheckRadio, false, true);
    insertAppearanceEntries(appearance);
    insertAppearanceEntries(menubarAppearance);
    insertAppearanceEntries(toolbarAppearance);
    insertAppearanceEntries(lvAppearance);
    insertAppearanceEntries(sliderAppearance);
    insertAppearanceEntries(tabAppearance, false);
    insertAppearanceEntries(progressAppearance);
    insertAppearanceEntries(menuitemAppearance);
    insertAppearanceEntries(titlebarAppearance);
    insertLineEntries(handles, false);
    insertLineEntries(sliderThumbs, true);
    insertLineEntries(toolbarSeparators, true);
    insertLineEntries(splitters, false);
    insertDefBtnEntries(defBtnIndicator);
    insertScrollbarEntries(scrollbarType);
    insertRoundEntries(round);
    insertMouseOverEntries(coloredMouseOver);
    insertToolbarBorderEntries(toolbarBorders);
    insertEffectEntries(buttonEffect);
    insertShadingEntries(shading);
    insertStripeEntries(stripedProgress);
    insertSliderStyleEntries(sliderStyle);

    highlightFactor->setRange(MIN_HIGHLIGHT_FACTOR, MAX_HIGHLIGHT_FACTOR);
    highlightFactor->setValue(((int)(DEFAULT_HIGHLIGHT_FACTOR*100))-100);

    connect(lighterPopupMenuBgnd, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(round, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(toolbarBorders, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(sliderThumbs, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(handles, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(appearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(customMenuTextColor, SIGNAL(toggled(bool)), SLOT(customMenuTextColorChanged()));
    connect(stripedProgress, SIGNAL(activated(int)), SLOT(stripedProgressChanged()));
    connect(animatedProgress, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(embolden, SIGNAL(toggled(bool)), SLOT(emboldenToggled()));
    connect(defBtnIndicator, SIGNAL(activated(int)), SLOT(dbiChanged()));
    connect(highlightTab, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(menubarAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(toolbarAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(lvAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(sliderAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(tabAppearance, SIGNAL(activated(int)), SLOT(tabAppearanceChanged()));
    connect(toolbarSeparators, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(splitters, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(fixParentlessDialogs, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(fillSlider, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(sliderStyle, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(roundMbTopOnly, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(gradientPbGroove, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(darkerBorders, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(vArrows, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(xCheck, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(colorSelTab, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(stdSidebarButtons, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(borderMenuitems, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(progressAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(menuitemAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(titlebarAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(shadeCheckRadio, SIGNAL(activated(int)), SLOT(shadeCheckRadioChanged()));
    connect(customCheckRadioColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));

#ifdef QTC_PLAIN_FOCUS_ONLY
    delete stdFocus;
#else
    connect(stdFocus, SIGNAL(toggled(bool)), SLOT(updateChanged()));
#endif
    connect(lvLines, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(drawStatusBarFrames, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(buttonEffect, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(coloredMouseOver, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(menubarMouseOver, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(shadeMenubarOnlyWhenActive, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(thinnerMenuItems, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(customSlidersColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(customMenubarsColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(customMenuSelTextColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(customMenuNormTextColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(shadeSliders, SIGNAL(activated(int)), SLOT(shadeSlidersChanged()));
    connect(shadeMenubars, SIGNAL(activated(int)), SLOT(shadeMenubarsChanged()));
    connect(highlightFactor, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(scrollbarType, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(shading, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(gtkScrollViews, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(gtkComboMenus, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(gtkButtonOrder, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(mapKdeIcons, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(passwordChar, SIGNAL(clicked()), SLOT(passwordCharClicked()));
    connect(framelessGroupBoxes, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(inactiveHighlight, SIGNAL(toggled(bool)), SLOT(updateChanged()));

    defaultSettings(&defaultStyle);
    if(!readConfig(NULL, &currentStyle, &defaultStyle))
        currentStyle=defaultStyle;

    setWidgetOptions(currentStyle);

    QMenu *menu=new QMenu(this),
          *subMenu=new QMenu(i18n("Predefined Style"), this);

    optionBtn->setMenu(menu);

    menu->addMenu(subMenu);
    menu->addSeparator();
    menu->addAction(i18n("Import..."), this, SLOT(importStyle()));
    menu->addAction(i18n("Export..."), this, SLOT(exportStyle()));
    menu->addSeparator();
    menu->addAction(i18n("Export Theme..."), this, SLOT(exportTheme()));
    loadStyles(subMenu);
}

QtCurveConfig::~QtCurveConfig()
{
}

void QtCurveConfig::loadStyles(QMenu *menu)
{
    QStringList files(KGlobal::dirs()->findAllResources("data", "QtCurve/*"QTC_EXTENSION, KStandardDirs::NoDuplicates));

    files.sort();

    QStringList::Iterator it(files.begin()),
                          end(files.end());
    Options               opts;

    for(; it!=end; ++it)
        if(readConfig(*it, &opts, &defaultStyle))
            styles[menu->addAction(QFileInfo(*it).fileName().remove(QTC_EXTENSION).replace('_', ' '))]=*it;

    connect(menu, SIGNAL(triggered(QAction *)), SLOT(setStyle(QAction *)));
}

void QtCurveConfig::save()
{
    Options opts=currentStyle;

    setOptions(opts);
    writeConfig(NULL, opts, defaultStyle);

    // This is only read by KDE3...
    KConfig      kglobals("kdeglobals", KConfig::CascadeConfig);
    KConfigGroup grp(&kglobals, "KDE");

    if(opts.gtkButtonOrder)
        grp.writeEntry("ButtonLayout", 2);
    else
        grp.deleteEntry("ButtonLayout");
}

void QtCurveConfig::defaults()
{
    setWidgetOptions(defaultStyle);
    if (settingsChanged())
        emit changed(true);
}

void QtCurveConfig::setStyle(QAction *s)
{
    loadStyle(styles[s]);
}

void QtCurveConfig::emboldenToggled()
{
    if(!embolden->isChecked() && IND_NONE==defBtnIndicator->currentIndex())
        defBtnIndicator->setCurrentIndex(IND_COLORED);
    updateChanged();
}

void QtCurveConfig::dbiChanged()
{
    if(IND_NONE==defBtnIndicator->currentIndex() && !embolden->isChecked())
        embolden->setChecked(true);
    updateChanged();
}

void QtCurveConfig::shadeSlidersChanged()
{
    customSlidersColor->setEnabled(SHADE_CUSTOM==shadeSliders->currentIndex());
    updateChanged();
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

void QtCurveConfig::stripedProgressChanged()
{
    animatedProgress->setEnabled(STRIPE_NONE!=stripedProgress->currentIndex());
    if(animatedProgress->isChecked() && STRIPE_NONE==stripedProgress->currentIndex())
        animatedProgress->setChecked(false);
    updateChanged();
}

void QtCurveConfig::tabAppearanceChanged()
{
    if(colorSelTab->isChecked() && APPEARANCE_GRADIENT!=tabAppearance->currentIndex())
        colorSelTab->setChecked(false);
    colorSelTab->setEnabled(APPEARANCE_GRADIENT==tabAppearance->currentIndex());
    updateChanged();
}

void QtCurveConfig::passwordCharClicked()
{
    int              cur(toInt(passwordChar->text()));
    CharSelectDialog dlg(this, cur);

    if(QDialog::Accepted==dlg.exec() && dlg.currentChar()!=cur)
        setPasswordChar(dlg.currentChar());
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

void QtCurveConfig::importStyle()
{
    QString file(KFileDialog::getOpenFileName(KUrl(),
                                              i18n("*"QTC_EXTENSION"|QtCurve Settings Files\n"
                                                   QTC_THEME_PREFIX"*"QTC_THEME_SUFFIX"|QtCurve KDE Theme Files"), this));

    if(!file.isEmpty())
        loadStyle(file);
}

void QtCurveConfig::exportStyle()
{
    QString file(KFileDialog::getSaveFileName(KUrl(), i18n("*"QTC_EXTENSION"|QtCurve Settings Files"), this));

    if(!file.isEmpty())
    {
        KConfig cfg(file, KConfig::NoGlobals);
        bool    rv(true);

        if(rv)
        {
            Options opts;

            setOptions(opts);
            rv=writeConfig(&cfg, opts, defaultStyle, true);
        }

        if(!rv)
            KMessageBox::error(this, i18n("Could not write to file:\n%1").arg(file));
    }
}

void QtCurveConfig::exportTheme()
{
    if(!exportDialog)
        exportDialog=new CExportThemeDialog(this);

    Options opts;

    setOptions(opts);
    exportDialog->run(opts);
}

void QtCurveConfig::loadStyle(const QString &file)
{
    Options opts;

    if(readConfig(file, &opts, &defaultStyle))
    {
        setWidgetOptions(opts);
        if (settingsChanged())
            emit changed(true);
    }
}

void QtCurveConfig::setOptions(Options &opts)
{
    opts.round=(ERound)round->currentIndex();
    opts.toolbarBorders=(ETBarBorder)toolbarBorders->currentIndex();
    opts.appearance=(EAppearance)appearance->currentIndex();
#ifndef QTC_PLAIN_FOCUS_ONLY
    opts.stdFocus=stdFocus->isChecked();
#endif
    opts.lvLines=lvLines->isChecked();
    opts.drawStatusBarFrames=drawStatusBarFrames->isChecked();
    opts.buttonEffect=(EEffect)buttonEffect->currentIndex();
    opts.coloredMouseOver=(EMouseOver)coloredMouseOver->currentIndex();
    opts.menubarMouseOver=menubarMouseOver->isChecked();
    opts.shadeMenubarOnlyWhenActive=shadeMenubarOnlyWhenActive->isChecked();
    opts.thinnerMenuItems=thinnerMenuItems->isChecked();
    opts.fixParentlessDialogs=fixParentlessDialogs->isChecked();
    opts.animatedProgress=animatedProgress->isChecked();
    opts.stripedProgress=(EStripe)stripedProgress->currentIndex();
    opts.lighterPopupMenuBgnd=lighterPopupMenuBgnd->isChecked();
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
    opts.toolbarSeparators=(ELine)toolbarSeparators->currentIndex();
    opts.splitters=(ELine)splitters->currentIndex();
    opts.customSlidersColor=customSlidersColor->color();
    opts.customMenubarsColor=customMenubarsColor->color();
    opts.highlightFactor=((double)(highlightFactor->value()+100))/100.0;
    opts.customMenuNormTextColor=customMenuNormTextColor->color();
    opts.customMenuSelTextColor=customMenuSelTextColor->color();
    opts.customMenuTextColor=customMenuTextColor->isChecked();
    opts.fillSlider=fillSlider->isChecked();
    opts.sliderStyle=(ESliderStyle)sliderStyle->currentIndex();
    opts.roundMbTopOnly=roundMbTopOnly->isChecked();
    opts.gradientPbGroove=gradientPbGroove->isChecked();
    opts.darkerBorders=darkerBorders->isChecked();
    opts.vArrows=vArrows->isChecked();
    opts.xCheck=xCheck->isChecked();
    opts.colorSelTab=colorSelTab->isChecked();
    opts.stdSidebarButtons=stdSidebarButtons->isChecked();
    opts.borderMenuitems=borderMenuitems->isChecked();
    opts.progressAppearance=(EAppearance)progressAppearance->currentIndex();
    opts.menuitemAppearance=(EAppearance)menuitemAppearance->currentIndex();
    opts.titlebarAppearance=(EAppearance)titlebarAppearance->currentIndex();
    opts.shadeCheckRadio=(EShade)shadeCheckRadio->currentIndex();
    opts.customCheckRadioColor=customCheckRadioColor->color();
    opts.shading=(EShading)shading->currentIndex();
    opts.gtkScrollViews=gtkScrollViews->isChecked();
    opts.gtkComboMenus=gtkComboMenus->isChecked();
    opts.gtkButtonOrder=gtkButtonOrder->isChecked();
    opts.mapKdeIcons=mapKdeIcons->isChecked();
    opts.passwordChar=toInt(passwordChar->text());
    opts.framelessGroupBoxes=framelessGroupBoxes->isChecked();
    opts.inactiveHighlight=inactiveHighlight->isChecked();
}

void QtCurveConfig::setWidgetOptions(const Options &opts)
{
    round->setCurrentIndex(opts.round);
    scrollbarType->setCurrentIndex(opts.scrollbarType);
    lighterPopupMenuBgnd->setChecked(opts.lighterPopupMenuBgnd);
    toolbarBorders->setCurrentIndex(opts.toolbarBorders);
    sliderThumbs->setCurrentIndex(opts.sliderThumbs);
    handles->setCurrentIndex(opts.handles);
    appearance->setCurrentIndex(opts.appearance);
#ifndef QTC_PLAIN_FOCUS_ONLY
    stdFocus->setChecked(opts.stdFocus);
#endif
    lvLines->setChecked(opts.lvLines);
    drawStatusBarFrames->setChecked(opts.drawStatusBarFrames);
    buttonEffect->setCurrentIndex(opts.buttonEffect);
    coloredMouseOver->setCurrentIndex(opts.coloredMouseOver);
    menubarMouseOver->setChecked(opts.menubarMouseOver);
    shadeMenubarOnlyWhenActive->setChecked(opts.shadeMenubarOnlyWhenActive);
    thinnerMenuItems->setChecked(opts.thinnerMenuItems);
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
    toolbarSeparators->setCurrentIndex(opts.toolbarSeparators);
    splitters->setCurrentIndex(opts.splitters);
    shadeSliders->setCurrentIndex(opts.shadeSliders);
    shadeMenubars->setCurrentIndex(opts.shadeMenubars);
    highlightFactor->setValue((int)(opts.highlightFactor*100)-100);
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

    animatedProgress->setEnabled(STRIPE_NONE!=stripedProgress->currentIndex());

    fillSlider->setChecked(opts.fillSlider);
    sliderStyle->setCurrentIndex(opts.sliderStyle);
    roundMbTopOnly->setChecked(opts.roundMbTopOnly);
    gradientPbGroove->setChecked(opts.gradientPbGroove);
    darkerBorders->setChecked(opts.darkerBorders);
    vArrows->setChecked(opts.vArrows);
    xCheck->setChecked(opts.xCheck);
    colorSelTab->setChecked(opts.colorSelTab);
    stdSidebarButtons->setChecked(opts.stdSidebarButtons);
    borderMenuitems->setChecked(opts.borderMenuitems);
    progressAppearance->setCurrentIndex(opts.progressAppearance);
    menuitemAppearance->setCurrentIndex(opts.menuitemAppearance);
    titlebarAppearance->setCurrentIndex(opts.titlebarAppearance);
    shadeCheckRadio->setCurrentIndex(opts.shadeCheckRadio);
    customCheckRadioColor->setColor(opts.customCheckRadioColor);

    shading->setCurrentIndex(opts.shading);
    gtkScrollViews->setChecked(opts.gtkScrollViews);
    gtkComboMenus->setChecked(opts.gtkComboMenus);
    gtkButtonOrder->setChecked(opts.gtkButtonOrder);
    mapKdeIcons->setChecked(opts.mapKdeIcons);
    setPasswordChar(opts.passwordChar);
    framelessGroupBoxes->setChecked(opts.framelessGroupBoxes);
    inactiveHighlight->setChecked(opts.inactiveHighlight);
}

bool QtCurveConfig::settingsChanged()
{
    return round->currentIndex()!=currentStyle.round ||
         toolbarBorders->currentIndex()!=currentStyle.toolbarBorders ||
         appearance->currentIndex()!=(int)currentStyle.appearance ||
#ifndef QTC_PLAIN_FOCUS_ONLY
         stdFocus->isChecked()!=currentStyle.stdFocus ||
#endif
         lvLines->isChecked()!=currentStyle.lvLines ||
         drawStatusBarFrames->isChecked()!=currentStyle.drawStatusBarFrames ||
         buttonEffect->currentIndex()!=(EEffect)currentStyle.buttonEffect ||
         coloredMouseOver->currentIndex()!=(int)currentStyle.coloredMouseOver ||
         menubarMouseOver->isChecked()!=currentStyle.menubarMouseOver ||
         shadeMenubarOnlyWhenActive->isChecked()!=currentStyle.shadeMenubarOnlyWhenActive ||
         thinnerMenuItems->isChecked()!=currentStyle.thinnerMenuItems ||
         fixParentlessDialogs->isChecked()!=currentStyle.fixParentlessDialogs ||
         animatedProgress->isChecked()!=currentStyle.animatedProgress ||
         stripedProgress->currentIndex()!=currentStyle.stripedProgress ||
         lighterPopupMenuBgnd->isChecked()!=currentStyle.lighterPopupMenuBgnd ||
         embolden->isChecked()!=currentStyle.embolden ||
         fillSlider->isChecked()!=currentStyle.fillSlider ||
         sliderStyle->currentIndex()!=currentStyle.sliderStyle ||
         roundMbTopOnly->isChecked()!=currentStyle.roundMbTopOnly ||
         gradientPbGroove->isChecked()!=currentStyle.gradientPbGroove ||
         darkerBorders->isChecked()!=currentStyle.darkerBorders ||
         vArrows->isChecked()!=currentStyle.vArrows ||
         xCheck->isChecked()!=currentStyle.xCheck ||
         colorSelTab->isChecked()!=currentStyle.colorSelTab ||
         stdSidebarButtons->isChecked()!=currentStyle.stdSidebarButtons ||
         borderMenuitems->isChecked()!=currentStyle.borderMenuitems ||
         defBtnIndicator->currentIndex()!=(int)currentStyle.defBtnIndicator ||
         sliderThumbs->currentIndex()!=(int)currentStyle.sliderThumbs ||
         handles->currentIndex()!=(int)currentStyle.handles ||
         scrollbarType->currentIndex()!=(int)currentStyle.scrollbarType ||
         highlightTab->isChecked()!=currentStyle.highlightTab ||
         shadeSliders->currentIndex()!=(int)currentStyle.shadeSliders ||
         shadeMenubars->currentIndex()!=(int)currentStyle.shadeMenubars ||
         shadeCheckRadio->currentIndex()!=(int)currentStyle.shadeCheckRadio ||
         menubarAppearance->currentIndex()!=currentStyle.menubarAppearance ||
         toolbarAppearance->currentIndex()!=currentStyle.toolbarAppearance ||
         lvAppearance->currentIndex()!=currentStyle.lvAppearance ||
         sliderAppearance->currentIndex()!=currentStyle.sliderAppearance ||
         tabAppearance->currentIndex()!=currentStyle.tabAppearance ||
         progressAppearance->currentIndex()!=currentStyle.progressAppearance ||
         menuitemAppearance->currentIndex()!=currentStyle.menuitemAppearance ||
         titlebarAppearance->currentIndex()!=currentStyle.titlebarAppearance ||
         toolbarSeparators->currentIndex()!=currentStyle.toolbarSeparators ||
         splitters->currentIndex()!=currentStyle.splitters ||

         shading->currentIndex()!=(int)currentStyle.shading ||
         gtkScrollViews->isChecked()!=currentStyle.gtkScrollViews ||
         gtkComboMenus->isChecked()!=currentStyle.gtkComboMenus ||
         gtkButtonOrder->isChecked()!=currentStyle.gtkButtonOrder ||
         mapKdeIcons->isChecked()!=currentStyle.mapKdeIcons ||
         framelessGroupBoxes->isChecked()!=currentStyle.framelessGroupBoxes ||
         inactiveHighlight->isChecked()!=currentStyle.inactiveHighlight ||

         toInt(passwordChar->text())!=currentStyle.passwordChar ||

         (highlightFactor->value()+100)!=(int)(currentStyle.highlightFactor*100) ||
         customMenuTextColor->isChecked()!=currentStyle.customMenuTextColor ||
         (SHADE_CUSTOM==currentStyle.shadeSliders &&
               customSlidersColor->color()!=currentStyle.customSlidersColor) ||
         (SHADE_CUSTOM==currentStyle.shadeMenubars &&
               customMenubarsColor->color()!=currentStyle.customMenubarsColor) ||
         (SHADE_CUSTOM==currentStyle.shadeCheckRadio &&
               customCheckRadioColor->color()!=currentStyle.customCheckRadioColor) ||
         (customMenuTextColor->isChecked() &&
               customMenuNormTextColor->color()!=currentStyle.customMenuNormTextColor) ||
         (customMenuTextColor->isChecked() &&
               customMenuSelTextColor->color()!=currentStyle.customMenuSelTextColor);
}

#include "qtcurveconfig.moc"
