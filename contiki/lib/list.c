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
 * $Id: list.c,v 1.3 2004/09/12 20:24:55 adamdunkels Exp $
 */
#include "list.h"

#define NULL 0

struct list {
  struct list *next;
};

/*------------------------------------------------------------------------------*/
void
list_init(void **list)
{
  *list = NULL;
}
/*------------------------------------------------------------------------------*/
void *
list_head(void **list)
{
  return *list;
}
/*------------------------------------------------------------------------------*/
void
list_copy(void **dest, void **src)
{
  *dest = *src;
}
/*------------------------------------------------------------------------------*/
void *
list_tail(void **list)
{
  struct list *l;
  
  if(*list == NULL) {
    return NULL;
  }
  
  for(l = *list; l->next != NULL; l = l->next);
  
  return l;
}
/*------------------------------------------------------------------------------*/
/**
 * Add an item at the end of the list.
 *
 */
void
list_add(void **list, void *item)
{
  struct list *l;

  ((struct list *)item)->next = NULL;
  
  l = list_tail(list);

  if(l == NULL) {
    *list = item;
  } else {
    l->next = item;
  }
}
/*------------------------------------------------------------------------------*/
/**
 * Add an item to the start of the list.
 */
void
list_push(void **list, void *item)
{
  struct list *l;

  ((struct list *)item)->next = *list;
  *list = item;
}
/*------------------------------------------------------------------------------*/
/**
 * Remove the last object on the list.
 *
 * @return The removed object
 */
void *
list_chop(void **list)
{
  struct list *l, *r;
  
  if(*list == NULL) {
    return NULL;
  }
  if(((struct list *)*list)->next == NULL) {
    l = *list;
    *list = NULL;
    return l;
  }
  
  for(l = *list; l->next->next != NULL; l = l->next);

  r = l->next;
  l->next = NULL;
  
  return r;
}
/*------------------------------------------------------------------------------*/
/**
 * Remove the first object on the list.
 *
 * @return The new head of the list.
 */
/*------------------------------------------------------------------------------*/
void *
list_pop(void **list)
{
  struct list *l;
  
  if(*list != NULL) {   
    *list = ((struct list *)*list)->next;
  }

  return *list;
}
/*------------------------------------------------------------------------------*/
