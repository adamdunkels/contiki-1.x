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
 * $Id: ctk-conio-conf.h,v 1.1 2003/04/12 01:46:39 sannyx Exp $
 *
 */

#ifndef __CTK_CONIO_CONF_H__
#define __CTK_CONIO_CONF_H__

#if 0
/* Light gray inverted color scheme: */
#define BORDERCOLOR         COLOR_WHITE
#define SCREENCOLOR         COLOR_WHITE

#define WINDOWCOLOR_FOCUS   COLOR_BLACK
#define WINDOWCOLOR         COLOR_GRAY2

#define DIALOGCOLOR         COLOR_RED

#define WIDGETCOLOR_HLINK   COLOR_BLUE
#define WIDGETCOLOR_FWIN    COLOR_BLACK
#define WIDGETCOLOR         COLOR_GRAY1
#define WIDGETCOLOR_DIALOG  COLOR_RED
#define WIDGETCOLOR_FOCUS   COLOR_BLACK

#define MENUCOLOR           COLOR_BLACK
#define OPENMENUCOLOR       COLOR_BLACK
#define ACTIVEMENUITEMCOLOR COLOR_BLACK
#endif /* 0 */

#if 0
/* Blue color scheme: */
#define BORDERCOLOR         COLOR_LIGHTBLUE
#define SCREENCOLOR         COLOR_BLUE

#define WINDOWCOLOR_FOCUS   COLOR_LIGHTBLUE
#define WINDOWCOLOR         COLOR_GRAY1

#define DIALOGCOLOR         COLOR_WHITE

#define WIDGETCOLOR_HLINK   COLOR_CYAN
#define WIDGETCOLOR_FWIN    COLOR_LIGHTBLUE
#define WIDGETCOLOR         COLOR_GRAY1
#define WIDGETCOLOR_DIALOG  COLOR_WHITE
#define WIDGETCOLOR_FOCUS   COLOR_YELLOW

#define MENUCOLOR           COLOR_WHITE
#define OPENMENUCOLOR       COLOR_LIGHTBLUE
#define ACTIVEMENUITEMCOLOR COLOR_YELLOW
#endif /* 0 */

#if 1
/* Black and white monocrome color scheme: */
#define BORDERCOLOR         COLOR_BLACK
#define SCREENCOLOR         COLOR_BLACK

#define WINDOWCOLOR_FOCUS   COLOR_WHITE
#define WINDOWCOLOR         COLOR_WHITE

#define DIALOGCOLOR         COLOR_WHITE

#define WIDGETCOLOR_HLINK   COLOR_WHITE
#define WIDGETCOLOR_FWIN    COLOR_WHITE
#define WIDGETCOLOR         COLOR_WHITE
#define WIDGETCOLOR_DIALOG  COLOR_WHITE
#define WIDGETCOLOR_FOCUS   COLOR_WHITE

#define MENUCOLOR           COLOR_WHITE
#define OPENMENUCOLOR       COLOR_WHITE
#define ACTIVEMENUITEMCOLOR COLOR_WHITE
#endif /* 0 */

#if 0
/* Blue monocrome color scheme: */
#define BORDERCOLOR         COLOR_BLUE
#define SCREENCOLOR         COLOR_BLUE

#define WINDOWCOLOR_FOCUS   COLOR_LIGHTBLUE
#define WINDOWCOLOR         COLOR_LIGHTBLUE

#define DIALOGCOLOR         COLOR_LIGHTBLUE

#define WIDGETCOLOR_HLINK   COLOR_LIGHTBLUE
#define WIDGETCOLOR_FWIN    COLOR_LIGHTBLUE
#define WIDGETCOLOR         COLOR_LIGHTBLUE
#define WIDGETCOLOR_DIALOG  COLOR_LIGHTBLUE
#define WIDGETCOLOR_FOCUS   COLOR_LIGHTBLUE

#define MENUCOLOR           COLOR_LIGHTBLUE
#define OPENMENUCOLOR       COLOR_LIGHTBLUE
#define ACTIVEMENUITEMCOLOR COLOR_LIGHTBLUE
#endif /* 0 */

#if 0
/* Gray color scheme: */
#define BORDERCOLOR         COLOR_GRAY1
#define SCREENCOLOR         COLOR_GRAY1

#define WINDOWCOLOR_FOCUS   COLOR_GRAY3
#define WINDOWCOLOR         COLOR_GRAY2

#define DIALOGCOLOR         COLOR_WHITE

#define WIDGETCOLOR_HLINK   COLOR_LIGHTBLUE
#define WIDGETCOLOR_FWIN    COLOR_GRAY3
#define WIDGETCOLOR         COLOR_GRAY2
#define WIDGETCOLOR_DIALOG  COLOR_WHITE
#define WIDGETCOLOR_FOCUS   COLOR_YELLOW

#define MENUCOLOR           COLOR_GRAY3
#define OPENMENUCOLOR       COLOR_WHITE
#define ACTIVEMENUITEMCOLOR COLOR_YELLOW
#endif /* 0 */

#if 0
/* Red color scheme: */
#define BORDERCOLOR         COLOR_BLACK
#define SCREENCOLOR         COLOR_BLACK

#define WINDOWCOLOR_FOCUS   COLOR_LIGHTRED
#define WINDOWCOLOR         COLOR_RED

#define DIALOGCOLOR         COLOR_WHITE

#define WIDGETCOLOR_HLINK   COLOR_LIGHTBLUE
#define WIDGETCOLOR_FWIN    COLOR_YELLOW
#define WIDGETCOLOR         COLOR_LIGHTRED
#define WIDGETCOLOR_DIALOG  COLOR_WHITE
#define WIDGETCOLOR_FOCUS   COLOR_YELLOW

#define MENUCOLOR           COLOR_LIGHTRED
#define OPENMENUCOLOR       COLOR_WHITE
#define ACTIVEMENUITEMCOLOR COLOR_YELLOW
#endif /* 0 */


#endif /* __CTK_CONIO_CONF_H__ */
