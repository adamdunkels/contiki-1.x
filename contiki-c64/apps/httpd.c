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
 * $Id: httpd.c,v 1.2 2003/08/20 19:52:41 adamdunkels Exp $
 *
 */


#include "uip.h"
#include "httpd.h"
#include "httpd-fs.h"
#include "httpd-fsdata.h"
#include "httpd-cgi.h"

#include "c64-fs-raw.h"

#include <stdio.h>

/* The HTTP server states: */
#define HTTP_DEALLOCATED  0
#define HTTP_NOGET        1
#define HTTP_FILE         2
#define HTTP_TEXT         3
#define HTTP_FUNC         4
#define HTTP_END          5

#define HTTP_DIRHDR       6
#define HTTP_DIR          7
#define HTTP_DIRFOOTER    8
#define HTTP_FSFILEHDR    9
#define HTTP_FSFILE       10

#ifdef DEBUG
#include <stdio.h>
#define PRINT(x) printf("%s", x)
#define PRINTLN(x) printf("%s\n", x)
#else /* DEBUG */
#define PRINT(x)
#define PRINTLN(x)
#endif /* DEBUG */

extern const struct httpd_fsdata_file file_index_html;
extern const struct httpd_fsdata_file file_404_html;
extern const struct httpd_fsdata_file file_dirheader_html;
extern const struct httpd_fsdata_file file_dirfooter_plain;
extern const struct httpd_fsdata_file file_binaryheader_plain;

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
#define ISO_8        0x38

static char getstring[] = {ISO_G, ISO_E, ISO_T, ISO_space};
static char cgistring[] = {ISO_slash, ISO_c, ISO_g, ISO_i,
			   ISO_slash};

struct httpd_state *hs;

#define HTTPD_CONF_NUMCONNS 4
static struct httpd_state conns[HTTPD_CONF_NUMCONNS];
static u8_t i;
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
static struct httpd_fs_file fsfile;  
static char
fileopen(const char *name, struct httpd_state *hs)
{
  char ret;
  
  ret = httpd_fs_open(name, &fsfile);
  hs->state = HTTP_FILE;
  hs->dataptr = fsfile.data;
  hs->count = fsfile.len;

  return ret;
}
/*-----------------------------------------------------------------------------------*/
DISPATCHER_UIPCALL(httpd_appcall, state)
{
  u8_t i;
  register char *appdata;
  register struct httpd_state *rhs;
  DISPATCHER_UIPCALL_ARG(state);

  rhs = hs = (struct httpd_state *)(state);
  appdata = uip_appdata;
  
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
    if(rhs == NULL) {
      rhs = alloc_state();
      if(rhs == NULL) {
	uip_close();
	return;
      }
      dispatcher_markconn(uip_conn, (void *)rhs);
    }
    /* Since we have just been connected with the remote host, we
       reset the state for this connection. The ->count variable
       contains the amount of data that is yet to be sent to the
       remote host, and the ->state is set to HTTP_NOGET to signal
       that we haven't received any HTTP GET request for this
       connection yet. */
    rhs->state = HTTP_NOGET;
    rhs->count = 0;
    rhs->poll = 0;
  } else if(uip_closed() || uip_aborted()) {
    if(rhs != NULL) {
      dealloc_state(rhs);
    }
    return;
  } else if(uip_poll()) {
    /* If we are polled ten times, we abort the connection. This is
       because we don't want connections lingering indefinately in
       the system. */
    if(rhs != NULL) {
      if(rhs->state == HTTP_DEALLOCATED) {
	uip_abort();
      } else if(rhs->poll++ >= 100) {
	uip_abort();
	dealloc_state(rhs);
      }
    }
    return;
  }


  if(uip_newdata() && rhs->state == HTTP_NOGET) {
    rhs->poll = 0;
    /* This is the first data we receive, and it should contain a
       GET. */
      
    /* Check for GET. */
    if(strncmp(appdata, getstring, 4) != 0) {
      /* If it isn't a GET, we abort the connection. */
      uip_abort();
      dealloc_state(rhs);
      return;
    }
	       
    /* Find the file we are looking for and null terminate the
       string. */
    for(i = 4; i < 40; ++i) {
      if(appdata[i] == ISO_space ||
	 appdata[i] == ISO_cr ||
	 appdata[i] == ISO_nl) {
	appdata[i] = 0;
	break;
      }
    }

    PRINT("request for file ");
    PRINTLN(&uip_appdata[4]);
    webserver_log_file(uip_conn->ripaddr, &appdata[4]);
    rhs->script = NULL;
    /* Check for a request for "/". */
    if(appdata[4] == ISO_slash &&
       appdata[5] == 0) {
      fileopen(file_index_html.name, rhs);
    } else if(appdata[4] == ISO_slash &&
	      appdata[5] == ISO_8 &&
	      appdata[6] == ISO_slash) {
      /* This is a request for a file system file. First check for
	 special request for disk directory. */      
      if(appdata[7] == 0) {
	fileopen(file_dirheader_html.name, rhs);
	rhs->state = HTTP_DIRHDR;
	c64_fs_opendir(&rhs->f.d);
      } else {
	if(c64_fs_open(&appdata[7], &rhs->f.f) == -1) {
	  fileopen(file_404_html.name, rhs);
	} else {
	  fileopen(file_binaryheader_plain.name, rhs);
	  rhs->state = HTTP_FSFILEHDR;
	}
      }
      
    } else {
      if(!fileopen((const char *)&appdata[4], rhs)) {
	PRINTLN("couldn't open file");
	fileopen(file_404_html.name, rhs);
      } 
    
      if(strncmp(&appdata[4], cgistring, 5) == 0) {
	/* If the request is for a file that starts with "/cgi/", we
	   prepare for invoking a script. */	
	rhs->script = fsfile.data;
	next_scriptstate();
      }     
    }
  }
    
  if(rhs->state != HTTP_FUNC) {
    /* Check if the client (remote end) has acknowledged any data that
       we've previously sent. If so, we move the file pointer further
       into the file and send back more data. If we are out of data to
       send, we close the connection. */
    if(uip_acked()) {
      rhs->poll = 0;	
      if(rhs->state == HTTP_DIR) {
	if(c64_fs_readdir_next(&rhs->f.d) != 0) {
	  /*	  uip_close();
		  dealloc_state(rhs);*/
	  fileopen(file_dirfooter_plain.name, rhs);
	  rhs->state = HTTP_DIRFOOTER;
	}
      } else if(rhs->state == HTTP_FSFILE) {
	if(c64_fs_read_next(&rhs->f.f, uip_mss()) == 0) {
	  uip_close();
	  dealloc_state(rhs);
	}
      } else {
	if(rhs->count >= uip_mss()) {
	  rhs->count -= uip_mss();
	  rhs->dataptr += uip_mss();
	} else {
	  rhs->count = 0;
	}	
	if(rhs->count == 0) {
	  if(rhs->script != NULL) {
	    next_scriptline();
	    next_scriptstate();
	  } else if(rhs->state == HTTP_DIRHDR) {
	    rhs->state = HTTP_DIR;
	  } else if(rhs->state == HTTP_FSFILEHDR) {
	    rhs->state = HTTP_FSFILE;
	  } else {
	    uip_close();
	    dealloc_state(rhs);
	  }
	}
      }
    }         
  }
    
  if(rhs->state == HTTP_FUNC) {
    /* Call the CGI function. */
    if(httpd_cgitab[rhs->script[2] - ISO_a]()) {
      /* If the function returns non-zero, we jump to the next line
	 in the script. */
      next_scriptline();
      next_scriptstate();
    }
  }

  if(!uip_poll()) {
    if(rhs->state == HTTP_DIR) {
      struct c64_fs_dirent de;
      c64_fs_readdir_dirent(&rhs->f.d, &de);
      uip_send(appdata,
	       sprintf((char *)appdata,
		       "<tr><td><a href=\"/8/%s\">%s</a></td><td>%d</td></tr>\r\n",
		       de.name, de.name, de.size));
    } else if(rhs->state == HTTP_FSFILE) {
      int len;
      len = c64_fs_read_raw(&rhs->f.f, appdata,
			    uip_mss());
      if(len <= 0) {
	uip_close();
	dealloc_state(rhs);
      } else {
	uip_send(appdata, len);
      }
    } else if(rhs->state != HTTP_FUNC) {
      rhs->poll = 0;
      /* Send a piece of data, but not more than the MSS of the
	 connection. */
      uip_send(rhs->dataptr,
	       rhs->count > uip_mss()? uip_mss(): rhs->count);
    }
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
  /* Loop until we find a newline character. */
  do {
    ++(hs->script);
  } while(hs->script[0] != ISO_nl);

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

 again:
  switch(hs->script[0]) {
  case ISO_t:
    /* Send a text string. */
    hs->state = HTTP_TEXT;
    hs->dataptr = &hs->script[2];

    /* Calculate length of string. */
    for(i = 0; hs->dataptr[i] != ISO_nl; ++i);
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
    if(!httpd_fs_open(&hs->script[2], &fsfile)) {
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
