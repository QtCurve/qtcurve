#ifndef __QTCURVE_H__
#define __QTCURVE_H__

/*
  QtCurve (C) Craig Drummond, 2007 - 2009 craig_p_drummond@yahoo.co.uk

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

#include <QProgressBar>
#include <QTime>
#include <QPalette>
#include <QMap>
#include <QList>
#include <QSet>
#include <QCache>
#include <QColor>
#include <QStyleOption>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
#include <QFormLayout>
#endif
#include <Q_UINT64>
typedef qulonglong QtcKey;
#include "common.h"

#if !defined QTC_QT_ONLY
#include <KDE/KComponentData>
#endif

// #ifdef QTC_KSTYLE
// #include <kstyle.h>
// #define QTC_BASE_STYLE KStyle
// #else
#include <QWindowsStyle>
#define QTC_BASE_STYLE QWindowsStyle
// #endif

class QStyleOptionSlider;
class QLabel;
class QMenuBar;
class QScrollBar;

class QtCurveStyle : public QWindowsStyle
{
    Q_OBJECT
    Q_CLASSINFO("X-KDE-CustomElements", "true")

    public:

    enum Version
    {
        VER_UNKNOWN,
        VER_4x,  // <=4.4
        VER_45   // >=4.5
    };

    enum CustomElements
    {
        CE_QtC_KCapacityBar = CE_CustomBase+0xFFFF00
    };
    
    QtCurveStyle(const QString &name=QString());
    ~QtCurveStyle();

    void polish(QApplication *app);
    void polish(QPalette &palette);
    void polish(QWidget *widget);

#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
    void polishFormLayout(QFormLayout *layout);
    void polishLayout(QLayout *layout);
#endif

    void unpolish(QApplication *app) { QTC_BASE_STYLE::unpolish(app); }
    void unpolish(QWidget *widget);
    bool eventFilter(QObject *object, QEvent *event);
    void timerEvent(QTimerEvent *event);
    int pixelMetric(PixelMetric metric, const QStyleOption *option=0, const QWidget *widget=0) const;
    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData=0) const;
    QPalette standardPalette() const;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawControl(ControlElement control, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;
    void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text,
                      QPalette::ColorRole textRole = QPalette::NoRole) const;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const;
    QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const;
    QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const;
    SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                     const QPoint &pos, const QWidget *widget) const;

    private:

    static QColor shadowColor(const QColor &col)
    {
        return qGray(col.rgb()) < 100 ? QColor(255, 255, 255, 75) : QColor(0, 0, 0, 75);
    }

    void drawHighlight(QPainter *p, const QRect &r, bool horiz, bool inc) const;
    void drawFadedLine(QPainter *p, const QRect &r, const QColor &col, bool fadeStart, bool fadeEnd, bool horiz) const;
    void drawLines(QPainter *p, const QRect &r, bool horiz, int nLines, int offset, const QColor *cols, int startOffset,
                   int dark, ELine type) const;
    void drawProgressBevelGradient(QPainter *p, const QRect &origRect, const QStyleOption *option, bool horiz,
                                   EAppearance bevApp) const;
    void drawBevelGradient(const QColor &base, QPainter *p, QRect const &r, const QPainterPath &path,
                           bool horiz, bool sel, EAppearance bevApp, EWidget w=WIDGET_OTHER, bool useCache=true) const;
    void drawBevelGradientReal(const QColor &base, QPainter *p, const QRect &r, const QPainterPath &path,
                               bool horiz, bool sel, EAppearance bevApp, EWidget w) const;

    void drawBevelGradient(const QColor &base, QPainter *p, QRect const &r,
                           bool horiz, bool sel, EAppearance bevApp, EWidget w=WIDGET_OTHER, bool useCache=true) const
    {
        drawBevelGradient(base, p, r, QPainterPath(), horiz, sel, bevApp, w, useCache);
    }
    void drawBevelGradientReal(const QColor &base, QPainter *p, const QRect &r, bool horiz, bool sel,
                               EAppearance bevApp, EWidget w) const
    {
        drawBevelGradientReal(base, p, r, QPainterPath(), horiz, sel, bevApp, w);
    }
    
    void drawLightBevel(QPainter *p, const QRect &r, const QStyleOption *option, const QWidget *widget, int round, const QColor &fill,
                        const QColor *custom=0, bool doBorder=true, EWidget w=WIDGET_OTHER) const;
    void drawLightBevelReal(QPainter *p, const QRect &r, const QStyleOption *option, const QWidget *widget, int round, const QColor &fill,
                            const QColor *custom, bool doBorder, EWidget w, bool useCache, ERound realRound) const;
    void drawGlow(QPainter *p, const QRect &r, EWidget w) const;
    void drawEtch(QPainter *p, const QRect &r,  const QWidget *widget, EWidget w, bool raised=false) const;
    void drawWindowBackground(QWidget *widget) const;
    QPainterPath buildPath(const QRectF &r, EWidget w, int round, double radius) const;
    QPainterPath buildPath(const QRect &r, EWidget w, int round, double radius) const;
    void buildSplitPath(const QRect &r, EWidget w, int round, double radius, QPainterPath &tl, QPainterPath &br) const;
    void drawBorder(QPainter *p, const QRect &r, const QStyleOption *option, int round, const QColor *custom=0,
                    EWidget w=WIDGET_OTHER, EBorder borderProfile=BORDER_FLAT, bool doBlend=true, int borderVal=QT_STD_BORDER) const;
    void drawMdiControl(QPainter *p, const QStyleOptionTitleBar *titleBar, SubControl sc, const QWidget *widget,
                        ETitleBarButtons btn, const QColor &textColor, const QColor &shadow, const QColor *btnCols, const QColor *bgndCols) const;
    void drawMdiButton(QPainter *painter, const QRect &r, bool hover, bool sunken, const QColor *cols) const;
    void drawMdiIcon(QPainter *painter, const QColor &color, const QColor &shadow, const QColor *btnCols, const QRect &r,
                     bool hover, bool sunken, SubControl button) const;
    void drawWindowIcon(QPainter *painter, const QColor &color, const QRect &r, bool sunken, SubControl button) const;
    void drawEntryField(QPainter *p, const QRect &rx,  const QWidget *widget, const QStyleOption *option, int round,
                        bool fill, bool doEtch, EWidget w=WIDGET_ENTRY) const;
    void drawMenuItem(QPainter *p, const QRect &r, const QStyleOption *option, bool mbi, int round, const QColor *cols) const;
    void drawProgress(QPainter *p, const QRect &r, const QStyleOption *option, int round, bool vertical=false, bool reverse=false) const;
    void drawArrow(QPainter *p, const QRect &r, PrimitiveElement pe, QColor col, bool small=false, bool mdi=false) const;
    void drawSbSliderHandle(QPainter *p, const QRect &r, const QStyleOption *option, bool slider=false) const;
    void drawSliderHandle(QPainter *p, const QRect &r, const QStyleOptionSlider *option) const;
    void drawSliderGroove(QPainter *p, const QRect &groove, const QRect &handle, const QStyleOptionSlider *slider, const QWidget *widget) const;
    void drawMenuOrToolBarBackground(QPainter *p, const QRect &r, const QStyleOption *option, bool menu=true, bool horiz=true) const;
    void drawHandleMarkers(QPainter *p, const QRect &r, const QStyleOption *option, bool tb, ELine handles) const;
    void fillTab(QPainter *p, const QRect &r, const QStyleOption *option, const QColor &fill, bool horiz, EWidget tab, bool tabOnly) const;
    void colorTab(QPainter *p, const QRect &r, bool horiz, EWidget tab, int round) const;
    void shadeColors(const QColor &base, QColor *vals) const;
    const QColor * buttonColors(const QStyleOption *option) const;
    const QColor * sliderColors(const QStyleOption *option) const;
    const QColor * backgroundColors(const QColor &col) const;
    const QColor * backgroundColors(const QStyleOption *option) const
        { return backgroundColors(option->palette.background().color()); }
    const QColor * borderColors(const QStyleOption *option, const QColor *use) const;
    const QColor * getSidebarButtons() const;
    void setMenuColors(const QColor &bgnd);
    bool           coloredMdiButtons(bool active, bool mouseOver) const;
    const QColor * getMdiColors(const QStyleOption *option, bool active) const;
    void           readMdiPositions() const;
    const QColor & getFill(const QStyleOption *option, const QColor *use, bool cr=false, bool darker=false) const;
    const QColor & getTabFill(bool current, bool highlight, const QColor *use) const;
    const QColor & menuStripeCol() const;
    QPixmap *      getPixmap(const QColor col, EPixmap p, double shade=1.0) const;
    int            konqMenuBarSize(const QMenuBar *menu) const;
    Version        qtVersion() const;
    const QColor & checkRadioCol(const QStyleOption *opt) const;
    QColor         shade(const QColor &a, float k) const;
    void           shade(const color &ca, color *cb, double k) const;
    QColor         getLowerEtchCol(const QWidget *widget) const;

    private Q_SLOTS:

    void           widgetDestroyed(QObject *o);
    QIcon          standardIconImplementation(StandardPixmap pix, const QStyleOption *option=0, const QWidget *widget=0) const;
    int            layoutSpacingImplementation(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
                                               Qt::Orientation orientation, const QStyleOption *option,
                                               const QWidget *widget) const;
    void           kdeGlobalSettingsChange(int type, int);
    void           setupKde4();

    private:

#if !defined QTC_QT_ONLY
    void           setDecorationColors();
    void           applyKdeSettings(bool pal);
#endif

    private:

    mutable Options                    opts;
    QColor                             itsHighlightCols[TOTAL_SHADES+1],
                                       itsBackgroundCols[TOTAL_SHADES+1],
                                       itsMenubarCols[TOTAL_SHADES+1],
                                       itsFocusCols[TOTAL_SHADES+1],
                                       itsMouseOverCols[TOTAL_SHADES+1],
                                       *itsSliderCols,
                                       *itsDefBtnCols,
                                       *itsComboBtnCols,
                                       *itsSortedLvColors,
                                       itsButtonCols[TOTAL_SHADES+1],
                                       itsLighterPopupMenuBgndCol,
                                       itsCheckRadioCol;
    bool                               itsSaveMenuBarStatus;
    mutable QColor                     *itsSidebarButtonsCols;
    mutable QColor                     *itsActiveMdiColors;
    mutable QColor                     *itsMdiColors;
    mutable QColor                     itsActiveMdiTextColor;
    mutable QColor                     itsMdiTextColor;
    mutable QColor                     itsColoredButtonCols[TOTAL_SHADES+1];
    mutable QColor                     itsColoredBackgroundCols[TOTAL_SHADES+1];
    mutable QCache<QtcKey, QPixmap>    itsPixmapCache;
    mutable bool                       itsActive;
    mutable const QWidget              *itsSbWidget;
    mutable QLabel                     *itsClickedLabel;
    QSet<QProgressBar *>               itsProgressBars;
    int                                itsProgressBarAnimateTimer,
                                       itsAnimateStep;
    QTime                              itsTimer;
    mutable QMap<int, QColor *>        itsTitleBarButtonsCols;
    mutable QMap<QWidget *, QWidget *> itsReparentedDialogs;
    mutable QList<int>                 itsMdiButtons[2]; // 0=left, 1=right

    // Required for Q3Header hover...
    QPoint                             itsPos;
    QWidget                            *itsHoverWidget;
    mutable Version                    itsQtVersion;
    mutable QScrollBar                 *itsSViewSBar;
    mutable QMap<QWidget *, QSet<QWidget *> > itsSViewContainers;
#if !defined QTC_QT_ONLY
    KComponentData                     itsComponentData;
#endif
};

#endif
