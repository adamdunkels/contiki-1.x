/*
 * Copyright (c) 2001, Adam Dunkels.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Adam Dunkels.
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
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: httpd.c,v 1.1 2003/07/04 10:54:51 adamdunkels Exp $
 *
 */


#include "uip.h"
#include "httpd.h"
#include "httpd-fs.h"
#include "httpd-fsdata.h"
#include "httpd-cgi.h"

#include "webserver.h"

#include <avr/pgmspace.h>

/* The HTTP server states: */
#define HTTP_DEALLOCATED  0
#define HTTP_NOGET        1
#define HTTP_FILE         2
#define HTTP_TEXT         3
#define HTTP_FUNC         4
#define HTTP_END          5

#ifdef DEBUG
#include <stdio.h>
#define PRINT(x) printf("%s", x)
#define PRINTLN(x) printf("%s\n", x)
#else /* DEBUG */
#define PRINT(x)
#define PRINTLN(x)
#endif /* DEBUG */

struct httpd_state *hs;

extern const struct httpd_fsdata_file file_index_html;
extern const struct httpd_fsdata_file file_404_html;

static void next_scriptline(void);
static void next_scriptstate(void);

#define ISO_G        0x47
#define ISO_E        0x45
#define ISO_T        0x54
#define ISO_slash    0x2f    
#define ISO_c        0x63
#define ISO_g        0x67
#define ISO_i        0x69
#define ISO_space    0x20
#define ISO_nl       0x0a
#define ISO_cr       0x0d
#define ISO_a        0x61
#define ISO_t        0x74
#define ISO_hash     0x23
#define ISO_period   0x2e

#define HTTPD_CONF_NUMCONNS UIP_CONNS
static struct httpd_state conns[HTTPD_CONF_NUMCONNS];
static u8_t i;

static char tmp[40];
/*-----------------------------------------------------------------------------------*/
static struct httpd_state *
alloc_state(void)
{
  
  for(i = 0; i < HTTPD_CONF_NUMCONNS; ++i) {
    if(conns[i].state == HTTP_DEALLOCATED) {
      return &conns[i];
    }
  }

  /* We are overloaded! XXX: we'll just kick all other connections! */
  for(i = 0; i < HTTPD_CONF_NUMCONNS; ++i) {
    conns[i].state = HTTP_DEALLOCATED;
  }
  
  return NULL;
}
/*-----------------------------------------------------------------------------------*/
static void
dealloc_state(struct httpd_state *s)
{
  s->state = HTTP_DEALLOCATED;
}
/*-----------------------------------------------------------------------------------*/
void
httpd_init(void)
{
  httpd_fs_init();
  
  /* Listen to port 80. */
  dispatcher_uiplisten(80);

  for(i = 0; i < HTTPD_CONF_NUMCONNS; ++i) {
    conns[i].state = HTTP_DEALLOCATED;
  }
}
/*-----------------------------------------------------------------------------------*/
DISPATCHER_UIPCALL(httpd_appcall, state)
{
  struct httpd_fs_file fsfile;  
  u8_t i;
  char c;
  DISPATCHER_UIPCALL_ARG(state);

  hs = (struct httpd_state *)(state);
  
  /* We use the uip_ test functions to deduce why we were
     called. If uip_connected() is non-zero, we were called
     because a remote host has connected to us. If
     uip_newdata() is non-zero, we were called because the
     remote host has sent us new data, and if uip_acked() is
     non-zero, the remote host has acknowledged the data we
     previously sent to it. */
  if(uip_connected()) {

    /* Since we've just been connected, the state pointer should be
       NULL and we need to allocate a new state object. If we have run
       out of memory for state objects, we'll have to abort the
       connection and return. */
    if(hs == NULL) {
      hs = alloc_state();
      if(hs == NULL) {
	uip_close();
	return;
      }
      dispatcher_markconn(uip_conn, (void *)hs);
    }
    /* Since we have just been connected with the remote host, we
       reset the state for this connection. The ->count variable
       contains the amount of data that is yet to be sent to the
       remote host, and the ->state is set to HTTP_NOGET to signal
       that we haven't received any HTTP GET request for this
       connection yet. */
    hs->state = HTTP_NOGET;
    hs->count = 0;
    hs->poll = 0;
  } else if(uip_closed() || uip_aborted()) {
    if(hs != NULL) {
      dealloc_state(hs);
    }
    return;
  } else if(uip_poll()) {
    /* If we are polled ten times, we abort the connection. This is
       because we don't want connections lingering indefinately in
       the system. */
    if(hs != NULL) {
      if(hs->state == HTTP_DEALLOCATED) {
	uip_abort();
      } else if(hs->poll++ >= 100) {
	uip_abort();
	dealloc_state(hs);
      }
    }
    return;
  }


  if(uip_newdata() && hs->state == HTTP_NOGET) {
    hs->poll = 0;
    /* This is the first data we receive, and it should contain a
       GET. */
      
    /* Check for GET. */
    if(uip_appdata[0] != ISO_G ||
       uip_appdata[1] != ISO_E ||
       uip_appdata[2] != ISO_T ||
       uip_appdata[3] != ISO_space) {
      /* If it isn't a GET, we abort the connection. */
      uip_abort();
      dealloc_state(hs);
      return;
    }
	       
    /* Find the file we are looking for. */
    for(i = 4; i < 40; ++i) {
      if(uip_appdata[i] == ISO_space ||
	 uip_appdata[i] == ISO_cr ||
	 uip_appdata[i] == ISO_nl) {
	uip_appdata[i] = 0;
	break;
      }
    }

    PRINT("request for file ");
    PRINTLN(&uip_appdata[4]);
    webserver_log_file(uip_conn->ripaddr, &uip_appdata[4]);
    /* Check for a request for "/". */
    if(uip_appdata[4] == ISO_slash &&
       uip_appdata[5] == 0) {
      memcpy_P(tmp, file_index_html.name, sizeof(tmp));
      httpd_fs_open((const char *)tmp, &fsfile);  
    } else {
      if(!httpd_fs_open((const char *)&uip_appdata[4], &fsfile)) {
	PRINTLN("couldn't open file");
	memcpy_P(tmp, file_404_html.name, sizeof(tmp));
	httpd_fs_open((const char *)tmp, &fsfile);  
      }
    } 
    httpd_fs_inc();
    
    if(uip_appdata[4] == ISO_slash &&
       uip_appdata[5] == ISO_c &&
       uip_appdata[6] == ISO_g &&
       uip_appdata[7] == ISO_i &&
       uip_appdata[8] == ISO_slash) {
      /* If the request is for a file that starts with "/cgi/", we
	 prepare for invoking a script. */	
      hs->script = fsfile.data;
      next_scriptstate();
    } else {
      hs->script = NULL;
      /* The web server is now no longer in the HTTP_NOGET state, but
	 in the HTTP_FILE state since is has now got the GET from
	 the client and will start transmitting the file. */
      hs->state = HTTP_FILE;

      /* Point the file pointers in the connection state to point to
	 the first byte of the file. */
      hs->dataptr = fsfile.data;
      hs->count = fsfile.len;	
    }     
  }

    
  if(hs->state != HTTP_FUNC) {
    /* Check if the client (remote end) has acknowledged any data that
       we've previously sent. If so, we move the file pointer further
       into the file and send back more data. If we are out of data to
       send, we close the connection. */
    if(uip_acked()) {
      hs->poll = 0;	
      if(hs->count >= uip_mss()) {
	hs->count -= uip_mss();
	hs->dataptr += uip_mss();
      } else {
	hs->count = 0;
      }
	
      if(hs->count == 0) {
	if(hs->script != NULL) {
	  next_scriptline();
	  next_scriptstate();
	} else {
	  uip_close();
	  dealloc_state(hs);
	}
      }
    }         
  }
    
  if(hs->state == HTTP_FUNC) {
    /* Call the CGI function. */
    memcpy_P(&c, &hs->script[2], 1);
    if(httpd_cgitab[c - ISO_a]()) {
      /* If the function returns non-zero, we jump to the next line
	 in the script. */
      next_scriptline();
      next_scriptstate();
    }
  }

  if(hs->state != HTTP_FUNC && !uip_poll()) {
    hs->poll = 0;
    /* Send a piece of data, but not more than the MSS of the
       connection. */
    memcpy_P(uip_appdata, hs->dataptr, uip_mss());
    uip_send(uip_appdata,
	     hs->count > uip_mss()? uip_mss(): hs->count);
  }

  /* Finally, return to uIP. Our outgoing packet will soon be on its
     way... */
}
/*-----------------------------------------------------------------------------------*/
/* next_scriptline():
 *
 * Reads the script until it finds a newline. */
static void
next_scriptline(void)
{
  char c;
  /* Loop until we find a newline character. */
  do {
    ++(hs->script);
    memcpy_P(&c, hs->script, 1);
  } while(c != ISO_nl);

  /* Eat up the newline as well. */
  ++(hs->script);
}
/*-----------------------------------------------------------------------------------*/
/* next_sciptstate:
 *
 * Reads one line of script and decides what to do next.
 */
static void
next_scriptstate(void)
{
  struct httpd_fs_file fsfile;
  u8_t i;
  char c;

 again:
  memcpy_P(&c, hs->script, 1);
  switch(c) {
  case ISO_t:
    /* Send a text string. */
    hs->state = HTTP_TEXT;
    hs->dataptr = hs->script + 2;

    /* Calculate length of string. */
    i = 0;
    do {
      memcpy_P(&c, &hs->dataptr[i], 1);
      ++i;
    } while(c != ISO_nl);
    hs->count = i;    
    break;
  case ISO_c:
    /* Call a function. */
    hs->state = HTTP_FUNC;
    hs->dataptr = NULL;
    hs->count = 0;
    uip_reset_acked();
    break;
  case ISO_i:   
    /* Include a file. */
    hs->state = HTTP_FILE;
    memcpy_P(tmp, &hs->script[2], sizeof(tmp));
    if(!httpd_fs_open(tmp, &fsfile)) {
      uip_abort();
      dealloc_state(hs);
    }
    hs->dataptr = fsfile.data;
    hs->count = fsfile.len;
    break;
  case ISO_hash:
    /* Comment line. */
    next_scriptline();
    goto again;
    break;
  case ISO_period:
    /* End of script. */
    hs->state = HTTP_END;
    uip_close();
    dealloc_state(hs);
    break;
  default:
    uip_abort();
    dealloc_state(hs);
    break;
  }
}
/*-----------------------------------------------------------------------------------*/
