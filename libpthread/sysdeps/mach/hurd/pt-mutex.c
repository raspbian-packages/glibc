/* Copyright (C) 2016 Free Software Foundation, Inc.
   Contributed by Agustina Arzille <avarzille@riseup.net>, 2016.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License
   as published by the Free Software Foundation; either
   version 2 of the license, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, see
   <http://www.gnu.org/licenses/>.
*/

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <pt-internal.h>
#include <hurdlock.h>

static const pthread_mutexattr_t dfl_attr =
{
  .__prioceiling = 0,
  .__protocol = PTHREAD_PRIO_NONE,
  .__pshared = PTHREAD_PROCESS_PRIVATE,
  .__mutex_type = __PTHREAD_MUTEX_TIMED
};

int pthread_mutexattr_init (pthread_mutexattr_t *attrp)
{
  *attrp = dfl_attr;
  return (0);
}

int pthread_mutexattr_destroy (pthread_mutexattr_t *attrp)
{
  (void)attrp;
  return (0);
}

int pthread_mutexattr_settype (pthread_mutexattr_t *attrp, int type)
{
  if (type < 0 || type > __PTHREAD_MUTEX_RECURSIVE)
    return (EINVAL);

  attrp->__mutex_type = type;
  return (0);
}

int pthread_mutexattr_gettype (const pthread_mutexattr_t *attrp, int *outp)
{
  *outp = attrp->__mutex_type;
  return (0);
}

int pthread_mutexattr_setpshared (pthread_mutexattr_t *attrp, int pshared)
{
  if (pshared != PTHREAD_PROCESS_PRIVATE &&
      pshared != PTHREAD_PROCESS_SHARED)
    return (EINVAL);

  attrp->__pshared = pshared;
  return (0);
}

int pthread_mutexattr_getpshared (const pthread_mutexattr_t *attrp, int *outp)
{
  *outp = attrp->__pshared;
  return (0);
}

int pthread_mutexattr_setrobust (pthread_mutexattr_t *attrp, int robust)
{
  if (robust != PTHREAD_MUTEX_ROBUST &&
      robust != PTHREAD_MUTEX_STALLED)
    return (EINVAL);

  attrp->__prioceiling |= robust;
  return (0);
}

weak_alias (pthread_mutexattr_setrobust, pthread_mutexattr_setrobust_np)

int pthread_mutexattr_getrobust (const pthread_mutexattr_t *attrp, int *outp)
{
  *outp = (attrp->__prioceiling & PTHREAD_MUTEX_ROBUST) ?
    PTHREAD_MUTEX_ROBUST : PTHREAD_MUTEX_STALLED;
  return (0);
}

weak_alias (pthread_mutexattr_getrobust, pthread_mutexattr_getrobust_np)

int pthread_mutexattr_setprioceiling (pthread_mutexattr_t *attrp, int cl)
{
  (void)attrp; (void)cl;
  return (ENOSYS);
}

stub_warning (pthread_mutexattr_setprioceiling)

int pthread_mutexattr_getprioceiling (const pthread_mutexattr_t *ap, int *clp)
{
  (void)ap; (void)clp;
  return (ENOSYS);
}

stub_warning (pthread_mutexattr_getprioceiling)

int pthread_mutexattr_setprotocol (pthread_mutexattr_t *attrp, int proto)
{
  (void)attrp;
  return (proto == PTHREAD_PRIO_NONE ? 0 :
    proto != PTHREAD_PRIO_INHERIT &&
    proto != PTHREAD_PRIO_PROTECT ? EINVAL : ENOTSUP);
}

int pthread_mutexattr_getprotocol (const pthread_mutexattr_t *attrp, int *ptp)
{
  *ptp = attrp->__protocol;
  return (0);
}

int _pthread_mutex_init (pthread_mutex_t *mtxp,
  const pthread_mutexattr_t *attrp)
{
  if (attrp == NULL)
    attrp = &dfl_attr;

  mtxp->__flags = (attrp->__pshared == PTHREAD_PROCESS_SHARED ?
    GSYNC_SHARED : 0) | ((attrp->__prioceiling & PTHREAD_MUTEX_ROBUST) ?
      PTHREAD_MUTEX_ROBUST : 0);

  mtxp->__type = attrp->__mutex_type +
    (attrp->__mutex_type != __PTHREAD_MUTEX_TIMED);

  mtxp->__owner_id = 0;
  mtxp->__shpid = 0;
  mtxp->__cnt = 0;
  mtxp->__lock = 0;

  return (0);
}

strong_alias (_pthread_mutex_init, pthread_mutex_init)

/* Lock routines. */

/* Special ID used to signal an unrecoverable robust mutex. */
#define NOTRECOVERABLE_ID   (1U << 31)

/* Common path for robust mutexes. Assumes the variable 'ret'
 * is bound in the function this is called from. */
#define ROBUST_LOCK(self, mtxp, cb, ...)   \
  if (mtxp->__owner_id == NOTRECOVERABLE_ID)   \
    return (ENOTRECOVERABLE);   \
  else if (mtxp->__owner_id == self->thread &&   \
      __getpid () == (int)(mtxp->__lock & LLL_OWNER_MASK))   \
    {   \
      if (mtxp->__type == PT_MTX_RECURSIVE)   \
        {   \
          if (__glibc_unlikely (mtxp->__cnt + 1 == 0))   \
            return (EAGAIN);   \
          \
          ++mtxp->__cnt;   \
          return (0);   \
        }   \
      else if (mtxp->__type == PT_MTX_ERRORCHECK)   \
        return (EDEADLK);   \
    }   \
  \
  ret = cb (&mtxp->__lock, ##__VA_ARGS__);   \
  if (ret == 0 || ret == EOWNERDEAD)   \
    {   \
      if (mtxp->__owner_id == ENOTRECOVERABLE)   \
        ret = ENOTRECOVERABLE;   \
      else   \
        {   \
          mtxp->__owner_id = self->thread;   \
          mtxp->__cnt = 1;   \
          if (ret == EOWNERDEAD)   \
            {   \
              mtxp->__lock = mtxp->__lock | LLL_DEAD_OWNER;   \
              atomic_write_barrier ();   \
            }   \
        }   \
    }   \
  (void)0

/* Check that a thread owns the mutex. For non-robust, task-shared
 * objects, we have to check the thread *and* process-id. */
#define mtx_owned_p(mtx, pt, flags)   \
  ((mtx)->__owner_id == (pt)->thread &&   \
    (((flags) & GSYNC_SHARED) == 0 ||   \
      (mtx)->__shpid == __getpid ()))

/* Record a thread as the owner of the mutex. */
#define mtx_set_owner(mtx, pt, flags)   \
  (void)   \
    ({   \
       (mtx)->__owner_id = (pt)->thread;   \
       if ((flags) & GSYNC_SHARED)   \
         (mtx)->__shpid = __getpid ();   \
     })

/* Redefined mutex types. The +1 is for binary compatibility. */
#define PT_MTX_NORMAL       __PTHREAD_MUTEX_TIMED
#define PT_MTX_RECURSIVE    (__PTHREAD_MUTEX_RECURSIVE + 1)
#define PT_MTX_ERRORCHECK   (__PTHREAD_MUTEX_ERRORCHECK + 1)

/* Mutex type, including robustness. */
#define MTX_TYPE(mtxp)   \
  ((mtxp)->__type | ((mtxp)->__flags & PTHREAD_MUTEX_ROBUST))

extern int __getpid (void) __attribute__ ((const));

int __pthread_mutex_lock (pthread_mutex_t *mtxp)
{
  struct __pthread *self;
  int flags = mtxp->__flags & GSYNC_SHARED;
  int ret = 0;

  switch (MTX_TYPE (mtxp))
    {
      case PT_MTX_NORMAL:
        lll_lock (&mtxp->__lock, flags);
        break;

      case PT_MTX_RECURSIVE:
        self = _pthread_self ();
        if (mtx_owned_p (mtxp, self, flags))
          {
            if (__glibc_unlikely (mtxp->__cnt + 1 == 0))
              return (EAGAIN);

            ++mtxp->__cnt;
            return (ret);
          }

        lll_lock (&mtxp->__lock, flags);
        mtx_set_owner (mtxp, self, flags);
        mtxp->__cnt = 1;
        break;

      case PT_MTX_ERRORCHECK:
        self = _pthread_self ();
        if (mtx_owned_p (mtxp, self, flags))
          return (EDEADLK);

        lll_lock (&mtxp->__lock, flags);
        mtx_set_owner (mtxp, self, flags);
        break;

      case PT_MTX_NORMAL     | PTHREAD_MUTEX_ROBUST:
      case PT_MTX_RECURSIVE  | PTHREAD_MUTEX_ROBUST:
      case PT_MTX_ERRORCHECK | PTHREAD_MUTEX_ROBUST:
        self = _pthread_self ();
        ROBUST_LOCK (self, mtxp, lll_robust_lock, flags);
        break;

      default:
        ret = EINVAL;
        break;
    }

  return (ret);
}

strong_alias (__pthread_mutex_lock, _pthread_mutex_lock)
strong_alias (__pthread_mutex_lock, pthread_mutex_lock)

int __pthread_mutex_trylock (pthread_mutex_t *mtxp)
{
  struct __pthread *self;
  int ret;

  switch (MTX_TYPE (mtxp))
    {
      case PT_MTX_NORMAL:
        ret = lll_trylock (&mtxp->__lock) == 0 ? 0 : EBUSY;
        break;

      case PT_MTX_RECURSIVE:
        self = _pthread_self ();
        if (mtx_owned_p (mtxp, self, mtxp->__flags))
          {
            if (__glibc_unlikely (mtxp->__cnt + 1 == 0))
              return (EAGAIN);

            ++mtxp->__cnt;
            ret = 0;
          }
        else if ((ret = lll_trylock (&mtxp->__lock)) == 0)
          {
            mtx_set_owner (mtxp, self, mtxp->__flags);
            mtxp->__cnt = 1;
          }
        else
          ret = EBUSY;

        break;

      case PT_MTX_ERRORCHECK:
        self = _pthread_self ();
        if ((ret = lll_trylock (&mtxp->__lock)) == 0)
          mtx_set_owner (mtxp, self, mtxp->__flags);
        else
          ret = EBUSY;
        break;

      case PT_MTX_NORMAL     | PTHREAD_MUTEX_ROBUST:
      case PT_MTX_RECURSIVE  | PTHREAD_MUTEX_ROBUST:
      case PT_MTX_ERRORCHECK | PTHREAD_MUTEX_ROBUST:
        self = _pthread_self ();
        ROBUST_LOCK (self, mtxp, lll_robust_trylock);
        break;

      default:
        ret = EINVAL;
        break;
    }

  return (ret);
}

strong_alias (__pthread_mutex_trylock, _pthread_mutex_trylock)
strong_alias (__pthread_mutex_trylock, pthread_mutex_trylock)

int pthread_mutex_timedlock (pthread_mutex_t *mtxp,
  const struct timespec *tsp)
{
  struct __pthread *self;
  int ret, flags = mtxp->__flags & GSYNC_SHARED;

  switch (MTX_TYPE (mtxp))
    {
      case PT_MTX_NORMAL:
        ret = lll_abstimed_lock (&mtxp->__lock, tsp, flags);
        break;

      case PT_MTX_RECURSIVE:
        self = _pthread_self ();
        if (mtx_owned_p (mtxp, self, flags))
          {
            if (__glibc_unlikely (mtxp->__cnt + 1 == 0))
              return (EAGAIN);

            ++mtxp->__cnt;
            ret = 0;
          }
        else if ((ret = lll_abstimed_lock (&mtxp->__lock,
            tsp, flags)) == 0)
          {
            mtx_set_owner (mtxp, self, flags);
            mtxp->__cnt = 1;
          }

        break;

      case PT_MTX_ERRORCHECK:
        self = _pthread_self ();
        if (mtx_owned_p (mtxp, self, flags))
          ret = EDEADLK;
        else if ((ret = lll_abstimed_lock (&mtxp->__lock,
            tsp, flags)) == 0)
          mtx_set_owner (mtxp, self, flags);

        break;

      case PT_MTX_NORMAL     | PTHREAD_MUTEX_ROBUST:
      case PT_MTX_RECURSIVE  | PTHREAD_MUTEX_ROBUST:
      case PT_MTX_ERRORCHECK | PTHREAD_MUTEX_ROBUST:
        self = _pthread_self ();
        ROBUST_LOCK (self, mtxp, lll_robust_abstimed_lock, tsp, flags);
        break;

      default:
        ret = EINVAL;
        break;
    }

  return (ret);
}

int __pthread_mutex_unlock (pthread_mutex_t *mtxp)
{
  struct __pthread *self;
  int ret = 0, flags = mtxp->__flags & GSYNC_SHARED;

  switch (MTX_TYPE (mtxp))
    {
      case PT_MTX_NORMAL:
        lll_unlock (&mtxp->__lock, flags);
        break;

      case PT_MTX_RECURSIVE:
        self = _pthread_self ();
        if (!mtx_owned_p (mtxp, self, flags))
          ret = EPERM;
        else if (--mtxp->__cnt == 0)
          {
            mtxp->__owner_id = mtxp->__shpid = 0;
            lll_unlock (&mtxp->__lock, flags);
          }

        break;

      case PT_MTX_ERRORCHECK:
        self = _pthread_self ();
        if (!mtx_owned_p (mtxp, self, flags))
          ret = EPERM;
        else
          {
            mtxp->__owner_id = mtxp->__shpid = 0;
            lll_unlock (&mtxp->__lock, flags);
          }

        break;

      case PT_MTX_NORMAL     | PTHREAD_MUTEX_ROBUST:
      case PT_MTX_RECURSIVE  | PTHREAD_MUTEX_ROBUST:
      case PT_MTX_ERRORCHECK | PTHREAD_MUTEX_ROBUST:
        self = _pthread_self ();
        if (mtxp->__owner_id == NOTRECOVERABLE_ID)
          ;   /* Nothing to do. */
        else if (mtxp->__owner_id != self->thread ||
            (int)(mtxp->__lock & LLL_OWNER_MASK) != __getpid ())
          ret = EPERM;
        else if (--mtxp->__cnt == 0)
          {
            /* Release the lock. If it's in an inconsistent
             * state, mark it as irrecoverable. */
            mtxp->__owner_id = (mtxp->__lock & LLL_DEAD_OWNER) ?
              NOTRECOVERABLE_ID : 0;
            lll_robust_unlock (&mtxp->__lock, flags);
          }

        break;

      default:
        ret = EINVAL;
        break;
    }

  return (ret);
}

strong_alias (__pthread_mutex_unlock, _pthread_mutex_unlock)
strong_alias (__pthread_mutex_unlock, pthread_mutex_unlock)

int pthread_mutex_consistent (pthread_mutex_t *mtxp)
{
  int ret = EINVAL;
  unsigned int val = mtxp->__lock;

  if ((mtxp->__flags & PTHREAD_MUTEX_ROBUST) != 0 &&
      (val & LLL_DEAD_OWNER) != 0 &&
      atomic_compare_and_exchange_bool_acq (&mtxp->__lock,
        __getpid () | LLL_WAITERS, val) == 0)
    {
      /* The mutex is now ours, and it's consistent. */
      mtxp->__owner_id = _pthread_self()->thread;
      mtxp->__cnt = 1;
      ret = 0;
    }

  return (ret);
}

weak_alias (pthread_mutex_consistent, pthread_mutex_consistent_np)

int __pthread_mutex_transfer_np (pthread_mutex_t *mtxp, pthread_t th)
{
  struct __pthread *self = _pthread_self ();
  struct __pthread *pt = __pthread_getid (th);

  if (!pt)
    return (ESRCH);
  else if (pt == self)
    return (0);

  int ret = 0;
  int flags = mtxp->__flags & GSYNC_SHARED;

  switch (MTX_TYPE (mtxp))
    {
      case PT_MTX_NORMAL:
        break;

      case PT_MTX_RECURSIVE:
      case PT_MTX_ERRORCHECK:
        if (!mtx_owned_p (mtxp, self, flags))
          ret = EPERM;
        else
          mtx_set_owner (mtxp, pt, flags);

        break;

      case PT_MTX_NORMAL     | PTHREAD_MUTEX_ROBUST:
      case PT_MTX_RECURSIVE  | PTHREAD_MUTEX_ROBUST:
      case PT_MTX_ERRORCHECK | PTHREAD_MUTEX_ROBUST:
        /* Note that this can be used to transfer an inconsistent
         * mutex as well. The new owner will still have the same
         * flags as the original. */
        if (mtxp->__owner_id != self->thread ||
            (int)(mtxp->__lock & LLL_OWNER_MASK) != __getpid ())
          ret = EPERM;
        else
          mtxp->__owner_id = pt->thread;

        break;

      default:
        ret = EINVAL;
    }

  return (ret);
}

strong_alias (__pthread_mutex_transfer_np, pthread_mutex_transfer_np)

int pthread_mutex_setprioceiling (pthread_mutex_t *mtxp, int cl, int *prp)
{
  (void)mtxp; (void)cl; (void)prp;
  return (ENOSYS);
}

stub_warning (pthread_mutex_setprioceiling)

int pthread_mutex_getprioceiling (const pthread_mutex_t *mtxp, int *clp)
{
  (void)mtxp; (void)clp;
  return (ENOSYS);
}

stub_warning (pthread_mutex_getprioceiling)

int _pthread_mutex_destroy (pthread_mutex_t *mtxp)
{
  atomic_read_barrier ();
  if (*(volatile unsigned int *)&mtxp->__lock != 0)
    return (EBUSY);

  mtxp->__type = -1;
  return (0);
}

strong_alias (_pthread_mutex_destroy, pthread_mutex_destroy)

