/*
 * Copyright (c) 2003, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution. 
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *        This product includes software developed by Adam Dunkels. 
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
 * This file is part of the Contiki Destop OS
 *
 * $Id: uip-conf.h,v 1.4 2006/05/17 15:55:29 oliverschmidt Exp $
 *
 */
#ifndef __UIP_CONF_H__

#define UIP_CONF_MAX_CONNECTIONS 10
#define UIP_CONF_MAX_LISTENPORTS 10
#define UIP_CONF_BUFFER_SIZE     1024 - 2
#define UIP_CONF_RECEIVE_WINDOW  UIP_TCP_MSS
#define UIP_CONF_BYTE_ORDER      LITTLE_ENDIAN
#define UIP_CONF_EXTERNAL_BUFFER

#ifdef UIP_CODE

#pragma codeseg("UIP");

#include "ek-conf.h"

#undef  EK_PROCESS_INIT
#define EK_PROCESS_INIT(name, arg)					\
  void _tcpip_init(void *arg)

#undef  EK_EVENTHANDLER
#define EK_EVENTHANDLER(name, ev, data)					\
  void _tcpip_eventhandler(ek_event_t ev, ek_data_t data)

#undef  EK_POLLHANDLER
#define EK_POLLHANDLER(name)						\
  void _tcpip_pollhandler(void)

#undef  EK_PROCESS
#define EK_PROCESS(name, strname, prio, eventh, pollh, stateptr)	\
  void tcpip_eventhandler(ek_event_t ev, ek_data_t data);		\
  void tcpip_pollhandler(void);						\
  static struct ek_proc name = {NULL, EK_ID_NONE, strname, prio, tcpip_eventhandler, tcpip_pollhandler, stateptr}

#define htons                _htons
#define uiplib_ipaddrconv    _uiplib_ipaddrconv
#define tcp_markconn         _tcp_markconn
#define tcp_listen           _tcp_listen
#define tcp_unlisten         _tcp_unlisten
#define tcp_connect          _tcp_connect
#define udp_new              _udp_new
#define tcpip_set_forwarding _tcpip_set_forwarding
#define tcpip_input          _tcpip_input
#define tcpip_output         _tcpip_output
#define tcpip_poll_tcp       _tcpip_poll_tcp
#define tcpip_poll_udp       _tcpip_poll_udp

#define ek_post_synch        _ek_post_synch
void _ek_post_synch(ek_id_t id, ek_event_t ev, ek_data_t data);

#endif /* UIP_CODE */

#endif /* __UIP_CONF_H__ */
