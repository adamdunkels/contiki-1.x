/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * $Id: psock.c,v 1.1 2005/02/22 22:23:07 adamdunkels Exp $
 */

#include "psock.h"

#define PSOCK_STATE_NONE 0
#define PSOCK_STATE_ACKED 1
#define PSOCK_STATE_READ 2
#define PSOCK_STATE_BLOCKED_NEWDATA 3
#define PSOCK_STATE_BLOCKED_CLOSE 4
#define PSOCK_STATE_BLOCKED_SEND 5
#define PSOCK_STATE_DATA_SENT 6

/*---------------------------------------------------------------------------*/
static char
send_data(register struct psock *s)
{
  if(s->state != PSOCK_STATE_DATA_SENT || uip_rexmit()) {
    if(s->sendlen > uip_mss()) {
      uip_send(s->sendptr, uip_mss());
    } else {
      uip_send(s->sendptr, s->sendlen);
    }
    s->state = PSOCK_STATE_DATA_SENT;
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static char
data_sent(register struct psock *s)
{
  if(s->state == PSOCK_STATE_DATA_SENT && uip_acked()) {
    if(s->sendlen > uip_mss()) {
      s->sendlen -= uip_mss();
      s->sendptr += uip_mss();
    } else {
      s->sendptr += s->sendlen;
      s->sendlen = 0;
    }
    s->state = PSOCK_STATE_ACKED;
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
PT_THREAD(psock_send(register struct psock *s, const char *buf, unsigned int len))
{
  PT_BEGIN(&s->psockpt);

  if(len == 0) {
    PT_EXIT(&s->psockpt);
  }
  
  s->sendptr = buf;
  s->sendlen = len;

  s->state = PSOCK_STATE_NONE;
  
  while(s->sendlen > 0) {
    PT_WAIT_UNTIL(&s->psockpt, data_sent(s) & send_data(s));
  }

  s->state = PSOCK_STATE_NONE;
  
  PT_END(&s->psockpt);
}
/*---------------------------------------------------------------------------*/
PT_THREAD(psock_generator_send(register struct psock *s,
			       unsigned short (*generate)(void *), void *arg))
{
  PT_BEGIN(&s->psockpt);

  if(generate == NULL) {
    PT_EXIT(&s->psockpt);
  }
  
  s->state = PSOCK_STATE_NONE;
  s->sendlen = generate(arg);
  s->sendptr = uip_appdata;
  do {

    if(uip_rexmit()) {
      generate(arg);
    }
    PT_WAIT_UNTIL(&s->psockpt, data_sent(s) & send_data(s));
  } while(s->sendlen > 0);    
  
  s->state = PSOCK_STATE_NONE;
  
  PT_END(&s->psockpt);
}
/*---------------------------------------------------------------------------*/
char
psock_newdata(struct psock *s)
{
  if(s->readlen > 0) {
    /* Data in uip_appdata buffer that has not yet been read. */
    return 1;
  } else if(s->state == PSOCK_STATE_READ) {
    /* Data in uip_appdata buffer already consumed. */
    s->state = PSOCK_STATE_BLOCKED_NEWDATA;
    return 0;
  } else if(uip_newdata()) {
    /* There is new data that has not been consumed. */
    return 1;
  } else {
    /* There is no new data. */
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
PT_THREAD(psock_readto(register struct psock *psock, unsigned char c))
{
  PT_BEGIN(&psock->psockpt);

  uipbuf_setup(&psock->buf, psock->bufptr, psock->bufsize);
  
  /* XXX: Should add uipbuf_checkmarker() before do{} loop, if
     incoming data has been handled while waiting for a write. */

  do {
    if(psock->readlen == 0) {
      PT_WAIT_UNTIL(&psock->psockpt, psock_newdata(psock));
      psock->state = PSOCK_STATE_READ;
      psock->readptr = (u8_t *)uip_appdata;
      psock->readlen = uip_datalen();
    }
  } while((uipbuf_bufto(&psock->buf, c,
			&psock->readptr,
			&psock->readlen) & UIPBUF_FOUND) == 0);

  if(uipbuf_len(&psock->buf) == 0) {
    psock->state = PSOCK_STATE_NONE;
    PT_RESTART(&psock->psockpt);
  }  
  PT_END(&psock->psockpt);
}
/*---------------------------------------------------------------------------*/
void
psock_init(register struct psock *psock, char *buffer, unsigned int buffersize)
{
  psock->state = PSOCK_STATE_NONE;
  psock->readlen = 0;
  psock->bufptr = buffer;
  psock->bufsize = buffersize;
  uipbuf_setup(&psock->buf, buffer, buffersize);
  PT_INIT(&psock->pt);
  PT_INIT(&psock->psockpt);
}
/*---------------------------------------------------------------------------*/
