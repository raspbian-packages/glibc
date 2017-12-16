/* glibc test for __tls_get_addr optimization.  */

#define TEST_FUNCTION do_test ()
static int
do_test (void)
{
  extern int tls_get_addr_opt_test (void);

  return tls_get_addr_opt_test ();
}

#include "../../test-skeleton.c"
