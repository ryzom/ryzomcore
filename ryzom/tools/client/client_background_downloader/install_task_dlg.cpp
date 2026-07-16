// install_task_dlg.cpp : implementation file
//
#include "stdpch.h"
#include "install_task_dlg.h"
#include "client_background_downloader.h"
#include "nel/misc/win32_util.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "game_share/bg_downloader_msg.h"
#include "error_box_dlg.h"
#include "blended_bitmap.h"
//
#include "web_page_embedder.h"

#include "install_logic/game_downloader.h"


using NLMISC::CI18N;
using NLMISC::CPath;

/////////////////////////////////////////////////////////////////////////////
// CInstallTaskDlg dialog

CInstallTaskDlg::CInstallTaskDlg(CWnd *pParent)
: CDialog(CInstallTaskDlg::IDD, pParent)
{
	_MustUpdateGui = true;
	_MustExit = false;
	_BrowserEventSink.Parent = this;
}


CInstallTaskDlg::CInstallTaskDlg(CPatchManager::SPatchInfo &infoOnPatch, CWnd* pParent /*=NULL*/)
	: CDialog(CInstallTaskDlg::IDD, pParent)
{
	_TaskType = TaskType_Unknwown;
	_BrowserEventSink.Parent = this;
	_Task = TaskCheck;
	_InfoOnPatch = &infoOnPatch;
	//{{AFX_DATA_INIT(CInstallTaskDlg)
	//}}AFX_DATA_INIT
	_TotalPatchSize = 0;
	_AnimStep = 0;
	//_Task = TestWebPage;		
}

CInstallTaskDlg::CInstallTaskDlg(const std::vector<std::string> &categories, uint totalPatchSize, CWnd *pParent /*=NULL*/)
: CDialog(CInstallTaskDlg::IDD, pParent)
{
	_TaskType = TaskType_Unknwown;
	_BrowserEventSink.Parent = this;
	_Task = TaskPatch;
	_InfoOnPatch = NULL;
	_TotalPatchSize = totalPatchSize;
	_Categories = categories;
}



void CInstallTaskDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInstallTaskDlg)	
	DDX_Control(pDX, IDC_PROGRESS_BITMAP, m_ProgressBM);
	DDX_Control(pDX, IDC_TASK_TYPE, m_TaskTypeText);
	DDX_Control(pDX, IDC_STATUS_TEXT, m_StatusText);
	DDX_Control(pDX, IDC_REMAINING_TIME, m_RemainingTimeText);	
	DDX_Control(pDX, IDC_PROGRESS, m_ProgressCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInstallTaskDlg, CDialog)
	//{{AFX_MSG_MAP(CInstallTaskDlg)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDEXIT, OnExit)
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInstallTaskDlg message handlers

BOOL CInstallTaskDlg::OnInitDialog() 
{		

	SetTimer(TIMER_MAIN_LOOP, 30, NULL);
	CDialog::OnInitDialog();	


	m_ProgressBM.setBitmap(IDB_PROGRESS_ANIM);
	m_ProgressBM.setBgColor(RGB(255, 255, 255));
	updateProgressIcon();


	::CBitmap progressBG;
	::CBitmap progressFG;
	progressBG.LoadBitmap(MAKEINTRESOURCE(IDB_PROGRESS_BG));
	progressFG.LoadBitmap(MAKEINTRESOURCE(IDB_PROGRESS_FG));
	m_ProgressCtrl.init(progressBG, progressFG);

	m_StatusText.SetBkColor(RGB(255, 255, 255));
	m_RemainingTimeText.SetBkColor(RGB(255, 255, 255));
	
	m_TaskTypeText.SetBackgroundColor(FALSE, RGB(255, 255, 255));
	

	GetDlgItem(IDC_WEB_PAGE)->ShowWindow(SW_HIDE);
	updateWebPageRegion();
	GetDlgItem(IDC_BG_BITMAP)->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
	GetDlgItem(IDC_WEB_PAGE)->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);

	
	
	

	SetIcon(AfxGetApp()->LoadIcon(IDI_MAIN_ICON), TRUE);
	NLMISC::CWin32Util::localizeWindow(this->m_hWnd);		
	CPatchManager *pPM = CPatchManager::getInstance();
	m_ProgressCtrl.SetRange(0, 1000);
	m_ProgressCtrl.SetPos(0);
	GetDlgItem(IDC_STATUS_TEXT)->SetWindowText("");
	
	//GetDlgItem(IDC_WEB_PAGE)->ShowWindow(SW_HIDE);

	// Get start URL
	NLMISC::CConfigFile::CVar *installWebPageVarPtr = theApp.ConfigFile.getVarPtr("InstallWebPage");
	if (installWebPageVarPtr)
	{
		StartURL = installWebPageVarPtr->asString();
		// add the language
		std::string languageCode = theApp.getLanguage();
		if (NLMISC::nlstricmp(languageCode, "wk") == 0)
		{
			languageCode = "en";
		}
		
		StartURL += "client_install_" + languageCode + ".htm";
	}

	IGameDownloader* gd = IGameDownloader::getInstance();
	gd->setInstallTaskDlg(this);


	// Deactivate nico code because Game download is different from classic patch because game_download can use Ryzom Patch or Torrent protocol	
#if 0
	switch(_Task)
	{
		case TaskCheck:
			pPM->startCheckThread(true);			
		break;
		case TaskPatch:
		{	
			beginBrowse();
			pPM->startPatchThread(_Categories, true);									
		}
		break;
		case TestWebPage:
		{	
			beginBrowse();			
		}
		break;
		default:
			nlassert(0);
		break;
	}
	updateTaskTypeText();
#endif
	
	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInstallTaskDlg::updateWebPageRegion()
{			
	std::vector<HWND> childWindows;
	NLMISC::CWin32Util::appendChildWindows(this->m_hWnd, childWindows);	
	CWnd *placeHolder = GetDlgItem(IDC_BG_BITMAP);	
	CWnd *webPage = GetDlgItem(IDC_WEB_PAGE);	
	CRect webPageRect;	
	webPage->GetClientRect(&webPageRect);
	CRgn rgn;
	// trick here : hide right black border
	rgn.CreateRectRgn(webPageRect.left, webPageRect.top, webPageRect.right - 1, webPageRect.bottom);		
	//nlwarning("%d, %d, %d, %d", webPageRect.top, webPageRect.right, webPageRect.bottom);
	for(uint k = 0; k < childWindows.size(); ++k)	
	{		
		if (webPage->m_hWnd == childWindows[k]) continue;
		if (placeHolder->m_hWnd == childWindows[k]) continue;
		CRect childRect;
		::GetWindowRect(childWindows[k], &childRect);		
		webPage->ScreenToClient(childRect);
		if (childRect == webPageRect) continue;
		CRgn childRgn;
		int result = ::GetWindowRgn(childWindows[k], childRgn);
		if (result != ERROR)
		{
			childRgn.OffsetRgn(childRect.left, childRect.top);
			rgn.CombineRgn(&rgn, &childRgn, RGN_DIFF );
		}
		else
		{	
			if (childWindows[k] == m_ProgressCtrl.m_hWnd)
			{
				childRgn.CreateRectRgn(childRect.left + 1, childRect.top + 1, childRect.right - 1, childRect.bottom - 1);
			}
			else
			{
				childRgn.CreateRectRgn(childRect.left, childRect.top, childRect.right, childRect.bottom);
			}
			rgn.CombineRgn(&rgn, &childRgn, RGN_DIFF);
		}
	}	
	webPage->SetWindowRgn((HRGN) rgn.Detach(), TRUE);
	
}

void CInstallTaskDlg::beginBrowse()
{
	// Do not use Cfg Variable. Use the one given by GameDownloader that is configured on server.
	IGameDownloader* gd = IGameDownloader::getInstance();
	StartURL = gd->getInstallHttpBackground();
	
	if (!StartURL.empty())
	{		
		CWebPageEmbedder::setBrowserEventSink(GetDlgItem(IDC_WEB_PAGE)->m_hWnd, &_BrowserEventSink);
		CWebPageEmbedder::displayHTMLPage(GetDlgItem(IDC_WEB_PAGE)->m_hWnd, StartURL.c_str());
	}
}


void CInstallTaskDlg::handleTaskEnd(bool ok)
{	
	if (!ok)
	{		
		CPatchManager *pPM = CPatchManager::getInstance();		
		ucstring restart;
		CErrorBoxDlg msgBox(pPM->getLastErrorMessage(), restart,  this);
		msgBox.DoModal();
		PostQuitMessage(0);	
	}
}



void CInstallTaskDlg::updateProgressIcon() 
{
	RECT progressAnimRect;
	m_ProgressBM.getBitmapRect(progressAnimRect);
	if (progressAnimRect.right)
	{
		uint animCount = progressAnimRect.bottom / progressAnimRect.right;
		//uint animCount = 16;
		RECT srcRect;
		srcRect.left = 0;
		srcRect.right = progressAnimRect.right;
		srcRect.top = srcRect.right * (_AnimStep / 3);
		srcRect.bottom = srcRect.right * ((_AnimStep / 3) + 1);
		m_ProgressBM.setSrcRect(srcRect);
		++ _AnimStep;
		if (_AnimStep >= 3 * animCount) _AnimStep = 0;
	}
}

void CInstallTaskDlg::OnTimer(UINT nIDEvent) 

{		
	static const char *markerFileName = "install_final_marker";
	if (nIDEvent == TIMER_MAIN_LOOP)
	{
		IGameDownloader* gd = IGameDownloader::getInstance();
		// update the progress icon animation
		if (gd->getUpdateDownloadIcon())
		{
			updateProgressIcon();
		}
		
		// limit multi entry (a atomic swap would be best)
		static  volatile bool ok = true;		
		if (ok)
		{					
			ok = false;
			// Exit if cancel or quit button was pressed
			if (_MustExit)
			{
				// display Warning message
				
				gd->doAction("download_cancel", true);
				_MustExit = false;
			}

			IGameDownloader* gd = IGameDownloader::getInstance();
			gd->update();
			// clossing process take 5 second so hide before closing
			if ( gd->getState() == IGameDownloader::InstallFinished &&  IsWindowVisible())
			{
				CWnd* win = AfxGetMainWnd();
				if (win) { win->ShowWindow(SW_HIDE);  }				
			}
			else
			{

			}
			// If GameDownloader State has changed then look state on update components			
			if (_MustUpdateGui)
			{
				// start browser (the browser is started before displaying the page content in ordre to no display and empty pages)
				static bool browserWasStarted = false;
				if (!browserWasStarted && gd->getMustStartBrowser()  )
				{
					browserWasStarted = true;
					beginBrowse();
				}
				// must be updated if browser is active
				if (gd->getBrowserActive())
				{
					updateWebPage();	
				}
				// update label
				ucstring str;				
				str = gd->getContentLabelText();			
				SetWindowTextW(GetDlgItem(IDC_STATUS_TEXT)->m_hWnd, (const WCHAR *) str.c_str());				
				//  update progress bar	
				sint32 value = gd->getProgressBarValue();
				if (-1 < value && value <= 100)
				{
					m_ProgressCtrl.SetPos(10*value);
				}								
				// update Title and text content
				updateTaskTypeText();
				if (gd->getBrowserActive())
				{
					updateWebPageRegion();
				}

				_MustUpdateGui = false;
			}
			
			ok = true;
		}
		
#if 0	
	{			
		updateProgressIcon();
		updateTaskTypeText();
		CPatchManager *pPM = CPatchManager::getInstance();
		// update text & progress bar
		switch(_Task)
		{
			case TaskCheck:
			{				
				bool ok;
				if (pPM->isCheckThreadEnded(ok))
				{
					KillTimer(TIMER_MAIN_LOOP);
					handleTaskEnd(ok);					
					EndDialog(IDOK);
				}			
				pPM->getInfoToDisp(*_InfoOnPatch);				
			}
			break;
			case TaskPatch:
			{	
				updateWebPage();				
				bool ok;
				if (pPM->isPatchThreadEnded(ok))
				{						
					handleTaskEnd(ok);				
					if (pPM->mustLaunchBatFile())
					{
						try
						{
							NLMISC::COFile marker(markerFileName);
							pPM->createBatchFile(pPM->getDescFile(), false);
							pPM->executeBatchFile();
							_Task = WaitPatchTaskEnd;
							SetWindowTextW(GetDlgItem(IDC_STATUS_TEXT)->m_hWnd, (const WCHAR *) CI18N::get("uiBGD_FinalMove").c_str());
							m_ProgressCtrl.SetPos(1000);
							return;
						}					
						catch(NLMISC::EDiskFullError &)
						{							
							errorMsg(CI18N::get("uiPatchDiskFull"));
						}
						catch(NLMISC::EWriteError &)
						{
							errorMsg(CI18N::get("uiPatchWriteError"));		
						}
						catch(std::exception &e)
						{
							errorMsg(CI18N::get("uiBGD_InstallFailed") + ucstring(e.what()));		
						}						
					}
					else
					{
						KillTimer(TIMER_MAIN_LOOP);
						EndDialog(IDOK);
					}
				}				
			}
			break;
			case WaitPatchTaskEnd:
				// wait until the batch file finish its execution
				if (!NLMISC::CPath::exists(markerFileName))
				{
					EndDialog(IDOK);
				}
				return;				
			break;
			case TestWebPage:
			{
				updateWebPage();
				static uint counter = 0;
				GetDlgItem(IDC_STATUS_TEXT)->SetWindowText(NLMISC::toString("%d", counter ++).c_str());
				updateWebPageRegion();
			}
			break;
			default:
				nlassert(0);
			break;
		}
		if (_Task != TestWebPage)
		{
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
								(uint32) _TotalPatchSize,
								pPM->getCurrentFileProgress()
							   );	
				
				float progressValue = (currentFilesToGet + pPM->getCurrentFileProgress()) / totalFilesToGet;				
				m_ProgressCtrl.SetPos((int) (1000 * progressValue));
			}
		}
		#endif
		
	}

	CDialog::OnTimer(nIDEvent);
}

//**************************************************************************************************
void CInstallTaskDlg::errorMsg(const ucstring &error)
{
	m_ProgressBM.setBitmap(IDB_PROGRESS_FAIL);
	KillTimer(TIMER_MAIN_LOOP);
	/*
	CErrorBoxDlg msgBox(error, this);
	msgBox.DoModal();
	*/
	PostQuitMessage(0);
}


//**************************************************************************************************
void CInstallTaskDlg::updateWebPage()
{
	if (!CWebPageEmbedder::isBusy(GetDlgItem(IDC_WEB_PAGE)->m_hWnd))
	{
		GetDlgItem(IDC_BG_BITMAP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_WEB_PAGE)->ShowWindow(SW_SHOW);
	}
}

//**************************************************************************************************
void CInstallTaskDlg::setStatusString(const ucstring &str, 
													   uint32 currentFilesToGet, 
													   uint32 totalFilesToGet,
													   uint32 patchingSize,
													   uint32 totalSize,
													   float currentFileProgress
													  )
{	
	ucstring statusString = str + NLMISC::toString(" (%d / %d)", (int) currentFilesToGet, (int) totalFilesToGet);	
	if (totalSize != 0)
	{
		statusString += " (" + BGDownloader::getWrittenSize(patchingSize) + " / " + BGDownloader::getWrittenSize(totalSize) + ")";
	}		
	SetWindowTextW(GetDlgItem(IDC_STATUS_TEXT)->m_hWnd, (const WCHAR *) statusString.c_str());
	m_StatusText.SetBkColor(RGB(255, 255, 255));
}
// Windows exit
void CInstallTaskDlg::OnExit() 
{
	_MustExit = true;
	
	//PostQuitMessage(0);	
}

void CInstallTaskDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);	
}


//**************************************************************************************************
void CInstallTaskDlg::CBrowserEventSink::onInvoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pDispParams, VARIANT FAR *pVarResult, EXCEPINFO FAR *pExcepInfo, UINT FAR *puArgErr)
{					
	switch (dispIdMember)
	{					
		case DISPID_BEFORENAVIGATE2:			
			std::string  url(ucstring((uint16 *) pDispParams[4].rgvarg->pcVal).toString());
			if (url != Parent->StartURL)
			{
				// any link will open a new window 
				*pDispParams[0].rgvarg->pboolVal = TRUE; // cancel current navigation
				NLMISC::openURL(url.c_str()); // ... & launch a new browser for that url
			}
		break;
	}				
}


void CInstallTaskDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CDialog::OnPaint() for painting messages
}
//MFC Button
void CInstallTaskDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	_MustExit = true;
	//CDialog::OnClose();
}

//**************************************************************************************************
void CInstallTaskDlg::updateTaskTypeText()
{
	// Use GameDownloader state to display Progression Label ( 1/Install => 2/ Download)
	static IGameDownloader::EState previousState = IGameDownloader::Init;
	IGameDownloader* gd = IGameDownloader::getInstance();
	IGameDownloader::EState currentState = gd->getState();
	//UpdatePackage <=>
	if (currentState != previousState && (currentState == IGameDownloader::Downloading || currentState == IGameDownloader::Installing))
	{
		m_TaskTypeText.clear();
		if (currentState == IGameDownloader::Downloading)
		{
				m_TaskTypeText.setFont(10, "Verdana", CFE_BOLD);
				m_TaskTypeText.append(NLMISC::CI18N::get("uiBGD_TaskTypeDownload"));
				m_TaskTypeText.setFont(10, "Verdana", 0);
				m_TaskTypeText.append(ucstring("\n"));				
				m_TaskTypeText.append(NLMISC::CI18N::get("uiBGD_TaskTypeInstall"));
		} else if (currentState == IGameDownloader::Installing)
		{				
				m_TaskTypeText.setFont(10, "Verdana", 0);
				m_TaskTypeText.append(NLMISC::CI18N::get("uiBGD_TaskTypeDownload"));
				m_TaskTypeText.setFont(10, "Verdana", CFE_BOLD);
				m_TaskTypeText.append(ucstring("\n"));				
				m_TaskTypeText.append(NLMISC::CI18N::get("uiBGD_TaskTypeInstall"));
		}
		// ?UpdatePackage
	}

}



BOOL CInstallTaskDlg::OnEraseBkgnd(CDC* pDC) 
{
	return CDialog::OnEraseBkgnd(pDC);
	//return TRUE;
}

