
#include "contiki.h"
#include "ircc.h"

#include "ircc-strings.h"

#include <string.h>

#define PORT 6667

#define SEND_STRING(s, str) SOCKET_SEND(s, str, strlen(str))

#define ISO_space 0x20
#define ISO_bang  0x21
#define ISO_at    0x40
#define ISO_cr    0x0d
#define ISO_nl    0x0a
#define ISO_colon 0x3a
#define ISO_O     0x4f

enum {
  COMMAND_NONE,
  COMMAND_JOIN,
  COMMAND_PART,  
  COMMAND_MSG,
  COMMAND_ACTIONMSG,
  COMMAND_LIST,
  COMMAND_QUIT
};

/*---------------------------------------------------------------------------*/
void
ircc_init(void)
{

}
/*---------------------------------------------------------------------------*/
static char *
copystr(char *dest, const char *src, int n)
{
  int len;

  len = strlen(src);
  strncpy(dest, src, n);

  if(len > n) {
    return dest + n;
  } else {
    return dest + len;
  }
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(setup_connection(struct ircc_state *s))
{
  char *ptr;

  
  SOCKET_BEGIN(&s->s);
  
  ptr = s->outputbuf;
  ptr = copystr(ptr, ircc_nick, sizeof(s->outputbuf));
  ptr = copystr(ptr, s->nick, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, ircc_crnl_user, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, s->nick, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, ircc_contiki, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, s->server, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, ircc_colon_contiki, sizeof(s->outputbuf) - (ptr - s->outputbuf));

  SEND_STRING(&s->s, s->outputbuf);

  SOCKET_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(join_channel(struct ircc_state *s))
{
  SOCKET_BEGIN(&s->s);
  
  SEND_STRING(&s->s, ircc_join_);
  SEND_STRING(&s->s, s->channel);
  SEND_STRING(&s->s, ircc_crnl);

  ircc_sent(s);
  
  SOCKET_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(part_channel(struct ircc_state *s))
{
  SOCKET_BEGIN(&s->s);

  SEND_STRING(&s->s, ircc_part_);
  SEND_STRING(&s->s, s->channel);
  SEND_STRING(&s->s, ircc_crnl);

  ircc_sent(s);
  
  SOCKET_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(list_channel(struct ircc_state *s))
{
  SOCKET_BEGIN(&s->s);

  SEND_STRING(&s->s, ircc_list_);
  SEND_STRING(&s->s, s->channel);
  SEND_STRING(&s->s, ircc_crnl);

  ircc_sent(s);
  
  SOCKET_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_message(struct ircc_state *s))
{
  char *ptr;
  
  SOCKET_BEGIN(&s->s);

  ptr = s->outputbuf;
  ptr = copystr(ptr, ircc_privmsg, sizeof(s->outputbuf));
  ptr = copystr(ptr, s->channel, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, ircc_colon, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, s->msg, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, ircc_crnl, sizeof(s->outputbuf) - (ptr - s->outputbuf));

  SEND_STRING(&s->s, s->outputbuf);

  ircc_sent(s);
  
  SOCKET_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_actionmessage(struct ircc_state *s))
{
  char *ptr;
  
  SOCKET_BEGIN(&s->s);

  ptr = s->outputbuf;
  ptr = copystr(ptr, ircc_privmsg, sizeof(s->outputbuf));
  ptr = copystr(ptr, s->channel, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, ircc_colon, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, ircc_action, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, s->msg, sizeof(s->outputbuf) - (ptr - s->outputbuf)); 
  ptr = copystr(ptr, ircc_actioncrnl, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  

  SEND_STRING(&s->s, s->outputbuf);

  ircc_sent(s);
  
  SOCKET_END(&s->s);
}
/*---------------------------------------------------------------------------*/
struct parse_result {
  char *msg;
  
  char *user;
  char *host;
  char *name;
  char *command;
  char *middle;
  char *trailing;
};
static struct parse_result r;
static void
parse_whitespace(void)
{
  while(*r.msg == ISO_space) ++r.msg;
}
static void
parse_word(void)
{
  char *ptr;
  ptr = strchr(r.msg, ISO_space);
  if(ptr != NULL) {
    r.msg = ptr;
  }  
}
static void
parse_user(void)
{
  parse_whitespace();
  r.user = r.msg;
  parse_word();
  *r.msg = 0;
  ++r.msg;
}
static void
parse_host(void)
{
  parse_whitespace();
  r.host = r.msg;
  parse_word();
  *r.msg = 0;
  ++r.msg;
}

static void
parse_name(void)
{
  parse_whitespace();
  r.name = r.msg;
  parse_word();
  *r.msg = 0;
  ++r.msg;
}

static void
parse_prefix(void)
{
  parse_name();
  if(*r.msg == ISO_bang) {
    ++r.msg;
    parse_user();
  }
  if(*r.msg == ISO_at) {
    ++r.msg;
    parse_host();
  }
}

static void
parse_command(void)
{
  parse_whitespace();
  r.command = r.msg;
  parse_word();
  *r.msg = 0;
  ++r.msg;
}

/*static void
parse_trailing(void)
{
  r.trailing = r.msg;
  while(*r.msg != 0 && *r.msg != ISO_cr && *r.msg != ISO_nl) ++r.msg;
  *r.msg = 0;
  ++r.msg;
}*/

static void
parse_params(void)
{
  char *ptr;

  parse_whitespace();
  ptr = strchr(r.msg, ISO_colon);
  if(ptr != NULL) {
    r.trailing = ptr + 1;
    ptr = strchr(ptr, ISO_cr);
    if(ptr != NULL) {
      *ptr = 0;
    }
  }
}

static void
parse(char *msg, struct parse_result *dummy)
{
  r.msg = msg;
  if(*r.msg == ISO_cr || *r.msg == ISO_nl) {
    return;
  }
  if(*r.msg == ISO_colon) {
    ++r.msg;
    parse_prefix();
  }
  
  parse_command();
  parse_params();

}

/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_input(struct ircc_state *s))
{
  int i;
  char *ptr;
  /*  struct parse_result r;*/
  
  SOCKET_BEGIN(&s->s);
  
  SOCKET_READTO(&s->s, ISO_nl);
  
  if(SOCKET_DATALEN(&s->s) > 0) {
    
    s->inputbuf[SOCKET_DATALEN(&s->s)] = 0;

    if(strncmp(s->inputbuf, ircc_ping, 5) == 0) {
      strncpy(s->outputbuf, s->inputbuf, sizeof(s->outputbuf));
      
      /* Turn "PING" into "PONG" */
      s->outputbuf[1] = ISO_O;
      SEND_STRING(&s->s, s->outputbuf);
    } else {

      memset(&r, 0, sizeof(r));

      parse(s->inputbuf, &r);

      if(r.name != NULL) {
	ptr = strchr(r.name, ISO_bang);
	if(ptr != NULL) {
	  *ptr = 0;
	}
      }
      
      if(r.command != NULL && strncmp(r.command, ircc_join_, 4) == 0) {
	  ircc_text_output(s, "Joined channel", r.name);
      } else if(r.command != NULL && strncmp(r.command, ircc_part_, 4) == 0) {
	ircc_text_output(s, "Left channel", r.name);
      } else {
	ircc_text_output(s, r.name, r.trailing);
      }
    }
  }

  SOCKET_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(data_or_command(struct ircc_state *s))
{
  SOCKET_BEGIN(&s->s);

  SOCKET_WAIT_UNTIL(&s->s, s->command != COMMAND_NONE);

  SOCKET_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_connection(struct ircc_state *s))
{
  PT_BEGIN(&s->pt);

  SOCKET_INIT(&s->s, s->inputbuf, sizeof(s->inputbuf) - 1);
  
  PT_WAIT_THREAD(&s->pt, setup_connection(s));

  while(1) {

    PT_WAIT_UNTIL(&s->pt, data_or_command(s));

    if(SOCKET_NEWDATA(&s->s)) {
      PT_WAIT_THREAD(&s->pt, handle_input(s));      
    } 
      
    switch(s->command) {
    case COMMAND_JOIN:
      s->command = COMMAND_NONE;
      PT_WAIT_THREAD(&s->pt, join_channel(s));
      break;
    case COMMAND_PART:
      s->command = COMMAND_NONE;
      PT_WAIT_THREAD(&s->pt, part_channel(s));
      break;
    case COMMAND_MSG:
      s->command = COMMAND_NONE;
      PT_WAIT_THREAD(&s->pt, send_message(s));
      break;
    case COMMAND_ACTIONMSG:
      s->command = COMMAND_NONE;
      PT_WAIT_THREAD(&s->pt, send_actionmessage(s));
      break;
    case COMMAND_LIST:
      s->command = COMMAND_NONE;
      PT_WAIT_THREAD(&s->pt, list_channel(s));
      break;
    case COMMAND_QUIT:
      s->command = COMMAND_NONE;
      tcp_markconn(uip_conn, NULL);
      SOCKET_CLOSE(&s->s);
      ek_post(EK_PROC_ID(EK_CURRENT()), EK_EVENT_REQUEST_EXIT, NULL);
      PT_EXIT(&s->pt);
      break;
    default:
      break;
    }
  }
  
  PT_END(&s->pt);
}
/*---------------------------------------------------------------------------*/
void
ircc_appcall(void *s)
{
  if(uip_closed() || uip_aborted() || uip_timedout()) {
    ircc_closed(s);
  } else if(uip_connected()) {
    ircc_connected(s);
    PT_INIT(&((struct ircc_state *)s)->pt);
    memset(((struct ircc_state *)s)->channel, 0,
	   sizeof(((struct ircc_state *)s)->channel));
    ((struct ircc_state *)s)->command = COMMAND_NONE;
    handle_connection(s);
  } else if(s != NULL) {
    handle_connection(s);
  }
}
/*---------------------------------------------------------------------------*/
struct ircc_state *
ircc_connect(struct ircc_state *s, char *servername, u16_t *ipaddr,
	     char *nick)
{
  s->conn = tcp_connect(ipaddr, HTONS(PORT), s);
  if(s->conn == NULL) {
    return NULL;
  }
  uip_set_mss(s->conn, 120);
  s->server = servername;  
  s->nick = nick;
  return s;
}
/*---------------------------------------------------------------------------*/
void
ircc_list(struct ircc_state *s)
{
  s->command = COMMAND_LIST;
}
/*---------------------------------------------------------------------------*/
void
ircc_join(struct ircc_state *s, char *channel)
{
  strncpy(s->channel, channel, sizeof(s->channel));
  s->command = COMMAND_JOIN;
}
/*---------------------------------------------------------------------------*/
void
ircc_part(struct ircc_state *s)
{
  s->command = COMMAND_PART;
}
/*---------------------------------------------------------------------------*/
void
ircc_quit(struct ircc_state *s)
{
  s->command = COMMAND_QUIT;
}
/*---------------------------------------------------------------------------*/
void
ircc_msg(struct ircc_state *s, char *msg)
{
  s->msg = msg;
  s->command = COMMAND_MSG;
}
/*---------------------------------------------------------------------------*/
void
ircc_actionmsg(struct ircc_state *s, char *msg)
{
  s->msg = msg;
  s->command = COMMAND_ACTIONMSG;
}
/*---------------------------------------------------------------------------*/
