/**
 * \addtogroup uip
 * @{
 */

/**
 * \defgroup uipsplit uIP TCP throughput booster hack
 * @{
 *
 * The basic uIP TCP implementation only allows each TCP connection to
 * have a single TCP segment in flight at any given time. Because of
 * the delayed ACK algorithm employed by most TCP receivers, uIP's
 * limit on the amount of in-flight TCP segments seriously reduces the
 * maximum achievable throughput for sending data from uIP.
 *
 * The uip-split module is a hack which tries to remedy this
 * situation. By splitting maximum sized outgoing TCP segments into
 * two, the delayed ACK algorithm is not invoked at TCP
 * receivers. This improves the throughput when sending data from uIP
 * by orders of magnitude.
 *
 * The uip-split module uses the uip-fw module (uIP IP packet
 * forwarding) for sending packets. Therefore, the uip-fw module must
 * be set up with the appropriate network interfaces for this module
 * to work.
 */


/**
 * \file
 * Module for splitting outbound TCP segments in two to avoid the
 * delayed ACK throughput degradation. 
 * \author
 * Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __UIP_SPLIT_H__
#define __UIP_SPLIT_H__

/**
 * Handle outgoing packets.
 *
 * This function inspects an outgoing packet in the uip_buf buffer and
 * sends it out using the uip_fw_output() function. If the packet is a
 * full-sized TCP segment it will be split into two segments and
 * transmitted separately. This function should be called instead of
 * the actual device driver output function, or the uip_fw_output()
 * function.
 *
 * The headers of the outgoing packet is assumed to be in the uip_buf
 * buffer and the payload is assumed to be wherever uip_appdata
 * points. The length of the outgoing packet is assumed to be in the
 * uip_len variable.
 *
 */
void uip_split_output(void);

#endif /* __UIP_SPLIT_H__ */

/** @} */
/** @} */
