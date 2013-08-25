#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#include "uip.h"
#include "log.h"

/*-----------------------------------------------------------------------------------*/
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
/*-----------------------------------------------------------------------------------*/
void
uip_log(char *message)
{
  debug_printf("%s\n", message);
}
/*-----------------------------------------------------------------------------------*/
void
log_message(const char *part1, const char *part2)
{
  debug_printf("%s%s\n", part1, part2);
}
/*-----------------------------------------------------------------------------------*/
