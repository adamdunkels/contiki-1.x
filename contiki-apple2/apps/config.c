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
 * $Id: config.c,v 1.1 2004/06/14 22:30:33 oliverschmidt Exp $
 *
 */

#include "uip.h"
#include "uiplib.h"
#include "uip_arp.h"
#include "resolv.h"
#include "ctk.h"
#include "ctk-draw.h"
#include "dispatcher.h"
#include "config.h"

#include "loader.h"

#include <string.h>


#pragma dataseg(push, "CONF");

static char header[] = "CONTIKI";
config_t config = {"x", "1"};

#pragma dataseg(pop);


static config_t screen;
static struct ctk_window window;

#ifdef __APPLE2ENH__
static struct ctk_label backgroundlabel =
  {CTK_LABEL(0, 1, 10, 1, "Background")};
static struct ctk_textentry backgroundtextentry =
  {CTK_TEXTENTRY(11, 1, 1, 1, screen.bkgnd, 1)};
static struct ctk_label backgrounddescr =
  {CTK_LABEL(16, 1, 12, 1, "('x' or ' ')")};
#endif /* __APPLE2ENH__ */
static struct ctk_label slotlabel =
  {CTK_LABEL(0, 3, 10, 1, "Slot")};
static struct ctk_textentry slottextentry =
  {CTK_TEXTENTRY(11, 3, 1, 1, screen.slot, 1)};
static struct ctk_label slotdescr =
  {CTK_LABEL(16, 3, 12, 1, "('1' to '7')")};
static struct ctk_label ipaddrlabel =
  {CTK_LABEL(0, 5, 10, 1, "IP address")};
static struct ctk_textentry ipaddrtextentry =
  {CTK_TEXTENTRY(11, 5, 15, 1, screen.ipaddr, 15)};
#ifdef WITH_ETHERNET
static struct ctk_label netmasklabel =
  {CTK_LABEL(0, 7, 10, 1, "Netmask")};
static struct ctk_textentry netmasktextentry =
  {CTK_TEXTENTRY(11, 7, 15, 1, screen.netmask, 15)};
static struct ctk_label gatewaylabel =
  {CTK_LABEL(0, 9, 10, 1, "Gateway")};
static struct ctk_textentry gatewaytextentry =
  {CTK_TEXTENTRY(11, 9, 15, 1, screen.gateway, 15)};
static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 11, 10, 1, "DNS server")};
static struct ctk_textentry dnsservertextentry =
  {CTK_TEXTENTRY(11, 11, 15, 1, screen.dnsserver, 15)};
#else /* WITH_ETHERNET */
static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 7, 10, 1, "DNS server")};
static struct ctk_textentry dnsservertextentry =
  {CTK_TEXTENTRY(11, 7, 15, 1, screen.dnsserver, 15)};
#endif /* WITH_ETHERNET */

static struct ctk_button okbutton =
  {CTK_BUTTON(0, 13, 2, "Ok")};
static struct ctk_button cancelbutton =
  {CTK_BUTTON(21, 13, 6, "Cancel")};

static DISPATCHER_SIGHANDLER(config_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Configuration", NULL, config_sighandler, NULL)};
static ek_id_t id;


/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(config_init, arg)
{
  arg_free(arg);
    
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    
    ctk_window_new(&window, 29, 14, "Configuration");
    
#ifdef __APPLE2ENH__
    CTK_WIDGET_ADD(&window, &backgroundlabel);
    CTK_WIDGET_ADD(&window, &backgroundtextentry);
    CTK_WIDGET_ADD(&window, &backgrounddescr);
#endif /* __APPLE2ENH__ */
    CTK_WIDGET_ADD(&window, &slotlabel);
    CTK_WIDGET_ADD(&window, &slottextentry);
    CTK_WIDGET_ADD(&window, &slotdescr);
    CTK_WIDGET_ADD(&window, &ipaddrlabel);  
    CTK_WIDGET_ADD(&window, &ipaddrtextentry);
#ifdef WITH_ETHERNET
    CTK_WIDGET_ADD(&window, &netmasklabel);
    CTK_WIDGET_ADD(&window, &netmasktextentry);
    CTK_WIDGET_ADD(&window, &gatewaylabel);
    CTK_WIDGET_ADD(&window, &gatewaytextentry);
#endif /* WITH_ETHERNET */
    CTK_WIDGET_ADD(&window, &dnsserverlabel);
    CTK_WIDGET_ADD(&window, &dnsservertextentry);
    
    CTK_WIDGET_ADD(&window, &okbutton);
    CTK_WIDGET_ADD(&window, &cancelbutton);
    
#ifdef __APPLE2ENH__
    CTK_WIDGET_FOCUS(&window, &backgroundtextentry);
#else /* __APPLE2ENH__ */
    CTK_WIDGET_FOCUS(&window, &slottextentry);
#endif /* __APPLE2ENH__ */

    memcpy(&screen, &config, sizeof(config));
    
    dispatcher_listen(ctk_signal_button_activate);
    dispatcher_listen(ctk_signal_window_close);
  }
  ctk_window_open(&window);
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
void
config_apply(void)
{
  u16_t addr[2];

#ifdef WITH_UIP
  nullterminate(config.ipaddr);
  if(uiplib_ipaddrconv(config.ipaddr, (unsigned char *)addr)) {
    uip_sethostaddr(addr);
  }
  
#ifdef WITH_ETHERNET
  nullterminate(config.netmask);
  if(uiplib_ipaddrconv(config.netmask, (unsigned char *)addr)) {
    uip_setnetmask(addr);
  }

  nullterminate(config.gateway);
  if(uiplib_ipaddrconv(config.gateway, (unsigned char *)addr)) {
    uip_setdraddr(addr);
  }
#endif /* WITH_ETHERNET */
  
  nullterminate(config.dnsserver);
  if(uiplib_ipaddrconv(config.dnsserver, (unsigned char *)addr)) {
    resolv_conf(addr);
  }
#endif /* WITH_UIP */
}
/*-----------------------------------------------------------------------------------*/
static void
config_quit(void)
{
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(config_sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_button_activate) {   
    if(data == (ek_data_t)&okbutton) {
      memcpy(&config, &screen, sizeof(screen));
      config_apply();
      goto quit;
    } else if(data == (ek_data_t)&cancelbutton) {
      goto quit;
    }
  } else if(s == ctk_signal_window_close ||
	    s == dispatcher_signal_quit) {
    goto quit;
  }
  return;

quit:
  ctk_window_close(&window);
  config_quit();
}
/*-----------------------------------------------------------------------------------*/
