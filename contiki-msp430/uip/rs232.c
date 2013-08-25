/**
 * \file
 * RS232 communication device driver for the MSP430.
 * \author Adam Dunkels <adam@sics.se>
 *
 * This file contains an RS232 device driver for the MSP430 microcontroller.
 *
 */

#include <io.h>
#include <signal.h>

#include "rs232.h"

void beep(void);

unsigned char lastchar, lastchar_changed;

#define RECVBUF_SIZE 32

static unsigned char recvbuf[RECVBUF_SIZE];
static unsigned char recvbufptr1 = 0,
  recvbufptr2 = 0;

interrupt(UART1RX_VECTOR) rs232_rx_usart1(void)
{
  volatile unsigned char dummy;
  
  /* Check status register for receive errors. */
  if(URCTL1 & RXERR) {
    /* Clear error flags by forcing a dummy read. */
    dummy = RXBUF1;
  } else {
    recvbuf[recvbufptr1] = RXBUF1;
    recvbufptr1 = (recvbufptr1 + 1) % RECVBUF_SIZE;
  }
}
/*-----------------------------------------------------------------------------------*/
/**
 * Initalize the RS232 port.
 *
 * \param none Not used.
 * \return An error value or RS_ERR_OK if initialization succeeded.
 */
/*-----------------------------------------------------------------------------------*/
unsigned char
rs232_init(char none)
{
  /* This code taken from the FU Berlin sources and reformatted. */
  
#define DELTA 600
  
  unsigned int compare, oldcapture = 0;
  unsigned int i;
  
  BCSCTL1 = 0xa4; /* ACLK is devided by 4. RSEL=6 no division for MCLK
		     and SSMCLK. XT2 is off. */
  
  BCSCTL2 = 0x00; /* Init FLL to desired frequency using the 32762Hz
		     crystal DCO frquenzy = 2,4576 MHz  */
  
  WDTCTL = WDTPW + WDTHOLD;             /* Stop WDT */
  BCSCTL1 |= DIVA1 + DIVA0;             /* ACLK = LFXT1CLK/8 */
  for(i = 0xffff; i > 0; --i);          /* Delay for XTAL to settle */

  /* Set DCO for 2,4576 Mhz: */
  CCTL2 = CCIS0 + CM0 + CAP;            /* Define CCR2, CAP, ACLK */
  TACTL = TASSEL1 + TACLR + MC1;        /* SMCLK, continous mode */
  while(1) {
    while((CCTL2 & CCIFG) != CCIFG);    /* Wait until capture occured! */
    CCTL2 &= ~CCIFG;                    /* Capture occured, clear flag */
    compare = CCR2;                     /* Get current captured SMCLK */
    compare = compare - oldcapture;     /* SMCLK difference */
    oldcapture = CCR2;                  /* Save current captured SMCLK */
    
    if(DELTA == compare) {
      break;                            /* if equal, leave "while(1)" */
    } else if(DELTA < compare) {        /* DCO is too fast, slow it down */
      DCOCTL--;
      if(DCOCTL == 0xFF) {              /* Did DCO role under? */
	BCSCTL1--;
      }
    } else {                            /* -> Select next lower RSEL */
      DCOCTL++;
      if(DCOCTL == 0x00) {              /* Did DCO role over? */
	BCSCTL1++;
      }
                                        /* -> Select next higher RSEL  */
    }
  }  
  CCTL2 = 0;                            /* Stop CCR2 function */
  TACTL = 0;                            /* Stop Timer_A */
  

  
  /*  Radio on 19200 Bit/s */
#if 0
  UCTL0 = CHAR;                         /* 8-bit character */
  UTCTL0 = SSEL1;                       /* UCLK = SMCLK */
  UBR00 = 0x80;                         /* 2,457MHz/19200 = 128 -> 0x80 */
  UBR10 = 0x00;                         /* */
  UMCTL0 = 0x00;                        /* no modulation */
  ME1 |= UTXE0 + URXE0;                 /* Enable USART0 TXD/RXD */
  /*IE1 |= URXIE0;*/                        /* Enable USART0 RX interrupt */
#endif /* 0 */
  
  /* RS232 */
  UCTL1 = CHAR;                         /* 8-bit character */
  UTCTL1 = SSEL1;                       /* UCLK = MCLK */

#define RS232_SPEED 57600
  
  if(RS232_SPEED == 19200) {
    /* Set RS232 to 19200 */
    UBR01 = 0x80;                         /* 2,457MHz/19200 = 128 -> 0x80 */
    UBR11 = 0x00;                         /* */
    UMCTL1 = 0x00;                        /* no modulation  */
  } else if(RS232_SPEED == 38400) {
    /* Set RS232 to 38400 */
    UBR01 = 0x40;                         /* 2,457MHz/38400 = 64 -> 0x40 */
    UBR11 = 0x00;                         /* */
    UMCTL1 = 0x00;                        /* no modulation  */
  } else if(RS232_SPEED == 57600) {
    UBR01 = 0x2a;                         /* 2,457MHz/57600 = 42.7 -> 0x2A */
    UBR11 = 0x00;                         /* */
    UMCTL1 = 0x5b;                        /* */ 
  } else if(RS232_SPEED == 115200) {
    UBR01 = 0x15;                         /* 2,457MHz/115200 = 21.4 -> 0x15 */
    UBR11 = 0x00;                         /* */
    UMCTL1 = 0x4a;                        /* */ 
  }
  
  
  ME2 |= (UTXE1 | URXE1);                 /* Enable USART1 TXD/RXD */
  IE2 |= URXIE1;                        /* Enable USART1 RX interrupt  */
  BCSCTL1 &= ~(DIVA1 + DIVA0);          /* remove /8 divisor from ACLK again    */


  rs232_put('!');
  
  return RS_ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Configure the RS232 port.
 *
 * \param params RS232 parameters.
 * \param parity Parity.
 * \return An error value or RS_ERR_OK if configuration succeeded.
 */
/*-----------------------------------------------------------------------------------*/
unsigned char
rs232_params(unsigned char params, unsigned char parity)
{
  return RS_ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Get a byte from the serial port.
 *
 * \param c A pointer to the location where the byte is to be stored.
 *
 * \return An error value or RS_ERR_OK. If no data is available, the
 * function returns RS_ERR_NO_DATA which is not a fatal error.
 */
/*-----------------------------------------------------------------------------------*/
unsigned char
rs232_get(char *c)
{
  if(recvbufptr1 == recvbufptr2) {
    return RS_ERR_NO_DATA;
  }
  *c = recvbuf[recvbufptr2];
  recvbufptr2 = (recvbufptr2 + 1) % RECVBUF_SIZE;
  return RS_ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Transmit a byte over the serial port.
 *
 * \param c A pointer to the location where the byte is to be stored.
 *
 * \return An error value or RS_ERR_OK. If the transmission buffer was
 * full, RS_ERR_OVERFLOW is returned.
 */
/*-----------------------------------------------------------------------------------*/
unsigned char
rs232_put(char c)
{
  /* Loop until the transmission buffer is available. */
  while ((IFG2 & UTXIFG1) == 0);

  /* Transmit the data. */
  TXBUF1 = c;

  return RS_ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
