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
 * $Id: contiki-main.c,v 1.2 2004/07/04 20:01:15 adamdunkels Exp $
 *
 */

#include "contiki.h"
#include "ctk.h"
#include "ctk-draw.h"


#include "tcpip.h"
#include "uip.h"
#include "uip_arp.h"
#if WITH_TFE
#include "cs8900a.h"
#endif /* WITH_TFE */
#include "resolv.h"

#include "program-handler.h"

/*-----------------------------------------------------------------------------------*/
int
main(void)
{
  ek_init();
    
#ifdef WITH_UIP
  tcpip_init(NULL);
  resolv_init(NULL);

#ifdef WITH_RS232
  rs232dev_init();
#endif /* WITH_RS232 */
  
#endif /* WITH_UIP */
  
  ctk_init();
  
  program_handler_init();
  
  while(1) {
    ek_run();
  }
  
  return 0;
}
/*-----------------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return clock();
}

int
uip_fw_forward(void)
{
  return 0;
}
void
uip_fw_periodic(void)
{

}
