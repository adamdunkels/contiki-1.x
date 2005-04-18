
Contiki on the Apple ][ (Oliver Schmidt)
========================================


Installation
------------

1. Build the two targets apple2enh and programsenh.


2. Get the 'Apple][ ProDOS 8 system program for loading binary programs':
   ftp://ftp.musoftware.de/pub/uz/cc65/contrib/loader-1.1.zip


3. Use the 'a2tools' by Terry Kyriacopoulos to copy these files into a DOS 3.3
   disk image:

   a2 in b      dos33.dsk CONTIKI.SYSTEM loader~1.sys
   a2 in -r b   dos33.dsk CONTIKI        contiki
   a2 in b.0000 dos33.dsk LANCEGS.DRV    lancegs.drv
   a2 in b.0000 dos33.dsk TFE.DRV        tfe.drv
   a2 in b.0000 dos33.dsk ABOUT.PRG      about.prg
   a2 in b.0000 dos33.dsk CALC.PRG       calc.prg
   a2 in b.0000 dos33.dsk CONFIG.PRG     config.prg
   a2 in b.0000 dos33.dsk CONFIGEDIT.PRG config~1.prg
   a2 in b.0000 dos33.dsk DHCP.PRG       dhcp.prg
   a2 in b.0000 dos33.dsk FTP.PRG        ftp.prg
   a2 in b.0000 dos33.dsk IRC.PRG        irc.prg
   a2 in b.0000 dos33.dsk MEMSTAT.PRG    memstat.prg
   a2 in b.0000 dos33.dsk PROCESSES.PRG  proces~1.prg
   a2 in b.0000 dos33.dsk SHELL.PRG      shell.prg
   a2 in b.0000 dos33.dsk TELNET.PRG     telnet.prg
   a2 in b.0000 dos33.dsk WELCOME.PRG    welcome.prg
   a2 in b.0000 dos33.dsk WGET.PRG       wget.prg
   a2 in b.0000 dos33.dsk WWW.PRG        www.prg


4. Use any ProDOS 8 file utility program capable of reading DOS 3.3 files to
   copy the files from the DOS 3.3 disk to a ProDOS 8 disk.


5. Use BASIC.SYSTEM to convert the loader from a BIN to a SYS file:

   BLOAD  CONTIKI.SYSTEM
   DELETE CONTIKI.SYSTEM
   CREATE CONTIKI.SYSTEM,TSYS
   BSAVE  CONTIKI.SYSTEM,TSYS,A$2000,L459


Usage
-----

Select CONTIKI.SYSTEM from the ProDOS 8 dispatcher or use BASIC.SYSTEM:

   - CONTIKI.SYSTEM


Note
----

Contiki uses $D400-$DFFF in the language card bank2 although this memory is
marked as reserved by ProDOS 8.


eof