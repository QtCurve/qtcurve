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

include(CMakeArrayMacros)

# cmake_parse_array_default(name [prefix] [start | stop | start stop])
macro(cmake_parse_array_default name)
  cmake_parse_array("${name}" "${OPTIONS}" "${SINGLES}"
    "${MULTIPLES}" "${LISTS}" ${ARGN})
endmacro()

# cmake_parse_array(name options single_args multi_args multi_lists
#                   [prefix] [start | stop | start stop])
macro(cmake_parse_array name options single_args multi_args multi_lists)
  cmake_parse_array_full("${name}" "${options}" "${single_args}"
    "${multi_args}" "${multi_lists}" "" ":" "" "[]" ${ARGN})
endmacro()

# cmake_parse_array_classic_default(name [prefix] [start | stop | start stop])
macro(cmake_parse_array_classic_default name)
  cmake_parse_array_classic("${name}" "${OPTIONS}" "${SINGLES}"
    "${MULTIPLES}" "${LISTS}" ${ARGN})
endmacro()

# cmake_parse_array_classic(name options single_args multi_args multi_lists
#                           [prefix] [start | stop | start stop])
macro(cmake_parse_array_classic name options single_args multi_args multi_lists)
  cmake_parse_array_full("${name}" "${options}" "${single_args}"
    "${multi_args}" "${multi_lists}" "" "" "" "[]" ${ARGN})
endmacro()

macro(__cmake_parse_array_full_set_state state)
  set(__cmake_parse_array_state "${state}")
  set(__cmake_parse_array_cur_option "${__cmake_parse_array_option}")
endmacro()

# cmake_parse_array_full(name options single_args multi_args multi_lists
#                        option_prefix option_suffix array_prefix
#                        array_suffix [prefix] [start | stop | start stop])
macro(cmake_parse_array_full name options single_args multi_args multi_lists
    op_pre op_suf ary_pre ary_suf)
  macro(__cmake_parse_array_full_multi_list_array_foreach)
    cmake_array_copy("${__cmake_parse_array_multi_list_array_value}"
      __cmake_parse_array_state)
    cmake_array_append("${${name}_${__cmake_parse_array_cur_option}}"
      "${__cmake_parse_array_state}")
  endmacro()

  # init
  foreach(__cmake_parse_array_full_option ${options})
    set("${name}_${__cmake_parse_array_full_option}" False)
  endforeach()
  foreach(__cmake_parse_array_full_option ${single_args})
    set("${name}_${__cmake_parse_array_full_option}")
  endforeach()
  foreach(__cmake_parse_array_full_option ${multi_args} ${multi_lists} UNUSED)
    cmake_array_new("${name}_${__cmake_parse_array_full_option}")
  endforeach()

  # define local variables in macro
  foreach(__cmake_parse_array_state unused)
    foreach(__cmake_parse_array_cur_option "")
      # Lexical scope used here.
      # Do not move the definition out of the parent macro.
      macro(__cmake_parse_array_full_foreach)
        # handle single argument first
        if("${__cmake_parse_array_state}" STREQUAL single)
          set("${name}_${__cmake_parse_array_cur_option}"
            "${__cmake_parse_array_full_value}")
          set(__cmake_parse_array_state unused)
          return()
        elseif("${__cmake_parse_array_state}" STREQUAL multi_array)
          cmake_array_concat("${${name}_${__cmake_parse_array_cur_option}}"
            "${__cmake_parse_array_full_value}")
          set(__cmake_parse_array_state unused)
          return()
        elseif("${__cmake_parse_array_state}" STREQUAL list_array)
          cmake_array_copy("${__cmake_parse_array_full_value}"
            __cmake_parse_array_state)
          cmake_array_append("${${name}_${__cmake_parse_array_cur_option}}"
            "${__cmake_parse_array_state}")
          set(__cmake_parse_array_state unused)
          return()
        elseif("${__cmake_parse_array_state}" STREQUAL multi_list_array)
          cmake_array_foreach(__cmake_parse_array_multi_list_array_value
            __cmake_parse_array_full_multi_list_array_foreach
            "${__cmake_parse_array_full_value}")
          set(__cmake_parse_array_state unused)
          return()
        endif()
        # Check if the argument is one of the options
        foreach(__cmake_parse_array_option ${options})
          if("x${op_pre}${__cmake_parse_array_option}${op_suf}x" STREQUAL
              "x${__cmake_parse_array_full_value}x")
            set("${name}_${__cmake_parse_array_option}" True)
            set(__cmake_parse_array_state unused)
            return()
          endif()
        endforeach()
        # Check if the argument is one of the single value arguments
        foreach(__cmake_parse_array_option ${single_args})
          if("x${op_pre}${__cmake_parse_array_option}${op_suf}x" STREQUAL
              "x${__cmake_parse_array_full_value}x")
            __cmake_parse_array_full_set_state(single)
            return()
          endif()
        endforeach()
        # Check if the argument is one of the multi value arguments
        foreach(__cmake_parse_array_option ${multi_args})
          if("x${op_pre}${__cmake_parse_array_option}${op_suf}x" STREQUAL
              "x${__cmake_parse_array_full_value}x")
            __cmake_parse_array_full_set_state(multi)
            return()
          endif()
          if("x${ary_pre}x" STREQUAL "xx" AND "x${ary_suf}x" STREQUAL "xx")
            if("x${op_pre}${__cmake_parse_array_option}[]${op_suf}x" STREQUAL
                "x${__cmake_parse_array_full_value}x")
              __cmake_parse_array_full_set_state(multi_array)
              return()
            endif()
          else()
            if("x${op_pre}${ary_pre}${__cmake_parse_array_option}${ary_suf}${op_suf}x" STREQUAL "x${__cmake_parse_array_full_value}x")
              __cmake_parse_array_full_set_state(multi_array)
              return()
            endif()
          endif()
        endforeach()
        # Check if the argument is one of the multi list arguments
        foreach(__cmake_parse_array_option ${multi_lists})
          if("x${op_pre}${__cmake_parse_array_option}${op_suf}x" STREQUAL
              "x${__cmake_parse_array_full_value}x")
            set(__cmake_parse_array_state list)
            cmake_array_new(__cmake_parse_array_cur_option)
            cmake_array_append("${${name}_${__cmake_parse_array_option}}"
              "${__cmake_parse_array_cur_option}")
            return()
          endif()
          if("x${ary_pre}x" STREQUAL "xx" AND "x${ary_suf}x" STREQUAL "xx")
            if("x${op_pre}${__cmake_parse_array_option}[]${op_suf}x" STREQUAL
                "x${__cmake_parse_array_full_value}x")
              __cmake_parse_array_full_set_state(list_array)
              return()
            elseif("x${op_pre}${__cmake_parse_array_option}[][]${op_suf}x"
                STREQUAL "x${__cmake_parse_array_full_value}x")
              __cmake_parse_array_full_set_state(multi_list_array)
              return()
            endif()
          elseif("x${op_pre}${ary_pre}${__cmake_parse_array_option}${ary_suf}${op_suf}x" STREQUAL "x${__cmake_parse_array_full_value}x")
            __cmake_parse_array_full_set_state(list_array)
            return()
          elseif("x${op_pre}${ary_pre}${ary_pre}${__cmake_parse_array_option}${ary_suf}${ary_suf}${op_suf}x" STREQUAL "x${__cmake_parse_array_full_value}x")
              __cmake_parse_array_full_set_state(multi_list_array)
            return()
          endif()
        endforeach()
        if("${__cmake_parse_array_state}" STREQUAL unused)
          cmake_array_append("${${name}_UNUSED}"
            "${__cmake_parse_array_full_value}")
          return()
        elseif("${__cmake_parse_array_state}" STREQUAL multi)
          cmake_array_append("${${name}_${__cmake_parse_array_cur_option}}"
            "${__cmake_parse_array_full_value}")
          return()
        elseif("${__cmake_parse_array_state}" STREQUAL list)
          cmake_array_append("${__cmake_parse_array_cur_option}"
            "${__cmake_parse_array_full_value}")
        endif()
      endmacro()
      cmake_array_foreach(__cmake_parse_array_full_value
        __cmake_parse_array_full_foreach ${ARGN})
    endforeach()
  endforeach()
endmacro()
