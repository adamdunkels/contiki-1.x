
#include "packet-service.h"
#include "ppp.h"

static void output(u8_t *hdr, u16_t hdrlen, u8_t *data, u16_t datalen);

static const struct packet_service_state state =
  {
    PACKET_SERVICE_VERSION,
    output
  };

EK_EVENTHANDLER(eventhandler, ev, data);
EK_POLLHANDLER(pollhandler);
EK_PROCESS(proc, PACKET_SERVICE_NAME, EK_PRIO_NORMAL,
	   eventhandler, pollhandler, (void *)&state);

/*---------------------------------------------------------------------------*/
EK_PROCESS_INIT(ppp_service_init, arg)
{
  ek_service_start(PACKET_SERVICE_NAME, &proc);
}
/*---------------------------------------------------------------------------*/
static void
output(u8_t *hdr, u16_t hdrlen, u8_t *data, u16_t datalen)
{
  ppp_send();
}
/*---------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{
  switch(ev) {
  case EK_EVENT_INIT:
  case EK_EVENT_REPLACE:
    ppp_init();
    break;
  case EK_EVENT_REQUEST_REPLACE:
    ek_replace((struct ek_proc *)data, NULL);
    LOADER_UNLOAD();
    break;
  case EK_EVENT_REQUEST_EXIT:
    ek_exit();
    LOADER_UNLOAD();
    break;
  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
EK_POLLHANDLER(pollhandler)
{
  ppp_poll();
  if(uip_len > 0) {
    tcpip_input();
  }
}
/*---------------------------------------------------------------------------*/
