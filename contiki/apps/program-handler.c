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
 * $Id: program-handler.c,v 1.6 2003/04/11 20:11:40 adamdunkels Exp $
 *
 */


#include "petsciiconv.h"
#include "uip_main.h"
#include "uip.h"
#include "ctk.h"
#include "ctk-draw.h"
#include "dispatcher.h"
#include "resolv.h"
#include "telnet.h"

#include "loader.h"


/* Menus */
static struct ctk_menu contikimenu;
static unsigned char menuitem_about, menuitem_processes, menuitem_tcpip;

static struct ctk_menu programsmenu;
static unsigned char menuitem_www,
  menuitem_email, menuitem_telnet, menuitem_webserver,
  menuitem_run;

/* Icons */
static unsigned char abouticon_bitmap[3*3*8] = {
  0x00, 0x7f, 0x43, 0x4c, 0x58, 0x53, 0x60, 0x6f,
  0x00, 0xff, 0x00, 0x7e, 0x00, 0xff, 0x00, 0xff,
  0x00, 0xfe, 0xc2, 0x32, 0x1a, 0xca, 0x06, 0xf6,

  0x40, 0x5f, 0x40, 0x5f, 0x40, 0x5f, 0x40, 0x4f,
  0x00, 0xff, 0x00, 0xff, 0x00, 0xfc, 0x01, 0xf3,
  0x02, 0xfa, 0x02, 0x82, 0x3e, 0xfe, 0xfe, 0xfe,

  0x60, 0x67, 0x50, 0x59, 0x4c, 0x43, 0x7f, 0x00,
  0x07, 0xe7, 0x0f, 0xef, 0x0f, 0x0f, 0xff, 0x00,
  0x8e, 0x06, 0x06, 0x06, 0x8e, 0xfe, 0xfe, 0x00
};

static char abouticon_textmap[9] = {
  ' ', ' ', 'c',
  ' ', '?', ' ',
  '.', ' ', ' '
};

static struct ctk_icon abouticon =
  {CTK_ICON("About Contiki", abouticon_bitmap, abouticon_textmap)};

static unsigned char tcpipconficon_bitmap[3*3*8] = {
  0x00, 0x79, 0x43, 0x73, 0x47, 0x77, 0x47, 0x6f,
  0x00, 0xfe, 0xfe, 0xfc, 0xfc, 0xfc, 0xf8, 0xfb,
  0x00, 0x16, 0x02, 0x00, 0x02, 0x00, 0x00, 0xc2,

  0x48, 0x4c, 0x5f, 0x5f, 0x1f, 0x3f, 0x3f, 0x03,
  0x79, 0xf0, 0xf0, 0xf0, 0xe0, 0xe0, 0xfe, 0xfc,
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  0x77, 0x47, 0x70, 0x43, 0x79, 0x41, 0x7c, 0x00,
  0xfc, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xf7, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x84, 0xf0, 0x00
};

static char tcpipconficon_textmap[9] = {
  't', 'c', 'p',
  '/', 'i', 'p',
  'c', 'f', 'g'
};


static struct ctk_icon tcpipconficon =
  {CTK_ICON("TCP/IP conf", tcpipconficon_bitmap, tcpipconficon_textmap)};


#ifdef WITH_WWW
/* The icon for the WWW browser */
static unsigned char wwwicon_bitmap[3*3*8] = {
  0x00, 0x7e, 0x40, 0x73, 0x46, 0x4c, 0x18, 0x13,
  0x00, 0x00, 0xff, 0x81, 0x34, 0xc9, 0x00, 0xb6,
  0x00, 0x7e, 0x02, 0xce, 0x72, 0x32, 0x18, 0x48,

  0x30, 0x27, 0x24, 0x20, 0x37, 0x24, 0x20, 0x33,
  0x00, 0x7b, 0x42, 0x00, 0x7b, 0x42, 0x00, 0x3b,
  0x0c, 0x24, 0x24, 0x04, 0xa4, 0x24, 0x04, 0x4c,

  0x12, 0x19, 0x4c, 0x46, 0x63, 0x40, 0x7c, 0x00,
  0x22, 0x91, 0x00, 0xc4, 0x81, 0xff, 0x00, 0x00,
  0x08, 0x18, 0x32, 0x62, 0xc6, 0x02, 0x3e, 0x00
};

static char wwwicon_textmap[9] = {
  'w', 'w', 'w',
  '(', ')', ' ',
  ' ', '(', ')'
};

static struct ctk_icon wwwicon =
  {CTK_ICON("Web browser", wwwicon_bitmap, wwwicon_textmap)};
#endif /* WITH_WWW */

#ifdef WITH_WEBSERVER
/* The icon for the web server */
static unsigned char webservericon_bitmap[3*3*8] = {
  0x00, 0x7f, 0x40, 0x41, 0x44, 0x48, 0x40, 0x50,
  0x00, 0xff, 0x5a, 0x00, 0x00, 0x00, 0x3c, 0x81,
  0x00, 0xfe, 0x02, 0x82, 0x22, 0x12, 0x02, 0x0a,

  0x41, 0x60, 0x42, 0x62, 0x62, 0x42, 0x60, 0x41,
  0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18,
  0x82, 0x06, 0x42, 0x46, 0x46, 0x42, 0x06, 0x82,

  0x50, 0x40, 0x48, 0x44, 0x41, 0x40, 0x7e, 0x00,
  0xc5, 0x34, 0x3c, 0x52, 0x7a, 0x7e, 0xa1, 0xfd,
  0x0a, 0x02, 0x12, 0x22, 0x82, 0x02, 0x7e, 0x00
};

static char webservericon_textmap[9] = {
  '+', '-', '+',
  '|', ')', '|',
  '+', '-', '+'
};

static struct ctk_icon webservericon =
  {CTK_ICON("Web server", webservericon_bitmap, webservericon_textmap)};
#endif /* WITH_WEBSERVER */

#ifdef WITH_TELNET 
/* The icon for the telnet client */
static unsigned char telneticon_bitmap[3*3*8] = {
  0x00, 0x7f, 0x43, 0x4c, 0x58, 0x53, 0x60, 0x6f,
  0x00, 0xff, 0x00, 0x7e, 0x00, 0xff, 0x00, 0xff,
  0x00, 0xfe, 0xc2, 0x32, 0x1a, 0xca, 0x06, 0xf6,

  0x40, 0x5f, 0x40, 0x5f, 0x40, 0x5f, 0x40, 0x4f,
  0x00, 0xff, 0x00, 0xff, 0x00, 0xfc, 0x01, 0xf3,
  0x02, 0xfa, 0x02, 0x82, 0x3e, 0xfe, 0xfe, 0xfe,

  0x60, 0x67, 0x50, 0x59, 0x4c, 0x43, 0x7f, 0x00,
  0x07, 0xe7, 0x0f, 0xef, 0x0f, 0x0f, 0xff, 0x00,
  0x8e, 0x06, 0x06, 0x06, 0x8e, 0xfe, 0xfe, 0x00
};

static char telneticon_textmap[9] = {
  't', 'e', 'l',
  'n', 'e', 't',
  '-', '-', '-'
};

static struct ctk_icon telneticon =
  {CTK_ICON("Telnet client", telneticon_bitmap, telneticon_textmap)};
#endif /* WITH_TELNET */


/* Main window */
static struct ctk_window runwindow;

static struct ctk_label namelabel =
  {CTK_LABEL(0, 0, 13, 1, "Program name:")};

static char name[31];
static struct ctk_textentry nameentry =
  {CTK_TEXTENTRY(0, 1, 14, 1, name, 30)};

static struct ctk_button loadbutton =
  {CTK_BUTTON(10, 2, 4, "Load")};

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
  "Memory error",
  "Open error"
};

static ek_signal_t loader_signal_load;

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


/* Function declarations for init() functions for external
   programs: */

void about_init(void);
void netconf_init(void);
void processes_init(void);
void www_init(void);
void webserver_init(void);
void simpletelnet_init(void);
void email_init(void);
/*-----------------------------------------------------------------------------------*/
void
program_handler_init(void)     
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);

    /* Create the menus */
    ctk_menu_new(&contikimenu, "Contiki");
    ctk_menu_add(&contikimenu);
    menuitem_about = ctk_menuitem_add(&contikimenu, "About");
    menuitem_processes = ctk_menuitem_add(&contikimenu, "Processes");
    menuitem_tcpip = ctk_menuitem_add(&contikimenu, "TCP/IP conf");
    
    ctk_menu_new(&programsmenu, "Programs");
    ctk_menu_add(&programsmenu);
#ifdef WITH_WWW
    menuitem_www = ctk_menuitem_add(&programsmenu, "Web browser");
#endif /* WITH_WWW */    
#ifdef WITH_EMAIL
    menuitem_email = ctk_menuitem_add(&programsmenu, "E-mail");
#endif /* WITH_EMAIL */
#ifdef WITH_WEBSERVER
    menuitem_webserver = ctk_menuitem_add(&programsmenu, "Web server");
#endif /* WITH_WEBSERVER */
#ifdef WITH_TELNET
    menuitem_telnet = ctk_menuitem_add(&programsmenu, "Telnet client");
#endif /* WITH_TELNET */

#ifdef WITH_LOADER_ARCH
    menuitem_run = ctk_menuitem_add(&programsmenu, "Run...");
#endif /* WITH_LOADER_ARCH */
    

    CTK_ICON_ADD(&abouticon, id);
    CTK_ICON_ADD(&tcpipconficon, id);

#ifdef WITH_WWW  
    CTK_ICON_ADD(&wwwicon, id);
#endif /* WITH_WWW */
#ifdef WITH_WEBSERVER
    CTK_ICON_ADD(&webservericon, id);
#endif /* WITH_WEBSERVER */  
#ifdef WITH_TELNET
    CTK_ICON_ADD(&telneticon, id);
#endif /* WITH_TELNET */  
    

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
  dispatcher_emit(loader_signal_load, name, DISPATCHER_CURRENT());
  ctk_label_set_text(&loadingname, name);
  ctk_dialog_open(&loadingdialog);
  ctk_redraw();
#endif /* WITH_LOADER_ARCH */
}

#ifdef WITH_LOADER_ARCH
#define RUN(prg, name) program_handler_load(prg)
#else /* WITH_LOADER_ARCH */
#define RUN(prg, initfunc) initfunc(); ctk_redraw()
#endif /* WITH_LOADER_ARCH */
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(program_handler_sighandler, s, data)
{
  unsigned char err;
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_button_activate) {
    if(data == (ek_data_t)&loadbutton) {
      ctk_window_close(&runwindow);
#ifdef WITH_LOADER_ARCH
      program_handler_load(name);
#endif /* WITH_LOADER_ARCH */
    } else if(data == (ek_data_t)&errorokbutton) {
      ctk_dialog_close();
      ctk_redraw();
    }
    
    if((struct ctk_widget *)data == (struct ctk_widget *)&abouticon) {
      RUN("about.prg", about_init);
    } 
    if((struct ctk_widget *)data == (struct ctk_widget *)&tcpipconficon) {
      RUN("netconf.prg", netconf_init);
    } 

#ifdef WITH_WWW    
    if((struct ctk_widget *)data == (struct ctk_widget *)&wwwicon) {
      RUN("www.prg", www_init);
    } 
#endif /* WITH_WWW */
#ifdef WITH_WEBSERVER      
    if((struct ctk_widget *)data ==
       (struct ctk_widget *)&webservericon) {
      RUN("webserver.prg", webserver_init);
    } 
#endif /* WITH_WEBSERVER */
#ifdef WITH_TELNET
    if((struct ctk_widget *)data ==
       (struct ctk_widget *)&telneticon) {
      RUN("simpletelnet.prg", simpletelnet_init);
    } 
#endif /* WITH_TELNET */  
    
  } else if(s == ctk_signal_menu_activate) {
    if((struct ctk_menu *)data == &programsmenu) {
      if(0) {
#ifdef WITH_WWW
      } else if(programsmenu.active == menuitem_www) {
	RUN("www.prg", www_init);
#endif /* WITH_WWW */
#ifdef WITH_EMAIL	
      } else if(programsmenu.active == menuitem_email) {
	RUN("email.prg", email_init);
#endif /* WITH_EMAIL */
#ifdef WITH_TELNET
      } else if(programsmenu.active == menuitem_telnet) {
	RUN("simpletelnet.prg", simpletelnet_init);
#endif /* WITH_TELNET */
#ifdef WITH_WEBSERVER
      } else if(programsmenu.active == menuitem_webserver) {
	RUN("webserver.prg", webserver_init);
#endif /* WITH_WEBSERVER */
#ifdef WITH_LOADER_ARCH
      } else if(programsmenu.active == menuitem_run) {
	ctk_window_open(&runwindow);
	ctk_redraw();
#endif /* WITH_LOADER_ARCH */
      }
    } else if((struct ctk_menu *)data == &contikimenu) {
      if(contikimenu.active == menuitem_about) {
	RUN("about.prg", about_init);
      } else if(contikimenu.active == menuitem_tcpip) {
	RUN("netconf.prg", netconf_init);
      } else if(contikimenu.active == menuitem_processes) {
	RUN("processes.prg", processes_init);
      } 
    }
  } else if(s == loader_signal_load) {
    ctk_dialog_close();
    err = LOADER_LOAD(data);
    if(err != LOADER_OK) {
      ctk_label_set_text(&errortype, errormsgs[err]);
      ctk_dialog_open(&errordialog);
    }
    ctk_redraw();
  } 
}
/*-----------------------------------------------------------------------------------*/
