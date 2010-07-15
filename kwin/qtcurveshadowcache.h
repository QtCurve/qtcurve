#ifndef __QTCURVESHADOWCACHE_H__
#define __QTCURVESHADOWCACHE_H__

/*
  QtCurve KWin window decoration
  Copyright (C) 2007 - 2010 Craig Drummond <craig.p.drummond@googlemail.com>
*/

//////////////////////////////////////////////////////////////////////////////
// Taken from: oxygenshadowcache.h
// handles caching of TileSet objects to draw shadows
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
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

//#define NEW_SHADOWS

#include <QtCore/QCache>
#include <QtGui/QRadialGradient>

#include "qtcurveshadowconfiguration.h"
#include "tileset.h"
#include <cmath>

class QtCurveHelper;

namespace KWinQtCurve
{

class QtCurveClient;

class QtCurveShadowCache
{
    public:

    QtCurveShadowCache();
    virtual ~QtCurveShadowCache() { }

    void invalidateCaches()
    {
        shadowCache_.clear();
    }

    //! returns true if provided shadow configuration changes with respect to stored
    /*!
    use QtCurveShadowConfiguration::colorRole() to decide whether it should be stored
    as active or inactive
    */
    bool shadowConfigurationChanged(const QtCurveShadowConfiguration &other) const;

    //! set shadowConfiguration
    /*!
    use QtCurveShadowConfiguration::colorRole() to decide whether it should be stored
    as active or inactive
    */
    void setShadowConfiguration(const QtCurveShadowConfiguration &other);

    //! shadow size
    qreal shadowSize() const
    {
        qreal size(qMax(activeShadowConfiguration_.shadowSize(), inactiveShadowConfiguration_.shadowSize()));

        // even if shadows are disabled, you need a minimum size to allow corner rendering
        return qMax(size, qreal(5.0));
    }

    TileSet * tileSet(const QtCurveClient *client, bool roundAllCorners);

    //! Key class to be used into QCache
    /*! class is entirely inline for optimization */
    class Key
    {
        public:

        explicit Key() : active(false), isShade(false) {}
        Key(const QtCurveClient *client);
        Key(int hash) : active((hash>>1)&1), isShade((hash)&1) {}

        int hash() const { return (active <<1)|(isShade); }

        bool active,
             isShade;
    };

    static qreal square(qreal x) { return x*x; }

    class Parabolic
    {
        public:

        //! constructor
        Parabolic(qreal amplitude, qreal width): amplitude_( amplitude ), width_( width ) {}

        //! destructor
        virtual ~Parabolic() {}

        //! value
        virtual qreal operator() (qreal x)  const { return qMax( 0.0, amplitude_*(1.0 - square(x/width_) ) ); }

        private:

        qreal amplitude_;
        qreal width_;

    };

    class Gaussian
    {
        public:

        Gaussian(qreal amplitude, qreal width): amplitude_(amplitude), width_(width) {}

        virtual ~Gaussian() {}

        //! value
        virtual qreal operator() (qreal x) const { return qMax( 0.0, amplitude_*(std::exp( -square(x/width_) -0.05 ) ) ); }

        private:

        qreal amplitude_,
              width_;
    };

    //! complex pixmap (when needed)
    QPixmap shadowPixmap(const QtCurveClient *client, bool active, bool roundAllCorners) const;

    //! simple pixmap
    QPixmap simpleShadowPixmap(const QColor &color, bool active, bool roundAllCorners) const;

    void reset() { shadowCache_.clear(); }

    private:

    //! draw gradient into rect
    /*! a separate method is used in order to properly account for corners */
    void renderGradient(QPainter &p, const QRectF &rect, const QRadialGradient &rg, bool hasBorder) const;

    typedef QCache<int, TileSet> TileSetCache;

    QtCurveShadowConfiguration activeShadowConfiguration_,
                               inactiveShadowConfiguration_;
    TileSetCache               shadowCache_;
};
}

#endif
