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
 * $Id: main-rrnet.c,v 1.2 2004/07/04 18:33:07 adamdunkels Exp $
 *
 */

#include "ctk.h"
#include "ctk-draw.h"
#include "dispatcher.h"

#include "program-handler.h"

#include "uip_main.h"
#include "uip-signal.h"
#include "uip.h"
#include "uip_arp.h"
#if WITH_TFE
void tfe_drv_init(void);
#endif /* WITH_TFE */
#if WITH_RRNET
void rrnet_drv_init(void);
#endif /* WITH_RRNET */
#include "resolv.h"
#include "rs232dev.h"

#include "about-dsc.h"
#include "netconf-dsc.h"
#include "processes-dsc.h"
#include "memstat-dsc.h"
#include "directory-dsc.h"
#include "wget-dsc.h"
#include "www-dsc.h"


#include "webserver.h"

/*-----------------------------------------------------------------------------------*/
int
main(int argc, char **argv)
{
  u16_t addr[2];
 
  c64_dio_init(8);
 
  dispatcher_init();
  ctk_init();

  uip_init();
  uip_signal_init();
  resolv_init();

  /* XXX: just for making it easier to test. */
#if 0
  uip_ipaddr(addr, 193,10,67,150);
  uip_sethostaddr(addr);
 
  uip_ipaddr(addr, 193,10,64,1);
  uip_setdraddr(addr);
 
  uip_ipaddr(addr, 255,255,252,0);
  uip_setnetmask(addr);

  uip_ipaddr(addr, 193,10,66,195);
  resolv_conf(addr);

#else
  uip_ipaddr(addr, 192,168,254,2);
  uip_sethostaddr(addr);
 
  uip_ipaddr(addr, 192,168,254,1);
  uip_setdraddr(addr);
 
  uip_ipaddr(addr, 255,255,255,0);
  uip_setnetmask(addr);

  uip_ipaddr(addr, 195,54,122,204);
  resolv_conf(addr);
#endif
  
  rrnet_drv_init();
	    
  program_handler_init();

    
  /*program_handler_add(&directory_dsc, "Directory", 1);*/
  program_handler_add(&netconf_dsc, "Network setup", 1);  
  /*  program_handler_add(&processes_dsc, "Processes", 1);

  program_handler_add(&memstat_dsc, "Memory stats", 1);*/
  program_handler_add(&wget_dsc, "Web downloader", 1);
  /*  program_handler_add(&about_dsc, "About", 0);*/

  /*  program_handler_add(&vnc_dsc, "VNC viewer", 1);
      program_handler_add(&www_dsc, "Web browser", 1);*/
  
  
  /*  webserver_init();*/
  /*  netconf_init();*/
  wget_init();

  /*  processes_init();*/
  
  dispatcher_run();

  clrscr();
  
  return 0;

  argv = argv;
  argc = argc;
}
/*-----------------------------------------------------------------------------------*/
