/* Copyright (C) 1999-2016 Free Software Foundation, Inc.
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

#ifndef _HURD_LOCK_H
#define _HURD_LOCK_H   1

#include <mach/lowlevellock.h>

struct timespec;

/* Flags for robust locks. */
#define LLL_WAITERS      (1U << 31)
#define LLL_DEAD_OWNER   (1U << 30)

#define LLL_OWNER_MASK   ~(LLL_WAITERS | LLL_DEAD_OWNER)

/* Wait on 64-bit address PTR, without blocking if its contents
 * are different from the pair <LO, HI>. */
extern int lll_xwait (void *__ptr, int __lo,
  int __hi, int __flags);

/* Same as 'lll_wait', but only block for MLSEC milliseconds. */
extern int lll_timed_wait (void *__ptr, int __val,
  int __mlsec, int __flags);

/* Same as 'lll_xwait', but only block for MLSEC milliseconds. */
extern int lll_timed_xwait (void *__ptr, int __lo,
  int __hi, int __mlsec, int __flags);

/* Same as 'lll_wait', but only block until TSP elapses,
 * using clock CLK. */
extern int __lll_abstimed_wait (void *__ptr, int __val,
  const struct timespec *__tsp, int __flags, int __clk);

/* Same as 'lll_xwait', but only block until TSP elapses,
 * using clock CLK. */
extern int __lll_abstimed_xwait (void *__ptr, int __lo, int __hi,
  const struct timespec *__tsp, int __flags, int __clk);

/* Same as 'lll_lock', but return with an error if TSP elapses,
 * using clock CLK. */
extern int __lll_abstimed_lock (void *__ptr,
  const struct timespec *__tsp, int __flags, int __clk);

/* Acquire the lock at PTR, but return with an error if
 * the process containing the owner thread dies. */
extern int lll_robust_lock (void *__ptr, int __flags);

/* Same as 'lll_robust_lock', but only block until TSP
 * elapses, using clock CLK. */
extern int __lll_robust_abstimed_lock (void *__ptr,
  const struct timespec *__tsp, int __flags, int __clk);

/* Same as 'lll_robust_lock', but return with an error
 * if the lock cannot be acquired without blocking. */
extern int lll_robust_trylock (void *__ptr);

/* Wake one or more threads waiting on address PTR,
 * setting its value to VAL before doing so. */
extern void lll_set_wake (void *__ptr, int __val, int __flags);

/* Release the robust lock at PTR. */
extern void lll_robust_unlock (void *__ptr, int __flags);

/* Rearrange threads waiting on address SRC to instead wait on
 * DST, waking one of them if WAIT_ONE is non-zero. */
extern void lll_requeue (void *__src, void *__dst,
  int __wake_one, int __flags);

/* The following are hacks that allow us to simulate optional
 * parameters in C, to avoid having to pass the clock id for
 * every one of these calls, defaulting to CLOCK_REALTIME if
 * no argument is passed. */

#define lll_abstimed_wait(ptr, val, tsp, flags, ...)   \
  ({   \
     const clockid_t __clk[] = { CLOCK_REALTIME, ##__VA_ARGS__ };   \
     __lll_abstimed_wait ((ptr), (val), (tsp), (flags),   \
       __clk[sizeof (__clk) / sizeof (__clk[0]) - 1]);   \
   })

#define lll_abstimed_xwait(ptr, lo, hi, tsp, flags, ...)   \
  ({   \
     const clockid_t __clk[] = { CLOCK_REALTIME, ##__VA_ARGS__ };   \
     __lll_abstimed_xwait ((ptr), (lo), (hi), (tsp), (flags),   \
       __clk[sizeof (__clk) / sizeof (__clk[0]) - 1]);   \
   })

#define lll_abstimed_lock(ptr, tsp, flags, ...)   \
  ({   \
     const clockid_t __clk[] = { CLOCK_REALTIME, ##__VA_ARGS__ };   \
     __lll_abstimed_lock ((ptr), (tsp), (flags),   \
       __clk[sizeof (__clk) / sizeof (__clk[0]) - 1]);   \
   })

#define lll_robust_abstimed_lock(ptr, tsp, flags, ...)   \
  ({   \
     const clockid_t __clk[] = { CLOCK_REALTIME, ##__VA_ARGS__ };   \
     __lll_robust_abstimed_lock ((ptr), (tsp), (flags),   \
       __clk[sizeof (__clk) / sizeof (__clk[0]) - 1]);   \
   })

#endif
