/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * $Id: ctk-arch.h,v 1.2 2004/07/31 14:55:17 oliverschmidt Exp $
 *
 */
#ifndef __CTK_ARCH_H__
#define __CTK_ARCH_H__

#define CH_ENTER	'\r'
#define CH_DEL		'\b'
#define CH_CURS_UP  	-4
#define CH_CURS_LEFT	-5
#define CH_CURS_RIGHT	-6
#define CH_CURS_DOWN	-7

#define CH_ULCORNER	0xDA
#define CH_URCORNER	0xBF
#define CH_LLCORNER	0xC0
#define CH_LRCORNER	0xD9

#include "ctk-conio.h"
#include "ctk-console.h"

#undef ctk_arch_getkey
#define cputc		putch

#define COLOR_BLACK	0
#define COLOR_BLUE	1
#define COLOR_GRAY	1 | 2 | 4
#define COLOR_CYAN	1 | 2 | 8
#define COLOR_YELLOW	2 | 4 | 8
#define COLOR_WHITE	1 | 2 | 4 | 8

#endif /* __CTK_ARCH_H__ */
