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
# $Id: Makefile,v 1.1 2003/04/05 12:35:32 adamdunkels Exp $
#

CONTIKI=../contiki
SYS=vic20

CC=cc65
AS=ca65
CFLAGS=-I ctk -I uip -I conf \
       -I $(CONTIKI)/apps -I $(CONTIKI)/ctk -I $(CONTIKI)/ek \
       -I $(CONTIKI)/lib -I $(CONTIKI)/uip \
       -t $(SYS) --add-source 
OPT=-Or

%.o: %.c
	$(CC) $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: ctk/%.c
	$(CC) $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: uip/%.c
	$(CC) $(CFLAGS)  -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/apps/%.c
	$(CC) $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/ctk/%.c
	$(CC) $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/ek/%.c
	$(CC) $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/lib/%.c
	$(CC) $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/uip/%.c
	$(CC) $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s


%.o: ctk/%.S
	$(AS) -o $@ $(AFLAGS) $<


UIP=uip.o uip_arch.o uip_main.o resolv.o

WWW=www.o webclient.o http-strings.o htmlparser.o html-strings.o 
EMAIL=email.o smtp.o smtp-strings.o
WEBSERVER=webserver.o http-strings.o 
TELNET=simpletelnet.o telnet.o

contiki: contiki-main.o strncasecmp.o petsciiconv.o \
 ctk-conio.o ctk.o ek.o dispatcher.o \
 programs.o contiki.o
	cl65 -C vic20.cfg -o contiki -t $(SYS) $^

clean:
	rm -f *.o *~ *core contiki *.s

depend:
	gcc -MM -Iapps -Ictk -Iuip -Iconf \
	-I$(CONTIKI)/lib -I$(CONTIKI)/ek \
	-I$(CONTIKI)/ctk -I$(CONTIKI)/apps -I$(CONTIKI)/uip \
	*.c */*.c $(CONTIKI)/*/*.c > Makefile.depend

include Makefile.depend