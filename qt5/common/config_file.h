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
#define qtcSetStatusBarHidden(A, H) qtcSetBarHidden((A), (H), QTC_STATUS_FILE_PREFIX)

#ifdef __cplusplus
bool qtcBarHidden(const QString &app, const char *prefix);
void qtcSetBarHidden(const QString &app, bool hidden, const char *prefix);
#else // __cplusplus
gboolean qtcBarHidden(const char *app, const char *prefix);
void qtcSetBarHidden(const char *app, bool hidden, const char *prefix);
#endif // __cplusplus

void qtcLoadBgndImage(QtCImage *img);

void qtcSetRgb(color *col, const char *str);
void qtcDefaultSettings(Options *opts);
void qtcCheckConfig(Options *opts);
#ifdef __cplusplus
bool qtcReadConfig(const QString &file, Options *opts, Options *defOpts=0L,
                   bool checkImages=true);
#else
bool qtcReadConfig(const char *file, Options *opts, Options *defOpts);
#endif
WindowBorders qtcGetWindowBorderSize(bool force);

#ifdef CONFIG_WRITE
class KConfig;
bool qtcWriteConfig(KConfig *cfg, const Options &opts, const Options &def,
                    bool exportingStyle=false);
#endif

#endif
