#ifndef __CTK_DRAW_SERVICE_H__
#define __CTK_DRAW_SERVICE_H__

#include "ek-service.h"

#include "ctk.h"

#define CTK_DRAW_SERVICE_NAME "CTK driver"
#define CTK_DRAW_SERVICE_VERSION 0x0001
struct ctk_draw_service_interface {
  unsigned char version;
  unsigned char windowborder_width,
    windowborder_height,
    windowtitle_height;
    
  void (* draw_init)(void);
  void (* draw_clear)(unsigned char clipy1, unsigned char clipy2);
  void (* draw_clear_window)(struct ctk_window *window,
			     unsigned char focus,
			     unsigned char clipy1,
			     unsigned char clipy2);
  void (* draw_window)(struct ctk_window *window,
		       unsigned char focus,
		       unsigned char clipy1,
		       unsigned char clipy2);
  void (* draw_dialog)(struct ctk_window *dialog);
  void (* draw_widget)(struct ctk_widget *widget,
		       unsigned char focus,
		       unsigned char clipy1,
		       unsigned char clipy2);
  void (* draw_menus)(struct ctk_menus *menus);

  unsigned char (* width)(void);
  unsigned char (* height)(void);

  unsigned short (* mouse_xtoc)(unsigned short);
  unsigned short (* mouse_ytoc)(unsigned short);
};

#endif /* __CTK_DRAW_SERVICE_H__ */
