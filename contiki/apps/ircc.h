#ifndef __IRCC_H__
#define __IRCC_H__

#include "socket.h"

struct ircc_state {
  
  struct pt pt;
  struct socket s;

  struct uip_conn *conn;
  
  unsigned char command;
  
  char *msg;
  char channel[32];
  char outputbuf[80];
  char inputbuf[120];
  char *nick;
  char *server;
};

void ircc_init(void);

void ircc_appcall(void *s);

struct ircc_state *ircc_connect(struct ircc_state *s,
				char *server, u16_t *ipaddr, char *nick);

void ircc_join(struct ircc_state *s, char *channel);
void ircc_part(struct ircc_state *s);
void ircc_list(struct ircc_state *s);
void ircc_msg(struct ircc_state *s, char *msg);

void ircc_sent(struct ircc_state *s);

void ircc_text_output(struct ircc_state *s, char *text1, char *text2);

void ircc_connected(struct ircc_state *s);
void ircc_closed(struct ircc_state *s);

#endif /* __IRCC_H__ */
