#ifndef __CTK_TERMARCH_H__
#define __CTK_TERMARCH_H__


typedef char ctk_arch_key_t;

unsigned char ctk_arch_keyavail(void);
ctk_arch_key_t ctk_arch_getkey(void);

#define CH_ENTER 0x0d
#define CH_DEL 0x08

#define CH_TAB  0x09

#define CH_CURS_LEFT  0x11
#define CH_CURS_UP    0x12
#define CH_CURS_RIGHT 0x13
#define CH_CURS_DOWN  0x14


#define CH_F1 0x15
#define CH_F2 0x16
#define CH_F3 0x17
#define CH_F4 0x18

#endif /* __CTK_TERMARCH_H__ */
