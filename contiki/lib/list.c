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
