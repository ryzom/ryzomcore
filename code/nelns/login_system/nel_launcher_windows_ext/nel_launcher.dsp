# Microsoft Developer Studio Project File - Name="nel_launcher" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=nel_launcher - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nel_launcher.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nel_launcher.mak" CFG="nel_launcher - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nel_launcher - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "nel_launcher - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "nel_launcher - Win32 DebugFast" (based on "Win32 (x86) Application")
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
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "C:\Dev\Ryzom\code\nel\include\nel" /I "C:\Dev\Ryzom\code\nel\src" /I "C:\Dev\Ryzom\code\nel\include\nel\stlport" /I "C:\Dev\zlib114" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 zlib.lib wininet.lib /nologo /subsystem:windows /incremental:yes /machine:I386 /nodefaultlib:"LIBCMTD" /libpath:"C:\Dev\Ryzom\code\nel\lib" /libpath:"C:\Dev\zlib114"

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
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GR /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "__STL_DEBUG" /U "LIBCMTD" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /Gf /X /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 zlib.lib Wininet.lib /nologo /subsystem:windows /pdb:none /debug /debugtype:both /machine:I386

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 DebugFast"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nel_launcher___Win32_DebugFast"
# PROP BASE Intermediate_Dir "nel_launcher___Win32_DebugFast"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugFast"
# PROP Intermediate_Dir "DebugFast"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "__STL_DEBUG" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GR /GX /Z7 /Od /Ob1 /Gf /I "D:\nevrax\code\nel\src" /I "D:\Launcher\src\include\stlport" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "NL_DEBUG_FAST" /U "LIBCMTD" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 zlib.lib Wininet.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 zlib.lib Wininet.lib /nologo /subsystem:windows /pdb:none /debug /machine:I386 /nodefaultlib:"LIBCMTD" /libpath:"D:\Launcher\src\lib" /libpath:"D:\nevrax\code\nel\lib"

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 ReleaseDebug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nel_launcher___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "nel_launcher___Win32_ReleaseDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Z7 /O2 /I "C:\Dev\Ryzom\code\nel\include\nel" /I "C:\Dev\Ryzom\code\nel\src" /I "C:\Dev\Ryzom\code\nel\include\nel\stlport" /I "C:\Dev\zlib114" /D "NL_RELEASE_DEBUG" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /U "LIBCMTD" /FR /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 zlib.lib Wininet.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 Zlib.lib Wininet.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCMTD" /libpath:"C:\Dev\Ryzom\code\nel\lib" /libpath:"C:\Dev\zlib114"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ENDIF 

# Begin Target

# Name "nel_launcher - Win32 Release"
# Name "nel_launcher - Win32 Debug"
# Name "nel_launcher - Win32 DebugFast"
# Name "nel_launcher - Win32 ReleaseDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BarTabsWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\BarWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\Configuration.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadingPageDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LoginDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Md5.cpp
# End Source File
# Begin Source File

SOURCE=.\MsgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\nel_launcher.cpp

!IF  "$(CFG)" == "nel_launcher - Win32 Release"

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 Debug"

# ADD CPP /Yu

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 DebugFast"

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 ReleaseDebug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nel_launcher.rc
# End Source File
# Begin Source File

SOURCE=.\nel_launcherDlg.cpp

!IF  "$(CFG)" == "nel_launcher - Win32 Release"

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 Debug"

# ADD CPP /Yu

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 DebugFast"

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 ReleaseDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\patch.cpp

!IF  "$(CFG)" == "nel_launcher - Win32 Release"

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 Debug"

# ADD CPP /Yu

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 DebugFast"

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 ReleaseDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PictureHlp.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\webbrowser2.cpp

!IF  "$(CFG)" == "nel_launcher - Win32 Release"

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 Debug"

# ADD CPP /Yu

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 DebugFast"

!ELSEIF  "$(CFG)" == "nel_launcher - Win32 ReleaseDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WebDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BarTabsWnd.h
# End Source File
# Begin Source File

SOURCE=.\BarWnd.h
# End Source File
# Begin Source File

SOURCE=.\Configuration.h
# End Source File
# Begin Source File

SOURCE=.\LoadingPageDlg.h
# End Source File
# Begin Source File

SOURCE=.\LoginDlg.h
# End Source File
# Begin Source File

SOURCE=.\Md5.h
# End Source File
# Begin Source File

SOURCE=.\MsgDlg.h
# End Source File
# Begin Source File

SOURCE=.\nel_launcher.h
# End Source File
# Begin Source File

SOURCE=.\nel_launcherDlg.h
# End Source File
# Begin Source File

SOURCE=.\patch.h
# End Source File
# Begin Source File

SOURCE=.\PictureHlp.h
# End Source File
# Begin Source File

SOURCE=.\ProgressDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\webbrowser2.h
# End Source File
# Begin Source File

SOURCE=.\WebDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\background.jpg
# End Source File
# Begin Source File

SOURCE=.\res\bg_login.jpg
# End Source File
# Begin Source File

SOURCE=.\res\btn_login_down.bmp
# End Source File
# Begin Source File

SOURCE=.\res\btn_login_up.bmp
# End Source File
# Begin Source File

SOURCE=.\res\btn_quit_down.bmp
# End Source File
# Begin Source File

SOURCE=.\res\btn_quit_up.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\iconic.bmp
# End Source File
# Begin Source File

SOURCE=.\res\nel_launcher.ico
# End Source File
# Begin Source File

SOURCE=.\res\nel_launcher.rc2
# End Source File
# Begin Source File

SOURCE=.\res\progress.jpg
# End Source File
# Begin Source File

SOURCE=.\res\ryzom.ico
# End Source File
# Begin Source File

SOURCE=.\res\tab_news.jpg
# End Source File
# Begin Source File

SOURCE=.\res\tab_news_focus.jpg
# End Source File
# Begin Source File

SOURCE=.\res\tab_rn.jpg
# End Source File
# Begin Source File

SOURCE=.\res\tab_rn_focus.jpg
# End Source File
# Begin Source File

SOURCE=.\res\tab_servers.jpg
# End Source File
# Begin Source File

SOURCE=.\res\tab_servers_focus.jpg
# End Source File
# End Group
# Begin Source File

SOURCE=.\nel_launcher.cfg
# End Source File
# End Target
# End Project
# Section nel_launcher : {8856F961-340A-11D0-A96B-00C04FD705A2}
# 	2:21:DefaultSinkHeaderFile:webbrowser2.h
# 	2:16:DefaultSinkClass:CWebBrowser2
# End Section
# Section nel_launcher : {D30C1661-CDAF-11D0-8A3E-00C04FC9E26E}
# 	2:5:Class:CWebBrowser2
# 	2:10:HeaderFile:webbrowser2.h
# 	2:8:ImplFile:webbrowser2.cpp
# End Section
