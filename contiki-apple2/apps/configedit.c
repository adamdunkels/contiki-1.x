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
 * $Id: configedit.c,v 1.2 2005/01/26 21:33:29 oliverschmidt Exp $
 *
 */


#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "uiplib.h"
#include "resolv.h"
#include "ctk.h"

#include "program-handler.h"
#include "packet-service.h"

#include "config.h"


static config_t config;
static struct ctk_window window;

#ifdef __APPLE2ENH__

static struct ctk_label backgroundlabel =
  {CTK_LABEL(0, 1, 10, 1, "Background")};
static char bkgnd[2];
static struct ctk_textentry backgroundtextentry =
  {CTK_TEXTENTRY(11, 1, 1, 1, bkgnd, 1)};
static struct ctk_label backgrounddescr =
  {CTK_LABEL(16, 1, 12, 1, "('x' or ' ')")};

#endif /* __APPLE2ENH__ */

static struct ctk_label slotlabel =
  {CTK_LABEL(0, 3, 10, 1, "LAN slot")};
static char slot[2];
static struct ctk_textentry slottextentry =
  {CTK_TEXTENTRY(11, 3, 1, 1, slot, 1)};
static struct ctk_label slotdescr =
  {CTK_LABEL(16, 3, 12, 1, "('1' to '7')")};

static struct ctk_label driverlabel =
  {CTK_LABEL(0, 5, 10, 1, "LAN driver")};
static char driver[16];
static struct ctk_textentry drivertextentry =
  {CTK_TEXTENTRY(11, 5, 15, 1, driver, 15)};

static struct ctk_label ipaddrlabel =
  {CTK_LABEL(0, 7, 10, 1, "IP address")};
static char ipaddr[16];
static struct ctk_textentry ipaddrtextentry =
  {CTK_TEXTENTRY(11, 7, 15, 1, ipaddr, 15)};

#ifdef WITH_ETHERNET

static struct ctk_label netmasklabel =
  {CTK_LABEL(0, 9, 10, 1, "Netmask")};
static char netmask[16];
static struct ctk_textentry netmasktextentry =
  {CTK_TEXTENTRY(11, 9, 15, 1, netmask, 15)};

static struct ctk_label gatewaylabel =
  {CTK_LABEL(0, 11, 10, 1, "Gateway")};
static char gateway[16];
static struct ctk_textentry gatewaytextentry =
  {CTK_TEXTENTRY(11, 11, 15, 1, gateway, 15)};

static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 13, 10, 1, "DNS server")};
static char dnsserver[16];
static struct ctk_textentry dnsservertextentry =
  {CTK_TEXTENTRY(11, 13, 15, 1, dnsserver, 15)};

#else /* WITH_ETHERNET */

static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 9, 10, 1, "DNS server")};
static char dnsserver[16];
static struct ctk_textentry dnsservertextentry =
  {CTK_TEXTENTRY(11, 9, 15, 1, dnsserver, 15)};

#endif /* WITH_ETHERNET */

static struct ctk_button okbutton =
  {CTK_BUTTON(0, 15, 12, "Save & close")};
static struct ctk_button cancelbutton =
  {CTK_BUTTON(21, 15, 6, "Cancel")};

EK_EVENTHANDLER(config_eventhandler, ev, data);
EK_PROCESS(p, "Configuration", EK_PRIO_NORMAL,
	   config_eventhandler, NULL, NULL);
static ek_id_t id = EK_ID_NONE;

/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(config_init, arg)
{
  arg_free(arg);
    
  if(id == EK_ID_NONE) {
    id = ek_start(&p);
  }
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
static int
makedriver(const char *name, char *str)
{
  char *pattern = PACKET_SERVICE_NAME ": ";

  while(*pattern) {
    if(*name++ != *pattern++) {
      return 0;
    }
  }

  while(*name) {
    *str++ = *name++;
  }
  strcpy(str, ".drv");

  return 1;
}
/*-----------------------------------------------------------------------------------*/
static void
makestrings(void)
{
  u16_t addr[2], *addrptr;
  struct ek_proc *p;

#ifdef __APPLE2ENH__

  if(ctk_draw_getbackground()) {
    *bkgnd = 'x';
  }

#endif /* __APPLE2ENH__ */

  *slot = config_getlanslot() + '0';

  for(p = EK_PROCS(); p != NULL; p = p->next) {
    if(makedriver(p->name, driver)) {
      break;
    }
  }

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
makeconfig(void)
{
  u16_t addr[2];

#ifdef __APPLE2ENH__

  if(*bkgnd == 'x' || *bkgnd == 'X') {
    config.bkgnd = 1;
  }

#endif /* __APPLE2ENH__ */

  if(*slot >= '1' && *slot <= '7') {
    config.slot = *slot - '0';
  }

  nullterminate(driver);
  strcpy(config.driver, driver);

#ifdef WITH_UIP

  nullterminate(ipaddr);
  if(uiplib_ipaddrconv(ipaddr, (unsigned char *)addr)) {
    config.ipaddr[0] = addr[0];
    config.ipaddr[1] = addr[1];
  }
  
#ifdef WITH_ETHERNET

  nullterminate(netmask);
  if(uiplib_ipaddrconv(netmask, (unsigned char *)addr)) {
    config.netmask[0] = addr[0];
    config.netmask[1] = addr[1];
  }

  nullterminate(gateway);
  if(uiplib_ipaddrconv(gateway, (unsigned char *)addr)) {
    config.gateway[0] = addr[0];
    config.gateway[1] = addr[1];
  }

#endif /* WITH_ETHERNET */
  
  nullterminate(dnsserver);
  if(uiplib_ipaddrconv(dnsserver, (unsigned char *)addr)) {
    config.dnsserver[0] = addr[0];
    config.dnsserver[1] = addr[1];
  }

#endif /* WITH_UIP */
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
static void
config_save(void)
{
  int fd;

  fd = open("contiki.cfg", 0);
  if(fd == -1) {
    return;
  }
  write(fd, &config, sizeof(config));
  close(fd);
}
/*-----------------------------------------------------------------------------------*/
EK_EVENTHANDLER(config_eventhandler, ev, data)
{
  EK_EVENTHANDLER_ARGS(ev, data);
  
  if(ev == EK_EVENT_INIT) {
    ctk_window_new(&window, 29, 16, "Config editor");
#ifdef __APPLE2ENH__
    CTK_WIDGET_ADD(&window, &backgroundlabel);
    CTK_WIDGET_ADD(&window, &backgroundtextentry);
    CTK_WIDGET_ADD(&window, &backgrounddescr);
#endif /* __APPLE2ENH__ */
    CTK_WIDGET_ADD(&window, &slotlabel);
    CTK_WIDGET_ADD(&window, &slottextentry);
    CTK_WIDGET_ADD(&window, &slotdescr);
    CTK_WIDGET_ADD(&window, &driverlabel);
    CTK_WIDGET_ADD(&window, &drivertextentry);
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

    /* Fill the configuration strings with values from the current
       configuration */
    makestrings();
    ctk_window_open(&window);
  } else if(ev == ctk_signal_button_activate) {   
    if(data == (ek_data_t)&okbutton) {

      /* Fill the configuration with values from the current
         configuration strings */
      makeconfig();
      config_apply();
      config_save();
      goto quit;
    } else if(data == (ek_data_t)&cancelbutton) {
      goto quit;
    }

  } else if(ev == ctk_signal_window_close ||
	    ev == EK_EVENT_REQUEST_EXIT) {
    goto quit;
  }
  return;

quit:
  ctk_window_close(&window);
  ek_exit();
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
