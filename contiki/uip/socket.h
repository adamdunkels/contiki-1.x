#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "pt.h"
#include "uipbuf.h"
#include "memb.h"

struct socket {
  struct pt pt, socketpt;
  unsigned char state;
#define SOCKET_STATE_NONE 0
#define SOCKET_STATE_ACKED 1
#define SOCKET_STATE_READ 2
#define SOCKET_STATE_BLOCKED_NEWDATA 3
#define SOCKET_STATE_BLOCKED_CLOSE 4
#define SOCKET_STATE_BLOCKED_SEND 5
  u8_t *sendptr;
  u16_t sendlen;
  u8_t *readptr;
  u16_t readlen;

  struct uipbuf_buffer buf;
  char *bufptr;
  int bufsize;
};

void socket_setup(struct socket *socket, char *buffer, int buffersize);
void *socket_alloc(struct memb_blocks *m);

#define SOCKET_INIT(socket, buffer, buffersize) \
  socket_setup(socket, buffer, buffersize);
 

#define SOCKET_ACCEPT(type, s, socket, states, buffer, buffersize)    	\
  do {									\
    if(uip_connected()) {						\
      s = socket_alloc(states);						\
      if(s == NULL) {							\
	uip_abort();							\
	return PT_THREAD_EXITED;					\
      }									\
      socket_setup(&s->socket, s->buffer, buffersize);			\
    } else if(uip_timedout() || uip_closed() || uip_aborted()) {	\
      memb_free(states, s);						\
      tcp_markconn(uip_conn, NULL);					\
      if(s == NULL || s->socket.state != SOCKET_STATE_BLOCKED_CLOSE) {	\
	return PT_THREAD_EXITED;					\
      }									\
    } else if(s == NULL) {						\
      return PT_THREAD_EXITED;						\
    }    								\
    PT_START(&s->socket.pt);						\
  } while(0)

PT_THREAD(socket_send(struct socket *socket));

#define SOCKET_SEND(socket, data, datalen)		\
  do {							\
    (socket)->sendlen = datalen;			\
    (socket)->sendptr = data;				\
    PT_WAIT_THREAD(&(socket)->pt, socket_send(socket));	\
  } while(0)

/*
#define SOCKET_SEND_DYNAMIC(socket, dataptr, function_call)
*/

PT_THREAD(socket_close(struct socket *socket));

#define SOCKET_CLOSE(socket)				\
  PT_WAIT_THREAD(&(socket)->pt, socket_close(socket))


PT_THREAD(socket_readto(struct socket *socket, unsigned char c));

#define SOCKET_READTO(socket, c)				\
  PT_WAIT_THREAD(&(socket)->pt, socket_readto(socket, c))


#define SOCKET_EXIT(socket) PT_EXIT(&(socket)->pt)

#endif /* __SOCKET_H__ */
