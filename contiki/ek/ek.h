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
 * $Id: ek.h,v 1.7 2004/09/12 20:24:55 adamdunkels Exp $
 */
#ifndef __EK_H__
#define __EK_H__

#include "ek-conf.h"

#include "cc.h"
#include "arg.h"
#include "loader.h"

void ek_init(void);

typedef unsigned char ek_err_t;


/* Errors: */
#define EK_ERR_OK        0
#define EK_ERR_FULL      1
#define EK_ERR_NOTUNIQUE 2
#define EK_ERR_NOTFOUND  3

/* Special IDs defined by ek: */

/**
 * Broadcast process ID.  
 *
 * Events emitted to the EK_BROADCAST process ID are
 * delivered to all processes.
 */
#define EK_ID_NONE               EK_CONF_MAXPROCS
#define EK_ID_ALL                EK_ID_NONE

#define EK_BROADCAST             EK_ID_ALL

#define EK_EVENT_NONE            0x80

#define EK_EVENT_INIT            0x81
#define EK_EVENT_REPLACE         0x82
#define EK_EVENT_CONTINUE        0x83
#define EK_EVENT_MSG             0x84
#define EK_EVENT_EXITED          0x85

#define EK_EVENT_REQUEST_EXIT    0x86
#define EK_EVENT_REQUEST_REPLACE 0x87


#define EK_EVENT_MAX             0x88
/**
 * Instantiating macro for the ek_proc struct.
 *
 * This macro is used when initializing a ek_proc structure
 * for a process. It hides the internals of the ek_proc struct
 * and provides an easy way to define process signature.
 *
 * The following example shows how to define a process.
 \code
 static EK_EVENTHANDLER(proc_eventhandler, s, data);
 static EK_UIPHANDLER(proc_uiphandler, s);
 static void proc_poll(void);
 static struct ek_proc p =
   {EK_PROC("An example process", proc_poll, proc_eventhandler,
                    proc_uiphandler)};		  
 \endcode
 *
 * \note Defining a ek_proc struct does not in any way start
 * the process and does not register the process with the
 * Dispatcher. Instead, the process' initialization function must
 * explicitly call the ek_start() function with a pointer to
 * the ek_proc struct containing the process definition.
 *
 * \param name The name of the process.
 *
 * \param poll A pointer to the poll function or NULL if no poll
 * handler should be registered.
 *
 * \param event A pointer to the process' event handler. All
 * processes are required to have a event handler.
 *
 * \param uip A pointer to the uIP TCP/IP event handler, or NULL if
 * uIP event handler should be registered.
 */
#define EK_PROCESS(name, strname, prio, eventh, pollh, stateptr)	\
  static struct ek_proc name = {NULL, EK_ID_NONE, strname, prio, eventh, pollh, stateptr}

struct ek_proc {
  struct ek_proc *next;
  ek_id_t id;
  const char *name;
  unsigned char prio;
  void (* eventhandler)(ek_event_t ev, ek_data_t data);
  void (* pollhandler)(void);
  void *procstate;
};

#define EK_PRIO_LOWEST  0x00
#define EK_PRIO_LOW     0x3f
#define EK_PRIO_NORMAL  0x7f
#define EK_PRIO_HIGH    0xbf
#define EK_PRIO_HIGHEST 0xff

void *ek_procstate(ek_id_t id);

/**
 * Obtain a pointer to the list of processes.
 */
#define EK_PROCS()   ek_procs

/**
 * Obtain a pointer to the currently running process.
 */
#define EK_CURRENT() ek_current

ek_event_t ek_alloc_event(void);

ek_id_t ek_start(struct ek_proc *p);
void ek_exit(void);

ek_err_t ek_post(ek_id_t id, ek_event_t s, ek_data_t data);

struct ek_proc *ek_process(ek_id_t id);

/* We must do some C macro trickery to make things work with sdcc,
   which doesn't support passing arguments to functions called as
   function pointers. */
#if CC_FUNCTION_POINTER_ARGS

/**
 * Declaration macro for a event handler function.
 *
 * This macro is used when declaring a process' event handler
 * function. The reason why a macro is used for this instead of a
 * regular C declaration is that certain C compilers cannot handle
 * function pointers with arguments and therefore the
 * EK_EVENTHANDLER() macro definition varies with different C
 * compilers.
 *
 * The following example shows how to use the EK_EVENTHANDLER()
 * declaration macro as well as the EK_EVENTHANDLER_ARGS() macro
 * and how to handle the ek_event_quit event.
 *
 \code
 static EK_EVENTHANDLER(example_eventhandler, s, data);

 static
 EK_EVENTHANDLER(example_eventhandler, s, data)
 {
   EK_EVENTHANDLER_ARGS(s, data);

   if(s == ek_event_quit) {
      ek_exit(&p);
      LOADER_UNLOAD();
   }
 }
 \endcode
 *
 * \param name The name of the event handler function.
 *
 * \param s The name of the event number parameter.
 *
 * \param data The name of the event data parameter.
 *
 */
#define EK_EVENTHANDLER(name, ev, data) \
        static void name(ek_event_t ev, ek_data_t data)

#define EK_POLLHANDLER(name) \
        static void name(void)

#define EK_PROCESS_INIT(name, arg) \
        void name(void *arg)

#define EK_PROC_STATE(p) ((p)->procstate)
#define EK_PROC_ID(p)    ((p)->id)

/**
 * Declaration macro for event handler arguments.
 *
 * This macro must be used for declaring the event handler function's
 * arguments. The reason why this is needed is that some C compilers
 * do not support function pointers with arguments and therefor a
 * workaround has been made using C macros.
 *
 * \note This macro call must come after the declarations of the local
 * variables but before the first program statement in the event
 * handler function.
 *
 * The following example shows a event handler function with local
 * variables.
 *
 \code
 static
 EK_EVENTHANDLER(example_eventhandler, s, data)
 {
   char c;
   EK_EVENTHANDLER_ARGS(s, data);

   if(s == ctk_event_keypress) {
      c = (char)data;
      process_key(c);
   } else if(s == ek_event_quit) {
      ek_exit(&p);
      LOADER_UNLOAD();
   }
 }
 \endcode
 *
 * \param s The name of the event number argument. Must match the one
 * in the EK_EVENTHANDLER() declaration.
 *
 * \param data The name of the event data argument. Must match the
 * one in the EK_EVENTHANDLER() declaration.
 *
 */
#define EK_EVENTHANDLER_ARGS(ev, data)

/*
#define EK_UIPCALL(name, state) \
        void name(void *state)

#define EK_UIPCALL_ARG(state)
*/

#else /* CC_FUNCTION_POINTER_ARGS */
#define EK_EVENTHANDLER(s, data) \
        void name(void)

#define EK_EVENTHANDLER_ARGS(s, data) ek_event_t s = ek_eventhandler_s; \
                                      ek_data_t data = ek_eventhandler_data

extern ek_event_t ek_eventhandler_s;
extern ek_data_t ek_eventhandler_data;

#endif /* CC_FUNCTION_POINTER_ARGS */

extern struct ek_proc *ek_current;
extern struct ek_proc *ek_procs;
extern struct ek_proc *ek_proclist[EK_CONF_MAXPROCS];

void ek_process_event(void);
void ek_process_poll(void);

int ek_run(void);

ek_id_t ek_find(const char *servicename);

void ek_replace(struct ek_proc *newp, void *arg);

/** @} */


/*-----------------------------------------------------------------------------------*/
/**
 * Post a synchronous event.
 *
 * This function emits a event and calls the listening processes'
 * event handlers immediately, before returning to the caller. This
 * function requires more call stack space than the ek_emit()
 * function and should be used with care, and only in situtations
 * where the exact implications are known.
 *
 * In most situations, the ek_emit() function should be used
 * instead.
 *
 * \param s The event to be emitted.
 *
 * \param data The auxillary data to be sent with the event
 *
 * \param id The process ID to which the event should be emitted, or
 * EK_BROADCAST if the event should be emitted to all
 * processes listening for the event.
 *
 */
/*-----------------------------------------------------------------------------------*/
void ek_post_synch(ek_id_t id, ek_event_t ev, ek_data_t data);

extern volatile unsigned char ek_poll_request;
#define EK_REQUEST_POLL() ek_poll_request = 1

#endif /* __EK_H__ */
