/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the Contiki operating system.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: memb.h,v 1.6 2004/09/12 20:24:55 adamdunkels Exp $
 */
/**
 * \addtogroup memb 
 * @{
 */

/**
 * \file
 * Memory block allocation routines.
 * \author Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __MEMB_H__
#define __MEMB_H__

/**
 * Declare a memory block.
 *
 * This macro is used to staticall declare a block of memory that can
 * be used by the block allocation functions. The macro statically
 * declares a C array with a size that matches the specified number of
 * blocks and their individual sizes.
 *
 * Example:
 \code
MEMB(connections, sizeof(struct connection), 16);
 \endcode
 *
 * \param name The name of the memory block (later used with
 * memb_init(), memb_alloc() and memb_free()).
 *
 * \param size The size of each memory chunk, in bytes.
 *
 * \param num The total number of memory chunks in the block.
 *
 */
#if CC_DOUBLE_HASH
#define MEMB(name, size, num) \
        static char name##_memb_mem[(size + 1) * num]; \
        static struct memb_blocks name = {size, num, name##_memb_mem}
#else /* CC_DOUBLE_HASH */
#define MEMB(name, size, num) \
        static char name_memb_mem[(size + 1) * num]; \
        static struct memb_blocks name = {size, num, name_memb_mem}
#endif /* CC_DOUBLE_HASH */

struct memb_blocks {
  unsigned short size;
  unsigned short num;
  char *mem;
};

void  memb_init(struct memb_blocks *m);
char *memb_alloc(struct memb_blocks *m);
char  memb_ref(struct memb_blocks *m, char *ptr);
char  memb_free(struct memb_blocks *m, void *ptr);

/** @} */

#endif /* __MEMB_H__ */
