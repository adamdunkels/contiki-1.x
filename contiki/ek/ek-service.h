#ifndef __EK_SERVICE_H__
#define __EK_SERVICE_H__

#include "ek.h"

struct ek_service {
  const char *name;
  ek_id_t id;
};
#define EK_SERVICE(service, name) \
 static struct ek_service service = {name, EK_ID_NONE}

ek_id_t ek_service_start(const char *name, struct ek_proc *p);

ek_err_t ek_service_find(struct ek_service *s);

void *ek_service_state(struct ek_service *s);


/*ek_err_t ek_service_call(struct ek_service *s,
  ek_event_t ev, ek_data_t data);*/

#endif /* __EK_SERVICE_H__ */
