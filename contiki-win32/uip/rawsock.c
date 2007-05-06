#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <malloc.h>

/* from mstcpip.h */

#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)

/* from iphlpapi.h */

#pragma pack(push, 4)

typedef struct mib_ipaddrrow_t {
  DWORD dwAddr;
  DWORD dwIndex;
  DWORD dwMask;
  DWORD dwBCastAddr;
  DWORD dwReasmSize;
  DWORD unused;
} MIB_IPADDRROW;

typedef struct mib_ipaddrtable_t {
  DWORD         dwNumEntries;
  MIB_IPADDRROW table[0];
} MIB_IPADDRTABLE;

#pragma pack(pop)

#define htons contiki_htons /* htons is defined in winsock2 */
#include "uip.h"
#undef htons
#include "tcpip.h"
#include "ctk.h"

#define BUF ((uip_tcpip_hdr *)uip_buf)

typedef DWORD (WINAPI *ARPFUNC)(DWORD, DWORD, DWORD);

extern void debug_printf(char *format, ...);

static void error_exit(const char *message, int value);
static void create_proxyarp(void);
static void delete_proxyarp(void);

static DWORD  interfaceindex;
static DWORD  proxyaddr;
static SOCKET rawsock = INVALID_SOCKET;


#define END_OF(ARRAY) (ARRAY + sizeof(ARRAY)/sizeof(ARRAY[0]))
#define HOSTADDR() (*(DWORD*)uip_hostaddr)
#define NETMASK() (*(DWORD*)uip_netmask)

#define DELAY_LOAD(NAME,MODULE) \
( GetProcAddress(GetModuleHandle(MODULE), NAME) \
  ? GetProcAddress(GetModuleHandle(MODULE), NAME) \
  : (error_exit("GetProcAddress("#NAME") error: %d\n", GetLastError()), 0))

#define SOCKCTL_(ON,CMD) \
{ unsigned long on = ON; if (SOCKET_ERROR == \
  ioctlsocket(rawsock, CMD, &on)) \
    error_exit("ioctlsocket("#CMD") error: %d\n", WSAGetLastError()); }

#define SOCKOPT_ON_(OPT,LAYER) \
{ unsigned long on = 1; if (SOCKET_ERROR == \
  setsockopt(rawsock, LAYER, OPT, (char*)&on, sizeof(on))) \
    error_exit("setsockopt("#OPT") error: %d\n", WSAGetLastError()); }

#define TRAP(ERR,HEAD,TAIL) \
( match(ERR,HEAD,TAIL) ? ERR : (error_exit("send/recv() error: %d\n",ERR),0) )

/*-----------------------------------------------------------------------------------*/
static int match(const int x, const int *head, const int *const tail)
{
  for( ;tail != head; ++head) if(*head == x) return 1;
  return 0;
}
/*-----------------------------------------------------------------------------------*/
static void
error_exit(const char *message, int value)
{
  console_exit();
  cprintf(message, value);
  exit(EXIT_FAILURE);
}
/*-----------------------------------------------------------------------------------*/
static DWORD
get_interface_index_for_(DWORD ip)
{
  DWORD (WINAPI *Table)(MIB_IPADDRTABLE *, DWORD *, BOOL);
  DWORD                retval;
  DWORD                size = sizeof(MIB_IPADDRROW);
  MIB_IPADDRTABLE     *d = 0;
  MIB_IPADDRROW       *r;

  (FARPROC)Table = DELAY_LOAD("GetIpAddrTable","iphlpapi");
  do { d = realloc(d, size *= 2); }
  while(ERROR_INSUFFICIENT_BUFFER == (retval = Table(d, &size, TRUE)));
  if(NO_ERROR != retval) error_exit("GetIpAddrTable error: %d\n", retval);

  for(r = d->table; r != d->table + d->dwNumEntries; ++r)
    if(r->dwAddr == ip) return retval = r->dwIndex, realloc(d,0), retval;
  return error_exit("Parameter error: Unknown host IP address\n", 0), 0;
}
/*-----------------------------------------------------------------------------------*/
static void*
sock_in_addr(unsigned long ipaddr, u16_t port)
{
  static struct sockaddr_in addr;
  addr.sin_family      = AF_INET;
  addr.sin_port        = port;
  addr.sin_addr.s_addr = ipaddr;
  return &addr;
}
/*-----------------------------------------------------------------------------------*/
void
rawsock_init(void)
{
  DWORD                hostaddr;
  WSADATA              wsadata;

  if(WSAStartup(2, &wsadata) != 0) {
    error_exit("WSAStartup() error\n", 0);
  }

  hostaddr = inet_addr(__argv[1]);
  if(hostaddr == INADDR_NONE) {
    error_exit("Parameter error: Invalid host IP address\n", 0);
  }

  rawsock = socket(PF_INET, SOCK_RAW, IPPROTO_IP);
  if(rawsock == INVALID_SOCKET) {
    error_exit("socket() error: %d\n", WSAGetLastError());
  }

  if(bind(rawsock, sock_in_addr(hostaddr,0), sizeof(struct sockaddr)) == SOCKET_ERROR) {
    error_exit("bind() error: %d\n", WSAGetLastError());
  }

  SOCKOPT_ON_(SO_BROADCAST, SOL_SOCKET);
  SOCKOPT_ON_(IP_HDRINCL, IPPROTO_IP);
  SOCKCTL_(1, SIO_RCVALL);
  SOCKCTL_(1, FIONBIO);

  LoadLibrary("iphlpapi"); /* map iphlpapi.dll to process address space */
  if(atexit(delete_proxyarp)) error_exit("atexit() error\n", 0);
  interfaceindex = get_interface_index_for_(hostaddr);
}
/*-----------------------------------------------------------------------------------*/
void
rawsock_fini(void)
{
  delete_proxyarp();
  closesocket(rawsock);
  rawsock = INVALID_SOCKET;
  FreeLibrary(GetModuleHandle("iphlpapi"));
  WSACleanup();
}
/*-----------------------------------------------------------------------------------*/
void
rawsock_send(u8_t *hdr, u16_t hdrlen, u8_t *data, u16_t datalen)
{
  static const int ignore[] = {WSAEADDRNOTAVAIL,WSAEHOSTUNREACH};
  WSABUF sendbuf[2] = {{hdrlen, hdr},{datalen, data}};
  DWORD  value;

  SOCKCTL_(0, FIONBIO); /* switch Non-blocking I/O off */

  value = WSASendTo(rawsock, sendbuf, 2, &value, 0
    , sock_in_addr(*(unsigned*)BUF->destipaddr,0), sizeof(struct sockaddr_in)
	, NULL, NULL);
  if(SOCKET_ERROR == value) TRAP(WSAGetLastError(), ignore, END_OF(ignore));

  SOCKCTL_(1, FIONBIO); /* switch Non-blocking I/O on */
}  
/*-----------------------------------------------------------------------------------*/
u16_t
rawsock_poll(void)
{
  static const int ignore[] = {WSAEWOULDBLOCK,WSAEMSGSIZE,WSANOTINITIALISED};
  const int nr = (create_proxyarp(),recv(rawsock, uip_buf, UIP_BUFSIZE, 0));
  return SOCKET_ERROR != nr ? nr : TRAP(WSAGetLastError(), ignore, END_OF(ignore));
}
/*-----------------------------------------------------------------------------------*/
void
create_proxyarp(void)
{
  if(proxyaddr != HOSTADDR()) {
    FARPROC f = DELAY_LOAD("CreateProxyArpEntry","iphlpapi");
    DWORD retval = (delete_proxyarp(),((ARPFUNC)f)(HOSTADDR(), NETMASK(), interfaceindex));
    proxyaddr = HOSTADDR();
    debug_printf("CreateProxyArpEntry(%s): %d\n", inet_ntoa(*(struct in_addr*)&proxyaddr), retval);
  }
}
/*-----------------------------------------------------------------------------------*/
void
delete_proxyarp(void)
{
  if(proxyaddr) {
    FARPROC f = DELAY_LOAD("DeleteProxyArpEntry","iphlpapi");
    DWORD retval = ((ARPFUNC)f)(HOSTADDR(), NETMASK(), interfaceindex);
    debug_printf("DeleteProxyArpEntry(%s): %d\n", inet_ntoa(*(struct in_addr*)&proxyaddr), retval);
    proxyaddr = 0;
  }
}
/*-----------------------------------------------------------------------------------*/
