#ifndef __POPC_H__
#define __POPC_H__

#include "socket.h"


struct popc_state {
  struct pt pt;
  struct socket s;
  struct uip_conn *conn;

  struct timer timer;
  
  char inputbuf[100];
  char outputbuf[10];
  
  char user[32], pass[32];

  char command;
  unsigned short num;
  unsigned short lines;
};

void popc_appcall(void *state);
void popc_init(void);
void *popc_connect(struct popc_state *s, u16_t *ipaddr, 
		  char *user, char *passwd);

void popc_retr(struct popc_state *s, unsigned short msg);
void popc_top(struct popc_state *s, unsigned short msg,
	      unsigned short numlines);


void popc_connected(struct popc_state *s);
void popc_messages(struct popc_state *s,
		   unsigned short num, unsigned long size);
void popc_msgbegin(struct popc_state *s);
void popc_msgline(struct popc_state *s, char *line, int len);
void popc_msgend(struct popc_state *s);


#endif /* __POPC_H__ */
