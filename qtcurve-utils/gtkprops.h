/*****************************************************************************
 *   Copyright 2014 - 2014 Yichao Yu <yyc1992@gmail.com>                     *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU Lesser General Public License as          *
 *   published by the Free Software Foundation; either version 2.1 of the    *
 *   License, or (at your option) version 3, or any later version accepted   *
 *   by the membership of KDE e.V. (or its successor approved by the         *
 *   membership of KDE e.V.), which shall act as a proxy defined in          *
 *   Section 6 of version 3 of the license.                                  *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 *   Lesser General Public License for more details.                         *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with this library. If not,                                *
 *   see <http://www.gnu.org/licenses/>.                                     *
 *****************************************************************************/

#ifndef __QTC_UTILS_GTK_PROPS_H__
#define __QTC_UTILS_GTK_PROPS_H__

#include "gtkutils.h"
#include "log.h"

typedef struct {
    GtkWidget *w;

    int blurBehind: 2;
    bool entryHacked: 1;
    bool comboBoxHacked: 1;
    bool menuShellHacked: 1;

    int entryEnter;
    int entryLeave;
    int entryDestroy;
    int entryUnrealize;
    int entryStyleSet;

    int comboBoxDestroy;
    int comboBoxUnrealize;
    int comboBoxStyleSet;
    int comboBoxEnter;
    int comboBoxLeave;
    int comboBoxStateChange;

    unsigned menuBarSize;
    int menuShellMotion;
    int menuShellLeave;
    int menuShellDestroy;
    int menuShellStyleSet;
    int menuShellButtonPress;
    int menuShellButtonRelease;
} _QtcGtkWidgetProps;

#define QTC_GTK_PROP_NAME "_gtk__QTCURVE_WIDGET_PROPERTIES__"

QTC_ALWAYS_INLINE static inline GQuark
_qtcWidgetPropName()
{
    static GQuark quark = 0;
    if (qtcUnlikely(!quark)) {
        quark = g_quark_from_static_string(QTC_GTK_PROP_NAME);
    }
    return quark;
}

QTC_ALWAYS_INLINE static inline _QtcGtkWidgetProps*
qtcWidgetPropsNew(GtkWidget *w)
{
    QTC_UNUSED(w);
    _QtcGtkWidgetProps *props = qtcNew(_QtcGtkWidgetProps);
    props->w = w;
    return props;
}

QTC_ALWAYS_INLINE static inline void
qtcWidgetPropsFree(_QtcGtkWidgetProps *props)
{
    free(props);
}

static void
qtcWidgetPropsDestroy(void *data)
{
    return qtcWidgetPropsFree((_QtcGtkWidgetProps*)data);
}

QTC_ALWAYS_INLINE static inline _QtcGtkWidgetProps*
_qtcGetWidgetProps(GObject *obj)
{
    _QtcGtkWidgetProps *props = g_object_get_qdata(obj, _qtcWidgetPropName());
    if (!props) {
        props = qtcWidgetPropsNew((GtkWidget*)obj);
        g_object_set_qdata_full(obj, _qtcWidgetPropName(), props,
                                qtcWidgetPropsDestroy);
    }
    return props;
}

typedef struct {
    GObject *obj;
    _QtcGtkWidgetProps *props;
} QtcGtkWidgetProps;

static inline _QtcGtkWidgetProps*
qtcGetWidgetProps(QtcGtkWidgetProps *props)
{
    if (!props->props && props->obj) {
        props->props = _qtcGetWidgetProps(props->obj);
    }
    return props->props;
}

#define QTC_DEF_WIDGET_PROPS(name, widget)                      \
    QtcGtkWidgetProps __qtc_gtk_widget_props_##name = {         \
        (GObject*)(widget), NULL                                \
    };                                                          \
    QtcGtkWidgetProps *name = &__qtc_gtk_widget_props_##name

static inline void
qtcConnectToProp(GObject *obj, int *prop, const char *sig_name,
                 GCallback cb, void *data)
{
    *prop = g_signal_connect(obj, sig_name, cb, data);
}

#define qtcConnectToProp(props, field, sig_name, cb, data)              \
    qtcConnectToProp(props->obj, &qtcGetWidgetProps(props)->field,      \
                     sig_name, G_CALLBACK(cb), data)

#define qtcDisconnectFromProp(props, field) do {                        \
        _QtcGtkWidgetProps *_props = qtcGetWidgetProps(props);          \
        if (qtcLikely(_props->field)) {                                 \
            g_signal_handler_disconnect(props->obj, _props->field);     \
            _props->field = 0;                                          \
        }                                                               \
    } while (0)

#endif
