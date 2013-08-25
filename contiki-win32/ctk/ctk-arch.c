/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * This file is part of the "ctk" console GUI toolkit for cc65
 *
 * $Id: ctk-arch.c,v 1.1 2004/07/15 00:36:57 oliverschmidt Exp $
 *
 */

#include "ctk-arch.h"

ctk_arch_key_t
ctk_arch_getkey(void)
{
  ctk_arch_key_t key = getch();

  if(key == (ctk_arch_key_t)0x00 || 
     key == (ctk_arch_key_t)0xE0) {
    key = getch();
    switch(key) {
    case (ctk_arch_key_t)0x44:	/* F10          */
      return -1; 
    case (ctk_arch_key_t)0x0F:	/* AltGr-Tab    */
      return -2;
    case (ctk_arch_key_t)0x94:	/* Ctrl-Tab     */
      return -3;
    case (ctk_arch_key_t)0x48:	/* Cursor Up    */
      return -4;
    case (ctk_arch_key_t)0x4B:	/* Cursor Left  */
      return -5;
    case (ctk_arch_key_t)0x4D:	/* Cursor Right */
      return -6;
    case (ctk_arch_key_t)0x50:	/* Cursor Down  */
      return -7;
    }
  }

  return key;
}
