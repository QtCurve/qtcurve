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
            itsColor = KColorScheme(itsColorGroup).decoration(KColorScheme::FocusColor).color();
            break;
        case CT_HOVER:
            itsColor = KColorScheme(itsColorGroup).decoration(KColorScheme::HoverColor).color();
            break;
        case CT_ACTIVE_TITLEBAR:
            itsColor = KGlobalSettings::activeTitleColor();
            break;
        case CT_INACTIVE_TITLEBAR:
            itsColor = KGlobalSettings::inactiveTitleColor();
            break;
        case CT_GRAY:
            itsColor = QColor("#393835");
            break;
    }
}

#define CFG_GROUP (QPalette::Active==itsColorGroup ? "ActiveShadows" : "InactiveShadows")
#define CFG_SIZE         "Size"
#define CFG_HORIZ_OFFSET "HOffset"
#define CFG_VERT_OFFSET  "VOffset"
#define CFG_COLOR_TYPE   "ColorType"
#define CFG_COLOR        "Color"

void QtCurveShadowConfiguration::load(KConfig *cfg)
{
    KConfigGroup               configGroup(cfg, CFG_GROUP);
    QtCurveShadowConfiguration def(itsColorGroup);
    itShadowSize=configGroup.readEntry(CFG_SIZE, itShadowSize);
    itsHorizontalOffset=configGroup.readEntry(CFG_HORIZ_OFFSET, itsHorizontalOffset);
    itsVerticalOffset=configGroup.readEntry(CFG_VERT_OFFSET, itsVerticalOffset);
    itsColorType=(ColorType)configGroup.readEntry(CFG_COLOR_TYPE, (int)itsColorType);
    if(CT_CUSTOM==itsColorType)
        itsColor=configGroup.readEntry(CFG_COLOR, itsColor);
    if(itShadowSize<MIN_SIZE || itShadowSize>MAX_SIZE)
        itShadowSize=def.shadowSize();
    if(itsHorizontalOffset<MIN_OFFSET || itsHorizontalOffset>MAX_OFFSET)
        itsHorizontalOffset=def.horizontalOffset();
    if(itsVerticalOffset<MIN_OFFSET || itsVerticalOffset>MAX_OFFSET)
        itsVerticalOffset=def.verticalOffset();
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
    
    if(CT_CUSTOM!=itsColorType || itsColor==def.itsColor)
        configGroup.deleteEntry(CFG_COLOR);
    else
        configGroup.writeEntry(CFG_COLOR, itsColor);
}

}

#endif
