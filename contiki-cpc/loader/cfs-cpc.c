/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * $Id: cfs-cpc.c,v 1.1 2006/04/17 15:02:35 kthacker Exp $
 */
#include "contiki.h"

#include "log.h"
#include "cfs.h"
#include "cfs-service.h"

extern void _readdir(void *);

struct cpc_dir 
{
	char *buffer; 
	char *ptr;
};

//#include <cbm.h>
#include <string.h>

static int  s_open(const char *n, int f);
static void s_close(int f);
static int  s_read(int f, char *b, unsigned int l);
static int  s_write(int f, char *b, unsigned int l);
static int  s_opendir(struct cfs_dir *p, const char *n);
static int  s_readdir(struct cfs_dir *p, struct cfs_dirent *e);
static int  s_closedir(struct cfs_dir *p);

static const struct cfs_service_interface interface =
  {
    CFS_SERVICE_VERSION,
    s_open,
    s_close,
    s_read,
    s_write,
    s_opendir,
    s_readdir,
    s_closedir
  };

EK_EVENTHANDLER(cpc_cfs_eventhandler, ev, data);
EK_PROCESS(proc, CFS_SERVICE_NAME ": KERNAL", EK_PRIO_NORMAL,
           cpc_cfs_eventhandler, NULL, (void *)&interface);

/*---------------------------------------------------------------------------*/
EK_PROCESS_INIT(cfs_cpc_init, arg)
{
  arg_free(arg);
  ek_service_start(CFS_SERVICE_NAME, &proc);
}
/*---------------------------------------------------------------------------*/
EK_EVENTHANDLER(cpc_cfs_eventhandler, ev, data)
{
  switch(ev) {
  case EK_EVENT_INIT:
  case EK_EVENT_REPLACE:
    log_message("Starting KERNAL CFS", "");
    break;
  case EK_EVENT_REQUEST_REPLACE:
    ek_replace((struct ek_proc *)data, &interface);
    break;
  case EK_EVENT_REQUEST_EXIT:
    ek_exit();
    break;
  }
}
/*---------------------------------------------------------------------------*/
static int
s_open(const char *n, int f)
{
 // if(cbm_open(2, 8, f, n) == 0) {
 //   return 2;
 // }
  return -1;
}
/*---------------------------------------------------------------------------*/
static void
s_close(int f)
{
 // cbm_close(f);
}
/*---------------------------------------------------------------------------*/
static int
s_read(int f, char *b, unsigned int l)
{
  return 0; //return cbm_read(f, b, l);
}
/*---------------------------------------------------------------------------*/
static int
s_write(int f, char *b, unsigned int l)
{
  return 0; //  return cbm_write(f, b, l);
}
/*---------------------------------------------------------------------------*/
static int
s_opendir(struct cfs_dir *p, const char *n)
{
	struct cpc_dir *cpcdir = (struct cpc_dir *)p;

	char *buffer = malloc(2048);
	
	if (buffer)
	{
		cpcdir->buffer = buffer;
		cpcdir->ptr = buffer;	
		_readdir(buffer);
		return 0;
	}
	return 1;
}
/*---------------------------------------------------------------------------*/
static int
s_readdir(struct cfs_dir *p, struct cfs_dirent *e)
{
  int i;
  int size;
  int npos;
  struct cpc_dir *cpcdir = (struct cpc_dir *)p;
  char *ptr = cpcdir->ptr;

  if (ptr[0]!=0xff)
	return 1;
  
  ptr++;

  npos = 0;
  for (i=0; i<8; i++)
  {  
    char ch = ptr[0]&0x07f;
    ptr++;
    e->name[npos] = ch;
    npos++;    
  }
  e->name[npos] = '.';
  npos++;
  for (i=0; i<3; i++)
  {
    char ch = ptr[0]&0x07f;
    ptr++;
    e->name[npos] = ch;
    npos++;
  }
  e->name[npos] = '\0';
 
  size = (ptr[0]&0x0ff) + ((ptr[1]&0x0ff)<<8);  
  size = size*1024;
  ptr+=2;
  e->size = size;
  cpcdir->ptr = ptr;

  return 0;

/* 1 = if no more dir entries
 0 = more dir entries */
}
/*---------------------------------------------------------------------------*/
static int
s_closedir(struct cfs_dir *p)
{
	struct cpc_dir *cpcdir = (struct cpc_dir *)p;
	free(cpcdir->buffer);
	return 1;
}
/*---------------------------------------------------------------------------*/
