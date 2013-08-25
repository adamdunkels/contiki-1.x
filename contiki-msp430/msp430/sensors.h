#ifndef __SENSORS_H__
#define __SENSORS_H__

extern volatile unsigned int sensors_mic, /**< On-board microphone. */
  sensors_extern,                  /**< External sensor. */
  sensors_battery,                 /**< Battery sensor. */
  sensors_temp,                    /**< Temperature sensor. */
  sensors_vib;                     /**< Vibration sensor. */

void sensors_init(void);

#endif /* __SENSORS_H__ */
