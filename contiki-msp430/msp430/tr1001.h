#ifndef __TR1001_H__
#define __TR1001_H__

void tr1001_init(void);

void tr1001_set_txpower(unsigned char p);

void tr1001_send_raw(char *data, unsigned short len);
void tr1001_send(void);

unsigned short tr1001_poll(void);


#endif /* __TR1001_H__ */
