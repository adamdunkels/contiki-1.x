/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the Contiki operating system.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: cfs.h,v 1.3 2004/09/12 20:24:55 adamdunkels Exp $
 */
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
