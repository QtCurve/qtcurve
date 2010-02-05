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

#include <KConfigGroup>
#include <QtGui/QPalette>

namespace KWinQtCurve
{

class QtCurveShadowConfiguration
{

    public:

    QtCurveShadowConfiguration(QPalette::ColorGroup);

    virtual ~QtCurveShadowConfiguration() { }

    QPalette::ColorGroup colorGroup() const             { return itsColorGroup; }
    qreal                shadowSize() const             { return itShadowSize; }
    void                 setShadowSize(qreal v)         { itShadowSize = v; }
    qreal                horizontalOffset() const       { return itsHorizontalOffset; }
    void                 setHorizontalOffset(qreal v)   { itsHorizontalOffset = v; }
    qreal                verticalOffset() const         { return itsVerticalOffset; }
    void                 setVerticalOffset(qreal v)     { itsVerticalOffset = v; }
    QColor               innerColor() const             { return itsInnerColor; }
    void                 setInnerColor(const QColor &c) { itsInnerColor=c; }
    QColor               outerColor() const             { return itsOuterColor ; }
    void                 setOuterColor(const QColor &c) { itsOuterColor=c; }

    bool operator == (const QtCurveShadowConfiguration& other) const
    {
        return  itsColorGroup == other.itsColorGroup &&
                itShadowSize == other.itShadowSize &&
                itsHorizontalOffset == other.itsHorizontalOffset &&
                itsVerticalOffset == other.itsVerticalOffset &&
                itsInnerColor == other.itsInnerColor &&
                itsOuterColor == other.itsOuterColor;
    }

    private:

    QPalette::ColorGroup itsColorGroup;
    qreal                itShadowSize,
                         itsHorizontalOffset,
                         itsVerticalOffset;
    QColor               itsInnerColor,
                         itsOuterColor;
};

}

#endif
