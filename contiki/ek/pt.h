/**
 * \defgroup pt Protothreads 
 * @{ 
 *
 * Protothreads are lightweight stackless threads that is used to
 * provide blocking contexts in event-driven systems. This is useful
 * for implementing sequential control flow, without requiring
 * ordinary threads and multiple stacks. Protothreads provides
 * conditional blocking inside a C function.
 *
 * The advantage of protothreads over ordinary threads is that a
 * protothread do not require a separate stack. In memory constrained
 * systems, the overhead of allocating multiple stacks can consume
 * large amounts of the available memory. In contrast, each
 * protothread only requires between two and twelve bytes of state,
 * depending on the architecture.
 *
 * Because protothreads are stackless, a protothread can only run
 * within a single C function. A protothread may call normal C
 * functions, but cannot block inside a called function. Blocking
 * inside nested function calls are made by spawning a separate
 * protothread for each potentially blocking function.
 *
 * A protothread is driven by repeated calls to the function in which
 * the protothread is running. Each time the function is called, the
 * protothread will run until it blocks or exits.
 *
 * Protothreads are implemented using <i>local continuations</i>. A
 * local continuation represents the current state of execution at a
 * particular place in the program, but does not provide any call
 * history or local variables.
 *
 * The protothreads API consists of four basic operations:
 * initialization: PT_INIT(), execution: PT_BEGIN(), conditional
 * blocking: PT_WAIT_UNTIL() and exit: PT_END(). On top of these, two
 * convenience functions are built: reversed condition blocking:
 * PT_WAIT_WHILE() and protothread blocking: PT_WAIT_THREAD().
 *
 */

/**
 * \file
 * Protothreads implementation.
 * \author
 * Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __PT_H__
#define __PT_H__

#include "lc.h"

struct pt {
  lc_t lc;
};

#define PT_THREAD_WAITING 0
#define PT_THREAD_EXITED  1

/**
 * Declaration of a protothread.
 *
 * This macro is used to declare a protothread.
 *
 * Example:
 \code
 PT_THREAD(consumer(struct pt *p, int event)) {
   PT_BEGIN(p);
   PT_WAIT_UNTIL(event == AVAILABLE);
   consume();
   PT_WAIT_UNTIL(event == CONSUMED);
   acknowledge_consumed();
   PT_END(p);
 }
 \endcode
 *
 * \hideinitializer
 */
#define PT_THREAD(name_args) char name_args

/**
 * Initialize a protothread.
 *
 * Initializes a protothread. Initialization must be done prior to
 * starting to execute the protothread.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * Example:
 *
 \code
 int main(void) {
   struct pt p;
   int event;
   
   PT_INIT(&p);
   while(PT_RUNNING(consumer(&p, event))) {
     event = get_event();
   }
 }
 \endcode
 *
 * \hideinitializer
 */
#define PT_INIT(pt)				\
  LC_INIT((pt)->lc)

/**
 * Start a protothread.
 *
 * This macro is used to set the starting point of a protothread. It
 * should be placed at the start of the function in which the
 * protothread runs. All C statements above the PT_BEGIN() invokation
 * will be executed each time the protothread is scheduled.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * Example:
 *
 \code
 PT_THREAD(producer(struct pt *p, int event)) {
   int empty;
   empty = (event == CONSUMED || event == DROPPED);
 
   PT_BEGIN(p);

   PT_WAIT_UNTIL(empty);
   produce();
   PT_WAIT_UNTIL(event == PRODUCED);   
   
   PT_EXIT(p);
 }
 \endcode
 *
 * \hideinitializer
 */
#define PT_BEGIN(pt) LC_RESUME((pt)->lc)
/*\
  do {						\
    if((pt)->lc != LC_NULL) {			\
      LC_RESUME((pt)->lc);			\
    } 						\
    } while(0)*/

/**
 * Block and wait until condition is true.
 *
 * This macro blocks the protothread until the specified condition is
 * true.
 *
 * \param pt A pointer to the protothread control structure.
 * \param condition The condition.
 *
 * Example:
 \code
 PT_THREAD(seconds(struct pt *p)) {
   PT_BEGIN(p);

   PT_WAIT_UNTIL(p, time >= 2 * SECOND);
   printf("Two seconds have passed\n");
   
   PT_EXIT(p);
 }
 \endcode
 *
 * \hideinitializer
 */
#define PT_WAIT_UNTIL(pt, condition)	        \
  do {						\
    LC_SET((pt)->lc);				\
    if(!(condition)) {				\
      return PT_THREAD_WAITING;			\
    }						\
  } while(0)

/**
 * Block and wait while condition is true.
 *
 * This function blocks and waits while condition is true. See
 * PT_WAIT_UNTIL().
 *
 * \param pt A pointer to the protothread control structure.
 * \param cond The condition.
 *
 * \hideinitializer
 */
#define PT_WAIT_WHILE(pt, cond)			\
  PT_WAIT_UNTIL((pt), !(cond))


/**
 * Block and wait until a child protothread completes.
 *
 * This macro schedules a child protothread. The current protothread
 * will block until the child protothread completes.
 *
 * \note The child protothread must be manually initialized with the
 * PT_INIT() function before this function is used.
 *
 * \param pt A pointer to the protothread control structure.
 * \param thread The child protothread with arguments
 *
 * Example:
 \code
 PT_THREAD(child(struct pt *p, int event)) {
   PT_BEGIN(p);

   PT_WAIT_UNTIL(event == EVENT1);   
   
   PT_END(p);
 }

 PT_THREAD(parent(struct pt *p, struct pt *child_pt, int event)) {
   PT_BEGIN(p);

   PT_INIT(child_pt);
   
   PT_WAIT_THREAD(p, child(child_pt, event));
   
   PT_END(p);
 }
 \endcode
 *
 * \hideinitializer 
 */
#define PT_WAIT_THREAD(pt, thread)		\
  PT_WAIT_UNTIL((pt), (thread))

/**
 * Spawn a child protothread and wait until it exits.
 *
 * This macro spawns a child protothread and waits until it exits. The
 * macro can only be used within a protothread.
 *
 * \param pt A pointer to the protothread control structure.
 * \param thread The child protothread with arguments
 *
 * \hideinitializer
 */
#define PT_SPAWN(pt, thread)			\
  do {						\
    PT_INIT((pt));				\
    PT_WAIT_THREAD((pt), (thread));		\
  } while(0)

/**
 * Restart the protothread.
 *
 * This macro will block and cause the protothread to restart its
 * execution at the place of the PT_BEGIN() call.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \hideinitializer
 */
#define PT_RESTART(pt)				\
  do {						\
    PT_INIT(pt);				\
    return PT_THREAD_WAITING;			\
  } while(0)

/**
 * Exit the protothread.
 *
 * This macro causes the protothread to exit. If the protothread was
 * spawned by another protothread, the parent protothread will become
 * unblocked and can continue to run.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \hideinitializer
 */
#define PT_EXIT(pt)				\
  do {						\
    PT_INIT(pt);				\
    return PT_THREAD_EXITED;			\
  } while(0)

/**
 * Declare the end of a protothread.
 *
 * This macro is used for declaring that a protothread ends. It should
 * always be used together with a matching PT_BEGIN() macro.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \hideinitializer
 */
#define PT_END(pt) LC_END((pt)->lc); PT_EXIT(pt)

#define PT_RUNNING(f) (f == PT_THREAD_WAITING)

#endif /* __PT_H__ */

/**
 * \defgroup lc Local continuations
 * @{
 *
 * Local continuations form the basis for implementing protothreads. A
 * local continuation can be <i>set</i> in a specific function to
 * capture the state of the function. After a local continuation has
 * been set can be <i>resumed</i> in order to restore the state of the
 * function at the point where the local continuation was set.
 *
 *
 */

/**
 * \file lc.h
 *
 */

/**
 * \def LC_INIT(lc)
 *
 * Initialize a local continuation.
 *
 * This operation initializes the local continuation, thereby
 * unsetting any previously set continuation state. 
 */

/**
 * \def LC_SET(lc)
 *
 * Set a local continuation.
 *
 * The set operation saves the state of the function at the point
 * where the operation is executed. As far as the set operation is
 * concerned, the state of the function does <b>not</b> include the
 * call-stack or local (automatic) variables, but only the program
 * counter and such CPU registers that needs to be saved.
 */

/**
 * \def LC_RESUME(lc)
 *
 * Resume a local continuation.
 *
 * The resume operation resumes a previously set local continuation, thus
 * restoring the state in which the function was when the local
 * continuation was set. If the local continuation has not been
 * previously set, the resume operation does nothing.
 *
 */

/**
 * \def LC_END(lc)
 *
 * Mark the end of local continuation usage.
 *
 * The end operation signifies that local continuations should not be
 * used any more in the function. This operation is not needed for
 * most implementations of local continuation, but is required by a
 * few implementations.
 *
 */

/**
 * \var typedef lc_t;
 *
 * The local continuation type.
 */

/** @} */
/** @} */
