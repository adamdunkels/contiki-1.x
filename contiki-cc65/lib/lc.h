/* Local continuations: a form of continuation that only works locally
   within a function.
*/

#ifndef __LC_H__
#define __LC_H__

typedef void * lc_t;

lc_t lc_get(void);
void __fastcall__ lc_call(lc_t s);

#define LC_SET(s) s = lc_get()
#define LC_CALL(s) lc_call(s) 

#define LC_NULL NULL

#endif /* __LC_H__ */
