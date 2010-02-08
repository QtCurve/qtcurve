#ifndef __QTCURVESHADOWCONFIGURATION_H__
#define __QTCURVESHADOWCONFIGURATION_H__

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

#include <QtGui/QPalette>

class KConfig;

namespace KWinQtCurve
{

class QtCurveShadowConfiguration
{
    public:

    enum ColorType
    {
        CT_FOCUS,
        CT_HOVER,
        CT_ACTIVE_TITLEBAR,
        CT_INACTIVE_TITLEBAR,
        CT_GRAY,
        CT_CUSTOM
    };
    
    QtCurveShadowConfiguration(QPalette::ColorGroup);

    virtual ~QtCurveShadowConfiguration() { }

    void                 defaults();
    void                 load(KConfig *cfg);
    void                 save(KConfig *cfg);

    QPalette::ColorGroup colorGroup() const         { return itsColorGroup; }
    int                  shadowSize() const         { return itShadowSize; }
    void                 setShadowSize(int v)       { itShadowSize = v; }
    int                  horizontalOffset() const   { return itsHorizontalOffset; }
    void                 setHorizontalOffset(int v) { itsHorizontalOffset = v; }
    int                  verticalOffset() const     { return itsVerticalOffset; }
    void                 setVerticalOffset(int v)   { itsVerticalOffset = v; }
    void                 setColor(const QColor &c)  { itsColor=c; }
    const QColor &       color() const              { return itsColor; }
    ColorType            colorType() const          { return itsColorType; }
    void                 setColorType(ColorType ct);

    // Keep compatible with Oxygen Shadow Cache code...
    const QColor &       innerColor() const         { return itsColor; }
    const QColor &       midColor() const           { return itsColor; }
    const QColor &       outerColor() const         { return itsColor; }

    bool operator == (const QtCurveShadowConfiguration& other) const
    {
        return  itsColorGroup == other.itsColorGroup &&
                itShadowSize == other.itShadowSize &&
                itsHorizontalOffset == other.itsHorizontalOffset &&
                itsVerticalOffset == other.itsVerticalOffset &&
                itsColor == other.itsColor;
    }

    private:

    QPalette::ColorGroup itsColorGroup;
    int                  itShadowSize,
                         itsHorizontalOffset,
                         itsVerticalOffset;
    ColorType            itsColorType;
    QColor               itsColor;
};

}

#endif
