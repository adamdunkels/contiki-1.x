# Microsoft Developer Studio Project File - Name="win32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=win32 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "win32.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "win32.mak" CFG="win32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "win32 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "win32 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "conf" /I "ctk" /I "lib" /I "ppp" /I "uip" /I "../contiki/apps" /I "../contiki/ctk" /I "../contiki/ek" /I "../contiki/lib" /I "../contiki/ppp" /I "../contiki/uip" /D "WITH_ASCII" /D "WITH_UIP" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"Release/Contiki.exe"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "conf" /I "ctk" /I "lib" /I "ppp" /I "uip" /I "../contiki/apps" /I "../contiki/ctk" /I "../contiki/ek" /I "../contiki/lib" /I "../contiki/ppp" /I "../contiki/uip" /D "WITH_ASCII" /D "WITH_UIP" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/Contiki.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "win32 - Win32 Release"
# Name "win32 - Win32 Debug"
# Begin Group "contiki-apps"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\contiki\apps\about-dsc.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\about-dsc.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\about.c
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\calc-dsc.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\calc-dsc.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\calc.c
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\email-dsc.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\email-dsc.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\email.c
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\email.h
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\html-strings.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\html-strings.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\htmlparser.c
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\htmlparser.h
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\http-strings.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\http-strings.h"
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\http-user-agent-string.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\http-user-agent-string.h"
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\netconf-dsc.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\netconf-dsc.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\netconf.c
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\processes-dsc.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\processes-dsc.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\processes.c
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\program-handler.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\program-handler.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\simpletelnet.c
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\simpletelnet.h
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\smtp-strings.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\smtp-strings.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\smtp.c
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\smtp.h
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\telnet-dsc.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\telnet-dsc.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\telnet.c
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\telnet.h
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\webclient.c
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\webclient.h
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\www-dsc.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\apps\www-dsc.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\www.c
# End Source File
# Begin Source File

SOURCE=..\contiki\apps\www.h
# End Source File
# End Group
# Begin Group "contiki-ctk"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\contiki\ctk\ctk-conio.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\ctk\ctk-conio.h"
# End Source File
# Begin Source File

SOURCE="..\contiki\ctk\ctk-draw.h"
# End Source File
# Begin Source File

SOURCE="..\contiki\ctk\ctk-mouse.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\ctk\ctk.c
# End Source File
# Begin Source File

SOURCE=..\contiki\ctk\ctk.h
# End Source File
# End Group
# Begin Group "contiki-ek"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\contiki\ek\arg.c
# End Source File
# Begin Source File

SOURCE=..\contiki\ek\arg.h
# End Source File
# Begin Source File

SOURCE=..\contiki\ek\dsc.h
# End Source File
# Begin Source File

SOURCE="..\contiki\ek\ek-service.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\ek\ek-service.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\ek\ek.c
# End Source File
# Begin Source File

SOURCE=..\contiki\ek\ek.h
# End Source File
# Begin Source File

SOURCE=..\contiki\ek\loader.h
# End Source File
# Begin Source File

SOURCE=..\contiki\ek\log.h
# End Source File
# End Group
# Begin Group "contiki-lib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\contiki\lib\cc.h
# End Source File
# Begin Source File

SOURCE=..\contiki\lib\clock.h
# End Source File
# Begin Source File

SOURCE="..\contiki\lib\ctk-textedit.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\lib\ctk-textedit.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\lib\petsciiconv.h
# End Source File
# Begin Source File

SOURCE=..\contiki\lib\timer.c
# End Source File
# Begin Source File

SOURCE=..\contiki\lib\timer.h
# End Source File
# End Group
# Begin Group "contiki-ppp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\contiki\ppp\ahdlc.c
# End Source File
# Begin Source File

SOURCE=..\contiki\ppp\ahdlc.h
# End Source File
# Begin Source File

SOURCE=..\contiki\ppp\ipcp.c
# End Source File
# Begin Source File

SOURCE=..\contiki\ppp\ipcp.h
# End Source File
# Begin Source File

SOURCE=..\contiki\ppp\lcp.c
# End Source File
# Begin Source File

SOURCE=..\contiki\ppp\lcp.h
# End Source File
# Begin Source File

SOURCE=..\contiki\ppp\pap.c
# End Source File
# Begin Source File

SOURCE=..\contiki\ppp\pap.h
# End Source File
# Begin Source File

SOURCE="..\contiki\ppp\ppp-service.c"
# End Source File
# Begin Source File

SOURCE=..\contiki\ppp\ppp.c
# End Source File
# Begin Source File

SOURCE=..\contiki\ppp\ppp.h
# End Source File
# End Group
# Begin Group "contiki-uip"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\contiki\uip\packet-service.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\uip\resolv.c
# End Source File
# Begin Source File

SOURCE=..\contiki\uip\resolv.h
# End Source File
# Begin Source File

SOURCE=..\contiki\uip\tcpip.c
# End Source File
# Begin Source File

SOURCE=..\contiki\uip\tcpip.h
# End Source File
# Begin Source File

SOURCE="..\contiki\uip\uip-fw.c"
# End Source File
# Begin Source File

SOURCE="..\contiki\uip\uip-fw.h"
# End Source File
# Begin Source File

SOURCE="..\contiki\uip\uip-split.h"
# End Source File
# Begin Source File

SOURCE=..\contiki\uip\uip.c
# End Source File
# Begin Source File

SOURCE=..\contiki\uip\uip.h
# End Source File
# Begin Source File

SOURCE=..\contiki\uip\uip_arch.h
# End Source File
# Begin Source File

SOURCE=..\contiki\uip\uip_arp.h
# End Source File
# Begin Source File

SOURCE=..\contiki\uip\uiplib.c
# End Source File
# Begin Source File

SOURCE=..\contiki\uip\uiplib.h
# End Source File
# Begin Source File

SOURCE=..\contiki\uip\uipopt.h
# End Source File
# End Group
# Begin Group "contiki-win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\main.c
# End Source File
# End Group
# Begin Group "contiki-win32-conf"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\conf\cc-conf.h"
# End Source File
# Begin Source File

SOURCE=".\conf\clock-conf.h"
# End Source File
# Begin Source File

SOURCE=".\conf\ctk-conf.h"
# End Source File
# Begin Source File

SOURCE=".\conf\ctk-conio-conf.h"
# End Source File
# Begin Source File

SOURCE=".\conf\ek-conf.h"
# End Source File
# Begin Source File

SOURCE=".\conf\log-conf.h"
# End Source File
# Begin Source File

SOURCE=".\conf\ppp-conf.h"
# End Source File
# Begin Source File

SOURCE=".\conf\uip-conf.h"
# End Source File
# Begin Source File

SOURCE=".\conf\www-conf.h"
# End Source File
# End Group
# Begin Group "contiki-win32-ctk"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\ctk\ctk-arch.c"
# End Source File
# Begin Source File

SOURCE=".\ctk\ctk-arch.h"
# End Source File
# Begin Source File

SOURCE=".\ctk\ctk-console.c"
# End Source File
# Begin Source File

SOURCE=".\ctk\ctk-console.h"
# End Source File
# End Group
# Begin Group "contiki-win32-lib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\lib\debug.c
# End Source File
# End Group
# Begin Group "contiki-win32-ppp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ppp\ppp_arch.h
# End Source File
# End Group
# Begin Group "contiki-win32-uip"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\uip\rawsock-service.c"
# End Source File
# Begin Source File

SOURCE=.\uip\rawsock.c
# End Source File
# Begin Source File

SOURCE=.\uip\rawsock.h
# End Source File
# Begin Source File

SOURCE=.\uip\uip_arch.c
# End Source File
# Begin Source File

SOURCE=.\uip\uip_arch.h
# End Source File
# End Group
# End Target
# End Project
