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
 * This file is part of the Contiki desktop OS
 *
 * $Id: loader-arch.c,v 1.6 2003/08/09 13:28:31 adamdunkels Exp $
 *
 */

#include <stdlib.h>
#include <modload.h>

#include "c64-fs.h"

#include "loader.h"

#include "loader-arch.h"

static struct mod_ctrl ctrl = {
  (void *)c64_fs_read            /* Read from disk */
};

static struct c64_fs_file file;

struct loader_arch_hdr {
  char arch[8];
  char version[8];

  char initfunc[1];
};

/*-----------------------------------------------------------------------------------*/
/* load(name)
 *
 * Loads a program from disk and executes it. Code originally written by
 * Ullrich von Bassewitz.
 */
/*-----------------------------------------------------------------------------------*/
static unsigned char
load(const char *name)
{
  unsigned char res;
  int ret;
  
  /* Now open the file */
  ret = c64_fs_open(name, &file);
  if(ret < 0) {
    /* Could not open the file, display an error and return */
    /* ### */
    return LOADER_ERR_OPEN;
  }
  ctrl.callerdata = (int)&file;
  
  /* Load the module */
  res = mod_load(&ctrl);
  
  /* Close the input file */
  c64_fs_close(&file);
  
  /* Check the return code */
  if(res != MLOAD_OK) {
    /* Wrong module, out of memory or whatever. Print an error
     * message and return.
     */
    /* ### */
    return res;
  }
  
  /* We've successfully loaded the module. */
  
  return LOADER_OK;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
loader_arch_load(const char *name)
{
  unsigned char r;
  struct loader_arch_hdr *hdr;
  
  r = load(name);
  if(r != MLOAD_OK) {
    return r;
  }
  hdr = (struct loader_arch_hdr *)ctrl.module;
  
  /* Check the program header and see that version and architecture
     matches. */
  
  /* Call the init function. */

  ((void (*)(void))hdr->initfunc)();

  return LOADER_OK;
}
/*-----------------------------------------------------------------------------------*/
struct dsc *
loader_arch_load_dsc(const char *name)
{
  unsigned char r;

  r = load(name);
  if(r == MLOAD_OK) {
    return (struct dsc *)ctrl.module;    
  }
  return NULL;
}
/*-----------------------------------------------------------------------------------*/
void
loader_arch_free(void *addr)
{
  mod_free(addr);
}
/*-----------------------------------------------------------------------------------*/

