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
 * $Id: ssfire.c,v 1.2 2003/07/31 23:34:42 adamdunkels Exp $
 *
 */

#include <stdlib.h>

#include "ctk.h"
#include "ctk-draw.h"
#include "dispatcher.h"
#include "loader.h"

static unsigned char save;

static DISPATCHER_SIGHANDLER(ssfire_sighandler, s, data);
static void ssfire_idle(void);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Fire screensaver", ssfire_idle,
		   ssfire_sighandler,
		   NULL)};
static ek_id_t id;


static unsigned char flames[8*17];

static unsigned char *flameptr, *colorptr1, *colorptr2;
static unsigned char x, y;


static const unsigned char flamecolors[16] =
  {COLOR_BLACK,  COLOR_BLACK, COLOR_BLACK,
   COLOR_RED,    COLOR_LIGHTRED,   COLOR_YELLOW, COLOR_WHITE,
   COLOR_WHITE,  COLOR_WHITE, COLOR_WHITE,    COLOR_WHITE,
   COLOR_WHITE,  COLOR_WHITE, COLOR_WHITE,    COLOR_WHITE,
   COLOR_WHITE};
   


/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(ssfire_init)
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    dispatcher_listen(ctk_signal_screensaver_start);
    dispatcher_listen(ctk_signal_screensaver_stop);
    dispatcher_listen(ctk_signal_screensaver_uninstall);      
  }
}
/*-----------------------------------------------------------------------------------*/
static void
fire_quit(void)
{
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static void
fire_init(void)
{
  unsigned char *ptr, *cptr;
  
  SID.v3.freq = 0xffff;
  SID.v3.ctrl = 0x80;
  SID.amp = 0;  

  VIC.ctrl1 = 0x1b;  /* $D011 */
  VIC.addr  = 0x17;  /* $D018 */
  VIC.ctrl2 = 0xc8;  /* $D016 */
  VIC.bordercolor = 0x00; /* $D020 */
  VIC.bgcolor0 = 0x00; /* $D021 */  
  CIA2.pra  = 0x03;  /* $DD00 */

  /* Fill screen with inverted spaces. */
  cptr = COLOR_RAM;
  for(ptr = (unsigned char *)0x0400;
      ptr != (unsigned char *)0x07e8;
      ++ptr) {
    *ptr = 0xa0;
    *cptr++ = 0x00;
  }
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(ssfire_sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_screensaver_start) {
    fire_init();
    save = 1;
  } else if(s == ctk_signal_screensaver_stop) {
    save = 0;
    ctk_draw_init();
    ctk_desktop_redraw(NULL);
  } else if(s == ctk_signal_screensaver_uninstall) {
    fire_quit();
    ctk_draw_init();
    ctk_desktop_redraw(NULL);
  }
}
/*-----------------------------------------------------------------------------------*/
void
ssfire_idle(void)
{
  if(save) {
  
  /* Calculate new flames. */
  asm("ldx #0");
  asm("loop:");
  asm("lda _flames+7,x");
  asm("clc");
  asm("adc _flames+8,x");
  asm("adc _flames+9,x");
  asm("adc _flames+16,x");
  asm("lsr");
  asm("lsr");
  asm("sta _flames,x");
  asm("inx");
  asm("cpx #(8*15)");
  asm("bne loop");

  /* Fill last line with pseudo-random data from noise generator on
     voice 3. */
  asm("ldx #$05");
  asm("loop2:");
  asm("ldy #$20");
  asm("delay:");
  asm("dey");
  asm("bne delay");
  asm("lda $d41b");
  asm("and #$0f");
  asm("sta _flames+8*15+1,x");
  asm("dex");
  asm("bpl loop2");

  /* Display flames on screen. */  
  flameptr = flames;
  colorptr1 = COLOR_RAM + 40*10;
  colorptr2 = colorptr1 + 0x20;
  for(y = 0; y < 15; ++y) {
    for(x = 0; x < 8; ++x) {
      *colorptr1 = *colorptr2 = flamecolors[*flameptr++];
      ++colorptr1;
      ++colorptr2;
    }
    colorptr1 += 0x20;
    colorptr2 += 0x20;
  }
  
  }
}
/*-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------*/
