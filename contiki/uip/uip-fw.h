#ifndef __UIP_FW_H__
#define __UIP_FW_H__

/**
 * Representation of a uIP network interface.
 */
struct uip_fw_netif {
  struct uip_fw_netif *next;  /**< Pointer to the next interface when
				 linked in a list. */
  u16_t ipaddr[2];            /**< The IP address of this interface. */
  u16_t netmask[2];           /**< The netmask of the interface. */
  void (* output)(void);
                              /**< A pointer to the function that
				 sends a packet. */
};

/**
 * Intantiating macro for a uIP network interface.
 *
 * Example:
 \code
 struct uip_fw_netif slipnetif =
   {UIP_FW_NETIF(192,168,76,1, 255,255,255,0, slip_output)};
 \endcode
 * \param ip1,ip2,ip3,ip4 The IP address of the network interface.
 *
 * \param nm1,nm2,nm3,nm4 The netmask of the network interface.
 *
 * \param outputfunc A pointer to the output function of the network interface.
 *
 */
#define UIP_FW_NETIF(ip1,ip2,ip3,ip4, nm1,nm2,nm3,nm4, outputfunc) \
        NULL, \
	{HTONS((ip1 << 8) | ip2), HTONS((ip3 << 8) | ip4)}, \
	{HTONS((nm1 << 8) | nm2), HTONS((nm3 << 8) | nm4)}, \
        outputfunc

void uip_fw_init(void);
unsigned char uip_fw_forward(void);
void uip_fw_output(void);
void uip_fw_register(struct uip_fw_netif *netif);
void uip_fw_default(struct uip_fw_netif *netif);

#endif /* __UIP_FW_H__ */
