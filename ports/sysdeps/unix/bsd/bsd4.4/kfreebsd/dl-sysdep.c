/* Operating system support for run-time dynamic linker.  GNU/kFreeBSD version.
   Copyright (C) 1995-2013 Free Software Foundation, Inc.
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

/* We conditionalize the whole of this file rather than simply eliding it
   from the static build, because other sysdeps/ versions of this file
   might define things needed by a static build.  */

#include <stdlib.h>
#include <sys/sysctl.h>

#ifdef SHARED

#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <libintl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <ldsodefs.h>
#include <_itoa.h>
#include <fpu_control.h>

#include <entry.h>
#include <dl-machine.h>
#include <dl-procinfo.h>
#include <dl-osinfo.h>
#include <hp-timing.h>
#include <tls.h>

extern char **_environ attribute_hidden;
extern char _end[] attribute_hidden;

/* Protect SUID program against misuse of file descriptors.  */
extern void __libc_check_standard_fds (void);

#ifdef NEED_DL_BASE_ADDR
ElfW(Addr) _dl_base_addr;
#endif
int __libc_enable_secure attribute_relro = 0;
INTVARDEF(__libc_enable_secure)
int __libc_multiple_libcs = 0;	/* Defining this here avoids the inclusion
				   of init-first.  */
/* This variable contains the lowest stack address ever used.  */
void *__libc_stack_end attribute_relro = NULL;
rtld_hidden_data_def(__libc_stack_end)
void *_dl_random attribute_relro = NULL;

#ifndef DL_FIND_ARG_COMPONENTS
# define DL_FIND_ARG_COMPONENTS(cookie, argc, argv, envp, auxp)	\
  do {									      \
    void **_tmp;							      \
    (argc) = *(long int *) cookie;					      \
    (argv) = (char **) ((long int *) cookie + 1);			      \
    (envp) = (argv) + (argc) + 1;					      \
    for (_tmp = (void **) (envp); *_tmp; ++_tmp)			      \
      continue;								      \
    (auxp) = (void *) ++_tmp;						      \
  } while (0)
#endif

#ifndef DL_STACK_END
# define DL_STACK_END(cookie) ((void *) (cookie))
#endif


extern const char *_self_program_name_from_auxv attribute_hidden;

ElfW(Addr)
_dl_sysdep_start (void **start_argptr,
		  void (*dl_main) (const ElfW(Phdr) *phdr, ElfW(Word) phnum,
				   ElfW(Addr) *user_entry, ElfW(auxv_t) *auxv))
{
  const ElfW(Phdr) *phdr = NULL;
  ElfW(Word) phnum = 0;
  ElfW(Addr) user_entry;
  ElfW(auxv_t) *av;
  uid_t uid = 0;
  gid_t gid = 0;
#ifdef NEED_DL_SYSINFO
  uintptr_t new_sysinfo = 0;
#endif

  __libc_stack_end = DL_STACK_END (start_argptr);
  DL_FIND_ARG_COMPONENTS (start_argptr, _dl_argc, INTUSE(_dl_argv), _environ,
			  GLRO(dl_auxv));

  user_entry = (ElfW(Addr)) ENTRY_POINT;
  GLRO(dl_platform) = NULL; /* Default to nothing known about the platform.  */

  for (av = GLRO(dl_auxv); av->a_type != AT_NULL; av++)
    switch (av->a_type)
      {
      case AT_PHDR:
	phdr = (void *) av->a_un.a_val;
	break;
      case AT_PHNUM:
	phnum = av->a_un.a_val;
	break;
      case AT_PAGESZ:
	GLRO(dl_pagesize) = av->a_un.a_val;
	break;
      case AT_ENTRY:
	user_entry = av->a_un.a_val;
	break;
      case AT_EXECPATH:
	_self_program_name_from_auxv = (char *) av->a_un.a_val;
	break;
#ifdef NEED_DL_BASE_ADDR
      case AT_BASE:
	_dl_base_addr = av->a_un.a_val;
	break;
#endif
      }

    {
      /* Fill in the values we have not gotten from the kernel through the
	 auxiliary vector.  */
#  undef SEE
#  define SEE(UID, var, uid) \
   var ^= __get##uid ()
      SEE (UID, uid, uid);
      SEE (EUID, uid, euid);
      SEE (GID, gid, gid);
      SEE (EGID, gid, egid);

      /* If one of the two pairs of IDs does not match this is a setuid
	 or setgid run.  */
      INTUSE(__libc_enable_secure) = uid | gid;
    }

#ifndef HAVE_AUX_PAGESIZE
  if (GLRO(dl_pagesize) == 0)
    GLRO(dl_pagesize) = __getpagesize ();
#endif

#if defined NEED_DL_SYSINFO
  /* Only set the sysinfo value if we also have the vsyscall DSO.  */
  if (GLRO(dl_sysinfo_dso) != 0 && new_sysinfo)
    GLRO(dl_sysinfo) = new_sysinfo;
#endif

#ifdef DL_SYSDEP_INIT
  DL_SYSDEP_INIT;
#endif

#ifdef DL_PLATFORM_INIT
  DL_PLATFORM_INIT;
#endif

  /* Determine the length of the platform name.  */
  if (GLRO(dl_platform) != NULL)
    GLRO(dl_platformlen) = strlen (GLRO(dl_platform));

  if (__sbrk (0) == _end)
    /* The dynamic linker was run as a program, and so the initial break
       starts just after our bss, at &_end.  The malloc in dl-minimal.c
       will consume the rest of this page, so tell the kernel to move the
       break up that far.  When the user program examines its break, it
       will see this new value and not clobber our data.  */
    __sbrk (GLRO(dl_pagesize)
	    - ((_end - (char *) 0) & (GLRO(dl_pagesize) - 1)));

  /* If this is a SUID program we make sure that FDs 0, 1, and 2 are
     allocated.  If necessary we are doing it ourself.  If it is not
     possible we stop the program.  */
  if (__builtin_expect (INTUSE(__libc_enable_secure), 0))
    __libc_check_standard_fds ();

  (*dl_main) (phdr, phnum, &user_entry, GLRO(dl_auxv));
  return user_entry;
}

void
internal_function
_dl_show_auxv (void)
{
  char buf[64];
  ElfW(auxv_t) *av;

  /* Terminate string.  */
  buf[63] = '\0';

  /* The following code assumes that the AT_* values are encoded
     starting from 0 with AT_NULL, 1 for AT_IGNORE, and all other values
     close by (otherwise the array will be too large).  In case we have
     to support a platform where these requirements are not fulfilled
     some alternative implementation has to be used.  */
  for (av = GLRO(dl_auxv); av->a_type != AT_NULL; ++av)
    {
      static const struct
      {
	const char label[17];
	enum { unknown = 0, dec, hex, str, ignore } form : 8;
      } auxvars[] =
	{
	  [AT_EXECFD - 2] =		{ "EXECFD:       ", dec },
	  [AT_PHDR - 2] =		{ "PHDR:         0x", hex },
	  [AT_PHENT - 2] =		{ "PHENT:        ", dec },
	  [AT_PHNUM - 2] =		{ "PHNUM:        ", dec },
	  [AT_PAGESZ - 2] =		{ "PAGESZ:       ", dec },
	  [AT_BASE - 2] =		{ "BASE:         0x", hex },
	  [AT_FLAGS - 2] =		{ "FLAGS:        0x", hex },
	  [AT_ENTRY - 2] =		{ "ENTRY:        0x", hex },
#ifndef __powerpc__
	  /* For some odd reason these are not in sys/powerpc/include/elf.h.  */
	  [AT_NOTELF - 2] =		{ "NOTELF:       ", hex },
	  [AT_UID - 2] =		{ "UID:          ", dec },
	  [AT_EUID - 2] =		{ "EUID:         ", dec },
	  [AT_GID - 2] =		{ "GID:          ", dec },
	  [AT_EGID - 2] =		{ "EGID:         ", dec },
#endif
	  [AT_EXECPATH - 2] =		{ "EXECPATH:     ", str },
	  [AT_CANARY - 2] =		{ "CANARY:       0x", hex },
	  [AT_CANARYLEN - 2] =		{ "CANARYLEN:    ", dec },
	  [AT_OSRELDATE - 2] =		{ "OSRELDATE:    ", dec },
	  [AT_NCPUS - 2] =		{ "NCPUS:        ", dec },
	  [AT_PAGESIZES - 2] =		{ "PAGESIZES:    0x", hex },
	  [AT_PAGESIZESLEN - 2] =	{ "PAGESIZESLEN: ", dec },
	  [AT_STACKPROT - 2] =		{ "STACKPROT:    0x", hex },
	};
      unsigned int idx = (unsigned int) (av->a_type - 2);

      if ((unsigned int) av->a_type < 2u || auxvars[idx].form == ignore)
	continue;

      assert (AT_NULL == 0);
      assert (AT_IGNORE == 1);

      if (idx < sizeof (auxvars) / sizeof (auxvars[0])
	  && auxvars[idx].form != unknown)
	{
	  const char *val = (char *) av->a_un.a_val;

	  if (__builtin_expect (auxvars[idx].form, dec) == dec)
	    val = _itoa ((unsigned long int) av->a_un.a_val,
			 buf + sizeof buf - 1, 10, 0);
	  else if (__builtin_expect (auxvars[idx].form, hex) == hex)
	    val = _itoa ((unsigned long int) av->a_un.a_val,
			 buf + sizeof buf - 1, 16, 0);

	  _dl_printf ("AT_%s%s\n", auxvars[idx].label, val);

	  continue;
	}

      /* Unknown value: print a generic line.  */
      char buf2[17];
      buf2[sizeof (buf2) - 1] = '\0';
      const char *val2 = _itoa ((unsigned long int) av->a_un.a_val,
				buf2 + sizeof buf2 - 1, 16, 0);
      const char *val =  _itoa ((unsigned long int) av->a_type,
				buf + sizeof buf - 1, 16, 0);
      _dl_printf ("AT_??? (0x%s): 0x%s\n", val, val2);
    }
}
void
internal_function
_dl_sysdep_start_cleanup (void)
{
}


#endif


int
attribute_hidden
_dl_discover_osversion (void)
{
  int request[2] = { CTL_KERN, KERN_OSRELDATE };
  size_t len;
  int version;
  int subrelease;

  len = sizeof(version);
  if (__sysctl (request, 2, &version, &len, NULL, 0) < 0)
    return -1;
    
/*
 * since 8.0 release
 * scheme is:  <major><two digit minor>Rxx
 *              'R' is in the range 0 to 4 if this is a release branch or
 *              x.0-CURRENT before RELENG_*_0 is created, otherwise 'R' is
 *              in the range 5 to 9.
 */

  /* Convert to the GLIBC versioning system */
  subrelease = version % 1000;
  if (subrelease >= 500)
      subrelease -= 300;	/* hopefull 55 values suffice for stable update */
  if (subrelease > 255)		/* it have to fit into byte */
      subrelease = 255;
  return ((version / 100000) << 16)		/* major */
	 | (((version % 100000) / 1000) << 8)   /* minor 	0 -  99 */
	 | subrelease;
}
