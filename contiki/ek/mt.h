/**
 * \file
 * Header file for the preemptive multitasking library for Contiki.
 * \author
 * Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __MT_H__
#define __MT_H__

#include "ek.h"

/**
 * \defgroup mt Contiki preemptive multi-threading library
 * @{
 *
 * The event driven Contiki kernel does not provide multi-threading
 * by itself - instead, preemptive multi-threading is implemented
 * as a library that optionally can be linked with applications. This
 * library constists of two parts: a platform independent part, which is
 * the same for all platforms on which Contiki runs, and a platform
 * specific part, which must be implemented specifically for the
 * platform that the multi-threading library should run.
 */


/**
 * \defgroup mtarch Architecture support for multi-threading
 * @{
 *
 * The Contiki multi-threading library requires some architecture
 * specific support for seting up and switching stacks. This support
 * requires three stack manipulation functions to be implemented:
 * mtarch_start(), which sets up the stack frame for a new thread,
 * mtarch_exec(), which switches in the stack of a thread, and
 * mtarch_yield(), which restores the kernel stack from a thread's
 * stack. Additionally, two functions for controlling the preemption
 * (if any) must be implemented: mtarch_preemption_start() and
 * mtarch_preemption_stop(). If no preemption is used, these functions
 * can be implemented as empty functions. Finally, the function
 * mtarch_init() is called by mt_init(), and can be used for
 * initalization of timer interrupts, or any other mechanisms required
 * for correct operation of the architecture specific support funcions.
 *
 */

/**
 * An opaque structure that is used for holding the state of a thread.
 *
 * The structure should be defined in the "mtarch.h" file. This
 * structure typically holds the entire stack for the thread.
 */
struct mtarch_thread;

/**
 * Initialize the architecture specific support functions for the
 * multi-thread library.
 *
 * This function is implemented by the architecture specific functions
 * for the multi-thread library and is called by the mt_init()
 * function as part of the initialization of the library. The
 * mtarch_init() function can be used for, e.g., starting preemtion
 * timers or other architecture specific mechanisms required for the
 * operation of the library.
 */
void mtarch_init(void);

/**
 * Uninstall library and clean up.
 *
 */
void mtarch_remove(void);

/**
 * Setup the stack frame for a thread that is being started.
 *
 * This function is called by the mt_start() function in order to set
 * up the architecture specific stack of the thread to be started.
 *
 * \param thread A pointer to a struct mtarch_thread for the thread to
 * be started.
 *
 * \param function A pointer to the function that the thread will
 * start executing the first time it is scheduled to run.
 *
 * \param data A pointer to the argument that the function should be
 * passed.
 */
void mtarch_start(struct mtarch_thread *thread,
		  void (* function)(void *data),
		  void *data);

/**
 * Yield the processor.
 *
 * This function is called by the mt_yield() function, which is called
 * from the running thread in order to give up the processor.
 *
 */
void mtarch_yield(void);

/**
 * Start executing a thread.
 *
 * This function is called from mt_exec() and the purpose of the
 * function is to start execution of the thread. The function should
 * switch in the stack of the thread, and does not return until the
 * thread has explicitly yielded (using mt_yield()) or until it is
 * preempted.
 *
 */
void mtarch_exec(struct mtarch_thread *thread);


void mtarch_pstart(void);
void mtarch_pstop(void);

/** @} */


#include "mtarch.h"
#include "ek.h"

struct mt_thread {
  int state;
  ek_event_t *evptr;
  ek_data_t *dataptr;
  struct mtarch_thread thread;
};

/**
 * No error.
 *
 * \hideinitializer
 */
#define MT_OK 1

/**
 * Initializes the multithreading library.
 *
 */
void mt_init(void);

/**
 * Uninstalls library and cleans up.
 *
 */
void mt_remove(void);


/**
 * Starts a multithreading thread.
 *
 * \param thread Pointer to an mt_thread struct that must have been
 * previously allocated by the caller.
 *
 * \param function A pointer to the entry function of the thread that is
 * to be set up.
 *
 * \param data A pointer that will be passed to the entry function.
 *
 */
void mt_start(struct mt_thread *thread, void (* function)(void *), void *data);

/**
 * Start executing a thread.
 *
 * This function is called by a Contiki process and starts running a
 * thread. The function does not return until the thread has yielded,
 * or is preempted.
 *
 * \note The thread must first be initialized with the mt_init() function.
 *
 * \param thread A pointer to a struct mt_thread block that must be
 * allocated by the caller.
 *
 */
void mt_exec(struct mt_thread *thread);

/**
 * Post an event to a thread.
 *
 * This function posts an event to a thread. The thread will be
 * scheduled if the thread currently is waiting for the posted event
 * number. If the thread is not waiting for the event, this function
 * does nothing.
 *
 * \note The thread must first be initialized with the mt_init() function.
 *
 * \param thread A pointer to a struct mt_thread block that must be
 * allocated by the caller.
 *
 * \param s The event that is posted to the thread.
 */
void mt_exec_event(struct mt_thread *thread, ek_event_t s, ek_data_t data);

/**
 * Voluntarily give up the processor.
 *
 * This function is called by a running thread in order to give up
 * control of the CPU.
 *
 */
void mt_yield(void);

/**
 * Emit a signal to another process.
 *
 * This function is called by a running thread and will emit a signal
 * to another Contiki process. This will cause the currently executing
 * thread to yield.
 *
 * \param s The signal to be emitted.
 * \param data A pointer to a message that is to be delivered together with the signal.
 * \param id The process ID of the receiver of the signal, or EK_ID_ALL for a broadcast signal.
 */
void mt_post(ek_id_t id, ek_event_t s, ek_data_t data);

/**
 * Block and wait for an event to occur.
 *
 * This function can be called by a running thread in order to block
 * and wait for an event. The function returns when an event has
 * occured. The event number and the associated data are placed in the
 * variables pointed to by the function arguments.
 * 
 */
void mt_wait(ek_event_t *s, ek_data_t *data);

/**
 * Exit a thread.
 *
 * This function is called from within an executing thread in order to
 * exit the thread. The function never returns.
 *
 */
void mt_exit(void);

/**
 * \defgroup mtp Multi-threading library convenience functions
 * @{
 *
 * The Contiki multi-threading library has an interface that might be
 * hard to use. Therefore, the mtp module provides a simpler
 * interface.
 *
 * Example:
\code
static void
example_thread_code(void *data)
{
  while(1) {
    printf("Test\n");
    mt_yield();
  }
} 
MTP(example_thread, "Example thread", p1, t1, t1_idle);

int
main(int argc, char *argv[])
{
  mtp_start(&example_thread, example_thread_code, NULL);
}
\endcode
*
*/

#include "mt.h"

/**
 * Declare a thread.
 *
 * This macro is used to covneniently declare a thread, and the
 * process in which the thread should execute. The names of the
 * variables provided to the macro should be chosen to be unique
 * within the file that the thread is used.
 *
 * Example:
\code
MTP(example_thread, "Example thread", p1, t1, t1_idle);
\endcode
 *
 * \param thread The name of the thread. 
 *
 * \param name A string that specifies the user-visible name of the
 * process in which the thread will run.
 *
 * \param name_p The name of the variable holding the process' state.
 *
 * \param name_t The name of the variable holding the threads' state.
 *
 * \param name_idle The name of the function that is to execute the
 * threads' code.
 *
 * \hideinitializer
 */
#define MTP(thread, proc, name) \
extern struct mtp_thread thread; \
EK_PROCESS(proc, name, EK_PRIO_NORMAL, mtp_eventhandler, \
           NULL, (void *)&thread); \
static struct mtp_thread thread = {&proc}

struct mtp_thread {
  struct ek_proc *p;
  struct mt_thread t;
};

/**
 * Start a thread.
 *
 * This function starts the process in which the thread is to run, and
 * also sets up the thread to run within the process. The function
 * should be passed variable names declared with the MTP() macro.
 *
 * Example:
\code
mtp_start(&t, example_thread_code, NULL);
\endcode
 * \param t A pointer to a thread structure previously declared with MTP().
 *
 * \param function A pointer to the function that the thread should
 * start executing.
 *
 * \param data A pointer that the function should be passed when first
 * invocated.
 */
void mtp_start(struct mtp_thread *t,
	       void (* function)(void *), void *data);

void mtp_exit(void);

/** @} */
/** @} */

void mtp_eventhandler(ek_event_t ev, ek_data_t data);


/** @} */
#endif /* __MT_H__ */

