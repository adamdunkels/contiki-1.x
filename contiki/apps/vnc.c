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
 * This file is part of the Contiki VNC client
 *
 * $Id: vnc.c,v 1.2 2003/04/09 13:45:05 adamdunkels Exp $
 *
 */

#include "petsciiconv.h"
#include "uip_main.h"
#include "uip.h"
#include "ctk.h"
#include "dispatcher.h"
#include "resolv.h"
#include "telnet.h"
#include "vnc.h"
#include "vnc-draw.h"
#include "vnc-viewer.h"
#include "vnc-conf.h"

#include "loader.h"

#define HEIGHT (4 + VNC_CONF_VIEWPORT_HEIGHT/8)

/* Main window */
static struct ctk_window mainwindow;

static char host[20];
static struct ctk_textentry hosttextentry =
  {CTK_TEXTENTRY(0, 0, 18, 1, host, 18)};

static char portentry[4];
static struct ctk_textentry porttextentry =
  {CTK_TEXTENTRY(21, 0, 3, 1, portentry, 3)};

static struct ctk_button connectbutton =
  {CTK_BUTTON(27, 0, 7, "Connect")};
/*static struct ctk_button disconnectbutton =
  {CTK_BUTTON(25, 3, 10, "Disconnect")};*/

static struct ctk_separator sep1 =
  {CTK_SEPARATOR(0, 1, 36)};

static struct ctk_bitmap vncbitmap =
  {CTK_BITMAP(2, 2,
	      VNC_CONF_VIEWPORT_WIDTH / 8,
	      VNC_CONF_VIEWPORT_HEIGHT / 8,
	      vnc_draw_bitmap)};

static struct ctk_button leftbutton =
  {CTK_BUTTON(6, HEIGHT - 1, 4, "Left")};

static struct ctk_button upbutton =
  {CTK_BUTTON(13, HEIGHT - 1, 2, "Up")};

static struct ctk_button downbutton =
  {CTK_BUTTON(18, HEIGHT - 1, 4, "Down")};

static struct ctk_button rightbutton =
  {CTK_BUTTON(25, HEIGHT - 1, 5, "Right")};

static DISPATCHER_SIGHANDLER(vnc_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("VNC client", NULL, vnc_sighandler,
		   (void (*)(void *))vnc_viewer_app)};
static ek_id_t id;

/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(vnc_init)
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);

    ctk_window_new(&mainwindow, 36, HEIGHT, "VNC client");
    ctk_window_move(&mainwindow, 0, 0);
    
    CTK_WIDGET_ADD(&mainwindow, &hosttextentry);
    CTK_WIDGET_FOCUS(&mainwindow, &hosttextentry);
    CTK_WIDGET_ADD(&mainwindow, &porttextentry);
    CTK_WIDGET_ADD(&mainwindow, &connectbutton);

    CTK_WIDGET_ADD(&mainwindow, &sep1);
    
    CTK_WIDGET_ADD(&mainwindow, &vncbitmap);

    CTK_WIDGET_ADD(&mainwindow, &leftbutton);
    CTK_WIDGET_ADD(&mainwindow, &upbutton);
    CTK_WIDGET_ADD(&mainwindow, &downbutton);
    CTK_WIDGET_ADD(&mainwindow, &rightbutton);

    dispatcher_listen(ctk_signal_window_close);
    dispatcher_listen(ctk_signal_button_activate);
    dispatcher_listen(resolv_signal_found);

    
    vnc_draw_init();
  }
  
  ctk_window_open(&mainwindow);

}
/*-----------------------------------------------------------------------------------*/
static void
show(char *text)
{

}
/*-----------------------------------------------------------------------------------*/
static void
connect(void)
{
  u16_t addr[2], *addrptr;
  u16_t port;
  char *cptr;

  /* Find the first space character in host and put a zero there
     to end the string. */
  for(cptr = host; *cptr != ' ' && *cptr != 0; ++cptr);
  *cptr = 0;

  addrptr = &addr[0];  
  if(uip_main_ipaddrconv(host, (unsigned char *)addr) == 0) {
    addrptr = resolv_lookup(host);
    if(addrptr == NULL) {
      resolv_query(host);
      show("Resolving host...");
      return;
    }
  }

  port = 0;
  for(cptr = portentry; *cptr != ' ' && *cptr != 0; ++cptr) {
    if(*cptr < '0' || *cptr > '9') {
      show("Port number error");
      return;
    }
    port = 10 * port + *cptr - '0';
  }


  vnc_viewer_connect(addrptr, port);

  show("Connecting...");

}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(vnc_sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);
    
  if(s == ctk_signal_button_activate) {
    if(data == (ek_data_t)&connectbutton) {
      connect();
    }
  } else if(s == ctk_signal_window_close) {
    dispatcher_exit(&p);
    id = EK_ID_NONE;
    LOADER_UNLOAD();
  } else if(s == resolv_signal_found) {
    if(strcmp(data, host) == 0) {
      if(resolv_lookup(host) != NULL) {
	connect();
      } else {
	show("Host not found");
      }
    }
  }
}
/*-----------------------------------------------------------------------------------*/
void
vnc_viewer_refresh(void)
{
  CTK_WIDGET_REDRAW(&vncbitmap);
}
/*-----------------------------------------------------------------------------------*/
