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
 * This file is part of the Contiki desktop OS
 *
 * $Id: loader.h,v 1.6 2003/08/24 22:41:31 adamdunkels Exp $
 *
 */
#ifndef __LOADER_H__
#define __LOADER_H__

/* Errors that the LOADER_LOAD() function may return: */

#define LOADER_OK                0
#define LOADER_ERR_READ          1       /* Read error */
#define LOADER_ERR_HDR           2       /* Header error */
#define LOADER_ERR_OS            3       /* Wrong OS */
#define LOADER_ERR_FMT           4       /* Data format error */
#define LOADER_ERR_MEM           5       /* Not enough memory */
#define LOADER_ERR_OPEN          6       /* Could not open file */
#define LOADER_ERR_ARCH          7       /* Wrong architecture */
#define LOADER_ERR_VERSION       8       /* Wrong version */



#ifdef WITH_LOADER_ARCH
#include "loader-arch.h"
#define LOADER_INIT_FUNC(name, arg) void init(char *arg)
#else /* WITH_LOADER_ARCH */
#define LOADER_INIT_FUNC(name, arg) void name(char *arg)
#endif /* WITH_LOADER_ARCH */

#ifndef LOADER_LOAD
#define LOADER_LOAD(name, arg) LOADER_OK
#endif /* LOADER_LOAD */

#ifndef LOADER_LOAD_DSC
#define LOADER_LOAD_DSC(name) NULL
#endif /* LOADER_LOAD_DSC */

#ifndef LOADER_UNLOAD
#define LOADER_UNLOAD()
#endif /* LOADER_UNLOAD */

#ifndef LOADER_UNLOAD_DSC
#define LOADER_UNLOAD_DSC(dsc)
#endif /* LOADER_UNLOAD */




#endif /* __LOADER_H__ */
