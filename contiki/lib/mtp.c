#include "mtp.h"

#include "loader.h"

/*--------------------------------------------------------------------------*/
void
mtp_start(struct mtp_thread *t,
	  void (* function)(void *), void *data)
{
  mt_init();
  mt_start(t->t, function, data);
  dispatcher_start(t->p);
}
/*--------------------------------------------------------------------------*/
DISPATCHER_SIGHANDLER(mtp_sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  if(s == dispatcher_signal_quit) {
    dispatcher_exit(DISPATCHER_CURPROC());
    mt_remove();
    LOADER_UNLOAD();        
  }
}
/*--------------------------------------------------------------------------*/
