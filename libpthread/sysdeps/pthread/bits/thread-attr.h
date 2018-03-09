/* Thread attribute type.  Generic version.
   Copyright (C) 2000, 2002, 2008 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _BITS_THREAD_ATTR_H
#define _BITS_THREAD_ATTR_H	1

#define __need_schedparam
#include <bits/sched.h>

#define __need_size_t
#include <stddef.h>

enum __pthread_detachstate;
enum __pthread_inheritsched;
enum __pthread_contentionscope;

/* This structure describes the attributes of a POSIX thread.  Note
   that not all of them are supported on all systems.  */
struct __pthread_attr
{
  struct __sched_param __schedparam;
  void *__stackaddr;
  size_t __stacksize;
  size_t __guardsize;
  enum __pthread_detachstate __detachstate;
  enum __pthread_inheritsched __inheritsched;
  enum __pthread_contentionscope __contentionscope;
  int __schedpolicy;
};

#endif /* bits/thread-attr.h */
