// nel_launcherDlg.cpp : implementation file
//
#include "stdafx.h"
#include "nel_launcher.h"
#include "nel_launcherDlg.h"
#include <string>
#include <vector>
#include <comdef.h>

#include "nel/misc/path.h"
#include "patch.h"
#include "LoginDlg.h"
#include "MsgDlg.h"

using namespace std;
using namespace NLMISC;

//CConfigFile ConfigFile;

/////////////////////////////////////////////////////////////////////////////
// CNel_launcherDlg dialog

CNel_launcherDlg::CNel_launcherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNel_launcherDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNel_launcherDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_iX		= 0;
	m_iY		= 0;
	m_bMoving	= FALSE;
	m_pdlgLogin	= NULL;
}

void CNel_launcherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNel_launcherDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNel_launcherDlg, CDialog)
	//{{AFX_MSG_MAP(CNel_launcherDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNel_launcherDlg message handlers

BOOL CNel_launcherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetWindowText(MAIN_DLG_TITLE);

	m_pictBG.LoadPicture(IDP_BACKGROUND);

	CreateProgressBar();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CNel_launcherDlg::OnPaint()  
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();

		Login();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNel_launcherDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CNel_launcherDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(point.y < TITLEBAR_H)
	{
		POINT p;

		m_bMoving = TRUE;
		SetCapture();
		GetCursorPos(&p);
		m_iX = p.x;
		m_iY = p.y;
	}
}

void CNel_launcherDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(m_bMoving)
	{
		m_bMoving	= FALSE;
		ReleaseCapture();
	}
	else if(point.x >= CLOSE_BTN_X && point.x < CLOSE_BTN_X + CLOSE_BTN_W &&
		point.y >= CLOSE_BTN_Y && point.y < CLOSE_BTN_Y + CLOSE_BTN_H)
	{
		// The user has clicked on the exit button
		PostQuitMessage(0);
	}
}

void CNel_launcherDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_bMoving) 
	{
		POINT	p;

		GetCursorPos(&p);

		MoveBy(p.x - m_iX, p.y - m_iY);

		m_iX = p.x;
		m_iY = p.y;
	}
}

void CNel_launcherDlg::MoveBy(int x, int y)
{
	CRect	rect;

	GetWindowRect(&rect);
	MoveWindow(rect.left+x, rect.top+y, rect.right-rect.left,rect.bottom-rect.top, TRUE);
}

string getValue(const string &str, const string &token)
{
	string	realtoken	= token+"=\"";
	uint	spos		= str.find (realtoken);
	
	if (spos == string::npos) 
		return "";

	uint spos2 = str.find("\"", spos+realtoken.size ());
	if(spos == string::npos) 
		return "";

	return str.substr(spos+realtoken.size(), spos2-spos-realtoken.size());
}

void CNel_launcherDlg::launch(CString cs)
{	
	string str((LPCSTR)cs);
	launch(str);
}

void CNel_launcherDlg::launch(const string &str)
{
/*	string exe = ConfigFile.getVar ("Application").asString(1);
	string path;
	
	if(ConfigFile.getVar ("Application").size() == 4)
		path	= ConfigFile.getVar ("Application").asString(3);

	string rawargs = getValue (str, "nelArgs");

	nlinfo("extracting string '%s'", str.c_str ());

	// convert the command line in an array of string for the _execvp() function

	vector<string> vargs;
	const char *args[50];
	int argspos = 0;

	uint pos1 = 0, pos2 = 0;
	while(TRUE)
	{
		pos2 = rawargs.find(" ", pos1);
		if(pos2 == string::npos)
		{
			if(pos1 != rawargs.size())
			{
				string res = rawargs.substr(pos1);
				vargs.push_back(res);
			}
			break;
		}
		if(pos1 != pos2)
		{
			string res = rawargs.substr (pos1, pos2-pos1);
			vargs.push_back(res);
		}
		pos1 = pos2 + 1;
	}

	int i;
	int size;

	if(vargs.size()>47) 
		size = 47;
	else 
		size = vargs.size();

	args[0] = exe.c_str();
	for(i = 0; i < size; i++)
	{
		args[i+1] = vargs[i].c_str();
	}
	args[i+1] = NULL;

	// go in the good path before launching
	if(!path.empty())
		_chdir(path.c_str());

	// execute, should better use CreateProcess()
	if(_execvp(exe.c_str(), args) == -1)
	{
		CMsgDlg	dlgMsg("Error", "Can't execute the game");

		dlgMsg.DoModal();
		APP.ResetConnection();
	}
	else	
	{
		APP.ResetConnection();
		PostQuitMessage(0);
	}*/

//	string exe = ConfigFile.getVar ("Application").asString(1);
//	string path;
	
//	if(ConfigFile.getVar ("Application").size() == 4)
//		path	= ConfigFile.getVar ("Application").asString(3);
	CString csPath	= APP.m_config.m_csAppBasePath;

	string rawargs = getValue (str, "nelArgs");

	// convert the command line in an array of string for the _execvp() function

	vector<string> vargs;
	const char *args[50];
	int argspos = 0;

	uint pos1 = 0, pos2 = 0;
	while(TRUE)
	{
		pos2 = rawargs.find(" ", pos1);
		if(pos2 == string::npos)
		{
			if(pos1 != rawargs.size())
			{
				string res = rawargs.substr(pos1);
				vargs.push_back(res);
			}
			break;
		}
		if(pos1 != pos2)
		{
			string res = rawargs.substr (pos1, pos2-pos1);
			vargs.push_back(res);
		}
		pos1 = pos2 + 1;
	}

	int i;
	int size;

	if(vargs.size()>47) 
		size = 47;
	else 
		size = vargs.size();

	args[0] = APP.m_config.m_csExe;
	for(i = 0; i < size; i++)
	{
		args[i+1] = vargs[i].c_str();
	}
	args[i+1] = NULL;

	// go in the good path before launching
	if(!csPath.IsEmpty())
		_chdir(csPath);

	// execute, should better use CreateProcess()
	if(_execvp(APP.m_config.m_csExe, args) == -1)
	{
		CMsgDlg	dlgMsg("Error", "Can't execute the game");

		dlgMsg.DoModal();
		APP.ResetConnection();
	}
	else	
	{
		APP.ResetConnection();
		PostQuitMessage(0);
	}
}

void CNel_launcherDlg::patch(CString cs)
{
	string str((LPCSTR)cs);
	patch(str);
}

void CNel_launcherDlg::patch(const string &str)
{
	string url = "http://" + APP.m_config.m_csHost;

	string serverVersion = getValue(str, "serverVersion");

	string urlok = url;
	urlok += getValue (str, "nelUrl");
	urlok += "&newClientVersion=";
	urlok += serverVersion;

	string urlfailed = url;
	urlfailed += getValue (str, "nelUrlFailed");
	urlfailed += "&reason=";

	startPatchThread(getValue (str, "nelServerPath"), serverVersion, urlok, urlfailed, "<br>");

	m_dlgProgress.SetRange(getTotalFilesToGet());
	m_dlgProgress.UpdatePos(0);
	m_dlgProgress.Show();

	SetTimer(0, 500, NULL);
}

void CNel_launcherDlg::OnTimer(UINT nIDEvent) 
{
	m_dlgProgress.SetRange(getTotalFilesToGet());
	m_dlgProgress.UpdatePos(getCurrentFilesToGet());

	string state, log;
	if(patchState(state, log))
		m_dlgProgress.UpdateMsg(state.c_str());

	string	url;
	bool	res;

	if(patchEnded(url, res))
	{
		KillTimer(0);
		if(res)
			m_dlgProgress.UpdateMsg("Patch completed succesfuly.");
		else
			m_dlgProgress.UpdateMsg("Patch could not be completed due to an error.");
		::Sleep(2000);
		m_dlgProgress.Show(FALSE);
		m_dlgProgress.UpdatePos(0);

		// If we are on the servers page, reload it
		if(m_wndTabs.GetFocusPos() == 0)
			OnTab(0);

//		m_dlgWeb.OpenUrl(url);
	}
}

void CNel_launcherDlg::OnTab(int iTab)
{
	CString csErrPage;
	CString csUrl;

	getcwd(csErrPage.GetBuffer(_MAX_PATH), _MAX_PATH);
	csErrPage.ReleaseBuffer();
	if(csErrPage.Right(1) != '\\')
		csErrPage	+= '\\';
	csErrPage	+= "error.html";

	switch(iTab)
	{
		// Only for Win98
		case -1:
		{
			if(!IS_WINNT)
			{
				m_dlgWeb.ShowWindow(SW_HIDE);
				string Version = getVersion();

				string url = "http://" + APP.m_config.m_csHost + APP.m_config.m_csUrlMain;
				url += "?newClientVersion=" + Version;
				url += "&newClientApplication=" + APP.m_config.m_csApp;

				m_dlgWeb.OpenUrl(url);

				OnTab(0);
				m_dlgWeb.ShowWindow(SW_SHOW);
			}
			break;
		}
		case 0:
		{
			// Servers tab
			string Version = getVersion();

			string url = "http://" + APP.m_config.m_csHost + APP.m_config.m_csUrlMain;
			url += "?newClientVersion=" + Version;
			url += "&newClientApplication=" + APP.m_config.m_csApp;

			m_dlgWeb.OpenUrl(url);
			break;
		}
		case 1:
		{
			// RN tab
			if(!APP.m_bAuthWeb)
				m_dlgWeb.OpenUrl(csErrPage);
			else
			{
				csUrl	= APP.m_config.m_csUrlRN;
				if(!csUrl.IsEmpty())
					m_dlgWeb.OpenUrl(csUrl);
			}
			break;
		}
		case 2:
		{
			// News tab
			if(!APP.m_bAuthWeb)
				m_dlgWeb.OpenUrl(csErrPage);
			else
			{
				csUrl	= APP.m_config.m_csUrlNews;
				if(!csUrl.IsEmpty())
					m_dlgWeb.OpenUrl(csUrl);
			}
			break;
		}
	}
}

BOOL CNel_launcherDlg::Login()
{
	CString csLogin	= APP.GetRegKeyValue(_T("Login"));
	
	if(!m_pdlgLogin && !APP.m_bAuthGame)
	{
		m_pdlgLogin	= new CLoginDlg(this);

		if(m_pdlgLogin->DoModal() == IDOK)
		{
			CreateTabs();			
			CreateWebZone();

			// remove temp files
			if (NLMISC::CFile::fileExists(PATCH_BAT_FILE))
				NLMISC::CFile::deleteFile(PATCH_BAT_FILE);
						
			if(IS_WINNT)
				OnTab(0);
			else
				OnTab(-1);

			delete m_pdlgLogin;
			m_pdlgLogin	= NULL;
			return TRUE;
		}
		else
		{
			delete m_pdlgLogin;
			m_pdlgLogin	= NULL;
			PostQuitMessage(0);
		}
		return FALSE;
	}
	return TRUE;
}

void CNel_launcherDlg::CreateTabs()
{
	if(m_wndTabs.Create(NULL, "Tabs", WS_CHILD, CRect(), this, 9645))
	{
		m_wndTabs.AddTab(IDP_TAB_SERVERS, IDP_TAB_SERVERS_FOCUS);
		if(APP.m_bAuthWeb)
		{
			m_wndTabs.AddTab(IDP_TAB_RN, IDP_TAB_RN_FOCUS);
			m_wndTabs.AddTab(IDP_TAB_NEWS, IDP_TAB_NEWS_FOCUS);
		}
		m_wndTabs.Move(BROWSER_X, BROWSER_Y - TAB_H);
		m_wndTabs.SetObserver(this);
		m_wndTabs.SetFocusPos(0);
		m_wndTabs.ShowWindow(SW_SHOW);
	}
}

void CNel_launcherDlg::CreateWebZone()
{
	// Creating the web browser window
	m_dlgWeb.Create(IDD_WEB, this);
	m_dlgWeb.MoveWindow(BROWSER_X, BROWSER_Y, BROWSER_W, BROWSER_H);
	m_dlgWeb.ShowWindow(SW_SHOW);			
}

void CNel_launcherDlg::CreateProgressBar()
{
	CRect	r;

	GetClientRect(&r);
	m_dlgProgress.Create(IDD_PROGRESS, this);
	m_dlgProgress.Show(SW_HIDE);
	m_dlgProgress.MoveWindow(0, r.Height() - PROGRESS_H, r.Width()+1, PROGRESS_H);
}

void CNel_launcherDlg::OnDestroy() 
{
	APP.ResetConnection();

	CDialog::OnDestroy();	
}

BOOL CNel_launcherDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	POINT	p;

	GetCursorPos(&p);
	ScreenToClient(&p);

	if(p.x >= CLOSE_BTN_X && p.x < CLOSE_BTN_X + CLOSE_BTN_W && p.y >= CLOSE_BTN_Y && p.y < CLOSE_BTN_Y + CLOSE_BTN_H)
		SetCursor(APP.m_hcPointer);
	else if(p.y < TITLEBAR_H && !m_pdlgLogin)
		SetCursor(APP.m_hcPointer);
	else
		return CDialog::OnSetCursor(pWnd, nHitTest, message);
	return TRUE;
}

BOOL CNel_launcherDlg::OnEraseBkgnd(CDC* pDC) 
{
	CRect	r;

	GetClientRect(&r);
	m_pictBG.Display(*pDC, r);

    return TRUE;
}
