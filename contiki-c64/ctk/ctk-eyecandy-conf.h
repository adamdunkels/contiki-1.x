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
 * $Id: ctk-eyecandy-conf.h,v 1.2 2003/03/28 12:16:23 adamdunkels Exp $
 *
 */

#ifndef __CTK_EYECANDY_CONF_H__
#define __CTK_EYECANDY_CONF_H__

static unsigned char colors[] = {
  /* Window background colors. */
  COLOR(COLOR_GRAY2, COLOR_GRAY1),
  COLOR(COLOR_GRAY2, COLOR_GRAY3),
  COLOR(COLOR_GRAY3, COLOR_GRAY1),
  COLOR(COLOR_GRAY3, COLOR_WHITE),
  COLOR(COLOR_WHITE, COLOR_BLACK),
  COLOR(COLOR_WHITE, COLOR_BLACK),

  /* Separator colors. */
  COLOR(COLOR_GRAY2, COLOR_GRAY1),
  COLOR(COLOR_GRAY2, COLOR_GRAY1),
  COLOR(COLOR_GRAY3, COLOR_GRAY2),
  COLOR(COLOR_GRAY3, COLOR_GRAY2),
  COLOR(COLOR_WHITE, COLOR_GRAY1),
  COLOR(COLOR_WHITE, COLOR_GRAY1),
  
  /* Label colors. */
  COLOR(COLOR_GRAY2, COLOR_GRAY1),
  COLOR(COLOR_GRAY2, COLOR_GRAY1),
  COLOR(COLOR_WHITE, COLOR_BLACK),
  COLOR(COLOR_WHITE, COLOR_BLACK),
  COLOR(COLOR_WHITE, COLOR_BLACK),
  COLOR(COLOR_WHITE, COLOR_BLACK),
  
  /* Button colors.*/
  COLOR(COLOR_GRAY2, COLOR_GRAY1),
  COLOR(COLOR_GRAY1, COLOR_GRAY2),
  COLOR(COLOR_GRAY3, COLOR_GRAY1),
  COLOR(COLOR_GRAY1, COLOR_GRAY3),
  COLOR(COLOR_GRAY3, COLOR_GRAY1),
  COLOR(COLOR_GRAY1, COLOR_GRAY3),

  /* Hyperlink colors. */
  COLOR(COLOR_GRAY2, COLOR_LIGHTBLUE),
  COLOR(COLOR_GRAY2, COLOR_LIGHTBLUE),
  COLOR(COLOR_WHITE, COLOR_BLUE),
  COLOR(COLOR_BLUE, COLOR_WHITE),
  COLOR(COLOR_WHITE, COLOR_BLUE),
  COLOR(COLOR_BLUE, COLOR_WHITE),

  /* Text entry colors. */
  COLOR(COLOR_GRAY2, COLOR_GRAY1),
  COLOR(COLOR_GRAY2, COLOR_BLACK),
  COLOR(COLOR_WHITE, COLOR_BLACK),
  COLOR(COLOR_WHITE, COLOR_BLACK),
  COLOR(COLOR_GRAY3, COLOR_BLACK),
  COLOR(COLOR_GRAY3, COLOR_BLACK),

  /* Bitmap colors. */
  COLOR(COLOR_GRAY2, COLOR_GRAY1),
  COLOR(COLOR_GRAY1, COLOR_BLACK),
  COLOR(COLOR_WHITE, COLOR_GRAY1),
  COLOR(COLOR_GRAY3, COLOR_BLACK),
  COLOR(COLOR_GRAY3, COLOR_BLACK),
  COLOR(COLOR_GRAY3, COLOR_BLACK),

  /* Button left corner colors.*/
  COLOR(COLOR_GRAY3, COLOR_GRAY2),
  COLOR(COLOR_GRAY1, COLOR_GRAY2),
  COLOR(COLOR_WHITE, COLOR_GRAY3),
  COLOR(COLOR_GRAY1, COLOR_GRAY3),
  COLOR(COLOR_WHITE, COLOR_GRAY3),
  COLOR(COLOR_GRAY1, COLOR_GRAY3),
  
  /* Button right corner colors.*/
  COLOR(COLOR_GRAY1, COLOR_GRAY2),
  COLOR(COLOR_GRAY3, COLOR_GRAY2),
  COLOR(COLOR_GRAY1, COLOR_GRAY3),
  COLOR(COLOR_WHITE, COLOR_GRAY3),
  COLOR(COLOR_GRAY1, COLOR_GRAY3),
  COLOR(COLOR_WHITE, COLOR_GRAY3),
  
  /* Menu colors */
  COLOR(COLOR_YELLOW, COLOR_BLACK),
  COLOR(COLOR_BLACK, COLOR_YELLOW),
  COLOR(COLOR_WHITE, COLOR_BLACK),
};



#define BGCOLOR1 0x06
#define BGCOLOR2 0x04
#define BGCOLOR3 0x0a
#define BGCOLOR4 0x07



#endif /* __CTK_EYECANDY_CONF_H__ */
