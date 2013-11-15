/*****************************************************************************
 *   Copyright 2007 - 2010 Craig Drummond <craig.p.drummond@gmail.com>       *
 *   Copyright 2013 - 2013 Yichao Yu <yyc1992@gmail.com>                     *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU Lesser General Public License as          *
 *   published by the Free Software Foundation; either version 2.1 of the    *
 *   License, or (at your option) version 3, or any later version accepted   *
 *   by the membership of KDE e.V. (or its successor approved by the         *
 *   membership of KDE e.V.), which shall act as a proxy defined in          *
 *   Section 6 of version 3 of the license.                                  *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 *   Lesser General Public License for more details.                         *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with this library. If not,                                *
 *   see <http://www.gnu.org/licenses/>.                                     *
 *****************************************************************************/

#include <kdeversion.h>
#if KDE_IS_VERSION(4, 3, 0)

#include <KColorScheme>
#include <KGlobalSettings>
#include <KConfig>
#include <KConfigGroup>
#include <QApplication>
#include "qtcurveshadowconfiguration.h"

namespace KWinQtCurve
{

QtCurveShadowConfiguration::QtCurveShadowConfiguration(QPalette::ColorGroup colorGroup)
                          : itsColorGroup(colorGroup)
{
    defaults();
}

void QtCurveShadowConfiguration::defaults()
{
    itsHOffset = 0;
    itsVOffset = 5;
    if(QPalette::Active==itsColorGroup)
    {
        itsSize = 35;
        setColorType(CT_FOCUS);
        itsShadowType = SH_ACTIVE;
    }
    else
    {
        itsSize = 30;
        setColorType(CT_GRAY);
        itsShadowType = SH_INACTIVE;
    }
}

void QtCurveShadowConfiguration::setColorType(ColorType ct)
{
    itsColorType=ct;
    switch(itsColorType)
    {
        default:
        case CT_FOCUS:
            itsColor = KColorScheme(itsColorGroup).decoration(KColorScheme::FocusColor).color();
            break;
        case CT_HOVER:
            itsColor = KColorScheme(itsColorGroup).decoration(KColorScheme::HoverColor).color();
            break;
        case CT_SELECTION:
            itsColor = QApplication::palette().color(itsColorGroup, QPalette::Highlight);
            break;
        case CT_TITLEBAR:
            itsColor = QPalette::Active==itsColorGroup
                        ? KGlobalSettings::activeTitleColor()
                        : KGlobalSettings::inactiveTitleColor();
            break;
        case CT_GRAY:
            itsColor = QColor("#393835");
            break;
        case CT_CUSTOM:
            break;
    }
}

#define CFG_GROUP (QPalette::Active==itsColorGroup ? "ActiveShadows" : "InactiveShadows")

#define READ_ENTRY(ENTRY) \
    its##ENTRY=group.readEntry(#ENTRY, def.its##ENTRY);

void QtCurveShadowConfiguration::load(KConfig *cfg)
{
    KConfigGroup               group(cfg, CFG_GROUP);
    QtCurveShadowConfiguration def(itsColorGroup);

    READ_ENTRY(Size)
    READ_ENTRY(HOffset)
    READ_ENTRY(VOffset)
    READ_ENTRY(ColorType)
    READ_ENTRY(ShadowType)

    if(CT_CUSTOM==itsColorType)
        READ_ENTRY(Color)
    if(itsSize<MIN_SIZE || itsSize>MAX_SIZE)
        itsSize=def.shadowSize();
    if(itsHOffset<MIN_OFFSET || itsHOffset>MAX_OFFSET)
        itsHOffset=def.horizontalOffset();
    if(itsVOffset<MIN_OFFSET || itsVOffset>MAX_OFFSET)
        itsVOffset=def.verticalOffset();
    setColorType((ColorType)itsColorType);
}

#define WRITE_ENTRY(ENTRY) \
    if (def.its##ENTRY==its##ENTRY) \
        group.deleteEntry(#ENTRY); \
    else \
        group.writeEntry(#ENTRY, its##ENTRY);

void QtCurveShadowConfiguration::save(KConfig *cfg)
{
    KConfigGroup               group(cfg, CFG_GROUP);
    QtCurveShadowConfiguration def(itsColorGroup);

    WRITE_ENTRY(Size)
    WRITE_ENTRY(HOffset)
    WRITE_ENTRY(VOffset)
    WRITE_ENTRY(ColorType)
    WRITE_ENTRY(ShadowType)

    if(CT_CUSTOM!=itsColorType)
        group.deleteEntry("Color");
    else
    {
        WRITE_ENTRY(Color);
    }
}

}

#endif
