/* Copyright (C) 1991-2017 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <hurd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

/* Replace the current process, executing FILE_NAME with arguments ARGV and
   environment ENVP.  ARGV and ENVP are terminated by NULL pointers.  */
int
__execve (const char *file_name, char *const argv[], char *const envp[])
{
  error_t err;
  char *concat_name = NULL;
  const char *abs_name;

  file_t file = __file_name_lookup (file_name, O_EXEC, 0);
  if (file == MACH_PORT_NULL)
    return -1;

  if (file_name[0] == '/')
    {
      /* Absolute path */
      abs_name = file_name;
    }
  else
    {
      /* Relative path */
      char *cwd = __getcwd (NULL, 0);
      if (cwd == NULL)
	{
	  __mach_port_deallocate (__mach_task_self (), file);
	  return -1;
	}
      int res = __asprintf (&concat_name, "%s/%s", cwd, file_name);
      free (cwd);
      if (res == -1)
	{
	  __mach_port_deallocate (__mach_task_self (), file);
	  return -1;
	}
      abs_name = concat_name;
    }

  /* Hopefully this will not return.  */
  err = _hurd_exec_file_name (__mach_task_self (), file,
			      abs_name, argv, envp);

  /* Oh well.  Might as well be tidy.  */
  __mach_port_deallocate (__mach_task_self (), file);

  free (concat_name);
  return __hurd_fail (err);
}

weak_alias (__execve, execve)
