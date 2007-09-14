#!/bin/bash

version=`grep VERSION config.h | awk '{print $3}' | sed s:\"::g`
dir="QtCurve-KDE4-$version"

if [ -d "$dir" ] ; then
    rm -rf "$dir"
fi

mkdir "$dir"
for d in kde4 kde4/config qt4 ; do
    mkdir "$dir"/$d
    cp $d/*.cpp $d/*.h $d/*.c $d/CMakeLists.txt $d/*.ui $d/*.pro $d/*.themerc $d/*.qtcurve "$dir"/$d 2>/dev/null
done

# Need dummy moc file!
cp qt4/qtcurve.moc "$dir/qt4"

# ... but dont want real Qt4 moc file!
if [ -f "$dir/qt4/moc_qtcurve.cpp" ] ; then
    rm "$dir/qt4/moc_qtcurve.cpp"
fi

cp ChangeLog CMakeLists.txt config.h COPYING INSTALL makedist.sh  README TODO "$dir"

tar chvf "$dir".tar "$dir"
gzip "$dir".tar
rm -rf "$dir"
