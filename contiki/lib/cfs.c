#include "contiki.h"

#include "cfs.h"
#include "cfs-service.h"


static int  null_open(char *n, int f)             {return -1;}
static void null_close(int f)                     {return;}
static int  null_read(int f, char *b, int l)      {return -1;}
static int  null_write(int f, char *b, int l)     {return -1;}
static int  null_dio_read(char *b, int t, int s)  {return -1;}
static int  null_dio_write(char *b, int t, int s) {return -1;}

static const struct cfs_service_interface nullinterface =
  {
    CFS_SERVICE_VERSION,
    null_open,
    null_close,
    null_read,
    null_write,
    null_dio_read,
    null_dio_write
  };

EK_SERVICE(service, CFS_SERVICE_NAME);

/*---------------------------------------------------------------------------*/
static struct cfs_service_interface *
find_service(void)
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
int
cfs_open(char *name, int flags)
{
  find_service()->open(name, flags);
}
/*---------------------------------------------------------------------------*/
void
cfs_close(int fd)
{
  find_service()->close(fd);
}
/*---------------------------------------------------------------------------*/
int
cfs_read(int fd, char *buf, int len)
{
  find_service()->read(fd, buf, len);
}
/*---------------------------------------------------------------------------*/
int
cfs_write(int fd, char *buf, int len)
{
  find_service()->write(fd, buf, len);
}
/*---------------------------------------------------------------------------*/
int
cfs_dio_read(char *buf, int track, int sector)
{
  find_service()->dio_read(buf, track, sector);
}
/*---------------------------------------------------------------------------*/
int
cfs_dio_write(char *buf, int track, int sector)
{
  find_service()->dio_write(buf, track, sector);
}
/*---------------------------------------------------------------------------*/
