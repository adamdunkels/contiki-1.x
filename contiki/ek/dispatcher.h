/**
 * \file
 * The dispatcher header file.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 * 
 *
 */

/**
 * \addtogroup kernel
 * @{
 */

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
 * 3. The name of the author may not be used to endorse or promote
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
 * This file is part of the Contiki desktop OS.
 *
 * $Id: dispatcher.h,v 1.14 2003/11/27 15:53:33 adamdunkels Exp $
 *
 */
#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include "ek.h"
#include "cc.h"
#include "arg.h"
#include "loader.h"

void dispatcher_init(void);

/**
 * Broadcast process ID.  
 *
 * Signals emitted to the DISPATCHER_BROADCAST process ID are
 * delivered to all processes that are listening for the signal.
 */
#define DISPATCHER_BROADCAST EK_ID_ALL

/**
 * Instantiating macro for the dispatcher_proc struct.
 *
 * This macro is used when initializing a dispatcher_proc structure
 * for a process. It hides the internals of the dispatcher_proc struct
 * and provides an easy way to define process signature.
 *
 * The following example shows how to define a process.
 \code
 static DISPATCHER_SIGHANDLER(proc_sighandler, s, data);
 static DISPATCHER_UIPHANDLER(proc_uiphandler, s);
 static void proc_idle(void);
 static struct dispatcher_proc p =
   {DISPATCHER_PROC("An example process", proc_idle, proc_sighandler,
                    proc_uiphandler)};		  
 \endcode
 *
 * \note Defining a dispatcher_proc struct does not in any way start
 * the process and does not register the process with the
 * Dispatcher. Instead, the process' initialization function must
 * explicitly call the dispatcher_start() function with a pointer to
 * the dispatcher_proc struct containing the process definition.
 *
 * \param name The name of the process.
 *
 * \param idle A pointer to the idle function or NULL if no idle
 * handler should be registered.
 *
 * \param signal A pointer to the process' signal handler. All
 * processes are required to have a signal handler.
 *
 * \param uip A pointer to the uIP TCP/IP event handler, or NULL if
 * uIP event handler should be registered.
 */
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

/**
 * Obtain a pointer to the list of processes.
 */
#define DISPATCHER_PROCS()   dispatcher_procs

/**
 * Obtain a pointer to the currently running process.
 */
#define DISPATCHER_CURRENT() dispatcher_current

ek_signal_t dispatcher_sigalloc(void);

ek_id_t dispatcher_start(struct dispatcher_proc *p);
void dispatcher_exit(struct dispatcher_proc *p);

void dispatcher_listen(ek_signal_t s);

ek_err_t dispatcher_emit(ek_signal_t s, ek_data_t data, ek_id_t id);

void dispatcher_fastemit(ek_signal_t s, ek_data_t data, ek_id_t id);

struct dispatcher_proc *dispatcher_process(ek_id_t id);

struct dispatcher_uipstate {
  ek_id_t id;  
  void *state;
};

/* We must do some C macro trickery to make things work with sdcc,
   which doesn't support passing arguments to functions called as
   function pointers. */
#if CC_FUNCTION_POINTER_ARGS

/**
 * Declaration macro for a signal handler function.
 *
 * This macro is used when declaring a process' signal handler
 * function. The reason why a macro is used for this instead of a
 * regular C declaration is that certain C compilers cannot handle
 * function pointers with arguments and therefore the
 * DISPATCHER_SIGHANDLER() macro definition varies with different C
 * compilers.
 *
 * The following example shows how to use the DISPATCHER_SIGHANDLER()
 * declaration macro as well as the DISPATCHER_SIGHANDLER_ARGS() macro
 * and how to handle the dispatcher_signal_quit signal.
 *
 \code
 static DISPATCHER_SIGHANDLER(example_sighandler, s, data);

 static
 DISPATCHER_SIGHANDLER(example_sighandler, s, data)
 {
   DISPATCHER_SIGHANDLER_ARGS(s, data);

   if(s == dispatcher_signal_quit) {
      dispatcher_exit(&p);
      LOADER_UNLOAD();
   }
 }
 \endcode
 *
 * \param name The name of the signal handler function.
 *
 * \param s The name of the signal number parameter.
 *
 * \param data The name of the signal data parameter.
 *
 */
#define DISPATCHER_SIGHANDLER(name, s, data) \
        void name(ek_signal_t s, ek_data_t data)

/**
 * Declaration macro for signal handler arguments.
 *
 * This macro must be used for declaring the signal handler function's
 * arguments. The reason why this is needed is that some C compilers
 * do not support function pointers with arguments and therefor a
 * workaround has been made using C macros.
 *
 * \note This macro call must come after the declarations of the local
 * variables but before the first program statement in the signal
 * handler function.
 *
 * The following example shows a signal handler function with local
 * variables.
 *
 \code
 static
 DISPATCHER_SIGHANDLER(example_sighandler, s, data)
 {
   char c;
   DISPATCHER_SIGHANDLER_ARGS(s, data);

   if(s == ctk_signal_keypress) {
      c = (char)data;
      process_key(c);
   } else if(s == dispatcher_signal_quit) {
      dispatcher_exit(&p);
      LOADER_UNLOAD();
   }
 }
 \endcode
 *
 * \param s The name of the signal number argument. Must match the one
 * in the DISPATCHER_SIGHANDLER() declaration.
 *
 * \param data The name of the signal data argument. Must match the
 * one in the DISPATCHER_SIGHANDLER() declaration.
 *
 */
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

/**
 * The uIP application function.
 *
 * This is the uIP application function that is called by uIP for any
 * TCP/IP events. The purpose of the function is to dispatch the uIP
 * event to the process that owns the connection on which the event
 * occured.
 */
#define UIP_APPCALL dispatcher_uipcall
#define UIP_APPSTATE_SIZE sizeof(struct dispatcher_uipstate)


#include "uip.h"

struct uip_conn;

void dispatcher_uipcall(void);

void dispatcher_markconn(struct uip_conn *conn,
			 void *appstate);
void dispatcher_uiplisten(u16_t port);

struct uip_conn *dispatcher_connect(u16_t *ripaddr, u16_t port, void *appstate);

struct uip_udp_conn *dispatcher_udp_new(u16_t *ripaddr, u16_t port, void *appstate);

extern ek_id_t dispatcher_current;
extern struct dispatcher_proc *dispatcher_procs;
extern ek_signal_t dispatcher_signal_quit, dispatcher_signal_msg;

void dispatcher_process_signal(void);
void dispatcher_process_idle(void);

void dispatcher_run(void);

/** @} */

#endif /* __DISPATCHER_H__ */
