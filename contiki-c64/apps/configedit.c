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
 * $Id: configedit.c,v 1.5 2003/08/15 18:44:26 adamdunkels Exp $
 *
 */

#include "uip_main.h"
#include "uip.h"
#include "uip_arp.h"
#include "resolv.h"
#include "ctk.h"
#include "ctk-draw.h"
#include "dispatcher.h"

#include "uip-signal.h"

#include "program-handler.h"

#include "c64-fs.h"

#include "loader.h"

#include <stdio.h>

/* TCP/IP configuration window. */
static struct ctk_window window;

#define LABELMAXWIDTH 12

static struct ctk_label themelabel =
  {CTK_LABEL(0, 1, LABELMAXWIDTH, 1, "CTK theme")};
static char theme[25];
static struct ctk_textentry themetextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 1, 16, 1, theme, 24)};

static struct ctk_label driverlabel =
  {CTK_LABEL(0, 3, LABELMAXWIDTH, 1, "Net driver")};
static char driver[25];
static struct ctk_textentry drivertextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 3, 16, 1, driver, 24)};

static struct ctk_label screensaverlabel =
  {CTK_LABEL(0, 5, LABELMAXWIDTH, 1, "Screensaver")};
static char screensaver[25];
static struct ctk_textentry screensavertextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 5, 16, 1, screensaver, 24)};


static struct ctk_label ipaddrlabel =
  {CTK_LABEL(0, 7, LABELMAXWIDTH, 1, "IP address")};
static char ipaddr[25];
static struct ctk_textentry ipaddrtextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 7, 16, 1, ipaddr, 24)};
static struct ctk_label netmasklabel =
  {CTK_LABEL(0, 9, LABELMAXWIDTH, 1, "Netmask")};
static char netmask[25];
static struct ctk_textentry netmasktextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 9, 16, 1, netmask, 24)};
static struct ctk_label gatewaylabel =
  {CTK_LABEL(0, 11, LABELMAXWIDTH, 1, "Gateway")};
static char gateway[25];
static struct ctk_textentry gatewaytextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 11, 16, 1, gateway, 24)};
static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 13, LABELMAXWIDTH, 1, "DNS server")};
static char dnsserver[25];
static struct ctk_textentry dnsservertextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 13, 16, 1, dnsserver, 24)};

static struct ctk_button savebutton =
  {CTK_BUTTON(0, 15, 12, "Save & close")};


static struct ctk_button applybutton =
  {CTK_BUTTON(17, 15, 13, "Apply & close")};

static DISPATCHER_SIGHANDLER(configedit_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Config editor", NULL, configedit_sighandler, NULL)};
static ek_id_t id;




/*-----------------------------------------------------------------------------------*/
struct ptentry {
  char c;
  char * (* pfunc)(char *str);
};

/*-----------------------------------------------------------------------------------*/
static char *
parse(char *str, struct ptentry *t)
{
  struct ptentry *p;

  /* Loop over the parse table entries in t in order to find one that
     matches the first character in str. */
  for(p = t; p->c != 0; ++p) {
    if(*str == p->c) {
      /* Skip rest of the characters up to the first space. */
      while(*str != ' ') {
	++str;
      }

      /* Skip all spaces.*/
      while(*str == ' ') {
	++str;
      }

      /* Call parse table entry function and return. */
      return p->pfunc(str);
    }
  }

  /* Did not find matching entry in parse table. We just call the
     default handler supplied by the caller and return. */
  return p->pfunc(str);
}
/*-----------------------------------------------------------------------------------*/
static char *
skipnewline(char *str)
{
  /* Skip all characters until the newline. */
  while(*str != '\n') {
    ++str;
  }

  /* Return a pointer to the first character after the newline. */
  return str + 1;
}
/*-----------------------------------------------------------------------------------*/
static char *
nullterminate(char *str)
{
  char *nt;

  /* Nullterminate string. Start with finding newline character. */
  for(nt = str; *nt != '\r' &&
	*nt != '\n'; ++nt);

  /* Replace newline with a null char. */
  *nt = 0;

  /* Return pointer to null char. */
  return nt;
}
/*-----------------------------------------------------------------------------------*/
static char * __fastcall__
copystr(char *dst, char *src, int len)
{
  char *nt = nullterminate(src);
  strncpy(dst, src, len);
  return nt + 1;
}
/*-----------------------------------------------------------------------------------*/
static char *
loaddriver(char *str)
{
  return copystr(driver, str, sizeof(driver));
}
/*-----------------------------------------------------------------------------------*/
static char *
loadtheme(char *str)
{
  return copystr(theme, str, sizeof(theme));
}
/*-----------------------------------------------------------------------------------*/
static char *
loadscreensaver(char *str)
{
  return copystr(screensaver, str, sizeof(screensaver));
}
/*-----------------------------------------------------------------------------------*/
static char *
ipaddrconf(char *str)
{
  return copystr(ipaddr, str, sizeof(ipaddr));
}
/*-----------------------------------------------------------------------------------*/
static char *
netmaskconf(char *str)
{
  return copystr(netmask, str, sizeof(netmask));
}
/*-----------------------------------------------------------------------------------*/
static char *
drconf(char *str)
{
  return copystr(gateway, str, sizeof(gateway));
}
/*-----------------------------------------------------------------------------------*/
static char *
dnsconf(char *str)
{
  return copystr(dnsserver, str, sizeof(dnsserver));
}
/*-----------------------------------------------------------------------------------*/
static struct ptentry initparsetab[] =
  {{'l', loaddriver},
   {'t', loadtheme},
   {'s', loadscreensaver},
   {'i', ipaddrconf},
   {'n', netmaskconf},
   {'r', drconf},
   {'d', dnsconf},
   {'#', skipnewline},

   /* Default action */
   {0, skipnewline}};
static void
initscript(void)
{
  char line[40], *lineptr;
  struct c64_fs_file f;

  if(c64_fs_open("config.cfg", &f) == -1) {
    return;
  }
  line[0] = ' ';
  while(line[0] != '.' &&
	line[0] != 0) {
    lineptr = line;
    do {
      if(c64_fs_read(&f, lineptr, 1) != 1) {
	c64_fs_close(&f);
	return;
      }
      ++lineptr;
    } while(*(lineptr - 1) != '\n' &&
	    *(lineptr - 1) != '\r');

    *lineptr = 0;

    if(line[0] != '.' &&
       line[0] != 0) {
      parse(line, initparsetab);
    }
    
  }
  c64_fs_close(&f);
  return;
}
/*-----------------------------------------------------------------------------------*/
static int
makeline(char *line, char c, char *str)
{
  int len;

  len = strlen(str);
  
  line[0] = c;
  line[1] = ' ';
  strcpy(&line[2], str);
  line[2 + len] = '\n';
  line[3 + len] = 0;
  return len + 3;
}
/*-----------------------------------------------------------------------------------*/
static void
savescript(void)
{
  char line[40];
  struct c64_fs_file f;

  if(c64_fs_open("config.cfg", &f) == -1) {
    return;
  }
  if(theme[0] != 0) {
    c64_fs_write(&f, line, makeline(line, 't', theme));
  }
  if(driver[0] != 0) {
    c64_fs_write(&f, line, makeline(line, 'l', driver));
  }
  if(ipaddr[0] != 0) {
    c64_fs_write(&f, line, makeline(line, 'i', ipaddr));
  }
  if(netmask[0] != 0) {
    c64_fs_write(&f, line, makeline(line, 'n', netmask));
  }
  if(gateway[0] != 0) {
    c64_fs_write(&f, line, makeline(line, 'r', gateway));
  }
  if(dnsserver[0] != 0) {
    c64_fs_write(&f, line, makeline(line, 'd', dnsserver));
  }
  
  if(screensaver[0] != 0) {
    c64_fs_write(&f, line, makeline(line, 's', screensaver));
  }
  
  strcpy(line, ".\n\0\n\n\n");
  c64_fs_write(&f, line, strlen(line));
  
  c64_fs_close(&f);
  
}
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(configedit_init)
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    
    /* Create window. */
    ctk_window_new(&window, 32, 16, "Config editor");

    CTK_WIDGET_ADD(&window, &themelabel);  
    CTK_WIDGET_ADD(&window, &themetextentry);

    CTK_WIDGET_ADD(&window, &driverlabel);  
    CTK_WIDGET_ADD(&window, &drivertextentry);

    CTK_WIDGET_ADD(&window, &screensaverlabel);  
    CTK_WIDGET_ADD(&window, &screensavertextentry);

    CTK_WIDGET_ADD(&window, &ipaddrlabel);  
    CTK_WIDGET_ADD(&window, &ipaddrtextentry);
    CTK_WIDGET_ADD(&window, &netmasklabel);
    CTK_WIDGET_ADD(&window, &netmasktextentry);
    CTK_WIDGET_ADD(&window, &gatewaylabel);
    CTK_WIDGET_ADD(&window, &gatewaytextentry);
    CTK_WIDGET_ADD(&window, &dnsserverlabel);
    CTK_WIDGET_ADD(&window, &dnsservertextentry);

    CTK_WIDGET_ADD(&window, &savebutton);
    CTK_WIDGET_ADD(&window, &applybutton);
    
    CTK_WIDGET_FOCUS(&window, &themetextentry);  

    /* Fill the configuration strings with values from the current
       configuration */
    initscript();
    
    dispatcher_listen(ctk_signal_button_activate);
    dispatcher_listen(ctk_signal_window_close);
  }
  ctk_window_open(&window);
}
/*-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------*/
static void
configedit_quit(void)
{
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(configedit_sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_button_activate) {   
    if(data == (ek_data_t)&savebutton) {
      savescript();
      ctk_window_close(&window);
      configedit_quit();      
    } else if(data == (ek_data_t)&applybutton) {
      savescript();
      ctk_window_close(&window);
      configedit_quit();
      dispatcher_emit(uip_signal_uninstall, NULL,
		      DISPATCHER_BROADCAST);
      program_handler_load("config.prg");
    }
  } else if(s == ctk_signal_window_close ||
	    s == dispatcher_signal_quit) {
    ctk_window_close(&window);
    configedit_quit();
  }
}
/*-----------------------------------------------------------------------------------*/
