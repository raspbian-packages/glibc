/* gettimeofday -- Get the current time of day.  Linux/Alpha/tv64 version.
   Copyright (C) 2019-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* We can use the generic implementation, but we have to override its
   default symbol version.  */
#define SET_VERSION
#include <time/gettimeofday.c>

weak_alias (___gettimeofday, __wgettimeofday);
default_symbol_version (___gettimeofday, __gettimeofday, GLIBC_2.1);
default_symbol_version (__wgettimeofday,   gettimeofday, GLIBC_2.1);
