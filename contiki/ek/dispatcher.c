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
 * $Id: dispatcher.c,v 1.12 2003/08/15 18:49:53 adamdunkels Exp $
 *
 */

#include "ek.h"
#include "dispatcher.h"

#include "uip.h"

/*#include <time.h>*/
/*#include <conio.h>*/

ek_id_t dispatcher_current;
struct dispatcher_proc *dispatcher_procs;

static struct dispatcher_proc *curproc;
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

struct signal_data {
  ek_signal_t s;
  ek_data_t data;
  ek_id_t id;
};

/* Static variables. */
static ek_num_signals_t nsignals, fsignal;
static struct signal_data signals[EK_CONF_NUMSIGNALS];

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

  /* Make sure we know which processes we are running at the
     moment. */
  dispatcher_current = id;
  curproc = p;
  
  /* All processes must listen to the dispatcher_signal_quit
     signal. */
  dispatcher_listen(dispatcher_signal_quit);
  return id;
}
/*-----------------------------------------------------------------------------------*/
void
dispatcher_exit(CC_REGISTER_ARG struct dispatcher_proc *p)
{
  struct dispatcher_proc *q;
  static unsigned char i;
  struct listenport *l;

  /* If this process has any listening TCP ports, we remove them. */
  l = listenports;
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    if(l->id == p->id) {
      uip_unlisten(l->port);
      l->port = 0;
      l->id = EK_ID_NONE;
    }
    ++l;
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

  dispatcher_current = EK_ID_NONE;
  curproc = NULL;
}
/*-----------------------------------------------------------------------------------*/
ek_clock_t
ek_clock(void)
{
  return clock();
}
/*-----------------------------------------------------------------------------------*/
#ifdef WITH_UIP
void
dispatcher_uipcall(void)     
{
  register struct dispatcher_proc *p;
  register struct dispatcher_uipstate *s;
  static u8_t i;
  struct listenport *l;


  s = (struct dispatcher_uipstate *)uip_conn->appstate;

  /* If this is a connection request for a listening port, we must
     mark the connection with the right process ID. */
  if(uip_connected()) {
    l = &listenports[0];
    for(i = 0; i < UIP_LISTENPORTS; ++i) {
      if(l->port == uip_conn->lport &&
	 l->id != EK_ID_NONE) {
	s->id = l->id;
	s->state = NULL;
	break;
      }
      ++l;
    }
  }
  

  for(p = dispatcher_procs; p != NULL; p = p->next) {
    if(p->id == s->id &&
       p->uiphandler != NULL) {
      dispatcher_current = p->id;
      curproc = p;
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
  static unsigned char i;
  struct listenport *l;

  l = listenports;
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    if(l->port == 0) {
      l->port = HTONS(port);
      l->id = dispatcher_current;
#ifdef WITH_UIP
      uip_listen(port);
#endif /* WITH_UIP */      
      break;
    }
    ++l;
  }

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
  if(curproc != NULL) {
    curproc->signals[s] = 1;
  }
}
/*-----------------------------------------------------------------------------------*/
void
dispatcher_timer(ek_signal_t s, ek_data_t data, ek_ticks_t t)
{
#if EK_CONF_TIMERS
  ek_timer(s, data, dispatcher_current, t);
#endif /* EK_CONF_TIMERS */
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
  static unsigned char i;
  
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    listenports[i].port = 0;
  }

  lastsig = 1;
  dispatcher_signal_quit = dispatcher_sigalloc();

  nsignals = fsignal = 0;
}
/*-----------------------------------------------------------------------------------*/
void
dispatcher_signal(void)
{
  struct dispatcher_proc *p;
  
  static ek_signal_t s;
  static ek_data_t data;
  static ek_id_t id;
  static ek_num_listeners_t i;
  
  /* If there are any signals in the queue, take the first one and
     walk through the list of processes to see if the signal should be
     delivered to any of them. If so, we call the signal handler
     function for the process. We only process one signal at a time
     and call the idle handlers inbetween. */

  if(nsignals > 0) {
    /* There are signals that we should deliver. */
    s = signals[fsignal].s;
    
    data = signals[fsignal].data;
    id = signals[fsignal].id;

    /* Since we have seen the new signal, we move pointer upwards
       and decrese number. */
    fsignal = (fsignal + 1) % EK_CONF_NUMSIGNALS;
    --nsignals;

    for(p = dispatcher_procs; p != NULL; p = p->next) {      
      if((id == DISPATCHER_BROADCAST ||
	  p->id == id) &&
	 p->signals[s] != 0 &&
	 p->signalhandler != NULL) {
	dispatcher_current = p->id;
	curproc = p;
#if CC_FUNCTION_POINTER_ARGS
	p->signalhandler(s, data);
#else /* CC_FUNCTION_POINTER_ARGS */
	dispatcher_sighandler_s = s;
	dispatcher_sighandler_data = data;
	p->signalhandler();
#endif /* CC_FUNCTION_POINTER_ARGS */      
      }
    }  
  }

}
/*-----------------------------------------------------------------------------------*/
void
dispatcher_idle(void)
{
  struct dispatcher_proc *p;
  
  /* Call idle handlers. */
  for(p = dispatcher_procs; p != NULL; p = p->next) {
    if(p->idle != NULL) {
      dispatcher_current = p->id;
      curproc = p;
      p->idle();
    }
  }
  
}
/*-----------------------------------------------------------------------------------*/
void
dispatcher_run(void)
{
  while(1) {
    /* Process one signal */
    dispatcher_signal();

    /* Run "idle" handlers */
    dispatcher_idle();
  }
}
/*-----------------------------------------------------------------------------------*/
ek_err_t
dispatcher_emit(ek_signal_t s, ek_data_t data, ek_id_t id)
{
  static unsigned char snum;
  
  if(nsignals == EK_CONF_NUMSIGNALS) {
    return EK_ERR_FULL;
  }
  
  if(s != EK_SIGNAL_NONE) {
    snum = (fsignal + nsignals) % EK_CONF_NUMSIGNALS;
    signals[snum].s = s;
    signals[snum].data = data;
    signals[snum].id = id;
    ++nsignals;
  }
  
  return EK_ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
