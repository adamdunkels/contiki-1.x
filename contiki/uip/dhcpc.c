
#include "contiki.h"
#include "dhcpc.h"
#include "uip_arp.h"
#include "pt.h"

#include <string.h>

#define STATE_INITIAL         0
#define STATE_SENDING         1
#define STATE_OFFER_RECEIVED  2
#define STATE_CONFIG_RECEIVED 3

static struct {
  struct pt pt;
  char state;
  struct uip_udp_conn *conn;
  struct timer timer;
  u16_t secs;
  
  u8_t serverid[4];
  
  u16_t ipaddr[2];
  u16_t netmask[2];
  u16_t dnsaddr[2];
  u16_t default_router[2];
} s;

struct dhcp_msg {
  u8_t op, htype, hlen, hops;
  u8_t xid[4];
  u16_t secs, flags;
  u8_t ciaddr[4];
  u8_t yiaddr[4];
  u8_t siaddr[4];
  u8_t giaddr[4];
  u8_t chaddr[16];
  u8_t sname[64];
  u8_t file[128];
  u8_t options[312];
};
#define DHCP_REQUEST        1
#define DHCP_REPLY          2
#define DHCP_HTYPE_ETHERNET 1
#define DHCP_HLEN_ETHERNET  6
#define DHCP_MSG_LEN      236

#define DHCPC_SERVER_PORT  67
#define DHCPC_CLIENT_PORT  68

#define DHCPDISCOVER  1
#define DHCPOFFER     2
#define DHCPREQUEST   3
#define DHCPDECLINE   4
#define DHCPACK       5
#define DHCPNAK       6
#define DHCPRELEASE   7 

#define DHCP_OPTION_SUBNET_MASK   1
#define DHCP_OPTION_ROUTER        3
#define DHCP_OPTION_DNS_SERVER    6
#define DHCP_OPTION_REQ_IPADDR   50
#define DHCP_OPTION_MSG_TYPE     53
#define DHCP_OPTION_SERVER_ID    54
#define DHCP_OPTION_REQ_LIST     55
#define DHCP_OPTION_END         255

static const u8_t xid[4] = {0xad, 0xde, 0x12, 0x23};
static const u8_t magic_cookie[4] = {99, 130, 83, 99};
/*---------------------------------------------------------------------------*/
static u8_t *
add_msg_type(u8_t *optptr, u8_t type)
{
  *optptr++ = DHCP_OPTION_MSG_TYPE;
  *optptr++ = 1;
  *optptr++ = type;
  return optptr;
}
/*---------------------------------------------------------------------------*/
static u8_t *
add_server_id(u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_SERVER_ID;
  *optptr++ = 4;
  memcpy(optptr, s.serverid, 4);  
  return optptr + 4;
}
/*---------------------------------------------------------------------------*/
static u8_t *
add_req_ipaddr(u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_REQ_IPADDR;
  *optptr++ = 4;
  memcpy(optptr, s.ipaddr, 4);  
  return optptr + 4;
}
/*---------------------------------------------------------------------------*/
static u8_t *
add_req_options(u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_REQ_LIST;
  *optptr++ = 3;
  *optptr++ = DHCP_OPTION_SUBNET_MASK;
  *optptr++ = DHCP_OPTION_ROUTER;
  *optptr++ = DHCP_OPTION_DNS_SERVER;
  return optptr;
}
/*---------------------------------------------------------------------------*/
static u8_t *
add_end(u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_END;
  return optptr;
}
/*---------------------------------------------------------------------------*/
static void
create_msg(register struct dhcp_msg *m)
{
  m->op = DHCP_REQUEST;
  m->htype = DHCP_HTYPE_ETHERNET;
  m->hlen = DHCP_HLEN_ETHERNET;
  m->hops = 0;
  memcpy(m->xid, xid, sizeof(m->xid));
  m->secs = 0;
  m->flags = 0;
  /*  uip_ipaddr_copy(m->ciaddr, uip_hostaddr);*/
  memcpy(m->ciaddr, uip_hostaddr, sizeof(m->ciaddr));
  memset(m->yiaddr, 0, sizeof(m->yiaddr));
  memset(m->siaddr, 0, sizeof(m->siaddr));
  memset(m->giaddr, 0, sizeof(m->giaddr));
  memcpy(m->chaddr, &uip_ethaddr, 6);
  memset(&m->chaddr[6], 0, 10);
  memset(m->sname, 0, sizeof(m->sname));
  memset(m->file, 0, sizeof(m->file));

  memcpy(m->options, magic_cookie, sizeof(magic_cookie));
}
/*---------------------------------------------------------------------------*/
static void
send_discover(void)
{
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;

  create_msg(m);

  add_end(add_req_options(add_msg_type(&m->options[4], DHCPDISCOVER)));


  uip_udp_send(300);
}
/*---------------------------------------------------------------------------*/
static void
send_request(void)
{
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;

  create_msg(m);
  
  add_end(add_req_ipaddr(add_server_id(add_msg_type(&m->options[4],
						    DHCPREQUEST))));
  
  uip_udp_send(300);
}
/*---------------------------------------------------------------------------*/
static u8_t
parse_options(u8_t *optptr, int len)
{
  u8_t *optptr2;
  u8_t type;

  type = 0;
  optptr2 = NULL;
  
  while(*optptr != DHCP_OPTION_END) {
    switch(*optptr) {
    case DHCP_OPTION_SUBNET_MASK:
      memcpy(s.netmask, optptr + 2, 4);
      break;
    case DHCP_OPTION_ROUTER:
      memcpy(s.default_router, optptr + 2, 4);
      break;
    case DHCP_OPTION_DNS_SERVER:
      memcpy(s.dnsaddr, optptr + 2, 4);
      break;
    case DHCP_OPTION_MSG_TYPE:
      type = *(optptr + 2);
      break;
    case DHCP_OPTION_SERVER_ID:
      memcpy(s.serverid, optptr + 2, 4);
      break;
    }

    /*    printf("option type %d len %d\n", *optptr, *(optptr + 1));*/
    len -= *(optptr + 1) + 2;
    optptr += *(optptr + 1) + 2;
    if(len <= 0) {
      return 0;
    }

    if(optptr == optptr2) {
      /* Abort if we don't make progress. */
      return 0;
    }
  }
  return type;
}
/*---------------------------------------------------------------------------*/
static void
parse_msg(void)
{
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;
  
  if(m->op == DHCP_REPLY &&
     memcmp(m->xid, xid, sizeof(xid)) == 0/* &&
					     memcmp(m->chaddr, &uip_ethaddr, sizeof(uip_ethaddr))*/) {
    memcpy(s.ipaddr, m->yiaddr, 4);
    parse_options(&m->options[4], uip_datalen());
  }
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_dhcp(void))
{
  PT_BEGIN(&s.pt);
  
  PT_WAIT_UNTIL(&s.pt, s.state == STATE_SENDING);

  s.secs = 0;
  timer_set(&s.timer, CLOCK_SECOND * 2);

  while(s.state != STATE_OFFER_RECEIVED) {
    send_discover();
    PT_WAIT_UNTIL(&s.pt, uip_newdata() || timer_expired(&s.timer));

    timer_reset(&s.timer);
    
    if(uip_newdata()) {
      parse_msg();
      s.state = STATE_OFFER_RECEIVED;
    }
    ++s.secs;
  }
  
  while(s.state != STATE_CONFIG_RECEIVED) {
    
    send_request();
    
    PT_WAIT_UNTIL(&s.pt, uip_newdata() || timer_expired(&s.timer));

    timer_reset(&s.timer);
    
    if(uip_newdata()) {
      s.state = STATE_CONFIG_RECEIVED;
    }
    ++s.secs;
  }
  
  /*  printf("Got IP address %d.%d.%d.%d\n",
	 uip_ipaddr1(s.ipaddr), uip_ipaddr2(s.ipaddr),
	 uip_ipaddr3(s.ipaddr), uip_ipaddr4(s.ipaddr));
  printf("Got netmask %d.%d.%d.%d\n",
	 uip_ipaddr1(s.netmask), uip_ipaddr2(s.netmask),
	 uip_ipaddr3(s.netmask), uip_ipaddr4(s.netmask));
  printf("Got DNS server %d.%d.%d.%d\n",
	 uip_ipaddr1(s.dnsaddr), uip_ipaddr2(s.dnsaddr),
	 uip_ipaddr3(s.dnsaddr), uip_ipaddr4(s.dnsaddr));
  printf("Got default router %d.%d.%d.%d\n",
	 uip_ipaddr1(s.default_router), uip_ipaddr2(s.default_router),
	 uip_ipaddr3(s.default_router), uip_ipaddr4(s.default_router));*/

  uip_sethostaddr(s.ipaddr);
  uip_setnetmask(s.netmask);
  uip_setdraddr(s.default_router);
  resolv_conf(s.dnsaddr);
  dhcpc_configured();
  
  PT_END(&s.pt);
}
/*---------------------------------------------------------------------------*/
void
dhcpc_init(void)
{
  u16_t addr[2];
  
  s.state = STATE_INITIAL;
  uip_ipaddr(addr, 255,255,255,255);
  s.conn = udp_new(addr, HTONS(DHCPC_SERVER_PORT), NULL);
  if(s.conn != NULL) {
    udp_bind(s.conn, HTONS(DHCPC_CLIENT_PORT));
  }
  PT_INIT(&s.pt);
}
/*---------------------------------------------------------------------------*/
void
dhcpc_appcall(void *state)
{
  handle_dhcp();
}
/*---------------------------------------------------------------------------*/
void
dhcpc_request(void)
{
  u16_t ipaddr[2];
  
  if(s.state == STATE_INITIAL) {
    uip_ipaddr(ipaddr, 0,0,0,0);
    uip_sethostaddr(ipaddr);
    s.state = STATE_SENDING;
    tcpip_poll_udp(s.conn);
  }
}
/*---------------------------------------------------------------------------*/
