/**
 * \defgroup protothreads Protothreads
 *
 * Protothreads are a primitive form of threads that can be used to
 * implement sequential control flow and blocked waiting in
 * event-driven environments. a non-threaded environment. Unlike
 * ordinary threads, a protothread only provides linear control flow
 * in a single C function, and not the entire execution context. With
 * protothreads, local variables are not stored across a
 * yield. Furthermore, it is not possible to yield a protothread
 * inside a deep function call tree. (This is not entirely true, the
 * PT_WAIT_COND() function can be used for this.)
 *
 * Protothreads are very cheap to implement 
 within a single function must manage its own Unlike an
 * ordinary thread, a protothread do not require its own stack.
 *
 * Each protothread runs within its own C function. The protothread is
 * driven by repeated calls to the function that implements the
 * protothread.
 *
 * Example functionality implemented as an explicit state machine in
 * an event handler:
 \code
 static void
 eventhandler(int state)
 {
   switch(state) {
   case 1:
     if(condition1) {
       return;
     } else {
       next_state = 2;
       do_something();
     }
     break;
   case 2:
     if(condition2) {
       next_state = 3;
     } else {
       next_state = 4;
       do_something_else();
     }
     break;
   case 3:
     do_something_else();
     next_state = 4;
     break;
   case 4:
     next_state = 1;
     break;
   }
 }
 \endcode
 *
 * The same functionality implemented as a protothread:
 \code
 static void
 protothread(struct pt *pt)
 {
   PT_START(pt);

   while(condition1) {
     PT_YIELD(pt);
   }

   do_something();

   if(condition2) {
     PT_YIELD(pt);
   }

   do_something_else();   
      
   PT_RESTART(pt);
 }
 \endcode
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

#define PT_INIT(pt)				\
  (pt)->lc = LC_NULL
  
#define PT_START(pt)				\
  do {						\
    if((pt)->lc == LC_NULL) {			\
      LC_SET((pt)->lc);				\
    } else {					\
      LC_RESUME((pt)->lc);			\
    } 						\
  } while(0)

#define PT_YIELD(pt)				\
  do {						\
    LC_SET((pt)->lc);				\
    return;					\
  } while(0)

#define PT_WAIT_COND(pt, condition)	        \
  do {						\
    LC_SET((pt)->lc);				\
    if(condition) {				\
      return;					\
    }						\
  } while(0)


#define PT_RESTART(pt)				\
  do {						\
    (pt)->lc = LC_NULL;				\
    return;					\
  } while(0)

#define PT_EXIT(pt) PT_INIT(pt)

#endif /* __PT_H__ */
