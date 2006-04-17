/*
 CONIO.H - Console I/O library for the Amsrad CPC
 To use with the Small Devices C Compiler
 CONIO.H is a implementation of the Contiki CONIO.H (Derived from the borland CONIO.H)

 2003 H. Hansen
*/

#ifndef  __conio_h__
#define __conio_h__

#include <stdio.h>
#include <stdlib.h>
#include "arch.h"

#define CH_ULCORNER 0x096
#define CH_URCORNER 0x09c
#define CH_LLCORNER 0x093
#define CH_LRCORNER 0x099

void clrscr (void);

unsigned char kbhit (void);

void gotox (unsigned char x);

void gotoy (unsigned char y);

void gotoxy (unsigned char x, unsigned char y);

unsigned char wherex (void);

unsigned char wherey (void);

void cputc (char c);

void cputcxy (unsigned char x, unsigned char y, char c);

void cputs (char* s);

char cgetc (void);

void  revers ();

void textcolor (unsigned char color);

void bgcolor (unsigned char color);

void  bordercolor (unsigned char color);

void chline (unsigned char length);

void chlinexy (unsigned char x, unsigned char y, unsigned char length);

void cvline (unsigned char length);

void cvlinexy (unsigned char x, unsigned char y, unsigned char length);

void cclear (unsigned char length);

void cclearxy (unsigned char x, unsigned char y, unsigned char length);

void screensize (unsigned char* x, unsigned char* y);

#endif /* __conio_h__ */

