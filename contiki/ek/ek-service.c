
#include "ek-service.h"

/*---------------------------------------------------------------------------*/
ek_id_t
ek_service_start(const char *name, struct ek_proc *p)
{
  ek_id_t service;
  
  service = ek_find(name);

  if(service == EK_ID_NONE) {
    return ek_start(p);
  } else {
    ek_post(service, EK_EVENT_REQUEST_REPLACE, p);
    return service;
  } 

}
/*---------------------------------------------------------------------------*/
ek_err_t
ek_service_find(struct ek_service *s)
{
  ek_id_t id;
  id = ek_find(s->name);
  s->id = id;
  if(s->id == EK_ID_NONE) {
    return EK_ERR_NOTFOUND;
  }
  return EK_ERR_OK;
}
/*---------------------------------------------------------------------------*/
void *
ek_service_state(struct ek_service *s)
{
  if(s->id == EK_ID_NONE) {
    if(ek_service_find(s) == EK_ERR_NOTFOUND) {
      return NULL;
    }
  }
  return ek_procstate(s->id);  
}
/*---------------------------------------------------------------------------*/
/*ek_err_t
ek_service_call(struct ek_service *s, ek_event_t ev, ek_data_t data)
{
  if(s->id == EK_ID_NONE) {
    if(ek_service_find(s) == EK_ERR_NOTFOUND) {
      return EK_ERR_NOTFOUND;
    }
  }
  ek_post_synch(s->id, ev, data);
  return EK_ERR_OK;
}*/
/*---------------------------------------------------------------------------*/

