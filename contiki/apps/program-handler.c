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
 * This file is part of the Contiki desktop OS
 *
 * $Id: program-handler.c,v 1.18 2003/08/20 20:52:22 adamdunkels Exp $
 *
 */


#include "petsciiconv.h"
#include "uip_main.h"
#include "uip.h"
#include "ctk.h"
#include "ctk-draw.h"
#include "ctk-conf.h"
#include "dispatcher.h"
#include "resolv.h"

#include "loader.h"

#include "program-handler.h"

/* Menus */
static struct ctk_menu contikimenu;

static struct dsc *contikidsc[10];
static unsigned char contikidsclast = 0;

#if WITH_LOADER_ARCH
/* "Run..." window */
static struct ctk_window runwindow;
static unsigned char runmenuitem;
static struct ctk_label namelabel =
  {CTK_LABEL(0, 0, 13, 1, "Program name:")};
static char name[31];
static struct ctk_textentry nameentry =
  {CTK_TEXTENTRY(0, 1, 14, 1, name, 30)};
static struct ctk_button loadbutton =
  {CTK_BUTTON(10, 2, 4, "Load")};

static struct ctk_window loadingdialog;
static struct ctk_label loadingmsg =
  {CTK_LABEL(0, 0, 8, 1, "Starting")};
static struct ctk_label loadingname =
  {CTK_LABEL(9, 0, 16, 1, name)};

static struct ctk_window errordialog;
static struct ctk_label errormsg =
  {CTK_LABEL(0, 1, 22, 1, "Error loading program:")};
static char errorfilename[22];
static struct ctk_label errorfilelabel =
  {CTK_LABEL(0, 3, 22, 1, errorfilename)};
static struct ctk_label errortype =
  {CTK_LABEL(4, 5, 16, 1, "")};
static struct ctk_button errorokbutton =
  {CTK_BUTTON(9, 7, 2, "Ok")};


#endif /* WITH_LOADER_ARCH */

static DISPATCHER_SIGHANDLER(program_handler_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Program handler", NULL, program_handler_sighandler, NULL)};
static ek_id_t id;


static char *errormsgs[] = {
  "Ok",
  "Read error",
  "Header error",
  "OS error",
  "Data format error",
  "Out of memory",
  "File not found"
};

static ek_signal_t loader_signal_load;
static ek_signal_t loader_signal_display_name;

static char *displayname;

static char screensaver[20];

/*-----------------------------------------------------------------------------------*/
void
program_handler_add(struct dsc *dsc, char *menuname,
		    unsigned char desktop)
{
  contikidsc[contikidsclast++] = dsc;
  ctk_menuitem_add(&contikimenu, menuname);
  if(desktop) {
    CTK_ICON_ADD(dsc->icon, id);
  }
}
/*-----------------------------------------------------------------------------------*/
void
program_handler_init(void)     
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);

    /* Create the menus */
    ctk_menu_new(&contikimenu, "Contiki");
    ctk_menu_add(&contikimenu);
#if WITH_LOADER_ARCH
    runmenuitem = ctk_menuitem_add(&contikimenu, "Run program...");
    
    ctk_window_new(&runwindow, 16, 3, "Run");
 
    CTK_WIDGET_ADD(&runwindow, &namelabel);
    CTK_WIDGET_ADD(&runwindow, &nameentry);
    CTK_WIDGET_ADD(&runwindow, &loadbutton);

    CTK_WIDGET_FOCUS(&runwindow, &nameentry);

    ctk_dialog_new(&loadingdialog, 25, 1);
    CTK_WIDGET_ADD(&loadingdialog, &loadingmsg);
    CTK_WIDGET_ADD(&loadingdialog, &loadingname);

    ctk_dialog_new(&errordialog, 22, 8);
    CTK_WIDGET_ADD(&errordialog, &errormsg);
    CTK_WIDGET_ADD(&errordialog, &errorfilelabel);
    CTK_WIDGET_ADD(&errordialog, &errortype);
    CTK_WIDGET_ADD(&errordialog, &errorokbutton);
    CTK_WIDGET_FOCUS(&errordialog, &errorokbutton);
#endif /* WITH_LOADER_ARCH */
    
    dispatcher_listen(ctk_signal_menu_activate);
    dispatcher_listen(ctk_signal_button_activate);

#if CTK_CONF_SCREENSAVER
    dispatcher_listen(ctk_signal_screensaver_start);
#endif /* CTK_CONF_SCREENSAVER */
    
    loader_signal_load = dispatcher_sigalloc();
    dispatcher_listen(loader_signal_load);
    loader_signal_display_name = dispatcher_sigalloc();
    dispatcher_listen(loader_signal_display_name);

    displayname = NULL;

    screensaver[0] = 0;
  }
  
}
/*-----------------------------------------------------------------------------------*/
#ifdef WITH_LOADER_ARCH
#define NUM_LOADERNAMES 6
#define NAMELEN 16
static char loadernames[(NAMELEN + 1) * NUM_LOADERNAMES];
static char * 
loadername_copy(char *name)
{
  char i;
  char *loadernamesptr;

  loadernamesptr = loadernames;
  /* Allocate a place in the loadernames table. */
  for(i = 0; i < NUM_LOADERNAMES; ++i) {
    if(*loadernamesptr == 0) {
      strncpy(loadernamesptr, name, NAMELEN);
      return loadernamesptr;
    }
    loadernamesptr += NAMELEN + 1;
  }
  return NULL;
}

static void
loadername_free(char *name)
{
  *name = 0;
}
#endif /* WITH_LOADER_ARCH */
/*-----------------------------------------------------------------------------------*/
void
program_handler_load(char *name)
{
#ifdef WITH_LOADER_ARCH
  name = loadername_copy(name);
  if(name != NULL) {
    dispatcher_emit(loader_signal_display_name, name, id);
  } else {
    ctk_label_set_text(&errortype, "Out of memory");
    ctk_dialog_open(&errordialog);
  }
  /*  ctk_redraw(); */
  /*  ctk_window_redraw(&loadingdialog);*/
#endif /* WITH_LOADER_ARCH */
}

#ifdef WITH_LOADER_ARCH
#define RUN(prg, name) program_handler_load(prg)
#else /* WITH_LOADER_ARCH */
#define RUN(prg, initfunc) initfunc()/*; ctk_desktop_redraw(NULL)*/
#endif /* WITH_LOADER_ARCH */
/*-----------------------------------------------------------------------------------*/
void
program_handler_screensaver(char *name)
{
  if(name == NULL) {
    screensaver[0] = 0;
  } else {
    strncpy(screensaver, name, sizeof(screensaver));
  }
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(program_handler_sighandler, s, data)
{
  unsigned char err, i;
  struct dsc *dsc;
  struct dsc **dscp;
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_button_activate) {
#ifdef WITH_LOADER_ARCH
    if(data == (ek_data_t)&loadbutton) {
      ctk_window_close(&runwindow);
      program_handler_load(name);
    } else if(data == (ek_data_t)&errorokbutton) {
      ctk_dialog_close();
    }
#endif /* WITH_LOADER_ARCH */
    dscp = &contikidsc[0];
    for(i = 0; i < CTK_CONF_MAXMENUITEMS; ++i) {    
      if(*dscp != NULL &&
	 data == (ek_data_t)(*dscp)->icon) {
	RUN((*dscp)->prgname, (*dscp)->init);
	break;
      }
      ++dscp;
    }
  } else if(s == ctk_signal_menu_activate) {
    if((struct ctk_menu *)data == &contikimenu) {
#if WITH_LOADER_ARCH
      dsc = contikidsc[contikimenu.active - 1];
      if(dsc != NULL) {
	RUN(dsc->prgname, dsc->init);
      } else if(contikimenu.active == runmenuitem) {
	ctk_window_open(&runwindow);
	CTK_WIDGET_FOCUS(&runwindow, &nameentry);
      }
#else /* WITH_LOADER_ARCH */
      if(contikidsc[contikimenu.active] != NULL) {
	RUN(contikidsc[contikimenu.active]->prgname,
	    contikidsc[contikimenu.active]->init);
      }
#endif /* WITH_LOADER_ARCH */
    }
#if CTK_CONF_SCREENSAVER
  } else if(s == ctk_signal_screensaver_start) {
#if WITH_LOADER_ARCH
    if(screensaver[0] != 0) {
      program_handler_load(screensaver);
    }
#endif /* WITH_LOADER_ARCH */
#endif /* CTK_CONF_SCREENSAVER */
  } else if(s == loader_signal_display_name) {
#if WITH_LOADER_ARCH
    if(displayname == NULL) {
      ctk_label_set_text(&loadingname, data);
      ctk_dialog_open(&loadingdialog);
      dispatcher_emit(loader_signal_load, data, id);
      displayname = data;
    } else {
      /* Try again. */
      dispatcher_emit(loader_signal_display_name, data, id);
    }
#endif /* WITH_LOADER_ARCH */
  } else if(s == loader_signal_load) {
#if WITH_LOADER_ARCH
    if(displayname == data) {
      ctk_dialog_close();
      displayname = NULL;
      err = LOADER_LOAD(data);
      if(err != LOADER_OK) {
	errorfilename[0] = '"';
	strncpy(errorfilename + 1, data, sizeof(errorfilename) - 2);
	errorfilename[1 + strlen(data)] = '"';
	ctk_label_set_text(&errortype, errormsgs[err]);
	ctk_dialog_open(&errordialog);
      }
      loadername_free(data);
    } else {
      /* Try again. */
      dispatcher_emit(loader_signal_display_name, data, id);
    }
#endif /* WITH_LOADEER_ARCH */
  } 
}
/*-----------------------------------------------------------------------------------*/
