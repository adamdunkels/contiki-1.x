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
 * $Id: uip_main.c,v 1.3 2003/04/25 08:48:57 adamdunkels Exp $
 *
 */


/* uip_main.c: initialization code and main event loop. */

#define NULL (void *)0



#include "uip.h"
#include "uip_arp.h"

#ifdef WITH_TFE
#include "cs8900a.h"
#endif /* WITH_TFE */

#include "ek.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

static u8_t i, arptimer;
static u16_t start, current;

#if 0
static void idle(void);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("uIP TCP/IP stack", idle, NULL, NULL)};
static ek_id_t id;
#endif /* 0 */

#if 0
/*-----------------------------------------------------------------------------------*/
static void
timer(void)
{
  for(i = 0; i < UIP_CONNS; ++i) {
    uip_periodic(i);
    if(uip_len > 0) {
#ifdef WITH_TFE
      uip_arp_out();
      cs8900a_send();
#endif /* WITH_TFE */
#ifdef WITH_RS232
      rs232dev_send();
#endif /* WITH_RS232 */
    }
  }

  for(i = 0; i < UIP_UDP_CONNS; i++) {
    uip_udp_periodic(i);
    /* If the above function invocation resulted in data that
       should be sent out on the network, the global variable
       uip_len is set to a value > 0. */
    if(uip_len > 0) {
#ifdef WITH_TFE
      uip_arp_out();
      cs8900a_send();
#endif /* WITH_TFE */
#ifdef WITH_RS232
      rs232dev_send();
#endif /* WITH_RS232 */
    }
  }   
}
#endif /* 0 */
/*-----------------------------------------------------------------------------------*/
void
uip_main_init(void)
{
#if 0
  u16_t ipaddr[2];

  id = dispatcher_start(&p);
  
  arptimer = 0;
  start = ek_clock();
#endif /* 0 */
}
/*-----------------------------------------------------------------------------------*/
#if 0
static void
idle(void)
{
#ifdef WITH_TFE
  /* Poll Ethernet device to see if there is a frame avaliable. */
  uip_len = cs8900a_poll();
  if(uip_len > 0) {
    /* A frame was avaliable (and is now read into the uip_buf), so
       we process it. */ 
    if(BUF->type == htons(UIP_ETHTYPE_IP)) {
      uip_arp_ipin();
      uip_len -= sizeof(struct uip_eth_hdr);
      uip_input();
      /* If the above function invocation resulted in data that
	 should be sent out on the network, the global variable
	 uip_len is set to a value > 0. */
      if(uip_len > 0) {
	uip_arp_out();
	cs8900a_send();
      }
    } else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
      uip_arp_arpin();
      /* If the above function invocation resulted in data that
	 should be sent out on the network, the global variable
	 uip_len is set to a value > 0. */	
      if(uip_len > 0) {
	cs8900a_send();
      }
    }
  }
#endif /* WITH_TFE */
  
#ifdef WITH_RS232
  uip_len = rs232dev_poll();
  if(uip_len > 0) {
    uip_process(UIP_DATA);
    if(uip_len > 0) {
      rs232dev_send();
    }
  }
#endif /* WITH_RS232 */

  /* Check the clock so see if we should call the periodic uIP
     processing. */
  current = ek_clock();

  if((current - start) >= CLK_TCK/2) {
    timer();
    start = current;
  }    
}
#endif /* 0 */
/*-----------------------------------------------------------------------------------*/
unsigned char
uip_main_ipaddrconv(char *addrstr, unsigned char *ipaddr)
{
  unsigned char tmp;
  char c;
  unsigned char i, j;

  tmp = 0;
  
  for(i = 0; i < 4; ++i) {
    j = 0;
    do {
      c = *addrstr;
      ++j;
      if(j > 4) {
	return 0;
      }
      if(c == '.' || c == 0) {
	*ipaddr = tmp;
	++ipaddr;
	tmp = 0;
      } else if(c >= '0' || c <= '9') {
	tmp = (tmp * 10) + (c - '0');
      } else {
	return 0;
      }
      ++addrstr;
    } while(c != '.' && c != 0);
  }
  return 1;
}

/*-----------------------------------------------------------------------------------*/
