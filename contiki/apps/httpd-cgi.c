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
 * $Id: httpd-cgi.c,v 1.4 2004/06/06 05:59:21 adamdunkels Exp $
 *
 */

/*
 * This file includes functions that are called by the web server
 * scripts. The functions takes no argument, and the return value is
 * interpreted as follows. A zero means that the function did not
 * complete and should be invoked for the next packet as well. A
 * non-zero value indicates that the function has completed and that
 * the web server should move along to the next script line.
 *
 */

#include "uip.h"
#include "httpd.h"
#include "httpd-cgi.h"
#include "httpd-fs.h"

#include "petsciiconv.h"

#ifdef __CBM__
#include <cbm.h>
#include <c64.h>
#endif /* __CBM__ */

#include <stdio.h>
#include <string.h>

static u8_t file_stats(void);
static u8_t tcp_stats(void);
static u8_t processes(void);

static u8_t d64output(void);

httpd_cgifunction httpd_cgitab[] = {
  file_stats,    /* CGI function "a" */
  tcp_stats,     /* CGI function "b" */
  processes,     /* CGI function "c" */

  d64output,     /* CGI function "d" */
};

static const char closed[] =   /*  "CLOSED",*/
{0x43, 0x4c, 0x4f, 0x53, 0x45, 0x44, 0};
static const char syn_rcvd[] = /*  "SYN-RCVD",*/
{0x53, 0x59, 0x4e, 0x2d, 0x52, 0x43, 0x56, 
 0x44,  0};
static const char syn_sent[] = /*  "SYN-SENT",*/
{0x53, 0x59, 0x4e, 0x2d, 0x53, 0x45, 0x4e, 
 0x54,  0};
static const char established[] = /*  "ESTABLISHED",*/
{0x45, 0x53, 0x54, 0x41, 0x42, 0x4c, 0x49, 0x53, 0x48, 
 0x45, 0x44, 0};
static const char fin_wait_1[] = /*  "FIN-WAIT-1",*/
{0x46, 0x49, 0x4e, 0x2d, 0x57, 0x41, 0x49, 
 0x54, 0x2d, 0x31, 0};
static const char fin_wait_2[] = /*  "FIN-WAIT-2",*/
{0x46, 0x49, 0x4e, 0x2d, 0x57, 0x41, 0x49, 
 0x54, 0x2d, 0x32, 0};
static const char closing[] = /*  "CLOSING",*/
{0x43, 0x4c, 0x4f, 0x53, 0x49, 
 0x4e, 0x47, 0};
static const char time_wait[] = /*  "TIME-WAIT,"*/
{0x54, 0x49, 0x4d, 0x45, 0x2d, 0x57, 0x41, 
 0x49, 0x54, 0};
static const char last_ack[] = /*  "LAST-ACK"*/
{0x4c, 0x41, 0x53, 0x54, 0x2d, 0x41, 0x43, 
 0x4b, 0};

static const char *states[] = {
  closed,
  syn_rcvd,
  syn_sent,
  established,
  fin_wait_1,
  fin_wait_2,
  closing,
  time_wait,
  last_ack};
  

/*-----------------------------------------------------------------------------------*/
static u8_t
file_stats(void)
{
  /* We use sprintf() to print the number of file accesses to a
     particular file (given as an argument to the function in the
     script). We then use uip_send() to actually send the data. */
  if(uip_acked()) {
    return 1;
  }
  uip_send(uip_appdata, sprintf((char *)uip_appdata, "%5u", httpd_fs_count(&hs->script[4])));
  return 0;
}
/*-----------------------------------------------------------------------------------*/
static u8_t
tcp_stats(void)
{
  struct uip_conn *conn;  

  if(uip_acked()) {
    /* If the previously sent data has been acknowledged, we move
       forward one connection. */
    if(++hs->count == UIP_CONNS) {
      /* If all connections has been printed out, we are done and
	 return 1. */
      return 1;
    }
  }

  conn = &uip_conns[hs->count];
  while((conn->tcpstateflags & TS_MASK) == CLOSED) {
    if(++hs->count == UIP_CONNS) {
      /* If all connections has been printed out, we are done and
	 return 1. */
      return 1;
    }
    conn = &uip_conns[hs->count];
  }

  uip_send(uip_appdata, sprintf((char *)uip_appdata,
				"<tr><td>%d</td><td>%u.%u.%u.%u:%u</td><td>%s</td><td>%u</td><td>%u</td><td>%c %c</td></tr>\r\n",
				htons(conn->lport),
				htons(conn->ripaddr[0]) >> 8,
				htons(conn->ripaddr[0]) & 0xff,
				htons(conn->ripaddr[1]) >> 8,
				htons(conn->ripaddr[1]) & 0xff,
				htons(conn->rport),
				states[conn->tcpstateflags & TS_MASK],
				conn->nrtx,
				conn->timer,
				(uip_outstanding(conn))? '*':' ',
				(uip_stopped(conn))? '!':' '));

  return 0;
}
/*-----------------------------------------------------------------------------------*/
static u8_t
processes(void)
{
  u8_t i;
  struct dispatcher_proc *p;
  char name[40];

  p = DISPATCHER_PROCS();
  for(i = 0; i < hs->count; ++i) {
    if(p != NULL) {
      p = p->next;
    }
  }

  if(uip_acked()) {
    /* If the previously sent data has been acknowledged, we move
       forward one connection. */
    ++hs->count;
    if(p != NULL) {
      p = p->next;
    }
    if(p == NULL) {
      /* If all processes have been printed out, we are done and
	 return 1. */
      return 1;
    }
  }

  strncpy(name, p->name, 40);
  petsciiconv_toascii(name, 40);
  uip_send(uip_appdata,
	   sprintf((char *)uip_appdata,
		   "<tr align=\"center\"><td>%3d</td><td>%s</td><td>0x%04x</td><td>0x%04x</td><td>0x%04x</td></tr>\r\n",
		   p->id, name,
		   p->idle, p->signalhandler, p->uiphandler));
  return 0;
}
/*-----------------------------------------------------------------------------------*/
#ifdef __CBM__
struct drv_state {
  u8_t track;
  u8_t sect;
};

static struct drv_state ds;


#include "c64-dio.h"

static void
read_sector(void)
{
  c64_dio_read_block(ds.track, ds.sect, uip_appdata);
}
#if 0
static void
x_open(u8_t f, u8_t d, u8_t cmd, u8_t *fname)
{
  u8_t ret;
  
  ret = cbm_open(f, d, cmd, fname);
  if(ret != 0) {
    /*    show_statustext("Open error");*/
  }
  

}


static u8_t cmd[32];
static void
read_sector(void)
{  
  int ret;
  
  x_open(15, 8, 15, NULL);
  x_open(2, 8, 2, "#");

  sprintf(cmd, "u1: 2 0 %d %d", ds.track, ds.sect);  
  cbm_write(15, cmd, strlen(cmd));
    
  ret = cbm_read(2, uip_appdata, 256);
  if(ret == -1) {
    /*    ctk_label_set_text(&statuslabel, "Read err");
	  CTK_WIDGET_REDRAW(&statuslabel);*/
  }
  cbm_close(2);
  cbm_close(15);
}
#endif /* 0 */
static u8_t
next_sector(void)
{
  ++ds.sect;
  if(ds.track < 18) {
    if(ds.sect == 21) {
      ++ds.track;
      ds.sect = 0;
    }
  } else if(ds.track < 25) {
    if(ds.sect == 19) {
      ++ds.track;
      ds.sect = 0;
    }
  } else if(ds.track < 31) {
    if(ds.sect == 18) {
      ++ds.track;
      ds.sect = 0;
    }
  } else if(ds.track < 36) {
    if(ds.sect == 17) {
      ++ds.track;
      ds.sect = 0;
    }
  }

  if(ds.track == 36) {
    return 1;
  }
  return 0;
}

static u8_t
d64output(void)
{
  if(hs->count == 0) {
    ds.track = 1;
    ds.sect = 0;
    /*    c64_dio_init(8);*/
  }
  
  if(uip_acked()) {
    ++hs->count;
    if(next_sector()) {
      return 1;
    }
  }

  read_sector();
  uip_send(uip_appdata, 256);
  return 0;
}
#else /* __CBM__ */
static u8_t
d64output(void)
{

}
#endif /* __CBM__ */
/*-----------------------------------------------------------------------------------*/
