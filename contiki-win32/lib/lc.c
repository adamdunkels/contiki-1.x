#include "lc.h"

/*-----------------------------------------------------------------------------------*/
__declspec(naked) void
lc_set(lc_t *lc)
{
  __asm {
	  mov	eax,[esp+4]

	  mov	ecx,[esp]
	  mov	[eax]lc.eip,ecx

	  mov	[eax]lc.ebx,ebx
	  mov	[eax]lc.edi,edi
	  mov	[eax]lc.esi,esi

	  ret
  }
}
/*-----------------------------------------------------------------------------------*/
__declspec(naked) void
lc_resume(lc_t *lc)
{
  __asm {
	  mov	eax,[esp+4]

	  mov	ecx,[eax]lc.eip
	  test  ecx,ecx
	  je	done
	  mov	[esp],ecx

	  mov	ebx,[eax]lc.ebx
	  mov	edi,[eax]lc.edi
	  mov	esi,[eax]lc.esi

    done: ret
  }
}
/*-----------------------------------------------------------------------------------*/
