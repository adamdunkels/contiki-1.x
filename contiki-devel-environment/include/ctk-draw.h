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
 * $Id: ctk-draw.h,v 1.2 2003/04/09 19:15:26 adamdunkels Exp $
 *
 */

#ifndef __CTK_DRAW_H__
#define __CTK_DRAW_H__

#include "ctk.h"

#include "ctk-arch.h"


/* See the file doc/ctk.txt for documentation on these function
   declarations. */

void ctk_draw_init(void);

void ctk_draw_clear(unsigned char clipy1, unsigned char clipy2);
void ctk_draw_clear_window(struct ctk_window *window,
			   unsigned char focus,
			   unsigned char clipy1, unsigned char clipy2);

void ctk_draw_widget(struct ctk_widget *w,
		     unsigned char focus,
		     unsigned char clipy1,
		     unsigned char clipy2);

void ctk_draw_window(struct ctk_window *window,
		     unsigned char focus,
		     unsigned char clipy1,
		     unsigned char clipy2);

void ctk_draw_dialog(struct ctk_window *dialog);

void ctk_draw_menus(struct ctk_menus *menus);



/* Returns width and height of screen. */
unsigned char ctk_draw_width(void);
unsigned char ctk_draw_height(void);


#endif /* __CTK_DRAW_H__ */
