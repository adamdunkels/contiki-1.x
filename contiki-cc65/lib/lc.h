/* Local continuations: a form of continuation that only works locally
   within a function.
*/

#ifndef __LC_H__
#define __LC_H__

#include "cc.h"

typedef void * lc_t;

lc_t lc_get(void);
void __fastcall__ lc_resume(lc_t s);

extern lc_t lc_tmp;

#define LC_SET(s) lc_tmp = lc_get(); s = lc_tmp
#define LC_RESUME(s) lc_resume(s)

#define LC_INIT(s) s = NULL

#define LC_END(s)

#endif /* __LC_H__ */
