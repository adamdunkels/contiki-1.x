#include "mt.h"

MTP(t, p, "Test thread");
/*--------------------------------------------------------------------------*/
#pragma optimize(push, off)
static void
test(void *data)
{
  while(1) {
    asm("inc $d020");
  }
}
#pragma optimize(pop)
/*--------------------------------------------------------------------------*/
LOADER_INIT_FUNC(mtest_init, arg)
{
  arg_free(arg);
  mtp_start(&t, test, NULL);
}
/*--------------------------------------------------------------------------*/
