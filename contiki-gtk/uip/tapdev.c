/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 *
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 *
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: tapdev.c,v 1.2 2003/05/28 05:22:40 adamdunkels Exp $
 */


#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include <gtk/gtk.h>

#ifdef linux
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#define DEVTAP "/dev/net/tun"
#else  /* linux */
#define DEVTAP "/dev/tap0"
#endif /* linux */

#include "uip.h"
#include "uip_arp.h"

#define DROP 0

static int drop = 0;
static int fd;

static unsigned long lasttime;
static struct timezone tz;

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

void tapdev_send(void);


static void
read_callback(gpointer data, gint source, GdkInputCondition condition)
{
  int ret;
  
  ret = read(fd, uip_buf, UIP_BUFSIZE);  
  if(ret == -1) {
    perror("tap_dev: tapdev_read: read");
  }

  uip_len = ret;
  
  if(BUF->type == htons(UIP_ETHTYPE_IP)) {
    uip_arp_ipin();
    uip_len -= sizeof(struct uip_eth_hdr);
    uip_input();
    /* If the above function invocation resulted in data that
       should be sent out on the network, the global variable
       uip_len is set to a value > 0. */
    if(uip_len > 0) {
      uip_arp_out();
      tapdev_send();
    }
  } else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
    uip_arp_arpin();
    /* If the above function invocation resulted in data that
       should be sent out on the network, the global variable
       uip_len is set to a value > 0. */	
    if(uip_len > 0) {
      tapdev_send();
    }
  }


}
gint
timeout_callback(gpointer data)
{
  static u8_t i, arptimer;
  
  for(i = 0; i < UIP_CONNS; i++) {
    uip_periodic(i);
    /* If the above function invocation resulted in data that
       should be sent out on the network, the global variable
       uip_len is set to a value > 0. */
    if(uip_len > 0) {
      uip_arp_out();
      tapdev_send();
    }
  }

  for(i = 0; i < UIP_UDP_CONNS; i++) {
    uip_udp_periodic(i);
    /* If the above function invocation resulted in data that
       should be sent out on the network, the global variable
       uip_len is set to a value > 0. */
    if(uip_len > 0) {
	uip_arp_out();
	tapdev_send();
      }
    }

  /* Call the ARP timer function every 10 seconds. */
  if(++arptimer == 20) {	
    uip_arp_timer();
    arptimer = 0;
  }
 
  return TRUE;
}

/*-----------------------------------------------------------------------------------*/
void
tapdev_init(void)
{
  char buf[1024];
  
  fd = open(DEVTAP, O_RDWR);
  if(fd == -1) {
    perror("tapdev: tapdev_init: open");
    exit(1);
  }

#ifdef linux
  {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP|IFF_NO_PI;
    if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
      perror(buf);
      exit(1);
    }
  }
#endif /* Linux */

  snprintf(buf, sizeof(buf), "ifconfig tap0 inet %d.%d.%d.%d",
	   UIP_DRIPADDR0, UIP_DRIPADDR1, UIP_DRIPADDR2, UIP_DRIPADDR3);
  system(buf);

  lasttime = 0;

  gdk_input_add(fd, GDK_INPUT_READ,
		read_callback, NULL);

  gtk_timeout_add(500, timeout_callback, NULL);
}
/*-----------------------------------------------------------------------------------*/
unsigned int
tapdev_read(void)
{
  fd_set fdset;
  struct timeval tv, now;
  int ret;
  
  if(lasttime >= 500000) {
    lasttime = 0;
    return 0;
  }
  
  tv.tv_sec = 0;
  tv.tv_usec = 500000 - lasttime;


  FD_ZERO(&fdset);
  FD_SET(fd, &fdset);

  gettimeofday(&now, &tz);  
  ret = select(fd + 1, &fdset, NULL, NULL, &tv);
  if(ret == 0) {
    lasttime = 0;    
    return 0;
  } 
  ret = read(fd, uip_buf, UIP_BUFSIZE);  
  if(ret == -1) {
    perror("tap_dev: tapdev_read: read");
  }
  gettimeofday(&tv, &tz);
  lasttime += (tv.tv_sec - now.tv_sec) * 1000000 + (tv.tv_usec - now.tv_usec);

#if DROP
  drop++;
  if(drop % 8 == 7) {
    printf("Dropped an input packet!\n");
    return 0;
  }
#endif /* DROP */

  /*  printf("--- tap_dev: tapdev_read: read %d bytes\n", ret);*/
  /*  {
    int i;
    for(i = 0; i < 20; i++) {
      printf("%x ", uip_buf[i]);
    }
    printf("\n");
    }*/
  /*  check_checksum(uip_buf, ret);*/
  return ret;
}
/*-----------------------------------------------------------------------------------*/
void
tapdev_send(void)
{
  int ret;
  char tmpbuf[UIP_BUFSIZE];
  int i;
  

  /*  printf("tapdev_send: sending %d bytes\n", size);*/
  /*  check_checksum(uip_buf, size);*/
#if DROP
  drop++;
  if(drop % 8 == 7) {
    printf("Dropped an output packet!\n");
    return;
  }
#endif /* DROP */
  
  for(i = 0; i < 40 + UIP_LLH_LEN; i++) {
    tmpbuf[i] = uip_buf[i];
  }
  
  for(; i < uip_len; i++) {
    tmpbuf[i] = uip_appdata[i - 40 - UIP_LLH_LEN];
  }
  
  ret = write(fd, tmpbuf, uip_len);
  
  if(ret == -1) {
    perror("tap_dev: tapdev_send: writev");
    exit(1);
  }
}  
/*-----------------------------------------------------------------------------------*/
