/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the Contiki operating system.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: psock.h,v 1.1 2005/02/22 22:23:07 adamdunkels Exp $
 */

/**
 * \defgroup psock Protosockets library
 * @{
 *
 * The protosocket library provides an interface to the uIP stack that is
 * similar to the traditional BSD socket interface. Unlike programs
 * written for the ordinary uIP event-driven interface, programs
 * written with the protosocket library are executed in a sequential
 * fashion and does not have to be implemented as explicit state
 * machines.
 *
 * Protosockets only work with TCP connections. 
 *
 * The protosocket library uses \ref pt protothreads to provide
 * sequential control flow. This makes the protosockets lightweight in
 * terms of memory, but also means that protosockets inherits the
 * functional limitations of protothreads. Each protosocket lives only
 * within a single function block. Automatic variables (stack
 * variables) are not necessarily retained across a protosocket
 * library function call.
 *
 * \note Because the protosocket library uses protothreads, local variables
 * will not always be saved across a call to a protosocket library
 * function. It is therefore advised that local variables are used
 * with extreme care.
 *
 * The protosocket library provides functions for sending data without
 * having to deal with retransmissions and acknowledgements, as well
 * as functions for reading data without having to deal with data
 * being split across more than one TCP segment.
 *
 * Because each protosocket runs as a protothread, the protosocket has to be
 * started with a call to PSOCK_BEGIN() at the start of the function
 * in which the protosocket is used. Similarly, the protosocket protothread can
 * be terminated by a call to PSOCK_EXIT().
 *
 * The example code below illustrates how to use the protosocket
 * library. The program implements a simple SMTP client that sends a
 * short email. The program is divided into two functions, one uIP
 * event handler (smtp_uipcall()) and one function that runs the
 * protosocket protothread and performs the SMTP communication
 * (smtp_socketthread()).
 *
 * An SMTP connection is represented by a smtp_state structure
 * containing a struct psock and a small input buffer. The input
 * buffer only needs to be 3 bytes long to accomodate the 3 byte
 * status codes used by SMTP. Connection structures can be allocated
 * from the memory buffer called connections, which is declared with
 * the MEMB() macro.
 *
 * The convenience macro SEND_STRING() is defined in order to simplify
 * the code, as it mostly involves sending strings.
 *  
 * The function smtp_socketthread() is declared as a protothread using
 * the PT_THREAD() macro. The PSOCK_BEGIN() call at the first line of
 * the smtp_socketthread() function starts the protothread. SMTP
 * specifies that the server will start with sending a welcome message
 * that should include the status code 220 if the server is ready to
 * accept messages. Therefore, the smtp_socketthread() first calls
 * PSOCK_READTO() to read all incoming data up to the first
 * newline. If the status code was anything else but 220, the protosocket
 * is closed and the protosocket's protothread is terminated with the call
 * to PSOCK_CLOSE_EXIT().
 *
 * If the connection is accepted by the server, smtp_socketthread()
 * continues with sending the HELO message. If this gets a positive
 * reply (a status code beginning with a 2), the protothread moves on
 * with the rest of the SMTP procedure. Finally, after all headers and
 * data is sent, the program sends a QUIT before it finally closes the
 * protosocket and exits the protosocket's protothread.
 * 
 *
 \code
#include <string.h>

#include "psock.h"
#include "memb.h"

struct smtp_state {  
  struct psock psock;
  char inputbuffer[3];
};

MEMB(connections, sizeof(struct smtp_state), 2);

#define SEND_STRING(s, str) PSOCK_SEND(s, str, strlen(str))

static
PT_THREAD(smtp_socketthread(struct smtp_state *s))
{
  PSOCK_BEGIN(&s->psock);

  PSOCK_READTO(&s->psock, '\n');
   
  if(strncmp(s->inputbuffer, "220", 3) != 0) {
    PSOCK_CLOSE_EXIT(&s->psock);
  }

  SEND_STRING(&s->psock, "HELO contiki.example.com\r\n");

  PSOCK_READTO(&s->psock, '\n');  
  if(s->inputbuffer[0] != '2') {
    PSOCK_CLOSE_EXIT(&s->psock);
  }

  SEND_STRING(&s->psock, "MAIL FROM: contiki@example.com\r\n");

  PSOCK_READTO(&s->psock, '\n');  
  if(s->inputbuffer[0] != '2') {
    PSOCK_CLOSE_EXIT(&s->psock);
  }

  SEND_STRING(&s->psock, "RCPT TO: contiki@example.com\r\n");

  PSOCK_READTO(&s->psock, '\n');  
  if(s->inputbuffer[0] != '2') {
    PSOCK_CLOSE_EXIT(&s->psock);
  }
  
  SEND_STRING(&s->psock, "DATA\r\n");
  
  PSOCK_READTO(&s->psock, '\n');
  if(s->inputbuffer[0] != '3') {
    PSOCK_CLOSE_EXIT(&s->psock);
  }

  SEND_STRING(&s->psock, "To: contiki@example.com\r\n");
  SEND_STRING(&s->psock, "From: contiki@example.com\r\n");
  SEND_STRING(&s->psock, "Subject: Example\r\n");

  SEND_STRING(&s->psock, "A test message from Contiki.\r\n");
  
  SEND_STRING(&s->psock, "\r\n.\r\n");

  PSOCK_READTO(&s->psock, '\n');  
  if(s->inputbuffer[0] != '2') {
    PSOCK_CLOSE_EXIT(&s->psock);
  }

  SEND_STRING(&s->psock, "QUIT\r\n");
  
  PSOCK_END(&s->psock);
}

void
smtp_uipcall(void *state)
{
  struct smtp_state *s = (struct smtp_state *)state;
  
  if(uip_closed() || uip_aborted() || uip_timedout()) {
    memb_free(&connections, s);
  } else if(uip_connected()) {
    PSOCK_INIT(s, s->inputbuffer, sizeof(s->inputbuffer));
  } else {
    smtp_socketthread(s);
  }
}
 \endcode
 *
 */

/**
 * \file
 * Protosocket library header file
 * \author
 * Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __PSOCK_H__
#define __PSOCK_H__

#include "pt.h"
#include "uipbuf.h"
#include "memb.h"

/**
 * The representation of a protosocket.
 *
 * The protosocket structrure is an opaque structure with no user-visible
 * elements.
 */
struct psock {
  struct pt pt, psockpt;
  unsigned char state;
  const u8_t *sendptr;
  u16_t sendlen;
  u8_t *readptr;
  u16_t readlen;

  struct uipbuf_buffer buf;
  char *bufptr;
  unsigned int bufsize;
};

void psock_init(struct psock *psock, char *buffer, unsigned int buffersize);
/**
 * Initialize a protosocket.
 *
 * This macro initializes a protosocket and must be called before the
 * protosocket is used. The initialization also specifies the input buffer
 * for the protosocket.
 *
 * \param psock (struct psock *) A pointer to the protosocket to be
 * initialized
 *
 * \param buffer (char *) A pointer to the input buffer for the
 * protosocket.
 *
 * \param buffersize (unsigned int) The size of the input buffer.
 *
 * \hideinitializer
 */
#define PSOCK_INIT(psock, buffer, buffersize) \
  psock_init(psock, buffer, buffersize)

/**
 * Start the protosocket protothread in a function.
 *
 * This macro starts the protothread associated with the protosocket and
 * must come before other protosocket calls in the function it is used.
 *
 * \param psock (struct psock *) A pointer to the protosocket to be
 * started.
 *
 * \hideinitializer
 */
#define PSOCK_BEGIN(psock) PT_BEGIN(&((psock)->pt))

PT_THREAD(psock_send(struct psock *psock, const char *buf, unsigned int len));
/**
 * Send data.
 *
 * This macro sends data over a protosocket. The protosocket protothread blocks
 * until all data has been sent and is known to have been received by
 * the remote end of the TCP connection.
 *
 * \param psock (struct psock *) A pointer to the protosocket over which
 * data is to be sent.
 *
 * \param data (char *) A pointer to the data that is to be sent.
 *
 * \param datalen (unsigned int) The length of the data that is to be
 * sent.
 *
 * \hideinitializer
 */
#define PSOCK_SEND(psock, data, datalen)		\
    PT_WAIT_THREAD(&((psock)->pt), psock_send(psock, data, datalen))

PT_THREAD(psock_generator_send(struct psock *psock,
				unsigned short (*f)(void *), void *arg));

#define PSOCK_GENERATOR_SEND(psock, generator, arg)     \
    PT_WAIT_THREAD(&((psock)->pt),					\
		   psock_generator_send(psock, generator, arg))


/*PT_THREAD(psock_closew(struct psock *psock));
#define PSOCK_CLOSEW(psock)				\
  PT_WAIT_THREAD(&(psock)->pt, psock_closew(psock))
*/

/**
 * Close a protosocket.
 *
 * This macro closes a protosocket and can only be called from within the
 * protothread in which the protosocket lives.
 *
 * \param psock (struct psock *) A pointer to the protosocket that is to
 * be closed.
 *
 * \hideinitializer
 */
#define PSOCK_CLOSE(psock) uip_close()

PT_THREAD(psock_readto(struct psock *psock, unsigned char c));

/**
 * Read data up to a specified character.
 *
 * This macro will block waiting for data and read the data into the
 * input buffer specified with the call to PSOCK_INIT(). Data is only
 * read until the specifieed character appears in the data stream.
 *
 * \param psock (struct psock *) A pointer to the protosocket from which
 * data should be read.
 *
 * \param c (char) The character at which to stop reading.
 *
 * \hideinitializer
 */
#define PSOCK_READTO(psock, c)				\
  PT_WAIT_THREAD(&((psock)->pt), psock_readto(psock, c))

/**
 * The length of the data that was previously read.
 *
 * This macro returns the length of the data that was previously read
 * using PSOCK_READTO() or PSOCK_READ().
 *
 * \param psock (struct psock *) A pointer to the protosocket holding the data.
 *
 * \hideinitializer
 */
#define PSOCK_DATALEN(psock) uipbuf_len(&(psock)->buf)

/**
 * Exit the protosocket's protothread.
 *
 * This macro terminates the protothread of the protosocket and should
 * almost always be used in conjunction with PSOCK_CLOSE().
 *
 * \sa PSOCK_CLOSE_EXIT()
 *
 * \param psock (struct psock *) A pointer to the protosocket.
 *
 * \hideinitializer
 */
#define PSOCK_EXIT(psock) PT_EXIT(&((psock)->pt))

/**
 * Close a protosocket and exit the protosocket's protothread.
 *
 * This macro closes a protosocket and exits the protosocket's protothread.
 *
 * \param psock (struct psock *) A pointer to the protosocket.
 *
 * \hideinitializer
 */
#define PSOCK_CLOSE_EXIT(psock)		\
  do {						\
    PSOCK_CLOSE(psock);			\
    PSOCK_EXIT(psock);			\
  } while(0)

#define PSOCK_END(psock) PT_END(&((psock)->pt))

char psock_newdata(struct psock *s);

/**
 * Check if new data has arrived on a protosocket.
 *
 * This macro is used in conjunction with the PSOCK_WAIT_UNTIL()
 * macro to check if data has arrived on a protosocket.
 *
 * \param psock (struct psock *) A pointer to the protosocket.
 *
 * \hideinitializer
 */
#define PSOCK_NEWDATA(psock) psock_newdata(psock)

/**
 * Wait until a condition is true.
 *
 * This macro blocks the protothread until the specified condition is
 * true. The macro PSOCK_NEWDATA() can be used to check if new data
 * arrives when the protosocket is waiting.
 *
 * Typically, this macro is used as follows:
 *
 \code
 PT_THREAD(thread(struct psock *s, struct timer *t))
 {
   PSOCK_BEGIN(s);

   PSOCK_WAIT_UNTIL(s, PSOCK_NEWADATA(s) || timer_expired(t));
   
   if(PSOCK_NEWDATA(s)) {
     PSOCK_READTO(s, '\n');
   } else {
     handle_timed_out(s);
   }
   
   PSOCK_END(s);
 }
 \endcode 
 *
 * \param psock (struct psock *) A pointer to the protosocket.
 * \param condition The condition to wait for.
 *
 * \hideinitializer
 */
#define PSOCK_WAIT_UNTIL(psock, condition)    \
  PT_WAIT_UNTIL(&((psock)->pt), (condition));

#define PSOCK_WAIT_THREAD(psock, condition)   \
  PT_WAIT_THREAD(&((psock)->pt), (condition))

#endif /* __PSOCK_H__ */
