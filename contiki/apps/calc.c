/*
 * Copyright (c) 2003, Adam Dunkels.
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
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *        This product includes software developed by Adam Dunkels. 
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
 * This an example program for the Contiki desktop OS
 *
 * $Id: calc.c,v 1.2 2003/04/11 20:12:34 adamdunkels Exp $
 *
 */

/* This is an example of how to write programs for Contiki. It
   displays a window with the famous message "Hello world" and two
   buttons; one which changes the message slightly and one which quits
   the application. */

#include "ctk.h"
#include "dispatcher.h"
#include "loader.h"

static struct ctk_window window;

static char input[16];
static struct ctk_label inputlabel =
  {CTK_LABEL(0, 0, 15, 1, input)};

static struct ctk_button addbutton =
  {CTK_BUTTON(0, 1, 1, "+")};
static struct ctk_button subbutton =
  {CTK_BUTTON(3, 1, 1, "-")};
static struct ctk_button mulbutton =
  {CTK_BUTTON(6, 1, 1, "*")};
static struct ctk_button divbutton =
  {CTK_BUTTON(9, 1, 1, "/")};
static struct ctk_button calcbutton =
  {CTK_BUTTON(12, 1, 1, "=")};

static DISPATCHER_SIGHANDLER(calc_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Calculator", NULL, calc_sighandler, NULL)};
static ek_id_t id;

static unsigned int operand1, operand2;
static unsigned char op;
#define OP_ADD 1
#define OP_SUB 2
#define OP_MUL 3
#define OP_DIV 4

/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(calc_init)
{
  unsigned char i;
  
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    
    ctk_window_new(&window, 15, 2, "Calc");

    CTK_WIDGET_ADD(&window, &inputlabel);

    CTK_WIDGET_ADD(&window, &addbutton);
    CTK_WIDGET_ADD(&window, &subbutton);
    CTK_WIDGET_ADD(&window, &mulbutton);
    CTK_WIDGET_ADD(&window, &divbutton);
    CTK_WIDGET_ADD(&window, &calcbutton);
    
    CTK_WIDGET_FOCUS(&window, &addbutton);
        
    dispatcher_listen(ctk_signal_button_activate);
    dispatcher_listen(ctk_signal_window_close);
    dispatcher_listen(ctk_signal_keypress);

    for(i = 0; i < sizeof(input); ++i) {
      input[i] = ' ';
    }
  }
  ctk_window_open(&window);
}
/*-----------------------------------------------------------------------------------*/
static void
calc_quit(void)
{
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static void
input_to_operand1(void)
{
  unsigned int m;
  unsigned char i;

  operand1 = 0;
  for(m = 1, i = 14;
      i > 9; --i, m *= 10) {
    if(input[i] >= '0' &&
       input[i] <= '9') {
      operand1 += (input[i] - '0') * m;
    }
    input[i] = ' ';
  }
  input[14] = ' ';
}
/*-----------------------------------------------------------------------------------*/
static void
operand2_to_input(void)
{
  unsigned char i;
  
  input[10] = (operand2/10000) % 10 + '0';
  input[11] = (operand2/1000) % 10 + '0';
  input[12] = (operand2/100) % 10 + '0';
  input[13] = (operand2/10) % 10 + '0';
  input[14] = operand2 % 10 + '0';

  for(i = 0; i < 4; ++i) {
    if(input[10 + i] == '0') {
      input[10 + i] = ' ';
    } else {
      break;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
static void
calculate(void)
{
  operand2 = operand1;
  input_to_operand1();
  switch(op) {
  case OP_ADD:
    operand2 = operand2 + operand1;
    break;
  case OP_SUB:
    operand2 = operand2 - operand1;
    break;
  case OP_MUL:
    operand2 = operand2 * operand1;
    break;
  case OP_DIV:
    operand2 = operand2 / operand1;
    break;
  }
  operand2_to_input();      
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(calc_sighandler, s, data)
{
  unsigned char i;
  DISPATCHER_SIGHANDLER_ARGS(s, data);

  if(s == ctk_signal_keypress) {
    if((char)data >= '0' &&
       (char)data <= '9') {
      for(i = 0; i < 14; ++i) {
	input[i] = input[i + 1];
      }
      input[14] = (char)data;
    } else if((char)data == ' ') {
      for(i = 0; i < sizeof(input); ++i) {
	input[i] = ' ';
      }      
    } else if((char)data == '+') {
      input_to_operand1();
      op = OP_ADD;
    } else if((char)data == '-') {
      input_to_operand1();
      op = OP_SUB;
    } else if((char)data == '*') {
      input_to_operand1();
      op = OP_MUL;
    } else if((char)data == '/') {
      input_to_operand1();
      op = OP_DIV;
    } else if((char)data == '=') {
      calculate();
    }

    CTK_WIDGET_REDRAW(&inputlabel);
  } else if(s == ctk_signal_button_activate) {
    if(data == (ek_data_t)&calcbutton) {
      calculate();
    } else if(data == (ek_data_t)&addbutton) {
      input_to_operand1();
      op = OP_ADD;
    } else if(data == (ek_data_t)&subbutton) {
      input_to_operand1();
      op = OP_SUB;
    } else if(data == (ek_data_t)&mulbutton) {
      input_to_operand1();
      op = OP_MUL;
    } else if(data == (ek_data_t)&divbutton) {
      input_to_operand1();
      op = OP_DIV;
    }
    CTK_WIDGET_REDRAW(&inputlabel);
  } else if(s == ctk_signal_window_close &&
	    data == (ek_data_t)&window) {
    calc_quit();
  }
}
/*-----------------------------------------------------------------------------------*/
