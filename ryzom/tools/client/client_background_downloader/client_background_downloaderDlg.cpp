	/*
 * $Id:
 */

// client_background_downloaderDlg.cpp : implementation file
//

#include "stdpch.h"
#include "client_background_downloader.h"
#include "client_background_downloaderDlg.h"
#include "CloseWarning.h"
#include "game_share/bg_downloader_msg.h"
#include "../client/login_patch.h"
//
#include "nel/misc/i18n.h"
#include "nel/misc/config_file.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/shared_memory.h"
#include "nel/misc/win_thread.h"
#include "nel/misc/mutex.h"
#include "nel/misc/co_task.h"
#include "nel/misc/big_file.h"


using namespace NLMISC;
using namespace BGDownloader;



//**************************************************************************************************
// download coroutine
class CDownloadTask : public NLMISC::CCoTask
{
public:
	CClient_background_downloaderDlg *Parent;
	CPatchManager::SPatchInfo		 InfoOnPatch;
	uint32							 TotalPatchSize;
public:
	CDownloadTask();
	// from NLMISC::CCoTask
	virtual void run();		
	void goIdle();
	bool waitTaskCompletion(bool &ok);	
	void doTask(const CTaskDesc &newTask);
	// compute total patch size from current 'InfoOnPatch'
	void updatePatchSize();	
};

//**************************************************************************************************
CDownloadTask::CDownloadTask()
{
	TotalPatchSize = 0;
}

//**************************************************************************************************
void CDownloadTask::updatePatchSize()
{	
	TotalPatchSize = CClient_background_downloaderApp::computePatchSize(InfoOnPatch, (Parent->_Task.DownloadBitfield & (1 << DownloadID_MainLand)) != 0); // 'optionnal category' will be the flag to indicate 'mainland'	
}

//**************************************************************************************************
void CDownloadTask::goIdle()
{
	Parent->_Task.State = DLState_Idle;
	Parent->_WantedTask.State = DLState_Idle;
}

//**************************************************************************************************
void CDownloadTask::run()
{
	CPatchManager *pPM = CPatchManager::getInstance();		
	while (!isTerminationRequested())
	{
		if (Parent->_WantedTask.State != DLState_Idle)
		{
			doTask(Parent->_WantedTask);
		}
		yield();
	}
}

//**************************************************************************************************
bool CDownloadTask::waitTaskCompletion(bool &ok)
{	
	CPatchManager *pPM = CPatchManager::getInstance();
	for (;;)
	{		
		bool finished;	
		switch (Parent->_Task.State)
		{
			case DLState_CheckPatch: 
				finished = pPM->isCheckThreadEnded(ok); 
				// special case : is client is down, continuing the check would be wasteful, stop right now
				if (!Parent->_ClientMsgQueue.connected())
				{
					PostQuitMessage(0);
					return false;
				}				
			break;
			case DLState_GetAndApplyPatch:
			case DLState_GetPatch: 
				finished = pPM->isPatchThreadEnded(ok);
			break;
			default:
				nlassert(0);
			break;

		}		
		if (finished) break;
		if (Parent->_StopWanted || isTerminationRequested() || Parent->_BypassWanted)
		{
			switch (Parent->_Task.State)
			{
				case DLState_CheckPatch: 
					pPM->forceStopCheckThread();
				break;
				case DLState_GetAndApplyPatch:
				case DLState_GetPatch:
					pPM->forceStopPatchThread();					
				break;
				default:
					nlassert(0);
				break;

			}			
		}
		else
		{
			Parent->updateTaskProgressDisplay();
		}
		yield();
	}

	if (isTerminationRequested() || Parent->_StopWanted)
	{
		Parent->_StopWanted = false;
		Parent->_LastTaskResult = TaskResult_Interrupted;
		Parent->_AvailablePatchs = 0;
		goIdle();
		return false;
	}
	return true;
}


//**************************************************************************************************
void CDownloadTask::doTask(const CTaskDesc &newTask)
{	
	CPatchManager *pPM = CPatchManager::getInstance();
	bool taskOK;
	switch(newTask.State)
	{		
		case DLState_CheckPatch:
		{			
			nlwarning("CPatchManager::getInstance()->init BEGIN");
			CPatchManager::getInstance()->init(theApp.PatchURIs, theApp.ServerPath, theApp.ServerVersion);
			nlwarning("CPatchManager::getInstance()->init END");
			TotalPatchSize = 0;
			InfoOnPatch.clear();
			Parent->_Task = newTask;
			Parent->_ThreadPriority = ThreadPriority_Unknown;
			nlwarning("Start check thread");
			pPM->startCheckThread(true);
			Parent->updateThreadPriority();
			bool completed = waitTaskCompletion(taskOK);			
			if (completed || Parent->_BypassWanted)
			{						
				Parent->_BypassWanted = false;				
				if (taskOK)
				{	
					nlwarning("Check thread succeeded");
					pPM->getInfoToDisp(InfoOnPatch);

										

					nlwarning("Non optionnal categories : ");
					nlwarning("================");

					for (uint k = 0; k < InfoOnPatch.NonOptCat.size(); ++k)
					{
						nlwarning(InfoOnPatch.NonOptCat[k].Name.c_str());
					}

					nlwarning("Required categories : ");
					nlwarning("================");

					for (uint k = 0; k < InfoOnPatch.ReqCat.size(); ++k)
					{
						nlwarning(InfoOnPatch.ReqCat[k].Name.c_str());
					}

					nlwarning("Optionnal categories : ");
					nlwarning("================");

					for (uint k = 0; k < InfoOnPatch.OptCat.size(); ++k)
					{
						nlwarning(InfoOnPatch.OptCat[k].Name.c_str());
					}

					Parent->_AvailablePatchs = InfoOnPatch.getAvailablePatchsBitfield();					
						
					Parent->_LastTaskResult = TaskResult_Success;					
					Parent->setProgress(FALSE, 0);
					Parent->setSuccessBitmap();
					//if (!Parent->_AvailablePatchs)
					{
						Parent->popBalloon(CI18N::get("uiBGD_CheckSuccess"), CI18N::get("uiBGD_RBG"));
					}
					Parent->setLocalStatusString(CI18N::get("uiBGD_CheckSuccess"));
														
					if (Parent->_Mode == DownloaderMode_Autonomous)
					{			
						goIdle(); // not handled for now
						/*
						// in autonomous mode, choose the next task to do
						if (Parent->_LastTaskResult == TaskResult_PatchNeeded)
						{
							// get the corresponding patch (without commit)
							Parent->_WantedTask.State = DLState_GetPatch;
						}
						else
						{
							// check the next category, or stop
							if (Parent->_WantedTask.ID == DownloadID_Count - 1)
							{
								goIdle();
							}
							else
							{
								Parent->_WantedTask.ID = (TDownloadID) (Parent->_WantedTask.ID + 1);
							}
						}
						*/
					}
					else
					{
						goIdle(); // just wait next task from client
					}
				}
				else
				{					
					nlwarning("Check thread failed");
					Parent->_LastTaskResult = TaskResult_Error;
					Parent->_AvailablePatchs = 0;
					if (!pPM->getLastErrorMessage().empty())
					{
						Parent->errorMessage(pPM->getLastErrorMessage());
					}
					else
					{
						Parent->errorMessage(CI18N::get("uiBGD_CheckFailed"));
					}
					goIdle();					
				}
			}
		}
		break;
		case DLState_GetAndApplyPatch:
		case DLState_GetPatch:
		{	
			if (newTask.DownloadBitfield & (1 << DownloadID_MainLand))
			{
				Parent->popBalloon(CI18N::get("uiBGD_BackgroundPatchStarting"), CI18N::get("uiBGD_RBG"));
			}
			else
			{
				Parent->popBalloon(CI18N::get("uiBGD_PatchStarting"), CI18N::get("uiBGD_RBG"));
			}
			Parent->_Task = newTask;
			updatePatchSize();
			std::vector<std::string> vCategories;
			// add 'optionnal' part for the mainland patch only
			if (newTask.DownloadBitfield & (1 << DownloadID_MainLand))
			{
				for(uint i = 0; i < InfoOnPatch.OptCat.size(); i++)
				{							
					vCategories.push_back(InfoOnPatch.OptCat[i].Name);
				}
			}
			// nb : for now, assume that when patching mainland, the part for R.o.S should have been patched, so we are 
			// patching BNP that are *NOT* opened yet
			Parent->_ThreadPriority = ThreadPriority_Unknown;
			pPM->startPatchThread(vCategories, (newTask.State == DLState_GetAndApplyPatch)); // don't commit the patch (but keep xdelta'ed version)			
			Parent->updateThreadPriority();

			bool completed = waitTaskCompletion(taskOK);			

			if (completed || Parent->_BypassWanted)
			{			
				Parent->_BypassWanted = false;
				if (taskOK)
				{						
					Parent->setProgress(FALSE, 0);
					Parent->setSuccessBitmap();
					Parent->setLocalStatusString(CI18N::get("uiBGD_PatchComplete"));
					Parent->_LastTaskResult = TaskResult_Success;
					Parent->_AvailablePatchs = 0;
					Parent->popBalloon(CI18N::get("uiBGD_PatchComplete"), CI18N::get("uiBGD_RBG"));
					//
					if (Parent->_Mode == DownloaderMode_Autonomous)
					{		
						goIdle(); // not handled for now
						/*
						// All category patched ?
						if (Parent->_WantedTask.ID == DownloadID_Count - 1)
						{
							goIdle();
						}
						else
						{
							Parent->_WantedTask.ID = (TDownloadID) (Parent->_WantedTask.ID + 1);
							Parent->_WantedTask.State = DLState_CheckPatch;
						}
						*/
					}
					else
					{						
						goIdle(); // just wait next task from client
					}
				}
				else
				{					
					Parent->_LastTaskResult = TaskResult_Error;
					Parent->_AvailablePatchs = 0;
					if (!pPM->getLastErrorMessage().empty())
					{
						Parent->errorMessage(pPM->getLastErrorMessage());
					}
					else
					{
						Parent->errorMessage(CI18N::get("uiBGD_PatchingFailed"));
					}
					goIdle();					
				}
			}
		}
		break;		
	};
}


/////////////////////////////////////////////////////////////////////////////
// CClient_background_downloaderDlg dialog

CClient_background_downloaderDlg::CClient_background_downloaderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClient_background_downloaderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClient_background_downloaderDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	_TrayIconAdded = FALSE;
	m_hIcon = AfxGetApp()->LoadIcon(IDI_MAIN_ICON);	
	m_hTrayIcon = AfxGetApp()->LoadIcon(IDI_TRAY_ICON);
	_WaitSingleClick	= false;	
	_BypassWanted		= false;		
	_Mode				= DownloaderMode_Unknown;
	_LastTaskResult		= TaskResult_Unknown;
	_StopWanted			= false;
	_Verbose			= false;
	_DownloadTask			= new CDownloadTask;	
	_DownloadTask->Parent	= this;
	_AvailablePatchs = 0;
	_ThreadPriority		  = ThreadPriority_Normal;
	_WantedThreadPriority = ThreadPriority_Normal;
	OSVERSIONINFO os = { sizeof(os) };
    GetVersionEx(&os);
	_IsWin2KOrMore = VER_PLATFORM_WIN32_NT == os.dwPlatformId && os.dwMajorVersion >= 5;		
}

//**************************************************************************************************
CClient_background_downloaderDlg::~CClient_background_downloaderDlg()
{	
	/*_DownloadTask->requestTerminate();
	while (!_DownloadTask->isFinished())
	{
		_DownloadTask->resume();
	}*/
}

void CClient_background_downloaderDlg::updateThreadPriority()
{	
	if (_WantedThreadPriority == _ThreadPriority) return;
	BGDownloader::TThreadPriority threadPriority = _WantedThreadPriority;
	CPatchManager *pPM = CPatchManager::getInstance();
	if (pPM->isDownloadInProgress() && (_WantedThreadPriority == ThreadPriority_Paused))
	{
		// Is a download is in progress, do not really pause, because we may loose the connection
		// The real pause is only useful for 'Apply Patch' phases, that are cpu & disk intensive
		threadPriority = ThreadPriority_Low;
	}
	NLMISC::CWinThread *winThread = dynamic_cast<NLMISC::CWinThread *>(pPM->getCurrThread());
	if (winThread)
	{		
		switch(threadPriority)
		{
			case ThreadPriority_Paused:
				winThread->suspend();
				winThread->enablePriorityBoost(false);
				winThread->setPriority(THREAD_PRIORITY_IDLE);
			break;
			case ThreadPriority_Low:
				winThread->resume();
				winThread->enablePriorityBoost(false);
				winThread->setPriority(THREAD_PRIORITY_BELOW_NORMAL);		
			break;
			case ThreadPriority_Normal:
				winThread->resume();
				winThread->enablePriorityBoost(true);
				winThread->setPriority(THREAD_PRIORITY_NORMAL);						
			break;
			default:
				nlassert(0);
			break;
		}
		_ThreadPriority = _WantedThreadPriority;
	}
}


//**************************************************************************************************
void CClient_background_downloaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClient_background_downloaderDlg)
	DDX_Control(pDX, IDC_STATUS_BITMAP, m_StatusBitmap);
	DDX_Control(pDX, IDC_THREAD_PRIORITY, m_ThreadPriorityCtrl);
	//}}AFX_DATA_MAP
}

const UINT RZBD_TRAY_NOTIFY = WM_APP;

BEGIN_MESSAGE_MAP(CClient_background_downloaderDlg, CDialog)
	//{{AFX_MSG_MAP(CClient_background_downloaderDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_MESSAGE(RZBD_TRAY_NOTIFY, OnTrayIconMsg)
	ON_COMMAND(IDM_SHOW_HIDE, OnMenuShowHide)
	ON_COMMAND(IDM_DOWNLOADER_EXIT, OnMenuExit)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BYPASS, OnBypass)
	ON_CBN_SELCHANGE(IDC_THREAD_PRIORITY, OnSelchangeThreadPriority)
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//**************************************************************************************************
void CClient_background_downloaderDlg::OnMenuShowHide()
{
	if (!IsWindowVisible())
	{
		ShowWindow(SW_RESTORE);
		SetForegroundWindow();		
	}
	else
	{
		this->ShowWindow(SW_HIDE);
	}	
}

//**************************************************************************************************
void CClient_background_downloaderDlg::OnMenuExit()
{
	PostQuitMessage(0);
}


//**************************************************************************************************
LRESULT CClient_background_downloaderDlg::OnTrayIconMsg(WPARAM wParam,LPARAM lParam)
{
	switch(lParam)
	{
		case WM_LBUTTONDBLCLK:
			_WaitSingleClick = false;
			ShowWindow(SW_RESTORE);
			SetForegroundWindow();			
		break;
		case WM_LBUTTONDOWN:
			_WaitSingleClick = true;
			_WaitSingleClickStartTime = NLMISC::CTime::getLocalTime();
		break;
		case WM_RBUTTONDOWN:		
			_WaitSingleClick = false;
			popTrayMenu();
		break;				
	}

	return S_OK;
}

//**************************************************************************************************
void CClient_background_downloaderDlg::popTrayMenu()
{
	CMenu mnu;
	mnu.LoadMenu(IDR_TRAY_MENU);			
	CMenu *popMenu = mnu.GetSubMenu(0);
	if (popMenu)
	{		
		SetForegroundWindow();
		CPoint pt;
		GetCursorPos(&pt);
		popMenu->RemoveMenu(0, MF_BYPOSITION);
		popMenu->RemoveMenu(0, MF_BYPOSITION);
		popMenu->InsertMenu(0, MF_BYPOSITION, IDM_DOWNLOADER_EXIT, CI18N::get("uiBGD_Exit").toString().c_str());
		popMenu->InsertMenu(0, MF_BYPOSITION, IDM_SHOW_HIDE, CI18N::get("uiBGD_ShowHide").toString().c_str());
		popMenu->TrackPopupMenu(TPM_RIGHTALIGN,pt.x,pt.y,this);
	}
}



/////////////////////////////////////////////////////////////////////////////
// CClient_background_downloaderDlg message handlers


//**************************************************************************************************
void CClient_background_downloaderDlg::initSearchPaths()
{
	CPath::clearMap();
	CPath::addSearchPath ("data", true, false);
}

//**************************************************************************************************
BOOL CClient_background_downloaderDlg::OnInitDialog()
{	
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hTrayIcon, FALSE);		// Set small icon	

	theApp.loadConfigFile(this->m_hWnd, "Ryzom Background Downloader");	

	// tmp : for patch debug
	extern void tmpFlagMainlandPatchCategories(NLMISC::CConfigFile &cf);
	extern void tmpFlagRemovedPatchCategories(NLMISC::CConfigFile &cf);
	tmpFlagMainlandPatchCategories(theApp.ConfigFile);	
	tmpFlagRemovedPatchCategories(theApp.ConfigFile);



	

	bool testBGDownloader = false;

	CConfigFile::CVar *testBGDownloaderVarPtr = theApp.ConfigFile.getVarPtr("TestBackgroundDownloader");
	if (testBGDownloaderVarPtr)
	{
		testBGDownloader = testBGDownloaderVarPtr->asInt() != 0;
	}


	
	GetDlgItem(IDC_BYPASS)->ShowWindow(testBGDownloader ? SW_SHOW : SW_HIDE);

	//CPath::addSearchPath ("patch", true, false);
	initSearchPaths();	
	std::string language = theApp.getLanguage();
	if (language == "wk")
	{
		CPath::addSearchPath ("translation/work", true, false);
	}

	// load translation file
	CI18N::load(language);


	// init communication with client			
	if (!theApp.parseCommandLine((LPCTSTR) theApp.m_lpCmdLine))
	{
		ShowWindow(SW_HIDE);
		MessageBox(CI18N::get("uiBGD_InvalidCommandLine").toString().c_str(),
				   CI18N::get("uiBGD_RBG").toString().c_str(),
				   MB_ICONEXCLAMATION);
		PostQuitMessage(0);
	}

	// at this point, big file not needed anymore
	NLMISC::CBigFile::getInstance().removeAll();


	if (_IsWin2KOrMore)
	{
		ZeroMemory(&_NID, sizeof(_NID));	
		_NID.cbSize = sizeof(NOTIFYICONDATAW_V2);
		wcsncpy(_NID.szTip, (const wchar_t *) CI18N::get("uiBGD_RBG").c_str(), 63);
		// Add tray icon			
		_NID.hIcon = m_hTrayIcon;
		_NID.hWnd = m_hWnd;	
		_NID.uCallbackMessage = RZBD_TRAY_NOTIFY;
		_NID.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
		_NID.uID = 0;
	}
	else
	{
		ZeroMemory(&_NID_V1, sizeof(_NID_V1));	
		_NID_V1.cbSize = sizeof(NOTIFYICONDATAW);
		wcsncpy(_NID_V1.szTip, (const wchar_t *) CI18N::get("uiBGD_RBG").c_str(), 127);
		// Add tray icon			
		_NID_V1.hIcon = m_hTrayIcon;
		_NID_V1.hWnd = m_hWnd;	
		_NID_V1.uCallbackMessage = RZBD_TRAY_NOTIFY;
		_NID_V1.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
		_NID_V1.uID = 0;
	}

	

	_TrayIconAdded = Shell_NotifyIconW(NIM_ADD, (NOTIFYICONDATAW *) &_NID);
	if (!_TrayIconAdded)
	{
		nlwarning("Failed to add icon in tray.");
	}


	if (_IsWin2KOrMore)
	{
		_NID.uVersion = NOTIFYICON_VERSION;
		BOOL ok = Shell_NotifyIconW(NIM_SETVERSION, (NOTIFYICONDATAW *) &_NID);
		nlwarning("ok = %s", toString(ok).c_str());
	}
	
	SetTimer(TIMER_CHECK_DBL_CLICK, 10, NULL);
	SetTimer(TIMER_MAIN_LOOP, 30, NULL);
	SetTimer(TIMER_SEND_PRIORITY, 250, NULL);

	this->SetWindowText(CI18N::get("uiBGD_RBG").toString().c_str());

	
	// populate the thread combo box
	m_ThreadPriorityCtrl.ResetContent();
	m_ThreadPriorityCtrl.AddString(CI18N::get("uiBGD_Paused").toString().c_str());
	m_ThreadPriorityCtrl.AddString(CI18N::get("uiBGD_LowPriority").toString().c_str());
	m_ThreadPriorityCtrl.AddString(CI18N::get("uiBGD_NormalPriority").toString().c_str());

	m_ThreadPriorityCtrl.EnableWindow(TRUE);
	m_ThreadPriorityCtrl.SetCurSel(2);	

	m_StatusBitmap.ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATUS_STRING)->ShowWindow(SW_HIDE);
	
	
	// init communication with client
	_ClientMsgQueue.init(theApp.m_hInstance, BGDownloader::DownloaderWndID, BGDownloader::ClientWndID);

	_DownloadTask->resume();


	GetDlgItem(IDC_STATUS_STRING)->SetFocus();	

	return FALSE;  // return TRUE  unless you set the focus to a control
}

//**************************************************************************************************
void CClient_background_downloaderDlg::popBalloon(const ucstring &text, const ucstring &title, DWORD balloonIcon /*= NIIF_INFO*/) 
{	
	if (!_IsWin2KOrMore) return;    	
	// remove a previous balloon	
	Shell_NotifyIconW(NIM_DELETE, (NOTIFYICONDATAW *) &_NID);
	_TrayIconAdded = Shell_NotifyIconW(NIM_ADD, (NOTIFYICONDATAW *) &_NID);
	if (_TrayIconAdded)
	{
		wcsncpy(_NID.szInfo, (const wchar_t *) text.c_str(), 255);	
		_NID.uFlags |= NIF_INFO;
		wcsncpy(_NID.szInfoTitle, (const wchar_t *) title.c_str(), 63);    
		_NID.uTimeout    = 10000; 
		_NID.dwInfoFlags = balloonIcon;
		Shell_NotifyIconW(NIM_MODIFY, (NOTIFYICONDATAW *) &_NID);		
		_NID.uFlags &= ~NIF_INFO;
		_NID.szInfo[0] = 0;
		_NID.szInfoTitle[0] = 0;
		_NID.uTimeout    = 0;
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

//**************************************************************************************************
void CClient_background_downloaderDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		::CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system call this to obtain the cursor to display while the user drags
//  the minimized window.
//**************************************************************************************************
HCURSOR CClient_background_downloaderDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//**************************************************************************************************
void CClient_background_downloaderDlg::removeTrayIcon() 
{
	if (_TrayIconAdded)
	{
		BOOL trayIconRemoved = Shell_NotifyIconW(NIM_DELETE, (NOTIFYICONDATAW *) &_NID);	
		if (!trayIconRemoved)
		{
			nlwarning("Couldn't remove tray icon");
		}
		_TrayIconAdded = false;
	}
}

//**************************************************************************************************
void CClient_background_downloaderDlg::OnDestroy() 
{
	_ClientMsgQueue.release();
	CDialog::OnDestroy();
	removeTrayIcon();
	#ifdef NL_DEBUG
		// test the case where the background downloader hang, to see if client handles 
		// it correctly
		CConfigFile::CVar *testBGDHung = theApp.ConfigFile.getVarPtr("TestBGDHung");
		if (testBGDHung && testBGDHung->asInt() != 0)
		{
			for (;;)
			{
				Sleep(100);
			}			
		}		
	#endif
}

//**************************************************************************************************
BOOL CClient_background_downloaderDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	
	
	return CDialog::OnNotify(wParam, lParam, pResult);
}

//**************************************************************************************************
void CClient_background_downloaderDlg::OnClose() 
{
	if (theApp.getCloseWarningFlag())
	{
		CCloseWarning cw;
		cw.DoModal();
	}
	this->ShowWindow(SW_HIDE);
	//CDialog::OnClose();
}

//**************************************************************************************************
void CClient_background_downloaderDlg::errorMessage(const ucstring &message)
{
	CMemStream outMsg(false /*output stream */);	
	TMsgType outMsgType;
	outMsgType = BGD_Error;
	outMsg.serialEnum(outMsgType);
	ucstring mutableMessage = message;
	outMsg.serial(mutableMessage);
	_ClientMsgQueue.sendMessage(outMsg);
	setProgress(FALSE, 0);
	setFailBitmap();	
	popBalloon(message.toString(), CI18N::get("uiBGD_RBG"));
	setLocalStatusString(message);
}


//**************************************************************************************************
void CClient_background_downloaderDlg::protocolError()
{
	errorMessage(CI18N::get("uiBGD_ProtocolError"));
	// this is fatal ...
	askStop();
}

//**************************************************************************************************
bool CClient_background_downloaderDlg::checkValidEnum(int value, int maxValue)
{
	if(value < 0 || value >= maxValue)
	{
		nlwarning("BG DOWNLOADER PROTOCOL : checkValidEnumFailed");
		protocolError();
		return false;
	}
	return true;
}

//**************************************************************************************************
void CClient_background_downloaderDlg::askStop()
{
	_StopWanted = true;
	_Mode = DownloaderMode_Slave;
}

//**************************************************************************************************
void CClient_background_downloaderDlg::handleClientMessage(NLMISC::CMemStream &inMsg) 
{
	CMemStream outMsg(false /*output stream */);
	TMsgType inMsgType = UnknownMessageType;
	TMsgType outMsgType;

	CPatchManager *pPM = CPatchManager::getInstance();		

	try
	{		
		inMsg.serialEnum(inMsgType);
		//nlwarning("##DOWNLOADER RCV message of type : %s", toString(inMsgType).c_str());
		switch(inMsgType)
		{
			case CL_Probe:				
				outMsgType = BGD_Ack;
				outMsg.serialEnum(outMsgType);
				outMsg.serialEnum(_Mode);				
				_ClientMsgQueue.sendMessage(outMsg);				
			break;
			case CL_GetMode:
				outMsgType = BGD_Mode;
				outMsg.serialEnum(outMsgType);
				outMsg.serialEnum(_Mode);				
				//nlwarning("##DOWNLOADER SENT message of type : %s", toString(outMsgType).c_str());
				_ClientMsgQueue.sendMessage(outMsg);
			break;
			case CL_GetState:	
				outMsgType = BGD_State;
				outMsg.serialEnum(outMsgType);
				outMsg.serial(_Task);
				//nlwarning("##DOWNLOADER SENT message of type : %s", toString(outMsgType).c_str());
				_ClientMsgQueue.sendMessage(outMsg);
			break;
			case CL_GetTaskResult:
			{
				outMsgType = BGD_TaskResult;
				outMsg.serialEnum(outMsgType);
				outMsg.serialEnum(_LastTaskResult);
				outMsg.serial(_AvailablePatchs);				
				bool mustLaunchBatFile = pPM->mustLaunchBatFile();
				outMsg.serial(mustLaunchBatFile);
				ucstring lastError = pPM->getLastErrorMessage();
				outMsg.serial(lastError);
				//nlwarning("##DOWNLOADER SENT message of type : %s", toString(outMsgType).c_str());
				_ClientMsgQueue.sendMessage(outMsg);
			}
			break;
			case CL_GetDescFile:
			{
				outMsgType = BGD_DescFile;
				outMsg.serialEnum(outMsgType);
				outMsg.serial(pPM->getDescFile());
				//nlwarning("##DOWNLOADER SENT message of type : %s", toString(outMsgType).c_str());
				_ClientMsgQueue.sendMessage(outMsg);
			}
			break;
			case CL_Stop:
				askStop();				
			break;
			case CL_StartTask:
			{
				// Starting a new task first require that the downloader 
				// be in an idle state. (so client must call stop, then wait until the state is idle, or this is an error case)
				if (_Task.State != DLState_Idle)
				{
					nlwarning("BG DOWNLOADER PROTOCOL : Starting task while not idle");
					protocolError();					
				}
				else
				{					
					CTaskDesc wantedTask;
					inMsg.serial(wantedTask);
					if (checkValidEnum(wantedTask.State, DLState_Count))						
					{
						_WantedTask = wantedTask;
						_Mode = DownloaderMode_Slave;
					}
				}
			}
			break;
			case CL_SetMode:
			{
				TDownloaderMode mode = DownloaderMode_Unknown;
				inMsg.serialEnum(mode);
				if (mode == _Mode) break;
				if (checkValidEnum(mode, DownloaderMode_Unknown))
				{
					if (mode == DownloaderMode_Autonomous)
					{
						if (_Task.State != DLState_Idle)
						{
							nlwarning("BG DOWNLOADER PROTOCOL : Starting task while not idle");
							protocolError();
							break;
						}
					}
					_Mode = mode;
					if (mode == DownloaderMode_Autonomous)
					{
						// not handled for now
						nlwarning("BG DOWNLOADER PROTOCOL : Trying to set autonomouse mode, not supported");
						protocolError();
						/*
						// when going to autonomous mode, take the initiative to start the first task
						_WantedTask.State = DLState_CheckPatch;
						//_WantedTask.ID = DownloadID_RoS;						
						*/
						//goIdle(); 
					}
				}
			}
			break;
			/*case CL_Reset:
				if (_Task.State != DLState_Idle)
				{
					protocolError();					
				}
				else
				{										
					std::string commandLine;
					inMsg.serial(commandLine);
					if (theApp.parseCommandLine((LPCTSTR) commandLine))
					{					
						CPatchManager::getInstance()->init(theApp.PatchURIs, theApp.ServerPath, theApp.ServerVersion);
						initSearchPaths();
					}
					else
					{
						protocolError();
					}
				}
			break;*/
			case CL_SetVerbose:
				inMsg.serial(_Verbose);
			break;
			case CL_Shutdown:
			{
				// remove icon from the tray right now
				removeTrayIcon();
				PostQuitMessage(0);
			}
			break;
			case CL_Show:
				ShowWindow(SW_RESTORE);
				SetForegroundWindow();
			break;
			case CL_Hide:
				ShowWindow(SW_HIDE);				
			break;
			case CL_SetPriority:
			{				
				bool freezeUI;
				TThreadPriority priority = ThreadPriority_Unknown;
				inMsg.serialEnum(priority);
				if (checkValidEnum(priority, ThreadPriority_Count))
				inMsg.serial(freezeUI);	
				setDownloadThreadPriority(priority);				
				m_ThreadPriorityCtrl.EnableWindow(freezeUI ? FALSE : TRUE);				
			}
			break;
		}
	}
	catch (EStream &e)
	{
		nlwarning(e.what());
		nlwarning("BG DOWNLOADER PROTOCOL : Stream error");
		protocolError();
	}
}

//**************************************************************************************************
void CClient_background_downloaderDlg::setDownloadThreadPriority(TThreadPriority priority)
{
	_WantedThreadPriority = priority;	
	m_ThreadPriorityCtrl.SetCurSel(priority);
}

//**************************************************************************************************
void CClient_background_downloaderDlg::OnTimer(UINT nIDEvent) 
{	
	if (_DownloadTask == CCoTask::getCurrentTask())
	{
		// Yes ! This may happen if a crash report is displayed from inside the patch task !
		// In this case, this timer will continue to be called 
		// _DownloadTask->resume() will be called again causing a crash (because no yield actually happened!)
		// And displaying another crash report etc ...
		return;
	}

	switch(nIDEvent)
	{
		case TIMER_CHECK_DBL_CLICK:
			if (_WaitSingleClick)
			{
				if (NLMISC::CTime::getLocalTime() - _WaitSingleClickStartTime > GetDoubleClickTime())
				{
					// too much time ellapsed since last click on the tray so this can't be a double click
					// just display the menu
					popTrayMenu();
					_WaitSingleClick = false;
				}
			}
		break;
		case TIMER_MAIN_LOOP:
			/*nlwarning("In msg queue = %d, out msg queue = %d", 
			  (int) _ClientMsgQueue.getReceiveQueueSize(), 
		      (int) _ClientMsgQueue.getSendQueueSize());*/
			// handle incoming messages from client
			for (;;)
			{
				NLMISC::CMemStream inMsg;
				if (!_ClientMsgQueue.pumpMessage(inMsg)) break;
				try
				{
					handleClientMessage(inMsg);
				}
				catch(EStream &)
				{
					nlwarning("BG DOWNLOADER PROTOCOL : Stream error 2");
					protocolError();
				}
			}
			// update download cotask
			_DownloadTask->resume();	

			updateThreadPriority();

			// if client is down and current task was paused, resume
			if (_Task.State != DLState_Idle)
			{
				if (!_ClientMsgQueue.connected())
				{
					if (!m_ThreadPriorityCtrl.IsWindowEnabled())
					{					
						setDownloadThreadPriority(BGDownloader::ThreadPriority_Low);
						m_ThreadPriorityCtrl.EnableWindow(TRUE);
					}
				}
			}
			else			
			{
				
				void *ryzomInstPIDPtr = NLMISC::CSharedMemory::accessSharedMemory(NLMISC::toSharedMemId(RYZOM_PID_SHM_ID));
				if (ryzomInstPIDPtr)
				{
					bool ok = NLMISC::CSharedMemory::closeSharedMemory(ryzomInstPIDPtr);
					if (!ok)
					{						
						LPVOID lpMsgBuf;
						FormatMessage
						( 
							FORMAT_MESSAGE_ALLOCATE_BUFFER | 
							FORMAT_MESSAGE_FROM_SYSTEM | 
							FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL,
							GetLastError(),
							0, // Default language
							(LPTSTR) &lpMsgBuf,
							0,
							NULL 
						);

						nlwarning("Fatal : failed to close shared memory handle, reason = %s", lpMsgBuf);
						LocalFree(lpMsgBuf);						
						// we should be able to close that handle, otherwise, ryzom would believe that another instance is running
						// (This situation has been seen already, but can't explain why ...:( :( :( )
						PostQuitMessage(0);
					}
				}
				else
				{
					// no client found, just quit
					PostQuitMessage(0);
				}				
			}
			
		break;
		case TIMER_SEND_PRIORITY: 
			if (_Verbose)
			{
				updateThreadPriority();
				CMemStream outMsg(false /*output stream */);
				TMsgType outMsgType = BGD_Priority;
				outMsg.serialEnum(outMsgType);
				outMsg.serialEnum(_WantedThreadPriority);				
				//nlwarning("##DOWNLOADER SENT message of type : %s", toString(outMsgType).c_str());
				_ClientMsgQueue.sendMessage(outMsg);				
			}
		break;
	}

	
	
	CDialog::OnTimer(nIDEvent);
}


//**************************************************************************************************
void CClient_background_downloaderDlg::setFailBitmap()
{
	m_StatusBitmap.ShowWindow(SW_SHOW);
	m_StatusBitmap.setBitmap(IDB_DL_STOP);
	m_ThreadPriorityCtrl.ShowWindow(SW_HIDE);
}

//**************************************************************************************************
void CClient_background_downloaderDlg::setSuccessBitmap()
{
	m_StatusBitmap.ShowWindow(SW_SHOW);
	m_StatusBitmap.setBitmap(IDB_PROGRESS_SUCCESS);
	m_ThreadPriorityCtrl.ShowWindow(SW_HIDE);
}

//**************************************************************************************************
bool CClient_background_downloaderDlg::isThreadPaused() const
{
	CPatchManager *pPM = CPatchManager::getInstance();
	return pPM->getCurrThread() && _WantedThreadPriority == ThreadPriority_Paused;
}

//**************************************************************************************************
void CClient_background_downloaderDlg::updateTaskProgressDisplay()
{
	m_ThreadPriorityCtrl.ShowWindow(SW_SHOW);
	CPatchManager *pPM = CPatchManager::getInstance();
	//ucstring state;	
	//std::vector<ucstring> log;
	//pPM->getThreadState(state, log);
	int bmRes;
	if (_ThreadPriority == ThreadPriority_Paused)
	{
		bmRes = IDB_PROGRESS_PAUSED;
	}
	else
	{
		bmRes = (NLMISC::CTime::getLocalTime() & 512) ? IDB_PROGRESS_1 : IDB_PROGRESS_0;
	}
	m_StatusBitmap.ShowWindow(SW_SHOW);	
	m_StatusBitmap.setBitmap(bmRes);
	
	ucstring state = "???";
	std::vector<ucstring> log;
	if (pPM->getThreadState(state, log))
	{
		int currentFilesToGet = pPM->getCurrentFilesToGet();
		int totalFilesToGet = pPM->getTotalFilesToGet();		
		setStatusString(state,
						currentFilesToGet,
						totalFilesToGet,
						pPM->getPatchingSize(),
						(uint32) _DownloadTask->TotalPatchSize,
						pPM->getCurrentFileProgress()
					   );	
		if (!isThreadPaused())
		{
			float progressValue;
			// FIXME (pPM->getPatchingSize() is always 0)
			/*if (_DownloadTask->TotalPatchSize != 0)
			{
				progressValue = (float) pPM->getPatchingSize() / _DownloadTask->TotalPatchSize;
			}
			else
			{*/				
				progressValue = (currentFilesToGet + pPM->getCurrentFileProgress()) / totalFilesToGet;
			//}
			setProgress(TRUE, progressValue);
		}
	}
}


//**************************************************************************************************
void CClient_background_downloaderDlg::setLocalStatusString(const ucstring &str)
{
	GetDlgItem(IDC_STATUS_STRING)->ShowWindow(SW_SHOW);	
	GetDlgItem(IDC_STATUS_STRING)->SetWindowText(str.toString().c_str());
	//nlwarning("Status String : %s", str.toString().c_str());
}


//**************************************************************************************************
void CClient_background_downloaderDlg::setStatusString(const ucstring &str, 
													   uint32 currentFilesToGet, 
													   uint32 totalFilesToGet,
													   uint32 patchingSize,
													   uint32 totalSize,
													   float currentFileProgress
													  )
{	
	ucstring statusString = str + toString(" (%d / %d)", (int) currentFilesToGet, (int) totalFilesToGet);	
	if (totalSize != 0)
	{
		statusString += " (" + getWrittenSize(patchingSize) + " / " + getWrittenSize(totalSize) + ")";
	}	
	CPatchManager *pPM = CPatchManager::getInstance();
	if (!isThreadPaused())
	{		
		setLocalStatusString(statusString);	
	}
	if (_Verbose)
	{
		NLMISC::CMemStream msg(false);
		nlassert(!msg.isReading());
		BGDownloader::TMsgType msgType = BGDownloader::BGD_UpdateStatusString;
		msg.serialEnum(msgType);
		ucstring mutableStr = str;
		msg.serial(mutableStr);
		msg.serial(currentFilesToGet);
		msg.serial(totalFilesToGet);
		msg.serial(patchingSize);
		msg.serial(totalSize);
		msg.serial(currentFileProgress);
		//nlwarning("##DOWNLOADER SENT message of type : %s", toString(msgType).c_str());
		_ClientMsgQueue.sendMessage(msg);
		//nlwarning(str.toString().c_str());
	}
}


//**************************************************************************************************
void CClient_background_downloaderDlg::setProgress(BOOL enabled, float progress)
{	
	((CProgressCtrl *) GetDlgItem(IDC_PROGRESS))->SetRange(0, 1000);
	((CProgressCtrl *) GetDlgItem(IDC_PROGRESS))->SetPos((int) (progress * 1000));
	GetDlgItem(IDC_PROGRESS)->EnableWindow(enabled);
}

//**************************************************************************************************
void CClient_background_downloaderDlg::OnBypass() 
{
	_BypassWanted = true;	
}


//**************************************************************************************************
void CClient_background_downloaderDlg::OnSelchangeThreadPriority() 
{	
	setDownloadThreadPriority((BGDownloader::TThreadPriority) m_ThreadPriorityCtrl.GetCurSel());
}

//**************************************************************************************************
int CClient_background_downloaderDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	
	return 0;
}

//**************************************************************************************************
void CClient_background_downloaderDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	
}
