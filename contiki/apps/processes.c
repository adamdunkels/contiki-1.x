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
 * $Id: processes.c,v 1.3 2003/04/09 13:45:05 adamdunkels Exp $
 *
 */

#include "ctk.h"
#include "dispatcher.h"
#include "loader.h"

#define MAX_PROCESSLABELS 16
static struct ctk_window processwindow;
static unsigned char ids[MAX_PROCESSLABELS][3];
static struct ctk_label processidlabels[MAX_PROCESSLABELS];
static struct ctk_label processnamelabels[MAX_PROCESSLABELS];

static struct ctk_button processupdatebutton =
  {CTK_BUTTON(0, 9, 6, "Update")};
static struct ctk_button processclosebutton =
  {CTK_BUTTON(13, 9, 5, "Close")};

static DISPATCHER_SIGHANDLER(processes_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Process listing", NULL, processes_sighandler, NULL)};
static ek_id_t id;

/*-----------------------------------------------------------------------------------*/
static void
update_processwindow(void)
{
  unsigned char i, j, *idsptr;
  struct dispatcher_proc *p;

  i = 0;
  j = 0;  
  do {
    p = dispatcher_process(i);
    if(p != NULL) {
      idsptr = ids[j];
      idsptr[0] = '0' + i / 10;
      idsptr[1] = '0' + i % 10;
      idsptr[2] = 0;
      CTK_LABEL_NEW(&processidlabels[j],
		    0, j + 1, 2, 1, idsptr);
      CTK_WIDGET_ADD(&processwindow, &processidlabels[j]);
      
      CTK_LABEL_NEW(&processnamelabels[j],
		    3, j + 1, 17, 1, p->name);
      CTK_WIDGET_ADD(&processwindow, &processnamelabels[j]);
      ++j;
    }
    ++i;
  } while(i != 0);
  

  CTK_WIDGET_ADD(&processwindow, &processupdatebutton);
  CTK_WIDGET_ADD(&processwindow, &processclosebutton);
  CTK_WIDGET_FOCUS(&processwindow, &processupdatebutton);
}
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(processes_init)
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    
    ctk_window_new(&processwindow, 20, 10, "Processes");
    ctk_window_move(&processwindow, 0, 1);
    update_processwindow();
    
    dispatcher_listen(ctk_signal_button_activate);
    dispatcher_listen(ctk_signal_window_close);
  }
  ctk_window_open(&processwindow);
}
/*-----------------------------------------------------------------------------------*/
static void
processes_quit(void)
{
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(processes_sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  if(s == ctk_signal_button_activate) {
    if(data == (ek_data_t)&processupdatebutton) {
      ctk_window_clear(&processwindow);
      update_processwindow();
      ctk_redraw();
    } else if(data == (ek_data_t)&processclosebutton) {
      ctk_window_close(&processwindow);
      processes_quit();
      ctk_redraw();
    }
  } else if(s == ctk_signal_window_close &&
	    data == (ek_data_t)&processwindow) {
    processes_quit();
  }
}
/*-----------------------------------------------------------------------------------*/
