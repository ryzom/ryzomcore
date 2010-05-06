# Microsoft Developer Studio Project File - Name="nel_launcher" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=nel_launcher - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nel_launcher.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nel_launcher.mak" CFG="nel_launcher - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nel_launcher - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "nel_launcher - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "nel_launcher - Win32 ReleaseDebug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nel_launcher - Win32 Release"

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
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"std_afx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"std_afx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 curllib_static.lib libcurl.lib zlib.lib wininet.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 Debug"

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
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"std_afx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "__STL_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_DEBUG" /D "WIN32" /D "_MBCS" /Yu"std_afx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 curllib_static.lib zlib.lib wininet.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 ReleaseDebug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nel_launcher___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "nel_launcher___Win32_ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"std_afx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "NL_RELEASE_DEBUG" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "WIN32" /D "_MBCS" /Yu"std_afx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 curllib_static.lib zlib.lib wininet.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "nel_launcher - Win32 Release"
# Name "nel_launcher - Win32 Debug"
# Name "nel_launcher - Win32 ReleaseDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\connection.cpp
# End Source File
# Begin Source File

SOURCE=.\nel_launcher.cpp
# End Source File
# Begin Source File

SOURCE=.\nel_launcher.rc
# End Source File
# Begin Source File

SOURCE=.\nel_launcher_dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\patch.cpp
# End Source File
# Begin Source File

SOURCE=.\std_afx.cpp
# ADD CPP /Yc"std_afx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\connection.h
# End Source File
# Begin Source File

SOURCE=.\nel_launcher.h
# End Source File
# Begin Source File

SOURCE=.\nel_launcher_dlg.h
# End Source File
# Begin Source File

SOURCE=.\patch.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\std_afx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\nel_launcher.ico
# End Source File
# Begin Source File

SOURCE=.\res\nel_launcher.rc2
# End Source File
# End Group
# End Target
# End Project
