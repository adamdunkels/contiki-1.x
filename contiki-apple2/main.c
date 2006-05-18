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
 * $Id: main.c,v 1.14 2006/05/18 16:20:08 oliverschmidt Exp $
 *
 */

#include <string.h>

#include "ctk.h"
#include "ctk-draw.h"
#include "ek.h"

#include "program-handler.h"

#include "uip.h"
#include "uip_arp.h"

#include "resolv.h"

#include "www-dsc.h"
#include "email-dsc.h"
#include "ftp-dsc.h"
#include "directory-dsc.h"

#include "clock.h"
#include "clock-arch.h"

unsigned char lanslot;

#pragma bssseg(push, "UIPBUF");
u8_t uip_buf[UIP_BUFSIZE + 2];
#pragma bssseg(pop);

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
EK_EVENTHANDLER(eventhandler, ev, data)
{
  switch(ev) {
  case EK_EVENT_INIT:
    program_handler_load("config.prg", NULL);
    break;
  }
}
/*-----------------------------------------------------------------------------------*/
EK_PROCESS(init, "Init", EK_PRIO_LOWEST,
	   eventhandler, NULL, NULL);
/*-----------------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  static clock_time_t count, clock;

  count += tick;

  if(count == 4200) {
    count = 0;
    ++clock;
  }

  return clock;
}
/*-----------------------------------------------------------------------------------*/
void
#ifdef __APPLE2__
main(void)
#else /* __APPLE2__ */
main(int argc, char *argv[])
#endif /* __APPLE2__ */
{
  ek_init();
  ek_start(&init);

  tcpip_init(NULL);
  resolv_init(NULL);
  
  ctk_init();
  
  program_handler_init();

  program_handler_add(&www_dsc,       "Web browser", 1);
  program_handler_add(&email_dsc,     "E-mail",      1);
  program_handler_add(&ftp_dsc,       "FTP client",  1);
  program_handler_add(&directory_dsc, "Directory",   1);

  while(1) {
    if(ek_run() == 0) {

#ifdef __APPLE2__

      program_handler_load("welcome.prg", NULL);

#else /* __APPLE2__ */

      static char *startup;
      static char *slash;

      if(argc == 1) {
        startup = "welcome.prg";
      } else {
	startup = argv[1];
	while(slash = strchr(startup, '/')) {
	  startup = slash + 1;
	}
      }
      program_handler_load(startup, NULL);

#endif /* __APPLE2__ */

      break;
    }
  }
  while(1) {
    ek_run();
  }
}
/*-----------------------------------------------------------------------------------*/
