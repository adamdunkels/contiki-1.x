

#include "ctk.h"
#include "ctk-draw.h"
#include "ctk-draw-service.h"

#include "ctk-80col-asm.h"

#include <string.h>

unsigned char ctk_80col_cursx, ctk_80col_cursy;
unsigned char ctk_80col_reversed;
unsigned char ctk_80col_color;
unsigned char *ctk_80col_bitmapptr;
unsigned char ctk_80col_underline = 0;

#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH  80

#define SCREENADDR 0xdc00
#define HIRESADDR  0xe000

unsigned char ctk_80col_lefttab[256];
unsigned char ctk_80col_righttab[256];

unsigned short ctk_80col_yhiresaddr[25] =
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

/*---------------------------------------------------------------------------*/
/*void
ctk_arch_draw_char(char c,
		   unsigned char xpos,
		   unsigned char ypos,
		   unsigned char reversedflag,
		   unsigned char color)
{
  ctk_80col_cursx = xpos;
  ctk_80col_cursy = ypos;
  ctk_80col_reversed = reversedflag;
  ctk_80col_color = color;
  
  ctk_80col_cputc(c);
}*/
/*---------------------------------------------------------------------------*/
#pragma optimize(push, off)
static void
nmi2(void)
{
  asm("pla");
  asm("sta $01");
  asm("pla");
  asm("rti");
}  
#pragma optimize(pop)
/*---------------------------------------------------------------------------*/
#pragma optimize(push, off)
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
#pragma optimize(pop)
/*---------------------------------------------------------------------------*/
#pragma optimize(push, off)
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
#pragma optimize(pop)
/*---------------------------------------------------------------------------*/
#pragma optimize(push, off)
void
ctk_80col_init(void)
{
  int i;

  
  setup_nmi();
  
  /* Turn on hires mode, bank 0 ($c000 - $ffff) and $e000/$c000 for
     hires/colors. */
  VIC.ctrl1 = 0x3b;  /* $D011 */
  VIC.addr  = 0x78;  /* $D018 */
  VIC.ctrl2 = 0xc8;  /* $D016 */
  CIA2.pra  = 0x00;  /* $DD00 */

  VIC.bordercolor = 0x0f; /* $D020 */
  VIC.bgcolor0 = 0x0b; /* $D021 */  

  /* Fill color memory. */
  asm("sei");
  asm("lda $01");
  asm("pha");
  asm("lda #$30");
  asm("sta $01");
  asm("ldx #0");
  asm("lda #$bf");
  asm("fillcolorloop:");
  asm("sta $dc00,x");
  asm("sta $dd00,x");
  asm("sta $de00,x");
  asm("sta $df00,x");
  asm("inx");
  asm("bne fillcolorloop");

  /* Setup sprite pointers */
  asm("ldx #$fd");
  asm("stx $dff8");
  asm("inx");
  asm("stx $dff9");
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

  
  /*  ctk_draw_clear(0, 24);*/

  for(i = 0; i < 256; ++i) {
    ctk_80col_lefttab[i] =
      ((i & 0x40) << 1) |
      ((i & 0x10) << 2) |
      ((i & 0x04) << 3) |
      ((i & 0x01) << 4);
    ctk_80col_righttab[i] =
      ((i & 0x40) >> 3) |
      ((i & 0x10) >> 2) |
      ((i & 0x04) >> 1) |
      ((i & 0x01));
  }
  
#if 0
  /* Setup mouse pointer sprite. */
  asm("lda %v+%w", ctk_80col_theme,
      offsetof(struct ctk_80col_theme, pointermaskcolor));
  asm("sta $d027");
  asm("lda %v+%w", ctk_80col_theme,
      offsetof(struct ctk_80col_theme, pointercolor));
  asm("sta $d028");

  ptr1 = ctk_80col_theme.pointer;
  ptr2 = (unsigned char *)0xff40;
  
  for(i = 0; i < 0x80; ++i) {
    *ptr2++ = *ptr1++;
  }
#endif
  return;
}
#pragma optimize(pop)
/*---------------------------------------------------------------------------*/
/*static unsigned char cursx, cursy;
  static unsigned char reversed;*/

/*-----------------------------------------------------------------------------------*/
static void CC_FASTCALL 
cputc(char c) 
{
  /*  ctk_arch_draw_char(c, cursx, cursy, reversed, 0);*/
  ctk_80col_cputc(c);
  /*  ++cursx;*/
}
/*-----------------------------------------------------------------------------------*/
unsigned char
wherex(void)
{
  return ctk_80col_cursx;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
wherey(void)
{
  return ctk_80col_cursy;
}
/*-----------------------------------------------------------------------------------*/
/*void
clrscr(void)
{
  unsigned char x, y;

  for(x = 0; x < SCREEN_WIDTH; ++x) {
    for(y = 0; y < SCREEN_HEIGHT; ++y) {
      gotoxy(x, y);
      cputc(' ');
    }
  }
}*/
/*-----------------------------------------------------------------------------------*/
#define revers(c) ctk_80col_reversed = c
/*-----------------------------------------------------------------------------------*/
static void CC_FASTCALL 
_cputs(char *str)
{
  char *ptr = str;
  
  while(*ptr != 0) {
    cputc(*ptr++);
  }

  /*  int i;
  for(i = 0; i < strlen(str); ++i) {
    cputc(str[i]);
    }*/
}
/*-----------------------------------------------------------------------------------*/
static void CC_FASTCALL
cclear(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    cputc(' ');
  } 
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
chline(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    cputc('-');
  }
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
cvline(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    cputc('|');
    --ctk_80col_cursx;
    ++ctk_80col_cursy;
  }
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
gotoxy(unsigned char x, unsigned char y)
{
  ctk_80col_cursx = x;
  ctk_80col_cursy = y;
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
cclearxy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  cclear(length);
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
chlinexy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  chline(length);
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
cvlinexy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  cvline(length);
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
_cputsxy(unsigned char x, unsigned char y, char *str)
{
  gotoxy(x, y);
  _cputs(str);
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
cputcxy(unsigned char x, unsigned char y, char c)
{
  gotoxy(x, y);
  cputc(c);
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
screensize(unsigned char *x, unsigned char *y)
{
  *x = SCREEN_WIDTH;
  *y = SCREEN_HEIGHT;
}
/*-----------------------------------------------------------------------------------*/
static unsigned char sizex, sizey;
/*-----------------------------------------------------------------------------------*/
static void
_cputsn(char *str, unsigned char len)
{
  /*  char c;

  while(len > 0) {
    --len;
    c = *str;
    if(c == 0) {
      break;
    }
    cputc(c);
    ++str;
    }*/  
  ctk_80col_cputsn(str, len);
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_init(void)
{
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
  char c, *text;
  unsigned char len, wfocus;

  wfocus = 0;
  if(focus & CTK_FOCUS_WINDOW) {    
    if(focus & CTK_FOCUS_WIDGET) {
      wfocus = 1;
    }
  } else if(focus & CTK_FOCUS_DIALOG) {
    if(focus & CTK_FOCUS_WIDGET) {
      wfocus = 1;
    }
  } else {
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
    for(i = 0; i < w->h; ++i) {
      if(ypos >= clipy1 && ypos < clipy2) {
	gotoxy(xpos, ypos);
	_cputsn(text, w->w);
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
      if(wfocus != 0) {
	revers(1);
      } else {
	revers(0);
      }
      cputcxy(xpos, ypos, '[');
      _cputsn(w->widget.button.text, w->w);
      cputc(']');
      revers(0);
    }
    break;
  case CTK_WIDGET_HYPERLINK:
    if(ypos >= clipy1 && ypos < clipy2) {
      if(wfocus != 0) {
	revers(0);
      } else {
	revers(1);
      }
      gotoxy(xpos, ypos);
      _cputsn(w->widget.button.text, w->w);
      revers(0);
    }
    break;
  case CTK_WIDGET_TEXTENTRY:
    text = w->widget.textentry.text;
    if(wfocus != 0) {
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
	  _cputsn(text, w->w);
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
      if(focus & 1) {
	revers(1);
      } else {
	revers(0);
      }
      gotoxy(xpos, ypos);
      if(w->widget.icon.textmap != NULL) {
	ctk_80col_bitmapptr = w->widget.icon.bitmap;
	for(i = 0; i < 3; ++i) {
	  gotoxy(xpos, ypos);
	  if(ypos >= clipy1 && ypos < clipy2) {
	    /*	    cputc(w->widget.icon.textmap[0 + 3 * i]);
	    cputc(w->widget.icon.textmap[1 + 3 * i]);
	    cputc(w->widget.icon.textmap[2 + 3 * i]);*/
	    gotoxy(xpos-3, ypos);
	    ctk_80col_draw_bitmapline(3);	    
	  }
	  ctk_80col_bitmapptr += 3 * 8;
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
	_cputs(w->widget.icon.title);
      }
      revers(0);
    }
    break;

  default:
    break;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_widget(struct ctk_widget *w,
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
  
#ifdef CTK_CONIO_CONF_UPDATE
  CTK_CONIO_CONF_UPDATE();
#endif /* CTK_CONIO_CONF_UPDATE */
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_clear_window(struct ctk_window *window,
		      unsigned char focus,
		      unsigned char clipy1,
		      unsigned char clipy2)
{
  unsigned char i;
  unsigned char h;

  if(focus & CTK_FOCUS_WINDOW) {
  } else {
  }
    
  h = window->y + 2 + window->h;
  /* Clear window contents. */
  for(i = window->y + 2; i < h; ++i) {
    if(i >= clipy1 && i < clipy2) {
      cclearxy(window->x + 1, i, window->w);
    }
  }
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
static void
s_ctk_draw_window(struct ctk_window *window, unsigned char focus,
		  unsigned char clipy1, unsigned char clipy2)
{
  unsigned char x, y;
  unsigned char h;
  unsigned char x1, y1, x2, y2;

  if(window->y + 1 >= clipy2) {
    return;
  }
    
  x = window->x;
  y = window->y + 1;
  
  if(focus & CTK_FOCUS_WINDOW) {
  } else {
  }

  x1 = x + 1;
  y1 = y + 1;
  x2 = x1 + window->w;
  y2 = y1 + window->h;

  /* Draw window frame. */  
  if(y >= clipy1) {
    cputcxy(x, y, CH_ULCORNER);
    gotoxy(wherex() + window->titlelen + 2, wherey());
    chline(window->w - (wherex() - x) - 2);
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

  cvlinexy(x, y1, h);
  cvlinexy(x2, y1, h);  

  if(y + window->h >= clipy1 &&
     y + window->h < clipy2) {
    cputcxy(x, y2, CH_LLCORNER);
    chlinexy(x1, y2, window->w);
    cputcxy(x2, y2, CH_LRCORNER);
  }

  draw_window_contents(window, focus & CTK_FOCUS_WINDOW, clipy1, clipy2,
		       x1, x2, y + 1, y2);
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_dialog(struct ctk_window *dialog)
{
  unsigned char x, y;
  unsigned char i;
  unsigned char x1, y1, x2, y2;
  

  x = dialog->x;
  y = dialog->y + 1;


  x1 = x + 1;
  y1 = y + 1;
  x2 = x1 + dialog->w;
  y2 = y1 + dialog->h;


  /* Draw dialog frame. */
  
  cvlinexy(x, y1,
	   dialog->h);
  cvlinexy(x2, y1,
	   dialog->h);

  chlinexy(x1, y,
	   dialog->w);
  chlinexy(x1, y2,
	   dialog->w);

  cputcxy(x, y, CH_ULCORNER);
  cputcxy(x, y2, CH_LLCORNER);
  cputcxy(x2, y, CH_URCORNER);
  cputcxy(x2, y2, CH_LRCORNER);
  
  
  /* Clear dialog contents. */
  for(i = y1; i < y2; ++i) {
    cclearxy(x1, i, dialog->w);
  }

  draw_window_contents(dialog, CTK_FOCUS_DIALOG, 0, sizey,
		       x1, x2, y1, y2);
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_clear(unsigned char y1, unsigned char y2)
{
  unsigned char i;
 
  for(i = y1; i < y2; ++i) {
    
    ctk_80col_clear_line(i);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
draw_menu(struct ctk_menu *m)
{
  unsigned char x, x2, y;
  revers(0);
  x = wherex();
  _cputs(m->title);
  cputc(' ');
  x2 = wherex();
  if(x + CTK_CONF_MENUWIDTH > sizex) {
    x = sizex - CTK_CONF_MENUWIDTH;
  }
  
  
  for(y = 0; y < m->nitems; ++y) {
    if(y == m->active) {
      revers(0);
    } else {
      revers(1);
    }
    gotoxy(x, y + 1);
    if(m->items[y].title[0] == '-') {
      chline(CTK_CONF_MENUWIDTH);
    } else {
      _cputs(m->items[y].title);
    }
    if(x + CTK_CONF_MENUWIDTH > wherex()) {
      cclear(x + CTK_CONF_MENUWIDTH - wherex());
    }
  
  }
  gotoxy(x2, 0);
  revers(1);
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_menus(struct ctk_menus *menus)
{
  struct ctk_menu *m;  

  /* Draw menus */
  gotoxy(0, 0);
  revers(1);
  cputc(' ');
  for(m = menus->menus->next; m != NULL; m = m->next) {
    if(m != menus->open) {
      _cputs(m->title);
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
    _cputs(menus->desktopmenu->title);
    cputc(' ');
  } else {
    draw_menu(menus->desktopmenu);
  }

  revers(0);
}
/*-----------------------------------------------------------------------------------*/
static unsigned char
s_ctk_draw_height(void)
{
  return sizey;
}
/*-----------------------------------------------------------------------------------*/
static unsigned char
s_ctk_draw_width(void)
{
  return sizex;
}
/*-----------------------------------------------------------------------------------*/
static unsigned short
s_ctk_mouse_xtoc(unsigned short x)
{
  return x / 4;
}
/*-----------------------------------------------------------------------------------*/
static unsigned short
s_ctk_mouse_ytoc(unsigned short y)
{
  return y / 8;
}
/*-----------------------------------------------------------------------------------*/
static const struct ctk_draw_service_interface interface =
  {CTK_DRAW_SERVICE_VERSION,
   1,
   1,
   1,
   s_ctk_draw_init,
   s_ctk_draw_clear,
   s_ctk_draw_clear_window,
   s_ctk_draw_window,
   s_ctk_draw_dialog,
   s_ctk_draw_widget,
   s_ctk_draw_menus,
   s_ctk_draw_width,
   s_ctk_draw_height,
   s_ctk_mouse_xtoc,
   s_ctk_mouse_ytoc,
  };

EK_EVENTHANDLER(eventhandler, ev, data);
EK_PROCESS(proc, CTK_DRAW_SERVICE_NAME ": 80col", EK_PRIO_NORMAL,
	   eventhandler, NULL, (void *)&interface);

/*--------------------------------------------------------------------------*/
LOADER_INIT_FUNC(ctk_80col_service_init, arg)
{ 
  ek_service_start(CTK_DRAW_SERVICE_NAME, &proc);
}
/*--------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{
  EK_EVENTHANDLER_ARGS(ev, data);
  
  switch(ev) {
  case EK_EVENT_INIT:
  case EK_EVENT_REPLACE:
    ctk_80col_init();
    ctk_restore();
    break;
  case EK_EVENT_REQUEST_REPLACE:
    ek_replace((struct ek_proc *)data, NULL);
    LOADER_UNLOAD();
    break;    
  }
}
/*--------------------------------------------------------------------------*/
