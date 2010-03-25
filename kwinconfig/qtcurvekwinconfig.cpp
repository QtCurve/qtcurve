/*
  QtCurve KWin window decoration
  Copyright (C) 2007 - 2010 Craig Drummond <craig.p.drummond@googlemail.com>

  based on the window decoration "Plastik":
  Copyright (C) 2003-2005 Sandro Giessl <sandro@giessl.com>

  based on the window decoration "Web":
  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
 */

#include <kdeversion.h>
#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include "config.h"
#include "qtcurvekwinconfig.h"

#if KDE_IS_VERSION(4, 3, 0)
static void insertColorEntries(QComboBox *combo)
{
    combo->insertItem(KWinQtCurve::QtCurveShadowConfiguration::CT_FOCUS, i18n("Focus"));
    combo->insertItem(KWinQtCurve::QtCurveShadowConfiguration::CT_HOVER, i18n("Hover"));
    combo->insertItem(KWinQtCurve::QtCurveShadowConfiguration::CT_SELECTION, i18n("Selection Background"));
    combo->insertItem(KWinQtCurve::QtCurveShadowConfiguration::CT_TITLEBAR, i18n("Titlebar"));
    combo->insertItem(KWinQtCurve::QtCurveShadowConfiguration::CT_GRAY, i18n("Gray"));
    combo->insertItem(KWinQtCurve::QtCurveShadowConfiguration::CT_CUSTOM, i18n("Custom:"));
}
#endif

QtCurveKWinConfig::QtCurveKWinConfig(KConfig *config, QWidget *parent)
                 : QObject(parent)
                 , itsConfig(new KConfig("kwinqtcurverc"))
                 , itsWidget(new QtCurveKWinConfigWidget(parent))
#if KDE_IS_VERSION(4, 3, 0)
                 , itsActiveShadows(QPalette::Active)
                 , itsInactiveShadows(QPalette::Inactive)
#endif
{
    KConfigGroup configGroup(itsConfig, "General");
    KGlobal::locale()->insertCatalog("qtcurve");

    itsWidget->show();

    load(configGroup);

    connect(itsWidget->resizeGrip, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(itsWidget->roundBottom, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(itsWidget->outerBorder, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(itsWidget->borderlessMax, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(itsWidget->titleBarPad, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    itsWidget->titleBarPad->setRange(0, 10);
#if KDE_IS_VERSION(4, 3, 0)
    insertColorEntries(itsWidget->activeShadowColorType);
    insertColorEntries(itsWidget->inactiveShadowColorType);
    connect(itsWidget->useShadows, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(itsWidget->activeShadowSize, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(itsWidget->activeShadowHOffset, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(itsWidget->activeShadowVOffset, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(itsWidget->activeShadowColorType, SIGNAL(currentIndexChanged(int)), this, SLOT(activeShadowColorTypeChanged()));
    connect(itsWidget->activeShadowColor, SIGNAL(changed(const QColor &)), this, SIGNAL(changed()));
    connect(itsWidget->inactiveShadowSize, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(itsWidget->inactiveShadowHOffset, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(itsWidget->inactiveShadowVOffset, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(itsWidget->inactiveShadowColorType, SIGNAL(currentIndexChanged(int)), this, SLOT(inactiveShadowColorTypeChanged()));
    connect(itsWidget->inactiveShadowColor, SIGNAL(changed(const QColor &)), this, SIGNAL(changed()));
    activeShadowColorTypeChanged();
    inactiveShadowColorTypeChanged();
    setShadows();
    itsWidget->activeShadowSize->setRange(KWinQtCurve::QtCurveShadowConfiguration::MIN_SIZE,
                                          KWinQtCurve::QtCurveShadowConfiguration::MAX_SIZE);
    itsWidget->inactiveShadowSize->setRange(KWinQtCurve::QtCurveShadowConfiguration::MIN_SIZE,
                                            KWinQtCurve::QtCurveShadowConfiguration::MAX_SIZE);
    itsWidget->activeShadowHOffset->setRange(KWinQtCurve::QtCurveShadowConfiguration::MIN_OFFSET,
                                             KWinQtCurve::QtCurveShadowConfiguration::MAX_OFFSET);
    itsWidget->inactiveShadowHOffset->setRange(KWinQtCurve::QtCurveShadowConfiguration::MIN_OFFSET,
                                               KWinQtCurve::QtCurveShadowConfiguration::MAX_OFFSET);
    itsWidget->activeShadowVOffset->setRange(KWinQtCurve::QtCurveShadowConfiguration::MIN_OFFSET,
                                             KWinQtCurve::QtCurveShadowConfiguration::MAX_OFFSET);
    itsWidget->inactiveShadowVOffset->setRange(KWinQtCurve::QtCurveShadowConfiguration::MIN_OFFSET,
                                               KWinQtCurve::QtCurveShadowConfiguration::MAX_OFFSET);
#else
    itsWidget->shadowTab->setVisible(false);
#endif
}

QtCurveKWinConfig::~QtCurveKWinConfig()
{
    delete itsWidget;
    delete itsConfig;
}

void QtCurveKWinConfig::load(const KConfigGroup &)
{
    KConfigGroup configGroup(itsConfig, "General");

    itsWidget->resizeGrip->setChecked(configGroup.readEntry("ShowResizeGrip", false));
    itsWidget->roundBottom->setChecked(configGroup.readEntry("RoundBottom", true));
    itsWidget->outerBorder->setChecked(configGroup.hasKey("NoBorder")
                                        ? !configGroup.readEntry("NoBorder", false)
                                        : configGroup.readEntry("OuterBorder", true));
    itsWidget->borderlessMax->setChecked(configGroup.readEntry("BorderlessMax", false));
    itsWidget->titleBarPad->setValue(configGroup.readEntry("TitleBarPad", 0));
    
#if KDE_IS_VERSION(4, 3, 0)
    itsWidget->useShadows->setChecked(configGroup.readEntry("CustomShadows", false));
    itsActiveShadows.load(itsConfig);
    itsInactiveShadows.load(itsConfig);
    setShadows();
#endif
}

void QtCurveKWinConfig::save(KConfigGroup &)
{
    KConfigGroup configGroup(itsConfig, "General");

    configGroup.writeEntry("ShowResizeGrip", itsWidget->resizeGrip->isChecked());
    configGroup.writeEntry("RoundBottom", itsWidget->roundBottom->isChecked());
    configGroup.writeEntry("OuterBorder", itsWidget->outerBorder->isChecked());
    configGroup.writeEntry("BorderlessMax", itsWidget->borderlessMax->isChecked());
    configGroup.writeEntry("TitleBarPad", itsWidget->titleBarPad->value());
    configGroup.deleteEntry("NoBorder");
#if KDE_IS_VERSION(4, 3, 0)
    configGroup.writeEntry("CustomShadows", itsWidget->useShadows->isChecked());
    if(itsWidget->useShadows->isChecked())
    {
        itsActiveShadows.setShadowSize(itsWidget->activeShadowSize->value());
        itsActiveShadows.setHorizontalOffset(itsWidget->activeShadowHOffset->value());
        itsActiveShadows.setVerticalOffset(itsWidget->activeShadowVOffset->value());
        itsActiveShadows.setColorType((KWinQtCurve::QtCurveShadowConfiguration::ColorType)itsWidget->activeShadowColorType->currentIndex());
        if(KWinQtCurve::QtCurveShadowConfiguration::CT_CUSTOM==itsWidget->activeShadowColorType->currentIndex())
            itsActiveShadows.setColor(itsWidget->activeShadowColor->color());
        itsInactiveShadows.setShadowSize(itsWidget->inactiveShadowSize->value());
        itsInactiveShadows.setHorizontalOffset(itsWidget->inactiveShadowHOffset->value());
        itsInactiveShadows.setVerticalOffset(itsWidget->inactiveShadowVOffset->value());
        itsInactiveShadows.setColorType((KWinQtCurve::QtCurveShadowConfiguration::ColorType)itsWidget->inactiveShadowColorType->currentIndex());
        if(KWinQtCurve::QtCurveShadowConfiguration::CT_CUSTOM==itsWidget->inactiveShadowColorType->currentIndex())
            itsInactiveShadows.setColor(itsWidget->inactiveShadowColor->color());
    }
    else
    {
        itsActiveShadows.defaults();
        itsInactiveShadows.defaults();
    }
    itsActiveShadows.save(itsConfig);
    itsInactiveShadows.save(itsConfig);
#endif
    itsConfig->sync();
}

void QtCurveKWinConfig::defaults()
{
    itsWidget->resizeGrip->setChecked(false);
    itsWidget->roundBottom->setChecked(true);
    itsWidget->outerBorder->setChecked(true);
    itsWidget->borderlessMax->setChecked(false);
    itsWidget->titleBarPad->setValue(0);
#if KDE_IS_VERSION(4, 3, 0)
    itsWidget->useShadows->setChecked(false);
    itsActiveShadows.defaults();
    itsInactiveShadows.defaults();
    setShadows();
#endif
}

void QtCurveKWinConfig::activeShadowColorTypeChanged()
{
#if KDE_IS_VERSION(4, 3, 0)
    itsWidget->activeShadowColor->setEnabled(KWinQtCurve::QtCurveShadowConfiguration::CT_CUSTOM==itsWidget->activeShadowColorType->currentIndex());
    if(itsActiveShadows.colorType()!=itsWidget->activeShadowColorType->currentIndex())
        emit changed();
#endif
}

void QtCurveKWinConfig::inactiveShadowColorTypeChanged()
{
#if KDE_IS_VERSION(4, 3, 0)
    itsWidget->inactiveShadowColor->setEnabled(KWinQtCurve::QtCurveShadowConfiguration::CT_CUSTOM==itsWidget->inactiveShadowColorType->currentIndex());
    if(itsInactiveShadows.colorType()!=itsWidget->inactiveShadowColorType->currentIndex())
        emit changed();
#endif
}
    
#if KDE_IS_VERSION(4, 3, 0)
void QtCurveKWinConfig::setShadows()
{
    itsWidget->activeShadowSize->setValue(itsActiveShadows.shadowSize());
    itsWidget->activeShadowHOffset->setValue(itsActiveShadows.horizontalOffset());
    itsWidget->activeShadowVOffset->setValue(itsActiveShadows.verticalOffset());
    itsWidget->activeShadowColor->setColor(itsActiveShadows.color());
    itsWidget->activeShadowColorType->setCurrentIndex(itsActiveShadows.colorType());
    itsWidget->inactiveShadowSize->setValue(itsInactiveShadows.shadowSize());
    itsWidget->inactiveShadowHOffset->setValue(itsInactiveShadows.horizontalOffset());
    itsWidget->inactiveShadowVOffset->setValue(itsInactiveShadows.verticalOffset());
    itsWidget->inactiveShadowColor->setColor(itsInactiveShadows.color());
    itsWidget->inactiveShadowColorType->setCurrentIndex(itsInactiveShadows.colorType());
}
#endif

extern "C"
{
    KDE_EXPORT QObject * allocate_config(KConfig *config, QWidget *parent)
    {
        return new QtCurveKWinConfig(config, parent);
    }
}

#include "qtcurvekwinconfig.moc"
