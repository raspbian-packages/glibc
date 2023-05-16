/* Read data into multiple buffers.  Generic LFS version.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
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

#define PREADV preadv64
#define PREAD __pread64
#define OFF_T off64_t
#include <sysdeps/posix/preadv_common.c>

libc_hidden_def (preadv64)
#ifdef __OFF_T_MATCHES_OFF64_T
strong_alias (preadv64, preadv)
libc_hidden_def (preadv)
#endif
