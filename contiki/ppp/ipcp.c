/*															
 *---------------------------------------------------------------------------
 * ipcp.c - PPP IPCP (intrnet protocol) Processor/Handler
 *
 *---------------------------------------------------------------------------
 *
 * Version 
 *		0.1 Original Version Jun 3, 2000
 *	
 *---------------------------------------------------------------------------
 *        
 * Copyright (C) 2000, Mycal Labs www.mycal.com
 * 
 *---------------------------------------------------------------------------
 */
/*
 * Copyright (c) 2003, Mike Johnson, Mycal Labs, www.mycal.net
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
 *      This product includes software developed by Mike Johnson/Mycal Labs
 *		www.mycal.net.
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
 * This file is part of the Mycal Modified uIP TCP/IP stack.
 *
 * $Id: ipcp.c,v 1.2 2004/08/29 15:11:45 oliverschmidt Exp $
 *
 */

/*			*/ 
/* include files 	*/
/*			*/ 

#if 1
#define DEBUG1(x)
#else
#include <stdio.h>
#define DEBUG1(x) printf x
#endif

#include "uip.h"
/*#include "time.h"*/
#include "ipcp.h"
#include "ppp.h"
#include "ahdlc.h"

#define TIMER_expire()
#define TIMER_set()
#define TIMER_timeout(x) 1

#ifdef IPCP_GET_PEER_IP
uip_ipaddr_t	peer_ip_addr;
#endif

#ifdef IPCP_GET_PRI_DNS
uip_ipaddr_t	pri_dns_addr;
#endif

#ifdef IPCP_GET_SEC_DNS
uip_ipaddr_t	sec_dns_addr;
#endif

/*
 * Local IPCP state
 */
u8_t ipcp_state;

/*
 * in the future add copression protocol and name servers (possibly for servers only)
 */
u8_t ipcplist[] = {0x3, 0};	

/*---------------------------------------------------------------------------*/
/*void
printip(uip_ipaddr_t ip)
{
DEBUG1((" %d.%d.%d.%d ",ip.ipb1,ip.ip8[1],ip.ip8[2],ip.ip8[3]));
    }*/
#define printip(x)
/*---------------------------------------------------------------------------*/
void
ipcp_init(void)
{
  DEBUG1(("ipcp init\n"));
  ipcp_state = 0;
  ppp_retry = 0;
  our_ipaddr.ip16[0] = our_ipaddr.ip16[1] = 0;
}
/*---------------------------------------------------------------------------*/
/*
 * IPCP RX protocol Handler
 */
void
ipcp_rx(u8_t *buffer, u16_t count)
{
  u8_t *bptr = buffer;
  IPCPPKT *pkt=(IPCPPKT *)buffer;
  u16_t len;

  DEBUG1(("IPCP len %d\n",count));
	
  switch(*bptr++) {
  case CONF_REQ:
    /* parce request and see if we can ACK it */
    ++bptr;
    len = (*bptr++ << 8);
    len |= *bptr++;
    /* len-=2; */

    DEBUG1(("check lcplist\n"));
    if(scan_packet(IPCP, ipcplist, buffer, bptr, (u16_t)(len - 4))) {
      DEBUG1(("option was bad\n"));
    } else {
      DEBUG1(("IPCP options are good\n"));
      /*
       * Parse out the results
       */
      /* lets try to implement what peer wants */
      /* Reject any protocol not */
      /* Error? if we we need to send a config Reject ++++ this is
	 good for a subroutine*/
      /* All we should get is the peer IP address */
      if(IPCP_IPADDRESS == *bptr++) {
	/* dump length */
	++bptr;
#ifdef IPCP_GET_PEER_IP
	peer_ip_addr.ip8[0] = *bptr++;
	peer_ip_addr.ip8[1] = *bptr++;
	peer_ip_addr.ip8[2] = *bptr++;
	peer_ip_addr.ip8[3] = *bptr++;
	DEBUG1(("Peer IP "));
	/*	printip(peer_ip_addr);*/
	DEBUG1(("\n"));
#else
	bptr += 4;
#endif
      } else {
	DEBUG1(("HMMMM this shouldn't happen IPCP1\n"));
      }
      
#if 0			
      if(error) {
	/* write the config NAK packet we've built above, take on the header */
	bptr = buffer;
	*bptr++ = CONF_NAK;		/* Write Conf_rej */
	*bptr++;
	/*tptr++;*/  /* skip over ID */

	/* Write new length */
	*bptr++ = 0;
	*bptr = tptr - buffer;
	
	/* write the reject frame */
	DEBUG1(("Writing NAK frame \n"));
	ahdlc_tx(IPCP, buffer, (u16_t)(tptr - buffer));
	DEBUG1(("- End NAK Write frame\n"));
	
      } else {
      }
#endif
      /*
       * If we get here then we are OK, lets send an ACK and tell the rest
       * of our modules our negotiated config.
       */
      ipcp_state |= IPCP_RX_UP;
      DEBUG1(("Send IPCP ACK!\n"));
      bptr = buffer;
      *bptr++ = CONF_ACK;		/* Write Conf_ACK */
      bptr++;				/* Skip ID (send same one) */
      /*
       * Set stuff
       */
      /* ppp_flags |= tflag; */
      DEBUG1(("SET- stuff -- are we up? c=%d dif=%d \n", count, (u16_t)(bptr-buffer)));
	
      /* write the ACK frame */
      DEBUG1(("Writing ACK frame \n"));
      /* Send packet ahdlc_txz(procol,header,data,headerlen,datalen);	*/
      ahdlc_tx(IPCP, 0, buffer, 0, count /*bptr-buffer*/);
      DEBUG1(("- End ACK Write frame\n"));
	
      /* expire the timer to make things happen after a state change */
      /*timer_expire(); */
	
      /*			} */
    }
    break;
  case CONF_ACK:			/* config Ack */
    DEBUG1(("CONF ACK\n"));
    /*
     * Parse out the results
     *
     * Dump the ID and get the length.
     */
    /* dump the ID */
    bptr++;

    /* get the length */
    len = (*bptr++ << 8);
    len |= *bptr++;
#if 0
    /* Parse ACK and set data */
    while(bptr < buffer + len) {
      switch(*bptr++) {
      case IPCP_IPADDRESS:
	/* dump length */
	bptr++;		
	ipaddr.ip8[0] = *bptr++;
	ipaddr.ip8[1] = *bptr++;
	ipaddr.ip8[2] = *bptr++;
	ipaddr.ip8[3] = *bptr++;
	break;
      case IPCP_PRIMARY_DNS:
	bptr++;
	pri_dns_addr.ip8[0] = *bptr++;
	pri_dns_addr.ip8[1] = *bptr++;
	pri_dns_addr.ip8[2] = *bptr++;
	pri_dns_addr.ip8[3] = *bptr++;
	break;
      case IPCP_SECONDARY_DNS:
	bptr++;
	sec_dns_addr.ip8[0] = *bptr++;
	sec_dns_addr.ip8[1] = *bptr++;
	sec_dns_addr.ip8[2] = *bptr++;
	sec_dns_addr.ip8[3] = *bptr++;
	break;
      default:
	DEBUG1(("IPCP CONFIG_ACK problem1\n"));
      }
    }
#endif
    ipcp_state |= IPCP_TX_UP;
    /*ipcp_state &= ~IPCP_RX_UP;*/
    DEBUG1(("were up! \n"));
    printip(our_ipaddr);
#ifdef IPCP_GET_PRI_DNS
    printip(pri_dns_addr);
#endif
#ifdef IPCP_GET_SEC_DNS
    printip(sec_dns_addr);
#endif
    DEBUG1(("\n"));
		
    /* expire the timer to make things happen after a state change */
    TIMER_expire();
    break;
  case CONF_NAK:			/* Config Nack */
    DEBUG1(("CONF NAK\n"));
    /* dump the ID */
    bptr++;
    /* get the length */
    len = (*bptr++ << 8);
    len |= *bptr++;

    /* Parse ACK and set data */
    while(bptr < buffer + len) {
      switch(*bptr++) {
      case IPCP_IPADDRESS:
	/* dump length */
	bptr++;
	our_ipaddr.ip8[0] = *bptr++;
	our_ipaddr.ip8[1] = *bptr++;
	our_ipaddr.ip8[2] = *bptr++;
	our_ipaddr.ip8[3] = *bptr++;
	break;
#ifdef IPCP_GET_PRI_DNS
      case IPCP_PRIMARY_DNS:
	bptr++;
	pri_dns_addr.ip8[0] = *bptr++;
	pri_dns_addr.ip8[1] = *bptr++;
	pri_dns_addr.ip8[2] = *bptr++;
	pri_dns_addr.ip8[3] = *bptr++;
	break;
#endif
#ifdef IPCP_GET_SEC_DNS
      case IPCP_SECONDARY_DNS:
	bptr++;
	sec_dns_addr.ip8[0] = *bptr++;
	sec_dns_addr.ip8[1] = *bptr++;
	sec_dns_addr.ip8[2] = *bptr++;
	sec_dns_addr.ip8[3] = *bptr++;
	break;
#endif
      default:
	DEBUG1(("IPCP CONFIG_ACK problem 2\n"));
      }
    }
    ppp_id++;
    printip(our_ipaddr);
#ifdef IPCP_GET_PRI_DNS
    printip(pri_dns_addr);
#endif
#ifdef IPCP_GET_PRI_DNS
    printip(sec_dns_addr);
#endif
    DEBUG1(("\n"));
    /* expire the timer to make things happen after a state change */
    TIMER_expire();
    break;
  case CONF_REJ:			/* Config Reject */
    DEBUG1(("CONF REJ\n"));
    /* Remove the offending options*/
    ppp_id++;
    /* dump the ID */
    bptr++;
    /* get the length */
    len = (*bptr++ << 8);
    len |= *bptr++;

    /* Parse ACK and set data */
    while(bptr < buffer + len) {
      switch(*bptr++) {
      case IPCP_IPADDRESS:
	ipcp_state |= IPCP_IP_BIT;
	bptr += 5;
	break;
#ifdef IPCP_GET_PRI_DNS
      case IPCP_PRIMARY_DNS:
	ipcp_state |= IPCP_PRI_DNS_BIT;
	bptr += 5;
	break;
#endif
#ifdef IPCP_GET_PRI_DNS
      case IPCP_SECONDARY_DNS:
	ipcp_state |= IPCP_SEC_DNS_BIT;
	bptr += 5;
	break;
#endif
      default:
	DEBUG1(("IPCP this shoudln't happen 3\n"));
      }
    }
    /* expire the timer to make things happen after a state change */
    /*timer_expire(); */
    break;
  default:
    DEBUG1(("-Unknown 4\n"));
  }
}
  
/*---------------------------------------------------------------------------*/
void
ipcp_task(u8_t *buffer)
{
  u8_t *bptr;
  u16_t	t;
  IPCPPKT *pkt;

  /* IPCP tx not up and hasn't timed out then lets see if we need to
     send a request */
  if(!(ipcp_state & IPCP_TX_UP) && !(ipcp_state & IPCP_TX_TIMEOUT)) {
    /* Check if we have a request pending */
    /*t=get_seconds()-ipcp_tx_time;*/
    if(TIMER_timeout(IPCP_TIMEOUT)) {
      /*
       * No pending request, lets build one
       */
      pkt=(IPCPPKT *)buffer;		
      
      /* Configure-Request only here, write id */
      pkt->code = CONF_REQ;
      pkt->id = ppp_id;
			
      bptr = pkt->data;       

      /*
       * Write options, we want IP address, and DNS addresses if set.
       */
			
      /* Write zeros for IP address the first time */
      *bptr++ = IPCP_IPADDRESS;
      *bptr++ = 0x6;
      *bptr++ = our_ipaddr.ip8[0];
      *bptr++ = our_ipaddr.ip8[1];
      *bptr++ = our_ipaddr.ip8[2];
      *bptr++ = our_ipaddr.ip8[3];

#ifdef IPCP_GET_PRI_DNS
      if(!(ipcp_state & IPCP_PRI_DNS_BIT)) {
	/* Write zeros for IP address the first time */
	*bptr++ = IPCP_PRIMARY_DNS;
	*bptr++ = 0x6;
	*bptr++ = pri_dns_addr.ip8[0];
	*bptr++ = pri_dns_addr.ip8[1];
	*bptr++ = pri_dns_addr.ip8[2];
	*bptr++ = pri_dns_addr.ip8[3];
      }
#endif
#ifdef IPCP_GET_SEC_DNS
      if(!(ipcp_state & IPCP_SEC_DNS_BIT)) {
	/* Write zeros for IP address the first time */
	*bptr++ = IPCP_SECONDARY_DNS;
	*bptr++ = 0x6;
	*bptr++ = sec_dns_addr.ip8[0];
	*bptr++ = sec_dns_addr.ip8[1];
	*bptr++ = sec_dns_addr.ip8[2];
	*bptr++ = sec_dns_addr.ip8[3];
      }
#endif
      /* Write length */
      t = bptr - buffer;
      /* length here -  code and ID + */
      pkt->len = htons(t);	
      
      DEBUG1(("\n**Sending IPCP Request packet\n"));
      
      /* Send packet ahdlc_txz(procol,header,data,headerlen,datalen); */
      ahdlc_tx(IPCP, 0, buffer, 0, t);

      /* Set timer */
      /*ipcp_tx_time=get_seconds();*/
      TIMER_set();
      /* Inc retry */
      /*ipcp_retry++;*/
      ppp_retry++;
      /*
       * Have we timed out? (combide the timers?)
       */
      if(ppp_retry > IPCP_RETRY_COUNT)
	ipcp_state &= IPCP_TX_TIMEOUT;	
    }
  }
}
/*---------------------------------------------------------------------------*/ 
