/*
 * Copyright (c) 2003, Adam Dunkels.
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
 * This file is part of the Contiki desktop environment
 *
 * $Id: ctk-textedit.c,v 1.1 2003/08/11 22:22:44 adamdunkels Exp $
 *
 */


#include "ctk-textedit.h"

/*-----------------------------------------------------------------------------------*/
void
ctk_textedit_add(struct ctk_window *w,
		 struct ctk_textedit *t)
{
  CTK_WIDGET_ADD(w, t);
}
/*-----------------------------------------------------------------------------------*/
void
ctk_textedit_sighandler(struct ctk_textedit *t,
			ek_signal_t s,
			ek_data_t data)
{
  char *textptr, *textptr2;
  unsigned char len;
  
  if(s == ctk_signal_keypress) {
    CTK_WIDGET_FOCUS(t->label.window, &t->label);
    textptr = &(t->label.text[t->ypos * t->label.w + t->xpos]);
    *textptr &= 0x7f;
    switch((ctk_arch_key_t)data) {
    case CH_CURS_DOWN:
      if(t->ypos < t->label.h) {
	++t->ypos;
      }
      break; 
    case CH_CURS_UP:
      if(t->ypos > 0) {
	--t->ypos;
      }
      break; 
    case CH_CURS_RIGHT:
      if(t->xpos < t->label.w) {
	++t->xpos;
      }
      break; 
    case CH_CURS_LEFT:
      if(t->xpos > 0) {
	--t->xpos;
      } else {
	if(t->ypos > 0) {
	  --t->ypos;
	  t->xpos = t->label.w - 1;
	}       
      }
      break;
    case CH_ENTER:
      t->xpos = 0;
      if(t->ypos < t->label.h) {
	++t->ypos;
      }
      break;
    case CH_DEL:
      len = t->label.w - t->xpos;
      if(t->xpos > 0 && len > 0) {
	strncpy(textptr - 1, textptr,
		len);
	*(textptr + len - 1) = 0;
	--t->xpos;
      }
      break;      
    default:
      len = t->label.w - t->xpos;
      if(len > 0) {
	textptr2 = textptr + len - 1;
	while(textptr2 + 1 > textptr) {
	  *(textptr2 + 1) = *textptr2;
	  --textptr2;
	}
	
	*textptr = (char)data;
	++t->xpos;
	if(t->xpos == t->label.w) {
	  t->xpos = 0;
	  if(t->ypos < t->label.h - 1) {
	    ++t->ypos;
	  }
	}
      }
      break;
    }
    textptr = &(t->label.text[t->ypos * t->label.w + t->xpos]);
    *textptr |= 0x80;
    CTK_WIDGET_REDRAW(&t->label);
  } else if(s == ctk_signal_widget_activate &&
	    data == (ek_data_t)t) {
    textptr = &(t->label.text[t->ypos * t->label.w + t->xpos]);
    *textptr &= 0x7f;
    t->xpos = 0;
    if(t->ypos < t->label.h - 1) {
      ++t->ypos;
    }
    textptr = &(t->label.text[t->ypos * t->label.w + t->xpos]);
    *textptr |= 0x80;
    CTK_WIDGET_REDRAW(&t->label);
  }
}
/*-----------------------------------------------------------------------------------*/
