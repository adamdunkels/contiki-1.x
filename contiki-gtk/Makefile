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
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#      This product includes software developed by Adam Dunkels.
# 4. The name of the author may not be used to endorse or promote
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
# $Id: Makefile,v 1.1 2003/04/02 09:17:18 adamdunkels Exp $
#

CONTIKI=../contiki
CC=gcc
CFLAGS=-g -pg\
       -Iapps -Ictk -Iuip -Iconf \
       -I$(CONTIKI)/apps -I$(CONTIKI)/ctk -I$(CONTIKI)/ek \
       -I$(CONTIKI)/lib -I$(CONTIKI)/uip \
	`gtk12-config --cflags`\
	-DWITH_ETHERNET -DWITH_TAPDEV -DWITH_ASCII \
	-DWITH_WWW -DWITH_EMAIL -DWITH_UIP
LDFLAGS=-pg `gtk12-config --libs`

%.o: %.c
	$(CC) $(CFLAGS) -c $(<:.o=.c)
%.o: apps/%.c
	$(CC) $(CFLAGS) -c $(<:.o=.c)
%.o: ctk/%.c
	$(CC) $(CFLAGS) -c $(<:.o=.c)
%.o: uip/%.c
	$(CC) $(CFLAGS) -c $(<:.o=.c)
%.o: $(CONTIKI)/apps/%.c
	$(CC) $(CFLAGS) -c $(<:.o=.c)
%.o: $(CONTIKI)/ctk/%.c
	$(CC) $(CFLAGS) -c $(<:.o=.c)
%.o: $(CONTIKI)/ek/%.c
	$(CC) $(CFLAGS) -c $(<:.o=.c)
%.o: $(CONTIKI)/lib/%.c
	$(CC) $(CFLAGS) -c $(<:.o=.c)
%.o: $(CONTIKI)/uip/%.c
	$(CC) $(CFLAGS) -c $(<:.o=.c)


WWW=www.o webclient.o http-strings.o htmlparser.o html-strings.o 
EMAIL=email.o smtp.o smtp-strings.o
WEBSERVER=webserver.o http-strings.o 
TELNET=simpletelnet.o telnet.o

contiki: contiki-main.o \
 programs.o contiki.o \
 ctk-gtksim.o \
 ctk.o ek.o dispatcher.o \
 uip.o uip_arch.o uip_arp.o tapdev.o resolv.o \
 uip_main.o \
 $(WWW) $(EMAIL)
	gcc $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o *~ *core contiki *.s

depend:
	gcc -MM -Iapps -Ictk -Iuip -Iconf \
	-I$(CONTIKI)/lib -I$(CONTIKI)/ek \
	-I$(CONTIKI)/ctk -I$(CONTIKI)/apps -I$(CONTIKI)/uip \
	*.c */*.c $(CONTIKI)/*/*.c > Makefile.depend

include Makefile.depend