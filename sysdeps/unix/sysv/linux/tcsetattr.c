/* Copyright (C) 1993-2022 Free Software Foundation, Inc.
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

#include <errno.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sysdep.h>

/* The difference here is that the termios structure used in the
   kernel is not the same as we use in the libc.  Therefore we must
   translate it here.  */
#include <kernel_termios.h>


/* This is a gross hack around a kernel bug.  If the cfsetispeed functions
   is called with the SPEED argument set to zero this means use the same
   speed as for output.  But we don't have independent input and output
   speeds and therefore cannot record this.

   We use an unused bit in the `c_iflag' field to keep track of this
   use of `cfsetispeed'.  The value here must correspond to the one used
   in `speed.c'.  */
#define IBAUD0	020000000000


/* Set the state of FD to *TERMIOS_P.  */
int
__tcsetattr (int fd, int optional_actions, const struct termios *termios_p)
{
  struct __kernel_termios k_termios;
  struct __kernel_termios k_termios_old;
  unsigned long int cmd;
  int retval, old_retval;

  /* Preserve the previous termios state if we can. */
  old_retval = INLINE_SYSCALL (ioctl, 3, fd, TCGETS, &k_termios_old);

  switch (optional_actions)
    {
    case TCSANOW:
      cmd = TCSETS;
      break;
    case TCSADRAIN:
      cmd = TCSETSW;
      break;
    case TCSAFLUSH:
      cmd = TCSETSF;
      break;
    default:
      return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);
    }

  k_termios.c_iflag = termios_p->c_iflag & ~IBAUD0;
  k_termios.c_oflag = termios_p->c_oflag;
  k_termios.c_cflag = termios_p->c_cflag;
  k_termios.c_lflag = termios_p->c_lflag;
  k_termios.c_line = termios_p->c_line;
#if _HAVE_C_ISPEED && _HAVE_STRUCT_TERMIOS_C_ISPEED
  k_termios.c_ispeed = termios_p->c_ispeed;
#endif
#if _HAVE_C_OSPEED && _HAVE_STRUCT_TERMIOS_C_OSPEED
  k_termios.c_ospeed = termios_p->c_ospeed;
#endif
  memcpy (&k_termios.c_cc[0], &termios_p->c_cc[0],
	  __KERNEL_NCCS * sizeof (cc_t));

  retval = INLINE_SYSCALL (ioctl, 3, fd, cmd, &k_termios);

  /* The Linux kernel silently ignores the invalid c_cflag on pty.
     We have to check it here, and return an error.  But if some other
     setting was successfully changed, POSIX requires us to report
     success. */
  if ((retval == 0) && (old_retval == 0))
    {
      int save = errno;
      retval = INLINE_SYSCALL (ioctl, 3, fd, TCGETS, &k_termios);
      if (retval)
	{
	  /* We cannot verify if the setting is ok. We don't return
	     an error (?). */
	  __set_errno (save);
	  retval = 0;
	}
      else if ((k_termios_old.c_oflag != k_termios.c_oflag) ||
	       (k_termios_old.c_lflag != k_termios.c_lflag) ||
	       (k_termios_old.c_line != k_termios.c_line) ||
	       ((k_termios_old.c_iflag | IBAUD0) != (k_termios.c_iflag | IBAUD0)))
	{
	  /* Some other setting was successfully changed, which
	     means we should not return an error. */
	  __set_errno (save);
	  retval = 0;
	}
      else if ((k_termios_old.c_cflag | (PARENB & CREAD & CSIZE)) !=
	       (k_termios.c_cflag | (PARENB & CREAD & CSIZE)))
	{
	  /* Some other c_cflag setting was successfully changed, which
	     means we should not return an error. */
	  __set_errno (save);
	  retval = 0;
	}
      else if ((termios_p->c_cflag & (PARENB | CREAD))
			!= (k_termios.c_cflag & (PARENB | CREAD))
	       || ((termios_p->c_cflag & CSIZE)
		   && (termios_p->c_cflag & CSIZE)
			!= (k_termios.c_cflag & CSIZE)))
	{
	  /* It looks like the Linux kernel silently changed the
	     PARENB/CREAD/CSIZE bits in c_cflag. Report it as an
	     error. */
	  __set_errno (EINVAL);
	  retval = -1;
	}
    }
   return retval;
}
weak_alias (__tcsetattr, tcsetattr)
libc_hidden_def (tcsetattr)
