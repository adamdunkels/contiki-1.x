#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>

/* from mstcpip.h */
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)

#define htons
#include "uip.h"
#include "tcpip.h"
#include "ctk.h"
#undef htons

#define BUF ((uip_tcpip_hdr *)uip_buf)

static SOCKET rawsock = INVALID_SOCKET;

/*-----------------------------------------------------------------------------------*/
static void
error_exit(char *message, int value)
{
  console_exit();
  cprintf(message, value);
  exit(EXIT_FAILURE);
}
/*-----------------------------------------------------------------------------------*/
void
rawsock_init(void)
{
  WSADATA wsadata;
  struct sockaddr_in addr;
  unsigned long on = 1;

  if(WSAStartup(2, &wsadata) != 0) {
    error_exit("WSAStartup() error\n", 0);
  }

  rawsock = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
  if(rawsock == INVALID_SOCKET) {
    error_exit("socket() error: %d\n", WSAGetLastError());
  }

  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = inet_addr(__argv[1]);
  addr.sin_port        = 0;
  if (bind(rawsock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
    error_exit("bind() error: %d\n", WSAGetLastError());
  }

  if(setsockopt(rawsock, IPPROTO_IP, IP_HDRINCL,
		(char *)&on, sizeof(on)) == SOCKET_ERROR) {
    error_exit("setsockopt(IP_HDRINCL) error: %d\n", WSAGetLastError());
  }

  if (ioctlsocket(rawsock, SIO_RCVALL, &on) == SOCKET_ERROR) {
    error_exit("ioctlsocket(SIO_RCVALL) error: %d\n", WSAGetLastError());
  }

  if(ioctlsocket(rawsock, FIONBIO, &on) == SOCKET_ERROR) {
    error_exit("ioctlsocket(FIONBIO) error: %d\n", WSAGetLastError());
  }
}
/*-----------------------------------------------------------------------------------*/
void
rawsock_send(void)
{
  char sendbuf[UIP_BUFSIZE];
  struct sockaddr_in addr;
  unsigned long off = 0;
  unsigned long on = 1;

  memcpy(sendbuf,      uip_buf,     40);
  memcpy(sendbuf + 40, uip_appdata, uip_len - 40);

  if(ioctlsocket(rawsock, FIONBIO, &off) == SOCKET_ERROR) {
    error_exit("ioctlsocket(!FIONBIO) error: %d\n", WSAGetLastError());
  }

  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = *(unsigned long *)BUF->destipaddr;
  addr.sin_port        = 0;
  if(sendto(rawsock, sendbuf, uip_len, 0,
	    (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
    error_exit("sendto() error: %d\n", WSAGetLastError());
  }

  if(ioctlsocket(rawsock, FIONBIO, &on) == SOCKET_ERROR) {
    error_exit("ioctlsocket(FIONBIO) error: %d\n", WSAGetLastError());
  }
}  
/*-----------------------------------------------------------------------------------*/
u16_t
rawsock_poll(void)
{
  int received;

  if (rawsock == INVALID_SOCKET) {
    return 0;
  }

  received = recv(rawsock, uip_buf, UIP_BUFSIZE, 0);
  if(received == SOCKET_ERROR) {

    if(WSAGetLastError() != WSAEWOULDBLOCK) {
      error_exit("recv() error: %d\n", WSAGetLastError());
    }
    return 0;

  }
  return received;
}
/*-----------------------------------------------------------------------------------*/
