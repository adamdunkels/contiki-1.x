/**
 * \file
 * Linked list manipulation routines.
 * \author Adam Dunkels <adam@sics.se>
 *
 *
\code

struct packet {
   struct packet *next;
   char data[1500];
   int len;
};

LIST(packets, struct packet *);

static void
init_function(void) {
   list_init(&packets);
}

static void
another_function(struct packet *p) {
   list_add(packets, p);

   p = list_head(packets);

   p = list_tail(packets);
}

\endcode 
 */

#ifndef __LIST_H__
#define __LIST_H__

/**
 */
#define LIST(name, type) \
         static type name##_list; \
         static void **name = (void **)&name##_list; 
         

void  list_init(void **list);
void *list_head(void **list);
void *list_tail(void **list);
void *list_pop (void **list);
void  list_push(void **list, void *item);
void  list_copy(void **dest, void **src);

#endif /* __LIST_H__ */




