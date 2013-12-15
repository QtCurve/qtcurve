/*****************************************************************************
 *   Copyright 2007 - 2010 Craig Drummond <craig.p.drummond@gmail.com>       *
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

/*
  based on the window decoration "Plastik":
  Copyright (C) 2003-2005 Sandro Giessl <sandro@giessl.com>

  based on the window decoration "Web":
  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>
 */

#include <QBitmap>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QStyleFactory>
#include <QStyle>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include "qtcurvehandler.h"
#include "qtcurveclient.h"
#include "qtcurvebutton.h"
#include "qtcurvedbus.h"
#include <KConfig>
#include <KConfigGroup>
#include <KColorUtils>
#include <KColorScheme>
#include <KGlobalSettings>
#include <KSaveFile>
#include <KWindowSystem>
#include <unistd.h>
#include <sys/types.h>
#include <kde_file.h>
#include <common/common.h>

#include <QX11Info>
#include <qtcurve-utils/x11utils.h>
#include <qtcurve-utils/dirs.h>

static time_t getTimeStamp(const QString &item)
{
    KDE_struct_stat info;

    return !item.isEmpty() && 0==KDE_lstat(QFile::encodeName(item), &info) ? info.st_mtime : 0;
}

static const QString&
xdgConfigFolder()
{
    static QString xdgDir = QString::fromLocal8Bit(qtcGetXDGConfigHome());
    return xdgDir;
}

namespace KWinQtCurve {

// make the handler accessible to other classes...
static QtCurveHandler *handler = 0;
QtCurveHandler*
Handler()
{
    return handler;
}

QtCurveHandler::QtCurveHandler() :
    itsLastMenuXid(0),
    itsLastStatusXid(0),
    itsStyle(NULL),
    itsDBus(NULL)
{
    qtcX11InitXlib(QX11Info::display());
    handler = this;
    setStyle();
    reset(0);

    itsDBus = new QtCurveDBus(this);
    QDBusConnection::sessionBus().registerObject("/QtCurve", this);
}

QtCurveHandler::~QtCurveHandler()
{
    handler = 0;
    delete itsStyle;
}

void QtCurveHandler::setStyle()
{
    // Need to use our own style instance, as want to update this when
    // settings change...
    if (!itsStyle) {
        KConfig kglobals("kdeglobals", KConfig::CascadeConfig);
        KConfigGroup general(&kglobals, "General");
        QString styleName = general.readEntry("widgetStyle", QString()).toLower();

        itsStyle = QStyleFactory::create(styleName.isEmpty() ||
                                         (styleName != "qtcurve"
#ifdef QTC_QT4_STYLE_SUPPORT
                                          && !styleName.startsWith(THEME_PREFIX)
#endif
                                             ) ? QString("QtCurve") : styleName);
        // Looks wrong with style support
        itsTimeStamp = getTimeStamp(xdgConfigFolder() + "/qtcurve/stylerc");
    }
}

bool QtCurveHandler::reset(unsigned long changed)
{
    bool styleChanged = false;
    if (abs(itsTimeStamp -
            getTimeStamp(xdgConfigFolder() + "/qtcurve/stylerc")) > 2) {
        delete itsStyle;
        itsStyle = 0L;
        setStyle();
        styleChanged = true;
    }

    // we assume the active font to be the same as the inactive font since the
    // control center doesn't offer different settings anyways.
    itsTitleFont = KDecoration::options()->font(true, false); // not small
    itsTitleFontTool = KDecoration::options()->font(true, true); // small

    itsHoverCols[0]=KColorScheme(QPalette::Inactive).decoration(KColorScheme::HoverColor).color();
    itsHoverCols[1]=KColorScheme(QPalette::Active).decoration(KColorScheme::HoverColor).color();

    // read in the configuration
    bool configChanged=readConfig(
#if KDE_IS_VERSION(4, 3, 85)
                                  changed & SettingCompositing
#endif
                                  );

    setBorderSize();

    for (int t = 0;t < 2;++t) {
        for (int i = 0;i < NumButtonIcons;i++) {
            itsBitmaps[t][i] = QPixmap();
        }
    }

    // Do we need to "hit the wooden hammer" ?
    bool needHardReset = true;
    // TODO: besides the Color and Font settings I can maybe handle more changes
    //       without a hard reset. I will do this later...
    if (!styleChanged && (changed & ~(SettingColors | SettingFont | SettingButtons)) == 0)
       needHardReset = false;

    if (needHardReset || configChanged) {
        return true;
    } else {
        resetDecorations(changed);
        return false;
    }
}

void QtCurveHandler::setBorderSize()
{
    switch (itsConfig.borderSize()) {
    case QtCurveConfig::BORDER_NONE:
    case QtCurveConfig::BORDER_NO_SIDES:
        itsBorderSize = 1;
        break;
    case QtCurveConfig::BORDER_TINY:
        itsBorderSize = 2;
        break;
    case QtCurveConfig::BORDER_LARGE:
        itsBorderSize = 8;
        break;
    case QtCurveConfig::BORDER_VERY_LARGE:
        itsBorderSize = 12;
        break;
    case QtCurveConfig::BORDER_HUGE:
        itsBorderSize = 18;
        break;
    case QtCurveConfig::BORDER_VERY_HUGE:
        itsBorderSize = 27;
        break;
    case QtCurveConfig::BORDER_OVERSIZED:
        itsBorderSize = 40;
        break;
    case QtCurveConfig::BORDER_NORMAL:
    default:
        itsBorderSize = 4;
    }

    if (!outerBorder() && (itsBorderSize == 1 || itsBorderSize > 4)) {
        itsBorderSize--;
    } else if (outerBorder() && innerBorder() &&
               itsConfig.borderSize() <= QtCurveConfig::BORDER_NORMAL) {
        itsBorderSize += 2;
    }
}

KDecoration*
QtCurveHandler::createDecoration(KDecorationBridge *bridge)
{
    return (new QtCurveClient(bridge, this))->decoration();
}

bool QtCurveHandler::supports(Ability ability) const
{
    switch (ability) {
    // announce
    case AbilityAnnounceButtons:
    case AbilityAnnounceColors:
    // buttons
    case AbilityButtonMenu:
    case AbilityButtonOnAllDesktops:
    case AbilityButtonSpacer:
    case AbilityButtonHelp:
    case AbilityButtonMinimize:
    case AbilityButtonMaximize:
    case AbilityButtonClose:
    case AbilityButtonAboveOthers:
    case AbilityButtonBelowOthers:
    case AbilityButtonShade:
    // TODO
    // case AbilityButtonResize:
#if KDE_IS_VERSION(4, 9, 85)
    case AbilityButtonApplicationMenu:
#endif
    // colors
    case AbilityColorTitleBack:
    case AbilityColorTitleFore:
    case AbilityColorFrame:
        return true;
#if KDE_IS_VERSION(4, 3, 0)
    case AbilityUsesAlphaChannel:
        return true; // !Handler()->outerBorder(); ???
    case AbilityProvidesShadow:
        return customShadows();
#endif
#if KDE_IS_VERSION(4, 3, 85) && !KDE_IS_VERSION(4, 8, 80)
    case AbilityClientGrouping:
        return grouping();
#endif
#if KDE_IS_VERSION(4, 5, 85)
    case AbilityUsesBlurBehind:
        return opacity(true)<100 || opacity(false)<100 || wStyle()->pixelMetric((QStyle::PixelMetric)QtC_CustomBgnd, 0L, 0L);
#endif
        // TODO's
    default:
        return false;
    }
}

bool QtCurveHandler::readConfig(bool compositingToggled)
{
    QtCurveConfig      oldConfig=itsConfig;
    KConfig            configFile("kwinqtcurverc");
    const KConfigGroup config(&configFile, "General");
    QFontMetrics       fm(itsTitleFont);  // active font = inactive font
    int                oldSize=itsTitleHeight,
                       oldToolSize=itsTitleHeightTool;
    bool               changedBorder=false;

    // The title should stretch with bigger font sizes!
    itsTitleHeight = qMax(16, fm.height() + 4); // 4 px for the shadow etc.
    // have an even title/button size so the button icons are fully centered...
    if (itsTitleHeight%2 == 0)
        itsTitleHeight++;

    fm = QFontMetrics(itsTitleFontTool);  // active font = inactive font
    // The title should stretch with bigger font sizes!
    itsTitleHeightTool = qMax(13, fm.height()); // don't care about the shadow etc.
    // have an even title/button size so the button icons are fully centered...
    if (itsTitleHeightTool%2 == 0)
        itsTitleHeightTool++;

    itsConfig.load(&configFile);

#if KDE_IS_VERSION(4, 3, 85)
    static bool borderHack=false;
    if(borderHack)
    {
    itsConfig.setOuterBorder(KWindowSystem::compositingActive() ? QtCurveConfig::SHADE_NONE :
                             (itsConfig.customShadows() ? QtCurveConfig::SHADE_SHADOW : QtCurveConfig::SHADE_DARK));
        changedBorder=true;
        borderHack=false;
    }
    else if(compositingToggled && !itsConfig.outerBorder() &&
           (itsConfig.borderSize()<QtCurveConfig::BORDER_TINY ||
            (wStyle()->pixelMetric((QStyle::PixelMetric)QtC_WindowBorder, 0L, 0L)&WINDOW_BORDER_COLOR_TITLEBAR_ONLY)))
    {
        QDBusConnection::sessionBus().send(QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig"));
        borderHack=true;
    }
#endif

    itsTitleHeight+=2*titleBarPad();

    QFile in(xdgConfigFolder() + "/qtcurve/" BORDER_SIZE_FILE);
    int   prevSize(-1), prevToolSize(-1), prevSide(-1), prevBot(-1);

    if(in.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&in);
        prevSize=in.readLine().toInt();
        prevToolSize=in.readLine().toInt();
        prevBot=in.readLine().toInt();
        prevSide=in.readLine().toInt();
        in.close();
    }

    setBorderSize();

    int borderEdge=borderEdgeSize()*2;
    bool borderSizesChanged=prevSize!=(itsTitleHeight+borderEdge) || prevToolSize!=(itsTitleHeightTool+borderEdge) ||
                            prevBot!=borderSize(true) || prevSide!=borderSize(false);
    if(borderSizesChanged)
    {
        KSaveFile sizeFile(xdgConfigFolder() + "/qtcurve/" BORDER_SIZE_FILE);

        if (sizeFile.open())
        {
            QTextStream stream(&sizeFile);
            stream << itsTitleHeight+borderEdge << endl
                   << itsTitleHeightTool+borderEdge << endl
                   << borderSize(true) << endl
                   << borderSize(false) << endl;
            stream.flush();
            sizeFile.finalize();
            sizeFile.close();
        }
    }
#if KDE_IS_VERSION(4, 3, 0)
    bool shadowChanged(false);

    if(customShadows())
    {
        QtCurveShadowConfiguration actShadow(QPalette::Active),
                                   inactShadow(QPalette::Inactive);

        actShadow.load(&configFile);
        inactShadow.load(&configFile);

        shadowChanged=itsShadowCache.shadowConfigurationChanged(actShadow) ||
                      itsShadowCache.shadowConfigurationChanged(inactShadow);

        itsShadowCache.setShadowConfiguration(actShadow);
        itsShadowCache.setShadowConfiguration(inactShadow);

        if(shadowChanged || oldConfig.roundBottom()!=roundBottom())
            itsShadowCache.reset();
    }
#endif

    if(itsDBus && (borderSizesChanged || changedBorder))
    {
        itsDBus->emitBorderSizes(); // KDE4 apps...
        borderSizeChanged(); // Gtk2 apps...
    }

    return changedBorder ||
           oldSize!=itsTitleHeight ||
           oldToolSize!=itsTitleHeightTool ||
#if KDE_IS_VERSION(4, 3, 0)
           shadowChanged ||
#endif
           itsConfig!=oldConfig;
}

const QBitmap & QtCurveHandler::buttonBitmap(ButtonIcon type, const QSize &size, bool toolWindow)
{
    int typeIndex(type),
        reduceW(size.width()>14 ? static_cast<int>((2.0*(size.width()/3.5))+0.5) : 6),
        reduceH(size.height()>14 ? static_cast<int>((2.0*(size.height()/3.5))+0.5) : 6),
        w(size.width() - reduceW),
        h(size.height() - reduceH);

    if (itsBitmaps[toolWindow][typeIndex].size()!=QSize(w,h))
        itsBitmaps[toolWindow][typeIndex] = IconEngine::icon(type /*icon*/, qMin(w, h), wStyle());
    return itsBitmaps[toolWindow][typeIndex];
}

int QtCurveHandler::borderSize(bool bot) const
{
    if(bot)
    {
        if(QtCurveConfig::BORDER_NO_SIDES==itsConfig.borderSize())
            return itsBorderSize+5;
        else if(QtCurveConfig::BORDER_TINY==itsConfig.borderSize() && itsConfig.roundBottom() && itsConfig.outerBorder())
            return itsBorderSize+1;
    }
    return itsBorderSize;
}

void QtCurveHandler::borderSizeChanged()
{
    foreach (QtCurveClient *client, itsClients) {
        client->informAppOfBorderSizeChanges();
    }
}

void QtCurveHandler::menuBarSize(unsigned int xid, int size)
{
    foreach (QtCurveClient *client, itsClients) {
        if (client->windowId() == xid) {
            client->menuBarSize(size);
            break;
        }
    }
    itsLastMenuXid = xid;
}

void QtCurveHandler::statusBarState(unsigned int xid, bool state)
{
    foreach (QtCurveClient *client, itsClients) {
        if (client->windowId() == xid) {
            client->statusBarState(state);
            break;
        }
    }
    itsLastStatusXid = xid;
}

void QtCurveHandler::emitToggleMenuBar(int xid)
{
    itsDBus->emitMbToggle(xid);
}

void QtCurveHandler::emitToggleStatusBar(int xid)
{
    itsDBus->emitSbToggle(xid);
}

int QtCurveHandler::borderEdgeSize() const
{
    return itsConfig.edgePad()+
                (outerBorder()
                    ? (itsConfig.borderSize()>QtCurveConfig::BORDER_NO_SIDES &&
                        wStyle()->pixelMetric((QStyle::PixelMetric)QtC_Round, 0L, 0L)<ROUND_FULL)
                        ? wStyle()->pixelMetric((QStyle::PixelMetric)QtC_WindowBorder, 0L, 0L)&WINDOW_BORDER_ADD_LIGHT_BORDER
                            ? 2
                            : 1
                        : 3
                    : 1);
}

void QtCurveHandler::removeClient(QtCurveClient *c)
{
    if(c->windowId()==itsLastMenuXid)
        itsLastMenuXid=0;
    if(c->windowId()==itsLastStatusXid)
        itsLastStatusXid=0;
    itsClients.removeAll(c);
}

}

#if KDE_IS_VERSION(4, 9, 0)
KWIN_DECORATION(KWinQtCurve::QtCurveHandler)
#else
extern "C" {
KDE_EXPORT KDecorationFactory*
create_factory()
{
    return new KWinQtCurve::QtCurveHandler();
}
}
#endif

#include "qtcurvedbus.moc"
#include "qtcurvehandler.moc"
