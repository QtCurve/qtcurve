#ifndef __QTCURVE_PLUTIN_H__
#define __QTCURVE_PLUTIN_H__

#include <QStylePlugin>

#include "qtcurve.h"

#ifdef QTC_ENABLE_X11
#  include <QApplication>
#  include <QX11Info>
#  include <qtcurve-utils/x11utils.h>
#endif

namespace QtCurve {
class StylePlugin: public QStylePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QStyleFactoryInterface_iid FILE "qtcurvestyle.json")
public:
    virtual QStyle*
    create(const QString &key) override
    {
#ifdef QTC_ENABLE_X11
        if (qApp->platformName() == "xcb")
            qtcX11InitXcb(QX11Info::connection(), QX11Info::appScreen());
#endif
        return "qtcurve" == key.toLower() ? new Style : 0;
    }
};
}

#endif
