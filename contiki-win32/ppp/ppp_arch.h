#ifndef __PPP_ARCH_H__
#define __PPP_ARCH_H__

/**
 * Put a character on the serial device.
 *
 * This function is used by the PPP implementation to put a character
 * on the serial device. It must be implemented specifically for the
 * system on which the PPP implementation is to be run.
 *
 * \param c The character to be put on the serial device.
 */
void ppp_arch_putchar(u8_t c);

/**
 * Poll the serial device for a character.
 *
 * This function is used by the PPP implementation to poll the serial
 * device for a character. It must be implemented specifically for the
 * system on which the PPP implementation is to be run.
 *
 * The function should return immediately regardless if a character is
 * available or not. If a character is available it should be placed
 * at the memory location pointed to by the pointer supplied by the
 * arguement c.
 *
 * \param c A pointer to a byte that is filled in by the function with
 * the received character, if available.
 *
 * \retval 0 If no character is available.
 * \retval Non-zero If a character is available.
 */
u8_t ppp_arch_getchar(u8_t *c);

#endif /* __PPP_ARCH_H__ */
