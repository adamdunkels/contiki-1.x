#ifndef __CTK_FILEDIALOG_H__
#define __CTK_FILEDIALOG_H__

#include "contiki.h"

struct ctk_filedialog_state {
  unsigned char fileptr;
  ek_event_t ev;
};

void ctk_filedialog_init(register struct ctk_filedialog_state *s);
void ctk_filedialog_open(register struct ctk_filedialog_state *s,
			 const char *buttontext, ek_event_t event);
char ctk_filedialog_eventhandler(struct ctk_filedialog_state *s,
				 ek_event_t ev, ek_data_t data);

#endif /* __CTK_FILEDIALOG_H__ */
