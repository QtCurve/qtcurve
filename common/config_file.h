#ifndef QTC_CONFIG_FILE_H
#define QTC_CONFIG_FILE_H

#if !defined QT_VERSION || QT_VERSION >= 0x040000

#define QTC_MENU_FILE_PREFIX   "menubar-"
#define QTC_STATUS_FILE_PREFIX "statusbar-"

#define qtcMenuBarHidden(A)         qtcBarHidden((A), QTC_MENU_FILE_PREFIX)
#define qtcSetMenuBarHidden(A, H)   qtcSetBarHidden((A), (H), QTC_MENU_FILE_PREFIX)
#define qtcStatusBarHidden(A)       qtcBarHidden((A), QTC_STATUS_FILE_PREFIX)
#define qtcSetStatusBarHidden(A, H) qtcSetBarHidden((A), (H), QTC_STATUS_FILE_PREFIX)

extern const char *qtcConfDir();

#ifdef __cplusplus
extern bool qtcBarHidden(const QString &app, const char *prefix);
extern void qtcSetBarHidden(const QString &app, bool hidden, const char *prefix);
extern WindowBorders qtcGetWindowBorderSize(bool force=false);
#else // __cplusplus
extern gboolean qtcBarHidden(const char *app, const char *prefix);
extern void qtcSetBarHidden(const char *app, bool hidden, const char *prefix);
extern WindowBorders qtcGetWindowBorderSize(gboolean force);
#endif // __cplusplus

extern void qtcLoadBgndImage(QtCImage *img);
extern void qtcDefaultSettings(Options *opts);
extern void qtcCheckConfig(Options *opts);
#ifdef __cplusplus
extern bool qtcReadConfig(const QString &file, Options *opts, Options *defOpts=0L, bool checkImages=true);
#else
extern bool qtcReadConfig(const char *file, Options *opts, Options *defOpts);
#endif

#endif // !defined QT_VERSION || QT_VERSION >= 0x040000)

#ifdef CONFIG_WRITE
class KConfig;
extern bool qtcWriteConfig(KConfig *cfg, const Options &opts, const Options &def, bool exportingStyle=false);
#endif

#endif
