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
 * $Id: ctk-vncserver.c,v 1.3 2003/08/24 22:38:44 adamdunkels Exp $
 *
 */

#include "ctk.h"
#include "ctk-draw.h"

#include "dispatcher.h"
#include "loader.h"
#include "vnc-server.h"
#include "vnc-out.h"

#include "ctk-vncserver.h"

static unsigned char sizex, sizey;

#define CH_ULCORNER 0x00
#define CH_TITLEBAR 0x01
#define CH_URCORNER 0x02
#define CH_WINDOWRBORDER 0x03
#define CH_LRCORNER 0x04
#define CH_WINDOWLOWERBORDER 0x05
#define CH_LLCORNER 0x06
#define CH_WINDOWLBORDER 0x07

#define CH_DIALOG_ULCORNER 0x12
#define CH_DIALOGUPPERBORDER 0x09
#define CH_DIALOG_URCORNER 0x0a
#define CH_DIALOGRBORDER 0x0b
#define CH_DIALOG_LRCORNER 0x0c
#define CH_DIALOGLOWERBORDER 0x0d
#define CH_DIALOG_LLCORNER 0x0e
#define CH_DIALOGLBORDER 0x0f

#define CH_BUTTONLEFT  0x10
#define CH_BUTTONRIGHT 0x11

#define CH_SEPARATOR   0x13

#include "libconio.h"

#define SCREENCOLOR         0
#define BORDERCOLOR         1

#define WIDGETCOLOR         2
#define WIDGETCOLOR_FWIN    3
#define WIDGETCOLOR_FOCUS   4
#define WIDGETCOLOR_DIALOG  5
#define WIDGETCOLOR_HLINK   6
#define WIDGETCOLOR_HLINK_FOCUS   7

#define WINDOWCOLOR         8
#define WINDOWCOLOR_FOCUS   9

#define WINDOWBORDER        10
#define WINDOWBORDER_FOCUS  11

#define DIALOGCOLOR         12

#define OPENMENUCOLOR       13

#define ACTIVEMENUITEMCOLOR 14

#define MENUCOLOR           15

static DISPATCHER_UIPCALL(ctk_vncserver_appcall, state);

static struct dispatcher_proc p =
  {DISPATCHER_PROC("CTK VNC server", NULL, NULL,
		   ctk_vncserver_appcall)};
static ek_id_t id;


#define VNCSERVER_CONF_NUMCONNS 8
static struct vnc_server_state conns[VNCSERVER_CONF_NUMCONNS];

#define PRINTF(x) 

#define revers(x)


/*-----------------------------------------------------------------------------------*/
void
vnc_server_update_add(struct vnc_server_state *vs,
		      struct vnc_server_update *a)
{
  /* XXX: test both head and tail placement!*/
  a->next = vs->updates_pending;
  vs->updates_pending = a;
}
struct vnc_server_update *
vnc_server_update_alloc(struct vnc_server_state *vs)
{
  struct vnc_server_update *a;

  a = vs->updates_free;
  if(a == NULL) {
    return NULL;
  }
  vs->updates_free = a->next;
  a->next = NULL;
  return a;
}
void
vnc_server_update_free(struct vnc_server_state *vs,
		       struct vnc_server_update *a)
{
  a->next = vs->updates_free;
  vs->updates_free = a;
}
struct vnc_server_update *
vnc_server_update_dequeue(struct vnc_server_state *vs)
{
  struct vnc_server_update *a;

  a = vs->updates_pending;
  if(a == NULL) {
    return a;
  }
  vs->updates_pending = a->next;
  a->next = NULL;
  return a;
}
void
vnc_server_update_remove(struct vnc_server_state *vs,
			 struct vnc_server_update *a)
{
  struct vnc_server_update *b, *c;

  if(a == vs->updates_pending) {
    vs->updates_pending = a->next;
  } else {
    
    for(c = vs->updates_pending; c != a; b = c, c = c->next);

    b->next = a->next;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
update_area(u8_t x, u8_t y, u8_t w, u8_t h)
{
  u8_t i;
  
  if(h == 0 || w == 0) {
    return;
  }
  
  /* Update for all active VNC connections. */
  for(i = 0; i < VNCSERVER_CONF_NUMCONNS; ++i) {
    if(conns[i].state != VNC_DEALLOCATED) {
      vnc_out_update_area(&conns[i],
			  x, y, w, h);
    }
  }

}
/*-----------------------------------------------------------------------------------*/
static struct vnc_server_state *
alloc_state(void)
{
  u8_t i;
  for(i = 0; i < VNCSERVER_CONF_NUMCONNS; ++i) {
    if(conns[i].state == VNC_DEALLOCATED) {
      return &conns[i];
    }
  }

  /* We are overloaded! XXX: we'll just kick all other connections! */
  for(i = 0; i < VNCSERVER_CONF_NUMCONNS; ++i) {
    conns[i].state = VNC_DEALLOCATED;
  }
  
  return NULL;
}
/*-----------------------------------------------------------------------------------*/
static void
dealloc_state(struct vnc_server_state *s)
{
  s->state = VNC_DEALLOCATED;
}
/*-----------------------------------------------------------------------------------*/
static char tmp[40];
static void
cputsn(char *str, unsigned char len)
{
  strncpy(tmp, str, len);
  tmp[len] = 0;
  cputs(tmp);
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_init(void)
{
  bgcolor(SCREENCOLOR);
  bordercolor(BORDERCOLOR);
  screensize(&sizex, &sizey);
  ctk_draw_clear(0, sizey);
}
/*-----------------------------------------------------------------------------------*/
static void
draw_widget(struct ctk_widget *w,
	    unsigned char x, unsigned char y,
	    unsigned char clipx,
	    unsigned char clipy,
	    unsigned char clipy1, unsigned char clipy2,
	    unsigned char focus)
{
  unsigned char xpos, ypos, xscroll;
  unsigned char i, j;
  unsigned char iconnum;
  char c, *text;
  unsigned char len;

  /*
  if(focus & CTK_FOCUS_WINDOW) {    
    textcolor(WIDGETCOLOR_FWIN);
    if(focus & CTK_FOCUS_WIDGET) {
      textcolor(WIDGETCOLOR_FOCUS);
    }
  } else if(focus & CTK_FOCUS_DIALOG) {
    textcolor(WIDGETCOLOR_DIALOG);
    if(focus & CTK_FOCUS_WIDGET) {
      textcolor(WIDGETCOLOR_FOCUS);
    }
  } else {
    textcolor(WIDGETCOLOR);
  }
*/  
  xpos = x + w->x;
  ypos = y + w->y;
    
  switch(w->type) {
  case CTK_WIDGET_SEPARATOR:
    textcolor(VNC_OUT_SEPARATORCOLOR + focus);
    if(ypos >= clipy1 && ypos < clipy2) {
      /*      chlinexy(xpos, ypos, w->w);*/
      gotoxy(xpos, ypos);
      for(i = 0; i < w->w; ++i) {
	cputc(CH_SEPARATOR);
      }
    }
    break;
  case CTK_WIDGET_LABEL:
    textcolor(VNC_OUT_LABELCOLOR + focus);
    text = w->widget.label.text;
    for(i = 0; i < w->h; ++i) {
      if(ypos >= clipy1 && ypos < clipy2) {
	gotoxy(xpos, ypos);
	cputsn(text, w->w);
	if(w->w - (wherex() - xpos) > 0) {
	  cclear(w->w - (wherex() - xpos));
	}
      }
      ++ypos;
      text += w->w;
    }
    break;
  case CTK_WIDGET_BUTTON:
    textcolor(VNC_OUT_BUTTONCOLOR + focus);
    if(ypos >= clipy1 && ypos < clipy2) {
      if(focus & CTK_FOCUS_WIDGET) {
	revers(1);
      } else {
	revers(0);
      }
      cputcxy(xpos, ypos, CH_BUTTONLEFT);
      cputsn(w->widget.button.text, w->w);
      cputc(CH_BUTTONRIGHT);
      revers(0);
    }
    break;
  case CTK_WIDGET_HYPERLINK:
    textcolor(VNC_OUT_HYPERLINKCOLOR + focus);
    if(ypos >= clipy1 && ypos < clipy2) {
      /*      if(focus & CTK_FOCUS_WIDGET) {
	textcolor(WIDGETCOLOR_HLINK_FOCUS);
	revers(0);	
      } else {
	textcolor(WIDGETCOLOR_HLINK);
	revers(1);
	}*/
      gotoxy(xpos, ypos);
      cputsn(w->widget.button.text, w->w);
      revers(0);
    }
    break;
  case CTK_WIDGET_TEXTENTRY:
    textcolor(VNC_OUT_TEXTENTRYCOLOR + focus);
    text = w->widget.textentry.text;
    if(focus & CTK_FOCUS_WIDGET) {
      revers(1);
    } else {
      revers(0);
    }
    xscroll = 0;
    if(w->widget.textentry.xpos >= w->w - 1) {
      xscroll = w->widget.textentry.xpos - w->w + 1;
    }
    for(j = 0; j < w->h; ++j) {
      if(ypos >= clipy1 && ypos < clipy2) {
	if(w->widget.textentry.state == CTK_TEXTENTRY_EDIT &&
	   w->widget.textentry.ypos == j) {
	  revers(0);
	  cputcxy(xpos, ypos, '>');
	  for(i = 0; i < w->w; ++i) {
	    c = text[i + xscroll];
	    if(i == w->widget.textentry.xpos - xscroll) {
	      textcolor(VNC_OUT_TEXTENTRYCOLOR + (focus ^ 0x01));
	      revers(1);
	    } else {
	      revers(0);
	    }
	    if(c == 0) {
	      cputc(' ');
	    } else {
	      cputc(c);
	    }
	    revers(0);
	    textcolor(VNC_OUT_TEXTENTRYCOLOR + focus);
	  }
	  cputc('<');
	} else {
	  cvlinexy(xpos, ypos, 1);
	  gotoxy(xpos + 1, ypos);          
	  cputsn(text, w->w);
	  i = wherex();
	  if(i - xpos - 1 < w->w) {
	    cclear(w->w - (i - xpos) + 1);
	  }
	  cvline(1);
	}
      }
      ++ypos;
      text += w->w;
    }
    revers(0);
    break;
  case CTK_WIDGET_ICON:
    if(ypos >= clipy1 && ypos < clipy2) {
      textcolor(VNC_OUT_ICONCOLOR + focus);
      if(focus & 1) {
	revers(1);
      } else {
	revers(0);
      }

      x = xpos;
      len = strlen(w->widget.icon.title);
      if(x + len >= sizex) {
	x = sizex - len;
      }

      gotoxy(x, ypos + 3);
      if(ypos >= clipy1 && ypos < clipy2) {
	cputs(w->widget.icon.title);
      }

      gotoxy(xpos, ypos);
      if(w->widget.icon.bitmap != NULL) {
	iconnum = vnc_out_add_icon((struct ctk_icon *)w);
	gotoxy(xpos, ypos);
	textcolor(iconnum | (focus << 6));
	cputc(0x80);
	cputc(0x81);
	cputc(0x82);
	cputc(0x83);
	++ypos;
	gotoxy(xpos, ypos);
	cputc(0x90);
	cputc(0x91);
	cputc(0x92);
	cputc(0x93);
	++ypos;
	gotoxy(xpos, ypos);
	cputc(0xa0);
	cputc(0xa1);
	cputc(0xa2);
	cputc(0xa3);
	++ypos;
	textcolor(0);
	/*	for(i = 0; i < 3; ++i) {

	  if(ypos >= clipy1 && ypos < clipy2) {
	    cputc(w->widget.icon.textmap[0 + 3 * i]);
	    cputc(w->widget.icon.textmap[1 + 3 * i]);
	    cputc(w->widget.icon.textmap[2 + 3 * i]);
	  }
	  ++ypos;
	  }*/
      }
      x = xpos;
  
      revers(0);
    }
    break;

  default:
    break;
  }
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_widget(struct ctk_widget *w,
		unsigned char focus,
		unsigned char clipy1,
		unsigned char clipy2)
{
  struct ctk_window *win = w->window;
  struct ctk_icon *icon;
  unsigned char posx, posy, x, len;

  posx = win->x + 1;
  posy = win->y + 2;

  if(w == win->focused) {
    focus |= CTK_FOCUS_WIDGET;
  }
  
  draw_widget(w, posx, posy,
	      posx + win->w,
	      posy + win->h,
	      clipy1, clipy2,
	      focus);

  if(w->type != CTK_WIDGET_ICON) {
    update_area(posx + w->x,
		posy + w->y, w->w + 2, w->h);
  } else {
    icon = (struct ctk_icon *)w;

    len = strlen(icon->title);
    x = posx + w->x;
    if(x + len >= sizex) {
      x = sizex - len;
    }

    update_area(x, posy + w->y, len > 4? len: 4, w->h);    
  }
  
#ifdef CTK_CONIO_CONF_UPDATE
  CTK_CONIO_CONF_UPDATE();
#endif /* CTK_CONIO_CONF_UPDATE */
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_clear_window(struct ctk_window *window,
		      unsigned char focus,
		      unsigned char clipy1,
		      unsigned char clipy2)
{
  unsigned char i;
  unsigned char h;
  /*
  if(focus & CTK_FOCUS_WINDOW){
    textcolor(WINDOWCOLOR_FOCUS);
  } else {
    textcolor(WINDOWCOLOR);
    }*/
  textcolor(VNC_OUT_WINDOWCOLOR + focus);
  
  h = window->y + 2 + window->h;
  /* Clear window contents. */
  for(i = window->y + 2; i < h; ++i) {
    if(i >= clipy1 && i < clipy2) {
      cclearxy(window->x + 1, i, window->w);
    }
  }

  update_area(window->x + 1, window->y + 2, window->w, window->h);
}
/*-----------------------------------------------------------------------------------*/
static void
draw_window_contents(struct ctk_window *window, unsigned char focus,
		     unsigned char clipy1, unsigned char clipy2,
		     unsigned char x1, unsigned char x2,
		     unsigned char y1, unsigned char y2)
{
  struct ctk_widget *w;
  unsigned char wfocus;
  
  /* Draw inactive widgets. */
  for(w = window->inactive; w != NULL; w = w->next) {
    draw_widget(w, x1, y1, x2, y2,
		clipy1, clipy2,
		focus);
  }
  
  /* Draw active widgets. */
  for(w = window->active; w != NULL; w = w->next) {  
    wfocus = focus;
    if(w == window->focused) {
      wfocus |= CTK_FOCUS_WIDGET;
    }

   draw_widget(w, x1, y1, x2, y2, 
	       clipy1, clipy2,
	       wfocus);
  }

#ifdef CTK_CONIO_CONF_UPDATE
  CTK_CONIO_CONF_UPDATE();
#endif /* CTK_CONIO_CONF_UPDATE */

}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_window(struct ctk_window *window, unsigned char focus,
		unsigned char clipy1, unsigned char clipy2)
{
  unsigned char x, y;
  unsigned char h;
  unsigned char x1, y1, x2, y2;
  unsigned char i;
  

  if(window->y + 1 >= clipy2) {
    return;
  }
    
  x = window->x;
  y = window->y + 1;

  textcolor(VNC_OUT_WINDOWCOLOR + focus);
  /*  if(focus & CTK_FOCUS_WINDOW) {
    textcolor(WINDOWCOLOR_FOCUS);
  } else {
    textcolor(WINDOWCOLOR);
    }*/

  x1 = x + 1;
  y1 = y + 1;
  x2 = x1 + window->w;
  y2 = y1 + window->h;

  /* Draw window frame. */  
  if(y >= clipy1) {
    cputcxy(x, y, CH_ULCORNER);
    for(i = wherex() + window->titlelen + 2; i < x2; ++i) {
      cputcxy(i, y, CH_TITLEBAR);
    }
    cputcxy(x2, y, CH_URCORNER);
  }

  h = window->h;
  
  if(clipy1 > y1) {
    if(clipy1 - y1 < h) {
      h = clipy1 - y1;
            y1 = clipy1;
    } else {
      h = 0;
    }
  }

  if(clipy2 < y1 + h) {
    if(y1 >= clipy2) {
      h = 0;
    } else {
      h = clipy2 - y1;
    }
  }
  
  for(i = y1; i < y1 + h; ++i) {
    cputcxy(x, i, CH_WINDOWLBORDER);
    cputcxy(x2, i, CH_WINDOWRBORDER);
  }

  /*  cvlinexy(x, y1, h);
      cvlinexy(x2, y1, h);  */

  if(y + window->h >= clipy1 &&
     y + window->h < clipy2) {
    cputcxy(x, y2, CH_LLCORNER);
    for(i = x1; i < x2; ++i) {
      cputcxy(i, y2, CH_WINDOWLOWERBORDER);
    }
    /*    chlinexy(x1, y2, window->w);*/
    cputcxy(x2, y2, CH_LRCORNER);
  }

  draw_window_contents(window, focus & CTK_FOCUS_WINDOW, clipy1, clipy2,
		       x1, x2, y + 1, y2);

  update_area(window->x, window->y, window->w + 2, window->h + 2);
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_dialog(struct ctk_window *dialog)
{
  unsigned char x, y;
  unsigned char i;
  unsigned char x1, y1, x2, y2;

  /*  textcolor(DIALOGCOLOR);*/
  textcolor(VNC_OUT_WINDOWCOLOR + CTK_FOCUS_DIALOG);

  x = dialog->x;
  y = dialog->y + 1;


  x1 = x + 1;
  y1 = y + 1;
  x2 = x1 + dialog->w;
  y2 = y1 + dialog->h;


  /* Draw dialog frame. */
  
  for(i = y1; i < y1 + dialog->h; ++i) {
    cputcxy(x, i, CH_DIALOGLBORDER);
    cputcxy(x2, i, CH_DIALOGRBORDER);
  }
  /*  cvlinexy(x, y1,
	   dialog->h);
  cvlinexy(x2, y1,
  dialog->h);*/

  
  for(i = x1; i < x2; ++i) {
    cputcxy(i, y, CH_DIALOGUPPERBORDER);
    cputcxy(i, y2, CH_DIALOGLOWERBORDER);
  }
    /*  chlinexy(x1, y,
	   dialog->w);
  chlinexy(x1, y2,
  dialog->w);*/

  cputcxy(x, y, CH_DIALOG_ULCORNER);
  cputcxy(x, y2, CH_DIALOG_LLCORNER);
  cputcxy(x2, y, CH_DIALOG_URCORNER);
  cputcxy(x2, y2, CH_DIALOG_LRCORNER);
  
  
  /* Clear dialog contents. */
  for(i = y1; i < y2; ++i) {
    cclearxy(x1, i, dialog->w);
  }

  draw_window_contents(dialog, CTK_FOCUS_DIALOG, 0, sizey,
		       x1, x2, y1, y2);

  update_area(dialog->x, dialog->y, dialog->w + 4, dialog->h + 4);
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_clear(unsigned char y1, unsigned char y2)
{
  unsigned char i;

  textcolor(VNC_OUT_BACKGROUNDCOLOR);
  for(i = y1; i < y2; ++i) {
    cclearxy(0, i, sizex);
  }

  update_area(0, y1, sizex, y2 - y1);
}
/*-----------------------------------------------------------------------------------*/
static void
draw_menu(struct ctk_menu *m)
{
  unsigned char x, x2, y;

  textcolor(VNC_OUT_MENUCOLOR);
  x = wherex();
  cputs(m->title);
  cputc(' ');
  x2 = wherex();
  if(x + CTK_CONF_MENUWIDTH > sizex) {
    x = sizex - CTK_CONF_MENUWIDTH;
  }
  
  
  for(y = 0; y < m->nitems; ++y) {
    if(y == m->active) {
      textcolor(VNC_OUT_ACTIVEMENUCOLOR);
      revers(0);
    } else {
      textcolor(VNC_OUT_MENUCOLOR);	  
    }
    gotoxy(x, y + 1);
    if(m->items[y].title[0] == '-') {
      chline(CTK_CONF_MENUWIDTH);
    } else {
      cputs(m->items[y].title);
    }
    if(x + CTK_CONF_MENUWIDTH > wherex()) {
      cclear(x + CTK_CONF_MENUWIDTH - wherex());
    }
    revers(1);
  }
  
  gotoxy(x2, 0);
  textcolor(VNC_OUT_MENUCOLOR);  

  update_area(x, 0, CTK_CONF_MENUWIDTH, m->nitems + 1);
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_menus(struct ctk_menus *menus)
{
  struct ctk_menu *m;  

  
  /* Draw menus */
  textcolor(VNC_OUT_MENUCOLOR);
  gotoxy(0, 0);
  revers(1);
  cputc(' ');
  for(m = menus->menus->next; m != NULL; m = m->next) {
    if(m != menus->open) {
      update_area(wherex(), 0, strlen(m->title) + 1, 1);
      cputs(m->title);
      cputc(' ');
    } else {
      draw_menu(m);
    }
  }


  if(wherex() + strlen(menus->desktopmenu->title) + 1>= sizex) {
    gotoxy(sizex - strlen(menus->desktopmenu->title) - 1, 0);
  } else {
    cclear(sizex - wherex() -
	   strlen(menus->desktopmenu->title) - 1);
    update_area(wherex(), 0, sizex - wherex() -
		strlen(menus->desktopmenu->title) - 1, 1);
  }
  
  /* Draw desktopmenu */
  if(menus->desktopmenu != menus->open) {
    update_area(wherex(), 0, strlen(menus->desktopmenu->title) + 1, 1);
    cputs(menus->desktopmenu->title);
    cputc(' ');
  } else {
    draw_menu(menus->desktopmenu);
  }

  revers(0);



}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_draw_height(void)
{
  return sizey;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_draw_width(void)
{
  return sizex;
}
/*-----------------------------------------------------------------------------------*/
static unsigned char
ascii2screen(unsigned char c)
{
  if(c == '|') {
    return 0x68;
  }
  if(c < 0x20) {
    return c + 0x60;
  }
  if(c > 0x20 && c < 0x40) {
    return c;
  }
  if(c >= 0x40 && c < 0x60) {
    return c;
  }
  if(c >= 0x60 && c < 0x80) {
    return c - 0x60;
  }
  if(c >= 0x80) {
    return c;
  }

  return 32;
}

void
ctk_arch_draw_char(char c,
		   unsigned char xpos,
		   unsigned char ypos,
		   unsigned char reversedflag,
		   unsigned char color)
{

  vnc_out_update_screen(xpos, ypos, ascii2screen(c),
			color);
  /*  vnc_out_update_screen(xpos, ypos, c |
      (reversedflag? 0x80: 0));*/
}

unsigned char
ctk_arch_keyavail(void)
{
  return vnc_out_keyavail();
}

ctk_arch_key_t
ctk_arch_getkey(void)
{
  return vnc_out_getkey() & 0x7f;
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_UIPCALL(ctk_vncserver_appcall, state)
{
  static struct vnc_server_state *vs;
  DISPATCHER_UIPCALL_ARG(state);

  vs = (struct vnc_server_state *)(state);

  if(uip_connected()) {

    /* Since we've just been connected, the state pointer should be
       NULL and we need to allocate a new state object. If we have run
       out of memory for state objects, we'll have to abort the
       connection and return. */
    if(vs == NULL) {
      vs = alloc_state();
      if(vs == NULL) {
	uip_close();
	return;
      }
      dispatcher_markconn(uip_conn, (void *)vs);
    }
  } else if(uip_closed() || uip_aborted()) {
    if(vs != NULL) {
      dealloc_state(vs);
    }
    return;
  }
  vnc_server_appcall(vs);
}
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(ctk_vncserver_init, arg)
{
  u8_t i;

  arg_free(arg);
  
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    dispatcher_uiplisten(5900);

    for(i = 0; i < VNCSERVER_CONF_NUMCONNS; ++i) {
      conns[i].state = VNC_DEALLOCATED;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
