#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

void
debug_printf(char *format, ...)
{
  va_list argptr;
  char buffer[1024];

  va_start(argptr, format);
  vsprintf(buffer, format, argptr);
  va_end(argptr);

  OutputDebugString(buffer);
}

void
uip_log(char *message)
{
  debug_printf("%s\n", message);
}