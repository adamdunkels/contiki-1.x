/*
 * uIP lan91c96 (smc9194) driver
 * Based on cs8900a driver, copyrighted (c) 2001, by Adam Dunkels
 * Copyright (c) 2003, Josef Soucek
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
 * 3. The name of the author may not be used to endorse or promote
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
 * $Id: lan91c96.c,v 1.4 2005/03/31 22:03:41 oliverschmidt Exp $
 *
 */

#include "lan91c96.h"
#include "uip.h"
#include "uip_arp.h"

#include <stdio.h>

#define ETHBASE 0xc080

#define ETHBSR     ETHBASE+0x0e	 /* Bank select register             R/W (2B) */

/* Register bank 0 */

#define ETHTCR     ETHBASE       /* Transmition control register     R/W (2B) */
#define ETHEPHSR   ETHBASE+2     /* EPH status register              R/O (2B) */
#define ETHRCR     ETHBASE+4     /* Receive control register         R/W (2B) */
#define ETHECR     ETHBASE+6     /* Counter register                 R/O (2B) */
#define ETHMIR     ETHBASE+8     /* Memory information register      R/O (2B) */
#define ETHMCR     ETHBASE+0x0a  /* Memory Config. reg.    +0 R/W +1 R/O (2B) */

/* Register bank 1 */

#define ETHCR      ETHBASE       /* Configuration register           R/W (2B) */
#define ETHBAR     ETHBASE+2     /* Base address register            R/W (2B) */
#define ETHIAR     ETHBASE+4     /* Individual address register      R/W (6B) */
#define ETHGPR     ETHBASE+0x0a  /* General address register         R/W (2B) */
#define ETHCTR     ETHBASE+0x0c  /* Control register                 R/W (2B) */

/* Register bank 2 */

#define ETHMMUCR   ETHBASE       /* MMU command register             W/O (1B) */
#define ETHAUTOTX  ETHBASE+1     /* AUTO TX start register           R/W (1B) */
#define ETHPNR     ETHBASE+2     /* Packet number register           R/W (1B) */
#define ETHARR     ETHBASE+3     /* Allocation result register       R/O (1B) */
#define ETHFIFO    ETHBASE+4     /* FIFO ports register              R/O (2B) */
#define ETHPTR     ETHBASE+6     /* Pointer register                 R/W (2B) */
#define ETHDATA    ETHBASE+8     /* Data register                    R/W (4B) */
#define ETHIST     ETHBASE+0x0c  /* Interrupt status register        R/O (1B) */
#define ETHACK     ETHBASE+0x0c  /* Interrupt acknowledge register   W/O (1B) */
#define ETHMSK     ETHBASE+0x0d  /* Interrupt mask register          R/W (1B) */

/* Register bank 3 */

#define ETHMT      ETHBASE       /* Multicast table                  R/W (8B) */
#define ETHMGMT    ETHBASE+8     /* Management interface             R/W (2B) */
#define ETHREV     ETHBASE+0x0a  /* Revision register                R/W (2B) */
#define ETHERCV    ETHBASE+0x0c  /* Early RCV register               R/W (2B) */

#define BANK(num) asm("lda #%b", num); asm("sta %w,x", ETHBSR);

extern u8_t lanslot;

static u8_t slot_index;
static u8_t packet_status;
static u16_t packet_length;


/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
void lan91c96_init(void)
{
  asm("lda %v", lanslot);
  asm("asl");
  asm("asl");
  asm("asl");
  asm("asl");
  asm("tax");

  /* Check if high byte is 0x33 */
  asm("lda %w,x", ETHBSR+1);
  asm("cmp #$33");
  asm("bne %g", L1);
  asm("stx %v", slot_index);

  /* Reset ETH card */
  BANK(0);
  asm("lda #%%10000000");        /* Software reset */
  asm("sta %w,x", ETHRCR+1);

  asm("lda #0");
  asm("sta %w,x", ETHRCR);
  asm("sta %w,x", ETHRCR+1);

  /* Enable transmit and receive */
  asm("lda #%%10000001");        /* Enable transmit TXENA, PAD_EN */
  asm("sta %w,x", ETHTCR);
  asm("lda #%%00000011");        /* Enable receive, strip CRC ??? */
  asm("sta %w,x", ETHRCR+1);

  BANK(1);
  asm("lda %w,x", ETHCR+1);
  asm("ora #%%00010000");        /* No wait (IOCHRDY) */
  asm("sta %w,x", ETHCR+1);

  asm("lda #%%00001001");        /* Auto release */
  asm("sta %w,x", ETHCTR+1);
  
  /* Set MAC address */
  asm("lda %v", uip_ethaddr);
  asm("sta %w,x", ETHIAR);
  asm("lda %v+1", uip_ethaddr);
  asm("sta %w,x", ETHIAR+1);
  asm("lda %v+2", uip_ethaddr);
  asm("sta %w,x", ETHIAR+2);
  asm("lda %v+3", uip_ethaddr);
  asm("sta %w,x", ETHIAR+3);
  asm("lda %v+4", uip_ethaddr);
  asm("sta %w,x", ETHIAR+4);
  asm("lda %v+5", uip_ethaddr);
  asm("sta %w,x", ETHIAR+5);

  BANK(2);
  asm("lda #%%00000000");        /* No interrupts */
  asm("sta %w,x", ETHMSK);

L1:
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
u16_t lan91c96_poll(void)
{
  asm("ldx %v", slot_index);
  asm("beq %g", L0);

  asm("lda %w,x", ETHIST);
  asm("and #%%00000001");        /* RCV INT */
  asm("bne %g", L1);

  /* No packet available */
L0:
  return 0;

L1:
  asm("lda #0");
  asm("sta %w,x", ETHPTR);
  asm("lda #%%11100000");        /* RCV,AUTO INCR.,READ */
  asm("sta %w,x", ETHPTR+1);

  asm("lda %w,x", ETHDATA);      /* Status word */
  asm("lda %w,x", ETHDATA);
  asm("sta %v", packet_status);  /* High byte only */

  asm("lda %w,x", ETHDATA);      /* Total number of bytes */
  asm("sta %v", packet_length);
  asm("lda %w,x", ETHDATA);
  asm("sta %v+1", packet_length);

  /* Last word contain 'last data byte' and 0x60 */
  /* or 'fill byte' and 0x40 */

  packet_length -= 6;            /* The packet contains 3 extra words */

  asm("lda %v", packet_status);
  asm("and #$10");
  asm("beq %g", L2);

  packet_length++;

L2:
  if(packet_length > UIP_BUFSIZE) {

    asm("ldx %v", slot_index);

    /* Remove and release RX packet from FIFO */ 
    asm("lda #%%10000000");
    asm("sta %w,x", ETHMMUCR);
    return 0;
  }

  asm("ldx %v", slot_index);

  asm("lda #<%v", uip_buf);
  asm("sta ptr1");
  asm("lda #>%v", uip_buf);
  asm("sta ptr1+1");

  asm("lda %v+1", packet_length);
  asm("sta tmp1");

  asm("ldy #0");
L3:
  asm("lda %w,x", ETHDATA);
  asm("sta (ptr1),y");
  asm("iny");
  asm("bne %g", L4);
  asm("inc ptr1+1");
L4:
  asm("cpy %v", packet_length);
  asm("bne %g", L3);
  asm("dec tmp1");
  asm("bpl %g", L3);

  /* Remove and release RX packet from FIFO */ 
  asm("lda #%%10000000");
  asm("sta %w,x", ETHMMUCR);
  return packet_length;
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
void lan91c96_send(void)
{
  /* First 14+40 (IP and TCP header) is send from uip_buf */
  /* than data from uip_appdata */

  asm("ldx %v", slot_index);

  asm("lda %v+1", uip_len);
  asm("ora #%%00100000");        /* Allocate memory for TX */
  asm("sta %w,x", ETHMMUCR);

  asm("ldy #8");                 /* Wait... */
L1:                              /* Wait for allocation ready */
  asm("lda %w,x", ETHIST);
  asm("and #%%00001000");        /* ALLOC INT */
  asm("bne %g", L2);
  asm("dey");
  asm("bne %g", L1);
  return;

L2:
  asm("lda #%%00001000");        /* Acknowledge int, is it necessary ??? */
  asm("sta %w,x", ETHACK);

  asm("lda %w,x", ETHARR);
  asm("sta %w,x", ETHPNR);       /* Set packet address */

  asm("lda #0");
  asm("sta %w,x", ETHPTR);
  asm("lda #%%01000000");        /* AUTO INCR. */
  asm("sta %w,x", ETHPTR+1);

  asm("lda #0");                 /* Status written by CSMA */
  asm("sta %w,x", ETHDATA);
  asm("sta %w,x", ETHDATA);

  asm("lda %v", uip_len);
  asm("and #$01");
  asm("beq %g", L3);

  packet_length = uip_len + 5;
  asm("jmp %g", L4);

L3:
  packet_length = uip_len + 6;   /* +6 for status word, length and ctl byte */

L4:
  asm("ldx %v", slot_index);

  asm("lda %v", packet_length);
  asm("sta %w,x", ETHDATA);
  asm("lda %v+1", packet_length);
  asm("sta %w,x", ETHDATA);

  /* Send 14+40=54 bytes of header */

  if(uip_len <= UIP_LLH_LEN + UIP_TCPIP_HLEN) {

    asm("ldx %v", slot_index);

    asm("ldy #0");
L5:
    asm("lda %v,y", uip_buf);
    asm("sta %w,x", ETHDATA);
    asm("iny");
    asm("cpy %v", uip_len);
    asm("bne %g", L5);

  } else {

    asm("ldx %v", slot_index);

    asm("ldy #0");
L6:
    asm("lda %v,y", uip_buf);
    asm("sta %w,x", ETHDATA);
    asm("iny");
    asm("cpy #%b", UIP_LLH_LEN + UIP_TCPIP_HLEN);
    asm("bne %g", L6);

    packet_length = uip_len - (UIP_LLH_LEN + UIP_TCPIP_HLEN);

    asm("ldx %v", slot_index);

    asm("lda %v", uip_appdata);  /* uip_appdata is pointer */
    asm("sta ptr1");
    asm("lda %v+1", uip_appdata);
    asm("sta ptr1+1");

    asm("ldy #0");
L7:
    asm("lda (ptr1),y");
    asm("sta %w,x", ETHDATA);
    asm("iny");
    asm("bne %g", L8);
    asm("inc ptr1+1");
L8:
    asm("cpy %v", packet_length);
    asm("bne %g", L7);
    asm("dec %v+1", packet_length);
    asm("bpl %g", L7);
  }

  asm("lda %v", packet_length);
  asm("and #$01");
  asm("beq %g", L9);

  asm("lda #%%00100000");
  asm("sta %w,x", ETHDATA);      /* Control byte */

  asm("lda #%%11000000");        /* ENQUEUE PACKET - transmit packet */
  asm("sta %w,x", ETHMMUCR);
  return;

L9:
  asm("lda #0");
  asm("sta %w,x", ETHDATA);      /* Fill byte */
  asm("sta %w,x", ETHDATA);      /* Control byte */

  asm("lda #%%11000000");        /* ENQUEUE PACKET - transmit packet */
  asm("sta %w,x", ETHMMUCR);
}
#pragma optimize(pop)
