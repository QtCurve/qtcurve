/*****************************************************************************
 *   Copyright 2013 - 2013 Yichao Yu <yyc1992@gmail.com>                     *
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

#include "conf-desc.h"
#include "ini-parse.h"
#include "log.h"

// New and find
static inline QtcConfFileDesc*
qtcConfFileDescNew()
{
    QtcConfFileDesc *conf_desc = qtcNew(QtcConfFileDesc);
    qtcListInit(&conf_desc->group_descs);
    return conf_desc;
}

typedef struct {
    const char *str;
    int len;
} QtcConfKey;

static int
_qtcConfKeyCompare(const void *_key, const void *_ele)
{
    const QtcConfKey *key = (const QtcConfKey*)_key;
    const char *const *name = (const char *const*)_ele;
    return strncmp(key->str, *name, key->len);
}

static QtcConfGroupDesc*
qtcConfGroupDescNew(QtcConfFileDesc *conf_desc, const char *name,
                    size_t name_len)
{
    QtcConfGroupDesc *grp_desc = qtcNew(QtcConfGroupDesc);
    grp_desc->name = qtcSetStr(NULL, name, name_len);
    qtcListInit(&grp_desc->link);
    qtcListInit(&grp_desc->option_descs);
    qtcListInsert(&conf_desc->group_descs, &grp_desc->link);
    int old_num = conf_desc->group_num;
    conf_desc->group_num++;
    size_t new_alloc = conf_desc->group_num * sizeof(QtcConfGroupDesc*);
    conf_desc->group_list = realloc(conf_desc->group_list, new_alloc);
    QtcConfKey key = {
        .str = name,
        .len = name_len,
    };
    QtcConfGroupDesc **p = qtcBSearch(&key, conf_desc->group_list, old_num,
                                      sizeof(QtcConfGroupDesc*),
                                      _qtcConfKeyCompare);
    memmove(p + sizeof(QtcConfGroupDesc*), p,
            conf_desc->group_list + new_alloc - p - sizeof(QtcConfGroupDesc*));
    *p = grp_desc;
    return grp_desc;
}

static QtcConfGroupDesc*
qtcConfGroupDescFind(QtcConfFileDesc *conf_desc, const char *name,
                     size_t name_len)
{
    QtcConfKey key = {
        .str = name,
        .len = name_len,
    };
    QtcConfGroupDesc **p =
        bsearch(&key, conf_desc->group_list, conf_desc->group_num,
                sizeof(QtcConfGroupDesc*), _qtcConfKeyCompare);
    if (p)
        return *p;
    return NULL;
}

static QtcConfOptionDesc*
qtcConfOptionDescNew(QtcConfGroupDesc *grp_desc, const char *name,
                     size_t name_len)
{
    QtcConfOptionDesc *opt_desc = qtcNew(QtcConfOptionDesc);
    opt_desc->name = qtcSetStr(NULL, name, name_len);
    qtcListInit(&opt_desc->link);
    qtcListInsert(&grp_desc->option_descs, &opt_desc->link);
    int old_num = grp_desc->option_num;
    grp_desc->option_num++;
    size_t new_alloc = grp_desc->option_num * sizeof(QtcConfOptionDesc*);
    grp_desc->option_list = realloc(grp_desc->option_list, new_alloc);
    QtcConfKey key = {
        .str = name,
        .len = name_len,
    };
    QtcConfOptionDesc **p = qtcBSearch(&key, grp_desc->option_list, old_num,
                                       sizeof(QtcConfOptionDesc*),
                                       _qtcConfKeyCompare);
    memmove(p + sizeof(QtcConfOptionDesc*), p,
            grp_desc->option_list + new_alloc - p -
            sizeof(QtcConfOptionDesc*));
    *p = opt_desc;
    return opt_desc;
}

static QtcConfOptionDesc*
qtcConfOptionDescFind(QtcConfGroupDesc *grp_desc, const char *name,
                      size_t name_len)
{
    QtcConfKey key = {
        .str = name,
        .len = name_len,
    };
    QtcConfOptionDesc **p =
        bsearch(&key, grp_desc->option_list, grp_desc->option_num,
                sizeof(QtcConfOptionDesc*), _qtcConfKeyCompare);
    if (p)
        return *p;
    return NULL;
}

// Loader
static void
_qtcConfDescLoadDescFileGroup(QtcConfFileDesc *conf_desc, QtcIniGroup *ini_grp)
{
    QtcIniEntry *domain_ety;
    if ((domain_ety = qtcIniGroupFindEntry(ini_grp, "LocaleDomain")) &&
        domain_ety->value && *domain_ety->value) {
        conf_desc->domain = strdup(domain_ety->value);
    }
}

static void
_qtcConfDescLoadOption(QtcConfGroupDesc *group_desc, QtcIniGroup *ini_grp,
                       const char *opt_name)
{
    size_t opt_name_len = strlen(opt_name);
    if (qtcUnlikely(!opt_name_len)) {
        qtcError("Option name cannot have zero length.\n");
        return;
    }
    QtcConfOptionDesc *opt_desc =
        qtcConfOptionDescFind(group_desc, opt_name, opt_name_len);
    if (!opt_desc) {
        opt_desc = qtcConfOptionDescNew(group_desc, opt_name, opt_name_len);
    }
    // TODO set option properties
}

static void
_qtcConfDescLoadGroupProps(QtcConfGroupDesc *group_desc, QtcIniGroup *ini_grp)
{
    // TODO set group properties
}

static void
_qtcConfDescLoadGroup(QtcConfFileDesc *conf_desc, QtcIniGroup *ini_grp)
{
    const char *ini_grp_name = ini_grp->name;
    if (strcmp(ini_grp_name, "DescriptionFile")) {
        _qtcConfDescLoadDescFileGroup(conf_desc, ini_grp);
        return;
    }
    size_t group_name_len = strcspn(ini_grp_name, "/");
    if (qtcUnlikely(!group_name_len)) {
        qtcError("Group name cannot have zero length.\n");
        return;
    }
    QtcConfGroupDesc *group_desc =
        qtcConfGroupDescFind(conf_desc, ini_grp_name, group_name_len);
    if (!group_desc) {
        group_desc = qtcConfGroupDescNew(conf_desc, ini_grp_name,
                                         group_name_len);
    }
    if (ini_grp_name[group_name_len]) {
        const char *opt_name = ini_grp_name + group_name_len + 1;
        _qtcConfDescLoadOption(group_desc, ini_grp, opt_name);
    } else {
        _qtcConfDescLoadGroupProps(group_desc, ini_grp);
    }
}

QTC_EXPORT QtcConfFileDesc*
qtcConfDescLoadFp(FILE *fp)
{
    QtcIniFile conf_ini;
    qtcIniFileInit(&conf_ini, NULL, NULL);
    qtcIniFileLoadFp(&conf_ini, fp);

    QtcConfFileDesc *conf_desc = qtcConfFileDescNew();
    for (QtcIniGroup *ini_grp = conf_ini.first;ini_grp;
         ini_grp = ini_grp->next) {
        _qtcConfDescLoadGroup(conf_desc, ini_grp);
    }
    if (!conf_desc->domain) {
        conf_desc->domain = strdup("qtcurve");
    }

    qtcIniFileDone(&conf_ini);
    return conf_desc;
}

QTC_EXPORT QtcConfFileDesc*
qtcConfDescLoad(const char *fname)
{
    FILE *fp = fopen(fname, "r");
    QtcConfFileDesc *desc = qtcConfDescLoadFp(fp);
    fclose(fp);
    return desc;
}

static void
qtcConfGroupDescFree(QtcConfGroupDesc *group)
{
    // TODO
    free(group);
}

QTC_EXPORT void
qtcConfDescFree(QtcConfFileDesc *desc)
{
    free(desc->domain);
    for (int i = 0;i < desc->group_num;i++) {
        qtcConfGroupDescFree(desc->group_list[i]);
    }
    free(desc);
}
