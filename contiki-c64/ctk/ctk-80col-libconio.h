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
 * $Id: ctk-80col-libconio.h,v 1.1 2004/08/09 20:53:08 adamdunkels Exp $
 *
 */

#ifndef __LIBCONIO_H__
#define __LIBCONIO_H__

#include "libconio-conf.h"

/* This function must be implemented specifically for the
   architecure: */
void ctk_arch_draw_char(char c,
			unsigned char xpos,
			unsigned char ypos,
			unsigned char reversedflag,
			unsigned char color);

/* Default definitions that should be overridden by calling module. */
#ifndef LIBCONIO_CONF_SCREEN_WIDTH
#define LIBCONIO_SCREEN_WIDTH 40
#else
#define LIBCONIO_SCREEN_WIDTH LIBCONIO_CONF_SCREEN_WIDTH
#endif /* LIBCONIO_SCREEN_WIDTH */

#ifndef LIBCONIO_CONF_SCREEN_HEIGHT
#define LIBCONIO_SCREEN_HEIGHT 25
#else
#define LIBCONIO_SCREEN_HEIGHT LIBCONIO_CONF_SCREEN_HEIGHT
#endif /* LIBCONIO_CONF_SCREEN_HEIGHT */


#ifndef _CONIO_H
/* These are function declarations for functions implemented in libconio.c */
unsigned char wherex(void);
unsigned char wherey(void);
void clrscr(void);
void __fastcall__ bgcolor(unsigned char c);
void __fastcall__ bordercolor(unsigned char c);
void __fastcall__ screensize(unsigned char *x, unsigned char *y);
void __fastcall__ revers(unsigned char c);
void __fastcall__ cputc(char c);
void __fastcall__ cputs(char *str);
void __fastcall__ cclear(unsigned char length);
void __fastcall__ chline(unsigned char length);
void __fastcall__ cvline(unsigned char length);
void __fastcall__ gotoxy(unsigned char x, unsigned char y);
void __fastcall__ cclearxy(unsigned char x, unsigned char y, unsigned char length);
void __fastcall__ chlinexy(unsigned char x, unsigned char y, unsigned char length);
void __fastcall__ cvlinexy(unsigned char x, unsigned char y, unsigned char length);
void __fastcall__ cputsxy(unsigned char x, unsigned char y, char *str);
void __fastcall__ cputcxy(unsigned char x, unsigned char y, char c);
void __fastcall__ textcolor(unsigned char c);
#endif /* _CONIO_H */


#endif /* __LIBCONIO_H__ */
