/**
 * \file
 * uIP data buffering helper functions.
 * \author
 * Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __UIPBUF_H__
#define __UIPBUF_H__

#include "uip.h"

/**
 * \defgroup uipbuf uIP data buffering helper library.
 *
 * The event driven API that uIP uses can be tricky to use when
 * dealing with incoming TCP data. The data can be split over any
 * number of incoming segments and uIP does not provide any stream
 * abstraction by itself. To remedy this, the uIP data buffering
 * helper library provides a set of functions that make buffering data
 * easier.
 *
 * The data buffering library provides a structure that holds a
 * pointer to a buffer and the state of the buffer, as well as a set
 * of functions for manipulating the buffer state. The functions are
 * intended to facilitate buffering of both data that is of a known
 * size and data that is terminated by a specified byte.
 *
 * @{
 */

/**
 * Return value of the buffering functions that indicates that a
 * buffer was not filled by incoming data.
 *
 * \hideinitializer
 */
#define UIPBUF_NOT_FULL 0
#define UIPBUF_NOT_FOUND 0

/**
 * Return value of the buffering functions that indicates that a
 * buffer was completely filled by incoming data.
 *
 * \hideinitializer
 */
#define UIPBUF_FULL 1

/**
 * Return value of the buffering functions that indicates that an
 * end-marker byte was found.
 *
 * \hideinitializer
 */
#define UIPBUF_FOUND 2

/**
 * The structure that holds the state of a uIP buffer.
 *
 * This structure holds the state of a uIP buffer. The structure has
 * no user-visible elements, but is used through the functions
 * provided by the library.
 *
 * \hideinitializer
 */
struct uipbuf_buffer {
  u8_t *ptr, *buffer;
  unsigned short left, bufsize;
};

/**
 * Set up a new uIP buffer structure.
 *
 * This function is used for setting up a uIP buffer structure with a
 * specified size. The function should be called the first time a uIP
 * buffer is used. The caller must provide the memory for holding the
 * buffered bytes and the size of the buffer memory.
 *
 * \param buf A pointer to a uipbuf_buffer structure that is to be
 * initialized.
 *
 * \param bufptr A pointer to the memory for holding the buffered
 * data.
 * 
 * \param size The size of the buffer memory.
 *
 */
void uipbuf_setup(struct uipbuf_buffer *buf,
		  u8_t *bufptr, u16_t size);

/**
 * Buffer data until the buffer is full.
 *
 * This function puts data into the buffer, but no more than the
 * buffer can hold.
 *
 * \param buf A pointer to the ::uipbuf_buffer structure that holds
 * the state of the buffer.
 *
 * \param dataptr A pointer to the data that is to be buffered.
 *
 * \param datalen The length of the data that is to be buffered.
 *
 * \return If the buffer was not filled, the value UIPBUF_NOT_FULL
 * is returned. If the buffer was filled, the number of bytes that
 * could not be buffered is returned. If the buffer was exactly filled
 * with the data, the value of 0 is returned.
 *
 */
u8_t uipbuf_bufdata(struct uipbuf_buffer *buf, u16_t len,
		    u8_t **dataptr, u16_t *datalen);

/**
 * Buffer data until a specific character is found or the buffer is full.
 *
 * This function puts data into the buffer until a specific marker
 * byte is found. The marker byte is put into the buffer at the end of
 * the data.
 *
 * \param buf A pointer to the ::uipbuf_buffer structure that holds
 * the state of the buffer.
 *
 * \param dataptr A pointer to the data that is to be buffered.
 *
 * \param datalen The length of the data that is to be buffered.
 *
 * \param byte The end-marker byte that indicates the end of the data
 * that is to be buffered.
 * 
 * \return This function returns the number of protruding bytes after
 * the end-marker byte, if the marker was found. If the marker was not
 * found and all of the data was buffered, the value of
 * UIPBUF_NOT_FOUND is returned. If the marker was not found, but the
 * data made the buffer fill up, the value of UIPBUF_FULL is returned.
 *
 */
u8_t uipbuf_bufdata_endmarker(struct uipbuf_buffer *buf, u8_t endmarker,
			      u8_t **dataptr, u16_t *datalen);

u16_t uipbuf_len(struct uipbuf_buffer *buf);

/** @} */

#endif /* __UIPBUF_H__ */
