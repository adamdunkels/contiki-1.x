/**
 * \file
 * Multi-threading library convenience functions.
 * \author Adam Dunkels <adam@sics.se>
 */
#ifndef __MTP_H__
#define __MTP_H__

/**
 * \addtogroup mt
 * @{
 */

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

#include "dispatcher.h"
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
#define MTP(thread, name, name_p, name_t, name_idle) \
static struct mt_thread name_t; \
static void name_idle(void) { mt_exec(&name_t); } \
static struct dispatcher_proc name_p = \
 {DISPATCHER_PROC(name, name_idle, mtp_sighandler, NULL)}; \
static struct mtp_thread thread = {&name_t, &name_p}

struct mtp_thread {
  struct mt_thread *t;
  struct dispatcher_proc *p;
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

/** @} */
/** @} */

DISPATCHER_SIGHANDLER(mtp_sighandler, signal, data);

#endif /* __MTP_H__ */
