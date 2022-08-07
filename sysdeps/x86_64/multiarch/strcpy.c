/* Multiple versions of strcpy.
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2017-2022 Free Software Foundation, Inc.
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

/* Define multiple versions only for the definition in libc.  */
#if IS_IN (libc)
# define strcpy __redirect_strcpy
# include <string.h>
# undef strcpy

# define SYMBOL_NAME strcpy
# include "ifunc-strcpy.h"

libc_ifunc_redirected (__redirect_strcpy, strcpy, IFUNC_SELECTOR ());

# ifdef SHARED
__hidden_ver1 (strcpy, __GI_strcpy, __redirect_strcpy)
  __attribute__ ((visibility ("hidden"))) __attribute_copy__ (strcpy);
# endif
#endif
