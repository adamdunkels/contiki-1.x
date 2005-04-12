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
 * $Id: wget.c,v 1.2 2005/04/12 21:56:30 oliverschmidt Exp $
 *
 */


#include "ctk.h"
#include "ek.h"
#include "webclient.h"
#include "resolv.h"
#include "uiplib.h"
#include "loader.h"
#include "cfs.h"

#include "contiki.h"

#include "program-handler.h"

#include <string.h>
#include <stdio.h>
#include <dio.h>


static struct ctk_window window;

static struct ctk_label urllabel =
  {CTK_LABEL(0, 1, 4, 1, "URL:")};
static char url[80];
static char urledit[80];
struct ctk_textentry urltextentry =
  {CTK_TEXTENTRY(5, 1, 30, 1, urledit, 78)};

static struct ctk_label savefilenamelabel =
  {CTK_LABEL(0, 3, 14, 1, "Save filename:")};
static char savefilename[40];
static struct ctk_textentry savefilenametextentry =
  {CTK_TEXTENTRY(15, 3, 20, 1, savefilename, 38)};

static struct ctk_button filebutton =
  {CTK_BUTTON(0, 5, 16, "Download to file")};

static struct ctk_button dskbutton =
  {CTK_BUTTON(19, 5, 16, "Download to disk")};

static struct ctk_label statustext =
  {CTK_LABEL(0, 7, 37, 1, "")};
static char statusmsg[40];

static struct ctk_window dskdialog;
static struct ctk_label overwritelabel =
  {CTK_LABEL(0, 1, 36, 1, "This will overwrite the entire disk!")};
static struct ctk_label makesurelabel1 =
  {CTK_LABEL(7, 3, 22, 1, "Make sure you have the")};
static struct ctk_label makesurelabel2 =
  {CTK_LABEL(4, 4, 28, 1, "right disk in slot 6 drive 1")};
static struct ctk_button overwritebutton =
  {CTK_BUTTON(2, 6, 14, "Overwrite disk")};
static struct ctk_button cancelbutton =
  {CTK_BUTTON(26, 6, 6, "Cancel")};

EK_EVENTHANDLER(wget_eventhandler, ev, data);
EK_PROCESS(p, "Web downloader", EK_PRIO_NORMAL,
	   wget_eventhandler, NULL, NULL);
static ek_id_t id = EK_ID_NONE;

/* State */

#define DLOAD_NONE 0
#define DLOAD_FILE 1
#define DLOAD_DSK  2
static u8_t dload_state;
static unsigned long dload_bytes;

static int savefile;
static void* savedsk;

static char buffer[16][256];
static u16_t bufferptr;
static u8_t relsector;
static u16_t absblock;

/*-----------------------------------------------------------------------------------*/
/* wget_init();
 *
 * Initializes and starts the web browser. Called either at startup or
 * to open the browser window.
 */
LOADER_INIT_FUNC(wget_init, arg)
{
  if(arg != NULL) {
    strncpy(url, arg, sizeof(url));
    strncpy(urledit, arg, sizeof(urledit));
  } else {
#ifdef WGET_CONF_URL
    strncpy(url, WGET_CONF_URL, sizeof(url));
    strncpy(urledit, WGET_CONF_URL, sizeof(urledit));
#endif /* WGET_CONF_URL  */
  }
  arg_free(arg);
  
  if(id == EK_ID_NONE) {
    id = ek_start(&p);

  }
}
/*-----------------------------------------------------------------------------------*/
static void
show_statustext(char *text)
{
  ctk_label_set_text(&statustext, text);
  CTK_WIDGET_REDRAW(&statustext);
}
/*-----------------------------------------------------------------------------------*/
static void
dload_close(char *text)
{
  show_statustext(text);
  if(dload_state == DLOAD_FILE) {
    cfs_close(savefile);
  } else if(dload_state == DLOAD_DSK) {
    dio_close(savedsk);
  }
  dload_state = DLOAD_NONE;
  webclient_close();
}
/*-----------------------------------------------------------------------------------*/
/* open_url():
 *
 * Called when the URL present in the global "url" variable should be
 * opened. It will call the hostname resolver as well as the HTTP
 * client requester.
 */
static void
start_get(void)
{
  u16_t addr[2];
  unsigned char i;
  static char host[32];
  char *file;
  register char *urlptr;

  /* Trim off any spaces in the end of the url. */
  urlptr = url + strlen(url) - 1;
  while(*urlptr == ' ' && urlptr > url) {
    *urlptr = 0;
    --urlptr;
  }

  /* Don't even try to go further if the URL is empty. */
  if(urlptr == url) {
    return;
  }

  /* See if the URL starts with http://, otherwise prepend it. */
  if(strncmp(url, http_http, 7) != 0) {
    while(urlptr >= url) {
      *(urlptr + 7) = *urlptr;
      --urlptr;
    }
    strncpy(url, http_http, 7);
  } 

  /* Find host part of the URL. */
  urlptr = &url[7];  
  for(i = 0; i < sizeof(host); ++i) {
    if(*urlptr == 0 ||
       *urlptr == '/' ||
       *urlptr == ' ' ||
       *urlptr == ':') {
      host[i] = 0;
      break;
    }
    host[i] = *urlptr;
    ++urlptr;
  }

  /* XXX: Here we should find the port part of the URL, but this isn't
     currently done because of laziness from the programmer's side
     :-) */
  
  /* Find file part of the URL. */
  while(*urlptr != '/' && *urlptr != 0) {
    ++urlptr;
  }
  if(*urlptr == '/') {
    file = urlptr;
  } else {
    file = "/";
  }
      
  /* First check if the host is an IP address. */
  if(uiplib_ipaddrconv(host, (unsigned char *)addr) == 0) {    
    
    /* Try to lookup the hostname. If it fails, we initiate a hostname
       lookup and print out an informative message on the
       statusbar. */
    if(resolv_lookup(host) == NULL) {
      resolv_query(host);
      show_statustext("Resolving host...");
      return;
    }
  }

  /* The hostname we present in the hostname table, so we send out the
     initial GET request. */
  if(webclient_get(host, 80, file) == 0) {
    show_statustext("Out of memory error.");
  } else {
    show_statustext("Connecting...");
  }
}
/*-----------------------------------------------------------------------------------*/
EK_EVENTHANDLER(wget_eventhandler, ev, data)
{
  EK_EVENTHANDLER_ARGS(ev, data);

  if(ev == EK_EVENT_INIT) {
    /* Create the main window. */
    ctk_window_new(&window, 37, 8, "Web downloader");

    CTK_WIDGET_ADD(&window, &urllabel);
    CTK_WIDGET_ADD(&window, &urltextentry);
    CTK_WIDGET_ADD(&window, &savefilenamelabel);
    CTK_WIDGET_ADD(&window, &savefilenametextentry);
    CTK_WIDGET_ADD(&window, &filebutton);
    CTK_WIDGET_ADD(&window, &dskbutton);
    CTK_WIDGET_ADD(&window, &statustext);

    dload_state = DLOAD_NONE;
      
    memset(savefilename, 0, sizeof(savefilename));
    memset(url, 0, sizeof(url));

    ctk_dialog_new(&dskdialog, 36, 8);
    CTK_WIDGET_ADD(&dskdialog, &overwritelabel);
    CTK_WIDGET_ADD(&dskdialog, &makesurelabel1);
    CTK_WIDGET_ADD(&dskdialog, &makesurelabel2);
    CTK_WIDGET_ADD(&dskdialog, &overwritebutton);
    CTK_WIDGET_ADD(&dskdialog, &cancelbutton);
    
    ctk_window_open(&window);
  } else if(ev == tcpip_event) {
    webclient_appcall(data);
  } else if(ev == ctk_signal_button_activate) {
    if(data == (void *)&filebutton) {
      dload_close("");
      savefile = cfs_open(savefilename, CFS_WRITE);
      if(savefile == -1) {
	sprintf(statusmsg, "Open error with '%s'", savefilename);
	show_statustext(statusmsg);
      } else {
	strncpy(url, urledit, sizeof(url));
	start_get();
	dload_bytes = 0;
	dload_state = DLOAD_FILE;
      }
    } else if(data == (void *)&dskbutton) {
      ctk_dialog_open(&dskdialog);
    } else if(data == (void *)&cancelbutton) {
      ctk_dialog_close();
    } else if(data == (void *)&overwritebutton) {
      ctk_dialog_close();
      dload_close("");
      savedsk = dio_open(/*slot*/6 * 2 + /*drive*/1 - 1);
      if(savedsk == NULL) {
	sprintf(statusmsg, "Access error with slot 6 drive 1");
	show_statustext(statusmsg);
      } else {
	strncpy(url, urledit, sizeof(url));
	start_get();
	dload_bytes = 0;
	dload_state = DLOAD_DSK;
	bufferptr = 0;
	relsector = 0;
	absblock = 0;
      }
    }
  } else if(ev == ctk_signal_hyperlink_activate) {
    if(dload_state == DLOAD_NONE) {
      /*      open_link(w->widget.hyperlink.url);*/
      strncpy(urledit,
	      ((struct ctk_widget *)data)->widget.hyperlink.url, sizeof(urledit));
      CTK_WIDGET_REDRAW(&urltextentry);
      CTK_WIDGET_FOCUS(&window, &urltextentry);
    }
  } else if(ev == resolv_event_found) {
    /* Either found a hostname, or not. */
    if((char *)data != NULL &&
       resolv_lookup((char *)data) != NULL) {
      start_get();
    } else {
      show_statustext("Host not found.");
    }
  } else if(ev == ctk_signal_window_close) {
    dload_close("");
    ek_exit();
    id = EK_ID_NONE;
    LOADER_UNLOAD();
  }
}
/*-----------------------------------------------------------------------------------*/
/* webclient_aborted():
 *
 * Callback function. Called from the webclient when the HTTP
 * connection was abruptly aborted.
 */
void
webclient_aborted(void)
{
  show_statustext("Connection reset by peer");
}
/*-----------------------------------------------------------------------------------*/
/* webclient_timedout():
 *
 * Callback function. Called from the webclient when the HTTP
 * connection timed out.
 */
void
webclient_timedout(void)
{
  dload_close("Connection timed out");
}
/*-----------------------------------------------------------------------------------*/
/* webclient_closed():
 *
 * Callback function. Called from the webclient when the HTTP
 * connection was closed after a request from the "webclient_close()"
 * function. .
 */
void
webclient_closed(void)
{
  if(dload_state == DLOAD_NONE) {
    return;
  }
  show_statustext("Done.");
}
/*-----------------------------------------------------------------------------------*/
/* webclient_closed():
 *
 * Callback function. Called from the webclient when the HTTP
 * connection is connected.
 */
void
webclient_connected(void)
{    
  show_statustext("Request sent...");
}
/*-----------------------------------------------------------------------------------*/
static unsigned char
write_buffer(void)
{
  u8_t sector;

  for(sector = 0; sector < 16; sector += 2) {
    if(dio_write(savedsk, absblock++, buffer[sector]) != 0) {
      return 0;
    }
  }
  return 1;
}
/*-----------------------------------------------------------------------------------*/
static unsigned char
handle_dsk_data(char *data, u16_t len)
{
  static u8_t skew[16] = {0x0, 0xE, 0xD, 0xC, 0xB, 0xA, 0x9, 0x8,
			  0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0xF};
  u16_t bufferlen;

  while(len > 0) {
    bufferlen = sizeof(buffer[0]) - bufferptr;
    if(len < bufferlen) {
      bufferlen = len;
    }
    
    memcpy(&buffer[skew[relsector]][bufferptr], data, bufferlen);

    data += bufferlen;
    bufferptr += bufferlen;
    len -= bufferlen;
    
    if(bufferptr == sizeof(buffer[0])) {
      bufferptr = 0;
      if(++relsector == 16) {
	relsector = 0;
	if(write_buffer() == 0) {
	  return 0;
	}
      }
    }
  }
  return 1;
}
/*-----------------------------------------------------------------------------------*/
/* webclient_datahandler():   
 *
 * Callback function. Called from the webclient module when HTTP data
 * has arrived.
 */
void
webclient_datahandler(char *data, u16_t len)
{
  int ret;
  
  if(len > 0) {
    dload_bytes += len;    
    sprintf(statusmsg, "Downloading (%lu bytes)", dload_bytes);
    show_statustext(statusmsg);
    if(dload_state == DLOAD_DSK) {
      if(handle_dsk_data(data, len) == 0) {
	dload_close("Write error with slot 6 drive 1");
      }
    } else if(dload_state == DLOAD_FILE) {      
      ret = cfs_write(savefile, data, len);       
      if(ret != len) {
	sprintf(statusmsg, "Wrote only %d bytes", ret);
	dload_close(statusmsg);
      }
    }
  }
  
  if(data == NULL) {
    sprintf(statusmsg, "Finished downloading %lu bytes", dload_bytes);
    dload_close(statusmsg);
  }
}
/*-----------------------------------------------------------------------------------*/
