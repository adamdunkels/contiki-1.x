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
 * $Id: config.c,v 1.5 2005/03/13 22:15:54 oliverschmidt Exp $
 *
 */


#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "uiplib.h"
#include "resolv.h"

#include "program-handler.h"

#include "config.h"


static config_t config = {0, 4, "LANceGS.drv"};

/*-----------------------------------------------------------------------------------*/
static void
config_load(void)
{
  int fd;

  fd = open("contiki.cfg", 0);
  if(fd == -1) {
    return;
  }
  read(fd, &config, sizeof(config));
  close(fd);
}
/*-----------------------------------------------------------------------------------*/
static void
config_apply(void)
{
#ifdef __APPLE2ENH__

  ctk_draw_setbackground(config.bkgnd);

#endif /* __APPLE2ENH__ */

  config_setlanslot(config.slot);

  if(*config.driver) {
    program_handler_load(config.driver, NULL);
  }

#ifdef WITH_UIP

  uip_sethostaddr(config.ipaddr);

#ifdef WITH_ETHERNET

  uip_setnetmask(config.netmask);
  uip_setdraddr(config.gateway);

#endif /* WITH_ETHERNET */

  resolv_conf(config.dnsserver);

#endif /* WITH_UIP */
}
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(config_init, arg)
{
  arg_free(arg);
  config_load();
  config_apply();
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
