#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <malloc.h>

/* from mstcpip.h */

#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)

/* from iphlpapi.h */

#pragma pack(push, 4)

typedef struct {
  DWORD          dwAddr;
  DWORD          dwIndex;
  DWORD          dwMask;
  DWORD          dwBCastAddr;
  DWORD          dwReasmSize;
  unsigned short unused1;
  unsigned short unused2;
} MIB_IPADDRROW;

typedef struct {
  DWORD         dwNumEntries;
  MIB_IPADDRROW table[0];
} MIB_IPADDRTABLE;

static DWORD (WINAPI *GetIpAddrTable)(MIB_IPADDRTABLE *, DWORD *, BOOL);

static DWORD (WINAPI *CreateProxyArpEntry)(DWORD, DWORD, DWORD);

static DWORD (WINAPI *DeleteProxyArpEntry)(DWORD, DWORD, DWORD);

#pragma pack(pop)

#define htons contiki_htons /* htons is defined in winsock2 */
#include "uip.h"
#undef htons
#include "tcpip.h"
#include "ctk.h"

#define SOCKIO_(ON,CMD) sockio_(ON,CMD,"ioctlsocket("#CMD") error: %d\n")

#define BUF ((uip_tcpip_hdr *)uip_buf)

void debug_printf(char *format, ...);

static DWORD  interfaceindex;
static DWORD  proxyaddr;
static SOCKET rawsock = INVALID_SOCKET;
static unsigned long off = 0;
static unsigned long on  = 1;

static const int recv_filt[] = {WSAEWOULDBLOCK,WSAEMSGSIZE};
static const int send_filt[] = {WSAEADDRNOTAVAIL,WSAEHOSTUNREACH};

static void sockio_(unsigned long on, long cmd, char *cmd_err_fmt);
static u16_t filt_err(int e, const int *head, const int *const tail);
static void update_proxy_arp(void);
static u16_t filter_recv(int value);
static u16_t filter_send(int value);
/*-----------------------------------------------------------------------------------*/
static int match(const int x, const int *head, const int *const tail)
{
  for( ;tail != head; ++head)
  {
    if(*head == x) return 1;
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
static void
create_proxyarp(void)
{
  DWORD          retval;
  struct in_addr addr;

  if(proxyaddr == 0) {
    return;
  }
  retval = CreateProxyArpEntry(proxyaddr, 0xFFFFFFFF, interfaceindex);
  addr.S_un.S_addr = proxyaddr;
  debug_printf("CreateProxyArpEntry(%s): %d\n", inet_ntoa(addr), retval);
}
/*-----------------------------------------------------------------------------------*/
static void
delete_proxyarp(void)
{
  DWORD          retval;
  struct in_addr addr;

  if(proxyaddr == 0) {
    return;
  }
  retval = DeleteProxyArpEntry(proxyaddr, 0xFFFFFFFF, interfaceindex);
  addr.S_un.S_addr = proxyaddr;
  debug_printf("DeleteProxyArpEntry(%s): %d\n", inet_ntoa(addr), retval);
}
/*-----------------------------------------------------------------------------------*/
static int
error_exit(char *message, int value)
{
  console_exit();
  cprintf(message, value);
  exit(EXIT_FAILURE);
  return 0;
}
/*-----------------------------------------------------------------------------------*/
void
rawsock_init(void)
{
  DWORD                hostaddr;
  HMODULE              iphlpapi;
  MIB_IPADDRTABLE      *addrtable;
  DWORD                addrtablesize = 0;
  DWORD                retval;
  DWORD                entry = 0;
  WSADATA              wsadata;
  struct sockaddr_in   addr;

  hostaddr = inet_addr(__argv[1]);
  if(hostaddr == INADDR_NONE) {
    error_exit("Parameter error: Invalid host IP address\n", 0);
  }

  iphlpapi = LoadLibrary("iphlpapi.dll");
  if(iphlpapi == NULL) {
    error_exit("LoadLibrary(iphlpapi.dll) error: %d\n", GetLastError());
  }

  (FARPROC)GetIpAddrTable = GetProcAddress(iphlpapi, "GetIpAddrTable");
  if(GetIpAddrTable == NULL) {
    error_exit("GetProcAddress(GetIpAddrTable) error: %d\n", GetLastError());
  }

  (FARPROC)CreateProxyArpEntry = GetProcAddress(iphlpapi, "CreateProxyArpEntry");
  if(GetIpAddrTable == NULL) {
    error_exit("GetProcAddress(CreateProxyArpEntry) error: %d\n", GetLastError());
  }

  (FARPROC)DeleteProxyArpEntry = GetProcAddress(iphlpapi, "DeleteProxyArpEntry");
  if(GetIpAddrTable == NULL) {
    error_exit("GetProcAddress(DeleteProxyArpEntry) error: %d\n", GetLastError());
  }

  retval = GetIpAddrTable(NULL, &addrtablesize, TRUE);
  if(retval != ERROR_INSUFFICIENT_BUFFER) {
    error_exit("GetIpAddrTable(NULL) error: %d\n", retval);
  }

  addrtable = alloca(addrtablesize);
  retval = GetIpAddrTable(addrtable, &addrtablesize, TRUE);
  if(retval != NO_ERROR) {
    error_exit("GetIpAddrTable(ptr) error: %d\n", retval);
  }

  while(1) {
    if(entry == addrtable->dwNumEntries) {
      error_exit("Parameter error: Unknown host IP address\n", 0);
    }
    if(addrtable->table[entry].dwAddr == hostaddr) {
      interfaceindex = addrtable->table[entry].dwIndex;
      break;
    }
    entry++;
  }

  if(WSAStartup(2, &wsadata) != 0) {
    error_exit("WSAStartup() error\n", 0);
  }

  rawsock = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
  if(rawsock == INVALID_SOCKET) {
    error_exit("socket() error: %d\n", WSAGetLastError());
  }

  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = hostaddr;
  addr.sin_port        = 0;
  if(bind(rawsock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
    error_exit("bind() error: %d\n", WSAGetLastError());
  }

  if(setsockopt(rawsock, SOL_SOCKET, SO_BROADCAST,
		(char *)&on, sizeof(on)) == SOCKET_ERROR) {
    error_exit("setsockopt(SO_BROADCAST) error: %d\n", WSAGetLastError());
  }

  if(setsockopt(rawsock, IPPROTO_IP, IP_HDRINCL,
		(char *)&on, sizeof(on)) == SOCKET_ERROR) {
    error_exit("setsockopt(IP_HDRINCL) error: %d\n", WSAGetLastError());
  }

  SOCKIO_(on, SIO_RCVALL);
  SOCKIO_(on, FIONBIO);

  if(atexit(delete_proxyarp) != 0) {
    error_exit("atexit() error\n", 0);
  }
}
/*-----------------------------------------------------------------------------------*/
void
rawsock_send(u8_t *hdr, u16_t hdrlen, u8_t *data, u16_t datalen)
{
  struct sockaddr_in   addr;
  DWORD  dummy;
  WSABUF sendbuf[2] = {{hdrlen, hdr},{datalen, data}};

  SOCKIO_(off, FIONBIO); /* switch Non-blocking I/O off */

  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = *(unsigned long *)BUF->destipaddr;
  addr.sin_port        = 0;
  filter_send(WSASendTo(rawsock, sendbuf, 2, &dummy, 0
    , (struct sockaddr *)&addr, sizeof(addr), NULL, NULL));

  SOCKIO_(on, FIONBIO); /* switch Non-blocking I/O on */
}  
/*-----------------------------------------------------------------------------------*/
u16_t
rawsock_poll(void)
{
  return INVALID_SOCKET == rawsock ? 0 :
    filter_recv((update_proxy_arp(), recv(rawsock, uip_buf, UIP_BUFSIZE, 0)));
}
/*-----------------------------------------------------------------------------------*/
u16_t filter_recv(int value)
{
  return SOCKET_ERROR == value
    ? filt_err(WSAGetLastError(), recv_filt, recv_filt + 2) : value;
}
/*-----------------------------------------------------------------------------------*/
u16_t filter_send(int value)
{
  return SOCKET_ERROR == value
    ? filt_err(WSAGetLastError(), send_filt, send_filt + 2) : value;
}
/*-----------------------------------------------------------------------------------*/
u16_t filt_err(int v, const int *hd, const int *const tl)
{
  return match(v, hd, tl) ? v : error_exit("send/recv() error: %d\n", v);
}
/*-----------------------------------------------------------------------------------*/
void
update_proxy_arp(void)
{
  if(proxyaddr != *(DWORD *)uip_hostaddr) {
    delete_proxyarp();
    proxyaddr = *(DWORD *)uip_hostaddr;
    create_proxyarp();
  }
}
/*-----------------------------------------------------------------------------------*/
void
sockio_(unsigned long on, long cmd, char *cmd_err_fmt)
{
  if(SOCKET_ERROR == ioctlsocket(rawsock, cmd, &on)) {
    error_exit(cmd_err_fmt, WSAGetLastError());
  }
}
/*-----------------------------------------------------------------------------------*/
