/*
 * Copyright (c) 2002-2004, Adam Dunkels.
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
 * This file is part of the Contiki desktop environment
 *
 * $Id: bounce.c,v 1.1 2006/05/30 20:50:26 oliverschmidt Exp $
 *
 */

#include "ctk.h"
#include "ctk-draw.h"
#include "ctk-mouse.h"
#include "ek.h"
#include "loader.h"

EK_EVENTHANDLER(bounce_eventhandler, ev, data);
EK_POLLHANDLER(bounce_pollhandler);
EK_PROCESS(p, "Bounce screensaver", EK_PRIO_LOWEST,
	   bounce_eventhandler, bounce_pollhandler, NULL);
static ek_id_t id = EK_ID_NONE;

static char pos[4] = {3, 17, 7, 13};
static char vec[4] = {-1, 1, -1, 1};
static char max[4];
static char rev[2];

/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(ssfire_init, arg)
{
  arg_free(arg);
  if(id == EK_ID_NONE) {
    id = ek_start(&p);
  }
}
/*-----------------------------------------------------------------------------------*/
EK_EVENTHANDLER(bounce_eventhandler, ev, data)
{
  EK_EVENTHANDLER_ARGS(ev, data);
  
  if(ev == EK_EVENT_INIT) {
    ctk_mode_set(CTK_MODE_SCREENSAVER);
    ctk_mouse_hide();
    max[0] = max[1] = ctk_draw_width();
    max[2] = max[3] = ctk_draw_height();
  } else if(ev == ctk_signal_screensaver_stop ||
	    ev == EK_EVENT_REQUEST_EXIT) {
    ctk_draw_init();
    ctk_desktop_redraw(NULL);
    ek_exit();
    id = EK_ID_NONE;
    LOADER_UNLOAD();
  }
}
/*-----------------------------------------------------------------------------------*/
EK_POLLHANDLER(bounce_pollhandler)
{
  static unsigned char i;

  if(ctk_mode_get() == CTK_MODE_SCREENSAVER) {
    for(i = 0; i < 4; ++i) {
      pos[i] += vec[i];
      if(pos[i] <= 0 || pos[i] >= max[i]) {
        vec[i] = -vec[i];
      }
    }
    if(pos[0] == pos[1] || pos[2] == pos[3]) {
      return;
    }
    for(i = 0; i < 2; ++i) {
      rev[i] = pos[i * 2] > pos[i * 2 + 1];
    }
    _textframexy(pos[0 + rev[0]],
		 pos[2 + rev[1]],
		 pos[1 - rev[0]] - pos[0 + rev[0]],
		 pos[3 - rev[1]] - pos[2 + rev[1]], _TEXTFRAME_TALL);
  }
}
/*-----------------------------------------------------------------------------------*/
