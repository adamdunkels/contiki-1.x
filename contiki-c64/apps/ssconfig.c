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
 * $Id: ssconfig.c,v 1.1 2003/07/31 23:36:29 adamdunkels Exp $
 *
 */

#include <stdlib.h>

#include "ctk.h"
#include "dispatcher.h"
#include "loader.h"

static struct ctk_window window;

static struct ctk_label availmsg =
  {CTK_LABEL(0, 0, 24, 1, "Available screensavers:")};

static struct ctk_label timeoutmsg =
  {CTK_LABEL(0, 9, 19, 1, "Timeout (seconds):")};
static char timeout[8];
static struct ctk_textentry timeoutentry =
  {CTK_TEXTENTRY(20, 9, 5, 1, timeout, 8)};

static struct ctk_button testbutton =
  {CTK_BUTTON(0, 10, 4, "Test")};
static struct ctk_button uninstallbutton =
  {CTK_BUTTON(6, 10, 9, "Uninstall")};
static struct ctk_button installbutton =
  {CTK_BUTTON(17, 10, 7, "Install")};

static struct ctk_label hintmsg1 =
  {CTK_LABEL(0, 11, 5, 1, "Hint:")};
static struct ctk_label hintmsg2 =
  {CTK_LABEL(0, 12, 29, 1, "Close apps before installing")};


static DISPATCHER_SIGHANDLER(ssconfig_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Screensaver configuration", NULL,
		   ssconfig_sighandler, NULL)};
static ek_id_t id;

/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(ssconfig_init)
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    
    ctk_window_new(&window, 30, 13, "Screensavers");

    CTK_WIDGET_ADD(&window, &availmsg);
    CTK_WIDGET_ADD(&window, &timeoutmsg);

    CTK_WIDGET_ADD(&window, &timeoutentry);

    CTK_WIDGET_ADD(&window, &testbutton);
    CTK_WIDGET_ADD(&window, &uninstallbutton);
    CTK_WIDGET_ADD(&window, &installbutton);

    CTK_WIDGET_ADD(&window, &hintmsg1);
    CTK_WIDGET_ADD(&window, &hintmsg2);
      
    dispatcher_listen(ctk_signal_button_activate);
    dispatcher_listen(ctk_signal_window_close);
  }
  ctk_window_open(&window);
}
/*-----------------------------------------------------------------------------------*/
static void
quit(void)
{
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(ssconfig_sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_button_activate) {
    /*    ctk_desktop_redraw(window.desktop);*/
  } else if(s == ctk_signal_window_close &&
	    data == (ek_data_t)&window) {
    quit();
  }
}
/*-----------------------------------------------------------------------------------*/
