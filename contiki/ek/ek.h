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
 * This file is part of the "ek" event kernel.
 *
 * $Id: ek.h,v 1.2 2003/04/18 00:17:09 adamdunkels Exp $
 *
 */
/*-----------------------------------------------------------------------------------*/
#ifndef __EK_H__
#define __EK_H__

#include "ek-conf.h"

/* Signals defined by ek: */
#define EK_SIGNAL_NONE 0

/* Errors: */
#define EK_ERR_OK        0
#define EK_ERR_FULL      1
#define EK_ERR_NOTUNIQUE 2
#define EK_ERR_NOTFOUND  3

/* Special IDs defined by ek: */
#define EK_ID_NONE 0
#define EK_ID_ALL  0

typedef unsigned char ek_err_t;

/* Callback functions (must be implemented by the system/application
   program using ek): */

/* ek_idle:
 *
 * Is called repeatedly by ek when there is nothing else to do. This
 * function can be used to implement polling operations by the
 * system/application program using ek.
 */
void ek_idle(void);

/* ek_dispatcher:
 *
 * Is called by ek when a signal has been omitted. The "id" parameter
 * can be used to distinguish listeners.
 *
 * Return values: TBA
 */
ek_err_t ek_dispatcher(ek_signal_t s, ek_data_t data, ek_id_t id);

/* ek_clock:
 *
 * Should return the current value of the system clock.
 */
ek_clock_t ek_clock(void);


/* API functions (are used by programs using ek): */

/* ek_listen:
 *
 * Registers the listener identified by "id" with the signal "s". When
 * the signal "s" is emitted, the ek dispatcher callback will be
 * invoked for the listener "id".
 *
 * The meaning of the identifier "id" is defined by the caller (i.e.,
 * the application program using the ek kernel).
 *
 * Return values: TBA
 */
ek_err_t ek_listen(ek_signal_t s, ek_id_t id);

/* ek_unlisten:
 *
 * Unregisters all previously registered listeners with ID id.
 *
 * Return values: TBA
 */
ek_err_t ek_unlisten(ek_id_t id);

/* ek_emit:
 *
 * Emits the signal "s". When control returns to ek, the ek signal
 * dispatcher will be called for each of the registered listeners for
 * the signal. If no listeners are registered, the signal is
 * dropped.
 *
 * Return values: TBA
 */
ek_err_t ek_emit(ek_signal_t s, ek_data_t data, ek_id_t id);

/* ek_timer:
 *
 * Sets a timer that will make the signal "s" to be emitted after "t"
 * number of clock ticks. The granularity of the clock ticks is
 * determined by the underlying system on which ek is run.
 *
 * Return values: TBA
 */ 
ek_err_t ek_timer(ek_signal_t s, ek_data_t data, ek_id_t id,
		  ek_ticks_t t);

/* ek_init:
 *
 * Initializes ek.
 */
void ek_init(void);

/* ek_signals:
 *
 * Called internally by ek_run(). Processes signals. 
 */
void ek_signals(void);

/* ek_run:
 *
 * The main function in ek that is called to start ek. This function
 * never returns.
 */
void ek_run(void);

#endif /* __EK_H__ */
/*-----------------------------------------------------------------------------------*/
