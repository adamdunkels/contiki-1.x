#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "clock-conf.h"

void clock_init(void);

clock_time_t clock_time(void);

#ifdef CLOCK_CONF_SECOND
#define CLOCK_SECOND CLOCK_CONF_SECOND
#else
#define CLOCK_SECOND (clock_time_t)32
#endif

#endif /* __CLOCK_H__ */
