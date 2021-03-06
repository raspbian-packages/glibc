/* Copyright (C) 1997, 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _SYS_UTSNAME_H
# error "Never include <bits/utsname.h> directly; use <sys/utsname.h> instead."
#endif

/* Length of the entries in 'struct utsname' is 32.  */
#define _UTSNAME_LENGTH 32

/* But the version entry is longer.  */
#define _UTSNAME_VERSION_LENGTH 256

/* If nonzero, the size of of the `domainname` field in `struct utsname'.
   This is zero to indicate that there should be no such field at all.  */
#define _UTSNAME_DOMAIN_LENGTH  0
