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

#include <QBitmap>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QStyleFactory>
#include <QStyle>
#include <QDir>
#include "qtcurvehandler.h"
#include "qtcurveclient.h"
#include "qtcurvebutton.h"
#include <QApplication>
#include <KConfig>
#include <KConfigGroup>
#include <KColorUtils>
#include <KColorScheme>
#include <KGlobalSettings>
#include <unistd.h>
#include <sys/types.h>
#include <kde_file.h>
#include "common.h"

static time_t getTimeStamp(const QString &item)
{
    KDE_struct_stat info;

    return !item.isEmpty() && 0==KDE_lstat(QFile::encodeName(item), &info) ? info.st_mtime : 0;
}

static const QString & xdgConfigFolder()
{
    static QString xdgDir;

    if(xdgDir.isEmpty())
    {
        /*
           Hmm... for 'root' dont bother to check env var, just set to ~/.config
           - as problems would arise if "sudo kcmshell style", and then
           "sudo su" / "kcmshell style". The 1st would write to ~/.config, but
           if root has a XDG_ set then that would be used on the second :-(
        */
        char *env=0==getuid() ? NULL : getenv("XDG_CONFIG_HOME");

        if(!env)
            xdgDir=QDir::homePath()+"/.config";
        else
            xdgDir=env;
    }

    return xdgDir;
}

namespace KWinQtCurve
{

QtCurveHandler::QtCurveHandler()
              : itsTitleBarPad(0)
              , itsStyle(NULL)
#if KDE_IS_VERSION(4, 3, 0)
              , itsCustomShadows(false)
#endif
{
    setStyle();
    reset(0);
}

QtCurveHandler::~QtCurveHandler()
{
    delete itsStyle;
}

void QtCurveHandler::setStyle()
{
#if 0
    if(!qstrcmp(QApplication::style()->metaObject()->className(), "QtCurveStyle")) // The user has select QtCurve...
    {
        if(itsStyle) // ...but it wasn't QtCurve before, so delete our QtC instance...
        {
            delete itsStyle;
            itsStyle=NULL;
        }
    }
    else if(!itsStyle) // ...user has not selected QtC, so need to create a QtC instance...
        itsStyle=QStyleFactory::create("QtCurve");
#endif

    // Need to use or ouwn style instance, as want to update this when settings change...
    if(!itsStyle)
    {
        KConfig      kglobals("kdeglobals", KConfig::CascadeConfig);
        KConfigGroup general(&kglobals, "General");
        QString      styleName=general.readEntry("widgetStyle", QString()).toLower();

        itsStyle=QStyleFactory::create(styleName.isEmpty() || styleName=="qtcurve" || !styleName.startsWith("qtc_")
                                        ? QString("QtCurve") : styleName);
        itsTimeStamp=getTimeStamp(xdgConfigFolder()+"/qtcurve/stylerc");
    }
}

bool QtCurveHandler::reset(unsigned long changed)
{
    bool styleChanged=false;
    if(abs(itsTimeStamp-getTimeStamp(xdgConfigFolder()+"/qtcurve/stylerc"))>2)
    {
        delete itsStyle;
        itsStyle=0L;
        setStyle();
        styleChanged=true;
    }
    
    // we assume the active font to be the same as the inactive font since the control
    // center doesn't offer different settings anyways.
    itsTitleFont = KDecoration::options()->font(true, false); // not small
    itsTitleFontTool = KDecoration::options()->font(true, true); // small

    // read in the configuration
    bool configChanged=readConfig();

    switch(KDecoration::options()->preferredBorderSize(this))
    {
        case BorderTiny:
            itsBorderSize = 1;
            break;
        case BorderLarge:
            itsBorderSize = 8;
            break;
        case BorderVeryLarge:
            itsBorderSize = 12;
            break;
        case BorderHuge:
            itsBorderSize = 18;
            break;
        case BorderVeryHuge:
            itsBorderSize = 27;
            break;
        case BorderOversized:
            itsBorderSize = 40;
            break;
        case BorderNormal:
        default:
            itsBorderSize = 4;
    }

    if(!itsOuterBorder && (itsBorderSize==1 || itsBorderSize>4))
        itsBorderSize--;

    for (int t=0; t < 2; ++t)
        for (int i=0; i < NumButtonIcons; i++)
            itsBitmaps[t][i]=QPixmap();

    // Do we need to "hit the wooden hammer" ?
    bool needHardReset = true;
    // TODO: besides the Color and Font settings I can maybe handle more changes
    //       without a hard reset. I will do this later...
    if (!styleChanged && (changed & ~(SettingColors | SettingFont | SettingButtons)) == 0)
       needHardReset = false;

    if (needHardReset || configChanged)
        return true;
    else
    {
        resetDecorations(changed);
        return false;
    }
}

KDecoration * QtCurveHandler::createDecoration(KDecorationBridge *bridge)
{
    return (new QtCurveClient(bridge, this))->decoration();
}

bool QtCurveHandler::supports(Ability ability) const
{
    switch(ability)
    {
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
        // colors
        case AbilityColorTitleBack:
        case AbilityColorTitleFore:
        case AbilityColorFrame:
            return true;
#if KDE_IS_VERSION(4, 3, 0)
        case AbilityUsesAlphaChannel:
            return true; // !Handler()->outerBorder(); ???
        case AbilityProvidesShadow:
            return itsCustomShadows;
#endif
        default:
            return false;
    };
}

bool QtCurveHandler::readConfig()
{
    KConfig configFile("kwinqtcurverc");
    const KConfigGroup config(&configFile, "General");

    QFontMetrics fm(itsTitleFont);  // active font = inactive font
    int titleHeightMin = config.readEntry("MinTitleHeight", 16);
    // The title should stretch with bigger font sizes!
    itsTitleHeight = qMax(titleHeightMin, fm.height() + 4); // 4 px for the shadow etc.
    // have an even title/button size so the button icons are fully centered...
    if (itsTitleHeight%2 == 0)
        itsTitleHeight++;

    fm = QFontMetrics(itsTitleFontTool);  // active font = inactive font
    int titleHeightToolMin = config.readEntry("MinTitleHeightTool", 13);
    // The title should stretch with bigger font sizes!
    itsTitleHeightTool = qMax(titleHeightToolMin, fm.height()); // don't care about the shadow etc.
    // have an even title/button size so the button icons are fully centered...
    if (itsTitleHeightTool%2 == 0)
        itsTitleHeightTool++;

    bool oldMenuClose=itsMenuClose,
         oldShowResizeGrip=itsShowResizeGrip,
         oldRoundBottom=itsRoundBottom,
         oldOuterBorder=itsOuterBorder;
    int  oldTitleBarPad=itsTitleBarPad;
#if KDE_IS_VERSION(4, 3, 0)
    bool oldCustomShadows(itsCustomShadows);
#endif

    itsMenuClose = config.readEntry("CloseOnMenuDoubleClick", true);
    itsShowResizeGrip = config.readEntry("ShowResizeGrip", false);
    itsRoundBottom = config.readEntry("RoundBottom", true);
    itsOuterBorder = config.hasKey("NoBorder")
                        ? !config.readEntry("NoBorder", false)
                        : config.readEntry("OuterBorder", true);
    itsTitleBarPad = config.readEntry("TitleBarPad", 0);

    itsTitleHeight+=2*itsTitleBarPad;
#if KDE_IS_VERSION(4, 3, 0)
    bool shadowChanged(false);

    itsCustomShadows = config.readEntry("CustomShadows", false);

    if(itsCustomShadows)
    {
        QtCurveShadowConfiguration actShadow(QPalette::Active),
                                   inactShadow(QPalette::Inactive);

        if(itsCustomShadows)
            itsOuterBorder=false;

        actShadow.load(&configFile);
        inactShadow.load(&configFile);

        shadowChanged=itsCustomShadows &&
                       (itsShadowCache.shadowConfigurationChanged(actShadow) ||
                        itsShadowCache.shadowConfigurationChanged(inactShadow));

        itsShadowCache.setShadowConfiguration(actShadow);
        itsShadowCache.setShadowConfiguration(inactShadow);
    }
#endif

    return oldMenuClose!=itsMenuClose ||
           oldShowResizeGrip!=itsShowResizeGrip ||
           oldRoundBottom!=itsRoundBottom ||
           oldOuterBorder!=itsOuterBorder ||
#if KDE_IS_VERSION(4, 3, 0)
           oldCustomShadows!=itsCustomShadows ||
           shadowChanged ||
#endif
           oldTitleBarPad!=itsTitleBarPad;
}

const QBitmap & QtCurveHandler::buttonBitmap(ButtonIcon type, const QSize &size, bool toolWindow)
{
    int typeIndex(type),
        reduceW(size.width()>14 ? static_cast<int>(2*(size.width()/3.5)) : 6),
        reduceH(size.height()>14 ? static_cast<int>(2*(size.height()/3.5)) : 6),
        w(size.width() - reduceW),
        h(size.height() - reduceH);

    if (itsBitmaps[toolWindow][typeIndex].size()!=QSize(w,h))
        itsBitmaps[toolWindow][typeIndex] = IconEngine::icon(type /*icon*/, qMin(w, h), wStyle());
    return itsBitmaps[toolWindow][typeIndex];
}

int QtCurveHandler::borderEdgeSize() const
{
    QtCurveHandler *that=(QtCurveHandler *)this;

    return outerBorder()
                ? (BorderTiny!=KDecoration::options()->preferredBorderSize(that) &&
                    wStyle()->pixelMetric((QStyle::PixelMetric)QtC_Round, NULL, NULL)<ROUND_FULL)
                    ? wStyle()->pixelMetric((QStyle::PixelMetric)QtC_TitleBarBorder, NULL, NULL)
                        ? 2
                        : 1
                    : 3
                : 1;
}

QList<QtCurveHandler::BorderSize> QtCurveHandler::borderSizes() const
{
    // the list must be sorted
    return QList<BorderSize>() << BorderTiny
                               << BorderNormal
                               << BorderLarge
                               << BorderVeryLarge
                               << BorderHuge
                               << BorderVeryHuge;
}

// make the handler accessible to other classes...
static QtCurveHandler *handler = 0;

QtCurveHandler * Handler()
{
    return handler;
}

}

extern "C"
{
    KDE_EXPORT KDecorationFactory *create_factory()
    {
        KWinQtCurve::handler = new KWinQtCurve::QtCurveHandler();
        return KWinQtCurve::handler;
    }
}
