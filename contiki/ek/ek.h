/**
 * \addtogroup ek
 * @{
 */

/**
 * \file
 * Contiki Kernel header file.
 * \author
 * Adam Dunkels <adam@sics.se>
 */

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
 * $Id: ek.h,v 1.8 2005/02/22 22:46:33 adamdunkels Exp $
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

/**
 * \defgroup events System events
 * @{
 *
 * The Contiki kernel defines a number of default events that can be
 * delivered to processes. 
 */

#define EK_EVENT_NONE            0x80

/**
 * Initialization event.
 *
 * This event is posted by the kernel to a process in order to let the
 * process to initialization.
 *
 */
#define EK_EVENT_INIT            0x81

/**
 * Service replacement event.
 *
 * This event is posted by the kernel in order to inform a process
 * that a service that the current process have requested to be
 * replaced, is now replaced.
 */
#define EK_EVENT_REPLACE         0x82

/**
 * Continuation event.
 *
 * This event can be used to implement continuations.
 */
#define EK_EVENT_CONTINUE        0x83

/**
 * Generic message event.
 *
 * This event can be used to pass messages between processes.
 */
#define EK_EVENT_MSG             0x84

/**
 * A process has exited.
 *
 * This event is posted to all processes to inform that another
 * process has exited.
 */
#define EK_EVENT_EXITED          0x85

/**
 * Request a process to exit itself.
 *
 * This event is posted to a process in order to tell it to remove
 * itself from the system. Since each program may have allocated
 * system resources that must be released before the process quits,
 * each program must implement the event handler by itself. A process
 * that receives this event must call LOADER_UNLOAD() to unload itself
 * after doing all necessary clean ups (such as closing open windows,
 * deallocate allocated memory, etc.).
 */
#define EK_EVENT_REQUEST_EXIT    0x86

/**
 * Request a service to be replaced.
 *
 * This event is posted by the kernel to a service that another
 * process have requested to be replaced.
 */
#define EK_EVENT_REQUEST_REPLACE 0x87


#define EK_EVENT_MAX             0x88

/** @}*/

/**
 * Instantiating macro for the ek_proc struct.
 *
 * This macro is used when initializing a ek_proc structure
 * for a process. It hides the internals of the ek_proc struct
 * and provides an easy way to define process signature.
 *
 * \note Defining a ek_proc struct does not start the process and does
 * not register the process with the kernel. Rather, the process'
 * initialization function must explicitly call the ek_start()
 * function with a pointer to the ek_proc struct containing the
 * process definition.
 *
 * \param name The name of the \c struct \c ek_proc of the process.
 *
 * \param strname A textual repressentation of the process' name.
 *
 * \param prio The priority of the process.
 *
 * \param pollh A pointer to the poll function or NULL if no poll
 * handler should be registered.
 *
 * \param eventh A pointer to the process' event handler. All
 * processes are required to have a event handler.
 *
 * \param stateptr An opaque pointer that can be associated with the
 * process.
 *
 * \hideinitializer
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

/**
 * Lowest priority.
 *
 * This value can be used in the \c prio field in the EK_PROCESS() macro.
 */
#define EK_PRIO_LOWEST  0x00
/**
 * Low priority.
 *
 * This value can be used in the \c prio field in the EK_PROCESS() macro.
 */
#define EK_PRIO_LOW     0x3f
/**
 * Normal priority.
 *
 * This value can be used in the \c prio field in the EK_PROCESS() macro.
 */
#define EK_PRIO_NORMAL  0x7f
/**
 * High priority.
 *
 * This value can be used in the \c prio field in the EK_PROCESS() macro.
 */
#define EK_PRIO_HIGH    0xbf
/**
 * Highest priority.
 *
 * This value can be used in the \c prio field in the EK_PROCESS() macro.
 */
#define EK_PRIO_HIGHEST 0xff

/**
 * Get the process state of a process.
 *
 * This function is used by the kernel service module, and is in most
 * cased not used directly by user programs.
 *
 * \param id The process ID of the process,
 */
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
 * \param ev The name of the event number parameter.
 *
 * \param data The name of the event data parameter.
 *
 *
 * \hideinitializer 
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
 * \param ev The name of the event number argument. Must match the one
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
 * \param ev The event to be emitted.
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
