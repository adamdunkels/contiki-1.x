/*
 * Copyright (c) 2002, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution. 
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *        This product includes software developed by Adam Dunkels. 
 * 4. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the Contiki desktop environment for the C64.
 *
 * $Id: email.c,v 1.6 2003/04/11 20:13:33 adamdunkels Exp $
 *
 */


#include "ctk.h"
#include "dispatcher.h"
#include "smtp.h"
#include "uip_main.h"
#include "petsciiconv.h"
#include "loader.h"


#define MAXNUMMSGS 6

static struct ctk_menu menu;
unsigned char menuitem_open, menuitem_setup, menuitem_quit;

/* The main window. */
static struct ctk_window mainwindow;

static struct ctk_button newmailbutton =
 {CTK_BUTTON(26, 0, 8, "New mail")};
static struct ctk_button checkbutton =
 {CTK_BUTTON(0, 0, 10, "Check mail")};

static struct ctk_button msgbuttons[MAXNUMMSGS];
static struct ctk_label msglabels[MAXNUMMSGS];
static char msgtitles[20][MAXNUMMSGS];
static struct ctk_separator sep1 =
 {CTK_SEPARATOR(0, 7, 36)};
static struct ctk_separator sep2 =
 {CTK_SEPARATOR(0, 20, 36)};
static struct ctk_label statuslabel =
 {CTK_LABEL(6, 21, 23, 1, "")};


static struct ctk_label tolabel =
  {CTK_LABEL(0, 8, 3, 1, "To:")};
static char to[40];
static struct ctk_textentry totextentry =
  {CTK_TEXTENTRY(8, 8, 26, 1, to, 38)};

static struct ctk_label cclabel =
  {CTK_LABEL(0, 9, 3, 1, "Cc:")};
static char cc[40];
static struct ctk_textentry cctextentry =
  {CTK_TEXTENTRY(8, 9, 26, 1, cc, 38)};

static struct ctk_label subjectlabel =
  {CTK_LABEL(0, 10, 8, 1, "Subject:")};
static char subject[40];
static struct ctk_textentry subjecttextentry =
  {CTK_TEXTENTRY(8, 10, 26, 1, subject, 38)};

static char mail[36*9];
static struct ctk_textentry mailtextentry =
  {CTK_TEXTENTRY(0, 11, 34, 9, mail, 36)};

static struct ctk_button sendbutton =
  {CTK_BUTTON(0, 21, 4, "Send")};
static struct ctk_button erasebutton =
  {CTK_BUTTON(29, 21, 5, "Erase")};

/* The "Really cancel message?" dialog. */
static struct ctk_window canceldialog;
static struct ctk_label canceldialoglabel1 =
  {CTK_LABEL(2, 1, 22, 1, "Really cancel message?")};
static struct ctk_label canceldialoglabel2 =
  {CTK_LABEL(0, 2, 26, 1, "All contents will be lost.")};
static struct ctk_button cancelyesbutton =
  {CTK_BUTTON(4, 4, 3, "Yes")};
static struct ctk_button cancelnobutton =
  {CTK_BUTTON(18, 4, 2, "No")};

/* The setup window. */
static struct ctk_window setupwindow;
static struct ctk_label fromaddresslabel =
  {CTK_LABEL(0, 0, 25, 1, "Name and e-mail address")};
static char fromaddress[40];
static struct ctk_textentry fromaddresstextentry =
  {CTK_TEXTENTRY(0, 1, 25, 1, fromaddress, 39)};

static struct ctk_label smtpserverlabel =
  {CTK_LABEL(0, 3, 20, 1, "Outgoing mailserver")};
static char smtpserver[40];
static struct ctk_textentry smtpservertextentry =
  {CTK_TEXTENTRY(0, 4, 25, 1, smtpserver, 39)};

static struct ctk_label pop3serverlabel =
  {CTK_LABEL(0, 6, 20, 1, "Incoming mailserver")};
static char pop3server[40];
static struct ctk_textentry pop3servertextentry =
  {CTK_TEXTENTRY(0, 7, 25, 1, pop3server, 39)};

static struct ctk_label pop3userlabel =
  {CTK_LABEL(0, 9, 20, 1, "Mailserver username")};
static char pop3user[40];
static struct ctk_textentry pop3usertextentry =
  {CTK_TEXTENTRY(0, 10, 25, 1, pop3user, 39)};
  
static struct ctk_label pop3passwordlabel =
  {CTK_LABEL(0, 12, 20, 1, "Mailserver password")};
static char pop3password[40];
static struct ctk_textentry pop3passwordtextentry =
  {CTK_TEXTENTRY(0, 13, 25, 1, pop3password, 39)};


static struct ctk_button setupokbutton =
  {CTK_BUTTON(24, 15, 2, "Ok")};

static DISPATCHER_SIGHANDLER(email_sighandler, s, data);
static struct dispatcher_proc p =
  {DISPATCHER_PROC("E-mail client", NULL, email_sighandler, smtp_appcall)};
static ek_id_t id;

/*-----------------------------------------------------------------------------------*/
static void
email_quit(void)
{
  ctk_window_close(&setupwindow);
  ctk_window_close(&mainwindow);
  ctk_menu_remove(&menu);
  dispatcher_exit(&p);
  id = EK_ID_NONE;
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static void
make_window(void)
{
  unsigned char i;
  struct ctk_button *button;
  struct ctk_label *label;
  
  /* Create the main window. */
  ctk_window_new(&mainwindow, 36, 22, "E-mail");
  ctk_window_move(&mainwindow, 1, 0);
  
  CTK_WIDGET_ADD(&mainwindow, &checkbutton);
  CTK_WIDGET_FOCUS(&mainwindow, &checkbutton);
  CTK_WIDGET_ADD(&mainwindow, &newmailbutton);
  CTK_WIDGET_ADD(&mainwindow, &sep1);
  CTK_WIDGET_ADD(&mainwindow, &sep2);
  CTK_WIDGET_ADD(&mainwindow, &statuslabel);
  
  for(i = 0; i < MAXNUMMSGS; ++i) {
    button = &msgbuttons[i];
    CTK_BUTTON_NEW(button, 0, i + 1, 1, " ");
    CTK_WIDGET_ADD(&mainwindow, button);
    label = &msglabels[i];
    CTK_LABEL_NEW(label, 3, i + 1, 33, 1, msgtitles[i]);
    CTK_WIDGET_ADD(&mainwindow, label);
  }

}
/*-----------------------------------------------------------------------------------*/
static void
make_composer(void)
{
  CTK_WIDGET_ADD(&mainwindow, &tolabel);
  CTK_WIDGET_ADD(&mainwindow, &cclabel);
  CTK_WIDGET_ADD(&mainwindow, &subjectlabel);
  
  CTK_WIDGET_ADD(&mainwindow, &totextentry);
  CTK_WIDGET_FOCUS(&mainwindow, &totextentry);  
  CTK_WIDGET_ADD(&mainwindow, &cctextentry);  
  CTK_WIDGET_ADD(&mainwindow, &subjecttextentry);

  CTK_WIDGET_ADD(&mainwindow, &mailtextentry);
  
  CTK_WIDGET_ADD(&mainwindow, &sendbutton);
  CTK_WIDGET_ADD(&mainwindow, &erasebutton);

  memset(mail, ' ', sizeof(mail));  
}
/*-----------------------------------------------------------------------------------*/
static void
make_read(void)
{
  CTK_WIDGET_ADD(&mainwindow, &tolabel);
  CTK_WIDGET_ADD(&mainwindow, &cclabel);
  CTK_WIDGET_ADD(&mainwindow, &subjectlabel);
  
  CTK_WIDGET_ADD(&mainwindow, &totextentry);
  /*  CTK_WIDGET_FOCUS(&mainwindow, &totextentry);  */
  CTK_WIDGET_ADD(&mainwindow, &cctextentry);  
  CTK_WIDGET_ADD(&mainwindow, &subjecttextentry);

  CTK_WIDGET_ADD(&mainwindow, &mailtextentry);
  
  CTK_WIDGET_ADD(&mainwindow, &sendbutton);
  CTK_WIDGET_ADD(&mainwindow, &erasebutton);
}
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(email_init)
{
  if(id == EK_ID_NONE) {
    id = dispatcher_start(&p);
    
    /* Create the "Really cancel message?" dialog. */
    ctk_dialog_new(&canceldialog, 26, 6);
    CTK_WIDGET_ADD(&canceldialog, &canceldialoglabel1);
    CTK_WIDGET_ADD(&canceldialog, &canceldialoglabel2);
    CTK_WIDGET_ADD(&canceldialog, &cancelyesbutton);
    CTK_WIDGET_ADD(&canceldialog, &cancelnobutton);
    CTK_WIDGET_FOCUS(&canceldialog, &cancelnobutton);
    
    /* Create setup window. */
    ctk_window_new(&setupwindow, 28, 16, "E-mail setup");
    ctk_window_move(&setupwindow, 5, 3);
    
    CTK_WIDGET_ADD(&setupwindow, &fromaddresslabel);
    CTK_WIDGET_ADD(&setupwindow, &fromaddresstextentry);
    CTK_WIDGET_ADD(&setupwindow, &smtpserverlabel);
    CTK_WIDGET_ADD(&setupwindow, &smtpservertextentry);
    CTK_WIDGET_ADD(&setupwindow, &pop3serverlabel);
    CTK_WIDGET_ADD(&setupwindow, &pop3servertextentry);
    CTK_WIDGET_ADD(&setupwindow, &pop3userlabel);
    CTK_WIDGET_ADD(&setupwindow, &pop3usertextentry);
    CTK_WIDGET_ADD(&setupwindow, &pop3passwordlabel);
    CTK_WIDGET_ADD(&setupwindow, &pop3passwordtextentry);
    CTK_WIDGET_ADD(&setupwindow, &setupokbutton);

    CTK_WIDGET_FOCUS(&setupwindow, &fromaddresstextentry);
    

    /* Create main window. */
    make_window();
    make_composer();  

    /* Create and add the menu */
    ctk_menu_new(&menu, "E-mail");
    menuitem_setup = ctk_menuitem_add(&menu, "Setup");
    menuitem_open = ctk_menuitem_add(&menu, "Open");
    menuitem_quit = ctk_menuitem_add(&menu, "Quit");
    ctk_menu_add(&menu);

    /* Attach listeners to signals. */
    dispatcher_listen(ctk_signal_button_activate);
    dispatcher_listen(ctk_signal_menu_activate);
    dispatcher_listen(ctk_signal_window_close);
    
    /* Open setup window */
    ctk_window_open(&setupwindow);
  } else {
    ctk_window_open(&mainwindow);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
applyconfig(void)
{
  u16_t addr[2];
  char *cptr;

  for(cptr = smtpserver; *cptr != ' ' && *cptr != 0; ++cptr);
  *cptr = 0;
  
  if(uip_main_ipaddrconv(smtpserver, (unsigned char *)addr)) {
    smtp_configure("contiki", addr);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
prepare_message(void)
{
  char *mptr1, *mptr2;

  /* Convert fields to ASCII. */
  petsciiconv_toascii(to, sizeof(to));
  petsciiconv_toascii(subject, sizeof(subject));  
  petsciiconv_toascii(mail, 255);
  petsciiconv_toascii(mail + 255, sizeof(mail) - 255);

  /* Insert line delimiters. */

}
/*-----------------------------------------------------------------------------------*/
static
DISPATCHER_SIGHANDLER(email_sighandler, s, data)
{
  struct ctk_widget *w;
  unsigned char i;

  DISPATCHER_SIGHANDLER_ARGS(s, data);
  
  if(s == ctk_signal_button_activate) {
    w = (struct ctk_widget *)data;
    if(w == (struct ctk_widget *)&newmailbutton) {
      /*      ctk_window_open(&composerwindow);*/
      ctk_window_close(&mainwindow);
      make_window();
      make_composer();
      ctk_window_open(&mainwindow);
      ctk_window_redraw(&mainwindow);
#if 0
    } else if(w == &replybutton) {
      /* XXX Fiddle in the from and subject fields into the new
	 mail. */
      ctk_window_open(&composerwindow);
      ctk_redraw();
#endif 
    } else if(w == (struct ctk_widget *)&checkbutton) {
      /* XXX Should actually check email. */
      ctk_label_set_text(&statuslabel, "Checking mail...");
      ctk_window_redraw(&mainwindow);
    } else if(w == (struct ctk_widget *)&sendbutton) {
      prepare_message();
      smtp_send(to, fromaddress, subject, mail, sizeof(mail));
      ctk_label_set_text(&statuslabel, "Sending message...");
      ctk_redraw();
    } else if(w == (struct ctk_widget *)&erasebutton) {
      ctk_dialog_open(&canceldialog);      
      ctk_redraw();
    } else if(w == (struct ctk_widget *)&cancelyesbutton) {
      ctk_dialog_close();
      ctk_redraw();     
    } else if(w == (struct ctk_widget *)&cancelnobutton) {
      ctk_dialog_close();
      ctk_redraw();
    } else if(w == (struct ctk_widget *)&setupokbutton) {
      applyconfig();
      ctk_window_close(&setupwindow);
      ctk_window_open(&mainwindow);
      ctk_redraw();
    } else {
      for(i = 0; i < MAXNUMMSGS; ++i) {
	if(w == (struct ctk_widget *)&msgbuttons[i]) {
	  ctk_window_close(&mainwindow);
	  make_window();
	  /*	  make_read(); download(i); */
	  ctk_window_open(&mainwindow);
	  ctk_window_redraw(&mainwindow);
	  break;
	}
      }
    }
  } else if(s == ctk_signal_menu_activate) {
    if((struct ctk_menu *)data == &menu) {
      if(menu.active == menuitem_open) {
	ctk_window_open(&mainwindow);
      } else if(menu.active == menuitem_setup) {
	ctk_window_open(&setupwindow);
      } else if(menu.active == menuitem_quit) {
	email_quit();
      }
      ctk_redraw();
    }
  } else if(s == ctk_signal_window_close &&
	    data == (ek_data_t)&mainwindow) {
    email_quit();
  }
}
/*-----------------------------------------------------------------------------------*/
void
smtp_done(unsigned char error)
{
  ctk_label_set_text(&statuslabel, "SMTP done");
  ctk_redraw();  
}
/*-----------------------------------------------------------------------------------*/
