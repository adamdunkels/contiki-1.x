#ifndef __CFS_SERVICE_H__
#define __CFS_SERVICE_H__

#include "ek-service.h"

#define CFS_SERVICE_VERSION 0x01
#define CFS_SERVICE_NAME "Filesystem"

struct cfs_service_interface {
  u8_t version;
  int  (* open)(char *name, int flags);
  void (* close)(int fd);
  int  (* read)(int fd, char *buf, int len);
  int  (* write)(int fd, char *buf, int len);

  int  (* dio_read)(char *buf, int track, int sector);
  int  (* dio_write)(char *buf, int track, int sector);
};

#endif /* __CFS_SERVICE_H__ */
