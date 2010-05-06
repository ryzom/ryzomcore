# Microsoft Developer Studio Project File - Name="logic_editor_dll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=logic_editor_dll - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "logic_editor_dll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "logic_editor_dll.mak" CFG="logic_editor_dll - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "logic_editor_dll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "logic_editor_dll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "logic_editor_dll - Win32 DebugFast" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "logic_editor_dll - Win32 ReleaseDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "logic_editor_dll - Win32 Release"

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
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../../../../code/nel/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 libxml2.lib /nologo /subsystem:windows /dll /machine:I386 /out:"release\logic_editor.dll"

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 Debug"

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
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "../../../../../code/nel/include" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "__STL_DEBUG" /D "WIN32" /D "_DEBUG" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libxml2_debug.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"debug\logic_editor_debug.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\logic_editor_debug.dll ..\logic_editor_exe	copy Debug\logic_editor_debug.pdb ..\logic_editor_exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 DebugFast"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "logic_editor___Win32_DebugFast"
# PROP BASE Intermediate_Dir "logic_editor___Win32_DebugFast"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugFast"
# PROP Intermediate_Dir "DebugFast"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "__STL_DEBUG" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "../../../../../code/nel/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "NL_DEBUG_FAST" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /out:"R:\code\nel\tools\leveldesign\logic_editor_exe\logic_editor.dll" /pdbtype:sept
# ADD LINK32 libxml2.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"debugfast\logic_editor_df.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 ReleaseDebug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "logic_editor___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "logic_editor___Win32_ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "../../../../../code/nel/include" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "NL_RELEASE_DEBUG" /D "WIN32" /D "NDEBUG" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 libxml2.lib libxml2_debug.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"releasedebug\logic_editor_rd.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "logic_editor_dll - Win32 Release"
# Name "logic_editor_dll - Win32 Debug"
# Name "logic_editor_dll - Win32 DebugFast"
# Name "logic_editor_dll - Win32 ReleaseDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\Condition.cpp
# End Source File
# Begin Source File

SOURCE=.\ConditionPage.cpp
# End Source File
# Begin Source File

SOURCE=.\ConditionsView.cpp
# End Source File
# Begin Source File

SOURCE=.\Counter.cpp
# End Source File
# Begin Source File

SOURCE=.\CounterPage.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorFormView.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorPropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=.\logic_editor.cpp
# End Source File
# Begin Source File

SOURCE=.\logic_editor.rc
# End Source File
# Begin Source File

SOURCE=.\logic_editorDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\logic_editorView.cpp
# End Source File
# Begin Source File

SOURCE=.\LogicTreeView.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\NumEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizablePage.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizableSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\State.cpp
# End Source File
# Begin Source File

SOURCE=.\StatePage.cpp
# End Source File
# Begin Source File

SOURCE=.\StatesView.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\VariablePage.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\Condition.h
# End Source File
# Begin Source File

SOURCE=.\ConditionPage.h
# End Source File
# Begin Source File

SOURCE=.\ConditionsView.h
# End Source File
# Begin Source File

SOURCE=.\Counter.h
# End Source File
# Begin Source File

SOURCE=.\CounterPage.h
# End Source File
# Begin Source File

SOURCE=.\EditorFormView.h
# End Source File
# Begin Source File

SOURCE=.\EditorPropertySheet.h
# End Source File
# Begin Source File

SOURCE=.\logic_editor.h
# End Source File
# Begin Source File

SOURCE=.\logic_editor_interface.h
# End Source File
# Begin Source File

SOURCE=.\logic_editorDoc.h
# End Source File
# Begin Source File

SOURCE=.\logic_editorView.h
# End Source File
# Begin Source File

SOURCE=.\LogicTreeView.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\NumEdit.h
# End Source File
# Begin Source File

SOURCE=.\ResizablePage.h
# End Source File
# Begin Source File

SOURCE=.\ResizableSheet.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\State.h
# End Source File
# Begin Source File

SOURCE=.\StatePage.h
# End Source File
# Begin Source File

SOURCE=.\StatesView.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TMenu.h
# End Source File
# Begin Source File

SOURCE=.\VariablePage.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\logic_editor.ico
# End Source File
# Begin Source File

SOURCE=.\res\logic_editor.rc2
# End Source File
# Begin Source File

SOURCE=.\res\logic_editorDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\logic_editor_debug.def

!IF  "$(CFG)" == "logic_editor_dll - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 Debug"

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 DebugFast"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 ReleaseDebug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\logic_editor_debug_fast.def

!IF  "$(CFG)" == "logic_editor_dll - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 DebugFast"

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 ReleaseDebug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\logic_editor_release.def

!IF  "$(CFG)" == "logic_editor_dll - Win32 Release"

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 DebugFast"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 ReleaseDebug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\logic_editor_release_debug.def

!IF  "$(CFG)" == "logic_editor_dll - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 DebugFast"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "logic_editor_dll - Win32 ReleaseDebug"

!ENDIF 

# End Source File
# End Target
# End Project
