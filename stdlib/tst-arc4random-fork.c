/* Test that subprocesses generate distinct streams of randomness.
   Copyright (C) 2022-2023 Free Software Foundation, Inc.
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

/* Collect random data from subprocesses and check that all the
   results are unique.  */

#include <array_length.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <support/check.h>
#include <support/support.h>
#include <support/xthread.h>
#include <support/xunistd.h>
#include <unistd.h>

/* Perform multiple runs.  The subsequent runs start with an
   already-initialized random number generator.  (The number 1500 was
   seen to reproduce failures reliable in case of a race condition in
   the fork detection code.)  */
enum { runs = 1500 };

/* One hundred processes in total.  This should be high enough to
   expose any issues, but low enough not to tax the overall system too
   much.  */
enum { subprocesses = 49 };

/* The total number of processes.  */
enum { processes = subprocesses + 1 };

/* Number of bytes of randomness to generate per process.  Large
   enough to make false positive duplicates extremely unlikely.  */
enum { random_size = 16 };

/* Generated bytes of randomness.  */
struct result
{
  unsigned char bytes[random_size];
};

/* Shared across all processes.  */
static struct shared_data
{
  pthread_barrier_t barrier;
  struct result results[runs][processes];
} *shared_data;

static void
generate_arc4random (unsigned char *bytes)
{
  for (int i = 0; i < random_size / sizeof (uint32_t); i++)
    {
      uint32_t x = arc4random ();
      memcpy (&bytes[4 * i], &x, sizeof x);
    }
}

static void
generate_arc4random_buf (unsigned char *bytes)
{
  arc4random_buf (bytes, random_size);
}

static void
generate_arc4random_uniform (unsigned char *bytes)
{
  for (int i = 0; i < random_size; i++)
    bytes[i] = arc4random_uniform (256);
}

/* Invoked to collect data from a subprocess.  */
static void
subprocess (int run, int process_index, void (*func)(unsigned char *))
{
  xpthread_barrier_wait (&shared_data->barrier);
  func (shared_data->results[run][process_index].bytes);
}

/* Used to sort the results.  */
struct index
{
  int run;
  int process_index;
};

/* Used to sort an array of struct index values.  */
static int
index_compare (const void *left1, const void *right1)
{
  const struct index *left = left1;
  const struct index *right = right1;

  return memcmp (shared_data->results[left->run][left->process_index].bytes,
                 shared_data->results[right->run][right->process_index].bytes,
                 random_size);
}

static int
do_test_func (void (*func)(unsigned char *bytes))
{
  /* Collect random data.  */
  for (int run = 0; run < runs; ++run)
    {
      pid_t pids[subprocesses];
      for (int process_index = 0; process_index < subprocesses;
           ++process_index)
        {
          pids[process_index] = xfork ();
          if (pids[process_index] == 0)
            {
              subprocess (run, process_index, func);
              _exit (0);
            }
        }

      /* Trigger all subprocesses.  Also add data from the parent
         process.  */
      subprocess (run, subprocesses, func);

      for (int process_index = 0; process_index < subprocesses;
           ++process_index)
        {
          int status;
          xwaitpid (pids[process_index], &status, 0);
          if (status != 0)
            FAIL_EXIT1 ("subprocess index %d (PID %d) exit status %d\n",
                        process_index, (int) pids[process_index], status);
        }
    }

  /* Check for duplicates.  */
  struct index indexes[runs * processes];
  for (int run = 0; run < runs; ++run)
    for (int process_index = 0; process_index < processes; ++process_index)
      indexes[run * processes + process_index]
        = (struct index) { .run = run, .process_index = process_index };
  qsort (indexes, array_length (indexes), sizeof (indexes[0]), index_compare);
  for (size_t i = 1; i < array_length (indexes); ++i)
    {
      if (index_compare (indexes + i - 1, indexes + i) == 0)
        {
          support_record_failure ();
          unsigned char *bytes
            = shared_data->results[indexes[i].run]
                [indexes[i].process_index].bytes;
          char *quoted = support_quote_blob (bytes, random_size);
          printf ("error: duplicate randomness data: \"%s\"\n"
                  "  run %d, subprocess %d\n"
                  "  run %d, subprocess %d\n",
                  quoted, indexes[i - 1].run, indexes[i - 1].process_index,
                  indexes[i].run, indexes[i].process_index);
          free (quoted);
        }
    }

  return 0;
}

static int
do_test (void)
{
  shared_data = support_shared_allocate (sizeof (*shared_data));
  {
    pthread_barrierattr_t attr;
    xpthread_barrierattr_init (&attr);
    xpthread_barrierattr_setpshared (&attr, PTHREAD_PROCESS_SHARED);
    xpthread_barrier_init (&shared_data->barrier, &attr, processes);
    xpthread_barrierattr_destroy (&attr);
  }

  do_test_func (generate_arc4random);
  do_test_func (generate_arc4random_buf);
  do_test_func (generate_arc4random_uniform);

  xpthread_barrier_destroy (&shared_data->barrier);
  support_shared_free (shared_data);
  shared_data = NULL;

  return 0;
}

#define TIMEOUT 40
#include <support/test-driver.c>
