/* Local continuations: a form of continuation that only works locally
   within a function.
*/

#ifndef __LC_H__
#define __LC_H__

#include "cc.h"

typedef void * lc_t;

lc_t lc_get(void);
void __fastcall__ lc_resume(lc_t s);

#define LC_SET(s) s = lc_get()
#define LC_RESUME(s) lc_resume(s)

#define LC_INIT(s) s = NULL

#define LC_END(s)

#endif /* __LC_H__ */
