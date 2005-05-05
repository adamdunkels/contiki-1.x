/*
 * Copyright (c) 2003, Adam Dunkels.
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
 * This file is part of the Contiki desktop OS.
 *
 * $Id: shell-gui.c,v 1.13 2005/05/05 23:32:01 oliverschmidt Exp $
 *
 */

#include "ek.h"

#include "program-handler.h"
#include "loader.h"

#include "uip.h"
#include "uip_arp.h"
#include "resolv.h"

#include "shell.h"

#include "ctk-textentry-cmdline.h"

#include "shell-gui-conf.h"

#include <string.h>


static struct ctk_window window;
static char log[SHELL_GUI_CONF_XSIZE * SHELL_GUI_CONF_YSIZE];
static struct ctk_label loglabel =
  {CTK_LABEL(0, 0, SHELL_GUI_CONF_XSIZE, SHELL_GUI_CONF_YSIZE, log)};
static char command[SHELL_GUI_CONF_XSIZE - 1];
static struct ctk_textentry commandentry =
  {CTK_TEXTENTRY_INPUT(0, SHELL_GUI_CONF_YSIZE, SHELL_GUI_CONF_XSIZE - 2, 1, command,
		       SHELL_GUI_CONF_XSIZE - 2, ctk_textentry_cmdline_input)};

EK_EVENTHANDLER(eventhandler, ev, data);
EK_PROCESS(p, "Command shell", EK_PRIO_NORMAL,
	   eventhandler, NULL, NULL);
static ek_id_t id = EK_ID_NONE;

/*static DISPATCHER_SIGHANDLER(sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Command shell", shell_idle, sighandler,
		   NULL)};
		   static ek_id_t id;*/

/*-----------------------------------------------------------------------------------*/
void
shell_quit(char *str)
{
  ctk_window_close(&window);
  ek_exit();
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
void
shell_output(char *str1, char *str2)
{
  static unsigned char i, len;
  
  for(i = 1; i < SHELL_GUI_CONF_YSIZE; ++i) {
    memcpy(&log[(i - 1) * SHELL_GUI_CONF_XSIZE],
	   &log[i * SHELL_GUI_CONF_XSIZE], SHELL_GUI_CONF_XSIZE);
  }
  memset(&log[(SHELL_GUI_CONF_YSIZE - 1) * SHELL_GUI_CONF_XSIZE],
	 0, SHELL_GUI_CONF_XSIZE);

  len = strlen(str1);

  strncpy(&log[(SHELL_GUI_CONF_YSIZE - 1) * SHELL_GUI_CONF_XSIZE],
	  str1, SHELL_GUI_CONF_XSIZE);
  if(len < SHELL_GUI_CONF_XSIZE) {
    strncpy(&log[(SHELL_GUI_CONF_YSIZE - 1) * SHELL_GUI_CONF_XSIZE] + len,
	    str2, SHELL_GUI_CONF_XSIZE - len);
  }

  CTK_WIDGET_REDRAW(&loglabel);
}
/*-----------------------------------------------------------------------------------*/
void
shell_prompt(char *str)
{
  
}
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(shell_gui_init, arg)
{
  arg_free(arg);
  
  if(id == EK_ID_NONE) {
    id = ek_start(&p);
  }
}
/*-----------------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{
  EK_EVENTHANDLER_ARGS(ev, data);
  
  if(ev == EK_EVENT_INIT) {
    ctk_window_new(&window, SHELL_GUI_CONF_XSIZE,
		   SHELL_GUI_CONF_YSIZE + 1, "Command shell");
    CTK_WIDGET_ADD(&window, &loglabel);
    /*    CTK_WIDGET_SET_FLAG(&loglabel, CTK_WIDGET_FLAG_MONOSPACE);*/
    CTK_WIDGET_ADD(&window, &commandentry);
    /*    CTK_WIDGET_SET_FLAG(&commandentry, CTK_WIDGET_FLAG_MONOSPACE);*/
    CTK_WIDGET_FOCUS(&window, &commandentry);
    memset(log, 0, sizeof(log));
    
    shell_init();
    ctk_window_open(&window);
    shell_start();
  } else if(ev == ctk_signal_widget_activate &&
     data == (ek_data_t)&commandentry) {
    shell_output("> ", command);
    shell_input(command);
    CTK_TEXTENTRY_CLEAR(&commandentry);
    CTK_WIDGET_REDRAW(&commandentry);
  } else if(ev == ctk_signal_window_close ||
	    ev == EK_EVENT_REQUEST_EXIT) {
    shell_quit(NULL);
  } else {
    shell_eventhandler(ev, data);
  }
}
/*-----------------------------------------------------------------------------------*/
