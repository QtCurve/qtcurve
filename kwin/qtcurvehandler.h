/*
  QtCurve KWin window decoration
  Copyright (C) 2007 - 2009 Craig Drummond <ee11cd@googlemail.com>

  based on the window decoration "Plastik":
  Copyright (C) 2003-2005 Sandro Giessl <sandro@giessl.com>

  based on the window decoration "Web":
  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

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

#ifndef KWIN_QTCURVE_H
#define KWIN_QTCURVE_H

#include <QFont>
#include <QtGui/QApplication>
#include <kdeversion.h>
#include <kdecoration.h>
#include <kdecorationfactory.h>
#include "config.h"

class QStyle;

namespace KWinQtCurve
{

enum ButtonIcon
{
    CloseIcon = 0,
    MaxIcon,
    MaxRestoreIcon,
    MinIcon,
    HelpIcon,
    OnAllDesktopsIcon,
    NotOnAllDesktopsIcon,
    KeepAboveIcon,
    NoKeepAboveIcon,
    KeepBelowIcon,
    NoKeepBelowIcon,
    ShadeIcon,
    UnShadeIcon,
    MenuIcon,
    NumButtonIcons
};

class QtCurveHandler : public QObject,
                       public KDecorationFactory
{
    Q_OBJECT

    public:

    QtCurveHandler();
    ~QtCurveHandler();
    void setStyle();
    virtual bool reset( unsigned long changed );

    virtual KDecoration * createDecoration( KDecorationBridge* );
    virtual bool supports( Ability ability ) const;

    const QBitmap & buttonBitmap(ButtonIcon type, const QSize &size, bool toolWindow);
    int             titleHeight() const     { return itsTitleHeight; }
    int             titleHeightTool() const { return itsTitleHeightTool; }
    const QFont &   titleFont()             { return itsTitleFont; }
    const QFont &   titleFontTool()         { return itsTitleFontTool; }
    int             borderSize() const      { return itsBorderSize; }
    bool            coloredShadow() const   { return itsColoredShadow; }
    bool            menuClose() const       { return itsMenuClose; }
    bool            showResizeGrip() const  { return itsShowResizeGrip; }
    bool            roundBottom() const     { return itsRoundBottom && itsBorderSize>1; }
    bool            outerBorder() const     { return itsOuterBorder; }
    QStyle *        wStyle() const          { return itsStyle ? itsStyle : QApplication::style(); }
    int             borderEdgeSize() const;
    int             titleBarPad() const     { return itsTitleBarPad; }

    QList<QtCurveHandler::BorderSize>  borderSizes() const;

    private:

    bool readConfig();

    private:

    bool    itsColoredShadow,
            itsMenuClose,
            itsShowResizeGrip,
            itsRoundBottom,
            itsOuterBorder;
    int     itsBorderSize,
            itsTitleHeight,
            itsTitleHeightTool,
            itsTimeStamp,
            itsTitleBarPad;
    QFont   itsTitleFont,
            itsTitleFontTool;
    QStyle  *itsStyle;
    QBitmap *itsBitmaps[2][NumButtonIcons];
};

QtCurveHandler * Handler();

}

#endif

