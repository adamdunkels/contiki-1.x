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
# $Id: Makefile,v 1.2 2005/04/18 22:45:29 oliverschmidt Exp $
#

CONTIKI=../contiki

include $(CONTIKI)/Makefile.common

# Change the target system type here
SYS=gp32

#	CC=/opt/ham/gcc-arm/bin/arm-elf-gcc
#	LD=/opt/ham/gcc-arm/bin/arm-elf-ld
#	OBJCOPY=/opt/ham/gcc-arm/bin/arm-elf-objcopy

CC=/opt/devkitadv/bin/arm-agb-elf-gcc
AS=/opt/devkitadv/bin/arm-agb-elf-as
LD=/opt/devkitadv/bin/arm-agb-elf-ld
OBJCOPY=/opt/devkitadv/bin/arm-agb-elf-objcopy

CFLAGS=-I arch/gp32 -I ctk -I uip -I conf \
       -I $(CONTIKI)/apps -I $(CONTIKI)/ctk -I $(CONTIKI)/ek \
       -I $(CONTIKI)/lib -I $(CONTIKI)/uip \
       -Wa,-I,arch -Wa,-I,arch/gp32 \
	-D__GP32__ -DCTK_CONIO -DWITH_UIP \
	-DWITH_WWW -D WITH_TELNET_WWW \
	-DWITH_TELNET -D WITH_TELNET_ICON \
	-DWITH_EMAIL -D WITH_EMAIL_ICON \
	-DWITH_CALC -D WITH_CALC_ICON

OPT=-O3
%.o: %.c
	$(CC) $(CFLAGS) -S $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: ctk/%.c
	$(CC) $(CFLAGS) -S $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: uip/%.c
	$(CC) $(CFLAGS) -S  -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/apps/%.c
	$(CC) $(CFLAGS) -S $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/ctk/%.c
	$(CC) $(CFLAGS) -S $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/ek/%.c
	$(CC) $(CFLAGS) -S $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/lib/%.c
	$(CC) $(CFLAGS) -S $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/uip/%.c
	$(CC) $(CFLAGS) -S $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	$(AS) -o $@ $(AFLAGS) $(*).s

all: contiki

contiki: contiki-main.o strncasecmp.o petsciiconv.o \
 contiki.o programs.o \
 ctk-conio.o ctk.o ek.o dispatcher.o \
 $(WWW) \
 $(EMAIL) \
 $(TELNET) \
 $(UIP)


		cd arch;bin2o .rodata gp32/font8x8.bin gp32/font8x8.o

		$(CC) $(CFLAGS) -O6 -c -I./gp32 -D__GP32__ \
			./arch/gp32/crt0.s \
			./arch/gp32/gp32.c \
			./arch/gp32/syscall.s \
			./arch/gp32/vsprintf.c \
			./arch/gp32/_sbrk.c \
			./arch/gp32/_oserror.c \
			./arch/gp32/_exit.c \
			./arch/gp32/_kill.c \
			./arch/gp32/_getpid.c \
			./arch/gp32/ltostr.c \
			./arch/gp32/joystick.c \
			./arch/none/joytokbd.c \
			./arch/gp32/gppaletteset.c \
			./arch/gp32/conio.c

		$(LD) -nostdlib -o contiki.elf -T ./arch/gp32/arm-gp32bin.x \
			crt0.o \
			$^ \
			gp32.o \
			./arch/gp32/font8x8.o \
			vsprintf.o \
			_sbrk.o \
			_exit.o \
			_kill.o \
			_getpid.o \
			ltostr.o \
			joystick.o \
			joytokbd.o \
			_oserror.o \
			conio.o \
			syscall.o \
			gppaletteset.o \
			/opt/devkitadv/lib/gcc-lib/arm-agb-elf/3.0.2/libgcc.a \
			-lc -lg -lm -lstdc++

		$(OBJCOPY) -v -O binary contiki.elf contiki.gxb 1> /dev/null 2> /dev/null

run:
	/home/groepaz/geepee32/geepee32 /EXE=contiki.gxb /RUN		

clean:
	rm -f *.o *~ *core contiki.gxb contiki.elf *.s
	rm -f arch/gp32/*.o
