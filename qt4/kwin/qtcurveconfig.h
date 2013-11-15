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

#ifndef __QTCURVE_CONFIG_H__
#define __QTCURVE_CONFIG_H__

class KConfig;

namespace KWinQtCurve {

class QtCurveConfig {
public:
    enum Size {
        BORDER_NONE = 0,
        BORDER_NO_SIDES,
        BORDER_TINY,
        BORDER_NORMAL,
        BORDER_LARGE,
        BORDER_VERY_LARGE,
        BORDER_HUGE,
        BORDER_VERY_HUGE,
        BORDER_OVERSIZED
    };

    enum Shade
    {
        SHADE_NONE,
        SHADE_DARK,
        SHADE_LIGHT,
        SHADE_SHADOW
    };

    QtCurveConfig()               { defaults(); }

    void defaults();
    void load(const KConfig *cfg, const char *grp=0L);
    void save(KConfig *cfg, const char *grp=0L);

    Size  borderSize() const        { return (Size)itsBorderSize; }
    bool  roundBottom() const       { return itsRoundBottom; }
    Shade outerBorder() const       { return itsOuterBorder; }
    Shade innerBorder() const       { return itsInnerBorder; }
    bool  borderlessMax() const     { return itsBorderlessMax; }
    bool  customShadows() const     { return itsCustomShadows; }
    bool  grouping() const          { return itsGrouping; }
    int   titleBarPad() const       { return itsTitleBarPad; }
    int   opacity(bool a) const     { return a ? itsActiveOpacity : itsInactiveOpacity; }
    bool  opaqueBorder() const      { return itsOpaqueBorder; }
    int   edgePad() const           { return itsEdgePad; }
    void  setBorderSize(Size v)     { itsBorderSize=v; }
    void  setRoundBottom(bool v)    { itsRoundBottom=v; }
    void  setOuterBorder(Shade v)   { itsOuterBorder=v; }
    void  setInnerBorder(Shade v)   { itsInnerBorder=v; }
    void  setBorderlessMax(bool v)  { itsBorderlessMax=v; }
    void  setCustomShadows(bool v)  { itsCustomShadows=v; }
    void  setGrouping(bool v)       { itsGrouping=v; }
    void  setTitleBarPad(int v)     { itsTitleBarPad=v; }
    void  setOpacity(int v, bool a) { a ? itsActiveOpacity=v : itsInactiveOpacity=v; }
    void  setOpaqueBorder(bool v)   { itsOpaqueBorder=v; }
    void  setEdgePad(int v)         { itsEdgePad=v; }

    bool operator==(const QtCurveConfig &o) const
    {
        return itsBorderSize==o.itsBorderSize &&
               itsRoundBottom==o.itsRoundBottom &&
               itsOuterBorder==o.itsOuterBorder &&
               itsInnerBorder==o.itsInnerBorder &&
               itsBorderlessMax==o.itsBorderlessMax &&
               itsCustomShadows==o.itsCustomShadows &&
               itsGrouping==o.itsGrouping &&
               itsTitleBarPad==o.itsTitleBarPad &&
               itsActiveOpacity==o.itsActiveOpacity &&
               itsInactiveOpacity==o.itsInactiveOpacity &&
               itsOpaqueBorder==o.itsOpaqueBorder &&
               itsEdgePad==o.itsEdgePad;
    }

    bool operator!=(const QtCurveConfig &o) const { return !(*this==o); }

    private:

    int   itsBorderSize,
          itsActiveOpacity,
          itsInactiveOpacity;
    bool  itsRoundBottom,
          itsBorderlessMax,
          itsCustomShadows,
          itsGrouping,
          itsOpaqueBorder;
    Shade itsOuterBorder,
          itsInnerBorder;
    int   itsTitleBarPad,
          itsEdgePad;
};

}

#endif
