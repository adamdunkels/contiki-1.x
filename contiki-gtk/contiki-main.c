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
 * $Id: contiki-main.c,v 1.4 2003/04/17 20:07:24 adamdunkels Exp $
 *
 */


#include "ctk.h"
#include "ctk-draw.h"
#include "dispatcher.h"

#include "program-handler.h"

#include "uip_main.h"
#include "uip.h"
#include "uip_arp.h"

#include "about-dsc.h"
#include "netconf-dsc.h"
#include "processes-dsc.h"

#include "www-dsc.h"

static gint
idle_callback(gpointer data)
{
  ek_signals();
  ek_idle();
  return TRUE;
}
/*-----------------------------------------------------------------------------------*/
int
main(int argc, char **argv)
{
  u16_t addr[2];

  uip_init();
  uip_main_init();
  resolv_init();

  /* XXX: just for making it easier to test. */
  uip_ipaddr(addr, 192,168,1,2);
  uip_sethostaddr(addr);

  uip_ipaddr(addr, 192,168,1,1);
  uip_setdraddr(addr);

  uip_ipaddr(addr, 255,255,255,0);
  uip_setnetmask(addr);

  
  tapdev_init();
  
  ek_init();
  ctk_gtksim_init();
  ctk_init();
  
  program_handler_init();
  
  gtk_timeout_add(20, idle_callback, NULL);

  /*  vnc_init();*/
  program_handler_add(&about_dsc, "About", 1);
  program_handler_add(&netconf_dsc, "Network setup", 1);
  program_handler_add(&processes_dsc, "Processes", 0);
  
  program_handler_add(&www_dsc, "Web browser", 1);

  
  ctk_redraw();
  gtk_main();
    
  return 0;

  argv = argv;
  argc = argc;
}
/*-----------------------------------------------------------------------------------*/
