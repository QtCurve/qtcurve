/*
  QtCurve KWin window decoration
  Copyright (C) 2007 Craig Drummond <Craig.Drummond@lycos.co.uk>

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
#include "qtcurvehandler.h"
#include "qtcurveclient.h"
#include "qtcurvebutton.h"
#include <QApplication>
#include <KConfig>
#include <KConfigGroup>
#include <KColorUtils>
#include <KColorScheme>
#include <KGlobalSettings>

namespace KWinQtCurve
{

QtCurveHandler::QtCurveHandler()
{
    memset(itsBitmaps, 0, sizeof(QBitmap*)*NumButtonIcons*2);

    // Create an instance of the new style...
    itsStyle = QStyleFactory::create("QtCurve");
    reset(0);
}

QtCurveHandler::~QtCurveHandler()
{
    for (int t=0; t < 2; ++t)
        for (int i=0; i < NumButtonIcons; ++i)
            delete itsBitmaps[t][i];
    delete itsStyle;
}

bool QtCurveHandler::reset(unsigned long changed)
{
    // we assume the active font to be the same as the inactive font since the control
    // center doesn't offer different settings anyways.
    itsTitleFont = KDecoration::options()->font(true, false); // not small
    itsTitleFontTool = KDecoration::options()->font(true, true); // small

    switch(KDecoration::options()->preferredBorderSize(this))
    {
        case BorderTiny:
            itsBorderSize = 3;
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

    // check if we are in reverse layout mode
    itsReverse = QApplication::isRightToLeft();

    // read in the configuration
    readConfig();

    for (int t=0; t < 2; ++t)
        for (int i=0; i < NumButtonIcons; i++)
            if (itsBitmaps[t][i])
            {
                delete itsBitmaps[t][i];
                itsBitmaps[t][i] = 0;
            }

    // Do we need to "hit the wooden hammer" ?
    bool needHardReset = true;
    // TODO: besides the Color and Font settings I can maybe handle more changes
    //       without a hard reset. I will do this later...
    if ((changed & ~(SettingColors | SettingFont | SettingButtons)) == 0)
        needHardReset = false;

    if (needHardReset)
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
        default:
            return false;
    };
}

void QtCurveHandler::readConfig()
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
}

const QBitmap & QtCurveHandler::buttonBitmap(ButtonIcon type, const QSize &size, bool toolWindow)
{
    int typeIndex(type),
        reduceW(size.width()>14 ? static_cast<int>(2*(size.width()/3.5)) : 6),
        reduceH(size.height()>14 ? static_cast<int>(2*(size.height()/3.5)) : 6),
        w(size.width() - reduceW),
        h(size.height() - reduceH);

    if (itsBitmaps[toolWindow][typeIndex] && itsBitmaps[toolWindow][typeIndex]->size()==QSize(w,h))
        return *itsBitmaps[toolWindow][typeIndex];

    // no matching pixmap found, create a new one...
    delete itsBitmaps[toolWindow][typeIndex];
    itsBitmaps[toolWindow][typeIndex] = 0;

    QBitmap bmp = IconEngine::icon(type /*icon*/, qMin(w, h), wStyle());
    QBitmap *bitmap = new QBitmap(bmp);
    itsBitmaps[toolWindow][typeIndex] = bitmap;
    return *bitmap;
}

QList<QtCurveHandler::BorderSize> QtCurveHandler::borderSizes() const
{
    // the list must be sorted
    return QList<BorderSize>() << BorderTiny
                               << BorderNormal
                               << BorderLarge
                               << BorderVeryLarge
                               << BorderHuge
                               << BorderVeryHuge
                               << BorderOversized;
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
