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
 * This file is part of the "ctk" console GUI toolkit for cc65
 *
 * $Id: ctk.c,v 1.5 2003/04/05 12:22:35 adamdunkels Exp $
 *
 */

#include "cc.h"
#include "ek.h"
#include "dispatcher.h"
#include "ctk.h"
#include "ctk-draw.h"
#include "ctk-conf.h"

static unsigned char height, width;

static unsigned char mode;

static struct ctk_window desktop_window;
static struct ctk_window *windows;
static struct ctk_window *dialog;

#if CTK_CONF_MENUS
static struct ctk_menus menus;
static struct ctk_menu *lastmenu;
static struct ctk_menu desktopmenu;
#endif /* CTK_CONF_MENUS */

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */


#define REDRAW_NONE         0
#define REDRAW_ALL          1
#define REDRAW_FOCUS        2
#define REDRAW_WIDGETS      4
#define REDRAW_MENUS        8
#define REDRAW_MENUPART     16

#define MAX_REDRAWWIDGETS 4
static unsigned char redraw;
static struct ctk_widget *redraw_widgets[MAX_REDRAWWIDGETS];
static unsigned char redraw_widgetptr;
static unsigned char maxnitems;

static unsigned char iconx, icony;
#define ICONX_START  (width - 5)
#define ICONY_START  0
#define ICONX_DELTA  -8
#define ICONY_DELTA  5
#define ICONY_MAX    (height - 4)

static void idle(void);
static void sighandler(ek_signal_t s, ek_data_t data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("CTK Contiki GUI", idle, sighandler, NULL)};
static ek_id_t ctkid;


ek_signal_t ctk_signal_keypress,
  ctk_signal_timer,
  ctk_signal_button_activate,
  ctk_signal_button_hover,
  ctk_signal_hyperlink_activate,
  ctk_signal_hyperlink_hover,
  ctk_signal_menu_activate,
  ctk_signal_window_close;
		       

static unsigned short screensaver_timer;
#define SCREENSAVER_TIMEOUT (5*60)

#if CTK_CONF_MENUS
/*-----------------------------------------------------------------------------------*/
/* make_desktopmenu(void)
 *
 * Creates the leftmost menu, "Desktop". Since the desktop menu
 * contains the list of all open windows, this function will be called
 * whenever a window is opened or closed.
 */
static void
make_desktopmenu(void)
{
  struct ctk_window *w;
  
  desktopmenu.nitems = 0;
  
  if(windows == NULL) {
    ctk_menuitem_add(&desktopmenu, "(No windows)");
  } else {
    for(w = windows; w != NULL; w = w->next) {
      ctk_menuitem_add(&desktopmenu, w->title);
    }
  }
}
#endif /* CTK_CONF_MENUS */
/*-----------------------------------------------------------------------------------*/
/* ctk_init(void)
 *
 * Initializes CTK. Must be called before any other CTK function.
 */
void
ctk_init(void)
{
  ctkid = dispatcher_start(&p);
  
  windows = NULL;
  dialog = NULL;

#if CTK_CONF_MENUS 
  ctk_menu_new(&desktopmenu, "Desktop");
  make_desktopmenu();
  menus.menus = menus.desktopmenu = &desktopmenu;
#endif /* CTK_CONF_MENUS */

  ctk_draw_init();

  height = ctk_draw_height();
  width = ctk_draw_width();

  desktop_window.active = NULL;


  ctk_signal_keypress = dispatcher_sigalloc();
  ctk_signal_timer = dispatcher_sigalloc();
  ctk_signal_button_activate = dispatcher_sigalloc();
  ctk_signal_button_hover = dispatcher_sigalloc();
  ctk_signal_hyperlink_activate = dispatcher_sigalloc();
  ctk_signal_hyperlink_hover = dispatcher_sigalloc();
  ctk_signal_menu_activate = dispatcher_sigalloc();
  ctk_signal_window_close = dispatcher_sigalloc();
  
  dispatcher_listen(ctk_signal_timer);
  dispatcher_timer(ctk_signal_timer, NULL, CLK_TCK);

  mode = CTK_MODE_NORMAL;

  iconx = ICONX_START;
  icony = ICONY_START;
  
}
/*-----------------------------------------------------------------------------------*/
/* void ctk_mode_set()
 */
void
ctk_mode_set(unsigned char m) {
  mode = m;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_mode_get(void) {
  return mode;
}
/*-----------------------------------------------------------------------------------*/
void
ctk_icon_add(CC_REGISTER_ARG struct ctk_widget *icon,
	     ek_id_t id)
{
#if CTK_CONF_ICONS
  icon->x = iconx;
  icon->y = icony;
  icon->widget.icon.owner = id;

  icony += ICONY_DELTA;
  if(icony >= ICONY_MAX) {
    icony = ICONY_START;
    iconx += ICONX_DELTA;
  }
  
  ctk_widget_add(&desktop_window, icon);
#endif /* CTK_CONF_ICONS */
}
/*-----------------------------------------------------------------------------------*/
void
ctk_dialog_open(struct ctk_window *d)
{
  dialog = d;
}
/*-----------------------------------------------------------------------------------*/
void
ctk_dialog_close(void)
{
  dialog = NULL;
}
/*-----------------------------------------------------------------------------------*/
void
ctk_window_open(CC_REGISTER_ARG struct ctk_window *w)
{
  struct ctk_window *w2;
  
  /* Check if already open. */
  for(w2 = windows; w2 != w && w2 != NULL; w2 = w2->next);
  if(w2 == NULL) {
   /* Not open, so we add it at the head of the list of open
       windows. */
    w->next = windows;
    if(windows != NULL) {
      windows->prev = w;
    }
    windows = w;
    w->prev = NULL;
  } else {
    /* Window already open, so we move it to the front of the windows
       list. */
    if(w != windows) {
      if(w->next != NULL) {
	w->next->prev = w->prev;
      }
      if(w->prev != NULL) {
	w->prev->next = w->next;
      }
      w->next = windows;
      windows->prev = w;
      windows = w;
      w->prev = NULL;
    }
  }
  
#if CTK_CONF_MENUS
  /* Recreate the Desktop menu's window entries.*/
  make_desktopmenu();
#endif /* CTK_CONF_MENUS */
}
/*-----------------------------------------------------------------------------------*/
void
ctk_window_close(struct ctk_window *w)
{
  struct ctk_window *w2;

  if(w == NULL) {
    return;
  }
  
  /* Check if the window to be closed is the first window on the
     list. */
  if(w == windows) {
    windows = w->next;
    if(windows != NULL) {
      windows->prev = NULL;
    }
    w->next = w->prev = NULL;
  } else {
    /* Otherwise we step through the list until we find the window
       before the one to be closed. We then redirect its ->next
       pointer and its ->next->prev. */
    for(w2 = windows; w2->next != w; w2 = w2->next);

    if(w->next != NULL) {
      w->next->prev = w->prev;
    }
    w2->next = w->next;
    
    w->next = w->prev = NULL;
  }
  
#if CTK_CONF_MENUS
  /* Recreate the Desktop menu's window entries.*/
  make_desktopmenu();
#endif /* CTK_CONF_MENUS */
}
/*-----------------------------------------------------------------------------------*/
static void 
make_windowbuttons(CC_REGISTER_ARG struct ctk_window *window)
{
#if CTK_CONF_WINDOWMOVE
  CTK_BUTTON_NEW(&window->titlebutton, 0, -1, window->titlelen, window->title);
#else
  CTK_LABEL_NEW(&window->titlebutton, 0, -1, window->titlelen, 1, window->title);
#endif /* CTK_CONF_WINDOWMOVE */
  CTK_WIDGET_ADD(window, &window->titlebutton);


#if CTK_CONF_WINDOWCLOSE
  CTK_BUTTON_NEW(&window->closebutton, window->w - 3, -1, 1, "x");
#else
  CTK_LABEL_NEW(&window->closebutton, window->w - 4, -1, 3, 1, "   ");
#endif /* CTK_CONF_WINDOWCLOSE */  
  CTK_WIDGET_ADD(window, &window->closebutton);
}
/*-----------------------------------------------------------------------------------*/
void
ctk_window_clear(struct ctk_window *window)
{
  window->active = window->inactive = window->focused = NULL;
  
  make_windowbuttons(window);
}
/*-----------------------------------------------------------------------------------*/
void
ctk_menu_add(struct ctk_menu *menu)
{
#if CTK_CONF_MENUS
  struct ctk_menu *m;

  if(lastmenu == NULL) {
    lastmenu = menu;
  }
    
  for(m = menus.menus; m->next != NULL; m = m->next) {
    if(m == menu) {
      return;
    }
  }
  m->next = menu;
  menu->next = NULL;
#endif /* CTK_CONF_MENUS */
}
/*-----------------------------------------------------------------------------------*/
void
ctk_menu_remove(struct ctk_menu *menu)
{
#if CTK_CONF_MENUS
  struct ctk_menu *m;

  for(m = menus.menus; m->next != NULL; m = m->next) {
    if(m->next == menu) {
      m->next = menu->next;
      return;
    }
  }
#endif /* CTK_CONF_MENUS */
}
/*-----------------------------------------------------------------------------------*/
static void
do_redraw_all(unsigned char clipy1, unsigned char clipy2)
{
  struct ctk_window *w;
  struct ctk_widget *widget;

  if(mode != CTK_MODE_NORMAL &&
     mode != CTK_MODE_WINDOWMOVE) {
    return;
  }
  
  ctk_draw_clear(clipy1, clipy2);

  /* Draw widgets in root window */
  for(widget = desktop_window.active;
      widget != NULL; widget = widget->next) {
    ctk_draw_widget(widget, 0, clipy1, clipy2);
  }
  
  /* Draw windows */
  if(windows != NULL) {
    /* Find the last window.*/
    for(w = windows; w->next != NULL; w = w->next);

    /* Draw the windows from back to front. */
    for(; w != windows; w = w->prev) {
      ctk_draw_clear_window(w, 0, clipy1, clipy2);
      ctk_draw_window(w, 0, clipy1, clipy2);
    }
    /* Draw focused window */
    ctk_draw_clear_window(windows, CTK_FOCUS_WINDOW, clipy1, clipy2);
    ctk_draw_window(windows, CTK_FOCUS_WINDOW, clipy1, clipy2);
  }

  /* Draw dialog (if any) */
  if(dialog != NULL) {
    ctk_draw_dialog(dialog);
  }

#if CTK_CONF_MENUS
  ctk_draw_menus(&menus);
#endif /* CTK_CONF_MENUS */
}
/*-----------------------------------------------------------------------------------*/
void
ctk_redraw(void)
{
  if(DISPATCHER_CURRENT() == ctkid) {
    if(mode == CTK_MODE_NORMAL ||
       mode == CTK_MODE_WINDOWMOVE) {
      do_redraw_all(1, height);
    }
  } else {
    redraw |= REDRAW_ALL;
  }
}
/*-----------------------------------------------------------------------------------*/
void
ctk_window_redraw(struct ctk_window *w)
{
  /* Only redraw the window if it is a dialog or if it is the foremost
     window. */
  if(mode != CTK_MODE_NORMAL) {
    return;
  }
  
  if(w == dialog) {
    ctk_draw_dialog(w);
  } else if(dialog == NULL &&
#if CTK_CONF_MENUS
	    menus.open == NULL &&
#endif /* CTK_CONF_MENUS */		    
	    windows == w) {
    ctk_draw_window(w, CTK_FOCUS_WINDOW,
		    0, height);
  }  
}
/*-----------------------------------------------------------------------------------*/
static void
window_new(CC_REGISTER_ARG struct ctk_window *window,
	   unsigned char w, unsigned char h,
	   char *title)
{
  window->x = window->y = 0;
  window->w = w;
  window->h = h;
  window->title = title;
  if(title != NULL) {
    window->titlelen = strlen(title);
  } else {
    window->titlelen = 0;
  }
  window->next = window->prev = NULL;
  window->owner = DISPATCHER_CURRENT();
  window->active = window->inactive = window->focused = NULL;
}
/*-----------------------------------------------------------------------------------*/
void
ctk_window_new(struct ctk_window *window,
	       unsigned char w, unsigned char h,
	       char *title)
{
  window_new(window, w, h, title);

  make_windowbuttons(window);  
}
/*-----------------------------------------------------------------------------------*/
void
ctk_dialog_new(CC_REGISTER_ARG struct ctk_window *window,
	       unsigned char w, unsigned char h)
{
  window_new(window, w, h, NULL);

  window->x = (width - w) / 2;
  window->y = (height - h - 1) / 2; 
}
/*-----------------------------------------------------------------------------------*/
void
ctk_menu_new(CC_REGISTER_ARG struct ctk_menu *menu,
	     char *title)
{
#if CTK_CONF_MENUS
  menu->next = NULL;
  menu->title = title;
  menu->titlelen = strlen(title);
  menu->active = 0;
  menu->nitems = 0;
#endif /* CTK_CONF_MENUS */
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_menuitem_add(CC_REGISTER_ARG struct ctk_menu *menu,
		 char *name)
{
#if CTK_CONF_MENUS
  if(menu->nitems == CTK_CONF_MAXMENUITEMS) {
    return 0;
  }
  menu->items[menu->nitems].title = name;
  menu->items[menu->nitems].titlelen = strlen(name); 
  return menu->nitems++;
#else
  return 0;
#endif /* CTK_CONF_MENUS */
}
/*-----------------------------------------------------------------------------------*/
static void
add_redrawwidget(struct ctk_widget *w)
{
  static unsigned char i;
  
  if(redraw_widgetptr == MAX_REDRAWWIDGETS) {
    redraw |= REDRAW_FOCUS;
  } else {
    redraw |= REDRAW_WIDGETS;
    /* Check if it is in the queue already. If so, we don't add it
       again. */
    for(i = 0; i < redraw_widgetptr; ++i) {
      if(redraw_widgets[i] == w) {
	return;
      }
    }
    redraw_widgets[redraw_widgetptr++] = w;
  }
}
/*-----------------------------------------------------------------------------------*/
void
ctk_widget_redraw(struct ctk_widget *widget)
{
  struct ctk_window *window;

  if(mode != CTK_MODE_NORMAL) {
    return;
  }

  /* If this function isn't called by CTK itself, we only queue the
     redraw request. */
  if(DISPATCHER_CURRENT() != ctkid) {
    redraw |= REDRAW_WIDGETS;
    add_redrawwidget(widget);
  } else {
    
    /* Only redraw widgets that are in the foremost window. If we
       would allow redrawing widgets in non-focused windows, we would
       have to redraw all the windows that cover the non-focused
       window as well, which would lead to flickering.

       Also, we avoid drawing any widgets when the menus are active.
    */
    
#if CTK_CONF_MENUS
    if(menus.open == NULL)
#endif /* CTK_CONF_MENUS */
      {
	window = widget->window;
	if(window == dialog) {
	  ctk_draw_widget(widget, CTK_FOCUS_DIALOG, 0, height);
	} else if(window == windows) {
	  ctk_draw_widget(widget, CTK_FOCUS_WINDOW, 0, height);	      
	} else if(window == &desktop_window) {
	  ctk_draw_widget(widget, 0, 0, height);
	}
      }
  }
}
/*-----------------------------------------------------------------------------------*/
void
ctk_widget_add(CC_REGISTER_ARG struct ctk_window *window,
	       CC_REGISTER_ARG struct ctk_widget *widget)
{  
  if(widget->type == CTK_WIDGET_LABEL ||
     widget->type == CTK_WIDGET_SEPARATOR) {
    widget->next = window->inactive;
    window->inactive = widget;
    widget->window = window;
  } else {
    widget->next = window->active;
    window->active = widget;
    widget->window = window;
    if(window->focused == NULL) {
      window->focused = widget;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
static void
switch_focus_widget(unsigned char direction)
{
  register struct ctk_window *window;
  register struct ctk_widget *focus;
  struct ctk_widget *widget;
  
  
  if(dialog != NULL) {
    window = dialog;
  } else {
    window = windows;
  }

  /* If there are no windows open, we move focus around between the
     icons on the root window instead. */
  if(window == NULL) {
    window = &desktop_window;
  }
 
  focus = window->focused;
  add_redrawwidget(focus);
  
  if((direction & 1) == 0) {
    /* Move focus "up" */
    focus = focus->next;
  } else {
    /* Move focus "down" */
    for(widget = window->active;
	widget != NULL; widget = widget->next) {
	if(widget->next == focus) {
	  break;
	}      
    }    
    focus = widget;
    if(focus == NULL) {
      if(window->active != NULL) {	
	for(focus = window->active;
	    focus->next != NULL; focus = focus->next);	
      }
    }
  }
  if(focus == NULL) {
    focus = window->active;
  }
  
  if(focus != window->focused) {
    window->focused = focus;
    /* The operation changed the focus, so we emit a "hover" signal
       for those widgets that support it. */
    
    if(window->focused->type == CTK_WIDGET_HYPERLINK) {    
      dispatcher_emit(ctk_signal_hyperlink_hover, window->focused,
		      window->owner);
    } else if(window->focused->type == CTK_WIDGET_BUTTON) {    
      dispatcher_emit(ctk_signal_button_hover, window->focused,
		      window->owner);      
    } 
    
    add_redrawwidget(window->focused);
  }
}
/*-----------------------------------------------------------------------------------*/
#if CTK_CONF_MENUS
static void 
switch_open_menu(unsigned char rightleft)
{
  struct ctk_menu *menu;
  
  if(rightleft == 0) {
    /* Move right */
    for(menu = menus.menus; menu != NULL; menu = menu->next) {
      if(menu->next == menus.open) {
	break;
      }
    }
    lastmenu = menus.open;
    menus.open = menu;
    if(menus.open == NULL) {
      for(menu = menus.menus;
	  menu->next != NULL; menu = menu->next);
      menus.open = menu;
    }
  } else {
    /* Move to left */
    lastmenu = menus.open;
    menus.open = menus.open->next;
    if(menus.open == NULL) {
      menus.open = menus.menus;
    }
  }

  menus.open->active = 0;  
  /*  ctk_redraw();*/
}
/*-----------------------------------------------------------------------------------*/
static void
switch_menu_item(unsigned char updown)
{
  register struct ctk_menu *m;

  m = menus.open;
  
  if(updown == 0) {
    /* Move up */
    if(m->active == 0) {
      m->active = m->nitems - 1;
    } else {
      --m->active;
      if(m->items[m->active].title[0] == '-') {
	--m->active;
      }
    }
  } else {
    /* Move down */
    if(m->active >= m->nitems - 1) {
      m->active = 0;
    } else {      
      ++m->active;
      if(m->items[m->active].title[0] == '-') {
	++m->active;
      }
    }
  }
  
}
#endif /* CTK_CONF_MENUS */
/*-----------------------------------------------------------------------------------*/
static unsigned char 
activate(CC_REGISTER_ARG struct ctk_widget *w)
{
  static unsigned char len;
  
  if(w->type == CTK_WIDGET_BUTTON) {
    if(w == (struct ctk_widget *)&windows->closebutton) {
#if CTK_CONF_WINDOWCLOSE
      dispatcher_emit(ctk_signal_window_close, windows, w->window->owner);
      ctk_window_close(windows);
      return REDRAW_ALL;
#endif /* CTK_CONF_WINDOWCLOSE */
    } else if(w == (struct ctk_widget *)&windows->titlebutton) {
#if CTK_CONF_WINDOWCLOSE
      mode = CTK_MODE_WINDOWMOVE;
#endif /* CTK_CONF_WINDOWCLOSE */
    } else {
      dispatcher_emit(ctk_signal_button_activate, w,
		      w->window->owner);
    }
#if CTK_CONF_ICONS
  } else if(w->type == CTK_WIDGET_ICON) {
    dispatcher_emit(ctk_signal_button_activate, w,
		    w->widget.icon.owner);
#endif /* CTK_CONF_ICONS */
  } else if(w->type == CTK_WIDGET_HYPERLINK) {    
    dispatcher_emit(ctk_signal_hyperlink_activate, w,
		    DISPATCHER_BROADCAST);
  } else if(w->type == CTK_WIDGET_TEXTENTRY) {
    if(w->widget.textentry.state == CTK_TEXTENTRY_NORMAL) {      
      w->widget.textentry.state = CTK_TEXTENTRY_EDIT;
      len = strlen(w->widget.textentry.text);
      if(w->widget.textentry.xpos > len) {
	w->widget.textentry.xpos = len;
      }
    } else if(w->widget.textentry.state == CTK_TEXTENTRY_EDIT) {
      w->widget.textentry.state = CTK_TEXTENTRY_NORMAL;
    }
    add_redrawwidget(w);
    return REDRAW_WIDGETS;
  }
  return REDRAW_NONE;
}
/*-----------------------------------------------------------------------------------*/
static void
textentry_input(ctk_arch_key_t c,
		CC_REGISTER_ARG struct ctk_textentry *t)
{
  static char *cptr, *cptr2;
  static unsigned char len, txpos, typos, tlen;

  txpos = t->xpos;
  typos = t->ypos;
  tlen = t->len;

  cptr = &t->text[txpos + typos * tlen];
      
  switch(c) {
  case CH_CURS_LEFT:
    if(txpos > 0) {
      --txpos;
    }
    break;
    
  case CH_CURS_RIGHT:    
    if(txpos < tlen &&
       *cptr != 0) {
      ++txpos;
    }
    break;

  case CH_CURS_UP:
#if CTK_CONF_TEXTENTRY_MULTILINE
    if(t->h == 1) {
      txpos = 0;
    } else {
      if(typos > 0) {
	--typos;
      } else {
	t->state = CTK_TEXTENTRY_NORMAL;
      }
    }
#else
    txpos = 0;
#endif /* CTK_CONF_TEXTENTRY_MULTILINE */
    break;
    
  case CH_CURS_DOWN:
#if CTK_CONF_TEXTENTRY_MULTILINE
    if(t->h == 1) {
      txpos = strlen(t->text);
    } else {
      if(typos < t->h - 1) {
	++typos;
      } else {
	t->state = CTK_TEXTENTRY_NORMAL;
      }
    }
#else
    txpos = strlen(t->text);
#endif /* CTK_CONF_TEXTENTRY_MULTILINE */
    break;
    
  case CH_ENTER:
#if CTK_CONF_TEXTENTRY_MULTILINE
    if(t->h == 1) {
      t->state = CTK_TEXTENTRY_NORMAL;
    } else {
      if(typos < t->h - 1) {
	++typos;
	txpos = 0;
      } else {
	t->state = CTK_TEXTENTRY_NORMAL;
      }
    }
#else
    t->state = CTK_TEXTENTRY_NORMAL;
#endif /* CTK_CONF_TEXTENTRY_MULTILINE */
    break;
    
  default:
    len = tlen - txpos - 1;
    if(c == CH_DEL) {
      if(txpos > 0 && len > 0) {
	strncpy(cptr - 1, cptr,
		len);
	*(cptr + len - 1) = 0;
	--txpos;
      }
    } else {
      if(len > 0) {
	cptr2 = cptr + len - 1;
	while(cptr2 + 1 > cptr) {
	  *(cptr2 + 1) = *cptr2;
	  --cptr2;
	}
	
	*cptr = c;
	++txpos;
      }
    }
    break;
  }

  t->xpos = txpos;
  t->ypos = typos;
}
/*-----------------------------------------------------------------------------------*/
#if CTK_CONF_MENUS
static unsigned char
menus_input(ctk_arch_key_t c)
{
  struct ctk_window *w;

  if(menus.open->nitems > maxnitems) {
    maxnitems = menus.open->nitems;
  }

  
  switch(c) {
  case CH_CURS_RIGHT:
    switch_open_menu(1);
	
    return REDRAW_MENUPART;

  case CH_CURS_DOWN:
    switch_menu_item(1);
    return REDRAW_MENUS;

  case CH_CURS_LEFT:
    switch_open_menu(0);
    return REDRAW_MENUPART;

  case CH_CURS_UP:
    switch_menu_item(0);
    return REDRAW_MENUS;
    
  case CH_ENTER:
    lastmenu = menus.open;
    if(menus.open == &desktopmenu) {
      for(w = windows; w != NULL; w = w->next) {
	if(w->title == desktopmenu.items[desktopmenu.active].title) {
	  ctk_window_open(w);
	  menus.open = NULL;
	  return REDRAW_ALL;
	}
      }
    } else {
      dispatcher_emit(ctk_signal_menu_activate, menus.open,
		      DISPATCHER_BROADCAST);
    }
    menus.open = NULL;
    return REDRAW_MENUPART;

  case CTK_CONF_MENU_KEY:
    lastmenu = menus.open;
    menus.open = NULL;
    return REDRAW_MENUPART;
  }
}
#endif /* CTK_CONF_MENUS */
/*-----------------------------------------------------------------------------------*/
static void
idle(void)     
{
  static ctk_arch_key_t c;
  static unsigned char i;  
  register struct ctk_window *window;
  register struct ctk_widget *widget;

#if CTK_CONF_MENUS
  if(menus.open != NULL) {
    maxnitems = menus.open->nitems;
  } else {
    maxnitems = 0;
  }
#endif /* CTK_CONF_MENUS */
  
  if(mode == CTK_MODE_SCREENSAVER) {
#ifdef CTK_SCREENSAVER_RUN
    CTK_SCREENSAVER_RUN();
#endif /* CTK_SCREENSAVER_RUN */
    if(ctk_arch_keyavail()) {
      mode = CTK_MODE_NORMAL;
      ctk_draw_init();
      ctk_redraw();
    }
  } else if(mode == CTK_MODE_NORMAL) {  
    while(ctk_arch_keyavail()) {
      
      screensaver_timer = 0;
      
      c = ctk_arch_getkey();
      
      
      if(dialog != NULL) {
	window = dialog;
      } else if(windows != NULL) {
	window = windows;
      } else {
	window = &desktop_window;
      }      
      widget = window->focused;

	  
      if(widget != NULL &&
	 widget->type == CTK_WIDGET_TEXTENTRY &&
	 widget->widget.textentry.state == CTK_TEXTENTRY_EDIT) {
	textentry_input(c, (struct ctk_textentry *)widget);
	add_redrawwidget(widget);
#if CTK_CONF_MENUS
      } else if(menus.open != NULL) {
	redraw |= menus_input(c);
#endif /* CTK_CONF_MENUS */
      } else {      
	switch(c) {
	case CH_CURS_RIGHT:
	  switch_focus_widget(RIGHT);
	  break;
	case CH_CURS_DOWN:
	  switch_focus_widget(DOWN);
	  break;
	case CH_CURS_LEFT:
	  switch_focus_widget(LEFT);
	  break;
	case CH_CURS_UP:
	  switch_focus_widget(UP);
	  break;
	case CH_ENTER:
	  redraw |= activate(widget);
	  break;
#if CTK_CONF_MENUS
	case CTK_CONF_MENU_KEY:
	  if(dialog == NULL) {
	    if(lastmenu == NULL) {
	      menus.open = menus.menus;
	    } else {
	      menus.open = lastmenu;
	    }
	    menus.open->active = 0;
	    redraw |= REDRAW_MENUS;
	  } 
	  break;
#endif /* CTK_CONF_MENUS */
	case CTK_CONF_WINDOWSWITCH_KEY:
	  if(windows != NULL) {
	    for(window = windows; window->next != NULL;
		window = window->next);
	    ctk_window_open(window);
	    ctk_redraw();
	  }
	  break;
	default:
	  if(widget->type == CTK_WIDGET_TEXTENTRY) {
	    widget->widget.textentry.state = CTK_TEXTENTRY_EDIT;
	    textentry_input(c, (struct ctk_textentry *)widget);
	    add_redrawwidget(widget);
	  } else {
	    dispatcher_emit(ctk_signal_keypress, (void *)c,
			    window->owner);
	  }
	  break;
	}
      }

      if(redraw & REDRAW_WIDGETS) {
	for(i = 0; i < redraw_widgetptr; ++i) {
	  ctk_widget_redraw(redraw_widgets[i]);
	}
	redraw &= ~REDRAW_WIDGETS;
	redraw_widgetptr = 0;
      }
    }
    if(redraw & REDRAW_ALL) {
      do_redraw_all(1, height);
#if CTK_CONF_MENUS
    } else if(redraw & REDRAW_MENUPART) {
      do_redraw_all(1, maxnitems + 1);
    } else if(redraw & REDRAW_MENUS) {
      ctk_draw_menus(&menus);
#endif /* CTK_CONF_MENUS */
    } else if(redraw & REDRAW_FOCUS) {
      if(dialog != NULL) {
	ctk_window_redraw(dialog);
      } else if(windows != NULL) {
	ctk_window_redraw(windows);	
      } else {
	ctk_window_redraw(&desktop_window);	
      }
    } else if(redraw & REDRAW_WIDGETS) {
      for(i = 0; i < redraw_widgetptr; ++i) {
	ctk_widget_redraw(redraw_widgets[i]);
      }
    }    
    redraw = 0;
    redraw_widgetptr = 0;
#if CTK_CONF_WINDOWMOVE
  } else if(mode == CTK_MODE_WINDOWMOVE) {

    redraw = 0;

    window = windows;
    
    while(mode == CTK_MODE_WINDOWMOVE && ctk_arch_keyavail()) {
    
      screensaver_timer = 0;
      
      c = ctk_arch_getkey();
      
      switch(c) {
      case CH_CURS_RIGHT:
	++window->x;
	if(window->x + window->w + 1 >= width) {
	  --window->x;
	}
	redraw = REDRAW_ALL;
	break;
      case CH_CURS_LEFT:
	if(window->x > 0) {
	  --window->x;
	}
	redraw = REDRAW_ALL;
	break;
      case CH_CURS_DOWN:
	++window->y;
	if(window->y + window->h + 2 >= height) {
	  --window->y;
	}
	redraw = REDRAW_ALL;
	break;
      case CH_CURS_UP:
	if(window->y > 0) {
	  --window->y;
	}
	redraw = REDRAW_ALL;
	break;
      case CH_ENTER:
      case CH_ESC:
	mode = CTK_MODE_NORMAL;
	redraw = REDRAW_ALL;
	break;
      }
    }
    if(redraw & REDRAW_ALL) {
      do_redraw_all(1, height);
    }
    redraw = 0;
#endif /* CTK_CONF_WINDOWMOVE */
  }
}
/*-----------------------------------------------------------------------------------*/
static void
sighandler(ek_signal_t s, ek_data_t data)
{
  if(s == ctk_signal_timer) {
    if(mode == CTK_MODE_NORMAL) {
      ++screensaver_timer;
      if(screensaver_timer == SCREENSAVER_TIMEOUT) {
#ifdef CTK_SCREENSAVER_INIT
	CTK_SCREENSAVER_INIT();
#endif /* CTK_SCREENSAVER_INIT */
	mode = CTK_MODE_SCREENSAVER;
	screensaver_timer = 0;
      }
    }
    dispatcher_timer(ctk_signal_timer, data, CLK_TCK);
  }
}
/*-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------*/
