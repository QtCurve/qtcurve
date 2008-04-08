#ifndef __QTCURVECONFIG_H__
#define __QTCURVECONFIG_H__

/*
  QtCurve (C) Craig Drummond, 2007 Craig.Drummond@lycos.co.uk

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

#define QTC_COMMON_ONLY_COVERTERS
#define QTC_CONFIG_DIALOG


#include <ui_qtcurveconfigbase.h>
#include <QMap>
#include "common.h"

class QMenu;
class QAction;
class CExportThemeDialog;

class QtCurveConfig : public QWidget, private Ui::QtCurveConfigBase
{
    Q_OBJECT

    public:

    QtCurveConfig(QWidget *parent);
    virtual ~QtCurveConfig();

    signals:

    void changed(bool);

    private:

    void loadStyles(QMenu *menu);

    public slots:

    void save();
    void defaults();

    private slots:

    void setStyle(QAction *s);
    void updateChanged();
    void importStyle();
    void exportStyle();
    void exportTheme();
    void emboldenToggled();
    void defBtnIndicatorChanged();
    void buttonEffectChanged();
    void coloredMouseOverChanged();
    void shadeSlidersChanged();
    void shadeMenubarsChanged();
    void shadeCheckRadioChanged();
    void customMenuTextColorChanged();
    void stripedProgressChanged();
    void tabAppearanceChanged();
    void passwordCharClicked();

    private:

    void setPasswordChar(int ch);
    void loadStyle(const QString &file);
    void setOptions(Options &opts);
    void setWidgetOptions(const Options &opts);
    bool settingsChanged();

    private:

    Options                  currentStyle,
                             defaultStyle;
    QMap<QAction *, QString> styles;
    CExportThemeDialog       *exportDialog;
};

#endif
