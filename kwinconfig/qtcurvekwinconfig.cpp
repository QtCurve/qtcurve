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
    combo->insertItem(KWinQtCurve::QtCurveShadowConfiguration::CT_ACTIVE_TITLEBAR, i18n("Active Titlebar"));
    combo->insertItem(KWinQtCurve::QtCurveShadowConfiguration::CT_INACTIVE_TITLEBAR, i18n("Inactive Titlebar"));
    combo->insertItem(KWinQtCurve::QtCurveShadowConfiguration::CT_GRAY, i18n("Gray"));
    combo->insertItem(KWinQtCurve::QtCurveShadowConfiguration::CT_CUSTOM, i18n("Custom"));
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

    connect(itsWidget->menuClose, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(itsWidget->resizeGrip, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(itsWidget->roundBottom, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(itsWidget->outerBorder, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(itsWidget->titleBarPad, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
#if KDE_IS_VERSION(4, 3, 0)
    insertColorEntries(itsWidget->activeShadowColor);
    insertColorEntries(itsWidget->inactiveShadowColor);
    connect(itsWidget->useShadows, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(itsWidget->activeShadowSize, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(itsWidget->activeShadowOffset, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(itsWidget->activeShadowColor, SIGNAL(currentIndexChanged(int)), this, SLOT(activeShadowColorChanged()));
    connect(itsWidget->activeShadowInnerColor, SIGNAL(changed(const QColor &)), this, SIGNAL(changed()));
    connect(itsWidget->activeShadowOuterColor, SIGNAL(changed(const QColor &)), this, SIGNAL(changed()));
    connect(itsWidget->inactiveShadowSize, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(itsWidget->inactiveShadowOffset, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(itsWidget->inactiveShadowColor, SIGNAL(currentIndexChanged(int)), this, SLOT(inactiveShadowColorChanged()));
    connect(itsWidget->inactiveShadowInnerColor, SIGNAL(changed(const QColor &)), this, SIGNAL(changed()));
    connect(itsWidget->inactiveShadowOuterColor, SIGNAL(changed(const QColor &)), this, SIGNAL(changed()));
    activeShadowColorChanged();
    inactiveShadowColorChanged();
    setShadows();
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

    itsWidget->menuClose->setChecked(configGroup.readEntry("CloseOnMenuDoubleClick", true));
    itsWidget->resizeGrip->setChecked(configGroup.readEntry("ShowResizeGrip", false));
    itsWidget->roundBottom->setChecked(configGroup.readEntry("RoundBottom", true));
    itsWidget->outerBorder->setChecked(configGroup.hasKey("NoBorder")
                                        ? !configGroup.readEntry("NoBorder", false)
                                        : configGroup.readEntry("OuterBorder", true));
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

    configGroup.writeEntry("CloseOnMenuDoubleClick", itsWidget->menuClose->isChecked());
    configGroup.writeEntry("ShowResizeGrip", itsWidget->resizeGrip->isChecked());
    configGroup.writeEntry("RoundBottom", itsWidget->roundBottom->isChecked());
    configGroup.writeEntry("OuterBorder", itsWidget->outerBorder->isChecked());
    configGroup.writeEntry("TitleBarPad", itsWidget->titleBarPad->value());
    configGroup.deleteEntry("NoBorder");
#if KDE_IS_VERSION(4, 3, 0)
    configGroup.writeEntry("CustomShadows", itsWidget->useShadows->isChecked());
    if(itsWidget->useShadows->isChecked())
    {
        itsActiveShadows.setShadowSize(itsWidget->activeShadowSize->value());
        itsActiveShadows.setVerticalOffset(itsWidget->activeShadowOffset->value());
        itsActiveShadows.setColorType((KWinQtCurve::QtCurveShadowConfiguration::ColorType)itsWidget->activeShadowColor->currentIndex());
        if(KWinQtCurve::QtCurveShadowConfiguration::CT_CUSTOM==itsWidget->activeShadowColor->currentIndex())
        {
            itsActiveShadows.setInnerColor(itsWidget->activeShadowInnerColor->color());
            itsActiveShadows.setOuterColor(itsWidget->activeShadowOuterColor->color());
        }
        itsInactiveShadows.setShadowSize(itsWidget->inactiveShadowSize->value());
        itsInactiveShadows.setVerticalOffset(itsWidget->inactiveShadowOffset->value());
        itsInactiveShadows.setColorType((KWinQtCurve::QtCurveShadowConfiguration::ColorType)itsWidget->inactiveShadowColor->currentIndex());
        if(KWinQtCurve::QtCurveShadowConfiguration::CT_CUSTOM==itsWidget->inactiveShadowColor->currentIndex())
        {
            itsInactiveShadows.setInnerColor(itsWidget->inactiveShadowInnerColor->color());
            itsInactiveShadows.setOuterColor(itsWidget->inactiveShadowOuterColor->color());
        }
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
    itsWidget->menuClose->setChecked(true);
    itsWidget->resizeGrip->setChecked(false);
    itsWidget->roundBottom->setChecked(true);
    itsWidget->outerBorder->setChecked(true);
    itsWidget->titleBarPad->setValue(0);
#if KDE_IS_VERSION(4, 3, 0)
    itsWidget->useShadows->setChecked(false);
    itsActiveShadows.defaults();
    itsInactiveShadows.defaults();
    setShadows();
#endif
}

void QtCurveKWinConfig::activeShadowColorChanged()
{
#if KDE_IS_VERSION(4, 3, 0)
    itsWidget->activeShadowInnerColor->setEnabled(KWinQtCurve::QtCurveShadowConfiguration::CT_CUSTOM==itsWidget->activeShadowColor->currentIndex());
    itsWidget->activeShadowOuterColor->setEnabled(KWinQtCurve::QtCurveShadowConfiguration::CT_CUSTOM==itsWidget->activeShadowColor->currentIndex());
    if(itsActiveShadows.colorType()!=itsWidget->activeShadowColor->currentIndex())
        emit changed();
#endif
}

void QtCurveKWinConfig::inactiveShadowColorChanged()
{
#if KDE_IS_VERSION(4, 3, 0)
    itsWidget->inactiveShadowInnerColor->setEnabled(KWinQtCurve::QtCurveShadowConfiguration::CT_CUSTOM==itsWidget->inactiveShadowColor->currentIndex());
    itsWidget->inactiveShadowOuterColor->setEnabled(KWinQtCurve::QtCurveShadowConfiguration::CT_CUSTOM==itsWidget->inactiveShadowColor->currentIndex());
    if(itsInactiveShadows.colorType()!=itsWidget->inactiveShadowColor->currentIndex())
        emit changed();
#endif
}
    
#if KDE_IS_VERSION(4, 3, 0)
void QtCurveKWinConfig::setShadows()
{
    itsWidget->activeShadowSize->setValue(itsActiveShadows.shadowSize());
    itsWidget->activeShadowOffset->setValue(itsActiveShadows.verticalOffset());
    itsWidget->activeShadowInnerColor->setColor(itsActiveShadows.innerColor());
    itsWidget->activeShadowOuterColor->setColor(itsActiveShadows.outerColor());
    itsWidget->activeShadowColor->setCurrentIndex(itsActiveShadows.colorType());
    itsWidget->inactiveShadowSize->setValue(itsInactiveShadows.shadowSize());
    itsWidget->inactiveShadowOffset->setValue(itsInactiveShadows.verticalOffset());
    itsWidget->inactiveShadowInnerColor->setColor(itsInactiveShadows.innerColor());
    itsWidget->inactiveShadowOuterColor->setColor(itsInactiveShadows.outerColor());
    itsWidget->inactiveShadowColor->setCurrentIndex(itsInactiveShadows.colorType());
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
