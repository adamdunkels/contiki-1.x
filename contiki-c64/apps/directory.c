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
 * $Id: directory.c,v 1.1 2003/04/18 00:10:47 adamdunkels Exp $
 *
 */

#include <stdlib.h>
#include <cbm.h>
#include <string.h>

#include "ctk.h"
#include "dispatcher.h"
#include "loader.h"

#include "program-handler.h"

#define MAX_NUMFILES 20

static struct dsc *dscs[MAX_NUMFILES];

static struct ctk_window window;

static struct ctk_button filenames[MAX_NUMFILES];

static struct ctk_bitmap icon =
  {CTK_BITMAP(0, 0, 3, 3, "")};
static struct ctk_label description =
  {CTK_LABEL(4, 1, 32, 1, "")};

static DISPATCHER_SIGHANDLER(directory_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Directory browser", NULL, directory_sighandler, NULL)};
static ek_id_t id;

/*-----------------------------------------------------------------------------------*/
static struct cbm_dirent dirent;
/*-----------------------------------------------------------------------------------*/
static void
show_statustext(char *text)
{
  ctk_label_set_text(&description, text);
  CTK_WIDGET_REDRAW(&description);
}
/*-----------------------------------------------------------------------------------*/
#define LFN 9
static void
loaddirectory(void)
{
  unsigned char i;
  
  if(cbm_opendir(LFN, 8) != 0) {
    show_statustext("cbm_opendir error");
  } else {
    i = 0;
    while(cbm_readdir(LFN, &dirent) == 0) {
      if(strcmp(&dirent.name[strlen(dirent.name) - 4], ".dsc") == 0) {
	dscs[i] = LOADER_LOAD_DSC(dirent.name);
	if(dscs[i] != NULL) {
	  ++i;
	}
	if(i >= MAX_NUMFILES) {
	  break;
	}
      }
    }
    cbm_closedir(LFN);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
makewindow(void)
{
  unsigned char i, x, y;

  ctk_window_clear(&window);
  CTK_WIDGET_ADD(&window, &description);
  CTK_WIDGET_ADD(&window, &icon);

  x = 0; y = 5;
  for(i = 0; dscs[i] != NULL; ++i) {
    CTK_BUTTON_NEW(&filenames[i], x, y, strlen(dscs[i]->prgname),
		   dscs[i]->prgname);
    CTK_WIDGET_ADD(&window, &filenames[i]);

    x += 18;
    if(x == 36) {
      x = 0;
      ++y;
    }
  }

  if(i > 0) {
    CTK_WIDGET_FOCUS(&window, &filenames[0]);
    show_statustext(dscs[0]->description);
    if(dscs[0]->icon->bitmap != NULL) {
      ctk_bitmap_set_bitmap(&icon, dscs[0]->icon->bitmap);
      CTK_WIDGET_REDRAW(&icon);
    }
  }
    
}
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(directory_init)
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    
    ctk_window_new(&window, 36, 16, "Directory browser");

    loaddirectory();
    makewindow();
    
    dispatcher_listen(ctk_signal_widget_activate);
    dispatcher_listen(ctk_signal_widget_select);
    dispatcher_listen(ctk_signal_window_close);
  }
  ctk_window_open(&window);
}
/*-----------------------------------------------------------------------------------*/
static void
quit(void)
{
  unsigned char i;

  ctk_window_close(&window);
  for(i = 0; dscs[i] != NULL; ++i) {
    LOADER_UNLOAD_DSC(dscs[i]);
  }
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(directory_sighandler, s, data)
{
  unsigned char i;
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_widget_activate) {
    for(i = 0; dscs[i] != NULL; ++i) {
      if(data == (ek_data_t)&filenames[i]) {
	program_handler_load(dscs[i]->prgname);
	quit();
	break;
      }
    }
  } else if(s == ctk_signal_widget_select) {
    for(i = 0; dscs[i] != NULL; ++i) {
      if(data == (ek_data_t)&filenames[i]) {
	show_statustext(dscs[i]->description);
	if(dscs[i]->icon->bitmap != NULL) {
	  ctk_bitmap_set_bitmap(&icon, dscs[i]->icon->bitmap);
	  CTK_WIDGET_REDRAW(&icon);
	}
	break;
      }
    }
  } else if(s == ctk_signal_window_close &&
	    data == (ek_data_t)&window) {
    quit();
  }
}
/*-----------------------------------------------------------------------------------*/
