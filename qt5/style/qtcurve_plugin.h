#ifndef __QTCURVE_PLUTIN_H__
#define __QTCURVE_PLUTIN_H__

#include <QStylePlugin>

#include "qtcurve.h"

namespace QtCurve {
class StylePlugin: public QStylePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface"
                      FILE "qtcurvestyle.json")
public:
    StylePlugin(QObject *parent=0): QStylePlugin(parent) {}
    ~StylePlugin() {}
    virtual QStyle *create(const QString &key) override {
        return "qtcurve" == key.toLower() ? new Style : 0;
    }
};
}

#endif
