
#include "contiki.h"
#include "ircc.h"

#include "ctk.h"
#include "ctk-textedit.h"

#include <string.h>

#define LOG_WIDTH 38
#define LOG_HEIGHT 21

EK_EVENTHANDLER(eventhandler, ev, data);
EK_PROCESS(p, "IRC client", EK_PRIO_NORMAL,
	   eventhandler, NULL, NULL);
static ek_id_t id = EK_ID_NONE;

static struct ctk_window window;
static char log[LOG_WIDTH * LOG_HEIGHT];
static char line[LOG_WIDTH];
static struct ctk_label loglabel =
  {CTK_LABEL(0, 0, LOG_WIDTH, LOG_HEIGHT, log)};
static struct ctk_textedit lineedit =
  {CTK_TEXTEDIT(0, LOG_HEIGHT, LOG_WIDTH, 1, line)};

static struct ctk_window setupwindow;
#define SETUPWINDOW_WIDTH 18
#define SETUPWINDOW_HEIGHT 9
#define MAX_SERVERLEN 32
#define MAX_NICKLEN 16
static u16_t serveraddr[2];
static char server[MAX_SERVERLEN];
static char nick[MAX_NICKLEN];
static struct ctk_label serverlabel =
  {CTK_LABEL(1, 1, 11, 1, "IRC server: ")};
static struct ctk_textentry serverentry =
  {CTK_TEXTENTRY(0, 2, 16, 1, server, sizeof(server))};

static struct ctk_label nicklabel =
  {CTK_LABEL(1, 4, 13, 1, "IRC nickname: ")};
static struct ctk_textentry nickentry =
  {CTK_TEXTENTRY(0, 5, 16, 1, nick, sizeof(nick))};

static struct ctk_button connectbutton =
  {CTK_BUTTON(0, 7, 7, "Connect")};
static struct ctk_button quitbutton =
  {CTK_BUTTON(12, 7, 4, "Quit")};

/*static char nick[] = "asdf";
  static char server[] = "efnet.demon.co.uk";*/

static struct ircc_state s;

/*---------------------------------------------------------------------------*/
LOADER_INIT_FUNC(irc_init, arg)
{
  arg_free(arg);
  
  if(id == EK_ID_NONE) {
    id = ek_start(&p);
  }
}
/*---------------------------------------------------------------------------*/
static void
quit(void)
{
  ctk_window_close(&window);
  ctk_window_close(&setupwindow);
  ek_exit();
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*---------------------------------------------------------------------------*/
void
ircc_text_output(struct ircc_state *s, char *text1, char *text2)
{
  char *ptr;
  int len, len2;
  
  if(text1 == NULL) {
    text1 = "";
  }
  
  if(text2 == NULL) {
    text2 = "";
  }
  
  /* Scroll previous entries upwards */
  memcpy(log, &log[LOG_WIDTH], LOG_WIDTH * (LOG_HEIGHT - 1));

  ptr = &log[LOG_WIDTH * (LOG_HEIGHT - 1)];
  len = strlen(text1);

  memset(ptr, 0, LOG_WIDTH);
  strncpy(ptr, text1, LOG_WIDTH);
  ptr += len;
  *ptr = ':';
  *(ptr + 1) = ' ';
  if(LOG_WIDTH - len - 2 > 0) {
    strncpy(ptr + 2, text2, LOG_WIDTH - len - 2);
  }

  len2 = strlen(text2);
  if(len2 > LOG_WIDTH - len - 2) {
    memcpy(log, &log[LOG_WIDTH], LOG_WIDTH * (LOG_HEIGHT - 1));
    strncpy(&log[LOG_WIDTH * (LOG_HEIGHT - 1)],
	    text2 + LOG_WIDTH - 2 - len, LOG_WIDTH);
  }
  CTK_WIDGET_REDRAW(&loglabel);
  
}
/*---------------------------------------------------------------------------*/
static void
parse_line(void)
{
  int i;
  for(i = 0; i < strlen(line); ++i) {
    line[i] &= 0x7f;
  }
  
  
  if(line[0] == '/') {
    if(strncmp(&line[1], "join", 4) == 0) {
      ircc_join(&s, &line[6]);
      ircc_text_output(&s, "Join", &line[6]);
    } else if(strncmp(&line[1], "list", 4) == 0) {
      ircc_list(&s);
      ircc_text_output(&s, "Channel list", "");
    } else if(strncmp(&line[1], "part", 4) == 0) {
      ircc_part(&s);
      ircc_text_output(&s, "Leaving channel", "");
    } else if(strncmp(&line[1], "quit", 4) == 0) {
      ircc_quit(&s);
    } else {
      ircc_text_output(&s, &line[1], "Not implemented");
      ircc_sent(&s);
    }
  } else {
    petsciiconv_toascii(line, sizeof(line));
    ircc_msg(&s, &line[0]);
    ircc_text_output(&s, nick, line);
  }
	   
}
/*---------------------------------------------------------------------------*/
void
ircc_sent(struct ircc_state *s)
{
  memset(line, 0, sizeof(line));
  ctk_textedit_init(&lineedit);
  CTK_WIDGET_REDRAW(&lineedit);
}
/*---------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{
  ctk_arch_key_t c;
  u16_t *ipaddr;
  
  if(ev == EK_EVENT_INIT) {
    ctk_textedit_init(&lineedit);
    memset(line, 0, sizeof(line));
    memset(log, 0, sizeof(log));
    ctk_window_new(&window, LOG_WIDTH, LOG_HEIGHT + 1, "IRC");
    CTK_WIDGET_ADD(&window, &loglabel);
    ctk_textedit_add(&window, &lineedit);    


    ctk_window_new(&setupwindow, SETUPWINDOW_WIDTH, SETUPWINDOW_HEIGHT,
		   "IRC setup");

    CTK_WIDGET_ADD(&setupwindow, &serverlabel);
    CTK_WIDGET_ADD(&setupwindow, &serverentry);
    CTK_WIDGET_ADD(&setupwindow, &nicklabel);
    CTK_WIDGET_ADD(&setupwindow, &nickentry);
    CTK_WIDGET_ADD(&setupwindow, &connectbutton);
    CTK_WIDGET_ADD(&setupwindow, &quitbutton);

    ctk_window_open(&setupwindow);

  } else if(ev == EK_EVENT_REQUEST_EXIT ||
	    ev == ctk_signal_window_close) {
    quit();
  } else if(ev == tcpip_event) {
    ircc_appcall(data);
  } else if(ev == ctk_signal_widget_activate) {
    if(data == (ek_data_t)&lineedit) {
      parse_line();
    } else if(data == (ek_data_t)&quitbutton) {
      quit();
    } else if(data == (ek_data_t)&connectbutton) {
      ctk_window_close(&setupwindow);
      ctk_window_open(&window);
      ipaddr = serveraddr;
      if(uiplib_ipaddrconv(server, (u8_t *)serveraddr) == 0) {
	ipaddr = resolv_lookup(server);
	if(ipaddr == NULL) {
	  resolv_query(server);
	} else {
	  uip_ipaddr_copy(serveraddr, ipaddr);
	}
      }
      if(ipaddr != NULL) {
	       
	ircc_connect(&s, server, serveraddr, nick);
      }
    }
  } else if(ev == resolv_event_found) {

    ipaddr = resolv_lookup(server);
    if(ipaddr == NULL) {
      ircc_text_output(&s, server, "hostname not found");
    } else {
      uip_ipaddr_copy(serveraddr, ipaddr);
      ircc_connect(&s, server, serveraddr, nick);
    }
	       
  } else if(ev == ctk_signal_keypress) {
    c = (ctk_arch_key_t)data;
    if(c == CH_ENTER) {
      parse_line();
    } else {
      ctk_textedit_eventhandler(&lineedit, ev, data);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
ircc_closed(struct ircc_state *s)
{
  ircc_text_output(s, server, "connection closed");
}
/*---------------------------------------------------------------------------*/
void
ircc_connected(struct ircc_state *s)
{
  ircc_text_output(s, server, "connected");
}
/*---------------------------------------------------------------------------*/
