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
 * $Id: contiki-main.c,v 1.13 2004/08/11 21:26:22 adamdunkels Exp $
 *
 */

#include "ek.h"
#include "clock.h"

#include <gdk/gdktypes.h>
#include <gtk/gtk.h>

#include "uip.h"
#include "ctk.h"

#include "ctk-vncserver.h"
#include "ctk-termtelnet.h"

#include "cfs-posix.h"

#include "uip-fw.h"

#include "about-dsc.h"
#include "calc-dsc.h"
#include "editor-dsc.h"
#include "email-dsc.h"
#include "ftp-dsc.h"
#include "netconf-dsc.h"
#include "processes-dsc.h"
#include "shell-dsc.h"
#include "www-dsc.h"



void tapdev_send(void);
static struct uip_fw_netif tapif =
  {UIP_FW_NETIF(0,0,0,0, 0,0,0,0, tapdev_send)};


static gint
idle_callback(gpointer data)
{

  ek_run();
  return TRUE;
}
/*-----------------------------------------------------------------------------------*/
int
main(int argc, char **argv)
{
  u16_t addr[2];
  
  /*  html_test();*/

  gtk_init(&argc, &argv);
  
  ek_init();
  
  tcpip_init(NULL);

  ctk_gtksim_service_init(NULL);
  ctk_init();

  /*  ctk_vncserver_init(NULL);*/
  /*  ctk_termtelnet_init(NULL);*/
  
  uip_init();
  uip_ipaddr(addr, 192,168,2,2);
  uip_sethostaddr(addr);

  uip_ipaddr(addr, 192,168,2,1);
  uip_setdraddr(addr);

  uip_ipaddr(addr, 255,255,255,0);
  uip_setnetmask(addr);  


  resolv_init(NULL);
  
  tapdev_init();
  uip_fw_service_init();
  uip_fw_init();
  uip_fw_default(&tapif);

  /*  tapdev_service_init(NULL);*/


  program_handler_init();
  program_handler_add(&about_dsc, "About", 1);
  program_handler_add(&netconf_dsc, "Network setup", 1);
  program_handler_add(&ftp_dsc, "FTP client", 1);
  program_handler_add(&editor_dsc, "Editor", 1);
  program_handler_add(&www_dsc, "Web browser", 1);
  program_handler_add(&processes_dsc, "Processes", 1);
  program_handler_add(&shell_dsc, "Command shell", 1);
  program_handler_add(&calc_dsc, "Calculator", 1);
  program_handler_add(&email_dsc, "E-mail", 1);
  
  gtk_timeout_add(20, idle_callback, NULL);


  cfs_posix_init(NULL);

  /*  sock_httpd_init(NULL);*/
 
  gtk_main();
    
    
  return 0;

  argv = argv;
  argc = argc;
}
/*-----------------------------------------------------------------------------------*/
#include <sys/time.h>
 
clock_time_t
clock_time(void)
{
  struct timeval tv;
  struct timezone tz;
   
  gettimeofday(&tv, &tz);
 
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
/*-----------------------------------------------------------------------------------*/

void nntpc_done(int i) {}

void log_message(char *m1, char *m2)
{
  printf("%s%s\n", m1, m2);
}
