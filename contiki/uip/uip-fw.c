/**
 * \addtogroup uip
 * @{
 */

/**
 * \defgroup uipfw uIP packet forwarding
 * @{
 *
 */

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
#include "uip_arch.h"
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

struct icmpip_hdr {
  /* IP header. */
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
  /* ICMP (echo) header. */
  u8_t type, icode;
  u16_t icmpchksum;
  u16_t id, seqno;
  u8_t payload[1];
};

/**
 * \internal
 * ICMP ECHO type definition.
 */
#define ICMP_ECHO 8 

/**
 * \internal
 * ICMP TIME-EXCEEDED type definition.
 */
#define ICMP_TE 11 

/**
 * \internal
 * Pointer to the TCP/IP headers of the packet in the uip_buf buffer.
 */
#define BUF ((struct tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

/**
 * \internal
 * Pointer to the ICMP/IP headers of the packet in the uip_buf buffer.
 */
#define ICMPBUF ((struct icmpip_hdr *)&uip_buf[UIP_LLH_LEN])

/**
 * \internal
 * Certain fields of an IP packet that are used for identifying
 * duplicate packets.
 */
struct fwcache_entry {
  u16_t timer;
  
  u16_t len, offset, ipid;
  u16_t srcipaddr[2];
  u16_t destipaddr[2];
  u16_t payload[2];
  u8_t proto;
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

/**
 * \internal
 * The time that a packet cache is active.
 */
#define FW_TIME 20

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
 * Send out an ICMP TIME-EXCEEDED message.
 *
 * This function replaces the packet in the uip_buf buffer with the
 * ICMP packet.
 */
/*------------------------------------------------------------------------------*/
static void
time_exceeded(void)
{
  u16_t tmp16;

  /* We don't send out ICMP errors for ICMP messages. */
  if(ICMPBUF->proto == UIP_PROTO_ICMP) {
    uip_len = 0;
    return;
  }
  /* Copy fields from packet header into payload of this ICMP packet. */
  memcpy(&(ICMPBUF->payload[0]), ICMPBUF, 28);

  /* Set the ICMP type and code. */
  ICMPBUF->type = ICMP_TE;
  ICMPBUF->icode = 0;

  /* Calculate the ICMP checksum. */
  ICMPBUF->icmpchksum = 0;
  ICMPBUF->icmpchksum = ~uip_chksum((u16_t *)&(ICMPBUF->type), 36);

  /* Set the IP destination address to be the source address of the
     original packet. */
  tmp16= BUF->destipaddr[0];
  BUF->destipaddr[0] = BUF->srcipaddr[0];
  BUF->srcipaddr[0] = tmp16;
  tmp16 = BUF->destipaddr[1];
  BUF->destipaddr[1] = BUF->srcipaddr[1];
  BUF->srcipaddr[1] = tmp16;

  /* Set our IP address as the source address. */
  BUF->srcipaddr[0] = uip_hostaddr[0];
  BUF->srcipaddr[1] = uip_hostaddr[1];

  /* The size of the ICMP time exceeded packet is 36 + the size of the
     IP header (20) = 56. */
  uip_len = 56;
  ICMPBUF->len[0] = 0;
  ICMPBUF->len[1] = uip_len;

  /* Fill in the other fields in the IP header. */
  ICMPBUF->vhl = 0x45;
  ICMPBUF->tos = 0;
  ICMPBUF->ipoffset[0] = ICMPBUF->ipoffset[1] = 0;
  ICMPBUF->ttl  = UIP_TTL;
  ICMPBUF->proto = UIP_PROTO_ICMP;
  
  /* Calculate IP checksum. */
  ICMPBUF->ipchksum = 0;
  ICMPBUF->ipchksum = ~(uip_ipchksum());


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
  int i, oldest;

  oldest = FW_TIME;
  fw = NULL;
  
  /* Find the oldest entry in the cache. */
  for(i = 0; i < FWCACHE_SIZE; ++i) {
    if(fwcache[i].timer == 0) {
      fw = &fwcache[i];
      break;
    } else if(fwcache[i].timer <= oldest) {
      fw = &fwcache[i];
      oldest = fwcache[i].timer;
    }
  }

  fw->timer = FW_TIME;
  fw->len = BUF->len;
  fw->offset = BUF->ipoffset;
  fw->ipid = BUF->ipid;
  fw->srcipaddr[0] = BUF->srcipaddr[0];
  fw->srcipaddr[1] = BUF->srcipaddr[1];
  fw->destipaddr[0] = BUF->destipaddr[0];
  fw->destipaddr[1] = BUF->destipaddr[1];
  fw->payload[0] = BUF->srcport;
  fw->payload[1] = BUF->destport;
  fw->proto = BUF->proto;
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Find a network interface for the IP packet in uip_buf.
 */
/*------------------------------------------------------------------------------*/
static struct uip_fw_netif *
find_netif(void)
{
  struct uip_fw_netif *netif;
  
  /* Walk through every network interface to check for a match. */
  for(netif = netifs; netif != NULL; netif = netif->next) {
    if(ipaddr_maskcmp(BUF->destipaddr, netif->ipaddr,
		      netif->netmask)) {
      /* If there was a match, we break the loop. */
      return netif;
    }
  }
  
  /* If no matching netif was found, we use default netif. */
  return defaultnetif;  
}    
/*------------------------------------------------------------------------------*/
/**
 * Output an IP packet on the correct network interface.
 *
 * The IP packet should be present in the uip_buf buffer and its
 * length in the global uip_len variable.
 *
 * \retval UIP_FW_ZEROLEN Indicates that a zero-length packet
 * transmission was attempted and that no packet was sent.
 *
 * \retval UIP_FW_NOROUTE No suitable network interface could be found
 * for the outbound packet, and the packet was not sent.
 *
 * \return The return value from the actual network interface output
 * function is passed unmodified as a return value.
 */
/*------------------------------------------------------------------------------*/
u8_t
uip_fw_output(void)
{
  struct uip_fw_netif *netif;

  if(uip_len == 0) {
    return UIP_FW_ZEROLEN;
  }
  
  netif = find_netif();
  /*  printf("uip_fw_output: netif %p ->output %p len %d\n", netif,
	 netif->output,
	 uip_len);*/

  if(netif == NULL) {
    return UIP_FW_NOROUTE;
  }
  /* If we now have found a suitable network interface, we call its
     output function to send out the packet. */
  fwcache_register();
  return netif->output();
}
/*------------------------------------------------------------------------------*/
/**
 * Forward an IP packet in the uip_buf buffer.
 *
 * 
 *
 * \return UIP_FW_FORWARDED if the packet was forwarded, UIP_FW_LOCAL if
 * the packet should be processed locally.
 */
/*------------------------------------------------------------------------------*/
u8_t
uip_fw_forward(void)
{
  struct uip_fw_netif *netif;
  struct fwcache_entry *fw;

  /* First check if the packet is destined for ourselves and return 0
     to indicate that the packet should be processed locally. */
  if(BUF->destipaddr[0] == uip_hostaddr[0] &&
     BUF->destipaddr[1] == uip_hostaddr[1]) {
    return UIP_FW_LOCAL;
  }

  /* If we use ping IP address configuration, and our IP address is
     not yet configured, we should intercept all ICMP echo packets. */
#if UIP_PINGADDRCONF
  if((uip_hostaddr[0] | uip_hostaddr[1]) == 0 &&
     BUF->proto == UIP_PROTO_ICMP &&
     ICMPBUF->type == ICMP_ECHO) {
    return UIP_FW_LOCAL;
  }
#endif /* UIP_PINGADDRCONF */

  /* Check if the packet is in the forwarding cache already, and if so
     we drop it. */

  for(fw = fwcache; fw <= &fwcache[FWCACHE_SIZE]; ++fw) {
    if(fw->timer != 0 &&
       fw->len == BUF->len &&
       fw->offset == BUF->ipoffset &&
       fw->ipid == BUF->ipid &&      
       fw->srcipaddr[0] == BUF->srcipaddr[0] &&
       fw->srcipaddr[1] == BUF->srcipaddr[1] &&
       fw->destipaddr[0] == BUF->destipaddr[0] &&
       fw->destipaddr[1] == BUF->destipaddr[1] &&
       fw->proto == BUF->proto &&
       fw->payload[0] == BUF->srcport &&
       fw->payload[1] == BUF->destport) {
      /* Drop packet. */
      return UIP_FW_FORWARDED;
    }       
  }

  netif = find_netif();

  /* Decrement the TTL (time-to-live) value in the IP header */
  BUF->ttl = BUF->ttl - 1;
  
  /* Update the IP checksum. */
  if(BUF->ipchksum >= HTONS(0xffff - 0x0100)) {
    BUF->ipchksum = BUF->ipchksum + HTONS(0x0100) + 1;
  } else {
    BUF->ipchksum = BUF->ipchksum + HTONS(0x0100);
  }
  
  /* If the TTL reaches zero we procude an ICMP time exceeded message
     in the uip_buf buffer and forward that packet back to the sender
     of the packet. */
  if(BUF->ttl == 0) {    
    time_exceeded();
    netif = find_netif();
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
  return UIP_FW_FORWARDED;
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
/**
 * Perform periodic processing.
 */
/*------------------------------------------------------------------------------*/
void
uip_fw_periodic(void)
{
  struct fwcache_entry *fw;
  for(fw = fwcache; fw <= &fwcache[FWCACHE_SIZE]; ++fw) {
    if(fw->timer > 0) {
      --fw->timer;
    }    
  }
}
/*------------------------------------------------------------------------------*/
