
#include <c64.h>

#include "libconio.h"

#include "ctk-hires-asm.h"

unsigned char ctk_hires_cursx, ctk_hires_cursy;
unsigned char ctk_hires_reversed;
unsigned char ctk_hires_color;
unsigned char ctk_hires_underline = 0;

#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH  80

#define SCREENADDR 0xdc00
#define HIRESADDR  0xe000

unsigned char ctk_80col_lefttab[256];
unsigned char ctk_80col_righttab[256];

unsigned short ctk_hires_yhiresaddr[25] =
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
void
ctk_arch_draw_char(char c,
		   unsigned char xpos,
		   unsigned char ypos,
		   unsigned char reversedflag,
		   unsigned char color)
{
  ctk_hires_cursx = xpos;
  ctk_hires_cursy = ypos;
  ctk_hires_reversed = reversedflag;
  ctk_hires_color = color;
  
  ctk_hires_cputc(c);
}
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
  asm("lda %v+%w", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, pointermaskcolor));
  asm("sta $d027");
  asm("lda %v+%w", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, pointercolor));
  asm("sta $d028");

  ptr1 = ctk_hires_theme.pointer;
  ptr2 = (unsigned char *)0xff40;
  
  for(i = 0; i < 0x80; ++i) {
    *ptr2++ = *ptr1++;
  }
#endif
  return;
}
#pragma optimize(pop)
/*---------------------------------------------------------------------------*/
