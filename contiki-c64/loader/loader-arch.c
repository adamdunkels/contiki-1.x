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
 * $Id: loader-arch.c,v 1.1 2003/04/08 11:49:12 adamdunkels Exp $
 *
 */

#include <modload.h>
#include <fcntl.h>
#include "loader-arch.h"

/*-----------------------------------------------------------------------------------*/
/* loader_arch_load(name)
 *
 * Loads a program from disk and executes it. Code originally written by
 * Ullrich von Bassewitz.
 */
/*-----------------------------------------------------------------------------------*/
unsigned char
loader_arch_load(const char *name)
{
  static struct mod_ctrl ctrl = {
    read            /* Read from disk */
  };
  unsigned char res;
  
  /* Now open the file */
  ctrl.callerdata = open(name, O_RDONLY);
  if(ctrl.callerdata >= 0) {    
    /* Load the module */
    res = mod_load(&ctrl);
                                                                    
    /* Close the input file */
    close(ctrl.callerdata);
    
    /* Check the return code */
    if(res != MLOAD_OK) {
      /* Wrong module, out of memory or whatever. Print an error
       * message and return.
       */
      /* ### */
      return res;
    }
    
    /* We've successfully loaded the module. Call its main function. We
     * could also evaluate the function result code if necessary.
     */
    ((void (*)(void))ctrl.module)();
    
  } else {
    
    /* Could not open the file, display an error and return */
    /* ### */
    return 6;
  }

  return MLOAD_OK;

}
/*-----------------------------------------------------------------------------------*/
void
loader_arch_free(void *addr)
{
  mod_free(addr);
}
/*-----------------------------------------------------------------------------------*/

