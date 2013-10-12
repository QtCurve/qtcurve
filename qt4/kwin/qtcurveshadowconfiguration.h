#ifndef __QTCURVESHADOWCONFIGURATION_H__
#define __QTCURVESHADOWCONFIGURATION_H__

/*
  QtCurve KWin window decoration
  Copyright (C) 2007 - 2010 Craig Drummond <craig.p.drummond@gmail.com>

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

#include <QPalette>
#ifdef midColor
#undef midColor
#endif

class KConfig;

namespace KWinQtCurve
{

class QtCurveShadowConfiguration
{
    public:

    enum ColorType
    {
        CT_FOCUS     = 0,
        CT_HOVER     = 1,
        CT_SELECTION = 2,
        CT_TITLEBAR  = 3,
        CT_GRAY      = 4,
        CT_CUSTOM    = 5
    };

    enum ShadowType
    {
        SH_ACTIVE   = 0,
        SH_INACTIVE = 1
    };

    enum Constants
    {
        MIN_SIZE   = 10,
        MAX_SIZE   = 100,
        MIN_OFFSET = 0,
        MAX_OFFSET = 20
    };

    QtCurveShadowConfiguration(QPalette::ColorGroup);

    virtual ~QtCurveShadowConfiguration() { }

    void                 defaults();
    void                 load(KConfig *cfg);
    void                 save(KConfig *cfg);

    QPalette::ColorGroup colorGroup() const           { return itsColorGroup; }
    int                  shadowSize() const           { return itsSize; }
    void                 setShadowSize(int v)         { itsSize = v; }
    int                  horizontalOffset() const     { return itsHOffset; }
    void                 setHorizontalOffset(int v)   { itsHOffset = v; }
    int                  verticalOffset() const       { return itsVOffset; }
    void                 setVerticalOffset(int v)     { itsVOffset = v; }
    void                 setColor(const QColor &c)    { itsColor=c; }
    const QColor &       color() const                { return itsColor; }
    ColorType            colorType() const            { return (ColorType)itsColorType; }
    void                 setColorType(ColorType ct);
    ShadowType           shadowType() const           { return (ShadowType)itsShadowType; }
    void                 setShadowType(ShadowType st) { itsShadowType=st; }

    // Keep compatible with Oxygen Shadow Cache code...
    const QColor &       innerColor() const           { return itsColor; }
    const QColor &       midColor() const             { return itsColor; }
    const QColor &       outerColor() const           { return itsColor; }

    bool operator == (const QtCurveShadowConfiguration& other) const
    {
        return  itsColorGroup == other.itsColorGroup &&
                itsSize == other.itsSize &&
                itsHOffset == other.itsHOffset &&
                itsVOffset == other.itsVOffset &&
                itsColor == other.itsColor &&
                itsShadowType == other.itsShadowType;
    }

    private:

    QPalette::ColorGroup itsColorGroup;
    int                  itsSize,
                         itsHOffset,
                         itsVOffset,
                         itsColorType,
                         itsShadowType;
    QColor               itsColor;
};

}

#endif
