#include "ircc.h"

#include <string.h>

#define PORT 6667

#define SEND_STRING(s, str) SOCKET_SEND(s, str, strlen(str))

enum {
  COMMAND_NONE,
  COMMAND_JOIN,
  COMMAND_PART,  
  COMMAND_MSG,
  COMMAND_LIST,
  COMMAND_CLOSE
};

/*---------------------------------------------------------------------------*/
void
ircc_init(void)
{

}
/*---------------------------------------------------------------------------*/
static char *
copystr(char *dest, char *src, int n)
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
  ptr = copystr(ptr, "NICK ", sizeof(s->outputbuf));
  ptr = copystr(ptr, s->nick, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, "\r\nUSER ", sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, s->nick, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, " contiki ", sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, s->server, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, " :Contiki\r\n", sizeof(s->outputbuf) - (ptr - s->outputbuf));

  SEND_STRING(&s->s, s->outputbuf);

  SOCKET_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(join_channel(struct ircc_state *s))
{
  SOCKET_BEGIN(&s->s);
  
  SEND_STRING(&s->s, "JOIN ");
  SEND_STRING(&s->s, s->channel);
  SEND_STRING(&s->s, "\r\n");

  ircc_sent(s);
  
  SOCKET_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(part_channel(struct ircc_state *s))
{
  SOCKET_BEGIN(&s->s);

  SEND_STRING(&s->s, "PART ");
  SEND_STRING(&s->s, s->channel);
  SEND_STRING(&s->s, "\r\n");

  ircc_sent(s);
  
  SOCKET_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(list_channel(struct ircc_state *s))
{
  SOCKET_BEGIN(&s->s);

  SEND_STRING(&s->s, "LIST ");
  SEND_STRING(&s->s, s->channel);
  SEND_STRING(&s->s, "\r\n");

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
  ptr = copystr(ptr, "PRIVMSG ", sizeof(s->outputbuf));
  ptr = copystr(ptr, s->channel, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, " :", sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, s->msg, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, "\r\n", sizeof(s->outputbuf) - (ptr - s->outputbuf));

  SEND_STRING(&s->s, s->outputbuf);

  ircc_sent(s);
  
  SOCKET_END(&s->s);
}
/*---------------------------------------------------------------------------*/
struct parse_result {
  char *user;
  char *host;
  char *name;
  char *command;
  char *middle;
  char *trailing;
};

static char *
parse_whitespace(char *msg)
{
  while(*msg == ' ') ++msg;
  return msg;
}
static char *
parse_word(char *msg)
{
  while(*msg != ' ') ++msg;
  return msg;  
}
static char *
parse_user(char *msg, struct parse_result *r)
{
  msg = parse_whitespace(msg);
  r->user = msg;
  msg = parse_word(msg);
  *msg = 0;
  return msg + 1;
}
static char *
parse_host(char *msg, struct parse_result *r)
{
  msg = parse_whitespace(msg);
  r->host = msg;
  msg = parse_word(msg);
  *msg = 0;
  return msg + 1;
}

static char *
parse_name(char *msg, struct parse_result *r)
{
  msg = parse_whitespace(msg);
  r->name = msg;
  msg = parse_word(msg);
  *msg = 0;
  return msg + 1;
}

static char *
parse_prefix(char *msg, struct parse_result *r)
{
  msg = parse_name(msg, r);
  if(*msg == '!') {
    msg = parse_user(msg + 1, r);
  }
  if(*msg == '@') {
    msg = parse_host(msg + 1, r);
  }
  return msg;
}

static char *
parse_command(char *msg, struct parse_result *r)
{
  msg = parse_whitespace(msg);
  r->command = msg;
  msg = parse_word(msg);
  *msg = 0;
  return msg + 1;
}

static char *
parse_trailing(char *msg, struct parse_result *r)
{
  r->trailing = msg;
  if(strncmp(msg + 1, "gtk", 3) == 0)  abort();
  while(*msg != 0 && *msg != '\r' && *msg != '\n') ++msg;
  *msg = 0;
  return msg + 1;
}

static char *
parse_params(char *msg, struct parse_result *r)
{
  msg = parse_whitespace(msg);
  if(*msg == ':') {
    msg = parse_trailing(msg + 1, r);
    return msg;
  } else if(*msg == 0 || *msg == '\r' || *msg == '\n') {
    return msg;
  } else {
    r->middle = msg;
    msg = parse_word(msg);
    *msg = 0;
    return parse_params(msg + 1, r);
  }
}

static void
parse(char *msg, struct parse_result *r)
{
  if(*msg == '\r' ||
     *msg == '\n') {
    return;
  }
  if(*msg == ':') {
    msg = parse_prefix(msg + 1, r);
  }
  
  msg = parse_command(msg, r);
  msg = parse_params(msg, r);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_input(struct ircc_state *s))
{
  int i;
  char *ptr;
  struct parse_result r;
  
  SOCKET_BEGIN(&s->s);
  
  SOCKET_READTO(&s->s, '\n');
  
  if(SOCKET_DATALEN(&s->s) > 0) {
    
    s->inputbuf[SOCKET_DATALEN(&s->s)] = 0;

    if(strncmp(s->inputbuf, "PING ", 5) == 0) {
      strncpy(s->outputbuf, s->inputbuf, sizeof(s->outputbuf));
      
      /* Turn "PING" into "PONG" */
      s->outputbuf[1] = 'O';
      SEND_STRING(&s->s, s->outputbuf);
    } else {

      memset(&r, 0, sizeof(r));

      parse(s->inputbuf, &r);

      if(r.name != NULL) {
	for(i = 0; i < strlen(r.name); ++i) {
	  if(r.name[i] == '!') {
	    r.name[i] = 0;
	    break;
	  }
	}
      }
      
      if(r.command != NULL && strncmp(r.command, "JOIN", 4) == 0) {
	  ircc_text_output(s, "Joined channel", r.name);
      } else if(r.command != NULL && strncmp(r.command, "PART", 4) == 0) {
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

  SOCKET_INIT(&s->s, s->inputbuf, sizeof(s->inputbuf));
  
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
    case COMMAND_LIST:
      s->command = COMMAND_NONE;
      PT_WAIT_THREAD(&s->pt, list_channel(s));
      break;
    case COMMAND_CLOSE:
      s->command = COMMAND_NONE;
      SOCKET_CLOSE(&s->s);
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
  } else {
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
ircc_msg(struct ircc_state *s, char *msg)
{
  s->msg = msg;
  s->command = COMMAND_MSG;
}
/*---------------------------------------------------------------------------*/
