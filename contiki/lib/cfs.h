#ifndef __CFS_H__
#define __CFS_H__

#include "cfs-service.h"

int cfs_open(char *name, int flags);
void cfs_close(int fd);
int cfs_read(int fd, char *buf, int len);
int cfs_write(int fd, char *buf, int len);

int cfs_dio_read(char *buf, int track, int sector);
int cfs_dio_write(char *buf, int track, int sector);

#endif /* __CFS_H__ */
