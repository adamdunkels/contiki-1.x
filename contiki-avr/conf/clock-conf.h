#ifndef __CLOCK_CONF_H__
#define __CLOCK_CONF_H__

typedef unsigned short clock_time_t;
#define CLOCK_CONF_SECOND 8

void clock_delay(unsigned int us2);

void clock_wait(int ms10);

void clock_set_seconds(unsigned long s);
unsigned long clock_seconds(void);


#endif /* __CLOCK_CONF_H__ */
