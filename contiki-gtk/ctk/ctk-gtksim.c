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
 * $Id: ctk-gtksim.c,v 1.1 2003/04/02 09:17:27 adamdunkels Exp $
 *
 */

/* This file provides a very simple implementation of CTK using the
   GTK (Gimp toolkit) under FreeBSD/Linux. */

#include <gtk/gtk.h>

#include "ctk.h"
#include "ctk-draw.h"

#include <string.h>

#define FONT_HEIGHT 20
#define FONT_WIDTH  16

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 25

static unsigned char sizex = SCREEN_WIDTH;
static unsigned char sizey = SCREEN_HEIGHT;

static char screen[SCREEN_WIDTH * SCREEN_HEIGHT];

static unsigned char cursx, cursy;
static unsigned char reversed;

/* Color defines */
#define COLOR_BLACK             0x00
#define COLOR_WHITE             0x01
#define COLOR_RED               0x02
#define COLOR_CYAN              0x03
#define COLOR_VIOLET            0x04
#define COLOR_GREEN             0x05
#define COLOR_BLUE              0x06
#define COLOR_YELLOW            0x07
#define COLOR_ORANGE            0x08
#define COLOR_BROWN             0x09
#define COLOR_LIGHTRED          0x0A
#define COLOR_GRAY1             0x0B
#define COLOR_GRAY2             0x0C
#define COLOR_LIGHTGREEN        0x0D
#define COLOR_LIGHTBLUE         0x0E
#define COLOR_GRAY3             0x0F

/*#define CH_ULCORNER '/'
#define CH_URCORNER '\\'
#define CH_LLCORNER '\\'
#define CH_LRCORNER '/'*/
#define CH_ULCORNER '+'
#define CH_URCORNER '+'
#define CH_LLCORNER '+'
#define CH_LRCORNER '+'

static GdkPixmap *pixmap = NULL;
static GtkWidget *drawing_area;
static GdkFont *font;


static void draw_screen(void);

/*-----------------------------------------------------------------------------------*/
static void
redraw(void)
{
  draw_screen();
  gtk_widget_draw(drawing_area, NULL);
}
/*-----------------------------------------------------------------------------------*/
static unsigned char
wherex(void)
{
  return cursx;
}
/*-----------------------------------------------------------------------------------*/
static void
clrscr(void)
{
  int i;
  for(i = 0; i < sizeof(screen); ++i) {
    screen[i] = ' ';
  }
  redraw();
}
/*-----------------------------------------------------------------------------------*/
static void
revers(unsigned char c)
{
  reversed = c;
}
/*-----------------------------------------------------------------------------------*/
static void
cputc(char c)
{
  if(reversed) {
    screen[cursy * SCREEN_WIDTH + cursx] = c | 0x80;
  } else {
    screen[cursy * SCREEN_WIDTH + cursx] = c;
  }
  ++cursx;
}
/*-----------------------------------------------------------------------------------*/
static void
cputs(char *str)
{
  int i;
  for(i = 0; i < strlen(str); ++i) {
    cputc(str[i]);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
cclear(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    cputc(' ');
  }  
}
/*-----------------------------------------------------------------------------------*/
static void
chline(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    cputc('-');
  }
}
/*-----------------------------------------------------------------------------------*/
static void
cvline(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    screen[cursy * SCREEN_WIDTH + cursx] = '|';
    ++cursy;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
gotoxy(unsigned char x, unsigned char y)
{
  cursx = x;
  cursy = y;
}
/*-----------------------------------------------------------------------------------*/
static void
cclearxy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  cclear(length);
}
/*-----------------------------------------------------------------------------------*/
static void
chlinexy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  chline(length);
}
/*-----------------------------------------------------------------------------------*/
static void
cvlinexy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  cvline(length);
}
/*-----------------------------------------------------------------------------------*/
static void
cputsxy(unsigned char x, unsigned char y, char *str)
{
  gotoxy(x, y);
  cputs(str);
}
/*-----------------------------------------------------------------------------------*/
static void
cputcxy(unsigned char x, unsigned char y, char c)
{
  gotoxy(x, y);
  cputc(c);
}
/*-----------------------------------------------------------------------------------*/
#define textcolor(c)
/*-----------------------------------------------------------------------------------*/
#define NUMKEYS 100
static ctk_arch_key_t keys[NUMKEYS];
static int firstkey, lastkey;

char
ctk_arch_keyavail(void)
{
  return firstkey != lastkey;
}
/*-----------------------------------------------------------------------------------*/
ctk_arch_key_t
ctk_arch_getkey(void)
{
  ctk_arch_key_t key;
  key = keys[firstkey];

  if(firstkey != lastkey) {
    ++firstkey;
    if(firstkey >= NUMKEYS) {
      firstkey = 0;
    }
  }
  
  return key;  
}
/*-----------------------------------------------------------------------------------*/
/* draw_screen paints the conio emulation screen on the top half of
   the drawing area. */
static void
draw_screen(void)
{
  char str[2];
  int x, y;

  gdk_draw_rectangle(pixmap,
		     drawing_area->style->white_gc,
		     TRUE,
		     0, 0,
		     drawing_area->allocation.width,
		     drawing_area->allocation.height);

  /*  return;*/
  
  str[1] = 0;
  for(y = 0; y < SCREEN_HEIGHT; ++y) {
    for(x = 0; x < SCREEN_WIDTH; ++x) {
      str[0] = screen[y * SCREEN_WIDTH + x];
      if(str[0] & 0x80) {
	str[0] = screen[y * SCREEN_WIDTH + x] & 0x7f;
	gdk_draw_rectangle(pixmap,
			   drawing_area->style->black_gc,
			   TRUE,
			   x * FONT_WIDTH,
			   y * FONT_HEIGHT,
			    FONT_WIDTH, FONT_HEIGHT);

	gdk_draw_string(pixmap,
			font,
			drawing_area->style->white_gc,
			x * FONT_WIDTH, FONT_HEIGHT + y * FONT_HEIGHT,
			str);
      } else {
	gdk_draw_string(pixmap,
			font,
			drawing_area->style->black_gc,
			x * FONT_WIDTH, FONT_HEIGHT + y * FONT_HEIGHT,
			str);
      }
    }
  }
}
/*-----------------------------------------------------------------------------------*/
static gint
configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
  if(pixmap != NULL) {
    gdk_pixmap_unref(pixmap);
  }

  pixmap = gdk_pixmap_new(widget->window,
			  widget->allocation.width,
			  widget->allocation.height,
			  -1);

  if(pixmap == NULL) {
    printf("gdk_pixmap_new == NULL\n");
    exit(1);
  }
  gdk_draw_rectangle(pixmap,
		     widget->style->white_gc,
		     TRUE,
		     0, 0,
		     widget->allocation.width,
		     widget->allocation.height);
  draw_screen();
  return TRUE;
}

/* Redraw the screen from the backing pixmap */
static gint
expose_event (GtkWidget * widget, GdkEventExpose * event)
{
  draw_screen();
  gdk_draw_pixmap(widget->window,
		  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		  pixmap,
		  event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);
  return FALSE;
}

static gint
key_press_event (GtkWidget * widget, GdkEventKey * event)
{
  if(event->keyval == GDK_Shift_L ||
     event->keyval == GDK_Shift_R) {
    return TRUE;
  }
  keys[lastkey] = event->keyval;
  ++lastkey;
  if(lastkey >= NUMKEYS) {
    lastkey = 0;
  }
  return TRUE;
}

static gint
key_release_event (GtkWidget * widget, GdkEventKey * event)
{
  return TRUE;
}

static void
quit(void)
{
  gtk_exit(0);
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
  GtkWidget *window;
  GtkWidget *vbox;

  gtk_init(NULL, NULL);
  
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(window, "Contiki GTKsim");

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER (window), vbox);
  gtk_widget_show(vbox);

  gtk_signal_connect(GTK_OBJECT (window), "destroy",
		     GTK_SIGNAL_FUNC (quit), NULL);
  
  /* Create the drawing area */

  drawing_area = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA (drawing_area),
			SCREEN_WIDTH * FONT_WIDTH,
			SCREEN_HEIGHT * FONT_HEIGHT);
  gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);

  gtk_widget_show(drawing_area);

  /* Load a fixed width font. */  
  font = gdk_font_load("-bitstream-courier-bold-r-normal-*-26-*-*-*-m-*-iso8859-1");
  if(font != NULL) {
    printf("Font loaded OK\n");
  } else {
    printf("Font loading failed\n");
    font = drawing_area->style->font;
  }

  
  /* Signals used to handle backing pixmap */

  gtk_signal_connect(GTK_OBJECT (drawing_area), "expose_event",
		     (GtkSignalFunc) expose_event, NULL);
  gtk_signal_connect(GTK_OBJECT (drawing_area), "configure_event",
		     (GtkSignalFunc) configure_event, NULL);

  /* Event signals */

  gtk_signal_connect(GTK_OBJECT (window), "key_press_event",
		     (GtkSignalFunc) key_press_event, NULL);
  gtk_signal_connect(GTK_OBJECT (window), "key_release_event",
		     (GtkSignalFunc) key_release_event, NULL);

  gtk_widget_set_events(drawing_area,GDK_KEY_PRESS_MASK
			| GDK_KEY_RELEASE_MASK);

  gtk_widget_show(window);

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
  char c, *text;
  unsigned char len;

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
  
  xpos = x + w->x;
  ypos = y + w->y;
    
  switch(w->type) {
  case CTK_WIDGET_SEPARATOR:
    if(ypos >= clipy1 && ypos < clipy2) {
      chlinexy(xpos, ypos, w->w);
    }
    break;
  case CTK_WIDGET_LABEL:
    text = w->widget.label.text;
    for(i = 0; i < w->widget.label.h; ++i) {
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
    if(ypos >= clipy1 && ypos < clipy2) {
      if(focus & CTK_FOCUS_WIDGET) {
	revers(1);
      } else {
	revers(0);
      }
      cputcxy(xpos, ypos, '[');
      cputsn(w->widget.button.text, w->w);
      cputc(']');
      revers(0);
    }
    break;
  case CTK_WIDGET_HYPERLINK:
    if(ypos >= clipy1 && ypos < clipy2) {
      if(focus & CTK_FOCUS_WIDGET) {
	revers(0);
      } else {
	revers(1);
      }
      gotoxy(xpos, ypos);
      textcolor(WIDGETCOLOR_HLINK);
      cputsn(w->widget.button.text, w->w);
      revers(0);
    }
    break;
  case CTK_WIDGET_TEXTENTRY:
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
    for(j = 0; j < w->widget.textentry.h; ++j) {
      if(ypos >= clipy1 && ypos < clipy2) {
	if(w->widget.textentry.state == CTK_TEXTENTRY_EDIT &&
	   w->widget.textentry.ypos == j) {
	  revers(0);
	  cputcxy(xpos, ypos, '>');
	  for(i = 0; i < w->w; ++i) {
	    c = text[i + xscroll];
	    if(i == w->widget.textentry.xpos - xscroll) {
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
      if(focus) {
	revers(1);
      } else {
	revers(0);
      }
      gotoxy(xpos, ypos);
      if(w->widget.icon.textmap != NULL) {
	for(i = 0; i < 3; ++i) {
	  gotoxy(xpos, ypos);
	  if(ypos >= clipy1 && ypos < clipy2) {
	    cputc(w->widget.icon.textmap[0 + 3 * i]);
	    cputc(w->widget.icon.textmap[1 + 3 * i]);
	    cputc(w->widget.icon.textmap[2 + 3 * i]);
	  }
	  ++ypos;
	}
      }
      x = xpos;
  
      len = strlen(w->widget.icon.title);
      if(x + len >= sizex) {
	x = sizex - len;
      }

      gotoxy(x, ypos);
      if(ypos >= clipy1 && ypos < clipy2) {
	cputs(w->widget.icon.title);
      }
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
  unsigned char posx, posy;

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
  redraw();  
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_clear_window(struct ctk_window *window,
		      unsigned char focus,
		      unsigned char clipy1,
		      unsigned char clipy2)
{
  unsigned char i;

  /* Clear window contents. */
  for(i = window->y + 2;
      i < window->y + 2 + window->h;
      ++i) {
    if(i >= clipy1 && i < clipy2) {
      cclearxy(window->x + 1, i, window->w);
    }
  }

  redraw();
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_window(struct ctk_window *window, unsigned char focus,
		unsigned char clipy1, unsigned char clipy2)
{
  unsigned char x, y;
  unsigned char y1, h;
  struct ctk_widget *w;
  unsigned char wfocus;

  if(window->y + 1 >= clipy2) {
    return;
  }
    
  x = window->x;
  y = window->y + 1;
  
  if(focus & CTK_FOCUS_WINDOW) {
    textcolor(CTK_CONF_WINDOWCOLOR_FOCUS);
  } else {
    textcolor(CTK_CONF_WINDOWCOLOR);
  }

  /* Draw window frame. */  


  if(y >= clipy1) {
    cputcxy(x, y, CH_ULCORNER);
    chlinexy(x + 1, y, 1);
    cputs(window->title);
    chline(window->w - (wherex() - x) + 1);
    cputcxy(x + window->w + 1, y, CH_URCORNER);
  }

  y1 = y + 1;
  h = window->h;

  if(clipy1 > y1) {
    if(clipy1 - y1 < h) {
      h = clipy1 - y1;
      y1 = clipy1;
    } else {
      h = 0;
    }
  }
  
  if(y1 + h >= clipy2) {
    if(y1 >= clipy2) {
      h = 0;
    } else {
      h = clipy2 - y1;
    }
  }
  
  cvlinexy(x, y1, h);
  cvlinexy(x + window->w + 1, y1, h);
  
    
  if(y + window->h >= clipy1 &&
     y + window->h < clipy2) {
    cputcxy(x, y + window->h + 1, CH_LLCORNER);
    chlinexy(x + 1, y + window->h + 1,
	     window->w);
    cputcxy(x + window->w + 1, y + window->h + 1, CH_LRCORNER);
  }
  

  
  /* Draw inactive. */  
  for(w = window->inactive; w != NULL; w = w->next) {
    draw_widget(w, x + 1, y + 1,
		x + 1 + window->w,
		y + 1 + window->h,
		clipy1, clipy2,
		focus & CTK_FOCUS_WINDOW);
  }
  
  /* Draw active. */
  for(w = window->active; w != NULL; w = w->next) { 
    wfocus = focus & CTK_FOCUS_WINDOW;
    if(w == window->focused) {
      wfocus |= CTK_FOCUS_WIDGET;
    }
    draw_widget(w, x + 1, y + 1,
		x + 1 + window->w,
		y + 1 + window->h,
		clipy1, clipy2,
		wfocus);
  }
  redraw();
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_dialog(struct ctk_window *dialog)
{
  unsigned char x, y;
  unsigned char i;
  struct ctk_widget *w;
  unsigned char wfocus;
  
  textcolor(CTK_CONF_DIALOGCOLOR);

  x = dialog->x;
  y = dialog->y + 1; 


  /* Draw dialog frame. */
  
  cvlinexy(x, y + 1,
	   dialog->h);
  cvlinexy(x + dialog->w + 1, y + 1,
	   dialog->h);

  chlinexy(x + 1, y,
	   dialog->w);
  chlinexy(x + 1, y + dialog->h + 1,
	   dialog->w);

  cputcxy(x, y, ' ');
  cputcxy(x, y + dialog->h + 1, ' ');
  cputcxy(x + dialog->w + 1, y, ' ');
  cputcxy(x + dialog->w + 1, y + dialog->h + 1, ' ');
  
  
  /* Clear window contents. */
  for(i = y + 1;
      i < y + 1 + dialog->h;
      ++i) {
    cclearxy(x + 1, i, dialog->w);
  }


  
  /* Clear dialog contents. */
  for(i = y + 1; i < y + 1 + dialog->h; ++i) {
    cclearxy(x + 1, i, dialog->w);
  }
  
  /* Draw inactive. */
  for(w = dialog->inactive; w != NULL; w = w->next) {
    draw_widget(w, x + 1, y + 1,
		x + 1 + dialog->w,
		y + 1 + dialog->h,
		0, SCREEN_HEIGHT,
		CTK_FOCUS_DIALOG);
  }


  /* Draw active. */
  for(w = dialog->active; w != NULL; w = w->next) {
    wfocus = CTK_FOCUS_DIALOG;
    if(w == dialog->focused) {
      wfocus |= CTK_FOCUS_WIDGET;
    }
    draw_widget(w, x + 1, y + 1,
		x + 1 + dialog->w,
		y + 1 + dialog->h,
		0, SCREEN_HEIGHT,
		wfocus);
  }

  redraw();
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_clear(unsigned char y1, unsigned char y2)
{
  unsigned char i;

  for(i = y1; i < y2; ++i) {
    cclearxy(0, i, SCREEN_WIDTH);
  }
  redraw();
}
/*-----------------------------------------------------------------------------------*/
static void
draw_menu(struct ctk_menu *m)
{
  unsigned char x, x2, y;
  textcolor(OPENMENUCOLOR);
  x = wherex();
  cputs(m->title);
  cputc(' ');
  x2 = wherex();
  if(x + CTK_CONF_MENUWIDTH > sizex) {
    x = sizex - CTK_CONF_MENUWIDTH;
  }
  
  
  for(y = 0; y < m->nitems; ++y) {
    if(y == m->active) {
      textcolor(ACTIVEMENUITEMCOLOR);
      revers(0);
    } else {
      textcolor(MENUCOLOR);	  
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
  textcolor(MENUCOLOR);  
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_menus(struct ctk_menus *menus)
{
    struct ctk_menu *m;  
  
  
  /* Draw menus */
  textcolor(MENUCOLOR);
  gotoxy(0, 0);
  revers(1);
  for(m = menus->menus->next; m != NULL; m = m->next) {
    if(m != menus->open) {
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
  }
  
  /* Draw desktopmenu */
  if(menus->desktopmenu != menus->open) {
    cputs(menus->desktopmenu->title);
    cputc(' ');
  } else {
    draw_menu(menus->desktopmenu);
  }

  revers(0);

  redraw();
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_draw_height(void)
{
  return SCREEN_HEIGHT;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_draw_width(void)
{
  return SCREEN_WIDTH;
}
/*-----------------------------------------------------------------------------------*/
