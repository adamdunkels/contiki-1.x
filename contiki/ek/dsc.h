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
 * $Id: dsc.h,v 1.3 2003/08/24 22:41:31 adamdunkels Exp $
 *
 */
#ifndef __DSC_H__
#define __DSC_H__

#include "ctk.h"

/* The dsc struct is used for describing a Contiki program. It
   includes a short textual description of the program, either the
   name of the program on disk, or a pointer to the init() function,
   and an icon for the program. */
struct dsc {
  char *description;
  
#if WITH_LOADER_ARCH
  char *prgname;
#else /* WITH_LOADER_ARCH */
  void (*init)(char *arg);
#endif /* WITH_LOADER_ARCH */
  
  struct ctk_icon *icon;

#if WITH_LOADER_ARCH
  void *loadaddr;
#endif /* WITH_LOADER_ARCH */
};

#if WITH_LOADER_ARCH
#define DSC(dscname, description, prgname, initfunc, icon) \
        const struct dsc dscname = {description, prgname, icon}
#else /* WITH_LOADER_ARCH */
#define DSC(dscname, description, prgname, initfunc, icon) \
    void initfunc(char *arg); \
    struct dsc dscname = {description, initfunc, icon}
#endif /* WITH_LOADER_ARCH */

#define DSC_HEADER(name) extern struct dsc name;

#ifndef NULL
#define NULL 0
#endif /* NULL */

#endif /* _DSC_H__ */
