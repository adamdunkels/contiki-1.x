#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void
debug_print(char *s)
{
  OutputDebugString(s);
}
