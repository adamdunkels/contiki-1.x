/*
 * Copyright (c) 2002, Adam Dunkels.
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
 * This file is part of the Contiki desktop environment 
 *
 * $Id: contiki-main.c,v 1.3 2003/08/25 12:42:41 adamdunkels Exp $
 *
 */

#include "ctk.h"
#include "ctk-draw.h"
#include "ctk-vncserver.h"
#include "dispatcher.h"


#include "uip_main.h"
#include "uip.h"
#include "uip_arp.h"
#include "rtl8019-drv.h"
#include "resolv.h"

#include "webserver.h"
#include "program-handler.h"
#include "about-dsc.h"
#include "netconf-dsc.h"
#include "processes-dsc.h"
#include "calc-dsc.h"
#include "www-dsc.h"
#include "webserver-dsc.h"
/*#include "directory-dsc.h"*/
#include "weblinks-dsc.h"

#include "debug.h"
#include "uip.h"
#include "uip_arp.h"
#include "compiler.h"
#include "rtl8019dev.h"


#include "avr/pgmspace.h"


static const struct uip_eth_addr ethaddr = {{0x00,0x06,0x98,0x01,0x02,0x26}};


static u16_t addr[2];

/*-----------------------------------------------------------------------------------*/
static void setup_xram(void) __attribute__ ((naked)) \
     __attribute__ ((section (".init1")));

static void
setup_xram(void)
{
  outp(BV(SRE) | BV(SRW), MCUCR);
}
/*-----------------------------------------------------------------------------------*/
static unsigned short count;

static void init_timer(void)
{
  /* timer overflows every 32.8ms (with 8MHz clock) */
  /* timer0 prescale 1/1024 (5) */
  outp(5, TCCR0);  

  /* interrupt on overflow */
  sbi(TIMSK, TOIE0);
        
  count = 0;
}

SIGNAL(SIG_OVERFLOW0)
{
  ++count;
}

unsigned short
clock(void)
{
  return count;
}
/*-----------------------------------------------------------------------------------*/
int
main(int argc, char **argv)
{
  
  /* Setup stack pointer so that it does not interfere with the rest
     of the RAM. */
  /*  asm("ldi     r28,lo8(0x00807fff)");
      asm("ldi     r29,hi8(0x00807fff)");
      asm("out     0x3e, r29");
      asm("out     0x3d, r28");*/

  /* Fiddle with RS232 settings of the AVR: */
  /* Enable transmit. */
  UCSR0B = _BV(TXEN);
  /* Set baud rate (23 =~ 38400) */
  UBRR0H = 0;
  UBRR0L = 23;
  UDR0 = '\n';

  
  outp(BV(SRE) | BV(SRW), MCUCR);

    
  /* end RS232 fiddling. */

  init_timer();
  sei();
  
  uip_init();
  
  resolv_init();
  

  debug_print8(1);
  
  
  uip_setethaddr(ethaddr);

 
    
#if 1
  uip_ipaddr(addr, 193,10,67,152);
  uip_sethostaddr(addr);
 
  uip_ipaddr(addr, 193,10,67,1);
  uip_setdraddr(addr);
 
  uip_ipaddr(addr, 255,255,255,0);
  uip_setnetmask(addr);

  uip_ipaddr(addr, 193,10,66,195);
  resolv_conf(addr);

#else

  uip_ipaddr(addr, 192,168,1,2);
  uip_sethostaddr(addr);
 
  uip_ipaddr(addr, 192,168,1,1);
  uip_setdraddr(addr);
 
  uip_ipaddr(addr, 255,255,255,0);
  uip_setnetmask(addr);

  uip_ipaddr(addr, 195,54,122,204);
  resolv_conf(addr);
#endif

  
    
  dispatcher_init();

  
  ctk_init();

  rtl8019_drv_init();

  ctk_vncserver_init(NULL);  

  program_handler_init();

  webserver_init(NULL);

  
  /*  program_handler_add(&directory_dsc, "Directory", 1);*/
  program_handler_add(&about_dsc, "About", 1);
  program_handler_add(&webserver_dsc, "Web server", 1);
  program_handler_add(&www_dsc, "Web browser", 1);
  /*  program_handler_add(&calc_dsc, "Calculator", 0);*/
  program_handler_add(&processes_dsc, "Processes", 0);
  program_handler_add(&weblinks_dsc, "Web links", 1);


  debug_print8(64);
  dispatcher_run();

  
  return 0;

  argv = argv;
  argc = argc;
}
/*-----------------------------------------------------------------------------------*/
