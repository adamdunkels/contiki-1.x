
Contiki on the Apple ][ (Oliver Schmidt)
========================================

To make as much as possible memory available to Contiki the DOS 3.3 (clone)
used for loading Contiki into memory will be overwritten on Contiki startup.

Contiki requires DOS to be moved into language card (bank 2). It has been
successfully tested with:

- DavidDOS with HIDOS
- DavidDOS II with HIDOS
- DiversiDOS with DDMOVER
- 64k DiversiDOS

With the 'a2tools by Terry Kyriacopoulos' you use the following commands to
insert Contiki into an (already existing) DOS 3.3 (clone) dsk image file:

a2tools in    t contiki.dsk CONTIKI.TEXT contiki.txt
a2tools in -r b contiki.dsk CONTIKI.CONF contiki.con
a2tools in -r b contiki.dsk CONTIKI.CODE contiki.cod
a2tools in -r b contiki.dsk CONTIKI.DATA contiki.dat

On the Apple ][ enter the following commands on the Applesoft prompt (']') to
create the Contiki wrapper program.

NEW
EXEC CONTIKI.TEXT
SAVE CONTIKI

Contiki uses the follwoing keys:

toggle menu: ESC
next widget: TAB
prev widget: CONTROL-A
next window: CONTROL-W

As there is no DOS available while Contiki is running the configuration data
is saved by running the Contiki wrapper program again immediately after exiting
Contiki via Ctrl-Reset and rebooting. The wrapper program checks if the data is
still available and prints "Saving Contiki Conf..." if it is.

eof