#ifndef __FTPC_H__
#define __FTPC_H__

#include "uip.h"

void ftpc_init(void);

void *ftpc_connect(u16_t *ipaddr, u16_t port);
char ftpc_list(void *connection);
void ftpc_cwd(void *connection, char *dir);
void ftpc_cdup(void *connection);
char ftpc_get(void *connection, char *filename);
void ftpc_close(void *connection);


void ftpc_appcall(void *state);

#define FTPC_OK        200
#define FTPC_COMPLETED 250
#define FTPC_NODIR     431
#define FTPC_NOTDIR    550

/* Functions to be implemented by the calling module: */
void ftpc_connected(void *connection);
void ftpc_cwd_done(unsigned short status);
char *ftpc_username(void);
char *ftpc_password(void);
void ftpc_closed(void);
void ftpc_aborted(void);
void ftpc_timedout(void);
void ftpc_list_file(char *filename);
void ftpc_data(u8_t *data, u16_t len);

#endif /* __FTPC_H__ */
