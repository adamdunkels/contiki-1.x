#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "pt-2.h"
#include "uipbuf.h"
#include "memb.h"

struct socket {
  struct pt pt;
  unsigned char state;
#define SOCKET_STATE_NONE 0
#define SOCKET_STATE_ACKED 1
#define SOCKET_STATE_READ 2
#define SOCKET_STATE_BLOCKED_NEWDATA 3
#define SOCKET_STATE_BLOCKED_CLOSE 4
#define SOCKET_STATE_BLOCKED_SEND 5
  u16_t tmplen;
  u8_t *tmpptr;
  u8_t *readptr;
  u16_t readlen;

  struct uipbuf_buffer buf;
  char *bufptr;
  int bufsize;
};

char socket_wait_data(struct socket *socket);
char socket_send(struct socket *socket);
char socket_closed(struct socket *socket);
void socket_setup(struct socket *socket, char *buffer, int buffersize);
void *socket_alloc(struct memb_blocks *m);

#define SOCKET_ACCEPT(type, s, socket, states, buffer, buffersize)		\
  do {									\
    if(uip_connected()) {						\
      s = socket_alloc(states);						\
      if(s == NULL) {							\
	uip_abort();							\
	return;								\
      }									\
      socket_setup(&s->socket, s->buffer, buffersize);			\
    } else if(uip_timedout() || uip_closed() || uip_aborted()) {	\
      memb_free(states, s);						\
      tcp_markconn(uip_conn, NULL);					\
      if(s == NULL || s->socket.state != SOCKET_STATE_BLOCKED_CLOSE) {	\
	return;								\
      }									\
    } else if(s == NULL) {						\
      return;								\
    }    								\
    PT_START(&s->socket.pt);						\
  } while(0)

     
#define SOCKET_WAIT_DATA(socket)			\
  PT_WAIT_COND(&(socket)->pt, socket_wait_data(socket))

#define SOCKET_SEND(socket, data, datalen)			\
  do {							\
    (socket)->tmplen = datalen;			       	\
    (socket)->tmpptr = data;				\
    PT_WAIT_COND(&(socket)->pt, socket_send(socket));	\
  } while(0)

/*
#define SOCKET_SEND_DYNAMIC(socket, dataptr, function, args)	  \
  do {							  \
    (socket)->tmpptr = dataptr;				  \
    (socket)->tmplen = function args;			  \
    LC_SET((socket)->lc);					  \
    if((socket)->state == SOCKET_STATE_ACKED || !uip_acked()) {	  \
      if(uip_rexmit() || (socket)->state != SOCKET_STATE_ACKED) { \
	function args;					  \
      }							  \
      if(socket_send(socket)) {					  \
	return;						  \
      }							  \
    }							  \
    (socket)->state = SOCKET_STATE_ACKED;                         \
  } while(0)
*/

#define SOCKET_CLOSE(socket)			        \
  do {							\
    uip_close();					\
    (socket)->state = SOCKET_STATE_BLOCKED_CLOSE;		\
    PT_WAIT_COND(&(socket)->pt, socket_closed(socket));	\
  } while(0)


/* XXX: Must add uipbuf_checkmarker() before do{} loop. */
/*#define SOCKET_READTO(socket, c)					\
  do {								\
    if((socket)->readlen == 0) {					\
      SOCKET_WAIT_DATA(socket);					\
      (socket)->readptr = (u8_t *)uip_appdata;			\
      (socket)->readlen = uip_datalen();				\
    }								\
  } while((uipbuf_bufdata_endmarker(&(socket)->buf, c,			\
				    &((socket)->readptr),		\
				    &((socket)->readlen)) &	\
	   UIPBUF_FOUND) == 0					\
	   && (socket)->readlen > 0)*/

#define SOCKET_READTO(socket, c)	 \
  do {							 \
    uipbuf_setup(&(socket)->buf, (socket)->bufptr, (socket)->bufsize);    	 \
    do {      						 \
      if((socket)->readlen == 0) {                           \
        SOCKET_WAIT_DATA(socket);					 \
        (socket)->readptr = (u8_t *)uip_appdata;		 \
        (socket)->readlen = uip_datalen();			 \
      }                            			 \
    } while((uipbuf_bufdata_endmarker(&(socket)->buf, c,	 \
				      &((socket)->readptr),	 \
				      &((socket)->readlen)) & \
	     UIPBUF_FOUND) == 0				 \
	    && (socket)->readlen > 0);			 \
  } while(0)

/*
#define SOCKET_READTO(socket, c, uipbuf, buffer, buffersize)	 \
  do {							 \
    uipbuf_setup(uipbuf, buffer, buffersize);    	 \
    do {      						 \
      if((socket)->readlen == 0) {                           \
        SOCKET_WAIT_DATA(socket);					 \
        (socket)->readptr = (u8_t *)uip_appdata;		 \
        (socket)->readlen = uip_datalen();			 \
      }                            			 \
    } while((uipbuf_bufdata_endmarker(uipbuf, c,	 \
				      &((socket)->readptr),	 \
				      &((socket)->readlen)) & \
	     UIPBUF_FOUND) == 0				 \
	    && (socket)->readlen > 0);			 \
  } while(0)

#define SOCKET_READ(socket, len, uipbuf, buffer, buffersize)	\
  do {							\
    uipbuf_setup(uipbuf, buffer, buffersize);		\
    do {						\
      SOCKET_NEWDATA(socket);					\
      (socket)->readptr = (u8_t *)uip_appdata;		\
      (socket)->readlen = uip_datalen();			\
    } while((uipbuf_bufdata(uipbuf, len,		\
			    &((socket)->readpt),		\
			    &((socket)->readlen)) &		\
	     UIPBUF_FOUND) == 0				\
	    && (socket)->readlen > 0);			\
  } while(0)
*/

#endif /* __SOCKET_H__ */
