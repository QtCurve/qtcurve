/*
  QtCurve KWin window decoration
  Copyright (C) 2007 - 2009 Craig Drummond <craig_p_drummond@yahoo.co.uk>

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
              : itsStyle(NULL)
{
    setStyle();

    memset(itsBitmaps, 0, sizeof(QBitmap*)*NumButtonIcons*2);
    reset(0);
}

QtCurveHandler::~QtCurveHandler()
{
    for (int t=0; t < 2; ++t)
        for (int i=0; i < NumButtonIcons; ++i)
            delete itsBitmaps[t][i];
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
        itsTimeStamp=getTimeStamp(xdgConfigFolder()+"/qtcurvestylerc");
    }
}

bool QtCurveHandler::reset(unsigned long changed)
{
    bool styleChanged=false;
    if(abs(itsTimeStamp-getTimeStamp(xdgConfigFolder()+"/qtcurvestylerc"))>2)
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

    switch(KDecoration::options()->preferredBorderSize(this))
    {
        case BorderTiny:
            itsBorderSize = 2;
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
            itsBorderSize =
// #if KDE_IS_VERSION(4,1,80)
//                 shadowsActive() ? 2 :
// #endif
                    4;
    }

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
    if (!styleChanged && (changed & ~(SettingColors | SettingFont | SettingButtons)) == 0)
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
#if KDE_IS_VERSION(4,1,80)
        case AbilityCompositingShadow:
            return itsColoredShadow;
#endif
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

    itsColoredShadow = config.readEntry("ColoredShadow", false);
    itsMenuClose = config.readEntry("CloseOnMenuDoubleClick", true);
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
    return QList<BorderSize>() << BorderNormal
                               << BorderLarge
                               << BorderVeryLarge
                               << BorderHuge
                               << BorderVeryHuge;
}

#if KDE_IS_VERSION(4,1,80)
// Shadows - Taken from Oxygen! rev873805
QList< QList<QImage> > QtCurveHandler::shadowTextures()
{
    QPalette palette = qApp->palette();

    // Set palette to the right group. Which is active right now while drawing the glow
    palette.setCurrentColorGroup(QPalette::Active);

    // TODO: THIS IS ALL VERY UGLY! Not recommended to do it this way.
    // Copied from the shadow effect's XRender picture generator

    // TODO: You can add fake anti-aliasing here :)

    QList< QList<QImage> > textureLists;
    QList<QImage> textures;

#define shadowFuzzyness 10
#define shadowSize 10

    //---------------------------------------------------------------
    // Active shadow texture

    QColor color = palette.window().color();
    QColor light = color.lighter(110);
    QColor dark = color;
    QColor glow = KColorScheme(QPalette::Active).decoration(KColorScheme::FocusColor).color();
    QColor glow2 = glow; // palette.color(QPalette::Active, QPalette::Highlight);

    qreal size = 25.5;
    QPixmap *shadow = new QPixmap( size*2, size*2 );
    shadow->fill( Qt::transparent );
    QRadialGradient rg( size, size, size );
    QColor c = color;
    c.setAlpha( 255 );  rg.setColorAt( 4.4/size, c );
    c = glow;
    c.setAlpha( 220 );  rg.setColorAt( 4.5/size, c );
    c.setAlpha( 180 );  rg.setColorAt( 5/size, c );
    c.setAlpha( 25 );  rg.setColorAt( 5.5/size, c );
    c.setAlpha( 0 );  rg.setColorAt( 6.5/size, c );
    QPainter p( shadow );
    p.setRenderHint( QPainter::Antialiasing );
    p.setPen( Qt::NoPen );
    p.setBrush( rg );
    p.drawRect( shadow->rect() );

    rg = QRadialGradient( size, size, size );
    c = color;
    c.setAlpha( 255 );  rg.setColorAt( 4.4/size, c );
    c = glow2;
    c.setAlpha( 0.65*255 );  rg.setColorAt( 4.5/size, c );
    c.setAlpha( 0.50*255 );  rg.setColorAt( 5.5/size, c );
    c.setAlpha( 0.38*255 );  rg.setColorAt( 6.5/size, c );
    c.setAlpha( 0.22*255 );  rg.setColorAt( 7.5/size, c );
    c.setAlpha( 0.15*255 );  rg.setColorAt( 8.5/size, c );
    c.setAlpha( 0.08*255 );  rg.setColorAt( 11.5/size, c );
    c.setAlpha( 0);  rg.setColorAt( 14.5/size, c );
    p.setRenderHint( QPainter::Antialiasing );
    p.setPen( Qt::NoPen );
    p.setBrush( rg );
    p.drawRect( shadow->rect() );

    // draw the corner of the window - actually all 4 corners as one circle
    p.setBrush( Qt::NoBrush );
    QLinearGradient lg = QLinearGradient(0.0, size-4.5, 0.0, size+4.5);
    lg.setColorAt(0.52, light);
    lg.setColorAt(1.0, dark);
    p.setPen(QPen(lg, 0.8));
    p.drawEllipse(QRectF(size-4, size-4, 8, 8));

    p.end();

    int w = shadow->width() / 2;
    int h = shadow->height() / 2;
    QPixmap dump;

#define MAKE_TEX( _W_, _H_, _XOFF_, _YOFF_ ) \
    dump = QPixmap( _W_, _H_ ); \
    dump.fill( Qt::transparent ); \
    p.begin( &dump ); \
    p.drawPixmap( 0, 0, *shadow, _XOFF_, _YOFF_, _W_, _H_ ); \
    p.end(); \
    textures.append( dump.toImage() );

    MAKE_TEX( w, h, 0, h+1 ); // corner
    MAKE_TEX( 1, h, w, h+1 );
    MAKE_TEX( w, h, w+1, h+1 );// corner
    MAKE_TEX( w, 1, 0, h );
    MAKE_TEX( w, 1, w+1, h );
    MAKE_TEX( w, h, 0, 0);// corner
    MAKE_TEX( 1, h, w, 0);
    MAKE_TEX( w, h, w+1, 0);// corner

    textureLists.append( textures );

    //---------------------------------------------------------------
    // Inactive shadow texture

    textures.clear();

    shadow->fill( Qt::transparent );
    p.begin(shadow);
    p.setRenderHint( QPainter::Antialiasing );
    p.setPen( Qt::NoPen );

    rg = QRadialGradient( size, size+4, size );
    c = QColor( Qt::black );
    c.setAlpha( 0.12*255 );  rg.setColorAt( 4.5/size, c );
    c.setAlpha( 0.11*255 );  rg.setColorAt( 6.6/size, c );
    c.setAlpha( 0.075*255 );  rg.setColorAt( 8.5/size, c );
    c.setAlpha( 0.06*255 );  rg.setColorAt( 11.5/size, c );
    c.setAlpha( 0.035*255 );  rg.setColorAt( 14.5/size, c );
    c.setAlpha( 0.025*255 );  rg.setColorAt( 17.5/size, c );
    c.setAlpha( 0.01*255 );  rg.setColorAt( 21.5/size, c );
    c.setAlpha( 0.0*255 );  rg.setColorAt( 25.5/size, c );
    p.setRenderHint( QPainter::Antialiasing );
    p.setPen( Qt::NoPen );
    p.setBrush( rg );
    p.drawRect( shadow->rect() );

    rg = QRadialGradient( size, size+2, size );
    c = QColor( Qt::black );
    c.setAlpha( 0.25*255 );  rg.setColorAt( 4.5/size, c );
    c.setAlpha( 0.20*255 );  rg.setColorAt( 5.5/size, c );
    c.setAlpha( 0.13*255 );  rg.setColorAt( 7.5/size, c );
    c.setAlpha( 0.06*255 );  rg.setColorAt( 8.5/size, c );
    c.setAlpha( 0.015*255 );  rg.setColorAt( 11.5/size, c );
    c.setAlpha( 0.0*255 );  rg.setColorAt( 14.5/size, c );
    p.setRenderHint( QPainter::Antialiasing );
    p.setPen( Qt::NoPen );
    p.setBrush( rg );
    p.drawRect( shadow->rect() );

//     rg = QRadialGradient( size, size+0.2, size );
//     c = color;
//     c = QColor( Qt::black );
//     c.setAlpha( 0.35*255 );  rg.setColorAt( 0/size, c );
//     c.setAlpha( 0.32*255 );  rg.setColorAt( 4.5/size, c );
//     c.setAlpha( 0.22*255 );  rg.setColorAt( 5.0/size, c );
//     c.setAlpha( 0.03*255 );  rg.setColorAt( 5.5/size, c );
//     c.setAlpha( 0.0*255 );  rg.setColorAt( 6.5/size, c );
//     p.setRenderHint( QPainter::Antialiasing );
//     p.setPen( Qt::NoPen );
//     p.setBrush( rg );
//     p.drawRect( shadow->rect() );

    rg = QRadialGradient( size, size, size );
    c = color;
    c.setAlpha( 255 );  rg.setColorAt( 4.0/size, c );
    c.setAlpha( 0 );  rg.setColorAt( 4.01/size, c );
    p.setRenderHint( QPainter::Antialiasing );
    p.setPen( Qt::NoPen );
    p.setBrush( rg );
    p.drawRect( shadow->rect() );

    // draw the corner of the window - actually all 4 corners as one circle
    p.setBrush( Qt::NoBrush );
    p.setPen(QPen(lg, 0.8));
    p.drawEllipse(QRectF(size-4, size-4, 8, 8));

    p.end();

    MAKE_TEX( w, h, 0, h+1 ); // corner
    MAKE_TEX( 1, h, w, h+1 );
    MAKE_TEX( w, h, w+1, h+1 );// corner
    MAKE_TEX( w, 1, 0, h );
    MAKE_TEX( w, 1, w+1, h );
    MAKE_TEX( w, h, 0, 0);// corner
    MAKE_TEX( 1, h, w, 0);
    MAKE_TEX( w, h, w+1, 0);// corner

    textureLists.append( textures );

    delete shadow;

    return textureLists;
}

int QtCurveHandler::shadowTextureList( ShadowType type ) const
{
    switch( type ) {
        case ShadowBorderedActive:
        case ShadowBorderlessActive:
            return 0;
        case ShadowBorderedInactive:
        case ShadowBorderlessInactive:
        case ShadowOther:
            return 1;
    }
    abort(); // Should never be reached
}

QList<QRect> QtCurveHandler::shadowQuads( ShadowType type, QSize size ) const
{
    int outside=20, underlap=5, cornersize=25;
    // These are underlap under the decoration so the corners look nicer 10px on the outside
    QList<QRect> quads;
    quads.append(QRect(-outside, size.height()-underlap, cornersize, cornersize));
    quads.append(QRect(underlap, size.height()-underlap, size.width()-2*underlap, cornersize));
    quads.append(QRect(size.width()-underlap, size.height()-underlap, cornersize, cornersize));
    quads.append(QRect(-outside, underlap, cornersize, size.height()-2*underlap));
    quads.append(QRect(size.width()-underlap, underlap, cornersize, size.height()-2*underlap));
    quads.append(QRect(-outside, -outside, cornersize, cornersize));
    quads.append(QRect(underlap, -outside, size.width()-2*underlap, cornersize));
    quads.append(QRect(size.width()-underlap,     -outside, cornersize, cornersize));
    return quads;
}

double QtCurveHandler::shadowOpacity( ShadowType type ) const
{
    return 1.0;
}
#endif

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
