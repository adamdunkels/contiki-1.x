
#ifndef _LAN91C96_H
#define _LAN91C96_H

#include "uip_arch.h"

void lan91c96_init(void);
void lan91c96_done(void);
void lan91c96_send(void);
#if UIP_BUFSIZE > 255
u16_t lan91c96_poll(void);
#else
u8_t lan91c96_poll(void);
#endif

/* End of lan91c96.h */
#endif

