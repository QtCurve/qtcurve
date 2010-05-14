#ifndef __QTCURVE_CONFIG_H__
#define __QTCURVE_CONFIG_H__

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

class KConfig;

namespace KWinQtCurve
{

class QtCurveConfig
{
    public:

    enum Size
    {
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

    QtCurveConfig()               { defaults(); }

    void defaults();
    void load(const KConfig *cfg);
    void save(KConfig *cfg);

    Size borderSize() const        { return (Size)itsBorderSize; }
    bool roundBottom() const       { return itsRoundBottom; }
    bool outerBorder() const       { return itsOuterBorder; }
    bool borderlessMax() const     { return itsBorderlessMax; }
    bool customShadows() const     { return itsCustomShadows; }
    bool grouping() const          { return itsGrouping; }
    int  titleBarPad() const       { return itsTitleBarPad; }
    int  opacity(bool a) const     { return a ? itsActiveOpacity : itsInactiveOpacity; }
    void setBorderSize(Size v)     { itsBorderSize=v; }
    void setRoundBottom(bool v)    { itsRoundBottom=v; }
    void setOuterBorder(bool v)    { itsOuterBorder=v; }
    void setBorderlessMax(bool v)  { itsBorderlessMax=v; }
    void setCustomShadows(bool v)  { itsCustomShadows=v; }
    void setGrouping(bool v)       { itsGrouping=v; }
    void setTitleBarPad(int v)     { itsTitleBarPad=v; }
    void setOpacity(int v, bool a) { a ? itsActiveOpacity=v : itsInactiveOpacity=v; }

    bool operator==(const QtCurveConfig &o) const
    {
        return itsBorderSize==o.itsBorderSize &&
               itsRoundBottom==o.itsRoundBottom &&
               itsOuterBorder==o.itsOuterBorder &&
               itsBorderlessMax==o.itsBorderlessMax &&
               itsCustomShadows==o.itsCustomShadows &&
               itsGrouping==o.itsGrouping &&
               itsTitleBarPad==o.itsTitleBarPad &&
               itsActiveOpacity==o.itsActiveOpacity &&
               itsInactiveOpacity==o.itsInactiveOpacity;
    }

    bool operator!=(const QtCurveConfig &o) const { return !(*this==o); }

    private:

    int  itsBorderSize,
         itsActiveOpacity,
         itsInactiveOpacity;
    bool itsRoundBottom,
         itsOuterBorder,
         itsBorderlessMax,
         itsCustomShadows,
         itsGrouping;
    int  itsTitleBarPad;
};

}

#endif