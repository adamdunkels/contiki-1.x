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
 * $Id: ctk-conio-conf.h,v 1.3 2004/08/12 22:07:43 oliverschmidt Exp $
 *
 */

#ifndef __CTK_CONIO_CONF_H__
#define __CTK_CONIO_CONF_H__

#define BORDERCOLOR         COLOR_BLACK
#define SCREENCOLOR         COLOR_BLACK
#define BACKGROUNDCOLOR     COLOR_BLACK

#define WINDOWCOLOR         COLOR_GRAY   | COLOR_BLUE * 0x10
#define WINDOWCOLOR_FOCUS   COLOR_WHITE  | COLOR_BLUE * 0x10
#define DIALOGCOLOR         COLOR_WHITE  | COLOR_BLUE * 0x10

#define WIDGETCOLOR         COLOR_GRAY   | COLOR_BLUE * 0x10
#define WIDGETCOLOR_FWIN    COLOR_WHITE  | COLOR_BLUE * 0x10
#define WIDGETCOLOR_DIALOG  COLOR_WHITE  | COLOR_BLUE * 0x10
#define WIDGETCOLOR_HLINK   COLOR_CYAN   | COLOR_BLUE * 0x10
#define WIDGETCOLOR_FOCUS   COLOR_YELLOW | COLOR_BLUE * 0x10

#define MENUCOLOR           COLOR_WHITE  | COLOR_BLUE * 0x10
#define OPENMENUCOLOR       COLOR_WHITE  | COLOR_BLUE * 0x10
#define ACTIVEMENUITEMCOLOR COLOR_YELLOW | COLOR_BLUE * 0x10

#endif /* __CTK_CONIO_CONF_H__ */
