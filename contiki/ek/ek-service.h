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
 * $Id: ek-service.h,v 1.4 2005/02/07 07:50:35 adamdunkels Exp $
 */

/**
 * \addtogroup ek
 * @{
 */

/**
 * \defgroup ekservice The service layer
 * @{
 */

/**
 * \file
 * Service layer header file.
 * \author Adam Dunkels <adam@sics.se>
 */

#ifndef __EK_SERVICE_H__
#define __EK_SERVICE_H__

#include "ek.h"

struct ek_service {
  const char *name;
  ek_id_t id;
};

/**
 * Declare a service state.
 *
 * This macro declares a service state. The service state is used to
 * access a particular service. The declared service state serves as
 * the input to all other service functions.
 *
 * \param service The name of the service state.
 * \param name The name of the service - this is defined in the header file of the service.
 *
 * \hideinitializer
 */
#define EK_SERVICE(service, name) \
 static struct ek_service service = {name, EK_ID_NONE}

/**
 * Start a service.
 *
 * This function is called from within a service in order to start the
 * service process and to register the service with Contiki. If
 * another instance of the same service is running, it is requested to
 * shut itself down before the current service is started.
 *
 * This function is the equivalent of ek_start() but for services.
 *
 * \note Do not call the function ek_start() from a service, use
 * ek_service_start() instead.
 *
 * \param name The name of the service. The first part of the name
 * must be indentical for all services of the same kind.
 *
 * \param p The process structure for the service.
 *
 * \return The process ID of the service.
 */ 
ek_id_t ek_service_start(const char *name, struct ek_proc *p);

/**
 * Find a running service.
 *
 * This function finds a running service. The function is called from
 * programs that utilize the service, not from within a service process.
 *
 * Example:
 \code
 #include "packet-service.h"
 
 EK_SERVICE(packetservice, PACKET_SERVICE_NAME);

 void stop_packetservice(void) {
   if(ek_service_find(&packetservice) == EK_ERR_OK) {
     ek_service_post(&packetservice, EK_EVENT_REQUEST_EXIT, NULL);
     ek_service_reset(&packetservice);
   }
 }
 \endcode
 *
 * \param s The service state. This state is filled in with
 * information by the ek_service_find() function.
 *
 * \retval EK_ERR_OK The requested service was found.
 * \retval EK_ERR_NOTFOUND The requested service was not found.
 */
ek_err_t ek_service_find(struct ek_service *s);

/**
 * Retrieve the service interface for a running service.
 *
 * This function retrieves the service interface (the API for the
 * service) for a running service. The service state must first have
 * been looked up using the ek_service_find() function.
 *
 * This function is used in the service stub.
 *
 * Example
 \code
 #include "packet-service.h"
  
 EK_SERVICE(packetservice, PACKET_SERVICE_NAME);

 static struct packet_service_interface *
 find_interface(void)
 {
   struct packet_service_interface *interface;
   interface = (struct packet_service_interface *)ek_service_state(&service);
   if(interface != NULL &&
      interface->version == PACKET_SERVICE_VERSION) {
     return interface;
   } else {
     return NULL;
   }
 }
 \endcode
 *
 * \param s The service state that previously has been looked up with
 * ek_service_find().
 *
 * \return A pointer to the service interface if the requested service
 * is running, or NULL if no such service was found.
 */
void *ek_service_state(struct ek_service *s);

/**
 * Reset a service.
 *
 * When a service is looked up by Contiki, the process ID of the
 * service is cached. This function resets the cache so that a running
 * service can be removed from the system.
 *
 * Example:
 \code
 #include "packet-service.h"
 
 EK_SERVICE(packetservice, PACKET_SERVICE_NAME);

 void stop_packetservice(void) {
   if(ek_service_find(&packetservice) == EK_ERR_OK) {
     ek_service_post(&packetservice, EK_EVENT_REQUEST_EXIT, NULL);
     ek_service_reset(&packetservice);
   }
 }
 \endcode
 *
 * \param s The service state.
 */
void ek_service_reset(struct ek_service *s);

/**
 * Post an event to a running service.
 *
 * This macro posts an event to a running service. The service must
 * first be looked up using ek_service_find().
 *
 * \sa ek_post().
 *
 * \param s The service state
 * \param ev The event to be posted.
 * \param data An opaque pointer to be posted with the event.
 * 
 * \hideinitializer
 */
#define ek_service_post(s, ev, data) ek_post((s)->id, (ev), (data))

/*ek_err_t ek_service_call(struct ek_service *s,
  ek_event_t ev, ek_data_t data);*/

/** @} */
/** @} */

#endif /* __EK_SERVICE_H__ */

