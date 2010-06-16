// krazy:excludeall=qclasses

// Copied from oxygenwindowmanager.cpp svnversion: 1137865

//////////////////////////////////////////////////////////////////////////////
// oxygenwindowmanager.cpp
// pass some window mouse press/release/move event actions to window manager
// -------------------
//
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
//
// Largely inspired from BeSpin style
// Copyright (C) 2007 Thomas Luebking <thomas.luebking@web.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "windowmanager.h"
#include "qtcurve.h"
#include "common.h"

#include <QtGui/QApplication>
#include <QtGui/QComboBox>
#include <QtGui/QDockWidget>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QListView>
#include <QtGui/QMenuBar>
#include <QtGui/QMouseEvent>
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionGroupBox>
#include <QtGui/QTabBar>
#include <QtGui/QTabWidget>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>
#include <QtGui/QTreeView>
#include <QtGui/QGraphicsView>

#include <QtCore/QTextStream>
#include <QtGui/QTextDocument>

#ifndef QTC_QT_ONLY
#include <KGlobalSettings>
#endif

#ifdef Q_WS_X11
#include <QX11Info>
#ifdef QTC_QT_ONLY
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "fixx11h.h"
#else
#include <NETRootInfo>
#endif
#endif

namespace QtCurve
{

    //_____________________________________________________________
    WindowManager::WindowManager( QObject* parent ):
        QObject( parent ),
        enabled_( true ),
#ifdef Q_WS_X11
        useWMMoveResize_( true ),
#else
        useWMMoveResize_( false ),
#endif
        dragMode_( WM_DRAG_NONE ),
#ifdef QTC_QT_ONLY
        dragDistance_( QApplication::startDragDistance() ),
#else
        dragDistance_( KGlobalSettings::dndEventDelay() ),
#endif
        dragDelay_( QApplication::startDragTime() ),
        dragAboutToStart_( false ),
        dragInProgress_( false ),
        locked_( false ),
        cursorOverride_( false )
    {

        // install application wise event filter
        appEventFilter_ = new AppEventFilter( this );
        qApp->installEventFilter( appEventFilter_ );

    }

    //_____________________________________________________________
    void WindowManager::initialize( int windowDrag )
    {

        setEnabled( windowDrag );
        setDragMode( windowDrag );
//CPD: Why???        setUseWMMoveResize( OxygenStyleConfigData::useWMMoveResize() );

#ifndef QTC_QT_ONLY
        setDragDistance( KGlobalSettings::dndEventDelay() );
#endif
        setDragDelay( QApplication::startDragTime() );

        initializeWhiteList();
        initializeBlackList();

    }

    //_____________________________________________________________
    void WindowManager::registerWidget( QWidget* widget )
    {

        if( isBlackListed( widget ) )
        {

            /*
            also install filter for blacklisted widgets
            to be able to catch the relevant events and prevent
            the drag to happen
            */
            widget->removeEventFilter( this );
            widget->installEventFilter( this );

        } else if( isDragable( widget ) ) {

            widget->removeEventFilter( this );
            widget->installEventFilter( this );

        }

    }

    //_____________________________________________________________
    void WindowManager::unregisterWidget( QWidget* widget )
    {
        if( widget )
        { widget->removeEventFilter( this ); }
    }

    //_____________________________________________________________
    void WindowManager::initializeWhiteList( void )
    {

        whiteList_.clear();

        // add user specified whitelisted classnames
        whiteList_.insert( ExceptionId( "MplayerWindow" ) );
        whiteList_.insert( ExceptionId( "ViewSliders@kmix" ) );
        whiteList_.insert( ExceptionId( "Sidebar_Widget@konqueror" ) );

#if 0 // CPD: TODO!!!
        foreach( const QString& exception, OxygenStyleConfigData::windowDragWhiteList() )
        {
            ExceptionId id( exception );
            if( !id.className().isEmpty() )
            { whiteList_.insert( exception ); }
        }
#endif
    }

    //_____________________________________________________________
    void WindowManager::initializeBlackList( void )
    {

        blackList_.clear();
        blackList_.insert( ExceptionId( "CustomTrackView@kdenlive" ) );
#if 0 // CPD: TODO!!!
        foreach( const QString& exception, OxygenStyleConfigData::windowDragBlackList() )
        {
            ExceptionId id( exception );
            if( !id.className().isEmpty() )
            { blackList_.insert( exception ); }
        }
#endif
    }

    //_____________________________________________________________
    bool WindowManager::eventFilter( QObject* object, QEvent* event )
    {
        if( !enabled() ) return false;

        switch ( event->type() )
        {
            case QEvent::MouseButtonPress:
                return mousePressEvent( object, event );
            break;

            case QEvent::MouseMove:
                if ( object == target_.data() ) return mouseMoveEvent( object, event );
            break;

            case QEvent::MouseButtonRelease:
                if ( target_ ) return mouseReleaseEvent( object, event );
            break;

            default:
            break;

        }

        return false;

    }

    //_____________________________________________________________
    void WindowManager::timerEvent( QTimerEvent* event )
    {

        if( event->timerId() == dragTimer_.timerId() )
        {
            dragTimer_.stop();
            if( target_ )
            { startDrag( target_.data(), globalDragPoint_ ); }

        } else {

            return QObject::timerEvent( event );

        }

    }

    //_____________________________________________________________
    bool WindowManager::mousePressEvent( QObject* object, QEvent* event )
    {

        // cast event and check buttons/modifiers
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>( event );
        if( !( mouseEvent->modifiers() == Qt::NoModifier && mouseEvent->button() == Qt::LeftButton ) )
        { return false; }

        // check lock
        if( isLocked() ) return false;
        else setLocked( true );

        // cast to widget
        QWidget *widget = static_cast<QWidget*>( object );

        // check if widget can be dragged from current position
        if( isBlackListed( widget ) || !canDrag( widget ) ) return false;

        // retrieve widget's child at event position
        QPoint position( mouseEvent->pos() );
        QWidget* child = widget->childAt( position );
        if( !canDrag( widget, child, position ) ) return false;

        // save target and drag point
        target_ = widget;
        dragPoint_ = position;
        globalDragPoint_ = mouseEvent->globalPos();
        dragAboutToStart_ = true;

        // send a move event to the current child with same position
        // if received, it is caught to actually start the drag
        QPoint localPoint( dragPoint_ );
        if( child ) localPoint = child->mapFrom( widget, localPoint );
        else child = widget;
        QMouseEvent localMouseEvent( QEvent::MouseMove, localPoint, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
        qApp->sendEvent( child, &localMouseEvent );

        // never eat event
        return false;

    }

    //_____________________________________________________________
    bool WindowManager::mouseMoveEvent( QObject* object, QEvent* event )
    {

        Q_UNUSED( object );

        // stop timer
        if( dragTimer_.isActive() ) dragTimer_.stop();

        // cast event and check drag distance
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>( event );
        if( !dragInProgress_ )
        {

            if( dragAboutToStart_ )
            {
                if( mouseEvent->globalPos() == globalDragPoint_ )
                {
                    // start timer,
                    dragAboutToStart_ = false;
                    if( dragTimer_.isActive() ) dragTimer_.stop();
                    dragTimer_.start( dragDelay_, this );

                } else resetDrag();

            } else if( QPoint( mouseEvent->globalPos() - globalDragPoint_ ).manhattanLength() >= dragDistance_ )
            { dragTimer_.start( 0, this ); }
            return true;

        } else if( !useWMMoveResize() ) {

            // use QWidget::move for the grabbing
            /* this works only if the sending object and the target are identical */
            QWidget* window( target_.data()->window() );
            window->move( window->pos() + mouseEvent->pos() - dragPoint_ );
            return true;

        } else return false;

    }

    //_____________________________________________________________
    bool WindowManager::mouseReleaseEvent( QObject* object, QEvent* event )
    {
        Q_UNUSED( object );
        Q_UNUSED( event );
        resetDrag();
        return false;
    }

    //_____________________________________________________________
    bool WindowManager::isDragable( QWidget* widget )
    {

        // check widget
        if( !widget ) return false;

        // all accepted default types
        if(
            ( widget->inherits( "QDialog" ) && widget->isWindow() ) ||
            ( widget->inherits( "QMainWindow" ) && widget->isWindow() ) ||
            widget->inherits( "QGroupBox" ) ||
            widget->inherits( "QMenuBar" ) ||
            widget->inherits( "QTabBar" ) ||
            widget->inherits( "QStatusBar" ) ||
            widget->inherits( "QToolBar" ) )
        { return true; }

        if( widget->inherits( "KScreenSaver" ) && widget->inherits( "KCModule" ) )
        { return true; }

        if( isWhiteListed( widget ) )
        { return true; }

        // flat toolbuttons
        if( QToolButton* toolButton = qobject_cast<QToolButton*>( widget ) )
        { if( toolButton->autoRaise() ) return true; }

        // viewports
        /*
        one needs to check that
        1/ the widget parent is a scrollarea
        2/ it matches its parent viewport
        3/ the parent is not blacklisted
        */
        if( QListView* listView = qobject_cast<QListView*>( widget->parentWidget() ) )
        { if( listView->viewport() == widget && !isBlackListed( listView ) ) return true; }

        if( QTreeView* treeView = qobject_cast<QTreeView*>( widget->parentWidget() ) )
        { if( treeView->viewport() == widget && !isBlackListed( treeView ) ) return true; }

        if( QGraphicsView* graphicsView = qobject_cast<QGraphicsView*>( widget->parentWidget() ) )
        { if( graphicsView->viewport() == widget && !isBlackListed( graphicsView ) ) return true; }

        /*
        catch labels in status bars.
        this is because of kstatusbar
        who captures buttonPress/release events
        */
        if( QLabel* label = qobject_cast<QLabel*>( widget ) )
        {
            if( label->textInteractionFlags().testFlag( Qt::TextSelectableByMouse ) ) return false;

            QWidget* parent = label->parentWidget();
            while( parent )
            {
                if( parent->inherits( "QStatusBar" ) ) return true;
                parent = parent->parentWidget();
            }
        }

        return false;

    }

    //_____________________________________________________________
    bool WindowManager::isBlackListed( QWidget* widget )
    {

        // hard coded blacklisted widgets
        if(
            widget->inherits( "QComboBox" ) ||
            widget->inherits( "KCategorizedView" )
        )
        { return true; }

        // list-based blacklisted widgets
        QString appName( qApp->applicationName() );
        foreach( const ExceptionId& id, blackList_ )
        {
            if( !id.appName().isEmpty() && id.appName() != appName ) continue;
            if( id.className() == "*" && !id.appName().isEmpty() )
            {
                // if application name matches and all classes are selected
                // disable the grabbing entirely
                setEnabled( false );
                return true;
            }
            if( widget->inherits( id.className().toLatin1() ) ) return true;
        }

        return false;
    }

    //_____________________________________________________________
    bool WindowManager::isWhiteListed( QWidget* widget ) const
    {

        //foreach( const QString& className, blackList_ )
        QString appName( qApp->applicationName() );
        foreach( const ExceptionId& id, whiteList_ )
        {
            if( !id.appName().isEmpty() && id.appName() != appName ) continue;
            if( widget->inherits( id.className().toLatin1() ) ) return true;
        }

        return false;
    }

    //_____________________________________________________________
    bool WindowManager::canDrag( QWidget* widget )
    {

        // check if enabled
        if( !enabled() ) return false;

        // assume isDragable widget is already passed
        // check some special cases where drag should not be effective

        // check mouse grabber
        if( QWidget::mouseGrabber() ) return false;

        /*
        check cursor shape.
        Assume that a changed cursor means that some action is in progress
        and should prevent the drag
        */
        if( widget->cursor().shape() != Qt::ArrowCursor ) return false;

        // accept
        return true;

    }

    //_____________________________________________________________
    bool WindowManager::canDrag( QWidget* widget, QWidget* child, const QPoint& position )
    {

        // retrieve child at given position and check cursor again
        if( child && child->cursor().shape() != Qt::ArrowCursor ) return false;

        // tool buttons
        if( QToolButton* toolButton = qobject_cast<QToolButton*>( widget ) )
        {
            if( dragMode() < WM_DRAG_ALL && !qobject_cast<QToolBar*>(widget->parentWidget() ) ) return false;
            return toolButton->autoRaise() && !toolButton->isEnabled();
        }

        // check menubar
        if( QMenuBar* menuBar = qobject_cast<QMenuBar*>( widget ) )
        {

            // check if there is an active action
            if( menuBar->activeAction() && menuBar->activeAction()->isEnabled() ) return false;

            // check if action at position exists and is enabled
            if( QAction* action = menuBar->actionAt( position ) )
            {
                if( action->isSeparator() ) return true;
                if( action->isEnabled() ) return false;
            }

            // return true in all other cases
            return true;

        }

        if(dragMode() < WM_DRAG_MENU_AND_TOOLBAR && qobject_cast<QToolBar*>( widget ))
            return false;

        /*
        in MINIMAL mode, anything that has not been already accepted
        and does not come from a toolbar is rejected
        */
        if( dragMode() < WM_DRAG_ALL )
        {
            if( widget->inherits( "QToolBar" ) ) return true;
            else return false;
        }

        /* following checks are relevant only for WD_FULL mode */

        // tabbar. Make sure no tab is under the cursor
        if( QTabBar* tabBar = qobject_cast<QTabBar*>( widget ) )
        { return tabBar->tabAt( position ) == -1; }

        /*
        check groupboxes
        prevent drag if unchecking grouboxes
        */
        if( QGroupBox *groupBox = qobject_cast<QGroupBox*>( widget ) )
        {
            // non checkable group boxes are always ok
            if( !groupBox->isCheckable() ) return true;

            // gather options to retrieve checkbox subcontrol rect
            QStyleOptionGroupBox opt;
            opt.initFrom( groupBox );
            if( groupBox->isFlat() ) opt.features |= QStyleOptionFrameV2::Flat;
            opt.lineWidth = 1;
            opt.midLineWidth = 0;
            opt.text = groupBox->title();
            opt.textAlignment = groupBox->alignment();
            opt.subControls = (QStyle::SC_GroupBoxFrame | QStyle::SC_GroupBoxCheckBox);
            if (!groupBox->title().isEmpty()) opt.subControls |= QStyle::SC_GroupBoxLabel;

            opt.state |= (groupBox->isChecked() ? QStyle::State_On : QStyle::State_Off);

            // check against groupbox checkbox
            if( groupBox->style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxCheckBox, groupBox ).contains( position ) )
            { return false; }

            // check against groupbox label
            if( !groupBox->title().isEmpty() && groupBox->style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxLabel, groupBox ).contains( position ) )
            { return false; }

            return true;

        }

        QAbstractItemView* itemView( NULL );
        if( (itemView = qobject_cast<QListView*>( widget->parentWidget() ) ) )
        {
            // QListView
            if( itemView->frameShape() != QFrame::NoFrame ) return false;
            else if(
                itemView->selectionMode() != QAbstractItemView::NoSelection &&
                itemView->selectionMode() != QAbstractItemView::SingleSelection &&
                itemView->model()->rowCount() ) return false;
            else if( itemView->indexAt( position ).isValid() ) return false;

        } else if( (itemView = qobject_cast<QAbstractItemView*>( widget->parentWidget() ) ) ) {

            // QAbstractItemView
            if( itemView->frameShape() != QFrame::NoFrame ) return false;
            else if( itemView->indexAt( position ).isValid() ) return false;

        } else if( QGraphicsView* graphicsView =  qobject_cast<QGraphicsView*>( widget->parentWidget() ) )  {

            // QGraphicsView
            if( graphicsView->frameShape() != QFrame::NoFrame ) return false;
            else if( graphicsView->dragMode() != QGraphicsView::NoDrag ) return false;
            else if( graphicsView->itemAt( position ) ) return false;

        }

        // labels
        if( QLabel* label = qobject_cast<QLabel*>( widget ) )
        { if( label->textInteractionFlags().testFlag( Qt::TextSelectableByMouse ) ) return false; }

        return true;

    }

    //____________________________________________________________
    void WindowManager::resetDrag( void )
    {

        if( (!useWMMoveResize() ) && target_ && cursorOverride_ ) {

          qApp->restoreOverrideCursor();
          cursorOverride_ = false;

        }

        target_.clear();
        if( dragTimer_.isActive() ) dragTimer_.stop();
        dragPoint_ = QPoint();
        globalDragPoint_ = QPoint();
        dragAboutToStart_ = false;
        dragInProgress_ = false;

    }

    //____________________________________________________________
    void WindowManager::startDrag( QWidget* widget, const QPoint& position )
    {

        if( !( enabled() && widget ) ) return;
        if( QWidget::mouseGrabber() ) return;

        // ungrab pointer
        if( useWMMoveResize() )
        {

            #ifdef Q_WS_X11
            #ifdef QTC_QT_ONLY
            static const Atom constNetMoveResize = XInternAtom(QX11Info::display(), "_NET_WM_MOVERESIZE", False);
            //...Taken from bespin...
            // stolen... errr "adapted!" from QSizeGrip
            QX11Info info;
            XEvent xev;
            xev.xclient.type = ClientMessage;
            xev.xclient.message_type = constNetMoveResize;
            xev.xclient.display = QX11Info::display();
            xev.xclient.window = widget->window()->winId();
            xev.xclient.format = 32;
            xev.xclient.data.l[0] = position.x();
            xev.xclient.data.l[1] = position.y();
            xev.xclient.data.l[2] = 8; // NET::Move
            xev.xclient.data.l[3] = Button1;
            xev.xclient.data.l[4] = 0;
            XUngrabPointer(QX11Info::display(), QX11Info::appTime());
            XSendEvent(QX11Info::display(), QX11Info::appRootWindow(info.screen()), False,
                       SubstructureRedirectMask | SubstructureNotifyMask, &xev);
            #else    
            XUngrabPointer(QX11Info::display(), QX11Info::appTime());
            NETRootInfo rootInfo(QX11Info::display(), NET::WMMoveResize);
            rootInfo.moveResizeRequest( widget->window()->winId(), position.x(), position.y(), NET::Move);
            #endif // QTC_QT_ONLY
            #endif

        }

        if( !useWMMoveResize() )
        {
            if( !cursorOverride_ )
            {
                qApp->setOverrideCursor( Qt::SizeAllCursor );
                cursorOverride_ = true;
            }
        }

        dragInProgress_ = true;

        return;

    }

    //____________________________________________________________
    bool WindowManager::supportWMMoveResize( void ) const
    {

        #ifdef Q_WS_X11
        return true;
        #endif

        return false;

    }

    //____________________________________________________________
    bool WindowManager::AppEventFilter::eventFilter( QObject* object, QEvent* event )
    {

        if( event->type() == QEvent::MouseButtonRelease )
        {

            // stop drag timer
            if( parent_->dragTimer_.isActive() )
            { parent_->resetDrag(); }

            // unlock
            if( parent_->isLocked() )
            { parent_->setLocked( false ); }

        }

        if( !parent_->enabled() ) return false;

        /*
        if a drag is in progress, the widget will not receive any event
        we trigger on the first MouseMove or MousePress events that are received
        by any widget in the application to detect that the drag is finished
        */
        if( parent_->useWMMoveResize() && parent_->dragInProgress_ && parent_->target_ && ( event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress ) )
        { return appMouseEvent( object, event ); }

        return false;

    }

    //_____________________________________________________________
    bool WindowManager::AppEventFilter::appMouseEvent( QObject* object, QEvent* event )
    {

        Q_UNUSED( object );

        // store target window (see later)
        QWidget* window( parent_->target_.data()->window() );

        /*
        post some mouseRelease event to the target, in order to counter balance
        the mouse press that triggered the drag. Note that it triggers a resetDrag
        */
        QMouseEvent mouseEvent( QEvent::MouseButtonRelease, parent_->dragPoint_, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
        qApp->sendEvent( parent_->target_.data(), &mouseEvent );

        if( event->type() == QEvent::MouseMove )
        {
            /*
            HACK: quickly move the main cursor out of the window and back
            this is needed to get the focus right for the window children
            the origin of this issue is unknown at the moment
            */
            const QPoint cursor = QCursor::pos();
            QCursor::setPos(window->mapToGlobal( window->rect().topRight() ) + QPoint(1, 0) );
            QCursor::setPos(cursor);

        }

        return true;

    }


}
