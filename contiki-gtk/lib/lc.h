/* Local continuations: a form of continuation that only works locally
   within a function.

   NOTE: this particular implementation only works with GCC!
*/

#ifndef __LC_H__
#define __LC_H__

typedef void * lc_t;

#define __LC_CONCAT2(s1, s2) s1##s2
#define __LC_CONCAT(s1, s2) __LC_CONCAT2(s1, s2)

#define LC_SET(s)				\
  do {						\
     __LC_CONCAT(LC_LABEL, __LINE__):   	\
      (s) = &&__LC_CONCAT(LC_LABEL, __LINE__);	\
  } while(0)

#define LC_RESUME(s) goto *s

#define LC_NULL (lc_t)0

#endif /* __LC_H__ */
