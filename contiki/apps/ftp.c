/* Note to self: It would be nice to have a "View" option in the download dialog. */

#include "ftpc.h"
#include "contiki.h"
#include <string.h>

#define MAX_USERNAMELEN 16
#define MAX_PASSWORDLEN 16
#define MAX_HOSTNAMELEN 32
#define MAX_FILENAMELEN 16
#define FILES_WIDTH 17
#define FILES_HEIGHT 18

EK_EVENTHANDLER(eventhandler, ev, data);
EK_PROCESS(p, "FTP client", EK_PRIO_NORMAL,
	   eventhandler, NULL, NULL);
static ek_id_t id = EK_ID_NONE;

static void *connection;

/*  ---  The main window --- */

static struct ctk_window window;

static struct ctk_label localtextlabel =
  {CTK_LABEL(1, 0, FILES_WIDTH, 1, "Local files")};
static struct ctk_label remotetextlabel =
  {CTK_LABEL(1 + FILES_WIDTH + 1, 0,
	     FILES_WIDTH, 1, "Remote files")};

static char leftptr[FILES_HEIGHT];
static struct ctk_label leftptrlabel =
  {CTK_LABEL(0, 1, 1, FILES_HEIGHT, leftptr)};
static char midptr[FILES_HEIGHT];
static struct ctk_label midptrlabel =
  {CTK_LABEL(1 + FILES_WIDTH, 1, 1, FILES_HEIGHT, midptr)};
static char rightptr[FILES_HEIGHT];
static struct ctk_label rightptrlabel =
   {CTK_LABEL(1 + FILES_WIDTH + 1 + FILES_WIDTH, 1,
	      1, FILES_HEIGHT, rightptr)};

static char localfiles[FILES_WIDTH * FILES_HEIGHT];
static struct ctk_label localfileslabel =
  {CTK_LABEL(1, 1,
	     FILES_WIDTH, FILES_HEIGHT, localfiles)};
static char remotefiles[FILES_WIDTH * FILES_HEIGHT];
static struct ctk_label remotefileslabel =
  {CTK_LABEL(FILES_WIDTH + 1 + 1, 1,
	     FILES_WIDTH, FILES_HEIGHT, remotefiles)};

static struct ctk_button reloadbutton =
  {CTK_BUTTON(0, 1 + FILES_HEIGHT, 6, "Reload")};

static struct ctk_button connectionbutton =
  {CTK_BUTTON(8, 1 + FILES_HEIGHT, 13, "Connection...")};
static struct ctk_button quitbutton =
  {CTK_BUTTON(1 + FILES_WIDTH + 1 + FILES_WIDTH - 5,
	      1 + FILES_HEIGHT, 4, "Quit")};

static char statustext[3 + FILES_WIDTH * 2 + 1];
static struct ctk_label statuslabel =
  {CTK_LABEL(0, FILES_HEIGHT + 2, 3 + FILES_WIDTH * 2, 1, statustext)};



/*  ---  The download/upload dialogs --- */
static char remotefilename[MAX_FILENAMELEN + 1];
static char localfilename[MAX_FILENAMELEN + 1];

static struct ctk_window dialog;
static struct ctk_label downloadlabel =
  {CTK_LABEL(6, 1, 13, 1, "Download file")};
static struct ctk_label uploadlabel =
  {CTK_LABEL(7, 1, 11, 1, "Upload file")};
static struct ctk_label localfilenametextlabel =
  {CTK_LABEL(2, 3, 15, 1, "Local filename")};
static struct ctk_label localfilenamelabel =
  {CTK_LABEL(3, 5, 16, 1, localfilename)};
static struct ctk_textentry localfilenameentry =
  {CTK_TEXTENTRY(2, 5, 16, 1, localfilename, sizeof(localfilename) - 1)};
static struct ctk_label remotefilenametextlabel =
  {CTK_LABEL(2, 7, 15, 1, "Remote filename")};
static struct ctk_label remotefilenamelabel =
  {CTK_LABEL(3, 9, 16, 1, remotefilename)};
static struct ctk_textentry remotefilenameentry =
  {CTK_TEXTENTRY(2, 9, 16, 1, remotefilename, sizeof(remotefilename) - 1)};
static struct ctk_button downloadbutton =
  {CTK_BUTTON(0, 11, 13, "Download file")};
static struct ctk_button uploadbutton =
  {CTK_BUTTON(0, 11, 11, "Upload file")};
static struct ctk_button cancelbutton =
  {CTK_BUTTON(16, 11, 6, "Cancel")};
  
/*  ---  The connection window --- */
static char hostname[MAX_HOSTNAMELEN + 1];
static char username[MAX_USERNAMELEN + 1];
static char password[MAX_PASSWORDLEN + 1];

static struct ctk_window connectionwindow;
static struct ctk_label serverlabel =
  {CTK_LABEL(0, 1, 10, 1, "FTP server")};
static struct ctk_textentry serverentry =
  {CTK_TEXTENTRY(0, 2, 16, 1, hostname, MAX_HOSTNAMELEN)};

static struct ctk_button anonymousbutton =
  {CTK_BUTTON(10, 4, 9, "Anonymous")};
static struct ctk_label userlabel =
  {CTK_LABEL(0, 4, 8, 1, "Username")};
static struct ctk_textentry userentry =
  {CTK_TEXTENTRY(0, 5, 16, 1, username, sizeof(username) - 1)};
static struct ctk_label passwordlabel =
  {CTK_LABEL(0, 7, 8, 1, "Password")};
static struct ctk_textentry passwordentry =
  {CTK_TEXTENTRY(0, 8, 16, 1, password, sizeof(password) - 1)};

static struct ctk_button connectbutton =
  {CTK_BUTTON(0, 10, 7, "Connect")};
static struct ctk_button closeconnectionbutton =
  {CTK_BUTTON(0, 10, 16, "Close connection")};

static struct ctk_button closebutton =
  {CTK_BUTTON(18, 10, 5, "Close")};

static struct cfs_dir dir;
static struct cfs_dirent dirent;
static unsigned char localfileptr = 0;

static unsigned char remotefileptr = 0;

static unsigned char ptrstate;
#define PTRSTATE_LOCALFILES 0
#define PTRSTATE_REMOTEFILES 1
static unsigned char localptr, remoteptr;

static int fd;

/*---------------------------------------------------------------------------*/
static void
make_uploaddialog(void)
{
  ctk_dialog_new(&dialog, 24, 13);
  CTK_WIDGET_ADD(&dialog, &uploadlabel);
  CTK_WIDGET_ADD(&dialog, &localfilenametextlabel);
  CTK_WIDGET_ADD(&dialog, &localfilenamelabel);
  CTK_WIDGET_ADD(&dialog, &remotefilenametextlabel);
  CTK_WIDGET_ADD(&dialog, &remotefilenameentry);
  CTK_WIDGET_ADD(&dialog, &uploadbutton);
  CTK_WIDGET_ADD(&dialog, &cancelbutton);  
}
/*---------------------------------------------------------------------------*/
static void
make_downloaddialog(void)
{
  ctk_dialog_new(&dialog, 24, 13);
  CTK_WIDGET_ADD(&dialog, &downloadlabel);
  CTK_WIDGET_ADD(&dialog, &localfilenametextlabel);
  CTK_WIDGET_ADD(&dialog, &localfilenameentry);
  CTK_WIDGET_ADD(&dialog, &remotefilenametextlabel);
  CTK_WIDGET_ADD(&dialog, &remotefilenamelabel);
  CTK_WIDGET_ADD(&dialog, &downloadbutton);
  CTK_WIDGET_ADD(&dialog, &cancelbutton);
}
/*---------------------------------------------------------------------------*/
static void
show_statustext(char *text1, char *text2)
{
  int len;

  len = strlen(text1);
  if(len < sizeof(statustext)) {
    strncpy(statustext, text1, sizeof(statustext));
    strncpy(statustext + len, text2, sizeof(statustext) - len);
    CTK_WIDGET_REDRAW(&statuslabel);
  }  
  
}
/*---------------------------------------------------------------------------*/
static void
quit(void)
{
  ctk_window_close(&window);
  ek_exit();
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*---------------------------------------------------------------------------*/
static void
clearptr(void)
{
  rightptr[remoteptr] = ' ';
  midptr[remoteptr] = ' ';
  leftptr[localptr] = ' ';
  midptr[localptr] = ' ';    
}
/*---------------------------------------------------------------------------*/
static void
showptr(void)
{
  if(ptrstate == PTRSTATE_LOCALFILES) {
    rightptr[remoteptr] = ' ';
    midptr[remoteptr] = ' ';
    leftptr[localptr] = '>';
    midptr[localptr] = '<';
  } else {
    leftptr[localptr] = ' ';
    midptr[localptr] = ' ';    
    rightptr[remoteptr] = '<';
    midptr[remoteptr] = '>';
  }

  CTK_WIDGET_REDRAW(&leftptrlabel);
  CTK_WIDGET_REDRAW(&midptrlabel);
  CTK_WIDGET_REDRAW(&rightptrlabel);
}
/*---------------------------------------------------------------------------*/
static void
start_loaddir(void)
{
  memset(localfiles, 0, sizeof(localfiles));
  localfileptr = 0;
  cfs_opendir(&dir, "/");
  ek_post(EK_PROC_ID(EK_CURRENT()), EK_EVENT_CONTINUE, NULL);
}
/*---------------------------------------------------------------------------*/
static void
start_loadremote(void)
{
  memset(remotefiles, 0, sizeof(remotefiles));
  remotefileptr = 0;
  clearptr();
  remoteptr = 0;
  showptr();
  ftpc_list(connection);
}
/*---------------------------------------------------------------------------*/
static void
make_connectionwindow(void)
{
  ctk_dialog_new(&connectionwindow, 24, 11);
  
  CTK_WIDGET_ADD(&connectionwindow, &serverlabel);
  CTK_WIDGET_ADD(&connectionwindow, &serverentry);
  
  CTK_WIDGET_ADD(&connectionwindow, &userlabel);
  CTK_WIDGET_ADD(&connectionwindow, &anonymousbutton);
  CTK_WIDGET_ADD(&connectionwindow, &userentry);
  CTK_WIDGET_ADD(&connectionwindow, &passwordlabel);
  CTK_WIDGET_ADD(&connectionwindow, &passwordentry);
  
  if(connection == NULL) {
    CTK_WIDGET_ADD(&connectionwindow, &connectbutton);
  } else {
    CTK_WIDGET_ADD(&connectionwindow, &closeconnectionbutton);
  }
  CTK_WIDGET_ADD(&connectionwindow, &closebutton);
}
/*---------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{
  u16_t ipaddr[2], *ipaddrptr;
  
  switch(ev) {
    
  case EK_EVENT_INIT:
    ftpc_init();

    memset(statustext, 0, sizeof(statustext));
    memset(remotefiles, 0, sizeof(remotefiles));
    memset(localfiles, 0, sizeof(localfiles));
    memset(leftptr, 0, sizeof(leftptr));
    memset(midptr, 0, sizeof(midptr));	    
    memset(rightptr, 0, sizeof(rightptr));

    ptrstate = PTRSTATE_LOCALFILES;
    localptr = remoteptr = 0;

    connection = NULL;
    
    ctk_window_new(&window,
		   3 + FILES_WIDTH * 2, 3 + FILES_HEIGHT,
		   "FTP Client");

    CTK_WIDGET_ADD(&window, &localtextlabel);
    CTK_WIDGET_ADD(&window, &remotetextlabel);
    
    CTK_WIDGET_ADD(&window, &leftptrlabel);
    CTK_WIDGET_ADD(&window, &localfileslabel);
    CTK_WIDGET_ADD(&window, &midptrlabel);
    CTK_WIDGET_ADD(&window, &remotefileslabel);
    CTK_WIDGET_ADD(&window, &rightptrlabel);

    CTK_WIDGET_ADD(&window, &reloadbutton);
    CTK_WIDGET_ADD(&window, &connectionbutton);
    CTK_WIDGET_ADD(&window, &quitbutton);

    
    CTK_WIDGET_ADD(&window, &statuslabel);
    ctk_window_open(&window);

    showptr();

    start_loaddir();
    
    break;
  case EK_EVENT_CONTINUE:
    if(cfs_readdir(&dir, &dirent) == 0 &&
       localfileptr < FILES_HEIGHT) {
      strncpy(&localfiles[localfileptr * FILES_WIDTH],
	      dirent.name, FILES_WIDTH);
      CTK_WIDGET_REDRAW(&localfileslabel);
      ++localfileptr;
      ek_post(EK_PROC_ID(EK_CURRENT()), EK_EVENT_CONTINUE, NULL);
    } else{
      cfs_closedir(&dir);
    }   
    break;
  case EK_EVENT_REQUEST_EXIT:
    quit();
    break;
  default:
    if(ev == tcpip_event) {
      ftpc_appcall(data);
    } else if(ev == resolv_event_found) {
      /* Either found a hostname, or not. */
      if((char *)data != NULL &&
	 (ipaddrptr = resolv_lookup((char *)data)) != NULL) {
	connection = ftpc_connect(ipaddrptr, HTONS(21));
	show_statustext("Connecting to ", hostname);
      } else {
	show_statustext("Host not found: ", hostname);
      }
      
    } else if(ev == ctk_signal_window_close &&
	      data == (ek_data_t)&window) {
      quit();
    } else if(ev == ctk_signal_widget_activate) {
      if((struct ctk_button *)data == &cancelbutton) {
	ctk_dialog_close();
      } else if((struct ctk_button *)data == &downloadbutton) {
	ctk_dialog_close();
	fd = cfs_open(localfilename, 0);
	if(fd > 0) {
	  show_statustext("Downloading ", remotefilename);
	  ftpc_get(connection, remotefilename);
	} else {
	  show_statustext("Could not create ", localfilename);
	}
      } else if((struct ctk_button *)data == &reloadbutton) {	
	start_loaddir();
      } else if((struct ctk_button *)data == &connectionbutton) {	
	make_connectionwindow();
	ctk_dialog_open(&connectionwindow);
      } else if((struct ctk_button *)data == &closebutton) {
	ctk_dialog_close();
      } else if((struct ctk_button *)data == &anonymousbutton) {
	strcpy(username, "ftp");
	strcpy(password, "contiki@ftp");
	CTK_WIDGET_REDRAW(&userentry);
	CTK_WIDGET_REDRAW(&passwordentry);
      } else if((struct ctk_button *)data == &closeconnectionbutton) {
	ctk_dialog_close();
	ftpc_close(connection);
      } else if((struct ctk_button *)data == &connectbutton) {
	ctk_dialog_close();
	if(uiplib_ipaddrconv(hostname, (unsigned char *)ipaddr) == 0) {
	  ipaddrptr = resolv_lookup(hostname);
	  if(ipaddrptr == NULL) {
	    resolv_query(hostname);
	    show_statustext("Resolving host ", hostname);
	    break;
	  }
	  connection = ftpc_connect(ipaddrptr, HTONS(21));
	  show_statustext("Connecting to ", hostname);
	} else {
	  connection = ftpc_connect(ipaddr, HTONS(21));
	  show_statustext("Connecting to ", hostname);
	}       	
      } 
      /*      if((struct ctk_button *)data == &closebutton) {
	ftpc_close(connection);
	}*/
    } else if(ev == ctk_signal_keypress) {
      if((ctk_arch_key_t)data == ' ') {
	if(ptrstate == PTRSTATE_LOCALFILES) {
	  ptrstate = PTRSTATE_REMOTEFILES;
	} else {
	  ptrstate = PTRSTATE_LOCALFILES;
	}
      } else if((ctk_arch_key_t)data == CH_CURS_UP) {
	clearptr();
	if(ptrstate == PTRSTATE_LOCALFILES) {
	  if(localptr > 0) {
	    --localptr;
	  }
	} else {
	  if(remoteptr > 0) {
	    --remoteptr;
	  }
	}
      } else if((ctk_arch_key_t)data == CH_CURS_DOWN) {
	clearptr();
	if(ptrstate == PTRSTATE_LOCALFILES) {
	  if(localptr < FILES_HEIGHT - 1) {
	    ++localptr;
	  }
	} else {
	  if(remoteptr < FILES_HEIGHT - 1) {
	    ++remoteptr;
	  }
	}
      } else if((ctk_arch_key_t)data == CH_ENTER) {	
	if(ptrstate == PTRSTATE_LOCALFILES) {
	  strncpy(localfilename,
		  &localfiles[localptr * FILES_WIDTH], FILES_WIDTH);
	  strncpy(remotefilename,
		  &localfiles[localptr * FILES_WIDTH], FILES_WIDTH);
	  make_uploaddialog();
	  ctk_dialog_open(&dialog);
	} else {
	  strncpy(localfilename,
		  &remotefiles[remoteptr * FILES_WIDTH], FILES_WIDTH);
	  strncpy(remotefilename,
		  &remotefiles[remoteptr * FILES_WIDTH], FILES_WIDTH);
	  ftpc_cwd(connection, remotefilename);
	  /*	  make_downloaddialog();
		  ctk_dialog_open(&dialog);*/
	}
      } else if((ctk_arch_key_t)data == 'u') {
	ftpc_cdup(connection);
      }

      showptr();
    }
    break;
  }
}
/*---------------------------------------------------------------------------*/
LOADER_INIT_FUNC(ftp_init, arg)
{
  arg_free(arg);
  if(id == EK_ID_NONE) {
    id = ek_start(&p);
  }
}
/*---------------------------------------------------------------------------*/
void
ftpc_closed(void)
{
  strcpy(statustext, "Connection closed");
  CTK_WIDGET_REDRAW(&statuslabel);
  connection = NULL;
}
/*---------------------------------------------------------------------------*/
void
ftpc_aborted(void)
{
  strcpy(statustext, "Connection reset");
  CTK_WIDGET_REDRAW(&statuslabel);
  connection = NULL;
}
/*---------------------------------------------------------------------------*/
void
ftpc_timedout(void)
{
  strcpy(statustext, "Connection timed out");
  CTK_WIDGET_REDRAW(&statuslabel);
  connection = NULL;
}
/*---------------------------------------------------------------------------*/
char *
ftpc_username(void)
{
  return username;
}
/*---------------------------------------------------------------------------*/
char *
ftpc_password(void)
{
  return password;
}
/*---------------------------------------------------------------------------*/
void
ftpc_list_file(char *filename)
{
  if(remotefileptr < FILES_HEIGHT && filename != NULL) {
    strncpy(&remotefiles[remotefileptr * FILES_WIDTH], filename, FILES_WIDTH);
    CTK_WIDGET_REDRAW(&remotefileslabel);
    ++remotefileptr;
  }

  if(filename == NULL) {
    strcpy(statustext, "Connected");
    CTK_WIDGET_REDRAW(&statuslabel);  
  }
}
/*---------------------------------------------------------------------------*/
void
ftpc_cwd_done(unsigned short status)
{
  if(status == FTPC_COMPLETED ||
     status == FTPC_OK) {
    start_loadremote();
  } else {
    make_downloaddialog();
    ctk_dialog_open(&dialog);
  }
}
/*---------------------------------------------------------------------------*/
void
ftpc_connected(void *connection)
{
  strcpy(statustext, "Loading remote directory");
  CTK_WIDGET_REDRAW(&statuslabel);

  start_loadremote();
}
/*---------------------------------------------------------------------------*/
void
ftpc_data(u8_t *data, u16_t len)
{
  if(data == NULL) {
    show_statustext("Download complete", "");
    cfs_close(fd);
    start_loaddir();
  } else {
    cfs_write(fd, data, len);
  }
}
/*---------------------------------------------------------------------------*/
