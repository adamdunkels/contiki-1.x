#ifndef __MTARCH_H__
#define __MTARCH_H__

#define MTARCH_CPUSTACKSIZE 256
#define MTARCH_CSTACKSIZE 256
#define MTARCH_ZPSIZE 32

struct mtarch_thread {
  unsigned char  spreg;
  unsigned char *sp;
  unsigned char  cpustack[MTARCH_CPUSTACKSIZE];
  unsigned char  cstack[MTARCH_CSTACKSIZE];
  unsigned char  zp[MTARCH_ZPSIZE];
};

#endif /* __MTARCH_H__ */
	
