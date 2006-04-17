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
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: httpd.c,v 1.1 2006/04/17 15:18:18 kthacker Exp $
 *
 */


#include "uip.h"

#include "http-strings.h"

#include "httpd.h"
#include "httpd-fs.h"
#include "httpd-fsdata.h"
#include "httpd-cgi.h"

#include "tcpip.h"

#include <stdio.h>

/* The HTTP server states: */
#define STATE_DEALLOCATED       0
#define STATE_INITIAL           1
#define STATE_GET               2
#define STATE_HEADERS           3
#define STATE_SEND_HEADERS      4
#define STATE_SEND_CONTENT_TYPE 5
#define STATE_SEND_DATA         6

#define ISO_nl                  0x0a
#define ISO_space               0x20
#define ISO_slash               0x2f

#ifdef DEBUG
#include <stdio.h>
#define PRINT(x) printf("%s", x)
#define PRINTLN(x) printf("%s\n", x)
#else /* DEBUG */
#define PRINT(x)
#define PRINTLN(x)
#endif /* DEBUG */


#define HTTPD_CONF_NUMCONNS 4
static struct httpd_state conns[HTTPD_CONF_NUMCONNS];
u8_t i;

struct http_filetype {
  const char *ext;
  unsigned char extlen;
  const char *type;
  unsigned char typelen;
};

static struct http_filetype filetypes[] = {
  {http_html, sizeof(http_html) - 1,
   http_content_type_html, sizeof(http_content_type_html) - 1},
  {http_htm, sizeof(http_htm) - 1,
   http_content_type_html, sizeof(http_content_type_html) - 1},
  {http_css, sizeof(http_css) - 1,
   http_content_type_css, sizeof(http_content_type_css) - 1},
  {http_png, sizeof(http_png) - 1,
   http_content_type_png, sizeof(http_content_type_png) - 1},
  {http_gif, sizeof(http_gif) - 1,
   http_content_type_gif, sizeof(http_content_type_gif) - 1},
  {http_jpg, sizeof(http_jpg) - 1,
   http_content_type_jpg, sizeof(http_content_type_jpg) - 1},
  {http_txt, sizeof(http_txt) - 1,
   http_content_type_text, sizeof(http_content_type_text) - 1},
  {http_text, sizeof(http_text) - 1,
   http_content_type_text, sizeof(http_content_type_text) - 1},  
  {NULL, 0, NULL, 0}
};
#define NUMFILETYPES (sizeof(filetypes) / sizeof(struct http_filetype))
/*-----------------------------------------------------------------------------*/
static struct httpd_state *
alloc_state(void)
{
  
  for(i = 0; i < HTTPD_CONF_NUMCONNS; ++i) {
    if(conns[i].state == STATE_DEALLOCATED) {
      return &conns[i];
    }
  }

  /* We are overloaded! XXX: we'll just kick all other connections! */
  for(i = 0; i < HTTPD_CONF_NUMCONNS; ++i) {
    conns[i].state = STATE_DEALLOCATED;
  }
  
  return NULL;
}
/*-----------------------------------------------------------------------------*/
static void
dealloc_state(struct httpd_state *s)
{
  s->state = STATE_DEALLOCATED;
}
/*-----------------------------------------------------------------------------*/
void
httpd_init(void)
{
  httpd_fs_init();
  
  /* Listen to port 80. */
  tcp_listen(HTONS(80));

  for(i = 0; i < HTTPD_CONF_NUMCONNS; ++i) {
    conns[i].state = STATE_DEALLOCATED;
  }
}
/*-----------------------------------------------------------------------------*/
/**
 * \internal
 * Buffer an amount of data from the incoming uIP data.
 *
 * \param buf A pointer to the buffer.
 * \param len The length of data to buffer.
 *
 * \return Zero if the function wasn't able to buffer to buffer all
 * data, non-zero otherwise.
 */
/*-----------------------------------------------------------------------------*/
static unsigned char
buffer(CC_REGISTER_ARG struct httpd_buffer *buf, int len, u8_t c)
{
  while(buf->ptr < len &&
	uip_len > 0) {
    buf->buf[buf->ptr] = *uip_appdata;
    ++uip_appdata;
    --uip_len;

    if(buf->buf[buf->ptr] == c) {
      return 1;
    }
    ++buf->ptr;
  }
  return (buf->ptr == len);
}
/*-----------------------------------------------------------------------------*/
static void
buffer_reset(struct httpd_buffer *buf)
{
  buf->ptr = 0;
}
/*-----------------------------------------------------------------------------*/
static void
newdata(CC_REGISTER_ARG struct httpd_state *hs)
{
  int i;
  struct http_filetype *filetype;
    
  while(uip_len > 0) {
    switch(hs->state) {
      
    case STATE_INITIAL:
      /* This is the first data that we receive, so we check if we can
	 identify the "GET" request. We call the buffer() function to
	 try to buffer 4 bytes of data from the incoming data stream. If
	 the function isn't able to buffer enough data, we return and
	 will try again next time around. */
      hs->buf.buf = hs->getbuffer;
      if(buffer(&(hs->buf), 4, ISO_space)) {
	if(strncmp(http_get, hs->buf.buf, 4) == 0) {
	  hs->state = STATE_GET;
	  buffer_reset(&(hs->buf));
	} else {
	  uip_abort();
	  return;
	}
      }
      break;
    
    case STATE_GET:
      /* We will read one byte at a time from the GET request until we
	 find a en end of the line (\n\r). The GET request is remembered
	 in the httpd state since we might need to look at it later. */
      hs->buf.buf = hs->getbuffer;
      if(buffer(&(hs->buf), sizeof(hs->getbuffer), ISO_nl)) {
	hs->state = STATE_HEADERS;
	hs->getlen = hs->buf.ptr - 1;
	/* Null-terminate GET request string. */
	hs->getbuffer[hs->getlen] = 0;

	/* If there is a space character in the get request, we find
	   it and null terminate the string there. */
	for(i = 0; i < hs->getlen; ++i) {
	  if(hs->getbuffer[i] == ISO_space) {
	    hs->getbuffer[i] = 0;
	    hs->getlen = i;
	    break;
	  }
	}
	buffer_reset(&(hs->buf));	
      }
      break;

    case STATE_HEADERS:
      hs->buf.buf = hs->hdrbuffer;
      if(buffer(&(hs->buf), sizeof(hs->hdrbuffer), '\n')) {
	if(hs->buf.ptr == 1) {
	  hs->state = STATE_SEND_HEADERS;
	  if(httpd_fs_open(hs->getbuffer, &hs->file)) {
	    hs->count = sizeof(http_header_200) - 1;
	    hs->dataptr = (char *)http_header_200;
	    hs->contenttype = http_content_type_binary;
	    hs->contentlen = sizeof(http_content_type_binary) - 1;
	    for(filetype = filetypes;
		filetype->ext != NULL;
		++filetype) {
	      if(strncmp(filetype->ext,
			 hs->getbuffer + (hs->getlen - filetype->extlen),
			 filetype->extlen) == 0) {
		hs->contenttype = filetype->type;
		hs->contentlen = filetype->typelen;
		break;
	      }
	    }

	  } else if(hs->getbuffer[0] == ISO_slash &&
	      hs->getbuffer[1] == 0) {
	    hs->count = sizeof(http_header_200) - 1;
	    hs->dataptr = (char *)http_header_200;
	    hs->contenttype = http_content_type_html;
	    hs->contentlen = sizeof(http_content_type_html) - 1;
	    httpd_fs_open(http_index_html, &hs->file);
	  } else {
	    hs->count = sizeof(http_header_404) - 1;
	    hs->dataptr = (char *)http_header_404;
	    hs->contenttype = http_content_type_html;
	    hs->contentlen = sizeof(http_content_type_html) - 1;
	    httpd_fs_open(http_404_html, &hs->file);
	  }
	} else {
	  buffer_reset(&(hs->buf));
	}
      }
      break;

    default:
      uip_len = 0;
      break;
    }
  }
}
/*-----------------------------------------------------------------------------*/
static void
acked(CC_REGISTER_ARG struct httpd_state *hs)
{
  hs->poll = 0;

  
  switch(hs->state) {

  case STATE_SEND_HEADERS:
    if(hs->count >= uip_mss()) {
      hs->count -= uip_mss();
      hs->dataptr += uip_mss();
    } else {
      hs->count = 0;
    }	
    if(hs->count == 0) {
      hs->state = STATE_SEND_CONTENT_TYPE;
      hs->count = hs->contentlen;
      hs->dataptr = (char *)hs->contenttype;
    }
    break;
    
  case STATE_SEND_CONTENT_TYPE:
    if(hs->count >= uip_mss()) {
      hs->count -= uip_mss();
      hs->dataptr += uip_mss();
    } else {
      hs->count = 0;
    }	
    if(hs->count == 0) {
      hs->state = STATE_SEND_DATA;
      hs->count = hs->file.len;
      hs->dataptr = hs->file.data;
    }
    break;
    
  case STATE_SEND_DATA:
    if(hs->count >= uip_mss()) {
      hs->count -= uip_mss();
      hs->dataptr += uip_mss();
    } else {
      hs->count = 0;
    }	
    if(hs->count == 0) {
      uip_close();
      dealloc_state(hs);
    }
    break;
  }
}
/*-----------------------------------------------------------------------------*/
static void
senddata(CC_REGISTER_ARG struct httpd_state *hs)
{
  if(hs->state == STATE_SEND_HEADERS ||
     hs->state == STATE_SEND_CONTENT_TYPE ||
     hs->state == STATE_SEND_DATA) {
    uip_send(hs->dataptr,
	     hs->count > uip_mss()? uip_mss(): hs->count);
  }
}
/*-----------------------------------------------------------------------------*/
void
httpd_appcall(void *state)
{
  register struct httpd_state *hs;

  hs = (struct httpd_state *)(state);


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
      tcp_markconn(uip_conn, (void *)hs);
    }
    /* Since we have just been connected with the remote host, we
       reset the state for this connection. The ->count variable
       contains the amount of data that is yet to be sent to the
       remote host, and the ->state is set to HTTP_NOGET to signal
       that we haven't received any HTTP GET request for this
       connection yet. */
    hs->state = STATE_INITIAL;
    hs->count = 0;
    hs->poll = 0;
    buffer_reset(&(hs->buf));
    return;
  }

  if(uip_closed() ||
     uip_aborted() ||
     uip_timedout()) {
    if(hs != NULL) {
      dealloc_state(hs);
    }
    return;
  } else if(uip_poll()) {
    /* If we are polled ten times, we abort the connection. This is
       because we don't want connections lingering indefinately in
       the system. */
    if(hs != NULL) {
      if(hs->state == STATE_DEALLOCATED) {
	uip_abort();
      } else if(hs->poll++ >= 100) {
	uip_abort();
	dealloc_state(hs);
      }
    }
    return;
  }
  
  if(uip_acked()) {
    acked(hs);
  }
  
  if(uip_newdata()) {
    newdata(hs);
  }
  
  if(uip_rexmit() ||
     uip_newdata() ||
     uip_acked()) {
    senddata(hs);
  } else if(uip_poll()) {    
    senddata(hs);
  }


}
/*-----------------------------------------------------------------------------*/
