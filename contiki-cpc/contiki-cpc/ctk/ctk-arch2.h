/*#include "fire.h"*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//char *strncpy(char *, const char *, unsigned int);

//typedef char ctk_arch_key_t;

#define ctk_arch_keyavail kbhit
#define ctk_arch_getkey cgetc

// have to undefine because of it being defined in
// ctk-conio.h
#undef CH_ENTER
#undef CH_F1
#undef CH_F3
#undef CH_CURS_LEFT
#undef CH_CURS_RIGHT

#define CH_ENTER 0x0d
#define CH_STOP 0x01b
#define CH_F1 0x031
#define CH_F2 0x032
#define CH_F3 0x033
#define CH_F4 0x034
#define CH_F5 0x035
#define CH_F6 0x036
#define CH_F7 0x037
#define CH_DEL 0x07f
#define CH_TAB 0x09
#define CH_CURS_LEFT 0x0f2
#define CH_CURS_DOWN 0x0f1
#define CH_CURS_RIGHT 0x0f3
#define CH_CURS_UP 0x0f0

#define CH_ESC CH_STOP

