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
 * 3. The name of the author may not be used to endorse or promote
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
 * $Id: contiki-main.c,v 1.3 2003/11/27 15:56:57 adamdunkels Exp $
 *
 */

#include <io.h>
#include <signal.h>


#include "ctk.h"
#include "ctk-draw.h"
#include "ctk-vncserver.h"
#include "dispatcher.h"


#include "uip_main.h"
#include "uip.h"
#include "uip_arp.h"
#include "resolv.h"

#include "webserver.h"
#include "program-handler.h"
#include "about-dsc.h"
#include "netconf-dsc.h"
#include "processes-dsc.h"
#include "www-dsc.h"
#include "webserver-dsc.h"

#include "sensorview-dsc.h"

#include "sensors.h"

#include "uip.h"
#include "uip_arp.h"

#include "msp430.h"


#include "rs232.h"
void slip_drv_init(char *arg);



static u16_t addr[2];
/*-----------------------------------------------------------------------------------*/

int
main(int argc, char **argv)
{

  msp430_init();

  rs232_init(0);
  
  sensors_init();
  
  rs232_print("uip_init()...\n"); 
  uip_init();

  /*  rs232_print("resolv_init()...\n");
      resolv_init();*/
  

  
#if 0
  uip_ipaddr(addr, 193,10,67,150);
  uip_sethostaddr(addr);
 
  /*  uip_ipaddr(addr, 193,10,66,195);
      resolv_conf(addr);*/

#else

  uip_ipaddr(addr, 172,16,0,2);
  uip_sethostaddr(addr);
 
  /*  uip_ipaddr(addr, 193,10,66,195);
      resolv_conf(addr);*/
#endif

  
  rs232_print("dispatcher_init()...\n");
  dispatcher_init();

  
  rs232_print("slip_drv_init()...\n");  
  slip_drv_init(NULL);


  rs232_print("ctk_init()...\n");  
  /*  ctk_init();*/

  rs232_print("ctk_vncserver_init()...\n");  
  /*  ctk_vncserver_init(NULL);  */

  rs232_print("program_handler_init()...\n");  
  /*  program_handler_init();*/

  /*  rs232_print("processes_init()...\n");  
      processes_init(NULL);*/

  rs232_print("webserver_init()...\n");  
  /*  webserver_init(NULL);*/

  sensorcheck_init(NULL);
  
  
  /*  program_handler_add(&directory_dsc, "Directory", 1);*/
  /*  program_handler_add(&about_dsc, "About", 1);*/
  /*  program_handler_add(&webserver_dsc, "Web server", 1);*/
  /*  program_handler_add(&www_dsc, "Web browser", 1);*/
  /*  program_handler_add(&calc_dsc, "Calculator", 0);*/
  /*  program_handler_add(&processes_dsc, "Processes", 0);*/

  rs232_print("program_handler_addd()...\n");  
  /*  program_handler_add(&sensorview_dsc, "Sensors", 1);*/
  

  rs232_print("dispatcher_run()...\n");
  beep();
  dispatcher_run();
  
  return 0;

  argv = argv;
  argc = argc;



}
/*-----------------------------------------------------------------------------------*/
char *arg_alloc(char size) {return NULL;}
void  arg_init(void) {}
void  arg_free(char *arg) {}
/*-----------------------------------------------------------------------------------*/
