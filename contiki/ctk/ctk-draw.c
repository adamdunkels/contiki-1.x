/*
 * Copyright (c) 2004, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the Contiki operating system.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: ctk-draw.c,v 1.4 2004/09/12 20:24:55 adamdunkels Exp $
 */

#include "ek-service.h"

#include "ctk-draw.h"
#include "ctk.h"

#include "ctk-draw-service.h"


unsigned char ctk_draw_windowborder_width = 1,
  ctk_draw_windowborder_height = 1,
  ctk_draw_windowtitle_height = 1;

EK_SERVICE(service, CTK_DRAW_SERVICE_NAME);

/*---------------------------------------------------------------------------*/
static struct ctk_draw_service_interface *
find_interface(void)
{
  struct ctk_draw_service_interface *interface;
  interface = (struct ctk_draw_service_interface *)ek_service_state(&service);
  if(interface != NULL &&
     interface->version == CTK_DRAW_SERVICE_VERSION) {
    return interface;
  } else {
    return NULL;
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_init(void)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    interface->draw_init();
    ctk_draw_windowborder_width = interface->windowborder_width;
    ctk_draw_windowborder_height = interface->windowborder_height;
    ctk_draw_windowtitle_height = interface->windowtitle_height;
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_clear(unsigned char clipy1, unsigned char clipy2)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    interface->draw_clear(clipy1, clipy2);
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_clear_window(struct ctk_window *window,
		      unsigned char focus,
		      unsigned char clipy1,
		      unsigned char clipy2)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    interface->draw_clear_window(window, focus, clipy1, clipy2);
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_window(struct ctk_window *window,
		unsigned char focus,
		unsigned char clipy1,
		unsigned char clipy2)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    interface->draw_window(window, focus, clipy1, clipy2);
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_dialog(struct ctk_window *dialog)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    interface->draw_dialog(dialog);
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_widget(struct ctk_widget *widget,
		unsigned char focus,
		unsigned char clipy1,
		unsigned char clipy2)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    interface->draw_widget(widget, focus, clipy1, clipy2);
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_menus(struct ctk_menus *menus)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    interface->draw_menus(menus);
  }
}
/*---------------------------------------------------------------------------*/
unsigned char
ctk_draw_width(void)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    return interface->width();
  }
  return 40;
}
/*---------------------------------------------------------------------------*/
unsigned char
ctk_draw_height(void)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    return interface->height();
  }
  return 24;
}
/*---------------------------------------------------------------------------*/
unsigned short
ctk_mouse_xtoc(unsigned short x)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    return interface->mouse_xtoc(x);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
unsigned short
ctk_mouse_ytoc(unsigned short y)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    return interface->mouse_ytoc(y);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_quit(void)
{
  ek_post(service.id, EK_EVENT_REQUEST_EXIT, NULL);
  ek_service_reset(&service);  
}
/*---------------------------------------------------------------------------*/
