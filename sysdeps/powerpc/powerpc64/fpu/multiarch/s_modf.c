/* Multiple versions of modf.
   Copyright (C) 2013-2019 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <math.h>
#include <math_ldbl_opt.h>
#include <shlib-compat.h>
#include "init-arch.h"
#include <libm-alias-double.h>

extern __typeof (__modf) __modf_ppc64 attribute_hidden;
extern __typeof (__modf) __modf_power5plus attribute_hidden;

libc_ifunc (__modf,
	    (hwcap & PPC_FEATURE_POWER5_PLUS)
	    ? __modf_power5plus
            : __modf_ppc64);

libm_alias_double (__modf, modf)

#if LONG_DOUBLE_COMPAT (libc, GLIBC_2_0)
compat_symbol (libc, __modf, modfl, GLIBC_2_0);
#endif
