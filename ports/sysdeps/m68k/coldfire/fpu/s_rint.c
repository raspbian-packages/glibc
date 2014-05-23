/* Copyright (C) 2006-2014 Free Software Foundation, Inc.
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

double
__rint (double x)
{
  asm ("fint.d %1,%0" : "=f" (x) : "fm" (x));
  return x;
}
weak_alias (__rint, rint)
#ifdef NO_LONG_DOUBLE
strong_alias (__rint, __rintl)
weak_alias (__rint, rintl)
#endif
