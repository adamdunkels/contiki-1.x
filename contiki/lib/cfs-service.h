#ifndef __CFS_SERVICE_H__
#define __CFS_SERVICE_H__

#include "ek-service.h"
#include "cfs.h"


#define CFS_SERVICE_VERSION 0x01
#define CFS_SERVICE_NAME "Filesystem"

struct cfs_dir {
  unsigned char dummy_space[32];
};

struct cfs_dirent {
  unsigned char name[32];
  unsigned int size;
};

struct cfs_service_interface {
  unsigned char version;
  int  (* open)(const char *name, int flags);
  void (* close)(int fd);
  int  (* read)(int fd, char *buf, unsigned int len);
  int  (* write)(int fd, char *buf, unsigned int len);

  int  (* opendir)(struct cfs_dir *dir, const char *name);
  int  (* readdir)(struct cfs_dir *dir, struct cfs_dirent *dirent);
  int  (* closedir)(struct cfs_dir *dir);
};

#endif /* __CFS_SERVICE_H__ */
