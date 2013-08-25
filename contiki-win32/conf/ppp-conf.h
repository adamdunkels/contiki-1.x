#ifndef __PPP_CONF_H__
#define __PPP_CONF_H__
/*  www.mycal.com
 *---------------------------------------------------------------------------
 *ppp-conf.h - pppconfig header file - -
 *---------------------------------------------------------------------------
 *Version - 0.1 Original Version June 3, 2000 -
 *---------------------------------------------------------------------------
 *Notes: - This is where you configure the timeout and buffer sizes.
 *-
 *---------------------------------------------------------------------------
*/
/*
 * Copyright (c) 2003, Mike Johnson, Mycal Labs, www.mycal.net
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
 *      This product includes software developed by Mike Johnson of Mycal
 *      Labs (www.mycal.net)
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
 * This file is part of the Mycal Modified uIP TCP/IP stack.
 *
 * $Id: ppp-conf.h,v 1.3 2004/08/22 21:10:45 oliverschmidt Exp $
 *
 */

#include <string.h>
#include "uip.h"

#define PPP_RX_BUFFER_SIZE		1024 
/*#define PPP_TX_BUFFER_SIZE		64*/

#define PAP_USERNAME_SIZE		32
#define PAP_PASSWORD_SIZE		32

#define	LCP_RETRY_COUNT			5
#define LCP_TIMEOUT			5

#define PAP_TIMEOUT			5

#define	IPCP_RETRY_COUNT		5
#define	IPCP_TIMEOUT			5

/* uncomment next line to get peer IP address */
#define IPCP_GET_PEER_IP		1
#define IPCP_GET_PRI_DNS		1	
#define IPCP_GET_SEC_DNS		1

#endif /* __PPP_CONF_H__ */
