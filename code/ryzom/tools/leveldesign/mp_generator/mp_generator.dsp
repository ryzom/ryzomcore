# Microsoft Developer Studio Project File - Name="mp_generator" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=mp_generator - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mp_generator.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mp_generator.mak" CFG="mp_generator - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mp_generator - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "mp_generator - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "mp_generator - Win32 ReleaseDebug" (based on "Win32 (x86) Console Application")
!MESSAGE "mp_generator - Win32 DebugFast" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mp_generator - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../../../../code/nel/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 nlmisc_r.lib /nologo /subsystem:console /pdb:"mp_generator_r.pdb" /machine:I386 /nodefaultlib:"msvcrtd.lib" /out:"mp_generator_r.exe" /libpath:"../../nel/lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "mp_generator - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../../../../code/nel/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 nlmisc_d.lib /nologo /subsystem:console /debug /machine:I386 /out:"mp_generator_d.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "mp_generator - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "mp_generator___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "mp_generator___Win32_ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "../../../../../code/nel/include" /D "NDEBUG" /D "NL_RELEASE_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 lua_r.lib nlligo_rd.lib nlsound_rd.lib nlmisc_rd.lib nl3d_rd.lib nlnet_rd.lib nlpacs_rd.lib nlgeorges_rd.lib freetype.lib libxml2.lib zlib.lib curllib_static.lib wwwfile.lib wwwmime.lib wwwmux.lib wwwcache.lib wwwapp.lib wwwcore.lib wwwhtml.lib wwwhttp.lib wwwtrans.lib wwwutils.lib libjpeg.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /nodefaultlib:"msvcrtd.lib" /libpath:"../../nel/lib"
# ADD LINK32 libxml2.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:"mp_generator.pdb" /debug /machine:I386 /out:"mp_generator_rd.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "mp_generator - Win32 DebugFast"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "mp_generator___Win32_DebugFast"
# PROP BASE Intermediate_Dir "mp_generator___Win32_DebugFast"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "DebugFast"
# PROP Intermediate_Dir "DebugFast"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "NDEBUG" /D "NL_RELEASE_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT BASE CPP /Fr /YX /Yc /Yu
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /I "../." /I "../../../../../code/nel/include" /D "NL_DEBUG_FAST" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libxml2.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib nlmisc_rd.lib /nologo /subsystem:console /pdb:"mp_generator.pdb" /debug /machine:I386 /out:"mp_generator_rd.exe"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 libxml2.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib nlmisc_df.lib /nologo /subsystem:console /pdb:"mp_generator.pdb" /debug /machine:I386 /out:"mp_generator_df.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "mp_generator - Win32 Release"
# Name "mp_generator - Win32 Debug"
# Name "mp_generator - Win32 ReleaseDebug"
# Name "mp_generator - Win32 DebugFast"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\moulinette.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
