
#include "ek-service.h"

#include "log.h"

/*---------------------------------------------------------------------------*/
ek_id_t
ek_service_start(const char *name, struct ek_proc *p)
{
  ek_id_t service;
  
  service = ek_find(name);

  if(service == EK_ID_NONE) {
    log_message("ek-service: starting ", name);
    return ek_start(p);
  } else {
    log_message("ek-service: replacing ", name);
    ek_post_synch(service, EK_EVENT_REQUEST_REPLACE, p);
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
void
ek_service_reset(struct ek_service *s)
{
  log_message("ek-service: reseting ", s->name);
  s->id = EK_ID_NONE;
}
/*---------------------------------------------------------------------------*/
#if 0
unsigned char
ek_service_ref(struct ek_service *s)
{
}
/*---------------------------------------------------------------------------*/
unsigned char
ek_service_unref(struct ek_service *s)
{
}
#endif 
/*---------------------------------------------------------------------------*/

