/*
 * Copyright (c) 2002, Adam Dunkels.
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
 * This file is part of the Contiki desktop environment
 *
 * $Id: sensorview.c,v 1.1 2003/09/04 19:46:33 adamdunkels Exp $
 *
 */

#include "ctk.h"
#include "dispatcher.h"
#include "loader.h"
#include "sensors.h"

#include <stdio.h>

static struct ctk_window window;

#if 1
static struct ctk_label templabel =
  {CTK_LABEL(1, 1, 12, 1,"Temperature:")};
static char tempval[7];
static struct ctk_label tempvallabel =
  {CTK_LABEL(4, 3, 6, 1, tempval)};
#endif 

static void sensorview_idle(void);   
static DISPATCHER_SIGHANDLER(sensorview_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Sensor view", sensorview_idle,
		   sensorview_sighandler, NULL)};
static ek_id_t id = EK_ID_NONE;

/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(sensorview_init, arg)
{
  arg_free(arg);
    
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);

    ctk_window_new(&window, 14, 4, "Sensors");
#if 1
    CTK_WIDGET_ADD(&window, &templabel);
    CTK_WIDGET_ADD(&window, &tempvallabel);
#endif
    dispatcher_listen(ctk_signal_window_close);
  }
  ctk_window_open(&window);
}
/*-----------------------------------------------------------------------------------*/
static void
sensorview_quit(void)
{
  ctk_window_close(&window);
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(sensorview_sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);

  if(s == dispatcher_signal_quit ||
     s == ctk_signal_window_close) {
    sensorview_quit();
  }
}
/*-----------------------------------------------------------------------------------*/
static void
sensorview_idle(void)
{
  sprintf(tempval, "%d", sensors_temp);
}
/*-----------------------------------------------------------------------------------*/
