/**
 * \file
 * Functions for reading the sensors of the sensor board
 * \author Based on the work of anonymous students at FU Berlin
 *
 *
 */

#include <io.h>
#include <signal.h>

volatile unsigned int sensors_mic, /**< On-board microphone. */
  sensors_extern,                  /**< External sensor. */
  sensors_battery,                 /**< Battery sensor. */
  sensors_temp;                    /**< Temperature sensor. */

/*-----------------------------------------------------------------------------------*/
/**
 * \internal
 * Interrupt handler which reads the values of the A/D converters.
 *
 */
/*-----------------------------------------------------------------------------------*/
interrupt (ADC_VECTOR)
     sensors_ad_irq(void)
{
  unsigned int temp;
  char vk;
  
  sensors_mic     = ADC12MEM0;
  sensors_extern  = ADC12MEM1;
  sensors_battery = ADC12MEM2;
  temp            = ADC12MEM3;

  vk = (temp >> 8);
  if(vk & 0x80) {
    vk = ~vk;
  }
  sensors_temp = vk;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Initialize the sensors. 
 */
/*-----------------------------------------------------------------------------------*/
void
sensors_init(void)
{

  /* Set up the ADC. */

  ADC12CTL0 = ADC12ON + REFON + REF2_5V + SHT0_6 + SHT1_6 + MSC;
  ADC12CTL1 = SHP + CONSEQ_3 + CSTARTADD_0;

  ADC12MCTL0 = INCH_3 + SREF_0;
  ADC12MCTL1 = INCH_4 + SREF_0;
  ADC12MCTL2 = INCH_0 + SREF_0;
  ADC12MCTL3 = INCH_5 + SREF_0 + EOS;

  ADC12IE = 0x0008;

  /* Delay */
  {unsigned int i,j; for(i = 0x200; i > 0; --i) {j = j*j;}}

  ADC12CTL0 |= ENC;
  ADC12CTL0 |= ADC12SC;
}
/*-----------------------------------------------------------------------------------*/
