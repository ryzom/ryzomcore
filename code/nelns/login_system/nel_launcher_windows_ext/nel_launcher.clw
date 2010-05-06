; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CBarTabsWnd
LastTemplate=generic CWnd
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "nel_launcher.h"

ClassCount=12
Class1=CNel_launcherApp
Class2=CNel_launcherDlg

ResourceCount=8
Resource2=IDD_NEL_LAUNCHER_DIALOG (English (U.S.))
Resource1=IDR_MAINFRAME
Class3=CProgressWnd
Resource3=IDD_PROGRESS
Class4=CProgressDlg
Resource4=IDD_MSG
Class5=CWebDlg
Class6=CBarWnd
Class7=CTabsWnd
Class8=CTabsDlg
Resource5=IDD_LOADING_PAGE
Resource6=IDD_WEB
Class9=CLoadingPageDlg
Resource7=IDD_NEL_LAUNCHER_DIALOG
Class10=CMsgDlg
Class11=CLoginDlg
Class12=CBarTabsWnd
Resource8=IDD_LOGIN

[CLS:CNel_launcherApp]
Type=0
HeaderFile=nel_launcher.h
ImplementationFile=nel_launcher.cpp
Filter=N
BaseClass=CWinApp
VirtualFilter=AC

[CLS:CNel_launcherDlg]
Type=0
HeaderFile=nel_launcherDlg.h
ImplementationFile=nel_launcherDlg.cpp
Filter=W
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_MINIMIZE



[DLG:IDD_NEL_LAUNCHER_DIALOG]
Type=1
Class=CNel_launcherDlg
ControlCount=0

[DLG:IDD_NEL_LAUNCHER_DIALOG (English (U.S.))]
Type=1
Class=CNel_launcherDlg
ControlCount=1
Control1=IDC_EXPLORER1,{8856F961-340A-11D0-A96B-00C04FD705A2},1342242816

[CLS:CProgressWnd]
Type=0
HeaderFile=ProgressWnd.h
ImplementationFile=ProgressWnd.cpp
BaseClass=CWnd
Filter=W
VirtualFilter=WC

[DLG:IDD_PROGRESS]
Type=1
Class=CProgressDlg
ControlCount=1
Control1=IDC_MSG_PROGRESS_STATIC,static,1342308364

[CLS:CProgressDlg]
Type=0
HeaderFile=ProgressDlg.h
ImplementationFile=ProgressDlg.cpp
BaseClass=CDialog
Filter=W
LastObject=CProgressDlg
VirtualFilter=dWC

[DLG:IDD_WEB]
Type=1
Class=CWebDlg
ControlCount=1
Control1=IDC_EXPLORER1,{8856F961-340A-11D0-A96B-00C04FD705A2},1342242816

[CLS:CWebDlg]
Type=0
HeaderFile=WebDlg.h
ImplementationFile=WebDlg.cpp
BaseClass=CDialog
Filter=W
VirtualFilter=dWC

[CLS:CBarWnd]
Type=0
HeaderFile=BarWnd.h
ImplementationFile=BarWnd.cpp
BaseClass=CWnd
Filter=W
VirtualFilter=WC

[CLS:CTabsWnd]
Type=0
HeaderFile=TabsWnd.h
ImplementationFile=TabsWnd.cpp
BaseClass=generic CWnd
Filter=W

[CLS:CTabsDlg]
Type=0
HeaderFile=TabsDlg.h
ImplementationFile=TabsDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC

[DLG:IDD_LOADING_PAGE]
Type=1
Class=CLoadingPageDlg
ControlCount=0

[CLS:CLoadingPageDlg]
Type=0
HeaderFile=LoadingPageDlg.h
ImplementationFile=LoadingPageDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC

[DLG:IDD_MSG]
Type=1
Class=CMsgDlg
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDC_MSG_STATIC,static,1342308352
Control3=IDC_TITLE_STATIC,static,1342308352

[CLS:CMsgDlg]
Type=0
HeaderFile=MsgDlg.h
ImplementationFile=MsgDlg.cpp
BaseClass=CDialog
Filter=W
VirtualFilter=dWC

[DLG:IDD_LOGIN]
Type=1
Class=CLoginDlg
ControlCount=9
Control1=IDC_LOGIND_STATIC,static,1342177294
Control2=IDC_ERR_STATIC,static,1342308353
Control3=IDC_LOGIN_STATIC,static,1342308354
Control4=IDC_PWD_STATIC,static,1342308354
Control5=IDC_LOGIN_EDIT,edit,1350631552
Control6=IDC_PWD_EDIT,edit,1350631584
Control7=IDC_LOGINU_STATIC,static,1342177294
Control8=IDC_QUITD_STATIC,static,1342177294
Control9=IDC_QUITU_STATIC,static,1342177294

[CLS:CLoginDlg]
Type=0
HeaderFile=LoginDlg.h
ImplementationFile=LoginDlg.cpp
BaseClass=CDialog
Filter=W
VirtualFilter=dWC

[CLS:CBarTabsWnd]
Type=0
HeaderFile=BarTabsWnd.h
ImplementationFile=BarTabsWnd.cpp
BaseClass=CWnd
Filter=W
VirtualFilter=WC

