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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Adam Dunkels.
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
 * This file is part of the Contiki desktop OS.
 *
 * $Id: shell-gui.c,v 1.3 2003/08/24 22:57:48 adamdunkels Exp $
 *
 */

#include "program-handler.h"
#include "loader.h"
#include "c64-fs.h"
#include "uip.h"
#include "uip_main.h"
#include "uip_arp.h"
#include "resolv.h"

#include "shell.h"

#include "uip-signal.h"

#include <string.h>

#define XSIZE 36
#define YSIZE 12

static struct ctk_window window;
static char log[XSIZE * YSIZE];
static struct ctk_label loglabel =
  {CTK_LABEL(0, 0, XSIZE, YSIZE, log)};
static char command[XSIZE + 1];
static struct ctk_textentry commandentry =
  {CTK_TEXTENTRY(0, YSIZE, XSIZE - 2, 1, command, XSIZE)};

static DISPATCHER_SIGHANDLER(sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Command shell", shell_idle, sighandler,
		   NULL)};
static ek_id_t id;

/*-----------------------------------------------------------------------------------*/
void
shell_quit(char *str)
{
  ctk_window_close(&window);
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
void
shell_output(char *str1, char *str2)
{
  static unsigned char i, len;
  
  for(i = 1; i < YSIZE; ++i) {
    memcpy(&log[(i - 1) * XSIZE], &log[i * XSIZE], XSIZE);
  }
  memset(&log[(YSIZE - 1) * XSIZE], 0, XSIZE);

  len = strlen(str1);

  strncpy(&log[(YSIZE - 1) * XSIZE], str1, XSIZE);
  if(len < XSIZE) {
    strncpy(&log[(YSIZE - 1) * XSIZE] + len, str2, XSIZE - len);
  }

  CTK_WIDGET_REDRAW(&loglabel);
}
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(config_init, arg)
{
  arg_free(arg);
  
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    dispatcher_listen(ctk_signal_window_close);
    dispatcher_listen(ctk_signal_widget_activate);    

    ctk_window_new(&window, XSIZE, YSIZE+1, "Command shell");
    CTK_WIDGET_ADD(&window, &loglabel);
    CTK_WIDGET_ADD(&window, &commandentry);
    CTK_WIDGET_FOCUS(&window, &commandentry);
    memset(log, ' ', sizeof(log));

    shell_output("Contiki command shell", "");
    shell_output("Type '?' for help", "");
  }
  ctk_window_open(&window);
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);

  if(s == ctk_signal_widget_activate &&
     data == (ek_data_t)&commandentry) {
    shell_input(command);
    CTK_TEXTENTRY_CLEAR(&commandentry);
    CTK_WIDGET_FOCUS(&window, &commandentry);
    CTK_WIDGET_REDRAW(&commandentry);
  } else if(s == ctk_signal_window_close ||
	    s == dispatcher_signal_quit) {
    shell_quit(NULL);
  }
}
/*-----------------------------------------------------------------------------------*/
