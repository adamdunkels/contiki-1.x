/**
 * \file
 * Implementation of the archtecture agnostic parts of the preemptive
 * multithreading library for Contiki.
 *
 * \author
 * Adam Dunkels <adam@sics.se>
 *
 */

#include "mt.h"
#include "cc.h"
#include "dispatcher.h"

#define MT_STATE_READY   1
#define MT_STATE_RUNNING 2
#define MT_STATE_WAITING 3
#define MT_STATE_EXITED  4

static struct mt_thread *current;

/*-----------------------------------------------------------------------------------*/
void
mt_init(void)
{
  mtarch_init();
}
/*-----------------------------------------------------------------------------------*/
void
mt_remove(void)
{
  mtarch_remove();
}
/*-----------------------------------------------------------------------------------*/
void
mt_start(struct mt_thread *thread, void (* function)(void *), void *data)
{
  /* Call the architecture dependant function to set up the processor
     stack with the correct parameters. */
  mtarch_start(&thread->thread, function, data);

  thread->state = MT_STATE_READY;
}
/*-----------------------------------------------------------------------------------*/
void
mt_exec(struct mt_thread *thread)
{
  if(thread->state == MT_STATE_READY) {
    thread->state = MT_STATE_RUNNING;
    current = thread;
    /* Switch context to the thread. The function call will not return
       until the the thread has yielded, or is preempted. */
    mtarch_exec(&thread->thread);
  }
}
/*-----------------------------------------------------------------------------------*/
void
mt_exit(void)
{
  current->state = MT_STATE_EXITED;
  current = NULL;
  mtarch_yield();
}
/*-----------------------------------------------------------------------------------*/
void
mt_exec_event(struct mt_thread *thread, ek_signal_t s, ek_data_t data)
{
  if(thread->state == MT_STATE_WAITING &&
     thread->s == s) {
    thread->data = data;
    thread->state = MT_STATE_RUNNING;
    current = thread;    
    /* Switch context to the thread. The function call will not return
       until the the thread has yielded, or is preempted. */
    mtarch_exec(&thread->thread);
  }
}
/*-----------------------------------------------------------------------------------*/
void
mt_yield(void)
{    
  mtarch_pstop();
  current->state = MT_STATE_READY;
  current = NULL;
  /* This function is called from the running thread, and we call the
     switch function in order to switch the thread to the main Contiki
     program instead. For us, the switch function will not return
     until the next time we are scheduled to run. */
  mtarch_yield();
  
}
/*-----------------------------------------------------------------------------------*/
void
mt_emit(ek_signal_t s, ek_data_t data, ek_id_t id)
{  
  /* Turn off preemption to ensure mutual exclusion of kernel. */
  mtarch_pstop();

  dispatcher_emit(s, data, id);
  
  /* Turn preemption on again. */  
  mtarch_pstart();  
}
/*-----------------------------------------------------------------------------------*/
void
mt_listen(ek_signal_t s)
{
  mtarch_pstop();
  dispatcher_listen(s);
  mtarch_pstart();
}
/*-----------------------------------------------------------------------------------*/
ek_data_t
mt_wait(ek_signal_t s)
{
  mtarch_pstop();
  current->s = s;
  current->state = MT_STATE_WAITING;
  current = NULL;
  mtarch_yield();
  return current->data;
}
/*-----------------------------------------------------------------------------------*/
