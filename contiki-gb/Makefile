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
# $Id: Makefile,v 1.1 2003/05/19 08:21:01 gpz Exp $
#

CONTIKI=../contiki

# Change the target system type here
SYS=gbc

OPT=

CC=/usr/local/bin/sdcc
AS=/usr/local/bin/as-gbz80
LD=/usr/local/bin/link-gbz80

CFLAGS=-I arch/gbc -I ctk -I uip -I conf \
       -I $(CONTIKI)/apps -I $(CONTIKI)/ctk -I $(CONTIKI)/ek \
       -I $(CONTIKI)/lib -I $(CONTIKI)/uip \
       \
	  -DCTK_CONIO \
     -I./apps -I./ctk -I./ek -I./lib -I./uip -I./arch/gb -I./arch -I.\
	-DCTK_THEME_H="ctk-conio.h" -D__SDCC__ -D__GAMEBOY__ \
	-DWITH_ASCII -mgbz80 -I /usr/local/share/sdcc/include \
	-Wall -A -A --noinvariant --noinduction --callee-saves --callee-saves-bc \
	--nojtbound --noloopreverse --int-long-reent --float-reent --no-peep \
	--xram-movc

LDFLAGS=
AFLAGS=-g

CCFLAGS=$(CFLAGS)
ASFLAGS=$(AFLAGS)


%.o: %.c
	@$(CC) -S $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

%.o: ctk/%.c
	@$(CC) -S $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

%.o: uip/%.c
	@$(CC) -S $(CFLAGS)  -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/apps/%.c
	@$(CC) -S $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/ctk/%.c
	@$(CC) -S $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/ek/%.c
	@$(CC) -S $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/lib/%.c
	@$(CC) -S $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s

%.o: $(CONTIKI)/uip/%.c
	@$(CC) -S $(CFLAGS) $(OPT) -o $(patsubst %c, %s, $(notdir $<)) $<
	@$(AS) -o $@ $(AFLAGS) $(*).s


UIP=uip.o uip_arch.o uip_main.o resolv.o
WWW=www.o webclient.o http-strings.o htmlparser.o html-strings.o
EMAIL=email.o smtp.o smtp-strings.o
TELNET=simpletelnet.o telnet.o

#cbm only :(
#WEBSERVER=webserver.o http-strings.o
#VNC=vnc.o vnc-viewer.o vnc-draw.o vnc-draw-asm.o

all: contiki

contiki: contiki-main.o strncasecmp.o \
 	contiki.o programs.o \
 	ctk-conio.o ctk.o ek.o dispatcher.o

		@cd arch;$(AS) $(ASFLAGS) -o ../crt0.o gb/crt0.s

		@$(CC) $(CCFLAGS) -S arch/gb/conio.c -o conio.asm
		@$(AS) $(ASFLAGS) -o conio.o conio.asm
		@$(CC) $(CCFLAGS) -S arch/gb/_oserror.c -o _oserror.asm
		@$(AS) $(ASFLAGS) -o _oserror.o _oserror.asm
		@$(CC) $(CCFLAGS) -S arch/gb/font.c -o font.asm
		@$(AS) $(ASFLAGS) -o font.o font.asm
		@$(CC) $(CCFLAGS) -S arch/gb/memmove.c -o memmove.asm
		@$(AS) $(ASFLAGS) -o memmove.o memmove.asm
		@$(CC) $(CCFLAGS) -S arch/gb/vsprintf.c -o vsprintf.asm
		@$(AS) $(ASFLAGS) -o vsprintf.o vsprintf.asm
		@$(CC) $(CCFLAGS) -S arch/gb/ltostr.c -o ltostr.asm
		@$(AS) $(ASFLAGS) -o ltostr.o ltostr.asm
		@$(CC) $(CCFLAGS) -S arch/gb/get_tv.c -o get_tv.asm
		@$(AS) $(ASFLAGS) -o get_tv.o get_tv.asm
		@$(CC) $(CCFLAGS) -S arch/none/joytokbd.c -o joytokbd.asm
		@$(AS) $(ASFLAGS) -o joytokbd.o joytokbd.asm

		@cd arch;$(AS) $(ASFLAGS) -o  ../init_tt.o gb/init_tt.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../set_spr.o gb/set_spr.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../mv_spr.o gb/mv_spr.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../arand.o gb/arand.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../hiramcpy.o gb/hiramcpy.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../cpy_data.o gb/cpy_data.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../delay.o gb/delay.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../joystick.o gb/joystick.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../rand.o gb/rand.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../scroll_b.o gb/scroll_b.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../scroll_s.o gb/scroll_s.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../get_bk_t.o gb/get_bk_t.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../scroll_w.o gb/scroll_w.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../get_data.o gb/get_data.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../serial.o gb/serial.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../get_prop.o gb/get_prop.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../set_bk_t.o gb/set_bk_t.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../set_data.o gb/set_data.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../get_wi_t.o gb/get_wi_t.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../set_prop.o gb/set_prop.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../get_xy_t.o gb/get_xy_t.s
		@cd arch;$(AS) $(ASFLAGS) -o  ../set_wi_t.o gb/set_wi_t.s
		@cd arch;$(AS) $(ASFLAGS) -o   ../set_xy_t.o gb/set_xy_t.s

		@cd arch;$(AS) $(ASFLAGS) -o ../fontms.o gb/fontms.s
		@cd arch;$(AS) $(ASFLAGS) -o ../drawing.o gb/drawing.s

		@$(LD) -- -z contiki.gb \
			crt0.o \
 		   /usr/local/share/sdcc/lib/gbz80/crt0_rle.o \
			$^ \
			memmove.o \
			vsprintf.o \
			joytokbd.o \
			_oserror.o \
		   /usr/local/share/sdcc/lib/gbz80/_strcat.o \
  		   /usr/local/share/sdcc/lib/gbz80/_strcpy.o \
  		   /usr/local/share/sdcc/lib/gbz80/_strncpy.o \
  		   /usr/local/share/sdcc/lib/gbz80/_strncmp.o \
  		   /usr/local/share/sdcc/lib/gbz80/_strlen.o \
  		   /usr/local/share/sdcc/lib/gbz80/_memcpy.o \
  		   /usr/local/share/sdcc/lib/gbz80/_memset.o \
		   /usr/local/share/sdcc/lib/gbz80/_modslong.o \
		   /usr/local/share/sdcc/lib/gbz80/_divslong.o \
		   /usr/local/share/sdcc/lib/gbz80/_modulong.o \
		   /usr/local/share/sdcc/lib/gbz80/_divulong.o \
		   /usr/local/share/sdcc/lib/gbz80/_mullong.o \
		   /usr/local/share/sdcc/lib/gbz80/shift.o \
		   /usr/local/share/sdcc/lib/gbz80/mul.o \
		   /usr/local/share/sdcc/lib/gbz80/div.o \
		   /usr/local/share/sdcc/lib/gbz80/stubs.o \
			\
			conio.o \
			ltostr.o \
			get_tv.o \
 			joystick.o\
			font.o \
		init_tt.o\
		set_spr.o\
		mv_spr.o\
		arand.o\
		hiramcpy.o\
		cpy_data.o \
		delay.o \
		drawing.o\
		rand.o\
		scroll_b.o\
		scroll_s.o\
		get_bk_t.o \
		scroll_w.o\
		get_data.o \
		serial.o\
		get_prop.o  \
		set_bk_t.o\
		set_data.o\
		get_wi_t.o \
		set_prop.o\
		get_xy_t.o \
		set_wi_t.o\
		set_xy_t.o\
		fontms.o

run:
	@xgnuboy contiki.gb

clean:
	@rm -f *.o *~ *core contiki.gb *.s *.asm *.sym *.rel *.lst *.lnk *.map
