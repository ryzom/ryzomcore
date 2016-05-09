;--------------------------------
;Include Modern UI
!include "MUI2.nsh"

;--------------------------------
;General
!define Company "Nevrax"
!define Product "Ryzom"
!define Version "2.1.0"
!define Patch "661"
!define RegistryCat "HKLM"
!define RegistryKey "Software\${Company}\${Product}"
!define Executable "client_ryzom_rd.exe"
!define DstDir "Output"
!define SrcDir "Ryzom"
!define Installer "ryzom_setup_${Patch}.exe"

;Registry key for uninstaller
!define UninstallRegistryRoot "Software\Microsoft\Windows\CurrentVersion\Uninstall"
!define UninstallRegistryKey "${UninstallRegistryRoot}\${Product}"

;Properly display all languages (Installer will not work on Windows 95, 98 or ME!)
Unicode true

;New XP style
XPStyle on

;Name and file
Name "${Product}"
OutFile "${DstDir}\${Installer}"

;Default installation folder
InstallDir "$PROGRAMFILES\${Product}"

;Get installation folder from registry if available
InstallDirRegKey "${RegistryCat}" "${RegistryKey}" "${Product} Install Path"

;Request application privileges for Windows Vista
RequestExecutionLevel admin

;Best compression
SetCompressor LZMA

; ???
AllowSkipFiles on

;--------------------------------
;Variables

;Will be used later
Var MUI_TEMP
Var STARTMENU_FOLDER

;--------------------------------
;Interface Settings

!define MUI_ICON "${SrcDir}\ryzom.ico"
!define MUI_UNICON "${SrcDir}\ryzom.ico"
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

;Start Menu Folder Page Configuration
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${RegistryCat}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${RegistryKey}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

!insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\client_ryzom_rd.exe"

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
;!insertmacro MUI_LANGUAGE "Spanish"

;--------------------------------
;Reserve Files

;If you are using solid compression, files that are required before
;the actual installation should be stored first in the data block,
;because this will make your installer start faster.

!insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
;Descriptions

;French
LangString MSG_SUPPORT_URL_TITLE       ${LANG_FRENCH}  "Support"
LangString MSG_SUPPORT_URL             ${LANG_FRENCH}  "http://app.ryzom.com/app_forum/index.php?page=topic/view/22047/1&post149889=fr#1"
LangString MSG_HOME_URL_TITLE          ${LANG_FRENCH}  "${Product} sur le Web"
LangString MSG_HOME_URL                ${LANG_FRENCH}  "http://ryzom.fr"
LangString MSG_CONFIRM_DELETE_BACKUPS  ${LANG_FRENCH}  "Voulez-vous supprimer vos fichiers de sauvegarde ? Appuyer sur Oui pour effacer vos fichiers de sauvegarde."

;English
LangString MSG_SUPPORT_URL_TITLE       ${LANG_ENGLISH} "Support"
LangString MSG_SUPPORT_URL             ${LANG_ENGLISH} "http://app.ryzom.com/app_forum/index.php?page=topic/view/22047/1&post149889=en#1"
LangString MSG_HOME_URL_TITLE          ${LANG_ENGLISH} "${Product} on the Web"
LangString MSG_HOME_URL                ${LANG_ENGLISH} "http://ryzom.com"
LangString MSG_CONFIRM_DELETE_BACKUPS  ${LANG_ENGLISH} "Deleting Save Files? Click Yes to delete your own save files."

;German
LangString MSG_SUPPORT_URL_TITLE       ${LANG_GERMAN}  "Support"
LangString MSG_SUPPORT_URL             ${LANG_GERMAN}  "http://app.ryzom.com/app_forum/index.php?page=topic/view/22047/1&post149889=de#1"
LangString MSG_HOME_URL_TITLE          ${LANG_GERMAN}  "${Product} im Internet"
LangString MSG_HOME_URL                ${LANG_GERMAN}  "http://ryzom.de"
LangString MSG_CONFIRM_DELETE_BACKUPS  ${LANG_GERMAN}  "Gespeicherte Dateien löschen? Klicke Ja um die eigenen Dateien zu löschen."

;--------------------------------
;Installer Sections

!macro CreateInternetShortcut FILENAME URL ICONFILE
  WriteINIStr "${FILENAME}.url" "InternetShortcut" "URL" "${URL}"
  WriteINIStr "${FILENAME}.url" "InternetShortcut" "IconFile" "${ICONFILE}"
  WriteINIStr "${FILENAME}.url" "InternetShortcut" "IconIndex" "0"
!macroend

; ----------------------------------------
; Default section
Section
  ;Install for All Users
  SetShellVarContext all

  ;Game directories
  SetOutPath "$INSTDIR\cfg"
  File /r "${SrcDir}\cfg\*.*"

  SetOutPath "$INSTDIR\data"
  File /r "${SrcDir}\data\*.*"

  SetOutPath "$INSTDIR\examples"
  File /r "${SrcDir}\examples\*.*"

  SetOutPath "$INSTDIR\user"
  File /r "${SrcDir}\user\*.*"

  SetOutPath "$INSTDIR"

  ;Client, configuration and misc files
  File "${SrcDir}\client_default.cfg"
  File "${SrcDir}\client_ryzom_rd.exe"
  File "${SrcDir}\configure.bat"
  File "${SrcDir}\d3dcompiler_43.dll"
  File "${SrcDir}\d3dx9_43.dll"
  File "${SrcDir}\fmod.dll"
  File "${SrcDir}\forum.url"
  File "${SrcDir}\launch.bat"
  File "${SrcDir}\msvcp100.dll"
  File "${SrcDir}\msvcr100.dll"
  File "${SrcDir}\nel_drv_direct3d_win_r.dll"
  File "${SrcDir}\nel_drv_fmod_win_r.dll"
  File "${SrcDir}\nel_drv_opengl_win_r.dll"
  File "${SrcDir}\ryzom.ico"
  File "${SrcDir}\ryzom.url"
  File "${SrcDir}\Ryzom6.ico"

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

  ;Create shortcuts in Start Menu
  CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\${Product}.lnk" "$INSTDIR\${Executable}"

  ;Create URLs
  !insertmacro CreateInternetShortcut "$SMPROGRAMS\$STARTMENU_FOLDER\$(MSG_SUPPORT_URL_TITLE)" "$(MSG_SUPPORT_URL)" "$INSTDIR\ryzom.ico"
  !insertmacro CreateInternetShortcut "$SMPROGRAMS\$STARTMENU_FOLDER\$(MSG_HOME_URL_TITLE)" "$(MSG_HOME_URL)" "$INSTDIR\ryzom.ico"

  !insertmacro MUI_STARTMENU_WRITE_END

  ;Shortcut on desktop
  CreateShortCut "$DESKTOP\${Product}.lnk" "$INSTDIR\${Executable}"

  ;Add/Remove Program entry
  WriteRegStr HKLM "${UninstallRegistryKey}" "DisplayIcon" "$INSTDIR\${Executable},0"
  WriteRegStr HKLM "${UninstallRegistryKey}" "DisplayName" "${Product}"
  WriteRegStr HKLM "${UninstallRegistryKey}" "DisplayVersion" "${Version}"
  WriteRegStr HKLM "${UninstallRegistryKey}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${UninstallRegistryKey}" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
  WriteRegStr HKLM "${UninstallRegistryKey}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "${UninstallRegistryKey}" "Publisher" "${Company}"
  WriteRegStr HKLM "${UninstallRegistryKey}" "HelpLink" "$(MSG_SUPPORT_URL)"
  WriteRegStr HKLM "${UninstallRegistryKey}" "URLInfoAbout" "$(MSG_HOME_URL)"

  ;Store installation folder
  WriteRegStr "${RegistryCat}" "${RegistryKey}" "${Product} Install Path" $INSTDIR
SectionEnd

;--------------------------------
;Uninstaller Section
Section "Uninstall"
  ;Install for All Users
  SetShellVarContext all

  ;Game directories
  RMDir /r "$INSTDIR\cfg"
  RMDir /r "$INSTDIR\data"
  RMDir /r "$INSTDIR\examples"

  ;Temporary directories
  RMDir /r "$INSTDIR\cache"
  RMDir /r "$INSTDIR\unpack"

  ;Client, configuration and misc files
  Delete "$INSTDIR\client_default.cfg"
  Delete "$INSTDIR\client_ryzom_rd.exe"
  Delete "$INSTDIR\configure.bat"
  Delete "$INSTDIR\d3dcompiler_43.dll"
  Delete "$INSTDIR\d3dx9_43.dll"
  Delete "$INSTDIR\fmod.dll"
  Delete "$INSTDIR\forum.url"
  Delete "$INSTDIR\launch.bat"
  Delete "$INSTDIR\msvcp100.dll"
  Delete "$INSTDIR\msvcr100.dll"
  Delete "$INSTDIR\nel_drv_direct3d_win_r.dll"
  Delete "$INSTDIR\nel_drv_fmod_win_r.dll"
  Delete "$INSTDIR\nel_drv_opengl_win_r.dll"
  Delete "$INSTDIR\ryzom.ico"
  Delete "$INSTDIR\ryzom.url"
  Delete "$INSTDIR\Ryzom6.ico"

  ;Delete uninstaller
  Delete "$INSTDIR\Uninstall.exe"

  MessageBox MB_YESNO "$(MSG_CONFIRM_DELETE_BACKUPS)" IDNO DontDeleteMiscFiles

  ;Delete directory
  RMDir /r "$INSTDIR"

DontDeleteMiscFiles:

  ;Delete directory if empty
  RMDir "$INSTDIR"

  ;Delete all shortcuts
  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

  ;Delete Start Menu shortcuts
  Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\${Product}.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\$(MSG_HOME_URL_TITLE).url"
  Delete "$SMPROGRAMS\$MUI_TEMP\$(MSG_SUPPORT_URL_TITLE).url"
  RMDir /r /REBOOTOK "$SMPROGRAMS\$MUI_TEMP"

  ;Delete desktop shortcut
  Delete "$DESKTOP\${Product}.lnk"

  ;Delete registry
  DeleteRegKey /ifempty "${RegistryCat}" "${RegistryKey}"
  DeleteRegKey "${RegistryCat}" "${UninstallRegistryKey}"
SectionEnd

;--------------------------------
;Installer Functions

Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

;--------------------------------
;Uninstaller Functions

Function un.onInit
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd
