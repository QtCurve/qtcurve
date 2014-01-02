#!/bin/sh
# Copyright (C) 2013~2014 by Yichao Yu
# yyc1992@gmail.com
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

helper_cache=${__CMAKE_HELPER_UTILS_CACHE}
cache_base=${CMAKE_GENERAL_BASE}
. "${cache_base}/cmake_helper_general_export.sh"
action=$1
shift

__load_command_setting() {
    for __varname in "$@"; do
        eval "${__varname}="'${__command_'"${name}_${__varname}}"
    done
}

__load_all_command_settings() {
    . "${helper_cache}/command_${name}_setting.sh"
    __load_command_setting working_dir input_file output_file error_file \
        env_file
}

rewrite_cmd_settings() {
    name=$1
    shift
    __load_all_command_settings
    for var in env_file input_file output_file error_file working_dir; do
        quoted=${!var//\'/\'\\\'\'}
        printf "__command_${name}_${var}='%s'\n" "${quoted}"
    done > "${helper_cache}/command_${name}_setting.sh"
}

run_command() {
    name=$1
    shift
    __load_all_command_settings
    [ "x${env_file}x" != xx ] && {
        . "${env_file}" || return 1
    }
    [ "x${input_file}x" != xx ] && {
        exec < "${input_file}" || return 1
    }
    [ "x${output_file}x" != xx ] && {
        exec > "${output_file}" || return 1
    }
    [ "x${error_file}x" != xx ] && {
        exec 2> "${error_file}" || return 1
    }
    [ "x${working_dir}x" != xx ] && {
        cd "${working_dir}" || return 1
    }
    "__command_${name}_run" "$@"
}

clear_cache() {
    for path in "$@"; do
        [ -d "${path}" ] || continue
        "${CMAKE_HELPER_CMAKE_COMMAND}" -E remove_directory "${path}"
        "${CMAKE_HELPER_CMAKE_COMMAND}" -E make_directory "${path}"
    done > /dev/null 2>&1
}

case "$action" in
    --clear-cache)
        clear_cache "$@"
        exit $?
        ;;
    --run-command)
        run_command "$@"
        exit $?
        ;;
    --rewrite-cmd-settings)
        rewrite_cmd_settings "$@"
        exit $?
        ;;
esac
exit 1
