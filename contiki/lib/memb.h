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
