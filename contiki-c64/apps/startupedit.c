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
 * $Id: startupedit.c,v 1.1 2003/08/05 23:26:44 adamdunkels Exp $
 *
 */

#include "uip_main.h"
#include "uip.h"
#include "uip_arp.h"
#include "resolv.h"
#include "ctk.h"
#include "ctk-draw.h"
#include "dispatcher.h"

#include "loader.h"

/* TCP/IP configuration window. */
static struct ctk_window window;

static struct ctk_label themelabel =
  {CTK_LABEL(0, 1, 9, 1, "CTK theme")};
static char theme[25];
static struct ctk_textentry themetextentry =
  {CTK_TEXTENTRY(11, 1, 16, 1, theme, 24)};

static struct ctk_label driverlabel =
  {CTK_LABEL(0, 3, 10, 1, "Net driver")};
static char driver[25];
static struct ctk_textentry drivertextentry =
  {CTK_TEXTENTRY(11, 3, 16, 1, driver, 24)};


static struct ctk_label ipaddrlabel =
  {CTK_LABEL(0, 5, 10, 1, "IP address")};
static char ipaddr[25];
static struct ctk_textentry ipaddrtextentry =
  {CTK_TEXTENTRY(11, 5, 16, 1, ipaddr, 24)};
static struct ctk_label netmasklabel =
  {CTK_LABEL(0, 7, 10, 1, "Netmask")};
static char netmask[25];
static struct ctk_textentry netmasktextentry =
  {CTK_TEXTENTRY(11, 7, 16, 1, netmask, 24)};
static struct ctk_label gatewaylabel =
  {CTK_LABEL(0, 9, 10, 1, "Gateway")};
static char gateway[25];
static struct ctk_textentry gatewaytextentry =
  {CTK_TEXTENTRY(11, 9, 16, 1, gateway, 24)};
static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 11, 10, 1, "DNS server")};
static char dnsserver[25];
static struct ctk_textentry dnsservertextentry =
  {CTK_TEXTENTRY(11, 11, 16, 1, dnsserver, 24)};

static struct ctk_button closebutton =
  {CTK_BUTTON(0, 13, 2, "Ok")};

static DISPATCHER_SIGHANDLER(startupedit_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Startup editor", NULL, startupedit_sighandler, NULL)};
static ek_id_t id;


static void makestrings(void);

/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(startupedit_init)
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    
    /* Create window. */
    ctk_window_new(&window, 30, 14, "Startup editor");

    CTK_WIDGET_ADD(&window, &themelabel);  
    CTK_WIDGET_ADD(&window, &themetextentry);

    CTK_WIDGET_ADD(&window, &driverlabel);  
    CTK_WIDGET_ADD(&window, &drivertextentry);

    CTK_WIDGET_ADD(&window, &ipaddrlabel);  
    CTK_WIDGET_ADD(&window, &ipaddrtextentry);
    CTK_WIDGET_ADD(&window, &netmasklabel);
    CTK_WIDGET_ADD(&window, &netmasktextentry);
    CTK_WIDGET_ADD(&window, &gatewaylabel);
    CTK_WIDGET_ADD(&window, &gatewaytextentry);
    CTK_WIDGET_ADD(&window, &dnsserverlabel);
    CTK_WIDGET_ADD(&window, &dnsservertextentry);

    CTK_WIDGET_ADD(&window, &closebutton);
    
    CTK_WIDGET_FOCUS(&window, &ipaddrtextentry);  

    /* Fill the configuration strings with values from the current
       configuration */
    makestrings();
    
    dispatcher_listen(ctk_signal_button_activate);
    dispatcher_listen(ctk_signal_window_close);
  }
  ctk_window_open(&window);
}
/*-----------------------------------------------------------------------------------*/
static char *
makebyte(u8_t byte, char *str)
{
  if(byte >= 100) {
    *str++ = (byte / 100 ) % 10 + '0';
  }
  if(byte >= 10) {
    *str++ = (byte / 10) % 10 + '0';
  }
  *str++ = (byte % 10) + '0';

  return str;
}
/*-----------------------------------------------------------------------------------*/
static void
makeaddr(u16_t *addr, char *str)
{
  str = makebyte(HTONS(addr[0]) >> 8, str);
  *str++ = '.';
  str = makebyte(HTONS(addr[0]) & 0xff, str);
  *str++ = '.';
  str = makebyte(HTONS(addr[1]) >> 8, str);
  *str++ = '.';
  str = makebyte(HTONS(addr[1]) & 0xff, str);
  *str++ = 0;
}
/*-----------------------------------------------------------------------------------*/
static void
makestrings(void)
{
  u16_t addr[2], *addrptr;

#ifdef WITH_UIP
  uip_gethostaddr(addr);
  makeaddr(addr, ipaddr);
  
#ifdef WITH_ETHERNET  
  uip_getnetmask(addr);
  makeaddr(addr, netmask);
  
  uip_getdraddr(addr);
  makeaddr(addr, gateway);
#endif /* WITH_ETHERNET */

  addrptr = resolv_getserver();
  if(addrptr != NULL) {
    makeaddr(addrptr, dnsserver);
  }
  
#endif /* WITH_UIP */

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
apply_config(void)
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
startupedit_quit(void)
{
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(startupedit_sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_button_activate) {   
    if(data == (ek_data_t)&closebutton) {
      apply_config();
      ctk_window_close(&window);
      startupedit_quit();
      /*      ctk_desktop_redraw(window.desktop);*/
    }
  } else if(s == ctk_signal_window_close ||
	    s == dispatcher_signal_quit) {
    ctk_window_close(&window);
    startupedit_quit();
  }
}
/*-----------------------------------------------------------------------------------*/
