/*
  QtCurve KWin window decoration
  Copyright (C) 2007 - 2009 Craig Drummond <craig_p_drummond@yahoo.co.uk>

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

#include <stdio.h>
QtCurveKWinConfig::QtCurveKWinConfig(KConfig *config, QWidget *parent)
                 : QObject(parent),
                   itsConfig(new KConfig("kwinqtcurverc")),
                   itsWidget(new QtCurveKWinConfigWidget(parent))
{
    KConfigGroup configGroup(itsConfig, "General");
    KGlobal::locale()->insertCatalog("qtcurve");

    itsWidget->show();

#if !KDE_IS_VERSION(4,1,80) || KDE_IS_VERSION(4,2,80)
    itsWidget->coloredShadow->setVisible(false);
#endif
    load(configGroup);

    connect(itsWidget->menuClose, SIGNAL(toggled(bool)),  this, SIGNAL(changed()));
    connect(itsWidget->coloredShadow, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(itsWidget->resizeGrip, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(itsWidget->roundBottom, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(itsWidget->outerBorder, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
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
    itsWidget->coloredShadow->setChecked(configGroup.readEntry("ColoredShadow", false));
    itsWidget->resizeGrip->setChecked(configGroup.readEntry("ShowResizeGrip", false));
    itsWidget->roundBottom->setChecked(configGroup.readEntry("RoundBottom", true));
    itsWidget->outerBorder->setChecked(configGroup.hasKey("NoBorder")
                                        ? !configGroup.readEntry("NoBorder", false)
                                        : configGroup.readEntry("OuterBorder", true));
}

void QtCurveKWinConfig::save(KConfigGroup &)
{
    KConfigGroup configGroup(itsConfig, "General");

    configGroup.writeEntry("CloseOnMenuDoubleClick", itsWidget->menuClose->isChecked());
    configGroup.writeEntry("ColoredShadow", itsWidget->coloredShadow->isChecked());
    configGroup.writeEntry("ShowResizeGrip", itsWidget->resizeGrip->isChecked());
    configGroup.writeEntry("RoundBottom", itsWidget->roundBottom->isChecked());
    configGroup.writeEntry("OuterBorder", itsWidget->outerBorder->isChecked());
    configGroup.deleteEntry("NoBorder");
    itsConfig->sync();
}

void QtCurveKWinConfig::defaults()
{
    itsWidget->menuClose->setChecked(true);
    itsWidget->coloredShadow->setChecked(false);
    itsWidget->resizeGrip->setChecked(false);
    itsWidget->roundBottom->setChecked(true);
    itsWidget->outerBorder->setChecked(true);
}

extern "C"
{
    KDE_EXPORT QObject * allocate_config(KConfig *config, QWidget *parent)
    {
        return new QtCurveKWinConfig(config, parent);
    }
}

#include "qtcurvekwinconfig.moc"
