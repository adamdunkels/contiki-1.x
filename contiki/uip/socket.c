#include "socket.h"

/*---------------------------------------------------------------------------*/
/*PT_THREAD(socket_wait_data(struct socket *socket))
{
  PT_START(&socket->socketpt);
  if(socket->state == SOCKET_STATE_READ || !uip_newdata()) {
    socket->state = SOCKET_STATE_BLOCKED_NEWDATA;
    PT_RESTART(&socket->socketpt);
  }
  socket->state = SOCKET_STATE_READ;
  PT_EXIT(&socket->socketpt);
}*/
/*---------------------------------------------------------------------------*/
PT_THREAD(socket_send(struct socket *socket))
{
  PT_START(&socket->socketpt);

  if(socket->sendlen == 0) {
    PT_EXIT(&socket->socketpt);
  }
  
  if(uip_newdata()) {
    socket->readptr = (u8_t *)uip_appdata;
    socket->readlen = uip_datalen();
    uipbuf_bufdata(&socket->buf, socket->bufsize,
		   &socket->readptr,
		   &socket->readlen);
  }
  
  if(socket->state == SOCKET_STATE_ACKED || !uip_acked()) {
    if(socket->sendlen > uip_mss()) {   
      uip_send(socket->sendptr, uip_mss());
    } else {
      uip_send(socket->sendptr, socket->sendlen);
    }
    socket->state = SOCKET_STATE_BLOCKED_SEND;
    PT_RESTART(&socket->socketpt);
  } else {
    if(socket->sendlen > uip_mss()) {
      socket->sendlen -= uip_mss();
      socket->sendptr += uip_mss();
      socket->state = SOCKET_STATE_BLOCKED_SEND;
      PT_RESTART(&socket->socketpt);
    }
  }
  socket->state = SOCKET_STATE_ACKED;
  PT_EXIT(&socket->socketpt);
}
/*---------------------------------------------------------------------------*/
PT_THREAD(socket_close(struct socket *socket))
{
  PT_START(&socket->socketpt);
  uip_close();
  socket->state = SOCKET_STATE_BLOCKED_CLOSE;  
  PT_WAIT_UNTIL(&socket->socketpt,
		uip_closed() || uip_timedout() || uip_aborted());
  PT_EXIT(&socket->socketpt);
}
/*---------------------------------------------------------------------------*/
static char
newdata(struct socket *socket)
{
  char cond;
  cond = (socket->state == SOCKET_STATE_READ || !uip_newdata());

  if(cond) {
    socket->state = SOCKET_STATE_BLOCKED_NEWDATA;
  }
  return cond;
}
/*---------------------------------------------------------------------------*/
PT_THREAD(socket_readto(struct socket *socket, unsigned char c))
{

  PT_START(&socket->socketpt);
  
  uipbuf_setup(&socket->buf, socket->bufptr, socket->bufsize);
  
  /* XXX: Must add uipbuf_checkmarker() before do{} loop. */
  
  do {
    if(socket->readlen == 0) {
      PT_WAIT_WHILE(&socket->socketpt, newdata(socket));
      socket->state = SOCKET_STATE_READ;
      socket->readptr = (u8_t *)uip_appdata;
      socket->readlen = uip_datalen();
    }
  } while((uipbuf_bufto(&socket->buf, c,
			&socket->readptr,
			&socket->readlen) & UIPBUF_FOUND) == 0 &&
	  socket->readlen > 0);

  PT_EXIT(&socket->socketpt);
}
/*---------------------------------------------------------------------------*/
void
socket_setup(struct socket *socket, char *buffer, int buffersize)
{
  socket->state = SOCKET_STATE_NONE;
  socket->readlen = 0;
  socket->bufptr = buffer;
  socket->bufsize = buffersize;
  uipbuf_setup(&socket->buf, buffer, buffersize);
  PT_INIT(&socket->pt);
  PT_INIT(&socket->socketpt);
}
/*---------------------------------------------------------------------------*/
void *
socket_alloc(struct memb_blocks *m)
{
  void *s;
  
  s = memb_alloc(m);
  tcp_markconn(uip_conn, s);
  return s;
}
/*---------------------------------------------------------------------------*/
