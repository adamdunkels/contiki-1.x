#ifndef __CFS_H__
#define __CFS_H__

#include "cfs-service.h"

#define CFS_READ  0
#define CFS_WRITE 1
int cfs_open(const char *name, int flags);
void cfs_close(int fd);
int cfs_read(int fd, char *buf, unsigned int len);
int cfs_write(int fd, char *buf, unsigned int len);

int cfs_opendir(struct cfs_dir *dirp, const char *name);
int cfs_readdir(struct cfs_dir *dirp, struct cfs_dirent *dirent);
int cfs_closedir(struct cfs_dir *dirp);

struct cfs_service_interface *cfs_find_service(void);

#define cfs_open(name, flags)   (cfs_find_service()->open(name, flags))
#define cfs_close(fd)           (cfs_find_service()->close(fd))
#define cfs_read(fd, buf, len)  (cfs_find_service()->read(fd, buf, len))
#define cfs_write(fd, buf, len) (cfs_find_service()->write(fd, buf, len))

#define cfs_opendir(dirp, name) (cfs_find_service()->opendir(dirp, name))
#define cfs_readdir(dirp, ent)  (cfs_find_service()->readdir(dirp, ent))
#define cfs_closedir(dirp)      (cfs_find_service()->closedir(dirp))


#endif /* __CFS_H__ */
