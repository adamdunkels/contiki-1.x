/**
 * \defgroup pt Protothreads 
 * @{ 
 *
 * Protothreads are lightweight stackless threads that can be used to
 * provide blocking contexts in event-driven systems. This is useful
 * for implementing sequential control flow, without requiring
 * ordinary threads and multiple stacks.
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
 * inside deep function calls are made by spawning a separate
 * protothread for each function.
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
 * initialization: PT_INIT(), execution: PT_START(), blocking:
 * PT_WAIT_UNTIL() and exit: PT_EXIT(). On top of these, two
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
   PT_START(p);
   PT_WAIT_UNTIL(event == AVAILABLE);
   consume();
   PT_WAIT_UNTIL(event == CONSUMED);
   acknowledge_consumed();
   PT_EXIT(p);
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
  (pt)->lc = LC_NULL

/**
 * Start a protothread.
 *
 * This macro is used to set the starting point of a protothread. It
 * should be placed at the start of the function in which the
 * protothread runs. All C statements above the PT_START() invokation
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
 
   PT_START(p);

   PT_WAIT_UNTIL(empty);
   produce();
   PT_WAIT_UNTIL(event == PRODUCED);   
   
   PT_EXIT(p);
 }
 \endcode
 *
 * \hideinitializer
 */
#define PT_START(pt)				\
  do {						\
    if((pt)->lc == LC_NULL) {			\
      LC_SET((pt)->lc);				\
    } else {					\
      LC_RESUME((pt)->lc);			\
    } 						\
  } while(0)

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
   PT_START(p);

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
    if(! condition) {				\
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
   PT_START(p);

   PT_WAIT_UNTIL(event == EVENT1);   
   
   PT_EXIT(p);
 }

 PT_THREAD(parent(struct pt *p, struct pt *child_pt, int event)) {
   PT_START(p);

   PT_INIT(child_pt);
   
   PT_WAIT_THREAD(p, child(child_pt, event));
   
   PT_EXIT(p);
 }
 \endcode
 *
 * \hideinitializer 
 */
#define PT_WAIT_THREAD(pt, thread)		\
  PT_WAIT_UNTIL((pt), (thread))

/**
 * Restart the protothread.
 *
 * This macro will block and cause the protothread to restart its
 * execution at the place of the PT_START() call.
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

#define PT_RUNNING(f) (f == PT_THREAD_WAITING)

#endif /* __PT_H__ */
/** @} */
