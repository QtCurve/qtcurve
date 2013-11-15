/*****************************************************************************
 *   Copyright 2010 Craig Drummond <craig.p.drummond@gmail.com>              *
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

#include "qtcurveconfig.h"
#include <KDE/KConfig>
#include <KDE/KConfigGroup>

namespace KWinQtCurve
{

#define GROUP "General"

void QtCurveConfig::defaults()
{
    itsBorderSize=BORDER_NORMAL;
    itsRoundBottom=true;
    itsOuterBorder=SHADE_NONE;
    itsInnerBorder=SHADE_NONE;
    itsBorderlessMax=false;
    itsCustomShadows=false;
    itsGrouping=true;
    itsTitleBarPad=0;
    itsActiveOpacity=itsInactiveOpacity=100;
    itsOpaqueBorder=true;
    itsEdgePad=0;
}

#define READ_ENTRY(ENTRY) \
    its##ENTRY=group.readEntry(#ENTRY, def.its##ENTRY);

static QtCurveConfig::Shade readShade(KConfigGroup &group, const char *key)
{
    QString entry=group.readEntry(key, QString());

    if(entry.isEmpty() || QLatin1String("false")==entry)
        return QtCurveConfig::SHADE_NONE;
    if(QLatin1String("true")==entry)
        return QtCurveConfig::SHADE_DARK;
    int val=entry.toInt();
    if(val>QtCurveConfig::SHADE_NONE && val<=QtCurveConfig::SHADE_SHADOW)
        return (QtCurveConfig::Shade)val;
    return QtCurveConfig::SHADE_NONE;
}

void QtCurveConfig::load(const KConfig *cfg, const char *grp)
{
    KConfigGroup  group(cfg, grp ? grp : GROUP);
    QtCurveConfig def;

    if(group.hasKey("BorderSize"))
        itsBorderSize=(Size)group.readEntry("BorderSize", (int)def.borderSize());
    else
    {
        KConfig      kwin("kwinrc");
        KConfigGroup style(&kwin, "Style");
        int          size=style.readEntry("BorderSize", 1);

        if(0==size) // KDecorationDefines::BorderTiny
        {
            if(group.readEntry("DrawBottom", false))
                itsBorderSize=BORDER_NO_SIDES;
            else
                itsBorderSize=BORDER_NONE;
        }
        else
            itsBorderSize=(Size)(size+2);
    }

    if(itsBorderSize<BORDER_NONE || itsBorderSize>BORDER_OVERSIZED)
        itsBorderSize=BORDER_NORMAL;
    READ_ENTRY(BorderlessMax)
    READ_ENTRY(CustomShadows)
    READ_ENTRY(Grouping)
    READ_ENTRY(TitleBarPad)
    READ_ENTRY(ActiveOpacity)
    READ_ENTRY(InactiveOpacity)
    READ_ENTRY(OpaqueBorder)
    READ_ENTRY(EdgePad)

    if(itsTitleBarPad<-5 || itsTitleBarPad>10)
        itsTitleBarPad=0;
    if(itsEdgePad<0 || itsEdgePad>10)
        itsEdgePad=0;
    if(BORDER_NONE==itsBorderSize)
        itsRoundBottom=false;
    else
        READ_ENTRY(RoundBottom)

    itsOuterBorder=readShade(group, "OuterBorder");
    if(itsBorderSize<BORDER_TINY || SHADE_NONE==itsOuterBorder)
        itsInnerBorder=SHADE_NONE;
    else
        itsInnerBorder=readShade(group, "InnerBorder");

    if(itsActiveOpacity<0 || itsActiveOpacity>100)
        itsActiveOpacity=100;
    if(itsInactiveOpacity<0 || itsInactiveOpacity>100)
        itsInactiveOpacity=100;
}

#define WRITE_ENTRY(ENTRY) \
    if (def.its##ENTRY==its##ENTRY) \
        group.deleteEntry(#ENTRY); \
    else \
        group.writeEntry(#ENTRY, its##ENTRY);

void QtCurveConfig::save(KConfig *cfg, const char *grp)
{
    KConfigGroup  group(cfg, grp ? grp : GROUP);
    QtCurveConfig def;

    //WRITE_ENTRY(BorderSize)
    // Have to write BorderSize, because if not found we read the kwin setting - to be
    // compatible with QtCurve<1.4
    group.writeEntry("BorderSize", itsBorderSize);
    WRITE_ENTRY(RoundBottom)
    group.writeEntry("OuterBorder", (int)itsOuterBorder);
    group.writeEntry("InnerBorder", (int)itsInnerBorder);
    WRITE_ENTRY(BorderlessMax)
    WRITE_ENTRY(CustomShadows)
    WRITE_ENTRY(Grouping)
    WRITE_ENTRY(TitleBarPad)
    WRITE_ENTRY(ActiveOpacity)
    WRITE_ENTRY(InactiveOpacity)
    WRITE_ENTRY(OpaqueBorder)
    WRITE_ENTRY(EdgePad)
}

}
