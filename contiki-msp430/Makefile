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
# $Id: Makefile,v 1.2 2003/09/05 21:23:37 adamdunkels Exp $
#

all: contiki

CONTIKI=../contiki

include Makefile.depend

CC=msp430-gcc
LD=msp430-ld
AS=msp430-as
OBJCOPY=msp430-objcopy
CFLAGS=-Os -I$(CONTIKI)/apps -I$(CONTIKI)/ctk -I$(CONTIKI)/ek \
       -I$(CONTIKI)/lib -I$(CONTIKI)/uip \
       -Iapps -Ictk -Iuip -Iconf \
       -DWITH_UIP -DWITH_ASCII \
       -Wall -g -I../uip -I. -mmcu=msp430x149
LDFLAGS=-mmcu=msp430x149 -Wl,-Map=contiki.map,--cref

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

UIP=uip.o uip_arch.o uip_main.o resolv.o

WWW=www.o webclient.o http-strings.o http-user-agent-string.o \
    htmlparser.o html-strings.o  www-dsc.o
EMAIL=email.o smtp.o smtp-strings.o
WEBSERVER=webserver.o http-strings.o httpd.o httpd-fs.o httpd-cgi.o
TELNET=simpletelnet.o telnet.o

clean:
	rm -f *.o *~ *core contiki contiki.srec *.lst *.map

depend:
	gcc $(CCDEPFLAGS) -MM -Iapps -Ictk -Iuip -Iconf \
	-I$(CONTIKI)/lib -I$(CONTIKI)/ek \
	-I$(CONTIKI)/ctk -I$(CONTIKI)/apps -I$(CONTIKI)/uip \
	*.c */*.c $(CONTIKI)/*/*.c > Makefile.depend

contiki: contiki-main.o strncasecmp.o sensors.o \
 dispatcher.o \
 uip.o uip_arch.o resolv.o uip-signal.o uip_main.o \
 rs232.o rs232dev.o slip-drv.o \
 ctk.o ctk-vncserver.o libconio.o vnc-server.o vnc-out.o ctk-vncfont.o \
 program-handler.o sensorview.o sensorview-dsc.o \
 $(WEBSERVER) 
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^


erase:
	jtag/pyjtag/jtag.py -l /dev/ppi0 -e

upload: erase
	jtag/pyjtag/jtag.py -l /dev/ppi0 -D -p contiki
	jtag/pyjtag/jtag.py -l /dev/ppi0 -D -r

upload-remote: contiki
	scp contiki sidewalker.sics.se:/tmp
	ssh sidewalker.sics.se "/adam/contiki-msp430/jtag/pyjtag/jtag.py -l /dev/ppi0 -e"
	ssh sidewalker.sics.se "/adam/contiki-msp430/jtag/pyjtag/jtag.py -l /dev/ppi0 -D -D -S -R 2048 -p /tmp/contiki"
	ssh sidewalker.sics.se "/adam/contiki-msp430/jtag/pyjtag/jtag.py -l /dev/ppi0 -r"

