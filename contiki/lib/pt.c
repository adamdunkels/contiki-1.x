#include "pt.h"

/*---------------------------------------------------------------------------*/
char
pt_newdata(struct pt_state *pt)
{
  if(pt->state == PT_STATE_READ || !uip_newdata()) {
    pt->state = PT_STATE_BLOCKED_NEWDATA;
    return 1;
  }
  pt->state = PT_STATE_READ;
  return 0;
}
/*---------------------------------------------------------------------------*/
char
pt_send(struct pt_state *pt)
{
  if(pt->state == PT_STATE_ACKED || !uip_acked()) {
    if(pt->tmplen > uip_mss()) {   
      uip_send(pt->tmpptr, uip_mss());
    } else {
      uip_send(pt->tmpptr, pt->tmplen);
    }
    pt->state = PT_STATE_BLOCKED_SEND;
    return 1;
  } else {
    if(pt->tmplen > uip_mss()) {
      pt->tmplen -= uip_mss();
      pt->tmpptr += uip_mss();
      pt->state = PT_STATE_BLOCKED_SEND;
      return 1;
    }
  }
  pt->state = PT_STATE_ACKED;
  return 0;
}
/*---------------------------------------------------------------------------*/
