#include "socket.h"

/*---------------------------------------------------------------------------*/
char
socket_wait_data(struct socket *socket)
{
  if(socket->state == SOCKET_STATE_READ || !uip_newdata()) {
    socket->state = SOCKET_STATE_BLOCKED_NEWDATA;
    return 1;
  }
  socket->state = SOCKET_STATE_READ;
  return 0;
}
/*---------------------------------------------------------------------------*/
char
socket_send(struct socket *socket)
{
  if(uip_newdata()) {
    socket->readptr = (u8_t *)uip_appdata;
    socket->readlen = uip_datalen();
    uipbuf_bufdata(&socket->buf, socket->bufsize,
		   &socket->readptr,
		   &socket->readlen);
  }
  
  if(socket->tmplen == 0) {
    return 0;
  }
  if(socket->state == SOCKET_STATE_ACKED || !uip_acked()) {
    if(socket->tmplen > uip_mss()) {   
      uip_send(socket->tmpptr, uip_mss());
    } else {
      uip_send(socket->tmpptr, socket->tmplen);
    }
    socket->state = SOCKET_STATE_BLOCKED_SEND;
    return 1;
  } else {
    if(socket->tmplen > uip_mss()) {
      socket->tmplen -= uip_mss();
      socket->tmpptr += uip_mss();
      socket->state = SOCKET_STATE_BLOCKED_SEND;
      return 1;
    }
  }
  socket->state = SOCKET_STATE_ACKED;
  return 0;
}
/*---------------------------------------------------------------------------*/
char
socket_closed(struct socket *socket)
{
  return !(uip_closed() || uip_timedout() || uip_aborted());
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
