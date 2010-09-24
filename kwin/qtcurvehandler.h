/*
  QtCurve KWin window decoration
  Copyright (C) 2007 - 2010 Craig Drummond <craig.p.drummond@googlemail.com>

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

#include <QtGui/QFont>
#include <QtGui/QApplication>
#include <QtGui/QBitmap>
#include <kdeversion.h>
#include <kdecoration.h>
#include <kdecorationfactory.h>
#include "config.h"
#include "qtcurveconfig.h"

#if KDE_IS_VERSION(4, 3, 0)
#include "qtcurveshadowcache.h"
#endif

class QStyle;

namespace KWinQtCurve
{

enum ButtonIcon
{
    CloseIcon = 0,
#if KDE_IS_VERSION(4, 3, 85)
    CloseTabIcon,
#endif
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

class QtCurveClient;
class QtCurveDBus;

class QtCurveHandler : public QObject,
#if KDE_IS_VERSION(4, 3, 0)
                       public KDecorationFactoryUnstable
#else
                       public KDecorationFactory
#endif
{
    Q_OBJECT

    public:

    QtCurveHandler();
    ~QtCurveHandler();
    void setStyle();
    virtual bool reset(unsigned long changed);
    void setBorderSize();

    virtual KDecoration * createDecoration(KDecorationBridge *);
    virtual bool supports(Ability ability) const;

    const QBitmap &       buttonBitmap(ButtonIcon type, const QSize &size, bool toolWindow);
    int                   titleHeight() const        { return itsTitleHeight; }
    int                   titleHeightTool() const    { return itsTitleHeightTool; }
    const QFont &         titleFont()                { return itsTitleFont; }
    const QFont &         titleFontTool()            { return itsTitleFontTool; }
    int                   borderSize(bool bot=false) const;
    bool                  showResizeGrip() const     { return QtCurveConfig::BORDER_NONE==itsConfig.borderSize(); }
    bool                  haveBottomBorder() const   { return QtCurveConfig::BORDER_NONE!=itsConfig.borderSize(); }
    bool                  roundBottom() const        { return itsConfig.roundBottom() && itsConfig.borderSize()>QtCurveConfig::BORDER_NONE; }
    QtCurveConfig::Shade  outerBorder() const        { return itsConfig.outerBorder(); }
    QtCurveConfig::Shade  innerBorder() const        { return itsConfig.innerBorder(); }
    QStyle *              wStyle() const             { return itsStyle ? itsStyle : QApplication::style(); }
    int                   borderEdgeSize() const;
    int                   titleBarPad() const        { return itsConfig.titleBarPad(); }
    int                   edgePad() const            { return itsConfig.edgePad(); }
    bool                  borderlessMax() const      { return itsConfig.borderlessMax(); }
    int                   opacity(bool active) const { return itsConfig.opacity(active); }
    bool                  opaqueBorder() const       { return itsConfig.opaqueBorder(); }
#if KDE_IS_VERSION(4, 3, 0)
    bool                  customShadows() const      { return itsConfig.customShadows(); }
    QtCurveShadowCache &  shadowCache()              { return itsShadowCache; }
#endif
#if KDE_IS_VERSION(4, 3, 85)
    bool                  grouping() const           { return itsConfig.grouping(); }
#endif
    void                  menuBarSize(unsigned int xid, int size);
    void                  statusBarState(unsigned int xid, bool state);
    void                  emitToggleMenuBar(int xid);
    void                  emitToggleStatusBar(int xid);
    void                  borderSizeChanged();
    void                  addClient(QtCurveClient *c)    { itsClients.append(c); }
    void                  removeClient(QtCurveClient *c);
    bool                  wasLastMenu(int id)            { return id==itsLastMenuXid; }
    bool                  wasLastStatus(int id)          { return id==itsLastStatusXid; }

    private:

    bool readConfig(bool compositingToggled=false);

    private:

    int                    itsBorderSize,
                           itsBotBorderSize,
                           itsTitleHeight,
                           itsTitleHeightTool,
                           itsTimeStamp,
                           itsLastMenuXid,
                           itsLastStatusXid;
    QFont                  itsTitleFont,
                           itsTitleFontTool;
    QStyle                 *itsStyle;
    QBitmap                itsBitmaps[2][NumButtonIcons];
    QtCurveConfig          itsConfig;
    QList<QtCurveClient *> itsClients;
    QtCurveDBus            *itsDBus;
#if KDE_IS_VERSION(4, 3, 0)
    QtCurveShadowCache     itsShadowCache;
#endif
};

QtCurveHandler * Handler();

}

#endif

