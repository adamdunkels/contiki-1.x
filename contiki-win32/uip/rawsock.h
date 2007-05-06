#ifndef __RAWSOCK_H__
#define __RAWSOCK_H__

void rawsock_init(void);
void rawsock_fini(void);
void rawsock_send(u8_t *hdr, u16_t hdrlen, u8_t *data, u16_t datalen);
u16_t rawsock_poll(void);

#endif /* __RAWSOCK_H__ */
