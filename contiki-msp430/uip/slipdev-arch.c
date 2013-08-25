#include "rs232.h"
#include "slipdev.h"

void slipdev_char_put(u8_t c) { rs232_put(c); }
u8_t slipdev_char_poll(u8_t *c) { return (rs232_get(c) != RS_ERR_NO_DATA); }
