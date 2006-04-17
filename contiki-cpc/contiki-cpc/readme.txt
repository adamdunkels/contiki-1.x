What I did to make contiki for CPC:

1. I took the contiki sources and made it compile with gcc on linux.
2. Next I made it compile with sdcc
3. Next I added cpc version of some files.
4. Then I prayed.

About contiki:

contiki is both the initial program to run to start the operating system 
AND contains a library of functions to use by the other programs. 

Compile contiki executable to generate the executable and to generate a 
list of symbols.

These symbols are used to compile the other programs. When they are run 
they will automatically call the appropiate functions.

This is not as good as referencing a fixed location jumptable, because 
each utility has to be recompiled if the contiki executable is recompiled 
:(

