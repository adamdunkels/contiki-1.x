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
 * $Id: about.c,v 1.11 2004/07/04 11:35:07 adamdunkels Exp $
 *
 */

#include <string.h>

#include "ek.h"
#include "ctk.h"
#include "ctk-draw.h"

#include "petsciiconv.h"

#include "loader.h"

static struct ctk_window aboutdialog;
static struct ctk_label aboutlabel1 =
  {CTK_LABEL(5, 0, 23, 1, "The Contiki Operating System")};
static struct ctk_label aboutlabel2 =
  {CTK_LABEL(3, 2, 28, 1, "A modern, Internet-enabled")};
static struct ctk_label aboutlabel3 =
  {CTK_LABEL(6, 3, 20, 1, "operating system and")};
static struct ctk_label aboutlabel4 =
  {CTK_LABEL(6, 4, 20, 1, "desktop environment.")};

static char abouturl_petscii[] = "http://dunkels.com/adam/contiki/";
static char abouturl_ascii[40];
static struct ctk_hyperlink abouturl = 
  {CTK_HYPERLINK(0, 6, 32, "http://dunkels.com/adam/contiki/",
		 abouturl_ascii)};
static struct ctk_button aboutclose =
  {CTK_BUTTON(12, 8, 5, "Close")};

EK_EVENTHANDLER(about_eventhandler, ev, data);

EK_PROCESS(p, "About Contiki", EK_PRIO_NORMAL, about_eventhandler, NULL, NULL);
/*static DISPATCHER_SIGHANDLER(about_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("About Contiki", NULL, about_sighandler, NULL)};
  static ek_id_t id;*/
static ek_id_t id = EK_ID_NONE;

/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(about_init, arg)
{
  arg_free(arg);
  
  if(id == EK_ID_NONE) {
    /*    id = dispatcher_start(&p); */
    id = ek_start(&p);
  }
  /*  ctk_desktop_redraw(aboutdialog.desktop);*/
}
/*-----------------------------------------------------------------------------------*/
static void
about_quit(void)
{
  ctk_dialog_close();
  ek_exit();
  id = EK_ID_NONE;
  /*  LOADER_UNLOAD();*/
}
/*-----------------------------------------------------------------------------------*/
/*static DISPATCHER_SIGHANDLER(about_sighandler, s, data)*/
EK_EVENTHANDLER(about_eventhandler, ev, data)
{
  unsigned char width;
  EK_EVENTHANDLER_ARGS(ev, data);

  if(ev == EK_EVENT_INIT) {
    width = ctk_desktop_width(NULL);
    
    strcpy(abouturl_ascii, abouturl_petscii);
    petsciiconv_toascii(abouturl_ascii, sizeof(abouturl_ascii));

    if(width > 34) {
      ctk_dialog_new(&aboutdialog, 32, 9);
    } else {
      ctk_dialog_new(&aboutdialog, width - 2, 9);
    }
    CTK_WIDGET_ADD(&aboutdialog, &aboutlabel1);
    CTK_WIDGET_ADD(&aboutdialog, &aboutlabel2);
    CTK_WIDGET_ADD(&aboutdialog, &aboutlabel3);
    CTK_WIDGET_ADD(&aboutdialog, &aboutlabel4);
    if(width > 34) {
      CTK_WIDGET_ADD(&aboutdialog, &abouturl);
      CTK_WIDGET_SET_FLAG(&abouturl, CTK_WIDGET_FLAG_MONOSPACE);
    } else {
      CTK_WIDGET_SET_XPOS(&aboutlabel1, 0);
      CTK_WIDGET_SET_XPOS(&aboutlabel2, 0);
      CTK_WIDGET_SET_XPOS(&aboutlabel3, 0);
      CTK_WIDGET_SET_XPOS(&aboutlabel4, 0);
      
      CTK_WIDGET_SET_XPOS(&aboutclose, 0);
    }
    CTK_WIDGET_ADD(&aboutdialog, &aboutclose);
    CTK_WIDGET_FOCUS(&aboutdialog, &aboutclose);
    
    /*    dispatcher_listen(ctk_signal_button_activate);
	  dispatcher_listen(ctk_signal_hyperlink_activate);*/
    ctk_dialog_open(&aboutdialog);
    
  } else if(ev == EK_EVENT_REQUEST_EXIT) {
    about_quit();
  } else if(ev == ctk_signal_button_activate) {
    if(data == (ek_data_t)&aboutclose) {
      about_quit();
    }
  } else if(ev == ctk_signal_hyperlink_activate) {
    if((struct ctk_widget *)data == (struct ctk_widget *)&abouturl) {
      about_quit();
    }
  }
}
/*-----------------------------------------------------------------------------------*/
