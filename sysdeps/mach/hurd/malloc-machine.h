/* Basic platform-independent macro definitions for mutexes,
   thread-specific data and parameters for malloc.
   Copyright (C) 2003-2016 Free Software Foundation, Inc.
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

#ifndef _MALLOC_MACHINE_H
#define _MALLOC_MACHINE_H

#include <libc-lock.h>
#include <mach/lock-intern.h>

#undef mutex_t
#define mutex_t unsigned int

#undef MUTEX_INITIALIZER
#define MUTEX_INITIALIZER   LLL_INITIALIZER

#undef mutex_init
#define mutex_init(m) ({ __mutex_init(m); 0; })

#undef mutex_lock
#define mutex_lock(m) ({ __mutex_lock(m); 0; })

#undef mutex_unlock
#define mutex_unlock(m) ({ __mutex_unlock(m); 0; })

#define mutex_trylock(m) (!__mutex_trylock(m))

/* No we're *not* using pthreads.  */
#define __pthread_initialize ((void (*)(void))0)

/* madvise is a stub on Hurd, so don't bother calling it.  */

#include <sys/mman.h>

#undef	__madvise
#define __madvise(addr, len, advice)	\
  ((void) (addr), (void) (len), (void) (advice))

#include <sysdeps/generic/malloc-machine.h>

#endif /* !defined(_MALLOC_MACHINE_H) */
