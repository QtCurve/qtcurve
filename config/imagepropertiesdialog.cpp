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
#include <klocale.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <QDir>
#include <QGridLayout>
#include <QLabel>


CImagePropertiesDialog::CImagePropertiesDialog(QWidget *parent)
                      : KDialog(parent)
{
    QWidget     *page = new QWidget(this);
//     QGridLayout *layout = new QGridLayout(page);

    setButtons(Ok|Cancel);
    setDefaultButton(Ok);
    setupUi(page);
    setMainWidget(page);
}

void CImagePropertiesDialog::run(const QString &title, const QString &file, int width, int height)
{
    setCaption(i18n("Edit %1", title));
    exec();
}

QSize CImagePropertiesDialog::sizeHint() const
{
    return QSize(400, 120);
}

void CImagePropertiesDialog::slotButtonClicked(int button)
{
    if(Ok==button)
    {
        QDialog::accept();
    }
    else
        QDialog::reject();
}

#include "imagepropertiesdialog.moc"
