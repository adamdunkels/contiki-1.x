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
# $Id: Makefile,v 1.2 2004/07/04 19:58:55 adamdunkels Exp $
#

all: contiki loader-arch-module.o contiki-labels.o \
     about.prg processes.prg netconf.prg memstat.prg calc.prg

CONTIKI=../contiki
CONTIKICC65=../contiki-cc65
SYS=pet

CC=cc65
AS=ca65
CFLAGS=-I apps -I ctk -I loader -I uip -I conf \
       -I $(CONTIKI)/apps -I $(CONTIKI)/ctk -I $(CONTIKI)/ek \
       -I $(CONTIKI)/lib -I $(CONTIKI)/uip \
       -t $(SYS) --add-source -DWITH_LOADER_ARCH 
OPT=-Or

%.o: %.c
	$(CC) $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: apps/%.c
	$(CC) $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: ctk/%.c
	$(CC) $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: loader/%.c
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

%.o: $(CONTIKICC65)/apps/%.c
	$(CC) $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s


%.o: apps/%.S
	$(AS) -o $@ $(AFLAGS) $<

%.o: ctk/%.S
	$(AS) -o $@ $(AFLAGS) $<

%.o: loader/%.S
	$(AS) -o $@ $(AFLAGS) $<

%.prg:  %.o contiki-labels.o
	cl65 --module -t $(SYS) -o $@ loader-arch-module.o $^


contiki-labels.o: contiki-labels.s
	ca65 -o contiki-labels.o contiki-labels.s
contiki-labels.s: contiki
	$(CONTIKICC65)/make-labels

contiki: contiki-main.o strncasecmp.o petsciiconv.o \
 ctk-conio.o ctk.o ek.o timer.o arg.o \
 program-handler.o loader-arch.o
	cl65 -Ln contiki-labels -o contiki -t $(SYS) $^


clean:
	rm -f *.o *~ *core contiki *.s *.prg

depend:
	gcc -MM -Iapps -Ictk -Iuip -Iconf \
	-I$(CONTIKI)/lib -I$(CONTIKI)/ek \
	-I$(CONTIKI)/ctk -I$(CONTIKI)/apps -I$(CONTIKI)/uip \
	*.c */*.c $(CONTIKI)/*/*.c > Makefile.depend

include Makefile.depend

codesize:
	od65 --dump-segsize *.o | egrep '\.o|CODE'
