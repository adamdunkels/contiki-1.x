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
 * $Id: httpd-cgi.c,v 1.1 2003/09/04 19:46:33 adamdunkels Exp $
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

#include "sensors.h"

static u8_t sensors(void);
static u8_t tcp_stats(void);
static u8_t processes(void);

const httpd_cgifunction httpd_cgitab[3] = {
  sensors,    /* CGI function "a" */
  tcp_stats,     /* CGI function "b" */
  processes,     /* CGI function "c" */
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

static const char * const states[] = {
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
sensors(void)
{
  if(uip_acked()) {
    return 1;
  }
  sprintf((char *)uip_appdata,
	  "<li>Microphone 0x%x<br><li>Battery 0x%x<br><li>Temp %d<br><li>Extern 0x%x<br>",
	  sensors_mic, sensors_battery, sensors_temp, sensors_extern);
  uip_send(uip_appdata, strlen((char *)uip_appdata));
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

  sprintf((char *)uip_appdata,
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
	  (uip_stopped(conn))? '!':' ');
  uip_send(uip_appdata, strlen((char *)uip_appdata));

  return 0;
}
/*-----------------------------------------------------------------------------------*/
static u8_t
processes(void)
{
  u8_t i;
  struct dispatcher_proc *p;


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

  sprintf((char *)uip_appdata,
	  "<tr align=\"center\"><td>%3d</td><td>%s</td><td>0x%04x</td><td>0x%04x</td><td>0x%04x</td></tr>\r\n",
	  p->id, p->name,
	  p->idle, p->signalhandler, p->uiphandler);
  uip_send(uip_appdata, strlen((char *)uip_appdata));
  return 0;
}
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
