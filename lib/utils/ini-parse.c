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

#include "ini-parse.h"
#include "log.h"
#include "atomic.h"

// TODO
#include <libintl.h>
#define _(x) gettext(x)

#define blank_chars " \t\b\n\f\v\r"

enum {
    QTC_INI_GROUP_UPDATED = 1 << 0,
};

enum {
    QTC_INI_ENTRY_UPDATED = 1 << 0,
};

/**
 * Create a new group with vtable that doesn't belongs to any file.
 **/
static QtcIniGroup*
qtcIniParseNewGroup(const QtcIniVTable *vtable, void *owner)
{
    QtcIniGroup *group;
    if (vtable && vtable->new_group) {
        group = vtable->new_group(owner);
        memset(group, 0, sizeof(QtcIniGroup));
    } else {
        group = qtcNew(QtcIniGroup);
    }
    group->vtable = vtable;
    group->owner = owner;
    group->ref_count = 1;
    return group;
}

/**
 * Create a new entry with vtable that doesn't belongs to any group
 **/
static QtcIniEntry*
qtcIniParseNewEntry(const QtcIniVTable *vtable, void *owner)
{
    QtcIniEntry *entry;
    if (vtable && vtable->new_entry) {
        entry = vtable->new_entry(owner);
        memset(entry, 0, sizeof(QtcIniEntry));
    } else {
        entry = qtcNew(QtcIniEntry);
    }
    entry->vtable = vtable;
    entry->owner = owner;
    entry->ref_count = 1;
    return entry;
}

/**
 * Init a file
 **/
QTC_EXPORT bool
qtcIniFileInit(QtcIniFile *file, const QtcIniVTable *vtable, void *owner)
{
    memset(file, 0, sizeof(QtcIniFile));
    file->vtable = vtable;
    file->owner = owner;
    return true;
}

static char*
_findNoneBlank(char *str)
{
    return str + strspn(str, blank_chars);
}

/**
 * Reset Functions:
 *     Clear flags and link. Ready to be reused.
 **/
static void
qtcIniEntryReset(QtcIniEntry *entry)
{
    entry->flags = 0;
    entry->prev = NULL;
    entry->next = NULL;
}

static void
qtcIniGroupReset(QtcIniGroup *group)
{
    QtcIniEntry *entry;
    for (entry = group->first;entry;entry = entry->next)
        qtcIniEntryReset(entry);
    group->flags = 0;
    group->prev = NULL;
    group->next = NULL;
    group->first = NULL;
    group->last = NULL;
}

static void
qtcIniFileReset(QtcIniFile *file)
{
    QtcIniGroup *group;
    for (group = file->first;group;group = group->next)
        qtcIniGroupReset(group);
    file->first = NULL;
    file->last = NULL;
}

static size_t
qtcIniGroupNameLen(const char *str)
{
    size_t res = strcspn(str, "[]");
    if (str[res] == ']')
        return res;
    return 0;
}

static size_t
qtcIniEntryKeyLen(const char *str)
{
    size_t res = strcspn(str, "=");
    if (str[res] != '=')
        return 0;
    for (;res > 0;res--) {
        if (strchr(blank_chars, str[res - 1]))
            continue;
        break;
    }
    return res;
}

/**
 * Free a entry that is already unlinked and removed from the hash table.
 **/
static void
qtcIniEntryFree(QtcIniEntry *entry)
{
    free(entry->name);
    qtcFree(entry->value);
    if (entry->vtable && entry->vtable->free_entry) {
        entry->vtable->free_entry(entry->owner, entry);
    } else {
        free(entry);
    }
}

QTC_EXPORT void
qtcIniEntryUnref(QtcIniEntry *entry)
{
    if (qtcAtomicAdd(&entry->ref_count, -1) <= 1) {
        qtcIniEntryFree(entry);
    }
}

QTC_EXPORT QtcIniEntry*
qtcIniEntryRef(QtcIniEntry *entry)
{
    qtcAtomicAdd(&entry->ref_count, 1);
    return entry;
}

/**
 * Remove a entry from a group and decrease the ref count by 1.
 **/
static void
qtcIniGroupHashRemoveEntry(QtcIniGroup *group, QtcIniEntry *entry)
{
    HASH_DEL(group->entries, entry);
    entry->prev = NULL;
    entry->next = NULL;
    entry->hh.tbl = NULL;
    qtcIniEntryUnref(entry);
}

/**
 * Remove all its entries from group and free it. group should be already
 * unlinked and removed from the hash table.
 **/
static void
qtcIniGroupFree(QtcIniGroup *group)
{
    QtcIniEntry *entry;
    QtcIniEntry *next;
    for (entry = group->entries;entry;entry = next) {
        next = entry->hh.next;
        qtcIniGroupHashRemoveEntry(group, entry);
    }
    free(group->name);
    if (group->vtable && group->vtable->free_group) {
        group->vtable->free_group(entry->owner, group);
    } else {
        free(group);
    }
}

QTC_EXPORT void
qtcIniGroupUnref(QtcIniGroup *group)
{
    if (qtcAtomicAdd(&group->ref_count, -1) <= 1) {
        qtcIniGroupFree(group);
    }
}

QTC_EXPORT QtcIniGroup*
qtcIniGroupRef(QtcIniGroup *group)
{
    qtcAtomicAdd(&group->ref_count, 1);
    return group;
}

/**
 * Remove a group from a file and decrease the ref count by 1.
 **/
static void
qtcIniFileHashRemoveGroup(QtcIniFile *file,
                                     QtcIniGroup *group)
{
    HASH_DEL(file->groups, group);
    group->prev = NULL;
    group->next = NULL;
    group->hh.tbl = NULL;
    qtcIniGroupUnref(group);
}

/**
 * Remove all groups from the file
 **/
QTC_EXPORT void
qtcIniFileDone(QtcIniFile *file)
{
    QtcIniGroup *group;
    QtcIniGroup *next;
    for (group = file->groups;group;group = next) {
        next = group->hh.next;
        qtcIniFileHashRemoveGroup(file, group);
    }
}

/**
 * Check and clear UPDATED flags, remove unused entries.
 **/
static void
qtcIniGroupClean(QtcIniGroup *group)
{
    QtcIniEntry *entry;
    QtcIniEntry *next;
    for (entry = group->entries;entry;entry = next) {
        next = entry->hh.next;
        if (entry->flags & QTC_INI_ENTRY_UPDATED) {
            entry->flags &= ~(QTC_INI_ENTRY_UPDATED);
        } else {
            qtcIniGroupHashRemoveEntry(group, entry);
        }
    }
}

/**
 * Check and clear UPDATED flags, remove unused groups.
 **/
static void
qtcIniFileClean(QtcIniFile *file)
{
    QtcIniGroup *group;
    QtcIniGroup *next;
    for (group = file->groups;group;group = next) {
        next = group->hh.next;
        if (group->flags & QTC_INI_GROUP_UPDATED) {
            group->flags &= ~(QTC_INI_GROUP_UPDATED);
            qtcIniGroupClean(group);
        } else {
            qtcIniFileHashRemoveGroup(file, group);
        }
    }
}

QTC_EXPORT QtcIniGroup*
qtcIniFileFindGroupWithLen(const QtcIniFile *file, const char *name,
                           size_t name_len)
{
    QtcIniGroup *group = NULL;
    HASH_FIND(hh, file->groups, name, name_len, group);
    return group;
}

static inline void
qtcIniFileHashAdd(QtcIniFile *file, QtcIniGroup *group, size_t name_len)
{
    HASH_ADD_KEYPTR(hh, file->groups, group->name, name_len, group);
}

/**
 * Create a new group and add it to the hash table of file.
 **/
static QtcIniGroup*
qtcIniFileHashNewGroup(QtcIniFile *file, const char *name, size_t name_len)
{
    QtcIniGroup *new_group;
    new_group = qtcIniParseNewGroup(file->vtable, file->owner);
    new_group->name = malloc(name_len + 1);
    memcpy(new_group->name, name, name_len);
    new_group->name[name_len] = '\0';
    qtcIniFileHashAdd(file, new_group, name_len);
    return new_group;
}

QTC_EXPORT QtcIniEntry*
qtcIniGroupFindEntryWithLen(const QtcIniGroup *group, const char *name,
                            size_t name_len)
{
    QtcIniEntry *entry = NULL;
    HASH_FIND(hh, group->entries, name, name_len, entry);
    return entry;
}

static inline void
qtcIniGroupHashAdd(QtcIniGroup *group, QtcIniEntry *entry, size_t name_len)
{
    HASH_ADD_KEYPTR(hh, group->entries, entry->name, name_len, entry);
}

/**
 * Create a new entry and add it to the hash table of group.
 **/
static QtcIniEntry*
qtcIniGroupHashNewEntry(QtcIniGroup *group, const char *name, size_t name_len)
{
    QtcIniEntry *new_entry;
    new_entry = qtcIniParseNewEntry(group->vtable, group->owner);
    new_entry->name = malloc(name_len + 1);
    memcpy(new_entry->name, name, name_len);
    new_entry->name[name_len] = '\0';
    qtcIniGroupHashAdd(group, new_entry, name_len);
    return new_entry;
}

static void
qtcIniGroupSetLink(QtcIniGroup *new_group, QtcIniGroup **prev_p,
                   QtcIniGroup **next_p)
{
    new_group->next = *prev_p;
    new_group->prev = *next_p;
    *prev_p = new_group;
    *next_p = new_group;
}

static void
qtcIniGroupUnlink(QtcIniFile *file, QtcIniGroup *group)
{
    if (group->prev) {
        group->prev->next = group->next;
    } else {
        file->first = group->next;
    }
    if (group->next) {
        group->next->prev = group->prev;
    } else {
        file->last = group->prev;
    }
}

static void
qtcIniFileLinkGroupAfter(QtcIniFile *file, QtcIniGroup *prev_group,
                         QtcIniGroup *new_group)
{
    QtcIniGroup **prev_p;
    if (!prev_group) {
        prev_p = &file->first;
    } else {
        prev_p = &prev_group->next;
    }
    qtcIniGroupSetLink(new_group, prev_p, &file->last);
}

static void
qtcIniFileLinkGroupBefore(QtcIniFile *file, QtcIniGroup *next_group,
                          QtcIniGroup *new_group)
{
    QtcIniGroup **next_p;
    if (!next_group) {
        next_p = &file->last;
    } else {
        next_p = &next_group->prev;
    }
    qtcIniGroupSetLink(new_group, &file->first, next_p);
}

static inline void
qtcIniFileLinkGroup(QtcIniFile *file, QtcIniGroup *base, QtcIniGroup *group,
                    bool before)
{
    if (before) {
        qtcIniFileLinkGroupBefore(file, base, group);
    } else {
        qtcIniFileLinkGroupAfter(file, base, group);
    }
}

static void
qtcIniEntrySetLink(QtcIniEntry *new_entry, QtcIniEntry **prev_p,
                   QtcIniEntry **next_p)
{
    new_entry->next = *prev_p;
    new_entry->prev = *next_p;
    *prev_p = new_entry;
    *next_p = new_entry;
}

static void
qtcIniEntryUnlink(QtcIniGroup *group, QtcIniEntry *entry)
{
    if (entry->prev) {
        entry->prev->next = entry->next;
    } else {
        group->first = entry->next;
    }
    if (entry->next) {
        entry->next->prev = entry->prev;
    } else {
        group->last = entry->prev;
    }
}

static void
qtcIniGroupLinkEntryAfter(QtcIniGroup *group, QtcIniEntry *prev_entry,
                          QtcIniEntry *new_entry)
{
    QtcIniEntry **prev_p;
    if (!prev_entry) {
        prev_p = &group->first;
    } else {
        prev_p = &prev_entry->next;
    }
    qtcIniEntrySetLink(new_entry, prev_p, &group->last);
}

static void
qtcIniGroupLinkEntryBefore(QtcIniGroup *group, QtcIniEntry *next_entry,
                           QtcIniEntry *new_entry)
{
    QtcIniEntry **next_p;
    if (!next_entry) {
        next_p = &group->last;
    } else {
        next_p = &next_entry->prev;
    }
    qtcIniEntrySetLink(new_entry, &group->first, next_p);
}

static inline void
qtcIniGroupLinkEntry(QtcIniGroup *group, QtcIniEntry *base,
                     QtcIniEntry *entry, bool before)
{
    if (before) {
        qtcIniGroupLinkEntryBefore(group, base, entry);
    } else {
        qtcIniGroupLinkEntryAfter(group, base, entry);
    }
}

QTC_EXPORT bool
qtcIniFileLoadFp(QtcIniFile *file, FILE *fp)
{
    if (!(file && fp))
        return false;
    char *buff = NULL;
    size_t buff_len = 0;
    int lineno = 0;
    qtcIniFileReset(file);
    QtcIniGroup *cur_group = NULL;

    while (getline(&buff, &buff_len, fp) != -1) {
        size_t line_len = strcspn(buff, "\n");
        buff[line_len] = '\0';
        lineno++;
        char *line = _findNoneBlank(buff);
        switch (*line) {
        case '#':
            line++;
        case '\0':
            continue;
        case '[': {
            line++;
            size_t name_len = qtcIniGroupNameLen(line);
            if (!name_len) {
                qtcError(_("Invalid group name line @ line %d %s"),
                         lineno, line - 1);
                continue;
            }
            QtcIniGroup *new_group;
            new_group = qtcIniFileFindGroupWithLen(file, line, name_len);
            if (!new_group) {
                new_group = qtcIniFileHashNewGroup(file, line, name_len);
            } else {
                if (new_group->flags & QTC_INI_GROUP_UPDATED) {
                    qtcWarn(_("Duplicate group %s in a ini file,"
                              "@ line %d, merge with previous one."),
                            new_group->name, lineno);
                    if (new_group == cur_group)
                        cur_group = new_group->prev;
                    qtcIniGroupUnlink(file, new_group);
                } else {
                    new_group->first = NULL;
                    new_group->last = NULL;
                }
            }
            new_group->flags |= QTC_INI_GROUP_UPDATED;
            qtcIniFileLinkGroupAfter(file, cur_group, new_group);
            cur_group = new_group;
            continue;
        }
        default:
            break;
        }
        if (!cur_group) {
            qtcError(_("Non-comment doesn't belongs to any group @ %d %s"),
                     lineno, line);
            continue;
        }
        size_t key_len = qtcIniEntryKeyLen(line);
        if (!key_len) {
            qtcError(_("Invalid entry line @ line %d %s"), lineno, line);
            continue;
        }
        QtcIniEntry *new_entry;
        new_entry = qtcIniGroupFindEntryWithLen(cur_group, line, key_len);
        if (!new_entry) {
            new_entry = qtcIniGroupHashNewEntry(cur_group, line, key_len);
        } else if (new_entry->flags & QTC_INI_ENTRY_UPDATED) {
            qtcWarn(_("Duplicate entry %s in group %s, @ line %d, "
                      "override previous one."), new_entry->name,
                    cur_group->name, lineno);
            qtcIniEntryUnlink(cur_group, new_entry);
            qtcIniEntryReset(new_entry);
        }
        new_entry->flags |= QTC_INI_ENTRY_UPDATED;
        qtcIniGroupLinkEntryAfter(cur_group, cur_group->last, new_entry);
        char *value = line + key_len + 1;
        int value_len = buff + line_len - value;
        new_entry->value = qtcSetStr(new_entry->value, value, value_len);
    }
    qtcIniFileClean(file);
    qtcFree(buff);
    return true;
}

QTC_EXPORT bool
qtcIniFileLoad(QtcIniFile *file, const char *name)
{
    bool res;
    FILE *fp = fopen(name, "r");
    res = qtcIniFileLoadFp(file, fp);
    if (fp)
        fclose(fp);
    return res;
}

static inline size_t
_writeLen(FILE *fp, const char *str, size_t len)
{
    if (!(str && len))
        return 0;
    return fwrite(str, len, 1, fp);
}

static inline size_t
_writeStr(FILE *fp, const char *str)
{
    return _writeLen(fp, str, strlen(str));
}

static size_t
_checkSingleLine(const char *str)
{
    if (!str)
        return 0;
    size_t len = strcspn(str, "\n");
    if (str[len])
        qtcError("Not a single line, ignore.");
    return len;
}

static size_t
_checkEntryKey(const char *str)
{
    if (!str)
        return 0;
    size_t len = strcspn(str, "=\n");
    if (str[len]) {
        qtcError("Not a valid key, skip.");
        return 0;
    }
    switch (str[len - 1]) {
    case ' ':
    case '\r':
    case '\t':
    case '\f':
    case '\v':
        qtcError("Not a valid key, skip.");
        return 0;
    }
    return len;
}

static void
qtcIniEntryWriteFp(const QtcIniEntry *entry, FILE *fp)
{
    if (!entry->value)
        return;
    size_t key_len = _checkEntryKey(entry->name);
    if (!key_len)
        return;
    size_t value_len = _checkSingleLine(entry->value);
    _writeLen(fp, entry->name, key_len);
    _writeStr(fp, "=");
    _writeLen(fp, entry->value, value_len);
    _writeStr(fp, "\n");
}

static size_t
_checkGroupName(const char *str)
{
    if (!str)
        return 0;
    size_t len = strcspn(str, "[]\n");
    if (str[len]) {
        qtcError("Not a valid group name, skip.");
        return 0;
    }
    return len;
}

static void
qtcIniGroupWriteFp(const QtcIniGroup *group, FILE *fp)
{
    QtcIniEntry *entry;
    size_t name_len = _checkGroupName(group->name);
    if (!name_len)
        return;
    _writeStr(fp, "[");
    _writeLen(fp, group->name, name_len);
    _writeStr(fp, "]\n");
    for (entry = group->first;entry;entry = entry->next) {
        qtcIniEntryWriteFp(entry, fp);
    }
}

QTC_EXPORT bool
qtcIniFileWriteFp(const QtcIniFile *file, FILE *fp)
{
    if (!(file && fp))
        return false;
    QtcIniGroup *group;
    for (group = file->first;group;group = group->next) {
        qtcIniGroupWriteFp(group, fp);
    }
    return true;
}

QTC_EXPORT bool
qtcIniFileWrite(const QtcIniFile *file, const char *name)
{
    bool res;
    FILE *fp = fopen(name, "w");
    res = qtcIniFileWriteFp(file, fp);
    if (fp)
        fclose(fp);
    return res;
}

static inline bool
_qtcIniFileHasGroup(QtcIniFile *file, QtcIniGroup *group)
{
    if (qtcUnlikely(!file->groups || !group))
        return false;
    return file->groups->hh.tbl == group->hh.tbl;
}

QTC_EXPORT bool
qtcIniFileHasGroup(QtcIniFile *file, QtcIniGroup *group)
{
    return _qtcIniFileHasGroup(file, group);
}

QTC_EXPORT bool
qtcIniFileDeleteGroup(QtcIniFile *file, QtcIniGroup *group)
{
    if (!_qtcIniFileHasGroup(file, group))
        return false;
    qtcIniGroupUnlink(file, group);
    qtcIniFileHashRemoveGroup(file, group);
    return true;
}

static QtcIniGroup*
qtcIniFileAddGroup(QtcIniFile *file, QtcIniGroup *base, const char *name,
                   size_t name_len, bool move, bool before)
{
    if (!base) {
        base = file->last;
    } else if (!_qtcIniFileHasGroup(file, base)) {
        qtcError("The given group doesn't belong to the given file.");
        return NULL;
    }
    QtcIniGroup *new_group;
    new_group =  qtcIniFileFindGroupWithLen(file, name, name_len);
    if (!new_group) {
        new_group = qtcIniFileHashNewGroup(file, name, name_len);
    } else if (move && !(new_group == base)) {
        qtcIniGroupUnlink(file, new_group);
    } else {
        return new_group;
    }
    qtcIniFileLinkGroup(file, base, new_group, before);
    return new_group;
}

QTC_EXPORT QtcIniGroup*
qtcIniFileAddGroupAfterWithLen(QtcIniFile *file, QtcIniGroup *base,
                               const char *name, size_t name_len, bool move)
{
    return qtcIniFileAddGroup(file, base, name, name_len, move, false);
}

QTC_EXPORT QtcIniGroup*
qtcIniFileAddGroupBeforeWithLen(QtcIniFile *file, QtcIniGroup *base,
                                const char *name, size_t name_len, bool move)
{
    return qtcIniFileAddGroup(file, base, name, name_len, move, true);
}

static bool
qtcIniFileInsertGroup(QtcIniFile *file, QtcIniGroup *base, QtcIniGroup *group,
                      bool move, bool before)
{
    if (qtcUnlikely(!group))
        return false;
    if (!base) {
        base = file->last;
    } else if (!_qtcIniFileHasGroup(file, base)) {
        qtcError("The given group doesn't belong to the given file.");
        return false;
    }
    if (!group->hh.tbl) {
        qtcIniFileHashAdd(file, group, strlen(group->name));
    } else if (!_qtcIniFileHasGroup(file, group)) {
        qtcError("The given group belongs to another file.");
        return false;
    } else if (!move || group == base) {
        return true;
    } else {
        qtcIniGroupUnlink(file, group);
    }
    qtcIniFileLinkGroup(file, base, group, before);
    return true;
}

QTC_EXPORT bool
qtcIniFileInsertGroupAfter(QtcIniFile *file, QtcIniGroup *base,
                           QtcIniGroup *group, bool move)
{
    return qtcIniFileInsertGroup(file, base, group, move, false);
}

QTC_EXPORT bool
qtcIniFileInsertGroupBefore(QtcIniFile *file, QtcIniGroup *base,
                            QtcIniGroup *group, bool move)
{
    return qtcIniFileInsertGroup(file, base, group, move, true);
}

static inline bool
_qtcIniGroupHasEntry(QtcIniGroup *group, QtcIniEntry *entry)
{
    if (qtcUnlikely(!group->entries || !entry))
        return false;
    return group->entries->hh.tbl == entry->hh.tbl;
}

QTC_EXPORT bool
qtcIniGroupHasEntry(QtcIniGroup *group, QtcIniEntry *entry)
{
    return _qtcIniGroupHasEntry(group, entry);
}

QTC_EXPORT bool
qtcIniGroupDeleteEntry(QtcIniGroup *group, QtcIniEntry *entry)
{
    if (!_qtcIniGroupHasEntry(group, entry))
        return false;
    qtcIniEntryUnlink(group, entry);
    qtcIniGroupHashRemoveEntry(group, entry);
    return true;
}

static QtcIniEntry*
qtcIniGroupAddEntry(QtcIniGroup *group, QtcIniEntry *base, const char *name,
                    size_t name_len, bool move, bool before)
{
    if (!base) {
        base = group->last;
    } else if (!_qtcIniGroupHasEntry(group, base)) {
        qtcError("The given entry doesn't belong to the given group.");
        return NULL;
    }
    QtcIniEntry *new_entry;
    new_entry =  qtcIniGroupFindEntryWithLen(group, name, name_len);
    if (!new_entry) {
        new_entry = qtcIniGroupHashNewEntry(group, name, name_len);
    } else if (move && !(new_entry == base)) {
        qtcIniEntryUnlink(group, new_entry);
    } else {
        return new_entry;
    }
    qtcIniGroupLinkEntry(group, base, new_entry, before);
    return new_entry;
}

QTC_EXPORT QtcIniEntry*
qtcIniGroupAddEntryAfterWithLen(QtcIniGroup *group, QtcIniEntry *base,
                                const char *name, size_t name_len, bool move)
{
    return qtcIniGroupAddEntry(group, base, name, name_len, move, false);
}

QTC_EXPORT QtcIniEntry*
qtcIniGroupAddEntryBeforeWithLen(QtcIniGroup *group, QtcIniEntry *base,
                                 const char *name, size_t name_len,
                                 bool move)
{
    return qtcIniGroupAddEntry(group, base, name, name_len, move, true);
}

static bool
qtcIniGroupInsertEntry(QtcIniGroup *group, QtcIniEntry *base,
                       QtcIniEntry *entry, bool move, bool before)
{
    if (qtcUnlikely(!entry))
        return false;
    if (!base) {
        base = group->last;
    } else if (!_qtcIniGroupHasEntry(group, base)) {
        qtcError("The given entry doesn't belong to the given group.");
        return false;
    }
    if (!entry->hh.tbl) {
        qtcIniGroupHashAdd(group, entry, strlen(entry->name));
    } else if (!_qtcIniGroupHasEntry(group, entry)) {
        qtcError("The given entry belongs to another group.");
        return false;
    } else if (!move || entry == base) {
        return true;
    } else {
        qtcIniEntryUnlink(group, entry);
    }
    qtcIniGroupLinkEntry(group, base, entry, before);
    return true;
}

QTC_EXPORT bool
qtcIniGroupInsertEntryAfter(QtcIniGroup *group, QtcIniEntry *base,
                            QtcIniEntry *entry, bool move)
{
    return qtcIniGroupInsertEntry(group, base, entry, move, false);
}

QTC_EXPORT bool
qtcIniGroupInsertEntryBefore(QtcIniGroup *group, QtcIniEntry *base,
                             QtcIniEntry *entry, bool move)
{
    return qtcIniGroupInsertEntry(group, base, entry, move, true);
}
