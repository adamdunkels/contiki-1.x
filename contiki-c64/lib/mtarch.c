#include "mtarch.h"
#include <string.h>

unsigned char  mtarch_asm_threadspreg;
unsigned char *mtarch_asm_threadsp;
unsigned char *mtarch_asm_threadzp;
unsigned char *mtarch_asm_threadstack;

void mtarch_asm_start(void);
void mtarch_asm_exec(void);


/*--------------------------------------------------------------------------*/
void
mtarch_start(struct mtarch_thread *thread,
	     void (* function)(void *data),
	     void *data)
{
  memset(thread->cpustack, 0, sizeof(thread->cpustack));
  memset(thread->cstack, 0, sizeof(thread->cstack));
  
  /* Create a CPU stack frame with the appropriate values... */
  thread->cpustack[MTARCH_CPUSTACKSIZE - 2] = ((unsigned short)function) >> 8; /* high byte of return address. */
  thread->cpustack[MTARCH_CPUSTACKSIZE - 3] = ((unsigned short)function) & 0xff; /* low byte of return address. */
  thread->cpustack[MTARCH_CPUSTACKSIZE - 4] = 0x21; /* processor flags. */
  thread->cpustack[MTARCH_CPUSTACKSIZE - 5] =       /* a register */
    thread->cpustack[MTARCH_CPUSTACKSIZE - 6] =     /* x register */
    thread->cpustack[MTARCH_CPUSTACKSIZE - 7] = 0;  /* y register */
  thread->spreg = MTARCH_CPUSTACKSIZE - 8;

  /* Setup the C stack with the data pointer. */
  thread->sp = &thread->cstack[MTARCH_CSTACKSIZE - 1];
  
  mtarch_asm_threadzp    = &(thread->zp);  
  mtarch_asm_start();
}
/*--------------------------------------------------------------------------*/
void
mtarch_exec(struct mtarch_thread *thread)
{
  /* Switch processor stack. The call to mtarch_asm_switch() will not
     return until the process that we switch to calls yield(). */
  mtarch_asm_threadspreg = thread->spreg;
  mtarch_asm_threadsp    = thread->sp;

  mtarch_asm_threadstack = &(thread->cpustack[0]);  
  mtarch_asm_threadzp    = &(thread->zp[0]);
  
  mtarch_asm_exec();

  thread->sp = mtarch_asm_threadsp;
  thread->spreg = mtarch_asm_threadspreg;  
}
/*--------------------------------------------------------------------------*/
void
mtarch_init(void) {

}
/*--------------------------------------------------------------------------*/
void
mtarch_remove(void)
{

}
