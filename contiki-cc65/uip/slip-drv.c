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
 * This file is part of the Contiki desktop OS fpr the C64
 *
 * $Id: slip-drv.c,v 1.5 2003/08/24 22:41:55 adamdunkels Exp $
 *
 */


/* uip_main.c: initialization code and main event loop. */

#include "uip.h"
#include "uip_arp.h"
#include "uip-signal.h"
#include "rs232dev.h"
#include "loader.h"
#include "ek.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

static u8_t i, arptimer;
static u16_t start, current;

static void slip_drv_idle(void);
static DISPATCHER_SIGHANDLER(slip_drv_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("TCP/IP/SLIP driver", slip_drv_idle,
		   slip_drv_sighandler, NULL)};
static ek_id_t id;


/*-----------------------------------------------------------------------------------*/
static void
timer(void)
{
  for(i = 0; i < UIP_CONNS; ++i) {
    uip_periodic(i);
    if(uip_len > 0) {
      rs232dev_send();
    }
  }

  for(i = 0; i < UIP_UDP_CONNS; i++) {
    uip_udp_periodic(i);
    /* If the above function invocation resulted in data that
       should be sent out on the network, the global variable
       uip_len is set to a value > 0. */
    if(uip_len > 0) {
      rs232dev_send();
    }
  }   
}
/*-----------------------------------------------------------------------------------*/
static void
slip_drv_idle(void)
{
  uip_len = rs232dev_poll();
  if(uip_len > 0) {
    uip_process(UIP_DATA);
    if(uip_len > 0) {
      rs232dev_send();
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
LOADER_INIT_FUNC(slip_drv_init, arg)
{
  arg_free(arg);
  
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);

    rs232dev_init();
    
    arptimer = 0;
    start = ek_clock();

    dispatcher_listen(uip_signal_poll);
    dispatcher_listen(uip_signal_poll_udp);
    dispatcher_listen(uip_signal_uninstall);    
  }
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(slip_drv_sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);

  if(s == uip_signal_poll) {
    uip_periodic_conn(data);
    if(uip_len > 0) {
      rs232dev_send();
    }
  } else if(s == uip_signal_poll_udp) {
    uip_udp_periodic_conn(data);
    if(uip_len > 0) {
      rs232dev_send();
    }    
  } else if(s == dispatcher_signal_quit ||
     s == uip_signal_uninstall) {
    dispatcher_exit(&p);
    id = EK_ID_NONE;
    LOADER_UNLOAD();   
  }
}
/*-----------------------------------------------------------------------------------*/
