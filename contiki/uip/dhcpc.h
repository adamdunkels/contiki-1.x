#ifndef __DHCPC_H__
#define __DHCPC_H__

void dhcpc_init(void);
void dhcpc_request(void);

void dhcpc_appcall(void *state);

void dhcpc_configured(void);

#endif /* __DHCPC_H__ */
