# Copyright (c) 2002-2004, Adam Dunkels.
# All rights reserved. 
#
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions 
# are met: 
# 1. Redistributions of source code must retain the above copyright 
#    notice, this list of conditions and the following disclaimer. 
# 2. Redistributions in binary form must reproduce the above copyright 
#    notice, this list of conditions and the following disclaimer in the 
#    documentation and/or other materials provided with the distribution. 
# 3. The name of the author may not be used to endorse or promote
#    products derived from this software without specific prior
#    written permission.  
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
#
# This file is part of the Contiki desktop environment
#
# $Id: Makefile,v 1.18 2005/04/19 22:18:48 oliverschmidt Exp $
#

all: contiki

CONTIKI=../contiki
SDIRS = apps conf ctk lib uip 
CSDIRS = ${addprefix $(CONTIKI)/,apps ctk ek lib uip}

include $(CONTIKI)/Makefile.common
-include Makefile.depend

CC=gcc
CFLAGS=-Wall -Wstrict-prototypes -Wmissing-prototypes \
	-g  \
	${addprefix -I,$(CSDIRS)} \
	${addprefix -I,$(SDIRS)} \
	`pkg-config --cflags gtk+-2.0`\
	-DWITH_ETHERNET -DWITH_TAPDEV -DWITH_ASCII \
	-DWITH_WWW -DWITH_EMAIL -DWITH_UIP \
	-DWITH_CTKGTK
LDFLAGS= `pkg-config --libs gtk+-2.0`

vpath %.c ${SDIRS} $(CSDIRS) ${filter-out CVS,${wildcard labs/*}}

%.o: %.c
	$(CC) $(CFLAGS) -c $(<:.o=.c)

CTKGTK=ctk-gtksim.o ctk-draw.o ctk-gtksim-service.o libconio.o ctk-gtksim-draw.o
CTKVNC=ctk-vncserver.o libconio.o vnc-server.o vnc-out.o ctk-vncfont.o
CTKTERM=libconio.o ctk-term.o ctk-term-in.o ctk-term-out.o ctk-termtelnet.o

POP=pop.o popc.o popc-strings.o

contiki: contiki-main.o ek.o arg.o ek-service.o \
 tcpip.o uip.o uip_arch.o uip-fw.o uip-split.o \
 timer.o uiplib.o resolv.o resolv.o uipbuf.o \
 cfs.o cfs-posix.o codeprop.o \
 tapdev-service.o tapdev.o uip_arp.o uip-fw.o uip-fw-service.o \
 ctk.o $(CTKGTK) program-handler.o \
 $(FTP) ftp-dsc.o \
 $(EDITOR) editor-dsc.o \
 $(WWW) www-dsc.o \
 $(SHELL_) shell-dsc.o \
 $(DHCP) dhcp-dsc.o  \
 processes.o processes-dsc.o \
 calc.o calc-dsc.o \
 about.o about-dsc.o \
 netconf.o netconf-dsc.o \
 $(TELNET) $(EMAIL) $(IRC) $(VNCVIEWER) $(WEBSERVER) \
# sock-httpd.o socket.o list.o filecache.o \
# program-handler.o about.o netconf.o processes.o \
# about-dsc.o netconf-dsc.o processes-dsc.o  \
# $(CTKGTK) \
# ctk.o dispatcher.o arg.o \
# uip.o uip_arch.o uip_arp.o uip-signal.o resolv.o uipbuf.o uip-split.o uip-fw.o\
# uiplib.o tapdev.o tapdev-drv.o $(WEBSERVER) $(WWW) \
# mailget.o popclient.o nntpc.o nntpc-strings.o \
# newslog.o nntpd.o nntpd-strings.o\
# $(EMAIL) email-dsc.o $(WEBSERVER) $(TELNET) telnet-dsc.o \
# vnc.o vnc-viewer.o vnc-draw.o \
# calc.o calc-dsc.o \
# $(WWW) telnetd.o telnetd-dsc.o shell.o memb.o\
# mt.o mtarch.o mtp.o \
# coderecv.o\
 #maze.o maze-dsc.o
	gcc $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o *~ *core contiki *.s

depend:
	gcc $(CCDEPFLAGS) -MM \
	${addprefix -I,$(CSDIRS)} \
	${addprefix -I,$(SDIRS)} \
	${addsuffix /*.c, $(CSDIRS)} \
	${addsuffix /*.c, $(SDIRS)} \
	*.c > Makefile.depend

