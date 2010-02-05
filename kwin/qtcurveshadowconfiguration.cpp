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
#include "qtcurveshadowconfiguration.h"

namespace KWinQtCurve
{

QtCurveShadowConfiguration::QtCurveShadowConfiguration(QPalette::ColorGroup colorGroup)
                          : itsColorGroup(colorGroup)
{
    if(QPalette::Active==colorGroup)
    {
        itShadowSize = 29;
        itsHorizontalOffset = 0;
        itsVerticalOffset = 0.05;
        itsInnerColor = itsOuterColor = KColorScheme(QPalette::Active).decoration(KColorScheme::FocusColor).color();
    }
    else
    {
        itShadowSize = 25;
        itsHorizontalOffset = 0;
        itsVerticalOffset = 0.05;
        itsInnerColor = itsOuterColor = QColor("#393835");
    }
}

}

#endif
