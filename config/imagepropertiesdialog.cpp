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

#include "imagepropertiesdialog.h"
#include <KLocale>
#include <KUrlRequester>
#include <KFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include "common.h"

#define MIN_SIZE 16
#define MAX_SIZE 1024
#define DEF_SIZE 256

static void insertPosEntries(QComboBox *combo)
{
    combo->insertItem(PP_TL, i18n("Top left"));
    combo->insertItem(PP_TR, i18n("Top right"));
    combo->insertItem(PP_BL, i18n("Bottom left"));
    combo->insertItem(PP_BR, i18n("Bottom right"));
    combo->insertItem(PP_CENTRED, i18n("Centred"));
}

CImagePropertiesDialog::CImagePropertiesDialog(const QString &title, QWidget *parent)
                      : KDialog(parent)
{
    QWidget *page = new QWidget(this);

    setButtons(Ok|Cancel);
    setDefaultButton(Ok);
    setupUi(page);
    setMainWidget(page);
    fileRequester->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);
    fileRequester->fileDialog()->setFilter("image/svg+xml image/png image/jpeg image/bmp image/gif image/xpixmap");
    scaleWidth->setRange(MIN_SIZE, MAX_SIZE);
    scaleWidth->setValue(DEF_SIZE);
    scaleHeight->setRange(MIN_SIZE, MAX_SIZE);
    scaleHeight->setValue(DEF_SIZE);
    setCaption(i18n("Edit %1", title));
    connect(scaleImage, SIGNAL(toggled(bool)), scaleWidth, SLOT(setEnabled(bool)));
    connect(scaleImage, SIGNAL(toggled(bool)), scaleHeight, SLOT(setEnabled(bool)));
    insertPosEntries(posCombo);
    posCombo->setCurrentIndex(PP_TR);
}

bool CImagePropertiesDialog::run()
{
    QString oldFile=fileName();
    int     oldWidth=imgWidth(),
            oldHeight=imgHeight(),
            oldPos=imgPos();
    bool    oldOnBorder=onBorder->isChecked();

    if(QDialog::Accepted==exec())
        return true;

    set(oldFile, oldWidth, oldHeight, oldPos, oldOnBorder);
    return false;
}
         
void CImagePropertiesDialog::set(const QString &file, int width, int height, int pos, bool onWindowBorder)
{
    scaleControls->setVisible(-1!=width);
    scaleImage->setVisible(-1!=width);
    onBorder->setVisible(-1!=width);
    onBorderLabel->setVisible(-1!=width);
    posCombo->setVisible(-1!=width);
    posLabel->setVisible(-1!=width);
    if(-1!=width)
    {
        onBorder->setChecked(onWindowBorder);
        scaleImage->setChecked(0!=width || 0!=height);
        scaleWidth->setValue(width<MIN_SIZE || width>MAX_SIZE ? DEF_SIZE : width);
        scaleHeight->setValue(height<MIN_SIZE || height>MAX_SIZE ? DEF_SIZE : height);
        posCombo->setCurrentIndex(pos);
    }
    fileRequester->setUrl(QFile::exists(file) && !QFileInfo(file).isDir() ? KUrl(file) : KUrl());
}

QSize CImagePropertiesDialog::sizeHint() const
{
    return QSize(400, 120);
}

