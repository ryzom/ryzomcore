;--------------------------------
;Include Modern UI
!include "MUI2.nsh"

;--------------------------------
;General
!define Company "Winch Gate"
!define GenericProduct "Ryzom"
!define Product "Ryzom Installer"
!define RegistryCat "HKCU"
!define RegistryKey "Software\${Company}\${GenericProduct}"
!define Executable "ryzom_installer_qt_r.exe"
!define SrcDir "RyzomInstaller"
!define Installer "ryzom_installer.exe"

;Properly display all languages (Installer will not work on Windows 95, 98 or ME!)
;Unicode true

;New XP style
XPStyle on

;Name and file
Name "${Product}"
OutFile "${Installer}"

;Default installation folder
InstallDir "$LOCALAPPDATA\${GenericProduct}"

;Get installation folder from registry if available
InstallDirRegKey "${RegistryCat}" "${RegistryKey}" "${GenericProduct} Install Path"

;Request application privileges for Windows Vista
RequestExecutionLevel user

;Best compression
SetCompressor LZMA

; ???
AllowSkipFiles on

;--------------------------------
;Interface Settings

!define MUI_ICON "${SrcDir}\${GenericProduct}.ico"
!define MUI_UNICON "${SrcDir}\${GenericProduct}.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "modern-header.bmp" ; optional
!define MUI_WELCOMEFINISHPAGE_BITMAP "modern-wizard.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "modern-wizard.bmp"
!define MUI_ABORTWARNING

;Show all languages, despite user's codepage
!define MUI_LANGDLL_ALLLANGUAGES

;--------------------------------
;Language Selection Dialog Settings

;Remember the installer language
!define MUI_LANGDLL_REGISTRY_ROOT "${RegistryCat}"
!define MUI_LANGDLL_REGISTRY_KEY "${RegistryKey}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Language"

;--------------------------------
;Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\${Executable}"

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

!insertmacro MUI_LANGUAGE "English" ;first language is the default language
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"

;--------------------------------
;Reserve Files

;If you are using solid compression, files that are required before
;the actual installation should be stored first in the data block,
;because this will make your installer start faster.

!insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
;Installer Sections

; ----------------------------------------
; Default section
Section
  SetOutPath "$INSTDIR"

  ;Client, configuration and misc files
  File "${SrcDir}\${Executable}"
  File "${SrcDir}\msvcp100.dll"
  File "${SrcDir}\msvcr100.dll"

  ;Shortcut on desktop
  CreateShortCut "$DESKTOP\${Product}.lnk" "$INSTDIR\${Executable}"

  ;Store installation folder
  WriteRegStr "${RegistryCat}" "${RegistryKey}" "${GenericProduct} Install Path" $INSTDIR
SectionEnd

;--------------------------------
;Installer Functions

Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd
