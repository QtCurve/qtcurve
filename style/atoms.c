#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>

static Atom qtcNetMoveResize;
static Atom qtcMenuSize;
static Atom qtcActiveWindow;

static void qtcCreateAtoms()
{
     Display *dpy=gdk_x11_get_default_xdisplay();

     qtcNetMoveResize=XInternAtom(dpy, "_NET_WM_MOVERESIZE", False);
     qtcMenuSize=XInternAtom(dpy, MENU_SIZE_ATOM, False);
     qtcActiveWindow=XInternAtom(dpy, ACTIVE_WINDOW_ATOM, False);
}
