/*****************************************************************************
 *   Copyright 2012 - 2013 Yichao Yu <yyc1992@gmail.com>                     *
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
QtcIniGroup *qtcIniFileFindGroupWithLen(const QtcIniFile *file,
                                        const char *name, size_t name_len);
QtcIniGroup *qtcIniFileAddGroupAfterWithLen(
    QtcIniFile *file, QtcIniGroup *base, const char *name, size_t name_len,
    bool move);
QtcIniGroup *qtcIniFileAddGroupBeforeWithLen(
    QtcIniFile *file, QtcIniGroup *base, const char *name, size_t name_len,
    bool move);
bool qtcIniFileDeleteGroup(QtcIniFile *file, QtcIniGroup *group);
bool qtcIniFileInsertGroupAfter(QtcIniFile *file, QtcIniGroup *base,
                                QtcIniGroup *group, bool move);
bool qtcIniFileInsertGroupBefore(QtcIniFile *file, QtcIniGroup *base,
                                 QtcIniGroup *group, bool move);

/**
 * Operations on QtcIniEntry's in a QtcIniGroup
 **/
QtcIniEntry *qtcIniGroupFindEntryWithLen(const QtcIniGroup *group,
                                         const char *name, size_t name_len);
QtcIniEntry *qtcIniGroupAddEntryAfterWithLen(
    QtcIniGroup *group, QtcIniEntry *base, const char *name, size_t name_len,
    bool move);
QtcIniEntry *qtcIniGroupAddEntryBeforeWithLen(
    QtcIniGroup *group, QtcIniEntry *base, const char *name, size_t name_len,
    bool move);
bool qtcIniGroupDeleteEntry(QtcIniGroup *group, QtcIniEntry *entry);
bool qtcIniGroupInsertEntryAfter(QtcIniGroup *group, QtcIniEntry *base,
                                 QtcIniEntry *entry, bool move);
bool qtcIniGroupInsertEntryBefore(QtcIniGroup *group, QtcIniEntry *base,
                                  QtcIniEntry *entry, bool move);

/**
 * Simplified inline functions
 **/
static inline QtcIniGroup*
qtcIniFileFindGroup(const QtcIniFile *file, const char *name)
{
    return qtcIniFileFindGroupWithLen(file, name, strlen(name));
}
static inline QtcIniEntry*
qtcIniGroupFindEntry(const QtcIniGroup *group, const char *name)
{
    return qtcIniGroupFindEntryWithLen(group, name, strlen(name));
}
static inline QtcIniGroup*
qtcIniFileEnsureGroupWithLen(QtcIniFile *file, const char *name,
                             size_t name_len)
{
    return qtcIniFileAddGroupAfterWithLen(file, NULL, name, name_len, false);
}
static inline QtcIniGroup*
qtcIniFileEnsureGroup(QtcIniFile *file, const char *name)
{
    return qtcIniFileEnsureGroupWithLen(file, name, strlen(name));
}
static inline QtcIniGroup*
qtcIniFileMoveGroupAfterWithLen(QtcIniFile *file, QtcIniGroup *base,
                                const char *name, size_t name_len)
{
    return qtcIniFileAddGroupAfterWithLen(file, base, name, name_len, true);
}
static inline QtcIniGroup*
qtcIniFileMoveGroupAfter(QtcIniFile *file, QtcIniGroup *base, const char *name)
{
    return qtcIniFileMoveGroupAfterWithLen(file, base, name, strlen(name));
}
static inline QtcIniGroup*
qtcIniFileAddGroupAfter(QtcIniFile *file, QtcIniGroup *base, const char *name,
                        bool move)
{
    return qtcIniFileAddGroupAfterWithLen(file, base, name, strlen(name), move);
}
static inline QtcIniGroup*
qtcIniFileMoveGroupBeforeWithLen(QtcIniFile *file, QtcIniGroup *base,
                                 const char *name, size_t name_len)
{
    return qtcIniFileAddGroupBeforeWithLen(file, base, name, name_len, true);
}
static inline QtcIniGroup*
qtcIniFileMoveGroupBefore(QtcIniFile *file, QtcIniGroup *base, const char *name)
{
    return qtcIniFileMoveGroupBeforeWithLen(file, base, name, strlen(name));
}
static inline QtcIniGroup*
qtcIniFileAddGroupBefore(QtcIniFile *file, QtcIniGroup *base,
                         const char *name, bool move)
{
    return qtcIniFileAddGroupBeforeWithLen(file, base, name,
                                           strlen(name), move);
}
static inline QtcIniEntry*
qtcIniGroupEnsureEntryWithLen(QtcIniGroup *group, const char *name,
                              size_t name_len)
{
    return qtcIniGroupAddEntryAfterWithLen(group, NULL, name, name_len, false);
}
static inline QtcIniEntry*
qtcIniGroupEnsureEntry(QtcIniGroup *group, const char *name)
{
    return qtcIniGroupEnsureEntryWithLen(group, name, strlen(name));
}
static inline QtcIniEntry*
qtcIniGroupMoveEntryAfterWithLen(QtcIniGroup *group, QtcIniEntry *base,
                                 const char *name, size_t name_len)
{
    return qtcIniGroupAddEntryAfterWithLen(group, base, name, name_len, true);
}
static inline QtcIniEntry*
qtcIniGroupMoveEntryAfter(QtcIniGroup *group, QtcIniEntry *base,
                          const char *name)
{
    return qtcIniGroupMoveEntryAfterWithLen(group, base, name, strlen(name));
}
static inline QtcIniEntry*
qtcIniGroupAddEntryAfter(QtcIniGroup *group, QtcIniEntry *base,
                         const char *name, bool move)
{
    return qtcIniGroupAddEntryAfterWithLen(group, base, name,
                                           strlen(name), move);
}
static inline QtcIniEntry*
qtcIniGroupMoveEntryBeforeWithLen(QtcIniGroup *group, QtcIniEntry *base,
                                  const char *name, size_t name_len)
{
    return qtcIniGroupAddEntryBeforeWithLen(group, base, name, name_len, true);
}
static inline QtcIniEntry*
qtcIniGroupMoveEntryBefore(QtcIniGroup *group, QtcIniEntry *base,
                           const char *name)
{
    return qtcIniGroupMoveEntryBeforeWithLen(group, base, name, strlen(name));
}
static inline QtcIniEntry*
qtcIniGroupAddEntryBefore(QtcIniGroup *group, QtcIniEntry *base,
                          const char *name, bool move)
{
    return qtcIniGroupAddEntryBeforeWithLen(group, base, name,
                                            strlen(name), move);
}

static inline QtcIniEntry*
qtcIniEntrySetValueWithLen(QtcIniEntry *entry, const char *value, size_t len)
{
    entry->value = qtcSetStr(entry->value, value, len);
    return entry;
}
static inline QtcIniEntry*
qtcIniEntrySetValue(QtcIniEntry *entry, const char *value)
{
    entry->value = qtcSetStr(entry->value, value);
    return entry;
}

static inline const char*
qtcIniGroupGetValueWithLen(QtcIniGroup *grp, const char *name, size_t len)
{
    QtcIniEntry *ety = qtcIniGroupFindEntryWithLen(grp, name, len);
    return ety ? ety->value : NULL;
}
static inline const char*
qtcIniGroupGetValue(QtcIniGroup *grp, const char *name)
{
    return qtcIniGroupGetValueWithLen(grp, name, strlen(name));
}

static inline char*
qtcIniGroupDupValueWithLen(QtcIniGroup *grp, const char *name, size_t len)
{
    const char *val = qtcIniGroupGetValueWithLen(grp, name, len);
    return val ? strdup(val) : NULL;
}
static inline char*
qtcIniGroupDupValue(QtcIniGroup *grp, const char *name)
{
    return qtcIniGroupDupValueWithLen(grp, name, strlen(name));
}

static inline bool
qtcIniGroupGetBoolWithLen(QtcIniGroup *grp, const char *name,
                          size_t len, bool def)
{
    return qtcStrToBool(qtcIniGroupGetValueWithLen(grp, name, len), def);
}
static inline bool
qtcIniGroupGetBool(QtcIniGroup *grp, const char *name, bool def)
{
    return qtcStrToBool(qtcIniGroupGetValue(grp, name), def);
}

QTC_END_DECLS

#endif
