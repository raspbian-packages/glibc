/* Copyright (C) 1993-2016 Free Software Foundation, Inc.
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

/* Useful declarations and support functions for MiG-generated stubs.  */

#ifndef	_MACH_MIG_SUPPORT_H

#define	_MACH_MIG_SUPPORT_H	1

#include <mach/std_types.h>
#include <mach/message.h>
#include <sys/types.h>
#include <string.h>

#ifndef __USE_GNU
/* The only problem that has come up so far is __stpncpy being undeclared
   below because <string.h> doesn't declare it without __USE_GNU.  We could
   work around that problem by just adding the declaration there, or by
   eliding the inline functions in the absence of __USE_GNU.  But either of
   these would result in unoptimized calls (because no inline version of
   __stpncpy will have been defined), and there may be other niggling
   problems lurking.  Instead we simply insist on _GNU_SOURCE for
   compiling mig output; anyway, that better reflects the fact that mig
   output requires nonstandard special support code not found elsewhere.  */
# error mig stubs must be compiled with -D_GNU_SOURCE
#endif

/* MiG initialization.  */
extern void __mig_init (void *__first);
extern void mig_init (void *__first);

/* Shorthand functions for vm_allocate and vm_deallocate on
   mach_task_self () (and with ANYWHERE=1).  */
extern void __mig_allocate (vm_address_t *__addr_p, vm_size_t __size);
extern void mig_allocate (vm_address_t *__addr_p, vm_size_t __size);
extern void __mig_deallocate (vm_address_t __addr, vm_size_t __size);
extern void mig_deallocate (vm_address_t __addr, vm_size_t __size);

/* Reply-port management support functions.  */
extern void __mig_dealloc_reply_port (mach_port_t);
extern void mig_dealloc_reply_port (mach_port_t);
extern mach_port_t __mig_get_reply_port (void);
extern mach_port_t mig_get_reply_port (void);
extern void __mig_put_reply_port (mach_port_t);
extern void mig_put_reply_port (mach_port_t);

extern void __mig_reply_setup (const mach_msg_header_t *__request,
			       mach_msg_header_t *__reply);
extern void mig_reply_setup (const mach_msg_header_t *__request,
			     mach_msg_header_t *__reply);

/* Idiocy support function.  */
extern vm_size_t mig_strncpy (char *__dst, const char *__src, vm_size_t __len);
extern vm_size_t __mig_strncpy (char *__dst, const char *__src, vm_size_t);

#if defined __USE_EXTERN_INLINES && defined _LIBC
__extern_inline vm_size_t
__mig_strncpy (char *__dst, const char *__src, vm_size_t __len)
{
  return __stpncpy (__dst, __src, __len) - __dst;
}
__extern_inline vm_size_t
mig_strncpy (char *__dst, const char *__src, vm_size_t __len)
{
  return __mig_strncpy (__dst, __src, __len);
}
#endif



#endif	/* mach/mig_support.h */
