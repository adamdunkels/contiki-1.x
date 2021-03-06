/*
 * Copyright (c) 2004, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the Contiki operating system.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: popc.h,v 1.3 2005/02/22 22:23:08 adamdunkels Exp $
 */
#ifndef __POPC_H__
#define __POPC_H__

#include "psock.h"


struct popc_state {
  struct pt pt;
  struct psock s;
  struct uip_conn *conn;

  struct timer timer;
  
  char inputbuf[100];
  char outputbuf[10];
  
  char user[32], pass[32];

  char command;
  unsigned short num;
  unsigned short lines;
};

void popc_appcall(void *state);
void popc_init(void);
void *popc_connect(struct popc_state *s, u16_t *ipaddr, 
		  char *user, char *passwd);

void popc_retr(struct popc_state *s, unsigned short msg);
void popc_top(struct popc_state *s, unsigned short msg,
	      unsigned short numlines);


void popc_connected(struct popc_state *s);
void popc_messages(struct popc_state *s,
		   unsigned short num, unsigned long size);
void popc_msgbegin(struct popc_state *s);
void popc_msgline(struct popc_state *s, char *line, int len);
void popc_msgend(struct popc_state *s);


#endif /* __POPC_H__ */
