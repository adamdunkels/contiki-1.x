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
 * This file is part of the Contiki desktop environment for the C64.
 *
 * $Id: webserver.c,v 1.5 2003/04/10 09:04:50 adamdunkels Exp $
 *
 */


#include "ctk.h"
#include "dispatcher.h"
#include "http-strings.h"
#include "uip_main.h"
#include "petsciiconv.h"

#include "loader.h"

#ifdef __C64__
#include <c64.h>
#include <cbm.h>
#endif /* __C64__ */


/* The main window. */
static struct ctk_window mainwindow;

static struct ctk_label message =
  {CTK_LABEL(0, 0, 14, 1, "Web server is ")};
static struct ctk_label onoffmessage =
  {CTK_LABEL(14, 0, 3, 1, "Off")};


static struct ctk_label tracklabel =
  {CTK_LABEL(1, 2, 5, 1, "Track")};
static struct ctk_label sectorlabel =
  {CTK_LABEL(1, 3, 6, 1, "Sector")};

static char tracknum[3], sectornum[3];

static struct ctk_label tracknumlabel =
  {CTK_LABEL(14, 2, 2, 1, tracknum)};
static struct ctk_label sectornumlabel =
  {CTK_LABEL(14, 3, 2, 1, sectornum)};

static struct ctk_button onbutton =
  {CTK_BUTTON(0, 5, 2, "On")};
static struct ctk_label statuslabel =
  {CTK_LABEL(4, 5, 8, 1, "")};
static struct ctk_button offbutton =
  {CTK_BUTTON(12, 5, 3, "Off")};

static unsigned char onoff;
#define ON  1
#define OFF 0

static DISPATCHER_SIGHANDLER(webserver_sighandler, s, data);
static DISPATCHER_UIPCALL(webserver_uipappcall, state);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Web server", NULL, webserver_sighandler,
		   webserver_uipappcall)};
static ek_id_t id;


struct drv_state {
  u8_t state;
#define STATE_NONE              0
#define STATE_RECEIVING_REQUEST 1  
#define STATE_SENDING_HEADERS   2
#define STATE_SENDING_DATA      3
#define STATE_DONE              4

  /*  char filename[40];*/
  
  u8_t track;
  u8_t sect;
};

#define ISO_slash    0x2f    
#define ISO_space    0x20
#define ISO_nl       0x0a
#define ISO_cr       0x0d


static struct drv_state ds;

/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(webserver_init)
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);

    ctk_window_new(&mainwindow, 17, 6, "Web server");
    ctk_window_move(&mainwindow, 21, 16);
  
    CTK_WIDGET_ADD(&mainwindow, &message);
    CTK_WIDGET_ADD(&mainwindow, &onoffmessage);

    CTK_WIDGET_ADD(&mainwindow, &tracklabel);
    CTK_WIDGET_ADD(&mainwindow, &tracknumlabel);
    CTK_WIDGET_ADD(&mainwindow, &sectorlabel);
    CTK_WIDGET_ADD(&mainwindow, &sectornumlabel);

    CTK_WIDGET_ADD(&mainwindow, &onbutton);
    CTK_WIDGET_ADD(&mainwindow, &offbutton);
    
    CTK_WIDGET_FOCUS(&mainwindow, &onbutton);
    
    /* Attach listeners to signals. */
    dispatcher_listen(ctk_signal_button_activate);

    dispatcher_uiplisten(80);
    
    onoff = OFF;
  }

  ctk_window_open(&mainwindow);
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(webserver_sighandler, s, data)
{
  struct ctk_button *b;
  unsigned char i;
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_button_activate) {
    b = (struct ctk_button *)data;
    if(b == &onbutton) {
      ctk_label_set_text(&onoffmessage, "On ");
      CTK_WIDGET_REDRAW(&onoffmessage);
      onoff = ON;
    } else if(b == &offbutton) {
      ctk_label_set_text(&onoffmessage, "Off");
      CTK_WIDGET_REDRAW(&onoffmessage);
      onoff = OFF;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
static void
x_open(u8_t f, u8_t d, u8_t cmd, u8_t *fname)
{
  u8_t ret;
  
  ret = cbm_open(f, d, cmd, fname);
  if(ret != 0) {
    /*    printf("open: error %d\n", ret);*/
    ctk_label_set_text(&statuslabel, "Open err");
    CTK_WIDGET_REDRAW(&statuslabel);
  }
  
}

static u8_t cmd[32];
static void
read_sector(u8_t device, u8_t track, u8_t sect, void *mem)
{  
  int ret;
  
  x_open(15, device, 15, NULL);
  x_open(2, device, 2, "#");

  /*  sprintf(cmd, "u1: 2  0%3d%3d", track, sect);  */
  strcpy(cmd, "u1: 2  0");
  cmd[8] = ' ';
  cmd[9] = '0' + track / 10;
  cmd[10] = '0' + track % 10;
  cmd[11] = ' ';
  cmd[12] = '0' + sect / 10;
  cmd[13] = '0' + sect % 10;
  cmd[14] = 0;
  cbm_write(15, cmd, strlen(cmd));
  /*  printf("%s\n", cmd);*/
    
  ret = cbm_read(2, mem, 256);
  if(ret == -1) {
    ctk_label_set_text(&statuslabel, "Read err");
    CTK_WIDGET_REDRAW(&statuslabel);
  }
  /*  printf("read: read %d bytes\n", ret);*/

  cbm_close(2);
  cbm_close(15);

}

static void
write_sector(u8_t device, u8_t track, u8_t sect, void *mem)
{
/*  u16_t ret;
  u8_t cmd[32];
  
  x_open(15, device, 15, NULL);
  x_open(2, device, 2, "#");

  sprintf(cmd, "u2: 2  0%3d%3d", track, sect);  
  cbm_write(15, cmd, strlen(cmd));
  printf("%s\n", cmd);
    
  ret = cbm_write(2, mem, 256);
  printf("write: wrote %d bytes\n", ret);

  cbm_close(2);
  cbm_close(15);*/
}

static u8_t
next_sector(void)
{
  ++ds.sect;
  if(ds.track < 18) {
    if(ds.sect == 21) {
      ++ds.track;
      ds.sect = 0;
    }
  } else if(ds.track < 25) {
    if(ds.sect == 19) {
      ++ds.track;
      ds.sect = 0;
    }
  } else if(ds.track < 31) {
    if(ds.sect == 18) {
      ++ds.track;
      ds.sect = 0;
    }
  } else if(ds.track < 36) {
    if(ds.sect == 17) {
      ++ds.track;
      ds.sect = 0;
    }
  }

  tracknum[0] = '0' + ds.track / 10;
  tracknum[1] = '0' + ds.track % 10;

  CTK_WIDGET_REDRAW(&tracknumlabel);
  
  sectornum[0] = '0' + ds.sect / 10;
  sectornum[1] = '0' + ds.sect % 10;
  
  CTK_WIDGET_REDRAW(&sectornumlabel);
  
    
  if(ds.track == 36) {
    return 1;
  }
  return 0;
}


/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_UIPCALL(webserver_uipappcall, state)
{
  u8_t i;
  int len;
  
  if(onoff == OFF) {
    cbm_close(4);
    uip_abort();
    return;
  }

  if(uip_connected()) {
    ds.state = STATE_RECEIVING_REQUEST;
  } else if(uip_acked()) {
    switch(ds.state) {
    case STATE_SENDING_HEADERS:      
      ds.state = STATE_SENDING_DATA;
      ds.track = 1;
      ds.sect = 0;    
      break;
    case STATE_SENDING_DATA:
      if(next_sector() != 0) {
	cbm_close(4);
	uip_close();
	ds.state = STATE_NONE;
	return;
      }
    }
  } else if(uip_newdata()) {
    if(ds.state == STATE_RECEIVING_REQUEST) {
      ds.state = STATE_SENDING_HEADERS;
#if 0
      if(strncmp(uip_appdata, http_get, sizeof(http_get)) == 0 &&
	 uip_appdata[4] == ISO_slash) {
	/*	if(uip_appdata[5] == ISO_space) {
	  ds.filename[0] = '$';
	  ds.filename[1] = 0;
	} else {
	  for(i = 0; i < sizeof(ds.filename); ++i) {
	    ds.filename[i] = uip_appdata[6 + i];
	    if(ds.filename[i] == ISO_space ||
	       ds.filename[i] == ISO_nl ||
	       ds.filename[i] == ISO_cr) {
	      ds.filename[i] = 0;
	      break;
	    }
	  }
	}
	cbm_open(4, 8, 4, ds.filename);*/
	ds.state = STATE_SENDING_HEADERS;
      }
#endif
    }		      
  }
  switch(ds.state) {
  case STATE_SENDING_HEADERS:
    uip_send(http_webserver_d64_headers, sizeof(http_webserver_d64_headers) - 1);
    break;
  case STATE_SENDING_DATA:
    read_sector(8, ds.track, ds.sect, (void *)uip_appdata);    
    /*    len = cbm_read(4, uip_appdata, 256);
	  if(len > 0) {*/
      uip_send(uip_appdata, 256);
      /*    }*/
    break;
  }
  
}
/*-----------------------------------------------------------------------------------*/
