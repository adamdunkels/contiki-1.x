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
 * $Id: program-handler.c,v 1.11 2003/07/31 23:47:30 adamdunkels Exp $
 *
 */


#include "petsciiconv.h"
#include "uip_main.h"
#include "uip.h"
#include "ctk.h"
#include "ctk-draw.h"
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
static struct ctk_label errortype =
  {CTK_LABEL(4, 3, 16, 1, "")};
static struct ctk_button errorokbutton =
  {CTK_BUTTON(9, 5, 2, "Ok")};


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
  "Could not open file"
};

static ek_signal_t loader_signal_load;

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

    ctk_dialog_new(&errordialog, 22, 6);
    CTK_WIDGET_ADD(&errordialog, &errormsg);
    CTK_WIDGET_ADD(&errordialog, &errortype);
    CTK_WIDGET_ADD(&errordialog, &errorokbutton);
#endif /* WITH_LOADER_ARCH */
    
    dispatcher_listen(ctk_signal_menu_activate);
    dispatcher_listen(ctk_signal_button_activate);

    loader_signal_load = dispatcher_sigalloc();
    dispatcher_listen(loader_signal_load);
  }
  
}
/*-----------------------------------------------------------------------------------*/
void
program_handler_load(char *name)
{
#ifdef WITH_LOADER_ARCH
  dispatcher_emit(loader_signal_load, name, id);
  ctk_label_set_text(&loadingname, name);
  ctk_dialog_open(&loadingdialog);
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
static
DISPATCHER_SIGHANDLER(program_handler_sighandler, s, data)
{
  unsigned char err, i;
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_button_activate) {
#ifdef WITH_LOADER_ARCH
    if(data == (ek_data_t)&loadbutton) {
      ctk_window_close(&runwindow);
      program_handler_load(name);
    } else if(data == (ek_data_t)&errorokbutton) {
      ctk_dialog_close();
      /*      ctk_redraw();*/
    }
#endif /* WITH_LOADER_ARCH */
    
    for(i = 0; i < CTK_CONF_MAXMENUITEMS; ++i) {
      if(contikidsc[i] != NULL &&
	 data == (ek_data_t)contikidsc[i]->icon) {
	RUN(contikidsc[i]->prgname, contikidsc[i]->init);
	break;
      }
    }    
  } else if(s == ctk_signal_menu_activate) {
    if((struct ctk_menu *)data == &contikimenu) {
#if WITH_LOADER_ARCH
      if(contikidsc[contikimenu.active - 1] != NULL) {
	RUN(contikidsc[contikimenu.active - 1]->prgname,
	    contikidsc[contikimenu.active - 1]->init);
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
    
  } else if(s == loader_signal_load) {
#if WITH_LOADER_ARCH
    ctk_dialog_close();
    err = LOADER_LOAD(data);
    if(err != LOADER_OK) {
      ctk_label_set_text(&errortype, errormsgs[err]);
      ctk_dialog_open(&errordialog);
    }
#endif /* WITH_LOADEER_ARCH */
  } 
}
/*-----------------------------------------------------------------------------------*/
