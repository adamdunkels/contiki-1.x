/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the Contiki operating system.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: ek-service.c,v 1.6 2005/02/07 07:50:35 adamdunkels Exp $
 */

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

