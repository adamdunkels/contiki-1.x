/* pt: TCP/IP application "protothread" (proto meaning both
   "primitive" and "protocol") programming based on "local
   continuations". */ 


#ifndef __PT_H__
#define __PT_H__

#include "uip.h"
#include "lc.h"
#include "uipbuf.h"

struct pt_state {
  lc_t lc;
  unsigned char state;
#define PT_STATE_NONE 0
#define PT_STATE_ACKED 1
#define PT_STATE_READ 2
#define PT_STATE_BLOCKED_NEWDATA 3
#define PT_STATE_BLOCKED_CLOSE 4
#define PT_STATE_BLOCKED_SEND 5
  u16_t tmplen;
  u8_t *tmpptr;
  u8_t *readptr;
  u16_t readlen;
};

char pt_newdata(struct pt_state *pt);
char pt_send(struct pt_state *pt);

#define PT_INIT(pt) (pt)->lc = LC_NULL

#define PT_CALL(pt, function, args)		\
  do {						\
    LC_SET((pt)->lc);				\
    if(function args) {				\
      return;					\
    }						\
  } while(0)


#define PT_ACCEPT(type, s, pt, states)					 \
  do {									 \
    if(uip_timedout() || uip_closed() || uip_aborted()) { 		 \
      if(s != NULL && s->pt.state == PT_STATE_BLOCKED_CLOSE) {		 \
	LC_CALL(s->pt.lc);						 \
      }									 \
      memb_free(states, s);                                     	 \
      tcp_markconn(uip_conn, NULL);                                      \
      return;								 \
    } else if(uip_connected()) {					 \
      s = (type *)memb_alloc(states);					 \
      if(s == NULL) {							 \
	uip_abort();							 \
	return;								 \
      }									 \
      tcp_markconn(uip_conn, s);					 \
      LC_SET(s->pt.lc);							 \
    } else if(s == NULL) {						 \
      return;								 \
    } else {								 \
      LC_CALL(s->pt.lc);					      	 \
    }									 \
    s->pt.state = PT_STATE_NONE;                                         \
    s->pt.readlen = 0;                                                   \
  } while(0)							

#define PT_CONNECT(type, s, pt, states)					 \
  do {									 \
    if(uip_timedout() || uip_closed() || uip_aborted()) {		 \
      if(s != NULL && s->pt.state == PT_STATE_BLOCKED_CLOSE) {		 \
	LC_CALL(s->pt.lc);						 \
      }									 \
      memb_free(states, s);                                     	 \
      tcp_markconn(uip_conn, NULL);                                      \
      return;								 \
    } else if(uip_connected()) {					 \
      s->pt.state = PT_STATE_NONE;                                       \
    } else {								 \
      LC_CALL(s->pt.lc);					      	 \
    }									 \
  } while(0)							

#define PT_NEWDATA(pt) \
  PT_CALL(pt, pt_newdata, (pt))

#define PT_SEND(pt, data, datalen)			\
  do {							\
    (pt)->tmplen = datalen;			       	\
    (pt)->tmpptr = data;				\
    PT_CALL(pt, pt_send, (pt));				\
  } while(0)

#define PT_SEND_DYNAMIC(pt, dataptr, function, args)	  \
  do {							  \
    (pt)->tmpptr = dataptr;				  \
    (pt)->tmplen = function args;			  \
    LC_SET((pt)->lc);					  \
    if((pt)->state == PT_STATE_ACKED || !uip_acked()) {	  \
      if(uip_rexmit() || (pt)->state != PT_STATE_ACKED) { \
	function args;					  \
      }							  \
      if(pt_send(pt)) {					  \
	return;						  \
      }							  \
    }							  \
    (pt)->state = PT_STATE_ACKED;                         \
  } while(0)

#define PT_CLOSED(pt)						\
  do {								\
    LC_SET((pt)->lc);						\
    if(!(uip_closed() || uip_timedout() || uip_aborted())) {	\
      (pt)->state = PT_STATE_BLOCKED_CLOSE;			\
      return;      						\
    }								\
    (pt)->state = PT_STATE_NONE;				\
  } while(0)

#define PT_CLOSE(pt)				\
  do {						\
    uip_close();				\
    PT_CLOSED(pt);				\
  } while(0)

#define PT_READTO(pt, c, uipbuf, buffer, buffersize)	 \
  do {							 \
    uipbuf_setup(uipbuf, buffer, buffersize);    	 \
    do {      						 \
      if((pt)->readlen == 0) {                           \
        PT_NEWDATA(pt);					 \
        (pt)->readptr = (u8_t *)uip_appdata;		 \
        (pt)->readlen = uip_datalen();			 \
      }                            			 \
    } while((uipbuf_bufdata_endmarker(uipbuf, c,	 \
				      &((pt)->readptr),	 \
				      &((pt)->readlen)) & \
	     UIPBUF_FOUND) == 0				 \
	    && (pt)->readlen > 0);			 \
  } while(0)

#define PT_READ(pt, len, uipbuf, buffer, buffersize)	\
  do {							\
    uipbuf_setup(uipbuf, buffer, buffersize);		\
    do {						\
      PT_NEWDATA(pt);					\
      (pt)->readptr = (u8_t *)uip_appdata;		\
      (pt)->readlen = uip_datalen();			\
    } while((uipbuf_bufdata(uipbuf, len,		\
			    &((pt)->readpt),		\
			    &((pt)->readlen)) &		\
	     UIPBUF_FOUND) == 0				\
	    && (pt)->readlen > 0);			\
  } while(0)


#endif /* __PT_H__ */
