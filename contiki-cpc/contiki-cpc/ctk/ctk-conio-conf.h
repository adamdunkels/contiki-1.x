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
 * $Id: ctk-conio-conf.h,v 1.1 2006/04/17 15:11:47 kthacker Exp $
 *
 */

#ifndef __CTK_CONIO_CONF_H__
#define __CTK_CONIO_CONF_H__

/* Black and white monocrome color scheme: */
#define BORDERCOLOR         0
#define SCREENCOLOR         0

#define BACKGROUNDCOLOR		0


#define WINDOWCOLOR_FOCUS   1		/* colour of window which has focus */
#define WINDOWCOLOR         2		/* colour of window which doesn't have focus */

#define DIALOGCOLOR         1

#define WIDGETCOLOR_HLINK   3
#define WIDGETCOLOR_FWIN    2		/* colour of widget */
#define WIDGETCOLOR         2
#define WIDGETCOLOR_DIALOG  2
#define WIDGETCOLOR_FOCUS   3		/* colour of widge which has focus */

// 0 = blue
// 1 = yellow
// 2 = cyan
// 3 = red

#define MENUCOLOR           2
#define OPENMENUCOLOR       1	
#define ACTIVEMENUITEMCOLOR 3

#endif
