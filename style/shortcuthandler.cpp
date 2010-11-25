/*
  QtCurve (C) Craig Drummond, 2007 - 2010 craig.p.drummond@gmail.com

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

#include "shortcuthandler.h"
#include <QtGui>

namespace QtCurve
{

ShortcutHandler::ShortcutHandler(QObject *parent)
               : QObject(parent)
{
}

ShortcutHandler::~ShortcutHandler()
{
}

bool ShortcutHandler::hasSeenAlt(const QWidget *widget) const
{
    if(qobject_cast<const QMenu *>(widget))
    {
        const QWidget *w=widget;
        
        while(w)
        {
            if(seenAlt.contains((QWidget *)w))
                return true;
            w=w->parentWidget();
        }
    }
    widget = widget->window();
    return seenAlt.contains((QWidget *)widget);
}

bool ShortcutHandler::showShortcut(const QWidget *widget) const
{
    return altDown && hasSeenAlt(widget);
}
                
bool ShortcutHandler::eventFilter(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return QObject::eventFilter(o, e);

    QWidget *widget = qobject_cast<QWidget*>(o);
    switch(e->type()) 
    {
        case QEvent::KeyPress:
            if (static_cast<QKeyEvent *>(e)->key() == Qt::Key_Alt) 
            {
                altDown = true;
                if(qobject_cast<QMenu *>(widget))
                {
                    QWidget *w=widget;
                    
                    while(w)
                    {
                        seenAlt.append(w);
                        w->update();
                        w=w->parentWidget();
                    }
                    if(!widget->parentWidget())
                    {
                        seenAlt.append(QApplication::activeWindow());
                        QApplication::activeWindow()->update();
                    }
                }
                widget = widget->window();
                seenAlt.append(widget);
                widget->update();
                QList<QWidget *> l = qFindChildren<QWidget *>(widget);
                for (int pos=0 ; pos < l.size() ; ++pos) 
                {
                    QWidget *w = l.at(pos);
                    if (!(w->isWindow() || !w->isVisible() ||
                        w->style()->styleHint(QStyle::SH_UnderlineShortcut, 0, w)))
                        l.at(pos)->update();
                }

                QList<QMenuBar *> m = qFindChildren<QMenuBar *>(widget);
                for (int i = 0; i < m.size(); ++i)
                    m.at(i)->update();
            }
            break;
        case QEvent::KeyRelease:
            if (static_cast<QKeyEvent*>(e)->key() == Qt::Key_Alt)
            {
                altDown = false;
                QList<QWidget *>::Iterator it(seenAlt.begin()),
                                           end(seenAlt.end());
                                           
                for (; it!=end; ++it)
                    (*it)->update();
                seenAlt.empty();
            }
            break;
        case QEvent::Close:
            // Reset widget when closing
            seenAlt.removeAll(widget);
            seenAlt.removeAll(widget->window());
            break;
        default:
            break;
    }
    return QObject::eventFilter(o, e);
}

}
