/***************************************************************************
 *   Copyright (C) 2013~2013 by Yichao Yu                                  *
 *   yyc1992@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include "qtcurve_p.h"
#include "utils.h"
#include "debug.h"
#include "shortcuthandler.h"
#include "windowmanager.h"
#include "blurhelper.h"
#include <common/config_file.h>

#ifdef QTC_ENABLE_X11
#include "shadowhelper.h"
#include <qtcurve-utils/x11utils.h>
#include <sys/time.h>
#endif

#include <QToolButton>
#include <QAbstractItemView>
#include <QDialog>
#include <QSplitter>
#include <QMdiSubWindow>
#include <QMainWindow>
#include <QComboBox>
#include <QTreeView>
#include <QGroupBox>
#include <QListView>
#include <QCheckBox>
#include <QRadioButton>
#include <QTextEdit>
#include <QDial>
#include <QLabel>
#include <QStackedLayout>
#include <QMenuBar>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWizard>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QHeaderView>
#include <QLineEdit>
#include <QSpinBox>
#include <QDir>
#include <QSettings>
#include <QPixmapCache>
#include <QTextStream>

namespace QtCurve {

void
Style::polish(QApplication *app)
{
    // appName = getFile(app->arguments()[0]);

    if ("kwin" == appName) {
        theThemedApp = APP_KWIN;
    } else if ("systemsettings" == appName) {
        theThemedApp = APP_SYSTEMSETTINGS;
    } else if ("plasma" == appName || appName.startsWith("plasma-")) {
        theThemedApp = APP_PLASMA;
    } else if ("krunner" == appName || "krunner_lock" == appName ||
               "kscreenlocker" == appName) {
        theThemedApp = APP_KRUNNER;
    } else if ("konqueror" == appName) {
        theThemedApp = APP_KONQUEROR;
    } else if ("kontact" == appName) {
        theThemedApp = APP_KONTACT;
    } else if ("k3b" == appName) {
        theThemedApp = APP_K3B;
    } else if("skype" == appName) {
        theThemedApp = APP_SKYPE;
    } else if("arora" == appName) {
        theThemedApp = APP_ARORA;
    } else if("rekonq" == appName) {
        theThemedApp = APP_REKONQ;
    } else if("Designer" == QCoreApplication::applicationName()) {
        theThemedApp = APP_QTDESIGNER;
    } else if("QtCreator" == QCoreApplication::applicationName()) {
        theThemedApp = APP_QTCREATOR;
    } else if("kdevelop" == appName || "kdevelop.bin" == appName) {
        theThemedApp = APP_KDEVELOP;
    } else if("soffice.bin" == appName) {
        theThemedApp = APP_OPENOFFICE;
    } else if("kdmgreet" == appName) {
        opts.forceAlternateLvCols=false;
    } else if("konsole" == appName) {
        theThemedApp = APP_KONSOLE;
    } else if("Kde4ToolkitLibrary" == appName) {
        theThemedApp = APP_OPERA;
    }
    if(APP_REKONQ == theThemedApp)
        opts.statusbarHiding=0;
    if(opts.menubarHiding)
        itsSaveMenuBarStatus=opts.menubarApps.contains("kde") || opts.menubarApps.contains(appName);
    if(opts.statusbarHiding)
        itsSaveStatusBarStatus=opts.statusbarApps.contains("kde") || opts.statusbarApps.contains(appName);

    if(!IS_FLAT_BGND(opts.bgndAppearance) && opts.noBgndGradientApps.contains(appName))
        opts.bgndAppearance = APPEARANCE_FLAT;
    if(IMG_NONE!=opts.bgndImage.type && opts.noBgndImageApps.contains(appName))
        opts.bgndImage.type=IMG_NONE;
    if(SHADE_NONE!=opts.menuStripe && opts.noMenuStripeApps.contains(appName))
        opts.menuStripe=SHADE_NONE;

#ifdef QTC_QT5_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT
    // Plasma and Kate do not like the 'Fix parentless dialogs' option...
    if(opts.fixParentlessDialogs && (APP_PLASMA == theThemedApp || opts.noDlgFixApps.contains(appName) || opts.noDlgFixApps.contains("kde")))
        opts.fixParentlessDialogs=false;
#endif

    if((100!=opts.bgndOpacity || 100!=opts.dlgOpacity) && (opts.noBgndOpacityApps.contains(appName) || appName.endsWith(".kss")))
        opts.bgndOpacity=opts.dlgOpacity=100;
    if (100 != opts.menuBgndOpacity &&
        opts.noMenuBgndOpacityApps.contains(appName))
        opts.menuBgndOpacity = 100;

    if(APP_PLASMA == theThemedApp)
        opts.bgndOpacity=100;
    else if(APP_KWIN == theThemedApp)
        opts.bgndOpacity=opts.dlgOpacity=100, opts.bgndAppearance = APPEARANCE_FLAT;
    else if(APP_OPENOFFICE == theThemedApp)
    {
        opts.scrollbarType=SCROLLBAR_WINDOWS;
        if(APPEARANCE_FADE == opts.menuitemAppearance)
            opts.menuitemAppearance = APPEARANCE_FLAT;
        opts.borderMenuitems=opts.etchEntry=false;

        if(opts.useHighlightForMenu && blendOOMenuHighlight(QApplication::palette(), itsHighlightCols[ORIGINAL_SHADE]))
        {
            itsOOMenuCols=new QColor [TOTAL_SHADES+1];
            shadeColors(tint(popupMenuCols()[ORIGINAL_SHADE], itsHighlightCols[ORIGINAL_SHADE], 0.5), itsOOMenuCols);
        }
        opts.menubarHiding=opts.statusbarHiding=HIDE_NONE;
        opts.square|=SQUARE_POPUP_MENUS|SQUARE_TOOLTIPS;
        if(!IS_FLAT_BGND(opts.menuBgndAppearance) && 0 == opts.lighterPopupMenuBgnd)
            opts.lighterPopupMenuBgnd=1; // shade so that we dont have 3d-ish borders...
        opts.menuBgndAppearance = APPEARANCE_FLAT;
    }

#ifdef QTC_QT5_ENABLE_KDE
    if(opts.useQtFileDialogApps.contains(appName)) {
        qt_filedialog_existing_directory_hook=0L;
        qt_filedialog_open_filename_hook=0L;
        qt_filedialog_open_filenames_hook=0L;
        qt_filedialog_save_filename_hook=0L;
    }
#endif

    BASE_STYLE::polish(app);
    if(opts.hideShortcutUnderline)
        Utils::addEventFilter(app, itsShortcutHandler);
}

void Style::polish(QPalette &palette)
{
    int  contrast(QSettings(QLatin1String("Trolltech")).value("/Qt/KDE/contrast", DEFAULT_CONTRAST).toInt());
    bool newContrast(false);

    if(contrast<0 || contrast>10)
        contrast=DEFAULT_CONTRAST;

    if(contrast!=opts.contrast)
    {
        opts.contrast=contrast;
        newContrast=true;
    }

    bool newHighlight(newContrast ||
                      itsHighlightCols[ORIGINAL_SHADE]!=palette.color(QPalette::Active, QPalette::Highlight)),
        newGray(newContrast ||
                itsBackgroundCols[ORIGINAL_SHADE]!=palette.color(QPalette::Active, QPalette::Background)),
        newButton(newContrast ||
                  itsButtonCols[ORIGINAL_SHADE]!=palette.color(QPalette::Active, QPalette::Button)),
        newSlider(itsSliderCols && itsHighlightCols!=itsSliderCols && SHADE_BLEND_SELECTED == opts.shadeSliders &&
                  (newButton || newHighlight)),
        newDefBtn(itsDefBtnCols && (IND_COLORED!=opts.defBtnIndicator || SHADE_BLEND_SELECTED!=opts.shadeSliders) &&
                  IND_SELECTED!=opts.defBtnIndicator && IND_GLOW!=opts.defBtnIndicator &&
                  (newContrast || newButton || newHighlight)),
        newComboBtn(itsComboBtnCols && itsHighlightCols!=itsComboBtnCols && itsSliderCols!=itsComboBtnCols &&
                    SHADE_BLEND_SELECTED == opts.comboBtn &&
                    (newButton || newHighlight)),
        newSortedLv(itsSortedLvColors && ( (SHADE_BLEND_SELECTED == opts.sortedLv && itsDefBtnCols!=itsSortedLvColors &&
                                            itsSliderCols!=itsSortedLvColors && itsComboBtnCols!=itsSortedLvColors) ||
                                           SHADE_DARKEN == opts.sortedLv) &&
                    (newContrast || (opts.lvButton ? newButton : newGray))),
        newCheckRadioSelCols(itsCheckRadioSelCols && ( (SHADE_BLEND_SELECTED == opts.crColor && itsDefBtnCols!=itsCheckRadioSelCols &&
                                                        itsSliderCols!=itsCheckRadioSelCols && itsComboBtnCols!=itsCheckRadioSelCols &&
                                                        itsSortedLvColors!=itsCheckRadioSelCols) ||
                                                       SHADE_DARKEN == opts.crColor) &&
                             (newContrast || newButton)),
        newProgressCols(itsProgressCols && SHADE_BLEND_SELECTED == opts.progressColor &&
                        itsSliderCols!=itsProgressCols && itsComboBtnCols!=itsProgressCols &&
                        itsSortedLvColors!=itsProgressCols && itsCheckRadioSelCols!=itsProgressCols && (newContrast || newButton));

    if(newGray)
    {
        shadeColors(palette.color(QPalette::Active, QPalette::Background), itsBackgroundCols);
        if(IMG_PLAIN_RINGS == opts.bgndImage.type || IMG_BORDERED_RINGS == opts.bgndImage.type ||
           IMG_SQUARE_RINGS == opts.bgndImage.type ||
           IMG_PLAIN_RINGS == opts.menuBgndImage.type || IMG_BORDERED_RINGS == opts.menuBgndImage.type ||
           IMG_SQUARE_RINGS == opts.menuBgndImage.type)
        {
            qtcCalcRingAlphas(&itsBackgroundCols[ORIGINAL_SHADE]);
            if(itsUsePixmapCache)
                QPixmapCache::clear();
        }
    }

    if(newButton)
        shadeColors(palette.color(QPalette::Active, QPalette::Button), itsButtonCols);

    if(newHighlight)
        shadeColors(palette.color(QPalette::Active, QPalette::Highlight), itsHighlightCols);

// Dont set these here, they will be updated in setDecorationColors()...
//     shadeColors(QApplication::palette().color(QPalette::Active, QPalette::Highlight), itsFocusCols);
//     if(opts.coloredMouseOver)
//         shadeColors(QApplication::palette().color(QPalette::Active, QPalette::Highlight), itsMouseOverCols);

    setMenuColors(palette.color(QPalette::Active, QPalette::Background));

    if(newSlider)
        shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE], itsButtonCols[ORIGINAL_SHADE]), itsSliderCols);

    if(newDefBtn)
    {
        if(IND_TINT == opts.defBtnIndicator)
            shadeColors(tint(itsButtonCols[ORIGINAL_SHADE],
                             itsHighlightCols[ORIGINAL_SHADE], DEF_BNT_TINT), itsDefBtnCols);
        else if(IND_GLOW!=opts.defBtnIndicator)
            shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE],
                                 itsButtonCols[ORIGINAL_SHADE]), itsDefBtnCols);
    }

    if(newComboBtn)
        shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE], itsButtonCols[ORIGINAL_SHADE]), itsComboBtnCols);

    if(newSortedLv)
    {
        if(SHADE_BLEND_SELECTED == opts.sortedLv)
            shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE],
                                 opts.lvButton ? itsButtonCols[ORIGINAL_SHADE] : itsBackgroundCols[ORIGINAL_SHADE]), itsSortedLvColors);
        else
            shadeColors(shade(opts.lvButton ? itsButtonCols[ORIGINAL_SHADE] : itsBackgroundCols[ORIGINAL_SHADE], LV_HEADER_DARK_FACTOR),
                        itsSortedLvColors);
    }

    if(itsSidebarButtonsCols && SHADE_BLEND_SELECTED!=opts.shadeSliders &&
       IND_COLORED!=opts.defBtnIndicator)
        shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE],
                             itsButtonCols[ORIGINAL_SHADE]), itsSidebarButtonsCols);

    switch(opts.shadeCheckRadio)
    {
    default:
        itsCheckRadioCol=palette.color(QPalette::Active, opts.crButton ? QPalette::ButtonText : QPalette::Text);
        break;
    case SHADE_BLEND_SELECTED:
    case SHADE_SELECTED:
        itsCheckRadioCol=palette.color(QPalette::Active, QPalette::Highlight);
        break;
    case SHADE_CUSTOM:
        itsCheckRadioCol=opts.customCheckRadioColor;
    }

    if(newCheckRadioSelCols)
    {
        if(SHADE_BLEND_SELECTED == opts.crColor)
            shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE], itsButtonCols[ORIGINAL_SHADE]), itsCheckRadioSelCols);
        else
            shadeColors(shade(itsButtonCols[ORIGINAL_SHADE], LV_HEADER_DARK_FACTOR), itsCheckRadioSelCols);
    }

    if(newProgressCols)
        shadeColors(midColor(itsHighlightCols[ORIGINAL_SHADE], itsBackgroundCols[ORIGINAL_SHADE]), itsProgressCols);

    if(APP_OPENOFFICE == theThemedApp && opts.useHighlightForMenu && (newGray || newHighlight))
    {
        if(blendOOMenuHighlight(palette, itsHighlightCols[ORIGINAL_SHADE]))
        {
            if(!itsOOMenuCols)
                itsOOMenuCols=new QColor [TOTAL_SHADES+1];
            shadeColors(tint(popupMenuCols()[ORIGINAL_SHADE], itsHighlightCols[ORIGINAL_SHADE], 0.5), itsOOMenuCols);
        }
        else if(itsOOMenuCols)
        {
            delete [] itsOOMenuCols;
            itsOOMenuCols=0L;
        }
    }

    palette.setColor(QPalette::Active, QPalette::Light, itsBackgroundCols[0]);
    palette.setColor(QPalette::Active, QPalette::Dark, itsBackgroundCols[QTC_STD_BORDER]);
    palette.setColor(QPalette::Inactive, QPalette::Light, itsBackgroundCols[0]);
    palette.setColor(QPalette::Inactive, QPalette::Dark, itsBackgroundCols[QTC_STD_BORDER]);
    palette.setColor(QPalette::Inactive, QPalette::WindowText, palette.color(QPalette::Active, QPalette::WindowText));
    palette.setColor(QPalette::Disabled, QPalette::Light, itsBackgroundCols[0]);
    palette.setColor(QPalette::Disabled, QPalette::Dark, itsBackgroundCols[QTC_STD_BORDER]);

    palette.setColor(QPalette::Disabled, QPalette::Base, palette.color(QPalette::Active, QPalette::Background));
    palette.setColor(QPalette::Disabled, QPalette::Background, palette.color(QPalette::Active, QPalette::Background));

    // Fix KDE4's palette...
    if(palette.color(QPalette::Active, QPalette::Highlight)!=palette.color(QPalette::Inactive, QPalette::Highlight))
        itsInactiveChangeSelectionColor=true;

    for(int i=QPalette::WindowText; i<QPalette::NColorRoles; ++i)
        //if(i!=QPalette::Highlight && i!=QPalette::HighlightedText)
        palette.setColor(QPalette::Inactive, (QPalette::ColorRole)i, palette.color(QPalette::Active, (QPalette::ColorRole)i));

    // Force this to be re-generated!
    if(SHADE_BLEND_SELECTED == opts.menuStripe)
        opts.customMenuStripeColor=Qt::black;
#ifdef QTC_QT5_ENABLE_KDE
    // Only set palette here...
    if(kapp)
        setDecorationColors();
#endif
}

void Style::polish(QWidget *widget)
{
    if (!widget)
        return;

    bool enableMouseOver(opts.highlightFactor || opts.coloredMouseOver);

    // 'Fix' konqueror's large menubar...
    if (APP_KONQUEROR == theThemedApp && widget->parentWidget() &&
        qobject_cast<QToolButton*>(widget) &&
        qobject_cast<QMenuBar*>(widget->parentWidget()))
        widget->parentWidget()->setMaximumSize(
            32768, konqMenuBarSize((QMenuBar*)widget->parentWidget()));

    if (EFFECT_NONE != opts.buttonEffect &&
        !USE_CUSTOM_ALPHAS(opts) && isNoEtchWidget(widget)) {
        theNoEtchWidgets.insert(static_cast<const QWidget*>(widget));
        connect(widget, &QWidget::destroyed,
                this, &Style::widgetDestroyed);
    }

    itsWindowManager->registerWidget(widget);
#ifdef QTC_ENABLE_X11
    itsShadowHelper->registerWidget(widget);
#endif

    // Need to register all widgets to blur helper, in order to have proper
    // blur_behind region set have proper regions removed for opaque widgets.
    // Note: that the helper does nothing as long as compositing and ARGB are
    // not enabled
    if ((100 != opts.menuBgndOpacity && qobject_cast<QMenu*>(widget)) ||
        (100 != opts.bgndOpacity &&
         (!widget->window() || Qt::Dialog != (widget->window()->windowFlags() &
                                              Qt::WindowType_Mask))) ||
        (100 != opts.dlgOpacity &&
         (!widget->window() || Qt::Dialog == (widget->window()->windowFlags() &
                                              Qt::WindowType_Mask)))) {
        itsBlurHelper->registerWidget(widget);
    }

    // Sometimes get background errors with QToolBox (e.g. in Bespin config), and setting WA_StyledBackground seems to
    // fix this,..
    if (CUSTOM_BGND || FRAME_SHADED == opts.groupBox ||
        FRAME_FADED == opts.groupBox) {
        switch (widget->windowFlags() & Qt::WindowType_Mask) {
        case Qt::Window:
        case Qt::Dialog: {
            // For non-transparent widgets, only need to set WA_StyledBackground - and PE_Widget will be called to
            // render background...
            widget->setAttribute(Qt::WA_StyledBackground);

            // Hack: stop here if application is of type Plasma
            /*
              Right now we need to reject window candidates if the application
              is of type plasma because it conflicts with some widgets embedded
              into the SysTray. Ideally one would rather find a "generic" reason,
              not to handle them
            */
            if (APP_PLASMA == theThemedApp && !widget->inherits("QDialog"))
                break;

#ifdef QTC_ENABLE_X11
            Utils::addEventFilter(widget, this);
#endif
            int opacity = (Qt::Dialog == (widget->windowFlags() &
                                         Qt::WindowType_Mask) ?
                           opts.dlgOpacity : opts.bgndOpacity);

#ifdef QTC_ENABLE_X11
            if (APP_KONSOLE == theThemedApp && 100 != opacity &&
                widget->testAttribute(Qt::WA_TranslucentBackground) &&
                widget->inherits("Konsole::MainWindow")) {
                // Background translucency does not work for konsole :-(
                // So, just set titlebar opacity...
                setOpacityProp(widget, (unsigned short)opacity);
                break;
            } else
#endif
                if (100 == opacity || !widget->isWindow() ||
                    Qt::Desktop == widget->windowType() ||
                    widget->testAttribute(Qt::WA_X11NetWmWindowTypeDesktop) ||
                    widget->testAttribute(Qt::WA_TranslucentBackground) ||
                    widget->testAttribute(Qt::WA_NoSystemBackground) ||
                    widget->testAttribute(Qt::WA_PaintOnScreen) ||
                    widget->inherits("KScreenSaver") ||
                    widget->inherits("QTipLabel") ||
                    widget->inherits("QSplashScreen") ||
                    widget->windowFlags().testFlag(Qt::FramelessWindowHint) ||
                    !(widget->testAttribute(Qt::WA_WState_Created) ||
                      widget->internalWinId())) {
                    break;
                }

            // whenever you set the translucency flag, Qt will create a new
            // widget under the hood, replacing the old
            // ...unfortunately some properties are lost,
            // among them the window icon.
            QIcon icon(widget->windowIcon());

            setTranslucentBackground(widget);
            widget->setWindowIcon(icon);
            // WORKAROUND: somehow the window gets repositioned to <1, <1 and
            // thus always appears in the upper left corner
            // we just move it faaaaar away so kwin will take back control
            // and apply smart placement or whatever
            if (!widget->isVisible()) {
                QWidget *pw = (Qt::Dialog == (widget->windowFlags() &
                                             Qt::WindowType_Mask) ?
                               widget->parentWidget() ?
                               widget->parentWidget()->window() :
                               QApplication::activeWindow() : 0L);

                if (pw && pw != widget) {
                    widget->adjustSize();
                    widget->move(pw->pos() +
                                 QPoint((pw->size().width() -
                                         widget->size().width()) / 2,
                                        (pw->size().height() -
                                         widget->size().height()) / 2));
                } else {
                    widget->move(900000, 900000);
                }
            }

            // PE_Widget is not called for transparent widgets,
            // so need event filter here...
            Utils::addEventFilter(widget, this);
            itsTransparentWidgets.insert(widget);
            connect(widget, &QWidget::destroyed,
                    this, &Style::widgetDestroyed);
            break;
        }
        case Qt::Popup:
            // we currently don't want that kind of gradient on menus etc
        case Qt::Tool:
            // this we exclude as it is used for dragging of icons etc
        default:
            break;
        }
        if (qobject_cast<QSlider*>(widget))
            widget->setBackgroundRole(QPalette::NoRole);

        if (widget->autoFillBackground() && widget->parentWidget() &&
            "qt_scrollarea_viewport"==widget->parentWidget()->objectName() &&
            widget->parentWidget()->parentWidget() && //grampa
            qobject_cast<QAbstractScrollArea*>(widget->parentWidget()
                                               ->parentWidget()) &&
            widget->parentWidget()->parentWidget()->parentWidget() && // grangrampa
            widget->parentWidget()->parentWidget()->parentWidget()->inherits("QToolBox")) {
            widget->parentWidget()->setAutoFillBackground(false);
            widget->setAutoFillBackground(false);
        }
    }

    if (itsIsPreview && qobject_cast<QMdiSubWindow*>(widget))
        widget->setAttribute(Qt::WA_StyledBackground);

    if (opts.menubarHiding && qobject_cast<QMainWindow *>(widget) &&
        static_cast<QMainWindow *>(widget)->menuWidget()) {
        Utils::addEventFilter(widget, this);
        if(itsSaveMenuBarStatus)
            Utils::addEventFilter(
                static_cast<QMainWindow*>(widget)->menuWidget(), this);
        if (itsSaveMenuBarStatus && qtcMenuBarHidden(appName)) {
            static_cast<QMainWindow*>(widget)->menuWidget()->setHidden(true);
#ifdef QTC_ENABLE_X11
            if (BLEND_TITLEBAR || opts.menubarHiding & HIDE_KWIN ||
                opts.windowBorder & WINDOW_BORDER_USE_MENUBAR_COLOR_FOR_TITLEBAR)
                emitMenuSize(static_cast<QMainWindow*>(widget)->menuWidget(), 0);
#endif
        }
    }

    if(opts.statusbarHiding && qobject_cast<QMainWindow *>(widget))
    {
        QList<QStatusBar *> sb=getStatusBars(widget);

        if(sb.count())
        {
            Utils::addEventFilter(widget, this);
            QList<QStatusBar *>::ConstIterator it(sb.begin()),
                end(sb.end());
            for(; it!=end; ++it)
            {
                if(itsSaveStatusBarStatus)
                    Utils::addEventFilter(*it, this);
                if(itsSaveStatusBarStatus && qtcStatusBarHidden(appName))
                    (*it)->setHidden(true);
            }
#ifdef QTC_ENABLE_X11
            setSbProp(widget);
            emitStatusBarState(sb.first());
#endif
        }
    }

    // Enable hover effects in all itemviews
    if (QAbstractItemView *itemView = qobject_cast<QAbstractItemView*>(widget))
    {
        QWidget *viewport=itemView->viewport();
        viewport->setAttribute(Qt::WA_Hover);

        if(opts.forceAlternateLvCols &&
           viewport->autoFillBackground() && // Dolphins Folders panel
           //255==viewport->palette().color(itemView->viewport()->backgroundRole()).alpha() && // KFilePlacesView
           !widget->inherits("KFilePlacesView") &&
           // Exclude non-editable combo popup...
           !(opts.gtkComboMenus &&
             widget->inherits("QComboBoxListView") &&
             widget->parentWidget() &&
             qtcCheckType0<QComboBox>(widget->parentWidget()->parentWidget()) &&
             !static_cast<QComboBox*>(widget->parentWidget()
                                      ->parentWidget())->isEditable()) &&
           // Exclude KAboutDialog...
           !qtcCheckKDEType0(getParent(widget, 5), KAboutApplicationDialog) &&
           (qobject_cast<QTreeView*>(widget) ||
            (qobject_cast<QListView*>(widget) &&
             QListView::IconMode != ((QListView*)widget)->viewMode())))
            itemView->setAlternatingRowColors(true);
    }

    if(APP_KONTACT==theThemedApp && qobject_cast<QToolButton *>(widget))
        ((QToolButton *)widget)->setAutoRaise(true);

    if(enableMouseOver &&
       (qobject_cast<QPushButton *>(widget) ||
        qobject_cast<QAbstractButton*>(widget) ||
        qobject_cast<QComboBox *>(widget) ||
        qobject_cast<QAbstractSpinBox *>(widget) ||
        qobject_cast<QCheckBox *>(widget) ||
        qobject_cast<QGroupBox *>(widget) ||
        qobject_cast<QRadioButton *>(widget) ||
        qobject_cast<QSplitterHandle *>(widget) ||
        qobject_cast<QSlider *>(widget) ||
        qobject_cast<QHeaderView *>(widget) ||
        qobject_cast<QTabBar *>(widget) ||
        qobject_cast<QAbstractScrollArea *>(widget) ||
        qobject_cast<QTextEdit *>(widget) ||
        qobject_cast<QLineEdit *>(widget) ||
        qobject_cast<QDial *>(widget) ||
//        qobject_cast<QDockWidget *>(widget) ||
        widget->inherits("QWorkspaceTitleBar") ||
        widget->inherits("QDockSeparator") ||
        widget->inherits("QDockWidgetSeparator")))
        widget->setAttribute(Qt::WA_Hover, true);

    if (qobject_cast<QSplitterHandle *>(widget))
        widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
    else if (qobject_cast<QScrollBar *>(widget))
    {
        if(enableMouseOver)
            widget->setAttribute(Qt::WA_Hover, true);
        widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
        if(!opts.gtkScrollViews)
            Utils::addEventFilter(widget, this);
    }
    else if (qobject_cast<QAbstractScrollArea *>(widget) && widget->inherits("KFilePlacesView"))
    {
        if(CUSTOM_BGND)
            polishScrollArea(static_cast<QAbstractScrollArea *>(widget), true);
        Utils::addEventFilter(widget, this);
    }
    else if (qobject_cast<QProgressBar *>(widget))
    {
        if(widget->palette().color(QPalette::Inactive, QPalette::HighlightedText)!=widget->palette().color(QPalette::Active, QPalette::HighlightedText))
        {
            QPalette pal(widget->palette());
            pal.setColor(QPalette::Inactive, QPalette::HighlightedText, pal.color(QPalette::Active, QPalette::HighlightedText));
            widget->setPalette(pal);
        }

        if(opts.boldProgress)
            setBold(widget);
        Utils::addEventFilter(widget, this);
    } else if(qobject_cast<QMenuBar*>(widget)) {
#ifdef QTC_ENABLE_X11
        if (BLEND_TITLEBAR || opts.menubarHiding&HIDE_KWIN ||
            opts.windowBorder&WINDOW_BORDER_USE_MENUBAR_COLOR_FOR_TITLEBAR)
            emitMenuSize(widget, PREVIEW_MDI==itsIsPreview ||
                         !widget->isVisible() ? 0 : widget->rect().height());
#endif
        if (CUSTOM_BGND)
            widget->setBackgroundRole(QPalette::NoRole);

        widget->setAttribute(Qt::WA_Hover, true);

//         if(opts.shadeMenubarOnlyWhenActive && SHADE_NONE!=opts.shadeMenubars)
        Utils::addEventFilter(widget, this);

        setMenuTextColors(widget, true);
    } else if(qobject_cast<QLabel*>(widget)) {
        Utils::addEventFilter(widget, this);
        if (WM_DRAG_ALL == opts.windowDrag &&
            ((QLabel*)widget)->textInteractionFlags()
            .testFlag(Qt::TextSelectableByMouse) &&
            qtcCheckType0<QFrame>(widget->parentWidget()) &&
            qtcCheckKDEType0(widget->parentWidget()->parentWidget(),
                             KTitleWidget))
            ((QLabel*)widget)->setTextInteractionFlags(
                ((QLabel*)widget)->textInteractionFlags() &
                ~Qt::TextSelectableByMouse);
    } else if(/*!opts.gtkScrollViews && */qobject_cast<QAbstractScrollArea*>(widget)) {
        if(CUSTOM_BGND)
            polishScrollArea(static_cast<QAbstractScrollArea *>(widget));
        if(!opts.gtkScrollViews && (((QFrame *)widget)->frameWidth()>0))
            Utils::addEventFilter(widget, this);
        if(APP_KONTACT==theThemedApp && widget->parentWidget())
        {
            QWidget *frame=scrollViewFrame(widget->parentWidget());

            if(frame)
            {
                Utils::addEventFilter(frame, this);
                itsSViewContainers[frame].insert(widget);
                connect(widget, &QWidget::destroyed,
                        this, &Style::widgetDestroyed);
                connect(frame, &QWidget::destroyed,
                        this, &Style::widgetDestroyed);
            }
        }
    } else if(qobject_cast<QDialog*>(widget) &&
              widget->inherits("QPrintPropertiesDialog") &&
              widget->parentWidget() && widget->parentWidget()->window() &&
              widget->window() && widget->window()->windowTitle().isEmpty() &&
              !widget->parentWidget()->window()->windowTitle().isEmpty()) {
        widget->window()->setWindowTitle(widget->parentWidget()->window()
                                         ->windowTitle());
    } else if(widget->inherits("QWhatsThat")) {
        QPalette pal(widget->palette());
        QColor   shadow(pal.shadow().color());

        shadow.setAlpha(32);
        pal.setColor(QPalette::Shadow, shadow);
        widget->setPalette(pal);
        widget->setMask(QRegion(widget->rect().adjusted(0, 0, -6, -6))+QRegion(widget->rect().adjusted(6, 6, 0, 0)));
    }
    else if(qobject_cast<QDockWidget *>(widget) &&
            widget->parentWidget() &&
            widget->parentWidget()->parentWidget() &&
            widget->parentWidget()->parentWidget()->parentWidget() &&
            qobject_cast<QSplitter *>(widget->parentWidget()) &&
            widget->parentWidget()->parentWidget()->inherits("KFileWidget") /*&&
                                                                              widget->parentWidget()->parentWidget()->parentWidget()->inherits("KFileDialog")*/)
        ((QDockWidget*)widget)->setTitleBarWidget(new QtCurveDockWidgetTitleBar(widget));
#ifdef QTC_QT5_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT
    else if(opts.fixParentlessDialogs && qobject_cast<QDialog*>(widget) &&
            widget->windowFlags()&Qt::WindowType_Mask &&
            (!widget->parentWidget()) /*|| widget->parentWidget()->isHidden())*/)
    {
        QWidget *activeWindow=getActiveWindow(widget);

        if(activeWindow)
        {
            itsReparentedDialogs[widget]=widget->parentWidget();
            widget->setParent(activeWindow, widget->windowFlags());
        }
        Utils::addEventFilter(widget, this);
    }
#endif
    else if((!IS_FLAT_BGND(opts.menuBgndAppearance) || 100!=opts.menuBgndOpacity || !(opts.square&SQUARE_POPUP_MENUS)) &&
            widget->inherits("QComboBoxPrivateContainer") && !widget->testAttribute(Qt::WA_TranslucentBackground))
        setTranslucentBackground(widget);

    if (widget->inherits("QTipLabel") && !IS_FLAT(opts.tooltipAppearance) &&
        APP_OPERA != theThemedApp) {
        widget->setBackgroundRole(QPalette::NoRole);
        setTranslucentBackground(widget);
    }

    if (!widget->isWindow())
        if (QFrame *frame = qobject_cast<QFrame *>(widget))
        {
            // kill ugly frames...
            if (QFrame::Box==frame->frameShape() || QFrame::Panel==frame->frameShape() || QFrame::WinPanel==frame->frameShape())
                frame->setFrameShape(QFrame::StyledPanel);
            //else if (QFrame::HLine==frame->frameShape() || QFrame::VLine==frame->frameShape())
            Utils::addEventFilter(widget, this);

            if (qtcCheckKDEType0(widget->parent(), KTitleWidget)) {
                if (CUSTOM_BGND) {
                    frame->setAutoFillBackground(false);
                } else {
                    frame->setBackgroundRole(QPalette::Window);
                }
                QLayout *layout(frame->layout());
                if (layout) {
                    layout->setMargin(0);
                }
            }

            QWidget *p=0L;

            if(opts.gtkComboMenus && widget->parentWidget() && (p=widget->parentWidget()->parentWidget()) &&
               qobject_cast<QComboBox *>(p) && !((QComboBox *)(p))->isEditable())
            {
                QPalette pal(widget->palette());
                QColor   col(popupMenuCols()[ORIGINAL_SHADE]);

                if (!IS_FLAT_BGND(opts.menuBgndAppearance) ||
                   100 != opts.menuBgndOpacity ||
                    !(opts.square & SQUARE_POPUP_MENUS))
                    col.setAlphaF(0);

                pal.setBrush(QPalette::Active, QPalette::Base, col);
                pal.setBrush(QPalette::Active, QPalette::Window, col);
                widget->setPalette(pal);
                if(opts.shadePopupMenu)
                    setMenuTextColors(widget, false);
            }
        }

    if(qobject_cast<QMenu *>(widget)/* && !(widget->parentWidget() &&
                                       #ifndef QTC_QT5_ENABLE_KDE
                                       widget->inherits("KMenu") && widget->parentWidget()->inherits("KXmlGuiWindow")
                                       #else
                                       qobject_cast<KMenu *>(widget) && qobject_cast<KXmlGuiWindow *>(widget->parentWidget())
                                       #endif
                                       && QLatin1String("QtCurvePreview")==widget->parentWidget()->objectName())*/)
    {
        if(!IS_FLAT_BGND(opts.menuBgndAppearance) || 100!=opts.menuBgndOpacity || !(opts.square&SQUARE_POPUP_MENUS))
        {
            Utils::addEventFilter(widget, this);
            if((100!=opts.menuBgndOpacity || !(opts.square&SQUARE_POPUP_MENUS)) && !widget->testAttribute(Qt::WA_TranslucentBackground))
                setTranslucentBackground(widget);
        }
        if(USE_LIGHTER_POPUP_MENU || opts.shadePopupMenu)
        {
            QPalette pal(widget->palette());

            pal.setBrush(QPalette::Active, QPalette::Window, popupMenuCols()[ORIGINAL_SHADE]);
            widget->setPalette(pal);
            if(opts.shadePopupMenu)
                setMenuTextColors(widget, false);
            if(IMG_NONE!=opts.menuBgndImage.type)
                Utils::addEventFilter(widget, this);
        }
    }

    if((!IS_FLAT_BGND(opts.menuBgndAppearance) || 100!=opts.menuBgndOpacity || !(opts.square&SQUARE_POPUP_MENUS)) &&
       widget->inherits("QComboBoxPrivateContainer"))
    {
        Utils::addEventFilter(widget, this);
        if((100!=opts.menuBgndOpacity || !(opts.square&SQUARE_POPUP_MENUS)) && !widget->testAttribute(Qt::WA_TranslucentBackground))
            setTranslucentBackground(widget);
    }

    bool parentIsToolbar(false);

    // Using dark menubars - konqueror's combo box texts get messed up. Seems to be when a plain QWidget has widget->setBackgroundRole(QPalette::Window);
    // and widget->setAutoFillBackground(false); set (below). These onyl happen if 'parentIsToolbar' - so dont bather detecting this if the widget
    // is a plain QWidget
    //
    // QWidget QComboBoxListView QComboBoxPrivateContainer SearchBarCombo KToolBar KonqMainWindow
    // QWidget KCompletionBox KLineEdit SearchBarCombo KToolBar KonqMainWindow
    if(strcmp(widget->metaObject()->className(), "QWidget"))
    {
        QWidget *wid=widget ? widget->parentWidget() : 0L;

        while(wid && !parentIsToolbar) {
            parentIsToolbar=qobject_cast<QToolBar*>(wid);
            wid=wid->parentWidget();
        }
    }

    if(APP_QTCREATOR==theThemedApp && qobject_cast<QMainWindow *>(widget) && static_cast<QMainWindow *>(widget)->menuWidget())
        static_cast<QMainWindow *>(widget)->menuWidget()->setStyle(this);

    if (APP_QTCREATOR == theThemedApp && qobject_cast<QDialog*>(widget) &&
        qtcCheckKDEType(widget, KFileDialog)) {
        QToolBar *tb=getToolBarChild(widget);

        if(tb)
        {
            int size = pixelMetric(PM_ToolBarIconSize);
            tb->setIconSize(QSize(size, size));
            tb->setMinimumSize(QSize(size+14, size+14));
            setStyleRecursive(tb, this, size+4);
        }
    }

    if(parentIsToolbar && (qobject_cast<QComboBox *>(widget) ||
                           qobject_cast<QLineEdit *>(widget)))
        widget->setFont(QApplication::font());

    if (qobject_cast<QMenuBar *>(widget) || qobject_cast<QToolBar*>(widget) ||
        parentIsToolbar)
        widget->setBackgroundRole(QPalette::Window);

    if(!IS_FLAT(opts.toolbarAppearance) && parentIsToolbar)
        widget->setAutoFillBackground(false);

    if(APP_SYSTEMSETTINGS==theThemedApp &&
       widget && widget->parentWidget() && widget->parentWidget()->parentWidget() &&
       qobject_cast<QFrame *>(widget) && QFrame::NoFrame!=((QFrame *)widget)->frameShape() &&
       qobject_cast<QFrame *>(widget->parentWidget()) &&
       qobject_cast<QTabWidget *>(widget->parentWidget()->parentWidget()))
        ((QFrame *)widget)->setFrameShape(QFrame::NoFrame);

    if (QLayout *layout = widget->layout()) {
        // explicitely check public layout classes,
        // QMainWindowLayout doesn't work here
        if (qobject_cast<QBoxLayout*>(layout)
            || qobject_cast<QFormLayout*>(layout)
            || qobject_cast<QGridLayout*>(layout)
            || qobject_cast<QStackedLayout*>(layout)) {
            polishLayout(layout);
        }
    }

    if( (APP_K3B==theThemedApp && widget->inherits("K3b::ThemedHeader") && qobject_cast<QFrame *>(widget)) ||
        widget->inherits("KColorPatch"))
    {
        ((QFrame *)widget)->setLineWidth(0);
        ((QFrame *)widget)->setFrameShape(QFrame::NoFrame);
    }

    if(APP_KDEVELOP==theThemedApp && !opts.stdSidebarButtons && widget->inherits("Sublime::IdealButtonBarWidget") && widget->layout())
    {
        widget->layout()->setSpacing(0);
        widget->layout()->setMargin(0);
    }

#ifdef QTC_ENABLE_X11
    QWidget *window=widget->window();

    if ((100!=opts.bgndOpacity &&
         Qt::Window==(window->windowFlags() & Qt::WindowType_Mask)) ||
        (100!=opts.dlgOpacity &&
         Qt::Dialog==(window->windowFlags() & Qt::WindowType_Mask))) {
        widget->removeEventFilter(this);
        Utils::addEventFilter(widget, this);

        if (widget->inherits("KFilePlacesView")) {
            widget->setAutoFillBackground(false);
            widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
        }
    }
#endif

#ifdef QTC_QT5_ENABLE_KDE
    // Make file selection button in QPrintDialog appear more KUrlRequester like...
    if (qobject_cast<QToolButton*>(widget) &&
        widget->parentWidget() && widget->parentWidget()->parentWidget() && widget->parentWidget()->parentWidget()->parentWidget() &&
       qobject_cast<QGroupBox*>(widget->parentWidget()) &&
       qobject_cast<QPrintDialog*>(widget->parentWidget()->parentWidget()->parentWidget()) &&
       static_cast<QToolButton*>(widget)->text() == QLatin1String("...")) {
        static_cast<QToolButton*>(widget)->setIcon(KIcon("document-open"));
        static_cast<QToolButton*>(widget)->setAutoRaise(false);
    }
#endif
}

void Style::unpolish(QApplication *app)
{
    if (opts.hideShortcutUnderline)
        app->removeEventFilter(itsShortcutHandler);
    BASE_STYLE::unpolish(app);
}

void Style::unpolish(QWidget *widget)
{
    if (!widget)
        return;

    if (EFFECT_NONE != opts.buttonEffect && theNoEtchWidgets.contains(widget)) {
        theNoEtchWidgets.remove(static_cast<const QWidget*>(widget));
        disconnect(widget, &QWidget::destroyed,
                   this, &Style::widgetDestroyed);
    }

    itsWindowManager->unregisterWidget(widget);
#ifdef QTC_ENABLE_X11
    itsShadowHelper->unregisterWidget(widget);
#endif
    itsBlurHelper->unregisterWidget(widget);
    unregisterArgbWidget(widget);

    // Sometimes get background errors with QToolBox (e.g. in Bespin config), and setting WA_StyledBackground seems to
    // fix this,..
    if(CUSTOM_BGND || FRAME_SHADED==opts.groupBox || FRAME_FADED==opts.groupBox)
    {
        switch (widget->windowFlags() & Qt::WindowType_Mask)
        {
        case Qt::Window:
        case Qt::Dialog:
            widget->removeEventFilter(this);
            widget->setAttribute(Qt::WA_StyledBackground, false);
            break;
        case Qt::Popup: // we currently don't want that kind of gradient on menus etc
        case Qt::Tool: // this we exclude as it is used for dragging of icons etc
        default:
            break;
        }

        if(qobject_cast<QSlider *>(widget))
            widget->setBackgroundRole(QPalette::Window);
    }

    if(itsIsPreview && qobject_cast<QMdiSubWindow *>(widget))
        widget->setAttribute(Qt::WA_StyledBackground, false);

    if(opts.menubarHiding && qobject_cast<QMainWindow *>(widget) && static_cast<QMainWindow *>(widget)->menuWidget())
    {
        widget->removeEventFilter(this);
        if(itsSaveMenuBarStatus)
            static_cast<QMainWindow *>(widget)->menuWidget()->removeEventFilter(this);
    }

    if(opts.statusbarHiding && qobject_cast<QMainWindow *>(widget))
    {
        QList<QStatusBar *> sb=getStatusBars(widget);

        if(sb.count())
        {
            widget->removeEventFilter(this);
            if(itsSaveStatusBarStatus)
            {
                QList<QStatusBar *>::ConstIterator it(sb.begin()),
                    end(sb.end());
                for(; it!=end; ++it)
                    (*it)->removeEventFilter(this);
            }
        }
    }

    if(qobject_cast<QPushButton *>(widget) ||
       qobject_cast<QComboBox *>(widget) ||
       qobject_cast<QAbstractSpinBox *>(widget) ||
       qobject_cast<QCheckBox *>(widget) ||
       qobject_cast<QGroupBox *>(widget) ||
       qobject_cast<QRadioButton *>(widget) ||
       qobject_cast<QSplitterHandle *>(widget) ||
       qobject_cast<QSlider *>(widget) ||
       qobject_cast<QHeaderView *>(widget) ||
       qobject_cast<QTabBar *>(widget) ||
       qobject_cast<QAbstractScrollArea *>(widget) ||
       qobject_cast<QTextEdit *>(widget) ||
       qobject_cast<QLineEdit *>(widget) ||
       qobject_cast<QDial *>(widget) ||
//       qobject_cast<QDockWidget *>(widget) ||
       widget->inherits("QWorkspaceTitleBar") ||
       widget->inherits("QDockSeparator") ||
       widget->inherits("QDockWidgetSeparator"))
        widget->setAttribute(Qt::WA_Hover, false);
    if (qobject_cast<QScrollBar *>(widget))
    {
        widget->setAttribute(Qt::WA_Hover, false);
        if(ROUNDED && !opts.flatSbarButtons)
            widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
        if(!opts.gtkScrollViews)
            widget->removeEventFilter(this);
    }
    else if (qobject_cast<QProgressBar *>(widget))
    {
        widget->removeEventFilter(this);
        if(opts.boldProgress)
            unSetBold(widget);
        itsProgressBars.remove((QProgressBar *)widget);
    }
    else if (qobject_cast<QMenuBar*>(widget)) {
        widget->setAttribute(Qt::WA_Hover, false);

        if(CUSTOM_BGND)
            widget->setBackgroundRole(QPalette::Background);

//         if(opts.shadeMenubarOnlyWhenActive && SHADE_NONE!=opts.shadeMenubars)
        widget->removeEventFilter(this);

        if(SHADE_WINDOW_BORDER==opts.shadeMenubars || opts.customMenuTextColor || SHADE_BLEND_SELECTED==opts.shadeMenubars ||
           SHADE_SELECTED==opts.shadeMenubars || (SHADE_CUSTOM==opts.shadeMenubars &&TOO_DARK(itsMenubarCols[ORIGINAL_SHADE])))
            widget->setPalette(QApplication::palette());
    }
    else if(qobject_cast<QLabel*>(widget))
        widget->removeEventFilter(this);
    else if(/*!opts.gtkScrollViews && */
        qobject_cast<QAbstractScrollArea*>(widget)) {
        if (!opts.gtkScrollViews && (((QFrame *)widget)->frameWidth() > 0))
            widget->removeEventFilter(this);
        if (APP_KONTACT==theThemedApp && widget->parentWidget()) {
            QWidget *frame = scrollViewFrame(widget->parentWidget());
            if (frame) {
                if (itsSViewContainers.contains(frame)) {
                    itsSViewContainers[frame].remove(widget);
                    if (0 == itsSViewContainers[frame].count()) {
                        frame->removeEventFilter(this);
                        itsSViewContainers.remove(frame);
                        disconnect(frame, &QWidget::destroyed,
                                   this, &Style::widgetDestroyed);
                    }
                }
            }
        }
    } else if(qobject_cast<QDockWidget *>(widget) &&
            ((QDockWidget *)widget)->titleBarWidget() &&
            dynamic_cast<QtCurveDockWidgetTitleBar *>(((QDockWidget *)widget)->titleBarWidget()) &&
            widget->parentWidget() &&
            widget->parentWidget()->parentWidget() &&
            widget->parentWidget()->parentWidget()->parentWidget() &&
            qobject_cast<QSplitter *>(widget->parentWidget()) &&
            widget->parentWidget()->parentWidget()->inherits("KFileWidget") /*&&
                                                                              widget->parentWidget()->parentWidget()->parentWidget()->inherits("KFileDialog")*/)
    {
        delete ((QDockWidget *)widget)->titleBarWidget();
        ((QDockWidget *)widget)->setTitleBarWidget(0L);
    }
#ifdef QTC_QT5_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT
    else if(opts.fixParentlessDialogs && qobject_cast<QDialog *>(widget))
        widget->removeEventFilter(this);
#endif
    else if(opts.boldProgress && "CE_CapacityBar"==widget->objectName())
        unSetBold(widget);

    if(widget->inherits("QTipLabel") && !IS_FLAT(opts.tooltipAppearance) && APP_OPERA!=theThemedApp)
    {
        widget->setAttribute(Qt::WA_PaintOnScreen, false);
        widget->setAttribute(Qt::WA_NoSystemBackground, false);
        widget->clearMask();
    }

    if (!widget->isWindow())
        if (QFrame *frame = qobject_cast<QFrame *>(widget))
        {
//             if (QFrame::HLine==frame->frameShape() || QFrame::VLine==frame->frameShape())
            widget->removeEventFilter(this);

            if (qtcCheckKDEType0(widget->parent(), KTitleWidget)) {
                if(CUSTOM_BGND) {
                    frame->setAutoFillBackground(true);
                } else {
                    frame->setBackgroundRole(QPalette::Base);
                }
                QLayout *layout(frame->layout());

                if(layout) {
                    layout->setMargin(6);
                }
            }

            QWidget *p=0L;

            if(opts.gtkComboMenus && widget->parentWidget() && (p=widget->parentWidget()->parentWidget()) &&
               qobject_cast<QComboBox *>(p) && !((QComboBox *)(p))->isEditable())
                widget->setPalette(QApplication::palette());
        }

    if (qobject_cast<QMenu*>(widget)) {
        widget->removeEventFilter(this);
        widget->setAttribute(Qt::WA_PaintOnScreen, false);
        widget->setAttribute(Qt::WA_NoSystemBackground, false);
        widget->setAttribute(Qt::WA_TranslucentBackground, false);
        widget->clearMask();

        if(USE_LIGHTER_POPUP_MENU || opts.shadePopupMenu)
            widget->setPalette(QApplication::palette());
    }

    if((!IS_FLAT_BGND(opts.menuBgndAppearance) || 100!=opts.menuBgndOpacity || !(opts.square&SQUARE_POPUP_MENUS)) &&
       widget->inherits("QComboBoxPrivateContainer")) {
        widget->removeEventFilter(this);
        widget->setAttribute(Qt::WA_PaintOnScreen, false);
        widget->setAttribute(Qt::WA_NoSystemBackground, false);
        widget->setAttribute(Qt::WA_TranslucentBackground, false);
        widget->clearMask();
    }

    if (qobject_cast<QMenuBar *>(widget) ||
        qobject_cast<QToolBar *>(widget) ||
        (widget && qobject_cast<QToolBar *>(widget->parent())))
        widget->setBackgroundRole(QPalette::Button);
#ifdef QTC_ENABLE_X11
    QWidget *window = widget->window();

    if ((100 != opts.bgndOpacity &&
         Qt::Window == (window->windowFlags() & Qt::WindowType_Mask)) ||
        (100 != opts.dlgOpacity && Qt::Dialog == (window->windowFlags() &
                                                  Qt::WindowType_Mask))) {
        widget->removeEventFilter(this);
    }
#endif
}

bool Style::eventFilter(QObject *object, QEvent *event)
{
    bool isSViewCont = (APP_KONTACT == theThemedApp &&
                        itsSViewContainers.contains((QWidget*)object));

    if (::qobject_cast<QMenuBar*>(object) &&
        dynamic_cast<QMouseEvent*>(event)) {
        if (updateMenuBarEvent((QMouseEvent*)event, (QMenuBar*)object)) {
            return true;
        }
    }

    if (QEvent::Show == event->type() &&
        qobject_cast<QAbstractScrollArea*>(object) &&
        object->inherits("KFilePlacesView")) {
        QWidget *view = ((QAbstractScrollArea*)object)->viewport();
        QPalette palette = view->palette();
        QColor color = ((QWidget*)object)->palette().background().color();

        if (CUSTOM_BGND)
            color.setAlphaF(0.0);

        palette.setColor(view->backgroundRole(), color);
        view->setPalette(palette);
        object->removeEventFilter(this);
    }

    if ((!opts.gtkScrollViews &&
         ::qobject_cast<QAbstractScrollArea*>(object)) || isSViewCont) {
        QPoint pos;
        switch (event->type()) {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
            pos = ((QMouseEvent*)event)->pos();
            break;
        case QEvent::Wheel:
            pos=((QWheelEvent*)event)->pos();
        default:
            break;
        }

        if (!pos.isNull()) {
            QAbstractScrollArea *area = 0L;
            QPoint mapped(pos);

            if (isSViewCont) {
                QSet<QWidget*>::ConstIterator it =
                    itsSViewContainers[(QWidget*)object].begin();
                QSet<QWidget*>::ConstIterator end =
                    itsSViewContainers[(QWidget*)object].end();

                for (;it != end && !area;++it) {
                    if ((*it)->isVisible()) {
                        mapped = (*it)->mapFrom((QWidget*)object, pos);
                        if ((*it)->rect().adjusted(0, 0, 4, 4).contains(mapped)) {
                            area = (QAbstractScrollArea*)(*it);
                        }
                    }
                }
            } else {
                area = (QAbstractScrollArea*)object;
            }

            if (area) {
                QScrollBar *sbars[2] = {
                    area->verticalScrollBar(),
                    area->horizontalScrollBar()
                };

                for (int i = 0;i < 2;++i) {
                    if (sbars[i]) {
                        QRect r(i ? 0 : area->rect().right()-3,
                                i ? area->rect().bottom()-3 : 0,
                                sbars[i]->rect().width(),
                                sbars[i]->rect().height());

                        if (r.contains(pos) ||
                            (sbars[i] == itsSViewSBar &&
                             (QEvent::MouseMove == event->type() ||
                              QEvent::MouseButtonRelease == event->type()))) {
                            if (QEvent::Wheel != event->type()) {
                                struct HackEvent : public QMouseEvent {
                                    void set(const QPoint &mapped, bool vert)
                                        {
                                            l = QPointF(vert ? 0 : mapped.x(),
                                                        vert ? mapped.y() : 0);
                                            s = QPointF(s.x() + (vert ? 0 : -3),
                                                        s.y() + (vert ? -3 : 0));
                                        }
                                };
                                ((HackEvent*)event)->set(mapped, 0 == i);
                            }
                            sbars[i]->event(event);
                            if (QEvent::MouseButtonPress == event->type()) {
                                itsSViewSBar = sbars[i];
                            } else if (QEvent::MouseButtonRelease ==
                                       event->type()) {
                                itsSViewSBar = 0L;
                            }
                            return true;
                        }
                    }
                }
            }
        }
    }

    switch((int)(event->type())) {
    case QEvent::Timer:
    case QEvent::Move:
        return false; // just for performance - they can occur really often
    case QEvent::Resize:
        if(!(opts.square & SQUARE_POPUP_MENUS) &&
           object->inherits("QComboBoxPrivateContainer")) {
            QWidget *widget = static_cast<QWidget*>(object);
            if (Utils::hasAlphaChannel(widget)) {
                widget->clearMask();
            } else {
                widget->setMask(windowMask(widget->rect(),
                                           opts.round>ROUND_SLIGHT));
            }
            return false;
        }
#ifdef QTC_ENABLE_X11
        else if((BLEND_TITLEBAR ||
                 opts.windowBorder &
                 WINDOW_BORDER_USE_MENUBAR_COLOR_FOR_TITLEBAR ||
                 opts.menubarHiding & HIDE_KWIN) &&
                qobject_cast<QMenuBar*>(object)) {
            QResizeEvent *re = static_cast<QResizeEvent*>(event);

            if (re->size().height() != re->oldSize().height()) {
                emitMenuSize((QMenuBar*)object,
                             PREVIEW_MDI == itsIsPreview ||
                             !((QMenuBar*)object)->isVisible() ? 0 :
                             re->size().height());
            }
        }
#endif
        break;
    case QEvent::ShortcutOverride:
        if ((opts.menubarHiding || opts.statusbarHiding) &&
            qobject_cast<QMainWindow*>(object)) {
            QMainWindow *window = static_cast<QMainWindow*>(object);

            if (window->isVisible()) {
                if (opts.menubarHiding & HIDE_KEYBOARD &&
                    window->menuWidget()) {
                    QKeyEvent *k = static_cast<QKeyEvent*>(event);

                    if (k->modifiers() & Qt::ControlModifier &&
                        k->modifiers() & Qt::AltModifier &&
                        Qt::Key_M == k->key()) {
                        toggleMenuBar(window);
                    }
                }
                if (opts.statusbarHiding & HIDE_KEYBOARD) {
                    QKeyEvent *k = static_cast<QKeyEvent*>(event);

                    if(k->modifiers()&Qt::ControlModifier && k->modifiers()&Qt::AltModifier && Qt::Key_S==k->key())
                        toggleStatusBar(window);
                }
            }
        }
        break;
    case QEvent::ShowToParent:
        if(opts.menubarHiding && itsSaveMenuBarStatus && qobject_cast<QMenuBar *>(object) &&
           qtcMenuBarHidden(appName))
            static_cast<QMenuBar *>(object)->setHidden(true);
        if(opts.statusbarHiding && itsSaveStatusBarStatus && qobject_cast<QStatusBar *>(object) &&
           qtcStatusBarHidden(appName))
            static_cast<QStatusBar *>(object)->setHidden(true);
        break;
#ifdef QTC_ENABLE_X11
    case QEvent::PaletteChange:
    {
        QWidget *widget = qobject_cast<QWidget*>(object);

        if (widget && widget->isWindow() &&
            ((widget->windowFlags() & Qt::WindowType_Mask) &
             (Qt::Window | Qt::Dialog)))
            setBgndProp(widget, opts.bgndAppearance,
                        IMG_NONE != opts.bgndImage.type);
        break;
    }
#endif
    case QEvent::Paint:
    {
        if (CUSTOM_BGND) {
            QWidget *widget = qobject_cast<QWidget*>(object);

            if(widget && widget->testAttribute(Qt::WA_StyledBackground) &&
               (widget->isWindow() && ((widget->windowFlags()&Qt::WindowType_Mask) & (Qt::Window|Qt::Dialog)) &&
                widget->testAttribute(Qt::WA_TranslucentBackground)))
            {
                bool isDialog=qobject_cast<QDialog *>(widget);

                if((100!=opts.bgndOpacity && !isDialog) || (100!=opts.dlgOpacity && isDialog) ||
                   !(IS_FLAT_BGND(opts.bgndAppearance)) || IMG_NONE!=opts.bgndImage.type)
                {
                    QPainter p(widget);
                    p.setClipRegion(static_cast<QPaintEvent*>(event)->region());
                    drawBackground(&p, widget, isDialog ? BGND_DIALOG : BGND_WINDOW);
                }
            }
        }

        //bool isCombo=false;
        if((!IS_FLAT_BGND(opts.menuBgndAppearance) || IMG_NONE!=opts.menuBgndImage.type || 100!=opts.menuBgndOpacity ||
            !(opts.square&SQUARE_POPUP_MENUS)) &&
           (qobject_cast<QMenu*>(object) || (/*isCombo=*/object->inherits("QComboBoxPrivateContainer"))))
        {
            QWidget      *widget=qobject_cast<QWidget *>(object);
            QPainter     p(widget);
            QRect        r(widget->rect());
            double       radius=MENU_AND_TOOLTIP_RADIUS;
            QStyleOption opt;
            opt.init(widget);
            const QColor *use(popupMenuCols(&opt));

            p.setClipRegion(static_cast<QPaintEvent*>(event)->region());
            if(!opts.popupBorder)
            {
                p.setRenderHint(QPainter::Antialiasing, true);
                p.setPen(use[ORIGINAL_SHADE]);
                p.drawPath(buildPath(r, WIDGET_OTHER, ROUNDED_ALL, radius));
                p.setRenderHint(QPainter::Antialiasing, false);
            }
            if(!(opts.square&SQUARE_POPUP_MENUS)) // && !isCombo)
                p.setClipRegion(windowMask(r, opts.round>ROUND_SLIGHT), Qt::IntersectClip);

            // In case the gradient uses alpha, we need to fill with the background colour - this makes it consistent with Gtk.
            if(100==opts.menuBgndOpacity)
                p.fillRect(r, opt.palette.brush(QPalette::Background));
            drawBackground(&p, widget, BGND_MENU);
            if(opts.popupBorder)
            {
                EGradientBorder border=qtcGetGradient(opts.menuBgndAppearance, &opts)->border;

                p.setClipping(false);
                p.setPen(use[QTC_STD_BORDER]);
                // For now dont round combos - getting weird effects with shadow/clipping in Gtk2 style :-(
                if(opts.square&SQUARE_POPUP_MENUS) // || isCombo)
                    drawRect(&p, r);
                else
                {
                    p.setRenderHint(QPainter::Antialiasing, true);
                    p.drawPath(buildPath(r, WIDGET_OTHER, ROUNDED_ALL, radius));
                }

                if(USE_BORDER(border) && APPEARANCE_FLAT!=opts.menuBgndAppearance)
                {
                    QRect ri(r.adjusted(1, 1, -1, -1));

                    p.setPen(use[0]);
                    if(GB_LIGHT==border)
                    {
                        if(opts.square&SQUARE_POPUP_MENUS) // || isCombo)
                            drawRect(&p, ri);
                        else
                            p.drawPath(buildPath(ri, WIDGET_OTHER, ROUNDED_ALL, radius-1.0));
                    }
                    else if(opts.square&SQUARE_POPUP_MENUS) // || isCombo)
                    {
                        if(GB_3D!=border)
                        {
                            p.drawLine(ri.x(), ri.y(), ri.x()+ri.width()-1,  ri.y());
                            p.drawLine(ri.x(), ri.y(), ri.x(), ri.y()+ri.height()-1);
                        }
                        p.setPen(use[FRAME_DARK_SHADOW]);
                        p.drawLine(ri.x(), ri.y()+ri.height()-1, ri.x()+ri.width()-1,  ri.y()+ri.height()-1);
                        p.drawLine(ri.x()+ri.width()-1, ri.y(), ri.x()+ri.width()-1,  ri.y()+ri.height()-1);
                    }
                    else
                    {
                        QPainterPath tl,
                            br;

                        buildSplitPath(ri, ROUNDED_ALL, radius-1.0, tl, br);
                        if(GB_3D!=border)
                            p.drawPath(tl);
                        p.setPen(use[FRAME_DARK_SHADOW]);
                        p.drawPath(br);
                    }
                }
            }
        }
        else if(itsClickedLabel==object && qobject_cast<QLabel*>(object) && ((QLabel *)object)->buddy() && ((QLabel *)object)->buddy()->isEnabled())
        {
            // paint focus rect
            QLabel                *lbl = (QLabel *)object;
            QPainter              painter(lbl);
            QStyleOptionFocusRect opts;

            opts.palette = lbl->palette();
            opts.rect    = QRect(0, 0, lbl->width(), lbl->height());
            drawPrimitive(PE_FrameFocusRect, &opts, &painter, lbl);
        }
        else
        {
            QFrame *frame = qobject_cast<QFrame*>(object);

            if (frame)
            {
                if(QFrame::HLine==frame->frameShape() || QFrame::VLine==frame->frameShape())
                {
                    QPainter painter(frame);
                    QRect    r(QFrame::HLine==frame->frameShape()
                               ? QRect(frame->rect().x(), frame->rect().y()+ (frame->rect().height()/2), frame->rect().width(), 1)
                               : QRect(frame->rect().x()+(frame->rect().width()/2),  frame->rect().y(), 1, frame->rect().height()));

                    drawFadedLine(&painter, r, backgroundColors(frame->palette().window().color())[QTC_STD_BORDER], true, true,
                                  QFrame::HLine==frame->frameShape());
                    return true;
                }
                else
                    return false;
            }
        }
        break;
    }
    case QEvent::MouseButtonPress:
        if(dynamic_cast<QMouseEvent*>(event) && qobject_cast<QLabel*>(object) && ((QLabel *)object)->buddy())
        {
            QLabel      *lbl = (QLabel *)object;
            QMouseEvent *mev = (QMouseEvent *)event;

            if (lbl->rect().contains(mev->pos()))
            {
                itsClickedLabel=lbl;
                lbl->repaint();
            }
        }
        break;
    case QEvent::MouseButtonRelease:
        if(dynamic_cast<QMouseEvent*>(event) && qobject_cast<QLabel*>(object) && ((QLabel *)object)->buddy())
        {
            QLabel      *lbl = (QLabel *)object;
            QMouseEvent *mev = (QMouseEvent *)event;

            if(itsClickedLabel)
            {
                itsClickedLabel=0;
                lbl->update();
            }

            // set focus to the buddy...
            if (lbl->rect().contains(mev->pos()))
                ((QLabel *)object)->buddy()->setFocus(Qt::ShortcutFocusReason);
        }
        break;
    case QEvent::StyleChange:
    case QEvent::Show:
    {
        QProgressBar *bar = qobject_cast<QProgressBar *>(object);

        if(bar)
        {
            itsProgressBars.insert(bar);
            if (1==itsProgressBars.size())
            {
                itsTimer.start();
                itsProgressBarAnimateTimer = startTimer(1000 / constProgressBarFps);
            }
        }
        else if(!(opts.square&SQUARE_POPUP_MENUS) && object->inherits("QComboBoxPrivateContainer"))
        {
            QWidget *widget=static_cast<QWidget *>(object);
            if (Utils::hasAlphaChannel(widget)) {
                widget->clearMask();
            } else {
                widget->setMask(windowMask(widget->rect(),
                                           opts.round > ROUND_SLIGHT));
            }
            return false;
        }
#ifdef QTC_ENABLE_X11
        else if ((BLEND_TITLEBAR ||
                  opts.windowBorder &
                  WINDOW_BORDER_USE_MENUBAR_COLOR_FOR_TITLEBAR ||
                  opts.menubarHiding & HIDE_KWIN) &&
                 qobject_cast<QMenuBar*>(object)) {
            QMenuBar *mb=(QMenuBar*)object;
            emitMenuSize((QMenuBar*)mb, PREVIEW_MDI==itsIsPreview ||
                         !((QMenuBar *)mb)->isVisible() ? 0 :
                         mb->size().height(), true);
        } else if (QEvent::Show==event->type()) {
            QWidget *widget = qobject_cast<QWidget*>(object);

            if(widget && widget->isWindow() && ((widget->windowFlags()&Qt::WindowType_Mask) & (Qt::Window|Qt::Dialog)))
            {
                setBgndProp(widget, opts.bgndAppearance, IMG_NONE!=opts.bgndImage.type);

                int opacity = Qt::Dialog == (widget->windowFlags() &
                                             Qt::WindowType_Mask) ?
                    opts.dlgOpacity : opts.bgndOpacity;
                setOpacityProp(widget, (unsigned short)opacity);
            }
        }
#endif
        break;
    }
    case QEvent::Destroy:
    case QEvent::Hide:
    {
#ifdef QTC_ENABLE_X11
        if ((BLEND_TITLEBAR ||
             opts.windowBorder &
             WINDOW_BORDER_USE_MENUBAR_COLOR_FOR_TITLEBAR ||
             opts.menubarHiding & HIDE_KWIN) &&
           qobject_cast<QMenuBar *>(object)) {
            QMenuBar *mb = (QMenuBar*)object;
            emitMenuSize((QMenuBar*)mb, 0);
        }
#endif
        // if(itsHoverWidget && object==itsHoverWidget) {
        //     // itsPos.setX(-1);
        //     // itsPos.setY(-1);
        //     itsHoverWidget=0L;
        // }

        // The Destroy event is sent from ~QWidget, which happens after ~QProgressBar - therefore, we can't cast to a QProgressBar.
        // So we have to check on object.
        if (object && !itsProgressBars.isEmpty()) {
            itsProgressBars.remove(reinterpret_cast<QProgressBar*>(object));
            if (itsProgressBars.isEmpty()) {
                killTimer(itsProgressBarAnimateTimer);
                itsProgressBarAnimateTimer = 0;
            }
        }
#ifdef QTC_QT5_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT
        if(opts.fixParentlessDialogs && qobject_cast<QDialog *>(object) && itsReparentedDialogs.contains((QWidget*)object))
        {
            QWidget *widget=(QWidget*)object;

            // OK, reset back to its original parent..
            if(widget->windowFlags()&Qt::WindowType_Mask)
            {
                widget->removeEventFilter(this);
                widget->setParent(itsReparentedDialogs[widget]);
                Utils::addEventFilter(widget, this);
            }
            itsReparentedDialogs.remove(widget);
        }
#endif
        break;
    }
    case QEvent::Enter:
        break;
    case QEvent::Leave:
        // if(itsHoverWidget && object==itsHoverWidget)
        // {
        //     // itsPos.setX(-1);
        //     // itsPos.setY(-1);
        //     itsHoverWidget=0L;
        //     ((QWidget *)object)->repaint();
        // }
        break;
    case QEvent::MouseMove:  // Only occurs for widgets with mouse tracking enabled
        break;
    case QEvent::FocusIn:
    case QEvent::FocusOut:
        break;
    case QEvent::WindowActivate:
        if(opts.shadeMenubarOnlyWhenActive && SHADE_NONE!=opts.shadeMenubars && qobject_cast<QMenuBar *>(object))
        {
            itsActive=true;
            ((QWidget *)object)->repaint();
            return false;
        }
        break;
    case QEvent::WindowDeactivate:
        if(opts.shadeMenubarOnlyWhenActive && SHADE_NONE!=opts.shadeMenubars && qobject_cast<QMenuBar *>(object))
        {
            itsActive=false;
            ((QWidget *)object)->repaint();
            return false;
        }
        break;
#ifdef QTC_QT5_ENABLE_PARENTLESS_DIALOG_FIX_SUPPORT
    case 70: // QEvent::ChildInserted - QT3_SUPPORT
        if(opts.fixParentlessDialogs && qobject_cast<QDialog *>(object))
        {
            QDialog *dlg=(QDialog *)object;

            // The parent->isHidden is needed for KWord. It's insert picture file dialog is a child of the insert picture dialog - but the file
            // dialog is shown *before* the picture dialog!
            if(dlg && dlg->windowFlags()&Qt::WindowType_Mask && (!dlg->parentWidget() || dlg->parentWidget()->isHidden()))
            {
                QWidget *activeWindow=getActiveWindow((QWidget *)object);

                if(activeWindow)
                {
                    dlg->removeEventFilter(this);
                    dlg->setParent(activeWindow, dlg->windowFlags());
                    dlg->installEventFilter(this);
                    itsReparentedDialogs[(QWidget *)dlg]=dlg->parentWidget();
                    return false;
                }
            }
        }
#endif
    default:
        break;
    }

    return BASE_STYLE::eventFilter(object, event);
}

void Style::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == itsProgressBarAnimateTimer) {
        itsAnimateStep = itsTimer.elapsed() / (1000 / constProgressBarFps);
        for (QProgressBar *bar: const_(itsProgressBars)) {
            if ((opts.animatedProgress && 0 == itsAnimateStep % 2 &&
                 bar->value() != bar->minimum() &&
                 bar->value() != bar->maximum()) ||
                (0 == bar->minimum() && 0 == bar->maximum())) {
                bar->update();
            }
        }
    }

    event->ignore();
}

int Style::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch((unsigned)metric) {
    case PM_ToolTipLabelFrameWidth:
        return !ROUNDED || opts.square&SQUARE_TOOLTIPS ? BASE_STYLE::pixelMetric(metric, option, widget) : 3;
    case PM_MdiSubWindowFrameWidth:
        return 3;
    case PM_DockWidgetTitleMargin:
        return !(opts.dwtSettings&DWT_TEXT_ALIGN_AS_PER_TITLEBAR) || ALIGN_LEFT==opts.titlebarAlignment ? 4 : 0;
    case PM_DockWidgetTitleBarButtonMargin:
        return 4;
    case PM_DockWidgetFrameWidth:
        return 2;
    case PM_ToolBarExtensionExtent:
        return 15;
#ifndef QTC_QT5_ENABLE_KDE
    case PM_SmallIconSize:
        return 16;
    case PM_ToolBarIconSize:
        return 22;
    case PM_IconViewIconSize:
    case PM_LargeIconSize:
        return 32;
#else
    case PM_TabCloseIndicatorWidth:
    case PM_TabCloseIndicatorHeight:
    case PM_SmallIconSize:
    case PM_ButtonIconSize:
        return KIconLoader::global()->currentSize(KIconLoader::Small);
    case PM_ToolBarIconSize:
        return KIconLoader::global()->currentSize(KIconLoader::Toolbar);
    case PM_IconViewIconSize:
    case PM_LargeIconSize:
        return KIconLoader::global()->currentSize(KIconLoader::Dialog);
    case PM_MessageBoxIconSize:
        // TODO return KIconLoader::global()->currentSize(KIconLoader::MessageBox);
        return KIconLoader::SizeHuge;
#endif
    case PM_SubMenuOverlap:
        return -2;
    case PM_ScrollView_ScrollBarSpacing:
        return opts.etchEntry ? 2 : 3;
    case PM_MenuPanelWidth:
        return opts.popupBorder ? pixelMetric(PM_DefaultFrameWidth, option, widget) : 0;
    case PM_SizeGripSize:
        return SIZE_GRIP_SIZE;
    case PM_TabBarScrollButtonWidth:
        return 18;
    case PM_HeaderMargin:
        return 3;
    case PM_DefaultChildMargin:
        return isOOWidget(widget)
            ? /*opts.round>=ROUND_FULL && !(opts.square&SQUARE_SCROLLVIEW)
                ?*/ 2
            /*: 1*/
            : 6;
    case PM_DefaultTopLevelMargin:
        return 9;
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutVerticalSpacing:
        return -1; // use layoutSpacing
    case PM_DefaultLayoutSpacing:
        return 6;
    case PM_LayoutLeftMargin:
    case PM_LayoutTopMargin:
    case PM_LayoutRightMargin:
    case PM_LayoutBottomMargin:
        return pixelMetric((option && (option->state&QStyle::State_Window)) || (widget && widget->isWindow())
                           ? PM_DefaultTopLevelMargin
                           : PM_DefaultChildMargin, option, widget);
    case PM_MenuBarItemSpacing:
        return 0;
    case PM_ToolBarItemMargin:
        return 0;
    case PM_ToolBarItemSpacing:
        return TBTN_JOINED==opts.tbarBtns ? 0 : 1;
    case PM_ToolBarFrameWidth:
        // Remove because, in KDE4 at least, if have two locked toolbars together then the last/first items are too close
        return /*TB_NONE==opts.toolbarBorders ? 0 : */1;
    case PM_FocusFrameVMargin:
    case PM_FocusFrameHMargin:
        return 2;
    case PM_MenuBarVMargin:
    case PM_MenuBarHMargin:
        // Bangarang (media player) has a 4 pixel high menubar at the top - when it doesn't actually have a menubar!
        // Seems to be because of the return 2 below (which was previously always returned unless XBar support and
        // size was 0). So, if we are askes for these metrics for a widet whose size<6, then return 0.
        return widget && widget->size().height() < 6 ? 0 : 2;
    case PM_MenuHMargin:
    case PM_MenuVMargin:
        return 0;
    case PM_MenuButtonIndicator:
        return (DO_EFFECT ? 10 : 9)+
                                                   (!widget || qobject_cast<const QToolButton *>(widget) ? 6 : 0);
    case PM_ButtonMargin:
        return (DO_EFFECT
                ? (opts.thin&THIN_BUTTONS) ? 4 : 6
                : (opts.thin&THIN_BUTTONS) ? 2 : 4)+MAX_ROUND_BTN_PAD;
    case PM_TabBarTabShiftVertical:
#if 0 // QT_VERSION < 0x040500
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option))
        {
            if((QTabBar::RoundedSouth==tab->shape || QTabBar::TriangularSouth==tab->shape))
                return -2;
        }
#endif
        return 2;
    case PM_TabBarTabShiftHorizontal:
        return 0;
    case PM_ButtonShiftHorizontal:
        // return Qt::RightToLeft==QApplication::layoutDirection() ? -1 : 1;
    case PM_ButtonShiftVertical:
        return APP_KDEVELOP==theThemedApp && !opts.stdSidebarButtons && widget && isMultiTabBarTab(getButton(widget, 0L)) ? 0 : 1;
    case PM_ButtonDefaultIndicator:
        return 0;
    case PM_DefaultFrameWidth:
        if ((/*!opts.popupBorder || */opts.gtkComboMenus) && widget && widget->inherits("QComboBoxPrivateContainer"))
            return opts.gtkComboMenus ? (opts.borderMenuitems || !(opts.square&SQUARE_POPUP_MENUS) ? 2 : 1) : 0;

        if ((!opts.gtkScrollViews || (opts.square&SQUARE_SCROLLVIEW)) && isKateView(widget))
            return (opts.square&SQUARE_SCROLLVIEW) ? 1 : 0;

        if ((opts.square&SQUARE_SCROLLVIEW) && widget && !opts.etchEntry &&
            (::qobject_cast<const QAbstractScrollArea *>(widget) ||
             isKontactPreviewPane(widget)))
            return (opts.gtkScrollViews || opts.thinSbarGroove || !opts.borderSbarGroove) && (!opts.highlightScrollViews) ? 1 : 2;

        if (!DRAW_MENU_BORDER && !opts.borderMenuitems && opts.square&SQUARE_POPUP_MENUS && qobject_cast<const QMenu *>(widget))
            return 1;

        if(DO_EFFECT && opts.etchEntry &&
           (!widget || // !isFormWidget(widget) &&
            ::qobject_cast<const QLineEdit *>(widget) || ::qobject_cast<const QAbstractScrollArea*>(widget) /*|| isKontactPreviewPane(widget)*/))
            return 3;
        else
            return 2;
    case PM_SpinBoxFrameWidth:
        return DO_EFFECT && opts.etchEntry ? 3 : 2;
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
        // TODO??
        // case PM_CheckListControllerSize:
        // case PM_CheckListButtonSize:
        return DO_EFFECT ? opts.crSize+2 : opts.crSize;
    case PM_TabBarTabOverlap:
        return TAB_MO_GLOW==opts.tabMouseOver ? 0 : 1;
    case PM_ProgressBarChunkWidth:
        return 4;
//         case PM_DockWindowHandleExtent:
//             return 10;
    case PM_DockWidgetSeparatorExtent:
    case PM_SplitterWidth:
        return LINE_1DOT==opts.splitters ? 7 : 6;
    case PM_ToolBarHandleExtent:
        return LINE_1DOT==opts.handles ? 7 : 8;
    case PM_ScrollBarSliderMin:
        // Leave a minimum of 21 pixels (which is the size used by Oxygen)
        // See https://github.com/QtCurve/qtcurve-qt4/issues/7
        // and https://bugs.kde.org/show_bug.cgi?id=317690
        return qtcMax(opts.sliderWidth, 20) + 1;
    case PM_SliderThickness:
        return (SLIDER_CIRCULAR==opts.sliderStyle
                ? CIRCULAR_SLIDER_SIZE+6
                : SLIDER_TRIANGULAR==opts.sliderStyle
                ? 19
                : (SLIDER_SIZE+(ROTATED_SLIDER ? 11 : 6)))+SLIDER_GLOW;
    case PM_SliderControlThickness:
        return (SLIDER_CIRCULAR==opts.sliderStyle
                ? CIRCULAR_SLIDER_SIZE
                : SLIDER_TRIANGULAR==opts.sliderStyle
                ? 11
                : (SLIDER_SIZE+(ROTATED_SLIDER ? 6 : -2)))+SLIDER_GLOW;
    case PM_SliderTickmarkOffset:
        return SLIDER_TRIANGULAR==opts.sliderStyle ? 5 : 4;
    case PM_SliderSpaceAvailable:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            int size(pixelMetric(PM_SliderControlThickness, slider, widget));

            if (slider->tickPosition & QSlider::TicksBelow)
                ++size;
            if (slider->tickPosition & QSlider::TicksAbove)
                ++size;
            return size;
        }
        return BASE_STYLE::pixelMetric(metric, option, widget);
    case PM_SliderLength:
        return (SLIDER_CIRCULAR==opts.sliderStyle
                ? CIRCULAR_SLIDER_SIZE
                : SLIDER_TRIANGULAR==opts.sliderStyle
                ? 11
                : (SLIDER_SIZE+(ROTATED_SLIDER ? -2 : 6)))+SLIDER_GLOW;
    case PM_ScrollBarExtent:
        return opts.sliderWidth;
    case PM_MaximumDragDistance:
        return -1;
    case PM_TabBarTabHSpace:
        return 14;
    case PM_TabBarTabVSpace:
        return opts.highlightTab ? 10 : 8;
    case PM_TitleBarHeight:
        return qMax(widget ? widget->fontMetrics().lineSpacing()
                    : option ? option->fontMetrics.lineSpacing()
                    : 0, 24);
    case PM_MenuBarPanelWidth:
        return 0;
    case QtC_Round:
        return (int)((opts.square&SQUARE_WINDOWS && opts.round>ROUND_SLIGHT) ? ROUND_SLIGHT : opts.round);
    case QtC_WindowBorder:
        return opts.windowBorder;
    case QtC_CustomBgnd:
        return CUSTOM_BGND;
    case QtC_TitleBarButtonAppearance:
        return (int)opts.titlebarButtonAppearance;
    case QtC_TitleAlignment:
        switch(opts.titlebarAlignment)
        {
        default:
        case ALIGN_LEFT:
            return Qt::AlignLeft;
        case ALIGN_CENTER:
            return Qt::AlignHCenter|Qt::AlignVCenter;
        case ALIGN_FULL_CENTER:
            return Qt::AlignHCenter;
        case ALIGN_RIGHT:
            return Qt::AlignRight;
        }
    case QtC_TitleBarButtons:
        return opts.titlebarButtons;
    case QtC_TitleBarIcon:
        return opts.titlebarIcon;
    case QtC_TitleBarIconColor:
        return titlebarIconColor(option).rgb();
    case QtC_TitleBarEffect:
        return opts.titlebarEffect;
    case QtC_BlendMenuAndTitleBar:
        return BLEND_TITLEBAR;
    case QtC_ShadeMenubarOnlyWhenActive:
        return opts.shadeMenubarOnlyWhenActive;
    case QtC_ToggleButtons:
        return (opts.menubarHiding&HIDE_KWIN   ? 0x1 : 0)+
            (opts.statusbarHiding&HIDE_KWIN ? 0x2 : 0);
    case QtC_MenubarColor:
        return itsMenubarCols[ORIGINAL_SHADE].rgb();
    case QtC_TitleBarApp:
        return !option || option->state&State_Active ? opts.titlebarAppearance : opts.inactiveTitlebarAppearance;
// The following is a somewhat hackyish fix for konqueror's show close button on tab setting...
// ...its hackish in the way that I'm assuming when KTabBar is positioning the close button and it
// asks for these options, it only passes in a QStyleOption  not a QStyleOptionTab
//.........
    case PM_TabBarBaseHeight:
        if (qtcCheckKDEType0(widget, KTabBar) &&
            !qstyleoption_cast<const QStyleOptionTab*>(option))
            return 10;
        return BASE_STYLE::pixelMetric(metric, option, widget);
    case PM_TabBarBaseOverlap:
        if (qtcCheckKDEType0(widget, KTabBar) &&
            !qstyleoption_cast<const QStyleOptionTab*>(option))
            return 0;
        // Fall through!
    default:
        return BASE_STYLE::pixelMetric(metric, option, widget);
    }
}

int Style::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    switch (hint) {
    case SH_ToolTip_Mask:
    case SH_Menu_Mask:
        if((SH_ToolTip_Mask==hint && (opts.square&SQUARE_TOOLTIPS)) ||
           (SH_Menu_Mask==hint && (opts.square&SQUARE_POPUP_MENUS)))
            return BASE_STYLE::styleHint(hint, option, widget, returnData);
        else
        {
            if(!Utils::hasAlphaChannel(widget) && (!widget || widget->isWindow()))
                if(QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask *>(returnData))
                    mask->region = windowMask(option->rect, opts.round>ROUND_SLIGHT);
            return true;
        }
    case SH_ComboBox_ListMouseTracking:
    case SH_PrintDialog_RightAlignButtons:
    case SH_ItemView_ArrowKeysNavigateIntoChildren:
    case SH_ToolBox_SelectedPageTitleBold:
    case SH_ScrollBar_MiddleClickAbsolutePosition:
    case SH_SpinControls_DisableOnBounds:
    case SH_Slider_SnapToValue:
    case SH_FontDialog_SelectAssociatedText:
    case SH_Menu_MouseTracking:
        return true;
    case SH_UnderlineShortcut:
        return widget && opts.hideShortcutUnderline ? itsShortcutHandler->showShortcut(widget) : true;
    case SH_GroupBox_TextLabelVerticalAlignment:
        if (const QStyleOptionGroupBox *frame = qstyleoption_cast<const QStyleOptionGroupBox *>(option))
        {
            if (frame->features & QStyleOptionFrame::Flat)
                return Qt::AlignVCenter;
        }
        return opts.gbLabel&GB_LBL_INSIDE
            ? Qt::AlignBottom
            : opts.gbLabel&GB_LBL_OUTSIDE
            ? Qt::AlignTop
            : Qt::AlignVCenter;
    case SH_MessageBox_CenterButtons:
    case SH_ProgressDialog_CenterCancelButton:
    case SH_DitherDisabledText:
    case SH_EtchDisabledText:
    case SH_Menu_AllowActiveAndDisabled:
    case SH_ItemView_ShowDecorationSelected: // Controls whether the highlighting of listview/treeview items highlights whole line.
    case SH_MenuBar_AltKeyNavigation:
        return false;
    case SH_ItemView_ChangeHighlightOnFocus: // gray out selected items when losing focus.
        return false;
    case SH_WizardStyle:
        return QWizard::ClassicStyle;
    case SH_RubberBand_Mask:
    {
        const QStyleOptionRubberBand *opt = qstyleoption_cast<const QStyleOptionRubberBand *>(option);
        if (!opt)
            return true;
        if (QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask*>(returnData))
        {
            mask->region = option->rect;
            mask->region -= option->rect.adjusted(1,1,-1,-1);
        }
        return true;
    }
    case SH_Menu_SubMenuPopupDelay:
        return opts.menuDelay;
    case SH_ToolButton_PopupDelay:
        return 250;
    case SH_ComboBox_PopupFrameStyle:
        return opts.popupBorder || !(opts.square&SQUARE_POPUP_MENUS) ? QFrame::StyledPanel|QFrame::Plain : QFrame::NoFrame;
    case SH_TabBar_Alignment:
        return Qt::AlignLeft;
    case SH_Header_ArrowAlignment:
        return Qt::AlignLeft;
    case SH_WindowFrame_Mask:
        if (QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask *>(returnData))
        {
            QRect r(option->rect);

            switch((opts.square&SQUARE_WINDOWS && opts.round>ROUND_SLIGHT) ? ROUND_SLIGHT : opts.round)
            {
            case ROUND_NONE:
                mask->region=r;
                break;
            case ROUND_SLIGHT:
                mask->region=QRegion(r.x()+1, r.y(), r.width()-2, r.height());
                mask->region += QRegion(r.x()+0, r.y()+1, 1, r.height()-2);
                mask->region += QRegion(r.x()+r.width()-1, r.y()+1, 1, r.height()-2);
                break;
            default: // ROUND_FULL
                mask->region=QRegion(r.x()+5, r.y(), r.width()-10, r.height());
                mask->region += QRegion(r.x()+0, r.y()+5, 1, r.height()-5);
                mask->region += QRegion(r.x()+1, r.y()+3, 1, r.height()-2);
                mask->region += QRegion(r.x()+2, r.y()+2, 1, r.height()-1);
                mask->region += QRegion(r.x()+3, r.y()+1, 2, r.height());
                mask->region += QRegion(r.x()+r.width()-1, r.y()+5, 1, r.height()-5);
                mask->region += QRegion(r.x()+r.width()-2, r.y()+3, 1, r.height()-2);
                mask->region += QRegion(r.x()+r.width()-3, r.y()+2, 1, r.height()-1);
                mask->region += QRegion(r.x()+r.width()-5, r.y()+1, 2, r.height()-0);
            }
        }
        return 1;
    case SH_TitleBar_NoBorder:
    case SH_TitleBar_AutoRaise:
        return 1;
    case SH_MainWindow_SpaceBelowMenuBar:
        return 0;
    case SH_DialogButtonLayout:
        return opts.gtkButtonOrder ? QDialogButtonBox::GnomeLayout :
        QDialogButtonBox::KdeLayout;
    case SH_MessageBox_TextInteractionFlags:
        return Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse;
    case SH_LineEdit_PasswordCharacter:
        if (opts.passwordChar) {
            int chars[4]={opts.passwordChar, 0x25CF, 0x2022, 0};
            const QFontMetrics &fm(option ? option->fontMetrics :
                                   (widget ? widget->fontMetrics() :
                                    QFontMetrics(QFont())));
            for (int i = 0;chars[i];++i) {
                if (fm.inFont(QChar(chars[i]))) {
                    return chars[i];
                }
            }
            return '*';
        } else {
            return '\0';
        }
    case SH_MenuBar_MouseTracking:
        // Always return 1, as setting to 0 dissables the effect when a menu is shown.
        return 1; // opts.menubarMouseOver ? 1 : 0;
    case SH_ScrollView_FrameOnlyAroundContents:
        return widget && widget->isWindow()
            ? false
            : opts.gtkScrollViews && (!widget || !widget->inherits("QComboBoxListView"));
    case SH_ComboBox_Popup:
        if(opts.gtkComboMenus)
        {
            if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(option))
                return !cmb->editable;
        }
        return 0;
    case SH_FormLayoutFormAlignment:
        return Qt::AlignLeft | Qt::AlignTop; // KDE4 HIG, align the contents in a form layout to the left
    case SH_FormLayoutLabelAlignment:
        return Qt::AlignRight; // KDE4  HIG, align the labels in a form layout to the right
    case SH_FormLayoutFieldGrowthPolicy:
        return QFormLayout::ExpandingFieldsGrow;
    case SH_FormLayoutWrapPolicy:
        return QFormLayout::DontWrapRows;
#ifdef QTC_QT5_ENABLE_KDE
    case SH_DialogButtonBox_ButtonsHaveIcons:
        return KGlobalSettings::showIconsOnPushButtons();
    case SH_ItemView_ActivateItemOnSingleClick:
        return KGlobalSettings::singleClick();
#endif
    default:
#ifdef QTC_QT5_ENABLE_KDE
        // Tell the calling app that we can handle certain custom widgets...
        if(hint>=SH_CustomBase && widget)
            if("CE_CapacityBar"==widget->objectName())
            {
                if (opts.boldProgress)
                    setBold((QWidget *)widget);
                return CE_QtC_KCapacityBar;
            }
#endif
        return BASE_STYLE::styleHint(hint, option, widget, returnData);
    }
}

QPalette Style::standardPalette() const
{
#ifndef QTC_QT5_ENABLE_KDE
    return BASE_STYLE::standardPalette();
#else
    return KGlobalSettings::createApplicationPalette(KSharedConfig::openConfig(itsComponentData));
#endif
}

void Style::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                          QPainter *painter, const QWidget *widget) const
{
    QRect r(option->rect);
    State state(option->state);
    const QPalette &palette(option->palette);
    bool reverse(Qt::RightToLeft==option->direction);

    switch ((unsigned)element) {
#ifdef QTC_QT5_ENABLE_KDE
    case PE_IndicatorTabClose: {
        int         size(pixelMetric(QStyle::PM_SmallIconSize));
        QIcon::Mode mode(state & State_Enabled
                         ? state & State_Raised
                         ? QIcon::Active
                         : QIcon::Normal
                         : QIcon::Disabled);

        if (!(state&State_Raised) && !(state&State_Sunken) && !(state&QStyle::State_Selected))
            mode = QIcon::Disabled;

        drawItemPixmap(painter, r, Qt::AlignCenter, KIcon("dialog-close").pixmap(size, mode, state&State_Sunken
                                                                                 ? QIcon::On : QIcon::Off));
        break;
    }
#endif
    case PE_Widget:
        if(widget && widget->testAttribute(Qt::WA_StyledBackground) &&
           ( (!widget->testAttribute(Qt::WA_NoSystemBackground) &&
              ((widget->windowFlags()&Qt::WindowType_Mask) & (Qt::Window|Qt::Dialog)) && widget->isWindow()) ||
             (itsIsPreview && qobject_cast<const QMdiSubWindow *>(widget)) ) )
        {
            bool isDialog=qobject_cast<const QDialog *>(widget);

            if(CUSTOM_BGND || itsIsPreview || (isDialog && opts.dlgOpacity!=100) || (!isDialog && opts.bgndOpacity!=100))
                drawBackground(painter, widget, isDialog ? BGND_DIALOG : BGND_WINDOW);
        }
        break;
    case PE_PanelScrollAreaCorner:
        // disable painting of PE_PanelScrollAreaCorner
        // the default implementation fills the rect with the window background color which does not work for windows that have gradients.
        // ...but need to for WebView!!!
        if(!opts.gtkScrollViews || !CUSTOM_BGND ||
           (widget && widget->inherits("WebView")))
            painter->fillRect(r, palette.brush(QPalette::Window));
        break;
    case PE_IndicatorBranch:
    {
        int middleH((r.x() + r.width() / 2)-1),
            middleV(r.y() + r.height() / 2),
#if 0
            beforeH(middleH),
#endif
            beforeV(middleV),
            afterH(middleH),
            afterV(middleV);

        painter->save();

        if (state&State_Children)
        {
            QRect ar(r.x()+((r.width()-(LV_SIZE+4))>>1), r.y()+((r.height()-(LV_SIZE+4))>>1), LV_SIZE+4,
                     LV_SIZE+4);
            if(/*LV_OLD==*/opts.lvLines)
            {
                beforeV=ar.y()-1;
                afterH=ar.x()+LV_SIZE+4;
                afterV=ar.y()+LV_SIZE+4;
#if 0
                beforeH=ar.x();
                int lo(ROUNDED ? 2 : 0);

                painter->setPen(palette.mid().color());
                painter->drawLine(ar.x()+lo, ar.y(), (ar.x()+ar.width()-1)-lo, ar.y());
                painter->drawLine(ar.x()+lo, ar.y()+ar.height()-1, (ar.x()+ar.width()-1)-lo, ar.y()+ar.height()-1);
                painter->drawLine(ar.x(), ar.y()+lo, ar.x(), (ar.y()+ar.height()-1)-lo);
                painter->drawLine(ar.x()+ar.width()-1, ar.y()+lo, ar.x()+ar.width()-1, (ar.y()+ar.height()-1)-lo);

                if(ROUNDED)
                {
                    painter->drawPoint(ar.x()+1, ar.y()+1);
                    painter->drawPoint(ar.x()+1, ar.y()+ar.height()-2);
                    painter->drawPoint(ar.x()+ar.width()-2, ar.y()+1);
                    painter->drawPoint(ar.x()+ar.width()-2, ar.y()+ar.height()-2);

                    QColor col(palette.mid().color());

                    col.setAlphaF(0.5);
                    painter->setPen(col);
                    painter->drawLine(ar.x()+1, ar.y()+1, ar.x()+2, ar.y());
                    painter->drawLine(ar.x()+ar.width()-2, ar.y(), ar.x()+ar.width()-1, ar.y()+1);
                    painter->drawLine(ar.x()+1, ar.y()+ar.height()-2, ar.x()+2, ar.y()+ar.height()-1);
                    painter->drawLine(ar.x()+ar.width()-2, ar.y()+ar.height()-1, ar.x()+ar.width()-1, ar.y()+ar.height()-2);
                }
#endif
            }
            drawArrow(painter, ar, state&State_Open
                      ? PE_IndicatorArrowDown
                      : reverse
                      ? PE_IndicatorArrowLeft
                      : PE_IndicatorArrowRight, MO_ARROW(QPalette::ButtonText));
        }

        const int constStep=/*LV_OLD==*/opts.lvLines
            ? 0
            : widget && qobject_cast<const QTreeView *>(widget)
            ? ((QTreeView *)widget)->indentation() : 20;

        if(opts.lvLines /*&& (LV_OLD==opts.lvLines || (r.x()>=constStep && constStep>0))*/)
        {
            painter->setPen(palette.mid().color());
            if (state&State_Item)
            {
                if (reverse)
                    painter->drawLine(r.left(), middleV, afterH, middleV);
                else
                {
#if 0
                    if(LV_NEW==opts.lvLines)
                    {
                        if(state&State_Children)
                            painter->drawLine(middleH-constStep, middleV, r.right()-constStep, middleV);
                        else
                            drawFadedLine(painter, QRect(middleH-constStep, middleV, r.right()-(middleH-constStep), middleV), palette.mid().color(),
                                          false, true, true);
                    }
                    else
#endif
                        painter->drawLine(afterH, middleV, r.right(), middleV);
                }
            }
            if (state&State_Sibling && afterV<r.bottom())
                painter->drawLine(middleH-constStep, afterV, middleH-constStep, r.bottom());
            if (state & (State_Open | State_Children | State_Item | State_Sibling) && (/*LV_NEW==opts.lvLines || */beforeV>r.y()))
                painter->drawLine(middleH-constStep, r.y(), middleH-constStep, beforeV);
        }
        painter->restore();
        break;
    }
    case PE_IndicatorViewItemCheck:
    {
        QStyleOption opt(*option);

        opt.state &= ~State_MouseOver;
        opt.state |= STATE_VIEW;
        drawPrimitive(PE_IndicatorCheckBox, &opt, painter, widget);
        break;
    }
    case PE_IndicatorHeaderArrow:
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option))
            drawArrow(painter, r, header->sortIndicator & QStyleOptionHeader::SortUp ? PE_IndicatorArrowUp : PE_IndicatorArrowDown,
                      MO_ARROW(QPalette::ButtonText));
        break;
    case PE_IndicatorArrowUp:
    case PE_IndicatorArrowDown:
    case PE_IndicatorArrowLeft:
    case PE_IndicatorArrowRight:
        if(State_None==state)
            state|=State_Enabled;
        if((QStyle::State_Enabled|QtC_StateKWin)==state)
            drawArrow(painter, r, element, Qt::color1, false, true);
        else
        {
            QColor col(MO_ARROW(QPalette::Text));
            if(state&(State_Sunken|State_On) &&
               !(widget && ( (opts.unifySpin && qobject_cast<const QSpinBox *>(widget)) ||
                             (opts.unifyCombo && qobject_cast<const QComboBox *>(widget) &&
                              ((const QComboBox *)widget)->isEditable()))))
                r.adjust(1, 1, 1, 1);
            if(col.alpha()<255 && PE_IndicatorArrowRight==element && widget && widget->inherits("KUrlButton"))
                col=blendColors(col, palette.background().color(), col.alphaF());

            drawArrow(painter, r, element, col, false, false);
        }
        break;
    case PE_IndicatorSpinMinus:
    case PE_IndicatorSpinPlus:
    case PE_IndicatorSpinUp:
    case PE_IndicatorSpinDown:
    {
        QRect        sr(r);
        const QColor *use(buttonColors(option)),
            col(MO_ARROW(QPalette::ButtonText));
        bool         down(PE_IndicatorSpinDown==element || PE_IndicatorSpinMinus==element);

        if((!opts.unifySpinBtns || state&State_Sunken) && !opts.unifySpin)
            drawLightBevel(painter, sr, option, widget, down
                           ? reverse
                           ? ROUNDED_BOTTOMLEFT
                           : ROUNDED_BOTTOMRIGHT
                           : reverse
                           ? ROUNDED_TOPLEFT
                           : ROUNDED_TOPRIGHT,
                           getFill(option, use), use, true, WIDGET_SPIN);

        if(PE_IndicatorSpinUp==element || PE_IndicatorSpinDown==element)
        {
            sr.setY(sr.y()+(down ? -2 : 1));

            if(opts.unifySpin)
            {
                sr.adjust(reverse ? 1 : -1, 0, reverse ? 1 : -1, 0);
                if(!opts.vArrows)
                    sr.setY(sr.y()+(down ? -2 : 2));
            }
            else if(state&State_Sunken)
                sr.adjust(1, 1, 1, 1);

            drawArrow(painter, sr, PE_IndicatorSpinUp==element ? PE_IndicatorArrowUp : PE_IndicatorArrowDown,
                      col, !opts.unifySpin);
        }
        else
        {
            int    l(qMin(r.width()-6, r.height()-6));
            QPoint c(r.x()+(r.width()/2), r.y()+(r.height()/2));

            l/=2;
            if(l%2 != 0)
                --l;

            if(state&State_Sunken && !opts.unifySpin)
                c+=QPoint(1, 1);

            painter->setPen(col);
            painter->drawLine(c.x()-l, c.y(), c.x()+l, c.y());
            if(!down)
                painter->drawLine(c.x(), c.y()-l, c.x(), c.y()+l);
        }
        break;
    }
    case PE_IndicatorToolBarSeparator:
    {
        painter->save();
        switch(opts.toolbarSeparators)
        {
        case LINE_NONE:
            break;
        case LINE_FLAT:
        case LINE_SUNKEN:
            if(r.width()<r.height())
            {
                int x(r.x()+((r.width()-2) / 2));
                drawFadedLine(painter, QRect(x, r.y()+TOOLBAR_SEP_GAP, 1, r.height()-(TOOLBAR_SEP_GAP*2)),
                              itsBackgroundCols[LINE_SUNKEN==opts.toolbarSeparators ? 3 : 4], true, true, false);

                if(LINE_SUNKEN==opts.toolbarSeparators)
                    drawFadedLine(painter, QRect(x+1, r.y()+6, 1, r.height()-12), itsBackgroundCols[0], true, true, false);
            }
            else
            {
                int y(r.y()+((r.height()-2) / 2));

                drawFadedLine(painter, QRect(r.x()+TOOLBAR_SEP_GAP, y, r.width()-(TOOLBAR_SEP_GAP*2), 1),
                              itsBackgroundCols[LINE_SUNKEN==opts.toolbarSeparators ? 3 : 4], true, true, true);
                if(LINE_SUNKEN==opts.toolbarSeparators)
                    drawFadedLine(painter, QRect(r.x()+TOOLBAR_SEP_GAP, y+1, r.width()-(TOOLBAR_SEP_GAP*2), 1),
                                  itsBackgroundCols[0], true, true, true);
            }
            break;
        default:
        case LINE_DOTS:
            drawDots(painter, r, !(state&State_Horizontal), 1, 5, itsBackgroundCols, 0, 5);
        }
        painter->restore();
        break;
    }
    case PE_FrameGroupBox:
        if(FRAME_NONE==opts.groupBox)
            break;
        if (const QStyleOptionFrame *_frame =
            qstyleoption_cast<const QStyleOptionFrame*>(option))
        {
            QStyleOptionFrame frame(*_frame);
            if (frame.features & QStyleOptionFrame::Flat ||
                FRAME_LINE == opts.groupBox) {
                drawFadedLine(painter, QRect(r.x(), r.y(), r.width(), 1),
                              backgroundColors(option)[QTC_STD_BORDER],
                              opts.gbLabel & GB_LBL_CENTRED || reverse,
                              opts.gbLabel & GB_LBL_CENTRED || !reverse, true);
            } else {
                if(opts.gbLabel&GB_LBL_OUTSIDE)
                    r.adjust(0, 2, 0, 0);

                if(FRAME_SHADED == opts.groupBox ||
                   FRAME_FADED == opts.groupBox) {
                    int round = opts.square & SQUARE_FRAME ?
                        ROUNDED_NONE : ROUNDED_ALL;
                    QPainterPath path(buildPath(r, WIDGET_FRAME, round,
                                                ROUNDED_ALL == round
                                                ? qtcGetRadius(&opts, r.width(),
                                                               r.height(),
                                                               WIDGET_FRAME,
                                                               RADIUS_EXTERNAL)
                                                : 0.0));

                    painter->save();
                    painter->setClipping(false);
                    if(0!=opts.gbFactor)
                    {
                        QColor col(opts.gbFactor<0 ? Qt::black : Qt::white);

                        col.setAlphaF(TO_ALPHA(opts.gbFactor));
                        if(FRAME_SHADED==opts.groupBox)
                            painter->fillPath(path, col);
                        else
                        {
                            QLinearGradient grad(r.topLeft(), r.bottomLeft());

                            grad.setColorAt(0, col);
                            col.setAlphaF(0.0);
                            grad.setColorAt(1, col);
                            painter->fillPath(path, grad);
                        }
                    }

                    if(!(opts.gbLabel&(GB_LBL_INSIDE|GB_LBL_OUTSIDE)))
                        painter->restore();

                    if(FRAME_SHADED==opts.groupBox)
                        drawBorder(painter, r, option, round, backgroundColors(option), WIDGET_FRAME,
                                   /*state&State_Raised && opts.gbFactor<0 ? BORDER_RAISED : */BORDER_SUNKEN);
                    else
                    {
                        QColor          col(backgroundColors(option)[QTC_STD_BORDER]);
                        QLinearGradient grad(r.topLeft(), r.bottomLeft());

                        col.setAlphaF(1.0);
                        grad.setColorAt(0, col);
                        col.setAlphaF(0.0);
                        grad.setColorAt(1, col);
                        painter->setRenderHint(QPainter::Antialiasing, true);
                        painter->setPen(QPen(QBrush(grad), 1));
                        painter->drawPath(path);
                    }
                    if(opts.gbLabel&(GB_LBL_INSIDE|GB_LBL_OUTSIDE))
                        painter->restore();
                }
                else
                {
                    frame.state &= ~(State_Sunken | State_HasFocus);
                    frame.rect = r;
                    drawPrimitive(PE_Frame, &frame, painter, widget);
                }
            }
        }
        break;
    case PE_Frame:
    {
        // Dont draw OO.o status bar frames...
        if (isOOWidget(widget) && r.height() < 22)
            break;
        if (widget && qtcCheckKDEType0(widget->parent(), KTitleWidget)) {
            break;
        } else if (widget && qtcCheckType0<QComboBox>(widget->parent())) {
            if(opts.gtkComboMenus && !((QComboBox*)(widget->parent()))->isEditable())
                drawPrimitive(PE_FrameMenu, option, painter, widget);
            else if(opts.square&SQUARE_POPUP_MENUS)
            {
                const QColor *use(APP_KRUNNER==theThemedApp ? itsBackgroundCols : backgroundColors(option));

                painter->save();
                painter->setPen(use[QTC_STD_BORDER]);
                drawRect(painter, r);
                painter->setPen(palette.base().color());
                drawRect(painter, r.adjusted(1, 1, -1, -1));
                painter->restore();
            }
        }
        else
        {
            const QStyleOptionFrame *fo = qstyleoption_cast<const QStyleOptionFrame *>(option);

            if(APP_K3B==theThemedApp && !(state&(State_Sunken|State_Raised)) && fo && 1==fo->lineWidth)
            {
                painter->save();
                painter->setPen(backgroundColors(option)[QTC_STD_BORDER]);
                drawRect(painter, r);
                painter->restore();
            }
            else if((QtC_StateKWin==state || (QtC_StateKWin|State_Active)==state) && fo && 1==fo->lineWidth && 1==fo->midLineWidth)
            {
                QColor border;
                if(fo->version==(TBAR_BORDER_VERSION_HACK+2))
                    border=palette.color(QPalette::Active, QPalette::Shadow);
                else
                {
                    const QColor *borderCols(opts.windowBorder&WINDOW_BORDER_COLOR_TITLEBAR_ONLY
                                             ? backgroundColors(palette.color(QPalette::Active, QPalette::Window))
                                             : theThemedApp==APP_KWIN
                                             ? buttonColors(option)
                                             : getMdiColors(option, state&State_Active));
                    border=borderCols[fo->version==TBAR_BORDER_VERSION_HACK ? 0 : QTC_STD_BORDER];
                }

                border.setAlphaF(1.0);
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, false);
                painter->setPen(border);
                drawRect(painter, r);
                painter->restore();
            }
            else
            {
                bool kateView(isKateView(widget)),
                    kontactPreview(!kateView && isKontactPreviewPane(widget)),
                    sv(isOOWidget(widget) ||
                       ::qobject_cast<const QAbstractScrollArea *>(widget) ||
                       ((opts.square&SQUARE_SCROLLVIEW) && (kateView || kontactPreview))),
                    squareSv(sv && ((opts.square&SQUARE_SCROLLVIEW) || (widget && widget->isWindow()))),
                    inQAbstractItemView(widget && widget->parentWidget() && isInQAbstractItemView(widget->parentWidget()));

                if(sv && (opts.etchEntry || squareSv || isOOWidget(widget)))
                {
                    // For some reason, in KPackageKit, the KTextBrower when polished is not in the scrollview,
                    // but is when painted. So check here if it should not be etched.
                    // Also, see not in getLowerEtchCol()
                    if(DO_EFFECT && !USE_CUSTOM_ALPHAS(opts) && widget && widget->parentWidget() &&
                       !theNoEtchWidgets.contains(widget) && inQAbstractItemView)
                        theNoEtchWidgets.insert(widget);

                    // If we are set to have sunken scrollviews, then the frame width is set to 3.
                    // ...but it we are a scrollview within a scrollview, then we dont draw sunken, therefore
                    // need to draw inner border...
                    bool doEtch=DO_EFFECT && opts.etchEntry,
                        noEtchW=doEtch && !USE_CUSTOM_ALPHAS(opts) && theNoEtchWidgets.contains(widget);

                    if(doEtch && noEtchW)
                    {
                        painter->setPen(palette.brush(QPalette::Base).color());
                        drawRect(painter, r.adjusted(2, 2, -2, -2));
                    }

                    if(!opts.highlightScrollViews && fo)
                    {
                        QStyleOptionFrame opt(*fo);
                        opt.state&=~State_HasFocus;
                        drawEntryField(painter, r, widget, &opt, squareSv ? ROUNDED_NONE : ROUNDED_ALL, false, doEtch && !noEtchW, WIDGET_SCROLLVIEW);
                    }
                    else
                        drawEntryField(painter, r, widget, option, squareSv ? ROUNDED_NONE : ROUNDED_ALL, false,
                                       doEtch && !noEtchW, WIDGET_SCROLLVIEW);
                }
                // K3b's Disk usage status bar, etc...
//                     else if(APP_K3B==theThemedApp && widget && widget->inherits("K3b::FillStatusDisplay"))
                else if (fo && fo->lineWidth>0)
                {
                    bool         kwinTab(APP_KWIN==theThemedApp &&  widget && !widget->parentWidget() &&
                                         0==strcmp(widget->metaObject()->className(), "KWin::TabBox"));
                    QStyleOption opt(*option);

                    painter->save();

                    if(kwinTab)
                        r.adjust(-1, -1, 1, 1);

                    if(!opts.highlightScrollViews)
                        opt.state&=~State_HasFocus;

                    if(opts.round && IS_FLAT_BGND(opts.bgndAppearance) && 100==opts.bgndOpacity &&
                       widget && widget->parentWidget() && !inQAbstractItemView/* &&
                                                                                  widget->palette().background().color()!=widget->parentWidget()->palette().background().color()*/)
                    {
                        painter->setPen(widget->parentWidget()->palette().background().color());
                        painter->drawRect(r);
                        painter->drawRect(r.adjusted(1, 1, -1, -1));
                    }

                    if(sv || kateView || kontactPreview)
                    {
                        painter->setRenderHint(QPainter::Antialiasing, true);
                        painter->setPen(option->palette.brush(opts.thin&THIN_FRAMES && !(opts.square&SQUARE_SCROLLVIEW)
                                                              ? QPalette::Window
                                                              : QPalette::Base).color());
                        painter->drawPath(buildPath(r.adjusted(1, 1, -1, -1), WIDGET_SCROLLVIEW, ROUNDED_ALL,
                                                    qtcGetRadius(&opts, r.width()-2, r.height()-2, WIDGET_SCROLLVIEW, RADIUS_INTERNAL)));
                        painter->setRenderHint(QPainter::Antialiasing, false);
                    }

                    drawBorder(painter, r, &opt,
                               opts.round ?  getFrameRound(widget) : ROUND_NONE, backgroundColors(option),
                               sv || kateView || kontactPreview ? WIDGET_SCROLLVIEW : WIDGET_FRAME,
                               state&State_Sunken || state&State_HasFocus
                               ? BORDER_SUNKEN
                               : state&State_Raised
                               ? BORDER_RAISED
                               : BORDER_FLAT);
                    painter->restore();
                }
            }
        }
        break;
    }
    case PE_PanelMenuBar:
        if (widget && widget->parentWidget() && (qobject_cast<const QMainWindow *>(widget->parentWidget())))
        {
            painter->save();
            drawMenuOrToolBarBackground(widget, painter, r, option);
            if(TB_NONE!=opts.toolbarBorders)
            {
                const QColor *use=itsActive
                    ? itsMenubarCols
                    : backgroundColors(option);
                bool         dark(TB_DARK==opts.toolbarBorders || TB_DARK_ALL==opts.toolbarBorders);

                if(TB_DARK_ALL==opts.toolbarBorders || TB_LIGHT_ALL==opts.toolbarBorders)
                {
                    painter->setPen(use[0]);
                    painter->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                    painter->drawLine(r.x(), r.y(), r.x(), r.y()+r.height()-1);
                    painter->setPen(use[dark ? 3 : 4]);
                    painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                    painter->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
                }
                else
                {
                    painter->setPen(use[dark ? 3 : 4]);
                    painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                }
            }
            painter->restore();
        }
        break;
    case PE_FrameTabBarBase:
        if (const QStyleOptionTabBarBase *tbb = qstyleoption_cast<const QStyleOptionTabBarBase *>(option))
        {
            if(tbb->shape != QTabBar::RoundedNorth && tbb->shape != QTabBar::RoundedWest &&
               tbb->shape != QTabBar::RoundedSouth && tbb->shape != QTabBar::RoundedEast)
                BASE_STYLE::drawPrimitive(element, option, painter, widget);
            else
            {
                static const int constSidePad=16*2;
                const QColor *use(backgroundColors(option));
                QRegion      region(tbb->rect);
                QLine        topLine(tbb->rect.bottomLeft() - QPoint(0, 1), tbb->rect.bottomRight() - QPoint(0, 1)),
                    bottomLine(tbb->rect.bottomLeft(), tbb->rect.bottomRight());
                bool         horiz(QTabBar::RoundedNorth==tbb->shape || QTabBar::RoundedSouth==tbb->shape);
                double       size=horiz ? tbb->rect.width() : tbb->rect.height(),
                    tabRectSize=horiz ? tbb->tabBarRect.width() : tbb->tabBarRect.height(),
                    tabFadeSize=tabRectSize+constSidePad > size ? 0.0 : 1.0-((tabRectSize+constSidePad)/size),
                    minFadeSize=1.0-((size-constSidePad)/size),
                    fadeSizeStart=minFadeSize,
                    fadeSizeEnd=tabFadeSize<minFadeSize ? minFadeSize : (tabFadeSize>FADE_SIZE ? FADE_SIZE : tabFadeSize);

                if(reverse && horiz)
                    fadeSizeStart=fadeSizeEnd, fadeSizeEnd=minFadeSize;

                region -= tbb->tabBarRect;

                painter->save();
                painter->setClipRegion(region);
                bool fadeState=true, fadeEnd=true;

                // Dont fade start/end of tabbar in KDevelop's menubar
                if(APP_KDEVELOP==theThemedApp && widget && widget->parentWidget() && widget->parentWidget()->parentWidget() &&
                   qobject_cast<const QTabBar *>(widget) && qobject_cast<const QMenuBar *>(widget->parentWidget()->parentWidget()))
                    fadeState=fadeEnd=false;

                drawFadedLine(painter, QRect(topLine.p1(), topLine.p2()),
                              QTabBar::RoundedSouth==tbb->shape && APPEARANCE_FLAT==opts.appearance
                              ? palette.background().color()
                              : use[QTabBar::RoundedNorth==tbb->shape ? QTC_STD_BORDER
                                    : (opts.borderTab ? 0 : FRAME_DARK_SHADOW)],
                              fadeState, fadeEnd, horiz, fadeSizeStart, fadeSizeEnd);
                if(!(opts.thin&THIN_FRAMES))
                    drawFadedLine(painter, QRect(bottomLine.p1(), bottomLine.p2()),
                                  use[QTabBar::RoundedNorth==tbb->shape ? 0 : QTC_STD_BORDER],
                                  fadeState, fadeEnd, horiz, fadeSizeStart, fadeSizeEnd);
                painter->restore();
            }
        }
        break;
    case PE_FrameStatusBar:
        if(!opts.drawStatusBarFrames)
            break;
    case PE_FrameMenu:
        if((opts.square&SQUARE_POPUP_MENUS) &&
           (IS_FLAT_BGND(opts.menuBgndAppearance) ||
            (opts.gtkComboMenus && widget && widget->parent() && qobject_cast<const QComboBox *>(widget->parent()))))
        {
            const QColor    *use(popupMenuCols(option));
            EGradientBorder border=qtcGetGradient(opts.menuBgndAppearance, &opts)->border;
            painter->save();
            painter->setPen(use[QTC_STD_BORDER]);
            drawRect(painter, r);

            if(USE_BORDER(border) && APPEARANCE_FLAT!=opts.menuBgndAppearance)
            {
                painter->setPen(use[0]);
                if(GB_LIGHT==border)
                    drawRect(painter, r.adjusted(1, 1, -1, -1));
                else
                {
                    if(GB_3D!=border)
                    {
                        painter->drawLine(r.x()+1, r.y()+1, r.x()+r.width()-2,  r.y()+1);
                        painter->drawLine(r.x()+1, r.y()+1, r.x()+1,  r.y()+r.height()-2);
                    }
                    painter->setPen(use[FRAME_DARK_SHADOW]);
                    painter->drawLine(r.x()+1, r.y()+r.height()-2, r.x()+r.width()-2,  r.y()+r.height()-2);
                    painter->drawLine(r.x()+r.width()-2, r.y()+1, r.x()+r.width()-2,  r.y()+r.height()-2);
                }
            }
            painter->restore();
        }
        break;
    case PE_FrameDockWidget:
    {
        const QColor *use(backgroundColors(option));

        painter->save();
        painter->setPen(use[0]);
        painter->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
        painter->drawLine(r.x(), r.y(), r.x(), r.y()+r.height()-1);
        painter->setPen(use[APPEARANCE_FLAT==opts.appearance ? ORIGINAL_SHADE : QTC_STD_BORDER]);
        painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
        painter->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
        painter->restore();
        break;
    }
    case PE_FrameButtonTool:
    case PE_PanelButtonTool:
        if(isMultiTabBarTab(getButton(widget, painter)))
        {
            if(!opts.stdSidebarButtons)
                drawSideBarButton(painter, r, option, widget);
            else if( (state&State_Enabled) || !(state&State_AutoRaise) )
            {
                QStyleOption opt(*option);
                opt.state|=STATE_TBAR_BUTTON;
                drawPrimitive(PE_PanelButtonCommand, &opt, painter, widget);
            }
            break;
        }
    case PE_IndicatorButtonDropDown: // This should never be called, but just in case - draw as a normal toolbutton...
    {
        bool dwt(widget && widget->inherits("QDockWidgetTitleButton")),
            koDwt(!dwt && widget && widget->parentWidget() && widget->parentWidget()->inherits("KoDockWidgetTitleBar"));

        if( ((state&State_Enabled) || !(state&State_AutoRaise)) &&
            (!widget || !(dwt || koDwt)|| (state&State_MouseOver)) )
        {
            QStyleOption opt(*option);

            if(dwt || koDwt)
                opt.state|=STATE_DWT_BUTTON;
            drawPrimitive(PE_PanelButtonCommand, &opt, painter, widget);
        }
        break;
    }
    case PE_IndicatorDockWidgetResizeHandle:
    {
        QStyleOption dockWidgetHandle = *option;
        bool horizontal = state&State_Horizontal;
        if (horizontal)
            dockWidgetHandle.state &= ~State_Horizontal;
        else
            dockWidgetHandle.state |= State_Horizontal;
        drawControl(CE_Splitter, &dockWidgetHandle, painter, widget);
        break;
    }
    case PE_PanelLineEdit:
        if (const QStyleOptionFrame *panel = qstyleoption_cast<const QStyleOptionFrame *>(option))
        {
            if(panel->lineWidth > 0)
            {
                QRect r2(r.adjusted(1, 1, -1, (DO_EFFECT ? -2 : -1)));
                painter->fillPath(buildPath(r2, WIDGET_ENTRY, ROUNDED_ALL, qtcGetRadius(&opts, r2.width(), r2.height(), WIDGET_ENTRY, RADIUS_INTERNAL)),
                                  palette.brush(QPalette::Base));
                drawPrimitive(PE_FrameLineEdit, option, painter, widget);
            }
            else
                painter->fillRect(r.adjusted(2, 2, -2, -2), palette.brush(QPalette::Base));
        }
        break;
    case PE_FrameLineEdit:
        if (const QStyleOptionFrame *lineEdit = qstyleoption_cast<const QStyleOptionFrame *>(option))
        {
            if ((lineEdit->lineWidth>0 || isOOWidget(widget)) &&
                !(widget &&
                  (qobject_cast<const QComboBox *>(widget->parentWidget()) ||
                   qobject_cast<const QAbstractSpinBox *>(widget->parentWidget()))))
            {
                QStyleOptionFrame opt(*lineEdit);

                if(opt.state&State_Enabled && state&State_ReadOnly)
                    opt.state^=State_Enabled;

                if(DO_EFFECT && opts.etchEntry && APP_ARORA==theThemedApp && widget &&
                   widget->parentWidget() && 0==strcmp(widget->metaObject()->className(), "LocationBar"))
                {
                    const QToolBar *tb=getToolBar(widget->parentWidget()/*, false*/);

                    if(tb)
                    {
                        QRect r2(r);

                        struct TB : public QToolBar
                        {
                            void initOpt(QStyleOptionToolBar *opt) { initStyleOption(opt); }
                        };

                        QStyleOptionToolBar opt;

                        ((TB *)tb)->initOpt(&opt);

                        painter->save();

                        // Only need to adjust coords if toolbar has a gradient...
                        if(!IS_FLAT(opts.toolbarAppearance))
                        {
                            r2.setY(-widget->mapTo((QWidget *)tb, QPoint(r.x(), r.y())).y());
                            r2.setHeight(tb->rect().height());
                        }
                        painter->setClipRegion(QRegion(r2).subtracted(QRegion(r2.adjusted(2, 2, -2, -2))));
                        drawMenuOrToolBarBackground(widget, painter, r2, &opt, false, true);
                        painter->restore();
                    }
                }
                painter->save();
                bool  isOO(isOOWidget(widget));
                QRect rect(r);
                int   round(ROUNDED_ALL);

                if(isOO)
                {
                    // This (hopefull) checks is we're OO.o 3.2 - in which case no adjustment is required...
                    const QImage *img=getImage(painter);

                    if(!img || img->rect()!=r) // OO.o 3.1?
                        rect.adjust(1, 2, -1, -2);
                    else
                    {
                        round=ROUNDED_NONE;
                        painter->fillRect(r, palette.brush(QPalette::Window));
                        rect.adjust(1, 1, -1, -1);
                    }
                }

                drawEntryField(painter, rect, widget, &opt, round, isOO, !isOO && DO_EFFECT);
                painter->restore();
            }
        }
        break;
    case PE_IndicatorMenuCheckMark:
    case PE_IndicatorCheckBox:
    {
        bool  menu(state&STATE_MENU),
            view(state&STATE_VIEW),
            doEtch(DO_EFFECT &&
                   (opts.crButton ||(PE_IndicatorMenuCheckMark!=element && !menu &&
                                     r.width()>=opts.crSize+2 && r.height()>=opts.crSize+2))),
            isOO(isOOWidget(widget)),
            selectedOOMenu(isOO && (r==QRect(0, 0, 15, 15) || r==QRect(0, 0, 14, 15)) &&  // OO.o 3.2 =14x15?
                           ((State_Sunken|State_Enabled)==state || (State_Sunken|State_Enabled|State_Selected)==state));
        int   crSize(opts.crSize+(doEtch ? 2 : 0));
        QRect rect(r.x(), r.y()+(view ? -1 : 0), crSize, crSize);

        painter->save();

        // For OO.o 3.2 need to fill widget background!
        if(isOO)
            painter->fillRect(r, palette.brush(QPalette::Window));

        if(selectedOOMenu)
        {
            if(r==QRect(0, 0, 14, 15)) // OO.o 3.2 =14x15?
                rect.adjust(-1, -1, -1, -1);
            painter->setPen(option ? option->palette.text().color() : QApplication::palette().text().color());
            drawRect(painter, r);
            // LibreOffice its 15x15 - and arrow is not centred, so adjust this...
            if(r==QRect(0, 0, 15, 15))
                rect.adjust(-1, -1, -1, -1);
        }
        else
        {
            if(isOO && r==QRect(0, 0, opts.crSize, opts.crSize))
                rect.adjust(0, -1, 0, -1);

            if(CR_SMALL_SIZE!=opts.crSize)
            {
                if(menu)
                    rect.adjust(0, -1, 0, -1);
                else if(r.height()>crSize)   // Can only adjust position if there is space!
                    rect.adjust(0, 1, 0, 1); // ...when used in a listview, usually there is no space.
            }

            if(opts.crButton)
            {
                const QColor *use(checkRadioColors(option));
                QStyleOption opt(*option);

                if(menu || selectedOOMenu)
                    opt.state&=~(State_MouseOver|State_Sunken);
                opt.state&=~State_On;
                opt.state|=State_Raised;
                opt.rect=rect;
                drawLightBevel(painter, rect, &opt, widget, ROUNDED_ALL, getFill(&opt, use, true, false),
                               use, true, WIDGET_CHECKBOX);
            }
            else
            {
                bool         sunken(!menu && !selectedOOMenu && (state&State_Sunken)),
                    mo(!sunken && state&State_MouseOver && state&State_Enabled),
                    glow(doEtch && MO_GLOW==opts.coloredMouseOver && mo);
                const QColor *bc(sunken ? 0L : borderColors(option, 0L)),
                    *btn(checkRadioColors(option)),
                    *use(bc ? bc : btn);
                const QColor &bgnd(state&State_Enabled && !sunken
                                   ? MO_NONE==opts.coloredMouseOver && !opts.crHighlight && mo
                                   ? use[CR_MO_FILL]
                                   : palette.base().color()
                                   : palette.background().color());
                bool         lightBorder=DRAW_LIGHT_BORDER(false, WIDGET_TROUGH, APPEARANCE_INVERTED);

                rect=QRect(doEtch ? rect.adjusted(1, 1, -1, -1) : rect);

                if(IS_FLAT(opts.appearance))
                    painter->fillRect(rect.adjusted(1, 1, -1, -1), bgnd);
                else
                    drawBevelGradient(bgnd, painter, rect.adjusted(1, 1, -1, -1), true, false, APPEARANCE_INVERTED, WIDGET_TROUGH);

                if(MO_NONE!=opts.coloredMouseOver && !glow && mo)
                {
                    painter->setRenderHint(QPainter::Antialiasing, true);
                    painter->setPen(use[CR_MO_FILL]);
                    drawAaRect(painter, rect.adjusted(1, 1, -1, -1));
                    painter->setRenderHint(QPainter::Antialiasing, false);
                }
                else
                {
                    painter->setPen(midColor(state&State_Enabled ? palette.base().color()
                                             : palette.background().color(), use[3]));
                    if(lightBorder)
                        drawRect(painter, rect.adjusted(1, 1, -1, -1));
                    else
                    {
                        painter->drawLine(rect.x()+1, rect.y()+1, rect.x()+1, rect.y()+rect.height()-2);
                        painter->drawLine(rect.x()+1, rect.y()+1, rect.x()+rect.width()-2, rect.y()+1);
                    }
                }

                if(doEtch && !view)
                {
                    if(glow && !(opts.thin&THIN_FRAMES))
                        drawGlow(painter, r, WIDGET_CHECKBOX);
                    else
                        drawEtch(painter, r, widget, WIDGET_CHECKBOX, opts.crButton && EFFECT_SHADOW==opts.buttonEffect ? !sunken : false);
                }

                drawBorder(painter, rect, option, ROUNDED_ALL, use, WIDGET_CHECKBOX);
            }
        }

        if(state&State_On || selectedOOMenu)
        {
            QPixmap *pix(getPixmap(checkRadioCol(option), PIX_CHECK, 1.0));

            painter->drawPixmap(rect.center().x()-(pix->width()/2), rect.center().y()-(pix->height()/2), *pix);
        }
        else if (state&State_NoChange)    // tri-state
        {
            int x(rect.center().x()), y(rect.center().y());

            painter->setPen(checkRadioCol(option));
            painter->drawLine(x-3, y, x+3, y);
            painter->drawLine(x-3, y+1, x+3, y+1);
        }

        painter->restore();
        break;
    }
    case PE_IndicatorRadioButton:
    {
        bool isOO(isOOWidget(widget)),
            selectedOOMenu(isOO && (r==QRect(0, 0, 15, 15) || r==QRect(0, 0, 14, 15)) &&  // OO.o 3.2 =14x15?
                           ((State_Sunken|State_Enabled)==state || (State_Sunken|State_Enabled|State_Selected)==state));

        if(isOO)
            painter->fillRect(r, palette.brush(QPalette::Background));

        if(selectedOOMenu)
            drawPrimitive(PE_IndicatorCheckBox, option, painter, widget);
        else
        {
            bool menu(state&STATE_MENU);
            int  x(r.x()), y(r.y());

            painter->save();

            if(opts.crButton)
            {
                const QColor *use(checkRadioColors(option));
                QStyleOption opt(*option);
                bool         doEtch(DO_EFFECT);
                QRect        rect(r.x(), r.y(), opts.crSize+(doEtch ? 2 : 0), opts.crSize+(doEtch ? 2 : 0));

                if(CR_SMALL_SIZE!=opts.crSize && menu)
                    rect.adjust(0, -1, 0, -1), y++;

                if(isOO && r==QRect(0, 0, opts.crSize, opts.crSize))
                    rect.adjust(-1, -1, -1, -1), --x, --y;

                if(menu || selectedOOMenu)
                    opt.state&=~(State_MouseOver|State_Sunken);
                opt.state&=~State_On;
                opt.state|=State_Raised;
                opt.rect=rect;

                if(doEtch)
                    x++, y++;
                if(CR_SMALL_SIZE!=opts.crSize && menu)
                    y-=2;

                drawLightBevel(painter, rect, &opt, widget, ROUNDED_ALL, getFill(&opt, use, true, false), use, true, WIDGET_RADIO_BUTTON);
            }
            else
            {
                bool         sunken(!menu && !selectedOOMenu && (state&State_Sunken)),
                    doEtch(!menu
                           && r.width()>=opts.crSize+2 && r.height()>=opts.crSize+2
                           && DO_EFFECT),
                    mo(!sunken && state&State_MouseOver && state&State_Enabled),
                    glow(doEtch && MO_GLOW==opts.coloredMouseOver && mo),
                    coloredMo(MO_NONE!=opts.coloredMouseOver && !glow && mo && !sunken);
                bool         lightBorder=DRAW_LIGHT_BORDER(false, WIDGET_TROUGH, APPEARANCE_INVERTED),
                    doneShadow=false;
                QRect        rect(doEtch ? r.adjusted(1, 1, -1, -1) : r);
                const QColor *bc(sunken ? 0L : borderColors(option, 0L)),
                    *btn(checkRadioColors(option)),
                    *use(bc ? bc : btn);

                if(doEtch)
                    x++, y++;

                const QColor &bgnd(state&State_Enabled && !sunken
                                   ? MO_NONE==opts.coloredMouseOver && !opts.crHighlight && mo
                                   ? use[CR_MO_FILL]
                                   : palette.base().color()
                                   : palette.background().color());
                QPainterPath path;

                path.addEllipse(QRectF(rect).adjusted(0.5, 0.5, -1.0, -1.0));
                drawBevelGradient(bgnd, painter, rect.adjusted(1, 1, -1, -1), path, true, false, APPEARANCE_INVERTED, WIDGET_TROUGH);
                painter->setRenderHint(QPainter::Antialiasing, true);
                if(coloredMo)
                {
                    painter->setBrush(Qt::NoBrush);
                    painter->setPen(use[CR_MO_FILL]);
                    painter->drawArc(QRectF(x+1, y+1, opts.crSize-2, opts.crSize-2), 0, 360*16);
                    painter->drawArc(QRectF(x+2, y+2, opts.crSize-4, opts.crSize-4), 0, 360*16);
                }

                painter->setBrush(Qt::NoBrush);
                if(!doneShadow && doEtch && (glow || EFFECT_NONE!=opts.buttonEffect || sunken))
                {
                    QColor topCol(glow ? itsMouseOverCols[GLOW_MO] : Qt::black);

                    if(!glow)
                        topCol.setAlphaF(ETCH_RADIO_TOP_ALPHA);

                    painter->setPen(topCol);
                    painter->drawArc(QRectF(x-0.5, y-0.5, opts.crSize+1, opts.crSize+1), 45*16, 180*16);
                    if(!glow)
                        painter->setPen(getLowerEtchCol(widget));
                    painter->drawArc(QRectF(x-0.5, y-0.5, opts.crSize+1, opts.crSize+1), 225*16, 180*16);
                }

                painter->setPen(use[BORDER_VAL(state&State_Enabled)]);
                painter->drawArc(QRectF(x+0.25, y+0.25, opts.crSize-0.5, opts.crSize-0.5), 0, 360*16);
                if(!coloredMo)
                {
                    painter->setPen(btn[state&State_MouseOver ? 3 : 4]);
                    painter->drawArc(QRectF(x+0.75, y+0.75, opts.crSize-1.5, opts.crSize-1.5), lightBorder ? 0 : 45*16, lightBorder ? 360*16 : 180*16);
                }
            }
            if(state&State_On || selectedOOMenu)
            {
                QPainterPath path;
                double       radius=opts.smallRadio ? 2.75 : 3.75,
                    offset=(opts.crSize/2.0)-radius;

                path.addEllipse(QRectF(x+offset, y+offset, radius*2.0, radius*2.0));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->fillPath(path, checkRadioCol(option));
            }

            painter->restore();
        }
        break;
    }
    case PE_IndicatorToolBarHandle:
        painter->save();
        drawHandleMarkers(painter, r, option, true, opts.handles);
        painter->restore();
        break;
    case PE_FrameFocusRect:
        if (const QStyleOptionFocusRect *focusFrame = qstyleoption_cast<const QStyleOptionFocusRect *>(option))
        {
            if (!(focusFrame->state&State_KeyboardFocusChange) ||
                (widget && widget->inherits("QComboBoxListView")))
                return;

            if(widget && FOCUS_GLOW==opts.focus)
            {
                if(::qobject_cast<const QAbstractButton *>(widget))
                {
                    if(!::qobject_cast<const QToolButton *>(widget) || !static_cast<const QToolButton *>(widget)->autoRaise())
                        return;
                }
                else if(::qobject_cast<const QComboBox *>(widget) || ::qobject_cast<const QGroupBox *>(widget) ||
                        ::qobject_cast<const QDial *>(widget))
                    return;
            }

            QRect r2(r);

            if(widget && (::qobject_cast<const QCheckBox *>(widget) || ::qobject_cast<const QRadioButton *>(widget)) &&
               ((QAbstractButton *)widget)->text().isEmpty() && r.height()<=widget->rect().height()-2 && r.width()<=widget->rect().width()-2 &&
               r.x()>=1 && r.y()>=1)
            {
                int adjust=qMin(qMin(abs(widget->rect().x()-r.x()), 2), abs(widget->rect().y()-r.y()));
                r2.adjust(-adjust, -adjust, adjust, adjust);
            }

            if(widget && ::qobject_cast<const QGroupBox *>(widget))
                r2.adjust(0, 2, 0, 0);

            if(FOCUS_STANDARD==opts.focus)
            {
                // Taken from QWindowsStyle...
                painter->save();
                painter->setBackgroundMode(Qt::TransparentMode);
                QColor bgCol(focusFrame->backgroundColor);
                if (!bgCol.isValid())
                    bgCol = painter->background().color();
                // Create an "XOR" color.
                QColor patternCol((bgCol.red() ^ 0xff) & 0xff,
                                  (bgCol.green() ^ 0xff) & 0xff,
                                  (bgCol.blue() ^ 0xff) & 0xff);
                painter->setBrush(QBrush(patternCol, Qt::Dense4Pattern));
                painter->setBrushOrigin(r.topLeft());
                painter->setPen(Qt::NoPen);
                painter->drawRect(r.left(), r.top(), r.width(), 1);    // Top
                painter->drawRect(r.left(), r.bottom(), r.width(), 1); // Bottom
                painter->drawRect(r.left(), r.top(), 1, r.height());   // Left
                painter->drawRect(r.right(), r.top(), 1, r.height());  // Right
                painter->restore();
            }
            else
            {
                //Figuring out in what beast we are painting...
                bool view(state&State_Item ||
                          ((widget && (qobject_cast<const QAbstractScrollArea*>(widget))) ||
                           (widget && widget->parent() &&
                            (qobject_cast<const QAbstractScrollArea*>(widget->parent())))));

                if(!view && !widget)
                {
                    // Try to determine if we are in a KPageView...
                    const QWidget *wid=getWidget(painter);

                    if(wid && wid->parentWidget())
                    {
                        if(wid->parentWidget()->inherits("KDEPrivate::KPageListView"))
                        {
                            r2.adjust(2, 2, -2, -2);
                            view=true;
                        }
                        else if(APP_KONTACT==theThemedApp && (wid->parentWidget()->inherits("KMail::MainFolderView") ||
                                                              wid->parentWidget()->inherits("MessageList::Core::View")))
                        {
                            view=true;
                        }
                    }
                }
                painter->save();
                QColor c(view && state&State_Selected
                         ? palette.highlightedText().color()
                         : itsFocusCols[FOCUS_SHADE(state&State_Selected)]);

                if(FOCUS_LINE==opts.focus || FOCUS_GLOW==opts.focus)
                {
                    if(!(state&State_Horizontal) && widget && qobject_cast<const QTabBar *>(widget))
                        drawFadedLine(painter, QRect(r2.x()+r2.width()-1, r2.y(), 1, r2.height()), c, true, true, false);
                    else
                        drawFadedLine(painter, QRect(r2.x(), r2.y()+r2.height()-(view ? 3 : 1), r2.width(), 1), c, true, true, true);
                }
                else
                {
                    //if(FOCUS_GLOW==opts.focus)
                    //    c.setAlphaF(FOCUS_GLOW_LINE_ALPHA);
                    painter->setPen(c);
                    if(FOCUS_FILLED==opts.focus)
                    {
                        c.setAlphaF(FOCUS_ALPHA);
                        painter->setBrush(c);
                    }

                    if(ROUNDED)
                    {
                        bool square((opts.square&SQUARE_LISTVIEW_SELECTION) &&
                                    ( (/*(!widget && r.height()<=40 && r.width()>=48) || */
                                        (widget && !widget->inherits("KFilePlacesView") &&
                                         (qobject_cast<const QTreeView *>(widget) ||
                                          (qobject_cast<const QListView *>(widget) &&
                                           QListView::IconMode!=((const QListView *)widget)->viewMode())))) ||
                                      (!widget && view) ) );

                        painter->setRenderHint(QPainter::Antialiasing, true);
                        painter->drawPath(buildPath(r2, WIDGET_SELECTION, ROUNDED_ALL,
                                                    square ? SLIGHT_INNER_RADIUS : qtcGetRadius(&opts, r2.width(), r2.height(), WIDGET_OTHER,
                                                                                                FULL_FOCUS ? RADIUS_EXTERNAL : RADIUS_SELECTION)));
                    }
                    else
                        drawRect(painter, r2);
                }
                painter->restore();
            }
        }
        break;
    case PE_FrameButtonBevel:
    case PE_PanelButtonBevel:
    case PE_PanelButtonCommand:
    {
        if(state&STATE_DWT_BUTTON && (opts.dwtSettings&DWT_BUTTONS_AS_PER_TITLEBAR))
            break;

        bool doEtch(DO_EFFECT);

        // This fixes the "Sign in" button at mail.lycos.co.uk
        // ...basically if KHTML gices us a fully transparent background colour, then
        // dont paint the button.
        if(0==option->palette.button().color().alpha())
        {
            if(state&State_MouseOver && state&State_Enabled && MO_GLOW==opts.coloredMouseOver && doEtch && !(opts.thin&THIN_FRAMES))
                drawGlow(painter, r, WIDGET_STD_BUTTON);
            return;
        }

        if(!widget)
            widget=getWidget(painter);

        const QColor *use(buttonColors(option));
        bool         isDefault(false),
            isFlat(false),
            isKWin(state&QtC_StateKWin),
            isDown(state&State_Sunken || state&State_On),
            isOnListView(!isKWin && widget && qobject_cast<const QAbstractItemView *>(widget));
        QStyleOption opt(*option);

        if(PE_PanelButtonBevel==element)
            opt.state|=State_Enabled;

        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton*>(option))
        {
            isDefault = (button->features & QStyleOptionButton::DefaultButton) && (button->state&State_Enabled);
            isFlat = (button->features & QStyleOptionButton::Flat);
        }

        if(!(opt.state&State_Enabled))
            opt.state&=~State_MouseOver;

        // For some reason with OO.o not all buttons are set as raised!
        if(!(opt.state&State_AutoRaise))
            opt.state|=State_Raised;

        isDefault=isDefault || (doEtch && FULL_FOCUS && MO_GLOW==opts.coloredMouseOver &&
                                opt.state&State_HasFocus && opt.state&State_Enabled);
        if(isFlat && !isDown && !(opt.state&State_MouseOver))
            return;

        painter->save();

        if(isOnListView)
            opt.state|=State_Horizontal|State_Raised;

        if(isDefault && state&State_Enabled && (IND_TINT==opts.defBtnIndicator || IND_SELECTED==opts.defBtnIndicator))
            use=itsDefBtnCols;
        else if(state&STATE_DWT_BUTTON && widget && opts.titlebarButtons&TITLEBAR_BUTTON_COLOR &&
                coloredMdiButtons(state&State_Active, state&State_MouseOver) &&
                !(opts.titlebarButtons&TITLEBAR_BUTTON_COLOR_SYMBOL))
        {
            if(constDwtClose==widget->objectName())
                use=itsTitleBarButtonsCols[TITLEBAR_CLOSE];
            else if(constDwtFloat==widget->objectName())
                use=itsTitleBarButtonsCols[TITLEBAR_MAX];
            else if(widget->parentWidget() && widget->parentWidget()->parentWidget() &&
                    widget->parentWidget()->inherits("KoDockWidgetTitleBar") &&
                    ::qobject_cast<QDockWidget *>(widget->parentWidget()->parentWidget()))
            {
                QDockWidget              *dw   = (QDockWidget *)widget->parentWidget()->parentWidget();
                QWidget                  *koDw = widget->parentWidget();
                int                      fw    = dw->isFloating()
                    ? pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, dw)
                    : 0;
                QRect                    geom(widget->geometry());
                QStyleOptionDockWidget dwOpt;
                dwOpt.initFrom(dw);
                dwOpt.rect = QRect(QPoint(fw, fw), QSize(koDw->geometry().width() - (fw * 2),
                                                         koDw->geometry().height() - (fw * 2)));
                dwOpt.title = dw->windowTitle();
                dwOpt.closable = (dw->features()&QDockWidget::DockWidgetClosable)==QDockWidget::DockWidgetClosable;
                dwOpt.floatable = (dw->features()&QDockWidget::DockWidgetFloatable)==QDockWidget::DockWidgetFloatable;

                if(dwOpt.closable &&
                   subElementRect(QStyle::SE_DockWidgetCloseButton, &dwOpt, widget->parentWidget()->parentWidget())==geom)
                    use=itsTitleBarButtonsCols[TITLEBAR_CLOSE];
                else if(dwOpt.floatable &&
                        subElementRect(QStyle::SE_DockWidgetFloatButton, &dwOpt,
                                       widget->parentWidget()->parentWidget())==geom)
                    use=itsTitleBarButtonsCols[TITLEBAR_MAX];
                else
                    use=itsTitleBarButtonsCols[TITLEBAR_SHADE];
            }
        }

        if(isKWin)
            opt.state|=STATE_KWIN_BUTTON;

        bool coloredDef=isDefault && state&State_Enabled && IND_COLORED==opts.defBtnIndicator;

        if(widget && qobject_cast<const QAbstractButton *>(widget) && (static_cast<const QAbstractButton *>(widget))->isCheckable())
            opt.state|=STATE_TOGGLE_BUTTON;

        drawLightBevel(painter, r, &opt, widget, ROUNDED_ALL,
                       coloredDef ? itsDefBtnCols[MO_DEF_BTN]
                       : getFill(&opt, use, false,
                                 isDefault && state&State_Enabled && IND_DARKEN==opts.defBtnIndicator),
                       coloredDef ? itsDefBtnCols : use,
                       true, isKWin || state&STATE_DWT_BUTTON
                       ? WIDGET_MDI_WINDOW_BUTTON
                       : isOnListView
                       ? WIDGET_NO_ETCH_BTN
                       : isDefault && state&State_Enabled
                       ? WIDGET_DEF_BUTTON
                       : state&STATE_TBAR_BUTTON
                       ? WIDGET_TOOLBAR_BUTTON
                       : WIDGET_STD_BUTTON);

        if (isDefault && state&State_Enabled)
            switch(opts.defBtnIndicator)
            {
            case IND_CORNER:
            {
                QPainterPath path;
                int          offset(isDown ? 5 : 4),
                    etchOffset(doEtch ? 1 : 0);
                double       xd(r.x()+0.5),
                    yd(r.y()+0.5);
                const QColor *cols(itsFocusCols ? itsFocusCols : itsHighlightCols);

                path.moveTo(xd+offset+etchOffset, yd+offset+etchOffset);
                path.lineTo(xd+offset+6+etchOffset, yd+offset+etchOffset);
                path.lineTo(xd+offset+etchOffset, yd+offset+6+etchOffset);
                path.lineTo(xd+offset+etchOffset, yd+offset+etchOffset);
                painter->setBrush(cols[isDown ? 0 : 4]);
                painter->setPen(cols[isDown ? 0 : 4]);
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawPath(path);
                painter->setRenderHint(QPainter::Antialiasing, false);
                break;
            }
            case IND_COLORED:
            {
                int   offset=COLORED_BORDER_SIZE+(doEtch ? 1 : 0);
                QRect r2(r.adjusted(offset, offset, -offset, -offset));

                drawBevelGradient(getFill(&opt, use), painter, r2, true, state &(State_On | State_Sunken), opts.appearance, WIDGET_STD_BUTTON);
            }
            default:
                break;
            }
        painter->restore();
        break;
    }
    case PE_FrameDefaultButton:
        break;
    case PE_FrameWindow:
    {
        bool         colTbarOnly=opts.windowBorder&WINDOW_BORDER_COLOR_TITLEBAR_ONLY,
            fillBgnd=!(state&QtC_StateKWin) && !itsIsPreview && !IS_FLAT_BGND(opts.bgndAppearance);
        const QColor *bgndCols(colTbarOnly || fillBgnd ? backgroundColors(palette.color(QPalette::Active, QPalette::Window)) : 0L),
            *borderCols(colTbarOnly
                        ? bgndCols
                        : theThemedApp==APP_KWIN
                        ? buttonColors(option)
                        : getMdiColors(option, state&State_Active));
        QColor       light(borderCols[0]),
            dark(option->version==(TBAR_BORDER_VERSION_HACK+2)
                 ? palette.color(QPalette::Active, QPalette::Shadow)
                 : borderCols[option && option->version==TBAR_BORDER_VERSION_HACK ? 0 : QTC_STD_BORDER]);
        bool         isKWin=state&QtC_StateKWin,
            addLight=opts.windowBorder&WINDOW_BORDER_ADD_LIGHT_BORDER && (!isKWin || qtcGetWindowBorderSize(false).sides>1);

        light.setAlphaF(1.0);
        dark.setAlphaF(1.0);

        painter->save();

        if(fillBgnd)
            painter->fillRect(r, bgndCols[ORIGINAL_SHADE]);
        if(opts.round<ROUND_SLIGHT || !isKWin || (state&QtC_StateKWinNotFull && state&QtC_StateKWin))
        {
            painter->setRenderHint(QPainter::Antialiasing, false);

            if(addLight)
            {
                painter->setPen(light);
                painter->drawLine(r.x()+1, r.y(), r.x()+1, r.y()+r.height()-1);
            }
            painter->setPen(dark);
            drawRect(painter, r);
        }
        else
        {
            if(addLight)
            {
                painter->setRenderHint(QPainter::Antialiasing, false);
                painter->setPen(light);
                painter->drawLine(r.x()+1, r.y(), r.x()+1, r.y()+r.height()-(1+(opts.round>ROUND_SLIGHT && state&QtC_StateKWin ? 3 : 1)));
            }
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(dark);
            painter->drawPath(buildPath(r, WIDGET_OTHER, ROUNDED_ALL,
                                        opts.round>ROUND_SLIGHT && state&QtC_StateKWin
                                        ? 6.0
                                        : 2.0));

            if(FULLLY_ROUNDED && !(state&QtC_StateKWinCompositing))
            {
                QColor col(opts.windowBorder&WINDOW_BORDER_COLOR_TITLEBAR_ONLY
                           ? backgroundColors(option)[QTC_STD_BORDER]
                           : buttonColors(option)[QTC_STD_BORDER]);

                painter->setRenderHint(QPainter::Antialiasing, false);
                painter->setPen(col);
                painter->drawPoint(r.x()+2, r.y()+r.height()-3);
                painter->drawPoint(r.x()+r.width()-3, r.y()+r.height()-3);
                painter->drawLine(r.x()+1, r.y()+r.height()-5, r.x()+1, r.y()+r.height()-4);
                painter->drawLine(r.x()+3, r.y()+r.height()-2, r.x()+4, r.y()+r.height()-2);
                painter->drawLine(r.x()+r.width()-2, r.y()+r.height()-5, r.x()+r.width()-2, r.y()+r.height()-4);
                painter->drawLine(r.x()+r.width()-4, r.y()+r.height()-2, r.x()+r.width()-5, r.y()+r.height()-2);
            }
        }
        painter->restore();
        break;
    }
    case PE_FrameTabWidget:
    {
        int round(opts.square&SQUARE_TAB_FRAME ? ROUNDED_NONE : ROUNDED_ALL);

        painter->save();

        if(const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option))
            if((opts.round || (/*CUSTOM_BGND && */0==opts.tabBgnd)) &&
               widget && ::qobject_cast<const QTabWidget *>(widget))
            {
                struct QtcTabWidget : public QTabWidget
                {
                    bool  tabsVisible()    const { return tabBar() && tabBar()->isVisible(); }
                    QRect currentTabRect() const { return tabBar()->tabRect(tabBar()->currentIndex()); }
                };

                const QTabWidget *tw((const QTabWidget *)widget);

                if(tw->count()>0 && ((const QtcTabWidget *)widget)->tabsVisible())
                {
                    if(!reverse && /*CUSTOM_BGND && */0==opts.tabBgnd) // Does not work for reverse :-(
                    {
                        QRect tabRect(((const QtcTabWidget *)widget)->currentTabRect());
                        int   adjust(TAB_MO_GLOW==opts.tabMouseOver && !(opts.thin&THIN_FRAMES) ? 2 : 1);

                        switch(tw->tabPosition())
                        {
                        case QTabWidget::South:
                            tabRect=QRect(tabRect.x()+adjust, r.y()+r.height()-2, tabRect.width()-(2*adjust), 4);
                            break;
                        case QTabWidget::North:
                        {
                            int leftAdjust=twf->leftCornerWidgetSize.width()>0 ? twf->leftCornerWidgetSize.width() : 0;
                            tabRect.adjust(leftAdjust+adjust, 0, leftAdjust-adjust, 2);
                            break;
                        }
                        case QTabWidget::West:
                            tabRect.adjust(0, adjust, 2, -adjust);
                            break;
                        case QTabWidget::East:
                            tabRect=QRect(r.x()+r.width()-2, tabRect.y()+adjust, 4, tabRect.height()-(2*adjust));
                            break;
                        }

                        painter->setClipRegion(QRegion(r).subtracted(tabRect), Qt::IntersectClip);
                    }

                    if(!(opts.square&SQUARE_TAB_FRAME) && 0==tw->currentIndex())
                    {
                        bool reverse(Qt::RightToLeft==twf->direction);

                        switch(tw->tabPosition())
                        {
                        case QTabWidget::North:
                            if(reverse && twf->rightCornerWidgetSize.isEmpty())
                                round-=CORNER_TR;
                            else if(!reverse && twf->leftCornerWidgetSize.isEmpty())
                                round-=CORNER_TL;
                            break;
                        case QTabWidget::South:
                            if(reverse && twf->rightCornerWidgetSize.isEmpty())
                                round-=CORNER_BR;
                            else if(!reverse && twf->leftCornerWidgetSize.isEmpty())
                                round-=CORNER_BL;
                            break;
                        case QTabWidget::West:
                            round-=CORNER_TL;
                            break;
                        case QTabWidget::East:
                            round-=CORNER_TR;
                            break;
                        }
                    }
                }
            }

        QStyleOption opt(*option);
        const QColor *use=backgroundColors(option);

        opt.state|=State_Enabled;
        if(0!=opts.tabBgnd)
        {
            QColor bgnd(shade(use[ORIGINAL_SHADE], TO_FACTOR(opts.tabBgnd)));

            painter->fillRect(r.adjusted(0, 1, 0, -1), bgnd);
            painter->fillRect(r.adjusted(1, 0, -1, 0), bgnd);
        }
        drawBorder(painter, r, &opt, round, use, WIDGET_TAB_FRAME, opts.borderTab ? BORDER_LIGHT : BORDER_RAISED, false);
        painter->restore();
        break;
    }
#if QT_VERSION >= 0x040400
    case PE_PanelItemViewItem:
    {
        const QStyleOptionViewItemV4 *v4Opt = qstyleoption_cast<const QStyleOptionViewItemV4*>(option);
        const QAbstractItemView      *view = qobject_cast<const QAbstractItemView *>(widget);
        bool                         hover = state&State_MouseOver && state&State_Enabled && (!view ||
                                                                                              QAbstractItemView::NoSelection!=view->selectionMode()),
            hasCustomBackground = v4Opt->backgroundBrush.style() != Qt::NoBrush &&
            !(option->state & State_Selected),
            hasSolidBackground = !hasCustomBackground || Qt::SolidPattern==v4Opt->backgroundBrush.style();

        if (!hover && !(state & State_Selected) && !hasCustomBackground &&
            !(v4Opt->features & QStyleOptionViewItem::Alternate))
            break;

        QPalette::ColorGroup cg(state&State_Enabled
                                ? state&State_Active
                                ? QPalette::Normal
                                : QPalette::Inactive
                                : QPalette::Disabled);

        if (v4Opt && (v4Opt->features & QStyleOptionViewItem::Alternate))
            painter->fillRect(r, option->palette.brush(cg, QPalette::AlternateBase));

        if (!hover && !(state&State_Selected) && !hasCustomBackground)
            break;

        if(hasCustomBackground)
        {
            const QPointF prevOrigin(painter->brushOrigin());

            painter->setBrushOrigin(r.topLeft());
            painter->fillRect(r, v4Opt->backgroundBrush);
            painter->setBrushOrigin(prevOrigin);
        }

        if(state&State_Selected || hover)
        {
            if(!widget)
            {
                widget=getWidget(painter);
                if(widget)
                    widget=widget->parentWidget();
            }

            QColor color(hasCustomBackground && hasSolidBackground
                         ? v4Opt->backgroundBrush.color()
                         : palette.color(cg, QPalette::Highlight));
            bool   square((opts.square&SQUARE_LISTVIEW_SELECTION) &&
                          (/*(!widget && r.height()<=40 && r.width()>=48) || */
                              (widget && !widget->inherits("KFilePlacesView") &&
                               (qobject_cast<const QTreeView *>(widget) ||
                                (qobject_cast<const QListView *>(widget) &&
                                 QListView::IconMode!=((const QListView *)widget)->viewMode()))))),
                modAlpha(!(state&State_Active) && itsInactiveChangeSelectionColor);

            if (hover && !hasCustomBackground)
            {
                if (!(state & State_Selected))
                    color.setAlphaF(APP_PLASMA==theThemedApp && !widget ? (0.5 * (modAlpha ? 0.75 : 1.0)) : 0.20);
                else
                {
                    color = color.lighter(110);
                    if(modAlpha)
                        color.setAlphaF(INACTIVE_SEL_ALPHA);
                }
            }
            else if(modAlpha)
                color.setAlphaF(color.alphaF()*INACTIVE_SEL_ALPHA);

            if(square)
                drawBevelGradient(color, painter, r, true, false, opts.selectionAppearance, WIDGET_SELECTION);
            else
            {
                QPixmap pix;
                QString key;

                key.sprintf("qtc-sel-%x-%x", r.height(), color.rgba());
                if(!itsUsePixmapCache || !QPixmapCache::find(key, pix))
                {
                    pix=QPixmap(QSize(24, r.height()));
                    pix.fill(Qt::transparent);

                    QPainter pixPainter(&pix);
                    QRect    border(0, 0, pix.width(), pix.height());
                    double   radius(qtcGetRadius(&opts, r.width(), r.height(), WIDGET_OTHER, RADIUS_SELECTION));

                    pixPainter.setRenderHint(QPainter::Antialiasing, true);
                    drawBevelGradient(color, &pixPainter, border, buildPath(QRectF(border), WIDGET_OTHER, ROUNDED_ALL, radius), true,
                                      false, opts.selectionAppearance, WIDGET_SELECTION, false);
                    if(opts.borderSelection)
                    {
                        pixPainter.setBrush(Qt::NoBrush);
                        pixPainter.setPen(color);
                        pixPainter.drawPath(buildPath(border, WIDGET_SELECTION, ROUNDED_ALL, radius));
                    }
                    pixPainter.end();
                    if(itsUsePixmapCache)
                        QPixmapCache::insert(key, pix);
                }

                bool roundedLeft  = false,
                    roundedRight = false;

                if (v4Opt)
                {
                    roundedLeft  = (QStyleOptionViewItemV4::Beginning==v4Opt->viewItemPosition);
                    roundedRight = (QStyleOptionViewItemV4::End==v4Opt->viewItemPosition);
                    if (QStyleOptionViewItemV4::OnlyOne==v4Opt->viewItemPosition ||
                        QStyleOptionViewItemV4::Invalid==v4Opt->viewItemPosition ||
                        (view && view->selectionBehavior() != QAbstractItemView::SelectRows))
                    {
                        roundedLeft=roundedRight=true;
                    }
                }

                int size(roundedLeft && roundedRight ? qMin(8, r.width()/2) : 8);

                if (!reverse ? roundedLeft : roundedRight)
                {
                    painter->drawPixmap(r.topLeft(), pix.copy(0, 0, size, r.height()));
                    r.adjust(size, 0, 0, 0);
                }
                if (!reverse ? roundedRight : roundedLeft)
                {
                    painter->drawPixmap(r.right() - size + 1, r.top(), pix.copy(24-size, 0, size, r.height()));
                    r.adjust(0, 0, -size, 0);
                }
                if (r.isValid())
                    painter->drawTiledPixmap(r, pix.copy(7, 0, 8, r.height()));
            }
        }
        break;
    }
#endif
    case QtC_PE_DrawBackground:
        if (const QtCurve::Style::BgndOption *bgnd = qstyleoption_cast<const QtCurve::Style::BgndOption *>(option))
            if(state&QtC_StateKWin)
            {
                QColor col(palette.brush(QPalette::Window).color());
                int    opacity(col.alphaF()*100);

                col.setAlphaF(1.0);
                drawBackground(painter, col, r, opacity, BGND_WINDOW, bgnd->app, bgnd->path);
                // APPEARANCE_RAISED is used to signal flat background, but have background image!
                if(APPEARANCE_FLAT!=bgnd->app)
                {
                    painter->save();
                    painter->setClipRect(bgnd->rect, Qt::IntersectClip);
                    drawBackgroundImage(painter, true, BGND_IMG_ON_BORDER ? bgnd->rect : bgnd->widgetRect);
                    painter->restore();
                }
            }
        break;
        // TODO: This is the only part left from QWindowsStyle - but I dont think its actually used!
        // case PE_IndicatorProgressChunk:
    case PE_PanelTipLabel:
    {
        bool         haveAlpha=Utils::hasAlphaChannel(widget)  && APP_OPERA!=theThemedApp,
            rounded=!(opts.square&SQUARE_TOOLTIPS) && APP_OPERA!=theThemedApp;
        QPainterPath path=rounded ? buildPath(QRectF(r), WIDGET_OTHER, ROUNDED_ALL, MENU_AND_TOOLTIP_RADIUS) : QPainterPath();
        QColor       col=palette.toolTipBase().color();

#ifdef QTC_ENABLE_X11
        if (widget && widget->window()) {
            itsShadowHelper->registerWidget(widget->window());
        }
#endif
        painter->save();
        if(rounded)
            painter->setRenderHint(QPainter::Antialiasing, true);
        if(haveAlpha)
            col.setAlphaF(0.875);
        drawBevelGradient(col, painter, r, path, true, false, opts.tooltipAppearance, WIDGET_TOOLTIP, !haveAlpha);
        if(IS_FLAT(opts.tooltipAppearance))
        {
            painter->setPen(QPen(palette.toolTipText(), 0));
            drawRect(painter, r);
        }
        painter->restore();
        break;
    }
    // Fall through!
    default:
        BASE_STYLE::drawPrimitive(element, option, painter, widget);
        break;
    }
}

void Style::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    QRect r(option->rect);
    const State &state(option->state);
    const QPalette &palette(option->palette);
    bool reverse(Qt::RightToLeft==option->direction);

    switch((unsigned)element)
    {
    case CE_QtC_SetOptions:
        if (const PreviewOption *preview = qstyleoption_cast<const PreviewOption *>(option))
        {
            if(!painter && widget && QLatin1String("QtCurveConfigDialog")==widget->objectName())
            {
                Style *that=(Style *)this;
                opts=preview->opts;
                qtcCheckConfig(&opts);
                that->init(true);
            }
        }
        break;
    case CE_QtC_Preview:
        if (const PreviewOption *preview = qstyleoption_cast<const PreviewOption *>(option))
        {
            if(widget && widget && QLatin1String("QtCurveConfigDialog-GradientPreview")==widget->objectName())
            {
                Options      old=opts;
                const QColor *use(buttonColors(option));
                opts=preview->opts;

                drawLightBevelReal(painter, r, option, widget, ROUNDED_ALL, getFill(option, use, false, false), use,
                                   true, WIDGET_STD_BUTTON, false, opts.round, false);
                opts=old;
            }
        }
        break;
    case CE_QtC_KCapacityBar:
        if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option))
        {
            QStyleOptionProgressBar mod(*bar);

            if(mod.rect.height()>16 && widget->parentWidget() &&
               (qobject_cast<const QStatusBar *>(widget->parentWidget()) ||
                widget->parentWidget()->inherits("DolphinStatusBar")))
            {
                int m=(mod.rect.height()-16)/2;
                mod.rect.adjust(0, m, 0, -m);
            }
            drawControl(CE_ProgressBarGroove, &mod, painter, widget);
            if(DO_EFFECT && opts.borderProgress)
                mod.rect.adjust(1, 1, -1, -1);
            drawControl(CE_ProgressBarContents, &mod, painter, widget);
            drawControl(CE_ProgressBarLabel, &mod, painter, widget);
        }
        break;
    case CE_ToolBoxTabShape:
    {
        const QStyleOptionToolBox *tb = qstyleoption_cast<const QStyleOptionToolBox *>(option);
        if(!(tb && widget))
            break;

        const QColor *use = backgroundColors(widget->palette().color(QPalette::Window));
        QPainterPath path;
        int          y = r.height()*15/100;

        painter->save();
        if (reverse)
        {
            path.moveTo(r.left()+52, r.top());
            path.cubicTo(QPointF(r.left()+50-8, r.top()), QPointF(r.left()+50-10, r.top()+y), QPointF(r.left()+50-10, r.top()+y));
            path.lineTo(r.left()+18+9, r.bottom()-y);
            path.cubicTo(QPointF(r.left()+18+9, r.bottom()-y), QPointF(r.left()+19+6, r.bottom()-1-0.3), QPointF(r.left()+19, r.bottom()-1-0.3));
        }
        else
        {
            path.moveTo(r.right()-52, r.top());
            path.cubicTo(QPointF(r.right()-50+8, r.top()), QPointF(r.right()-50+10, r.top()+y), QPointF(r.right()-50+10, r.top()+y));
            path.lineTo(r.right()-18-9, r.bottom()-y);
            path.cubicTo(QPointF(r.right()-18-9, r.bottom()-y), QPointF(r.right()-19-6, r.bottom()-1-0.3), QPointF(r.right()-19, r.bottom()-1-0.3));
        }

        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->translate(0, 1);
        painter->setPen(use[0]);
        painter->drawPath(path);
        painter->translate(0, -1);
        painter->setPen(use[4]);
        painter->drawPath(path);
        painter->setRenderHint(QPainter::Antialiasing, false);
        if (reverse)
        {
            painter->drawLine(r.left()+50-1, r.top(), r.right(), r.top());
            painter->drawLine(r.left()+20, r.bottom()-2, r.left(), r.bottom()-2);
            painter->setPen(use[0]);
            painter->drawLine(r.left()+50, r.top()+1, r.right(), r.top()+1);
            painter->drawLine(r.left()+20, r.bottom()-1, r.left(), r.bottom()-1);
        }
        else
        {
            painter->drawLine(r.left(), r.top(), r.right()-50+1, r.top());
            painter->drawLine(r.right()-20, r.bottom()-2, r.right(), r.bottom()-2);
            painter->setPen(use[0]);
            painter->drawLine(r.left(), r.top()+1, r.right()-50, r.top()+1);
            painter->drawLine(r.right()-20, r.bottom()-1, r.right(), r.bottom()-1);
        }
        painter->restore();
        break;
    }
    case CE_MenuScroller:
    {
        const QColor *use(popupMenuCols());
        painter->fillRect(r, use[ORIGINAL_SHADE]);
        painter->setPen(use[QTC_STD_BORDER]);
        drawRect(painter, r);
        drawPrimitive(((state&State_DownArrow) ? PE_IndicatorArrowDown : PE_IndicatorArrowUp), option, painter, widget);
        break;
    }
    case CE_RubberBand: // Rubber band used in such things as iconview.
    {
        if(r.width()>0 && r.height()>0)
        {
            painter->save();
            QColor c(itsHighlightCols[ORIGINAL_SHADE]);

            painter->setClipRegion(r);
            painter->setPen(c);
            c.setAlpha(50);
            painter->setBrush(c);
            drawRect(painter, r);
            painter->restore();
        }
        break;
    }
    case CE_Splitter:
    {
        const QColor *use(buttonColors(option));
        const QColor *border(borderColors(option, use));
        // In Amarok nightly (2.2) State_Horizontal doesn't seem to always be set...
        bool         horiz(state&State_Horizontal || (r.height()>6 && r.height()>r.width()));

        painter->save();
        if(/*IS_FLAT_BGND(opts.bgndAppearance) || */state&State_MouseOver && state&State_Enabled)
        {
            QColor color(palette.color(QPalette::Active, QPalette::Window));

            if(state&State_MouseOver && state&State_Enabled && opts.splitterHighlight)
                if(ROUND_NONE!=opts.round)
                {
                    painter->save();
                    painter->setRenderHint(QPainter::Antialiasing, true);
                    double   radius(qtcGetRadius(&opts, r.width(), r.height(), WIDGET_OTHER, RADIUS_SELECTION));

                    drawBevelGradient(shade(palette.background().color(), TO_FACTOR(opts.splitterHighlight)),
                                      painter, r, buildPath(QRectF(r), WIDGET_OTHER, ROUNDED_ALL, radius),
                                      !(state&State_Horizontal), false, opts.selectionAppearance, WIDGET_SELECTION, false);
                    painter->restore();
                }
                else
                    drawBevelGradient(shade(palette.background().color(), TO_FACTOR(opts.splitterHighlight)), painter,
                                      r, !(state&State_Horizontal), false, opts.selectionAppearance, WIDGET_SELECTION);
            else
                painter->fillRect(r, color);
        }

        switch(opts.splitters)
        {
        case LINE_NONE:
            break;
        case LINE_1DOT:
            painter->drawPixmap(r.x()+((r.width()-5)/2), r.y()+((r.height()-5)/2), *getPixmap(border[QTC_STD_BORDER], PIX_DOT, 1.0));
            break;
        default:
        case LINE_DOTS:
            drawDots(painter, r, horiz, NUM_SPLITTER_DASHES, 1, border, 0, 5);
            break;
        case LINE_FLAT:
        case LINE_SUNKEN:
        case LINE_DASHES:
            drawLines(painter, r, horiz, NUM_SPLITTER_DASHES, 3, border, 0, 3, opts.splitters);
        }
        painter->restore();
        break;
    }
    case CE_SizeGrip:
    {
        QPolygon   triangle(3);
        Qt::Corner corner;
        int        size=SIZE_GRIP_SIZE-2;

        if (const QStyleOptionSizeGrip *sgrp = qstyleoption_cast<const QStyleOptionSizeGrip *>(option))
            corner = sgrp->corner;
        else if (Qt::RightToLeft==option->direction)
            corner = Qt::BottomLeftCorner;
        else
            corner = Qt::BottomRightCorner;

        switch(corner)
        {
        case Qt::BottomLeftCorner:
            triangle.putPoints(0, 3, 0,0, size,size, 0,size);
            triangle.translate(r.x(), r.y()+(r.height()-(SIZE_GRIP_SIZE-1)));
            break;
        case Qt::BottomRightCorner:
            triangle.putPoints(0, 3, size,0, size,size, 0,size);
            triangle.translate(r.x()+(r.width()-(SIZE_GRIP_SIZE-1)), r.y()+(r.height()-(SIZE_GRIP_SIZE-1)));
            break;
        case Qt::TopRightCorner:
            triangle.putPoints(0, 3, 0,0, size,0, size,size);
            triangle.translate(r.x()+(r.width()-(SIZE_GRIP_SIZE-1)), r.y());
            break;
        case Qt::TopLeftCorner:
            triangle.putPoints(0, 3, 0,0, size,0, 0,size);
            triangle.translate(r.x(), r.y());
        }
        painter->save();
        painter->setPen(itsBackgroundCols[2]);
        painter->setBrush(itsBackgroundCols[2]);
        painter->drawPolygon(triangle);
        painter->restore();
        break;
    }
    case CE_ToolBar:
        if (const QStyleOptionToolBar *toolbar = qstyleoption_cast<const QStyleOptionToolBar *>(option))
        {
            if(!widget || !widget->parent() || qobject_cast<QMainWindow *>(widget->parent()))
            {
                painter->save();
                drawMenuOrToolBarBackground(widget, painter, r, option, false, Qt::NoToolBarArea==toolbar->toolBarArea ||
                                            Qt::BottomToolBarArea==toolbar->toolBarArea ||
                                            Qt::TopToolBarArea==toolbar->toolBarArea);
                if(TB_NONE!=opts.toolbarBorders)
                {
                    const QColor *use=/*PE_PanelMenuBar==pe && itsActive
                                        ? itsMenubarCols
                                        : */ backgroundColors(option);
                    bool         dark(TB_DARK==opts.toolbarBorders || TB_DARK_ALL==opts.toolbarBorders);

                    if(TB_DARK_ALL==opts.toolbarBorders || TB_LIGHT_ALL==opts.toolbarBorders)
                    {
                        painter->setPen(use[0]);
                        painter->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                        painter->drawLine(r.x(), r.y(), r.x(), r.y()+r.height()-1);
                        painter->setPen(use[dark ? 3 : 4]);
                        painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                        painter->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
                    }
                    else
                    {
                        bool paintH(true),
                            paintV(true);

                        switch (toolbar->toolBarArea)
                        {
                        case Qt::BottomToolBarArea:
                        case Qt::TopToolBarArea:
                            paintV=false;
                            break;
                        case Qt::RightToolBarArea:
                        case Qt::LeftToolBarArea:
                            paintH=false;
                        default:
                            break;
                        }

                        painter->setPen(use[0]);
                        if(paintH)
                            painter->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                        if(paintV)
                            painter->drawLine(r.x(), r.y(), r.x(), r.y()+r.height()-1);
                        painter->setPen(use[dark ? 3 : 4]);
                        if(paintH)
                            painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                        if(paintV)
                            painter->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
                    }
                }
                painter->restore();
            }
        }
        break;
    case CE_DockWidgetTitle:
        if (const QStyleOptionDockWidget *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget*>(option))
        {
            bool verticalTitleBar(dwOpt->verticalTitleBar);
            bool isKOffice(widget && widget->inherits("KoDockWidgetTitleBar"));
            QRect fillRect(r);

            // This fixes the look of KOffice's dock widget titlebars...
            if(isKOffice)
                fillRect.adjust(-r.x(), -r.y(), 0, 0);

            if(!IS_FLAT(opts.dwtAppearance))
            {
                painter->save();

                QColor col((opts.dwtSettings&DWT_COLOR_AS_PER_TITLEBAR)
                           ? getMdiColors(option, state&State_Active)[ORIGINAL_SHADE]
                           : palette.background().color());
                if(opts.round<ROUND_FULL)
                    drawBevelGradient(col, painter, fillRect, !verticalTitleBar,
                                      false, opts.dwtAppearance, WIDGET_DOCK_WIDGET_TITLE);
                else
                {
                    double radius(qtcGetRadius(&opts, fillRect.width(), fillRect.height(), WIDGET_OTHER, RADIUS_EXTERNAL));
                    int    round=ROUNDED_ALL;

                    if(opts.dwtSettings&DWT_ROUND_TOP_ONLY)
                        round=verticalTitleBar ? ROUNDED_LEFT : ROUNDED_TOP;
                    painter->setRenderHint(QPainter::Antialiasing, true);
                    drawBevelGradient(col, painter, fillRect, buildPath(QRectF(fillRect), WIDGET_OTHER, round, radius), !verticalTitleBar,
                                      false, opts.dwtAppearance, WIDGET_DOCK_WIDGET_TITLE, false);
                }

                painter->restore();
            }

            if (!dwOpt->title.isEmpty()) {
                QRect titleRect(subElementRect(SE_DockWidgetTitleBarText, option, widget));

                if (verticalTitleBar)
                {
                    QRect rVert(r);
                    QSize s(rVert.size());

                    s.transpose();
                    rVert.setSize(s);

                    titleRect = QRect(rVert.left() + r.bottom() - titleRect.bottom(),
                                      rVert.top() + titleRect.left() - r.left(),
                                      titleRect.height(), titleRect.width());

                    painter->translate(rVert.left(), rVert.top() + rVert.width());
                    painter->rotate(-90);
                    painter->translate(-rVert.left(), -rVert.top());
                }
#ifdef QTC_QT5_ENABLE_KDE
                if(opts.dwtSettings&DWT_FONT_AS_PER_TITLEBAR)
                    painter->setFont(KGlobalSettings::windowTitleFont());
#endif
                QFontMetrics fm(painter->fontMetrics());
                QString      title(fm.elidedText(dwOpt->title, Qt::ElideRight, titleRect.width(), QPalette::WindowText));
                painter->save();
                getMdiColors(option, state&State_Active);

                QColor textColor((opts.dwtSettings&DWT_COLOR_AS_PER_TITLEBAR)
                                 ? state&State_Active
                                 ? itsActiveMdiTextColor
                                 : itsMdiTextColor
                                 : palette.color(QPalette::WindowText)),
                    shadow(WINDOW_SHADOW_COLOR(opts.titlebarEffect));
                int    textOpt(Qt::AlignVCenter); // TODO: dwtPosAsPerTitleBar ?

                if(opts.dwtSettings&DWT_TEXT_ALIGN_AS_PER_TITLEBAR)
                    switch(opts.titlebarAlignment)
                    {
                    case ALIGN_FULL_CENTER:
                        if(!verticalTitleBar && !reverse)
                        {
                            QFontMetrics fm(painter->fontMetrics());
                            int          width=fm.boundingRect(title).width();

                            if(((fillRect.width()+width)/2)<=titleRect.width()+(isKOffice ? r.x() : 0))
                            {
                                titleRect=fillRect;
                                textOpt|=Qt::AlignHCenter;
                            }
                            else
                                textOpt|=Qt::AlignRight;
                            break;
                        }
                    case ALIGN_CENTER:
                        textOpt|=Qt::AlignHCenter;
                        break;
                    case ALIGN_RIGHT:
                        textOpt|=Qt::AlignRight;
                        break;
                    default:
                    case ALIGN_LEFT:
                        textOpt|=Qt::AlignLeft;
                    }
                else
                    textOpt|=Qt::AlignLeft;

                if (!styleHint(SH_UnderlineShortcut, dwOpt, widget))
                    textOpt|=Qt::TextHideMnemonic;
                else
                    textOpt|=Qt::TextShowMnemonic;

                if((opts.dwtSettings&DWT_EFFECT_AS_PER_TITLEBAR) &&
                   EFFECT_NONE!=opts.titlebarEffect)
                {
                    shadow.setAlphaF(WINDOW_TEXT_SHADOW_ALPHA(opts.titlebarEffect));
                    painter->setPen(shadow);
                    painter->drawText(titleRect.adjusted(1, 1, 1, 1), textOpt, title);

                    if (!(state&State_Active) && DARK_WINDOW_TEXT(textColor))
                        textColor.setAlpha((textColor.alpha() * 180) >> 8);
                }
                painter->setPen(textColor);
                painter->drawText(titleRect, textOpt, title);
                painter->restore();
            }
        }
        break;
#if QT_VERSION >= 0x040300
    case CE_HeaderEmptyArea:
    {
        const QStyleOptionHeader *ho = qstyleoption_cast<const QStyleOptionHeader *>(option);
        bool                     horiz(ho ? Qt::Horizontal==ho->orientation : state&State_Horizontal);
        QStyleOption             opt(*option);
        const QColor             *use(opts.lvButton ? buttonColors(option) : backgroundColors(option));

        opt.state&=~State_MouseOver;
        painter->save();

        drawBevelGradient(getFill(&opt, use), painter, r, horiz, false, opts.lvAppearance, WIDGET_LISTVIEW_HEADER);

        painter->setRenderHint(QPainter::Antialiasing, true);
        if(APPEARANCE_RAISED==opts.lvAppearance)
        {
            painter->setPen(use[4]);
            if(horiz)
                drawAaLine(painter, r.x(), r.y()+r.height()-2, r.x()+r.width()-1, r.y()+r.height()-2);
            else
                drawAaLine(painter, r.x()+r.width()-2, r.y(), r.x()+r.width()-2, r.y()+r.height()-1);
        }

        painter->setPen(use[QTC_STD_BORDER]);
        if(horiz)
            drawAaLine(painter, r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
        else if(reverse)
            drawAaLine(painter, r.x(), r.y(), r.x(), r.y()+r.height()-1);
        else
            drawAaLine(painter, r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->restore();
        break;
    }
#endif
    case CE_HeaderSection:
        if (const QStyleOptionHeader *ho = qstyleoption_cast<const QStyleOptionHeader *>(option))
        {
            const QColor *use(state&State_Enabled && itsSortedLvColors && QStyleOptionHeader::None!=ho->sortIndicator
                              ? itsSortedLvColors
                              : opts.lvButton ? buttonColors(option) : backgroundColors(option));

            painter->save();

            if(state & (State_Raised | State_Sunken))
            {
                bool sunken(state &(/*State_Down |*/ /*State_On | */State_Sunken));
                QStyleOption opt(*option);

                opt.state&=~State_On;

                if(-1==ho->section && !(state&State_Enabled) && widget && widget->isEnabled())
                    opt.state|=State_Enabled;

                drawBevelGradient(getFill(&opt, use), painter, r, Qt::Horizontal==ho->orientation, sunken, opts.lvAppearance, WIDGET_LISTVIEW_HEADER);

                painter->setRenderHint(QPainter::Antialiasing, true);
                if(APPEARANCE_RAISED==opts.lvAppearance)
                {
                    painter->setPen(use[4]);
                    if(Qt::Horizontal==ho->orientation)
                        drawAaLine(painter, r.x(), r.y()+r.height()-2, r.x()+r.width()-1, r.y()+r.height()-2);
                    else
                        drawAaLine(painter, r.x()+r.width()-2, r.y(), r.x()+r.width()-2, r.y()+r.height()-1);
                }

                if(Qt::Horizontal==ho->orientation)
                {
                    painter->setPen(use[QTC_STD_BORDER]);
                    drawAaLine(painter, r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                    if(opts.coloredMouseOver && state&State_MouseOver && state&State_Enabled)
                        drawHighlight(painter, QRect(r.x(), r.y()+r.height()-2, r.width(), 2), true, true);

                    if(QStyleOptionHeader::End!=ho->position && QStyleOptionHeader::OnlyOneSection!=ho->position)
                    {
                        drawFadedLine(painter, QRect(r.x()+r.width()-2, r.y()+5, 1, r.height()-10), use[QTC_STD_BORDER], true, true, false);
                        drawFadedLine(painter, QRect(r.x()+r.width()-1, r.y()+5, 1, r.height()-10), use[0], true, true, false);
                    }
                }
                else
                {
                    painter->setPen(use[QTC_STD_BORDER]);
                    if(reverse)
                        drawAaLine(painter, r.x(), r.y(), r.x(), r.y()+r.height()-1);
                    else
                        drawAaLine(painter, r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);

                    if(QStyleOptionHeader::End!=ho->position && QStyleOptionHeader::OnlyOneSection!=ho->position)
                    {
                        drawFadedLine(painter, QRect(r.x()+5, r.y()+r.height()-2, r.width()-10, 1), use[QTC_STD_BORDER], true, true, true);
                        drawFadedLine(painter, QRect(r.x()+5, r.y()+r.height()-1, r.width()-10, 1), use[0], true, true, true);
                    }
                    if(opts.coloredMouseOver && state&State_MouseOver && state&State_Enabled)
                        drawHighlight(painter, QRect(r.x(), r.y()+r.height()-3, r.width(), 2), true, true);
                }
                painter->setRenderHint(QPainter::Antialiasing, false);
            }
            else if(!IS_FLAT(opts.lvAppearance) && !reverse && ((State_Enabled|State_Active)==state || State_Enabled==state))
            {
                QPolygon     top;
                const QColor &col(getFill(option, use));

                top.setPoints(3, r.x(), r.y(), r.x()+r.width(), r.y(), r.x()+r.width(), r.y()+r.height());
                painter->setClipRegion(QRegion(top));
                drawBevelGradient(col, painter, r, true, false, opts.lvAppearance, WIDGET_LISTVIEW_HEADER);
                painter->setClipRegion(QRegion(r).xored(QRegion(top)));
                drawBevelGradient(col, painter, r, false, false, opts.lvAppearance, WIDGET_LISTVIEW_HEADER);
            }
            else
                painter->fillRect(r, getFill(option, use));
            painter->restore();
        }
        break;
    case CE_HeaderLabel:
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option))
        {
            if (!header->icon.isNull())
            {
                QPixmap pixmap(getIconPixmap(header->icon, pixelMetric(PM_SmallIconSize), header->state));
                int     pixw(pixmap.width());
                QRect   aligned(alignedRect(header->direction, QFlag(header->iconAlignment), pixmap.size(), r)),
                    inter(aligned.intersected(r));

                painter->drawPixmap(inter.x(), inter.y(), pixmap, inter.x() - aligned.x(), inter.y() - aligned.y(), inter.width(), inter.height());

                if (header->direction == Qt::LeftToRight)
                    r.setLeft(r.left() + pixw + 2);
                else
                    r.setRight(r.right() - pixw - 2);
            }
            drawItemTextWithRole(painter, r, header->textAlignment, palette, state&State_Enabled, header->text, QPalette::ButtonText);
        }
        break;
    case CE_ProgressBarGroove:
    {
        bool   doEtch(DO_EFFECT && opts.borderProgress),
            horiz(true);
        QColor col;

        if (const QStyleOptionProgressBar *bar =
            qstyleoption_cast<const QStyleOptionProgressBar*>(option))
            horiz = Qt::Horizontal==bar->orientation;

        painter->save();

        if(doEtch)
            r.adjust(1, 1, -1, -1);

        switch(opts.progressGrooveColor)
        {
        default:
        case ECOLOR_BASE:
            col=palette.base().color();
            break;
        case ECOLOR_BACKGROUND:
            col=palette.background().color();
            break;
        case ECOLOR_DARK:
            col=itsBackgroundCols[2];
        }

        drawBevelGradient(col, painter, r, opts.borderProgress
                          ? buildPath(r, WIDGET_PBAR_TROUGH, ROUNDED_ALL,
                                      qtcGetRadius(&opts, r.width(), r.height(), WIDGET_PBAR_TROUGH, RADIUS_EXTERNAL))
                          : QPainterPath(),
                          horiz, false, opts.progressGrooveAppearance, WIDGET_PBAR_TROUGH);

        if(doEtch)
            drawEtch(painter, r.adjusted(-1, -1, 1, 1), widget, WIDGET_PBAR_TROUGH);
        else if(!opts.borderProgress)
        {
            painter->setPen(itsBackgroundCols[QTC_STD_BORDER]);
            if(horiz)
            {
                painter->drawLine(r.topLeft(), r.topRight());
                painter->drawLine(r.bottomLeft(), r.bottomRight());
            }
            else
            {
                painter->drawLine(r.topLeft(), r.bottomLeft());
                painter->drawLine(r.topRight(), r.bottomRight());
            }
        }

        if(opts.borderProgress)
            drawBorder(painter, r, option, ROUNDED_ALL, backgroundColors(option), WIDGET_PBAR_TROUGH,
                       IS_FLAT(opts.progressGrooveAppearance) && ECOLOR_DARK!=opts.progressGrooveColor ? BORDER_SUNKEN : BORDER_FLAT);
        painter->restore();
        break;
    }
    case CE_ProgressBarContents:
        if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option))
        {
            bool vertical(false),
                inverted(false),
                indeterminate(0==bar->minimum && 0==bar->maximum);

            // Get extra style options if version 2
            if (const QStyleOptionProgressBar *bar =
                qstyleoption_cast<const QStyleOptionProgressBar*>(option)) {
                vertical = Qt::Vertical==bar->orientation;
                inverted = bar->invertedAppearance;
            }

            if (!indeterminate && -1==bar->progress)
                break;

            bool reverse = (!vertical && (bar->direction == Qt::RightToLeft)) || vertical;

            if (inverted)
                reverse = !reverse;

            painter->save();

            if(indeterminate) //Busy indicator
            {
                int chunkSize(PROGRESS_CHUNK_WIDTH*3.4),
                    measure(vertical ? r.height() : r.width());

                if(chunkSize>(measure/2))
                    chunkSize=measure/2;

                int          step(itsAnimateStep % ((measure-chunkSize) * 2));
                QStyleOption opt(*option);

                if (step > (measure-chunkSize))
                    step = 2 * (measure-chunkSize) - step;

                opt.state|=State_Raised|State_Horizontal;
                drawProgress(painter, vertical ? QRect(r.x(), r.y()+step, r.width(), chunkSize) : QRect(r.x()+step, r.y(), chunkSize, r.height()),
                             option, vertical);
            }
            else if(r.isValid() && bar->progress>0)
            {
                qint64 progress = qMax<qint64>(bar->progress, bar->minimum); // workaround for bug in QProgressBar
                double pg = ((progress - qint64(bar->minimum)) /
                             qMax(double(1.0), double(qint64(bar->maximum) - qint64(bar->minimum))));

                if(vertical)
                {
                    int height(qMin(r.height(), (int)(pg * r.height())));

                    if(inverted)
                        drawProgress(painter, QRect(r.x(), r.y(), r.width(), height), option, true);
                    else
                        drawProgress(painter, QRect(r.x(), r.y()+(r.height()-height), r.width(), height), option, true);
                }
                else
                {
                    int width(qMin(r.width(), (int)(pg * r.width())));

                    if(reverse || inverted)
                        drawProgress(painter, QRect(r.x()+(r.width()-width), r.y(), width, r.height()), option, false, true);
                    else
                        drawProgress(painter, QRect(r.x(), r.y(), width, r.height()), option);
                }
            }

            painter->restore();
        }
        break;
    case CE_ProgressBarLabel:
        if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option))
        {
            // The busy indicator doesn't draw a label
            if (0==bar->minimum && 0==bar->maximum)
                return;

            bool vertical(false),
                inverted(false),
                bottomToTop(false);

            // Get extra style options if version 2
            if (const QStyleOptionProgressBar *bar =
                qstyleoption_cast<const QStyleOptionProgressBar*>(option)) {
                vertical = (bar->orientation == Qt::Vertical);
                inverted = bar->invertedAppearance;
                bottomToTop = bar->bottomToTop;
            }

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);

            if (vertical) {
                // flip width and height
                r = QRect(r.left(), r.top(), r.height(), r.width());

                QTransform m;
                if (bottomToTop) {
                    m.translate(0.0, r.width());
                    m.rotate(-90);
                } else {
                    m.translate(r.height(), 0.0);
                    m.rotate(90);
                }
                painter->setTransform(m);
            }

            int progressIndicatorPos = (bar->progress - qreal(bar->minimum)) /
                qMax(qreal(1.0), qreal(bar->maximum) - bar->minimum) * r.width();
            bool flip((!vertical && (((Qt::RightToLeft==bar->direction) &&
                                      !inverted) ||
                                     ((Qt::LeftToRight==bar->direction) &&
                                      inverted))) ||
                      (vertical && ((!inverted && !bottomToTop) ||
                                    (inverted && bottomToTop))));
            QRect leftRect;
            QRegion rightRect(r);
            QPalette::ColorGroup cg = state&State_Enabled ||
                State_None==state ? QPalette::Active : QPalette::Current;

            if (flip) {
                int indicatorPos(r.width() - progressIndicatorPos);

                if (indicatorPos >= 0 && indicatorPos <= r.width()) {
                    painter->setPen(palette.brush(cg, QPalette::Base).color());
                    leftRect = QRect(r.left(), r.top(), indicatorPos, r.height());
                    //rightRect = QRect(r.left()+indicatorPos, r.top(), r.width()-indicatorPos, r.height());
                } else if (indicatorPos > r.width()) {
                    painter->setPen(palette.brush(cg, QPalette::Text).color());
                } else {
                    painter->setPen(palette.brush(cg, QPalette::HighlightedText).color());
                }
            } else {
                if (progressIndicatorPos >= 0 && progressIndicatorPos <= r.width())
                {
                    leftRect = QRect(r.left(), r.top(), progressIndicatorPos, r.height());
                    //rightRect = QRect(r.left()+progressIndicatorPos, r.top(), r.width()-progressIndicatorPos, r.height());
                }
                else if (progressIndicatorPos > r.width())
                    painter->setPen(palette.brush(cg, QPalette::HighlightedText).color());
                else
                    painter->setPen(palette.brush(cg, QPalette::Text).color());
            }

            QString text = bar->fontMetrics.elidedText(bar->text, Qt::ElideRight, r.width());

            rightRect = rightRect.subtracted(leftRect);
            painter->setClipRegion(rightRect);
            painter->drawText(r, text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
            if (!leftRect.isNull())
            {
                painter->setPen(palette.brush(cg, flip ? QPalette::Text : QPalette::HighlightedText).color());
                painter->setClipRect(leftRect);
                painter->drawText(r, text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
            }

            painter->restore();
        }
        break;
    case CE_MenuBarItem:
        if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            bool    down(state&(State_On|State_Sunken)),
                active(state&State_Enabled && (down || (state&State_Selected && opts.menubarMouseOver)));
            uint    alignment(Qt::AlignCenter|Qt::TextShowMnemonic|Qt::TextDontClip|Qt::TextSingleLine);
            QPixmap pix(getIconPixmap(mbi->icon, pixelMetric(PM_SmallIconSize), mbi->state));

            if (!styleHint(SH_UnderlineShortcut, mbi, widget))
                alignment|=Qt::TextHideMnemonic;

            painter->save();
            drawMenuOrToolBarBackground(widget, painter, mbi->menuRect, option);
            if(active)
                drawMenuItem(painter, !opts.roundMbTopOnly && !(opts.square&SQUARE_POPUP_MENUS) ? r.adjusted(1, 1, -1, -1) : r,
                             option, MENU_BAR,
                             (down || APP_OPENOFFICE==theThemedApp) && opts.roundMbTopOnly ? ROUNDED_TOP : ROUNDED_ALL,
                             opts.useHighlightForMenu && (opts.colorMenubarMouseOver || down || APP_OPENOFFICE==theThemedApp)
                             ? (itsOOMenuCols ? itsOOMenuCols : itsHighlightCols) : itsBackgroundCols);

            if (!pix.isNull())
                drawItemPixmap(painter, mbi->rect, alignment, pix);
            else
            {
                const QColor &col=state&State_Enabled
                    ? ((opts.colorMenubarMouseOver && active) || (!opts.colorMenubarMouseOver && down))
                    ? opts.customMenuTextColor
                    ? opts.customMenuSelTextColor
                    : opts.useHighlightForMenu
                    ? palette.highlightedText().color()
                    : palette.foreground().color()
                    : palette.foreground().color()
                    : palette.foreground().color();

                painter->setPen(col);
                painter->drawText(r, alignment, mbi->text);
            }
            painter->restore();
        }
        break;
    case CE_MenuItem:
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            bool           comboMenu(qobject_cast<const QComboBox*>(widget)),
                reverse(Qt::RightToLeft==menuItem->direction),
                isOO(isOOWidget(widget));
            int            checkcol(qMax(menuItem->maxIconWidth, 20)),
                stripeWidth(qMax(checkcol, constMenuPixmapWidth)-2);
            const QColor * use(popupMenuCols(option));

            QRect rx(r);

            if(isOO)
            {
                if(opts.borderMenuitems)
                    r.adjust(2, 0, -2, 0);
                else if(APPEARANCE_FADE==opts.menuitemAppearance)
                    r.adjust(1, 0, -1, 0);
            }

            painter->save();

            if (QStyleOptionMenuItem::Separator==menuItem->menuItemType)
            {
                bool isMenu(!widget || qobject_cast<const QMenu*>(widget)),
                    doStripe(isMenu && opts.menuStripe && !comboMenu);

                if(doStripe)
                    drawBevelGradient(menuStripeCol(), painter, QRect(reverse ? r.right()-stripeWidth : r.x(), r.y(),
                                                                      stripeWidth, r.height()),
                                      false, false, opts.menuStripeAppearance, WIDGET_OTHER);

                if(!menuItem->text.isEmpty())
                {
                    QStyleOption opt;
                    opt.rect = r.adjusted(2, 2, -3, -2);
                    opt.state=State_Raised|State_Enabled|State_Horizontal;
                    drawLightBevel(painter, opt.rect, &opt, widget, ROUNDED_ALL, getFill(&opt, use), use, true, WIDGET_NO_ETCH_BTN);

                    QFont font(menuItem->font);

                    font.setBold(true);
                    painter->setFont(font);
                    drawItemTextWithRole(painter, r, Qt::AlignHCenter | Qt::AlignVCenter,
                                         palette, state&State_Enabled, menuItem->text, QPalette::Text);
                }
                else
                {
                    QRect miRect(menuItem->rect.left() + 3 +
                                 (!reverse && doStripe ? stripeWidth : 0),
                                 menuItem->rect.center().y(),
                                 menuItem->rect.width() - (7 + (doStripe ? stripeWidth : 0)),
                                 1);
                    drawFadedLine(painter, miRect, use[MENU_SEP_SHADE], true, true, true);
                }

                if(isOO)
                {
                    painter->setPen(use[QTC_STD_BORDER]);
                    painter->drawLine(rx.topLeft(), rx.bottomLeft());
                    painter->drawLine(rx.topRight(), rx.bottomRight());
                }
                painter->restore();
                break;
            }

            bool selected(state&State_Selected),
                checkable(QStyleOptionMenuItem::NotCheckable!=menuItem->checkType),
                checked(menuItem->checked),
                enabled(state&State_Enabled);

            if(opts.menuStripe && !comboMenu)
                drawBevelGradient(menuStripeCol(), painter,
                                  QRect(reverse ? r.right()-stripeWidth : r.x(), r.y(), stripeWidth, r.height()),
                                  false, false, opts.menuStripeAppearance, WIDGET_OTHER);

            if (selected && enabled)
                drawMenuItem(painter, r, option, /*comboMenu ? MENU_COMBO : */MENU_POPUP, ROUNDED_ALL,
                             opts.useHighlightForMenu ? (itsOOMenuCols ? itsOOMenuCols : itsHighlightCols) : use);

            if(comboMenu)
            {
                if (menuItem->icon.isNull())
                    checkcol = 0;
                else
                    checkcol = menuItem->maxIconWidth;
            }
            else
            {
                // Check
                QRect checkRect(r.left() + 3, r.center().y() - 6, opts.crSize, opts.crSize);
                checkRect = visualRect(menuItem->direction, menuItem->rect, checkRect);
                if (checkable)
                {
                    if ((menuItem->checkType & QStyleOptionMenuItem::Exclusive) && menuItem->icon.isNull())
                    {
                        QStyleOptionButton button;
                        button.rect = checkRect;
                        button.state = menuItem->state|STATE_MENU;
                        if (checked)
                            button.state |= State_On;
                        button.palette = palette;
                        drawPrimitive(PE_IndicatorRadioButton, &button, painter, widget);
                    }
                    else
                    {
                        if (menuItem->icon.isNull() || !opts.menuIcons)
                        {
                            QStyleOptionButton button;
                            button.rect = checkRect;
                            button.state = menuItem->state|STATE_MENU;
                            if (checked)
                                button.state |= State_On;
                            button.palette = palette;
                            drawPrimitive(PE_IndicatorCheckBox, &button, painter, widget);
                        }
                        else if (checked)
                        {
                            int          iconSize(qMax(menuItem->maxIconWidth, 20));
                            QRect        sunkenRect(r.left() + 1, r.top() + (r.height() - iconSize) / 2,
                                                    iconSize, iconSize);
                            QStyleOption opt(*option);

                            sunkenRect = visualRect(menuItem->direction, menuItem->rect, sunkenRect);
                            opt.state = menuItem->state;
                            opt.state|=State_Raised|State_Horizontal;
                            if (checked)
                                opt.state |= State_On;
                            drawLightBevel(painter, sunkenRect, &opt, widget, ROUNDED_ALL, getFill(&opt, itsButtonCols), itsButtonCols);
                        }
                    }
                }
            }

            // Text and icon, ripped from windows style
            bool  dis(!(state&State_Enabled)),
                act(state&State_Selected);
            QRect vCheckRect(visualRect(option->direction, menuItem->rect,
                                        QRect(menuItem->rect.x(), menuItem->rect.y(), checkcol, menuItem->rect.height())));

            if (opts.menuIcons && !menuItem->icon.isNull())
            {
                QIcon::Mode mode(dis ? QIcon::Disabled : QIcon::Normal);

                if (act && !dis)
                    mode = QIcon::Active;

                QPixmap pixmap(getIconPixmap(menuItem->icon, pixelMetric(PM_SmallIconSize), mode,
                                             checked ? QIcon::On : QIcon::Off));

                int     pixw(pixmap.width()),
                    pixh(pixmap.height());
                QRect   pmr(0, 0, pixw, pixh);

                pmr.moveCenter(vCheckRect.center());
                painter->setPen(palette.text().color());
                if (checkable && checked)
                    painter->drawPixmap(QPoint(pmr.left() + 1, pmr.top() + 1), pixmap);
                else
                    painter->drawPixmap(pmr.topLeft(), pixmap);
            }

            painter->setPen(dis
                            ? palette.text().color()
                            : selected && opts.useHighlightForMenu && !itsOOMenuCols
                            ? palette.highlightedText().color()
                            : palette.foreground().color());

            int x, y, w, h,
                tab(menuItem->tabWidth);

            menuItem->rect.getRect(&x, &y, &w, &h);

            int     xm(windowsItemFrame + checkcol + windowsItemHMargin -2),
                xpos(menuItem->rect.x() + xm);
            QRect   textRect(xpos, y + windowsItemVMargin,
                             opts.menuIcons ? (w - xm - windowsRightBorder - tab + 1)
                             : (w - ((xm*2) + tab)),
                             h - 2 * windowsItemVMargin),
                vTextRect = visualRect(option->direction, menuItem->rect, textRect);
            QString s(menuItem->text);

            if (!s.isEmpty())                      // draw text
            {
                int t(s.indexOf(QLatin1Char('\t'))),
                    textFlags(Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine);

                if (!styleHint(SH_UnderlineShortcut, menuItem, widget))
                    textFlags |= Qt::TextHideMnemonic;
                textFlags |= Qt::AlignLeft;

                if (t >= 0)
                {
                    QRect vShortcutRect(visualRect(option->direction, menuItem->rect,
                                                   QRect(textRect.topRight(), QPoint(menuItem->rect.right(), textRect.bottom()))));

                    painter->drawText(vShortcutRect, textFlags, s.mid(t + 1));
                    s = s.left(t);
                }

                QFont font(menuItem->font);

                if (menuItem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                    font.setBold(true);

                painter->setFont(font);
                painter->drawText(vTextRect, textFlags, s.left(t));
            }

            // Arrow
            if (QStyleOptionMenuItem::SubMenu==menuItem->menuItemType) // draw sub menu arrow
            {
                int              dim((menuItem->rect.height() - 4) / 2),
                    xpos(menuItem->rect.left() + menuItem->rect.width() - 3 - dim);
                PrimitiveElement arrow(Qt::RightToLeft==option->direction ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight);
                QRect            vSubMenuRect(visualRect(option->direction, menuItem->rect,
                                                         QRect(xpos, menuItem->rect.top() + menuItem->rect.height() / 2 - dim / 2, dim, dim)));

                drawArrow(painter, vSubMenuRect, arrow,
                          opts.useHighlightForMenu && state&State_Enabled && state&State_Selected && !itsOOMenuCols
                          ? palette.highlightedText().color()
                          : palette.text().color());
            }

            if(isOO)
            {
                painter->setPen(use[QTC_STD_BORDER]);
                painter->drawLine(rx.topLeft(), rx.bottomLeft());
                painter->drawLine(rx.topRight(), rx.bottomRight());
            }
            painter->restore();
        }
        break;
    case CE_MenuHMargin:
    case CE_MenuVMargin:
    case CE_MenuEmptyArea:
        break;
    case CE_PushButton:
        if(const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            // For OO.o 3.2 need to fill widget background!
            if(isOOWidget(widget))
                painter->fillRect(r, palette.brush(QPalette::Window));
            drawControl(CE_PushButtonBevel, btn, painter, widget);

            QStyleOptionButton subopt(*btn);

            subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
            drawControl(CE_PushButtonLabel, &subopt, painter, widget);

            if (state&State_HasFocus &&
                !(state&State_MouseOver && FULL_FOCUS && MO_NONE!=opts.coloredMouseOver))
            {
                QStyleOptionFocusRect fropt;
                fropt.QStyleOption::operator=(*btn);
                fropt.rect = subElementRect(SE_PushButtonFocusRect, btn, widget);
                drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
            }
        }
        break;
    case CE_PushButtonBevel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            int dbi(pixelMetric(PM_ButtonDefaultIndicator, btn, widget));

            if (btn->features & QStyleOptionButton::DefaultButton)
                drawPrimitive(PE_FrameDefaultButton, option, painter, widget);
            if (btn->features & QStyleOptionButton::AutoDefaultButton)
                r.setCoords(r.left() + dbi, r.top() + dbi, r.right() - dbi, r.bottom() - dbi);
            if (!(btn->features & (QStyleOptionButton::Flat |
                                   QStyleOptionButton::CommandLinkButton)) ||
                state&(State_Sunken | State_On | State_MouseOver)) {
                QStyleOptionButton tmpBtn(*btn);

                tmpBtn.rect = r;
                drawPrimitive(PE_PanelButtonCommand, &tmpBtn, painter, widget);
            }
            if (btn->features & QStyleOptionButton::HasMenu)
            {
                int   mbi(pixelMetric(PM_MenuButtonIndicator, btn, widget));
                QRect ar(Qt::LeftToRight==btn->direction
                         ? btn->rect.right() - (mbi+6)
                         : btn->rect.x() + 6,
                         ((btn->rect.height() - mbi)/2),
                         mbi, mbi);

                if(option->state &(State_On | State_Sunken))
                    ar.adjust(1, 1, 1, 1);

                drawArrow(painter, ar, PE_IndicatorArrowDown, MO_ARROW(QPalette::ButtonText));
            }
        }
        break;
    case CE_PushButtonLabel:
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            uint tf(Qt::AlignVCenter | Qt::TextShowMnemonic);

            if (!styleHint(SH_UnderlineShortcut, button, widget))
                tf |= Qt::TextHideMnemonic;

            if (!button->icon.isNull())
            {
                //Center both icon and text
                QIcon::Mode mode(button->state&State_Enabled ? QIcon::Normal : QIcon::Disabled);

                if (QIcon::Normal==mode && button->state&State_HasFocus)
                    mode = QIcon::Active;

                QIcon::State state((button->state&State_On) || (button->state&State_Sunken) ? QIcon::On : QIcon::Off);
                QPixmap      pixmap(getIconPixmap(button->icon, button->iconSize, mode, state));
                int          labelWidth(pixmap.width()),
                    labelHeight(pixmap.height()),
                    iconSpacing (4);//### 4 is currently hardcoded in QPushButton::sizeHint()

                if (!button->text.isEmpty())
                    labelWidth += (button->fontMetrics.boundingRect(r, tf, button->text).width() + iconSpacing);

                QRect iconRect(r.x() + (r.width() - labelWidth) / 2,
                               r.y() + (r.height() - labelHeight) / 2,
                               pixmap.width(), pixmap.height());

                iconRect = visualRect(button->direction, r, iconRect);

                tf |= Qt::AlignLeft; //left align, we adjust the text-rect instead

                if (Qt::RightToLeft==button->direction)
                    r.setRight(iconRect.left() - iconSpacing);
                else
                    r.setLeft(iconRect.left() + iconRect.width() + iconSpacing);

                if (button->state & (State_On|State_Sunken))
                    iconRect.translate(pixelMetric(PM_ButtonShiftHorizontal, option, widget),
                                       pixelMetric(PM_ButtonShiftVertical, option, widget));
                painter->drawPixmap(iconRect, pixmap);
            }
            else
                tf |= Qt::AlignHCenter;

            if (button->state & (State_On|State_Sunken))
                r.translate(pixelMetric(PM_ButtonShiftHorizontal, option, widget),
                            pixelMetric(PM_ButtonShiftVertical, option, widget));

            // The following is mainly for DejaVu Sans 11...
            if(button->fontMetrics.height()==19 && r.height()==(23+((opts.thin&THIN_BUTTONS) ? 0 : 2)))
                r.translate(0, 1);

            if (button->features&QStyleOptionButton::HasMenu)
            {
                int mbi(pixelMetric(PM_MenuButtonIndicator, button, widget));

                if (Qt::LeftToRight==button->direction)
                    r = r.adjusted(0, 0, -mbi, 0);
                else
                    r = r.adjusted(mbi, 0, 0, 0);

                if(APP_SKYPE==theThemedApp)
                {
                    // Skype seems to draw a blurry arrow in the lower right corner,
                    // ...draw over this with a nicer sharper arrow...
                    QRect ar(button->rect.x()+(button->rect.width()-(LARGE_ARR_WIDTH+3)),
                             button->rect.y()+(button->rect.height()-(LARGE_ARR_HEIGHT+2)),
                             LARGE_ARR_WIDTH,
                             LARGE_ARR_HEIGHT);

                    if(option->state &(State_On | State_Sunken))
                        ar.adjust(1, 1, 1, 1);
                    drawArrow(painter, ar, PE_IndicatorArrowDown, MO_ARROW(QPalette::ButtonText));
                }
            }

            int num(opts.embolden && button->features&QStyleOptionButton::DefaultButton ? 2 : 1);

            for(int i=0; i<num; ++i)
                drawItemTextWithRole(painter, r.adjusted(i, 0, i, 0), tf, palette, (button->state&State_Enabled),
                                     button->text, QPalette::ButtonText);
        }
        break;
    case CE_ComboBoxLabel:
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
        {
            QRect editRect = subControlRect(CC_ComboBox, comboBox, SC_ComboBoxEditField, widget);
            bool  sunken=!comboBox->editable && (state&(State_On|State_Sunken));
            int   shiftH=sunken ? pixelMetric(PM_ButtonShiftHorizontal, option, widget) : 0,
                shiftV=sunken ? pixelMetric(PM_ButtonShiftVertical, option, widget) : 0;

            painter->save();

            if (!comboBox->currentIcon.isNull())
            {
                QPixmap pixmap = getIconPixmap(comboBox->currentIcon, comboBox->iconSize, state);
                QRect   iconRect(editRect);

                iconRect.setWidth(comboBox->iconSize.width() + 5);
                if(!comboBox->editable)
                    iconRect = alignedRect(QApplication::layoutDirection(), Qt::AlignLeft|Qt::AlignVCenter,
                                           iconRect.size(), editRect);
                if (comboBox->editable)
                {
                    int adjust=opts.etchEntry ? 2 : 1;

                    if(opts.square&SQUARE_ENTRY || opts.round<ROUND_FULL)
                        painter->fillRect(iconRect.adjusted(adjust-1, adjust, -(adjust-1), -adjust), palette.brush(QPalette::Base));
                    else
                    {
                        painter->fillRect(iconRect.adjusted(1, adjust, -1, -adjust), palette.brush(QPalette::Base));
                        painter->fillRect(iconRect.adjusted(0, adjust+1, 0, -(adjust+1)), palette.brush(QPalette::Base));
                    }
                }

                if (sunken)
                    iconRect.translate(shiftH, shiftV);

                drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

                if (reverse)
                    editRect.translate(-4 - comboBox->iconSize.width(), 0);
                else
                    editRect.translate(comboBox->iconSize.width() + 4, 0);
            }

            if (!comboBox->currentText.isEmpty() && !comboBox->editable)
            {
                if (sunken)
                    editRect.translate(shiftH, shiftV);

                int margin=comboBox->frame && widget && widget->rect().height()<(DO_EFFECT ? 22 : 20)  ? 4 : 0;
                editRect.adjust(1, -margin, -1, margin);
                painter->setClipRect(editRect);
                drawItemTextWithRole(painter, editRect, Qt::AlignLeft|Qt::AlignVCenter, palette,
                                     state&State_Enabled, comboBox->currentText, QPalette::ButtonText);
            }
            painter->restore();
        }
        break;
    case CE_MenuBarEmptyArea:
    {
        painter->save();

        drawMenuOrToolBarBackground(widget, painter, r, option);
        if (TB_NONE!=opts.toolbarBorders && widget && widget->parentWidget() &&
            (qobject_cast<const QMainWindow *>(widget->parentWidget())))
        {
            const QColor *use=menuColors(option, itsActive);
            bool         dark(TB_DARK==opts.toolbarBorders || TB_DARK_ALL==opts.toolbarBorders);

            if(TB_DARK_ALL==opts.toolbarBorders || TB_LIGHT_ALL==opts.toolbarBorders)
            {
                painter->setPen(use[0]);
                painter->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                painter->drawLine(r.x(), r.y(), r.x(), r.y()+r.width()-1);
                painter->setPen(use[dark ? 3 : 4]);
                painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                painter->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
            }
            else
            {
                painter->setPen(use[dark ? 3 : 4]);
                painter->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
            }
        }
        painter->restore();
    }
    break;
    case CE_TabBarTabLabel:
        if (const QStyleOptionTab *_tab =
            qstyleoption_cast<const QStyleOptionTab*>(option))
        {
            QStyleOptionTab tab(*_tab);
            bool verticalTabs(QTabBar::RoundedEast == tab.shape ||
                              QTabBar::RoundedWest == tab.shape ||
                              QTabBar::TriangularEast == tab.shape ||
                              QTabBar::TriangularWest == tab.shape);
            bool toolbarTab = (!opts.toolbarTabs &&
                               widget && widget->parentWidget() &&
                               qobject_cast<QToolBar*>(widget->parentWidget()));

            if (verticalTabs) {
                painter->save();
                int newX, newY, newRot;
                if (QTabBar::RoundedEast == tab.shape ||
                    QTabBar::TriangularEast == tab.shape) {
                    newX = r.width();
                    newY = r.y();
                    newRot = 90;
                } else {
                    newX = 0;
                    newY = r.y() + r.height();
                    newRot = -90;
                }
                r.setRect(0, 0, r.height(), r.width());

                QTransform m;
                m.translate(newX, newY);
                m.rotate(newRot);
                painter->setTransform(m, true);
            }

            int alignment(Qt::AlignVCenter | Qt::TextShowMnemonic |
                          (opts.centerTabText ? Qt::AlignHCenter : Qt::AlignLeft));

            if (!styleHint(SH_UnderlineShortcut, option, widget))
                alignment |= Qt::TextHideMnemonic;

            if(toolbarTab)
                tab.state &= ~State_Selected;
            r = subElementRect(SE_TabBarTabText, &tab, widget);
            if (!tab.icon.isNull()) {
                QSize iconSize(tab.iconSize);
                if (!iconSize.isValid()) {
                    int iconExtent(pixelMetric(PM_SmallIconSize));
                    iconSize = QSize(iconExtent, iconExtent);
                }

                QPixmap tabIcon(getIconPixmap(tab.icon, iconSize,
                                              state & State_Enabled));
                QSize tabIconSize = tab.icon.actualSize(iconSize,
                                                        tab.state & State_Enabled
                                                        ? QIcon::Normal
                                                        : QIcon::Disabled);

                int offset = 4,
                    left = option->rect.left();
                if (tab.leftButtonSize.isNull() || tab.leftButtonSize.width()<=0) {
                    offset += 2;
                } else {
                    left += tab.leftButtonSize.width() + 2;
                }
                QRect iconRect = QRect(left + offset,
                                       r.center().y() - tabIcon.height() / 2,
                                       tabIconSize.width(),
                                       tabIconSize.height());
                if (!verticalTabs)
                    iconRect = visualRect(option->direction, option->rect,
                                          iconRect);
                painter->drawPixmap(iconRect.x(), iconRect.y(), tabIcon);
            }

            if(!_tab->text.isEmpty()) {
                drawItemTextWithRole(painter, r, alignment, _tab->palette,
                                     _tab->state & State_Enabled, _tab->text,
                                     !opts.stdSidebarButtons && toolbarTab &&
                                     state&State_Selected ?
                                     QPalette::HighlightedText :
                                     QPalette::WindowText);
            }

            if (verticalTabs)
                painter->restore();

            if (tab.state & State_HasFocus) {
                const int constOffset = 1 + pixelMetric(PM_DefaultFrameWidth);

                int x1 = tab.rect.left();
                int x2 = tab.rect.right() - 1;
                QStyleOptionFocusRect fropt;
                fropt.QStyleOption::operator=(*_tab);
                fropt.rect.setRect(x1 + 1 + constOffset,
                                   tab.rect.y() + constOffset,
                                   x2 - x1 - 2 * constOffset,
                                   tab.rect.height() - 2 * constOffset);

                fropt.state|=State_Horizontal;
                if(FOCUS_LINE != opts.focus) {
                    if(QTabBar::RoundedNorth == tab.shape ||
                       QTabBar::TriangularNorth == tab.shape) {
                        fropt.rect.adjust(0, 1, 0, 0);
                    }
                } else if(TAB_MO_BOTTOM == opts.tabMouseOver &&
                          FOCUS_LINE==opts.focus)
                    switch(tab.shape) {
                    case QTabBar::RoundedNorth:
                    case QTabBar::TriangularNorth:
                        fropt.rect.adjust(0, 0, 0, 1);
                        break;
                    case QTabBar::RoundedEast:
                    case QTabBar::TriangularEast:
                        fropt.rect.adjust(-2, 0, -(fropt.rect.width()+1), 0);
                        fropt.state&=~State_Horizontal;
                        break;
                    case QTabBar::RoundedSouth:
                    case QTabBar::TriangularSouth:
                        fropt.rect.adjust(0, 0, 0, 1);
                        break;
                    case QTabBar::RoundedWest:
                    case QTabBar::TriangularWest:
                        fropt.rect.adjust(0, 0, 2, 0);
                        fropt.state&=~State_Horizontal;
                    default:
                        break;
                    }

                drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
            }
        }
        break;
    case CE_TabBarTabShape:
        if(!opts.toolbarTabs && widget && widget->parentWidget() && qobject_cast<QToolBar *>(widget->parentWidget()))
        {
            QStyleOption opt(*option);
            if(state&State_Selected)
                opt.state|=State_On;
            if(opts.stdSidebarButtons)
            {
                if(state&(State_Selected|State_MouseOver))
                {
                    opt.state|=STATE_TBAR_BUTTON;
                    drawPrimitive(PE_PanelButtonTool, &opt, painter, widget);
                }
            }
            else
                drawSideBarButton(painter, r, &opt, widget);
        }
        else if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option))
        {
            bool onlyTab(widget && widget->parentWidget() ?
                         qobject_cast<const QTabWidget*>(widget->parentWidget()) ?
                         false : true : false),
                selected(state&State_Selected),
                horiz(QTabBar::RoundedNorth==tab->shape || QTabBar::RoundedSouth==tab->shape);
            QRect        r2(r);
            bool         rtlHorTabs(Qt::RightToLeft==tab->direction && horiz),
                oneTab(QStyleOptionTab::OnlyOneTab==tab->position),
                leftCornerWidget(tab->cornerWidgets&QStyleOptionTab::LeftCornerWidget),
                rightCornerWidget(tab->cornerWidgets&QStyleOptionTab::RightCornerWidget),
                firstTab((tab->position == (Qt::LeftToRight==tab->direction || !horiz ?
                                            QStyleOptionTab::Beginning : QStyleOptionTab::End)) || oneTab),
                lastTab((tab->position == (Qt::LeftToRight==tab->direction  || !horiz ?
                                           QStyleOptionTab::End : QStyleOptionTab::Beginning)) || oneTab);
            int          tabBarAlignment(styleHint(SH_TabBar_Alignment, tab, widget)),
                tabOverlap(oneTab ? 0 : pixelMetric(PM_TabBarTabOverlap, option, widget)),
                moOffset(ROUNDED_NONE==opts.round || TAB_MO_TOP!=opts.tabMouseOver ? 1 : opts.round),
                highlightOffset(opts.highlightTab && opts.round>ROUND_SLIGHT ? 2 : 1),
                highlightBorder(opts.round>ROUND_FULL ? 4 : 3),
                sizeAdjust(!selected && TAB_MO_GLOW==opts.tabMouseOver ? 1 : 0);
            bool leftAligned((!rtlHorTabs && Qt::AlignLeft==tabBarAlignment) ||
                             (rtlHorTabs && Qt::AlignRight==tabBarAlignment)),
                rightAligned((!rtlHorTabs && Qt::AlignRight==tabBarAlignment) ||
                             (rtlHorTabs && Qt::AlignLeft==tabBarAlignment)),
                docMode(tab->documentMode),
                docFixLeft(!leftCornerWidget && leftAligned && firstTab && (docMode || onlyTab)),
                fixLeft(!onlyTab && !leftCornerWidget && leftAligned && firstTab && !docMode),
                fixRight(!onlyTab && !rightCornerWidget && rightAligned && lastTab && !docMode),
                mouseOver(state&State_Enabled && state&State_MouseOver),
                glowMo(!selected && mouseOver && opts.coloredMouseOver &&
                       TAB_MO_GLOW==opts.tabMouseOver),
                thin(opts.thin&THIN_FRAMES),
                drawOuterGlow(glowMo && !thin);
            const QColor *use(backgroundColors(option));
            QColor       fill(getTabFill(selected, mouseOver, use));
            double       radius=qtcGetRadius(&opts, r.width(), r.height(), WIDGET_TAB_TOP, RADIUS_EXTERNAL);
            EBorder      borderProfile(selected || opts.borderInactiveTab
                                       ? opts.borderTab
                                       ? BORDER_LIGHT
                                       : BORDER_RAISED
                                       : BORDER_FLAT);

            painter->save();

            if(!selected && (100!=opts.bgndOpacity || 100!=opts.dlgOpacity))
            {
                QWidget *top=widget ? widget->window() : 0L;
                bool    isDialog=top && Qt::Dialog==(top->windowFlags() & Qt::WindowType_Mask);

                // Note: opacity is divided by 150 to make dark inactive tabs more translucent
                if(isDialog && 100!=opts.dlgOpacity)
                    fill.setAlphaF(opts.dlgOpacity/150.0);
                else if(!isDialog && 100!=opts.bgndOpacity)
                    fill.setAlphaF(opts.bgndOpacity/150.0);
            }

            switch(tab->shape)
            {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            {
                int round=selected || oneTab || TAB_MO_GLOW==opts.tabMouseOver || opts.roundAllTabs
                    ? ROUNDED_TOP
                    : firstTab
                    ? ROUNDED_TOPLEFT
                    : lastTab
                    ? ROUNDED_TOPRIGHT
                    : ROUNDED_NONE;
                if(!selected)
                    r.adjust(0, 2, 0, -2);

                if(!firstTab)
                    r.adjust(-tabOverlap, 0, 0, 0);
                painter->setClipPath(buildPath(r.adjusted(0, 0, 0, 4), WIDGET_TAB_TOP, round, radius));
                fillTab(painter, r.adjusted(1+sizeAdjust, 1, -(1+sizeAdjust), 0), option, fill, true, WIDGET_TAB_TOP, (docMode || onlyTab));
                // This clipping (for selected) helps with plasma's tabs and nvidia
                if(selected || thin)
                    painter->setClipRect(r2.adjusted(-1, 0, 1, -1));
                else
                    painter->setClipping(false);
                drawBorder(painter, r.adjusted(sizeAdjust, 0, -sizeAdjust, 4), option, round, glowMo ? itsMouseOverCols : 0L,
                           WIDGET_TAB_TOP, borderProfile, false);
                if(drawOuterGlow)
                    drawGlow(painter, r.adjusted(0, -1, 0, 5), WIDGET_TAB_TOP);

                if(selected || thin)
                    painter->setClipping(false);

                if(selected)
                {
                    if(!thin)
                    {
                        painter->setPen(use[0]);

                        // The point drawn below is because of the clipping above...
                        if(fixLeft)
                            painter->drawPoint(r2.x()+1, r2.y()+r2.height()-1);
                        else
                            painter->drawLine(r2.left()-1, r2.bottom(), r2.left(), r2.bottom());
                        if(!fixRight)
                            painter->drawLine(r2.right()-1, r2.bottom(), r2.right(), r2.bottom());
                    }

                    if(docFixLeft)
                    {
                        QColor col(use[QTC_STD_BORDER]);
                        col.setAlphaF(0.5);
                        painter->setPen(col);
                        painter->drawPoint(r2.x(), r2.y()+r2.height()-1);
                    }
                }
                else
                {
                    int l(fixLeft ? r2.left()+(opts.round>ROUND_SLIGHT && !(opts.square&SQUARE_TAB_FRAME) ? 2 : 1) : r2.left()-1),
                        r(fixRight ? r2.right()-2 : r2.right()+1);
                    painter->setPen(use[QTC_STD_BORDER]);
                    painter->drawLine(l, r2.bottom()-1, r, r2.bottom()-1);
                    if(!thin)
                    {
                        painter->setPen(use[0]);
                        painter->drawLine(l, r2.bottom(), r, r2.bottom());
                    }
                }

                if(selected)
                {
                    if(opts.highlightTab)
                    {
                        QColor col(itsHighlightCols[0]);
                        painter->setRenderHint(QPainter::Antialiasing, true);
                        painter->setPen(col);
                        drawAaLine(painter, r.left()+highlightOffset, r.top()+1, r.right()-highlightOffset, r.top()+1);
                        col.setAlphaF(0.5);
                        painter->setPen(col);
                        drawAaLine(painter, r.left()+1, r.top()+2, r.right()-1, r.top()+2);
                        painter->setRenderHint(QPainter::Antialiasing, false);
                        painter->setClipRect(QRect(r.x(), r.y(), r.width(), highlightBorder));
                        drawBorder(painter, r, option, ROUNDED_ALL, itsHighlightCols, WIDGET_TAB_TOP, BORDER_FLAT, false, 3);
                    }

                    if(opts.colorSelTab)
                        colorTab(painter, r.adjusted(1+sizeAdjust, 1, -(1+sizeAdjust), 0), true, WIDGET_TAB_TOP, round);
                }
                else if(mouseOver && opts.coloredMouseOver && TAB_MO_GLOW!=opts.tabMouseOver)
                    drawHighlight(painter, QRect(r.x()+(firstTab ? moOffset : 1),
                                                 r.y()+(TAB_MO_TOP==opts.tabMouseOver ? 0 : r.height()-1),
                                                 r.width()-(firstTab || lastTab ? moOffset : 1), 2),
                                  true, TAB_MO_TOP==opts.tabMouseOver);
                break;
            }
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            {
                int round=selected || oneTab || TAB_MO_GLOW==opts.tabMouseOver || opts.roundAllTabs
                    ? ROUNDED_BOTTOM
                    : firstTab
                    ? ROUNDED_BOTTOMLEFT
                    : lastTab
                    ? ROUNDED_BOTTOMRIGHT
                    : ROUNDED_NONE;
                if(!selected)
                    r.adjust(0, 2, 0, -2);
                if(!firstTab)
                    r.adjust(-tabOverlap, 0, 0, 0);

                painter->setClipPath(buildPath(r.adjusted(0, -4, 0, 0), WIDGET_TAB_BOT, round, radius));
                fillTab(painter, r.adjusted(1+sizeAdjust, 0, -(1+sizeAdjust), -1), option, fill, true, WIDGET_TAB_BOT, (docMode || onlyTab));
                if(thin)
                    painter->setClipRect(r2.adjusted(0, 1, 0, 0));
                else
                    painter->setClipping(false);
                drawBorder(painter, r.adjusted(sizeAdjust, -4, -sizeAdjust, 0), option, round, glowMo ? itsMouseOverCols : 0L,
                           WIDGET_TAB_BOT, borderProfile, false);
                if(thin)
                    painter->setClipping(false);
                if(drawOuterGlow)
                    drawGlow(painter, r.adjusted(0, -5, 0, 1), WIDGET_TAB_BOT);

                if(selected)
                {
                    if(!thin)
                    {
                        painter->setPen(use[opts.borderTab ? 0 : FRAME_DARK_SHADOW]);
                        if(!fixLeft)
                            painter->drawPoint(r2.left()-(TAB_MO_GLOW==opts.tabMouseOver ? 0 : 1), r2.top());
                        if(!fixRight)
                            painter->drawLine(r2.right()-(TAB_MO_GLOW==opts.tabMouseOver ? 0 : 1), r2.top(), r2.right(), r2.top());
                    }
                    if(docFixLeft)
                    {
                        QColor col(use[QTC_STD_BORDER]);
                        col.setAlphaF(0.5);
                        painter->setPen(col);
                        painter->drawPoint(r2.x(), r2.y());
                    }
                }
                else
                {
                    int l(fixLeft ? r2.left()+(opts.round>ROUND_SLIGHT && !(opts.square&SQUARE_TAB_FRAME)? 2 : 1) : r2.left()-1),
                        r(fixRight ? r2.right()-2 : r2.right());
                    painter->setPen(use[QTC_STD_BORDER]);
                    painter->drawLine(l, r2.top()+1, r, r2.top()+1);
                    if(!thin)
                    {
                        painter->setPen(use[opts.borderTab ? 0 : FRAME_DARK_SHADOW]);
                        painter->drawLine(l, r2.top(), r, r2.top());
                    }
                }

                if(selected)
                {
                    if(opts.highlightTab)
                    {
                        QColor col(itsHighlightCols[0]);
                        painter->setRenderHint(QPainter::Antialiasing, true);
                        painter->setPen(col);
                        drawAaLine(painter, r.left()+highlightOffset, r.bottom()-1, r.right()-highlightOffset, r.bottom()-1);
                        col.setAlphaF(0.5);
                        painter->setPen(col);
                        drawAaLine(painter, r.left()+1, r.bottom()-2, r.right()-1, r.bottom()-2);
                        painter->setRenderHint(QPainter::Antialiasing, false);
                        painter->setClipRect(QRect(r.x(), r.y()+r.height()-highlightBorder, r.width(), r.y()+r.height()-1));
                        drawBorder(painter, r, option, ROUNDED_ALL, itsHighlightCols, WIDGET_TAB_BOT, BORDER_FLAT, false, 3);
                    }

                    if(opts.colorSelTab)
                        colorTab(painter, r.adjusted(1+sizeAdjust, 0, -(1+sizeAdjust), -1), true, WIDGET_TAB_BOT, round);
                }
                else if(mouseOver && opts.coloredMouseOver && TAB_MO_GLOW!=opts.tabMouseOver)
                    drawHighlight(painter, QRect(r.x()+(firstTab ? moOffset : 1),
                                                 r.y()+(TAB_MO_TOP==opts.tabMouseOver ? r.height()-2 : -1),
                                                 r.width()-(firstTab || lastTab ? moOffset : 1), 2),
                                  true, TAB_MO_TOP!=opts.tabMouseOver);
                break;
            }
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            {
                int round=selected || oneTab || TAB_MO_GLOW==opts.tabMouseOver || opts.roundAllTabs
                    ? ROUNDED_LEFT
                    : firstTab
                    ? ROUNDED_TOPLEFT
                    : lastTab
                    ? ROUNDED_BOTTOMLEFT
                    : ROUNDED_NONE;
                if(!selected)
                    r.adjust(2, 0, -2, 0);

                if(!firstTab)
                    r.adjust(0, -tabOverlap, 0, 0);
                painter->setClipPath(buildPath(r.adjusted(0, 0, 4, 0), WIDGET_TAB_TOP, round, radius));
                fillTab(painter, r.adjusted(1, sizeAdjust, 0, -(1+sizeAdjust)), option, fill, false, WIDGET_TAB_TOP, (docMode || onlyTab));
                if(thin)
                    painter->setClipRect(r2.adjusted(0, 0, -1, 0));
                else
                    painter->setClipping(false);
                drawBorder(painter, r.adjusted(0, sizeAdjust, 4, -sizeAdjust), option, round, glowMo ? itsMouseOverCols : 0L,
                           WIDGET_TAB_TOP, borderProfile, false);
                if(thin)
                    painter->setClipping(false);
                if(drawOuterGlow)
                    drawGlow(painter, r.adjusted(-1, 0, 5, 0), WIDGET_TAB_TOP);

                if(selected)
                {
                    if(!thin)
                    {
                        painter->setPen(use[0]);
                        if(!firstTab)
                            painter->drawPoint(r2.right(), r2.top()-(TAB_MO_GLOW==opts.tabMouseOver ? 0 : 1));
                        painter->drawLine(r2.right(), r2.bottom()-1, r2.right(), r2.bottom());
                    }
                }
                else
                {
                    int t(firstTab ? r2.top()+(opts.round>ROUND_SLIGHT && !(opts.square&SQUARE_TAB_FRAME)? 2 : 1) : r2.top()-1),
                        b(/*lastTab ? r2.bottom()-2 : */ r2.bottom()+1);

                    painter->setPen(use[QTC_STD_BORDER]);
                    painter->drawLine(r2.right()-1, t, r2.right()-1, b);
                    if(!thin)
                    {
                        painter->setPen(use[0]);
                        painter->drawLine(r2.right(), t, r2.right(), b);
                    }
                }

                if(selected)
                {
                    if(opts.highlightTab)
                    {
                        QColor col(itsHighlightCols[0]);
                        painter->setRenderHint(QPainter::Antialiasing, true);
                        painter->setPen(col);
                        drawAaLine(painter, r.left()+1, r.top()+highlightOffset, r.left()+1, r.bottom()-highlightOffset);
                        col.setAlphaF(0.5);
                        painter->setPen(col);
                        drawAaLine(painter, r.left()+2, r.top()+1, r.left()+2, r.bottom()-1);
                        painter->setRenderHint(QPainter::Antialiasing, false);
                        painter->setClipRect(QRect(r.x(), r.y(), highlightBorder, r.height()));
                        drawBorder(painter, r, option, ROUNDED_ALL, itsHighlightCols, WIDGET_TAB_TOP, BORDER_FLAT, false, 3);
                    }

                    if(opts.colorSelTab)
                        colorTab(painter, r.adjusted(1, sizeAdjust, 0, -(1+sizeAdjust)), false, WIDGET_TAB_TOP, round);
                }
                else if(mouseOver && opts.coloredMouseOver && TAB_MO_GLOW!=opts.tabMouseOver)
                    drawHighlight(painter, QRect(r.x()+(TAB_MO_TOP==opts.tabMouseOver ? 0 : r.width()-1),
                                                 r.y()+(firstTab ? moOffset : 1),
                                                 2, r.height()-(firstTab || lastTab ? moOffset : 1)),
                                  false, TAB_MO_TOP==opts.tabMouseOver);
                break;
            }
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            {
                int round=selected || oneTab || TAB_MO_GLOW==opts.tabMouseOver || opts.roundAllTabs
                    ? ROUNDED_RIGHT
                    : firstTab
                    ? ROUNDED_TOPRIGHT
                    : lastTab
                    ? ROUNDED_BOTTOMRIGHT
                    : ROUNDED_NONE;
                if(!selected)
                    r.adjust(2, 0, -2, 0);

                if(!firstTab)
                    r.adjust(0, -tabOverlap, 0, 0);
                painter->setClipPath(buildPath(r.adjusted(-4, 0, 0, 0), WIDGET_TAB_BOT, round, radius));
                fillTab(painter, r.adjusted(0, sizeAdjust, -1, -(1+sizeAdjust)), option, fill, false, WIDGET_TAB_BOT, (docMode || onlyTab));
                if(thin)
                    painter->setClipRect(r2.adjusted(1, 0, 0, 0));
                else
                    painter->setClipping(false);
                drawBorder(painter, r.adjusted(-4, sizeAdjust, 0, -sizeAdjust), option, round, glowMo ? itsMouseOverCols : 0L,
                           WIDGET_TAB_BOT, borderProfile, false);
                if(thin)
                    painter->setClipping(false);
                if(drawOuterGlow)
                    drawGlow(painter, r.adjusted(-5, 0, 1, 0), WIDGET_TAB_BOT);

                if(selected)
                {
                    if(!thin)
                    {
                        painter->setPen(use[opts.borderTab ? 0 : FRAME_DARK_SHADOW]);
                        if(!firstTab)
                            painter->drawPoint(r2.left(), r2.top()-(TAB_MO_GLOW==opts.tabMouseOver ? 0 : 1));
                        painter->drawLine(r2.left(), r2.bottom()-(TAB_MO_GLOW==opts.tabMouseOver ? 0 : 1), r2.left(), r2.bottom());
                    }
                }
                else
                {
                    int t(firstTab ? r2.top()+(opts.round>ROUND_SLIGHT && !(opts.square&SQUARE_TAB_FRAME)? 2 : 1) : r2.top()-1),
                        b(/*lastTab ? r2.bottom()-2 : */ r2.bottom()+1);

                    painter->setPen(use[QTC_STD_BORDER]);
                    painter->drawLine(r2.left()+1, t, r2.left()+1, b);
                    if(!thin)
                    {
                        painter->setPen(use[opts.borderTab ? 0 : FRAME_DARK_SHADOW]);
                        painter->drawLine(r2.left(), t, r2.left(), b);
                    }
                }

                if(selected)
                {
                    if(opts.highlightTab)
                    {
                        QColor col(itsHighlightCols[0]);
                        painter->setRenderHint(QPainter::Antialiasing, true);
                        painter->setPen(col);
                        drawAaLine(painter, r.right()-1, r.top()+highlightOffset, r.right()-1, r.bottom()-highlightOffset);
                        col.setAlphaF(0.5);
                        painter->setPen(col);
                        drawAaLine(painter, r.right()-2, r.top()+1, r.right()-2, r.bottom()-1);
                        painter->setRenderHint(QPainter::Antialiasing, false);
                        painter->setClipRect(QRect(r.x()+r.width()-highlightBorder, r.y(), r.x()+r.width()-1, r.height()));
                        drawBorder(painter, r, option, ROUNDED_ALL, itsHighlightCols, WIDGET_TAB_TOP, BORDER_FLAT, false, 3);
                    }

                    if(opts.colorSelTab)
                        colorTab(painter, r.adjusted(0, sizeAdjust, -1, -(1+sizeAdjust)), false, WIDGET_TAB_BOT, round);
                }
                else if(mouseOver && opts.coloredMouseOver && TAB_MO_GLOW!=opts.tabMouseOver)
                    drawHighlight(painter, QRect(r.x()+(TAB_MO_TOP==opts.tabMouseOver ? r.width()-2 : -1),
                                                 r.y()+(firstTab ? moOffset : 1),
                                                 2, r.height()-(firstTab || lastTab ? moOffset : 1)),
                                  false, TAB_MO_TOP!=opts.tabMouseOver);
                break;
            }
            }
            painter->restore();
        }
        break;
    case CE_ScrollBarAddLine:
    case CE_ScrollBarSubLine:
    {
        QRect            br(r),
            ar(r);
        const QColor     *use(state&State_Enabled ? itsButtonCols : itsBackgroundCols); // buttonColors(option));
        bool             reverse(option && Qt::RightToLeft==option->direction);
        PrimitiveElement pe=state&State_Horizontal
            ? CE_ScrollBarAddLine==element ? (reverse ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight)
            : (reverse ? PE_IndicatorArrowRight : PE_IndicatorArrowLeft)
            : CE_ScrollBarAddLine==element ? PE_IndicatorArrowDown : PE_IndicatorArrowUp;
        int              round=PE_IndicatorArrowRight==pe ? ROUNDED_RIGHT :
            PE_IndicatorArrowLeft==pe ? ROUNDED_LEFT :
            PE_IndicatorArrowDown==pe ? ROUNDED_BOTTOM :
            PE_IndicatorArrowUp==pe ? ROUNDED_TOP : ROUNDED_NONE;

        switch(opts.scrollbarType)
        {
        default:
        case SCROLLBAR_WINDOWS:
            break;
        case SCROLLBAR_KDE:
        case SCROLLBAR_PLATINUM:
            if(!reverse && PE_IndicatorArrowLeft==pe && r.x()>3)
            {
                round=ROUNDED_NONE;
                br.adjust(0, 0, 1, 0);
                if(opts.flatSbarButtons || !opts.vArrows)
                    ar.adjust(1, 0, 1, 0);
            }
            else if(reverse && PE_IndicatorArrowRight==pe && r.x()>3)
            {
                if(SCROLLBAR_PLATINUM==opts.scrollbarType)
                {
                    round=ROUNDED_NONE;
                    br.adjust(-1, 0, 0, 0);
                    if(opts.flatSbarButtons || !opts.vArrows)
                        ar.adjust(-1, 0, -1, 0);
                }
                else
                {
                    if(r.x()<pixelMetric(PM_ScrollBarExtent, option, widget)+2)
                        round=ROUNDED_NONE;
                    br.adjust(0, 0, 1, 0);
                    if(opts.flatSbarButtons || !opts.vArrows)
                        ar.adjust(1, 0, 1, 0);
                }
            }
            else if(PE_IndicatorArrowUp==pe && r.y()>3)
            {
                round=ROUNDED_NONE;
                br.adjust(0, 0, 0, 1);
                if(opts.flatSbarButtons || !opts.vArrows)
                    ar.adjust(0, 1, 0, 1);
            }
            break;
        case SCROLLBAR_NEXT:
            if(!reverse && PE_IndicatorArrowRight==pe)
            {
                round=ROUNDED_NONE;
                br.adjust(-1, 0, 0, 0);
                if(opts.flatSbarButtons || !opts.vArrows)
                    ar.adjust(-1, 0, 0, -1);
            }
            else if(reverse && PE_IndicatorArrowLeft==pe)
            {
                round=ROUNDED_NONE;
                br.adjust(0, 0, 1, 0);
                if(opts.flatSbarButtons || !opts.vArrows)
                    ar.adjust(-1, 0, 0, 1);
            }
            else if(PE_IndicatorArrowDown==pe)
            {
                round=ROUNDED_NONE;
                br.adjust(0, -1, 0, 0);
                if(opts.flatSbarButtons || !opts.vArrows)
                    ar.adjust(0, -1, 0, -1);
            }
            break;
        }

        painter->save();
        if(opts.flatSbarButtons && !IS_FLAT(opts.sbarBgndAppearance) /*&& SCROLLBAR_NONE!=opts.scrollbarType*/)
            drawBevelGradientReal(palette.brush(QPalette::Background).color(), painter, r, state&State_Horizontal, false,
                                  opts.sbarBgndAppearance, WIDGET_SB_BGND);

        QStyleOption opt(*option);

        opt.state|=State_Raised;

        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            if((CE_ScrollBarSubLine==element && slider->sliderValue==slider->minimum) ||
               (CE_ScrollBarAddLine==element && slider->sliderValue==slider->maximum))
                opt.state&=~(State_MouseOver|State_Sunken|State_On);

            if(slider->minimum==slider->maximum && opt.state&State_Enabled)
                opt.state^=State_Enabled;
        }

        if(opts.flatSbarButtons)
            opt.state&=~(State_Sunken|State_On);
        else
            drawLightBevel(painter, br, &opt, widget, round, getFill(&opt, use), use, true, WIDGET_SB_BUTTON);

        opt.rect = ar;

        if(!(opt.state&State_Enabled))
            opt.palette.setCurrentColorGroup(QPalette::Disabled);

        if(opt.palette.text().color()!=opt.palette.buttonText().color()) // The following fixes gwenviews scrollbars...
            opt.palette.setColor(QPalette::Text, opt.palette.buttonText().color());

        drawPrimitive(pe, &opt, painter, widget);
        painter->restore();
        break;
    }
    case CE_ScrollBarSubPage:
    case CE_ScrollBarAddPage:
    {
        const QColor *use(itsBackgroundCols); // backgroundColors(option));
        int          borderAdjust(0);

        painter->save();
#ifndef SIMPLE_SCROLLBARS
        if(ROUNDED && (SCROLLBAR_NONE==opts.scrollbarType || opts.flatSbarButtons))
            painter->fillRect(r, palette.background().color());
#endif

        switch(opts.scrollbarType)
        {
        case SCROLLBAR_KDE:
        case SCROLLBAR_WINDOWS:
            borderAdjust=1;
            break;
        case SCROLLBAR_PLATINUM:
            if(CE_ScrollBarAddPage==element)
                borderAdjust=1;
            break;
        case SCROLLBAR_NEXT:
            if(CE_ScrollBarSubPage==element)
                borderAdjust=1;
        default:
            break;
        }

        if(state&State_Horizontal)
        {
            if(IS_FLAT(opts.appearance))
                painter->fillRect(r.x(), r.y()+1, r.width(), r.height()-2, use[2]);
            else
                drawBevelGradient(use[2], painter, QRect(r.x(), r.y()+1, r.width(), r.height()-2),
                                  true, false, opts.grooveAppearance, WIDGET_TROUGH);

#ifndef SIMPLE_SCROLLBARS
            if(ROUNDED && (SCROLLBAR_NONE==opts.scrollbarType || opts.flatSbarButtons))
            {
                if(CE_ScrollBarAddPage==element)
                    drawBorder(painter, r.adjusted(-5, 0, 0, 0), option, ROUNDED_RIGHT, use, WIDGET_TROUGH);
                else
                    drawBorder(painter, r.adjusted(0, 0, 5, 0), option, ROUNDED_LEFT, use, WIDGET_TROUGH);
            }
            else
#endif
                if(CE_ScrollBarAddPage==element)
                    drawBorder(painter, r.adjusted(-5, 0, borderAdjust, 0), option, ROUNDED_NONE, use, WIDGET_TROUGH);
                else
                    drawBorder(painter, r.adjusted(-borderAdjust, 0, 5, 0), option, ROUNDED_NONE, use, WIDGET_TROUGH);
        }
        else
        {
            if(IS_FLAT(opts.appearance))
                painter->fillRect(r.x()+1, r.y(), r.width()-2, r.height(), use[2]);
            else
                drawBevelGradient(use[2], painter, QRect(r.x()+1, r.y(), r.width()-2, r.height()),
                                  false, false, opts.grooveAppearance, WIDGET_TROUGH);

#ifndef SIMPLE_SCROLLBARS
            if(ROUNDED && (SCROLLBAR_NONE==opts.scrollbarType || opts.flatSbarButtons))
            {
                if(CE_ScrollBarAddPage==element)
                    drawBorder(painter, r.adjusted(0, -5, 0, 0), option, ROUNDED_BOTTOM, use, WIDGET_TROUGH);
                else
                    drawBorder(painter, r.adjusted(0, 0, 0, 5), option, ROUNDED_TOP, use, WIDGET_TROUGH);
            }
            else
#endif
                if(CE_ScrollBarAddPage==element)
                    drawBorder(painter, r.adjusted(0, -5, 0, borderAdjust), option, ROUNDED_NONE, use, WIDGET_TROUGH);
                else
                    drawBorder(painter, r.adjusted(0, -borderAdjust, 0, 5), option, ROUNDED_NONE, use, WIDGET_TROUGH);
        }
        painter->restore();
        break;
    }
    case CE_ScrollBarSlider:
        painter->save();
        drawSbSliderHandle(painter, r, option);
        painter->restore();
        break;
#ifdef FIX_DISABLED_ICONS
        // Taken from QStyle - only required so that we can corectly set the disabled icon!!!
    case CE_ToolButtonLabel:
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option))
        {
            int shiftX = 0,
                shiftY = 0;
            if (state & (State_Sunken|State_On))
            {
                shiftX = pixelMetric(PM_ButtonShiftHorizontal, tb, widget);
                shiftY = pixelMetric(PM_ButtonShiftVertical, tb, widget);
            }

            // Arrow type always overrules and is always shown
            bool hasArrow = tb->features & QStyleOptionToolButton::Arrow;

            if (((!hasArrow && tb->icon.isNull()) && !tb->text.isEmpty()) || Qt::ToolButtonTextOnly==tb->toolButtonStyle)
            {
                int alignment = Qt::AlignCenter|Qt::TextShowMnemonic;

                if (!styleHint(SH_UnderlineShortcut, option, widget))
                    alignment |= Qt::TextHideMnemonic;

                r.translate(shiftX, shiftY);

                drawItemTextWithRole(painter, r, alignment, palette, state&State_Enabled, tb->text, QPalette::ButtonText);
            }
            else
            {
                QPixmap pm;
                QSize   pmSize = tb->iconSize;
                QRect   pr = r;

                if (!tb->icon.isNull())
                {
                    QIcon::State state = tb->state & State_On ? QIcon::On : QIcon::Off;
                    QIcon::Mode  mode=!(tb->state & State_Enabled)
                        ? QIcon::Disabled
                        : (state&State_MouseOver) && (state&State_AutoRaise)
                        ? QIcon::Active
                        : QIcon::Normal;
                    QSize        iconSize = tb->iconSize;

                    if (!iconSize.isValid())
                    {
                        int iconExtent = pixelMetric(PM_ToolBarIconSize);
                        iconSize = QSize(iconExtent, iconExtent);
                    }
                    /* Not required?
                       else if(iconSize.width()>iconSize.height())
                       iconSize.setWidth(iconSize.height());
                       else if(iconSize.width()<iconSize.height())
                       iconSize.setHeight(iconSize.width());
                    */

                    if(iconSize.width()>tb->rect.size().width())
                        iconSize=QSize(tb->rect.size().width(), tb->rect.size().width());
                    if(iconSize.height()>tb->rect.size().height())
                        iconSize=QSize(tb->rect.size().height(), tb->rect.size().height());

                    pm=getIconPixmap(tb->icon, iconSize, mode, state);
                    pmSize = pm.size(); // tb->icon.actualSize(iconSize, mode);
                    /*if(pmSize.width()<pm.width())
                      pr.setX(pr.x()+((pm.width()-pmSize.width())));
                      if(pmSize.height()<pm.height())
                      pr.setY(pr.y()+((pm.height()-pmSize.height())));
                    */
                }

                if (Qt::ToolButtonIconOnly!=tb->toolButtonStyle)
                {
                    QRect tr = r;
                    int   alignment = Qt::TextShowMnemonic;

                    painter->setFont(tb->font);
                    if (!styleHint(SH_UnderlineShortcut, option, widget))
                        alignment |= Qt::TextHideMnemonic;

                    if (Qt::ToolButtonTextUnderIcon==tb->toolButtonStyle)
                    {
                        pr.setHeight(pmSize.height() + 6);

                        tr.adjust(0, pr.bottom()-3, 0, 0); // -3);
                        pr.translate(shiftX, shiftY);
                        if (hasArrow)
                            drawTbArrow(this, tb, pr, painter, widget);
                        else
                            drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
                        alignment |= Qt::AlignCenter;
                    }
                    else
                    {
                        pr.setWidth(pmSize.width() + 8);
                        tr.adjust(pr.right(), 0, 0, 0);
                        pr.translate(shiftX, shiftY);
                        if (hasArrow)
                            drawTbArrow(this, tb, pr, painter, widget);
                        else
                            drawItemPixmap(painter, QStyle::visualRect(option->direction, r, pr), Qt::AlignCenter, pm);
                        alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                    }
                    tr.translate(shiftX, shiftY);
                    drawItemTextWithRole(painter, QStyle::visualRect(option->direction, r, tr), alignment, palette,
                                         state & State_Enabled, tb->text, QPalette::ButtonText);
                }
                else
                {
                    pr.translate(shiftX, shiftY);

                    if (hasArrow)
                        drawTbArrow(this, tb, pr, painter, widget);
                    else
                    {
                        if (!(tb->subControls&SC_ToolButtonMenu) && tb->features&QStyleOptionToolButton::HasMenu &&
                            pr.width()>pm.width() && ((pr.width()-pm.width())>LARGE_ARR_WIDTH))
                            pr.adjust(-LARGE_ARR_WIDTH, 0, 0, 0);
                        drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
                    }
                }
            }
        }
        break;
    case CE_RadioButtonLabel:
    case CE_CheckBoxLabel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            uint    alignment = visualAlignment(btn->direction, Qt::AlignLeft | Qt::AlignVCenter);
            QPixmap pix;
            QRect   textRect = r;

            if (!styleHint(SH_UnderlineShortcut, btn, widget))
                alignment |= Qt::TextHideMnemonic;

            if (!btn->icon.isNull())
            {
                pix = getIconPixmap(btn->icon, btn->iconSize, btn->state);
                drawItemPixmap(painter, r, alignment, pix);
                if (reverse)
                    textRect.setRight(textRect.right() - btn->iconSize.width() - 4);
                else
                    textRect.setLeft(textRect.left() + btn->iconSize.width() + 4);
            }
            if (!btn->text.isEmpty())
                drawItemTextWithRole(painter, textRect, alignment | Qt::TextShowMnemonic,
                                     palette, state&State_Enabled, btn->text, QPalette::WindowText);
        }
        break;
    case CE_ToolBoxTabLabel:
        if (const QStyleOptionToolBox *tb = qstyleoption_cast<const QStyleOptionToolBox *>(option))
        {
            bool    enabled = state & State_Enabled,
                selected = state & State_Selected;
            QPixmap pm = getIconPixmap(tb->icon, pixelMetric(QStyle::PM_SmallIconSize, tb, widget) ,state);
            QRect   cr = subElementRect(QStyle::SE_ToolBoxTabContents, tb, widget);
            QRect   tr, ir;
            int     ih = 0;

            if (pm.isNull())
            {
                tr = cr;
                tr.adjust(4, 0, -8, 0);
            }
            else
            {
                int iw = pm.width() + 4;
                ih = pm.height();
                ir = QRect(cr.left() + 4, cr.top(), iw + 2, ih);
                tr = QRect(ir.right(), cr.top(), cr.width() - ir.right() - 4, cr.height());
            }

            if (selected && styleHint(QStyle::SH_ToolBox_SelectedPageTitleBold, tb, widget))
            {
                QFont f(painter->font());
                f.setBold(true);
                painter->setFont(f);
            }

            QString txt = tb->fontMetrics.elidedText(tb->text, Qt::ElideRight, tr.width());

            if (ih)
                painter->drawPixmap(ir.left(), (tb->rect.height() - ih) / 2, pm);

            int alignment = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic;
            if (!styleHint(QStyle::SH_UnderlineShortcut, tb, widget))
                alignment |= Qt::TextHideMnemonic;
            drawItemTextWithRole(painter, tr, alignment, tb->palette, enabled, txt, QPalette::ButtonText);

            if (!txt.isEmpty() && state&State_HasFocus)
            {
                QStyleOptionFocusRect opt;
                opt.rect = tr;
                opt.palette = palette;
                opt.state = QStyle::State_None;
                drawPrimitive(PE_FrameFocusRect, &opt, painter, widget);
            }
        }
        break;
#endif
    case CE_RadioButton:
    case CE_CheckBox:
        if (opts.crHighlight && (r.width()>opts.crSize*2))
            if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option))
            {
                QStyleOptionButton copy(*button);

                copy.rect.adjust(2, 0, -2, 0);

                if(button->state&State_MouseOver && button->state&State_Enabled)
                {
                    QRect highlightRect(subElementRect(CE_RadioButton==element ? SE_RadioButtonFocusRect : SE_CheckBoxFocusRect,
                                                       option, widget));

                    if(Qt::RightToLeft==button->direction)
                        highlightRect.setRight(r.right());
                    else
                        highlightRect.setX(r.x());
                    highlightRect.setWidth(highlightRect.width()+1);

                    if(ROUND_NONE!=opts.round)
                    {
                        painter->save();
                        painter->setRenderHint(QPainter::Antialiasing, true);
                        double   radius(qtcGetRadius(&opts, highlightRect.width(), highlightRect.height(),
                                                     WIDGET_OTHER, RADIUS_SELECTION));

                        drawBevelGradient(shade(palette.background().color(), TO_FACTOR(opts.crHighlight)),
                                          painter, highlightRect,
                                          buildPath(QRectF(highlightRect), WIDGET_OTHER, ROUNDED_ALL, radius), true,
                                          false, opts.selectionAppearance, WIDGET_SELECTION, false);
                        painter->restore();
                    }
                    else
                        drawBevelGradient(shade(palette.background().color(), TO_FACTOR(opts.crHighlight)), painter,
                                          highlightRect, true, false, opts.selectionAppearance, WIDGET_SELECTION);
                }
                BASE_STYLE::drawControl(element, &copy, painter, widget);
                break;
            }
        // Fall through!
    default:
        BASE_STYLE::drawControl(element, option, painter, widget);
    }
}

void Style::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    QRect               r(option->rect);
    const State &state(option->state);
    const QPalette      &palette(option->palette);
    bool                reverse(Qt::RightToLeft==option->direction);

    switch (control)
    {
    case CC_Dial:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            r.adjust(1, 1, -1, -1);

            QStyleOptionComplex opt(*option);
            bool                mo(state&State_Enabled && state&State_MouseOver);
            QRect               outer(r);
            int                 sliderWidth = /*qMin(2*r.width()/5, */CIRCULAR_SLIDER_SIZE/*)*/;
#ifdef DIAL_DOT_ON_RING
            int                 halfWidth=sliderWidth/2;
#endif

            opt.state|=State_Horizontal;

            // Outer circle...
            if (outer.width() > outer.height())
            {
                outer.setLeft(outer.x()+(outer.width()-outer.height())/2);
                outer.setWidth(outer.height());
            }
            else
            {
                outer.setTop(outer.y()+(outer.height()-outer.width())/2);
                outer.setHeight(outer.width());
            }

            opt.state&=~State_MouseOver;
#ifdef DIAL_DOT_ON_RING
            opt.rect=outer.adjusted(halfWidth, halfWidth, -halfWidth, -halfWidth);
#else
            opt.rect=outer;
#endif
            drawLightBevel(painter, opt.rect, &opt, widget, ROUNDED_ALL,
                           getFill(&opt, itsBackgroundCols), itsBackgroundCols,
                           true, WIDGET_DIAL);

            // Inner 'dot'
            if(mo)
                opt.state|=State_MouseOver;

            // angle calculation from qcommonstyle.cpp (c) Trolltech 1992-2007, ASA.
            qreal               angle(0);
            if(slider->maximum == slider->minimum)
                angle = M_PI / 2;
            else
            {
                const qreal fraction(qreal(slider->sliderValue - slider->minimum)/
                                     qreal(slider->maximum - slider->minimum));
                if(slider->dialWrapping)
                    angle = 1.5*M_PI - fraction*2*M_PI;
                else
                    angle = (M_PI*8 - fraction*10*M_PI)/6;
            }

            QPoint center = outer.center();
#ifdef DIAL_DOT_ON_RING
            const qreal radius=0.5*(outer.width() - sliderWidth);
#else
            const qreal radius=0.5*(outer.width() - 2*sliderWidth);
#endif
            center += QPoint(radius*cos(angle), -radius*sin(angle));

            opt.rect=QRect(outer.x(), outer.y(), sliderWidth, sliderWidth);
            opt.rect.moveCenter(center);

            const QColor *use(buttonColors(option));

            drawLightBevel(painter, opt.rect, &opt, widget, ROUNDED_ALL,
                           getFill(&opt, use), use, true, WIDGET_RADIO_BUTTON);

            // Draw value...
#ifdef DIAL_DOT_ON_RING
            drawItemTextWithRole(painter, outer.adjusted(sliderWidth, sliderWidth, -sliderWidth, -sliderWidth),
                                 Qt::AlignCenter, palette, state&State_Enabled,
                                 QString::number(slider->sliderValue), QPalette::ButtonText);
#else
            int adjust=2*sliderWidth;
            drawItemTextWithRole(painter, outer.adjusted(adjust, adjust, -adjust, -adjust),
                                 Qt::AlignCenter, palette, state&State_Enabled,
                                 QString::number(slider->sliderValue), QPalette::ButtonText);
#endif

            if(state&State_HasFocus)
            {
                QStyleOptionFocusRect fr;
                fr.rect = outer.adjusted(-1, -1, 1, 1);
                drawPrimitive(PE_FrameFocusRect, &fr, painter, widget);
            }
        }
        break;
    case CC_ToolButton:
        // For OO.o 3.2 need to fill widget background!
        if(isOOWidget(widget))
            painter->fillRect(r, palette.brush(QPalette::Window));
        if (const QStyleOptionToolButton *toolbutton = qstyleoption_cast<const QStyleOptionToolButton *>(option))
        {
            int widthAdjust(0),
                heightAdjust(0);

            if (widget)
            {
                if((opts.dwtSettings&DWT_BUTTONS_AS_PER_TITLEBAR) &&
                   (widget->inherits("QDockWidgetTitleButton") ||
                    (widget->parentWidget() && widget->parentWidget()->inherits("KoDockWidgetTitleBar"))))
                {
                    ETitleBarButtons btn=TITLEBAR_CLOSE;
                    Icon             icon=ICN_CLOSE;

                    if(constDwtFloat==widget->objectName())
                        btn=TITLEBAR_MAX, icon=ICN_RESTORE;
                    else if(constDwtClose!=widget->objectName() &&
                            widget->parentWidget() && widget->parentWidget()->parentWidget() &&
                            widget->parentWidget()->inherits("KoDockWidgetTitleBar") &&
                            ::qobject_cast<QDockWidget *>(widget->parentWidget()->parentWidget()))
                    {
                        QDockWidget *dw = (QDockWidget*)widget->parentWidget()->parentWidget();
                        QWidget *koDw = widget->parentWidget();
                        int fw = dw->isFloating()
                            ? pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, dw)
                            : 0;
                        QRect geom(widget->geometry());
                        QStyleOptionDockWidget dwOpt;
                        dwOpt.initFrom(dw);
                        dwOpt.rect = QRect(QPoint(fw, fw),
                                           QSize(koDw->geometry().width() -
                                                 (fw * 2),
                                                 koDw->geometry().height() -
                                                 (fw * 2)));
                        dwOpt.title = dw->windowTitle();
                        dwOpt.closable = (dw->features() &
                                          QDockWidget::DockWidgetClosable) == QDockWidget::DockWidgetClosable;
                        dwOpt.floatable = (dw->features() &
                                           QDockWidget::DockWidgetFloatable) ==
                            QDockWidget::DockWidgetFloatable;

                        if(dwOpt.closable && subElementRect(QStyle::SE_DockWidgetCloseButton, &dwOpt,
                                                            widget->parentWidget()->parentWidget())==geom)
                            btn=TITLEBAR_CLOSE, icon=ICN_CLOSE;
                        else if(dwOpt.floatable && subElementRect(QStyle::SE_DockWidgetFloatButton, &dwOpt,
                                                                  widget->parentWidget()->parentWidget())==geom)
                            btn=TITLEBAR_MAX, icon=ICN_RESTORE;
                        else
                            btn=TITLEBAR_SHADE, icon=dw && dw->widget() && dw->widget()->isVisible()
                                ? ICN_SHADE
                                : ICN_UNSHADE;
                    }

                    QColor        shadow(WINDOW_SHADOW_COLOR(opts.titlebarEffect));
                    const QColor *bgndCols((opts.dwtSettings&DWT_COLOR_AS_PER_TITLEBAR)
                                           ? getMdiColors(option, state&State_Active)
                                           : buttonColors(option)),
                        *btnCols((opts.dwtSettings&DWT_COLOR_AS_PER_TITLEBAR)
                                 ? opts.titlebarButtons&TITLEBAR_BUTTON_STD_COLOR
                                 ? buttonColors(option)
                                 : getMdiColors(option, state&State_Active)
                                 : bgndCols);

                    drawDwtControl(painter, state, r.adjusted(-1, -1, 1, 1), btn,
                                   icon, option->palette.color(QPalette::WindowText), btnCols,
                                   bgndCols);
                    break;
                }
                if(qobject_cast<QTabBar *>(widget->parentWidget()))
                {
                    QStyleOptionToolButton btn(*toolbutton);

                    if(Qt::LeftArrow==toolbutton->arrowType || Qt::RightArrow==toolbutton->arrowType)
                        btn.rect.adjust(0, 4, 0, -4);
                    else
                        btn.rect.adjust(4, 0, -4, 0);
                    if(!(btn.state&State_Enabled))
                        btn.state&=~State_MouseOver;
                    drawPrimitive(PE_PanelButtonTool, &btn, painter, widget);
                    if(opts.vArrows)
                        switch(toolbutton->arrowType)
                        {
                        case Qt::LeftArrow:
                            btn.rect.adjust(-1, 0, -1, 0);
                            break;
                        case Qt::RightArrow:
                            btn.rect.adjust(1, 0, 1, 0);
                            break;
                        case Qt::UpArrow:
                            btn.rect.adjust(0, -1, 0, -1);
                            break;
                        case Qt::DownArrow:
                            btn.rect.adjust(0, 1, 0, 1);
                        default:
                            break;
                        }
                    drawTbArrow(this, &btn, btn.rect, painter, widget);
                    break;
                }

                const QToolButton *btn = qobject_cast<const QToolButton *>(widget);

                if(btn && btn->isDown() && Qt::ToolButtonTextBesideIcon==btn->toolButtonStyle() &&
                   widget->parentWidget() && qobject_cast<QMenu *>(widget->parentWidget()))
                {
                    painter->save();
                    if(opts.menuStripe)
                    {
                        int stripeWidth(qMax(20, constMenuPixmapWidth));

                        drawBevelGradient(menuStripeCol(),
                                          painter, QRect(reverse ? r.right()-stripeWidth : r.x(), r.y(),
                                                         stripeWidth, r.height()), false,
                                          false, opts.menuStripeAppearance, WIDGET_OTHER);
                    }

#if 0
                    // For some reason the MenuTitle has a larger border on the left, so adjust the width by 1 pixel to make this look nicer.
                    //drawBorder(painter, r.adjusted(2, 2, -3, -2), option, ROUNDED_ALL, 0L, WIDGET_OTHER, BORDER_SUNKEN);
                    QStyleOptionToolButton opt(*toolbutton);
                    opt.rect = r.adjusted(2, 2, -3, -2);
                    opt.state=State_Raised|State_Enabled|State_Horizontal;
                    drawLightBevel(painter, opt.rect, &opt, widget, ROUNDED_ALL,
                                   getFill(&opt, itsBackgroundCols), itsBackgroundCols, true, WIDGET_NO_ETCH_BTN);
#else
                    if(!opts.menuStripe)
                        drawFadedLine(painter, QRect(r.x()+3, r.y()+r.height()-1, r.width()-7, 1),
                                      popupMenuCols(option)[MENU_SEP_SHADE], true, true, true);
#endif
                    QFont font(toolbutton->font);

                    font.setBold(true);
                    painter->setFont(font);
                    drawItemTextWithRole(painter, r, Qt::AlignHCenter | Qt::AlignVCenter,
                                         palette, state&State_Enabled, toolbutton->text, QPalette::Text);
                    painter->restore();
                    break;
                }

                // Amarok's toolbars (the one just above the collection list) are much thinner then normal,
                // and QToolBarExtension does not seem to take this into account - so adjust the size here...
                if(widget->inherits("QToolBarExtension") && widget->parentWidget())
                {
                    if(r.height()>widget->parentWidget()->rect().height())
                        heightAdjust=(r.height()-widget->parentWidget()->rect().height())+2;
                    if(r.width()>widget->parentWidget()->rect().width())
                        widthAdjust=(r.width()-widget->parentWidget()->rect().width())+2;
                }
            }
            QRect button(subControlRect(control, toolbutton, SC_ToolButton, widget)),
                menuarea(subControlRect(control, toolbutton, SC_ToolButtonMenu, widget));
            State bflags(toolbutton->state);
            bool  etched(DO_EFFECT),
                raised=widget && (TBTN_RAISED==opts.tbarBtns || TBTN_JOINED==opts.tbarBtns),
                horizTBar(true);
            int   round=ROUNDED_ALL,
                leftAdjust(0), topAdjust(0), rightAdjust(0), bottomAdjust(0);

            if(raised)
            {
                const QToolBar *toolbar=getToolBar(widget);

                if(toolbar)
                {
                    if(TBTN_JOINED==opts.tbarBtns)
                    {
                        horizTBar=Qt::Horizontal==toolbar->orientation();
                        adjustToolbarButtons(widget, toolbar, leftAdjust, topAdjust, rightAdjust, bottomAdjust, round);
                    }
                }
                else
                    raised=false;
            }

            if (!(bflags&State_Enabled))
                bflags &= ~(State_MouseOver/* | State_Raised*/);

            if(bflags&State_MouseOver)
                bflags |= State_Raised;
            else if(!raised && (bflags&State_AutoRaise))
                bflags &= ~State_Raised;

            if(state&State_AutoRaise || toolbutton->subControls&SC_ToolButtonMenu)
                bflags|=STATE_TBAR_BUTTON;

            State mflags(bflags);

            if(!isOOWidget(widget))
            {
#if QT_VERSION >= 0x040500
                if (state&State_Sunken && !(toolbutton->activeSubControls&SC_ToolButton))
                    bflags&=~State_Sunken;
#else
                if (toolbutton->activeSubControls&SC_ToolButtonMenu && state&State_Enabled)
                    mflags |= State_Sunken;
#endif
            }

            bool         drawMenu=TBTN_JOINED==opts.tbarBtns
                ? mflags & (State_Sunken | State_On)
                : raised || (mflags & (State_Sunken | State_On | State_Raised)),
                drawnBevel=false;
            QStyleOption tool(0);
            tool.palette = toolbutton->palette;

            if ( raised ||
                 (toolbutton->subControls&SC_ToolButton && (bflags & (State_Sunken | State_On | State_Raised))) ||
                 (toolbutton->subControls&SC_ToolButtonMenu && drawMenu))
            {
                const QColor *use(buttonColors(toolbutton));

                tool.rect = (toolbutton->subControls&SC_ToolButtonMenu ? button.united(menuarea) : button)
                    .adjusted(leftAdjust, topAdjust, rightAdjust, bottomAdjust);
                tool.state = bflags|State_Horizontal;

                if(raised && TBTN_JOINED==opts.tbarBtns && !horizTBar)
                    tool.state &= ~State_Horizontal;

                tool.rect.adjust(0, 0, -widthAdjust, -heightAdjust);
                if(!(bflags&State_Sunken) && (mflags&State_Sunken))
                    tool.state &= ~State_MouseOver;
                drawnBevel=true;
                drawLightBevel(painter, tool.rect, &tool, widget, round, getFill(&tool, use), use, true, WIDGET_TOOLBAR_BUTTON);

                if(raised && TBTN_JOINED==opts.tbarBtns)
                {
                    const int constSpace=4;

                    QRect br(tool.rect.adjusted(-leftAdjust, -topAdjust, -rightAdjust, -bottomAdjust));

                    if(leftAdjust)
                        drawFadedLine(painter, QRect(br.x(), br.y()+constSpace, 1, br.height()-(constSpace*2)), use[0], true, true, false);
                    if(topAdjust)
                        drawFadedLine(painter, QRect(br.x()+constSpace, br.y(), br.width()-(constSpace*2), 1), use[0], true, true, true);
                    if(rightAdjust)
                        drawFadedLine(painter, QRect(br.x()+br.width()-1, br.y()+constSpace, 1, br.height()-(constSpace*2)),
                                      use[QTC_STD_BORDER], true, true, false);
                    if(bottomAdjust)
                        drawFadedLine(painter, QRect(br.x()+constSpace, br.y()+br.height()-1, br.width()-(constSpace*2), 1),
                                      use[QTC_STD_BORDER], true, true, true);
                }
            }

            if (toolbutton->subControls&SC_ToolButtonMenu)
            {
                if(etched)
                {
                    if(reverse)
                        menuarea.adjust(1, 1, 0, -1);
                    else
                        menuarea.adjust(0, 1, -1, -1);
                }

                tool.state = mflags|State_Horizontal;

                if(drawMenu)
                {
                    const QColor *use(buttonColors(option));
                    int          mRound=reverse ? ROUNDED_LEFT : ROUNDED_RIGHT;

                    if(mflags&State_Sunken)
                        tool.state&=~State_MouseOver;

                    if(raised && TBTN_JOINED==opts.tbarBtns)
                    {
                        if(!horizTBar)
                            tool.state &= ~State_Horizontal;
                        painter->save();
                        painter->setClipRect(menuarea, Qt::IntersectClip);
                        if((reverse && leftAdjust) || (!reverse && rightAdjust))
                            mRound=ROUNDED_NONE;
                        if(reverse)
                            tool.rect.adjust(1, 0, 0, 0);
                        else
                            tool.rect.adjust(0, 0, -1, 0);
                    }
                    else
                        tool.rect = menuarea;

                    drawLightBevel(painter, tool.rect, &tool, widget, mRound, getFill(&tool, use), use, true,
                                   MO_GLOW==opts.coloredMouseOver ? WIDGET_MENU_BUTTON : WIDGET_NO_ETCH_BTN);
                    if(raised && TBTN_JOINED==opts.tbarBtns)
                        painter->restore();
                }

                tool.rect = menuarea;

                if(mflags&State_Sunken)
                    tool.rect.adjust(1, 1, 1, 1);
                drawArrow(painter, tool.rect, PE_IndicatorArrowDown,
                          MO_ARROW_X(toolbutton->activeSubControls&SC_ToolButtonMenu,
                                     QPalette::ButtonText));
            }

            if ((FOCUS_GLOW!=opts.focus || !drawnBevel) && toolbutton->state&State_HasFocus)
            {
                QStyleOptionFocusRect fr;

                fr.QStyleOption::operator=(*toolbutton);
                if(FULL_FOCUS)
                {
                    if(etched)
                        fr.rect.adjust(1, 1, -1, -1);
                }
                else
                {
                    if(FOCUS_GLOW==opts.focus)
                        fr.rect.adjust(1, 1, -1, -1);
                    else if(etched)
                        fr.rect.adjust(4, 4, -4, -4);
                    else
                        fr.rect.adjust(3, 3, -3, -3);

#if QT_VERSION >= 0x040300
                    if (toolbutton->features & QStyleOptionToolButton::MenuButtonPopup)
#else
                        if (toolbutton->features & QStyleOptionToolButton::Menu)
#endif
                            fr.rect.adjust(0, 0, -(pixelMetric(QStyle::PM_MenuButtonIndicator, toolbutton, widget)-1), 0);
                }
                if(!(state&State_MouseOver && FULL_FOCUS && MO_NONE!=opts.coloredMouseOver))
                    drawPrimitive(PE_FrameFocusRect, &fr, painter, widget);
            }
            QStyleOptionToolButton label = *toolbutton;
            int fw = pixelMetric(PM_DefaultFrameWidth, option, widget);
            label.rect = button.adjusted(fw, fw, -(fw+widthAdjust), -(fw+heightAdjust));
            label.state = bflags;
            drawControl(CE_ToolButtonLabel, &label, painter, widget);

            if (!(toolbutton->subControls&SC_ToolButtonMenu) &&
                (toolbutton->features&QStyleOptionToolButton::HasMenu))
            {
                QRect arrow(r.right()-(LARGE_ARR_WIDTH+(etched ? 3 : 2)),
                            r.bottom()-(LARGE_ARR_HEIGHT+(etched ? 4 : 3)),
                            LARGE_ARR_WIDTH, LARGE_ARR_HEIGHT);

                if(bflags&State_Sunken)
                    arrow.adjust(1, 1, 1, 1);

                drawArrow(painter, arrow, PE_IndicatorArrowDown, MO_ARROW(QPalette::ButtonText));
            }
        }
        break;
    case CC_GroupBox:
        if (const QStyleOptionGroupBox *groupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(option)) {
            // Draw frame
            QRect textRect = /*proxy()->*/subControlRect(CC_GroupBox, option, SC_GroupBoxLabel, widget);
            QRect checkBoxRect = /*proxy()->*/subControlRect(CC_GroupBox, option, SC_GroupBoxCheckBox, widget);
            if (groupBox->subControls & QStyle::SC_GroupBoxFrame)
            {
                QStyleOptionFrame frame;
                frame.QStyleOption::operator=(*groupBox);
                frame.features = groupBox->features;
                frame.lineWidth = groupBox->lineWidth;
                frame.midLineWidth = groupBox->midLineWidth;
                frame.rect = /*proxy()->*/subControlRect(CC_GroupBox, option, SC_GroupBoxFrame, widget);

                if((groupBox->features & QStyleOptionFrame::Flat) || !(opts.gbLabel&(GB_LBL_INSIDE|GB_LBL_OUTSIDE)))
                {
                    painter->save();
                    QRegion region(r);
                    if (!groupBox->text.isEmpty())
                        region -= QRect(groupBox->subControls&QStyle::SC_GroupBoxCheckBox
                                        ? checkBoxRect.united(textRect).adjusted(reverse ? 0 : -2, 0, reverse ? 2 : 0, 0)
                                        : textRect);
                    painter->setClipRegion(region);
                }
                /*proxy()->*/drawPrimitive(PE_FrameGroupBox, &frame, painter, widget);
                if((groupBox->features&QStyleOptionFrame::Flat) || !(opts.gbLabel&(GB_LBL_INSIDE|GB_LBL_OUTSIDE)))
                    painter->restore();
            }

            // Draw title
            if ((groupBox->subControls & QStyle::SC_GroupBoxLabel) && !groupBox->text.isEmpty())
            {
                QColor textColor = groupBox->textColor;
                if (textColor.isValid())
                    painter->setPen(textColor);
                int alignment = int(groupBox->textAlignment);
                if (!/*proxy()->*/styleHint(QStyle::SH_UnderlineShortcut, option, widget))
                    alignment |= Qt::TextHideMnemonic;

                if(opts.gbLabel&GB_LBL_BOLD)
                {
                    QFont font(painter->font());

                    font.setBold(true);
                    painter->save();
                    painter->setFont(font);
                }
                /*proxy()->*/drawItemText(painter, textRect,  Qt::TextShowMnemonic | Qt::AlignHCenter | alignment,
                                          palette, state & State_Enabled, groupBox->text,
                                          textColor.isValid() ? QPalette::NoRole : QPalette::WindowText);

                if(opts.gbLabel&GB_LBL_BOLD)
                    painter->restore();

                if (state & State_HasFocus)
                {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*groupBox);
                    fropt.rect = textRect;
                    /*proxy()->*/drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
                }
            }

            // Draw checkbox
            if (groupBox->subControls & SC_GroupBoxCheckBox)
            {
                QStyleOptionButton box;
                box.QStyleOption::operator=(*groupBox);
                box.rect = checkBoxRect;
                /*proxy()->*/drawPrimitive(PE_IndicatorCheckBox, &box, painter, widget);
            }
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinBox = qstyleoption_cast<const QStyleOptionSpinBox *>(option))
        {
            QRect frame(subControlRect(CC_SpinBox, option, SC_SpinBoxFrame, widget)),
                up(subControlRect(CC_SpinBox, option, SC_SpinBoxUp, widget)),
                down(subControlRect(CC_SpinBox, option, SC_SpinBoxDown, widget)),
                all(frame.united(up).united(down));
            bool  doFrame(spinBox->frame && frame.isValid()),
                sunken(state&State_Sunken),
                enabled(state&State_Enabled),
                mouseOver(state&State_MouseOver),
                upIsActive(SC_SpinBoxUp==spinBox->activeSubControls),
                downIsActive(SC_SpinBoxDown==spinBox->activeSubControls),
                doEtch(DO_EFFECT && opts.etchEntry),
                isOO(isOOWidget(widget)),
                oldUnify=opts.unifySpin; // See Krita note below...

            if(!doFrame && isOO && !opts.unifySpin)
            {
                doFrame=true;
                frame=all;
            }

            if(isOO)
                painter->fillRect(r, palette.brush(QPalette::Window));

            if(up.isValid())
            {
                if(reverse)
                    frame.adjust(up.width(), 0, 0, 0);
                else
                    frame.adjust(0, 0, -up.width(), 0);
            }

            if(doEtch)
            {
                drawEtch(painter, all, widget, WIDGET_SPIN, false,
                         opts.square&SQUARE_ENTRY
                         ? opts.unifySpin
                         ? ROUNDED_NONE
                         : reverse
                         ? ROUNDED_LEFT
                         : ROUNDED_RIGHT
                         : ROUNDED_ALL);
                down.adjust(reverse ? 1 : 0, 0, reverse ? 0 : -1, -1);
                up.adjust(reverse ? 1 : 0, 1, reverse ? 0 : -1, 0);
                frame.adjust(reverse ? 0 : 1, 1, reverse ? -1 : 0, -1);
                all.adjust(1, 1, -1, -1);
            }

            // Krita/KOffice uses a progressbar with spin buttons at the end
            // ...when drawn, the frame part is not set - so in this case dont draw the background behind the buttons!
            if(!isOO && !doFrame)
                opts.unifySpin=true; // So, set this to true to fake the above scenario!
            else
                if(opts.unifySpin)
                    drawEntryField(painter, all, widget, option, ROUNDED_ALL, true, false);
                else
                {
                    if(opts.unifySpinBtns)
                    {
                        QRect btns=up.united(down);
                        const QColor *use(buttonColors(option));
                        QStyleOption opt(*option);

                        opt.state&=~(State_Sunken|State_MouseOver);
                        opt.state|=State_Horizontal;

                        drawLightBevel(painter, btns, &opt, widget, reverse ?  ROUNDED_LEFT : ROUNDED_RIGHT,
                                       getFill(&opt, use), use, true, WIDGET_SPIN);

                        if(state&State_MouseOver && state&State_Enabled && !(state&State_Sunken))
                        {
                            opt.state|=State_MouseOver;
                            painter->save();
                            painter->setClipRect(upIsActive ? up : down);
                            drawLightBevel(painter, btns, &opt, widget, reverse ?  ROUNDED_LEFT : ROUNDED_RIGHT,
                                           getFill(&opt, use), use, true, WIDGET_SPIN);
                            painter->restore();
                        }
                        drawFadedLine(painter, down.adjusted(2, 0, -2, 0), use[BORDER_VAL(state&State_Enabled)], true, true, true);
                    }
                }

            if(up.isValid())
            {
                QStyleOption opt(*option);

                up.setHeight(up.height()+1);
                opt.rect=up;
                opt.direction=option->direction;
                opt.state=(enabled && (spinBox->stepEnabled&QAbstractSpinBox::StepUpEnabled ||
                                       (QAbstractSpinBox::StepNone==spinBox->stepEnabled && isOO))
                           ? State_Enabled : State_None)|
                    (upIsActive && sunken ? State_Sunken : State_Raised)|
                    (upIsActive && !sunken && mouseOver ? State_MouseOver : State_None)|State_Horizontal;

                drawPrimitive(QAbstractSpinBox::PlusMinus==spinBox->buttonSymbols ? PE_IndicatorSpinPlus : PE_IndicatorSpinUp,
                              &opt, painter, widget);
            }

            if(down.isValid())
            {
                QStyleOption opt(*option);

                opt.rect=down;
                opt.state=(enabled && (spinBox->stepEnabled&QAbstractSpinBox::StepDownEnabled ||
                                       (QAbstractSpinBox::StepNone==spinBox->stepEnabled && isOO))
                           ? State_Enabled : State_None)|
                    (downIsActive && sunken ? State_Sunken : State_Raised)|
                    (downIsActive && !sunken && mouseOver ? State_MouseOver : State_None)|State_Horizontal;
                opt.direction=option->direction;

                drawPrimitive(QAbstractSpinBox::PlusMinus==spinBox->buttonSymbols ? PE_IndicatorSpinMinus : PE_IndicatorSpinDown,
                              &opt, painter, widget);
            }
            if(doFrame && !opts.unifySpin)
            {
                if(reverse)
                    frame.setX(frame.x()-1);
                else
                    frame.setWidth(frame.width()+1);
                drawEntryField(painter, frame, widget, option, reverse ? ROUNDED_RIGHT : ROUNDED_LEFT, true, false);
            }
            opts.unifySpin=oldUnify;
        }
        break;
    case CC_Slider:
        if (const QStyleOptionSlider *slider =
            qstyleoption_cast<const QStyleOptionSlider*>(option)) {
            QRect groove(subControlRect(CC_Slider, option,
                                        SC_SliderGroove, widget));
            QRect handle(subControlRect(CC_Slider, option,
                                        SC_SliderHandle, widget));
            // QRect ticks(subControlRect(CC_Slider, option,
            //                            SC_SliderTickmarks, widget));
            bool  horizontal(slider->orientation == Qt::Horizontal),
                ticksAbove(slider->tickPosition & QSlider::TicksAbove),
                ticksBelow(slider->tickPosition & QSlider::TicksBelow);

            //The clickable region is 5 px wider than the visible groove for improved usability
//                 if (groove.isValid())
//                     groove = horizontal ? groove.adjusted(0, 5, 0, -5) : groove.adjusted(5, 0, -5, 0);

            if ((option->subControls&SC_SliderGroove) && groove.isValid())
                drawSliderGroove(painter, groove, handle, slider, widget);

            if ((option->subControls&SC_SliderHandle) && handle.isValid())
            {
                QStyleOptionSlider s(*slider);
                if(!(s.activeSubControls & QStyle::SC_SliderHandle))
                {
                    s.state &= ~QStyle::State_MouseOver;
                    s.state &= ~QStyle::State_Sunken;
                }

                drawSliderHandle(painter, handle, &s);

                if (state&State_HasFocus && FOCUS_GLOW!=opts.focus)
                {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*slider);
                    fropt.rect = slider->rect;

                    if(horizontal)
                        fropt.rect.adjust(0, 0, 0, -1);
                    else
                        fropt.rect.adjust(0, 0, -1, 0);

                    drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
                }
            }

            if (option->subControls&SC_SliderTickmarks)
            {
                QPen oldPen = painter->pen();
                painter->setPen(backgroundColors(option)[QTC_STD_BORDER]);
                int tickSize(pixelMetric(PM_SliderTickmarkOffset, option, widget)),
                    available(pixelMetric(PM_SliderSpaceAvailable, slider, widget)),
                    interval(slider->tickInterval);
                if (interval <= 0)
                {
                    interval = slider->singleStep;
                    if (QStyle::sliderPositionFromValue(slider->minimum, slider->maximum, interval,
                                                        available)
                        - QStyle::sliderPositionFromValue(slider->minimum, slider->maximum,
                                                          0, available) < 3)
                        interval = slider->pageStep;
                }
                if (interval <= 0)
                    interval = 1;

                int sliderLength(slider->maximum - slider->minimum + 1),
                    nticks(sliderLength / interval); // add one to get the end tickmark
                if (sliderLength % interval > 0)
                    nticks++; // round up the number of tick marks

                int v(slider->minimum),
                    len(pixelMetric(PM_SliderLength, slider, widget));

                while (v <= slider->maximum + 1)
                {
                    if (v == slider->maximum + 1 && interval == 1)
                        break;

                    int pos(sliderPositionFromValue(slider->minimum, slider->maximum,
                                                    qMin(v, slider->maximum), (horizontal
                                                                               ? slider->rect.width()
                                                                               : slider->rect.height()) - len,
                                                    slider->upsideDown) + len / 2);

                    int extra(2); // - ((v == slider->minimum || v == slider->maximum) ? 1 : 0);

                    if (horizontal)
                    {
                        if (ticksAbove)
                            painter->drawLine(QLine(pos, slider->rect.top() + extra,
                                                    pos, slider->rect.top() + tickSize));
                        if (ticksBelow)
                            painter->drawLine(QLine(pos, slider->rect.bottom() - extra,
                                                    pos, slider->rect.bottom() - tickSize));
                    }
                    else
                    {
                        if (ticksAbove)
                            painter->drawLine(QLine(slider->rect.left() + extra, pos,
                                                    slider->rect.left() + tickSize, pos));
                        if (ticksBelow)
                            painter->drawLine(QLine(slider->rect.right() - extra, pos,
                                                    slider->rect.right() - tickSize, pos));
                    }

                    // in the case where maximum is max int
                    int nextInterval = v + interval;
                    if (nextInterval < v)
                        break;
                    v = nextInterval;
                }
                painter->setPen(oldPen);
            }
        }
        break;
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *titleBar = qstyleoption_cast<const QStyleOptionTitleBar *>(option))
        {
            painter->save();

            EAppearance  app=qtcWidgetApp(WIDGET_MDI_WINDOW_TITLE, &opts, option->state&State_Active);
            bool         active(state&State_Active),
                kwin(theThemedApp==APP_KWIN || titleBar->titleBarState&QtC_StateKWin);
            const QColor *bgndCols(APPEARANCE_NONE==app
                                   ? kwin ? backgroundColors(option) : backgroundColors(palette.color(QPalette::Active, QPalette::Window))
                                   : kwin ? buttonColors(option) : getMdiColors(titleBar, active)),
                *btnCols(kwin || opts.titlebarButtons&TITLEBAR_BUTTON_STD_COLOR
                         ? buttonColors(option)
                         : getMdiColors(titleBar, active)),
                *titleCols(APPEARANCE_NONE==app
                           ? bgndCols
                           : kwin || !(opts.titlebarButtons&TITLEBAR_BUTTON_STD_COLOR)
                           ? btnCols : getMdiColors(titleBar, active));
            QColor       textColor(theThemedApp==APP_KWIN
                                   ? option->palette.color(QPalette::WindowText)
                                   : active
                                   ? itsActiveMdiTextColor
                                   : itsMdiTextColor),
                iconColor(textColor),
                shadow(WINDOW_SHADOW_COLOR(opts.titlebarEffect));
            QStyleOption opt(*option);
            QRect        tr(r),
                menuRect(subControlRect(CC_TitleBar, titleBar, SC_TitleBarSysMenu, widget));
            ERound       round=(opts.square&SQUARE_WINDOWS && opts.round>ROUND_SLIGHT) ? ROUND_SLIGHT : opts.round;
            QColor       borderCol(kwin && option->version==(TBAR_BORDER_VERSION_HACK+2)
                                   ? palette.color(QPalette::Active, QPalette::Shadow)
                                   : titleCols[kwin && option->version==TBAR_BORDER_VERSION_HACK ? 0 : QTC_STD_BORDER]);

            if(!kwin && widget && BLEND_TITLEBAR && qobject_cast<const QMdiSubWindow *>(widget))
            {
                const QWidget *w=NULL;
                if(qobject_cast<const QMainWindow *>(widget))
                    w=widget;
                else if (static_cast<const QMdiSubWindow *>(widget)->widget())
                    w=qobject_cast<const QMainWindow *>(static_cast<const QMdiSubWindow *>(widget)->widget());
                if(w)
                {
                    const QMenuBar *menuBar=static_cast<const QMainWindow *>(w)->menuBar();

                    if(menuBar)
                        tr.adjust(0, 0, 0, menuBar->rect().height());
                }
            }

            opt.state=State_Horizontal|State_Enabled|State_Raised|(active ? State_Active : State_None);

#ifndef QTC_QT5_ENABLE_KDE
            QPainterPath path;
#else
#if KDE_IS_VERSION(4, 3, 0)
            QPainterPath path(round<ROUND_SLIGHT
                              ? QPainterPath()
                              : buildPath(QRectF(state&QtC_StateKWinNoBorder ? tr : tr.adjusted(1, 1, -1, 0)),
                                          WIDGET_MDI_WINDOW_TITLE, state&QtC_StateKWin && state&QtC_StateKWinTabDrag
                                          ? ROUNDED_ALL : ROUNDED_TOP,
                                          (round>ROUND_SLIGHT /*&& kwin*/ ? 6.0 : 2.0)));
#else
            QPainterPath path;
#endif
#endif
            if(!kwin && !CUSTOM_BGND)
                painter->fillRect(tr, borderCol);

            painter->setRenderHint(QPainter::Antialiasing, true);

            if(kwin && (state&QtC_StateKWinFillBgnd))
                drawBevelGradient(titleCols[ORIGINAL_SHADE], painter, tr, path, true, false, APPEARANCE_FLAT, WIDGET_MDI_WINDOW, false);
            if((!kwin && !itsIsPreview) ||
               (APPEARANCE_NONE!=app && (!IS_FLAT(app) || (titleCols[ORIGINAL_SHADE]!=QApplication::palette().background().color()))))
                drawBevelGradient(titleCols[ORIGINAL_SHADE], painter, tr, path, true, false, app, WIDGET_MDI_WINDOW, false);

            if(!(state&QtC_StateKWinNoBorder))
            {
                QColor light(titleCols[0]),
                    dark(borderCol);
                bool   addLight=opts.windowBorder&WINDOW_BORDER_ADD_LIGHT_BORDER && (!kwin || qtcGetWindowBorderSize(false).sides>1);

                if(kwin)
                {
                    light.setAlphaF(1.0);
                    dark.setAlphaF(1.0);
                }

                if(addLight)
                {
                    painter->setPen(light);
                    painter->save();
                    painter->setClipRect(r.adjusted(0, 0, -1, -1));
                    painter->drawPath(buildPath(r.adjusted(1, 1, 0, 1), WIDGET_MDI_WINDOW_TITLE, ROUNDED_TOP,
                                                round<ROUND_SLIGHT
                                                ? 0
                                                : round>ROUND_SLIGHT /*&& kwin*/
                                                ? 5.0
                                                : 1.0));
                    painter->restore();
                }

                painter->setPen(dark);
                painter->drawPath(buildPath(r, WIDGET_MDI_WINDOW_TITLE, ROUNDED_TOP,
                                            round<ROUND_SLIGHT
                                            ? 0
                                            : round>ROUND_SLIGHT /*&& kwin*/
                                            ? 6.0
                                            : 2.0));

                painter->setRenderHint(QPainter::Antialiasing, false);

                if(addLight)
                {
                    painter->setPen(light);
                    painter->drawPoint(r.x()+1, r.y()+r.height()-1);
                }

                if(round>ROUND_SLIGHT && FULLLY_ROUNDED)
                {
                    if(!(state&QtC_StateKWinCompositing))
                    {
                        painter->setPen(dark);

                        painter->drawLine(r.x()+1, r.y()+4, r.x()+1, r.y()+3);
                        painter->drawPoint(r.x()+2, r.y()+2);
                        painter->drawLine(r.x()+3, r.y()+1, r.x()+4, r.y()+1);
                        painter->drawLine(r.x()+r.width()-2, r.y()+4, r.x()+r.width()-2, r.y()+3);
                        painter->drawPoint(r.x()+r.width()-3, r.y()+2);
                        painter->drawLine(r.x()+r.width()-4, r.y()+1, r.x()+r.width()-5, r.y()+1);
                    }

                    if(addLight &&
                       (APPEARANCE_SHINY_GLASS!=(active ? opts.titlebarAppearance : opts.inactiveTitlebarAppearance)))
                    {
                        painter->setPen(light);
                        painter->drawLine(r.x()+2, r.y()+4, r.x()+2, r.y()+3);
                        painter->drawLine(r.x()+3, r.y()+2, r.x()+4, r.y()+2);
                        painter->drawLine(r.x()+r.width()-4, r.y()+2, r.x()+r.width()-5, r.y()+2);
                    }
                }

                if(opts.windowBorder&WINDOW_BORDER_BLEND_TITLEBAR && (!kwin || !(state&QtC_StateKWinNoBorder)))
                {
                    static const int constFadeLen=8;
                    QPoint          start(0, r.y()+r.height()-(1+constFadeLen)),
                        end(start.x(), start.y()+constFadeLen);
                    QLinearGradient grad(start, end);

                    grad.setColorAt(0, dark);
                    grad.setColorAt(1, itsBackgroundCols[QTC_STD_BORDER]);
                    painter->setPen(QPen(QBrush(grad), 1));
                    painter->drawLine(r.x(), start.y(), r.x(), end.y());
                    painter->drawLine(r.x()+r.width()-1, start.y(), r.x()+r.width()-1, end.y());

                    if(addLight)
                    {
                        grad.setColorAt(0, light);
                        grad.setColorAt(1, itsBackgroundCols[0]);
                        painter->setPen(QPen(QBrush(grad), 1));
                        painter->drawLine(r.x()+1, start.y(), r.x()+1, end.y());
                    }
                }
            }
            else
                painter->setRenderHint(QPainter::Antialiasing, false);

            if(kwin)
            {
                painter->restore();
                break;
            }

            int   adjust(0);
            QRect captionRect(subControlRect(CC_TitleBar, titleBar, SC_TitleBarLabel, widget));

            if(opts.titlebarButtons&TITLEBAR_BUTTON_SUNKEN_BACKGROUND && captionRect!=r)
            {
                bool menuIcon=TITLEBAR_ICON_MENU_BUTTON==opts.titlebarIcon,
                    menuLeft=menuRect.isValid() && !titleBar->icon.isNull() && menuRect.left()<(r.left()+constWindowMargin+4);
                int  height=r.height()-(1+(2*constWindowMargin));

                adjust=1;
                if(captionRect.left()>(r.left()+constWindowMargin))
                {
                    int width=captionRect.left()-(r.left()+(2*constWindowMargin));

                    if(!(menuIcon && menuLeft) || width>(height+4))
                        drawSunkenBevel(painter, QRect(r.left()+constWindowMargin+1, r.top()+constWindowMargin+1, width, height), titleCols[ORIGINAL_SHADE]);
                }
                if(captionRect.right()<(r.right()-constWindowMargin))
                {
                    int width=r.right()-(captionRect.right()+(2*constWindowMargin));

                    if(!(menuIcon && !menuLeft) || width>(height+4))
                        drawSunkenBevel(painter, QRect(captionRect.right()+constWindowMargin, r.top()+constWindowMargin+1, width, height), titleCols[ORIGINAL_SHADE]);
                }
            }

            bool    showIcon=TITLEBAR_ICON_NEXT_TO_TITLE==opts.titlebarIcon && !titleBar->icon.isNull();
            int     iconSize=showIcon ? pixelMetric(QStyle::PM_SmallIconSize) : 0,
                iconX=r.x();
            QPixmap pixmap;

            if(showIcon)
                pixmap=getIconPixmap(titleBar->icon, iconSize, titleBar->state);

            if(!titleBar->text.isEmpty())
            {
                static const int constPad=4;

                Qt::Alignment alignment((Qt::Alignment)pixelMetric((QStyle::PixelMetric)QtC_TitleAlignment, 0L, 0L));
                bool          alignFull(Qt::AlignHCenter==alignment),
                    iconRight((!reverse && alignment&Qt::AlignRight) || (reverse && alignment&Qt::AlignLeft));
                QRect         textRect(alignFull
                                       ? QRect(r.x(), captionRect.y(), r.width(), captionRect.height())
                                       : captionRect);

#ifndef QTC_QT5_ENABLE_KDE
                QFont         font(painter->font());
                font.setBold(true);
                painter->setFont(font);
#else
                painter->setFont(KGlobalSettings::windowTitleFont());
#endif

                QFontMetrics fm(painter->fontMetrics());
                QString str(fm.elidedText(titleBar->text, Qt::ElideRight, textRect.width(), QPalette::WindowText));

                int           textWidth=alignFull || (showIcon && alignment&Qt::AlignHCenter)
                    ? fm.boundingRect(str).width()+(showIcon ? iconSize+constPad : 0) : 0;

                if(alignFull &&
                   ( (captionRect.left()>((textRect.width()-textWidth)>>1)) ||
                     (captionRect.right()<((textRect.width()+textWidth)>>1)) ) )
                {
                    alignment=Qt::AlignVCenter|Qt::AlignRight;
                    textRect=captionRect;
                }

                if(alignment&Qt::AlignLeft && constWindowMargin==textRect.x())
                    textRect.adjust(showIcon ? 4 : 6, 0, 0, 0);

                if(showIcon)
                {
                    if(alignment&Qt::AlignHCenter)
                    {
                        if(reverse)
                        {
                            iconX=((textRect.width()-textWidth)/2.0)+0.5+textWidth+iconSize;
                            textRect.setX(textRect.x()-(iconSize+constPad));
                        }
                        else
                        {
                            iconX=((textRect.width()-textWidth)/2.0)+0.5;
                            textRect.setX(iconX+iconSize+constPad);
                            alignment=Qt::AlignVCenter|Qt::AlignLeft;
                        }
                    }
                    else if((!reverse && alignment&Qt::AlignLeft) || (reverse && alignment&Qt::AlignRight))
                    {
                        iconX=textRect.x();
                        textRect.setX(textRect.x()+(iconSize+constPad));
                    }
                    else if((!reverse && alignment&Qt::AlignRight) || (reverse && alignment&Qt::AlignLeft))
                    {
                        if(iconRight)
                        {
                            iconX=textRect.x()+textRect.width()-iconSize;
                            textRect.setWidth(textRect.width()-(iconSize+constPad));
                        }
                        else
                        {
                            iconX=textRect.x()+textRect.width()-textWidth;
                            if(iconX<textRect.x())
                                iconX=textRect.x();
                        }
                    }
                }

                QTextOption textOpt(alignment|Qt::AlignVCenter);
                textOpt.setWrapMode(QTextOption::NoWrap);

                if(EFFECT_NONE!=opts.titlebarEffect)
                {
                    shadow.setAlphaF(WINDOW_TEXT_SHADOW_ALPHA(opts.titlebarEffect));
                    //painter->setPen(shadow);
                    painter->setPen(blendColors(WINDOW_SHADOW_COLOR(opts.titlebarEffect), titleCols[ORIGINAL_SHADE],
                                                WINDOW_TEXT_SHADOW_ALPHA(opts.titlebarEffect)));
                    painter->drawText(EFFECT_SHADOW==opts.titlebarEffect
                                      ? textRect.adjusted(1, 1, 1, 1)
                                      : textRect.adjusted(0, 1, 0, 1),
                                      str, textOpt);

                    if (!active && DARK_WINDOW_TEXT(textColor))
                    {
                        //textColor.setAlpha((textColor.alpha() * 180) >> 8);
                        textColor=blendColors(textColor, titleCols[ORIGINAL_SHADE], ((255 * 180) >> 8)/256.0);
                    }
                }
                painter->setPen(textColor);
                painter->drawText(textRect, str, textOpt);
            }

            if(showIcon && iconX>=0)
                painter->drawPixmap(iconX, r.y()+((r.height()-iconSize)/2)+1, pixmap);

            if ((titleBar->subControls&SC_TitleBarMinButton) && (titleBar->titleBarFlags&Qt::WindowMinimizeButtonHint) &&
                !(titleBar->titleBarState&Qt::WindowMinimized))
                drawMdiControl(painter, titleBar, SC_TitleBarMinButton, widget, TITLEBAR_MIN, iconColor, btnCols, bgndCols,
                               adjust, active);

            if ((titleBar->subControls&SC_TitleBarMaxButton) && (titleBar->titleBarFlags&Qt::WindowMaximizeButtonHint) &&
                !(titleBar->titleBarState&Qt::WindowMaximized))
                drawMdiControl(painter, titleBar, SC_TitleBarMaxButton, widget, TITLEBAR_MAX, iconColor, btnCols, bgndCols,
                               adjust, active);

            if ((titleBar->subControls&SC_TitleBarCloseButton) && (titleBar->titleBarFlags&Qt::WindowSystemMenuHint))
                drawMdiControl(painter, titleBar, SC_TitleBarCloseButton, widget, TITLEBAR_CLOSE, iconColor, btnCols, bgndCols,
                               adjust, active);

            if ((titleBar->subControls&SC_TitleBarNormalButton) &&
                (((titleBar->titleBarFlags&Qt::WindowMinimizeButtonHint) &&
                  (titleBar->titleBarState&Qt::WindowMinimized)) ||
                 ((titleBar->titleBarFlags&Qt::WindowMaximizeButtonHint) &&
                  (titleBar->titleBarState&Qt::WindowMaximized))))
                drawMdiControl(painter, titleBar, SC_TitleBarNormalButton, widget, TITLEBAR_MAX, iconColor, btnCols, bgndCols,
                               adjust, active);

            if (titleBar->subControls&SC_TitleBarContextHelpButton && (titleBar->titleBarFlags&Qt::WindowContextHelpButtonHint))
                drawMdiControl(painter, titleBar, SC_TitleBarContextHelpButton, widget, TITLEBAR_HELP, iconColor, btnCols, bgndCols,
                               adjust, active);

            if (titleBar->subControls&SC_TitleBarShadeButton && (titleBar->titleBarFlags&Qt::WindowShadeButtonHint))
                drawMdiControl(painter, titleBar, SC_TitleBarShadeButton, widget, TITLEBAR_SHADE, iconColor, btnCols, bgndCols,
                               adjust, active);

            if (titleBar->subControls&SC_TitleBarUnshadeButton && (titleBar->titleBarFlags&Qt::WindowShadeButtonHint))
                drawMdiControl(painter, titleBar, SC_TitleBarUnshadeButton, widget, TITLEBAR_SHADE, iconColor, btnCols, bgndCols,
                               adjust, active);

            if ((titleBar->subControls&SC_TitleBarSysMenu) && (titleBar->titleBarFlags&Qt::WindowSystemMenuHint))
            {
                if(TITLEBAR_ICON_MENU_BUTTON==opts.titlebarIcon)
                {
                    bool hover((titleBar->activeSubControls&SC_TitleBarSysMenu) && (titleBar->state&State_MouseOver));

                    if(active || hover || !(opts.titlebarButtons&TITLEBAR_BUTTOM_HIDE_ON_INACTIVE_WINDOW))
                    {
                        if (menuRect.isValid())
                        {
                            bool sunken((titleBar->activeSubControls&SC_TitleBarSysMenu) && (titleBar->state&State_Sunken));
                            int  offset(sunken ? 1 : 0);

//                                 if(!(opts.titlebarButtons&TITLEBAR_BUTTON_ROUND))
//                                     drawMdiButton(painter, menuRect, hover, sunken,
//                                                   coloredMdiButtons(state&State_Active, hover)
//                                                     ? itsTitleBarButtonsCols[TITLEBAR_MENU] : btnCols);

                            if (!titleBar->icon.isNull())
                                titleBar->icon.paint(painter, menuRect.adjusted(offset, offset, offset, offset));
                            else
                            {
                                QStyleOption tool(0);

                                tool.palette = palette;
                                tool.rect = menuRect;
                                painter->save();
                                drawItemPixmap(painter, menuRect.adjusted(offset, offset, offset, offset), Qt::AlignCenter,
                                               standardIcon(SP_TitleBarMenuButton, &tool, widget).pixmap(16, 16));
                                painter->restore();
                            }
                        }
                    }
                }
                else
                    drawMdiControl(painter, titleBar, SC_TitleBarSysMenu, widget, TITLEBAR_MENU, iconColor, btnCols, bgndCols,
                                   adjust, active);

                if(active && opts.windowBorder&WINDOW_BORDER_SEPARATOR)
                {
                    QColor        color(active ? itsActiveMdiTextColor : itsMdiTextColor);
                    Qt::Alignment align(pixelMetric((QStyle::PixelMetric)QtC_TitleAlignment, 0L, 0L));
                    QRect         lr(r.x(), captionRect.y(), r.width(), captionRect.height());

                    lr.adjust(16, lr.height()-2, -16, 0);
                    color.setAlphaF(0.5);
                    drawFadedLine(painter, lr, color, align&(Qt::AlignHCenter|Qt::AlignRight),
                                  align&(Qt::AlignHCenter|Qt::AlignLeft), true);
                }
            }

            painter->restore();
        }
        break;
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            bool               useThreeButtonScrollBar(SCROLLBAR_KDE==opts.scrollbarType),
                horiz(Qt::Horizontal==scrollbar->orientation),
                maxed(scrollbar->minimum == scrollbar->maximum),
                atMin(maxed || scrollbar->sliderValue==scrollbar->minimum),
                atMax(maxed || scrollbar->sliderValue==scrollbar->maximum)/*,
                                                                            inStack(0!=opts.tabBgnd && inStackWidget(widget))*/;
            QRect subline(subControlRect(control, option,
                                         SC_ScrollBarSubLine, widget));
            QRect addline(subControlRect(control, option,
                                         SC_ScrollBarAddLine, widget));
            QRect subpage(subControlRect(control, option,
                                         SC_ScrollBarSubPage, widget));
            QRect addpage(subControlRect(control, option,
                                         SC_ScrollBarAddPage, widget));
            QRect slider(subControlRect(control, option,
                                        SC_ScrollBarSlider, widget));
            QRect first(subControlRect(control, option,
                                       SC_ScrollBarFirst, widget));
            QRect last(subControlRect(control, option,
                                      SC_ScrollBarLast, widget));
            QRect subline2(addline);
            // QRect sbRect(scrollbar->rect);
            QStyleOptionSlider opt(*scrollbar);

            // For OO.o 3.2 need to fill widget background!
            if(isOOWidget(widget))
                painter->fillRect(r, palette.brush(QPalette::Window));

            if(reverse && horiz)
            {
                bool tmp(atMin);

                atMin=atMax;
                atMax=tmp;
            }

            if (useThreeButtonScrollBar)
            {
                int sbextent(pixelMetric(PM_ScrollBarExtent, scrollbar, widget));

                if(horiz && reverse)
                    subline2=QRect((r.x()+r.width()-1)-sbextent, r.y(), sbextent, sbextent);
                else if (horiz)
                    subline2.translate(-addline.width(), 0);
                else
                    subline2.translate(0, -addline.height());

                if (horiz)
                    subline.setWidth(sbextent);
                else
                    subline.setHeight(sbextent);
            }

            // Draw trough...
            bool  noButtons(ROUNDED && (SCROLLBAR_NONE==opts.scrollbarType || opts.flatSbarButtons));
            QRect s2(subpage), a2(addpage);

#ifndef SIMPLE_SCROLLBARS
            if(noButtons)
            {
                // Increase clipping to allow trough to "bleed" into slider corners...
                a2.adjust(-3, -3, 3, 3);
                s2.adjust(-3, -3, 3, 3);
            }
#endif

            painter->save();

            bool needsBaseBgnd=(opts.thinSbarGroove || opts.flatSbarButtons) &&
                widget && widget->parentWidget() && widget->parentWidget()->parentWidget() &&
                (widget->parentWidget()->parentWidget()->inherits("QComboBoxListView")/* ||
                                                                                         !opts.gtkScrollViews && widget->parentWidget()->parentWidget()->inherits("QAbstractScrollArea")*/);

            if(needsBaseBgnd)
                painter->fillRect(r, palette.brush(QPalette::Base));
            else if(opts.thinSbarGroove && APP_ARORA==theThemedApp && widget && widget->inherits("WebView"))
                painter->fillRect(r, itsBackgroundCols[ORIGINAL_SHADE]);

            if(!opts.gtkScrollViews ||
               (opts.flatSbarButtons && !IS_FLAT(opts.sbarBgndAppearance)/* && SCROLLBAR_NONE!=opts.scrollbarType*/))
                drawBevelGradientReal(palette.brush(QPalette::Background).color(), painter, r, horiz, false,
                                      opts.sbarBgndAppearance, WIDGET_SB_BGND);

            if(noButtons || opts.flatSbarButtons)
            {
                int mod=THIN_SBAR_MOD;
                // Draw complete groove here, as we want to round both ends...
                opt.rect=subpage.united(addpage);
                opt.state=scrollbar->state;
                opt.state&=~(State_MouseOver|State_Sunken|State_On);

                if(opts.thinSbarGroove && slider.isValid())
                {
                    painter->save();
                    painter->setClipRegion(QRegion(opt.rect).subtracted(slider.adjusted(1, 1, -1, -1)));
                }
                drawLightBevel(painter, opts.thinSbarGroove
                               ? horiz
                               ? opt.rect.adjusted(0, mod, 0, -mod)
                               : opt.rect.adjusted(mod, 0, -mod, 0)
                               : opt.rect, &opt, widget,
#ifndef SIMPLE_SCROLLBARS
                               !(opts.square&SQUARE_SB_SLIDER) && (SCROLLBAR_NONE==opts.scrollbarType || opts.flatSbarButtons)
                               ? ROUNDED_ALL :
#endif
                               ROUNDED_NONE,
                               itsBackgroundCols[2], itsBackgroundCols, true,
                               opts.thinSbarGroove ? WIDGET_SLIDER_TROUGH : WIDGET_TROUGH);
                if(opts.thinSbarGroove && slider.isValid())
                    painter->restore();
            }
            else
            {
                if((option->subControls&SC_ScrollBarSubPage) && subpage.isValid())
                {
                    opt.state=scrollbar->state;
                    opt.rect = subpage;
//                         if (!(scrollbar->activeSubControls&SC_ScrollBarSubPage))
                    opt.state &= ~(State_Sunken|State_MouseOver|State_On);
                    drawControl(CE_ScrollBarSubPage, &opt, painter, widget);
                }

                if((option->subControls&SC_ScrollBarAddPage) && addpage.isValid())
                {
                    opt.state=scrollbar->state;
                    opt.rect = addpage;
//                         if (!(scrollbar->activeSubControls&SC_ScrollBarAddPage))
                    opt.state &= ~(State_Sunken|State_MouseOver|State_On);
                    drawControl(CE_ScrollBarAddPage, &opt, painter, widget);
                }
            }

            if((option->subControls&SC_ScrollBarSubLine) && subline.isValid())
            {
                opt.rect=subline;
                opt.state=scrollbar->state/*|(inStack ? NO_BGND_BUTTON : State_None)*/;
                if(maxed || atMin)
                    opt.state&=~State_Enabled;
                if (!(scrollbar->activeSubControls&SC_ScrollBarSubLine) ||
                    (useThreeButtonScrollBar && itsSbWidget && itsSbWidget==widget))
                    opt.state &= ~(State_Sunken | State_MouseOver);

                drawControl(CE_ScrollBarSubLine, &opt, painter, widget);

                if (useThreeButtonScrollBar && subline2.isValid())
                {
                    opt.rect=subline2;
                    opt.state=scrollbar->state/*|(inStack ? NO_BGND_BUTTON : State_None)*/;
                    if(maxed || atMin)
                        opt.state&=~State_Enabled;
                    if ((!(scrollbar->activeSubControls&SC_ScrollBarSubLine)) || (itsSbWidget && itsSbWidget!=widget))
                        opt.state &= ~(State_Sunken | State_MouseOver);

                    drawControl(CE_ScrollBarSubLine, &opt, painter, widget);
                }
            }

            if((option->subControls&SC_ScrollBarAddLine) && addline.isValid())
            {
                opt.rect=addline;
                opt.state=scrollbar->state/*|(inStack ? NO_BGND_BUTTON : State_None)*/;
                if(maxed || atMax)
                    opt.state&=~State_Enabled;
                if (!(scrollbar->activeSubControls&SC_ScrollBarAddLine))
                    opt.state &= ~(State_Sunken | State_MouseOver);
                drawControl(CE_ScrollBarAddLine, &opt, painter, widget);
            }

            if((option->subControls&SC_ScrollBarFirst) && first.isValid())
            {
                opt.rect=first;
                opt.state=scrollbar->state;
                if (!(scrollbar->activeSubControls&SC_ScrollBarFirst))
                    opt.state &= ~(State_Sunken | State_MouseOver);
                drawControl(CE_ScrollBarFirst, &opt, painter, widget);
            }

            if((option->subControls&SC_ScrollBarLast) && last.isValid())
            {
                opt.rect=last;
                opt.state=scrollbar->state;
                if (!(scrollbar->activeSubControls&SC_ScrollBarLast))
                    opt.state &= ~(State_Sunken | State_MouseOver);
                drawControl(CE_ScrollBarLast, &opt, painter, widget);
            }

            if(((option->subControls&SC_ScrollBarSlider) || noButtons) && slider.isValid())
            {
                // If "SC_ScrollBarSlider" wasn't specified, then we only want to draw the portion
                // of the slider that overlaps with the trough. So, once again set the clipping
                // region...

                // NO! Seeems to mess things up with Arora, su just dsiable all clipping when drawing
                // the slider...
                painter->setClipping(false);
#ifdef INCREASE_SB_SLIDER
                if(!opts.flatSbarButtons)
                {
                    if(atMax)
                        switch(opts.scrollbarType)
                        {
                        case SCROLLBAR_KDE:
                        case SCROLLBAR_WINDOWS:
                        case SCROLLBAR_PLATINUM:
                            if(horiz)
                                slider.adjust(0, 0, 1, 0);
                            else
                                slider.adjust(0, 0, 0, 1);
                        default:
                            break;
                        }
                    if(atMin)
                        switch(opts.scrollbarType)
                        {
                        case SCROLLBAR_KDE:
                        case SCROLLBAR_WINDOWS:
                        case SCROLLBAR_NEXT:
                            if(horiz)
                                slider.adjust(-1, 0, 0, 0);
                            else
                                slider.adjust(0, -1, 0, 0);
                        default:
                            break;
                        }
                }
#endif
                opt.rect=slider;
                opt.state=scrollbar->state;
                if (!(scrollbar->activeSubControls&SC_ScrollBarSlider))
                    opt.state &= ~(State_Sunken | State_MouseOver);
                drawControl(CE_ScrollBarSlider, &opt, painter, widget);

                // ### perhaps this should not be able to accept focus if maxedOut?
                if(state&State_HasFocus)
                {
                    opt.state=scrollbar->state;
                    opt.rect=QRect(slider.x()+2, slider.y()+2, slider.width()-5, slider.height()-5);
                    drawPrimitive(PE_FrameFocusRect, &opt, painter, widget);
                }
            }
            painter->restore();
        }
        break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
        {
            painter->save();

            QRect        frame(subControlRect(CC_ComboBox, option, SC_ComboBoxFrame, widget)),
                arrow(subControlRect(CC_ComboBox, option, SC_ComboBoxArrow, widget)),
                field(subControlRect(CC_ComboBox, option, SC_ComboBoxEditField, widget));
            const QColor *use(buttonColors(option));
            bool         sunken(state&State_On), // comboBox->listBox() ? comboBox->listBox()->isShown() : false),
                glowOverFocus(state&State_MouseOver && FULL_FOCUS &&
                              MO_GLOW==opts.coloredMouseOver && DO_EFFECT && !sunken && !comboBox->editable &&
                              state&State_Enabled && state&State_HasFocus),
                doEffect(DO_EFFECT && (!comboBox->editable || opts.etchEntry)),
                isOO(isOOWidget(widget)),
                isOO31(isOO);

            if(isOO)
            {
                // This (hopefull) checks is we're OO.o 3.2 - in which case no adjustment is required...
                const QImage *img=getImage(painter);

                isOO31=!img || img->rect()!=r;

                if(isOO31)
                    frame.adjust(0, 0, 0, -2), arrow.adjust(0, 0, 0, -2), field.adjust(0, 0, 0, -2);
                else
                    arrow.adjust(1, 0, 0, 0);
            }

//                 painter->fillRect(r, Qt::transparent);
            if(doEffect)
            {
                bool glowFocus(state&State_HasFocus && state&State_Enabled && USE_GLOW_FOCUS(state&State_MouseOver));

                if(!glowOverFocus && !(opts.thin&THIN_FRAMES) && !sunken && MO_GLOW==opts.coloredMouseOver &&
                   (((FULL_FOCUS || glowFocus) && state&State_HasFocus) || state&State_MouseOver) &&
                   state&State_Enabled && !comboBox->editable)
                    drawGlow(painter, r, FULL_FOCUS && state&State_HasFocus ? WIDGET_DEF_BUTTON : WIDGET_COMBO,
                             glowFocus ? itsFocusCols : 0L);
                else
                    drawEtch(painter, r, widget, WIDGET_COMBO,
                             !comboBox->editable && EFFECT_SHADOW==opts.buttonEffect && !sunken,
                             comboBox->editable && opts.square&SQUARE_ENTRY
                             ? opts.unifyCombo
                             ? ROUNDED_NONE
                             : reverse
                             ? ROUNDED_LEFT
                             : ROUNDED_RIGHT
                             : ROUNDED_ALL);

                frame.adjust(1, 1, -1, -1);
            }

            if(/*comboBox->frame &&*/ frame.isValid() && (!comboBox->editable || !opts.unifyCombo))
            {
                const QColor *cols=itsComboBtnCols && comboBox->editable && state&State_Enabled ? itsComboBtnCols : use;

                QStyleOption frameOpt(*option);

                if (comboBox->editable && !(comboBox->activeSubControls&SC_ComboBoxArrow))
                    frameOpt.state &= ~(State_Sunken | State_MouseOver);

                if(!sunken)
                    frameOpt.state|=State_Raised;

                //if(opts.coloredMouseOver && frameOpt.state&State_MouseOver && comboBox->editable && !sunken)
                //    frame.adjust(reverse ? 0 : 1, 0, reverse ? 1 : 0, 0);

                drawLightBevel(painter, frame, &frameOpt, widget,
                               comboBox->editable ? (reverse ? ROUNDED_LEFT : ROUNDED_RIGHT) : ROUNDED_ALL,
                               getFill(&frameOpt, cols, false,
                                       (SHADE_DARKEN==opts.comboBtn || (SHADE_NONE!=opts.comboBtn &&
                                                                        !(state&State_Enabled))) &&
                                       comboBox->editable),
                               cols, true, comboBox->editable ? WIDGET_COMBO_BUTTON : WIDGET_COMBO);
            }

            if(/*controls&SC_ComboBoxEditField &&*/ field.isValid())
            {
                if(comboBox->editable)
                {
                    if(opts.unifyCombo)
                    {
                        field=r;
                        if(doEffect)
                            field.adjust(1, 1, -1, -1);
                        if(isOO31)
                            field.adjust(0, 0, 0, -2);
                    }
                    else if(doEffect)
                        field.adjust(reverse ? -4 : -3, -1, reverse ? 3 : 4, 1);
                    else
                        field.adjust(reverse ? -4 : -2, -1, reverse ? 2 : 4, 1);
                    drawEntryField(painter, field, widget, option, opts.unifyCombo ? ROUNDED_ALL : reverse ? ROUNDED_RIGHT : ROUNDED_LEFT,
                                   true, false);
                }
                else if(opts.comboSplitter && !(SHADE_DARKEN==opts.comboBtn || itsComboBtnCols))
                {
                    drawFadedLine(painter, QRect(reverse ? arrow.right()+1 : arrow.x()-1, arrow.top()+2,
                                                 1, arrow.height()-4),
                                  use[BORDER_VAL(state&State_Enabled)], true, true, false);
                    if(!sunken)
                        drawFadedLine(painter, QRect(reverse ? arrow.right()+2 : arrow.x(), arrow.top()+2,
                                                     1, arrow.height()-4),
                                      use[0], true, true, false);
                }
            }

            if(/*controls&SC_ComboBoxArrow && */arrow.isValid())
            {
                bool mouseOver=comboBox->editable && !(comboBox->activeSubControls&SC_ComboBoxArrow)
                    ? false : (state&State_MouseOver ? true : false);

                if(!comboBox->editable && (SHADE_DARKEN==opts.comboBtn || itsComboBtnCols))
                {
                    if(!comboBox->editable && isOO && !isOO31)
                        arrow.adjust(reverse ? 0 : 1, 0, reverse ? -1 : 0, 0);

                    QStyleOption frameOpt(*option);
                    QRect        btn(arrow.x(), frame.y(), arrow.width()+1, frame.height());
                    const QColor *cols=SHADE_DARKEN==opts.comboBtn || !(state&State_Enabled) ? use : itsComboBtnCols;
                    if(!sunken)
                        frameOpt.state|=State_Raised;
                    painter->save();
                    painter->setClipRect(btn, Qt::IntersectClip);
                    drawLightBevel(painter, opts.comboSplitter
                                   ? btn.adjusted(reverse ? -2 : 0, 0, reverse ? 2 : 1, 0)
                                   : btn.adjusted(reverse ? -3 : -2, 0, reverse ? 2 : 1, 0),
                                   &frameOpt, widget, reverse ? ROUNDED_LEFT : ROUNDED_RIGHT,
                                   getFill(&frameOpt, cols, false,
                                           SHADE_DARKEN==opts.comboBtn || (SHADE_NONE!=opts.comboBtn &&
                                                                           !(state&State_Enabled))),
                                   cols, true, WIDGET_COMBO);
                    painter->restore();
                }

                if(sunken && (!comboBox->editable || !opts.unifyCombo))
                    arrow.adjust(1, 1, 1, 1);

                QColor arrowColor(MO_ARROW_X(mouseOver, QPalette::ButtonText));
                if(comboBox->editable || !(opts.gtkComboMenus && opts.doubleGtkComboArrow))
                    drawArrow(painter, arrow, PE_IndicatorArrowDown, arrowColor, false);
                else
                {
                    int middle=arrow.y()+(arrow.height()>>1),
                        gap=(opts.vArrows ? 2 : 1);

                    QRect ar=QRect(arrow.x(), middle-(LARGE_ARR_HEIGHT+gap), arrow.width(), LARGE_ARR_HEIGHT);
                    drawArrow(painter, ar, PE_IndicatorArrowUp, arrowColor, false);
                    ar=QRect(arrow.x(), middle+gap, arrow.width(), LARGE_ARR_HEIGHT);
                    drawArrow(painter, ar, PE_IndicatorArrowDown, arrowColor, false);
                }
            }

            if(state&State_Enabled && state&State_HasFocus &&
               /*state&State_KeyboardFocusChange &&*/ !comboBox->editable && FOCUS_GLOW!=opts.focus)
            {
                QStyleOptionFocusRect focus;
                bool                  listViewCombo=comboBox->frame && widget && widget->rect().height()<(DO_EFFECT ? 22 : 20);

                if(FULL_FOCUS)
                    focus.rect=frame;
                else if(opts.comboSplitter)
                {
                    focus.rect=reverse
                        ? field.adjusted(0, -1, 1, 1)
                        : field.adjusted(-1, -1, 0, 1);

                    if(listViewCombo)
                        focus.rect.adjust(0, -2, 0, 2);
                }
                else if(listViewCombo)
                    focus.rect=frame.adjusted(1, 1, -1, -1);
                else
                    focus.rect=frame.adjusted(3, 3, -3, -3);

                // Draw glow over top of filled focus
                if(glowOverFocus && !(opts.thin&THIN_FRAMES))
                    drawGlow(painter, frame.adjusted(-1, -1, 1, 1), WIDGET_COMBO);
                else
                    drawPrimitive(PE_FrameFocusRect, &focus, painter, widget);
            }
            painter->restore();
        }
        break;
    default:
        BASE_STYLE::drawComplexControl(control, option, painter, widget);
        break;
    }
}

void Style::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text,
                         QPalette::ColorRole textRole) const
{
    if(QPalette::ButtonText==textRole && !opts.stdSidebarButtons)
    {
        const QAbstractButton *button=getButton(NULL, painter);

        if(button && isMultiTabBarTab(button) && button->isChecked())
        {
            QPalette p(pal);

            if(itsInactiveChangeSelectionColor && QPalette::Inactive==p.currentColorGroup())
                p.setCurrentColorGroup(QPalette::Active);
            BASE_STYLE::drawItemText(painter, rect, flags, p, enabled, text, QPalette::HighlightedText);
            return;
        }
    }

    BASE_STYLE::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
}

QSize Style::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    QSize newSize(BASE_STYLE::sizeFromContents(type, option, size, widget));

    switch (type)
    {
    case CT_TabBarTab:
        newSize+=QSize(1, 1);
        break;
    case CT_Splitter:
    {
        int sw=pixelMetric(PM_SplitterWidth, 0L, 0L);
        return QSize(sw, sw);
    }
    case CT_PushButton:
    {
        newSize=size;
        newSize.setWidth(newSize.width()+(ROUND_MAX==opts.round ? 12 : 8));

        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            if(!opts.stdBtnSizes)
            {
                bool dialogButton=
                    // Cant rely on AutoDefaultButton - as VirtualBox does not set this!!!
                    // btn->features&QStyleOptionButton::AutoDefaultButton &&
                    widget && widget->parentWidget() &&
                    (::qobject_cast<const QDialogButtonBox *>(widget->parentWidget()) || widget->parentWidget()->inherits("KFileWidget"));

                if(dialogButton)
                {
                    int iconHeight=btn->icon.isNull() ? btn->iconSize.height() : 16;
                    if(size.height()<iconHeight+2)
                        newSize.setHeight(iconHeight+2);
                }
            }

            int margin = (pixelMetric(PM_ButtonMargin, btn, widget)+
                          (pixelMetric(PM_DefaultFrameWidth, btn, widget) * 2))-MAX_ROUND_BTN_PAD;

            newSize+=QSize(margin, margin);

            if (btn->features&QStyleOptionButton::HasMenu)
                newSize+=QSize(4, 0);

            if (!btn->text.isEmpty() && "..."!=btn->text && newSize.width() < 80)
                newSize.setWidth(80);

            newSize.rheight() += ((1 - newSize.rheight()) & 1);
        }
        break;
    }
//         case CT_RadioButton:
//             ++newSize.rheight();
//             ++newSize.rwidth();
//             break;
    case CT_RadioButton:
    case CT_CheckBox:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            bool isRadio = CT_RadioButton==type;
            int  w = /*proxy()->*/pixelMetric(isRadio ? PM_ExclusiveIndicatorWidth : PM_IndicatorWidth, btn, widget),
                h = /*proxy()->*/pixelMetric(isRadio ? PM_ExclusiveIndicatorHeight : PM_IndicatorHeight, btn, widget),
                margins = 0;

            newSize=size;
            // we add 4 pixels for label margins
            if (btn->icon.isNull() || !btn->text.isEmpty())
                margins = 0+/*proxy()->*/pixelMetric(isRadio ? PM_RadioButtonLabelSpacing : PM_CheckBoxLabelSpacing, option, widget)+
                    (opts.crHighlight ? 4 : 0);

            newSize += QSize(w + margins, 4);
            newSize.setHeight(qMax(newSize.height(), h));
        }
        break;
    case CT_ScrollBar:
        if (const QStyleOptionSlider *scrollBar =
            qstyleoption_cast<const QStyleOptionSlider*>(option)) {
            int scrollBarExtent =
                pixelMetric(PM_ScrollBarExtent, option, widget);
            // See https://github.com/QtCurve/qtcurve-qt4/issues/7
            // and https://bugs.kde.org/show_bug.cgi?id=317690.
            int scrollBarLen =
                (qtcMax(scrollBarExtent, 13) *
                 qtcScrollbarButtonNumSize(opts.scrollbarType) +
                 pixelMetric(PM_ScrollBarSliderMin, option, widget));

            if (scrollBar->orientation == Qt::Horizontal) {
                newSize = QSize(scrollBarLen, scrollBarExtent);
            } else {
                newSize = QSize(scrollBarExtent, scrollBarLen);
            }
        }
        break;
    case CT_LineEdit:
        if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(option))
            newSize = size+QSize(2*f->lineWidth, 2*f->lineWidth);
        break;
    case CT_SpinBox:
        if(!opts.unifySpin)
            newSize.rheight() -= ((1 - newSize.rheight()) & 1);
        break;
    case CT_ToolButton:
    {
        newSize = QSize(size.width()+8, size.height()+8);
        // -- from kstyle & oxygen --
        // We want to avoid super-skiny buttons, for things like "up" when icons + text
        // For this, we would like to make width >= height.
        // However, once we get here, QToolButton may have already put in the menu area
        // (PM_MenuButtonIndicator) into the width. So we may have to take it out, fix things
        // up, and add it back in. So much for class-independent rendering...
        int menuAreaWidth(0);

        if (const QStyleOptionToolButton* tbOpt = qstyleoption_cast<const QStyleOptionToolButton*>(option))
        {
            // Make Kate/KWrite's option toolbuton have the same size as the next/prev buttons...
            if(widget && !getToolBar(widget) && !tbOpt->text.isEmpty() &&
               tbOpt->features&QStyleOptionToolButton::MenuButtonPopup)
            {
                QStyleOptionButton btn;

                btn.init(widget);
                btn.text=tbOpt->text;
                btn.icon=tbOpt->icon;
                btn.iconSize=tbOpt->iconSize;
                btn.features=tbOpt->features&QStyleOptionToolButton::MenuButtonPopup
                    ? QStyleOptionButton::HasMenu : QStyleOptionButton::None;
                return sizeFromContents(CT_PushButton, &btn, size, widget);
            }

            if (!tbOpt->icon.isNull() && !tbOpt->text.isEmpty() && Qt::ToolButtonTextUnderIcon==tbOpt->toolButtonStyle)
                newSize.setHeight(newSize.height()-4);

            if (tbOpt->features & QStyleOptionToolButton::MenuButtonPopup)
                menuAreaWidth = pixelMetric(QStyle::PM_MenuButtonIndicator, option, widget);
            else if (tbOpt->features & QStyleOptionToolButton::HasMenu)
                switch(tbOpt->toolButtonStyle)
                {
                case Qt::ToolButtonIconOnly:
                    newSize.setWidth(newSize.width()+LARGE_ARR_WIDTH+2);
                    break;
                case Qt::ToolButtonTextBesideIcon:
                    newSize.setWidth(newSize.width()+3);
                    break;
                case Qt::ToolButtonTextOnly:
                    newSize.setWidth(newSize.width()+8);
                    break;
                case Qt::ToolButtonTextUnderIcon:
                    newSize.setWidth(newSize.width()+8);
                    break;
                default:
                    break;
                }
        }

        newSize.setWidth(newSize.width() - menuAreaWidth);
        if (newSize.width() < newSize.height())
            newSize.setWidth(newSize.height());
        newSize.setWidth(newSize.width() + menuAreaWidth);

        break;
    }
    case CT_ComboBox:
    {
        newSize=size;
        newSize.setWidth(newSize.width()+4);

        const QStyleOptionComboBox *combo = qstyleoption_cast<const QStyleOptionComboBox *>(option);

        int  margin = (pixelMetric(PM_ButtonMargin, option, widget)+
                       (pixelMetric(PM_DefaultFrameWidth, option, widget) * 2))-MAX_ROUND_BTN_PAD,
            textMargins = 2*(pixelMetric(PM_FocusFrameHMargin) + 1),
            // QItemDelegate::sizeHint expands the textMargins two times, thus the 2*textMargins...
            other = qMax(DO_EFFECT ? 20 : 18, 2*textMargins + pixelMetric(QStyle::PM_ScrollBarExtent, option, widget));
        bool editable=combo ? combo->editable : false;
        newSize+=QSize(margin+other, margin-2);
        newSize.rheight() += ((1 - newSize.rheight()) & 1);

        if(!opts.etchEntry && DO_EFFECT && editable)
            newSize.rheight()-=2;
        // KWord's zoom combo clips 'Fit Page Width' without the following...
        if(editable)
            newSize.rwidth()+=6;
        break;
    }
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            // Taken from QWindowStyle...
            int w = size.width();

            if (QStyleOptionMenuItem::Separator==mi->menuItemType)
                newSize = QSize(10, windowsSepHeight);
            else if (mi->icon.isNull())
            {
                newSize.setHeight(newSize.height() - 2);
                w -= 6;
            }

            if (QStyleOptionMenuItem::Separator!=mi->menuItemType && !mi->icon.isNull())
            {
                int iconExtent = pixelMetric(PM_SmallIconSize, option, widget);
                newSize.setHeight(qMax(newSize.height(),
                                       mi->icon.actualSize(QSize(iconExtent, iconExtent)).height()
                                       + 2 * windowsItemFrame));
            }
            int maxpmw = mi->maxIconWidth,
                tabSpacing = 20;

            if (mi->text.contains(QLatin1Char('\t')))
                w += tabSpacing;
            else if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                w += 2 * windowsArrowHMargin;
            else if (mi->menuItemType == QStyleOptionMenuItem::DefaultItem)
            {
                // adjust the font and add the difference in size.
                // it would be better if the font could be adjusted in the initStyleOption qmenu func!!
                QFontMetrics fm(mi->font);
                QFont fontBold = mi->font;
                fontBold.setBold(true);
                QFontMetrics fmBold(fontBold);
                w += fmBold.width(mi->text) - fm.width(mi->text);
            }

            int checkcol = qMax<int>(maxpmw, windowsCheckMarkWidth); // Windows always shows a check column
            w += checkcol + windowsRightBorder + 10;
            newSize.setWidth(w);
            // ....

            int h(newSize.height()-8); // Fix mainly for Qt4.4

            if (QStyleOptionMenuItem::Separator==mi->menuItemType && mi->text.isEmpty())
                h = 7;
            else
            {
                h = qMax(h, mi->fontMetrics.height());
                if (!mi->icon.isNull())
                    h = qMax(h, mi->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal).height());

                if (h < 18)
                    h = 18;
                h+=((opts.thin&THIN_MENU_ITEMS) ? 2 : 4);

                if(QStyleOptionMenuItem::Separator==mi->menuItemType)
                    h+=4;
            }

            newSize.setHeight(h);
            // Gtk2's icon->text spacing is 2 pixels smaller - so adjust here...
            newSize.setWidth(newSize.width()-2);
        }
        break;
    case CT_MenuBarItem:
#if QT_VERSION >= 0x040500
        if (!size.isEmpty())
            newSize=size+QSize((windowsItemHMargin * 4)+2, windowsItemVMargin+1);
#else
        if (!size.isEmpty())
            newSize=size+QSize((windowsItemHMargin * 4)+2, windowsItemVMargin);
#endif
        break;
    case CT_MenuBar:
        if(APP_KONQUEROR==theThemedApp && widget && qobject_cast<const QMenuBar *>(widget))
        {
            int height=konqMenuBarSize((const QMenuBar *)widget);
            newSize.setHeight(height);
        }
        break;
    default:
        break;
    }

    return newSize;
}

QRect Style::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    QRect rect;
    switch (element) {
    case SE_SliderFocusRect:
    case SE_ToolBoxTabContents:
        return visualRect(option->direction, option->rect, option->rect);
    case SE_DockWidgetTitleBarText: {
        const QStyleOptionDockWidget *dwopt = qstyleoption_cast<const QStyleOptionDockWidget*>(option);
        bool verticalTitleBar = dwopt ? dwopt->verticalTitleBar : false;
        int m = pixelMetric(PM_DockWidgetTitleMargin, option, widget);

        rect = BASE_STYLE::subElementRect(element, option, widget);

        if (verticalTitleBar)
            rect.adjust(0, 0, 0, -m);
        else if (Qt::LeftToRight==option->direction )
            rect.adjust(m, 0, 0, 0);
        else
            rect.adjust(0, 0, -m, 0);
        return rect;
    }
#if QT_VERSION >= 0x040500
    case SE_TabBarTabLeftButton:
        return BASE_STYLE::subElementRect(element, option, widget).translated(-2, -1);
    case SE_TabBarTabRightButton:
        return BASE_STYLE::subElementRect(element, option, widget).translated(2, -1);
    case SE_TabBarTabText:
        if (const QStyleOptionTab *_tab = qstyleoption_cast<const QStyleOptionTab*>(option))
        {
            QStyleOptionTab tab(*_tab);
            bool verticalTabs = QTabBar::RoundedEast == tab.shape ||
                QTabBar::RoundedWest == tab.shape ||
                QTabBar::TriangularEast == tab.shape ||
                QTabBar::TriangularWest == tab.shape;

            rect=tab.rect;
            if (verticalTabs)
                rect.setRect(0, 0, rect.height(), rect.width());
            int verticalShift = pixelMetric(QStyle::PM_TabBarTabShiftVertical,
                                            _tab, widget);
            int horizontalShift = pixelMetric(QStyle::PM_TabBarTabShiftHorizontal,
                                              _tab, widget);
            if (tab.shape == QTabBar::RoundedSouth ||
                tab.shape == QTabBar::TriangularSouth)
                verticalShift = -verticalShift;
            rect.adjust(0, 0, horizontalShift, verticalShift);
            bool selected = tab.state & State_Selected;
            if (selected) {
                rect.setBottom(rect.bottom() - verticalShift);
                rect.setRight(rect.right() - horizontalShift);
            }

            // left widget
            if(opts.centerTabText) {
                if (!tab.leftButtonSize.isEmpty())  // left widget
                    rect.setLeft(rect.left() + constTabPad +
                                 (verticalTabs ? tab.leftButtonSize.height() :
                                  tab.leftButtonSize.width()));
                if (!tab.rightButtonSize.isEmpty()) // right widget
                    rect.setRight(rect.right() - constTabPad -
                                  (verticalTabs ? tab.rightButtonSize.height() :
                                   tab.rightButtonSize.width()));
            } else {
                if (tab.leftButtonSize.isNull()) {
                    rect.setLeft(rect.left()+constTabPad);
                } else if(tab.leftButtonSize.width()>0) {
                    rect.setLeft(rect.left() + constTabPad + 2 +
                                 (verticalTabs ? tab.leftButtonSize.height() :
                                  tab.leftButtonSize.width()));
                } else if(tab.icon.isNull()) {
                    rect.setLeft(rect.left() + constTabPad);
                } else {
                    rect.setLeft(rect.left() + 2);
                }
            }

            // icon
            if (!tab.icon.isNull()) {
                QSize iconSize = tab.iconSize;
                if (!iconSize.isValid()) {
                    int iconExtent = pixelMetric(PM_SmallIconSize);
                    iconSize = QSize(iconExtent, iconExtent);
                }
                QSize tabIconSize = tab.icon.actualSize(iconSize,
                                                        (tab.state &
                                                         State_Enabled) ?
                                                        QIcon::Normal :
                                                        QIcon::Disabled);
                int offset = 4;

                if (!opts.centerTabText && tab.leftButtonSize.isNull())
                    offset += 2;

                QRect iconRect = QRect(rect.left() + offset, rect.center().y() - tabIconSize.height() / 2,
                                       tabIconSize.width(), tabIconSize .height());
                if (!verticalTabs)
                    iconRect = visualRect(option->direction, option->rect, iconRect);
                rect.setLeft(rect.left() + tabIconSize.width() + offset + 2);
            }

            // right widget
            if (!opts.centerTabText && !tab.rightButtonSize.isNull() &&
                tab.rightButtonSize.width() > 0) {
                rect.setRight(rect.right() - constTabPad - 2 -
                              (verticalTabs ? tab.rightButtonSize.height() :
                               tab.rightButtonSize.width()));
            } else {
                rect.setRight(rect.right() - constTabPad);
            }

            if (!verticalTabs)
                rect = visualRect(option->direction, option->rect, rect);
            return rect;
        }
        break;
#endif
    case SE_RadioButtonIndicator:
        rect = visualRect(option->direction, option->rect,
                          BASE_STYLE::subElementRect(element, option, widget)).adjusted(0, 0, 1, 1);
        break;
    case SE_ProgressBarContents:
        return opts.fillProgress
            ? DO_EFFECT && opts.borderProgress
            ? option->rect.adjusted(1, 1, -1, -1)
            : option->rect
            : DO_EFFECT && opts.borderProgress
            ? option->rect.adjusted(3, 3, -3, -3)
            : option->rect.adjusted(2, 2, -2, -2);
    case SE_ProgressBarGroove:
    case SE_ProgressBarLabel:
        return option->rect;
#if QT_VERSION >= 0x040300
    case SE_GroupBoxLayoutItem:
        rect = option->rect;
//             if (const QStyleOptionGroupBox *groupBoxOpt = qstyleoption_cast<const QStyleOptionGroupBox *>(option))
//                 if (groupBoxOpt->subControls & (SC_GroupBoxCheckBox | SC_GroupBoxLabel))
//                     rect.setTop(rect.top() + 2);    // eat the top margin a little bit
        break;
#endif
    case SE_PushButtonFocusRect:
        if(FULL_FOCUS)
        {
            rect=subElementRect(SE_PushButtonContents, option, widget);
            if(DO_EFFECT)
                rect.adjust(-1, -1, 1, 1);
            else
                rect.adjust(-2, -2, 2, 2);
        }
        else
        {
            rect=BASE_STYLE::subElementRect(element, option, widget);
            if(DO_EFFECT)
                rect.adjust(1, 1, -1, -1);
        }
        return rect;
    default:
        return BASE_STYLE::subElementRect(element, option, widget);
    }

    return visualRect(option->direction, option->rect, rect);
}

QRect Style::subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const
{
    QRect r(option->rect);
    bool  reverse(Qt::RightToLeft==option->direction);

    switch (control)
    {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
        {
            bool ed(comboBox->editable),
                doEtch((!ed || opts.etchEntry) && DO_EFFECT);
            int  x(r.x()),
                y(r.y()),
                w(r.width()),
                h(r.height());

            switch (subControl)
            {
            case SC_ComboBoxFrame:
                if(ed)
                {
                    int btnWidth(doEtch ? 22 : 20);

                    r=QRect(x+w-btnWidth, y, btnWidth, h);
                }
                break;
            case SC_ComboBoxArrow:
            {
                int bmarg(comboBox->frame ? 2 : 0);

                r.setRect(x + w - bmarg - (doEtch ? 17 : 16), y + bmarg, 16, h - 2*bmarg);
                if(ed && opts.unifyCombo)
                    r.adjust(-1, 0, 0, 0);
                break;
            }
            case SC_ComboBoxEditField:
            {
                int margin(comboBox->frame ? 3 : 0);

                r.setRect(x + margin+(opts.unifyCombo ? 0 : 2), y + margin,
                          w - 2 * margin - (opts.unifyCombo ? 15 : 23), h - 2 * margin);
                if(doEtch)
                    r.adjust(ed ? 0 : 1, 1, ed ? 0 : -1, -1);
                if(ed)
                    r.adjust(-1, -2, 1, 2);
                break;
            }
            case SC_ComboBoxListBoxPopup:
            default:
                break;
            }
            return visualRect(comboBox->direction, comboBox->rect, r);
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinbox = qstyleoption_cast<const QStyleOptionSpinBox *>(option))
        {
            int   fw(spinbox->frame ? pixelMetric(PM_SpinBoxFrameWidth, spinbox, widget) : 0);
            QSize bs;

            bs.setHeight(r.height()>>1);
            if(bs.height()< 8)
                bs.setHeight(8);
            bs.setWidth(DO_EFFECT && opts.etchEntry ? 16 : 15);
            bs=bs.expandedTo(QApplication::globalStrut());

            int y(0), x(reverse ? 0 : r.width()-bs.width());

            switch(subControl)
            {
            case SC_SpinBoxUp:
                return QAbstractSpinBox::NoButtons==spinbox->buttonSymbols
                    ? QRect()
                    : QRect(x, y, bs.width(), bs.height());
            case SC_SpinBoxDown:
                if(QAbstractSpinBox::NoButtons==spinbox->buttonSymbols)
                    return QRect();
                else
                    return QRect(x, y+bs.height(), bs.width(), bs.height()+(bs.height()*2==r.height() ? 0 : 1));
            case SC_SpinBoxEditField:
            {
                int pad=opts.round>ROUND_FULL ? 2 : 0;

                if (QAbstractSpinBox::NoButtons==spinbox->buttonSymbols)
                    return QRect(fw, fw, (x-fw*2)-pad, r.height()-2*fw);
                else
                    return QRect(fw+(reverse ? bs.width() : 0), fw, (x-fw*2)-pad, r.height()-2*fw);
            }
            case SC_SpinBoxFrame:
            default:
                return visualRect(spinbox->direction, spinbox->rect, spinbox->rect);
            }
        }
        break;
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            // Taken from kstyle.cpp (KDE 3) , and modified so as to allow for no scrollbar butttons...
            bool  threeButtonScrollBar(SCROLLBAR_KDE==opts.scrollbarType),
                platinumScrollBar(SCROLLBAR_PLATINUM==opts.scrollbarType),
                nextScrollBar(SCROLLBAR_NEXT==opts.scrollbarType),
                noButtons(SCROLLBAR_NONE==opts.scrollbarType);
            QRect ret;
            bool  horizontal(Qt::Horizontal==scrollBar->orientation);
            int   sbextent(pixelMetric(PM_ScrollBarExtent, scrollBar, widget)),
                sliderMaxLength(((scrollBar->orientation == Qt::Horizontal) ?
                                 scrollBar->rect.width() : scrollBar->rect.height()) - (sbextent * qtcScrollbarButtonNum(opts.scrollbarType))),
                sliderMinLength(pixelMetric(PM_ScrollBarSliderMin, scrollBar, widget)),
                sliderLength;

            if (scrollBar->maximum != scrollBar->minimum)
            {
                uint valueRange = scrollBar->maximum - scrollBar->minimum;
                sliderLength = (scrollBar->pageStep * sliderMaxLength) / (valueRange + scrollBar->pageStep);

                if (sliderLength < sliderMinLength || (!isOOWidget(widget) && valueRange > INT_MAX / 2))
                    sliderLength = sliderMinLength;
                if (sliderLength > sliderMaxLength)
                    sliderLength = sliderMaxLength;
            }
            else
                sliderLength = sliderMaxLength;

            int sliderstart(sliderPositionFromValue(scrollBar->minimum,
                                                    scrollBar->maximum,
                                                    scrollBar->sliderPosition,
                                                    sliderMaxLength - sliderLength,
                                                    scrollBar->upsideDown));

            switch(opts.scrollbarType)
            {
            case SCROLLBAR_KDE:
            case SCROLLBAR_WINDOWS:
                sliderstart+=sbextent;
                break;
            case SCROLLBAR_NEXT:
                sliderstart+=sbextent*2;
            default:
                break;
            }

            // Subcontrols
            switch(subControl)
            {
            case SC_ScrollBarSubLine:
                if(noButtons)
                    return QRect();

                // top/left button
                if (platinumScrollBar)
                    if (horizontal)
                        ret.setRect(scrollBar->rect.width() - 2 * sbextent, 0, sbextent, sbextent);
                    else
                        ret.setRect(0, scrollBar->rect.height() - 2 * sbextent, sbextent, sbextent);
                else if(threeButtonScrollBar)
                    if (horizontal)
                        ret.setRect(0, 0, scrollBar->rect.width() - sbextent +1, sbextent);
                    else
                        ret.setRect(0, 0, sbextent, scrollBar->rect.height() - sbextent +1);
                else
                    ret.setRect(0, 0, sbextent, sbextent);
                break;
            case SB_SUB2:
                if(threeButtonScrollBar)
                    if (horizontal)
                        if(reverse)
                            ret.setRect(sbextent, 0, sbextent, sbextent);
                        else
                            ret.setRect(scrollBar->rect.width() - 2 * sbextent, 0, sbextent, sbextent);
                    else
                        ret.setRect(0, scrollBar->rect.height() - 2 * sbextent, sbextent, sbextent);
                else
                    return QRect();
                break;
            case SC_ScrollBarAddLine:
                if(noButtons)
                    return QRect();

                // bottom/right button
                if (nextScrollBar)
                    if (horizontal)
                        ret.setRect(sbextent, 0, sbextent, sbextent);
                    else
                        ret.setRect(0, sbextent, sbextent, sbextent);
                else
                    if (horizontal)
                        ret.setRect(scrollBar->rect.width() - sbextent, 0, sbextent, sbextent);
                    else
                        ret.setRect(0, scrollBar->rect.height() - sbextent, sbextent, sbextent);
                break;
            case SC_ScrollBarSubPage:
                // between top/left button and slider
                if (platinumScrollBar)
                    if (horizontal)
                        ret.setRect(0, 0, sliderstart, sbextent);
                    else
                        ret.setRect(0, 0, sbextent, sliderstart);
                else if (nextScrollBar)
                    if (horizontal)
                        ret.setRect(sbextent*2, 0, sliderstart-2*sbextent, sbextent);
                    else
                        ret.setRect(0, sbextent*2, sbextent, sliderstart-2*sbextent);
                else
                    if (horizontal)
                        ret.setRect(noButtons ? 0 : sbextent, 0,
                                    noButtons ? sliderstart
                                    : (sliderstart - sbextent), sbextent);
                    else
                        ret.setRect(0, noButtons ? 0 : sbextent, sbextent,
                                    noButtons ? sliderstart : (sliderstart - sbextent));
                break;
            case SC_ScrollBarAddPage:
            {
                // between bottom/right button and slider
                int fudge;

                if (platinumScrollBar)
                    fudge = 0;
                else if (nextScrollBar)
                    fudge = 2*sbextent;
                else if(noButtons)
                    fudge = 0;
                else
                    fudge = sbextent;

                if (horizontal)
                    ret.setRect(sliderstart + sliderLength, 0,
                                sliderMaxLength - sliderstart - sliderLength + fudge, sbextent);
                else
                    ret.setRect(0, sliderstart + sliderLength, sbextent,
                                sliderMaxLength - sliderstart - sliderLength + fudge);
                break;
            }
            case SC_ScrollBarGroove:
                if(noButtons)
                {
                    if (horizontal)
                        ret=QRect(0, 0, scrollBar->rect.width(), scrollBar->rect.height());
                    else
                        ret=QRect(0, 0, scrollBar->rect.width(), scrollBar->rect.height());
                }
                else
                {
                    int multi = threeButtonScrollBar ? 3 : 2,
                        fudge;

                    if (platinumScrollBar)
                        fudge = 0;
                    else if (nextScrollBar)
                        fudge = 2*sbextent;
                    else
                        fudge = sbextent;

                    if (horizontal)
                        ret=QRect(fudge, 0, scrollBar->rect.width() - sbextent * multi, scrollBar->rect.height());
                    else
                        ret=QRect(0, fudge, scrollBar->rect.width(), scrollBar->rect.height() - sbextent * multi);
                }
                break;
            case SC_ScrollBarSlider:
                if (horizontal)
                    ret=QRect(sliderstart, 0, sliderLength, sbextent);
                else
                    ret=QRect(0, sliderstart, sbextent, sliderLength);
                break;
            default:
                ret = BASE_STYLE::subControlRect(control, option, subControl, widget);
                break;
            }
            return visualRect(scrollBar->direction/*Qt::LeftToRight*/, scrollBar->rect, ret);
        }
        break;
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            if(SLIDER_TRIANGULAR==opts.sliderStyle)
            {
                int   tickSize(pixelMetric(PM_SliderTickmarkOffset, option, widget)),
                    mod=MO_GLOW==opts.coloredMouseOver && DO_EFFECT ? 2 : 0;
                QRect rect(BASE_STYLE::subControlRect(control, option, subControl, widget));

                switch (subControl)
                {
                case SC_SliderHandle:
                    if (slider->orientation == Qt::Horizontal)
                    {
                        rect.setWidth(11+mod);
                        rect.setHeight(15+mod);
                        int centerY(r.center().y() - rect.height() / 2);
                        if (slider->tickPosition & QSlider::TicksAbove)
                            centerY += tickSize;
                        if (slider->tickPosition & QSlider::TicksBelow)
                            centerY -= (tickSize-1);
                        rect.moveTop(centerY);
                    }
                    else
                    {
                        rect.setWidth(15+mod);
                        rect.setHeight(11+mod);
                        int centerX(r.center().x() - rect.width() / 2);
                        if (slider->tickPosition & QSlider::TicksAbove)
                            centerX += tickSize;
                        if (slider->tickPosition & QSlider::TicksBelow)
                            centerX -= (tickSize-1);
                        rect.moveLeft(centerX);
                    }
                    break;
                case SC_SliderGroove:
                {
                    QPoint grooveCenter(r.center());

                    if (Qt::Horizontal==slider->orientation)
                    {
                        rect.setHeight(13);
                        --grooveCenter.ry();
                        if (slider->tickPosition & QSlider::TicksAbove)
                            grooveCenter.ry() += (tickSize+2);
                        if (slider->tickPosition & QSlider::TicksBelow)
                            grooveCenter.ry() -= (tickSize-1);
                    }
                    else
                    {
                        rect.setWidth(13);
                        --grooveCenter.rx();
                        if (slider->tickPosition & QSlider::TicksAbove)
                            grooveCenter.rx() += (tickSize+2);
                        if (slider->tickPosition & QSlider::TicksBelow)
                            grooveCenter.rx() -= (tickSize-1);
                    }
                    rect.moveCenter(grooveCenter);
                    break;
                }
                default:
                    break;
                }
                return rect;
            }
            else
            {
                bool horizontal(Qt::Horizontal==slider->orientation);
                int  thickness(pixelMetric(PM_SliderControlThickness, slider, widget)),
                    tickOffset(slider->tickPosition&QSlider::TicksAbove ||
                               slider->tickPosition&QSlider::TicksBelow
                               ? pixelMetric(PM_SliderTickmarkOffset, slider, widget)
                               : ((horizontal ? r.height() : r.width()) - thickness)/2);

                switch (subControl)
                {
                case SC_SliderHandle:
                {
                    int len(pixelMetric(PM_SliderLength, slider, widget)),
                        sliderPos(sliderPositionFromValue(slider->minimum, slider->maximum,
                                                          slider->sliderPosition,
                                                          (horizontal ? r.width()
                                                           : r.height()) - len,
                                                          slider->upsideDown));

                    if (horizontal)
                        r.setRect(r.x() + sliderPos, r.y() + tickOffset, len, thickness);
                    else
                        r.setRect(r.x() + tickOffset, r.y() + sliderPos, thickness, len);
                    break;
                }
                case SC_SliderGroove:
                    if (horizontal)
                        r.setRect(r.x(), r.y() + tickOffset, r.width(), thickness);
                    else
                        r.setRect(r.x() + tickOffset, r.y(), thickness, r.height());
                    break;
                default:
                    break;
                }
                return visualRect(slider->direction, r, r);
            }
        }
        break;
    case CC_GroupBox:
        if(SC_GroupBoxCheckBox==subControl || SC_GroupBoxLabel==subControl)
            if (const QStyleOptionGroupBox *groupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(option))
            {
                QFont font(widget ? widget->font() : QApplication::font());

                font.setBold(opts.gbLabel&GB_LBL_BOLD);

                QFontMetrics fontMetrics(font);
                int          h(fontMetrics.height()),
                    tw(fontMetrics.size(Qt::TextShowMnemonic, groupBox->text + QLatin1Char(' ')).width()),
                    marg((groupBox->features & QStyleOptionFrame::Flat) ||
                         NO_FRAME(opts.groupBox) || opts.gbLabel&GB_LBL_OUTSIDE
                         ? 0
                         : opts.gbLabel&GB_LBL_INSIDE
                         ? 2
                         : 6),
                    indicatorWidth(pixelMetric(PM_IndicatorWidth, option, widget)),
                    indicatorSpace(pixelMetric(PM_CheckBoxLabelSpacing, option, widget) - 1);
                bool         hasCheckBox(groupBox->subControls & QStyle::SC_GroupBoxCheckBox);
                int          checkBoxSize(hasCheckBox ? (indicatorWidth + indicatorSpace) : 0),
                    checkAdjust(NO_FRAME(opts.groupBox) || opts.gbLabel&GB_LBL_OUTSIDE ? 0 : 2);

                if(0==checkAdjust)
                    checkBoxSize-=2;

                r.adjust(marg, 0, -marg, 0);
                if(!NO_FRAME(opts.groupBox) && opts.gbLabel&GB_LBL_INSIDE)
                    r.adjust(0, 2, 0, 2);
                r.setHeight(h);

                // Adjusted rect for label + indicatorWidth + indicatorSpace
                Qt::Alignment align(groupBox->textAlignment);
                if(opts.gbLabel&GB_LBL_CENTRED)
                {
                    align&=~(Qt::AlignLeft|Qt::AlignRight);
                    align|=Qt::AlignHCenter;
                }
                r=alignedRect(groupBox->direction, align, QSize(tw + checkBoxSize, h), r);

                // Adjust totalRect if checkbox is set
                if (hasCheckBox)
                {
                    if (SC_GroupBoxCheckBox==subControl) // Adjust for check box
                    {
                        int indicatorHeight(pixelMetric(PM_IndicatorHeight, option, widget)),
                            top(r.top() + (fontMetrics.height() - indicatorHeight) / 2);

                        r.setRect(reverse ? (r.right() - indicatorWidth) : r.left()+checkAdjust, top, indicatorWidth, indicatorHeight);
                    }
                    else // Adjust for label
                        r.setRect(reverse ? r.left() : (r.left() + checkBoxSize), r.top(), r.width() - checkBoxSize, r.height());
                }
                return r;
            }
        break;
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(option))
        {
            bool isMinimized(tb->titleBarState&Qt::WindowMinimized),
                isMaximized(tb->titleBarState&Qt::WindowMaximized);

            if( (isMaximized && SC_TitleBarMaxButton==subControl) ||
                (isMinimized && SC_TitleBarMinButton==subControl) ||
                (isMinimized && SC_TitleBarShadeButton==subControl) ||
                (!isMinimized && SC_TitleBarUnshadeButton==subControl))
                return QRect();

            readMdiPositions();

            const int controlSize(tb->rect.height() - constWindowMargin *2);

            QList<int>::ConstIterator it(itsMdiButtons[0].begin()),
                end(itsMdiButtons[0].end());
            int                       sc(SC_TitleBarUnshadeButton==subControl
                                         ? SC_TitleBarShadeButton
                                         : SC_TitleBarNormalButton==subControl
                                         ? isMaximized
                                         ? SC_TitleBarMaxButton
                                         : SC_TitleBarMinButton
                                         : subControl),
                pos(0),
                totalLeft(0),
                totalRight(0);
            bool                      rhs(false),
                found(false);

            for(; it!=end; ++it)
                if(SC_TitleBarCloseButton==(*it) || WINDOWTITLE_SPACER==(*it) || tb->titleBarFlags&(toHint(*it)))
                {
                    totalLeft+=WINDOWTITLE_SPACER==(*it) ? controlSize/2 : controlSize;
                    if(*it==sc)
                        found=true;
                    else if(!found)
                        pos+=WINDOWTITLE_SPACER==(*it) ? controlSize/2 : controlSize;
                }

            if(!found)
            {
                pos=0;
                rhs=true;
            }

            it=itsMdiButtons[1].begin();
            end=itsMdiButtons[1].end();
            for(; it!=end; ++it)
                if(SC_TitleBarCloseButton==(*it) || WINDOWTITLE_SPACER==(*it) || tb->titleBarFlags&(toHint(*it)))
                {
                    if(WINDOWTITLE_SPACER!=(*it) || totalRight)
                        totalRight+=WINDOWTITLE_SPACER==(*it) ? controlSize/2 : controlSize;
                    if(rhs)
                    {
                        if(*it==sc)
                        {
                            pos+=controlSize;
                            found=true;
                        }
                        else if(found)
                            pos+=WINDOWTITLE_SPACER==(*it) ? controlSize/2 : controlSize;
                    }
                }

            totalLeft+=(constWindowMargin*(totalLeft ? 2 : 1));
            totalRight+=(constWindowMargin*(totalRight ? 2 : 1));

            if(SC_TitleBarLabel==subControl)
                r.adjust(totalLeft, 0, -totalRight, 0);
            else if(!found)
                return QRect();
            else if(rhs)
                r.setRect(r.right()-(pos+constWindowMargin), r.top()+constWindowMargin, controlSize, controlSize);
            else
                r.setRect(r.left()+constWindowMargin+pos, r.top()+constWindowMargin, controlSize, controlSize);
            if(0==(r.height()%2))
                r.adjust(0, 0, 1, 1);
            return visualRect(tb->direction, tb->rect, r);
        }
    default:
        break;
    }

    return BASE_STYLE::subControlRect(control, option, subControl, widget);
}

QStyle::SubControl
Style::hitTestComplexControl(ComplexControl control,
                             const QStyleOptionComplex *option,
                             const QPoint &pos, const QWidget *widget) const
{
    itsSbWidget = 0L;
    switch (control) {
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            if (subControlRect(control, scrollBar, SC_ScrollBarSlider, widget).contains(pos))
                return SC_ScrollBarSlider;

            if (subControlRect(control, scrollBar, SC_ScrollBarAddLine, widget).contains(pos))
                return SC_ScrollBarAddLine;

            if (subControlRect(control, scrollBar, SC_ScrollBarSubPage, widget).contains(pos))
                return SC_ScrollBarSubPage;

            if (subControlRect(control, scrollBar, SC_ScrollBarAddPage, widget).contains(pos))
                return SC_ScrollBarAddPage;

            if (subControlRect(control, scrollBar, SC_ScrollBarSubLine, widget).contains(pos))
            {
                if (SCROLLBAR_KDE==opts.scrollbarType && subControlRect(control, scrollBar, SB_SUB2, widget).contains(pos))
                    itsSbWidget=widget;
                return SC_ScrollBarSubLine;
            }
        }
    default:
        break;
    }

    return BASE_STYLE::hitTestComplexControl(control, option,  pos, widget);
}

}
