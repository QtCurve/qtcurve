/***************************************************************************
 *   Copyright (C) 2013~2013 by Yichao Yu                                  *
 *   yyc1992@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

// TODO multi screen?

#include "x11utils.h"
#include "x11shadow_p.h"
#include "log.h"

#include <X11/Xlib-xcb.h>

static Display *qtc_disp = NULL;
static xcb_connection_t *qtc_xcb_conn = NULL;
static int qtc_default_screen_no = -1;
static xcb_window_t qtc_root_window = {0};
static xcb_screen_t *qtc_default_screen = NULL;
QTC_EXPORT xcb_atom_t qtc_x11_atoms[_QTC_X11_ATOM_NUMBER];

static char wm_cm_s_atom_name[100] = "_NET_WM_CM_S";

static const char *const qtc_x11_atom_names[_QTC_X11_ATOM_NUMBER] = {
    [QTC_X11_ATOM_NET_WM_MOVERESIZE] = "_NET_WM_MOVERESIZE",
    [QTC_X11_ATOM_NET_WM_CM_S_DEFAULT] = wm_cm_s_atom_name,

    [QTC_X11_ATOM_KDE_NET_WM_SKIP_SHADOW] = "_KDE_NET_WM_SKIP_SHADOW",
    [QTC_X11_ATOM_KDE_NET_WM_FORCE_SHADOW] = "_KDE_NET_WM_FORCE_SHADOW",
    [QTC_X11_ATOM_KDE_NET_WM_SHADOW] = "_KDE_NET_WM_SHADOW",
    [QTC_X11_ATOM_KDE_NET_WM_BLUR_BEHIND_REGION] =
    "_KDE_NET_WM_BLUR_BEHIND_REGION",

    [QTC_X11_ATOM_QTC_MENUBAR_SIZE] = "_QTCURVE_MENUBAR_SIZE_",
    [QTC_X11_ATOM_QTC_STATUSBAR] = "_QTCURVE_STATUSBAR_",
    [QTC_X11_ATOM_QTC_TITLEBAR_SIZE] = "_QTCURVE_TITLEBAR_SIZE_",
    [QTC_X11_ATOM_QTC_ACTIVE_WINDOW] = "_QTCURVE_ACTIVE_WINDOW_",
    [QTC_X11_ATOM_QTC_TOGGLE_MENUBAR] = "_QTCURVE_TOGGLE_MENUBAR_",
    [QTC_X11_ATOM_QTC_TOGGLE_STATUSBAR] = "_QTCURVE_TOGGLE_STATUSBAR_",
    [QTC_X11_ATOM_QTC_OPACITY] = "_QTCURVE_OPACITY_",
    [QTC_X11_ATOM_QTC_BGND] = "_QTCURVE_BGND_",

    [QTC_X11_ATOM_XEMBED_INFO] = "_XEMBED_INFO",
};

QTC_EXPORT xcb_window_t
(qtcX11RootWindow)(int scrn_no)
{
    if (scrn_no < 0 || scrn_no == qtc_default_screen_no)
        return qtc_root_window;
    return qtcX11GetScreen()->root;
}

QTC_EXPORT void
qtcX11FlushXlib()
{
    if (qtc_disp) {
        XFlush(qtc_disp);
    }
}

QTC_EXPORT int
qtcX11DefaultScreenNo()
{
    return qtc_default_screen_no;
}

QTC_EXPORT xcb_screen_t*
qtcX11DefaultScreen()
{
    return qtc_default_screen;
}

static xcb_screen_t*
screen_of_display(xcb_connection_t *c, int screen)
{
    xcb_screen_iterator_t iter;

    iter = xcb_setup_roots_iterator(xcb_get_setup(c));
    for (;iter.rem;--screen, xcb_screen_next(&iter)) {
        if (screen == 0) {
            return iter.data;
        }
    }
    return NULL;
}

QTC_EXPORT xcb_screen_t*
(qtcX11GetScreen)(int screen_no)
{
    if (screen_no == -1 || screen_no == qtc_default_screen_no) {
        return qtc_default_screen;
    }
    if (qtcUnlikely(!qtc_xcb_conn)) {
        return NULL;
    }
    return screen_of_display(qtc_xcb_conn, screen_no);
}

QTC_EXPORT void
qtcX11InitXcb(xcb_connection_t *conn, int screen_no)
{
    if (qtcUnlikely(qtc_xcb_conn) || !conn) {
        return;
    }
    if (screen_no < 0) {
        screen_no = 0;
    }
    qtc_xcb_conn = conn;
    qtc_default_screen_no = screen_no;
    qtc_default_screen = screen_of_display(conn, screen_no);
    if (qtc_default_screen) {
        qtc_root_window = qtc_default_screen->root;
    }
    const size_t base_len = strlen("_NET_WM_CM_S");
    sprintf(wm_cm_s_atom_name + base_len, "%d", screen_no);
    qtcX11GetAtoms(_QTC_X11_ATOM_NUMBER, qtc_x11_atoms,
                   qtc_x11_atom_names, true);
    qtcX11ShadowInit();
}

QTC_EXPORT void
qtcX11InitXlib(Display *disp)
{
    if (qtcUnlikely(qtc_xcb_conn) || !disp) {
        return;
    }
    qtc_disp = disp;
    qtcX11InitXcb(XGetXCBConnection(disp), DefaultScreen(disp));
}

QTC_EXPORT xcb_connection_t*
qtcX11GetConn()
{
    return qtc_xcb_conn;
}

QTC_EXPORT Display*
qtcX11GetDisp()
{
    return qtc_disp;
}

QTC_EXPORT void
qtcX11Flush()
{
    if (qtcLikely(qtc_xcb_conn)) {
        xcb_flush(qtc_xcb_conn);
    }
}

QTC_EXPORT uint32_t
qtcX11GenerateId()
{
    if (qtcLikely(qtc_xcb_conn)) {
        return xcb_generate_id(qtc_xcb_conn);
    }
    return 0;
}

QTC_EXPORT void
qtcX11GetAtoms(size_t n, xcb_atom_t *atoms, const char *const names[],
               bool create)
{
    xcb_connection_t *conn = qtc_xcb_conn;
    memset(atoms, 0, sizeof(xcb_atom_t) * n);
    if (qtcUnlikely(!conn)) {
        return;
    }
    xcb_intern_atom_cookie_t cookies[n];
    for (size_t i = 0;i < n;i++) {
        cookies[i] = xcb_intern_atom(conn, !create,
                                     strlen(names[i]), names[i]);
    }
    for (size_t i = 0;i < n;i++) {
        xcb_intern_atom_reply_t *r =
            xcb_intern_atom_reply(conn, cookies[i], 0);
        if (r) {
            atoms[i] = r->atom;
            free(r);
        }
    }
}

QTC_EXPORT int32_t
qtcX11GetShortProp(xcb_window_t win, xcb_atom_t atom)
{
    if (qtcUnlikely(!win))
        return -1;
    int32_t res = -1;
    xcb_get_property_reply_t *reply =
        qtcX11Call(get_property, 0, win, atom, XCB_ATOM_CARDINAL, 0, 1);
    if (!reply) {
        return -1;
    }
    if (xcb_get_property_value_length(reply) > 0) {
        uint32_t val = *(int32_t*)xcb_get_property_value(reply);
        if (val < 512) {
            res = val;
        }
    }
    free(reply);
    return res;
}

QTC_EXPORT void
qtcX11MapRaised(xcb_window_t win)
{
    if (qtcUnlikely(!win))
        return;
    static const uint32_t val = XCB_STACK_MODE_ABOVE;
    qtcX11CallVoid(configure_window, win, XCB_CONFIG_WINDOW_STACK_MODE, &val);
    qtcX11CallVoid(map_window, win);
}

QTC_EXPORT bool
qtcX11CompositingActive()
{
    xcb_get_selection_owner_reply_t *reply =
        qtcX11Call(get_selection_owner,
                   qtc_x11_atoms[QTC_X11_ATOM_NET_WM_CM_S_DEFAULT]);
    if (!reply) {
        return false;
    }
    bool res = (reply->owner != 0);
    free(reply);
    return res;
}

QTC_EXPORT bool
qtcX11HasAlpha(xcb_window_t win)
{
    if (qtcUnlikely(!win))
        return false;
    if (!qtcX11CompositingActive()) {
        return false;
    }
    if (!win) {
        return true;
    }
    xcb_get_geometry_reply_t *reply = qtcX11Call(get_geometry, win);
    if (!reply) {
        return false;
    }
    bool res = (reply->depth == 32);
    free(reply);
    return res;
}

QTC_EXPORT bool
qtcX11IsEmbed(xcb_window_t win)
{
    if (qtcUnlikely(!win))
        return false;
    xcb_atom_t xembed_atom = qtc_x11_atoms[QTC_X11_ATOM_XEMBED_INFO];
    xcb_get_property_reply_t *reply =
        qtcX11Call(get_property, 0, win, xembed_atom,
                   xembed_atom, 0, 1);
    if (!reply) {
        return false;
    }
    bool res = xcb_get_property_value_length(reply) > 0;
    free(reply);
    return res;
}
