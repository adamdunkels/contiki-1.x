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
# $Id: Makefile,v 1.1 2003/05/19 09:13:37 gpz Exp $
#

CONTIKI=../contiki

# Change the target system type here
SYS=gba

#	CC=/opt/ham/gcc-arm/bin/arm-elf-gcc
#	LD=/opt/ham/gcc-arm/bin/arm-elf-ld
#	OBJCOPY=/opt/ham/gcc-arm/bin/arm-elf-objcopy

CC=/opt/devkitadv/bin/arm-agb-elf-gcc
AS=/opt/devkitadv/bin/arm-agb-elf-as
LD=/opt/devkitadv/bin/arm-agb-elf-ld
OBJCOPY=/opt/devkitadv/bin/arm-agb-elf-objcopy

CFLAGS=-I arch/gba -I ctk -I uip -I conf \
       -I $(CONTIKI)/apps -I $(CONTIKI)/ctk -I $(CONTIKI)/ek \
       -I $(CONTIKI)/lib -I $(CONTIKI)/uip \
       -Wa,-I,arch -Wa,-I,arch/gba \
	-D__GBA__ -DCTK_CONIO -DWITH_UIP \
	-DWITH_TELNET \
	-DWITH_EMAIL \
	-DWITH_CALC -D WITH_CALC_ICON

	#-DWITH_WWW

OPT=-O3

CINC=-Iarch/gba -Ictk -Iuip -Iconf \
       -I$(CONTIKI)/apps -I$(CONTIKI)/ctk -I$(CONTIKI)/ek \
       -I$(CONTIKI)/lib -I$(CONTIKI)/uip \
	   -D__GBA__

splint:
	@splint $(CINC) +ansi89limits +continuecomment +controlnestdepth 15 \
			+evalorder +evalorderuncon +exitarg +fcnderef +fcnmacros +fcnuse \
			+hints +forcehints +formatconst -gnuextensions +immediatetrans \
			+includenest 8 +incondefs +incondefslib +infloops +infloopsuncon \
			+internalnamelookalike \
			+iso99limits +isolib +isoreserved +isoreservedinternal \
			+libmacros +macroassign +macroempty \
			./contiki-main.c \
			\
			strncasecmp.c \
			petsciiconv.c \
 			./../contiki/apps/contiki.c \
			programs.c \
 			ctk-conio.c \
			ctk.c \
			ek.c \
			dispatcher.c \
 			program-handler.c \
			\
			./arch/gba/conio.c \
			./arch/gba/joystick.c \
			./arch/none/joytokbd.c \
			./arch/gba/vsprintf.c \
			./arch/gba/ltostr.c \
			./arch/gba/_sbrk.c \
			./arch/gba/_oserror.c \
			./arch/gba/_exit.c \
			./arch/gba/_kill.c \
			./arch/gba/_getpid.c \
			./arch/gba/gba.c \
		2>&1 | less

%.o: %.c
	@$(CC) $(CFLAGS) -S $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

%.o: ctk/%.c
	@$(CC) $(CFLAGS) -S $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

%.o: uip/%.c
	@$(CC) $(CFLAGS) -S  -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/apps/%.c
	@$(CC) $(CFLAGS) -S $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/ctk/%.c
	@$(CC) $(CFLAGS) -S $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/ek/%.c
	@$(CC) $(CFLAGS) -S $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/lib/%.c
	@$(CC) $(CFLAGS) -S $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/uip/%.c
	@$(CC) $(CFLAGS) -S $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

UIP=uip.o uip_arch.o uip_main.o resolv.o
WWW=www.o webclient.o http-strings.o htmlparser.o html-strings.o
EMAIL=email.o smtp.o smtp-strings.o
TELNET=simpletelnet.o telnet.o

#cbm only :(
#WEBSERVER=webserver.o http-strings.o
#VNC=vnc.o vnc-viewer.o vnc-draw.o vnc-draw-asm.o

all: contiki

# about-dsc.o netconf-dsc.o processes-dsc.o \

contiki: contiki-main.o strncasecmp.o petsciiconv.o \
 contiki.o programs.o \
 ctk-conio.o ctk.o ek.o dispatcher.o \
 $(UIP) \
 $(EMAIL) \
 $(TELNET) \
 program-handler.o

		cd arch;bin2o .rodata gba/ascchar.raw gba/ascchar.o

		@$(CC) $(CFLAGS) -O6 -c -I./gba -D__GBA__ \
			./arch/gba/crt0.s \
			./arch/gba/conio.c \
			./arch/gba/joystick.c \
			./arch/none/joytokbd.c \
			./arch/gba/vsprintf.c \
			./arch/gba/ltostr.c \
			./arch/gba/_sbrk.c \
			./arch/gba/_oserror.c \
			./arch/gba/_exit.c \
			./arch/gba/_kill.c \
			./arch/gba/_getpid.c \
			./arch/gba/gba.c

		@$(LD) -nostdlib -o contiki.elf -T ./arch/gba/arm-gbabin.x \
			crt0.o \
			$^ \
			./arch/gba/ascchar.o \
			gba.o \
			conio.o \
			joystick.o \
			joytokbd.o \
			vsprintf.o \
			ltostr.o \
			_sbrk.o \
			_oserror.o \
			_exit.o \
			_kill.o \
			_getpid.o \
			/opt/devkitadv/lib/gcc-lib/arm-agb-elf/3.0.2/libgcc.a \
			-lc -lg -lm -lstdc++

		@$(OBJCOPY) -v -O binary contiki.elf contiki.gba 1> /dev/null 2> /dev/null

run:
		@VisualBoyAdvance -T 500000 -f 0 -1 -s 9 --ifb-none contiki.gba

clean:
	rm -f *.o *~ *core contiki.gba contiki.elf *.s
