/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * $Id: main.c,v 1.5 2004/10/09 07:24:26 oliverschmidt Exp $
 *
 */

#include "ctk.h"
#include "ctk-draw.h"
#include "ek.h"

#include "program-handler.h"

#include "uip.h"
#include "uip_arp.h"

#include "resolv.h"

#include "config.h"

#include "config-dsc.h"
#include "www-dsc.h"
#include "telnet-dsc.h"
#include "calc-dsc.h"
#include "processes-dsc.h"
#include "about-dsc.h"

#include "clock.h"

/*-----------------------------------------------------------------------------------*/
unsigned char
uip_fw_forward(void)
{
  return 0;
}
/*-----------------------------------------------------------------------------------*/
void
uip_fw_periodic(void)
{
  return;
}
/*-----------------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  static clock_time_t counter;

  return ++counter;
}
/*-----------------------------------------------------------------------------------*/
void
main(void)
{
  ek_init();
  tcpip_init(NULL);
  
  ctk_init();

  resolv_init(NULL);
  
  program_handler_init();

  program_handler_add(&config_dsc,    "Configuration", 0);
  program_handler_add(&www_dsc,       "Web browser",   1);
  program_handler_add(&telnet_dsc,    "Telnet",        1);
  program_handler_add(&calc_dsc,      "Calculator",    1);
  program_handler_add(&processes_dsc, "Processes",     1);
  program_handler_add(&about_dsc,     "About Contiki", 0);

  config_apply();

  while(1) {
    ek_run();
  }
}
/*-----------------------------------------------------------------------------------*/
