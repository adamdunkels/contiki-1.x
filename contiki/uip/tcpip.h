/**
 * \defgroup tcpip The Contiki/uIP interface
 * @{
 *
 * TCP/IP support in Contiki is implemented using the uIP TCP/IP
 * stack. For sending and receiving data, Contiki uses the functions
 * provided by the uIP module, but Contiki adds a set of functions for
 * connection management. The connection management functions make
 * sure that the uIP TCP/IP connections are connected to the correct
 * process.
 *
 * Contiki also includes an optional socket-like library for TCP
 * connections.
 *
 * \sa \ref uip "The uIP TCP/IP stack"
 * \sa \ref socket "Socket-like library"
 *
 */

/**
 * \file
 * Header for the Contiki/uIP interface.
 * \author
 * Adam Dunkels <adam@sics.se>
 */
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
 * $Id: tcpip.h,v 1.5 2005/02/07 07:07:24 adamdunkels Exp $
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

/**
 * Mark a TCP connection with the current process.
 *
 * This function ties a TCP connection with the current process. Each
 * TCP connection must be tied to a process in order for the process
 * to be able to receive and send data. Additionally, this function
 * can add a pointer with connection state to the connection.
 *
 * \param conn A pointer to the TCP connection.
 *
 * \param appstate An opaque pointer that will be passed to the
 * process whenever an event occurs on the connection.
 *
 */
void tcp_markconn(struct uip_conn *conn,
		  void *appstate);

/**
 * Open a TCP port.
 *
 * This function opens a TCP port for listening. When a TCP connection
 * request occurs for the port, the process will be sent a tcpip_event
 * with the new connection request.
 *
 * \note Port numbers must always be given in network byte order. The
 * functions HTONS() and htons() can be used to convert port numbers
 * from host byte order to network byte order.
 *
 * Example
 \code
 static void init_ports(void) {
   tcp_listen(HTONS(80));
   tcp_listen(HTONS(81));
 }
 \endcode
 *
 * \param port The port number in network byte order. 
 *
 */
void tcp_listen(u16_t port);

/**
 * Close a listening TCP port.
 *
 * This function closes a listening TCP port.
 *
 * \note Port numbers must always be given in network byte order. The
 * functions HTONS() and htons() can be used to convert port numbers
 * from host byte order to network byte order.
 *
 * Example
 \code
 static void close_ports(void) {
   tcp_unlisten(HTONS(80));
   tcp_unlisten(HTONS(81));
 }
 \endcode
 *
 * \param port The port number in network byte order. 
 *
 */
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
