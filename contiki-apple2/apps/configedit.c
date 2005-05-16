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
 * $Id: configedit.c,v 1.8 2005/05/16 12:56:14 oliverschmidt Exp $
 *
 */


#include <string.h>

#include "uiplib.h"
#include "resolv.h"
#include "uip_arp.h"
#include "ctk.h"

#include "program-handler.h"
#include "packet-service.h"
#include "kfs.h"
#include "cfs.h"

#include "ctk-textentry-checkbox.h"

#include "config.h"


static config_t config;
static struct ctk_window window;

#ifdef __APPLE2ENH__

static struct ctk_label backgroundlabel =
  {CTK_LABEL(0, 1, 11, 1, "Background")};
static char bkgnd[2];
static struct ctk_textentry backgroundtextentry =
  {CTK_TEXTENTRY_INPUT(12, 1, 1, 1, bkgnd, 1, ctk_textentry_checkbox_input)};
static struct ctk_label backgrounddescr =
  {CTK_LABEL(16, 1, 12, 1, "('x' or ' ')")};

#endif /* __APPLE2ENH__ */

static struct ctk_label screensaverlabel =
  {CTK_LABEL(0, 3, 11, 1, "Screensaver")};
static char screensaver[16];
static struct ctk_textentry screensavertextentry =
  {CTK_TEXTENTRY(12, 3, 15, 1, screensaver, 15)};

static struct ctk_label timeoutlabel =
  {CTK_LABEL(0, 4, 11, 1, "Timeout")};
static char timeout[2];
static struct ctk_textentry timeouttextentry =
  {CTK_TEXTENTRY(12, 4, 1, 1, timeout, 1)};
static struct ctk_label timeoutdescr =
  {CTK_LABEL(16, 4, 12, 1, "(in Minutes)")};

static struct ctk_label driverlabel =
  {CTK_LABEL(0, 6, 11, 1, "LAN driver")};
static char driver[16];
static struct ctk_textentry drivertextentry =
  {CTK_TEXTENTRY(12, 6, 15, 1, driver, 15)};

static struct ctk_label slotlabel =
  {CTK_LABEL(0, 7, 11, 1, "LAN slot")};
static char slot[2];
static struct ctk_textentry slottextentry =
  {CTK_TEXTENTRY(12, 7, 1, 1, slot, 1)};
static struct ctk_label slotdescr =
  {CTK_LABEL(16, 7, 12, 1, "('1' to '7')")};

static struct ctk_label ipaddrlabel =
  {CTK_LABEL(0, 9, 10, 1, "IP address")};
static char ipaddr[16];
static struct ctk_textentry ipaddrtextentry =
  {CTK_TEXTENTRY(12, 9, 15, 1, ipaddr, 15)};

#ifdef WITH_ETHERNET

static struct ctk_label netmasklabel =
  {CTK_LABEL(0, 10, 11, 1, "Netmask")};
static char netmask[16];
static struct ctk_textentry netmasktextentry =
  {CTK_TEXTENTRY(12, 10, 15, 1, netmask, 15)};

static struct ctk_label gatewaylabel =
  {CTK_LABEL(0, 11, 11, 1, "Gateway")};
static char gateway[16];
static struct ctk_textentry gatewaytextentry =
  {CTK_TEXTENTRY(12, 11, 15, 1, gateway, 15)};

static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 12, 11, 1, "DNS server")};
static char dnsserver[16];
static struct ctk_textentry dnsservertextentry =
  {CTK_TEXTENTRY(12, 12, 15, 1, dnsserver, 15)};

static struct ctk_label maclsblabel =
  {CTK_LABEL(0, 14, 11, 1, "MAC address")};
static char maclsb[4];
static struct ctk_textentry maclsbtextentry =
  {CTK_TEXTENTRY(12, 14, 3, 1, maclsb, 3)};
static struct ctk_label maclsbdescr =
  {CTK_LABEL(18, 14, 10, 1, "(LSB only)")};

#else /* WITH_ETHERNET */

static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 10, 11, 1, "DNS server")};
static char dnsserver[16];
static struct ctk_textentry dnsservertextentry =
  {CTK_TEXTENTRY(12, 10, 15, 1, dnsserver, 15)};

#endif /* WITH_ETHERNET */

static struct ctk_button okbutton =
  {CTK_BUTTON(0, 16, 12, "Save & close")};
static struct ctk_button cancelbutton =
  {CTK_BUTTON(22, 16, 6, "Cancel")};

EK_EVENTHANDLER(config_eventhandler, ev, data);
EK_PROCESS(p, "Configuration", EK_PRIO_NORMAL,
	   config_eventhandler, NULL, NULL);
static ek_id_t id = EK_ID_NONE;
static ek_id_t driverid = EK_ID_NONE;

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

  strncpy(screensaver, program_handler_getscreensaver(), sizeof(screensaver));

  *timeout = (CTK_SCREENSAVER_TIMEOUT() / 60) % 10 + '0';

  for(p = EK_PROCS(); p != NULL; p = p->next) {
    if(makedriver(p->name, driver)) {
      driverid = p->id;
      break;
    }
  }

  *slot = config_getlanslot() + '0';

#ifdef WITH_UIP

  uip_gethostaddr(addr);
  makeaddr(addr, ipaddr);
  
#ifdef WITH_ETHERNET

  uip_getnetmask(addr);
  makeaddr(addr, netmask);
  
  uip_getdraddr(addr);
  makeaddr(addr, gateway);

  makebyte(uip_ethaddr.addr[5], maclsb);

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
  char *cptr;
  u8_t tmp;

#ifdef __APPLE2ENH__

  config.bkgnd = (*bkgnd == 'x');

#endif /* __APPLE2ENH__ */

  nullterminate(screensaver);
  strcpy(config.screensaver, screensaver);

  if(*timeout >= '1' && *timeout <= '9') {
    config.timeout = *timeout - '0';
  }

  nullterminate(driver);
  strcpy(config.driver, driver);

  if(*slot >= '1' && *slot <= '7') {
    config.slot = *slot - '0';
  }

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

  *(char *)0x02FF = 0;
  tmp = 0;
  for(cptr = maclsb; *cptr >= '0' && *cptr <= '9'; ++cptr) {
    tmp = (tmp * 10) + (*cptr - '0');
  }
  if(tmp != 0) {
    config.maclsb = tmp;
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

  program_handler_setscreensaver(config.screensaver);

  CTK_SCREENSAVER_SET_TIMEOUT(config.timeout * 60);

  if(driverid != EK_ID_NONE) {
    ek_post(driverid, EK_EVENT_REQUEST_EXIT, NULL);
  }

  config_setlanslot(config.slot);

  if(*config.driver) {
    program_handler_load(config.driver, NULL);
  }

#ifdef WITH_UIP

  uip_sethostaddr(config.ipaddr);

#ifdef WITH_ETHERNET

  uip_setnetmask(config.netmask);
  uip_setdraddr(config.gateway);
  uip_ethaddr.addr[5] = config.maclsb;

#endif /* WITH_ETHERNET */

  resolv_conf(config.dnsserver);

#endif /* WITH_UIP */
}
/*-----------------------------------------------------------------------------------*/
static void
config_save(void)
{
  int fd;

  fd = cfs_open(strcat(kfs_getdir(), "contiki.cfg"), CFS_WRITE);
  if(fd == -1) {
    return;
  }
  cfs_write(fd, &config, sizeof(config));
  cfs_close(fd);
}
/*-----------------------------------------------------------------------------------*/
EK_EVENTHANDLER(config_eventhandler, ev, data)
{
  EK_EVENTHANDLER_ARGS(ev, data);
  
  if(ev == EK_EVENT_INIT) {
    ctk_window_new(&window, 30, 17, "Config editor");
#ifdef __APPLE2ENH__
    CTK_WIDGET_ADD(&window, &backgroundlabel);
    CTK_WIDGET_ADD(&window, &backgroundtextentry);
    CTK_WIDGET_ADD(&window, &backgrounddescr);
#endif /* __APPLE2ENH__ */
    CTK_WIDGET_ADD(&window, &screensaverlabel);
    CTK_WIDGET_ADD(&window, &screensavertextentry);
    CTK_WIDGET_ADD(&window, &timeoutlabel);
    CTK_WIDGET_ADD(&window, &timeouttextentry);
    CTK_WIDGET_ADD(&window, &timeoutdescr);
    CTK_WIDGET_ADD(&window, &driverlabel);
    CTK_WIDGET_ADD(&window, &drivertextentry);
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
#ifdef WITH_ETHERNET
    CTK_WIDGET_ADD(&window, &maclsblabel);
    CTK_WIDGET_ADD(&window, &maclsbtextentry);
    CTK_WIDGET_ADD(&window, &maclsbdescr);
#endif /* WITH_ETHERNET */
    CTK_WIDGET_ADD(&window, &okbutton);
    CTK_WIDGET_ADD(&window, &cancelbutton);
#ifdef __APPLE2ENH__
    CTK_WIDGET_FOCUS(&window, &backgroundtextentry);
#else /* __APPLE2ENH__ */
    CTK_WIDGET_FOCUS(&window, &screensavertextentry);
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
