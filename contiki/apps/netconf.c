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
 * $Id: netconf.c,v 1.1 2003/04/08 17:56:44 adamdunkels Exp $
 *
 */

#include "uip_main.h"
#include "uip.h"
#include "uip_arp.h"
#include "ctk.h"
#include "dispatcher.h"

#include "loader.h"

/* TCP/IP configuration window. */
static struct ctk_window tcpipwindow;

#ifdef WITH_ETHERNET
static struct ctk_label ipaddrlabel =
  {CTK_LABEL(0, 1, 10, 1, "IP address")};
static char ipaddr[17];
static struct ctk_textentry ipaddrtextentry =
  {CTK_TEXTENTRY(11, 1, 16, 1, ipaddr, 16)};
static struct ctk_label netmasklabel =
  {CTK_LABEL(0, 3, 10, 1, "Netmask")};
static char netmask[17];
static struct ctk_textentry netmasktextentry =
  {CTK_TEXTENTRY(11, 3, 16, 1, netmask, 16)};
static struct ctk_label gatewaylabel =
  {CTK_LABEL(0, 5, 10, 1, "Gateway")};
static char gateway[17];
static struct ctk_textentry gatewaytextentry =
  {CTK_TEXTENTRY(11, 5, 16, 1, gateway, 16)};
static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 7, 10, 1, "DNS server")};
static char dnsserver[17];
static struct ctk_textentry dnsservertextentry =
  {CTK_TEXTENTRY(11, 7, 16, 1, dnsserver, 16)};
#else /* WITH_ETHERNET */
static struct ctk_label ipaddrlabel =
  {CTK_LABEL(0, 2, 10, 1, "IP address")};
static char ipaddr[17];
static struct ctk_textentry ipaddrtextentry =
  {CTK_TEXTENTRY(11, 2, 16, 1, ipaddr, 16)};
static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 4, 10, 1, "DNS server")};
static char dnsserver[17];
static struct ctk_textentry dnsservertextentry =
  {CTK_TEXTENTRY(11, 4, 16, 1, dnsserver, 16)};
#endif /* WITH_ETHERNET */

static struct ctk_button tcpipclosebutton =
  {CTK_BUTTON(0, 9, 2, "Ok")};

static void netconf_sighandler(ek_signal_t s, ek_data_t data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Network config", NULL, netconf_sighandler, NULL)};
static ek_id_t id;


/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(netconf_init)
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    
    /* Create TCP/IP configuration window. */
    ctk_window_new(&tcpipwindow, 30, 10, "TCP/IP config");
    ctk_window_move(&tcpipwindow, 4, 6);
    
    
#ifdef WITH_ETHERNET
    CTK_WIDGET_ADD(&tcpipwindow, &ipaddrlabel);  
    CTK_WIDGET_ADD(&tcpipwindow, &ipaddrtextentry);
    CTK_WIDGET_ADD(&tcpipwindow, &netmasklabel);
    CTK_WIDGET_ADD(&tcpipwindow, &netmasktextentry);
    CTK_WIDGET_ADD(&tcpipwindow, &gatewaylabel);
    CTK_WIDGET_ADD(&tcpipwindow, &gatewaytextentry);
    CTK_WIDGET_ADD(&tcpipwindow, &dnsserverlabel);
    CTK_WIDGET_ADD(&tcpipwindow, &dnsservertextentry);
#else
    CTK_WIDGET_ADD(&tcpipwindow, &ipaddrlabel);  
    CTK_WIDGET_ADD(&tcpipwindow, &ipaddrtextentry);
    CTK_WIDGET_ADD(&tcpipwindow, &dnsserverlabel);
    CTK_WIDGET_ADD(&tcpipwindow, &dnsservertextentry);  
#endif /* WITH_ETHERNET */
    
    CTK_WIDGET_ADD(&tcpipwindow, &tcpipclosebutton);
    
    CTK_WIDGET_FOCUS(&tcpipwindow, &ipaddrtextentry);  
    
    dispatcher_listen(ctk_signal_button_activate);
    dispatcher_listen(ctk_signal_window_close);
  }
  ctk_window_open(&tcpipwindow);
}
/*-----------------------------------------------------------------------------------*/
static void
nullterminate(char *cptr)
{
  /* Find the first space character in the ipaddr and put a zero there
     to end the string. */
  for(; *cptr != ' ' && *cptr != 0; ++cptr);
  *cptr = 0;
}
/*-----------------------------------------------------------------------------------*/
static void
apply_tcpipconfig(void)
{
  u16_t addr[2];

#ifdef WITH_UIP
  nullterminate(ipaddr);
  if(uip_main_ipaddrconv(ipaddr, (unsigned char *)addr)) {
    uip_sethostaddr(addr);
  }
  
#ifdef WITH_ETHERNET
  nullterminate(netmask);
  if(uip_main_ipaddrconv(netmask, (unsigned char *)addr)) {
    uip_setnetmask(addr);
  }

  nullterminate(gateway);
  if(uip_main_ipaddrconv(gateway, (unsigned char *)addr)) {
    uip_setdraddr(addr);
  }
#endif /* WITH_ETHERNET */
  
  nullterminate(dnsserver);
  if(uip_main_ipaddrconv(dnsserver, (unsigned char *)addr)) {
    resolv_conf(addr);
  }
#endif /* WITH_UIP */
}
/*-----------------------------------------------------------------------------------*/
static void
quit(void)
{
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
  ctk_redraw();
}
/*-----------------------------------------------------------------------------------*/
static void
netconf_sighandler(ek_signal_t s, ek_data_t data)
{
  if(s == ctk_signal_button_activate) {   
    if(data == (ek_data_t)&tcpipclosebutton) {
      apply_tcpipconfig();
      ctk_window_close(&tcpipwindow);
      quit();
    }
  } else if(s == ctk_signal_window_close) {
    quit();
  }
}
/*-----------------------------------------------------------------------------------*/
