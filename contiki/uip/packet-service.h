#ifndef __PACKET_SERVICE_H__
#define __PACKET_SERVICE_H__

#include "ek-service.h"

#include "uip.h"

/* Packet service interface. */

#define PACKET_SERVICE_NAME "TCP/IP packet service"
#define PACKET_SERVICE_VERSION 0x01
struct packet_service_state {
  u8_t version;
  void (* output)(u8_t *hdr, u16_t hdrlen, u8_t *data, u16_t datalen);
};

EK_PROCESS_INIT(packet_service_init, arg);

#endif /* __PACKET_SERVICE_H__ */
