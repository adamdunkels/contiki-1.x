
#include "uipbuf.h"

#include <string.h>

/*---------------------------------------------------------------------------*/
void
uipbuf_setup(struct uipbuf_buffer *buf,
	     u8_t *bufptr, u16_t bufsize)
{
  buf->buffer = buf->ptr = bufptr;
  buf->bufsize = buf->left = bufsize;
}
/*---------------------------------------------------------------------------*/
u8_t
uipbuf_bufdata(struct uipbuf_buffer *buf, u16_t len,
	       u8_t **dataptr, u16_t *datalen)
{
  if(*datalen < buf->left) {
    memcpy(buf->ptr, *dataptr, *datalen);
    buf->ptr += *datalen;
    buf->left -= *datalen;
    *dataptr += *datalen;
    *datalen = 0;
    return UIPBUF_NOT_FULL;
  } else if(*datalen == buf->left) {
    memcpy(buf->ptr, *dataptr, *datalen);
    buf->ptr += *datalen;
    buf->left = 0;
    *dataptr += *datalen;
    *datalen = 0;
    return UIPBUF_FULL;
  } else {
    memcpy(buf->ptr, *dataptr, buf->left);
    buf->ptr += buf->left;
    *datalen -= buf->left;
    *dataptr += buf->left;
    buf->left = 0;
    return UIPBUF_FULL;
  }

}
/*---------------------------------------------------------------------------*/
u8_t
uipbuf_bufdata_endmarker(struct uipbuf_buffer *buf, u8_t endmarker,
			 u8_t **dataptr, u16_t *datalen)
{
  u8_t c;
    
  while(buf->left > 0 && *datalen > 0) {
    c = *buf->ptr = **dataptr;
    ++*dataptr;
    ++buf->ptr;
    --*datalen;
    --buf->left;
    
    if(c == endmarker) {
      return UIPBUF_FOUND;
    }
  }

  if(*datalen == 0) {
    return UIPBUF_NOT_FOUND;
  }

  while(*datalen > 0) {
    c = **dataptr;
    --*datalen;
    ++*dataptr;
    
    if(c == endmarker) {
      return UIPBUF_FOUND | UIPBUF_FULL;
    }
  }
  
  return UIPBUF_FULL;
}
/*----------------------------------------------------------------------------*/u16_t
uipbuf_len(struct uipbuf_buffer *buf)
{
  return buf->bufsize - buf->left;
}
/*----------------------------------------------------------------------------*/
