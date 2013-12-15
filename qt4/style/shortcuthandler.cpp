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

#include "shortcuthandler.h"
#include <QWidget>
#include <QMenu>
#include <QEvent>
#include <QKeyEvent>
#include <QMenuBar>

namespace QtCurve {
ShortcutHandler::ShortcutHandler(QObject *parent) : QObject(parent),
                                                    itsAltDown(false)
{
}

ShortcutHandler::~ShortcutHandler()
{
}

bool ShortcutHandler::hasSeenAlt(const QWidget *widget) const
{
    if (widget && !widget->isEnabled())
        return false;

    if (qobject_cast<const QMenu*>(widget)) {
        return itsOpenMenus.count() && itsOpenMenus.last()==widget;
        // const QWidget *w = widget;
        // while (w) {
        //     if (itsSeenAlt.contains((QWidget*)w))
        //         return true;
        //     w = w->parentWidget();
        // }
    } else {
        return (itsOpenMenus.isEmpty() &&
                itsSeenAlt.contains((QWidget*)(widget->window())));
    }
    return false;
}

bool ShortcutHandler::showShortcut(const QWidget *widget) const
{
    return itsAltDown && hasSeenAlt(widget);
}

void ShortcutHandler::widgetDestroyed(QObject *o)
{
    itsUpdated.remove(static_cast<QWidget *>(o));
    itsOpenMenus.removeAll(static_cast<QWidget *>(o));
}

void ShortcutHandler::updateWidget(QWidget *w)
{
    if(!itsUpdated.contains(w))
    {
        itsUpdated.insert(w);
        w->update();
        connect(w, SIGNAL(destroyed(QObject *)), this, SLOT(widgetDestroyed(QObject *)));
    }
}

bool ShortcutHandler::eventFilter(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return QObject::eventFilter(o, e);

    QWidget *widget = static_cast<QWidget*>(o);
    switch (e->type()) {
    case QEvent::KeyPress:
        if (Qt::Key_Alt == static_cast<QKeyEvent*>(e)->key()) {
            itsAltDown = true;
            if (qobject_cast<QMenu*>(widget)) {
                itsSeenAlt.insert(widget);
                updateWidget(widget);
                if (widget->parentWidget() &&
                    widget->parentWidget()->window()) {
                    itsSeenAlt.insert(widget->parentWidget()->window());
                }
            } else {
                widget = widget->window();
                itsSeenAlt.insert(widget);
                QList<QWidget*> l = widget->findChildren<QWidget*>();
                for (int pos = 0;pos < l.size();++pos) {
                    QWidget *w = l.at(pos);
                    if (!(w->isWindow() || !w->isVisible())) {
                        // || w->style()->styleHint(
                        //     QStyle::SH_UnderlineShortcut, 0, w)))
                        updateWidget(w);
                    }
                }
                QList<QMenuBar*> m = widget->findChildren<QMenuBar*>();
                for (int i = 0; i < m.size(); ++i)
                    updateWidget(m.at(i));
            }
        }
        break;
    case QEvent::WindowDeactivate:
    case QEvent::KeyRelease:
        if (QEvent::WindowDeactivate == e->type() ||
            Qt::Key_Alt == static_cast<QKeyEvent*>(e)->key()) {
            itsAltDown = false;
            foreach (QWidget *widget, itsUpdated) {
                widget->update();
            }
            if (!itsUpdated.contains(widget))
                widget->update();
            itsSeenAlt.clear();
            itsUpdated.clear();
        }
        break;
    case QEvent::Show:
        if (qobject_cast<QMenu*>(widget)) {
            QWidget *prev = itsOpenMenus.count() ? itsOpenMenus.last() : 0L;
            itsOpenMenus.append(widget);
            if (itsAltDown && prev)
                prev->update();
            connect(widget, SIGNAL(destroyed(QObject*)),
                    this, SLOT(widgetDestroyed(QObject*)));
        }
        break;
    case QEvent::Hide:
        if (qobject_cast<QMenu*>(widget)) {
            itsSeenAlt.remove(widget);
            itsUpdated.remove(widget);
            itsOpenMenus.removeAll(widget);
            if (itsAltDown) {
                if (itsOpenMenus.count()) {
                    itsOpenMenus.last()->update();
                } else if (widget->parentWidget() &&
                           widget->parentWidget()->window())
                    widget->parentWidget()->window()->update();
            }
        }
        break;
    case QEvent::Close:
        // Reset widget when closing
        itsSeenAlt.remove(widget);
        itsUpdated.remove(widget);
        itsSeenAlt.remove(widget->window());
        itsOpenMenus.removeAll(widget);
        if (itsAltDown) {
            if (itsOpenMenus.count()) {
                itsOpenMenus.last()->update();
            } else if (widget->parentWidget() &&
                       widget->parentWidget()->window())
                widget->parentWidget()->window()->update();
        }
        break;
    default:
        break;
    }
    return QObject::eventFilter(o, e);
}

}
