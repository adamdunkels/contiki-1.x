/**
 * \file
 * Device driver and packet framing for the RFM-TR1001 radio module.
 * \author FU Berlin (original implementation)
 * \author Adam Dunkels <adam@sics.se>
 *
 * This file implements a device driver for the RFM-TR1001 radio
 * tranciever as well as a simple packet framing format, simimilar to
 * PPP (AHDLC) framing.
 *
 * \note A lot of the stuff in this file is currently a "hack" form
 * (particularly the "MAC" stuff and the ackowledgements)!
 */

#include "tr1001.h"
#include "uip.h"
#include "uip-fw.h"

#include "sensors.h"

#include <io.h>
#include <signal.h>


#define RXSTATE_READY   0  /**< \internal Receive state: ready for a
			      new packet. */
#define RXSTATE_START1  1  /**< \internal Receive state: first   start
			      byte read, waiting for second. */
#define RXSTATE_START2  2  /**< \internal Receive state: second start
			      byte read, waiting for header. */
#define RXSTATE_RAWHEADER 3  /**< \internal Receive state: reading raw
			      packet header, without encoding. */
#define RXSTATE_HEADER1 4  /**< \internal Receive state: reading
			      packet header, non-negative byte. */
#define RXSTATE_HEADER2 5  /**< \internal Receive state: reading
			      packet header, negative byte. */
#define RXSTATE_RAWDATA 6  /**< \internal Receive state: reading raw
			      packet data, without encoding. */
#define RXSTATE_DATA1   7  /**< \internal Receive state: reading
			      packet data, non-negative byte. */
#define RXSTATE_DATA2   8  /**< \internal Receive state: reading
			      packet data, negative byte. */
#define RXSTATE_ERROR   9  /**< \internal Receive state: error in
			      packet reception. */
#define RXSTATE_FULL    10  /**< \internal Receive state: a full packet
			      has been received. */

#define NEG(b) (0xff - b)  /**< \internal Logical negation of an 8-bit
			      quantity. */

/**
 * \internal
 * The buffer which holds incoming data.
 */
#define RXBUFSIZE UIP_BUFSIZE
static unsigned char rxbuf[RXBUFSIZE];

/**
 * \internal
 * A pointer into the rxbuf buffer which points to the next available byte. 
 */
static unsigned short rxpos;

/**
 * \internal
 * The length of the packet that currently is being received.
 */
static unsigned short rxlen;

/**
 * \internal
 * The reception state.
 */
static unsigned char rxstate = RXSTATE_READY;


/**
 * \internal
 * The structure of the packet header.
 */
struct tr1001_hdr {
  u8_t type;      /**< The packet type. */
  u8_t id;        /**< A packet identifier. */
  u8_t len[2];    /**< The 16-bit length of the packet in network byte
		     order. */
};

/**
 * \internal The length of the packet header.
 */
#define TR1001_HDRLEN 4

/**
 * \internal 
 * The "data" packet type.
 */
#define TR1001_TYPE_DATA 1

/**
 * \internal 
 * The "acknowledgement" packet type.
 */
#define TR1001_TYPE_ACK  2

/**
 * \internal
 * Determines if the negencoding should be used or not. This
 * causes each byte which is sent out to be sent twice, first normally
 * and second with all bits inverted. */
#define TR1001_CONF_NEGENCODING 1

/**
 * \internal
 * The incremental packet identifier.
 */
static u8_t packet_id;

/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Turn on data transmission in On-Off-Keyed mode.
 */
/*------------------------------------------------------------------------------*/
static void
txook(void)
{
  P3SEL = 0xf0;
  P5OUT |= 0x40;
  P5OUT &= 0x7f;
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Turn on data reception for the radio tranceiver.
 */
/*------------------------------------------------------------------------------*/
static void
rxon(void)
{
  P3SEL = 0xe0; 
  P5OUT |= 0xc0; 
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Clear the recevie buffer and reset the receiver state.
 */
/*------------------------------------------------------------------------------*/
static void
rxclear(void)
{
  rxpos = 0;
  rxstate = RXSTATE_READY;
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Send a byte of data over the radio.
 *
 * \param b The byte to be sent.
 */
/*------------------------------------------------------------------------------*/
static void
send(unsigned char b)
{
  /* Wait until the USART0 TX buffer is ready. */
  while((IFG1 & UTXIFG0) == 0);

  /* Send the byte. */
  TXBUF0 = b;
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Send a byte of data and its logical negation (all bits inverted)
 * over the radio.
 *
 * \param b The byte to be sent.
 */
/*------------------------------------------------------------------------------*/
static void
send2(unsigned char b)
{
  send(b);
#if TR1001_CONF_NEGENCODING
  send(NEG(b));
#endif /* TR1001_CONF_NEGENCODING */
}
/*------------------------------------------------------------------------------*/
/**
 * Set the transmission power of the tranciever.
 *
 * The sensor board is equipped with a DS1804 100 position trimmer
 * potentiometer which is used to set the transmission input current
 * to the radio tranciever chip, thus setting the transmission power
 * of the radio tranciever.
 *
 * This function sets the trimmer potentiometer to a value between 1
 * and 100.
 *
 * \param p The power of the tranciever, between 1 (lowest) and 100
 * (highest).
 */
/*------------------------------------------------------------------------------*/
void
tr1001_set_txpower(unsigned char p)
{
  int i;
  
  /* Clamp maximum power. */
  if(p > 100) {
    p = 100;
  }

  /* First, run the potentiometer down to zero so that we know the
     start value of the potentiometer. */  
  P2OUT &= 0xDF;                                /* P25 = 0 (down selected) */
  P2OUT &= 0xBF;                                /* P26 = 0 (chipselect on) */
  for(i = 0; i < 102; ++i) {
    P2OUT &= 0xEF;                              /* P24 = 0 (inc) */
    P2OUT |= 0x10;    
  }
  
  /* Now, start to increase the value of the potentiometer until it
     reaches the desired value.*/
  
  P2OUT |= 0x20;                                /* P25 = 1 (up selected) */
  for(i = 0; i < p; ++i) {
    P2OUT &= 0xEF;                              /* P24 = 0 (inc) */
    P2OUT |= 0x10;    
  }
  P2OUT |= 0x40;                                /* P26 = 1 (chipselect off) */
}
/*------------------------------------------------------------------------------*/
/**
 * Initialize the radio tranciever.
 *
 * Turns on reception of bytes and installs the receive interrupt
 * handler.
 */
/*------------------------------------------------------------------------------*/
void
tr1001_init(void)
{
  /* Turn on reception. */
  rxon();

  /* Reset reception state. */
  rxclear();

    
  UCTL0 = CHAR;                         /* 8-bit character */
  UTCTL0 = SSEL1;                       /* UCLK = SMCLK */

#define TR1001_SPEED 19200
  
  if(TR1001_SPEED == 19200) {
    /* Set TR1001 to 19200 */
    UBR00 = 0x80;                         /* 2,457MHz/19200 = 128 -> 0x80 */
    UBR10 = 0x00;                         /* */
    UMCTL0 = 0x00;                        /* no modulation  */
  } else if(TR1001_SPEED == 38400) {
    /* Set TR1001 to 38400 */
    UBR00 = 0x40;                         /* 2,457MHz/38400 = 64 -> 0x40 */
    UBR10 = 0x00;                         /* */
    UMCTL0 = 0x00;                        /* no modulation  */
  } else if(TR1001_SPEED == 57600) {
    UBR00 = 0x2a;                         /* 2,457MHz/57600 = 42.7 -> 0x2A */
    UBR10 = 0x00;                         /* */
    UMCTL0 = 0x5b;                        /* */ 
  } else if(TR1001_SPEED == 115200) {
    UBR00 = 0x15;                         /* 2,457MHz/115200 = 21.4 -> 0x15 */
    UBR10 = 0x00;                         /* */
    UMCTL0 = 0x4a;                        /* */ 
  }

#if 0
  /* 19200 bps */
  UBR00 = 0x80;                         /* 2,457MHz/19200 = 128 -> 0x80 */
  UBR10 = 0x00;                         /* */
  UMCTL0 = 0x00;                        /* no modulation */
#endif /* 0 */
  
  ME1 |= UTXE0 + URXE0;                 /* Enable USART0 TXD/RXD */
  
  /* Turn on receive interrupt. */
  IE1 |= URXIE0;
}
/*------------------------------------------------------------------------------*/
/**
 * Check if an incoming packet has been received.
 *
 * This function checks the receive buffer to see if an entire packet
 * has been received. The actual reception is handled by an interrupt
 * handler.
 * 
 * \return The length of the received packet, or 0 if no packet has
 * been received.
 */
/*------------------------------------------------------------------------------*/
unsigned short
tr1001_poll(void)
{
  unsigned short tmplen;
  
  if(rxstate == RXSTATE_FULL) {
    if(rxlen > UIP_BUFSIZE - UIP_LLH_LEN) {
      rxlen = UIP_BUFSIZE - UIP_LLH_LEN;
    }
    memcpy(&uip_buf[UIP_LLH_LEN], rxbuf + TR1001_HDRLEN, rxlen);
    tmplen = rxlen;
    rxclear();
    return tmplen;
  }

  if(rxstate == RXSTATE_ERROR) {
    blink();
    rxclear();
  }
  
  return 0;
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Interrupt handler for the data reception.
 */
/*------------------------------------------------------------------------------*/
interrupt (UART0RX_VECTOR)
     tr1001_rxhandler(void)
{
  unsigned char c;

  c = RXBUF0;

  switch(rxstate) {
  case RXSTATE_ERROR:
    /* Do nothing, wait until error has been hendled. */
    break;
  case RXSTATE_READY:
    if(c == 0xff) {
      rxstate = RXSTATE_START1;
    }
    break;
  case RXSTATE_START1:
    if(c == 0x01) {
      rxstate = RXSTATE_START2;
    } else {
      rxstate = RXSTATE_READY;
    }
    break;
  case RXSTATE_START2:
    if(c == 0x7f) {
#if TR1001_CONF_NEGENCODING
      rxstate = RXSTATE_HEADER1;
#else /* TR1001_CONF_NEGENCODING */
      rxstate = RXSTATE_RAWHEADER;
#endif /* TR1001_CONF_NEGENCODING */
    } else {
      rxstate = RXSTATE_READY;
    }
    break;
  case RXSTATE_RAWHEADER:
    rxbuf[rxpos] = c;
    ++rxpos;
    if(rxpos == TR1001_HDRLEN) {
      rxlen = ((((struct tr1001_hdr *)rxbuf)->len[0] << 8) +
	       ((struct tr1001_hdr *)rxbuf)->len[1]);
      rxstate = RXSTATE_RAWDATA;
    }
    break;
  case RXSTATE_HEADER1:
    /* Receive a data byte of the header. */
    rxbuf[rxpos] = c;
    rxstate = RXSTATE_HEADER2;
    break;
  case RXSTATE_HEADER2:
    /* We should have received the bit-wise negation of the previously
       received byte, or else something has gone wrong and we set the
       error flag. */
    if(NEG(c) == rxbuf[rxpos]) {
      ++rxpos;
      if(rxpos == TR1001_HDRLEN) {
	rxlen = ((((struct tr1001_hdr *)rxbuf)->len[0] << 8) +
		 ((struct tr1001_hdr *)rxbuf)->len[1]);
	rxstate = RXSTATE_DATA1;
      } else {
	rxstate = RXSTATE_HEADER1;
      }
    } else {
      rxstate = RXSTATE_ERROR;
    }
    break;
  case RXSTATE_DATA1:
    /* Receive a data byte. */
    rxbuf[rxpos] = c;
    rxstate = RXSTATE_DATA2;
    break;
  case RXSTATE_DATA2:
    /* We should have received the bit-wise negation of the previously
       received byte, or else something has gone wrong and we set the
       error flag. */
    if(NEG(c) == rxbuf[rxpos]) {
      ++rxpos;
      if(rxpos == rxlen + TR1001_HDRLEN) {
	rxstate = RXSTATE_FULL;
      } else if(rxpos > sizeof(rxbuf)) {
	rxstate = RXSTATE_ERROR;
      } else {
	rxstate = RXSTATE_DATA1;
      }
    } else {
      rxstate = RXSTATE_ERROR;
    }
    break;
  case RXSTATE_RAWDATA:
    rxbuf[rxpos] = c;
    ++rxpos;
    if(rxpos == rxlen + TR1001_HDRLEN) {
      rxstate = RXSTATE_FULL;
    } else if(rxpos > sizeof(rxbuf)) {
      rxstate = RXSTATE_ERROR;
    }
    break;
  case RXSTATE_FULL:
    /* Just drop the incoming byte. */
    break;
  default:
    /* Just drop the incoming byte. */
    rxstate = RXSTATE_READY;
    break;
  }
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 *
 * A stupid function which causes a small delay based on its argument.
 *
 * It is a hack, not intended for "real" use.
 *
 */
/*------------------------------------------------------------------------------*/
static void
delay_hack(int d)
{
  int i, j = 2;
  
  for(i = 0; i < d; ++i) {
    j = j * j;
  }
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 *
 * Prepare a transmission.
 *
 * This function does the necessary setup before a packet can be sent
 * out.  
 */
/*------------------------------------------------------------------------------*/
static void
prepare_transmission(void)
{
  unsigned short i, laststate;
    
  /* Delay the transmission for a while. The length of the delay is
     based on the lowest bits of the battery sensor, which seems to be
     jumping up and down somewhat unpredictably (but I might very well
     be wrong). */
  /*  P2OUT &= 0xFE;*/
  delay_hack(400 * (sensors_battery & 0x0f));
  /*  P2OUT |= 0x01;  */
  
  /* First check that we don't currently are receiveing a packet, and
     if so we wait until the reception has been completed. Reception
     is done with interrupts so it is OK for us to wait in a while()
     loop. */
  i = 0;
  laststate = rxstate;
  while(rxstate != RXSTATE_READY &&
	rxstate != RXSTATE_FULL &&
	rxstate != RXSTATE_ERROR) {

    /* Make sure that we don't stay too long in the same rxstate
       (which would indicate that the receiving interrupt somehow has
       locked up). */
    if(laststate == rxstate) {
      ++i;
      if(i == 0xffff) {
	rxstate = RXSTATE_ERROR;
	break;
      }
    } else {
      i = 0;
    }
    laststate = rxstate;
  }

  /* Turn on OOK mode with transmission. */
  txook();

  /* Delay for a while to let the transmitted settle in its new
     state. */
  delay_hack(400);
  
}
/*------------------------------------------------------------------------------*/
/**
 * Send a packet and wait for an acknowledgement.
 *
 * This function calls tr1001_send() to send out a packet, and waits
 * for an acknowledgement from the receiver.
 *
 */
/*------------------------------------------------------------------------------*/
u8_t
tr1001_send_acked(void)
{
  unsigned short tmplen, count;
  struct tr1001_hdr *ackhdr = (struct tr1001_hdr *)rxbuf;
  ek_clock_t start;
  
  
  tr1001_send();

  start = ek_clock();
  count = 0;
  P2OUT &= 0xFB;
  /* Block while checking if a packet has arrived. */
  while((volatile u8_t)rxstate != RXSTATE_FULL &&
	(volatile u8_t)rxstate != RXSTATE_ERROR) {
    /* Wait no more than one second. */
    if((ek_clock_t)(ek_clock() - start) > (ek_clock_t)CLK_TCK) {
      break;
    }
  }
  P2OUT |= 0x04;
    
  
  if(rxstate == RXSTATE_FULL) {
    if(ackhdr->type == TR1001_TYPE_ACK &&
       ackhdr->id == packet_id) {
      beep();
      rxclear();
      return UIP_FW_OK;
    }
  }

  if(rxstate == RXSTATE_ERROR) {
    rxclear();
    blink();
  }
  return UIP_FW_DROPPED;
}
/*------------------------------------------------------------------------------*/
/**
 * Send out a packet from the uip_buf buffer.
 *
 * This function causes a packet to be sent out after a small random
 * delay, but without doing any MAC layer collision detection or
 * back-offs. The packet is sent with a 4 byte header that contains a
 * a "type" identifier, an 8-bit packet ID field and the length of the
 * packet in network byte order.
 */
/*------------------------------------------------------------------------------*/
u8_t
tr1001_send(void)
{
  u8_t *hdr;
  u16_t hdrlen;
  u8_t *data;
  u16_t datalen;
  int i;

  hdr = &uip_buf[UIP_LLH_LEN];
  hdrlen = UIP_TCPIP_HLEN;
  data = uip_appdata;
  if(uip_len < UIP_TCPIP_HLEN) {
    datalen = 0;
  } else {
    datalen = uip_len - UIP_TCPIP_HLEN;
  }

  /* Prepare the transmission. */
  prepare_transmission();

  /* Send first preamble byte. */
  send(0xaa);

  /* Send second preamble byte. */
  send(0xaa);

  /* Send sync byte. */
  send(0x0ff);

  /* Send first start byte. */
  send(0x01);

  /* Send second start byte. */
  send(0x07f);

  /* Send packet header. */
  send2(TR1001_TYPE_DATA);
  send2(++packet_id);
  send2(uip_len >> 8);
  send2(uip_len & 0xff);

  /* Send packet data. */
  for(i = 0; i < hdrlen; ++i) {
    send2(hdr[i]);
  }
  for(i = 0;i < datalen; ++i) {
    send2(data[i]);
  }

  /* Send trailing bytes. */
  send(0xaa);
  send(0xaa);

  /* Turn on reception again. */
  rxon();

  return UIP_FW_OK;
}
/*------------------------------------------------------------------------------*/
/**
 * Acknowledge a received packet.
 *
 * This function sends out an acknowledgement packet for the
 * previously received packet, which must be present in the uip_buf
 * buffer.
 *
 * \retval UIP_FW_OK The acknowledgement was successfully.
 * \retval UIP_FW_DROPPED The acknowledgement was dropped before transmission.
 */
/*------------------------------------------------------------------------------*/
u8_t
tr1001_ack(void)
{
  /* Prepare the transmission. */
  prepare_transmission();

  /* Send first preamble byte. */
  send(0xaa);

  /* Send second preamble byte. */
  send(0xaa);

  /* Send sync byte. */
  send(0x0ff);

  /* Send first start byte. */
  send(0x01);

  /* Send second start byte. */
  send(0x07f);

  /* Send packet header. */
  send2(TR1001_TYPE_ACK);
  send2(((struct tr1001_hdr *)uip_buf)->id);
  send2(0);
  send2(0);

  /* Send trailing bytes. */
  send(0xaa);
  send(0xaa);

  /* Turn on reception again. */
  rxon();

  /*  beep();*/
    
  return UIP_FW_OK;
  
}
/*------------------------------------------------------------------------------*/
