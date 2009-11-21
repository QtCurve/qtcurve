/*
  QtCurve KWin window decoration
  Copyright (C) 2007 - 2009 Craig Drummond <ee11cd@googlemail.com>

  This class is taken, and modified from the Bespin KWin window decoration.
  Copyright (c) 2008 Thomas Lübking <baghira-style@gmx.net>

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

#include <QPainter>
#include <QPolygon>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>
#include <QStyleOptionSizeGrip>

#ifdef Q_WS_X11
#include <QX11Info>
#include <X11/Xlib.h>
#include "fixx11h.h"
#endif

#include "qtcurveclient.h"
#include "qtcurvehandler.h"
#include "resizecorner.h"

#define CORNER_SIZE 12

namespace KWinQtCurve
{

ResizeCorner::ResizeCorner(QtCurveClient *parent, const QColor &c)
            : QWidget(parent->widget()),
              client(parent)
{
    hide();
    if (!(parent->widget() && parent->windowId()))
    {
        deleteLater();
        return;
    }
    setColor(c);
    setAutoFillBackground(true);
    setCursor(QCursor(Qt::SizeFDiagCursor));
    setFixedSize(CORNER_SIZE, CORNER_SIZE);
    QPolygon triangle(3);
    triangle.putPoints(0, 3, CORNER_SIZE,0, CORNER_SIZE,CORNER_SIZE, 0,CORNER_SIZE);
    setMask(triangle);
    raise();
    installEventFilter(this);
    show();
}

void ResizeCorner::raise()
{
    WId  root,
         daddy = 0,
         *kids = 0L;
    uint numKids = 0;

    XQueryTree(QX11Info::display(), client->windowId(), &root, &daddy, &kids, &numKids);
    if (daddy)
        XReparentWindow(QX11Info::display(), winId(), daddy, 0, 0);
    move(client->width() - CORNER_SIZE, client->height() - CORNER_SIZE);
    client->widget()->removeEventFilter(this);
    client->widget()->installEventFilter(this);
}

void ResizeCorner::move(int x, int y)
{
   int l, r, t, b;
   client->borders(l, r, t, b);
   QWidget::move(x-(l+r), y-(t+b));
}

static inline bool similar(const QColor &a, const QColor &b)
{
    static const int constDiff=3;

    return abs(a.red()-b.red())<constDiff &&
           abs(a.green()-b.green())<constDiff &&
           abs(a.blue()-b.blue())<constDiff;
}

void ResizeCorner::setColor(const QColor &c)
{
    QColor   col=c;
    QPalette pal(palette());

    if(similar(col, client->widget()->palette().color(backgroundRole())))
        col = col.value() > 100 ? col.dark(130) : col.light(120);
    pal.setColor(backgroundRole(), col);
    setPalette(pal);
}
 
bool ResizeCorner::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == this && QEvent::ZOrderChange==ev->type())
    {
        removeEventFilter(this);
        raise();
        installEventFilter(this);
        return false;
    }
    
    if (obj == parent() && QEvent::Resize==ev->type())
        move(client->width() - CORNER_SIZE, client->height() - CORNER_SIZE);

    return false;
}

static Atom netMoveResize = XInternAtom(QX11Info::display(), "_NET_WM_MOVERESIZE", False);

void ResizeCorner::mousePressEvent(QMouseEvent *ev)
{
    if(Qt::LeftButton==ev->button())
    {
        // complex way to say: client->performWindowOperation(KDecoration::ResizeOp);
        // stolen... errr "adapted!" from QSizeGrip
        QX11Info info;
        QPoint p = ev->globalPos();
        XEvent xev;
        xev.xclient.type = ClientMessage;
        xev.xclient.message_type = netMoveResize;
        xev.xclient.display = QX11Info::display();
        xev.xclient.window = client->windowId();
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = p.x();
        xev.xclient.data.l[1] = p.y();
        xev.xclient.data.l[2] = 4; // _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHTMove
        xev.xclient.data.l[3] = Button1;
        xev.xclient.data.l[4] = 0;
        XUngrabPointer(QX11Info::display(), QX11Info::appTime());
        XSendEvent(QX11Info::display(), QX11Info::appRootWindow(info.screen()), False,
                    SubstructureRedirectMask | SubstructureNotifyMask, &xev);
    }
}

void ResizeCorner::mouseReleaseEvent(QMouseEvent *)
{
   client->performWindowOperation(KDecoration::NoOp);
}

// void ResizeCorner::paintEvent(QPaintEvent *)
// {
// //     QRect           r(rect());
// //     QLinearGradient grad(QPoint(r.x(), r.y()), QPoint(r.x()+r.width()-1, r.y()+r.height()-1));
// //     QColor          s(col);
// // 
// //     s.setAlphaF(0);
// //     grad.setColorAt(0, s);
// //     grad.setColorAt(0.5, s);
// //     grad.setColorAt(1, col);
// //     QPainter(this).fillRect(r, QBrush(grad));
//     QPainter p(this);
//         p.setPen(Qt::red);
// 
//     QPolygon triangle(3);
//     triangle.putPoints(0, 3, CORNER_SIZE,0, CORNER_SIZE,CORNER_SIZE, 0,CORNER_SIZE);
//     p.drawPolygon(triangle);
// //     p.fillRect(rect(), col);
// }

}

