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
# $Id: Makefile,v 1.6 2003/04/24 17:02:11 adamdunkels Exp $
#

CONTIKI=../contiki
CONTIKICC65=../contiki-cc65

usage:
	@echo "Make sure the Contiki sources is in the directory $(CONTIKI)"
	@echo 'To compile Contiki, use "'$(MAKE)' target" where target'
	@echo 'is one of the following:'
	@echo tfe
	@echo rs232 programs
	@echo rs232-conio programs
	@echo d64
	@echo programs
	@echo '(Also check the Makefile for more targets to try...)'

programs:
	$(MAKE) CONTIKI=$(CONTIKI) CONTIKICC65=$(CONTIKICC65) -f Makefile.programs
tfe:
	$(MAKE) CONTIKI=$(CONTIKI) CONTIKICC65=$(CONTIKICC65) -f Makefile.c64-tfe
rs232:
	$(MAKE) CONTIKI=$(CONTIKI) CONTIKICC65=$(CONTIKICC65) -f Makefile.c64-rs232
rs232-conio:
	$(MAKE) CONTIKI=$(CONTIKI) CONTIKICC65=$(CONTIKICC65) -f Makefile.c64-rs232-conio


CCDEPFLAGS=-D__CBM__ -D__C64__ -DCTK_HIRES

include $(CONTIKICC65)/Makefile.common

