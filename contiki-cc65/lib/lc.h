/* Local continuations: a form of continuation that only works locally
   within a function.
*/

#ifndef __LC_H__
#define __LC_H__

#include "cc.h"

typedef void * lc_t;

void __fastcall__ lc_set(lc_t *lc);
void __fastcall__ lc_resume(lc_t *lc);

#define LC_SET(lc)    lc_set(&(lc))
#define LC_RESUME(lc) lc_resume(&(lc))
#define LC_INIT(lc)   (lc) = NULL
#define LC_END(lc)

#endif /* __LC_H__ */
