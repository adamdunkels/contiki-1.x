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
 * This file is part of the Contiki desktop environment for the C64.
 *
 * $Id: webserver.c,v 1.2 2003/08/25 12:42:41 adamdunkels Exp $
 *
 */


#include "ctk.h"
#include "dispatcher.h"
#include "http-strings.h"
#include "uip_main.h"
#include "petsciiconv.h"

#include "loader.h"

#include "webserver.h"
#include "httpd.h"

/* The main window. */
static struct ctk_window mainwindow;

static struct ctk_label message =
  {CTK_LABEL(0, 0, 15, 1, "Latest requests")};

static DISPATCHER_SIGHANDLER(webserver_sighandler, s, data);

static struct dispatcher_proc p =
{DISPATCHER_PROC("Web server", NULL, webserver_sighandler,
		 httpd_appcall)};
static ek_id_t id;


#define LOG_WIDTH  30
#define LOG_HEIGHT 20
static char log[LOG_WIDTH*LOG_HEIGHT];

static struct ctk_label loglabel =
{CTK_LABEL(0, 1, LOG_WIDTH, LOG_HEIGHT, log)};
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(webserver_init, arg)
{
  arg_free(arg);
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);

    ctk_window_new(&mainwindow, LOG_WIDTH, LOG_HEIGHT+1, "Web server");

    
    CTK_WIDGET_ADD(&mainwindow, &message);
    CTK_WIDGET_ADD(&mainwindow, &loglabel);
 
    /* Attach listeners to signals. */
    /*    dispatcher_listen(ctk_signal_button_activate);*/

    httpd_init();
  }

  ctk_window_open(&mainwindow);
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(webserver_sighandler, s, data)
{
  unsigned char i;
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_button_activate) {
  }
}
/*-----------------------------------------------------------------------------------*/
void
webserver_log_file(u16_t *requester, char *file)
{
  int size;
  
  /* Scroll previous entries upwards */
  memcpy(log, &log[LOG_WIDTH], LOG_WIDTH * (LOG_HEIGHT - 1));

  /* Print out IP address of requesting host. */
  size = sprintf(&log[LOG_WIDTH * (LOG_HEIGHT - 1)],
		 "%d.%d.%d.%d: ",
		 htons(requester[0]) >> 8,
		 htons(requester[0]) & 0xff,
		 htons(requester[1]) >> 8,
		 htons(requester[1]) & 0xff);
  
  /* Copy filename into last line. */		 
  strncpy(&log[LOG_WIDTH * (LOG_HEIGHT - 1) + size], file, LOG_WIDTH - size);
	   
  /* Update log display. */
  CTK_WIDGET_REDRAW(&loglabel);
}
/*-----------------------------------------------------------------------------------*/
