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
 * 3. The name of the author may not be used to endorse or promote
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
 * This file is part of the "ctk" GUI toolkit for cc65
 *
 * $Id: ctk-mouse.c,v 1.2 2006/05/07 23:05:57 oliverschmidt Exp $
 *
 */

#include <stdlib.h>
#include <mouse.h>

#include "ctk.h"
#include "ctk-mouse.h"
#include "ctk-conf.h"

#include "ctk-mouse-conf.h"

#if CTK_CONF_MOUSE_SUPPORT

/* These two are defined in loader-arch.c */
extern struct mod_ctrl ctrl;
extern unsigned char load(const char *name);

static struct mouse_pos pos;
static unsigned char okay;

/*-----------------------------------------------------------------------------------*/
void
ctk_mouse_init(void)
{
  okay = load(MOUSE_CONF_DRIVER) == LOADER_OK;
  if(okay) {
    okay = mouse_install(&mouse_def_callbacks, ctrl.module) == MOUSE_ERR_OK;
    if(okay) {
      atexit((void (*)(void))mouse_uninstall);
    } else {
      mod_free(ctrl.module);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
unsigned short
ctk_mouse_x(void)
{
  if(okay) {
    mouse_pos(&pos);
  }
  return pos.x;
}
/*-----------------------------------------------------------------------------------*/
unsigned short
ctk_mouse_y(void)
{
  if(okay) {
    mouse_pos(&pos);
  }
  return pos.y;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_mouse_button(void)
{
  if(okay) {
    return mouse_buttons();
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_mouse_xtoc(unsigned short x)
{
  return MOUSE_CONF_XTOC(x);
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_mouse_ytoc(unsigned short y)
{
  return MOUSE_CONF_YTOC(y);
}
/*-----------------------------------------------------------------------------------*/
void
ctk_mouse_hide(void)
{
  if(okay) {
    mouse_hide();
  }
}
/*-----------------------------------------------------------------------------------*/
void
ctk_mouse_show(void)
{
  if(okay) {
    mouse_show();
  }
}
/*-----------------------------------------------------------------------------------*/
#endif /* CTK_CONF_MOUSE_SUPPORT */
