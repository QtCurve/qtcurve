/*
 * Taken from Qt4.6's qsharedpointer_impl.h
 * Requried so that QtCurve can build against Qt4.5
 */
/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <new>
#include <QtCore/qatomic.h>
#include <QtCore/qobject.h>    // for qobject_cast

template <class T>
class QtCWeakPointer
{
#ifndef Q_CC_NOKIAX86
    typedef T *QtCWeakPointer:: *RestrictedBool;
#endif
    typedef QtSharedPointer::ExternalRefCountData Data;

public:
    typedef T element_type;
    typedef T value_type;
    typedef value_type *pointer;
    typedef const value_type *const_pointer;
    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef qptrdiff difference_type;

    inline bool isNull() const { return d == 0 || d->strongref == 0 || value == 0; }
#ifndef Q_CC_NOKIAX86
    inline operator RestrictedBool() const { return isNull() ? 0 : &QtCWeakPointer::value; }
#else
    inline operator bool() const { return isNull() ? 0 : &QtCWeakPointer::value; }
#endif
    inline bool operator !() const { return isNull(); }
    inline T *data() const { return d == 0 || d->strongref == 0 ? 0 : value; }

    inline QtCWeakPointer() : d(0), value(0) { }
    inline ~QtCWeakPointer() { if (d && !d->weakref.deref()) delete d; }

#ifndef QT_NO_QOBJECT
    // special constructor that is enabled only if X derives from QObject
    template <class X>
    inline QtCWeakPointer(X *ptr) : d(ptr ? d->getAndRef(ptr) : 0), value(ptr)
    { }
#endif
    template <class X>
    inline QtCWeakPointer &operator=(X *ptr)
    { return *this = QtCWeakPointer(ptr); }

    inline QtCWeakPointer(const QtCWeakPointer<T> &o) : d(o.d), value(o.value)
    { if (d) d->weakref.ref(); }
    inline QtCWeakPointer<T> &operator=(const QtCWeakPointer<T> &o)
    {
        internalSet(o.d, o.value);
        return *this;
    }

    inline QtCWeakPointer(const QSharedPointer<T> &o) : d(o.d), value(o.data())
    { if (d) d->weakref.ref();}
    inline QtCWeakPointer<T> &operator=(const QSharedPointer<T> &o)
    {
        internalSet(o.d, o.value);
        return *this;
    }

    template <class X>
    inline QtCWeakPointer(const QtCWeakPointer<X> &o) : d(0), value(0)
    { *this = o; }

    template <class X>
    inline QtCWeakPointer<T> &operator=(const QtCWeakPointer<X> &o)
    {
        // conversion between X and T could require access to the virtual table
        // so force the operation to go through QSharedPointer
        *this = o.toStrongRef();
        return *this;
    }

    template <class X>
    inline bool operator==(const QtCWeakPointer<X> &o) const
    { return d == o.d && value == static_cast<const T *>(o.value); }

    template <class X>
    inline bool operator!=(const QtCWeakPointer<X> &o) const
    { return !(*this == o); }

    template <class X>
    inline QtCWeakPointer(const QSharedPointer<X> &o) : d(0), value(0)
    { *this = o; }

    template <class X>
    inline QtCWeakPointer<T> &operator=(const QSharedPointer<X> &o)
    {
        QSHAREDPOINTER_VERIFY_AUTO_CAST(T, X); // if you get an error in this line, the cast is invalid
        internalSet(o.d, o.data());
        return *this;
    }

    template <class X>
    inline bool operator==(const QSharedPointer<X> &o) const
    { return d == o.d; }

    template <class X>
    inline bool operator!=(const QSharedPointer<X> &o) const
    { return !(*this == o); }

    inline void clear() { *this = QtCWeakPointer<T>(); }

    inline QSharedPointer<T> toStrongRef() const { return QSharedPointer<T>(*this); }

#if defined(QWEAKPOINTER_ENABLE_ARROW)
    inline T *operator->() const { return data(); }
#endif

private:

#if defined(Q_NO_TEMPLATE_FRIENDS)
public:
#else
    template <class X> friend class QSharedPointer;
#endif

    inline void internalSet(Data *o, T *actual)
    {
        if (d == o) return;
        if (o)
            o->weakref.ref();
        if (d && !d->weakref.deref())
            delete d;
        d = o;
        value = actual;
    }

    Data *d;
    T *value;
};

