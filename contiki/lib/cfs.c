#include "contiki.h"

#include "cfs.h"
#include "cfs-service.h"


static int  null_open(const char *n, int f)                       {return -1;}
static void null_close(int f)                                     {return;}
static int  null_read(int f, char *b, unsigned int l)             {return -1;}
static int  null_write(int f, char *b, unsigned int l)            {return -1;}
static int  null_opendir(struct cfs_dir *p, const char *n)        {return -1;}
static int  null_readdir(struct cfs_dir *p, struct cfs_dirent *e) {return -1;}
static int  null_closedir(struct cfs_dir *p)                      {return -1;}

static const struct cfs_service_interface nullinterface =
  {
    CFS_SERVICE_VERSION,
    null_open,
    null_close,
    null_read,
    null_write,
    null_opendir,
    null_readdir,
    null_closedir
  };

EK_SERVICE(service, CFS_SERVICE_NAME);

/*---------------------------------------------------------------------------*/
struct cfs_service_interface *
cfs_find_service(void)
{
  struct cfs_service_interface *interface;
  interface = (struct cfs_service_interface *)ek_service_state(&service);
  if(interface != NULL &&
     interface->version == CFS_SERVICE_VERSION) {
    return interface;
  } else {
    return (struct cfs_service_interface *)&nullinterface;
  }
}
/*---------------------------------------------------------------------------*/
