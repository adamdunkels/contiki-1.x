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
 * $Id: dispatcher.c,v 1.9 2003/08/05 13:51:08 adamdunkels Exp $
 *
 */

#include "ek.h"
#include "dispatcher.h"

#include "uip.h"

/*#include <time.h>*/
/*#include <conio.h>*/

ek_id_t dispatcher_current;
struct dispatcher_proc *dispatcher_procs;
static ek_id_t ids = 1;

ek_signal_t dispatcher_signal_quit;

static ek_signal_t lastsig = 1;

struct listenport {
  u16_t port;
  ek_id_t id;
};
static struct listenport listenports[UIP_LISTENPORTS];
/*static u8_t listenportsptr;*/

#if CC_FUNCTION_POINTER_ARGS
#else /* CC_FUNCTION_POINTER_ARGS */
ek_signal_t dispatcher_sighandler_s;
ek_data_t dispatcher_sighandler_data;

void *dispatcher_uipcall_state;
#endif /* CC_FUNCTION_POINTER_ARGS */      

/*-----------------------------------------------------------------------------------*/
ek_signal_t
dispatcher_sigalloc(void)
{
  return lastsig++;
}
/*-----------------------------------------------------------------------------------*/
ek_id_t
dispatcher_start(CC_REGISTER_ARG struct dispatcher_proc *p)
{
  ek_id_t id;
  struct dispatcher_proc *q;
  
 again:
  do {
    id = ids++;
  } while(id == EK_ID_NONE);
  
  /* Check if this ID is use. */
  for(q = dispatcher_procs; q != NULL; q = q->next) {
    if(id == q->id) {
      goto again;
    }
  }

  /* Put first on the procs list.*/
  p->next = dispatcher_procs;
  dispatcher_procs = p;
    
  p->id = id;

  dispatcher_current = id;

  /* All processes must listen to the dispatcher_signal_quit signal. */
  dispatcher_listen(dispatcher_signal_quit);
  return id;
}
/*-----------------------------------------------------------------------------------*/
void
dispatcher_exit(CC_REGISTER_ARG struct dispatcher_proc *p)
{
  struct dispatcher_proc *q;
  unsigned char i;

  /* Unlisten all signals. */
  ek_unlisten(p->id);
  
  /* If this process has any listening TCP ports, we remove them. */
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    if(listenports[i].id == p->id) {
      listenports[i].port = 0;
    } 
  }
  /* Remove process from the process list. */
  if(p == dispatcher_procs) {
    dispatcher_procs = dispatcher_procs->next;    
  } else {
    for(q = dispatcher_procs; q != NULL; q = q->next) {
      if(q->next == p) {
	q->next = p->next;
	break;
      }
    }
  }
}
/*-----------------------------------------------------------------------------------*/
void
ek_idle(void)
{
  struct dispatcher_proc *p;
  for(p = dispatcher_procs; p != NULL; p = p->next) {
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
  for(p = dispatcher_procs; p != NULL; p = p->next) {
    if(p->id == id &&
       p->signalhandler != NULL) {
      dispatcher_current = id;
#if CC_FUNCTION_POINTER_ARGS
      p->signalhandler(s, data);
#else /* CC_FUNCTION_POINTER_ARGS */
      dispatcher_sighandler_s = s;
      dispatcher_sighandler_data = data;
      p->signalhandler();
#endif /* CC_FUNCTION_POINTER_ARGS */      
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
  register struct dispatcher_proc *p;
  register struct dispatcher_uipstate *s;
  u8_t i;

  s = (struct dispatcher_uipstate *)uip_conn->appstate;

  /* If this is a connection request for a listening port, we must
     mark the connection with the right process ID. */
  if(uip_connected()) {
    for(i = 0; i < UIP_LISTENPORTS; ++i) {
      if(listenports[i].port == uip_conn->lport) {
	s->id = listenports[i].id;
	s->state = NULL;
	break;
      }
    }
  }
  

  for(p = dispatcher_procs; p != NULL; p = p->next) {
    if(p->id == s->id &&
       p->uiphandler != NULL) {
      dispatcher_current = p->id;
#if CC_FUNCTION_POINTER_ARGS
      p->uiphandler(s->state);
#else /* CC_FUNCTION_POINTER_ARGS */
      dispatcher_uipcall_state = s->state;
      p->uiphandler();
#endif /* CC_FUNCTION_POINTER_ARGS */            
      return;
    }
  }
  /* If we get here, the process that created the connection has
     exited, so we abort this connection. */
  uip_abort();
}
#endif /* WITH_UIP */
/*-----------------------------------------------------------------------------------*/
void
dispatcher_uiplisten(u16_t port)
{
  unsigned char i;

  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    if(listenports[i].port == 0) {
      listenports[i].port = HTONS(port);
      listenports[i].id = dispatcher_current;
      break;
    }
  }

  /*  ++listenportsptr;*/

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
  for(p = dispatcher_procs; p != NULL; p = p->next) {
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
  unsigned char i;

  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    listenports[i].port = 0;
  }

  lastsig = 1;
  dispatcher_signal_quit = dispatcher_sigalloc();
  
}
/*-----------------------------------------------------------------------------------*/
