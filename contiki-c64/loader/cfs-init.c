#include "contiki.h"

#include "cfs.h"
#include "cfs-service.h"

#include <cbm.h>
#include <string.h>

static int  s_open(const char *n, int f);
static void s_close(int f);
static int  s_read(int f, char *b, unsigned int l);
static int  s_write(int f, char *b, unsigned int l) {return -1;}
static int  s_opendir(struct cfs_dir *p, const char *n) {return -1;}
static int  s_readdir(struct cfs_dir *p, struct cfs_dirent *e) {return -1;}
static int  s_closedir(struct cfs_dir *p) {return -1;}

static const struct cfs_service_interface interface =
  {
    CFS_SERVICE_VERSION,
    s_open,
    s_close,
    s_read,
    s_write,
    s_opendir,
    s_readdir,
    s_closedir
  };

EK_EVENTHANDLER(eventhandler, ev, data);
EK_PROCESS(proc, CFS_SERVICE_NAME ": init", EK_PRIO_NORMAL,
           eventhandler, NULL, (void *)&interface);

/*---------------------------------------------------------------------------*/
EK_PROCESS_INIT(cfs_init_init, arg)
{
  arg_free(arg);
  ek_service_start(CFS_SERVICE_NAME, &proc);
}
/*---------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{
  switch(ev) {
  case EK_EVENT_INIT:
    break;
  case EK_EVENT_REQUEST_REPLACE:
    ek_replace((struct ek_proc *)data, &interface);
    break;
  case EK_EVENT_REQUEST_EXIT:
    ek_exit();
    break;
  }
}
/*---------------------------------------------------------------------------*/
static int
s_open(const char *n, int f)
{
  if(cbm_open(2, 8, CBM_READ, n) == 0) {
    return 2;
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static void
s_close(int f)
{
  cbm_close(f);
}
/*---------------------------------------------------------------------------*/
static int
s_read(int f, char *b, unsigned int l)
{
  return cbm_read(f, b, l);
}
/*---------------------------------------------------------------------------*/
