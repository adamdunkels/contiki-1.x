
Contiki on the Apple ][ (Oliver Schmidt)
========================================


Contiki for the Apple ][ comes in two variants:

1. 'Contiki40' runs in 40 column text mode and requires at least an Apple ][+
   with Language Card resulting in 64kB memory.

2. 'Contiki80' runs in 80 column text mode and requires at least an enhanced
   Apple //e with Extended 80 Column Board resulting in 128kB memory.
   
The remainder of this text describes Contiki80.


Installation
------------

1. Get the 'cc65 complete cross development package' from 06/01/2006 or later:
   http://www.cc65.org


2. Build the two targets apple2enh and programsenh.


3. Get the 'a2tools' by Terry Kyriacopoulos:
   ftp://ftp.apple.asimov.net/pub/apple_II/unsorted/a2tools.zip


4. Get the 'Apple][ ProDOS 8 system program for loading binary programs':
   ftp://ftp.musoftware.de/pub/uz/cc65/contrib/loader-1.2.zip


5. Use the 'a2tools' to copy these files into a DOS 3.3 disk image:

   a2tools in b      dos33.dsk CONTIKI.SYSTEM loader~1.sys
   a2tools in -r b   dos33.dsk CONTIKI        contiki
   a2tools in b.0000 dos33.dsk A2E.STDMOU.MOU a2estd~1.mou
   a2tools in b.0000 dos33.dsk ABOUT.PRG      about.prg
   a2tools in b.0000 dos33.dsk ABOUT.DSC      about.dsc
   a2tools in b.0000 dos33.dsk CALC.PRG       calc.prg
   a2tools in b.0000 dos33.dsk CALC.DSC       calc.dsc
   a2tools in b.0000 dos33.dsk CONFIG.PRG     config.prg
   a2tools in b.0000 dos33.dsk CONFIGEDIT.PRG config~1.prg
   a2tools in b.0000 dos33.dsk CONFIGEDIT.DSC config~1.dsc
   a2tools in b.0000 dos33.dsk DHCP.PRG       dhcp.prg
   a2tools in b.0000 dos33.dsk DHCP.DSC       dhcp.dsc
   a2tools in b.0000 dos33.dsk DIRECTORY.PRG  direct~1.prg
   a2tools in b.0000 dos33.dsk DIRECTORY.DSC  direct~1.dsc
   a2tools in b.0000 dos33.dsk EMAIL.PRG      email.prg
   a2tools in b.0000 dos33.dsk EMAIL.DSC      email.dsc
   a2tools in b.0000 dos33.dsk FTP.PRG        ftp.prg
   a2tools in b.0000 dos33.dsk FTP.DSC        ftp.dsc
   a2tools in b.0000 dos33.dsk IRC.PRG        irc.prg
   a2tools in b.0000 dos33.dsk IRC.DSC        irc.dsc
   a2tools in b.0000 dos33.dsk MEMSTAT.PRG    memstat.prg
   a2tools in b.0000 dos33.dsk MEMSTAT.DSC    memstat.dsc
   a2tools in b.0000 dos33.dsk PROCESSES.PRG  proces~1.prg
   a2tools in b.0000 dos33.dsk PROCESSES.DSC  proces~1.dsc
   a2tools in b.0000 dos33.dsk SHELL.PRG      shell.prg
   a2tools in b.0000 dos33.dsk SHELL.DSC      shell.dsc
   a2tools in b.0000 dos33.dsk TELNET.PRG     telnet.prg
   a2tools in b.0000 dos33.dsk TELNET.DSC     telnet.dsc
   a2tools in b.0000 dos33.dsk WEBSERVER.PRG  webser~1.prg
   a2tools in b.0000 dos33.dsk WEBSERVER.DSC  webser~1.dsc
   a2tools in b.0000 dos33.dsk WELCOME.PRG    welcome.prg
   a2tools in b.0000 dos33.dsk WGET.PRG       wget.prg
   a2tools in b.0000 dos33.dsk WGET.DSC       wget.dsc
   a2tools in b.0000 dos33.dsk WWW.PRG        www.prg
   a2tools in b.0000 dos33.dsk WWW.DSC        www.dsc
   a2tools in b.0000 dos33.dsk LANCEGS.DRV    lancegs.drv
   a2tools in b.0000 dos33.dsk LANCEGS.DSC    lancegs.dsc
   a2tools in b.0000 dos33.dsk UTHER.DRV      uther.drv
   a2tools in b.0000 dos33.dsk UTHER.DSC      uther.dsc
   a2tools in b.0000 dos33.dsk BOUNCE.SAV     bounce.sav
   a2tools in b.0000 dos33.dsk BOUNCE.DSC     bounce.dsc
   a2tools in b.0000 dos33.dsk PLASMA.SAV     plasma.sav
   a2tools in b.0000 dos33.dsk PLASMA.DSC     plasma.dsc
   a2tools in b.0000 dos33.dsk SSFIRE.SAV     ssfire.sav
   a2tools in b.0000 dos33.dsk SSFIRE.DSC     ssfire.dsc


6. Use any ProDOS 8 file utility program capable of reading DOS 3.3 files to
   copy the files from the DOS 3.3 disk to a ProDOS 8 disk.


7. Use BASIC.SYSTEM to convert the loader from a BIN to a SYS file:

   BLOAD  CONTIKI.SYSTEM
   DELETE CONTIKI.SYSTEM
   CREATE CONTIKI.SYSTEM,TSYS
   BSAVE  CONTIKI.SYSTEM,TSYS,A$2000,L465


Usage
-----

Select CONTIKI.SYSTEM from the ProDOS 8 dispatcher or use BASIC.SYSTEM:

   - CONTIKI.SYSTEM

1. CONTIKI.SYSTEM supports the ProDOS 8 startup file protocol. The startup file
   will be initially launched instead of WELCOME.PRG.

   The startup file support allows to open *.PRG files from the GS/OS finder:
   
   a) Give your Contiki disk the volume name /CONTIKI
   b) Copy contiki.icon to /CONTIKI/ICONS/CONTIKI.ICON
   c) Set the filetype of CONTIKI.ICON to $CA aka ICN
   d) Set the filetype of the *.PRG files to $FE aka REL
   
2. Contiki includes mouse support for the AppleMouse II Card, the Apple //c and
   the Apple IIgs. All slots are searched for mouse firmware but if non is found
   the mouse support is silently turned off.
   
   IIgs ROM 3 users: Make sure to have set 'Slot 4' in the 'Control Panel' to
                     'Mouse Port' although this isn't necessary for GS/OS.


Implementation Notes
--------------------

1. File I/O

   Contiki loads programs, drivers and screensavers from disk using ProDOS 8.
   The ProDOS 8 file I/O library in the cc65 C-library is to large to be used
   to load WWW.PRG into a 64k Apple ][. Fortunately WWW.PRG itself does no file
   I/O but delegates that to WGET.PRG which is a lot smaller than WWW.PRG. so
   the solution is to have two different file I/O libaries.
   
   The first one is as small as possible and is only capable of reading one file
   at a time and only from the directory CONTIKI resides in. It is part of the
   Contiki kernel and thus called 'KernelFileSystem' (kfs). ProDOS 8 needs a 1kB
   I/O buffer aligned to a page boundary for every opoen file. The one and only
   I/O buffer necessary for fks is placed at $0800-$0BFF and serves as buffer
   for incoming/outgoing IP data as well.
   
   The other one which is called 'ContikiFileSystem' (cfs) is mapped to the
   existing cc65 C-library and linked into the programs (like WGET.PRG).
   
2. Memory Layout

   The Contiki memory layout is based on the standard cc65 layout which means
   that the binary is loaded to the lowest available address ($0C00 here) and
   contains the CODE, RODATA and DATA segments. The BSS segment is located
   directly above those and set to zero programatically. The cc65 stack (not
   to be confused with the 6502 stack at $0100-$01FF) starts at the highest
   available address and grows downwards. The heap is located between the BSS
   and the stack.
   
   But to satisfy the memory requirements of the larger Contiki programs
   several modifications were necessary:
   
   Generally it's very hard to make use of memory mapping for an event driven
   system like Contiki but the Apple //e allows to map only 8kB of Aux memory
   to the address space $2000-$3FFF to facilitate double hires graphics. This
   feature is accompanied by ProDOS 8 which allows to keep /RAM generally active
   while doing double hires graphics by saving a 8kB file as first file to /RAM
   and thus preserving $2000-$3FFF of Aux memory. Contiki makes use of all this
   double hires support while just staying in text mode and using the 8kB as
   additional memory.

   The additional 8kB are used to store the code of the uIP TCP/IP stack. They
   are mapped in both on calls from the Contiki event kernel to the TCP/IP stack
   process (poll and event handler) and calls to the uIP API from programs. Most
   of the time the 8kB are just kept mapped in when the uIP code calls other
   code. This is possible because the only memory not reachable from the uIP
   code are the corresponding 8kB of Main memory - and the Contiki kernel
   objects are linked in an order which makes only CTK code (which is never
   called by uIP code) use that 8kB of Main memory. The only call from uIP code
   that triggers mapping out the 8kB of Aux memory is the uIP upcall into a
   program for processing incoming IP data as some program may potentially call
   CTK code while processing that data.

   Beside that Contiki makes use of the Language Card bank 2 to store the code
   of the C-libary. Most of this 4 kB can be considered free although officially
   marked as reserved by ProDOS 8. Only $D100-$D3FF are actually used to store
   the ProDOS 8 dispatcher. As it is only used after terminating Contiki it can
   be saved to /RAM on startup and restored from there while cleanup.

   The code doing all that code relocation gets overwritten on setting the BSS
   segment to zero. The code doing this (and the C-library initialization) gets
   overwritten later by the heap content.
   
eof