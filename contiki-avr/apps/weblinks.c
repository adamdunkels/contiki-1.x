/*
 * Copyright (c) 2003, Adam Dunkels.
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
 * $Id: weblinks.c,v 1.1 2003/07/04 10:54:51 adamdunkels Exp $
 *
 */


#include "ctk.h"
#include "ctk-draw.h"
#include "dispatcher.h"
#include "loader.h"

#include "program-handler.h"

#include "www.h"

#include <string.h>

#define WIDTH 45
#define HEIGHT 16

#define LINK(name, descr, url) \
 {CTK_HYPERLINK(1, 0, sizeof(name) - 1, name, url)}, \
 {CTK_LABEL(1, 20, sizeof(descr) - 1, 1, descr)}
struct link {
  struct ctk_hyperlink hyperlink;
  struct ctk_label description;
};

static struct link links[] =
  {
    {LINK("Contiki",
	  "The Contiki web site",
	  "http://dunkels.com/adam/contiki/index-text.html")},
    {LINK("Contiki AVR",
	  "The server software",
	  "http://dunkels.com/adam/contiki/ports/avr-text.html")},
    {LINK("Ethernut",
	  "The server hardware",
	  "http://www.ethernut.de/en/")},
    {LINK("cc65",
	  "The 6502 C cross compiler",
	  "http://www.cc65.org/")},
    {LINK("Google",
	  "Google",
	  "http://www.google.com/")},
    {LINK("OSNews",
	  "Exploring the future of computing",
	  "http://www.osnews.com/")},
    {LINK("Slashdot",
	  "News for nerds, stuff that matters",
	  "http://slashdot.org/")},
  };

#define NUMLINKS 7

static struct ctk_window window;
static struct ctk_label hintslabel1 =
  {CTK_LABEL(1, 1, 39, 1, "Open the web browser in the background,")};
static struct ctk_label hintslabel2 =
  {CTK_LABEL(1, 2, 24, 1, "then click on the links.")};


static DISPATCHER_SIGHANDLER(weblinks_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Web links", NULL, weblinks_sighandler, NULL)};
static ek_id_t id;

/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(weblinks_init)
{
  unsigned char y, i;
		     
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);

    ctk_window_new(&window, WIDTH, HEIGHT, "Web links");

    CTK_WIDGET_ADD(&window, &hintslabel1);
    CTK_WIDGET_ADD(&window, &hintslabel2);
    
    y = 4;
    for(i = 0; i < NUMLINKS; ++i) {
      
      CTK_WIDGET_SET_XPOS(&(links[i].hyperlink), 1);
      CTK_WIDGET_SET_YPOS(&(links[i].hyperlink), y);
      CTK_WIDGET_ADD(&window, &(links[i].hyperlink));   
      
      CTK_WIDGET_SET_XPOS(&(links[i].description),
			  strlen(links[i].hyperlink.text) + 2);
      CTK_WIDGET_SET_YPOS(&(links[i].description), y);
      CTK_WIDGET_ADD(&window, &(links[i].description));
      
      ++y;
      
    }

    dispatcher_listen(ctk_signal_window_close);
  }
  ctk_window_open(&window);
}
/*-----------------------------------------------------------------------------------*/
static void
quit(void)
{
  ctk_window_close(&window);
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(weblinks_sighandler, s, data)
{
  unsigned char i;
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_window_close &&
	    data == (ek_data_t)&window) {
    quit();
  }
}
/*-----------------------------------------------------------------------------------*/
