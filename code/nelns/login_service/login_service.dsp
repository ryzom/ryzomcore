# Microsoft Developer Studio Project File - Name="login_service" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=login_service - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "login_service.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "login_service.mak" CFG="login_service - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "login_service - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "login_service - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "login_service - Win32 ReleaseDebug" (based on "Win32 (x86) Console Application")
!MESSAGE "login_service - Win32 DebugFast" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "login_service - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../nel/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libmySQL.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "login_service - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /Gf /I "../../nel/include" /D "__WIN__" /D "_DEBUG" /D "__STL_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libmySQL.lib /nologo /subsystem:windows /pdb:none /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /nodefaultlib:"libcd.lib"

!ELSEIF  "$(CFG)" == "login_service - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "login_service___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "login_service___Win32_ReleaseDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "../../nel/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "NL_RELEASE_DEBUG" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libmySQL.lib /nologo /subsystem:windows /pdb:none /debug /machine:I386

!ELSEIF  "$(CFG)" == "login_service - Win32 DebugFast"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "login_service___Win32_DebugFast"
# PROP BASE Intermediate_Dir "login_service___Win32_DebugFast"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugFast"
# PROP Intermediate_Dir "DebugFast"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /Zi /Od /Gf /D "_DEBUG" /D "__STL_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /Ob1 /Gf /I "../../nel/include" /D "_WINDOWS" /D "_DEBUG" /D "WIN32" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /nodefaultlib:"libcd.lib" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libmySQL.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /nodefaultlib:"libcd.lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "login_service - Win32 Release"
# Name "login_service - Win32 Debug"
# Name "login_service - Win32 ReleaseDebug"
# Name "login_service - Win32 DebugFast"
# Begin Source File

SOURCE=.\connection_web.cpp
# End Source File
# Begin Source File

SOURCE=.\connection_web.h
# End Source File
# Begin Source File

SOURCE=.\connection_ws.cpp
# End Source File
# Begin Source File

SOURCE=.\connection_ws.h
# End Source File
# Begin Source File

SOURCE=.\login_service.cfg
# End Source File
# Begin Source File

SOURCE=.\login_service.cpp
# End Source File
# Begin Source File

SOURCE=.\login_service.h
# End Source File
# End Target
# End Project
