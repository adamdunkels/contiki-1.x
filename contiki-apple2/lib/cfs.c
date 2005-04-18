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
 * $Id: cfs.c,v 1.1 2005/04/18 21:41:07 oliverschmidt Exp $
 */


#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "contiki.h"

#include "cfs.h"


static char cwd[FILENAME_MAX+1];

/*---------------------------------------------------------------------------*/
int
cfs_opendir(struct cfs_dir *dirp, const char *name)
{
  char *openname = strcmp(name, "/") ? (char *)name : getcwd(cwd, sizeof(cwd));

  if((dirp->fd = cfs_open(openname, CFS_READ)) != -1) {
    if(cfs_read(dirp->fd,
		dirp->block.bytes,
		sizeof(dirp->block)) == sizeof(dirp->block)) {
      dirp->entry_length      = dirp->block.bytes[0x23];
      dirp->entries_per_block = dirp->block.bytes[0x24];
      dirp->current_entry     = 1;
      return 0;
    }
    cfs_close(dirp->fd);
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
int
cfs_readdir(struct cfs_dir *dirp, struct cfs_dirent *dirent)
{
  char* entry;

  do {
    if(dirp->current_entry == dirp->entries_per_block) {
      if(cfs_read(dirp->fd,
		  dirp->block.bytes,
		  sizeof(dirp->block)) != sizeof(dirp->block)) {
	return -1;
      }
      dirp->current_entry = 0;
    }
    entry = dirp->block.content.entries +
	    dirp->current_entry * dirp->entry_length;
    ++dirp->current_entry;
  } while (entry[0x00] == 0);

  entry[0x01 + (entry[0x00] & 15)] = '\0';
  strcpy(dirent->name, &entry[0x01]);
  dirent->size = *(unsigned int *)&entry[0x13];
  return 0;
}
/*---------------------------------------------------------------------------*/
int
cfs_closedir(struct cfs_dir *dirp)
{
  return cfs_close(dirp->fd);
}
/*---------------------------------------------------------------------------*/
