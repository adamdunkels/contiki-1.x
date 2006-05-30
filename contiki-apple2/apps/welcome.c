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
 * $Id: welcome.c,v 1.3 2006/05/30 21:02:32 oliverschmidt Exp $
 *
 */

#include "contiki.h"
#include <string.h>


static struct ctk_window welcomedialog;
static struct ctk_label welcomelabel1 =
  {CTK_LABEL(2, 1, 30, 1, "Welcome to " CONTIKI_VERSION_STRING " !")};
static struct ctk_label welcomelabel2 =
  {CTK_LABEL(2, 4, 30, 1, "Esc        - open menus")};
static struct ctk_label welcomelabel3 =
  {CTK_LABEL(2, 6, 30, 1, "Ctrl-W     - cycle windows")};
static struct ctk_label welcomelabel4 =
  {CTK_LABEL(2, 8, 30, 1, "Ctrl-A     - select up")};
static struct ctk_label welcomelabel5 =
  {CTK_LABEL(2, 10, 30, 1, "Tab        - select down")};
static struct ctk_label welcomelabel6 =
  {CTK_LABEL(2, 12, 30, 1, "Return     - activate selected")};
#ifdef __APPLE2__
static struct ctk_label welcomelabel7 =
  {CTK_LABEL(2, 14, 30, 1, "Ctrl-D     - delete character")};
static struct ctk_label welcomelabel8 =
  {CTK_LABEL(2, 16, 30, 1, "Ctrl-Reset - quit Contiki")};
static struct ctk_label welcomeclose =
  {CTK_BUTTON(13, 19, 5, "Close")};
#else /* __APPLE2__ */
static struct ctk_label welcomelabel7 =
  {CTK_LABEL(2, 14, 30, 1, "Ctrl-Reset - quit Contiki")};
static struct ctk_label welcomeclose =
  {CTK_BUTTON(13, 17, 5, "Close")};
#endif /* __APPLE2__ */


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
  EK_EVENTHANDLER_ARGS(ev, data);

  if(ev == EK_EVENT_INIT) {
#ifdef __APPLE2__
    ctk_dialog_new(&welcomedialog, 34, 21);
#else /* __APPLE2__ */
    ctk_dialog_new(&welcomedialog, 34, 19);
#endif /* __APPLE2__ */
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel1);
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel2);
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel3);
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel4);
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel5);
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel6);
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel7);
#ifdef __APPLE2__
    CTK_WIDGET_ADD(&welcomedialog, &welcomelabel8);
#endif /* __APPLE2__ */
    CTK_WIDGET_ADD(&welcomedialog, &welcomeclose);
    CTK_WIDGET_FOCUS(&welcomedialog, &welcomeclose);
    
    ctk_dialog_open(&welcomedialog);
    
  } else if(ev == EK_EVENT_REQUEST_EXIT ||
	    ev == ctk_signal_button_activate) {
    welcome_quit();
  }
}
/*-----------------------------------------------------------------------------------*/
