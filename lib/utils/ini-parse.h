/*****************************************************************************
 *   Copyright 2012 - 2014 Yichao Yu <yyc1992@gmail.com>                     *
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

#ifndef _QTC_UTILS_INI_PARSE_H_
#define _QTC_UTILS_INI_PARSE_H_

#include "utils.h"
#include "strs.h"
#include "uthash.h"

typedef struct _QtcIniFile QtcIniFile;
typedef struct _QtcIniGroup QtcIniGroup;
typedef struct _QtcIniEntry QtcIniEntry;

typedef struct {
    QtcIniGroup *(*new_group)(void *owner);
    void (*free_group)(void *owner, QtcIniGroup *group);
    QtcIniEntry *(*new_entry)(void *owner);
    void (*free_entry)(void *owner, QtcIniEntry *entry);
} QtcIniVTable;

struct _QtcIniFile {
    QtcIniGroup *first;
    QtcIniGroup *last;

    /* private */
    const QtcIniVTable *vtable;
    QtcIniGroup *groups;
    void *owner;
    void *padding[3];
};

struct _QtcIniGroup {
    QtcIniEntry *first;
    QtcIniEntry *last;
    QtcIniGroup *prev;
    QtcIniGroup *next;
    char *name; /* Read-only */

    /* private */
    const QtcIniVTable *vtable;
    QtcIniEntry *entries;
    UT_hash_handle hh;
    uint32_t flags;
    void *owner;
    int32_t ref_count;
    void *padding[3];
};

struct _QtcIniEntry {
    QtcIniEntry *prev;
    QtcIniEntry *next;
    char *name; /* Read-only */
    char *value;

    /* private */
    const QtcIniVTable *vtable;
    UT_hash_handle hh;
    uint32_t flags;
    void *owner;
    int32_t ref_count;
    void *padding[3];
};

QTC_BEGIN_DECLS

/**
 * Memory management
 **/
bool qtcIniFileInit(QtcIniFile *file, const QtcIniVTable *vtable, void *owner);
void qtcIniFileDone(QtcIniFile *file);
QtcIniGroup *qtcIniGroupRef(QtcIniGroup *group);
void qtcIniGroupUnref(QtcIniGroup *group);
QtcIniEntry *qtcIniEntryRef(QtcIniEntry *entry);
void qtcIniEntryUnref(QtcIniEntry *entry);

/**
 * Read and Write
 **/
bool qtcIniFileLoadFp(QtcIniFile *file, FILE *fp);
bool qtcIniFileLoad(QtcIniFile *file, const char *name);
bool qtcIniFileWriteFp(const QtcIniFile *file, FILE *fp);
bool qtcIniFileWrite(const QtcIniFile *file, const char *name);

/**
 * Operations on QtcIniGroups in a QtcIniFile.
 **/
QtcIniGroup *qtcIniFileFindGroup(const QtcIniFile *file, const char *name,
                                 size_t name_len);
static inline QtcIniGroup*
_qtcIniFileFindGroup(const QtcIniFile *file, const char *name)
{
    return qtcIniFileFindGroup(file, name, strlen(name));
}
#define qtcIniFileFindGroup(file, name, name_len...)                    \
    QTC_SWITCH_(name_len, qtcIniFileFindGroup)(file, name, ##name_len)
QtcIniGroup*
qtcIniFileAddGroup(QtcIniFile *file, QtcIniGroup *base, const char *name,
                   size_t name_len, bool move, bool before);
QTC_ALWAYS_INLINE static inline QtcIniGroup*
_qtcIniFileAddGroup(QtcIniFile *file, QtcIniGroup *base, const char *name,
                    bool move, bool before)
{
    return qtcIniFileAddGroup(file, base, name, strlen(name), move, before);
}
#define qtcIniFileAddGroup(f, b, name, len, move, before...)            \
    QTC_SWITCH_(before, qtcIniFileAddGroup)(f, b, name, len, move, ##before)

bool qtcIniFileDeleteGroup(QtcIniFile *file, QtcIniGroup *group);
bool qtcIniFileInsertGroup(QtcIniFile *file, QtcIniGroup *base,
                           QtcIniGroup *group, bool move, bool before);

/**
 * Operations on QtcIniEntry's in a QtcIniGroup
 **/
QtcIniEntry *qtcIniGroupFindEntry(const QtcIniGroup *group, const char *name,
                                  size_t name_len);
static inline QtcIniEntry*
_qtcIniGroupFindEntry(const QtcIniGroup *group, const char *name)
{
    return qtcIniGroupFindEntry(group, name, strlen(name));
}
#define qtcIniGroupFindEntry(group, name, name_len...)                  \
    QTC_SWITCH_(name_len, qtcIniGroupFindEntry)(group, name, ##name_len)

QtcIniEntry *qtcIniGroupAddEntry(QtcIniGroup *group, QtcIniEntry *base,
                                 const char *name, size_t name_len, bool move,
                                 bool before);
static inline QtcIniEntry*
_qtcIniGroupAddEntry(QtcIniGroup *group, QtcIniEntry *base, const char *name,
                     bool move, bool before)
{
    return qtcIniGroupAddEntry(group, base, name, strlen(name), move, before);
}
#define qtcIniGroupAddEntry(g, b, name, len, move, before...)           \
    QTC_SWITCH_(before, qtcIniGroupAddEntry)(g, b, name, len, move, ##before)

bool qtcIniGroupDeleteEntry(QtcIniGroup *group, QtcIniEntry *entry);
bool qtcIniGroupInsertEntry(QtcIniGroup *group, QtcIniEntry *base,
                            QtcIniEntry *entry, bool move, bool before);

/**
 * Simplified macros
 **/
#define qtcIniFileAddGroupAfter(args...)        \
    qtcIniFileAddGroup(args, false)
#define qtcIniFileAddGroupBefore(args...)       \
    qtcIniFileAddGroup(args, true)
#define qtcIniFileEnsureGroup(file, name...)            \
    qtcIniFileAddGroupAfter(file, NULL, name, false)
#define qtcIniFileMoveGroupAfter(args...)       \
    qtcIniFileAddGroupAfter(args, true)
#define qtcIniFileMoveGroupBefore(args...)      \
    qtcIniFileAddGroupBefore(args, true)
#define qtcIniFileInsertGroupAfter(args...)     \
    qtcIniFileInsertGroup(args, false)
#define qtcIniFileInsertGroupBefore(args...)    \
    qtcIniFileInsertGroup(args, true)

#define qtcIniGroupEnsureEntry(group, name...)          \
    qtcIniGroupAddEntryAfter(group, NULL, name, false)
#define qtcIniGroupMoveEntryAfter(args...)      \
    qtcIniGroupAddEntryAfter(args, true)
#define qtcIniGroupMoveEntryBefore(args...)     \
    qtcIniGroupAddEntryBefore(args, true)
#define qtcIniGroupAddEntryAfter(args...)       \
    qtcIniGroupAddEntry(args, false)
#define qtcIniGroupAddEntryBefore(args...)      \
    qtcIniGroupAddEntry(args, true)
#define qtcIniGroupInsertEntryAfter(args...)    \
    qtcIniGroupInsertEntry(args, false)
#define qtcIniGroupInsertEntryBefore(args...)   \
    qtcIniGroupInsertEntry(args, true)

/**
 * Get/Set value
 **/
#define qtcIniEntrySetValue(entry, value...) do {               \
        QtcIniEntry *_entry = (entry);                          \
        _entry->value = qtcSetStr(_entry->value, value);        \
    } while (0)
static inline const char*
qtcIniGroupGetValue(QtcIniGroup *grp, const char *name, size_t len)
{
    QtcIniEntry *ety = qtcIniGroupFindEntry(grp, name, len);
    return ety ? ety->value : NULL;
}
static inline const char*
_qtcIniGroupGetValue(QtcIniGroup *grp, const char *name)
{
    return qtcIniGroupGetValue(grp, name, strlen(name));
}
#define qtcIniGroupGetValue(grp, name, len...)                  \
    QTC_SWITCH_(len, qtcIniGroupGetValue)(grp, name, ##len)

static inline char*
qtcIniGroupDupValue(QtcIniGroup *grp, const char *name, size_t len)
{
    const char *val = qtcIniGroupGetValue(grp, name, len);
    return val ? strdup(val) : NULL;
}
static inline char*
_qtcIniGroupDupValue(QtcIniGroup *grp, const char *name)
{
    return qtcIniGroupDupValue(grp, name, strlen(name));
}
#define qtcIniGroupDupValue(grp, name, len...)                  \
    QTC_SWITCH_(len, qtcIniGroupDupValue)(grp, name, ##len)

static inline bool
qtcIniGroupGetBool(QtcIniGroup *grp, const char *name, size_t len, bool def)
{
    return qtcStrToBool(qtcIniGroupGetValue(grp, name, len), def);
}
static inline bool
_qtcIniGroupGetBool(QtcIniGroup *grp, const char *name, bool def)
{
    return qtcStrToBool(qtcIniGroupGetValue(grp, name), def);
}
#define qtcIniGroupGetBool(grp, name, len, def...)              \
    QTC_SWITCH_(def, qtcIniGroupGetBool)(grp, name, len, ##def)

static inline long
qtcIniGroupGetInt(QtcIniGroup *grp, const char *name, size_t len, long def)
{
    return qtcStrToInt(qtcIniGroupGetValue(grp, name, len), def);
}
static inline long
_qtcIniGroupGetInt(QtcIniGroup *grp, const char *name, long def)
{
    return qtcIniGroupGetInt(grp, name, strlen(name), def);
}
#define qtcIniGroupGetInt(grp, name, len, def...)               \
    QTC_SWITCH_(def, qtcIniGroupGetInt)(grp, name, len, ##def)

static inline double
qtcIniGroupGetFloat(QtcIniGroup *grp, const char *name, size_t len, double def)
{
    return qtcStrToFloat(qtcIniGroupGetValue(grp, name, len), def);
}
static inline double
_qtcIniGroupGetFloat(QtcIniGroup *grp, const char *name, double def)
{
    return qtcIniGroupGetFloat(grp, name, strlen(name), def);
}
#define qtcIniGroupGetFloat(grp, name, len, def...)                     \
    QTC_SWITCH_(def, qtcIniGroupGetFloat)(grp, name, len, ##def)

QTC_END_DECLS

#endif
