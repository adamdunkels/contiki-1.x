#include "timer.h"

/*---------------------------------------------------------------------------*/
void
timer_set(struct timer *t, clock_time_t interval)
{
  t->interval = interval;
  t->start = clock_time();
}
/*---------------------------------------------------------------------------*/
void
timer_reset(struct timer *t)
{
  t->start = t->start + t->interval;
}
/*---------------------------------------------------------------------------*/
int
timer_expired(struct timer *t)
{
  return (clock_time_t)(clock_time() - t->start) >= (clock_time_t)t->interval;
}
/*---------------------------------------------------------------------------*/

