#ifndef __CTK_TERM_H__
#define __CTK_TERM_H__

struct ctk_term_state* ctk_term_alloc_state(void);
void ctk_term_dealloc_state(struct ctk_term_state *s);
void ctk_term_redraw(struct ctk_term_state *s);
void ctk_term_sent(struct ctk_term_state* ts);
unsigned short ctk_term_send(struct ctk_term_state* ts, 
				 unsigned char* buf, 
				 unsigned short maxlen);

void ctk_term_input(struct ctk_term_state* ts, unsigned char b);

#endif /* __CTK_TERM_H__ */
