/**
 * \file
 * uIP packet forwarding.
 * \author Adam Dunkels <adam@sics.se>
 *
 * This file implements a number of simple functions which do packet
 * forwarding over multiple network interfaces with uIP.
 *
 */

#include "uip.h"
#include "uip-fw.h"

/**
 * \internal
 * The list of registered network interfaces.
 */
static struct uip_fw_netif *netifs = NULL;

/**
 * \internal
 * A pointer to the default network interface.
 */
static struct uip_fw_netif *defaultnetif = NULL;

struct tcpip_hdr {
  /* IP header. */
  u8_t vhl,
    tos;     
  u16_t len,
    ipid,        
    ipoffset;
  u8_t ttl,          
    proto;     
  u16_t ipchksum;
  u16_t srcipaddr[2], 
    destipaddr[2];
  
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
};

/**
 * \internal
 * Pointer to the TCP/IP headers of the packet in the uip_buf buffer.
 */
#define BUF ((struct tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

/**
 * \internal
 * Certain fields of an IP packet that are used for identifying
 * duplicate packets.
 */
struct fwcache_entry {
  u16_t timestamp;
  
  u16_t len, offset, ipid;
  u16_t srcipaddr[2];
  u16_t destipaddr[2];
  u16_t payload[2];  
};

/**
 * \internal
 * The number of packets to remember when looking for duplicates.
 */
#define FWCACHE_SIZE 2

/**
 * \internal
 * A cache of packet header fields which are used for
 * identifying duplicate packets.
 */
static struct fwcache_entry fwcache[FWCACHE_SIZE];

/*------------------------------------------------------------------------------*/
/**
 * Initialize the uIP packet forwarding module.
 */
/*------------------------------------------------------------------------------*/
void
uip_fw_init(void)
{
  defaultnetif = netifs = NULL;
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Check if an IP address is within the network defined by an IP
 * address and a netmask.
 *
 * \param ipaddr The IP address to be checked.
 * \param netipaddr The IP address of the network.
 * \param netmask The netmask of the network.
 *
 * \return Non-zero if IP address is in network, zero otherwise.
 */
/*------------------------------------------------------------------------------*/
static unsigned char
ipaddr_maskcmp(u16_t *ipaddr, u16_t *netipaddr, u16_t *netmask)
{
  return (ipaddr[0] & netmask [0]) == (netipaddr[0] & netmask[0]) &&
    (ipaddr[1] & netmask[1]) == (netipaddr[1] & netmask[1]);
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Register a packet in the forwarding cache so that it won't be
 * forwarded again.
 */
/*------------------------------------------------------------------------------*/
static void
fwcache_register(void)
{
  struct fwcache_entry *fw;
    
  /* Register packet in forwarding cache. */
  fw = &fwcache[0];
  fw->len = BUF->len;
  fw->offset = BUF->ipoffset;
  fw->ipid = BUF->ipid;
  fw->srcipaddr[0] = BUF->srcipaddr[0];
  fw->srcipaddr[1] = BUF->srcipaddr[1];
  fw->destipaddr[0] = BUF->destipaddr[0];
  fw->destipaddr[1] = BUF->destipaddr[1];
  fw->payload[0] = BUF->srcport;
  fw->payload[1] = BUF->destport;
}
/*------------------------------------------------------------------------------*/
/**
 * Output an IP packet on the correct network interface.
 *
 * The IP packet should be present in the uip_buf buffer and its
 * length in the global uip_len variable.
 */
/*------------------------------------------------------------------------------*/
void
uip_fw_output(void)
{
  struct uip_fw_netif *netif;
  
  /* Walk through every network interface to check for a match. */
  for(netif = netifs; netif != NULL; netif = netif->next) {
    if(ipaddr_maskcmp(BUF->destipaddr, netif->ipaddr,
		      netif->netmask)) {
      /* If there was a match, we break the loop. */
      break;
    }
  }
  
  /* If no matching netif was found, we use default netif. */
  if(netif == NULL) {
    netif = defaultnetif;
  }

  /* If we now have found a suitable network interface, we call its
     output function to send out the packet. */
  if(netif != NULL && uip_len > 0) {
    fwcache_register();
    netif->output();
  }
}
/*------------------------------------------------------------------------------*/
/**
 * Forward an IP packet in the uip_buf buffer.
 *
 * 
 *
 * \return Non-zero if the packet was forwarded, zero if the packet
 * should be processed locally.
 */
/*------------------------------------------------------------------------------*/
unsigned char
uip_fw_forward(void)
{
  struct uip_fw_netif *netif;
  struct fwcache_entry *fw;
  
  /* First check if the packet is destined for ourselves and return 0
     to indicate that the packet should be processed locally. */
  if(BUF->destipaddr[0] == uip_hostaddr[0] &&
     BUF->destipaddr[1] == uip_hostaddr[1]) {
    return 0;
  }

  /* Check if the packet is in the forwarding cache already, and if so
     we drop it. */
  /*  for(fw = fwcache; fw <= &fwcache[FWCACHE_SIZE]; ++fw) {*/
  fw = &fwcache[0];
  {
    if(fw->len == BUF->len &&
       fw->offset == BUF->ipoffset &&
       fw->ipid == BUF->ipid &&
       fw->srcipaddr[0] == BUF->srcipaddr[0] &&
       fw->srcipaddr[1] == BUF->srcipaddr[1] &&
       fw->destipaddr[0] == BUF->destipaddr[0] &&
       fw->destipaddr[1] == BUF->destipaddr[1] &&
       fw->payload[0] == BUF->srcport &&
       fw->payload[1] == BUF->destport) {
      /* Drop packet. */
      return 1;
    }       
  }

  
  /* Walk through every network interface to check for a match. */
  for(netif = netifs; netif != NULL; netif = netif->next) {
    if(ipaddr_maskcmp(BUF->destipaddr, netif->ipaddr,
		      netif->netmask)) {
      /* If there was a match, we break the loop. */
      break;
    }
  }
  
  /* If no matching netif was found, we use default netif. */
  if(netif == NULL) {
    netif = defaultnetif;
  }

  /* Decrement the TTL (time-to-live) value in the IP header */
  BUF->ttl = htons(htons(BUF->ttl) - 1);
  
  /* We should really send an ICMP message if TTL == 0, but we skip it
     for now... Instead, we just drop the packet (and pretend that it
     was forwarded by returning 1). */
  if(BUF->ttl == 0) {
    return 1;
  }

  /* Incrementally update the IP checksum. */
  if(BUF->ipchksum >= htons(0xffff - 0x0100)) {
    BUF->ipchksum = BUF->ipchksum + HTONS(0x0100) + 1;
  } else {
    BUF->ipchksum = BUF->ipchksum + HTONS(0x0100);
  }
  
  /* If we now have found a suitable network interface, we call its
     output function to send out the packet. */
  if(netif != NULL && uip_len > 0) {
    uip_appdata = &uip_buf[UIP_LLH_LEN + 40];
    fwcache_register();
    netif->output();
  }

  /* Return non-zero to indicate that the packet was forwarded and that no
     other processing should be made. */
  return 1;
}
/*------------------------------------------------------------------------------*/
/**
 * Register a network interface with the forwarding module.
 *
 * \param netif A pointer to the network interface that is to be
 * registered.  
 */
/*------------------------------------------------------------------------------*/
void
uip_fw_register(struct uip_fw_netif *netif)
{
  netif->next = netifs;
  netifs = netif;
}
/*------------------------------------------------------------------------------*/
/**
 * Register a default network interface.
 *
 * All packets that don't go out on any of the other interfaces will
 * be routed to the default interface.
 *
 * \param netif A pointer to the network interface that is to be
 * registered.  
 */
/*------------------------------------------------------------------------------*/
void
uip_fw_default(struct uip_fw_netif *netif)
{
  defaultnetif = netif;
}
/*------------------------------------------------------------------------------*/
