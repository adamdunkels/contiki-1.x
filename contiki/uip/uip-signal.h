/**
 * \file
 * uIP TCP/IP stack signals.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 * The signals defined in this module usually is not used by
 * application programmers, but knowledge of these signals are useful
 * for developers of network device drivers.
 *
 * These signals can be used to force a poll of a specific uIP
 * connection. This usually is used after doing e.g. a TCP connect
 * that normally would not take place until next time the connection
 * was periodically polled (normally up to 0.5 seconds afterwards). By
 * emitting the signals defined by this module, the poll will take
 * place at once the signal is delivered.
 *
 */


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
 * $Id: uip-signal.h,v 1.6 2003/10/01 11:25:37 adamdunkels Exp $
 *
 */

#ifndef __UIP_SIGNAL_H__
#define __UIP_SIGNAL_H__

#include "ek.h"

/**
 * \addtogroup signals
 * @{
 */

extern ek_signal_t
 uip_signal_uninstall,  /**< Cause a network device driver to unload itself. */
  
 uip_signal_poll,       /**< Cause a poll of a TCP connection to take
			   place. A pointer to the uip_conn struct
			   must be passed as signal data with the
			   signal. */
  
 uip_signal_poll_udp;   /**< Cause a poll of a UDP connection to take
			   place. A pointer to the uip_udp_conn struct
			   must be passed as signal data with the
			   signal. */

/** @} */

/**
 * Initialize the uIP signal module.
 */
void uip_signal_init(void);

#endif /* __UIP_SIGNAL_H__ */
