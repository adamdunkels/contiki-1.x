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
 * $Id: programs.c,v 1.1 2003/03/19 14:13:33 adamdunkels Exp $
 *
 */

#include "ctk.h"
#include "ek.h"
#include "dispatcher.h"

#include "webserver.h"
#include "www.h"
#include "email.h"
#include "simpletelnet.h"


static struct ctk_menu menu;

static unsigned char menuitem_www,
  menuitem_email, menuitem_telnet, menuitem_webserver;

#if WITH_WWW
/* The icon for the WWW browser */
static unsigned char wwwicon_bitmap[3*3*8] = {
  0x00, 0x7e, 0x40, 0x73, 0x46, 0x4c, 0x18, 0x13,
  0x00, 0x00, 0xff, 0x81, 0x34, 0xc9, 0x00, 0xb6,
  0x00, 0x7e, 0x02, 0xce, 0x72, 0x32, 0x18, 0x48,

  0x30, 0x27, 0x24, 0x20, 0x37, 0x24, 0x20, 0x33,
  0x00, 0x7b, 0x42, 0x00, 0x7b, 0x42, 0x00, 0x3b,
  0x0c, 0x24, 0x24, 0x04, 0xa4, 0x24, 0x04, 0x4c,

  0x12, 0x19, 0x4c, 0x46, 0x63, 0x40, 0x7c, 0x00,
  0x22, 0x91, 0x00, 0xc4, 0x81, 0xff, 0x00, 0x00,
  0x08, 0x18, 0x32, 0x62, 0xc6, 0x02, 0x3e, 0x00
};

static char wwwicon_textmap[9] = {
  'w', 'w', 'w',
  '(', ')', ' ',
  ' ', '(', ')'
};

static struct ctk_icon wwwicon =
  {CTK_ICON("Web browser", wwwicon_bitmap, wwwicon_textmap)};
#endif /* WITH_WWW */

#if WITH_WEBSERVER
/* The icon for the web server */
static unsigned char webservericon_bitmap[3*3*8] = {
  0x00, 0x7f, 0x40, 0x41, 0x44, 0x48, 0x40, 0x50,
  0x00, 0xff, 0x5a, 0x00, 0x00, 0x00, 0x3c, 0x81,
  0x00, 0xfe, 0x02, 0x82, 0x22, 0x12, 0x02, 0x0a,

  0x41, 0x60, 0x42, 0x62, 0x62, 0x42, 0x60, 0x41,
  0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18,
  0x82, 0x06, 0x42, 0x46, 0x46, 0x42, 0x06, 0x82,

  0x50, 0x40, 0x48, 0x44, 0x41, 0x40, 0x7e, 0x00,
  0xc5, 0x34, 0x3c, 0x52, 0x7a, 0x7e, 0xa1, 0xfd,
  0x0a, 0x02, 0x12, 0x22, 0x82, 0x02, 0x7e, 0x00
};

static char webservericon_textmap[9] = {
  '+', '-', '+',
  '|', ')', '|',
  '+', '-', '+'
};

static struct ctk_icon webservericon =
  {CTK_ICON("Web server", webservericon_bitmap, webservericon_textmap)};
#endif /* WITH_WEBSERVER */


#if WITH_TELNET
/* The icon for the telnet client */
static unsigned char telneticon_bitmap[3*3*8] = {
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

static char telneticon_textmap[9] = {
  't', 'e', 'l',
  'n', 'e', 't',
  '-', '-', '-'
};

static struct ctk_icon telneticon =
  {CTK_ICON("Telnet client", telneticon_bitmap, telneticon_textmap)};
#endif /* WITH_TELNET */


static void sighandler(ek_signal_t s, ek_data_t data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Program handler", NULL, sighandler, NULL)};
static ek_id_t id;

/*-----------------------------------------------------------------------------------*/
void
programs_init(void)     
{
  id = dispatcher_start(&p);
  
  ctk_menu_new(&menu, "Programs");
  ctk_menu_add(&menu);
#ifdef WITH_WWW
  menuitem_www = ctk_menuitem_add(&menu, "Web browser");
#endif /* WITH_WWW */
#ifdef WITH_EMAIL
  menuitem_email = ctk_menuitem_add(&menu, "E-mail");
#endif /* WITH_EMAIL */
#ifdef WITH_WEBSERVER
  menuitem_webserver = ctk_menuitem_add(&menu, "Web server");
#endif /* WITH_WEBSERVER */
#ifdef WITH_TELNET
  menuitem_telnet = ctk_menuitem_add(&menu, "Telnet client");
#endif /* WITH_TELNET */

#ifdef WITH_WWW  
  CTK_ICON_ADD(&wwwicon, id);
#endif /* WITH_WWW */
#ifdef WITH_WEBSERVER
  CTK_ICON_ADD(&webservericon, id);
#endif /* WITH_WEBSERVER */  
#ifdef WITH_TELNET
  CTK_ICON_ADD(&telneticon, id);
#endif /* WITH_TELNET */  
  
  dispatcher_listen(ctk_signal_menu_activate);
  dispatcher_listen(ctk_signal_button_activate);
}
/*-----------------------------------------------------------------------------------*/
static void
sighandler(ek_signal_t s, ek_data_t data)
{
  if(s == ctk_signal_menu_activate) {
    if((struct ctk_menu *)data == &menu) {
      if(menu.active == menuitem_www) {
#ifdef WITH_WWW
	www_init();
	ctk_redraw();
#endif /* WITH_WWW */
#ifdef WITH_EMAIL
      } else if(menu.active == menuitem_email) {
	email_init();
	ctk_redraw();
#endif /* WITH_EMAIL */
#ifdef WITH_TELNET
      } else if(menu.active == menuitem_telnet) {
	simpletelnet_init();
	ctk_redraw();
#endif /* WITH_TELNET */
#ifdef WITH_WEBSERVER
      } else if(menu.active == menuitem_webserver) {
	webserver_init();
	ctk_redraw();
#endif /* WITH_WEBSERVER */
      }      
    }
  } else if(s == ctk_signal_button_activate) {
#ifdef WITH_WWW    
    if((struct ctk_widget *)data == (struct ctk_widget *)&wwwicon) {
      www_init();
      ctk_redraw();
    } 
#endif /* WITH_WWW */
#ifdef WITH_WEBSERVER      
    if((struct ctk_widget *)data ==
       (struct ctk_widget *)&webservericon) {
      webserver_init();
      ctk_redraw();
    } 
#endif /* WITH_WEBSERVER */
#ifdef WITH_TELNET
    if((struct ctk_widget *)data ==
       (struct ctk_widget *)&telneticon) {
      simpletelnet_init();
      ctk_redraw();
    } 
#endif /* WITH_TELNET */
  }
  
}
/*-----------------------------------------------------------------------------------*/
