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
 * $Id: ctk-conf.h,v 1.8 2005/05/05 00:14:38 oliverschmidt Exp $
 *
 */

#ifndef __CTK_CONF_H__
#define __CTK_CONF_H__

/*
 * This file is used for setting various compile time settings for the
 * CTK GUI toolkit.
*/

/* Defines which key that is to be used for activating the menus */
#define CTK_CONF_MENU_KEY             CH_ESC

/* Defines which key that is to be used for switching the frontmost
   window.  */
#define CTK_CONF_WINDOWSWITCH_KEY     0x17	/* Ctrl-W */

/* Defines which key that is to be used for switching to the prevoius
   widget.  */
#define CTK_CONF_WIDGETUP_KEY         0x01	/* Ctrl-A */

/* Defines which key that is to be used for switching to the next
   widget.  */
#define CTK_CONF_WIDGETDOWN_KEY       '\t'	/* Tab or Ctrl-I */

/* Toggles mouse support (must have support functions in the
architecture specific files to work). */
#define CTK_CONF_MOUSE_SUPPORT        0 /* 1342 bytes */

/* Toggles support for icons. */
#ifdef __APPLE2__
#define CTK_CONF_ICONS                0 /* 107 bytes */
#else /* __APPLE2__ */
#define CTK_CONF_ICONS                1 /* 107 bytes */
#endif /* __APPLE2__ */

/* Toggles support for icon bitmaps. */
#define CTK_CONF_ICON_BITMAPS         0

/* Toggles support for icon textmaps. */
#ifdef __APPLE2__
#define CTK_CONF_ICON_TEXTMAPS        0
#else /* __APPLE2__ */
#define CTK_CONF_ICON_TEXTMAPS        1
#endif /* __APPLE2__ */

/* Toggles support for movable windows. */
#ifdef __APPLE2__
#define CTK_CONF_WINDOWMOVE           0 /* 333 bytes */
#else /* __APPLE2__ */
#define CTK_CONF_WINDOWMOVE           1 /* 333 bytes */
#endif /* __APPLE2__ */

/* Toggles support for closable windows. */
#define CTK_CONF_WINDOWCLOSE          1 /* 14 bytes */

/* Toggles support for menus. */
#define CTK_CONF_MENUS                1 /* 1384 bytes */

/* Defines the default width of a menu. */
#define CTK_CONF_MENUWIDTH            16
/* The maximum number of menu items in each menu. */
#define CTK_CONF_MAXMENUITEMS         13

/* Toggles support for screen savers. */
#define CTK_CONF_SCREENSAVER          0

#endif /* __CTK_CONF_H__ */
