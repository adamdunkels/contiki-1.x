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
 * $Id: dispatcher.h,v 1.9 2003/08/24 22:41:31 adamdunkels Exp $
 *
 */
#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include "ek.h"
#include "cc.h"
#include "arg.h"

void dispatcher_init(void);

#define DISPATCHER_BROADCAST EK_ID_ALL

#define DISPATCHER_PROC(name, idle, signal, uip) \
 NULL, 0, name, idle, signal, uip
struct dispatcher_proc {
  struct dispatcher_proc *next;
  ek_id_t id;
  char *name;
  void (* idle)(void);
#if CC_FUNCTION_POINTER_ARGS  
  void (* signalhandler)(ek_signal_t s, ek_data_t data);
  void (* uiphandler)(void *state);
#else /* CC_FUNCTION_POINTER_ARGS */
  void (* signalhandler)(void);
  void (* uiphandler)(void);
#endif /* CC_FUNCTION_POINTER_ARGS */

  unsigned char signals[EK_CONF_NUMSIGNALS];
};

#define DISPATCHER_PROCS()   dispatcher_procs
#define DISPATCHER_CURRENT() dispatcher_current

ek_signal_t dispatcher_sigalloc(void);

ek_id_t dispatcher_start(struct dispatcher_proc *p);
void dispatcher_exit(struct dispatcher_proc *p);

void dispatcher_listen(ek_signal_t s);

void dispatcher_timer(ek_signal_t s, ek_data_t data, ek_ticks_t t);
/*#define dispatcher_emit ek_emit*/
ek_err_t dispatcher_emit(ek_signal_t s, ek_data_t data, ek_id_t id);

struct dispatcher_proc *dispatcher_process(ek_id_t id);

struct dispatcher_uipstate {
  ek_id_t id;  
  void *state;
};

/* We must do some C macro trickery to make things work with sdcc,
   which doesn't support passing arguments to functions called as
   function pointers. */
#if CC_FUNCTION_POINTER_ARGS
#define DISPATCHER_SIGHANDLER(name, s, data) \
        void name(ek_signal_t s, ek_data_t data)

#define DISPATCHER_SIGHANDLER_ARGS(s, data)

#define DISPATCHER_UIPCALL(name, state) \
        void name(void *state)

#define DISPATCHER_UIPCALL_ARG(state)

#else /* CC_FUNCTION_POINTER_ARGS */
#define DISPATCHER_SIGHANDLER(name, s, data) \
        void name(void)

#define DISPATCHER_SIGHANDLER_ARGS(s, data) ek_signal_t s = dispatcher_sighandler_s; \
                                            ek_data_t data = dispatcher_sighandler_data

extern ek_signal_t dispatcher_sighandler_s;
extern ek_data_t dispatcher_sighandler_data;

#define DISPATCHER_UIPCALL(name, state) \
        void name(void)
#define DISPATCHER_UIPCALL_ARG(state) \
        void *state = dispatcher_uipcall_state

extern void *dispatcher_uipcall_state;

#endif /* CC_FUNCTION_POINTER_ARGS */

#define UIP_APPCALL dispatcher_uipcall
#define UIP_APPSTATE_SIZE sizeof(struct dispatcher_uipstate)


#include "uip.h"

struct uip_conn;

void dispatcher_uipcall(void);

void dispatcher_markconn(struct uip_conn *conn,
			 void *appstate);
void dispatcher_uiplisten(u16_t port);

extern ek_id_t dispatcher_current;
extern struct dispatcher_proc *dispatcher_procs;
extern ek_signal_t dispatcher_signal_quit;

void dispatcher_signal(void);
void dispatcher_idle(void);

void dispatcher_run(void);

#endif /* __DISPATCHER_H__ */
