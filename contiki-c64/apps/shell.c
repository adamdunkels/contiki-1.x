/*
 * Copyright (c) 2003, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Adam Dunkels.
 * 4. The name of the author may not be used to endorse or promote
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
 * This file is part of the Contiki desktop OS.
 *
 * $Id: shell.c,v 1.1 2003/08/20 19:50:10 adamdunkels Exp $
 *
 */

#include "program-handler.h"
#include "loader.h"
#include "c64-fs.h"
#include "uip.h"
#include "uip_main.h"
#include "uip_arp.h"
#include "resolv.h"

#include "uip-signal.h"

#include <string.h>

#define XSIZE 36
#define YSIZE 12

static struct ctk_window window;
static char log[XSIZE * YSIZE];
static struct ctk_label loglabel =
  {CTK_LABEL(0, 0, XSIZE, YSIZE, log)};
static char command[XSIZE + 1];
static struct ctk_textentry commandentry =
  {CTK_TEXTENTRY(0, YSIZE, XSIZE - 2, 1, command, XSIZE)};

static DISPATCHER_SIGHANDLER(sighandler, s, data);
static void idle(void);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Command shell", idle, sighandler,
		   NULL)};
static ek_id_t id;

static char showingdir = 0;
static struct c64_fs_dir dir;
static unsigned int totsize;

struct ptentry {
  char c;
  void (* pfunc)(char *str);
};

/*-----------------------------------------------------------------------------------*/
static void
quit(char *str)
{
  ctk_window_close(&window);
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static void
logf(char *str1, char *str2)
{
  static unsigned char i, len;
  
  for(i = 1; i < YSIZE; ++i) {
    memcpy(&log[(i - 1) * XSIZE], &log[i * XSIZE], XSIZE);
  }
  memset(&log[(YSIZE - 1) * XSIZE], 0, XSIZE);

  len = strlen(str1);

  strncpy(&log[(YSIZE - 1) * XSIZE], str1, XSIZE);
  if(len < XSIZE) {
    strncpy(&log[(YSIZE - 1) * XSIZE] + len, str2, XSIZE - len);
  }

  CTK_WIDGET_REDRAW(&loglabel);
}

static void
parse(register char *str, struct ptentry *t)
{
  register struct ptentry *p;
  char *sstr;

  sstr = str;
  
  /* Loop over the parse table entries in t in order to find one that
     matches the first character in str. */
  for(p = t; p->c != 0; ++p) {
    if(*str == p->c) {
      /* Skip rest of the characters up to the first space. */
      while(*str != ' ') {
	++str;
      }

      /* Skip all spaces.*/
      while(*str == ' ') {
	++str;
      }

      logf("> ", sstr);
      /* Call parse table entry function and return. */
      p->pfunc(str);
      return;
    }
  }

  /* Did not find matching entry in parse table. We just call the
     default handler supplied by the caller and return. */
  p->pfunc(str);
}
/*-----------------------------------------------------------------------------------*/
static void
inttostr(register char *str, unsigned int i)
{
  str[0] = '0' + i / 100;
  if(str[0] == '0') {
    str[0] = ' ';
  }
  str[1] = '0' + (i / 10) % 10;
  if(str[1] == '0') {
    str[1] = ' ';
  }
  str[2] = '0' + i % 10;
  str[3] = ' ';
  str[4] = 0;
}
/*-----------------------------------------------------------------------------------*/
static void
processes(char *str)
{
  static char idstr[5];
  struct dispatcher_proc *p;

  logf("Processes:", "");
  /* Step through each possible process ID and see if there is a
     matching process. */
  for(p = DISPATCHER_PROCS(); p != NULL; p = p->next) {
    inttostr(idstr, p->id);
    logf(idstr, p->name);
  }
}
/*-----------------------------------------------------------------------------------*/
static char *
nullterminate(char *str)
{
  char *nt;

  /* Nullterminate string. Start with finding newline character. */
  for(nt = str; *nt != '\r' &&
	*nt != '\n'; ++nt);

  /* Replace newline with a null char. */
  *nt = 0;

  /* Remove trailing spaces. */
  while(nt > str && *(nt - 1) == ' ') {
    *(nt - 1) = 0;
    --nt;
  }
  
  /* Return pointer to null char. */
  return nt;
}
/*-----------------------------------------------------------------------------------*/
static void
runfile(char *str)
{
  nullterminate(str);
  
  /* Call loader function. */
  program_handler_load(str);

  logf("Starting program ", str);  
}
/*-----------------------------------------------------------------------------------*/
static void
execfile(char *str)
{
  runfile(str);
  quit(NULL);
}
/*-----------------------------------------------------------------------------------*/
static void
killproc(char *str)
{
  char procnum, j, c;
  char procstr[5];

  nullterminate(str);
  
  procnum = 0;
  
  for(j = 0; j < 4; ++j) {
    c = str[j];
    if(c >= '0' && c <= '9') {
      procnum = procnum * 10 + (str[j] - '0');
    } else {
      break;
    }
  }
  if(procnum != 0) {
    inttostr(procstr, procnum);
    logf("Killing process ", procstr);
    dispatcher_emit(dispatcher_signal_quit, NULL, procnum);
  } else {
    logf("Could not parse process number", "");
  }
  
}
/*-----------------------------------------------------------------------------------*/
static void
help(char *str)
{
  logf("Available commands:", "");
  logf("run  - start program", "");
  logf("exec - start program & exit shell", "");  
  logf("ps   - show processes", "");
  logf("kill - kill process", "");
  logf("ls   - display directory", "");
  logf("quit - quit shell", "");
  logf("?    - show this help", "");      
}
/*-----------------------------------------------------------------------------------*/
static void
directory(char *str)
{
  if(c64_fs_opendir(&dir) != 0) {
    logf("Cannot open directory", "");
    showingdir = 0;
  } else {
    logf("Disk directory:", "");
    showingdir = 1;
    totsize = 0;
  }
  
}
/*-----------------------------------------------------------------------------------*/
static void
none(char *str)
{
}
/*-----------------------------------------------------------------------------------*/
static struct ptentry configparsetab[] =
  {{'e', execfile},
   {'r', runfile},
   {'k', killproc},   
   {'p', processes},
   {'l', directory},
   {'q', quit},
   {'?', help},

   /* Default action */
   {0, none}};
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(config_init)
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    dispatcher_listen(ctk_signal_window_close);
    dispatcher_listen(ctk_signal_widget_activate);    

    ctk_window_new(&window, XSIZE, YSIZE+1, "Command shell");
    CTK_WIDGET_ADD(&window, &loglabel);
    CTK_WIDGET_ADD(&window, &commandentry);
    CTK_WIDGET_FOCUS(&window, &commandentry);
    memset(log, ' ', sizeof(log));

    logf("Contiki command shell", "");
    help(NULL);
  }
  ctk_window_open(&window);
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);

  if(s == ctk_signal_widget_activate &&
     data == (ek_data_t)&commandentry) {
    if(showingdir != 0) {
      showingdir = 0;
      logf("Directory stopped", "");
      c64_fs_closedir(&dir);
    }
    parse(command, configparsetab);
    CTK_TEXTENTRY_CLEAR(&commandentry);
    CTK_WIDGET_REDRAW(&commandentry);
  } else if(s == ctk_signal_window_close ||
     s == dispatcher_signal_quit) {
    quit(NULL);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
idle(void)
{
  static struct c64_fs_dirent dirent;
  static char size[10];
  if(showingdir != 0) {
    c64_fs_readdir_dirent(&dir, &dirent);
    totsize += dirent.size;
    inttostr(size, dirent.size);
    logf(size, dirent.name);
    if(c64_fs_readdir_next(&dir) != 0) {
      c64_fs_closedir(&dir);
      showingdir = 0;
      inttostr(size, totsize);
      logf("Total number of blocks: ", size);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
