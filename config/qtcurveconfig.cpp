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
#include <QGridLayout>
#include <QTreeWidget>
#include <QPainter>
#include <QSettings>
#include <KGuiItem>
#include <KInputDialog>
#include <klocale.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kcharselect.h>
#include <kdialog.h>
#include <knuminput.h>
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

CGradientPreview::CGradientPreview(QWidget *p)
                : QWidget(p)
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
        QLinearGradient              grad(r.topLeft(), r.bottomLeft());
        GradientCont::const_iterator it(stops.begin()),
                                     end(stops.end());

        for(; it!=end; ++it)
        {
            QColor col;
            shade(color, &col, (*it).val);
            grad.setColorAt((*it).pos, col);
        }
        p.fillRect(r, QBrush(grad));
    }
    else
        p.fillRect(r, color);
    p.end();
}

void CGradientPreview::setGrad(const GradientCont &s)
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

static void insertAppearanceEntries(QComboBox *combo, bool split=true, bool bev=true)
{
    for(int i=APPEARANCE_CUSTOM1; i<(APPEARANCE_CUSTOM1+QTC_NUM_CUSTOM_GRAD); ++i)
        combo->insertItem(i, i18n("Custom %1", (i-APPEARANCE_CUSTOM1)+1));

    combo->insertItem(APPEARANCE_FLAT, i18n("Flat"));
    combo->insertItem(APPEARANCE_RAISED, i18n("Raised"));
    combo->insertItem(APPEARANCE_DULL_GLASS, i18n("Dull glass"));
    combo->insertItem(APPEARANCE_SHINY_GLASS, i18n("Shiny glass"));
    combo->insertItem(APPEARANCE_GRADIENT, i18n("Gradient"));
    combo->insertItem(APPEARANCE_INVERTED, i18n("Inverted gradient"));
    if(split)
    {
        combo->insertItem(APPEARANCE_SPLIT_GRADIENT, i18n("Split gradient"));
        if(bev)
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
    combo->insertItem(IND_TINT, i18n("Tint with selected background"));
    combo->insertItem(IND_GLOW, i18n("Add a slight glow"));
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
    titleLabel->setText("QtCurve " VERSION " - (C) Craig Drummond, 2003-2008");
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
    insertAppearanceEntries(titlebarAppearance, true, false);
    insertAppearanceEntries(selectionAppearance, true, false);
    insertAppearanceEntries(menuStripeAppearance, true, false);
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
    connect(menuStripe, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(menuStripeAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(round, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(toolbarBorders, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(sliderThumbs, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(handles, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(appearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(customMenuTextColor, SIGNAL(toggled(bool)), SLOT(customMenuTextColorChanged()));
    connect(stripedProgress, SIGNAL(activated(int)), SLOT(stripedProgressChanged()));
    connect(animatedProgress, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(embolden, SIGNAL(toggled(bool)), SLOT(emboldenToggled()));
    connect(defBtnIndicator, SIGNAL(activated(int)), SLOT(defBtnIndicatorChanged()));
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
    connect(fillProgress, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(darkerBorders, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(comboSplitter, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(vArrows, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(xCheck, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(crHighlight, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(colorSelTab, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(stdSidebarButtons, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(borderMenuitems, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(progressAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(menuitemAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(titlebarAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(selectionAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(shadeCheckRadio, SIGNAL(activated(int)), SLOT(shadeCheckRadioChanged()));
    connect(customCheckRadioColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));

#ifdef QTC_PLAIN_FOCUS_ONLY
    delete stdFocus;
#else
    connect(stdFocus, SIGNAL(toggled(bool)), SLOT(updateChanged()));
#endif
    connect(lvLines, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(drawStatusBarFrames, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(buttonEffect, SIGNAL(activated(int)), SLOT(buttonEffectChanged()));
    connect(coloredMouseOver, SIGNAL(activated(int)), SLOT(coloredMouseOverChanged()));
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
    connect(shading, SIGNAL(activated(int)), SLOT(shadingChanged()));
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

    setupShadesTab();
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
    setupGradientsTab();
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
        defBtnIndicator->setCurrentIndex(IND_TINT);
    updateChanged();
}

void QtCurveConfig::defBtnIndicatorChanged()
{
    if(IND_NONE==defBtnIndicator->currentIndex() && !embolden->isChecked())
        embolden->setChecked(true);
    else if(IND_GLOW==defBtnIndicator->currentIndex() && EFFECT_NONE==buttonEffect->currentIndex())
        buttonEffect->setCurrentIndex(EFFECT_SHADOW);

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

void QtCurveConfig::shadingChanged()
{
    ::shading=(EShading)shading->currentIndex();
    updateChanged();
}

void QtCurveConfig::passwordCharClicked()
{
    int              cur(toInt(passwordChar->text()));
    CharSelectDialog dlg(this, cur);

    if(QDialog::Accepted==dlg.exec() && dlg.currentChar()!=cur)
        setPasswordChar(dlg.currentChar());
}

void QtCurveConfig::gradChanged(int i)
{
    CustomGradientCont::const_iterator it(customGradient.find((EAppearance)i));

    gradStops->clear();

    if(it!=customGradient.end())
    {
        gradPreview->setGrad((*it).second.grad);
        gradLightBorder->setChecked((*it).second.lightBorder);

        GradientCont::const_iterator git((*it).second.grad.begin()),
                                     gend((*it).second.grad.end());

        for(; git!=gend; ++git)
        {
            QStringList details;

            details << QString().setNum((*git).pos)
                    << QString().setNum((*git).val);

            QTreeWidgetItem *i=new QTreeWidgetItem(gradStops, details);

            i->setFlags(i->flags()|Qt::ItemIsEditable);
        }
    }
    else
    {
        gradPreview->setGrad(GradientCont());
        gradLightBorder->setChecked(false);
    }

    gradLightBorder->setEnabled(APPEARANCE_SUNKEN!=i);
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
    double val=i->text(col).toDouble(&ok);

    if(!ok || (0==col && (val<0.0 || val>1.0)) || (1==col && (val<0.0 || val>2.0)))
        i->setText(col, QString().setNum(prev));
    else
    {
        double other=i->text(col ? 0 : 1).toDouble(&ok);

        CustomGradientCont::iterator it=customGradient.find((EAppearance)gradCombo->currentIndex());

        if(it!=customGradient.end())
        {
            (*it).second.grad.erase(Gradient(col ? other : prev, col ? prev : other));
            (*it).second.grad.insert(Gradient(col ? other : val, col ? val : other));
            gradPreview->setGrad((*it).second.grad);
            i->setText(col, QString().setNum(val));
            emit changed(true);
        }
    }
}

void QtCurveConfig::addGradStop()
{
    bool    ok;
    QString val(KInputDialog::getText(i18n("New Gradient Stops"),
                                      i18n("Please enter a set of new \"position value\" pairs\n"
                                           "(e.g. \"0.0 0.8 1.0 1.1\")"),
                                      QString(), &ok, this/*, QValidator *validator*/));

    if(ok)
    {
        QStringList list(val.split(QRegExp("[\\s,]"), QString::SkipEmptyParts));

        if(list.size() && 0==list.size()%2)
        {
            GradientCont                grads;
            QStringList::const_iterator it(list.begin()),
                                        end(list.end());

            for(; it!=end && ok; ++it)
            {
                double pos=(*it).toDouble(&ok),
                        val=ok ? (*(++it)).toDouble(&ok) : 0.0;

                if(ok && pos>=0.0 && pos<=1.0 &&  val>=0.0 && val<=2.0)
                    grads.insert(Gradient(pos, val));
            }

            if(ok)
                ok=grads.size()>0;

            if(ok)
            {
                CustomGradientCont::iterator cg=customGradient.find((EAppearance)gradCombo->currentIndex());

                if(cg==customGradient.end())
                {
                    CustomGradient cust;

                    cust.lightBorder=gradLightBorder->isChecked();
                    cust.grad=grads;
                    customGradient[(EAppearance)gradCombo->currentIndex()]=cust;
                    ok=true;
                }
                else
                {
                    unsigned int                 b4=(*cg).second.grad.size();
                    GradientCont::const_iterator git(grads.begin()),
                                                 gend(grads.end());

                    for(; git!=gend; ++git)
                        (*cg).second.grad.insert(*git);

                    ok=(*cg).second.grad.size()!=b4;
                }
            }
        }
        else
            ok=false;
    }

    if(ok)
    {
        gradChanged(gradCombo->currentIndex());
        emit changed(true);
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

        CustomGradientCont::iterator it=customGradient.find((EAppearance)gradCombo->currentIndex());

        if(it!=customGradient.end())
        {
            bool   ok;
            double pos=cur->text(0).toDouble(&ok),
                   val=cur->text(1).toDouble(&ok);

            (*it).second.grad.erase(Gradient(pos, val));
            gradPreview->setGrad((*it).second.grad);
            emit changed(true);

            delete cur;
            if(next)
                gradStops->setCurrentItem(next);
        }
    }
}

void QtCurveConfig::setupGradientsTab()
{
    for(int i=APPEARANCE_CUSTOM1; i<(APPEARANCE_CUSTOM1+QTC_NUM_CUSTOM_GRAD); ++i)
        gradCombo->insertItem(i-APPEARANCE_CUSTOM1, i18n("Custom %1", (i-APPEARANCE_CUSTOM1)+1));
    gradCombo->insertItem(APPEARANCE_CUSTOM1+QTC_NUM_CUSTOM_GRAD, i18n("Sunken"));

    gradCombo->setCurrentIndex(APPEARANCE_CUSTOM1);

    gradPreview=new CGradientPreview(previewWidgetContainer);
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

    connect(gradCombo, SIGNAL(currentIndexChanged(int)), SLOT(gradChanged(int)));
    connect(previewColor, SIGNAL(changed(const QColor &)), gradPreview, SLOT(setColor(const QColor &)));
    connect(gradStops, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(editItem(QTreeWidgetItem *, int)));
    connect(gradStops, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(itemChanged(QTreeWidgetItem *, int)));
    connect(addButton, SIGNAL(clicked(bool)), SLOT(addGradStop()));
    connect(removeButton, SIGNAL(clicked(bool)), SLOT(removeGradStop()));
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

    customShading->setChecked(opts.customShades.size());

    for(int i=0; i<NUM_STD_SHADES; ++i)
        shadeVals[i]->setValue(opts.customShades.size()
                                  ? opts.customShades[i]
                                  : shades[SHADING_SIMPLE==shading->currentIndex()
                                            ? 1 : 0]
                                          [contrast]
                                          [i]);
}

bool QtCurveConfig::diffShades(const Options &opts)
{
    if( (0==opts.customShades.size() && customShading->isChecked()) ||
        (opts.customShades.size() && !customShading->isChecked()) )
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
    opts.menuStripe=menuStripe->isChecked();
    opts.menuStripeAppearance=(EAppearance)menuStripeAppearance->currentIndex();
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
    opts.fillProgress=fillProgress->isChecked();
    opts.darkerBorders=darkerBorders->isChecked();
    opts.comboSplitter=comboSplitter->isChecked();
    opts.vArrows=vArrows->isChecked();
    opts.xCheck=xCheck->isChecked();
    opts.crHighlight=crHighlight->isChecked();
    opts.colorSelTab=colorSelTab->isChecked();
    opts.stdSidebarButtons=stdSidebarButtons->isChecked();
    opts.borderMenuitems=borderMenuitems->isChecked();
    opts.progressAppearance=(EAppearance)progressAppearance->currentIndex();
    opts.menuitemAppearance=(EAppearance)menuitemAppearance->currentIndex();
    opts.titlebarAppearance=(EAppearance)titlebarAppearance->currentIndex();
    opts.selectionAppearance=(EAppearance)selectionAppearance->currentIndex();
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
    opts.customGradient=customGradient;

    if(customShading->isChecked())
    {
        opts.customShades.resize(NUM_STD_SHADES);
        for(int i=0; i<NUM_STD_SHADES; ++i)
            opts.customShades[i]=shadeVals[i]->value();
    }
    else
        opts.customShades.clear();
}

void QtCurveConfig::setWidgetOptions(const Options &opts)
{
    round->setCurrentIndex(opts.round);
    scrollbarType->setCurrentIndex(opts.scrollbarType);
    lighterPopupMenuBgnd->setChecked(opts.lighterPopupMenuBgnd);
    menuStripe->setChecked(opts.menuStripe);
    menuStripeAppearance->setCurrentIndex(opts.menuStripeAppearance);
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
    fillProgress->setChecked(opts.fillProgress);
    darkerBorders->setChecked(opts.darkerBorders);
    comboSplitter->setChecked(opts.comboSplitter);
    vArrows->setChecked(opts.vArrows);
    xCheck->setChecked(opts.xCheck);
    crHighlight->setChecked(opts.crHighlight);
    colorSelTab->setChecked(opts.colorSelTab);
    stdSidebarButtons->setChecked(opts.stdSidebarButtons);
    borderMenuitems->setChecked(opts.borderMenuitems);
    progressAppearance->setCurrentIndex(opts.progressAppearance);
    menuitemAppearance->setCurrentIndex(opts.menuitemAppearance);
    titlebarAppearance->setCurrentIndex(opts.titlebarAppearance);
    selectionAppearance->setCurrentIndex(opts.selectionAppearance);
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
    customGradient=opts.customGradient;
    gradCombo->setCurrentIndex(APPEARANCE_CUSTOM1);

    populateShades(opts);
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
         menuStripe->isChecked()!=currentStyle.menuStripe ||
         menuStripeAppearance->currentIndex()!=currentStyle.menuStripeAppearance ||
         embolden->isChecked()!=currentStyle.embolden ||
         fillSlider->isChecked()!=currentStyle.fillSlider ||
         sliderStyle->currentIndex()!=currentStyle.sliderStyle ||
         roundMbTopOnly->isChecked()!=currentStyle.roundMbTopOnly ||
         gradientPbGroove->isChecked()!=currentStyle.gradientPbGroove ||
         fillProgress->isChecked()!=currentStyle.fillProgress ||
         darkerBorders->isChecked()!=currentStyle.darkerBorders ||
         comboSplitter->isChecked()!=currentStyle.comboSplitter ||
         vArrows->isChecked()!=currentStyle.vArrows ||
         xCheck->isChecked()!=currentStyle.xCheck ||
         crHighlight->isChecked()!=currentStyle.crHighlight ||
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
         selectionAppearance->currentIndex()!=currentStyle.selectionAppearance ||
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
               customMenuSelTextColor->color()!=currentStyle.customMenuSelTextColor) ||

         customGradient!=currentStyle.customGradient ||

         diffShades(currentStyle);
}

#include "qtcurveconfig.moc"
