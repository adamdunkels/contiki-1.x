#include "ctk.h"
#include "dispatcher.h"
#include "loader.h"
#include "sensors.h"

#include "uip-signal.h"

#include <stdio.h>


static void sensorcheck_idle(void);   
static DISPATCHER_SIGHANDLER(sensorcheck_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("Sensor check", sensorcheck_idle,
		   sensorcheck_sighandler, NULL)};
static ek_id_t id = EK_ID_NONE;

static unsigned char dataavail;

static struct uip_udp_conn *conn;
static int vib;

/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(sensorcheck_init, arg)
{
  u16_t server[2];
  arg_free(arg);
    
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    uip_ipaddr(server, 192,168,1,1);
    conn = uip_udp_new(server, 5125);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
sensorcheck_quit(void)
{
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(sensorcheck_sighandler, s, data)
{
  DISPATCHER_SIGHANDLER_ARGS(s, data);

  if(s == dispatcher_signal_quit) {
    sensorcheck_quit();
  }
}
/*-----------------------------------------------------------------------------------*/
static void
sensorcheck_idle(void)
{
  /* Check vibration sensor, and send out data. */
  if(sensors_vib != vib) {
    vib = sensors_vib;
    ++dataavail;
    if(conn != NULL) {
      dispatcher_emit(uip_signal_poll_udp, conn, DISPATCHER_BROADCAST);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
void
udp_appcall(void)
{
  if(dataavail) {
    *(int *)uip_appdata = vib;
    uip_udp_send(sizeof(int));
    dataavail = 0;
  }
}
/*-----------------------------------------------------------------------------------*/
