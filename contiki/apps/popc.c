
#include "contiki.h"
#include "popc.h"
#include "popc-strings.h"

#define SEND_STRING(s, str) SOCKET_SEND(s, str, strlen(str))

enum {
  COMMAND_NONE,
  COMMAND_RETR,
  COMMAND_TOP,  
  COMMAND_QUIT
};

/*---------------------------------------------------------------------------*/
static
PT_THREAD(init_connection(struct popc_state *s))
{
  SOCKET_BEGIN(&s->s);

  SOCKET_READTO(&s->s, '\n');
  if(s->inputbuf[0] != '+') {
    SOCKET_CLOSE_EXIT(&s->s);    
  }

  SEND_STRING(&s->s, popc_strings_user);
  SEND_STRING(&s->s, s->user);
  SEND_STRING(&s->s, popc_strings_crnl);
  
  SOCKET_READTO(&s->s, '\n');
  if(s->inputbuf[0] != '+') {
    SOCKET_CLOSE_EXIT(&s->s);    
  }

  SEND_STRING(&s->s, popc_strings_pass);
  SEND_STRING(&s->s, s->pass);
  SEND_STRING(&s->s, popc_strings_crnl);
  
  SOCKET_READTO(&s->s, '\n');
  if(s->inputbuf[0] != '+') {
    SOCKET_CLOSE_EXIT(&s->s);    
  }

  popc_connected(s);

  SOCKET_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(stat(struct popc_state *s))
{
  unsigned short num;
  unsigned long size;
  char *ptr;
  
  SOCKET_BEGIN(&s->s);
  
  SEND_STRING(&s->s, popc_strings_stat);
  
  SOCKET_READTO(&s->s, '\n');
  if(s->inputbuf[0] != '+') {
    SOCKET_CLOSE_EXIT(&s->s);    
  }

  num = 0;
  for(ptr = &s->inputbuf[4]; *ptr >= '0' && *ptr <= '9'; ++ptr) {
    num *= 10;
    num += *ptr - '0';
  }

  size = 0;
  for(ptr = ptr + 1; *ptr >= '0' && *ptr <= '9'; ++ptr) {
    size *= 10;
    size += *ptr - '0';
  }

  popc_messages(s, num, size);

  SOCKET_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(retr(struct popc_state *s))
{
  SOCKET_BEGIN(&s->s);

  SEND_STRING(&s->s, popc_strings_retr);
  snprintf(s->outputbuf, sizeof(s->outputbuf), "%d", s->num);
  SEND_STRING(&s->s, s->outputbuf);
  SEND_STRING(&s->s, popc_strings_crnl);
  
  SOCKET_READTO(&s->s, '\n');
  if(s->inputbuf[0] != '+') {
    SOCKET_CLOSE_EXIT(&s->s);    
  }

  popc_msgbegin(s);
  while(s->inputbuf[0] != '.') {
    SOCKET_READTO(&s->s, '\n');
    if(s->inputbuf[0] != '.') {
      s->inputbuf[SOCKET_DATALEN(&s->s)] = 0;
      popc_msgline(s, s->inputbuf, SOCKET_DATALEN(&s->s));
    }
  }
  popc_msgend(s);
  
  SOCKET_END(&s->s);

}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_connection(struct popc_state *s))
{
  PT_BEGIN(&s->pt);

  SOCKET_INIT(&s->s, s->inputbuf, sizeof(s->inputbuf) - 1);

  PT_WAIT_UNTIL(&s->pt, init_connection(s));
  PT_WAIT_UNTIL(&s->pt, stat(s));  

  timer_set(&s->timer, CLOCK_SECOND * 30);
	
  while(1) {
    PT_WAIT_UNTIL(&s->pt, s->command != COMMAND_NONE ||
		  timer_expired(&s->timer));

    if(timer_expired(&s->timer)) {
      PT_WAIT_UNTIL(&s->pt, stat(s));
      timer_set(&s->timer, CLOCK_SECOND * 30);
    }

    switch(s->command) {
    case COMMAND_RETR:
      PT_WAIT_UNTIL(&s->pt, retr(s));
      break;
    case COMMAND_QUIT:
      tcp_markconn(uip_conn, NULL);
      SOCKET_CLOSE(&s->s);
      PT_EXIT(&s->pt);
      break;
    default:
      break;
    }
    s->command = COMMAND_NONE;
    
  }
  PT_END(&s->pt);
}
/*---------------------------------------------------------------------------*/
void
popc_appcall(void *state)
{
  struct popc_state *s = (struct popc_state *)state;
  
  if(uip_closed() || uip_aborted() || uip_timedout()) {
    popc_closed(s);
  } else if(uip_connected()) {
    PT_INIT(&s->pt);
    handle_connection(s);
  } else if(s != NULL) {
    handle_connection(s);
  }

}
/*---------------------------------------------------------------------------*/
void *
popc_connect(struct popc_state *s, u16_t *addr,
	     char *user, char *pass)
{
  strncpy(s->user, user, sizeof(s->user));
  strncpy(s->pass, pass, sizeof(s->pass));
  s->conn = tcp_connect(addr, HTONS(110), s);
  return s->conn;
}
/*---------------------------------------------------------------------------*/
void
popc_retr(struct popc_state *s, unsigned short num)
{
  s->command = COMMAND_RETR;
  s->num = num;
}
/*---------------------------------------------------------------------------*/
void
popc_top(struct popc_state *s, unsigned short num, unsigned short lines)
{
  s->command = COMMAND_TOP;
  s->num = num;
  s->lines = lines;
}
/*---------------------------------------------------------------------------*/
