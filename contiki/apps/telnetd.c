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
 * $Id: telnetd.c,v 1.5 2003/10/08 10:29:04 adamdunkels Exp $
 *
 */

#include "program-handler.h"
#include "loader.h"
#include "uip.h"
#include "uip_main.h"
#include "petsciiconv.h"
#include "uip_arp.h"
#include "resolv.h"

#include "memb.h"

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
  {DISPATCHER_PROC("Shell server", shell_idle, sighandler,
		   telnetd_app)};
static ek_id_t id;

#define LINELEN 36
#define NUMLINES 24
/*static char lines[NUMLINES][LINELEN];*/
MEMB(linemem, LINELEN, NUMLINES);

static u8_t i;

struct telnetd_state {
  char *lines[NUMLINES];
  char buf[LINELEN];
  char bufptr;
  u8_t state;
#define STATE_NORMAL 0
#define STATE_IAC    1
#define STATE_WILL   2
#define STATE_WONT   3
#define STATE_DO     4  
#define STATE_DONT   5
  
#define STATE_CLOSE  6
};
static struct telnetd_state s;

#define TELNET_IAC   255
#define TELNET_WILL  251
#define TELNET_WONT  252
#define TELNET_DO    253
#define TELNET_DONT  254
/*-----------------------------------------------------------------------------------*/
static char *
alloc_line(void)
{  
  /*  for(i = 0; i < NUMLINES; ++i) {
    if(*(lines[i]) == 0) {
      return lines[i];
    }
  }
  return NULL;*/
  return memb_alloc(&linemem);
}
/*-----------------------------------------------------------------------------------*/
static void
dealloc_line(char *line)
{
  /*  *line = 0;*/
  memb_free(&linemem, line);
}
/*-----------------------------------------------------------------------------------*/
void
shell_quit(char *str)
{
  s.state = STATE_CLOSE;
}
/*-----------------------------------------------------------------------------------*/
void
quit(char *str)
{
  ctk_window_close(&window);
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static void
sendline(char *line)
{
  static unsigned int i;
  for(i = 0; i < NUMLINES; ++i) {
    if(s.lines[i] == NULL) {
      s.lines[i] = line;
      break;
    }
  }
  if(i == NUMLINES) {
    dealloc_line(line);
  }
}
/*-----------------------------------------------------------------------------------*/
void
shell_prompt(char *str)
{
  char *line;
  line = alloc_line();
  if(line != NULL) {
    strncpy(line, str, LINELEN);
    petsciiconv_toascii(line, LINELEN);
    sendline(line);
  }         
}
/*-----------------------------------------------------------------------------------*/
void
shell_output(char *str1, char *str2)
{
  static unsigned len;
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
    sendline(line);
    /*  } else {
	for(i = 1; i < YSIZE; ++i) {
	memcpy(&log[(i - 1) * XSIZE], &log[i * XSIZE], XSIZE);
	}
	memset(&log[(YSIZE - 1) * XSIZE], 0, XSIZE);
	strncpy(&log[(YSIZE - 1) * XSIZE], "Could not alloc line", XSIZE);*/
  }

  CTK_WIDGET_REDRAW(&loglabel);
}
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(telnetd_init, arg)
{
  arg_free(arg);
  
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    dispatcher_listen(ctk_signal_window_close);
    dispatcher_listen(ctk_signal_widget_activate);    

    ctk_window_new(&window, XSIZE, YSIZE, "Shell server");
    CTK_WIDGET_ADD(&window, &loglabel);
    memset(log, ' ', sizeof(log));

    dispatcher_uiplisten(HTONS(23));
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
    quit(NULL);
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
getchar(u8_t c)
{
  if(c == ISO_cr) {
    return;
  }
  
  s.buf[s.bufptr] = c;  
  if( s.buf[s.bufptr] == ISO_nl ||
     s.bufptr == sizeof(s.buf) - 1) {    
    if(s.bufptr > 0) {
      s.buf[s.bufptr] = 0;
      petsciiconv_topetscii(s.buf, LINELEN);
    }
    shell_input(s.buf);
    s.bufptr = 0;
  } else {
    ++s.bufptr;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
sendopt(u8_t option, u8_t value)
{
  char *line;
  line = alloc_line();
  if(line != NULL) {
    line[0] = TELNET_IAC;
    line[1] = option;
    line[2] = value;
    line[3] = 0;
    sendline(line);
  }       
}
/*-----------------------------------------------------------------------------------*/
static void
newdata(void)
{
  u16_t len;
  u8_t c;
    
  
  len = uip_datalen();
  
  while(len > 0 && s.bufptr < sizeof(s.buf)) {
    c = *uip_appdata;
    ++uip_appdata;
    --len;
    switch(s.state) {
    case STATE_IAC:
      if(c == TELNET_IAC) {
	getchar(c);
	s.state = STATE_NORMAL;
      } else {
	switch(c) {
	case TELNET_WILL:
	  s.state = STATE_WILL;
	  break;
	case TELNET_WONT:
	  s.state = STATE_WONT;
	  break;
	case TELNET_DO:
	  s.state = STATE_DO;
	  break;
	case TELNET_DONT:
	  s.state = STATE_DONT;
	  break;
	default:
	  s.state = STATE_NORMAL;
	  break;
	}
      }
      break;
    case STATE_WILL:
      /* Reply with a DONT */
      sendopt(TELNET_DONT, c);
      s.state = STATE_NORMAL;
      break;
      
    case STATE_WONT:
      /* Reply with a DONT */
      sendopt(TELNET_DONT, c);
      s.state = STATE_NORMAL;
      break;
    case STATE_DO:
      /* Reply with a WONT */
      sendopt(TELNET_WONT, c);
      s.state = STATE_NORMAL;
      break;
    case STATE_DONT:
      /* Reply with a WONT */
      sendopt(TELNET_WONT, c);
      s.state = STATE_NORMAL;
      break;
    case STATE_NORMAL:
      if(c == TELNET_IAC) {
	s.state = STATE_IAC;
      } else {
	getchar(c);
      }      
      break;
    } 

    
  }  
  
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_UIPCALL(telnetd_app, ts)
{
  if(uip_connected()) {
    memb_init(&linemem);
    dispatcher_markconn(uip_conn, &s);
    for(i = 0; i < NUMLINES; ++i) {
      s.lines[i] = NULL;
    }
    s.bufptr = 0;
    s.state = STATE_NORMAL;

    shell_init();
    senddata();
    return;
  }

  if(s.state == STATE_CLOSE) {
    s.state = STATE_NORMAL;
    uip_close();
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
