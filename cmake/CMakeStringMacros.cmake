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
#     cmake_str_quote_cmake
#     cmake_str_quote_shell

if(COMMAND cmake_str_quote_cmake)
  return()
endif()

# cmake_str_quote_cmake(string var_name)
#     @string: the string to be quoted
#     @var_name: the variable to save the result
#
#     This function quote the @string and save it into @var_name so that
#     it can be used directly as an argument in a cmake command.
function(cmake_str_quote_cmake str var)
  string(REGEX REPLACE "\\\\" "\\\\\\\\" str "${str}")
  string(REGEX REPLACE "\\(" "\\\\\(" str "${str}")
  string(REGEX REPLACE "\\)" "\\\\\)" str "${str}")
  string(REGEX REPLACE "\\\$" "\\\\\$" str "${str}")
  string(REGEX REPLACE "\n" "\\\\n" str "${str}")
  string(REGEX REPLACE "\"" "\\\\\"" str "${str}")
  set("${var}" "\"${str}\"" PARENT_SCOPE)
endfunction()

# cmake_str_quote_shell(string var_name)
#     @string: the string to be quoted
#     @var_name: the variable to save the result
#
#     This function quote the @string and save it into @var_name so that
#     it can be used directly as an argument in a shell command.
function(cmake_str_quote_shell str var)
  if("x${str}x" MATCHES "^x[-_a-zA-Z0-9]+x\$")
    set("${var}" "${str}" PARENT_SCOPE)
  else()
    string(REGEX REPLACE "'" "'\\\\''" escaped "${str}")
    set("${var}" "'${escaped}'" PARENT_SCOPE)
  endif()
endfunction()
