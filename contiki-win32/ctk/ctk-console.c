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
 * $Id: ctk-console.c,v 1.7 2007/09/05 12:43:20 oliverschmidt Exp $
 *
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <conio.h>

#include "ctk-console.h"

static HANDLE stdouthandle;

static unsigned char width;
static unsigned char height;

static unsigned char       saved_color;
static char                saved_title[1024];
static DWORD               saved_consolemode;
static CONSOLE_CURSOR_INFO saved_cursorinfo;

static unsigned char color;
static unsigned char reversed;

/*-----------------------------------------------------------------------------------*/
static BOOL WINAPI
consolehandler(DWORD event)
{
  console_exit();
  exit(EXIT_SUCCESS);
}
/*-----------------------------------------------------------------------------------*/
void
console_init(void)
{
  CONSOLE_SCREEN_BUFFER_INFO consoleinfo;
  CONSOLE_CURSOR_INFO cursorinfo = {1, FALSE};

  stdouthandle = GetStdHandle(STD_OUTPUT_HANDLE);

  screensize(&width, &height);

  GetConsoleScreenBufferInfo(stdouthandle, &consoleinfo);
  saved_color = (unsigned char)consoleinfo.wAttributes;

  GetConsoleTitle(saved_title, sizeof(saved_title));
  SetConsoleTitle("Contiki");

  GetConsoleMode(stdouthandle, &saved_consolemode);
  SetConsoleMode(stdouthandle, ENABLE_PROCESSED_OUTPUT);

  GetConsoleCursorInfo(stdouthandle, &saved_cursorinfo);
  SetConsoleCursorInfo(stdouthandle, &cursorinfo);

  SetConsoleCtrlHandler(consolehandler, TRUE);
}
/*-----------------------------------------------------------------------------------*/
void
console_exit(void)
{
  textcolor(saved_color);
  revers(0);
  clrscr();
  gotoxy(0, 0);

  SetConsoleTitle(saved_title);
  SetConsoleMode(stdouthandle, saved_consolemode);
  SetConsoleCursorInfo(stdouthandle, &saved_cursorinfo);
}
/*-----------------------------------------------------------------------------------*/
unsigned char
console_resize(void)
{
  unsigned char new_width;
  unsigned char new_height;

  screensize(&new_width, &new_height);

  if(new_width  != width ||
     new_height != height) {
    width  = new_width;
    height = new_height;
    return 1;
  }

  return 0;
}
/*-----------------------------------------------------------------------------------*/
static void
setcolor(void)
{
  SetConsoleTextAttribute(stdouthandle, (WORD)(reversed? (color & 0x0F) << 4 |
							 (color & 0xF0) >> 4
						       : color));
}
/*-----------------------------------------------------------------------------------*/
unsigned char
wherex(void)
{
  CONSOLE_SCREEN_BUFFER_INFO consoleinfo;

  GetConsoleScreenBufferInfo(stdouthandle, &consoleinfo);
  return (unsigned char)consoleinfo.dwCursorPosition.X;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
wherey(void)
{
  CONSOLE_SCREEN_BUFFER_INFO consoleinfo;

  GetConsoleScreenBufferInfo(stdouthandle, &consoleinfo);
  return (unsigned char)consoleinfo.dwCursorPosition.Y;
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
  unsigned char i;

  for(i = 0; i < length; ++i) {
    putch(' ');
  }  
}
/*-----------------------------------------------------------------------------------*/
void
chline(unsigned char length)
{
  unsigned char i;

  for(i = 0; i < length; ++i) {
    putch('-');
  }
}
/*-----------------------------------------------------------------------------------*/
void
cvline(unsigned char length)
{
  unsigned char i, x, y;

  x = wherex();
  y = wherey();

  for(i = 0; i < length; ++i) {
    cputcxy(x, (unsigned char)(y + i), '|');
  }
}
/*-----------------------------------------------------------------------------------*/
void
gotoxy(unsigned char x, unsigned char y)
{
  COORD coord = {x, y};

  SetConsoleCursorPosition(stdouthandle, coord);
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
  color = c;
  setcolor();
}
/*-----------------------------------------------------------------------------------*/
void
bgcolor(unsigned char c)
{
}
/*-----------------------------------------------------------------------------------*/
void
bordercolor(unsigned char c)
{
}
/*-----------------------------------------------------------------------------------*/
void
screensize(unsigned char *x, unsigned char *y)
{
  CONSOLE_SCREEN_BUFFER_INFO consoleinfo;

  GetConsoleScreenBufferInfo(stdouthandle, &consoleinfo);
  *x = consoleinfo.srWindow.Right - consoleinfo.srWindow.Left + 1;
  *y = consoleinfo.srWindow.Bottom - consoleinfo.srWindow.Top + 1;
}
/*-----------------------------------------------------------------------------------*/
