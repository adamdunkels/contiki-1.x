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
 * This file is part of the "ek" event kernel.
 *
 * $Id: dispatcher.c,v 1.1 2003/03/19 14:16:05 adamdunkels Exp $
 *
 */

#include "ek.h"
#include "dispatcher.h"

#include "uip.h"

#include <time.h>
/*#include <conio.h>*/

ek_id_t dispatcher_current;
static struct dispatcher_proc *procs;
static ek_id_t ids = 0;


static ek_signal_t lastsig = 1;

struct listenport {
  u16_t port;
  ek_id_t id;
};
static struct listenport listenports[UIP_LISTENPORTS];
static u8_t listenportsptr;

/*-----------------------------------------------------------------------------------*/
ek_signal_t
dispatcher_sigalloc(void)
{
  return lastsig++;
}
/*-----------------------------------------------------------------------------------*/
ek_id_t
dispatcher_start(struct dispatcher_proc *p)
{
  ek_id_t id;

  id = ids++;
  
  if(procs == NULL) {
    procs = p;
  } else {
    p->next = procs;
    procs = p;
  }
  p->id = id;

  dispatcher_current = id;
  
  return id;
}
/*-----------------------------------------------------------------------------------*/
void
ek_idle(void)
{
  struct dispatcher_proc *p;
  for(p = procs; p != NULL; p = p->next) {
    if(p->idle != NULL) {
      dispatcher_current = p->id;
      p->idle();
    }
  }
}
/*-----------------------------------------------------------------------------------*/
ek_clock_t
ek_clock(void)
{
  return clock();
}
/*-----------------------------------------------------------------------------------*/
ek_err_t
ek_dispatcher(ek_signal_t s, ek_data_t data, ek_id_t id)
{
  struct dispatcher_proc *p;
  for(p = procs; p != NULL; p = p->next) {
    if(p->id == id &&
       p->signalhandler != NULL) {
      dispatcher_current = id;
      p->signalhandler(s, data);
      return EK_ERR_OK;
    }
  }
  return EK_ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
#ifdef WITH_UIP
void
dispatcher_uipcall(void)     
{
  struct dispatcher_proc *p;
  struct dispatcher_uipstate *s;
  u8_t i;

  s = (struct dispatcher_uipstate *)uip_conn->appstate;

  /* If this is a connection request for a listening port, we must
     mark the connection with the right process ID. */
  if(uip_connected()) {
    for(i = 0; i < listenportsptr; ++i) {
      if(listenports[i].port == uip_conn->lport) {
	s->id = listenports[i].id;
	break;
      }
    }
  }
  

  for(p = procs; p != NULL; p = p->next) {
    if(p->id == s->id &&
       p->uiphandler != NULL) {
      dispatcher_current = p->id;
      p->uiphandler(s->state);
    }
  }
}
#endif /* WITH_UIP */
/*-----------------------------------------------------------------------------------*/
void
dispatcher_uiplisten(u16_t port)
{
  listenports[listenportsptr].port = htons(port);
  listenports[listenportsptr].id = dispatcher_current;

  ++listenportsptr;

#ifdef WITH_UIP
  uip_listen(port);
#endif /* WITH_UIP */
}
/*-----------------------------------------------------------------------------------*/
void
dispatcher_markconn(struct uip_conn *conn,
		    void *appstate)
{
  struct dispatcher_uipstate *s;

  s = (struct dispatcher_uipstate *)conn->appstate;
  s->id = dispatcher_current;
  s->state = appstate;
}
/*-----------------------------------------------------------------------------------*/
void
dispatcher_listen(ek_signal_t s)
{
  ek_listen(s, dispatcher_current);
}
/*-----------------------------------------------------------------------------------*/
void
dispatcher_timer(ek_signal_t s, ek_data_t data, ek_ticks_t t)
{
  ek_timer(s, data, dispatcher_current, t);
}
/*-----------------------------------------------------------------------------------*/
struct dispatcher_proc *
dispatcher_process(ek_id_t id)
{
  struct dispatcher_proc *p;
  for(p = procs; p != NULL; p = p->next) {
    if(p->id == id) {
      return p;
    }
  }
  return NULL;
}
/*-----------------------------------------------------------------------------------*/
void
dispatcher_init(void)
{
  listenportsptr = 0;
}
/*-----------------------------------------------------------------------------------*/
