# Copyright (c) 2002, Adam Dunkels.
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
# $Id: Makefile,v 1.7 2004/09/13 10:34:30 adamdunkels Exp $
#

all: contiki.srec

CONTIKI=../contiki

include Makefile.depend

CC=avr-gcc
LD=avr-ld
AS=avr-as
OBJCOPY=avr-objcopy
CFLAGS=-Os -Iapps -Ictk -Ilib -Iuip -Iconf \
	-I$(CONTIKI)/apps -I$(CONTIKI)/ctk -I$(CONTIKI)/ek \
       -I$(CONTIKI)/lib -I$(CONTIKI)/uip \
	-DWITH_UIP -DWITH_ASCII -DWITH_AVR \
	-Wall -g -fpack-struct -I../uip -I. -mmcu=atmega128 
LDFLAGS=-mmcu=atmega128 -Wl,-Map=contiki.map,--cref,-Tdata,0x801100

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $(notdir $(<:.c=.o))

%.o: apps/%.c
	$(CC) $(CFLAGS) -c $< -o $(notdir $(<:.c=.o))

%.o: ctk/%.c
	$(CC) $(CFLAGS) -c $< -o $(notdir $(<:.c=.o))

%.o: uip/%.c
	$(CC) $(CFLAGS) -c $< -o $(notdir $(<:.c=.o))

%.o: $(CONTIKICC65)/uip/%.c
	$(CC) $(CFLAGS) -c $< -o $(notdir $(<:.c=.o))
%.o: $(CONTIKI)/apps/%.c
	$(CC) $(CFLAGS) -c $< -o $(notdir $(<:.c=.o))
%.o: $(CONTIKI)/ctk/%.c
	$(CC) $(CFLAGS) -c $< -o $(notdir $(<:.c=.o))
%.o: $(CONTIKI)/ek/%.c
	$(CC) $(CFLAGS) -c $< -o $(notdir $(<:.c=.o))
%.o: $(CONTIKI)/lib/%.c
	$(CC) $(CFLAGS) -c $< -o $(notdir $(<:.c=.o))
%.o: $(CONTIKI)/uip/%.c
	$(CC) $(CFLAGS) -c $< -o $(notdir $(<:.c=.o))

%.srec:
	$(OBJCOPY) -O srec $< $@

UIP=uip.o uip_arch.o uiplib.o resolv.o 

WWW=www.o webclient.o http-strings.o http-user-agent-string.o \
    htmlparser.o html-strings.o  www-dsc.o
EMAIL=email.o smtp.o smtp-strings.o
WEBSERVER=webserver.o http-strings.o 
TELNET=simpletelnet.o telnet.o

SYSTEM=ek.o ek-service.o loader-arch.o
CONTIKIFILES= $(SYSTEM) $(UIP) uip-fw.o uip-fw-service.o timer.o 

CTKVNC=ctk-vncserver.o libconio.o vnc-server.o vnc-out.o ctk-vncfont.o
CTKTELNET=libconio.o ctk-term.o ctk-term-in.o ctk-term-out.o ctk-termtelnet.o

IRC=irc.o ircc.o socket.o uipbuf.o ctk-textedit.o irc-dsc.o ircc-strings.o

clean:
	rm -f *.o *~ *core contiki contiki.srec *.lst *.map

depend:
	gcc $(CCDEPFLAGS) -MM -Iapps -Ictk -Iuip -Iconf \
	-I$(CONTIKI)/lib -I$(CONTIKI)/ek \
	-I$(CONTIKI)/ctk -I$(CONTIKI)/apps -I$(CONTIKI)/uip \
	*.c */*.c $(CONTIKI)/*/*.c > Makefile.depend

contiki: $(CONIKIFILES) contiki-main.o strncasecmp.o \
 ctk.o ek.o arg.o timer.o ek-service.o \
 uip.o uip_arch.o uip_arp.o resolv.o uip-event.o uiplib.o tcpip.o uip-split.o \
 rtl8019.o rtl8019dev.o delay.o debug.o rtl8019-drv.o \
 $(CTKVNC) program-handler.o  \
 webserver.o httpd.o httpd-cgi.o httpd-fs.o webserver-dsc.o \
 netconf.o netconf-dsc.o \
 $(WWW) weblinks.o weblinks-dsc.o \
 about.o about-dsc.o \
 processes.o processes-dsc.o calc.o calc-dsc.o 
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^


contiki.srec: contiki

erase:
	uisp -dprog=stk200 --erase -v

upload: erase
	uisp -dprog=stk200 --upload -v if=contiki.srec

upload-remote: contiki.srec
	scp contiki.srec sidewalker.sics.se:/tmp
	ssh sidewalker.sics.se "uisp -dprog=stk200 --erase -v"
	ssh sidewalker.sics.se "uisp -dprog=stk200 --upload -v if=/tmp/contiki.srec"

