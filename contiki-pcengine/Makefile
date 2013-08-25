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
# $Id: Makefile,v 1.2 2005/04/18 22:46:19 oliverschmidt Exp $
#

CONTIKI=../contiki

include $(CONTIKI)/Makefile.common

# Change the target system type here
#SYS=pce
SYS=none

CC=cc65
AS=ca65
CFLAGS=-I arch/pcengine -I ctk -I uip -I conf \
       -I $(CONTIKI)/apps -I $(CONTIKI)/ctk -I $(CONTIKI)/ek \
       -I $(CONTIKI)/lib -I $(CONTIKI)/uip \
       -t $(SYS) --add-source --cpu 65C02 \
		-D __PCENGINE__ -DCTK_CONIO
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

# needed for zeropage.inc, joy-kernel.inc etc
CC65_ASMINC=/usr/lib/cc65/asminc
export CC65_ASMINC
CCFLAGS+=--asm-include-dir $(CC65_ASMINC)

#CC65_INCLUDE=/usr/lib/cc65/include
#export CC65_INC

# location of the joystick drivers
# not yet :)
#JOYDRV=/usr/lib/cc65/joy

#OPTFLAGS=-Or
#CCFLAGS+=$(OPTFLAGS)
CAFLAGS=--cpu 65C02 -D___PCENGINE__ -I arch/ -I arch/pcengine -t none
#CCFLAGS=-g --mapfile main.map

contiki: contiki-main.o strncasecmp.o petsciiconv.o \
 contiki.o programs.o \
 ctk-conio.o ctk.o ek.o dispatcher.o
		@ca65 $(CAFLAGS) -o chline.o arch/conio/chline.s
		@ca65 $(CAFLAGS) -o cvline.o arch/conio/cvline.s
		@ca65 $(CAFLAGS) -o cclear.o arch/conio/cclear.s
		@ca65 $(CAFLAGS) -o where.o arch/conio/where.s
		@ca65 $(CAFLAGS) -o gotoxy.o arch/conio/gotoxy.s
		@ca65 $(CAFLAGS) -o revers.o arch/conio/revers.s
		@ca65 $(CAFLAGS) -o oserrlist.o arch/none/oserrlist.s
		@ca65 $(CAFLAGS) -o ctype.o arch/none/ctype.s
		@ca65 $(CAFLAGS) -o crt0.o arch/pcengine/crt0.s
		@ca65 $(CAFLAGS) -o psg.o arch/pcengine/psg.s
		@ca65 $(CAFLAGS) -o vce.o arch/pcengine/vce.s
		@ca65 $(CAFLAGS) -o vdc.o arch/pcengine/vdc.s
		@ca65 $(CAFLAGS) -o conio.o arch/pcengine/conio.s
		@ca65 $(CAFLAGS) -o color.o arch/pcengine/color.s
		@ca65 $(CAFLAGS) -o kplot.o arch/pcengine/kplot.s
		@ca65 $(CAFLAGS) -o cputc.o arch/pcengine/cputc.s
		@ca65 $(CAFLAGS) -o clock.o arch/pcengine/clock.s
		@ca65 $(CAFLAGS) -o clrscr.o arch/pcengine/clrscr.s
		@ca65 $(CAFLAGS) -o _scrsize.o arch/pcengine/_scrsize.s
		@ca65 $(CAFLAGS) -o pce-stdjoy.o arch/pcengine/pce-stdjoy.s
		@cl65 --cpu 65C02 $(CFLAGS) --asm-include-dir ./arch/pcengine \
		-o contiki.pce -t none -C arch/pcengine/pcengine.x -D__PCENGINE__ \
			arch/pcengine/crt0.s $^ \
			arch/none/joytokbd.c \
			chline.o \
			cvline.o \
			cclear.o \
			where.o \
			gotoxy.o \
			psg.o \
			vce.o \
			vdc.o \
			conio.o \
			color.o \
			revers.o \
			kplot.o \
			cputc.o \
			clock.o \
			clrscr.o \
			_scrsize.o \
			pce-stdjoy.o \
			oserrlist.o \
			ctype.o \
			arch/pcengine/none65C02.lib

clean:
	rm -f *.o *~ *core contiki.pce *.s
	rm -f arch/conio/*.o
	rm -f arch/none/*.o
	rm -f arch/pcengine/*.o
