# Microsoft Developer Studio Project File - Name="sheets_packer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=sheets_packer - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sheets_packer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sheets_packer.mak" CFG="sheets_packer - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sheets_packer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "sheets_packer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "sheets_packer - Win32 ReleaseDebug" (based on "Win32 (x86) Application")
!MESSAGE "sheets_packer - Win32 DebugFast" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sheets_packer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".."
# PROP Intermediate_Dir "../tmp/sheets_packer/release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../nel/include" /I "." /I "client" /I "client/interfaces_manager" /I "client/interfaces" /I "client/motion" /I "client/motion/modes" /I "client/interface_v2" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Yu"stdpch.h" /FD /Zm200 /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"../tmp/sheets_packer/release/sheets_packer.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 nlligo_r.lib nl3d_r.lib nlnet_r.lib nlmisc_r.lib nlpacs_r.lib nlsound_r.lib nlgeorges_r.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib freetype.lib libxml2.lib /nologo /subsystem:console /pdb:none /machine:I386 /out:"../sheets_packer_r.exe" /libpath:"../../nel/lib" /fixed:no

!ELSEIF  "$(CFG)" == "sheets_packer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".."
# PROP Intermediate_Dir "../tmp/sheets_packer/debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /I "../../nel/include" /I "." /I "client" /I "client/interfaces_manager" /I "client/interfaces" /I "client/motion" /I "client/motion/modes" /I "client/interface_v2" /D "_DEBUG" /D "__STL_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fr /Yu"stdpch.h" /FD /GZ /Zm"200" /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"../tmp/sheets_packer/debug/sheets_packer.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 nlligo_d.lib nl3d_d.lib nlnet_d.lib nlmisc_d.lib nlpacs_d.lib nlgeorges_d.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib freetype_debug.lib libxml2.lib /nologo /subsystem:console /debug /machine:I386 /out:"../sheets_packer_d.exe" /libpath:"../../nel/lib" /fixed:no
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "sheets_packer - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "sheets_packer___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "sheets_packer___Win32_ReleaseDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".."
# PROP Intermediate_Dir "../tmp/sheets_packer/ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "." /I "client" /I "client/interfaces_manager" /I "client/interfaces" /I "client/motion" /I "client/motion/modes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "../../nel/include" /I "." /I "client" /I "client/interfaces_manager" /I "client/interfaces" /I "client/motion" /I "client/motion/modes" /I "client/interface_v2" /D "NDEBUG" /D "NL_RELEASE_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Yu"stdpch.h" /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"../tmp/sheets_packer/release_debug/sheets_packer.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /debug /machine:I386
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib freetype.lib libxml2.lib /nologo /subsystem:console /incremental:yes /debug /machine:I386 /out:"../sheets_packer_rd.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "sheets_packer - Win32 DebugFast"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sheets_packer___Win32_DebugFast"
# PROP BASE Intermediate_Dir "sheets_packer___Win32_DebugFast"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".."
# PROP Intermediate_Dir "../tmp/sheets_packer/debugfast"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "." /I "client" /I "client/interfaces_manager" /I "client/interfaces" /I "client/motion" /I "client/motion/modes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "__STL_DEBUG" /FR /YX /FD /GZ /Zm"200" /c
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /Ob1 /I "../../nel/include" /I "." /I "client" /I "client/interfaces_manager" /I "client/interfaces" /I "client/motion" /I "client/motion/modes" /I "client/interface_v2" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fr /Yu"stdpch.h" /FD /GZ /Zm"200" /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"../tmp/sheets_packer/debugfast/sheets_packer.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /map:"../client_ryzom_d.map" /debug /machine:I386 /out:"../client_ryzom_d.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib freetype_debug.lib libxml2_debug.lib /nologo /subsystem:console /debug /machine:I386 /out:"../sheets_packer_df.exe" /libpath:"../../nel/lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "sheets_packer - Win32 Release"
# Name "sheets_packer - Win32 Debug"
# Name "sheets_packer - Win32 ReleaseDebug"
# Name "sheets_packer - Win32 DebugFast"
# Begin Group "Manager"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\client\continent_manager_build.cpp
# End Source File
# Begin Source File

SOURCE=.\client\continent_manager_build.h
# End Source File
# Begin Source File

SOURCE=.\client\sheet_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\sheet_manager.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\sheets_packer.cfg
# End Source File
# Begin Source File

SOURCE=.\sheets_packer\sheets_packer.cpp
# End Source File
# Begin Source File

SOURCE=.\sheets_packer\sheets_packer_cfg.cpp
# End Source File
# Begin Source File

SOURCE=.\sheets_packer\sheets_packer_cfg.h
# End Source File
# Begin Source File

SOURCE=.\sheets_packer\sheets_packer_init.cpp
# End Source File
# Begin Source File

SOURCE=.\sheets_packer\sheets_packer_init.h
# End Source File
# Begin Source File

SOURCE=.\sheets_packer\sheets_packer_release.cpp
# End Source File
# Begin Source File

SOURCE=.\sheets_packer\sheets_packer_release.h
# End Source File
# Begin Source File

SOURCE=.\sheets_packer\stdpch.cpp
# ADD CPP /Yc"stdpch.h"
# End Source File
# Begin Source File

SOURCE=.\sheets_packer\stdpch.h
# End Source File
# End Target
# End Project
