/* Copyright (C) 1991-2019 Free Software Foundation, Inc.
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

#include <libio/libioP.h>


/* Write formatted output into S, according to the format
   string FORMAT, writing no more than MAXLEN characters.  */
int
___vsnprintf_chk (char *s, size_t maxlen, int flag, size_t slen,
		  const char *format, va_list ap)
{
  if (__glibc_unlikely (slen < maxlen))
    __chk_fail ();

  /* For flag > 0 (i.e. __USE_FORTIFY_LEVEL > 1) request that %n
     can only come from read-only format strings.  */
  unsigned int mode = (flag > 0) ? PRINTF_FORTIFY : 0;

  return __vsnprintf_internal (s, maxlen, format, ap, mode);
}
ldbl_strong_alias (___vsnprintf_chk, __vsnprintf_chk)
