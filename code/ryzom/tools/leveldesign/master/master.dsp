# Microsoft Developer Studio Project File - Name="master" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=master - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "master.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "master.mak" CFG="master - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "master - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "master - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "master - Win32 DebugFast" (based on "Win32 (x86) Application")
!MESSAGE "master - Win32 ReleaseDebug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "master - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../../../../code/nel/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 libxml2.lib freetype.lib /nologo /subsystem:windows /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Imports all DLLs
PostBuild_Cmds=copy ..\..\3d\ligo\worldeditor\release\worldeditor.dll .	copy ..\georges_dll\georges_dll.dll .	copy ..\logic_editor_dll\release\logic_editor.dll .
# End Special Build Tool

!ELSEIF  "$(CFG)" == "master - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "../../../../../code/nel/include" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "WIN32" /D "_DEBUG" /D "__STL_DEBUG" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libxml2_debug.lib freetype.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Imports all DLLs
PostBuild_Cmds=copy ..\..\3d\ligo\worldeditor\debug\worldeditor_debug.dll .	copy ..\georges_dll\georges_dll_debug.dll .	copy ..\logic_editor_dll\debug\logic_editor_debug.dll .
# End Special Build Tool

!ELSEIF  "$(CFG)" == "master - Win32 DebugFast"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugFast"
# PROP BASE Intermediate_Dir "DebugFast"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugFast"
# PROP Intermediate_Dir "DebugFast"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "../../../../../code/nel/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "NL_DEBUG_FAST" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libxml2.lib msvcrtd.lib libc.lib freetype.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Imports all DLLs
PostBuild_Cmds=copy ..\..\3d\ligo\worldeditor\debugfast\worldeditor_df.dll .	copy ..\georges_dll\georges_dll_debug_fast.dll .	copy ..\logic_editor_dll\debugfast\logic_editor_df.dll .
# End Special Build Tool

!ELSEIF  "$(CFG)" == "master - Win32 ReleaseDebug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseDebug"
# PROP BASE Intermediate_Dir "ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "../../../../../code/nel/include" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "NL_RELEASE_DEBUG" /D "WIN32" /D "NDEBUG" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 freetype.lib libxml2.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Imports all DLLs
PostBuild_Cmds=copy ..\..\3d\ligo\worldeditor\releasedebug\worldeditor_rd.dll .	copy ..\georges_dll\georges_dll_rd.dll .	copy ..\logic_editor_dll\releasedebug\logic_editor_rd.dll .
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "master - Win32 Release"
# Name "master - Win32 Debug"
# Name "master - Win32 DebugFast"
# Name "master - Win32 ReleaseDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ChooseDir.cpp
# End Source File
# Begin Source File

SOURCE=.\ChooseTag.cpp
# End Source File
# Begin Source File

SOURCE=.\ContinentCfg.cpp
# End Source File
# Begin Source File

SOURCE=.\ContinentPropertiesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\easy_cfg.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportCBDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\master.cpp
# End Source File
# Begin Source File

SOURCE=.\MasterTree.cpp
# End Source File
# Begin Source File

SOURCE=.\NameEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ChooseDir.h
# End Source File
# Begin Source File

SOURCE=.\ChooseTag.h
# End Source File
# Begin Source File

SOURCE=.\ContinentCfg.h
# End Source File
# Begin Source File

SOURCE=.\ContinentPropertiesDlg.h
# End Source File
# Begin Source File

SOURCE=.\easy_cfg.h
# End Source File
# Begin Source File

SOURCE=.\ExportCBDlg.h
# End Source File
# Begin Source File

SOURCE=.\ExportDlg.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\master.h
# End Source File
# Begin Source File

SOURCE=.\MasterTree.h
# End Source File
# Begin Source File

SOURCE=.\NameEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\backup.ico
# End Source File
# Begin Source File

SOURCE=.\res\continents.ico
# End Source File
# Begin Source File

SOURCE=.\res\file.ico
# End Source File
# Begin Source File

SOURCE=.\res\folder.ico
# End Source File
# Begin Source File

SOURCE=.\res\master.ico
# End Source File
# Begin Source File

SOURCE=.\master.rc
# End Source File
# Begin Source File

SOURCE=.\res\master.rc2
# End Source File
# Begin Source File

SOURCE=.\res\masterDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\regions.ico
# End Source File
# Begin Source File

SOURCE=.\res\trash.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
