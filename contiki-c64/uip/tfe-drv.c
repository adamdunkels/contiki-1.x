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
 * $Id: tfe-drv.c,v 1.7 2003/08/29 20:35:13 adamdunkels Exp $
 *
 */


#include "uip.h"
#include "uip_arp.h"
#include "uip-signal.h"
#include "loader.h"
#include "cs8900a.h"

#include "dispatcher.h"
#include "ek.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

static u8_t i, arptimer;
static u16_t start, current;

static void tfe_drv_idle(void);
static DISPATCHER_SIGHANDLER(tfe_drv_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("TCP/IP/TFE driver", tfe_drv_idle,
		   tfe_drv_sighandler, NULL)};
static ek_id_t id;

/*-----------------------------------------------------------------------------------*/
static void
send(void)
{
  if(uip_len > 0) {
    uip_arp_out();
    cs8900a_send();
  }
}
/*-----------------------------------------------------------------------------------*/
static void
timer(void)
{
  for(i = 0; i < UIP_CONNS; ++i) {
    uip_periodic(i);
    send();
  }

  for(i = 0; i < UIP_UDP_CONNS; i++) {
    uip_udp_periodic(i);
    /* If the above function invocation resulted in data that
       should be sent out on the network, the global variable
       uip_len is set to a value > 0. */
    send();
  }   
}
/*-----------------------------------------------------------------------------------*/
static void
tfe_drv_idle(void)
{
  /* Poll Ethernet device to see if there is a frame avaliable. */
  uip_len = cs8900a_poll();
  if(uip_len > 0) {
    /* A frame was avaliable (and is now read into the uip_buf), so
       we process it. */ 
    if(BUF->type == HTONS(UIP_ETHTYPE_IP)) {
      uip_arp_ipin();
      uip_len -= sizeof(struct uip_eth_hdr);
      uip_input();
      /* If the above function invocation resulted in data that
	 should be sent out on the network, the global variable
	 uip_len is set to a value > 0. */
      send();
    } else if(BUF->type == HTONS(UIP_ETHTYPE_ARP)) {
      uip_arp_arpin();
      /* If the above function invocation resulted in data that
	 should be sent out on the network, the global variable
	 uip_len is set to a value > 0. */	
      if(uip_len > 0) {
	cs8900a_send();
      }
    }
  }
  /* Check the clock so see if we should call the periodic uIP
     processing. */
  current = ek_clock();

  if((current - start) >= CLK_TCK/2) {
    timer();
    start = current;
  }    
}
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(tfe_drv_init, arg)
{
  arg_free(arg);
  
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    
    arptimer = 0;
    start = ek_clock();

    cs8900a_init();
    
    dispatcher_listen(uip_signal_poll);
    dispatcher_listen(uip_signal_poll_udp);
    dispatcher_listen(uip_signal_uninstall);
  }
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(tfe_drv_sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);

  if(s == uip_signal_poll) {
    uip_periodic_conn(data);
    send();
  } else if(s == uip_signal_poll_udp) {
    uip_udp_periodic_conn(data);
    send();
  } else if(s == dispatcher_signal_quit ||
	    s == uip_signal_uninstall) {
    dispatcher_exit(&p);
    id = EK_ID_NONE;
    LOADER_UNLOAD();   
  }
}
/*-----------------------------------------------------------------------------------*/
