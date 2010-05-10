#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>

static Atom qtcNetMoveResizeAtom;
static Atom qtcMenuSizeAtom;
static Atom qtcActiveWindowAtom;
static Atom qtcActiveWindowAtom;
static Atom qtcTitleBarSizeAtom;
static Atom qtcToggleMenuBarAtom;
static Atom qtcToggleStatusBarAtom;
static Atom qtcStatusBarAtom;

static void qtcCreateAtoms()
{
    Display *dpy=gdk_x11_get_default_xdisplay();

    qtcNetMoveResizeAtom=XInternAtom(dpy, "_NET_WM_MOVERESIZE", False);
    qtcMenuSizeAtom=XInternAtom(dpy, MENU_SIZE_ATOM, False);
    qtcActiveWindowAtom=XInternAtom(dpy, ACTIVE_WINDOW_ATOM, False);
    qtcTitleBarSizeAtom=XInternAtom(dpy, TITLEBAR_SIZE_ATOM, False);
    qtcToggleMenuBarAtom=XInternAtom(dpy, TOGGLE_MENUBAR_ATOM, False);
    qtcToggleStatusBarAtom=XInternAtom(dpy, TOGGLE_STATUSBAR_ATOM, False);
    qtcStatusBarAtom=XInternAtom(dpy, STATUSBAR_ATOM, False);
}
