#ifndef __TR1001_H__
#define __TR1001_H__

#include "uip.h"

void tr1001_init(void);

void tr1001_set_txpower(unsigned char p);

u8_t tr1001_send(void);
u8_t tr1001_ack(void);
u8_t tr1001_send_acked(void);
unsigned short tr1001_poll(void);


#endif /* __TR1001_H__ */
