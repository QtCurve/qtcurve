#ifndef __IMAGE_PROPERTIES_DIALOG_H__
#define __IMAGE_PROPERTIES_DIALOG_H__

/*
  QtCurve (C) Craig Drummond, 2010 craig.p.drummond@gmail.com

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

#include <KDE/KDialog>
#include <KDE/KIntSpinBox>
#include <KDE/KUrlRequester>
#include <KDE/KUrl>
#include <QCheckBox>
#include <QComboBox>

#include "ui_imageproperties.h"

class CImagePropertiesDialog : public KDialog,  public Ui::ImageProperties
{
    public:

    enum
    {
        BASIC  = 0x00,
        POS    = 0x01,
        SCALE  = 0x02,
        BORDER = 0x04
    };

    CImagePropertiesDialog(const QString &title, QWidget *parent, int props);

    bool  run();
    void  set(const QString &file, int width=-1, int height=-1, int pos=1, bool onWindowBorder=false);
    QSize sizeHint() const;

    QString fileName()       { return fileRequester->url().toLocalFile(); }
    int     imgWidth()       { return (properties&SCALE) && scaleImage->isChecked() ? scaleWidth->value() : 0; }
    int     imgHeight()      { return (properties&SCALE) && scaleImage->isChecked() ? scaleHeight->value() : 0; }
    int     imgPos()         { return (properties&POS) ? posCombo->currentIndex() : 0; }
    bool    onWindowBorder() { return (properties&BORDER) && onBorder->isChecked(); }

    private:

    int properties;
};

#endif
