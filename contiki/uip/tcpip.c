/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the Contiki operating system.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: tcpip.c,v 1.6 2005/02/27 09:44:33 adamdunkels Exp $
 */
#include "tcpip.h"

#include "ek-service.h"

#include "tcpip.h"

#include "uip.h"
#include "uip-fw.h"

#include "timer.h"

#include "packet-service.h"

#include "uip-split.h"

#include <string.h>

ek_event_t tcpip_event;

EK_SERVICE(packetservice, PACKET_SERVICE_NAME);

/**
 * \internal Structure for holding a TCP port and a process ID.
 */
struct listenport {
  u16_t port;
  ek_id_t id;
};

/*static struct tcpip_event_args ev_args;*/

static struct timer periodic;

static struct internal_state {
  struct listenport listenports[UIP_LISTENPORTS];
  ek_event_t event;
  ek_id_t id;
} s;

enum {
  TCP_POLL,
  UDP_POLL
};

static unsigned char forwarding = 0;

EK_EVENTHANDLER(eventhandler, ev, data);
EK_POLLHANDLER(pollhandler);
EK_PROCESS(proc, "TCP/IP stack", EK_PRIO_NORMAL,
	   eventhandler, pollhandler, NULL);
/*---------------------------------------------------------------------------*/
EK_PROCESS_INIT(tcpip_init, arg)
{
  uip_init();
  ek_start(&proc);
  forwarding = 0;
}
/*---------------------------------------------------------------------------*/
void
tcpip_set_forwarding(unsigned char f)
{
  forwarding = f;
}
/*---------------------------------------------------------------------------*/
void
tcpip_input(void)
{
  if(uip_len > 0) {
    if(forwarding) {
      if(uip_fw_forward() == 0) {
	uip_input();
	if(uip_len > 0) {
#if UIP_CONF_TCP_SPLIT
	  uip_split_output();
#else
	  tcpip_output();
#endif
	}
      }
    } else {
      uip_input();
      if(uip_len > 0) {
#if UIP_CONF_TCP_SPLIT
	uip_split_output();
#else
	tcpip_output();
#endif
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void
tcpip_output(void)
{
  struct packet_service_state *state;
  u16_t hdrlen, datalen;  
  
  state = (struct packet_service_state *)ek_service_state(&packetservice);

  if(state != NULL &&
     state->version == PACKET_SERVICE_VERSION) {

    hdrlen = UIP_TCPIP_HLEN;
    if(uip_len < UIP_TCPIP_HLEN) {
      hdrlen = uip_len;
      datalen = 0;
    } else {
      datalen = uip_len - UIP_TCPIP_HLEN;
    }
    
    state->output(&uip_buf[UIP_LLH_LEN], hdrlen, uip_appdata, datalen);
  }
}
/*---------------------------------------------------------------------------*/
struct uip_conn *
tcp_connect(u16_t *ripaddr, u16_t port, void *appstate)
{
  struct uip_conn *c;
  
  c = uip_connect(ripaddr, port);
  if(c == NULL) {
    return NULL;
  }

  ((struct tcpip_uipstate *)(c->appstate))->id = EK_PROC_ID(EK_CURRENT());
  ((struct tcpip_uipstate *)(c->appstate))->state = appstate;

  ek_post(s.id, TCP_POLL, c);
  
  return c;
}
/*---------------------------------------------------------------------------*/
void
tcp_unlisten(u16_t port)
{
  static unsigned char i;
  struct listenport *l;

  l = s.listenports;
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    if(l->port == port &&
       l->id == EK_PROC_ID(EK_CURRENT())) {
      l->port = 0;      
      uip_unlisten(port);
      break;
    }
    ++l;
  }
}
/*---------------------------------------------------------------------------*/
void
tcp_listen(u16_t port)
{
  static unsigned char i;
  struct listenport *l;

  l = s.listenports;
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    if(l->port == 0) {
      l->port = port;
      l->id = EK_PROC_ID(EK_CURRENT());
      uip_listen(port);
      break;
    }
    ++l;
  }
}
/*---------------------------------------------------------------------------*/
void
tcp_markconn(struct uip_conn *conn,
             void *appstate)
{
  register struct tcpip_uipstate *s;

  s = (struct tcpip_uipstate *)conn->appstate;
  s->id = ek_current->id;
  s->state = appstate;
}
/*---------------------------------------------------------------------------*/
struct uip_udp_conn *
udp_new(u16_t *ripaddr, u16_t port, void *appstate)
{
  struct uip_udp_conn *c;
  struct tcpip_uipstate *s;
  
  c = uip_udp_new(ripaddr, port);
  if(c == NULL) {
    return NULL;
  }

  s = (struct tcpip_uipstate *)c->appstate;
  s->id = EK_PROC_ID(EK_CURRENT());
  s->state = appstate;

  return c;
}
/*---------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{
  static unsigned char i;
  register struct listenport *l;
  ek_id_t id;
  struct internal_state *state;
  
  switch(ev) {
  case EK_EVENT_INIT:       
    for(i = 0; i < UIP_LISTENPORTS; ++i) {
      s.listenports[i].port = 0;
    }
    s.id = EK_PROC_ID(EK_CURRENT());
    tcpip_event = s.event = ek_alloc_event();
    timer_set(&periodic, CLOCK_SECOND/2);  
    break;
  case EK_EVENT_REPLACE:
    memcpy(&s, data, sizeof(s));
    arg_free(data);
    break;
  case EK_EVENT_REQUEST_REPLACE:
    state = (struct internal_state *)arg_alloc(sizeof(s));
    /* Copy state */
    memcpy(state, &s, sizeof(s)); 
    ek_replace((struct ek_proc *)data, state);
    break;
    
  case EK_EVENT_EXITED:
    id = (ek_id_t)data;
    l = s.listenports;
    for(i = 0; i < UIP_LISTENPORTS; ++i) {
      if(l->id == id) {
	uip_unlisten(l->port);
	l->port = 0;
	l->id = EK_ID_NONE;
      }
      ++l;
    }
    /*    for(i = 0; i < UIP_CONNS; ++i) {
      if(((struct tcpip_uipstate *)uip_conns[i].appstate)->id == id) {
      ((struct tcpip_uipstate *)uip_conns[i].appstate)->id = EK_ID_NONE;
	uip_conns[i].tcpstateflags = CLOSED;
      }
      }*/
    {
      register struct uip_conn *cptr;
     
      for(cptr = &uip_conns[0]; cptr < &uip_conns[UIP_CONNS]; ++cptr) {
	if(((struct tcpip_uipstate *)cptr->appstate)->id == id) {
	  ((struct tcpip_uipstate *)cptr->appstate)->id = EK_ID_NONE;
	  cptr->tcpstateflags = CLOSED;
	}
      
      }
      
    }
#if UIP_UDP
    {
      register struct uip_udp_conn *cptr;
      for(cptr = &uip_udp_conns[0];
	  cptr < &uip_udp_conns[UIP_UDP_CONNS]; ++cptr) {
	if(((struct tcpip_uipstate *)cptr->appstate)->id == id) {
	  cptr->lport = 0;
	}
      }
      
    }
    /*    for(i = 0; i < UIP_UDP_CONNS; ++i) {
      if(((struct tcpip_uipstate *)uip_udp_conns[i].appstate)->id == id) {
	uip_udp_conns[i].lport = 0;
      }
      }*/
#endif /* UIP_UDP */
    break;
  case TCP_POLL:
    if(data != NULL) {
      uip_poll_conn(data);
      if(uip_len > 0) {
	tcpip_output();
      }
    }
    break;
  case UDP_POLL:
    if(data != NULL) {
      uip_udp_periodic_conn(data);
      if(uip_len > 0) {
	tcpip_output();
      }
    }
    break;
  };
}
/*---------------------------------------------------------------------------*/
void
tcpip_poll_udp(struct uip_udp_conn *conn)
{
  ek_post(s.id, UDP_POLL, conn);
}
/*---------------------------------------------------------------------------*/
void
tcpip_poll_tcp(struct uip_conn *conn)
{
  ek_post(s.id, TCP_POLL, conn);
}
/*---------------------------------------------------------------------------*/
void
tcpip_uipcall(void)     
{
  register struct tcpip_uipstate *ts;
  static unsigned char i;
  register struct listenport *l;

  if(uip_conn != NULL) {
    ts = (struct tcpip_uipstate *)uip_conn->appstate;
  } else {
    ts = (struct tcpip_uipstate *)uip_udp_conn->appstate;
  }

  /* If this is a connection request for a listening port, we must
     mark the connection with the right process ID. */
  if(uip_connected()) {
    l = &s.listenports[0];
    for(i = 0; i < UIP_LISTENPORTS; ++i) {
      if(l->port == uip_conn->lport &&
	 l->id != EK_ID_NONE) {
	ts->id = l->id;
	ts->state = NULL;
	break;
      }
      ++l;
    }
  }

  ek_post_synch(ts->id, s.event, ts->state);
}
/*---------------------------------------------------------------------------*/
EK_POLLHANDLER(pollhandler)
{
  static unsigned char i;
  
  /* Check the clock so see if we should call the periodic uIP
     processing. */
  if(timer_expired(&periodic)) {
    timer_restart(&periodic);
    for(i = 0; i < UIP_CONNS; ++i) {
      uip_periodic(i);
      if(uip_len > 0) {
	tcpip_output();
	/*	uip_fw_output();*/
      }
    }
    
    for(i = 0; i < UIP_UDP_CONNS; i++) {
      uip_udp_periodic(i);
      if(uip_len > 0) {
	tcpip_output();
	/*	uip_fw_output();*/
      }
    }
    uip_fw_periodic();
  }
}
/*---------------------------------------------------------------------------*/
