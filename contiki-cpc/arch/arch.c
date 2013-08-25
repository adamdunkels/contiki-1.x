#include "clock.h"
#include "arch.h"

extern clock_time_t _clock_val;

clock_time_t clock_time()
{
	return _clock_val;
}

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int ctk_arch_isprint(char c)
{
	return isprint(c);
}
