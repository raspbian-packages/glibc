/* Set flags signalling availability of kernel features based on given
   kernel version number.
   Copyright (C) 2006-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include_next <kernel-features.h>

/* The ARM kernel before 3.14.3 may or may not support
   futex_atomic_cmpxchg_inatomic, depending on kernel
   configuration.  */
#if __LINUX_KERNEL_VERSION < 0x020620
# undef __ASSUME_REQUEUE_PI
# undef __ASSUME_SET_ROBUST_LIST
#endif

/* Define this if your 32-bit syscall API requires 64-bit register
   pairs to start with an even-number register.  */
#define __ASSUME_ALIGNED_REGISTER_PAIRS	1
