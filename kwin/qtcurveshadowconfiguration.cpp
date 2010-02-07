/*
  QtCurve KWin window decoration
  Copyright (C) 2007 - 2010 Craig Drummond <craig.p.drummond@googlemail.com>

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
#if KDE_IS_VERSION(4, 3, 0)

#include <KColorScheme>
#include <KGlobalSettings>
#include <KConfig>
#include <KConfigGroup>
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
    itsHorizontalOffset = 0;
    itsVerticalOffset = 5;
        
    if(QPalette::Active==itsColorGroup)
    {
        itShadowSize = 29;
        setColorType(CT_FOCUS);
    }
    else
    {
        itShadowSize = 25;
        setColorType(CT_GRAY);
    }
}

void QtCurveShadowConfiguration::setColorType(ColorType ct)
{
    itsColorType=ct;
    
    switch(itsColorType)
    {
        default:
        case CT_FOCUS:
            itsInnerColor = itsOuterColor = KColorScheme(itsColorGroup).decoration(KColorScheme::FocusColor).color();
            break;
        case CT_HOVER:
            itsInnerColor = itsOuterColor = KColorScheme(itsColorGroup).decoration(KColorScheme::HoverColor).color();
            break;
        case CT_ACTIVE_TITLEBAR:
            itsInnerColor = itsOuterColor = KGlobalSettings::activeTitleColor();
            break;
        case CT_INACTIVE_TITLEBAR:
            itsInnerColor = itsOuterColor = KGlobalSettings::inactiveTitleColor();
            break;
        case CT_GRAY:
            itsInnerColor = itsOuterColor = QColor("#393835");
            break;
    }
}

#define CFG_GROUP (QPalette::Active==itsColorGroup ? "ActiveShadows" : "InactiveShadows")
#define CFG_SIZE         "Size"
#define CFG_HORIZ_OFFSET "HOffset"
#define CFG_VERT_OFFSET  "VOffset"
#define CFG_COLOR_TYPE   "ColorType"
#define CFG_INNER_COL    "InnerColor"
#define CFG_OUTER_COL    "OuterColor"

void QtCurveShadowConfiguration::load(KConfig *cfg)
{
    KConfigGroup configGroup(cfg, CFG_GROUP);
    itShadowSize=configGroup.readEntry(CFG_SIZE, itShadowSize);
    itsHorizontalOffset=configGroup.readEntry(CFG_HORIZ_OFFSET, itsHorizontalOffset);
    itsVerticalOffset=configGroup.readEntry(CFG_VERT_OFFSET, itsVerticalOffset);
    itsColorType=(ColorType)configGroup.readEntry(CFG_COLOR_TYPE, (int)itsColorType);
    if(CT_CUSTOM==itsColorType)
    {
        itsInnerColor=configGroup.readEntry(CFG_INNER_COL, itsInnerColor);
        itsOuterColor=configGroup.readEntry(CFG_OUTER_COL, itsOuterColor);
    }
}

void QtCurveShadowConfiguration::save(KConfig *cfg)
{
    KConfigGroup               configGroup(cfg, CFG_GROUP);
    QtCurveShadowConfiguration def(itsColorGroup);
    
    if(itShadowSize==def.itShadowSize)
        configGroup.deleteEntry(CFG_SIZE);
    else
        configGroup.writeEntry(CFG_SIZE, itShadowSize);

    if(itsHorizontalOffset==def.itsHorizontalOffset)
        configGroup.deleteEntry(CFG_HORIZ_OFFSET);
    else
        configGroup.writeEntry(CFG_HORIZ_OFFSET, itsHorizontalOffset);

    if(itsVerticalOffset==def.itsVerticalOffset)
        configGroup.deleteEntry(CFG_VERT_OFFSET);
    else
        configGroup.writeEntry(CFG_VERT_OFFSET, itsVerticalOffset);

    if(itsColorType==def.itsColorType)
        configGroup.deleteEntry(CFG_COLOR_TYPE);
    else
        configGroup.writeEntry(CFG_COLOR_TYPE, (int)itsColorType);
    
    if(CT_CUSTOM!=itsColorType || itsInnerColor==def.itsInnerColor)
        configGroup.deleteEntry(CFG_INNER_COL);
    else
        configGroup.writeEntry(CFG_INNER_COL, itsInnerColor);

    if(CT_CUSTOM!=itsColorType || itsOuterColor==def.itsOuterColor)
        configGroup.deleteEntry(CFG_OUTER_COL);
    else
        configGroup.writeEntry(CFG_OUTER_COL, itsOuterColor);
}

}

#endif
