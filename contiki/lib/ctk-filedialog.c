
#include "contiki.h"
#include "ctk-filedialog.h"

#include <string.h>

#define MAX_NUMFILES 40
#define FILES_WIDTH 17
#define FILES_HEIGHT 14

static struct ctk_window dialog;
static char leftptr[FILES_HEIGHT];
static struct ctk_label leftptrlabel =
  {CTK_LABEL(0, 1, 1, FILES_HEIGHT, leftptr)};

static char files[FILES_WIDTH * MAX_NUMFILES];
static struct ctk_label fileslabel =
  {CTK_LABEL(1, 1,
	     FILES_WIDTH, FILES_HEIGHT, files)};

static char rightptr[FILES_HEIGHT];
static struct ctk_label rightptrlabel =
  {CTK_LABEL(1 + FILES_WIDTH, 1, 1, FILES_HEIGHT, rightptr)};

static char filename[FILES_WIDTH + 1];
static struct ctk_textentry filenameentry =
  {CTK_TEXTENTRY(1, 2 + FILES_HEIGHT, FILES_WIDTH, 1, filename,
		 FILES_WIDTH)};

static struct ctk_button button;

#define STATE_CLOSED 0
#define STATE_OPEN 1
static char state = STATE_CLOSED;
static unsigned char fileptr, dirfileptr;
static struct cfs_dir dir;
/*---------------------------------------------------------------------------*/
static void
clearptr(void)
{
  leftptr[fileptr] = ' ';
  rightptr[fileptr] = ' '; 
}
/*---------------------------------------------------------------------------*/
static void
showptr(void)
{
  leftptr[fileptr] = '>';
  rightptr[fileptr] = '<';

  strncpy(filename,
	  &files[fileptr * FILES_WIDTH],
	  FILES_WIDTH);
  
  CTK_WIDGET_REDRAW(&filenameentry);
  CTK_WIDGET_REDRAW(&leftptrlabel);
  CTK_WIDGET_REDRAW(&rightptrlabel);
}
/*---------------------------------------------------------------------------*/
void
ctk_filedialog_init(register struct ctk_filedialog_state *s)
{
  state = STATE_CLOSED;
}
/*---------------------------------------------------------------------------*/
void
ctk_filedialog_open(register struct ctk_filedialog_state *s,
		    const char *buttontext, ek_event_t event)
{
  ctk_dialog_new(&dialog, 20, 5 + FILES_HEIGHT);
  CTK_WIDGET_ADD(&dialog, &leftptrlabel);
  CTK_WIDGET_ADD(&dialog, &fileslabel);
  CTK_WIDGET_ADD(&dialog, &rightptrlabel);
  CTK_WIDGET_ADD(&dialog, &filenameentry);
  CTK_BUTTON_NEW(&button, 1, 4 + FILES_HEIGHT, strlen(buttontext), (char *)buttontext);
  CTK_WIDGET_ADD(&dialog, &button);
  ctk_dialog_open(&dialog);
  state = STATE_OPEN;
  memset(filename, 0, sizeof(filename));
  memset(leftptr, ' ', sizeof(leftptr));
  memset(rightptr, ' ', sizeof(rightptr));
  memset(files, ' ', sizeof(files));
  
  fileptr = 0;
  dirfileptr = 0;
  showptr();
  cfs_opendir(&dir, "/");
  ek_post(EK_PROC_ID(EK_CURRENT()), EK_EVENT_CONTINUE, s);
}
/*---------------------------------------------------------------------------*/
char
ctk_filedialog_eventhandler(struct ctk_filedialog_state *s,
			    ek_event_t ev, ek_data_t data)
{
  static struct cfs_dirent dirent;
  
  if(state == STATE_OPEN) {
    if(ev == ctk_signal_widget_activate &&
       data == (ek_data_t)&button) {
      ctk_dialog_close();
      state = STATE_CLOSED;
      ek_post(EK_PROC_ID(EK_CURRENT()), s->ev, &filename);
      return 1;
    } else if(ev == EK_EVENT_CONTINUE &&
	      (ek_data_t)s == data) {
      if(cfs_readdir(&dir, &dirent) == 0 &&
	 dirfileptr < MAX_NUMFILES) {
	strncpy(&files[dirfileptr * FILES_WIDTH],
		dirent.name, FILES_WIDTH);
	CTK_WIDGET_REDRAW(&fileslabel);
	++dirfileptr;
	ek_post(EK_PROC_ID(EK_CURRENT()), EK_EVENT_CONTINUE, s);
      } else {
	fileptr = 0;	
	cfs_closedir(&dir);
      }
      return 1; 
    } else if(ev == ctk_signal_keypress) {
      if((ctk_arch_key_t)data == CH_CURS_UP) {
	clearptr();
	if(fileptr > 0) {
	  --fileptr;
	}
	showptr();
	return 1;
      } else if((ctk_arch_key_t)data == CH_CURS_DOWN) {
	clearptr();
	if(fileptr < FILES_HEIGHT - 1) {
	  ++fileptr;
	}
	showptr();
	return 1;
      }
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
