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
 * $Id: c64-fs.c,v 1.5 2003/08/09 13:27:41 adamdunkels Exp $
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
  unsigned char unused1, unused2, unused3, unused4;
  unsigned char tmptrack, tmpsect;
  unsigned char blockslo, blockshi;
};

unsigned char _c64_fs_dirbuf[256];
unsigned char _c64_fs_dirbuftrack = 0, _c64_fs_dirbufsect = 0;

unsigned char _c64_fs_filebuf[256];
unsigned char _c64_fs_filebuftrack = 0, _c64_fs_filebufsect = 0;

static struct c64_fs_dirent lastdirent;

/*-----------------------------------------------------------------------------------*/
static struct c64_fs_dir opendir;
static struct c64_fs_dirent opendirent;
int
c64_fs_open(const char *name, register struct c64_fs_file *f)
{
  /* First check if we already have the file cached. If so, we don't
     need to do an expensive directory lookup. */
  if(strncmp(lastdirent.name, name, 16) == 0) {
    f->track = lastdirent.track;
    f->sect = lastdirent.sect;
    f->ptr = 2;
    return 0;
  }

  /* Not in cache, so we walk through directory instead. */
  c64_fs_opendir(&opendir);

  while(c64_fs_readdir(&opendir, &opendirent) == 0) {
    if(strncmp(opendirent.name, name, 16) == 0) {
      f->track = opendirent.track;
      f->sect = opendirent.sect;
      f->ptr = 2;
      return 0;
    }
  }

  /* The file was not found in the directory. We flush the directory
     buffer cache now in order to prevent a nasty problem from
     happening: If the first directory block of an empty disk was
     cached, *all* subsequent file opens would return "file not
     found". */
  _c64_fs_dirbuftrack = 0; /* There are no disk blocks on track 0. */
  return -1;
}
/*-----------------------------------------------------------------------------------*/
int __fastcall__
c64_fs_read(register struct c64_fs_file *f, char *buf, int len)
{
  int i;

  /* Check if current block is already in buffer, and if not read it
     from disk. */
  if(_c64_fs_filebuftrack != f->track ||
     _c64_fs_filebufsect != f->sect) {
    _c64_fs_filebuftrack = f->track;
    _c64_fs_filebufsect = f->sect;
    c64_dio_read_block(_c64_fs_filebuftrack, _c64_fs_filebufsect, _c64_fs_filebuf);
  }

  if(_c64_fs_filebuf[0] == 0 &&
     f->ptr == _c64_fs_filebuf[1]) {
    return 0; /* EOF */
  }
  
  for(i = 0; i < len; ++i) {
    *buf = _c64_fs_filebuf[f->ptr];
    
    ++f->ptr;
    if(_c64_fs_filebuf[0] == 0) {
      if(f->ptr == _c64_fs_filebuf[1]) {
	/* End of file reached, we return the amount of bytes read so
	   far. */
	return i + 1;
      }
    } else if(f->ptr == 0) {

      /* Read new block into buffer and set buffer state
	 accordingly. */
      _c64_fs_filebuftrack = f->track = _c64_fs_filebuf[0];
      _c64_fs_filebufsect = f->sect = _c64_fs_filebuf[1];
      f->ptr = 2;
      c64_dio_read_block(_c64_fs_filebuftrack,
			 _c64_fs_filebufsect, _c64_fs_filebuf);
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
void
_c64_fs_readdirbuf(unsigned char track, unsigned char sect)
{
  if(_c64_fs_dirbuftrack == track &&
     _c64_fs_dirbufsect == sect) {
    /* Buffer already contains requested block, return. */
    return;
  }
  c64_dio_read_block(track, sect, _c64_fs_dirbuf);
  _c64_fs_dirbuftrack = track;
  _c64_fs_dirbufsect = sect;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
c64_fs_opendir(register struct c64_fs_dir *d)
{
  d->track = 18;
  d->sect = 1;
  d->ptr = 2;

  return 0;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
c64_fs_readdir(register struct c64_fs_dir *d,
	       register struct c64_fs_dirent *f)
{
  struct directory_entry *de;
  register char *nameptr;
  int i;
  
  _c64_fs_readdirbuf(d->track, d->sect);
  de = (struct directory_entry *)&_c64_fs_dirbuf[d->ptr];
  nameptr = de->name;
  for(i = 0; i < 16; ++i) {
    if(*nameptr == 0xa0) {
      *nameptr = 0;
      break;
    }
    ++nameptr;
  }
  strncpy(f->name, de->name, 16);
  f->track = de->track;
  f->sect = de->sect;
  f->size = de->blockslo + (de->blockshi >> 8);

  /* Save directory entry as a cache for a file open that might follow
     this readdir. */
  memcpy(&lastdirent, f, sizeof(struct c64_fs_dirent));
  
  if(d->ptr == 226) {
    if(_c64_fs_dirbuf[0] == 0) {
      return 1;
    }
    d->track = _c64_fs_dirbuf[0];
    d->sect = _c64_fs_dirbuf[1];
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
