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
 * $Id: www.c,v 1.2 2003/03/28 12:09:13 adamdunkels Exp $
 *
 */


#include "ctk.h"
#include "dispatcher.h"
#include "webclient.h"
#include "htmlparser.h"
#include "http-strings.h"
#include "resolv.h"

#include "petsciiconv.h"

#include "www-conf.h"

#if 0
#define PRINTF(x)
#else
#include <stdio.h>
#define PRINTF(x) printf x
#endif


/* The array that holds the current URL. */
static char url[WWW_CONF_MAX_URLLEN + 1];
static char tmpurl[WWW_CONF_MAX_URLLEN + 1];

/* The array that holds the web page text. */
#define WEBPAGE_WIDTH 36
#define WEBPAGE_HEIGHT 17
static char webpage[WEBPAGE_WIDTH * WEBPAGE_HEIGHT + 1];

/* The CTK widgets for the main window. */
static struct ctk_window mainwindow;

static struct ctk_button backbutton =
  {CTK_BUTTON(0, 0, 4, "Back")};
static struct ctk_button downbutton =
  {CTK_BUTTON(10, 0, 4, "Down")};
static struct ctk_button stopbutton =
  {CTK_BUTTON(20, 0, 4, "Stop")};
static struct ctk_button gobutton =
  {CTK_BUTTON(32, 0, 2, "Go")};

static struct ctk_separator sep1 =
  {CTK_SEPARATOR(0, 2, 36)};

static char editurl[WWW_CONF_MAX_URLLEN + 1];
static struct ctk_textentry urlentry =
  {CTK_TEXTENTRY(0, 1, 34, 1, editurl, WWW_CONF_MAX_URLLEN)};
static struct ctk_label webpagelabel =
  {CTK_LABEL(0, 3, WEBPAGE_WIDTH, WEBPAGE_HEIGHT, webpage)};

static char statustexturl[36];
static struct ctk_label statustext =
  {CTK_LABEL(0, 21, 36, 1, "")};
static struct ctk_separator sep2 =
  {CTK_SEPARATOR(0, 20, 36)};

/* The char arrays that hold the history of visited URLs. */
#define HISTORY_SIZE 4
static char history[HISTORY_SIZE][WWW_CONF_MAX_URLLEN];
static char history_last, history_first;


/* The CTK widget definitions for the hyperlinks and the char arrays
   that hold the link URLs. */
struct formattribs {
  char formaction[WWW_CONF_MAX_FORMACTIONLEN];
  char formname[WWW_CONF_MAX_FORMNAMELEN];
#define FORMINPUTTYPE_SUBMITBUTTON 1
#define FORMINPUTTYPE_INPUTFIELD   2
  unsigned char inputtype;
  char inputname[WWW_CONF_MAX_INPUTNAMELEN];
  char *inputvalue;
};

union pagewidgetattrib {
  char url[WWW_CONF_MAX_URLLEN];
  struct formattribs form;
};
#define MAX_NUMPAGEWIDGETS 20
static struct ctk_widget pagewidgets[MAX_NUMPAGEWIDGETS];
static union pagewidgetattrib pagewidgetattribs[MAX_NUMPAGEWIDGETS];
static unsigned char pagewidgetptr;


/* The "scrolly" variable holds the line number (in the web page) of
   the first line of text shown on screen. */
static unsigned short scrolly;

#if WWW_CONF_RENDERSTATE
static unsigned char renderstate;
#endif /* WWW_CONF_RENDERSTATE */

/* The "run" flag is used to determine if the web page should be
   continuosly scrolled upward with new data coming in from below. */
static unsigned char run;

#define ISO_nl    0x0a
#define ISO_space 0x20
#define ISO_ampersand 0x26
#define ISO_plus 0x2b
#define ISO_slash 0x2f
#define ISO_eq    0x3d
#define ISO_questionmark  0x3f

/* The state of the rendering code. */
static u8_t x;
static u16_t starty;
static char nextword[WEBPAGE_WIDTH + 1];
static unsigned char nextwordptr;

static unsigned char count;
static char receivingmsgs[4][23] = {
  "Receiving web page ...",
  "Receiving web page. ..",
  "Receiving web page.. .",
  "Receiving web page... "
};


static void sighandler(ek_signal_t s, ek_data_t data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Web browser", NULL, sighandler, webclient_appcall)};
static ek_id_t id;


static void formsubmit(struct formattribs *attribs);
/*-----------------------------------------------------------------------------------*/
/* make_window()
 *
 * Creates the web browser's window.
 */
static void
make_window(void)
{ 

  CTK_WIDGET_ADD(&mainwindow, &backbutton);
  CTK_WIDGET_ADD(&mainwindow, &downbutton);
  CTK_WIDGET_ADD(&mainwindow, &stopbutton);
  CTK_WIDGET_ADD(&mainwindow, &gobutton);
  CTK_WIDGET_ADD(&mainwindow, &urlentry);
  CTK_WIDGET_ADD(&mainwindow, &sep1);
  CTK_WIDGET_ADD(&mainwindow, &webpagelabel);
  CTK_WIDGET_ADD(&mainwindow, &sep2);
  CTK_WIDGET_ADD(&mainwindow, &statustext);

  CTK_WIDGET_FOCUS(&mainwindow, &stopbutton);
  
  pagewidgetptr = 0;
}
/*-----------------------------------------------------------------------------------*/
/* redraw_window():
 *
 * Convenience function that calls upon CTK to redraw the browser
 * window. */
static void
redraw_window(void)
{
  ctk_window_redraw(&mainwindow);
}
/*-----------------------------------------------------------------------------------*/
/* www_init();
 *
 * Initializes and starts the web browser. Called either at startup or
 * to open the browser window.
 */
void
www_init(void)     
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    
    /* Create the main window. */
    memset(webpage, 0, sizeof(webpage));
    ctk_window_new(&mainwindow, 36, 22, "WWW");
    make_window();
    CTK_WIDGET_FOCUS(&mainwindow, &urlentry);
    
    /* Attach as a listener to a number of signals ("Button activate",
       "Hyperlink activate" and "Hyperlink hover", and the resolver's
       signal. */
    dispatcher_listen(ctk_signal_button_activate);
    dispatcher_listen(ctk_signal_hyperlink_activate);
    dispatcher_listen(ctk_signal_hyperlink_hover);
    dispatcher_listen(resolv_signal_found);
  }
  ctk_window_open(&mainwindow);
}
/*-----------------------------------------------------------------------------------*/
static void
clear_page(void)
{
  if(ctk_window_isopen(&mainwindow)) {
    ctk_window_close(&mainwindow);
  }
  ctk_window_clear(&mainwindow);
  make_window();
  ctk_window_open(&mainwindow);
  memset(webpage, 0, WEBPAGE_WIDTH * WEBPAGE_HEIGHT);  
}
/*-----------------------------------------------------------------------------------*/
static void
show_url(void)
{
  memcpy(editurl, url, WWW_CONF_MAX_URLLEN);
  strncpy(editurl, "http://", 7);
  petsciiconv_topetscii(editurl + 7, WWW_CONF_MAX_URLLEN - 7);
  CTK_WIDGET_REDRAW(&urlentry);
}
/*-----------------------------------------------------------------------------------*/
static void
show_statustext(char *text)
{
  ctk_label_set_text(&statustext, text);
  CTK_WIDGET_REDRAW(&statustext);
}
/*-----------------------------------------------------------------------------------*/
/* open_url():
 *
 * Called when the URL present in the global "url" variable should be
 * opened. It will call the hostname resolver as well as the HTTP
 * client requester.
 */
static void
open_url(void)
{
  unsigned char i;
  static char host[32];
  char *file;
  register char *urlptr;
  unsigned short port;

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
  
  /* Try to lookup the hostname. If it fails, we initiate a hostname
     lookup and print out an informative message on the statusbar. */
  if(resolv_lookup(host) == NULL) {
    resolv_query(host);
    show_statustext("Resolving host...");
    return;
  }

  /* The hostname we present in the hostname table, so we send out the
     initial GET request. */
  if(webclient_get(host, 80, file) == 0) {
    show_statustext("Out of memory error.");
  } else {
    show_statustext("Connecting...");
  }
  redraw_window();
}
/*-----------------------------------------------------------------------------------*/
/* open_link(link):
 *
 * Will format a link from the current web pages so that it suits the
 * open_url() function and finally call it to open the requested URL.
 */
static void
open_link(char *link)
{
  char *urlptr;
    
  if(strncmp(link, http_http, 7) == 0) {
    /* The link starts with http://. We just copy the contents of the
       link into the url string and jump away. */
    strncpy(url, link, WWW_CONF_MAX_URLLEN);
  } else if(*link == ISO_slash &&
	    *(link + 1) == ISO_slash) {
    /* The link starts with //, so we'll copy it into the url
       variable, starting after the http (which already is present in
       the url variable since we were able to open the web page on
       which this link was found in the first place). */
    strncpy(&url[5], link, WWW_CONF_MAX_URLLEN);   
  } else if(*link == ISO_slash) {
    /* The link starts with a slash, so it is a non-relative link
       within the same web site. We find the start of the filename of
       the current URL and paste the contents of this link there, and
       head off to the new URL. */
    for(urlptr = &url[7];
	*urlptr != 0 && *urlptr != ISO_slash;
	++urlptr);    
    strncpy(urlptr, link, WWW_CONF_MAX_URLLEN - (urlptr - url));    
  } else {
    /* A fully relative link is found. We find the last slash in the
       current URL and paste the link there. */
    
    /* XXX: we should really parse any ../ in the link as well. */
    for(urlptr = url + strlen(url);
	urlptr != url && *urlptr != ISO_slash;
	--urlptr);
    ++urlptr;
    strncpy(urlptr, link, WWW_CONF_MAX_URLLEN - (urlptr - url));    
  }

  /* Open the URL. */
  scrolly = 0;
  show_url();
  open_url();
}
/*-----------------------------------------------------------------------------------*/
/* log_back():
 *
 * Copies the current URL from the url variable and into the log for
 * the back button.
 */
static void
log_back(void)
{
  memcpy(history[history_last], url, WWW_CONF_MAX_URLLEN);
  ++history_last;
  if(history_last > HISTORY_SIZE) {
    history_last = 0;
  }
}
/*-----------------------------------------------------------------------------------*/
/* www_dispatcher():
 *
 * The program's signal dispatcher function. Is called by the ek
 * dispatcher whenever a signal arrives.
 */
static void
sighandler(ek_signal_t s, ek_data_t data)
{
  static struct ctk_widget *w;
  static unsigned char i;

  
  w = (struct ctk_widget *)data;
  if(s == ctk_signal_button_activate) {
    if(w == (struct ctk_widget *)&backbutton) {
      scrolly = 0;
      run = 1;

      --history_last;
      if(history_last > HISTORY_SIZE) {
	history_last = HISTORY_SIZE - 1;
      }
      memcpy(url, history[history_last], WWW_CONF_MAX_URLLEN);
      open_url();
      CTK_WIDGET_FOCUS(&mainwindow, &backbutton);      
    } else if(w == (struct ctk_widget *)&downbutton) {
      run = 1;
      open_url();
      CTK_WIDGET_FOCUS(&mainwindow, &downbutton);
    } else if(w == (struct ctk_widget *)&gobutton) {
      scrolly = 0;
      run = 1;
      log_back();
      memcpy(url, editurl, WWW_CONF_MAX_URLLEN);
      petsciiconv_toascii(url, WWW_CONF_MAX_URLLEN);
      open_url();
      CTK_WIDGET_FOCUS(&mainwindow, &gobutton);
    } else if(w == (struct ctk_widget *)&stopbutton) {
      run = 0;
      webclient_close();
#if WWW_CONF_FORMS
    } else {
      /* Check form buttons */
      for(i = 0; i < pagewidgetptr; ++i) {
	if(&pagewidgets[i] == w) {
	  formsubmit(&pagewidgetattribs[i].form);
	  /*	  show_statustext(pagewidgetattribs[i].form.formaction);*/
	  /*	  PRINTF(("Formaction %s formname %s inputname %s\n",
		  pagewidgetattribs[i].form.formaction,
		  pagewidgetattribs[i].form.formname,
		  pagewidgetattribs[i].form.inputname));*/
	  break;
	}
      }
#endif /* WWW_CONF_FORMS */
    }
  } else if(s == ctk_signal_hyperlink_activate) {
    log_back();
    open_link(w->widget.hyperlink.url);
    CTK_WIDGET_FOCUS(&mainwindow, &stopbutton);
    ctk_window_open(&mainwindow);
    run = 1;
  } else if(s == ctk_signal_hyperlink_hover) {
    strncpy(statustexturl, w->widget.hyperlink.url,
	    sizeof(statustexturl));
    petsciiconv_topetscii(statustexturl, sizeof(statustexturl));
    show_statustext(statustexturl);
  } else if(s == resolv_signal_found) {
    /* Either found a hostname, or not. */
    if((char *)data != NULL &&
       resolv_lookup((char *)data) != NULL) {
      open_url();
    } else {
      show_statustext("Host not found.");
    }
  }
}
/*-----------------------------------------------------------------------------------*/
/* set_url():
 *
 * Constructs an URL from the arguments and puts it into the global
 * "url" variable and the visible "editurl" (which is shown in the URL
 * text entry widget in the browser window).
 */
static void
set_url(char *host, u16_t port, char *file)
{
  char *urlptr;

  memset(url, 0, WWW_CONF_MAX_URLLEN);
  
  if(strncmp(file, http_http, 7) == 0) {
    strncpy(url, file, sizeof(url));
  } else {
    strncpy(url, http_http, 7);
    urlptr = url + 7;
    strcpy(urlptr, host);
    urlptr += strlen(host);
    strcpy(urlptr, file);
  }

  show_url();
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
  show_statustext("Connection timed out");
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
  show_statustext("Stopped.");
  petsciiconv_topetscii(&webpage[(WEBPAGE_HEIGHT - 1) *
				 WEBPAGE_WIDTH], WEBPAGE_WIDTH);
  redraw_window();
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
  x = nextwordptr = 0;
  starty = scrolly;
  nextword[0] = 0;

  if(scrolly == 0) {
    clear_page();
    redraw_window();
  }
    
  show_statustext("Request sent...");
  set_url(webclient_hostname(), webclient_port(), webclient_filename());

#if WWW_CONF_RENDERSTATE 
  renderstate = HTMLPARSER_RENDERSTATE_NONE;
#endif /* WWW_CONF_RENDERSTATE */
  htmlparser_init();
}
/*-----------------------------------------------------------------------------------*/
/* scroll():
 *
 * Scrolls the entire web page display (text and hyperlinks) one line
 * upwards.
 */
static void
scroll(void)
{
  unsigned char i;
  unsigned char lptr, linkptrtmp;
  struct ctk_widget *linksptr;
  char *statustexttext;
  struct ctk_widget *focuswidget;
  
  /* Scroll text up. */
  memcpy(webpage, &webpage[WEBPAGE_WIDTH],
	 (WEBPAGE_HEIGHT - 1) * WEBPAGE_WIDTH);
  
  /* Clear last line of text. */
  memset(&webpage[(WEBPAGE_HEIGHT - 1) * WEBPAGE_WIDTH],
	 ' ', WEBPAGE_WIDTH);
  
  /* Scroll links up. */

  lptr = 0;
  linksptr = pagewidgets;
  for(i = 0; i < pagewidgetptr; ++i) {    
    /* First, check which links that scroll off the top of the page
       and should be removed. */
    if(CTK_WIDGET_YPOS(linksptr) == 3) {
      lptr = i + 1;
    } else {
      /* Else, move them upward one notch. */
      
      /* XXX: this is really a hack! These values should not be used
	 like this, but should be obtained and set using some CTK API
	 function. */
      linksptr->widget.hyperlink.text -= WEBPAGE_WIDTH;

      --(linksptr->y);
    }
    ++linksptr;
  }

  /* See if there are any links that scroll off the top. */
  if(lptr != 0) {
    memcpy(pagewidgets, &pagewidgets[lptr],
	   sizeof(struct ctk_widget) * (MAX_NUMPAGEWIDGETS - lptr));
    memcpy(pagewidgetattribs, &pagewidgetattribs[lptr],
	   sizeof(union pagewidgetattrib) * (MAX_NUMPAGEWIDGETS - lptr));    

    /* Compute new value of linkptr and tuck it away in
       linkptrtmp. make_window() destroys linkptr, so we need to
       restore it after the call. */
    linkptrtmp = pagewidgetptr - lptr;

    /* XXX: hack - these values should *not* be obtained this way, but
       through some CTK API instead! */
    statustexttext = statustext.text;
    focuswidget = mainwindow.focused;
    ctk_window_clear(&mainwindow);
    make_window();
    CTK_WIDGET_FOCUS(&mainwindow, focuswidget);
    show_statustext(statustexttext);
    
    pagewidgetptr = linkptrtmp;
	
    linksptr = pagewidgets;
    for(i = 0; i < pagewidgetptr; ++i) {
      if(linksptr->type == CTK_WIDGET_HYPERLINK) {
	linksptr->widget.hyperlink.url -= lptr *
	  sizeof(union pagewidgetattrib);
      }
      CTK_WIDGET_ADD(&mainwindow, linksptr);
      ++linksptr;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
static char tmpcenterline[WEBPAGE_WIDTH];
/* inc_y():
 *
 * Called from the rendering code when it is time to move on line
 * downwards.
 */
static void
inc_y(void)
{
  unsigned char spaces, i;
  char *cptr;
  struct ctk_widget *linksptr;
  
  if(starty > 0) {
    --starty;
  } else {
#if WWW_CONF_RENDERSTATE
    /* Check if current line should be centered and if so, center
       it. */
    if(renderstate & HTMLPARSER_RENDERSTATE_CENTER) {
      cptr = &webpage[(WEBPAGE_HEIGHT - 0) * WEBPAGE_WIDTH - 1];
      for(spaces = 0; spaces < WEBPAGE_WIDTH; ++spaces) {
	if(*cptr-- != ' ') {
	  break;
	}
      }

      spaces = spaces / 2;

      strncpy(tmpcenterline,
	      &webpage[(WEBPAGE_HEIGHT - 1) *
		       WEBPAGE_WIDTH],
	      WEBPAGE_WIDTH);
      strncpy(&webpage[(WEBPAGE_HEIGHT - 1) *
		       WEBPAGE_WIDTH] + spaces,
	      tmpcenterline,
	      WEBPAGE_WIDTH - spaces);
      memset(&webpage[(WEBPAGE_HEIGHT - 1) *
		      WEBPAGE_WIDTH], ' ', spaces);
      
      linksptr = pagewidgets;
      for(i = 0; i < pagewidgetptr; ++i) {
	if(CTK_WIDGET_YPOS(linksptr) == 3 + WEBPAGE_HEIGHT - 1) {
	  linksptr->x += spaces;
	  linksptr->widget.hyperlink.text += spaces;
	}
	++linksptr;
      }
    }
#endif /* WWW_CONF_RENDERSTATE */
        
    petsciiconv_topetscii(&webpage[(WEBPAGE_HEIGHT - 1) *
				   WEBPAGE_WIDTH], WEBPAGE_WIDTH);
    redraw_window();
    scroll();
    ++scrolly;
  }
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
  if(len > 0) {
    /*    if(strcmp(webclient_mimetype(), http_texthtml) == 0) {*/
      count = (count + 1) & 3;
      show_statustext(receivingmsgs[count]);
      htmlparser_parse(data, len);
      /*    } else {
      show_statustext("Receiving non-HTML data...");
      }*/
  } else {
    /* Clear remaining parts of page. */
    run = 0;
  }
  
  if(data == NULL) {
    run = 0;
    show_statustext("Done.");
    petsciiconv_topetscii(&webpage[(WEBPAGE_HEIGHT - 1) *
				   WEBPAGE_WIDTH], WEBPAGE_WIDTH);
    redraw_window();
  }
}
/*-----------------------------------------------------------------------------------*/
/* output_word():
 *
 * Called from the rendering code when a full word has been received
 * and should be put on screen.
 */
static void
output_word(char c)
{
  char *webpageptr;

  if(nextwordptr == 0) {
    if(c == ISO_nl) {
      x = 0;
      inc_y();
    }
    return;
  }
  
  if(x + nextwordptr > WEBPAGE_WIDTH) {
    inc_y();
    x = 0;
  }

  nextword[nextwordptr] = 0;
  if(starty == 0) {
    webpageptr = &webpage[(WEBPAGE_HEIGHT - 1) * WEBPAGE_WIDTH + x];
    if(nextwordptr > 0) {
      strcpy(webpageptr, nextword);
    }
    webpageptr[nextwordptr] = ' ';
  }
  if(c == ISO_nl) {
    x = 0;
    inc_y();
  } else {
    x += nextwordptr + 1;
  }
  nextwordptr = 0;  
}
/*-----------------------------------------------------------------------------------*/
static void *
add_pagewidget(char *text, unsigned char type,
	       unsigned char border)
{  
  static unsigned char len, maxwidth;
  static unsigned char *webpageptr;
  static void *dataptr;
  register struct ctk_widget *lptr;

  len = strlen(text);

  if(len + border == 0) {
    return NULL;
  }
  
  maxwidth = WEBPAGE_WIDTH - (1 + 2 * border);
  
  /* If the text of the link is too long so that it does not fit into
     the width of the current window, counting from the current x
     coordinate, we first try to jump to the next line. */
  if(len + x > maxwidth) {
    output_word(ISO_nl);
  }

  /* If the text of the link still is too long, we just chop it off!
     XXX: this is not really the right thing to do, we should probably
     either make the link a multiline button, or add multiple
     buttons. But this will do for now. */
  if(len > maxwidth) {
    text[maxwidth] = 0;
    len = maxwidth;
  }

  dataptr = NULL;
  
  if(starty == 0) {
    webpageptr = &webpage[(WEBPAGE_HEIGHT - 1) * WEBPAGE_WIDTH + x];
    /* To save memory, we'll copy the widget text to the web page
       drawing area and reference it from there. */
    webpageptr[0] = 0;
    webpageptr += border;
    strncpy(webpageptr, text, len);
    webpageptr[len] = 0;
    webpageptr[len + border] = ' ';
    if(pagewidgetptr < MAX_NUMPAGEWIDGETS) {
      dataptr = &pagewidgetattribs[pagewidgetptr];
      lptr = &pagewidgets[pagewidgetptr];
      
      switch(type) {
      case CTK_WIDGET_HYPERLINK:
	CTK_HYPERLINK_NEW((struct ctk_hyperlink *)lptr, x,
			  WEBPAGE_HEIGHT + 2, len,
			  webpageptr, dataptr);
	break;
      case CTK_WIDGET_BUTTON:
	CTK_BUTTON_NEW((struct ctk_button *)lptr, x,
		       WEBPAGE_HEIGHT + 2, len,
		       webpageptr);
	((struct formattribs *)dataptr)->inputvalue = webpageptr;
	break;
      case CTK_WIDGET_TEXTENTRY:
	CTK_TEXTENTRY_NEW((struct ctk_textentry *)lptr,
			  x, WEBPAGE_HEIGHT + 2, len, 1,
			  webpageptr, len);
	((struct formattribs *)dataptr)->inputvalue = webpageptr;
	break;	
      }
      CTK_WIDGET_ADD(&mainwindow, lptr);

      ++pagewidgetptr;
    }
  }
  /* Increase the x coordinate with the length of the link text plus
     the extra space behind it and the CTK button markers. */
  x += len + 1 + 2 * border;

  if(x >= WEBPAGE_WIDTH) {
    inc_y();
    x = 0;
  }

  return dataptr;
}
/*-----------------------------------------------------------------------------------*/
/* htmlparser_link:
 *
 * Callback function. Will be called when the HTML parser has parsed a
 * link. We will put a CTK hyperlink widget at the appropriate position
 * in the window.
 */
void
htmlparser_link(char *text, char *url)
{
  static unsigned char *linkurlptr;

  linkurlptr = add_pagewidget(text, CTK_WIDGET_HYPERLINK, 0);
  if(linkurlptr != NULL &&
     strlen(url) < WWW_CONF_MAX_URLLEN) {
    strcpy(linkurlptr, url);
  }
}
/*-----------------------------------------------------------------------------------*/
/* htmlparser_char():
 *
 * Callback function. Called by the HTML parser module for every
 * printable character in the HTML file.
 */
void
htmlparser_char(char c)
{
  if(c == ' ' ||
     c == ISO_nl) {
    output_word(c);
  } else if(c != 0) {    
    nextword[nextwordptr] = c;
    if(nextwordptr < WEBPAGE_WIDTH) {
      ++nextwordptr;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
#if WWW_CONF_RENDERSTATE
void
htmlparser_renderstate(unsigned char s)
{
  if((s & HTMLPARSER_RENDERSTATE_STATUSMASK) ==
     HTMLPARSER_RENDERSTATE_BEGIN) {
    renderstate |= s & ~HTMLPARSER_RENDERSTATE_STATUSMASK;
  } else {
    renderstate &= ~(s & ~HTMLPARSER_RENDERSTATE_STATUSMASK);
  }
}
#endif /* WWW_CONF_RENDERSTATE */

#if WWW_CONF_FORMS
/*-----------------------------------------------------------------------------------*/
void
htmlparser_submitbutton(char *text, char *name,
			char *formname, char *formaction)
{
  register struct formattribs *form;
  form = add_pagewidget(text, CTK_WIDGET_BUTTON, 1);
  if(form != NULL) {
    strncpy(form->formaction, formaction, WWW_CONF_MAX_FORMACTIONLEN);
    strncpy(form->formname, formname, WWW_CONF_MAX_FORMNAMELEN);
    strncpy(form->inputname, name, WWW_CONF_MAX_INPUTNAMELEN);
    form->inputtype = FORMINPUTTYPE_SUBMITBUTTON;
  }
}
/*-----------------------------------------------------------------------------------*/
void
htmlparser_inputfield(char *text, char *name,
		      char *formname, char *formaction)
{
  register struct formattribs *form;
  
  form = add_pagewidget(text, CTK_WIDGET_TEXTENTRY, 1);
  if(form != NULL) {
    strncpy(form->formaction, formaction, WWW_CONF_MAX_FORMACTIONLEN);
    strncpy(form->formname, formname, WWW_CONF_MAX_FORMNAMELEN);
    strncpy(form->inputname, name, WWW_CONF_MAX_INPUTNAMELEN);
    form->inputtype = FORMINPUTTYPE_INPUTFIELD;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
formsubmit(struct formattribs *attribs)
{
  unsigned char i, j;
  char *urlptr, *valueptr;
  struct formattribs *faptr;

  urlptr = &tmpurl[0];

  strncpy(urlptr, attribs->formaction, WWW_CONF_MAX_URLLEN);
  tmpurl[WWW_CONF_MAX_URLLEN] = 0;
  urlptr += strlen(urlptr);
  *urlptr = ISO_questionmark;
  ++urlptr;
  
  
  /* Construct an URL by finding all input field forms with the same
     formname as the current submit button, and add the submit button
     URL stuff as well. */
  for(i = 0; i < pagewidgetptr; ++i) {
    if(urlptr - &tmpurl[0] >= WWW_CONF_MAX_URLLEN) {
      break;
    }

    faptr = &pagewidgetattribs[i].form;
    
    if(strcmp(attribs->formaction, faptr->formaction) == 0 &&
       strcmp(attribs->formname, faptr->formname) == 0 &&
       (faptr->inputtype == FORMINPUTTYPE_INPUTFIELD ||
	faptr == attribs)) {

      /* Copy the name of the input field into the URL and append a
	 questionmark. */
      strncpy(urlptr, faptr->inputname, WWW_CONF_MAX_URLLEN - strlen(tmpurl));
      tmpurl[WWW_CONF_MAX_URLLEN] = 0;
      urlptr += strlen(urlptr);
      *urlptr = ISO_eq;
      ++urlptr;

      /* Convert and copy the contents of the input field to the URL
	 and append an ampersand. */
      valueptr = pagewidgets[i].widget.textentry.text;
      petsciiconv_toascii(valueptr, WWW_CONF_MAX_INPUTVALUELEN);
      for(j = 0; j < WWW_CONF_MAX_INPUTVALUELEN; ++j) {
	if(urlptr - &tmpurl[0] >= WWW_CONF_MAX_URLLEN) {
	  break;
	}
	*urlptr = *valueptr;
	if(*urlptr == ISO_space) {
	  *urlptr = ISO_plus;
	}
	if(*urlptr == 0) {
	  break;
	}
	++urlptr;
	++valueptr;
      }
      
      *urlptr = ISO_ampersand;
      ++urlptr;
    }
  }
  --urlptr;
  *urlptr = 0;
  /*  PRINTF(("formsubmit: URL '%s'\n", tmpurl));*/
  open_link(tmpurl);
}
/*-----------------------------------------------------------------------------------*/
#endif /* WWW_CONF_FORMS */
