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
 * $Id: welcome.c,v 1.1 2004/09/12 13:11:51 adamdunkels Exp $
 *
 */

#include "contiki.h"
#include <string.h>


static struct ctk_window welcomedialog;
static struct ctk_label welcomelabel1 =
  {CTK_LABEL(1, 1, 30, 1, "Welcome to " CONTIKI_VERSION_STRING "!")};
static struct ctk_label welcomelabel2 =
  {CTK_LABEL(2, 4, 28, 1, "F1     - open menus")};
static struct ctk_label welcomelabel3 =
  {CTK_LABEL(2, 6, 28, 1, "F3     - cycle windows")};
static struct ctk_label welcomelabel4 =
  {CTK_LABEL(2, 8, 28, 1, "F5     - select up")};
static struct ctk_label welcomelabel5 =
  {CTK_LABEL(2, 10, 28, 1, "F7     - select down")};
static struct ctk_label welcomelabel6 =
  {CTK_LABEL(2, 12, 28, 1, "Return - activate selected")};
static struct ctk_label welcomelabel7 =
  {CTK_LABEL(3, 15, 25, 1, "Press any key to continue")};


EK_EVENTHANDLER(welcome_eventhandler, ev, data);

EK_PROCESS(p, "Welcome", EK_PRIO_NORMAL, welcome_eventhandler, NULL, NULL);
static ek_id_t id = EK_ID_NONE;

/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(welcome_init, arg)
{
  arg_free(arg);
  
  if(id == EK_ID_NONE) {
    id = ek_start(&p);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
welcome_quit(void)
{
  ctk_dialog_close();
  ek_exit();
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
/*static DISPATCHER_SIGHANDLER(welcome_sighandler, s, data)*/
EK_EVENTHANDLER(welcome_eventhandler, ev, data)
{
  unsigned char width;
  EK_EVENTHANDLER_ARGS(ev, data);

  if(ev == EK_EVENT_INIT) {
    width = ctk_desktop_width(NULL);
    
    if(width > 34) {
      ctk_dialog_new(&welcomedialog, 32, 17);
    } else {
      ctk_dialog_new(&welcomedialog, width - 2, 17);
    }
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel1);
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel2);
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel3);
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel4);
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel5);
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel6);
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel7);    
    
    ctk_dialog_open(&welcomedialog);
    
  } else if(ev == EK_EVENT_REQUEST_EXIT ||
	    ev == ctk_signal_keypress) {
    welcome_quit();
  }
}
/*-----------------------------------------------------------------------------------*/
