/*
 * Copyright (c) 2001-2002, Adam Dunkels.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Adam Dunkels.
 * 4. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: uip.h,v 1.6 2003/08/21 22:26:57 adamdunkels Exp $
 *
 */

#ifndef __UIP_H__
#define __UIP_H__

#include "uipopt.h"

#ifndef UIP_IPV6
#define UIP_IPV6 0
#endif

/*-----------------------------------------------------------------------------------*/
/* First, the functions that should be called from the
 * system. Initialization, the periodic timer and incoming packets are
 * handled by the following three functions.
 */

/* uip_init(void):
 *
 * Must be called at boot up to configure the uIP data structures.
 */
void uip_init(void);

/* uip_periodic(conn):
 *
 * Should be called when the periodic timer has fired. Should be
 * called once per connection (0 - UIP_CONNS).
 */
#define uip_periodic(conn) do { uip_conn = &uip_conns[conn]; \
                                uip_process(UIP_TIMER); } while (0)

/* uip_periodic_conn(conn):
 *
 */
#define uip_periodic_conn(conn) do { uip_conn = conn; \
                                     uip_process(UIP_TIMER); } while (0)

/* uip_input(void):
 *
 * Is called when the network device driver has received new data.
 */
#define uip_input()        uip_process(UIP_DATA)

/* uip_sethostaddr(addr):
 *
 * Is used to set the IP address.
 */
#define uip_sethostaddr(addr) do { uip_hostaddr[0] = addr[0]; \
                              uip_hostaddr[1] = addr[1]; } while(0)

/* uip_gethostaddr(&addr):
 *
 * Obtains the IP address.
 */
#define uip_gethostaddr(addr) do { addr[0] = uip_hostaddr[0]; \
                              addr[1] = uip_hostaddr[1]; } while(0)

#if UIP_UDP
/* uip_udp_periodic(conn):
 */
#define uip_udp_periodic(conn) do { uip_udp_conn = &uip_udp_conns[conn]; \
                                uip_process(UIP_UDP_TIMER); } while (0)

/* uip_udp_periodic(conn):
 */
#define uip_udp_periodic_conn(conn) do { uip_udp_conn = conn; \
                                         uip_process(UIP_UDP_TIMER); } while (0)
#endif /* UIP_UDP */
/*-----------------------------------------------------------------------------------*/
/* Functions that are used by the uIP application program. Opening and
 * closing connections, sending and receiving data, etc. is all
 * handled by the functions below.
*/

/* uip_listen(port):
 *
 * Starts listening to the specified port.
 */
void uip_listen(u16_t port);

/* uip_unlisten(port):
 *
 * Stops listening to the specified port.
 */
void uip_unlisten(u16_t port);

/* uip_connect(ripaddr, port):
 *
 * Returns a connection identifier that connects to a port on the
 * specified host (given in ripaddr). If no connections are avaliable,
 * the function returns NULL. This function is avaliable only if
 * support for active open has been configured (#define
 * UIP_ACTIVE_OPEN 1 in uipopt.h)
 */
struct uip_conn *uip_connect(u16_t *ripaddr, u16_t port);

#if UIP_UDP
/* uip_udp_new(ripaddr, rport):
 *
 * Sets up a new UDP "connection" with the specified parameters.
 */
struct uip_udp_conn *uip_udp_new(u16_t *ripaddr, u16_t rport);

/* uip_udp_remove(conn):
 *
 * Removes the UDP "connection".
 */
#define uip_udp_remove(conn) (conn)->lport = 0

/* uip_udp_send(len):
 *
 * Sends a UDP datagram of length len. The data must be present in the
 * uip_buf buffer (pointed to by uip_appdata).
 */
#define uip_udp_send(len) uip_slen = (len)
#endif /* UIP_UDP */


/* uip_outstanding(conn):
 *
 * Checks whether a connection has outstanding (i.e., unacknowledged)
 * data.
 */
#define uip_outstanding(conn) ((conn)->len)

/* uip_send(data, len):
 *
 * Send data on the current connection. The length of the data must
 * not exceed the maxium segment size (MSS) for the connection.
 */
#define uip_send(data, len) do { uip_appdata = (data); uip_slen = (len);} while(0)   

/* uip_datalen():
 *
 * The length of the data that is currently avaliable (if avaliable)
 * in the uip_appdata buffer. The test function uip_data() is
 * used to check if data is avaliable.
 */
#define uip_datalen()       uip_len

#define uip_urgdatalen()    uip_urglen

/* uip_close():
 *
 * Close the current connection.
 */
#define uip_close()         (uip_flags = UIP_CLOSE)

/* uip_abort():
 *
 * Abort the current connection.
 */
#define uip_abort()         (uip_flags = UIP_ABORT)

/* uip_stop():
 *
 * Close our receiver's window so that we stop receiving data for the
 * current connection.
 */
#define uip_stop()          (uip_conn->tcpstateflags |= UIP_STOPPED)

/* uip_stopped():
 *
 * Find out if the current connection has been previously stopped.
 */
#define uip_stopped(conn)   ((conn)->tcpstateflags & UIP_STOPPED)

/* uip_restart():
 *
 * Open the window again so that we start receiving data for the
 * current connection.
 */
#define uip_restart()         do { uip_flags |= UIP_NEWDATA; \
                                   uip_conn->tcpstateflags &= ~UIP_STOPPED; \
                              } while(0)


/* uIP tests that can be made to determine in what state the current
   connection is, and what the application function should do. */

/* uip_newdata():
 *
 * Will reduce to non-zero if there is new data for the application
 * present at the uip_appdata pointer. The size of the data is
 * avaliable through the uip_len variable.
 */
#define uip_newdata()   (uip_flags & UIP_NEWDATA)

/* uip_acked():
 *
 * Will reduce to non-zero if the previously sent data has been
 * acknowledged by the remote host. This means that the application
 * can send new data. uip_reset_acked() can be used to reset the acked
 * flag.
 */
#define uip_acked()   (uip_flags & UIP_ACKDATA)
#define uip_reset_acked() (uip_flags &= ~UIP_ACKDATA)

/* uip_connected():
 *
 * Reduces to non-zero if the current connection has been connected to
 * a remote host. This will happen both if the connection has been
 * actively opened (with uip_connect()) or passively opened (with
 * uip_listen()).
 */
#define uip_connected() (uip_flags & UIP_CONNECTED)

/* uip_closed():
 *
 * Is non-zero if the connection has been closed by the remote
 * host. The application may do the necessary clean-ups.
 */
#define uip_closed()    (uip_flags & UIP_CLOSE)

/* uip_aborted():
 *
 * Non-zero if the current connection has been aborted (reset) by the
 * remote host.
 */
#define uip_aborted()    (uip_flags & UIP_ABORT)

/* uip_timedout():
 *
 * Non-zero if the current connection has been aborted due to too many
 * retransmissions.
 */
#define uip_timedout()    (uip_flags & UIP_TIMEDOUT)

/* uip_rexmit():
 *
 * Reduces to non-zero if the previously sent data has been lost in
 * the network, and the application should retransmit it. The
 * application should set the uip_appdata buffer and the uip_len
 * variable just as it did the last time this data was to be
 * transmitted.
 */
#define uip_rexmit()     (uip_flags & UIP_REXMIT)

/* uip_poll():
 *
 * Is non-zero if the reason the application is invoked is that the
 * current connection has been idle for a while and should be
 * polled.
 */ 
#define uip_poll()       (uip_flags & UIP_POLL)

/* uip_mss():
 *
 * Gives the current maxium segment size (MSS) of the current
 * connection.
 */
#define uip_mss()             (uip_conn->mss)


/* uIP convenience and converting functions. */

/* uip_ipaddr(&ipaddr, addr0,addr1,addr2,addr3):
 *
 * Packs an IP address into a two element 16-bit array. Such arrays
 * are used to represent IP addresses in uIP.
 */
#define uip_ipaddr(addr, addr0,addr1,addr2,addr3) do { \
                     (addr)[0] = HTONS(((addr0) << 8) | (addr1)); \
                     (addr)[1] = HTONS(((addr2) << 8) | (addr3)); \
                  } while(0)

/* HTONS():
 *
 * Macros for converting 16-bit quantities between host and network
 * byte order.
 */
#ifndef HTONS
#   if BYTE_ORDER == BIG_ENDIAN
#      define HTONS(n) (n)
#   else /* BYTE_ORDER == BIG_ENDIAN */
#      define HTONS(n) ((((u16_t)((n) & 0xff)) << 8) | (((n) & 0xff00) >> 8))
#   endif /* BYTE_ORDER == BIG_ENDIAN */
#endif /* HTONS */

#ifndef htons
u16_t htons(u16_t val);
#endif /* htons */

/*-----------------------------------------------------------------------------------*/
/* The following global variables are used for passing parameters
 * between uIP, the network device driver and the application. */
/*-----------------------------------------------------------------------------------*/

/* u8_t uip_buf[UIP_BUFSIZE]:
 *
 * The uip_buf array is used to hold incoming and outgoing
 * packets. The device driver fills this with incoming packets.
 */
extern u8_t uip_buf[UIP_BUFSIZE+2];

/* u8_t *uip_appdata:
 *
 * This pointer points to the application data when the application is
 * called. If the application wishes to send data, this is where the
 * application should write it. The application can also point this to
 * another location.
 */
extern volatile u8_t *uip_appdata; 

#if UIP_URGDATA > 0 
/* u8_t *uip_urgdata:
 *
 * This pointer points to any urgent data that has been received. Only
 * present if compiled with support for urgent data (UIP_URGDATA).
 */
extern volatile u8_t *uip_urgdata; 
#endif /* UIP_URGDATA > 0 */


/* u[8|16]_t uip_len:
 *
 * When the application is called, uip_len contains the length of any
 * new data that has been received from the remote host. The
 * application should set this variable to the size of any data that
 * the application wishes to send. When the network device driver
 * output function is called, uip_len should contain the length of the
 * outgoing packet.
 */
#if UIP_BUFSIZE > 255
extern volatile u16_t uip_len, uip_slen;
#else
extern volatile u8_t uip_len, uip_slen;
#endif 

#if UIP_URGDATA > 0 
extern volatile u8_t uip_urglen, uip_surglen;
#endif /* UIP_URGDATA > 0 */

extern volatile u8_t uip_acc32[4];

/* struct uip_conn:
 *
 * The uip_conn structure is used for identifying a connection. All
 * but one field in the structure are to be considered read-only by an
 * application. The only exception is the appstate field whos purpose
 * is to let the application store application-specific state (e.g.,
 * file pointers) for the connection. The size of this field is
 * configured in the "uipopt.h" header file.
 */
struct uip_conn {
#if UIP_IPV6
  u16_t ripaddr[8];   /* The IP address of the remote peer. */
#else /* UIP_IPV6 */
  u16_t ripaddr[2];   /* The IP address of the remote peer. */
#endif /* UIP_IPV6 */
  
  u16_t lport, rport; /* The local and the remote port. */
  
  u8_t rcv_nxt[4];    /* The sequence number that we expect to receive
			 next. */
  u8_t snd_nxt[4];    /* The sequence number that was last sent by
                         us. */
#if UIP_TCP_MSS > 255
  u16_t len;
  u16_t mss;          /* Maximum segment size for the connection. */
#else
  u8_t len;
  u8_t mss;
#endif /* UIP_TCP_MSS */
  u8_t sa, sv, rto;
  u8_t tcpstateflags; /* TCP state and flags. */
  u8_t timer;         /* The retransmission timer. */
  u8_t nrtx;          /* Counts the number of retransmissions for a
                         particular segment. */
  
  u8_t appstate[UIP_APPSTATE_SIZE];
};

/* struct uip_conn *uip_conn:
 *
 * When the application is called, uip_conn will point to the current
 * conntection, the one that should be processed by the
 * application. The uip_conns[] array is a list containing all
 * connections.
 */
extern struct uip_conn *uip_conn;  
extern struct uip_conn uip_conns[UIP_CONNS];

#if UIP_UDP
/* struct uip_udp_conn:
 *
 * The uip_udp_conn structure is used for identifying UDP
 * "connections".
 */
struct uip_udp_conn {
#if UIP_IPV6
  u16_t ripaddr[8];   /* The IP address of the remote peer. */
#else /* UIP_IPV6 */
  u16_t ripaddr[2];   /* The IP address of the remote peer. */
#endif /* UIP_IPV6 */
  u16_t lport, rport;
};

extern struct uip_udp_conn *uip_udp_conn;
extern struct uip_udp_conn uip_udp_conns[UIP_UDP_CONNS];
#endif /* UIP_UDP */

/* struct uip_stats:
 *
 * Contains statistics about the TCP/IP stack.
 */
struct uip_stats {
  struct {
    uip_stats_t drop;
    uip_stats_t recv;
    uip_stats_t sent;
    uip_stats_t vhlerr;   /* Number of packets dropped due to wrong IP version
			     or header length. */
    uip_stats_t hblenerr; /* Number of packets dropped due to wrong IP length,
			     high byte. */
    uip_stats_t lblenerr; /* Number of packets dropped due to wrong IP length,
			     low byte. */
    uip_stats_t fragerr;  /* Number of packets dropped since they were IP
			     fragments. */
    uip_stats_t chkerr;   /* Number of packets dropped due to IP checksum errors. */
    uip_stats_t protoerr; /* Number of packets dropped since they were neither
			     ICMP nor TCP. */
  } ip;
  struct {
    uip_stats_t drop;
    uip_stats_t recv;
    uip_stats_t sent;
    uip_stats_t typeerr;
  } icmp;
  struct {
    uip_stats_t drop;
    uip_stats_t recv;
    uip_stats_t sent;
    uip_stats_t chkerr;
    uip_stats_t ackerr;
    uip_stats_t rst;
    uip_stats_t rexmit;
    uip_stats_t syndrop;  /* Number of dropped SYNs due to too few
			     connections was avaliable. */
    uip_stats_t synrst;   /* Number of SYNs for closed ports, triggering a
			     RST. */
  } tcp;
};

extern struct uip_stats uip_stat;


/*-----------------------------------------------------------------------------------*/
/* All the stuff below this point is internal to uIP and should not be
 * used directly by an application or by a device driver.
 */
/*-----------------------------------------------------------------------------------*/
/* u8_t uip_flags:
 *
 * When the application is called, uip_flags will contain the flags
 * that are defined in this file. Please read below for more
 * infomation.
 */
extern volatile u8_t uip_flags;

/* The following flags may be set in the global variable uip_flags
   before calling the application callback. The UIP_ACKDATA and
   UIP_NEWDATA flags may both be set at the same time, whereas the
   others are mutualy exclusive. Note that these flags should *NOT* be
   accessed directly, but through the uIP functions/macros. */

#define UIP_ACKDATA   1     /* Signifies that the outstanding data was
			       acked and the application should send
			       out new data instead of retransmitting
			       the last data. */
#define UIP_NEWDATA   2     /* Flags the fact that the peer has sent
			       us new data. */
#define UIP_REXMIT    4     /* Tells the application to retransmit the
			       data that was last sent. */
#define UIP_POLL      8     /* Used for polling the application, to
			       check if the application has data that
			       it wants to send. */
#define UIP_CLOSE     16    /* The remote host has closed the
			       connection, thus the connection has
			       gone away. Or the application signals
			       that it wants to close the
			       connection. */
#define UIP_ABORT     32    /* The remote host has aborted the
			       connection, thus the connection has
			       gone away. Or the application signals
			       that it wants to abort the
			       connection. */
#define UIP_CONNECTED 64    /* We have got a connection from a remote
                               host and have set up a new connection
                               for it, or an active connection has
                               been successfully established. */

#define UIP_TIMEDOUT  128   /* The connection has been aborted due to
			       too many retransmissions. */


/* uip_process(flag):
 *
 * The actual uIP function which does all the work.
 */
void uip_process(u8_t flag);

/* The following flags are passed as an argument to the uip_process()
   function. They are used to distinguish between the two cases where
   uip_process() is called. It can be called either because we have
   incoming data that should be processed, or because the periodic
   timer has fired. */

#define UIP_DATA    1     /* Tells uIP that there is incoming data in
                             the uip_buf buffer. The length of the
                             data is stored in the global variable
                             uip_len. */
#define UIP_TIMER   2     /* Tells uIP that the periodic timer has
                             fired. */
#if UIP_UDP
#define UIP_UDP_TIMER 3
#endif /* UIP_UDP */

/* The TCP states used in the uip_conn->tcpstateflags. */
#define CLOSED      0
#define SYN_RCVD    1
#define SYN_SENT    2
#define ESTABLISHED 3
#define FIN_WAIT_1  4
#define FIN_WAIT_2  5
#define CLOSING     6
#define TIME_WAIT   7
#define LAST_ACK    8
#define TS_MASK     15
  
#define UIP_STOPPED      16

#if UIP_IPV6
#define UIP_TCPIP_HLEN 60
#else /* UIP_IPV6 */
#define UIP_TCPIP_HLEN 40
#endif /* UIP_IPV6 */

/* The TCP and IP headers. */
typedef struct {
  /* IP header. */
#if UIP_IPV6
  u8_t vtc,
    tcfl;
  u16_t fl;
  u8_t len[2];
  u8_t nxthdr, hoplim;
  u16_t srcipaddr[8],
    destipaddr[8];
#else /* UIP_IPV6 */
  u8_t vhl,
    tos,          
    len[2],       
    ipid[2],        
    ipoffset[2],  
    ttl,          
    proto;     
  u16_t ipchksum;
  u16_t srcipaddr[2], 
    destipaddr[2];
#endif /* UIP_IPV6 */
  
  /* TCP header. */
  u16_t srcport,
    destport;
  u8_t seqno[4],  
    ackno[4],
    tcpoffset,
    flags,
    wnd[2];     
  u16_t tcpchksum;
  u8_t urgp[2];
  u8_t optdata[4];
} uip_tcpip_hdr;

/* The ICMP and IP headers. */
typedef struct {
  /* IP header. */
#if UIP_IPV6
  u16_t vtcfl;
  u16_t fl;
  u8_t len[2];
  u8_t nxthdr, hoplim;
  u16_t srcipaddr[8],
    destipaddr[8];
#else /* UIP_IPV6 */
  u8_t vhl,
    tos,          
    len[2],       
    ipid[2],        
    ipoffset[2],  
    ttl,          
    proto;     
  u16_t ipchksum;
  u16_t srcipaddr[2], 
    destipaddr[2];
#endif /* UIP_IPV6 */
  /* ICMP (echo) header. */
  u8_t type, icode;
  u16_t icmpchksum;
  u16_t id, seqno;  
} uip_icmpip_hdr;


/* The UDP and IP headers. */
typedef struct {
  /* IP header. */
#if UIP_IPV6
  u8_t vtc,
    tcfl;
  u16_t fl;
  u8_t len[2];
  u8_t nxthdr, hoplim;
  u16_t srcipaddr[8],
    destipaddr[8];
#else /* UIP_IPV6 */
  u8_t vhl,
    tos,          
    len[2],       
    ipid[2],        
    ipoffset[2],  
    ttl,          
    proto;     
  u16_t ipchksum;
  u16_t srcipaddr[2], 
    destipaddr[2];
#endif /* UIP_IPV6 */
  
  /* UDP header. */
  u16_t srcport,
    destport;
  u16_t udplen;
  u16_t udpchksum;
} uip_udpip_hdr;

#define UIP_PROTO_ICMP  1
#define UIP_PROTO_TCP   6
#define UIP_PROTO_UDP   17

#if UIP_FIXEDADDR
#if UIP_IPV6
extern const u16_t uip_hostaddr[8];
#else /* UIP_IPV6 */
extern const u16_t uip_hostaddr[2];
#endif /* UIP_IPV6 */
#else /* UIP_FIXEDADDR */
#if UIP_IPV6
extern u16_t uip_hostaddr[8];
#else /* UIP_IPV6 */
extern u16_t uip_hostaddr[2];
#endif /* UIP_IPV6 */
#endif /* UIP_FIXEDADDR */

#endif /* __UIP_H__ */







