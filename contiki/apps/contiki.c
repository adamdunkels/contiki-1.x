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
 * $Id: contiki.c,v 1.2 2003/03/28 12:07:33 adamdunkels Exp $
 *
 */

#include "uip_main.h"
#include "uip.h"
#include "uip_arp.h"
#include "ctk.h"
#include "dispatcher.h"
#include "resolv.h"
#include "email.h"

#include "petsciiconv.h"

static struct ctk_menu menu;
unsigned char menuitem_about, menuitem_processes, menuitem_tcpip;

#define MAX_PROCESSLABELS 10
static struct ctk_window processwindow;
static unsigned char ids[MAX_PROCESSLABELS][3];
static struct ctk_label processidlabels[MAX_PROCESSLABELS];
static struct ctk_label processnamelabels[MAX_PROCESSLABELS];

static struct ctk_button processupdatebutton =
  {CTK_BUTTON(0, 9, 6, "Update")};
static struct ctk_button processclosebutton =
  {CTK_BUTTON(13, 9, 5, "Close")};

static struct ctk_window aboutdialog;
static struct ctk_label aboutlabel1 =
  {CTK_LABEL(5, 0, 23, 1, "The Contiki Desktop OS")};
static struct ctk_label aboutlabel2 =
  {CTK_LABEL(3, 3, 28, 1, "A modern, Internet-enabled")};
static struct ctk_label aboutlabel3 =
  {CTK_LABEL(2, 4, 28, 1, "operating system and desktop")};
static struct ctk_label aboutlabel4 =
  {CTK_LABEL(0, 5, 32, 1, "environment for the Commodore 64")};
static struct ctk_label aboutlabel5 =
  {CTK_LABEL(4, 6, 26, 1, "written by Adam Dunkels")};

static char abouturl_petscii[] = "http://dunkels.com/adam/contiki/";
static char abouturl_ascii[40];
static struct ctk_hyperlink abouturl = 
  {CTK_HYPERLINK(0, 8, 32, "http://dunkels.com/adam/contiki/",
		 abouturl_ascii)};
static struct ctk_button aboutclose =
  {CTK_BUTTON(12, 10, 5, "Close")};

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

/* . == 0, + == 1 */
static unsigned char abouticon_bitmap[3*3*8] = {
  0x00, 0x7f, 0x43, 0x4c, 0x58, 0x53, 0x60, 0x6f,
  0x00, 0xff, 0x00, 0x7e, 0x00, 0xff, 0x00, 0xff,
  0x00, 0xfe, 0xc2, 0x32, 0x1a, 0xca, 0x06, 0xf6,

  0x40, 0x5f, 0x40, 0x5f, 0x40, 0x5f, 0x40, 0x4f,
  0x00, 0xff, 0x00, 0xff, 0x00, 0xfc, 0x01, 0xf3,
  0x02, 0xfa, 0x02, 0x82, 0x3e, 0xfe, 0xfe, 0xfe,

  0x60, 0x67, 0x50, 0x59, 0x4c, 0x43, 0x7f, 0x00,
  0x07, 0xe7, 0x0f, 0xef, 0x0f, 0x0f, 0xff, 0x00,
  0x8e, 0x06, 0x06, 0x06, 0x8e, 0xfe, 0xfe, 0x00
};

static char abouticon_textmap[9] = {
  ' ', ' ', 'c',
  ' ', '?', ' ',
  '.', ' ', ' '
};

static struct ctk_icon abouticon =
  {CTK_ICON("About Contiki", abouticon_bitmap, abouticon_textmap)};

/* . == 0, + == 1 */
static unsigned char tcpipconficon_bitmap[3*3*8] = {
  0x00, 0x79, 0x43, 0x73, 0x47, 0x77, 0x47, 0x6f,
  0x00, 0xfe, 0xfe, 0xfc, 0xfc, 0xfc, 0xf8, 0xfb,
  0x00, 0x16, 0x02, 0x00, 0x02, 0x00, 0x00, 0xc2,

  0x48, 0x4c, 0x5f, 0x5f, 0x1f, 0x3f, 0x3f, 0x03,
  0x79, 0xf0, 0xf0, 0xf0, 0xe0, 0xe0, 0xfe, 0xfc,
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  0x77, 0x47, 0x70, 0x43, 0x79, 0x41, 0x7c, 0x00,
  0xfc, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xf7, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x84, 0xf0, 0x00
};

static char tcpipconficon_textmap[9] = {
  't', 'c', 'p',
  '/', 'i', 'p',
  'c', 'f', 'g'
};


static struct ctk_icon tcpipconficon =
  {CTK_ICON("TCP/IP conf", tcpipconficon_bitmap, tcpipconficon_textmap)};

static void sighandler(ek_signal_t s, ek_data_t data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Contiki", NULL, sighandler, NULL)};
static ek_id_t id;

/*-----------------------------------------------------------------------------------*/
static void
update_processwindow(void)
{
  unsigned char i, *idsptr;
  struct dispatcher_proc *p;
  
  for(i = 0; i < MAX_PROCESSLABELS; ++i) {
    p = dispatcher_process(i);
    if(p != NULL) {
      idsptr = ids[i];
      idsptr[0] = '0' + i / 10;
      idsptr[1] = '0' + i % 10;
      idsptr[2] = 0;
      CTK_LABEL_NEW(&processidlabels[i],
		    0, i + 1, 2, 1, idsptr);
      CTK_WIDGET_ADD(&processwindow, &processidlabels[i]);
      
      CTK_LABEL_NEW(&processnamelabels[i],
		    3, i + 1, 17, 1, p->name);
      CTK_WIDGET_ADD(&processwindow, &processnamelabels[i]);
    }
  }

  CTK_WIDGET_ADD(&processwindow, &processupdatebutton);
  CTK_WIDGET_ADD(&processwindow, &processclosebutton);
  CTK_WIDGET_FOCUS(&processwindow, &processupdatebutton);
}
/*-----------------------------------------------------------------------------------*/
static void
make_processwindow(void)
{  
  ctk_window_new(&processwindow, 20, 10, "Processes");
  ctk_window_move(&processwindow, 0, 1);
  update_processwindow();
}
/*-----------------------------------------------------------------------------------*/
void
contiki_init(void)     
{
  id = dispatcher_start(&p);
  
  ctk_dialog_new(&aboutdialog, 32, 11);
  CTK_WIDGET_ADD(&aboutdialog, &aboutlabel1);
  CTK_WIDGET_ADD(&aboutdialog, &aboutlabel2);
  CTK_WIDGET_ADD(&aboutdialog, &aboutlabel3);
  CTK_WIDGET_ADD(&aboutdialog, &aboutlabel4);
  CTK_WIDGET_ADD(&aboutdialog, &aboutlabel5);
  CTK_WIDGET_ADD(&aboutdialog, &abouturl);
  CTK_WIDGET_ADD(&aboutdialog, &aboutclose);
  CTK_WIDGET_FOCUS(&aboutdialog, &aboutclose);
  
  ctk_menu_new(&menu, "Contiki");
  menuitem_about = ctk_menuitem_add(&menu, "About");
  menuitem_processes = ctk_menuitem_add(&menu, "Processes");

  menuitem_tcpip = ctk_menuitem_add(&menu, "TCP/IP conf");
#if 0
#ifdef WITH_ETHERNET
  menuitem_ethernet = ctk_menuitem_add(&menu, "Ethernet");
#endif /* WITH_ETHERNET */
#ifdef WITH_RS232
  menuitem_serial = ctk_menuitem_add(&menu, "Serial link");
#endif /* WITH_RS232 */
#endif /* 0 */

  strcpy(abouturl_ascii, abouturl_petscii);
  petsciiconv_toascii(abouturl_ascii, sizeof(abouturl_ascii));
  
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

  ctk_menu_add(&menu);

  CTK_ICON_ADD(&abouticon, id);
  CTK_ICON_ADD(&tcpipconficon, id);

  dispatcher_listen(ctk_signal_button_activate);
  dispatcher_listen(ctk_signal_menu_activate);
  dispatcher_listen(ctk_signal_hyperlink_activate);
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
sighandler(ek_signal_t s, ek_data_t data)
{
  struct ctk_widget *w;

  if(s == ctk_signal_button_activate) {
    w = (struct ctk_widget *)data;
    if(w == (struct ctk_widget *)&aboutclose) {
      ctk_dialog_close();
      ctk_redraw();
    } else if(w == (struct ctk_widget *)&abouticon) {
      ctk_dialog_open(&aboutdialog);
      ctk_redraw();
    } else if(w == (struct ctk_widget *)&tcpipclosebutton) {
      apply_tcpipconfig();
      ctk_window_close(&tcpipwindow);
      ctk_redraw();
    } else if(w == (struct ctk_widget *)&tcpipconficon) {
      ctk_window_open(&tcpipwindow);
      ctk_redraw();
    } else if(w == (struct ctk_widget *)&processupdatebutton) {
      ctk_window_clear(&processwindow);
      update_processwindow();
      ctk_window_redraw(&processwindow);
    } else if(w == (struct ctk_widget *)&processclosebutton) {
      ctk_window_close(&processwindow);
      ctk_redraw();
    }

  } else if(s == ctk_signal_menu_activate) {
    if((struct ctk_menu *)data == &menu) {
      if(menu.active == menuitem_about) {
	ctk_dialog_open(&aboutdialog);
      } else if(menu.active == menuitem_processes) {
	make_processwindow();
	ctk_window_open(&processwindow);
      } else if(menu.active == menuitem_tcpip) {
	ctk_window_open(&tcpipwindow);
      }
      ctk_redraw();
    }
  } else if(s == ctk_signal_hyperlink_activate) {
    if((struct ctk_widget *)data == (struct ctk_widget *)&abouturl) {
      ctk_dialog_close();
      ctk_redraw();
    }
  }
}
/*-----------------------------------------------------------------------------------*/
