#ifndef __QTCURVE_PLUTIN_H__
#define __QTCURVE_PLUTIN_H__

#include <QStylePlugin>

#include "qtcurve.h"

#ifdef QTC_ENABLE_X11
#include <QDBusConnection>
#include <QDBusInterface>
#include <QX11Info>
#include <qtcurve-utils/x11utils.h>
#endif

namespace QtCurve {
class StylePlugin: public QStylePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface"
                      FILE "qtcurvestyle.json")
public:
    StylePlugin(QObject *parent=0): QStylePlugin(parent) {}
    ~StylePlugin() {}
    virtual QStyle *create(const QString &key) override
        {
#ifdef QTC_ENABLE_X11
            qtc_x11_init_xcb(QX11Info::connection(), QX11Info::appScreen());
#endif
            return "qtcurve" == key.toLower() ? new Style : 0;
        }
};
}

#endif
