# Microsoft Developer Studio Project File - Name="nel_patch_edit" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=nel_patch_edit - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nel_patch_edit.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nel_patch_edit.mak" CFG="nel_patch_edit - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nel_patch_edit - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nel_patch_edit - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nel_patch_edit - Win32 Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nel_patch_edit - Win32 ReleaseDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nel_patch_edit - Win32 DebugFast" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nel_patch_edit - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G6 /MD /W3 /GR /GX /O2 /I "..\..\..\..\..\..\code\nel\include" /I "..\..\..\..\..\..\code\nel\src" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 Maxscrpt.lib helpsys.lib freetype.lib winmm.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib delayimp.lib bmm.lib core.lib edmodel.lib geom.lib gfx.lib mesh.lib mnmath.lib paramblk2.lib maxutil.lib acap.lib version.lib /nologo /base:"0x05830000" /subsystem:windows /dll /machine:I386 /out:"C:\3dsmax3_1\plugins\neleditpatch.dlm" /release
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Desc=Change version number
PreLink_Cmds=buildinc version.ver mods.rc	rc /l 0x409 /fo"Release/mods.res" /d "NDEBUG" mods.rc
# End Special Build Tool

!ELSEIF  "$(CFG)" == "nel_patch_edit - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\..\..\..\..\code\nel\include" /I "..\..\..\..\..\..\code\nel\src" /D "_WINDOWS" /D "__STL_DEBUG" /D "WIN32" /D "_DEBUG" /FR /Yu"stdafx.h" /FD /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 Maxscrpt.lib helpsys.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib delayimp.lib bmm.lib core.lib edmodel.lib geom.lib gfx.lib mesh.lib mnmath.lib paramblk2.lib maxutil.lib acap.lib version.lib winmm.lib stlport_vc6.lib freetype.lib /nologo /base:"0x05830000" /subsystem:windows /dll /debug /machine:I386 /out:"C:\3dsmax3_1 debug\exe\plugins\neleditpatch.dlm" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nel_patch_edit - Win32 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\nel_patch_edit___W"
# PROP BASE Intermediate_Dir ".\nel_patch_edit___W"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Hybrid"
# PROP Intermediate_Dir "Hybrid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MD /W3 /Gm /GX /Zi /Od /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"mods.h" /c
# ADD CPP /nologo /G6 /MD /W3 /Gm /GR /GX /ZI /Od /I "..\..\..\..\..\..\code\nel\include" /I "..\..\..\..\..\..\code\nel\src" /D "WIN32" /D "_WINDOWS" /D "__STL_DEBUG" /Fr /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\..\maxsdk\plugin\mods.dlm"
# ADD LINK32 nl3d_debug.lib nlmisc_debug.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib delayimp.lib bmm.lib core.lib edmodel.lib geom.lib gfx.lib mesh.lib mnmath.lib paramblk2.lib maxutil.lib acap.lib version.lib freetype.lib /nologo /base:"0x05830000" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"msvcrtd.lib" /out:"C:\3dsmax3_1\plugins\neleditpatch.dlm" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nel_patch_edit - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nel_patch_edit___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "nel_patch_edit___Win32_ReleaseDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /GR /GX /O2 /I "..\..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GR /GX /Zi /O2 /I "..\..\..\..\..\..\code\nel\include" /I "..\..\..\..\..\..\code\nel\src" /D "_WINDOWS" /D "NL_RELEASE_DEBUG" /D "WIN32" /D "NDEBUG" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib delayimp.lib bmm.lib core.lib edmodel.lib geom.lib gfx.lib mesh.lib mnmath.lib paramblk2.lib maxutil.lib acap.lib Maxscrpt.lib helpsys.lib freetype.lib winmm.lib /nologo /base:"0x05830000" /subsystem:windows /dll /machine:I386 /out:"C:\3dsmax3_1\plugins\neleditpatch.dlm" /release
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 Maxscrpt.lib helpsys.lib freetype.lib winmm.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib delayimp.lib bmm.lib core.lib edmodel.lib geom.lib gfx.lib mesh.lib mnmath.lib paramblk2.lib maxutil.lib acap.lib version.lib /nologo /base:"0x05830000" /subsystem:windows /dll /debug /machine:I386 /out:"C:\3dsmax3_1\plugins\neleditpatch.dlm" /pdbtype:sept /release
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nel_patch_edit - Win32 DebugFast"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nel_patch_edit___Win32_DebugFast"
# PROP BASE Intermediate_Dir "nel_patch_edit___Win32_DebugFast"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugFast"
# PROP Intermediate_Dir "DebugFast"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\include" /D "_WINDOWS" /D "__STL_DEBUG" /D "WIN32" /D "_DEBUG" /FR /Yu"stdafx.h" /FD /Zm200 /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\..\..\..\..\code\nel\include" /I "..\..\..\..\..\..\code\nel\src" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "NL_DEBUG_FAST" /FR /Yu"stdafx.h" /FD /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 Maxscrpt.lib helpsys.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib delayimp.lib bmm.lib core.lib edmodel.lib geom.lib gfx.lib mesh.lib mnmath.lib paramblk2.lib maxutil.lib acap.lib version.lib winmm.lib stlport_vc6.lib freetype.lib /nologo /base:"0x05830000" /subsystem:windows /dll /debug /machine:I386 /out:"C:\3dsmax3_1 debug\exe\plugins\neleditpatch.dlm"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 Maxscrpt.lib helpsys.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib delayimp.lib bmm.lib core.lib edmodel.lib geom.lib gfx.lib mesh.lib mnmath.lib paramblk2.lib maxutil.lib acap.lib version.lib winmm.lib stlport_vc6.lib freetype.lib /nologo /base:"0x05830000" /subsystem:windows /dll /debug /machine:I386 /out:"C:\3dsmax3_1 debug\exe\plugins\neleditpatch.dlm" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy "C:\3dsmax3_1 debug\exe\plugins\neleditpatch.dlm" "c:\3dsmax3_1\plugins"	echo copie dans max
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "nel_patch_edit - Win32 Release"
# Name "nel_patch_edit - Win32 Debug"
# Name "nel_patch_edit - Win32 Hybrid"
# Name "nel_patch_edit - Win32 ReleaseDebug"
# Name "nel_patch_edit - Win32 DebugFast"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\mods.def
# End Source File
# Begin Source File

SOURCE=.\mods.rc
# End Source File
# Begin Source File

SOURCE=.\NP.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EditPatchData.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EditPatchMod.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_editpops.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_AddPatches.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_Attach.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_Bevel.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_Del.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_Detach.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_Extrude.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_File.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_GUI.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_Hide.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_Hook.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_Material.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_Remember.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_Selection.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_Subdivide.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_Surface.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPM_Tess.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_EPVertMapper.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_GUI_Bind.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_Main.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_mods.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_PatchPointTab.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_PatchRestore.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_PatchSelRestore.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_PatchVertexDelta.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_PickPatchAttach.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_Record.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\NP_Rollup.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\EDITPAT.H
# End Source File
# Begin Source File

SOURCE=.\MODS.H
# End Source File
# Begin Source File

SOURCE=.\modsres.h
# End Source File
# Begin Source File

SOURCE=.\NP_GUI_Paint_fill.h
# End Source File
# Begin Source File

SOURCE=.\NP_GUI_Paint_tileset.h
# End Source File
# Begin Source File

SOURCE=.\NP_GUI_Paint_to_nel.h
# End Source File
# Begin Source File

SOURCE=.\NP_GUI_Paint_ui.h
# End Source File
# Begin Source File

SOURCE=.\NP_GUI_Paint_undo.h
# End Source File
# Begin Source File

SOURCE=.\NP_GUI_Paint_vcolor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\code\nel\include\nel\3d\quad_tree.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\addvertc.cur
# End Source File
# Begin Source File

SOURCE=.\attach.cur
# End Source File
# Begin Source File

SOURCE=.\Bevel.cur
# End Source File
# Begin Source File

SOURCE=.\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00003.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00004.bmp
# End Source File
# Begin Source File

SOURCE=.\booleant.bmp
# End Source File
# Begin Source File

SOURCE=.\boolinte.cur
# End Source File
# Begin Source File

SOURCE=.\boolsubt.cur
# End Source File
# Begin Source File

SOURCE=.\boolunio.cur
# End Source File
# Begin Source File

SOURCE=.\bulbmask.bmp
# End Source File
# Begin Source File

SOURCE=.\bulbs.bmp
# End Source File
# Begin Source File

SOURCE=.\chamfer.cur
# End Source File
# Begin Source File

SOURCE=.\CROSSHR.CUR
# End Source File
# Begin Source File

SOURCE=.\crossins.cur
# End Source File
# Begin Source File

SOURCE=.\cur00001.cur
# End Source File
# Begin Source File

SOURCE=.\cur00002.cur
# End Source File
# Begin Source File

SOURCE=.\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\echamfer.cur
# End Source File
# Begin Source File

SOURCE=.\extrudec.cur
# End Source File
# Begin Source File

SOURCE=.\faceselt.bmp
# End Source File
# Begin Source File

SOURCE=.\fillet.cur
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\icon3.ico
# End Source File
# Begin Source File

SOURCE=.\magnify.cur
# End Source File
# Begin Source File

SOURCE=.\mask_boo.bmp
# End Source File
# Begin Source File

SOURCE=.\mask_fac.bmp
# End Source File
# Begin Source File

SOURCE=.\mask_unw.bmp
# End Source File
# Begin Source File

SOURCE=.\move_x.cur
# End Source File
# Begin Source File

SOURCE=.\move_y.cur
# End Source File
# Begin Source File

SOURCE=.\outline.cur
# End Source File
# Begin Source File

SOURCE=.\panhand.cur
# End Source File
# Begin Source File

SOURCE=.\patchsel.bmp
# End Source File
# Begin Source File

SOURCE=.\patselm.bmp
# End Source File
# Begin Source File

SOURCE=.\patselt.bmp
# End Source File
# Begin Source File

SOURCE=.\pick_color.cur
# End Source File
# Begin Source File

SOURCE=.\region.cur
# End Source File
# Begin Source File

SOURCE=.\scale_x.cur
# End Source File
# Begin Source File

SOURCE=.\scale_y.cur
# End Source File
# Begin Source File

SOURCE=.\segbreak.cur
# End Source File
# Begin Source File

SOURCE=.\segrefin.cur
# End Source File
# Begin Source File

SOURCE=.\selmask.bmp
# End Source File
# Begin Source File

SOURCE=.\splselm.bmp
# End Source File
# Begin Source File

SOURCE=.\thselcur.cur
# End Source File
# Begin Source File

SOURCE=.\Trim.cur
# End Source File
# Begin Source File

SOURCE=.\trimbut.bmp
# End Source File
# Begin Source File

SOURCE=.\trimmask.bmp
# End Source File
# Begin Source File

SOURCE=.\unwrap_option.bmp
# End Source File
# Begin Source File

SOURCE=.\unwrapto.bmp
# End Source File
# Begin Source File

SOURCE=.\vchamfer.cur
# End Source File
# Begin Source File

SOURCE=.\vertconn.cur
# End Source File
# Begin Source File

SOURCE=.\vinsert.cur
# End Source File
# Begin Source File

SOURCE=.\weld.cur
# End Source File
# End Group
# Begin Source File

SOURCE=.\keys.cfg
# End Source File
# Begin Source File

SOURCE=.\user_guide.txt
# End Source File
# End Target
# End Project
