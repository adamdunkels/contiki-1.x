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
  u8_t (* output)(void);
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
 * \hideinitializer
 */
#define UIP_FW_NETIF(ip1,ip2,ip3,ip4, nm1,nm2,nm3,nm4, outputfunc) \
        NULL, \
	{HTONS((ip1 << 8) | ip2), HTONS((ip3 << 8) | ip4)}, \
	{HTONS((nm1 << 8) | nm2), HTONS((nm3 << 8) | nm4)}, \
        outputfunc

/**
 * Set the IP address of a network interface.
 *
 * \param netif A pointer to the uip_fw_netif structure for the network interface.
 *
 * \param addr A pointer to an IP address.
 *
 * \hideinitializer
 */
#define uip_fw_setipaddr(netif, addr) \
        do { (netif)->ipaddr[0] = ((u16_t *)(addr))[0]; \
             (netif)->ipaddr[1] = ((u16_t *)(addr))[1]; } while(0)
/**
 * Set the netmask of a network interface.
 *
 * \param netif A pointer to the uip_fw_netif structure for the network interface.
 *
 * \param addr A pointer to an IP address representing the netmask.
 *
 * \hideinitializer
 */
#define uip_fw_setnetmask(netif, addr) \
        do { (netif)->netmask[0] = ((u16_t *)(addr))[0]; \
             (netif)->netmask[1] = ((u16_t *)(addr))[1]; } while(0)

void uip_fw_init(void);
u8_t uip_fw_forward(void);
u8_t uip_fw_output(void);
void uip_fw_register(struct uip_fw_netif *netif);
void uip_fw_default(struct uip_fw_netif *netif);


/**
 * A non-error message that indicates that a packet should be
 * processed locally.
 *
 * \hideinitializer
 */
#define UIP_FW_LOCAL     0

/**
 * A non-error message that indicates that something went OK.
 *
 * \hideinitializer
 */
#define UIP_FW_OK        0

/**
 * A non-error message that indicates that a packet was forwarded.
 *
 * \hideinitializer
 */
#define UIP_FW_FORWARDED 1

/**
 * A non-error message that indicates that a zero-length packet
 * transmission was attempted, and that no packet was sent.
 *
 * \hideinitializer
 */
#define UIP_FW_ZEROLEN   2

/**
 * An error message that indicates that a packet that was too large
 * for the outbound network interface was detected.
 *
 * \hideinitializer
 */
#define UIP_FW_TOOLARGE  3

/**
 * An error message that indicates that no suitable interface could be
 * found for an outbound packet.
 *
 * \hideinitializer
 */
#define UIP_FW_NOROUTE   4

/**
 * An error message that indicates that a packet that should be
 * forwarded or output was dropped.
 *
 * \hideinitializer
 */
#define UIP_FW_DROPPED   5

#endif /* __UIP_FW_H__ */
