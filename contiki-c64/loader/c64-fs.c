/*
 * Copyright (c) 2003, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution. 
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *        This product includes software developed by Adam Dunkels. 
 * 4. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the Contiki desktop environment 
 *
 * $Id: c64-fs.c,v 1.1 2003/08/04 00:12:50 adamdunkels Exp $
 *
 */

#include "c64-dio.h"
#include "c64-fs.h"

#include <string.h>
#include <stdio.h>

struct directory_entry {
  unsigned char type;
  unsigned char track, sect;
  unsigned char name[16];
  unsigned char reltrack, relsect, relreclen;
  unsigned char unused1, unused2, unused3;
  unsigned char tmptrack, tmpsect;
  unsigned char blockslo, blockshi;
};

static unsigned char dirbuf[256];
static unsigned char dirbuftrack = 0, dirbufsect = 0;

static unsigned char filebuf[256];
static unsigned char filebuftrack = 0, filebufsect = 0;


/*-----------------------------------------------------------------------------------*/
static void
readfilebuf(unsigned char track, unsigned char sect)
{
  if(filebuftrack == track &&
     filebufsect == sect) {
    /* Buffer already contains requested block, return. */
    return;
  }
  c64_dio_read_block(track, sect, filebuf);
  filebuftrack = track;
  filebufsect = sect;
}
/*-----------------------------------------------------------------------------------*/
static struct c64_fs_dir opendir;
static struct c64_fs_dirent opendirent;
int
c64_fs_open(const char *name, struct c64_fs_file *f)
{
  c64_fs_opendir(&opendir);

  while(c64_fs_readdir(&opendir, &opendirent) == 0) {
    if(strncmp(opendirent.name, name, 16) == 0) {
      f->track = opendirent.track;
      f->sect = opendirent.sect;
      f->ptr = 2;
      return 0;
    }
  }

  return -1;
}
/*-----------------------------------------------------------------------------------*/
static unsigned char
readbyte(struct c64_fs_file *f, char *buf)
{
  readfilebuf(f->track, f->sect);
  *buf = filebuf[f->ptr];

  ++f->ptr;
  if(filebuf[0] == 0) {
    if(f->ptr == filebuf[1]) {
      return 0;
    }
  } else if(f->ptr == 0) {
    f->track = filebuf[0];
    f->sect = filebuf[1];
    f->ptr = 2;
  }
  return 1;
}
/*-----------------------------------------------------------------------------------*/
int __fastcall__
c64_fs_read(struct c64_fs_file *f, char *buf, int len)
{
  int i;
  for(i = 0;i < len; ++i) {
    if(readbyte(f, buf) == 0) {
      break;
    }
    ++buf;
  }
  return i;
}
/*-----------------------------------------------------------------------------------*/
void
c64_fs_close(struct c64_fs_file *f)
{
  
}
/*-----------------------------------------------------------------------------------*/
static void
readdirbuf(unsigned char track, unsigned char sect)
{
  if(dirbuftrack == track &&
     dirbufsect == sect) {
    /* Buffer already contains requested block, return. */
    return;
  }
  c64_dio_read_block(track, sect, dirbuf);
  dirbuftrack = track;
  dirbufsect = sect;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
c64_fs_opendir(struct c64_fs_dir *d)
{
  d->track = 18;
  d->sect = 1;
  d->ptr = 2;

  return 0;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
c64_fs_readdir(struct c64_fs_dir *d,
	       struct c64_fs_dirent *f)
{
  struct directory_entry *de;
  int i;
  
  readdirbuf(d->track, d->sect);
  de = (struct directory_entry *)&dirbuf[d->ptr];
  for(i = 0; i < 16; ++i) {
    if(de->name[i] == 0xa0) {
      de->name[i] = 0;
      break;
    }
  }
  strncpy(f->name, de->name, 16);
  f->track = de->track;
  f->sect = de->sect;

  if(d->ptr == 226) {
    if(dirbuf[0] == 0) {
      return 1;
    }
    d->track = dirbuf[0];
    d->sect = dirbuf[1];
    d->ptr = 2;
  } else {
    d->ptr += 32;
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
void
c64_fs_closedir(struct c64_fs_dir *d)
{
  
}
/*-----------------------------------------------------------------------------------*/
