/*
 * Copyright (c) 2004, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution. 
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the Contiki desktop environment
 *
 * $Id: ctk-console.c,v 1.1 2004/07/15 00:36:57 oliverschmidt Exp $
 *
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>

#include "ctk-console.h"

static unsigned char reversed;
static unsigned char foreground;
static unsigned char background;

/*-----------------------------------------------------------------------------------*/
static HANDLE
getconsole(void)
{
  static HANDLE console = INVALID_HANDLE_VALUE;

  if(console == INVALID_HANDLE_VALUE) {
    console = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE,
			 FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                         OPEN_EXISTING, 0, NULL);
  }
  return console;
}
/*-----------------------------------------------------------------------------------*/
static void
setcolor(void)
{
  SetConsoleTextAttribute(getconsole(), reversed? background + foreground * 0x10 
						: foreground + background * 0x10);
}
/*-----------------------------------------------------------------------------------*/
unsigned char
wherex(void)
{
  CONSOLE_SCREEN_BUFFER_INFO consoleinfo;

  GetConsoleScreenBufferInfo(getconsole(), &consoleinfo);
  return consoleinfo.dwCursorPosition.X;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
wherey(void)
{
  CONSOLE_SCREEN_BUFFER_INFO consoleinfo;

  GetConsoleScreenBufferInfo(getconsole(), &consoleinfo);
  return consoleinfo.dwCursorPosition.Y;
}
/*-----------------------------------------------------------------------------------*/
void
clrscr(void)
{
  unsigned char i, width, height;

  screensize(&width, &height);
  for(i = 0; i < height; ++i) {
    cclearxy(0, i, width);
  }
}
/*-----------------------------------------------------------------------------------*/
void
revers(unsigned char c)
{
  reversed = c;
  setcolor();
}
/*-----------------------------------------------------------------------------------*/
void
cclear(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    putch(' ');
  }  
}
/*-----------------------------------------------------------------------------------*/
void
chline(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    putch(0xC4);
  }
}
/*-----------------------------------------------------------------------------------*/
void
cvline(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    putch(0xB3);
    gotoxy(wherex() - 1, wherey() + 1);
  }
}
/*-----------------------------------------------------------------------------------*/
void
gotoxy(unsigned char x, unsigned char y)
{
  COORD coord = {x, y};

  SetConsoleCursorPosition(getconsole(), coord);
}
/*-----------------------------------------------------------------------------------*/
void
cclearxy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  cclear(length);
}
/*-----------------------------------------------------------------------------------*/
void
chlinexy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  chline(length);
}
/*-----------------------------------------------------------------------------------*/
void
cvlinexy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  cvline(length);
}
/*-----------------------------------------------------------------------------------*/
void
cputsxy(unsigned char x, unsigned char y, char *str)
{
  gotoxy(x, y);
  cputs(str);
}
/*-----------------------------------------------------------------------------------*/
void
cputcxy(unsigned char x, unsigned char y, char c)
{
  gotoxy(x, y);
  putch(c);
}
/*-----------------------------------------------------------------------------------*/
void
textcolor(unsigned char c)
{
  foreground = c;
  setcolor();
}
/*-----------------------------------------------------------------------------------*/
void
bgcolor(unsigned char c)
{
  background = c;
  setcolor();
}
/*-----------------------------------------------------------------------------------*/
void
bordercolor(unsigned char c)
{
  CONSOLE_CURSOR_INFO cursorinfo = {1, FALSE};

  SetConsoleCursorInfo(getconsole(), &cursorinfo);
  SetConsoleMode(getconsole(), 0);
  SetConsoleTitle("Contiki");
}
/*-----------------------------------------------------------------------------------*/
void
screensize(unsigned char *x, unsigned char *y)
{
  CONSOLE_SCREEN_BUFFER_INFO consoleinfo;

  GetConsoleScreenBufferInfo(getconsole(), &consoleinfo);
  *x = consoleinfo.dwMaximumWindowSize.X;
  *y = consoleinfo.dwMaximumWindowSize.Y;
}
/*-----------------------------------------------------------------------------------*/
