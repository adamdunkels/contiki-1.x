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
 * $Id: webserver.c,v 1.6 2003/05/28 05:21:51 adamdunkels Exp $
 *
 */


#include "ctk.h"
#include "dispatcher.h"
#include "http-strings.h"
#include "uip_main.h"
#include "petsciiconv.h"

#include "loader.h"

#include "httpd.h"

/* The main window. */
static struct ctk_window mainwindow;

static struct ctk_label message =
  {CTK_LABEL(0, 0, 14, 1, "Web server is ")};
static struct ctk_label onoffmessage =
  {CTK_LABEL(14, 0, 3, 1, "On ")};


static struct ctk_button onbutton =
  {CTK_BUTTON(0, 5, 2, "On")};
static struct ctk_label statuslabel =
  {CTK_LABEL(4, 5, 8, 1, "")};
static struct ctk_button offbutton =
  {CTK_BUTTON(12, 5, 3, "Off")};

static unsigned char onoff;
#define ON  1
#define OFF 0

static DISPATCHER_SIGHANDLER(webserver_sighandler, s, data);

static struct dispatcher_proc p =
  {DISPATCHER_PROC("Web server", NULL, webserver_sighandler,
		   httpd_appcall)};
static ek_id_t id;

/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(webserver_init)
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);

    ctk_window_new(&mainwindow, 17, 6, "Web server");
    ctk_window_move(&mainwindow, 21, 16);
  
    CTK_WIDGET_ADD(&mainwindow, &message);
    CTK_WIDGET_ADD(&mainwindow, &onoffmessage);

    CTK_WIDGET_ADD(&mainwindow, &onbutton);
    CTK_WIDGET_ADD(&mainwindow, &offbutton);
    
    CTK_WIDGET_FOCUS(&mainwindow, &onbutton);
    
    /* Attach listeners to signals. */
    dispatcher_listen(ctk_signal_button_activate);

    httpd_init();
    
    onoff = ON;
  }

  ctk_window_open(&mainwindow);
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(webserver_sighandler, s, data)
{
  struct ctk_button *b;
  unsigned char i;
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_button_activate) {
    b = (struct ctk_button *)data;
    if(b == &onbutton) {
      ctk_label_set_text(&onoffmessage, "On ");
      CTK_WIDGET_REDRAW(&onoffmessage);
      onoff = ON;
    } else if(b == &offbutton) {
      ctk_label_set_text(&onoffmessage, "Off");
      CTK_WIDGET_REDRAW(&onoffmessage);
      onoff = OFF;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
