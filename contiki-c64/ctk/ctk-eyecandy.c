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
 * $Id: ctk-eyecandy.c,v 1.2 2003/03/28 12:16:24 adamdunkels Exp $
 *
 */
#include "ctk.h"
#include "ctk-draw.h"
#include "ctk-eyecandy.h"
#include "ctk-eyecandy-asm.h"

#include <string.h>

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH  40

#define SCREENADDR 0xdc00
#define HIRESADDR  0xe000

static unsigned char lineptr;

unsigned char ctk_eyecandy_cursx, ctk_eyecandy_cursy;
unsigned char ctk_eyecandy_reversed;
unsigned char ctk_eyecandy_color;
unsigned char ctk_eyecandy_underline;

static unsigned char cchar;

static unsigned char tmp01;
static unsigned char tmph, tmpl, tmpborder;
static unsigned char *tmpptr;


static unsigned char x, y, i;
static unsigned char h;
static unsigned char wfocus;
static unsigned char x1, y1, x2, y2;

struct ctk_eyecandy_windowparams ctk_eyecandy_windowparams;
unsigned char *ctk_eyecandy_bitmapptr;
/*-----------------------------------------------------------------------------------*/
/* Tables. */

#define COLOR(bg, fg) ((fg << 4) | (bg))

#define MENUCOLOR       54
#define OPENMENUCOLOR   55
#define ACTIVEMENUCOLOR 56

#include "ctk-eyecandy-conf.h"


unsigned short ctk_eyecandy_yscreenaddr[25] =
  {0 * SCREEN_WIDTH + SCREENADDR, 1 * SCREEN_WIDTH + SCREENADDR,
   2 * SCREEN_WIDTH + SCREENADDR, 3 * SCREEN_WIDTH + SCREENADDR,
   4 * SCREEN_WIDTH + SCREENADDR, 5 * SCREEN_WIDTH + SCREENADDR,
   6 * SCREEN_WIDTH + SCREENADDR, 7 * SCREEN_WIDTH + SCREENADDR,
   8 * SCREEN_WIDTH + SCREENADDR, 9 * SCREEN_WIDTH + SCREENADDR,
   10 * SCREEN_WIDTH + SCREENADDR, 11 * SCREEN_WIDTH + SCREENADDR,
   12 * SCREEN_WIDTH + SCREENADDR, 13 * SCREEN_WIDTH + SCREENADDR,
   14 * SCREEN_WIDTH + SCREENADDR, 15 * SCREEN_WIDTH + SCREENADDR,
   16 * SCREEN_WIDTH + SCREENADDR, 17 * SCREEN_WIDTH + SCREENADDR,
   18 * SCREEN_WIDTH + SCREENADDR, 19 * SCREEN_WIDTH + SCREENADDR,
   20 * SCREEN_WIDTH + SCREENADDR, 21 * SCREEN_WIDTH + SCREENADDR,
   22 * SCREEN_WIDTH + SCREENADDR, 23 * SCREEN_WIDTH + SCREENADDR,
   24 * SCREEN_WIDTH + SCREENADDR};

unsigned short ctk_eyecandy_yhiresaddr[25] =
  {0 * 320 + HIRESADDR, 1 * 320 + HIRESADDR,
   2 * 320 + HIRESADDR, 3 * 320 + HIRESADDR,
   4 * 320 + HIRESADDR, 5 * 320 + HIRESADDR,
   6 * 320 + HIRESADDR, 7 * 320 + HIRESADDR,
   8 * 320 + HIRESADDR, 9 * 320 + HIRESADDR,
   10 * 320 + HIRESADDR, 11 * 320 + HIRESADDR,
   12 * 320 + HIRESADDR, 13 * 320 + HIRESADDR,
   14 * 320 + HIRESADDR, 15 * 320 + HIRESADDR,
   16 * 320 + HIRESADDR, 17 * 320 + HIRESADDR,
   18 * 320 + HIRESADDR, 19 * 320 + HIRESADDR,
   20 * 320 + HIRESADDR, 21 * 320 + HIRESADDR,
   22 * 320 + HIRESADDR, 23 * 320 + HIRESADDR,
   24 * 320 + HIRESADDR};

static unsigned char linecolors[25] =
  {COLOR(BGCOLOR1,BGCOLOR1),
   COLOR(BGCOLOR2,BGCOLOR1),COLOR(BGCOLOR2,BGCOLOR1),
   COLOR(BGCOLOR2,BGCOLOR1),COLOR(BGCOLOR2,BGCOLOR1),
   COLOR(BGCOLOR2,BGCOLOR1),COLOR(BGCOLOR2,BGCOLOR1),
   COLOR(BGCOLOR2,BGCOLOR1),COLOR(BGCOLOR2,BGCOLOR1),
   COLOR(BGCOLOR3,BGCOLOR2),COLOR(BGCOLOR3,BGCOLOR2),
   COLOR(BGCOLOR3,BGCOLOR2),COLOR(BGCOLOR3,BGCOLOR2),
   COLOR(BGCOLOR3,BGCOLOR2),COLOR(BGCOLOR3,BGCOLOR2),
   COLOR(BGCOLOR3,BGCOLOR2),COLOR(BGCOLOR3,BGCOLOR2),
   COLOR(BGCOLOR4,BGCOLOR3),COLOR(BGCOLOR4,BGCOLOR3),
   COLOR(BGCOLOR4,BGCOLOR3),COLOR(BGCOLOR4,BGCOLOR3),
   COLOR(BGCOLOR4,BGCOLOR3),COLOR(BGCOLOR4,BGCOLOR3),
   COLOR(BGCOLOR4,BGCOLOR3),COLOR(BGCOLOR4,BGCOLOR3)};

static unsigned char dither48[4*8] =
  {0x88,0x00,0x22,0x00,
   0xff,0xff,0xff,0xff,
   0xff,0xdd,0xff,0x77,
   0xff,0x55,0xff,0x55,
   0xee,0x55,0xbb,0x55,
   0xaa,0x55,0xaa,0x55,
   0xaa,0x44,0xaa,0x11,
   0xaa,0x00,0xaa,0x00};
/*-----------------------------------------------------------------------------------*/
#define hires_wherex() ctk_eyecandy_cursx
#define hires_revers(c)   ctk_eyecandy_reversed = c
#define hires_color(c)   ctk_eyecandy_color = c
#define hires_underline(c)   ctk_eyecandy_underline = c
/*-----------------------------------------------------------------------------------*/
static void
hires_cvline(unsigned char length)
{
  unsigned char i;
  
  for(i = 0; i < length; ++i) {
    ctk_eyecandy_cputc('|');
    --ctk_eyecandy_cursx;
    ++ctk_eyecandy_cursy;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
hires_gotoxy(unsigned char x, unsigned char y)
{
  ctk_eyecandy_cursx = x;
  ctk_eyecandy_cursy = y;
}
/*-----------------------------------------------------------------------------------*/
static void
hires_cclearxy(unsigned char x, unsigned char y, unsigned char length)
{
  hires_gotoxy(x, y);
  ctk_eyecandy_cclear(length);
}
/*-----------------------------------------------------------------------------------*/
static void
hires_chlinexy(unsigned char x, unsigned char y, unsigned char length)
{
  hires_gotoxy(x, y);
  ctk_eyecandy_chline(length);
}
/*-----------------------------------------------------------------------------------*/
static void
hires_cvlinexy(unsigned char x, unsigned char y, unsigned char length)
{
  hires_gotoxy(x, y);
  hires_cvline(length);
}
/*-----------------------------------------------------------------------------------*/
static void
hires_cputcxy(unsigned char x, unsigned char y, char c)
{
  hires_gotoxy(x, y);
  ctk_eyecandy_cputc(c);
}
/*-----------------------------------------------------------------------------------*/
static void
clear_line(unsigned char line)
{
  lineptr = line;
  asm("lda _lineptr");
  asm("asl");
  asm("tax");
  asm("lda _ctk_eyecandy_yhiresaddr,x");
  asm("sta ptr2");
  asm("lda _ctk_eyecandy_yhiresaddr+1,x");
  asm("sta ptr2+1");
  asm("lda _ctk_eyecandy_yscreenaddr,x");
  asm("sta ptr1");
  asm("lda _ctk_eyecandy_yscreenaddr+1,x");
  asm("sta ptr1+1");

  
  asm("sei");
  asm("lda $01");
  asm("pha");
  asm("lda #$30");
  asm("sta $01");
  asm("ldy #39");
  asm("ldx _lineptr");
  asm("lda _linecolors,x");
  asm("clearlineloop1:");
  asm("sta (ptr1),y");
  asm("dey");
  asm("bpl clearlineloop1");
  asm("pla");
  asm("sta $01");
  asm("cli");


  asm("lda _lineptr");
  asm("and #7");
  asm("asl");
  /*  asm("asl");*/
  asm("asl");
  asm("tax");
  asm("ldy #0");
  asm("clearlineloop2:");
  asm("lda _dither48+0,x");
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda _dither48+1,x");
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda _dither48+2,x");
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda _dither48+3,x");
  asm("sta (ptr2),y");
  asm("iny");
  /*  asm("lda _dither48+4,x");
      asm("sta (ptr2),y");
      asm("iny");
      asm("lda _dither48+5,x");
      asm("sta (ptr2),y");
      asm("iny");
      asm("lda _dither48+6,x");
      asm("sta (ptr2),y");
      asm("iny");
      asm("lda _dither48+7,x");
      asm("sta (ptr2),y");
      asm("iny");*/
  asm("bne clearlineloop2");
  
  asm("inc ptr2+1");

  asm("ldy #0");
  asm("clearlineloop3:");
  asm("lda _dither48+0,x");
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda _dither48+1,x");
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda _dither48+2,x");
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda _dither48+3,x");
  asm("sta (ptr2),y");
  asm("iny");
  /*  asm("lda _dither48+4,x");
      asm("sta (ptr2),y");
      asm("iny");
      asm("lda _dither48+5,x");
      asm("sta (ptr2),y");
      asm("iny");
      asm("lda _dither48+6,x");
      asm("sta (ptr2),y");
      asm("iny");
      asm("lda _dither48+7,x");
      asm("sta (ptr2),y");
      asm("iny");*/
  asm("cpy #$40");
  asm("bne clearlineloop3");
  
}
/*-----------------------------------------------------------------------------------*/
static void
nmi2(void)
{
  asm("pla");
  asm("sta $01");
  asm("pla");
  asm("rti");
}  
/*-----------------------------------------------------------------------------------*/
static void
nmi(void)
{
  asm("sei");
  asm("pha");
  asm("inc $d020");
  asm("lda $01");
  asm("pha");
  asm("lda #$36");
  asm("sta $01");
  asm("lda #>_nmi2");
  asm("pha");
  asm("lda #<_nmi2");
  asm("pha");
  asm("php");
  asm("jmp ($0318)");

  nmi2();
}
/*-----------------------------------------------------------------------------------*/
static void
setup_nmi(void)
{
  asm("lda #<_nmi");
  asm("sta $fffa");
  asm("lda #>_nmi");
  asm("sta $fffb");
  return;
  nmi();
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_init(void)
{
  setup_nmi();
  
  /* Turn on hires mode, bank 0 ($c000 - $ffff) and $e000/$c000 for
     hires/colors. */
  VIC.ctrl1 = 0x3b;  /* $D011 */
  VIC.addr  = 0x78;  /* $D018 */
  VIC.ctrl2 = 0xc8;  /* $D016 */
  VIC.bordercolor = 0x06; /* $D020 */
  VIC.bgcolor0 = 0x00; /* $D021 */  
  CIA2.pra  = 0x00;  /* $DD00 */

  /* Fill color memory. */
  asm("sei");
  asm("lda $01");
  asm("pha");
  asm("lda #$30");
  asm("sta $01");
  asm("ldx #0");
  asm("lda #$c0");
  asm("fillcolorloop:");
  asm("sta $dc00,x");
  asm("sta $dd00,x");
  asm("sta $de00,x");
  asm("sta $df00,x");
  asm("inx");
  asm("bne fillcolorloop");
  asm("pla");
  asm("sta $01");
  asm("cli");

  /* Fill hires memory with 0. */

  asm("lda $fd");
  asm("pha");
  asm("lda $fe");
  asm("pha");
  asm("lda #0");
  asm("sta $fd");
  asm("lda #$e0");
  asm("sta $fe");
  asm("ldy #0");
  asm("lda #0");
  asm("clrscrnloop:");
  asm("lda #$55");
  asm("sta ($fd),y");
  asm("iny");
  asm("lda #$aa");
  asm("sta ($fd),y");
  asm("iny");
  asm("bne clrscrnloop");
  asm("inc $fe");
  asm("lda $fe");
  asm("cmp #$ff");
  asm("bne clrscrnloop");

  asm("ldy #$00");
  asm("clrscrnloop2:");
  asm("lda #$55");
  asm("sta $ff00,y");
  asm("iny");
  asm("lda #$aa");
  asm("sta $ff00,y");
  asm("iny");
  asm("cpy #$40");
  asm("bne clrscrnloop2");

  
  asm("pla");
  asm("sta $fe");
  asm("pla");
  asm("sta $fd");

  ctk_draw_clear(0, SCREEN_HEIGHT);
  
  return;
}
/*-----------------------------------------------------------------------------------*/
static void
draw_bitmap_icon(unsigned char *bitmap)
{
  tmpptr = bitmap;

  /* Find screen address. */
  asm("lda _ctk_eyecandy_cursy");
  asm("asl");
  asm("tax");
  asm("lda _ctk_eyecandy_yscreenaddr,x");
  asm("sta ptr1");
  asm("lda _ctk_eyecandy_yscreenaddr+1,x");
  asm("sta ptr1+1");

  /* Turn off interrupts, prepare $01 to store color data in RAM under
     I/O area. */
  asm("sei");
  asm("lda $01");
  asm("sta _tmp01");
  asm("and #$f8");
  asm("sta $01");

  /* Actually store color value in color RAM. */
  asm("ldy _ctk_eyecandy_cursx");
  asm("lda _ctk_eyecandy_color");
  asm("sta (ptr1),y");
  asm("iny");
  asm("sta (ptr1),y");
  asm("iny");
  asm("sta (ptr1),y");
  asm("tya");
  asm("clc");
  asm("adc #$26");
  asm("tay");
  asm("lda _ctk_eyecandy_color");
  asm("sta (ptr1),y");
  asm("iny");
  asm("sta (ptr1),y");
  asm("iny");
  asm("sta (ptr1),y");
  asm("tya");
  asm("clc");
  asm("adc #$26");
  asm("tay");
  asm("lda _ctk_eyecandy_color");
  asm("sta (ptr1),y");
  asm("iny");
  asm("sta (ptr1),y");
  asm("iny");
  asm("sta (ptr1),y");



  /* Find hires address. */
  asm("lda _ctk_eyecandy_cursy");
  asm("asl");
  asm("tax");
  asm("lda _ctk_eyecandy_yhiresaddr,x");
  asm("sta ptr2");
  asm("lda _ctk_eyecandy_yhiresaddr+1,x");
  asm("sta ptr2+1");

  /* Add X coordinate to the hires address. */
  asm("lda #0");
  asm("sta ptr1+1");
  asm("lda _ctk_eyecandy_cursx");
  asm("asl");
  asm("rol ptr1+1");
  asm("asl");
  asm("rol ptr1+1");
  asm("asl");
  asm("rol ptr1+1");
  asm("clc");
  asm("adc ptr2");
  asm("sta ptr2");
  asm("lda ptr2+1");
  asm("adc ptr1+1");
  asm("sta ptr2+1");

  asm("lda _tmpptr");
  asm("sta ptr1");
  asm("lda _tmpptr+1");
  asm("sta ptr1+1");

  asm("ldx #3");
  asm("iconloop1:");
  asm("ldy #0");
  asm("iconloop2:");
  asm("lda (ptr1),y");
  asm("sta (ptr2),y");
  asm("iny");
  asm("cpy #$18");
  asm("bne iconloop2");
  asm("lda ptr1");
  asm("clc");
  asm("adc #$18");
  asm("sta ptr1");
  asm("lda ptr1+1");
  asm("adc #0");
  asm("sta ptr1+1");
  asm("lda ptr2");
  asm("clc");
  asm("adc #$40");
  asm("sta ptr2");
  asm("lda ptr2+1");
  asm("adc #1");
  asm("sta ptr2+1");
  asm("dex");
  asm("bne iconloop1");

  
  
  asm("lda _tmp01");
  asm("sta $01");  
  asm("cli"); 
}
/*-----------------------------------------------------------------------------------*/
static void
draw_widget(struct ctk_widget *w,
	    unsigned char x, unsigned char y,
	    unsigned char clipy1, unsigned char clipy2,
	    unsigned char focus)
{
  unsigned char xpos, ypos, xscroll;
  unsigned char i, j;
  char c, *text;
  unsigned char len;

  xpos = x + w->x;
  ypos = y + w->y;

  hires_color(colors[w->type * 6 + focus]);
  
  switch(w->type) {
  case CTK_WIDGET_SEPARATOR:
    if(ypos >= clipy1 && ypos < clipy2) {
      hires_chlinexy(xpos, ypos, w->w);
    }
    break;
  case CTK_WIDGET_LABEL:
    text = w->widget.label.text;
    for(i = 0; i < w->widget.label.h; ++i) {
      if(ypos >= clipy1 && ypos < clipy2) {
	hires_gotoxy(xpos, ypos);
	ctk_eyecandy_cputsn(text, w->w);
	if(w->w - (hires_wherex() - xpos) > 0) {
	  ctk_eyecandy_cclear(w->w - (hires_wherex() - xpos));
	}
      }
      ++ypos;
      text += w->w;
    }
    break;
  case CTK_WIDGET_BUTTON:
    if(ypos >= clipy1 && ypos < clipy2) {
      hires_color(colors[7 * 6 + focus]);
      hires_gotoxy(xpos, ypos);
      ctk_eyecandy_draw_buttonleft();
      hires_color(colors[w->type * 6 + focus]);
      hires_gotoxy(xpos + 1, ypos);
      ctk_eyecandy_cputsn(w->widget.button.text, w->w);
      hires_color(colors[8 * 6 + focus]);      
      ctk_eyecandy_draw_buttonright();
    }
    break;
  case CTK_WIDGET_HYPERLINK:
    if(ypos >= clipy1 && ypos < clipy2) {
      hires_underline(1);
      hires_gotoxy(xpos, ypos);
      ctk_eyecandy_cputsn(w->widget.button.text, w->w);
      hires_underline(0);
    }
    break;
  case CTK_WIDGET_TEXTENTRY:
    text = w->widget.textentry.text;
    if(focus & CTK_FOCUS_WIDGET &&
       w->widget.textentry.state != CTK_TEXTENTRY_EDIT) {
      hires_revers(1);
    } else {
      hires_revers(0);
    }
    xscroll = 0;
    if(w->widget.textentry.xpos >= w->w - 1) {
      xscroll = w->widget.textentry.xpos - w->w + 1;
    }
    for(j = 0; j < w->widget.textentry.h; ++j) {
      if(ypos >= clipy1 && ypos < clipy2) {
	if(w->widget.textentry.state == CTK_TEXTENTRY_EDIT &&
	   w->widget.textentry.ypos == j) {
	  hires_revers(0);
	  hires_cputcxy(xpos, ypos, '>');
	  for(i = 0; i < w->w; ++i) {
	    c = text[i + xscroll];
	    if(i == w->widget.textentry.xpos - xscroll) {
	      hires_revers(1);
	    } else {
	      hires_revers(0);
	    }
	    if(c == 0) {
	      ctk_eyecandy_cputc(' ');
	    } else {
	      ctk_eyecandy_cputc(c);
	    }
	    hires_revers(0);
	  }
	  ctk_eyecandy_cputc('<');
	} else {
	  hires_cputcxy(xpos, ypos, '|');
	  /*	  hires_gotoxy(xpos + 1, ypos);          */
	  ctk_eyecandy_cputsn(text, w->w);
	  i = hires_wherex();
	  if(i - xpos - 1 < w->w) {
	    ctk_eyecandy_cclear(w->w - (i - xpos) + 1);
	  }
	  ctk_eyecandy_cputc('|');
	}
      }
      ++ypos;
      text += w->widget.textentry.len;
    }
    hires_revers(0);
    break;
  case CTK_WIDGET_ICON:
    if(ypos >= clipy1 && ypos < clipy2) {
      if(focus) {
	hires_color(COLOR(COLOR_BLACK, COLOR_YELLOW));
      } else {
	hires_color(COLOR(COLOR_WHITE, COLOR_BLACK));
      }
      hires_gotoxy(xpos, ypos);
      if(w->widget.icon.bitmap != NULL) {
	draw_bitmap_icon(w->widget.icon.bitmap);
      }
      x = xpos;

      len = strlen(w->widget.icon.title);
      if(x + len >= SCREEN_WIDTH) {
	x = SCREEN_WIDTH - len;
      }
      
      hires_gotoxy(x, ypos + 3);
      ctk_eyecandy_cputsn(w->widget.icon.title, len);
    }
    break;
  case CTK_WIDGET_BITMAP:
    ctk_eyecandy_bitmapptr = w->widget.bitmap.bitmap;
    for(i = 0; i < w->widget.bitmap.h; ++i) {
      if(ypos >= clipy1 && ypos < clipy2) {
	hires_gotoxy(xpos, ypos);
	ctk_eyecandy_draw_bitmapline(w->w);
      }
      ctk_eyecandy_bitmapptr += w->w * 8;
      ++ypos;
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
	      clipy1, clipy2,
	      focus);
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_window(register struct ctk_window *window,
		unsigned char focus,
		unsigned char clipy1, unsigned char clipy2)
{
  register struct ctk_widget *w;
  
  /*  if(clipy1 >= clipy2) {
    return;
    }*/
  
  
  x = window->x;
  y = window->y + 1;

  ++clipy2;
  
  if(clipy2 < y) {
    return;
  }

  x1 = x + 1;
  y1 = y + 1;
  x2 = x1 + window->w;
  y2 = y1 + window->h;
 
  
  hires_color(colors[focus]);
  
  h = y1 + window->h;
  /* Clear window contents. */
  for(i = y1; i < h; ++i) {
    if(i >= clipy1 && i < clipy2) {
      hires_cclearxy(x1, i, window->w);
    }
  }

  hires_color(colors[focus+1]);
    
  hires_gotoxy(x, y);
  ctk_eyecandy_windowparams.w = window->w;
  ctk_eyecandy_windowparams.h = window->h;
  if(clipy1 < y) {
    ctk_eyecandy_windowparams.clipy1 = 0;
  } else {
    ctk_eyecandy_windowparams.clipy1 = clipy1 - y;
  }
  ctk_eyecandy_windowparams.clipy2 = clipy2 - y;
  ctk_eyecandy_windowparams.color1 = colors[focus+1];
  ctk_eyecandy_windowparams.color2 = colors[focus];
  ctk_eyecandy_windowparams.title = window->title;
  ctk_eyecandy_windowparams.titlelen = window->titlelen;

  ctk_eyecandy_draw_windowborders();
  
#if 0
  /* Draw window frame. */  
  if(y >= clipy1) {
    windowulcorner(x, y);
    windowupperborder(x + 3 + strlen(window->title),
		      y, window->w - 5 - strlen(window->title));
    windowurcorner(x + window->w + 1, y);
    hires_gotoxy(x + 1, y);
    hires_color(colors[focus]);
    ctk_eyecandy_cputc(' ');
    ctk_eyecandy_cputsn(window->title, window->titlelen);
    ctk_eyecandy_cputc(' ');
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

  if(y1 + h >= clipy2) {
    if(y1 >= clipy2) {
      h = 0;
    } else {
      h = clipy2 - y1;
    }
  }

  if(h == 0) {
    return;
  }
  
  hires_color(colors[focus+1]);  
  windowsideborder(x, y1, h+1, 0xc0);
  hires_color(colors[focus]);
  windowsideborder(x2, y+1, h, 0x0b);

  if(y + window->h >= clipy1 &&
     y + window->h < clipy2) {
    windowlowerborder(x1, y2, window->w);
  }

#endif /* 0 */
  
  focus = focus & CTK_FOCUS_WINDOW;
  
  /* Draw inactive widgets. */
  for(w = window->inactive; w != NULL; w = w->next) {
    draw_widget(w, x1, y1,
		clipy1, clipy2,
		focus);
  }
  
  /* Draw active widgets. */
  for(w = window->active; w != NULL; w = w->next) {  
    wfocus = focus;
    if(w == window->focused) {
      wfocus |= CTK_FOCUS_WIDGET;
    }
    draw_widget(w, x1, y1,
		clipy1, clipy2,
		wfocus);
  }
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_dialog(register struct ctk_window *dialog)
{
  register struct ctk_widget *w;

  hires_color(colors[CTK_FOCUS_DIALOG]);

  /*  x = (SCREEN_WIDTH - dialog->w) / 2;
      y = (SCREEN_HEIGHT - 1 - dialog->h) / 2; */
  x = dialog->x;
  y = dialog->y + 1;


  x1 = x + 1;
  y1 = y + 1;
  x2 = x1 + dialog->w;
  y2 = y1 + dialog->h;


  /* Draw dialog frame. */
  
  hires_cvlinexy(x, y1,
		 dialog->h);
  hires_cvlinexy(x2, y1,
		 dialog->h);

  hires_chlinexy(x1, y,
		 dialog->w);
  hires_chlinexy(x1, y2,
		 dialog->w);

  hires_cputcxy(x, y, CH_ULCORNER);
  hires_cputcxy(x, y2, CH_LLCORNER);
  hires_cputcxy(x2, y, CH_URCORNER);
  hires_cputcxy(x2, y2, CH_LRCORNER);
  
  
  /* Clear window contents. */
  for(i = y1; i < y2;  ++i) {
    hires_cclearxy(x1, i, dialog->w);
  }
  
  /* Clear dialog contents. */
  for(i = y1; i < y2; ++i) {
    hires_cclearxy(x1, i, dialog->w);
  }
  
  /* Draw inactive widgets. */
  for(w = dialog->inactive; w != NULL; w = w->next) {
    draw_widget(w, x1, y1,
		0, SCREEN_HEIGHT, CTK_FOCUS_DIALOG);
  }


  /* Draw active widgets. */
  for(w = dialog->active; w != NULL; w = w->next) {
    wfocus = CTK_FOCUS_DIALOG;
    if(w == dialog->focused) {
      wfocus |= CTK_FOCUS_WIDGET;
    }
    draw_widget(w, x1, y1, 
		0, SCREEN_HEIGHT, wfocus);
  }

}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_clear(unsigned char y1, unsigned char y2)
{
  for(i = y1; i < y2; ++i) {
    clear_line(i);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
draw_menu(struct ctk_menu *m)
{
  unsigned char x, x2, y;
  
  hires_color(colors[OPENMENUCOLOR]);
  x = hires_wherex();
  ctk_eyecandy_cputsn(m->title, m->titlelen);
  ctk_eyecandy_cputc(' ');
  x2 = hires_wherex();
  if(x + CTK_CONF_MENUWIDTH > SCREEN_WIDTH) {
    x = SCREEN_WIDTH - CTK_CONF_MENUWIDTH;
  }      
  for(y = 0; y < m->nitems; ++y) {
    if(y == m->active) {
      hires_color(colors[ACTIVEMENUCOLOR]);
    } else {
      hires_color(colors[MENUCOLOR]);
    }
    hires_gotoxy(x, y + 1);
    if(m->items[y].title[0] == '-') {
      ctk_eyecandy_chline(CTK_CONF_MENUWIDTH);
    } else {
      ctk_eyecandy_cputsn(m->items[y].title,
			  strlen(m->items[y].title));
    }
    ctk_eyecandy_cclear(x + CTK_CONF_MENUWIDTH - hires_wherex());
    hires_revers(0);
  }
  hires_gotoxy(x2, 0);
  hires_color(colors[MENUCOLOR]);  
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_menus(struct ctk_menus *menus)
{
  struct ctk_menu *m;
  
  /* Draw menus */

  hires_color(colors[MENUCOLOR]); 
  hires_gotoxy(0, 0);
  hires_revers(0);
  ctk_eyecandy_cputc(' ');
  for(m = menus->menus->next; m != NULL; m = m->next) {
    if(m != menus->open) {
      ctk_eyecandy_cputsn(m->title, m->titlelen);
      ctk_eyecandy_cputc(' ');
    } else {
      draw_menu(m);
    }
  }
  ctk_eyecandy_cclear(SCREEN_WIDTH - hires_wherex() -
		      strlen(menus->desktopmenu->title) - 1);
  
  /* Draw desktopmenu */  
  if(menus->desktopmenu != menus->open) {
    ctk_eyecandy_cputsn(menus->desktopmenu->title,
			menus->desktopmenu->titlelen);
    ctk_eyecandy_cputc(' ');
  } else {
    draw_menu(menus->desktopmenu);
  }

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
