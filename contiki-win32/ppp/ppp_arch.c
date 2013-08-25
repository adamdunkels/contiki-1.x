#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

#include "ppp.h"
#include "ctk.h"

static HANDLE port = INVALID_HANDLE_VALUE;

/*-----------------------------------------------------------------------------------*/
static void
error_exit(char *message, int value)
{
  console_exit();
  cprintf(message, value);
  exit(EXIT_FAILURE);
}
/*-----------------------------------------------------------------------------------*/
void
ppp_arch_init(void)
{
  int i;
  char buf[256] = "";
  DCB dcb;
  COMMTIMEOUTS commtimeouts = {MAXDWORD, 0, 0, 0, 0};
  DWORD written;
  DWORD read;

  port = CreateFile(__argv[1], GENERIC_READ | GENERIC_WRITE,
		    0, NULL, OPEN_EXISTING, 0, NULL);
  if(port == INVALID_HANDLE_VALUE) {
    error_exit("CreateFile() error: %d\n", GetLastError());
  }

  if(!GetCommState(port, &dcb)) {
    error_exit("GetCommState() error: %d\n", GetLastError());
  }

  for(i = 2; i < __argc; i++) {
    strcat(buf, __argv[i]);
    strcat(buf, " ");
  }
  if(!BuildCommDCB(buf, &dcb)) {
    error_exit("BuildCommDCB() error: %d\n", GetLastError());
  }

  if(!SetCommState(port, &dcb)) {
    error_exit("SetCommState() error: %d\n", GetLastError());
  }

  if(!SetCommTimeouts(port, &commtimeouts)) {
    error_exit("SetCommTimeouts() error: %d\n", GetLastError());
  }

  if(!WriteFile(port, "CLIENTCLIENT", strlen("CLIENTCLIENT"), &written, NULL) || written != strlen("CLIENTCLIENT")) {
    error_exit("WriteFile(CLIENTCLIENT) error: %d\n", GetLastError());
  }

  Sleep(500);

  if(!ReadFile(port, buf, strlen("CLIENTSERVER"), &read, NULL) || read != strlen("CLIENTSERVER")) {
    error_exit("ReadFile(CLIENTSERVER) error: %d\n", GetLastError());
  }
}
/*-----------------------------------------------------------------------------------*/
void
ppp_arch_putchar(u8_t c)
{
  DWORD written;

  if(!WriteFile(port, &c, 1, &written, NULL)) {
    error_exit("WriteFile() error: %d\n", GetLastError());
  }

  if(written != 1) {
    error_exit("WriteFile() bytes written: %d\n", written);
  }
}
/*-----------------------------------------------------------------------------------*/
u8_t
ppp_arch_getchar(u8_t *c)
{
  DWORD read;

  if (port == INVALID_HANDLE_VALUE) {
    return 0;
  }

  if(!ReadFile(port, (LPVOID)c, 1, &read, NULL)) {
    error_exit("ReadFile() error: %d\n", GetLastError());
  }

  return (u8_t)read;
}
/*-----------------------------------------------------------------------------------*/
