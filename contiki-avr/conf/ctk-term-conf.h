#ifndef __CTK_TERM_CONF_H__
#define __CTK_TERM_CONF_H__

/* Maximum number of clients to the telnet server */
#define CTK_TERM_CONF_MAX_TELNET_CLIENTS 3

/* Telnet server port */
#define CTK_TERM_CONF_TELNET_PORT 23

/* Serial server output buffer size */
#define CTK_TERM_CONF_SERIAL_BUFFER_SIZE 300

/* Maximum number of clients to the terminal module.
   Should be set to CTK_TERM_CONF_MAX_TELNET_CLIENTS or
   CTK_TERM_CONF_MAX_TELNET_CLIENTS+1 if the serial server is used too
*/
#define CTK_TERM_CONF_MAX_CLIENTS (CTK_TERM_CONF_MAX_TELNET_CLIENTS+1)

#endif /* __CTK_TERM_CONF_H__ */

