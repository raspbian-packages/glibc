/* libresolv interfaces for internal use across glibc.
   Copyright (C) 2016-2017 Free Software Foundation, Inc.
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

#ifndef _RESOLV_INTERNAL_H
#define _RESOLV_INTERNAL_H 1

#include <resolv.h>
#include <stdbool.h>

/* Internal version of RES_USE_INET6 which does not trigger a
   deprecation warning.  */
#define DEPRECATED_RES_USE_INET6 0x00002000

static inline bool
res_use_inet6 (void)
{
  return _res.options & DEPRECATED_RES_USE_INET6;
}

enum
  {
    /* The advertized EDNS buffer size.  The value 1200 is derived
       from the IPv6 minimum MTU (1280 bytes) minus some arbitrary
       space for tunneling overhead.  If the DNS server does not react
       to ICMP Fragmentation Needed But DF Set messages, this should
       avoid all UDP fragments on current networks.  Avoiding UDP
       fragments is desirable because it prevents fragmentation-based
       spoofing attacks because the randomness in a DNS packet is
       concentrated in the first fragment (with the headers) and does
       not protect subsequent fragments.  */
    RESOLV_EDNS_BUFFER_SIZE = 1200,
  };

/* Add an OPT record to a DNS query.  */
int __res_nopt (res_state, int n0, unsigned char *buf, int buflen,
                int anslen) attribute_hidden;

#endif  /* _RESOLV_INTERNAL_H */
