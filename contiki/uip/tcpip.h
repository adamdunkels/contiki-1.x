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
 * $Id: tcpip.h,v 1.4 2004/09/17 20:49:49 adamdunkels Exp $
 */
#ifndef __TCPIP_H__
#define __TCPIP_H__

#include "ek.h"




struct uip_conn;

struct tcpip_uipstate {
  ek_id_t id;  
  void *state;
};

#define UIP_APPCALL tcpip_uipcall
#define UIP_UDP_APPCALL tcpip_uipcall
#define UIP_APPSTATE_SIZE sizeof(struct tcpip_uipstate)

#include "uip.h"

EK_PROCESS_INIT(tcpip_init, arg);
     
void tcpip_uipcall(void);

void tcp_markconn(struct uip_conn *conn,
		  void *appstate);

void tcp_listen(u16_t port);

void tcp_unlisten(u16_t port);

struct uip_conn *tcp_connect(u16_t *ripaddr, u16_t port,
			     void *appstate);

struct uip_udp_conn *udp_new(u16_t *ripaddr, u16_t port,
			     void *appstate);
extern ek_event_t tcpip_event;

#define udp_bind(conn, port) uip_udp_bind(conn, port)


void tcpip_set_forwarding(unsigned char f);
void tcpip_input(void);
void tcpip_output(void);


void tcpip_poll_tcp(struct uip_conn *conn);
void tcpip_poll_udp(struct uip_udp_conn *conn);

#endif /* __TCPIP_H__ */
