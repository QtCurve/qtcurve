#ifndef QTC_CONFIG_FILE_H
#define QTC_CONFIG_FILE_H

#include "common.h"

#define MAX_CONFIG_FILENAME_LEN   1024
#define MAX_CONFIG_INPUT_LINE_LEN 256
#define QTC_MENU_FILE_PREFIX   "menubar-"
#define QTC_STATUS_FILE_PREFIX "statusbar-"

#define qtcMenuBarHidden(A)         qtcBarHidden((A), QTC_MENU_FILE_PREFIX)
#define qtcSetMenuBarHidden(A, H)   qtcSetBarHidden((A), (H), QTC_MENU_FILE_PREFIX)
#define qtcStatusBarHidden(A)       qtcBarHidden((A), QTC_STATUS_FILE_PREFIX)
#define qtcSetStatusBarHidden(A, H)                     \
    qtcSetBarHidden((A), (H), QTC_STATUS_FILE_PREFIX)

bool qtcBarHidden(const char *app, const char *prefix);
void qtcSetBarHidden(const char *app, bool hidden, const char *prefix);
void qtcLoadBgndImage(QtCImage *img);

void qtcSetRgb(color *col, const char *str);
void qtcDefaultSettings(Options *opts);
void qtcCheckConfig(Options *opts);
bool qtcReadConfig(const char *file, Options *opts, Options *defOpts);
WindowBorders qtcGetWindowBorderSize(bool force);

#endif
