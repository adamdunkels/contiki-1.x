
#include "ek-service.h"

#include "ctk-draw.h"
#include "ctk.h"

#include "ctk-draw-service.h"


unsigned char ctk_draw_windowborder_width,
  ctk_draw_windowborder_height,
  ctk_draw_windowtitle_height;

EK_SERVICE(service, CTK_DRAW_SERVICE_NAME);

/*---------------------------------------------------------------------------*/
static struct ctk_draw_service_interface *
find_interface(void)
{
  struct ctk_draw_service_interface *interface;
  interface = (struct ctk_draw_service_interface *)ek_service_state(&service);
  if(interface != NULL &&
     interface->version == CTK_DRAW_SERVICE_VERSION) {
    return interface;
  } else {
    return NULL;
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_init(void)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    interface->draw_init();
    ctk_draw_windowborder_width = interface->windowborder_width;
    ctk_draw_windowborder_height = interface->windowborder_height;
    ctk_draw_windowtitle_height = interface->windowtitle_height;
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_clear(unsigned char clipy1, unsigned char clipy2)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    interface->draw_clear(clipy1, clipy2);
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_clear_window(struct ctk_window *window,
		      unsigned char focus,
		      unsigned char clipy1,
		      unsigned char clipy2)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    interface->draw_clear_window(window, focus, clipy1, clipy2);
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_window(struct ctk_window *window,
		unsigned char focus,
		unsigned char clipy1,
		unsigned char clipy2)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    interface->draw_window(window, focus, clipy1, clipy2);
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_dialog(struct ctk_window *dialog)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    interface->draw_dialog(dialog);
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_widget(struct ctk_widget *widget,
		unsigned char focus,
		unsigned char clipy1,
		unsigned char clipy2)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    interface->draw_widget(widget, focus, clipy1, clipy2);
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_menus(struct ctk_menus *menus)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    interface->draw_menus(menus);
  }
}
/*---------------------------------------------------------------------------*/
unsigned char
ctk_draw_width(void)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    return interface->width();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
unsigned char
ctk_draw_height(void)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    return interface->height();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
unsigned short
ctk_mouse_xtoc(unsigned short x)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    return interface->mouse_xtoc(x);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
unsigned short
ctk_mouse_ytoc(unsigned short y)
{
  struct ctk_draw_service_interface *interface;
  
  if((interface = find_interface()) != NULL) {   
    return interface->mouse_ytoc(y);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
