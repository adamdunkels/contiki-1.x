/**
 * \file
 * Event kernel, event dispatcher and handler of uIP events.
 * \author Adam Dunkels <adam@dunkels.com> 
 *
 * The dispatcher module is the event kernel in Contiki and handles
 * processes, events and uIP events. All process execution is
 * initiated by the dispatcher.
 */
/*
 * Copyright (c) 2002-2003, Adam Dunkels.
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
 * This file is part of the "ek" event kernel.
 *
 * $Id: ek.c,v 1.5 2004/07/04 11:54:10 adamdunkels Exp $
 *
 */

#include "ek.h"

#include <string.h> /* for strncmp() */

/*#include "uip.h"

#include "uip-event.h"*/

/**
 * \internal Pointer to the currently running process structure.
 *
 */
struct ek_proc *ek_procs;
struct ek_proc *ek_proclist[EK_CONF_MAXPROCS];
struct ek_proc *ek_current;

/**
 * \defgroup events System events
 * @{
 *
 * The Contiki system defines a number of default events that can be
 * delivered to processes. 
 */

/**
 * The "quit" event.
 *
 * This event is posted to a process in order to tell it to remove
 * itself from the system. Since each program may have allocated
 * system resources that must be released before the process quits,
 * each program must implement the event handler by itself. A process
 * that receives this event must call LOADER_UNLOAD() to unload itself
 * after doing all necessary clean ups (such as closing open windows,
 * deallocate allocated memory, etc.). The following code shows how
 * this can be implemented:
 * \code
 static struct ctk_window mainwindow;
 static EK_EVENTHANDLER(example_eventhandler, s, data);

 static
 EK_EVENTHANDLER(example_eventhandler, s, data)
 {
   EK_EVENTHANDLER_ARGS(s, data);

   if(s == ek_event_quit) {
      ctk_window_close(&mainwindow);
      ek_exit(&p);
      LOADER_UNLOAD();
   }
 }
 \endcode
 *
 */
ek_event_t ek_event_quit;

/**
 * A generic message event.
 *
 * This event may be used to send messages between processes. The
 * actual interpretation of the message is up to the applications to
 * decide.  
 */
ek_event_t ek_event_msg;
 
/** @} */

static ek_event_t lastevent;

#if CC_FUNCTION_POINTER_ARGS
#else /* CC_FUNCTION_POINTER_ARGS */
ek_event_t ek_eventhandler_s;
ek_data_t ek_eventhandler_data;

void *ek_uipcall_state;

#endif /* CC_FUNCTION_POINTER_ARGS */      


/**
 * \internal Structure used for keeping the queue of active events.
 */
struct event_data {
  ek_event_t s;
  ek_data_t data;
  ek_id_t id;
};

static ek_num_events_t nevents, fevent;
static struct event_data events[EK_CONF_NUMEVENTS];

unsigned char ek_poll_request;


/**
 * \defgroup kernel The Contiki event kernel
 * @{
 *
 * At the heart of the Contiki desktop environment is the event driven
 * Contiki kernel. Using non-preemptive multitasking, the Contiki
 * event kernel makes it possible to run several programs in
 * parallel. It also provides message passing mechanisms to the
 * running programs.
 *
 * The Contiki kernel is a simple event driven dispatcher which
 * handles processes, events and uIP events. All code execution is
 * initiated by the dispatcher, and applications are implemented as C
 * functions that must return within a short time after being
 * called. It therefore is not possible to implement processes with,
 * e.g., long-lasting while() loops such as the infamous while(1);
 * loop.
 *
 */

/**
 * \page dispatcher The dispatcher
 *
 * The dispatcher is the initiator of all program execution in
 * Contiki. After the system has been initialized by the boot up code,
 * the ek_run() function is called. This function never
 * returns, but will sit in a loop in which it does two things.
 * 
 * - Pulls the first event of the event queue and dispatches this to
 *   all listening processes (ek_process_event()).
 *
 * - Executes the "poll" handlers of all processes that have
 *   registered (ek_process_poll()).
 *
 * Only one event is processes at a time, and the poll handlers of
 * all processes are called between two events are handled.
 *
 * 
 * A process is defined by an initialization function, a event
 * handler, a uIP event handler, and an poll handler. The event
 * handler is called when a event has been posted, for which the
 * process is currently listening. The uIP event handler is called
 * when the uIP TCP/IP stack has an event to deliver to the
 * process. Such events can be that new data has arrived on a
 * connection, that previously sent data has been acknowledged or that
 * a connection has been closed. The poll handler is periodically
 * called by the system.
 *
 * A process is started by calling the ek_start()
 * function. This function must be called by the initialization
 * function before any other dispatcher function is called. When the
 * function returns, the new process is running. 
 *
 * The initialization function is declared with the special
 * LOADER_INIT() macro. The initializaition function takes a single
 * argument; a char * pointer.
 *
 * The function ek_exit() is used to tell the dispatcher that
 * a process has exited. This function must be called by the process
 * itself, and must be called the process unloads itself.
 *
 * \note It is not possible to call ek_exit() on behalf of
 * another process - instead, post the event ek_event_quit
 * with the process as a receiver. The other process should then
 * listen for this event, and call ek_exit() when the event
 * is received. 
 *
 *
 * The dispatcher can pass events between different
 * processes. Events are simple messages that consist of a event
 * number and a generic data pointer called the event data. The
 * event data can be used to pass messages between processes. In
 * order for a event to be delivered to a process, the process must
 * be listening for the event number.
 *
 * If a process has registered an poll handler, the dispatcher will
 * call it as often as possible. The poll handler can be used to
 * implement timer based functionality (by checking the ek_clock()
 * function), or other background processing. The poll handler must
 * return to the caller within a short time, or otherwise the system
 * will feel sluggish.
 *
 *
 *
 * The uIP TCP/IP stack will call the dispatcher when a uIP event has
 * occured. The dispatcher will find the right process for which the
 * event is intended and call the process' uIP handler function.
 *
 */

/*-----------------------------------------------------------------------------------*/
/**
 * Allocates a event number.
 *
 * \return The allocated event number or EK_EVENT_NONE if no event
 * number could be allocated.
 */
/*-----------------------------------------------------------------------------------*/
ek_event_t
ek_alloc_event(void)
{
  return lastevent++;
}
/*-----------------------------------------------------------------------------------*/
static void
procs_add(struct ek_proc *p)
{
  /* The process should be placed on the process list according to the
     process' priority. The higher the priority, the earlier on the
     list. */
  
  /* XXX: not implemented, just place first on list... */
  p->next = ek_procs;
  ek_procs = p;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Starts a new process.
 *
 * Is called by a program in order to start a new process for the
 * program. This function should be called quite early in the
 * initialization procedure of a new process. In partcular, it must be
 * called before any other dispatcher functions, or functions of other
 * modules that make use of dispatcher functions. Most CTK functions
 * call dispatcher functions, and should therefore not be called
 * before ek_start() is called.
 *
 * Example:
 \code
static void app_poll(void);
static EK_EVENTHANDLER(app_eventhandler, s, data);
static struct ek_proc p =
  {EK_PROC("Generic applications", app_poll, app_eventhandler, NULL)};
static ek_id_t id = EK_ID_NONE;
 
LOADER_INIT_FUNC(app_init, arg)
{
  arg_free(arg);
  
  if(id == EK_ID_NONE) {
    id = ek_start(&p);

    rest_of_initialization();
  }
}
 \endcode
 *
 * \param p A pointer to a ek_proc struct that must be found
 * in the process own memory space.
 *
 * \return The process identifier for the new process or EK_ID_NONE
 * if the process could not be started.
 */
/*-----------------------------------------------------------------------------------*/
ek_id_t
ek_start(CC_REGISTER_ARG struct ek_proc *p)
{
  ek_id_t id;

  for(id = 1; id < EK_CONF_MAXPROCS; ++id) {
    if(ek_proclist[id] == NULL) {
      break;
    }
  }
  if(id == EK_CONF_MAXPROCS) {
    return EK_ID_NONE;
  }

  ek_proclist[id] = p;
  
  /* Put on the procs list.*/
  procs_add(p);
  
  p->id = id;

  /* Post an asynchronous event to the process. */
  ek_post(id, EK_EVENT_INIT, p);
  
  /* Make sure we know which processes we are running at the
     moment. */
  /*  ek_current = p;*/
  
  return id;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Exit the currently running process
 *
 * This function causes the currently running process to exit. The
 * function must be called by the process before it unloads itself, or
 * the system will crash.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
ek_exit(void)
{
  struct ek_proc *q, *p;
  
  p = ek_current;

  /* Post a synchronous event to all processes to inform them that
     this process is about to exit. This will allow services to
     dealloc state associated with this process. */
  for(q = ek_procs; q != NULL; q = q->next) {
    if(p != q) {
      ek_current = q;
      if(q->eventhandler != NULL) {
	q->eventhandler(EK_EVENT_EXITED, (ek_data_t)p->id);
      }
    }    
  }
  
  /* Remove process from the process lists. */
  ek_proclist[p->id] = NULL;
  
  if(p == ek_procs) {
    ek_procs = ek_procs->next;    
  } else {
    for(q = ek_procs; q != NULL; q = q->next) {
      if(q->next == p) {
	q->next = p->next;
	break;
      }
    }
  }


  
  ek_current = NULL;
}
/**
 * \addtogroup kernel
 * @{
 */

/*-----------------------------------------------------------------------------------*/
/**
 * Finds the process structure for a specific process ID.
 *
 * \param id The process ID for the process.
 *
 * \return The process structure for the process, or NULL if there
 * process ID was not found.
 */
/*-----------------------------------------------------------------------------------*/
struct ek_proc *
ek_process(ek_id_t id)
{
  struct ek_proc *p;
  for(p = ek_procs; p != NULL; p = p->next) {
    if(p->id == id) {
      return p;
    }
  }
  return NULL;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Initializes the dispatcher module.
 *
 * Must be called during the initialization of Contiki.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
ek_init(void)
{
  int i;
  
  lastevent = EK_EVENT_MAX;

  nevents = fevent = 0;

  arg_init();

  for(i = 0; i < EK_CONF_MAXPROCS; ++i) {
    ek_proclist[i] = NULL;
  }
}
/*-----------------------------------------------------------------------------------*/
/**
 * Process the next event in the event queue and deliver it to
 * listening processes.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
ek_process_event(void)
{ 
  static ek_event_t s;
  static ek_data_t data;
  static ek_id_t id;
  static struct ek_proc *p;
  
  /* If there are any events in the queue, take the first one and
     walk through the list of processes to see if the event should be
     delivered to any of them. If so, we call the event handler
     function for the process. We only process one event at a time
     and call the poll handlers inbetween. */

  if(nevents > 0) {
    
    /* There are events that we should deliver. */
    s = events[fevent].s;
    
    data = events[fevent].data;
    id = events[fevent].id;

    /* Since we have seen the new event, we move pointer upwards
       and decrese number. */
    fevent = (fevent + 1) % EK_CONF_NUMEVENTS;
    --nevents;

    /* If this is a broadcast event, we deliver it to all events, in
       order of their priority. */
    if(id == EK_BROADCAST) {
      for(p = ek_procs; p != NULL; p = p->next) {

	if(ek_poll_request) {
	  ek_poll_request = 0;
	  ek_process_poll();
	}
	
	ek_current = p;
	if(p->eventhandler != NULL) {
	  p->eventhandler(s, data);
	}
      }
    } else {
      if(ek_poll_request) {
	ek_poll_request = 0;
	ek_process_poll();
      }

      p = ek_proclist[id];
      if(p != NULL &&
	 p->eventhandler != NULL) {
	ek_current = p;
	p->eventhandler(s, data);	
      }
    }    
  }

}
/*-----------------------------------------------------------------------------------*/
/**
 * Call each process' poll handler.
 */
/*-----------------------------------------------------------------------------------*/
void
ek_process_poll(void)
{
  struct ek_proc *p;
  
  /* Call poll handlers. */
  for(p = ek_procs; p != NULL; p = p->next) {
    if(p->pollhandler != NULL) {
      ek_current = p;
      p->pollhandler();
    }
  }
  
}
/*-----------------------------------------------------------------------------------*/
/**
 * Run the system once - call poll handlers and process one event.
 *
 * This function should be called repeatedly from the main() program
 * to actuall run the Contiki system. It calls the necessary poll
 * handlers, and processes one event. The function returns the number
 * of events that are waiting in the event queue so that the caller
 * may choose to put the CPU to sleep when there are no pending
 * events.
 *
 * \return The number of events that are currently waiting in the
 * event queue.
 */
/*-----------------------------------------------------------------------------------*/
int
ek_run(void)
{
  /* Process "poll" events. */
  ek_process_poll();
  
  /* Process one event */
  ek_process_event();

  return nevents;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Post an asynchronous event.
 *
 * This function posts an asynchronous event to one or more
 * processes. The handing of the event is deferred until the target
 * process is scheduled by the kernel. An event can be broadcast to
 * all processes, in which case all processes in the system will be
 * scheduled to handle the event.
 * 
 * \param s The event to be posted.
 *
 * \param data The auxillary data to be sent with the event
 *
 * \param id The process ID to which the event should be posted, or
 * EK_BROADCAST if the event should be posted to all
 * processes.
 *
 * \retval EK_ERR_OK The event could be posted.
 *
 * \retval EK_ERR_FULL The event queue was full and the event could
 * not be posted.
 */
/*-----------------------------------------------------------------------------------*/
ek_err_t
ek_post(ek_id_t id, ek_event_t s, ek_data_t data)
{
  static unsigned char snum;
  
  if(nevents == EK_CONF_NUMEVENTS) {
    return EK_ERR_FULL;
  }
  
  snum = (fevent + nevents) % EK_CONF_NUMEVENTS;
  events[snum].s = s;
  events[snum].data = data;
  events[snum].id = id;
  ++nevents;
  
  return EK_ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
void
ek_post_synch(ek_id_t id, ek_event_t ev, ek_data_t data)
{
  struct ek_proc *p = ek_current; 
  ek_current = ek_proclist[id]; 
  ek_current->eventhandler(ev, data); 
  ek_current = p;
}
/*-----------------------------------------------------------------------------------*/
ek_id_t
ek_find(const char *prefix)
{
  struct ek_proc *p;
  unsigned short len;

  /* Search through all processes and search for the specified process
     name. */
  len = strlen(prefix);
  for(p = ek_procs; p != NULL; p = p->next) {
    if(strncmp(prefix, p->name, len) == 0) {
      return p->id;
    }
  }

  return EK_ID_NONE;
}
/*-----------------------------------------------------------------------------------*/
void
ek_replace(struct ek_proc *newp, void *arg)
{
  struct ek_proc *p = ek_current;
  
  /* Remove the currently executing process. */
  ek_exit();

  ek_proclist[p->id] = newp;
  
  /* Put on the procs list.*/
  procs_add(newp);
  
  newp->id = p->id;

  /* Post an asynchronous event to the process. */
  ek_post(p->id, EK_EVENT_REPLACE, arg);  
}
/*-----------------------------------------------------------------------------------*/
void *
ek_procstate(ek_id_t id)
{
  struct ek_proc *p;

  p = ek_proclist[id];
  if(p == NULL) {
    return NULL;
  }
  return p->procstate;
}
/*-----------------------------------------------------------------------------------*/
/** @} */
