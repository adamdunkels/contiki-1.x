#ifndef __MSP430_H__
#define __MSP430_H__


void msp430_init(void);
void msp430_timer_init(void);
void beep(void);
void blink(void);

unsigned short clock(void);

#endif /* __MSP430_H__ */
