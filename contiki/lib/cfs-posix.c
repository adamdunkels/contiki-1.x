#include "contiki.h"

#include "cfs.h"
#include "cfs-service.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

static int  s_open(const char *n, int f);
static void s_close(int f);
static int  s_read(int f, char *b, unsigned int l);
static int  s_write(int f, char *b, unsigned int l);
static int  s_opendir(struct cfs_dir *p, const char *n);
static int  s_readdir(struct cfs_dir *p, struct cfs_dirent *e);
static int  s_closedir(struct cfs_dir *p);

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
EK_PROCESS(proc, CFS_SERVICE_NAME ": POSIX", EK_PRIO_NORMAL,
           eventhandler, NULL, (void *)&interface);

struct cfs_posix_dir {
  DIR *dirp;
};

/*---------------------------------------------------------------------------*/
EK_PROCESS_INIT(cfs_posix_init, arg)
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
  char filename[255];
  sprintf(filename, "cfs-root/%s", n);
  if(f == CFS_READ) {
    return open(filename, O_RDONLY);
  } else {
    return open(filename, O_CREAT|O_RDWR);
  }
}
/*---------------------------------------------------------------------------*/
static void
s_close(int f)
{
  close(f);
}
/*---------------------------------------------------------------------------*/
static int
s_read(int f, char *b, unsigned int l)
{
  return read(f, b, l);
}
/*---------------------------------------------------------------------------*/
static int
s_write(int f, char *b, unsigned int l)
{
  return write(f, b, l);
}
/*---------------------------------------------------------------------------*/
static int
s_opendir(struct cfs_dir *p, const char *n)
{
  struct cfs_posix_dir *dir = (struct cfs_posix_dir *)p;
  char dirname[255];

  if(n == NULL) {
    n = "";
  }
  sprintf(dirname, "cfs-root/%s", n);
  
  dir->dirp = opendir(dirname);
    
  return dir->dirp == NULL;
}
/*---------------------------------------------------------------------------*/
static int
s_readdir(struct cfs_dir *p, struct cfs_dirent *e)
{
  struct cfs_posix_dir *dir = (struct cfs_posix_dir *)p;
  struct dirent *res;
  int ret;
  
  res = readdir(dir->dirp);
  if(res == NULL) {
    return 1;
  }
  strncpy(e->name, res->d_name, sizeof(e->name));
  e->size = 0;
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
s_closedir(struct cfs_dir *p)
{
  struct cfs_posix_dir *dir = (struct cfs_posix_dir *)p;
  closedir(dir->dirp);
  return 1;
}
/*---------------------------------------------------------------------------*/
