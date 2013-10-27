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

#ifndef _QTC_UTILS_CONF_DESC_H_
#define _QTC_UTILS_CONF_DESC_H_

#include "color.h"
#include "list.h"

typedef enum {
    QTC_CONF_STR,
    QTC_CONF_INT,
    QTC_CONF_ENUM,
    QTC_CONF_FLOAT,
    QTC_CONF_BOOL,
    QTC_CONF_COLOR,
    QTC_CONF_STR_LIST,
    QTC_CONF_INT_LIST,
    QTC_CONF_FLOAT_LIST,
} QtcConfType;

typedef struct {
    char *id;
    char *name;
    char *enum_name;
} QtcConfEnumDesc;

typedef union {
    struct {
        unsigned max_len;
        bool is_static;
    } str_c;
    struct {
        long min;
        long max;
    } int_c;
    struct {
        unsigned num;
        QtcConfEnumDesc *descs;

        char *enum_type;
    } enum_c;
    struct {
        double min;
        double max;
    } float_c;
    struct {
        unsigned max_strlen;
        bool is_str_static;

        unsigned min_count;
        unsigned max_count;
        bool is_array_static;
    } str_list_c;
    struct {
        long min_val;
        long max_val;

        unsigned min_count;
        unsigned max_count;
        bool is_array_static;
    } int_list_c;
    struct {
        double min_val;
        double max_val;

        unsigned min_count;
        unsigned max_count;
        bool is_array_static;
    } float_list_c;
} QtcConfConstrain;

typedef union {
    char *str_def;
    long int_def;
    char *enum_def;
    double float_def;
    bool bool_def;
    QtcColor color_def;
    struct {
        bool is_ele_def;
        size_t len;
        union {
            char *val;
            char **vals;
        };
    } str_list_def;
    struct {
        bool is_ele_def;
        size_t len;
        union {
            long val;
            long *vals;
        };
    } int_list_def;
    struct {
        bool is_ele_def;
        size_t len;
        union {
            double val;
            double *vals;
        };
    } float_list_def;
} QtcConfDefault;

typedef struct {
    QtcConfType type;
    QtcConfConstrain constrain;
    QtcConfDefault def;
} QtcConfValueDesc;

typedef enum {
    // Hidden by default
    QTC_CONF_OP_ADVANCED = (1 << 0),
    // Replaced by other options, kept just for backward compatibility.
    QTC_CONF_OP_DEPRECATED = (1 << 1),
} QtcConfOptionFlags;

// Keep the name field as the first one in both group and option structure
// in order to use the same compare function for them.
typedef struct {
    char *name;
    QtcConfValueDesc vdesc;
    char *sub_group;
    char *desc;
    char *long_desc;
    QtcConfOptionFlags flags;
    QtcList link;
} QtcConfOptionDesc;

typedef struct {
    char *name;
    char *desc;
    char *long_desc;
    QtcList link;
    QtcList option_descs;
    unsigned option_num;
    QtcConfOptionDesc **option_list;
} QtcConfGroupDesc;

typedef struct {
    char *domain;
    QtcList group_descs;
    unsigned group_num;
    QtcConfGroupDesc **group_list;
} QtcConfFileDesc;

QTC_BEGIN_DECLS

QtcConfFileDesc *qtcConfDescLoadFp(FILE *fp);
QtcConfFileDesc *qtcConfDescLoad(const char *fname);
void qtcConfDescFree(QtcConfFileDesc *desc);
QtcConfGroupDesc *qtcConfGroupDescFind(
    QtcConfFileDesc *conf_desc, const char *name, size_t name_len);
QTC_ALWAYS_INLINE static inline QtcConfGroupDesc*
_qtcConfGroupDescFind(QtcConfFileDesc *conf_desc, const char *name)
{
    return qtcConfGroupDescFind(conf_desc, name, strlen(name));
}
#define qtcConfGroupDescFind(conf_desc, name, name_len...)              \
    QTC_SWITCH_(name_len, qtcConfGroupDescFind)(conf_desc, name, ##name_len)

QtcConfOptionDesc *qtcConfOptionDescFind(
    QtcConfGroupDesc *grp_desc, const char *name, size_t name_len);
QTC_ALWAYS_INLINE static inline QtcConfOptionDesc*
_qtcConfOptionDescFind(QtcConfGroupDesc *grp_desc, const char *name)
{
    return qtcConfOptionDescFind(grp_desc, name, strlen(name));
}
#define qtcConfOptionDescFind(grp_desc, name, name_len...)              \
    QTC_SWITCH_(name_len, qtcConfOptionDescFind)(grp_desc, name, ##name_len)

QTC_END_DECLS

#endif
