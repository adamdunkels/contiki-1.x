
#include "uip.h"

#include <stdio.h>

 struct ip_hdr {
  /* IP header. */
   u8_t vhl,
    tos,          
     len[2],       
     ipid[2],        
     ipoffset[2],  
     ttl,          
     proto;     
   u16_t ipchksum;
   u8_t srcipaddr[4], 
     destipaddr[4];
 };

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20
#define TCP_CTL 0x3f

struct tcpip_hdr {
  /* IP header. */
   u8_t vhl,
    tos,          
     len[2],       
     ipid[2],        
     ipoffset[2],  
     ttl,          
     proto;     
   u16_t ipchksum;
   u8_t srcipaddr[4], 
     destipaddr[4];
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

#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO       8     

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
  u8_t srcipaddr[4], 
    destipaddr[4];
  /* The ICMP and IP headers. */
  /* ICMP (echo) header. */
  u8_t type, icode;
  u16_t icmpchksum;
  u16_t id, seqno;  
};


/* The UDP and IP headers. */
struct udpip_hdr {
  /* IP header. */
   u8_t vhl,
    tos,          
     len[2],       
     ipid[2],        
     ipoffset[2],  
     ttl,          
     proto;     
   u16_t ipchksum;
  u8_t srcipaddr[4], 
    destipaddr[4];
  
  /* UDP header. */
  u16_t srcport,
    destport;
  u16_t udplen;
  u16_t udpchksum;
};

#define ETHBUF    ((struct eth_hdr *)&uip_buf[0])
#define IPBUF     ((struct ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UDPBUF  ((struct udpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define ICMPBUF ((struct icmpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define TCPBUF  ((struct tcpip_hdr *)&uip_buf[UIP_LLH_LEN])


/*---------------------------------------------------------------------------*/
static void
tcpflags(unsigned char flags, char *flagsstr)
{
  if(flags & TCP_FIN) {
    *flagsstr++ = 'F';
  }
  if(flags & TCP_SYN) {
    *flagsstr++ = 'S';
  }
  if(flags & TCP_RST) {
    *flagsstr++ = 'R';
  }
  if(flags & TCP_ACK) {
    *flagsstr++ = 'A';
  }
  if(flags & TCP_URG) {
    *flagsstr++ = 'U';
  }

  *flagsstr = 0;
}
/*---------------------------------------------------------------------------*/
int
tcpdump_print(char *buf, u16_t buflen)
{
  char flags[8];
  if(IPBUF->proto == UIP_PROTO_ICMP) {
    if(ICMPBUF->type == ICMP_ECHO) {
      return sprintf(buf, "%d.%d.%d.%d %d.%d.%d.%d ping",
		     IPBUF->srcipaddr[0], IPBUF->srcipaddr[1],
		     IPBUF->srcipaddr[2], IPBUF->srcipaddr[3],
		     IPBUF->destipaddr[0], IPBUF->destipaddr[1],
		     IPBUF->destipaddr[2], IPBUF->destipaddr[3]);
    } else if(ICMPBUF->type == ICMP_ECHO_REPLY) {
      return sprintf(buf, "%d.%d.%d.%d %d.%d.%d.%d pong",
		     IPBUF->srcipaddr[0], IPBUF->srcipaddr[1],
		     IPBUF->srcipaddr[2], IPBUF->srcipaddr[3],
		     IPBUF->destipaddr[0], IPBUF->destipaddr[1],
		     IPBUF->destipaddr[2], IPBUF->destipaddr[3]);
    }
  } else if(IPBUF->proto == UIP_PROTO_UDP) {
    return sprintf(buf, "%d.%d.%d.%d.%d %d.%d.%d.%d.%d UDP",
		   IPBUF->srcipaddr[0], IPBUF->srcipaddr[1],
		   IPBUF->srcipaddr[2], IPBUF->srcipaddr[3],
		   htons(UDPBUF->srcport),
		   IPBUF->destipaddr[0], IPBUF->destipaddr[1],
		   IPBUF->destipaddr[2], IPBUF->destipaddr[3],
		   htons(UDPBUF->destport));
  } else if(IPBUF->proto == UIP_PROTO_TCP) {
    tcpflags(TCPBUF->flags, flags);
    return sprintf(buf, "%d.%d.%d.%d.%d %d.%d.%d.%d.%d %s",
		   IPBUF->srcipaddr[0], IPBUF->srcipaddr[1],
		   IPBUF->srcipaddr[2], IPBUF->srcipaddr[3],
		   htons(TCPBUF->srcport),
		   IPBUF->destipaddr[0], IPBUF->destipaddr[1],
		   IPBUF->destipaddr[2], IPBUF->destipaddr[3],
		   htons(TCPBUF->destport),
		   flags);  
  }
}
/*---------------------------------------------------------------------------*/
