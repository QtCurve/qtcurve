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

#include <QtCore/QCache>
#include <QtGui/QRadialGradient>

#include "qtcurveshadowconfiguration.h"
#include "tileset.h"

class QtCurveHelper;

namespace KWinQtCurve
{

  class QtCurveClient;
  class QtCurveShadowCache
  {
    public:

    //! constructor
    QtCurveShadowCache( int maxIndex=256 );

    //! destructor
    virtual ~QtCurveShadowCache( void )
    {}

    //! invalidate caches
    void invalidateCaches( void )
    {
      shadowCache_.clear();
//       animatedShadowCache_.clear();
    }

    //! returns true if provided shadow configuration changes with respect to stored
    /*!
    use QtCurveShadowConfiguration::colorRole() to decide whether it should be stored
    as active or inactive
    */
    bool shadowConfigurationChanged( const QtCurveShadowConfiguration& ) const;

    //! set shadowConfiguration
    /*!
    use QtCurveShadowConfiguration::colorRole() to decide whether it should be stored
    as active or inactive
    */
    void setShadowConfiguration( const QtCurveShadowConfiguration& );

    //! shadow size
    qreal shadowSize( void ) const
    {
        qreal size( qMax( activeShadowConfiguration_.shadowSize(), inactiveShadowConfiguration_.shadowSize() ) );

        // even if shadows are disabled,
        // you need a minimum size to allow corner rendering
        return qMax( size, qreal(5.0) );
    }

    //! get shadow matching client
    TileSet* tileSet( const QtCurveClient* );

    //! get shadow matching client and animation index
//     TileSet* tileSet( const QtCurveClient*, int );

    //! Key class to be used into QCache
    /*! class is entirely inline for optimization */
    class Key
    {

      public:

      //! explicit constructor
      explicit Key( void ):
        index(0),
        active(false),
        isShade(false)
      {}

      //! constructor from client
      Key( const QtCurveClient* );

      //! constructor from int
      Key( int hash ):
        index( hash>>5 ),
        active( (hash>>4)&1 ),
        isShade( (hash>>2)&1 )
      {}

      //! hash function
      int hash( void ) const
      {

        // note this can be optimized because not all of the flag configurations are actually relevant
        // allocate 3 empty bits for flags
        return
          ( index << 5 ) |
          ( active << 4 ) |
          (isShade<<2);

      }

      int index;
      bool active;
      bool isShade;
    };

    //! complex pixmap (when needed)
    QPixmap shadowPixmap( const QtCurveClient*, bool active ) const;

    //! simple pixmap
    QPixmap simpleShadowPixmap( const QColor& color, const Key& key ) const
    { return simpleShadowPixmap( color, key, key.active ); }

    //! simple pixmap
    QPixmap simpleShadowPixmap( const QColor&, const Key&, bool active ) const;

    void reset()
    {
        shadowCache_.clear();
//         animatedShadowCache_.clear();
    }
    
    protected:

//     QtCurveHelper& helper( void ) const
//     { return helper_; }

    private:

    //! draw gradient into rect
    /*! a separate method is used in order to properly account for corners */
    void renderGradient( QPainter&, const QRectF&, const QRadialGradient&/*, bool hasBorder = true*/ ) const;

    //! max index
    /*! it is used to set caches max cost, and calculate animation opacity */
    int maxIndex_;

    //! shadow configuration
    QtCurveShadowConfiguration activeShadowConfiguration_;

    //! shadow configuration
    QtCurveShadowConfiguration inactiveShadowConfiguration_;

    //! cache
    typedef QCache<int, TileSet> TileSetCache;

    //! shadow cache
    TileSetCache shadowCache_;

    //! animated shadow cache
//     TileSetCache animatedShadowCache_;

  };

}

#endif
