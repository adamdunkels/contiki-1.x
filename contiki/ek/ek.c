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
 * $Id: ek.c,v 1.3 2003/08/09 13:35:17 adamdunkels Exp $
 *
 */

#include "ek.h"
#include "ek-conf.h"

/* Structure definitions. */
struct ek_listener {
  ek_signal_t s;
  ek_id_t id;
};

#if EK_CONF_TIMERS
struct ek_timer {
  struct ek_timer *next;
  ek_ticks_t t;
  ek_signal_t s;
  ek_data_t data;
  ek_id_t id;
};
#endif /* EK_CONF_TIMERS */

struct ek_signal_data {
  ek_signal_t s;
  ek_data_t data;
  ek_id_t id;
};

/* Static variables. */
static ek_num_signals_t nsignals, fsignal;
static struct ek_signal_data signals[EK_CONF_NUMSIGNALS];

static struct ek_listener listeners[EK_CONF_NUMLISTENERS];

#if EK_CONF_TIMERS
static struct ek_timer timers[EK_CONF_NUMTIMERS];
static struct ek_timer *timers_free, *timers_used;
#endif /* EK_CONF_TIMERS */

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

/*-----------------------------------------------------------------------------------*/
void
ek_init(void)
{
  ek_num_timers_t i;
  
  nsignals = fsignal = 0;

#if EK_CONF_TIMERS
  timers_used = NULL;
  for(i = 0; i < EK_CONF_NUMTIMERS - 1; ++i) {
    timers[i].next = &timers[i+1];
  }
  timers[i].next = NULL;
  timers_free = &timers[0];
#endif /* EK_CONF_TIMERS */
}
/*-----------------------------------------------------------------------------------*/
void
ek_signals(void)
{
  static ek_signal_t s;
  static ek_data_t data;
  static ek_id_t id;
  static ek_num_listeners_t i;

  if(nsignals > 0) {
    /* There are signals that we should deliver. */
    s = signals[fsignal].s;
    
    data = signals[fsignal].data;
    id = signals[fsignal].id;
    
    /* Since we have seen the new signal, we move pointer upwards
       and decrese number. */
    fsignal = (fsignal + 1) & (EK_CONF_NUMSIGNALS - 1);
    --nsignals;
    /* Check for listeners. */
    for(i = 0; i < EK_CONF_NUMLISTENERS; ++i) {
      if(listeners[i].s == s &&
	 (id == EK_ID_NONE ||
	  id == listeners[i].id)) {
	ek_dispatcher(s, data, listeners[i].id);
      }
    }
  }
}
/*-----------------------------------------------------------------------------------*/
void
ek_run(void)
{
  static ek_signal_t s;
  static ek_data_t data;
  static ek_id_t id;
#if EK_CONF_TIMERS
  static ek_clock_t lasttime, curtime;
  static struct ek_timer *tptr;
#endif /* EK_CONF_TIMERS */

#if EK_CONF_TIMERS
  lasttime = ek_clock();
#endif /* EK_CONF_TIMERS */
  
  while(1) {
    /* Check the signal list to see if we have signals that should be
       emitted. */
    ek_signals();

#if EK_CONF_TIMERS
    /* If no signals should be emitted, we first check outstanding
       timers and then call the ek_idle function. */
    curtime = ek_clock();

    
    while(timers_used != NULL &&
	  (curtime - lasttime) >= (ek_clock_t)timers_used->t) {
      s = timers_used->s;
      data = timers_used->data;
      id = timers_used->id;
      
      /* Move this timer slot to the free list. */
      tptr = timers_used;
      timers_used = timers_used->next;
      tptr->next = timers_free;
      timers_free = tptr;
      lasttime = curtime;
      
      /* Emit the actual signal. Note that this will not trigger
	 any action until the dispatcher code is executed the next
	 run through the loop. */
      ek_emit(s, data, id);
    }
#endif /* EK_CONF_TIMERS */
    ek_idle();
    
  }
}
/*-----------------------------------------------------------------------------------*/
ek_err_t
ek_emit(ek_signal_t s, ek_data_t data, ek_id_t id)
{
  static unsigned char snum;
  
  if(nsignals == EK_CONF_NUMSIGNALS) {
    return EK_ERR_FULL;
  }
  
  if(s != EK_SIGNAL_NONE) {
    snum = (fsignal + nsignals) & (EK_CONF_NUMSIGNALS - 1);
    signals[snum].s = s;
    signals[snum].data = data;
    signals[snum].id = id;
    ++nsignals;
  }
  return EK_ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
ek_err_t
ek_listen(ek_signal_t s, ek_id_t id)
{
  ek_num_listeners_t i, empty;
  
  /* Go through the listeners list to make sure that the registering
     listener is unique. At the same time, we find the first empty
     listener in the list. */
  empty = EK_CONF_NUMLISTENERS;
  for(i = 0; i < EK_CONF_NUMLISTENERS; ++i) {
    if(listeners[i].id == id &&
       listeners[i].s == s) {
      return EK_ERR_NOTUNIQUE;
    }
    if(empty == EK_CONF_NUMLISTENERS &&
       listeners[i].s == EK_SIGNAL_NONE) {
      empty = i;
    }
  }

  /* We return an error if there were no empty listener slots. */
  if(empty == EK_CONF_NUMLISTENERS) {
    return EK_ERR_FULL;
  }

  /* Else we register the listener and return OK. */
  listeners[empty].s = s;
  listeners[empty].id = id;

  return EK_ERR_OK;  
}
/*-----------------------------------------------------------------------------------*/
ek_err_t
ek_unlisten(ek_id_t id)
{
  ek_num_listeners_t i;
  
  /* Go through the listeners list to find the listener to
     unregister. */
  for(i = 0; i < EK_CONF_NUMLISTENERS; ++i) {
    if(listeners[i].id == id) {
      listeners[i].s = EK_SIGNAL_NONE;
    }
  }
  return EK_ERR_NOTFOUND;
}
/*-----------------------------------------------------------------------------------*/
#if EK_CONF_TIMERS
ek_err_t
ek_timer(ek_signal_t s, ek_data_t data, ek_id_t id, ek_ticks_t t)
{
  register struct ek_timer *tptr, *tptr2;

  /* First check if all timer slots are used already. If so, we return
     an error. */
  if(timers_free == NULL) {
    return EK_ERR_FULL;
  }

  /* We take the first unused timer slot. */  
  tptr = timers_free;
  timers_free = timers_free->next;

  tptr->next = NULL;
  tptr->s = s;
  tptr->data = data;
  tptr->id = id;
  
  /* If there are no running timers, we put this one first on the list
     and return. */
  if(timers_used == NULL) {

    tptr->t = t;
    timers_used = tptr;
    return EK_ERR_OK;
  }

  /* Check if this timeout should be placed first in the list. */
  if(timers_used->t > t) {
    tptr->t = t;
    timers_used->t -= t;
    tptr->next = timers_used;
    timers_used = tptr;

  } else {
    /* Calculate where in the list of timers this one should be
       inserted. This is done by walking through the list while
       subtracting the cumulative time in ticks from the delay for the
       current timer. */
    
    for(tptr2 = timers_used; tptr2 != NULL; tptr2 = tptr2->next) {
      t -= tptr2->t;
      if(tptr2->next == NULL ||
	 tptr2->next->t > t) {
	if(tptr2->next != NULL) {
	  tptr2->next->t -= t;
	}
	tptr->t = t;
	tptr->next = tptr2->next;
	tptr2->next = tptr;
	break;
      }
    }    
  }  
  
  return EK_ERR_OK;  
}
#endif /* EK_CONF_TIMERS */
/*-----------------------------------------------------------------------------------*/
