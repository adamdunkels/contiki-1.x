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
 * Contiki also includes an optional protosocket library that provides
 * an API similar to the BSD socket API.
 *
 * \sa \ref uip "The uIP TCP/IP stack"
 * \sa \ref psock "Protosockets library"
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
 * $Id: tcpip.h,v 1.6 2005/02/22 22:34:46 adamdunkels Exp $
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

/**
 * Open a TCP connection to the specified IP address and port.
 *
 * This function opens a TCP connection to the specified port at the
 * host specified with an IP address. Additionally, an opaque pointer
 * can be attached to the connection. This pointer will be sent
 * together with uIP events to the process.
 *
 * \note The port number must be provided in network byte order so a
 * conversion with HTONS() usually is necessary.
 *
 * \note This function will only create the connection. The connection
 * is not opened directly. uIP will try to open the connection the
 * next time the uIP stack is scheduled by Contiki.
 *
 * Example:
 \code
 static struct uip_conn *conn;
 
 EK_EVENTHANDLER(eventhandler, ev, data)
 {
   u16_t addr[2];
   
   if(ev == EK_EVENT_INIT) {
     uip_ipaddr(addr, 192,168,1,1);
     conn = tcp_connect(addr, HTONS(80), NULL);
   }
 }
 \endcode
 *
 * \param ripaddr Pointer to the IP address of the remote host.
 * \param port Port number in network byte order.
 * \param appstate Pointer to application defined data.
 *
 * \return A pointer to the newly created connection, or NULL if
 * memory could not be allocated for the connection.
 *
 */
struct uip_conn *tcp_connect(u16_t *ripaddr, u16_t port,
			     void *appstate);

/**
 * Create a new UDP connection.
 *
 * This function creates a new UDP connection with the specified
 * remote endpoint.
 *
 * \note The port number must be provided in network byte order so a
 * conversion with HTONS() usually is necessary.
 *
 * \sa udp_bind()
 *
 * \param ripaddr Pointer to the IP address of the remote host.
 * \param port Port number in network byte order.
 * \param appstate Pointer to application defined data.
 *
 * \return A pointer to the newly created connection, or NULL if
 * memory could not be allocated for the connection.
 */
struct uip_udp_conn *udp_new(u16_t *ripaddr, u16_t port,
			     void *appstate);

/**
 * Bind a UDP connection to a local port.
 *
 * This function binds a UDP conncetion to a specified local port.
 *
 * When a connction is created with udp_new(), it gets a local port number assigned automatically. If the application needs to bind the connection to a specified local port, this function should be used.
 *
 * \note The port number must be provided in network byte order so a
 * conversion with HTONS() usually is necessary.
 *
 * Example
 \code
  EK_EVENTHANDLER(eventhandler, ev, data)
  {
    u16_t addr[2];
  
    if(ev == EK_EVENT_INIT) {
      uip_ipaddr(addr, 255,255,255,255);
      conn = udp_new(addr, HTONS(PORT), NULL);
      if(conn != NULL) {
        udp_bind(conn, HTONS(PORT));
      }
    }
  }
 \endcode
 *
 * \param conn A pointer to the UDP connection that is to be bound.
 * \param port The port number in network byte order to which to bind
 * the connection.
 */
#define udp_bind(conn, port) uip_udp_bind(conn, port)

/**
 * The uIP event.
 *
 * This event is posted to a process whenever a uIP event has occured.
 */
extern ek_event_t tcpip_event;



void tcpip_set_forwarding(unsigned char f);
void tcpip_input(void);
void tcpip_output(void);


/**
 * Cause a specified TCP connection to be polled.
 *
 * This function causes uIP to poll the specified TCP connection. The
 * function is used when the application has data that is to be sent
 * immediately and do not wish to wait for the periodic uIP polling
 * mechanism.
 *
 * \param conn A pointer to the TCP connection that should be polled.
 *
 */
void tcpip_poll_tcp(struct uip_conn *conn);

/**
 * Cause a specified UDP connection to be polled.
 *
 * This function causes uIP to poll the specified UDP connection. The
 * function is used when the application has data that is to be sent
 * immediately and do not wish to wait for the periodic uIP polling
 * mechanism.
 *
 * \param conn A pointer to the UDP connection that should be polled.
 *
 */
void tcpip_poll_udp(struct uip_udp_conn *conn);

#endif /* __TCPIP_H__ */
