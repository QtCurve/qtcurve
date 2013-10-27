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
#include "map.h"

#include <float.h>
#include <limits.h>

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
    unsigned old_num = conf_desc->group_num;
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

QTC_EXPORT QtcConfGroupDesc*
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
    if (p) {
        return *p;
    }
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
    unsigned old_num = grp_desc->option_num;
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

QTC_EXPORT QtcConfOptionDesc*
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
    if (p) {
        return *p;
    }
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

static int
_qtcConfLoadType(const char *str)
{
    QTC_DEF_ENUM(conf_type_map, true, {"Str", QTC_CONF_STR},
                 {"Int", QTC_CONF_INT}, {"Enum", QTC_CONF_ENUM},
                 {"Float", QTC_CONF_FLOAT}, {"Bool", QTC_CONF_BOOL},
                 {"Color", QTC_CONF_COLOR}, {"StrList", QTC_CONF_STR_LIST},
                 {"IntList", QTC_CONF_INT_LIST},
                 {"FloatList", QTC_CONF_FLOAT_LIST});
    return qtcEnumSearch(&conf_type_map, str);
}

static void
_qtcConfDescLoadConstrain(QtcConfValueDesc *vdesc, QtcIniGroup *ini_grp)
{
    switch (vdesc->type) {
    default:
    case QTC_CONF_STR:
        vdesc->str_c.max_len =
            qtcMax(qtcIniGroupGetInt(ini_grp, "MaxLength", 0), 0);
        if (!vdesc->str_c.max_len) {
            vdesc->str_c.is_static = false;
        } else {
            vdesc->str_c.is_static =
                qtcIniGroupGetBool(ini_grp, "Static", false);
        }
        return;
    case QTC_CONF_INT:
        vdesc->int_c.max = qtcIniGroupGetInt(ini_grp, "Max", LONG_MAX);
        vdesc->int_c.min = qtcIniGroupGetInt(ini_grp, "Min", LONG_MIN);
        return;
    case QTC_CONF_ENUM: {
        unsigned i = 0;
        char enum_opt[sizeof("Enum.EnumName") + sizeof(int) * 8] = "Enum";
        unsigned enum_alloc = 8;
        QtcConfEnumDesc *enum_descs = qtcNew(QtcConfEnumDesc, enum_alloc);
        for (;;i++) {
            const unsigned base_len =
                strlen("Enum") + sprintf(enum_opt + strlen("Enum"), "%u", i);
            char *enum_id = qtcIniGroupDupValue(ini_grp, enum_opt, base_len);
            if (!enum_id) {
                break;
            }
            if (enum_alloc <= i) {
                enum_alloc *= 2;
                enum_descs = realloc(enum_descs,
                                     enum_alloc * sizeof(QtcConfEnumDesc));
            }
            enum_descs[i].id = enum_id;
            memcpy(enum_opt + base_len, ".Name", sizeof(".Name"));
            enum_descs[i].name = qtcIniGroupDupValue(
                ini_grp, enum_opt, base_len + strlen(".Name"));
            memcpy(enum_opt + base_len, ".EnumName", sizeof(".EnumName"));
            enum_descs[i].enum_name = qtcIniGroupDupValue(
                ini_grp, enum_opt, base_len + strlen(".EnumName"));
        }
        if (i) {
            enum_descs = realloc(enum_descs, i * sizeof(QtcConfEnumDesc));
        }
        vdesc->enum_c.num = i;
        vdesc->enum_c.descs = enum_descs;
        vdesc->enum_c.enum_type = qtcIniGroupDupValue(ini_grp, "EnumType");
        break;
    }
    case QTC_CONF_FLOAT:
        vdesc->float_c.max = qtcIniGroupGetFloat(ini_grp, "Max", DBL_MAX);
        vdesc->float_c.min = qtcIniGroupGetFloat(ini_grp, "Min", -DBL_MAX);
        return;
    case QTC_CONF_BOOL:
    case QTC_CONF_COLOR:
        break;
    case QTC_CONF_STR_LIST:
        vdesc->str_list_c.max_strlen =
            qtcMax(qtcIniGroupGetInt(ini_grp, "MaxStrlen", 0), 0);
        if (!vdesc->str_list_c.max_strlen) {
            vdesc->str_list_c.is_str_static = false;
        } else {
            vdesc->str_list_c.is_str_static =
                qtcIniGroupGetBool(ini_grp, "StaticStr", false);
        }

        vdesc->str_list_c.max_count =
            qtcMax(qtcIniGroupGetInt(ini_grp, "MaxCount", 0), 0);
        vdesc->str_list_c.min_count =
            qtcMax(qtcIniGroupGetInt(ini_grp, "MinCount", 0), 0);
        if (!vdesc->str_list_c.max_count) {
            vdesc->str_list_c.is_array_static = false;
        } else {
            vdesc->str_list_c.is_array_static =
                qtcIniGroupGetBool(ini_grp, "StaticArray", false);
        }
        return;
    case QTC_CONF_INT_LIST:
        vdesc->int_list_c.max_val =
            qtcIniGroupGetInt(ini_grp, "Max", LONG_MAX);
        vdesc->int_list_c.min_val =
            qtcIniGroupGetInt(ini_grp, "Min", LONG_MIN);

        vdesc->int_list_c.max_count =
            qtcMax(qtcIniGroupGetInt(ini_grp, "MaxCount", 0), 0);
        vdesc->int_list_c.min_count =
            qtcMax(qtcIniGroupGetInt(ini_grp, "MinCount", 0), 0);
        if (!vdesc->int_list_c.max_count) {
            vdesc->int_list_c.is_array_static = false;
        } else {
            vdesc->int_list_c.is_array_static =
                qtcIniGroupGetBool(ini_grp, "StaticArray", false);
        }
        return;
    case QTC_CONF_FLOAT_LIST:
        vdesc->float_list_c.max_val =
            qtcIniGroupGetFloat(ini_grp, "Max", DBL_MAX);
        vdesc->float_list_c.min_val =
            qtcIniGroupGetFloat(ini_grp, "Min", -DBL_MAX);

        vdesc->float_list_c.max_count =
            qtcMax(qtcIniGroupGetFloat(ini_grp, "MaxCount", 0), 0);
        vdesc->float_list_c.min_count =
            qtcMax(qtcIniGroupGetFloat(ini_grp, "MinCount", 0), 0);
        if (!vdesc->float_list_c.max_count) {
            vdesc->float_list_c.is_array_static = false;
        } else {
            vdesc->float_list_c.is_array_static =
                qtcIniGroupGetBool(ini_grp, "StaticArray", false);
        }
        return;
    }
}

static void
_qtcConfDescLoadDefault(QtcConfValueDesc *vdesc, QtcIniGroup *ini_grp)
{
    switch (vdesc->type) {
    default:
    case QTC_CONF_STR:
        vdesc->str_def = qtcIniGroupDupValue(ini_grp, "Default");
        return;
    case QTC_CONF_INT:
        vdesc->int_def = qtcIniGroupGetInt(ini_grp, "Default", 0);
        return;
    case QTC_CONF_ENUM:
        vdesc->enum_def = qtcIniGroupDupValue(ini_grp, "Default");
        return;
    case QTC_CONF_FLOAT:
        vdesc->float_def = qtcIniGroupGetFloat(ini_grp, "Default", 0);
        return;
    case QTC_CONF_BOOL:
        vdesc->bool_def = qtcIniGroupGetBool(ini_grp, "Default", false);
        return;
    case QTC_CONF_COLOR:
        qtcColorFromStr(&vdesc->color_def,
                        qtcIniGroupGetValue(ini_grp, "Default"));
        return;
    case QTC_CONF_STR_LIST:
        vdesc->str_list_def.vals =
            qtcStrLoadStrList(qtcIniGroupGetValue(ini_grp, "Default"), , ,
                              &vdesc->str_list_def.len, , ,);
        if (vdesc->str_list_def.vals) {
            vdesc->str_list_def.is_ele_def = false;
        } else {
            vdesc->str_list_def.is_ele_def = true;
            vdesc->str_list_def.len =
                qtcMax(qtcIniGroupGetInt(ini_grp, "Length", 0), 0);
            vdesc->str_list_def.val =
                qtcIniGroupDupValue(ini_grp, "EleDefault");
        }
        return;
    case QTC_CONF_INT_LIST:
        vdesc->int_list_def.vals =
            qtcStrLoadIntList(qtcIniGroupGetValue(ini_grp, "Default"), , ,
                              &vdesc->int_list_def.len, , ,);
        if (vdesc->int_list_def.vals) {
            vdesc->int_list_def.is_ele_def = false;
        } else {
            vdesc->int_list_def.is_ele_def = true;
            vdesc->int_list_def.len =
                qtcMax(qtcIniGroupGetInt(ini_grp, "Length", 0), 0);
            vdesc->int_list_def.val =
                qtcIniGroupGetInt(ini_grp, "EleDefault", 0);
        }
        return;
    case QTC_CONF_FLOAT_LIST:
        vdesc->float_list_def.vals =
            qtcStrLoadFloatList(qtcIniGroupGetValue(ini_grp, "Default"), , ,
                                &vdesc->float_list_def.len, , ,);
        if (vdesc->float_list_def.vals) {
            vdesc->float_list_def.is_ele_def = false;
        } else {
            vdesc->float_list_def.is_ele_def = true;
            vdesc->float_list_def.len =
                qtcMax(qtcIniGroupGetFloat(ini_grp, "Length", 0), 0);
            vdesc->float_list_def.val =
                qtcIniGroupGetFloat(ini_grp, "EleDefault", 0);
        }
        return;
    }
}

static void
_qtcConfDescLoadOption(QtcConfOptionDesc *opt_desc, QtcIniGroup *ini_grp)
{
    opt_desc->desc = qtcIniGroupDupValue(ini_grp, "Desc");
    opt_desc->long_desc = qtcIniGroupDupValue(ini_grp, "LongDesc");
    opt_desc->sub_group = qtcIniGroupDupValue(ini_grp, "SubGroup");
    if (qtcIniGroupGetBool(ini_grp, "Advanced", false)) {
        opt_desc->flags |= QTC_CONF_OP_ADVANCED;
    }
    if (qtcIniGroupGetBool(ini_grp, "Deprecated", false)) {
        opt_desc->flags |= QTC_CONF_OP_DEPRECATED;
    }
    QtcConfValueDesc *vdesc = &opt_desc->vdesc;
    vdesc->type = _qtcConfLoadType(qtcIniGroupGetValue(ini_grp, "Type"));
    _qtcConfDescLoadConstrain(vdesc, ini_grp);
    _qtcConfDescLoadDefault(vdesc, ini_grp);
}

static void
_qtcConfDescLoadGroupProps(QtcConfGroupDesc *group_desc, QtcIniGroup *ini_grp)
{
    group_desc->desc = qtcIniGroupDupValue(ini_grp, "Desc");
    group_desc->long_desc = qtcIniGroupDupValue(ini_grp, "LongDesc");
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
        size_t opt_name_len = strlen(opt_name);
        if (qtcUnlikely(!opt_name_len)) {
            qtcError("Option name cannot have zero length.\n");
            return;
        }
        _qtcConfDescLoadOption(
            qtcConfOptionDescNew(group_desc, opt_name, opt_name_len), ini_grp);
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
qtcConfValueDescConstrainFree(QtcConfValueDesc *vdesc)
{
    switch (vdesc->type) {
    default:
    case QTC_CONF_STR:
    case QTC_CONF_INT:
    case QTC_CONF_FLOAT:
    case QTC_CONF_BOOL:
    case QTC_CONF_COLOR:
    case QTC_CONF_STR_LIST:
    case QTC_CONF_INT_LIST:
    case QTC_CONF_FLOAT_LIST:
        break;
    case QTC_CONF_ENUM:
        for (unsigned i = 0;i < vdesc->enum_c.num;i++) {
            free(vdesc->enum_c.descs[i].id);
            qtcFree(vdesc->enum_c.descs[i].name);
            qtcFree(vdesc->enum_c.descs[i].enum_name);
        }
        qtcFree(vdesc->enum_c.enum_type);
        free(vdesc->enum_c.descs);
    }
}

static void
qtcConfValueDescDefaultFree(QtcConfValueDesc *vdesc)
{
    switch (vdesc->type) {
    case QTC_CONF_INT:
    case QTC_CONF_FLOAT:
    case QTC_CONF_BOOL:
    case QTC_CONF_COLOR:
        break;
    default:
    case QTC_CONF_STR:
        qtcFree(vdesc->str_def);
        return;
    case QTC_CONF_ENUM:
        qtcFree(vdesc->enum_def);
        return;
    case QTC_CONF_STR_LIST:
        if (vdesc->str_list_def.is_ele_def) {
            qtcFree(vdesc->str_list_def.val);
        } else {
            for (unsigned i = 0;i < vdesc->str_list_def.len;i++) {
                free(vdesc->str_list_def.vals[i]);
            }
            qtcFree(vdesc->str_list_def.vals);
        }
        return;
    case QTC_CONF_INT_LIST:
        if (!vdesc->int_list_def.is_ele_def) {
            qtcFree(vdesc->int_list_def.vals);
        }
        return;
    case QTC_CONF_FLOAT_LIST:
        if (!vdesc->float_list_def.is_ele_def) {
            qtcFree(vdesc->float_list_def.vals);
        }
        return;
    }
}

static void
qtcConfOptionDescFree(QtcConfOptionDesc *option)
{
    qtcConfValueDescDefaultFree(&option->vdesc);
    qtcConfValueDescConstrainFree(&option->vdesc);
    qtcFree(option->sub_group);
    qtcFree(option->desc);
    qtcFree(option->long_desc);
    free(option->name);
    free(option);
}

static void
qtcConfGroupDescFree(QtcConfGroupDesc *group)
{
    for (unsigned i = 0;i < group->option_num;i++) {
        qtcConfOptionDescFree(group->option_list[i]);
    }
    qtcFree(group->desc);
    qtcFree(group->long_desc);
    qtcFree(group->option_list);
    free(group->name);
    free(group);
}

QTC_EXPORT void
qtcConfDescFree(QtcConfFileDesc *desc)
{
    for (unsigned i = 0;i < desc->group_num;i++) {
        qtcConfGroupDescFree(desc->group_list[i]);
    }
    qtcFree(desc->group_list);
    free(desc->domain);
    free(desc);
}
