#include "timer.h"

/*---------------------------------------------------------------------------*/
void
timer_set(struct timer *t, clock_time_t interval)
{
  t->interval = interval;
  timer_reset(t);
}
/*---------------------------------------------------------------------------*/
void
timer_reset(struct timer *t)
{
  t->current = t->start = clock_time();
}
/*---------------------------------------------------------------------------*/
int
timer_expired(struct timer *t)
{
  t->current = clock_time();
  return (clock_time_t)(t->current - t->start) >= (clock_time_t)t->interval;
}
/*---------------------------------------------------------------------------*/

