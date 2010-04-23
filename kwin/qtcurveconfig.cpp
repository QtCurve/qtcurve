/*
  QtCurve (C) Craig Drummond, 2010 craig.p.drummond@googlemail.com

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
#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <kdecoration.h>

namespace KWinQtCurve
{

#define GROUP "General"
#define NEW_GROUP "KWinQtCurve"

void QtCurveConfig::defaults()
{
    itsBorderSize=BORDER_NORMAL;
    itsRoundBottom=true;
    itsOuterBorder=false;
    itsBorderlessMax=false;
    itsCustomShadows=false;
    itsGrouping=true;
    itsTitleBarPad=0;
}

void QtCurveConfig::load(const KConfig *cfg)
{
    KConfigGroup  group(cfg, GROUP);
    QtCurveConfig def;

    if(group.hasKey("BorderSize"))
        itsBorderSize=(Size)group.readEntry("BorderSize", (int)def.borderSize());
    else
    {
        KConfig      kwin("kwinrc");
        KConfigGroup style(&kwin, "Style");
        int          size=style.readEntry("BorderSize", 1);

        if(KDecorationDefines::BorderTiny==size)
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
    itsBorderlessMax=group.readEntry("BorderlessMax", def.borderlessMax());
    itsCustomShadows=group.readEntry("CustomShadows", def.customShadows());
    itsGrouping=group.readEntry("Grouping", def.grouping());
    itsTitleBarPad=group.readEntry("TitleBarPad", def.titleBarPad());
    if(itsTitleBarPad<0 || itsTitleBarPad>10)
        itsTitleBarPad=0;
    if(BORDER_NONE==itsBorderSize)
        itsRoundBottom=false;
    else
        itsRoundBottom=group.readEntry("RoundBottom", def.roundBottom());

    if(itsBorderSize<BORDER_TINY)
        itsOuterBorder=false;
    else
        itsOuterBorder=group.readEntry("OuterBorder", def.outerBorder());
}

#define WRITE_ENTRY(ENTRY) \
    if (def.its##ENTRY==its##ENTRY) \
        group.deleteEntry(#ENTRY); \
    else \
        group.writeEntry(#ENTRY, its##ENTRY);

void QtCurveConfig::save(KConfig *cfg)
{
    KConfigGroup  group(cfg, GROUP);
    QtCurveConfig def;

    WRITE_ENTRY(BorderSize)
    WRITE_ENTRY(RoundBottom)
    WRITE_ENTRY(OuterBorder)
    WRITE_ENTRY(BorderlessMax)
    WRITE_ENTRY(CustomShadows)
    WRITE_ENTRY(Grouping)
    WRITE_ENTRY(TitleBarPad)
}

}
