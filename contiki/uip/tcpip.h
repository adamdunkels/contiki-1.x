#ifndef __TCPIP_H__
#define __TCPIP_H__

#include "ek.h"




struct uip_conn;

struct tcpip_uipstate {
  ek_id_t id;  
  void *state;
};

#define UIP_APPCALL tcpip_uipcall
#define UIP_UDP_APPCALL tcpip_uipcall
#define UIP_APPSTATE_SIZE sizeof(struct tcpip_uipstate)

#include "uip.h"

EK_PROCESS_INIT(tcpip_init, arg);
     
void tcpip_uipcall(void);

void tcp_markconn(struct uip_conn *conn,
		  void *appstate);

void tcp_listen(u16_t port);

void tcp_unlisten(u16_t port);

struct uip_conn *tcp_connect(u16_t *ripaddr, u16_t port,
			     void *appstate);

struct uip_udp_conn *udp_new(u16_t *ripaddr, u16_t port,
			     void *appstate);
extern ek_event_t tcpip_event;

#define udp_bind(conn, port) uip_udp_bind(conn, port)


void tcpip_set_forwarding(unsigned char f);
void tcpip_input(void);
void tcpip_output(void);


#endif /* __TCPIP_H__ */
