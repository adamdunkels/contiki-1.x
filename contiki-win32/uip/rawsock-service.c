#include "packet-service.h"
#include "rawsock.h"

static struct packet_service_state
state = {PACKET_SERVICE_VERSION, rawsock_send};

EK_EVENTHANDLER(eventhandler, ev, data);
EK_POLLHANDLER(pollhandler);
EK_PROCESS(proc, PACKET_SERVICE_NAME, EK_PRIO_NORMAL,
	   eventhandler, pollhandler, &state);

/*---------------------------------------------------------------------------*/
EK_PROCESS_INIT(packet_service_init, arg)
{
  ek_service_start(PACKET_SERVICE_NAME, &proc);
}
/*---------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{
  switch(ev) {
  case EK_EVENT_INIT:
  case EK_EVENT_REPLACE:
    rawsock_init();
    break;
  case EK_EVENT_REQUEST_REPLACE:
    ek_replace(data, NULL);
    LOADER_UNLOAD();
    break;
  case EK_EVENT_REQUEST_EXIT:
    rawsock_fini();
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
  uip_len = rawsock_poll();
  if(0 < uip_len) tcpip_input();
}
/*---------------------------------------------------------------------------*/
