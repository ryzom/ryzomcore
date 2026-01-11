// nel_launcherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "nel_launcher.h"
#include "nel_launcherDlg.h"
#include <string>
#include <vector>
#include <process.h>
#include <direct.h>
#include "mshtml.h"

#include "nel/misc/config_file.h"
#include "nel/misc/path.h"

#include "patch.h"

using namespace std;
using namespace NLMISC;

CConfigFile ConfigFile;

/*#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif*/

const char *PleaseWaitFilename = "pleasewait.html";

void CNel_launcherDlg::openUrl (const std::string &url)
{
	m_explore.Navigate(url.c_str(), NULL, NULL, NULL, NULL);
}

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
}

void CNel_launcherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNel_launcherDlg)
	DDX_Control(pDX, IDC_EXPLORER1, m_explore);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNel_launcherDlg, CDialog)
	//{{AFX_MSG_MAP(CNel_launcherDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNel_launcherDlg message handlers

BOOL CNel_launcherDlg::OnInitDialog()
{
	NLMISC::CApplicationContext ApplicationContext;

	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//CWebBrowser2 m_browser - member variable  

//	downloadFile ();
//	return TRUE;

	ConfigFile.load ("nel_launcher.cfg");

	// get the working path
	string workingPath = CPath::getFullPath (ConfigFile.getVar ("Application").asString(2));
	
	if (!NLMISC::CFile::isExists (workingPath))
	{
		nlerror ("Working path '%s' doesn't exists", workingPath.c_str ());
	}

	// load the pleasewait html page if available in the nel_launcher directory
	bool PleaseWaitLoaded = false;
	string PleaseWaitFullPath;
	PleaseWaitFullPath = CPath::getFullPath (PleaseWaitFilename, false);
	if (NLMISC::CFile::isExists (PleaseWaitFullPath))
	{
		openUrl(PleaseWaitFullPath.c_str());
		PleaseWaitLoaded = true;
	}

	_chdir (workingPath.c_str ());

	// load the pleasewait html page if available in the client directory
	PleaseWaitFullPath = CPath::getFullPath (PleaseWaitFilename, false);
	if (!PleaseWaitLoaded && NLMISC::CFile::isExists (PleaseWaitFullPath))
	{
		openUrl(PleaseWaitFullPath.c_str());
	}

	// remove temp files
	if (NLMISC::CFile::fileExists ("update_nel_launcher.bat"))
		NLMISC::CFile::deleteFile ("update_nel_launcher.bat");
				
	string Version = getVersion ();

	string url = "http://"+ConfigFile.getVar ("StartupHost").asString()+ConfigFile.getVar ("StartupPage").asString();
	url += "?newClientVersion=" + Version;
	url += "&newClientApplication=" + ConfigFile.getVar ("Application").asString(0);

	openUrl (url);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

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
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNel_launcherDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BEGIN_EVENTSINK_MAP(CNel_launcherDlg, CDialog)
    //{{AFX_EVENTSINK_MAP(CNel_launcherDlg)
	ON_EVENT(CNel_launcherDlg, IDC_EXPLORER1, 250 /* BeforeNavigate2 */, OnBeforeNavigate2Explorer1, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CNel_launcherDlg, IDC_EXPLORER1, 259 /* DocumentComplete */, OnDocumentCompleteExplorer1, VTS_DISPATCH VTS_PVARIANT)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CNel_launcherDlg::OnBeforeNavigate2Explorer1(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel) 
{
	// set the user agent to nel_launcher changing the header
    CString strHeader(Headers->bstrVal);
    if (strHeader == "")
	{
		IWebBrowser2 *pBrowser;
		LPDISPATCH pWebDisp;

		pDisp->QueryInterface(IID_IWebBrowser2, (void**) &pBrowser);
		pBrowser->get_Container(&pWebDisp);

		BSTR bstr = SysAllocString(L"User-Agent: nel_launcher\r\n");
		Headers->bstrVal =  bstr;

		pBrowser->Navigate2(URL, Flags, TargetFrameName, PostData, Headers);

		if (!pWebDisp)
			(*Cancel) = true;

		if (pWebDisp)
			pWebDisp->Release();
		if (pBrowser)
			pBrowser->Release();

		SysFreeString (bstr);

		return;
    }


/*	CString cstr = URL->bstrVal;
	string str = string((const char*)cstr);

	if (str.find ("clientVersion=") == string::npos)
	{
		string Version;

		FILE *fp = fopen ("VERSION", "rb");
		if (fp!=NULL)
		{
			char ver[1000];
			if (fgets (ver, 1000, fp) != NULL)
			{
				Version = ver;
			}
			fclose (fp);
		}

		string url = str;

		if (url.find ("?") != string::npos)
		{
			url += "&";
		}
		else
		{
			url += "?";
		}
//		url += "clientVersion=" + Version;
//		url += "&clientApplication=" + ConfigFile.getVar ("Application").asString(0);
		url += "toto=10";
		*Cancel = TRUE;
		m_explore.Navigate(url.c_str(), Flags, TargetFrameName, PostData, Headers);
*/
/*
		VARIANT vurl;
		VariantInit (&vurl);
		vurl.vt = VT_BSTR;
		vector<unsigned short> v;
		for (uint i = 0; i < url.size (); i++)
			v.push_back ((unsigned short)url[i]);
		v.push_back (0);
		BSTR bstr = SysAllocString((const unsigned short *)&(v[0]));
		vurl.bstrVal = bstr;

		OnBeforeNavigate2Explorer1(pDisp, &vurl, Flags, TargetFrameName, PostData, Headers, Cancel);

		SysFreeString (bstr);
*/
/*	}
	else
	{
		nlinfo ("coucou");
	}
*/
/*	if (str.find("?nel_quit=1") != string::npos)	
	{
		exit(0);
	}

	string token ("nel_exe=");

	int spos = str.find (token);
	if (spos == string::npos) return;

	int spos2 = str.find ("&", spos+token.size ()+1);
	if (spos == string::npos) return;

	string path;
	string exe = str.substr (spos+token.size (), spos2-spos-token.size ());

	CConfigFile::CVar *var = ConfigFile.getVarPtr (exe);
	if (var == NULL)
	{
		char str[1024];
		smprintf (str, 1024, "Don't know the executable filename for the application '%s'", exe.c_str ());
		MessageBox (str, "nel_launcher error");
		exit(0);
	}
	else
	{
		exe = var->asString (0);
		path = var->asString (1);
	}

	token = "nel_args=";
	spos = str.find (token);
	if (spos == string::npos) return;

	string rawargs = str.substr (spos+token.size ());

	// decode arguments (%20 -> space)

	vector<string> vargs;
	const char *args[50];
	int argspos = 0;

	int pos1 = 0, pos2 = 0;
	while (true)
	{
		pos2 = rawargs.find("%20", pos1);
		if (pos2==string::npos)
		{
			if(pos1!=rawargs.size())
			{
				string res = rawargs.substr (pos1);
				vargs.push_back (res);
			}
			break;
		}
		if(pos1 != pos2)
		{
			string res = rawargs.substr (pos1, pos2-pos1);
			vargs.push_back (res);
		}
		pos1 = pos2+3;
	}

	int i;
	int size;
	if(vargs.size()>47) size = 47;
	else size = vargs.size();

	args[0] = exe.c_str ();
	for(i = 0; i < size; i++)
	{
		args[i+1] = vargs[i].c_str ();
	}
	args[i+1] = NULL;


	// execute, should better use CreateProcess()

	_chdir (path.c_str());
	_execvp (exe.c_str(), args);
	exit(0);
*/	
/*	string cmd;

	cmd=exe+" "+rawargs;
	while (true)
	{
		int pos = cmd.find ("%20");
		if (pos == string::npos)
			break;

		cmd.replace (pos, 3, " ");
	}

	WinExec (cmd.c_str(), SW_SHOWNORMAL);
*/
}

void CNel_launcherDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if (m_explore.m_hWnd == NULL) return;
	CRect rect;
	GetClientRect (&rect);	
	m_explore.SetWindowPos ((CWnd* )HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);
}

string getValue (const string &str, const string &token)
{
	string realtoken = token+"=\"";
	uint spos = str.find (realtoken);
	if (spos == string::npos) return "";

	uint spos2 = str.find ("\"", spos+realtoken.size ());
	if (spos == string::npos) return "";

	return str.substr (spos+realtoken.size (), spos2-spos-realtoken.size ());
}


void CNel_launcherDlg::launch (const string &str)
{
	string exe = ConfigFile.getVar ("Application").asString(1);
	string path;
	
	if (ConfigFile.getVar ("Application").size() == 4)
		path = ConfigFile.getVar ("Application").asString(3);

	string rawargs = getValue (str, "nelArgs");

	nlinfo ("extracting string '%s'", str.c_str ());

	// convert the command line in an array of string for the _execvp() function

	vector<string> vargs;
	const char *args[50];
	int argspos = 0;

	uint pos1 = 0, pos2 = 0;
	while (true)
	{
		pos2 = rawargs.find(" ", pos1);
		if (pos2==string::npos)
		{
			if(pos1!=rawargs.size())
			{
				string res = rawargs.substr (pos1);
				vargs.push_back (res);
			}
			break;
		}
		if(pos1 != pos2)
		{
			string res = rawargs.substr (pos1, pos2-pos1);
			vargs.push_back (res);
		}
		pos1 = pos2+1;
	}

	int i;
	int size;
	if(vargs.size()>47) size = 47;
	else size = vargs.size();

	args[0] = exe.c_str ();
	for(i = 0; i < size; i++)
	{
		args[i+1] = vargs[i].c_str ();
	}
	args[i+1] = NULL;

	// go in the good path before launching
	if (!path.empty())
		_chdir (path.c_str ());

	// execute, should better use CreateProcess()
	if (_execvp (exe.c_str(), args) == -1)
	{
		// error occurs during the launch
		string str = toString("Can't execute '%s': code=%d %s", exe.c_str(), errno, strerror(errno));
		MessageBox (str.c_str(), "Error");
	}
	exit(0);
}

void CNel_launcherDlg::patch (const string &str)
{
	string url = "http://"+ConfigFile.getVar ("StartupHost").asString();

	string serverVersion = getValue (str, "serverVersion");

	string urlok = url;
	urlok += getValue (str, "nelUrl");
	urlok += "&newClientVersion=";
	urlok += serverVersion;

	string urlfailed = url;
	urlfailed += getValue (str, "nelUrlFailed");
	urlfailed += "&reason=";

	startPatchThread (getValue (str, "nelServerPath"), serverVersion, urlok, urlfailed, "<br>");

	SetTimer(0,100,NULL);
	
/*	try
	{
		string serverVersion = getValue (str, "serverVersion");
		patchVersion (getValue (str, "nelServerPath"), serverVersion);

		// this url is where we have to go after patching success
		url += getValue (str, "nelUrl");
		url += "&newClientVersion=";
		url += serverVersion;
	}
	catch (Exception &e)
	{
		nlwarning ("Patching failed '%s'", e.what());

		url += getValue (str, "nelUrlFailed");
		url += "&reason=";
		url += e.what ();
	}

	// go to the next url
	openUrl (url.c_str());*/
}

void CNel_launcherDlg::OnTimer(UINT nIDEvent) 
{
	string state, log;
	if (patchState(state, log))
	{
		LPDISPATCH lpDispatch;
		lpDispatch = m_explore.GetDocument();
		if (lpDispatch == NULL) return;

		IHTMLDocument2* pHTMLDocument2;
		HRESULT hr;
		hr = lpDispatch->QueryInterface(IID_IHTMLDocument2, (LPVOID*) &pHTMLDocument2);
		lpDispatch->Release();
		if (FAILED(hr)) return;
		if (pHTMLDocument2 == NULL) return;

		IHTMLElement* pBody;
		hr = pHTMLDocument2->get_body(&pBody);
		if (FAILED(hr)) return;
		if (pBody == NULL) return;

		BSTR bstr;
		pBody->get_innerHTML(&bstr);
		CString csourceCode( bstr );
		string htmlCode( (LPCSTR)csourceCode );
		SysFreeString(bstr);

		string tokenStart = "<!--nel_start_state-->";
		string tokenEnd = "<!--nel_end_state-->";

		int pos = csourceCode.Find (tokenStart.c_str());
		if (pos == -1) return;
		int pos2 = csourceCode.Find (tokenEnd.c_str());
		if (pos2 == -1) return;

		CString oldstate = csourceCode.Mid (pos, pos2-pos);

		if (csourceCode.Replace (oldstate, string(tokenStart+state).c_str ()) == 0)
			return;

		string tokenStartLog = "<!--nel_start_log-->";
		string tokenEndLog = "<!--nel_end_log-->";

		int pos3 = csourceCode.Find (tokenStartLog.c_str());
		if (pos3 == -1) return;

		int pos4 = csourceCode.Find (tokenEndLog.c_str());
		if (pos4 == -1) return;

		CString oldlog = csourceCode.Mid (pos3, pos4-pos3);

		if (csourceCode.Replace (oldlog, string(tokenStartLog+log).c_str ()) == 0)
			return;

	    pBody->put_innerHTML(csourceCode.AllocSysString ());	//insert the html

		pBody->Release();
	}

	string url;
	bool res;
	if (patchEnded(url, res))
	{
		nlinfo ("finnish");
		KillTimer (0);
		if (res)
			MessageBox ("Patch completed succesfuly.", "patch");
		else
			MessageBox ("Patch could not be completed due to an error.", "patch");
		openUrl (url.c_str());
	}
}


void CNel_launcherDlg::OnDocumentCompleteExplorer1(LPDISPATCH pDisp, VARIANT FAR* URL) 
{
	IHTMLDocument2* pHTMLDocument2;
	LPDISPATCH lpDispatch;
	lpDispatch = m_explore.GetDocument();

    if (lpDispatch)
	{
		HRESULT hr;
		hr = lpDispatch->QueryInterface(IID_IHTMLDocument2, (LPVOID*) &pHTMLDocument2);
		lpDispatch->Release();

		if (FAILED(hr))
			return;

		if (pHTMLDocument2 == NULL)
			return;

		IHTMLElement* pBody;
		hr = pHTMLDocument2->get_body(&pBody);

		if (FAILED(hr))
			return;

		if (pBody == NULL)
			return;

		BSTR bstr;
		pBody->get_innerHTML(&bstr);
		CString csourceCode( bstr );
		string str( (LPCSTR)csourceCode );

		SysFreeString(bstr);
		pBody->Release();


		// now I have the web page, look if there's something interesting in it.

		// if something start with <!--nel it s cool
		string action = getValue (str, "<!--nel");

		if (action.empty ())
			return;

		string token = "<!--nel";
		uint spos = str.find (token);
		uint spos2 = str.find ("-->", spos+token.size ()+1);
		string comment = str.substr (spos+token.size (), spos2-spos-token.size ());

		if (action=="launch")
			launch (comment);
		else if (action=="patch")
			patch (comment);
		return;
	}
}






















