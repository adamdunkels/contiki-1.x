/**
 * \file
 * Header file for MSP430 RS232 driver.
 * \author Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __RS232_H__
#define __RS232_H__

/* These values were taken from the "rs232.h" include file from cc65,
   written by Ullrich von Bassewitz. */

#define RS_BAUD_50                      0x00
#define RS_BAUD_110                     0x01
#define RS_BAUD_134_5                   0x02
#define RS_BAUD_300                     0x03
#define RS_BAUD_600                     0x04
#define RS_BAUD_1200                    0x05
#define RS_BAUD_2400                    0x06
#define RS_BAUD_4800                    0x07
#define RS_BAUD_9600                    0x08
#define RS_BAUD_19200                   0x09
#define RS_BAUD_38400                   0x0A
#define RS_BAUD_57600                   0x0B
#define RS_BAUD_115200                  0x0C
#define RS_BAUD_230400                  0x0D

#define RS_BITS_5                       0x60
#define RS_BITS_6                       0x40
#define RS_BITS_7                       0x20
#define RS_BITS_8                       0x00

#define RS_PAR_NONE                     0x00
#define RS_PAR_ODD                      0x20
#define RS_PAR_EVEN                     0x60
#define RS_PAR_MARK                     0xA0
#define RS_PAR_SPACE                    0xE0

#define RS_STOP_1                       0x00
#define RS_STOP_2                       0x80

#define RS_ERR_OK                       0x00    /* Not an error - relax */
#define RS_ERR_NOT_INITIALIZED          0x01    /* Module not initialized */
#define RS_ERR_BAUD_TOO_FAST            0x02    /* Cannot handle baud rate */
#define RS_ERR_BAUD_NOT_AVAIL           0x03    /* Baud rate not available */
#define RS_ERR_NO_DATA                  0x04    /* Nothing to read */
#define RS_ERR_OVERFLOW                 0x05    /* No room in send buffer */
#define RS_ERR_INIT_FAILED              0x06    /* Initialization of RS232 routines f
ailed */

unsigned char rs232_init(char none);
unsigned char rs232_params(unsigned char params, unsigned char parity);
unsigned char rs232_get(char *c);
unsigned char rs232_put(char c);

#endif /* __RS232_H__ */
