/**
 * \defgroup socket Socket library
 * @{
 *
 * The socket library provides an interface to the uIP stack that is
 * similar to the traditional BSD socket interface. Unlike programs
 * written for the ordinary uIP event-driven interface, programs
 * written with the socket library are executed in a sequential
 * fashion and does not have to be implemented as explicit state
 * machines.
 *
 * Sockets only work with TCP connections. 
 *
 * The socket library uses protothreads to provide sequential control
 * flow. This makes the sockets lightweight in terms of memory, but
 * also means that sockets inherits the functional limitations of
 * protothreads. Each socket lives only within a single function
 * block. Automatic variables (stack variables) are not necessarily
 * retained across a socket library function call.
 *
 * The socket library provides functions for sending data without
 * having to deal with retransmissions and acknowledgements, as well
 * as functions for reading data without having to deal with data
 * being split across more than one TCP segment.
 *
 * Because each socket runs as a protothread, the socket has to be
 * started with a call to SOCKET_START() at the start of the function
 * in which the socket is used. Similarly, the socket protothread can
 * be terminated by a call to SOCKET_EXIT().
 *
 * The example code below illustrates how to use the socket
 * library. The program implements a simple SMTP client that sends a
 * short email. The program is divided into two functions, one uIP
 * event handler (smtp_uipcall()) and one function that runs the
 * socket protothread and performs the SMTP communication
 * (smtp_socketthread()).
 *
 * An SMTP connection is represented by a smtp_state structure
 * containing a struct socket and a small input buffer. The input
 * buffer only needs to be 3 bytes long to accomodate the 3 byte
 * status codes used by SMTP. Connection structures can be allocated
 * from the memory buffer called connections, which is declared with
 * the MEMB() macro.
 *
 * The convenience macro SEND_STRING() is defined in order to simplify
 * the code, as it mostly involves sending strings.
 *  
 * The function smtp_socketthread() is declared as a protothread using
 * the PT_THREAD() macro. The SOCKET_START() call at the first line of
 * the smtp_socketthread() function starts the protothread. SMTP
 * specifies that the server will start with sending a welcome message
 * that should include the status code 220 if the server is ready to
 * accept messages. Therefore, the smtp_socketthread() first calls
 * SOCKET_READTO() to read all incoming data up to the first
 * newline. If the status code was anything else but 220, the socket
 * is closed and the socket's protothread is terminated with the call
 * to SOCKET_CLOSE_EXIT().
 *
 * If the connection is accepted by the server, smtp_socketthread()
 * continues with sending the HELO message. If this gets a positive
 * reply (a status code beginning with a 2), the protothread moves on
 * with the rest of the SMTP procedure. Finally, after all headers and
 * data is sent, the program sends a QUIT before it finally closes the
 * socket and exits the socket's protothread.
 * 
 *
 \code
#include <string.h>

#include "socket.h"
#include "memb.h"

struct smtp_state {  
  struct socket socket;
  char inputbuffer[3];
};

MEMB(connections, sizeof(struct smtp_state), 2);

#define SEND_STRING(s, str) SOCKET_SEND(s, str, strlen(str))

static
PT_THREAD(smtp_socketthread(struct smtp_state *s))
{
  SOCKET_START(&s->socket);

  SOCKET_READTO(&s->socket, '\n');
   
  if(strncmp(s->inputbuffer, "220", 3) != 0) {
    SOCKET_CLOSE_EXIT(&s->socket);
  }

  SEND_STRING(&s->socket, "HELO contiki.example.com\r\n");

  SOCKET_READTO(&s->socket, '\n');  
  if(s->inputbuffer[0] != '2') {
    SOCKET_CLOSE_EXIT(&s->socket);
  }

  SEND_STRING(&s->socket, "MAIL FROM: contiki@example.com\r\n");

  SOCKET_READTO(&s->socket, '\n');  
  if(s->inputbuffer[0] != '2') {
    SOCKET_CLOSE_EXIT(&s->socket);
  }

  SEND_STRING(&s->socket, "RCPT TO: contiki@example.com\r\n");

  SOCKET_READTO(&s->socket, '\n');  
  if(s->inputbuffer[0] != '2') {
    SOCKET_CLOSE_EXIT(&s->socket);
  }
  
  SEND_STRING(&s->socket, "DATA\r\n");
  
  SOCKET_READTO(&s->socket, '\n');
  if(s->inputbuffer[0] != '3') {
    SOCKET_CLOSE_EXIT(&s->socket);
  }

  SEND_STRING(&s->socket, "To: contiki@example.com\r\n");
  SEND_STRING(&s->socket, "From: contiki@example.com\r\n");
  SEND_STRING(&s->socket, "Subject: Example\r\n");

  SEND_STRING(&s->socket, "A test message from Contiki.\r\n");
  
  SEND_STRING(&s->socket, "\r\n.\r\n");

  SOCKET_READTO(&s->socket, '\n');  
  if(s->inputbuffer[0] != '2') {
    SOCKET_CLOSE_EXIT(&s->socket);
  }

  SEND_STRING(&s->socket, "QUIT\r\n");
  
  SOCKET_CLOSE_EXIT(&s->socket);
}

void
smtp_uipcall(void *state)
{
  struct smtp_state *s = (struct smtp_state *)state;
  
  if(uip_closed() || uip_aborted() || uip_timedout()) {
    memb_free(&connections, s);
  } else if(uip_connected()) {
    SOCKET_INIT(s, s->inputbuffer, sizeof(s->inputbuffer));
  } else {
    smtp_socketthread(s);
  }
}
 \endcode
 *
 */

/**
 * \file
 * Socket library header file
 * \author
 * Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "pt.h"
#include "uipbuf.h"
#include "memb.h"

/**
 * The representation of a socket.
 *
 * The socket structrure is an opaque structure with no user-visible
 * elements.
 */
struct socket {
  struct pt pt, socketpt;
  unsigned char state;
  u8_t *sendptr;
  u16_t sendlen;
  u8_t *readptr;
  u16_t readlen;

  struct uipbuf_buffer buf;
  char *bufptr;
  unsigned int bufsize;
};

void socket_init(struct socket *socket, char *buffer, unsigned int buffersize);
/**
 * Initialize a socket.
 *
 * This macro initializes a socket and must be called before the
 * socket is used. The initialization also specifies the input buffer
 * for the socket.
 *
 * \param socket (struct socket *) A pointer to the socket to be
 * initialized
 *
 * \param buffer (char *) A pointer to the input buffer for the
 * socket.
 *
 * \param buffersize (unsigned int) The size of the input buffer.
 *
 * \hideinitializer
 */
#define SOCKET_INIT(socket, buffer, buffersize) \
  socket_init(socket, buffer, buffersize)

/**
 * Start the socket protothread in a function.
 *
 * This macro starts the protothread associated with the socket and
 * must come before other socket calls in the function it is used.
 *
 * \param socket (struct socket *) A pointer to the socket to be
 * started.
 *
 * \hideinitializer
 */
#define SOCKET_START(socket) PT_START(&(socket)->pt)

PT_THREAD(socket_send(struct socket *socket, char *buf, unsigned int len));
/**
 * Send data.
 *
 * This macro sends data over a socket. The socket protothread blocks
 * until all data has been sent and is known to have been received by
 * the remote end of the TCP connection.
 *
 * \param socket (struct socket *) A pointer to the socket over which
 * data is to be sent.
 *
 * \param data (char *) A pointer to the data that is to be sent.
 *
 * \param datalen (unsigned int) The length of the data that is to be
 * sent.
 *
 * \hideinitializer
 */
#define SOCKET_SEND(socket, data, datalen)		\
    PT_WAIT_THREAD(&(socket)->pt, socket_send(socket, data, datalen));	

/*
#define SOCKET_SEND_DYNAMIC(socket, dataptr, function_call)
*/

/*PT_THREAD(socket_closew(struct socket *socket));
#define SOCKET_CLOSEW(socket)				\
  PT_WAIT_THREAD(&(socket)->pt, socket_closew(socket))
*/

/**
 * Close a socket.
 *
 * This macro closes a socket and can only be called from within the
 * protothread in which the socket lives.
 *
 * \param socket (struct socket *) A pointer to the socket that is to
 * be closed.
 *
 * \hideinitializer
 */
#define SOCKET_CLOSE(socket) uip_close()

PT_THREAD(socket_readto(struct socket *socket, unsigned char c));

/**
 * Read data up to a specified character.
 *
 * This macro will block waiting for data and read the data into the
 * input buffer specified with the call to SOCKET_INIT(). Data is only
 * read until the specifieed character appears in the data stream.
 *
 * \param socket (struct socket *) A pointer to the socket from which
 * data should be read.
 *
 * \param c (char) The character at which to stop reading.
 *
 * \hideinitializer
 */
#define SOCKET_READTO(socket, c)				\
  PT_WAIT_THREAD(&(socket)->pt, socket_readto(socket, c))

/**
 * The length of the data that was previously read.
 *
 * This macro returns the length of the data that was previously read
 * using SOCKET_READTO() or SOCKET_READ().
 *
 * \param socket (struct socket *) A pointer to the socket holding the data.
 *
 * \hideinitializer
 */
#define SOCKET_READLEN(socket) uipbuf_len(&(socket)->buf)

/**
 * Exit the socket's protothread.
 *
 * This macro terminates the protothread of the socket and should
 * almost always be used in conjunction with SOCKET_CLOSE().
 *
 * \sa SOCKET_CLOSE_EXIT()
 *
 * \param socket (struct socket *) A pointer to the socket.
 *
 * \hideinitializer
 */
#define SOCKET_EXIT(socket) PT_EXIT(&(socket)->pt)

/**
 * Close a socket and exit the socket's protothread.
 *
 * This macro closes a socket and exits the socket's protothread.
 *
 * \param socket (struct socket *) A pointer to the socket.
 *
 * \hideinitializer
 */
#define SOCKET_CLOSE_EXIT(socket)		\
  do {						\
    SOCKET_CLOSE(socket);			\
    SOCKET_EXIT(socket);			\
  } while(0)

#endif /* __SOCKET_H__ */
