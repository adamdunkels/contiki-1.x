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
 * $Id: contiki-main.c,v 1.1 2003/03/19 16:26:18 adamdunkels Exp $
 *
 */

#include "ctk.h"
#include "ctk-draw.h"
#include "dispatcher.h"

#include "simpletelnet.h"
#include "programs.h"
#include "email.h"
#include "www.h"
#include "contiki.h"

#include "uip_main.h"
#include "uip.h"
#include "uip_arp.h"
#if WITH_TFE
#include "cs8900a.h"
#endif /* WITH_TFE */
#include "resolv.h"

#include <time.h>
#include <stdio.h>
#include <6502.h>
#include <conio.h>
#include <rs232.h>
#include <time.h>
#include <string.h>


/*-----------------------------------------------------------------------------------*/
int
main(int argc, char **argv)
{
  /*  irqload_init();*/

#ifdef WITH_UIP
  uip_init();
  uip_main_init();
  resolv_init();

#ifdef WITH_TFE
  cs8900a_init();
#endif /* WITH_TFE */

  
#ifdef WITH_RS232
  rs232dev_init();
#endif /* WITH_RS232 */
  
#ifdef WITH_TAPDEV
  tapdev_init();
#endif /* WITH_TAPDEV */


#endif /* WITH_UIP */
	    
  ek_init();
  dispatcher_init();
  ctk_init();
  
  contiki_init();
  
  programs_init();
  
  ctk_redraw();
  ek_run();

  clrscr();
  
  return 0;

  argv = argv;
  argc = argc;
}
/*-----------------------------------------------------------------------------------*/
