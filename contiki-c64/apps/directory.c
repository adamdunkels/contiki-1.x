/*
 * Copyright (c) 2003, Adam Dunkels.
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
 * $Id: directory.c,v 1.5 2003/08/13 22:49:23 adamdunkels Exp $
 *
 */

#include <stdlib.h>
#include <cbm.h>
#include <string.h>

#include "ctk.h"
#include "ctk-draw.h"
#include "dispatcher.h"
#include "loader.h"

#include "c64-fs.h"

#include "program-handler.h"

#define MAX_NUMFILES 40
#define WIDTH 36
#define HEIGHT 22

static struct dsc *dscs[MAX_NUMFILES];
static unsigned char numfiles, morestart;

static struct ctk_window window;

static struct ctk_label description =
  {CTK_LABEL(0, HEIGHT - 1, WIDTH, 1, "")};

static char autoexit = 1;
static struct ctk_button autoexitbutton =
  {CTK_BUTTON(0, 20, 9, "Auto-exit")};
static char autoexiton[] = "is On ";
static char autoexitoff[] = "is Off";
static struct ctk_label autoexitlabel =
  {CTK_LABEL(12, 20, 6, 1, autoexiton)};

static struct ctk_button morebutton =
  {CTK_BUTTON(0, 20, 4, "More")};

static struct ctk_button backbutton =
  {CTK_BUTTON(0, 20, 4, "Back")};

static struct ctk_button reloadbutton =
  {CTK_BUTTON(30, 20, 6, "Reload")};

static DISPATCHER_SIGHANDLER(directory_sighandler, s, data);
static void directory_idle(void);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Directory browser", directory_idle,
		   directory_sighandler, NULL)};
static ek_id_t id;

static unsigned char width, height;

static char loading = 0;
static struct c64_fs_dir dir;
/*-----------------------------------------------------------------------------------*/
static void
show_statustext(char *text)
{
  ctk_label_set_text(&description, text);
  CTK_WIDGET_REDRAW(&description);
}
/*-----------------------------------------------------------------------------------*/
static void
startloading(void)
{
  if(c64_fs_opendir(&dir) != 0) {
    show_statustext("Cannot open directory");
  } else {
    loading = 1;
  }
}    
/*-----------------------------------------------------------------------------------*/
#if 0
static void
loaddirectory(void)
{
  unsigned char i;
  
  if(c64_fs_opendir(&dir) != 0) {
    show_statustext("Cannot open directory");
  } else {
    i = 0;
    while(c64_fs_readdir(&dir, &dirent) == 0) {
      if(strcmp(&dirent.name[strlen(dirent.name) - 4], ".dsc") == 0) {	
	dscs[i] = LOADER_LOAD_DSC(dirent.name);
	if(dscs[i] != NULL) {
	  ++i;
	  if(i == MAX_NUMFILES) {
	    break;
	  }
	}

      }
    }
    c64_fs_closedir(&dir);

    numfiles = i;

    show_statustext("Directory loaded");
  }
}
#endif /* 0 */
/*-----------------------------------------------------------------------------------*/
static void
makewindow(unsigned char i)
{
  unsigned char x, y;

  ctk_window_clear(&window);
  CTK_WIDGET_SET_YPOS(&description, height - 3);
  CTK_WIDGET_SET_WIDTH(&description, width);
  CTK_WIDGET_ADD(&window, &description);

  morestart = i;
  
  x = 0; y = 1;
  for(; dscs[i] != NULL; ++i) {

    if(x + strlen(dscs[i]->icon->title) >= width) {
      y += 5;
      x = 0;
      if(y >= height - 2 - 4) {
	morestart = i;
	break;
      }
    }
    CTK_WIDGET_SET_XPOS(dscs[i]->icon, x);
    CTK_WIDGET_SET_YPOS(dscs[i]->icon, y);
    CTK_WIDGET_ADD(&window, dscs[i]->icon);

    x += strlen(dscs[i]->icon->title) + 1;
  }
  CTK_WIDGET_SET_YPOS(&autoexitbutton, height - 2);
  CTK_WIDGET_ADD(&window, &autoexitbutton);
  CTK_WIDGET_SET_YPOS(&autoexitlabel, height - 2);
  CTK_WIDGET_ADD(&window, &autoexitlabel);

  if(i != morestart) {
    CTK_WIDGET_SET_YPOS(&backbutton, height - 1);
    CTK_WIDGET_ADD(&window, &backbutton);
  } else {
    CTK_WIDGET_SET_YPOS(&morebutton, height - 1);
    CTK_WIDGET_ADD(&window, &morebutton);
  }
  CTK_WIDGET_SET_XPOS(&reloadbutton, width - 8);
  CTK_WIDGET_SET_YPOS(&reloadbutton, height - 1);
  CTK_WIDGET_ADD(&window, &reloadbutton);    
}
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(directory_init)
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);

    width = ctk_draw_width() - 4;
    height = ctk_draw_height() - 4;
    
    ctk_window_new(&window, width, height, "Directory");

    /*    loaddirectory();*/
    makewindow(0);
    show_statustext("Loading directory...");
    startloading();
    
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
    if(data == (ek_data_t)&reloadbutton) {
      for(i = 0; dscs[i] != NULL; ++i) {
	LOADER_UNLOAD_DSC(dscs[i]);
      }     
      /*      loaddirectory();*/
      startloading();
      makewindow(0);
      ctk_window_open(&window);
    } else if(data == (ek_data_t)&morebutton) {
      makewindow(morestart);
      ctk_window_open(&window);
    } else if(data == (ek_data_t)&backbutton) {
      makewindow(0);
      ctk_window_open(&window);
    } else if(data == (ek_data_t)&autoexitbutton) {
      autoexit = 1 - autoexit;
      if(autoexit == 1) {
	ctk_label_set_text(&autoexitlabel, autoexiton);
      } else {
	ctk_label_set_text(&autoexitlabel, autoexitoff);
      }
      CTK_WIDGET_REDRAW(&autoexitlabel);
    } else {
      for(i = 0; dscs[i] != NULL; ++i) {
	if(data == (ek_data_t)(dscs[i]->icon)) {
	  program_handler_load(dscs[i]->prgname);
	  if(autoexit) {
	    ctk_window_close(&window);
	    quit();
	  }
	  break;
	}
      }
    }
  } else if(s == ctk_signal_widget_select) {
    if(data == (ek_data_t)&reloadbutton) {
      show_statustext("Reload directory");
    } else if(data == (ek_data_t)&morebutton) {
      show_statustext("Show more files");
    } else if(data == (ek_data_t)&backbutton) {
      show_statustext("Show first files");
    } else if(data == (ek_data_t)&autoexitbutton) {
      show_statustext("Exit when loading program");
    } else {
      for(i = 0; dscs[i] != NULL; ++i) {
	if(data == (ek_data_t)(dscs[i]->icon)) {
	  show_statustext(dscs[i]->description);
	  break;
	}
      }
    }
  } else if(s == ctk_signal_window_close &&
	    data == (ek_data_t)&window) {
    quit();
  } else if(s == dispatcher_signal_quit) {
    ctk_window_close(&window);
    quit();
  }
}
/*-----------------------------------------------------------------------------------*/
static void
directory_idle(void)
{
  static struct c64_fs_dirent dirent;
  static char message[40];
    
  if(loading != 0) {  
    if(c64_fs_readdir(&dir, &dirent) == 0) {
      if(strcmp(&dirent.name[strlen(dirent.name) - 4], ".dsc") == 0) {	
	dscs[numfiles] = LOADER_LOAD_DSC(dirent.name);
	if(dscs[numfiles] != NULL) {
	  ++numfiles;
	  if(numfiles == MAX_NUMFILES) {
	    c64_fs_closedir(&dir);
	    loading = 0;
	    makewindow(0);
	    ctk_window_redraw(&window);
	    return;
	  }
	}
	strcpy(message, "Loading \"");
	strcpy(message + 9, dirent.name);
	strcpy(message + 9 + strlen(dirent.name), "\"...");
	show_statustext(message);
      }
    } else {
      c64_fs_closedir(&dir);
      loading = 0;
      makewindow(0);
      show_statustext("Directory loaded");
      ctk_window_redraw(&window);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
