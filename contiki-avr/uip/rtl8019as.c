/*
 * Copyright (C) 2001-2002 by egnite Software GmbH. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *
 *    This product includes software developed by egnite Software GmbH
 *    and its contributors.
 *
 * THIS SOFTWARE IS PROVIDED BY EGNITE SOFTWARE GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL EGNITE
 * SOFTWARE GMBH OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: rtl8019as.c,v 1.1 2003/07/04 10:54:52 adamdunkels Exp $ 
 *
 */

/* This file contains the device driver for the RTL8019as chip. The
   code has been written by looking at the device driver written by
   Louis Beaudoin (www.embedded-creations.com), as well as the one
   provided by the NutOS code (www.ethernut.de). */
/*-----------------------------------------------------------------------------------*/
void
rtl8019as_init(void)
{
  /*
   * Mask all interrupts and clear any interrupt status flag to set the 
   * INT pin back to low.
   */
  nic_write(NIC_PG0_IMR, 0);
  nic_write(NIC_PG0_ISR, 0xff);

  /*
   * During reset the nic loaded its initial configuration from an 
   * external eeprom. On the ethernut board we do not have any 
   * configuration eeprom, but simply tied the eeprom data line to 
   * high level. So we have to clear some bits in the configuration 
   * register. Switch to register page 3.
   */
  nic_write(NIC_CR, NIC_CR_STP | NIC_CR_RD2 | NIC_CR_PS0 | NIC_CR_PS1);

  /*
   * The nic configuration registers are write protected unless both 
   * EEM bits are set to 1.
   */
  nic_write(NIC_PG3_EECR, NIC_EECR_EEM0 | NIC_EECR_EEM1);

  /*
   * Disable sleep and power down.
   */
  nic_write(NIC_PG3_CONFIG3, 0);

  /*
   * Network media had been set to 10Base2 by the virtual EEPROM and
   * will be set now to auto detect. This will initiate a link test.
   * We don't force 10BaseT, because this would disable the link test.
   */
  nic_write(NIC_PG3_CONFIG2, NIC_CONFIG2_BSELB);

  /*
   * Reenable write protection of the nic configuration registers
   * and wait for link test to complete.
   */
  nic_write(NIC_PG3_EECR, 0);
  /*    NutSleep(WAIT500);*/
  Delay_10ms(50);

  /*
   * Switch to register page 0 and set data configuration register
   * to byte-wide DMA transfers, normal operation (no loopback),
   * send command not executed and 8 byte fifo threshold.
   */
  nic_write(NIC_CR, NIC_CR_STP | NIC_CR_RD2);
  nic_write(NIC_PG0_DCR, NIC_DCR_LS | NIC_DCR_FT1);

  /*
   * Clear remote dma byte count register.
   */
  nic_write(NIC_PG0_RBCR0, 0);
  nic_write(NIC_PG0_RBCR1, 0);

  /*
   * Temporarily set receiver to monitor mode and transmitter to 
   * internal loopback mode. Incoming packets will not be stored 
   * in the nic ring buffer and no data will be send to the network.
   */
  nic_write(NIC_PG0_RCR, NIC_RCR_MON);
  nic_write(NIC_PG0_TCR, NIC_TCR_LB0);

  /*
   * Configure the nic's ring buffer page layout.
   * NIC_PG0_BNRY: Last page read.
   * NIC_PG0_PSTART: First page of receiver buffer.
   * NIC_PG0_PSTOP: Last page of receiver buffer.
   */
  nic_write(NIC_PG0_TPSR, NIC_FIRST_TX_PAGE);
  nic_write(NIC_PG0_BNRY, NIC_STOP_PAGE - 1);
  nic_write(NIC_PG0_PSTART, NIC_FIRST_RX_PAGE);
  nic_write(NIC_PG0_PSTOP, NIC_STOP_PAGE);

  /*
   * Once again clear interrupt status register.
   */
  nic_write(NIC_PG0_ISR, 0xff);

  /*
   * Switch to register page 1 and copy our MAC address into the nic. 
   * We are still in stop mode.
   */
  nic_write(NIC_CR, NIC_CR_STP | NIC_CR_RD2 | NIC_CR_PS0);
  for(i = 0; i < 6; i++)
    nic_write(NIC_PG1_PAR0 + i, mac[i]);

  /*
   * Clear multicast filter bits to disable all packets.
   */
  for(i = 0; i < 8; i++)
    nic_write(NIC_PG1_MAR0 + i, 0);

  /*
   * Set current page pointer to one page after the boundary pointer.
   */
  nic_write(NIC_PG1_CURR, NIC_START_PAGE + TX_PAGES);

  /*
   * Switch back to register page 0, remaining in stop mode.
   */
  nic_write(NIC_CR, NIC_CR_STP | NIC_CR_RD2);

  /*
   * Take receiver out of monitor mode and enable it for accepting 
   * broadcasts.
   */
  nic_write(NIC_PG0_RCR, NIC_RCR_AB);

  /*
   * Clear all interrupt status flags and enable interrupts.
   */
  nic_write(NIC_PG0_ISR, 0xff);
  nic_write(NIC_PG0_IMR, NIC_IMR_PRXE | NIC_IMR_PTXE | NIC_IMR_RXEE | 
	    NIC_IMR_TXEE | NIC_IMR_OVWE);

  /*
   * Fire up the nic by clearing the stop bit and setting the start bit. 
   * To activate the local receive dma we must also take the nic out of
   * the local loopback mode.
   */
  nic_write(NIC_CR, NIC_CR_STA | NIC_CR_RD2);
  nic_write(NIC_PG0_TCR, 0);

  /*    NutSleep(WAIT500);*/
  Delay_10ms(50);

  
}
/*-----------------------------------------------------------------------------------*/
