/*
 * Copyright (c) 2003, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
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
 * This file is part of the Contiki desktop OS.
 *
 * $Id: telnetd.c,v 1.3 2003/08/24 22:41:31 adamdunkels Exp $
 *
 */

#include "program-handler.h"
#include "loader.h"
#include "uip.h"
#include "uip_main.h"
#include "petsciiconv.h"
#include "uip_arp.h"
#include "resolv.h"

#include "shell.h"

#include "uip-signal.h"

#include <string.h>

#define ISO_nl       0x0a
#define ISO_cr       0x0d

#define XSIZE 36
#define YSIZE 12

static struct ctk_window window;
static char log[XSIZE * YSIZE];
static struct ctk_label loglabel =
  {CTK_LABEL(0, 0, XSIZE, YSIZE, log)};

static DISPATCHER_SIGHANDLER(sighandler, s, data);
static DISPATCHER_UIPCALL(telnetd_app, ts);

static struct dispatcher_proc p =
  {DISPATCHER_PROC("Remote command shell", shell_idle, sighandler,
		   telnetd_app)};
static ek_id_t id;

#define LINELEN 36
#define NUMLINES 16
static char lines[NUMLINES][LINELEN];
static u8_t i;

struct telnetd_state {
  char *lines[NUMLINES];
  char buf[LINELEN];
  char bufptr;
};
static struct telnetd_state s;

/*-----------------------------------------------------------------------------------*/
static char *
alloc_line(void)
{  
  for(i = 0; i < NUMLINES; ++i) {
    if(*(lines[i]) == 0) {
      return lines[i];
    }
  }
  return NULL;
}
/*-----------------------------------------------------------------------------------*/
static void
dealloc_line(char *line)
{
  *line = 0;
}
/*-----------------------------------------------------------------------------------*/
void
shell_quit(char *str)
{
  ctk_window_close(&window);
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
void
shell_output(char *str1, char *str2)
{
  static unsigned char i, len;
  char *line;
  
  for(i = 1; i < YSIZE; ++i) {
    memcpy(&log[(i - 1) * XSIZE], &log[i * XSIZE], XSIZE);
  }
  memset(&log[(YSIZE - 1) * XSIZE], 0, XSIZE);

  len = strlen(str1);

  strncpy(&log[(YSIZE - 1) * XSIZE], str1, XSIZE);
  if(len < XSIZE) {
    strncpy(&log[(YSIZE - 1) * XSIZE] + len, str2, XSIZE - len);
  }

  CTK_WIDGET_REDRAW(&loglabel);

  line = alloc_line();
  if(line != NULL) {
    len = strlen(str1);
    strncpy(line, str1, LINELEN);
    if(len < LINELEN) {
      strncpy(line + len, str2, LINELEN - len);
    }
    len = strlen(line);
    if(len < LINELEN - 2) {
      line[len] = ISO_cr;
      line[len+1] = ISO_nl;
      line[len+2] = 0;
    }
    petsciiconv_toascii(line, LINELEN);
    for(i = 0; i < NUMLINES; ++i) {
      if(s.lines[i] == NULL) {
	s.lines[i] = line;
	break;
      }
    }
    if(i == NUMLINES) {
      dealloc_line(line);
    }
    /*  } else {
	printf("COld not aloce\n");*/
  }
}
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(telnetd_init, arg)
{
  arg_free(arg);
  
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    dispatcher_listen(ctk_signal_window_close);
    dispatcher_listen(ctk_signal_widget_activate);    

    ctk_window_new(&window, XSIZE, YSIZE, "Remote command shell");
    CTK_WIDGET_ADD(&window, &loglabel);
    memset(log, ' ', sizeof(log));

    dispatcher_uiplisten(23);
  }
  ctk_window_open(&window);
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);

  if(s == ctk_signal_window_close ||
	    s == dispatcher_signal_quit) {
    shell_quit(NULL);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
acked(void)     
{
  dealloc_line(s.lines[0]);
  for(i = 1; i < NUMLINES; ++i) {
    s.lines[i - 1] = s.lines[i];
  }
}
/*-----------------------------------------------------------------------------------*/
static void
senddata(void)    
{
  if(s.lines[0] != NULL) {
    uip_send(s.lines[0], strlen(s.lines[0]));
  }
}
/*-----------------------------------------------------------------------------------*/
static void
newdata(void)
{
  u16_t len;

  len = uip_datalen();
  
  while(len > 0 && s.bufptr < sizeof(s.buf)) {

    s.buf[s.bufptr] = *uip_appdata;
    ++uip_appdata;
    --len;
    if(s.buf[s.bufptr] == ISO_cr ||
       s.buf[s.bufptr] == ISO_nl) {
      s.buf[s.bufptr] = 0;
      petsciiconv_topetscii(s.buf, LINELEN);
      shell_output(s.buf, "");
      shell_input(s.buf);
      s.bufptr = 0;
    } else {
      ++s.bufptr;
    }
  }
  
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_UIPCALL(telnetd_app, ts)
{
  if(uip_connected()) {
    dispatcher_markconn(uip_conn, &s);
    for(i = 0; i < NUMLINES; ++i) {
      s.lines[i] = NULL;
    }
    s.bufptr = 0;

    shell_output("Contiki remote command shell", "");
    shell_output("Type '?' for help", "");

    senddata();
    return;
  }
  
  if(uip_closed()) {
    shell_output("Connection closed", "");
  }

  
  if(uip_aborted()) {
    shell_output("Connection reset", "");
    /*    aborted();*/
  }
  
  if(uip_timedout()) {
    shell_output("Connection timed out", "");
  }
  
  if(uip_acked()) {
    acked();
  }
  
  if(uip_newdata()) {
    newdata();
  }
  
  if(uip_rexmit() ||
     uip_newdata() ||
     uip_acked()) {
    senddata();
  } else if(uip_poll()) {    
    senddata();
  }
}
/*-----------------------------------------------------------------------------------*/
