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

# Public functions and macros provided by this file
# (See comment of each for more detail):
#     cmake_c_get_fix_include
#     cmake_c_fix_include_path
#     cmake_c_include_fix_path
#     cmake_c_add_flags

include(CMakeBaseMacros)
include(CMakeStringMacros)
include(CMakeHelperMacros)

function(__cmake_c_macro_init)
  cmake_utils_get_global(CMAKE_GENERAL_BASE base_dir)
  cmake_utils_set_and_make_dir("${base_dir}/cmake_c_macros"
    CMAKE_C_UTILS_BASE c_base_dir)
  cmake_helper_clear_cache("${c_base_dir}")
  cmake_utils_set_and_make_dir("${c_base_dir}/include_fix"
    CMAKE_C_UTILS_INCLUDE_FIX_PREFIX c_include_fix)
  cmake_helper_reg(cmake_c_utils cmake-c-utils-helper.sh
    cmake_c_helper_command CMAKE_C_UTILS_BASE "${c_base_dir}"
    CMAKE_C_UTILS_INCLUDE_FIX_PREFIX "${c_include_fix}")
  cmake_utils_set_global(_CMAKE_C_UTILS_HELPER_COMMAND
    "${cmake_c_helper_command}")
endfunction()
cmake_utils_call_once(__cmake_c_macro_init)

function(cmake_c_get_fix_include var)
  cmake_utils_get_global(CMAKE_C_UTILS_INCLUDE_FIX_PREFIX c_include_fix)
  set(name)
  if(ARGC GREATER 1)
    set(name "${ARGV1}")
  endif()
  set(dir "${c_include_fix}${name}")
  file(MAKE_DIRECTORY "${dir}")
  set("${var}" "${dir}" PARENT_SCOPE)
endfunction()

function(cmake_c_fix_include_path src tgt)
  set(name)
  if(ARGC GREATER 2)
    set(name "${ARGV2}")
  endif()
  cmake_utils_to_abs(src)
  cmake_c_get_fix_include(fix_path "${name}")
  cmake_helper_wrap_helper(cmake_c_utils command --fix-include
    "${src}" "${tgt}" "${fix_path}")
  execute_process(COMMAND "${command}")
endfunction()

function(cmake_c_include_fix_path)
  set(name)
  if(ARGC GREATER 0)
    set(name "${ARGV0}")
  endif()
  cmake_c_get_fix_include(fix_path "${name}")
  include_directories("${fix_path}")
endfunction()

macro(__cmake_c_add_flags_foreach)
  cmake_str_quote_shell("${new_flag}" new_flag)
  set(new_flags "${new_flags} ${new_flag}")
endmacro()

function(cmake_c_add_flags var)
  set(old_flags "${${var}}")
  set(new_flags "")
  cmake_array_foreach(new_flag __cmake_c_add_flags_foreach 1)
  set("${var}" "${old_flags} ${new_flags}" PARENT_SCOPE)
endfunction()
