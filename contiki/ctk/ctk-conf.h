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
 * $Id: ctk-conf.h,v 1.2 2003/03/28 12:12:26 adamdunkels Exp $
 *
 */

#ifndef __CTK_CONF_H__
#define __CTK_CONF_H__

/*
 * This file is used for setting various compile time settings for the
 * CTK GUI toolkit.
*/

/* Defines which key that is to be used for activating the menus */
#define CTK_CONF_MENU_KEY             CH_F1

/* Defines which key that is to be used for switching the frontmost
   window.  */
#define CTK_CONF_WINDOWSWITCH_KEY     CH_F3

/* Toggles support for desktop icons. */
#define CTK_CONF_ICONS                1 /* 107 bytes */

/* Toggles support for movable windows. */
#define CTK_CONF_WINDOWMOVE           1 /* 333 bytes */

/* Toggles support for closable windows. */
#define CTK_CONF_WINDOWCLOSE          1 /* 14 bytes */

/* Toggles support for multiline text entry editing. */
#define CTK_CONF_TEXTENTRY_MULTILINE  1 /* 118 bytes */

/* Toggles support for menus. */
#define CTK_CONF_MENUS                1 /* 1384 bytes */

/* Defines the default width of a menu. */
#define CTK_CONF_MENUWIDTH            16
/* The maximum number of menu items in each menu. */
#define CTK_CONF_MAXMENUITEMS         10

#endif /* __CTK_CONF_H__ */
