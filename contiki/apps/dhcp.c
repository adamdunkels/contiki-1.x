#include "contiki.h"
#include "ctk.h"
#include "dhcpc.h"

EK_EVENTHANDLER(eventhandler, ev, data);
EK_PROCESS(p, "DHCP", EK_PRIO_NORMAL,
	   eventhandler, NULL, NULL);
static ek_id_t id = EK_ID_NONE;

static struct ctk_window window;
static struct ctk_button getbutton =
  {CTK_BUTTON(0, 0, 16, "Request address")};
static struct ctk_label statuslabel =
  {CTK_LABEL(0, 1, 16, 1, "")};


static struct ctk_label ipaddrlabel =
  {CTK_LABEL(0, 3, 10, 1, "IP address")};
static char ipaddr[17];
static struct ctk_textentry ipaddrentry =
  {CTK_LABEL(11, 3, 16, 1, ipaddr)};
static struct ctk_label netmasklabel =
  {CTK_LABEL(0, 4, 10, 1, "Netmask")};
static char netmask[17];
static struct ctk_textentry netmaskentry =
  {CTK_LABEL(11, 4, 16, 1, netmask)};
static struct ctk_label gatewaylabel =
  {CTK_LABEL(0, 5, 10, 1, "Gateway")};
static char gateway[17];
static struct ctk_textentry gatewayentry =
  {CTK_LABEL(11, 5, 16, 1, gateway)};
static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 6, 10, 1, "DNS server")};
static char dnsserver[17];
static struct ctk_textentry dnsserverentry =
  {CTK_LABEL(11, 6, 16, 1, dnsserver)};

enum {
  SHOWCONFIG
};
/*---------------------------------------------------------------------------*/
LOADER_INIT_FUNC(dhcp_init, arg)
{
  arg_free(arg);
  if(id == EK_ID_NONE) {
    id = ek_start(&p);
  }
}
/*---------------------------------------------------------------------------*/
static void
set_statustext(char *text)
{
  ctk_label_set_text(&statuslabel, text);
  CTK_WIDGET_REDRAW(&statuslabel);
}
/*---------------------------------------------------------------------------*/
static char *
makebyte(u8_t byte, char *str)
{
  if(byte >= 100) {
    *str++ = (byte / 100 ) % 10 + '0';
  }
  if(byte >= 10) {
    *str++ = (byte / 10) % 10 + '0';
  }
  *str++ = (byte % 10) + '0';

  return str;
}
/*---------------------------------------------------------------------------*/
static void
makeaddr(u16_t *addr, char *str)
{
  str = makebyte(HTONS(addr[0]) >> 8, str);
  *str++ = '.';
  str = makebyte(HTONS(addr[0]) & 0xff, str);
  *str++ = '.';
  str = makebyte(HTONS(addr[1]) >> 8, str);
  *str++ = '.';
  str = makebyte(HTONS(addr[1]) & 0xff, str);
  *str++ = 0;
}
/*---------------------------------------------------------------------------*/
static void
makestrings(void)
{
  u16_t addr[2], *addrptr;

  uip_gethostaddr(addr);
  makeaddr(addr, ipaddr);
  
  uip_getnetmask(addr);
  makeaddr(addr, netmask);
  
  uip_getdraddr(addr);
  makeaddr(addr, gateway);

  addrptr = resolv_getserver();
  if(addrptr != NULL) {
    makeaddr(addrptr, dnsserver);
  }
 
}
/*---------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{
  if(ev == EK_EVENT_INIT) {
    ctk_window_new(&window, 28, 7, "DHCP");
    CTK_WIDGET_ADD(&window, &getbutton);
    CTK_WIDGET_ADD(&window, &statuslabel);
    
    CTK_WIDGET_ADD(&window, &ipaddrlabel);
    CTK_WIDGET_ADD(&window, &ipaddrentry);
    CTK_WIDGET_ADD(&window, &netmasklabel);
    CTK_WIDGET_ADD(&window, &netmaskentry);
    CTK_WIDGET_ADD(&window, &gatewaylabel);
    CTK_WIDGET_ADD(&window, &gatewayentry);
    CTK_WIDGET_ADD(&window, &dnsserverlabel);
    CTK_WIDGET_ADD(&window, &dnsserverentry);
    
    ctk_window_open(&window);
    dhcpc_init();
  } else if(ev == ctk_signal_widget_activate) {
    if(data == (ek_data_t)&getbutton) {
      dhcpc_request();
      set_statustext("Requesting...");
    }
  } else if(ev == tcpip_event) {
    dhcpc_appcall(data);
  } else if(ev == EK_EVENT_REQUEST_EXIT ||
	    ev == ctk_signal_window_close) {
    ctk_window_close(&window);
    ek_exit();
    id = EK_ID_NONE;
    LOADER_UNLOAD();
  } else if(ev == SHOWCONFIG) {
    makestrings();
    ctk_window_redraw(&window);  
  }
}
/*---------------------------------------------------------------------------*/
void
dhcpc_configured(void)
{
  set_statustext("Configured.");
  ek_post(EK_PROC_ID(EK_CURRENT()), SHOWCONFIG, NULL);
}
/*---------------------------------------------------------------------------*/
