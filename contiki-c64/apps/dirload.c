/*
 * Copyright (c) 2002, Adam Dunkels.
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
 * $Id: dirload.c,v 1.1 2003/04/17 19:54:04 adamdunkels Exp $
 *
 */

#include <stdlib.h>

#include "ctk.h"
#include "dispatcher.h"
#include "loader.h"

static struct ctk_window window;
static struct ctk_label filename =
  {CTK_LABEL(0, 0, 20, 1, "")};
static struct ctk_label description =
  {CTK_LABEL(0, 1, 20, 1, "")};

static DISPATCHER_SIGHANDLER(dirload_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Directory browser", NULL, dirload_sighandler, NULL)};
static ek_id_t id;

static struct dsc *about_dsc;

/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(dirload_init)
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    
    ctk_window_new(&window, 24, 5, "Directory browser");

    CTK_WIDGET_ADD(&window, &filename);
    CTK_WIDGET_ADD(&window, &description);
    
    dispatcher_listen(ctk_signal_button_activate);
    dispatcher_listen(ctk_signal_window_close);

    about_dsc = LOADER_LOAD_DSC("about.dsc");
    if(about_dsc != NULL) {
      ctk_label_set_text(&filename, about_dsc->prgname);
      ctk_label_set_text(&description, about_dsc->description);
    }
    
  }
  ctk_window_open(&window);
}
/*-----------------------------------------------------------------------------------*/
static void
quit(void)
{
  LOADER_UNLOAD_DSC(about_dsc);
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(dirload_sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_button_activate) {
  } else if(s == ctk_signal_window_close &&
	    data == (ek_data_t)&window) {
    quit();
  }
}
/*-----------------------------------------------------------------------------------*/
