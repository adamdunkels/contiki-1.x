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
 * $Id: memb.c,v 1.5 2004/09/12 20:24:55 adamdunkels Exp $
 */
/**
 * \defgroup memb Memory block management functions
 *
 * The memory block allocation routines provide a simple yet powerful
 * set of functions for managing a set of memory blocks of fixed
 * size. A set of memory blocks is statically declared with the
 * MEMB() macro. Memory blocks are allocated from the declared
 * memory by the memb_alloc() function, and are deallocated with the
 * memb_free() function.
 *
 * \note Because of namespace clashes only one MEMB() can be
 * declared per C module, and the name scope of a MEMB() memory
 * block is local to each C module.
 *
 * The following example shows how to declare and use a memory block
 * called "cmem" which has 8 chunks of memory with each memory chunk
 * being 20 bytes large.
 *
 \code
 #include "memb.h"
 
 MEMB(cmem, 20, 8);

 int main(int argc, char *argv[]) {
    char *ptr;
    
    memb_init(&cmem);

    ptr = memb_alloc(&cmem);

    if(ptr != NULL) {
       do_something(ptr);
    } else {
       printf("Could not allocate memory.\n");
    }

    if(memb_free(&cmem, ptr) == 0) {
       printf("Deallocation succeeded.\n");
    }
 }
 \endcode
 *
 * @{
 */

 /**
 * \file
 * Memory block allocation routines.
 * \author Adam Dunkels <adam@sics.se>
 */
#include <string.h>

#include "memb.h"

/*------------------------------------------------------------------------------*/
/**
 * Initialize a memory block that was declared with MEMB().
 *
 * \param m A memory block previosly declared with MEMB().
 */
/*------------------------------------------------------------------------------*/
void
memb_init(struct memb_blocks *m)
{
  memset(m->mem, 0, (m->size + 1) * m->num);
}
/*------------------------------------------------------------------------------*/
/**
 * Allocate a memory block from a block of memory declared with MEMB().
 *
 * \param m A memory block previosly declared with MEMB().
 */
/*------------------------------------------------------------------------------*/
char *
memb_alloc(struct memb_blocks *m)
{
  int i;
  char *ptr;

  ptr = m->mem;
  for(i = 0; i < m->num; ++i) {
    if(*ptr == 0) {
      /* If this block was unused, we increase the reference count to
	 indicate that it now is used and return a pointer to the
	 first byte following the reference counter. */
      ++*ptr;
      return ptr + 1;
    }
    ptr += m->size + 1;
  }

  /* No free block was found, so we return NULL to indicate failure to
     allocate block. */
  return NULL;
}
/*------------------------------------------------------------------------------*/
/**
 * Deallocate a memory block from a memory block previously declared
 * with MEMB().
 *
 * \param m m A memory block previosly declared with MEMB().
 *
 * \param ptr A pointer to the memory block that is to be deallocated.
 *
 * \return The new reference count for the memory block (should be 0
 * if successfully deallocated) or -1 if the pointer "ptr" did not
 * point to a legal memory block.
 */
/*------------------------------------------------------------------------------*/
char
memb_free(struct memb_blocks *m, void *ptr)
{
  int i;
  char *ptr2;

  /* Walk through the list of blocks and try to find the block to
     which the pointer "ptr" points to. */
  ptr2 = m->mem;
  for(i = 0; i < m->num; ++i) {
    
    if(ptr2 == (char *)ptr - 1) {
      /* We've found to block to which "ptr" points so we decrease the
	 reference count and return the new value of it. */      
      return --*ptr2;
    }
    ptr2 += m->size + 1;
  }
  return -1;
}
/*------------------------------------------------------------------------------*/
/**
 * Increase the reference count for a memory chunk.
 *
 * \note No sanity checks are currently made.
 *
 * \param m m A memory block previosly declared with MEMB().
 *
 * \param ptr A pointer to the memory chunk for which the reference
 * count should be increased.
 *
 * \return The new reference count.
 */
/*------------------------------------------------------------------------------*/
char
memb_ref(struct memb_blocks *m, char *ptr)
{
  return ++*(ptr - 1);
}
/*------------------------------------------------------------------------------*/

/** @} */
