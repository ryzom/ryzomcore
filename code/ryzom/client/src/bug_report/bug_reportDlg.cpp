// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// http://www.fsck.com/pub/rt/contrib/2.0/rt-addons/enhanced-mailgate.README

// bug_reportDlg.cpp : implementation file
//

#include "stdafx.h"

#include "bug_report.h"
#include "bug_reportDlg.h"
#include "EmailDlg.h"

#undef min
#undef max

#include "nel/misc/system_info.h"
#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/file.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/path.h"
#include "nel/net/email.h"




using namespace std;
using namespace NLMISC;
using namespace NLNET;





//
// Functions helpers
//

string CBug_reportDlg::checkEmail (const string &email)
{
	// check email value
	string mailerr;
	if (email.size() == 0)
	{
		mailerr += "Email can't be empty\r\n";
	}
	bool at=false;
	for (uint i = 0; i < email.size(); i++)
	{
		if (email[i]=='@')
		{
			if(at)
			{
				mailerr += "Email can have only one @\r\n";
			}
			at=true;
		}
		if (!isalnum(email[i]) && email[i]!='-' && email[i]!='.' && email[i]!='_' && email[i]!='@')
		{
			mailerr += toString("character '%c' is not allowed in email\r\n", email[i]);
		}
	}
	if (!at)
	mailerr += "Email don't have @ symbol\r\n";

	return mailerr;
}

string CBug_reportDlg::askEmail ()
{
	CEmailDlg emailDlg;
	while (true)
	{
		if (emailDlg.DoModal() == IDOK)
		{
			//				email.UpdateData(true);
			string email = emailDlg.m_email;
			string res = checkEmail (email);
			if (res.empty())
			{
				return email;
			}
			else
			{
				res = "Bad email format:\r\n\r\n" + res;
				MessageBox (res.c_str(), "Warning");
			}
		}
	}
}

void CBug_reportDlg::createConfigFile ()
{
	string email = askEmail ();
	FILE *fp = fopen ("bug_report.cfg", "w");
	if (fp != NULL)
	{
		fputs ("// Your email address\n", fp);
		fprintf (fp, "FromEMail = \"%s\";\n", email.c_str());
		fputs ("\n", fp);
		fputs ("// You should not change these variables\n", fp);
		fputs ("ToEMail = \"bug@itsalive.nevrax.org\";\n", fp);
		fputs ("SMTPServer = \"itsalive.nevrax.org\";\n", fp);
		fclose (fp);
	}
}



//
//
//

string CBug_reportDlg::get (int nID)
{
	char val[10000];
	GetDlgItemText(nID, val, 10000-1);

	if (strlen(val) == 0)
		setFocus (nID);

	return val;
}

void CBug_reportDlg::set (int nID, const string &val, bool enable)
{
	CWnd *wnd = (CWnd *)GetDlgItem (nID);

	if (wnd == NULL)
		return;

	char name[1024];
	GetClassName (wnd->m_hWnd, name, 1023);

	if (string(name) == "ComboBox")
	{
		if (val.empty())
			((CComboBox *) wnd)->SetCurSel (-1);
		else
			((CComboBox *) wnd)->SelectString (-1, val.c_str());
	}
	else
		SetDlgItemText(nID, val.c_str());

	wnd->EnableWindow (enable);
}


void CBug_reportDlg::setFocus (int nID)
{
	CWnd *wnd = GetDlgItem (nID);
	if (wnd != NULL)
		wnd->SetFocus();
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}

string getGraphicCardInformation ()
{
	uint width = 128;
	uint height = 128;
	HWND _hWnd = NULL;
	PIXELFORMATDESCRIPTOR _pfd;
	uint8 _Depth = 0;
	int pf;
	WNDCLASS		wc;

	static bool reg = false;

	if (!reg)
	{
		memset(&wc,0,sizeof(wc));
		wc.style			= CS_HREDRAW | CS_VREDRAW ;//| CS_DBLCLKS;
		wc.lpfnWndProc		= (WNDPROC)WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetModuleHandle(NULL);
		wc.hIcon			= NULL;
		wc.hCursor			= LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground	= WHITE_BRUSH;
		wc.lpszClassName	= "BRClass";
		wc.lpszMenuName		= NULL;
		if ( !RegisterClass(&wc) )
			return "<NoInfo>";

		reg = true;
	}

	ULONG WndFlags=WS_OVERLAPPEDWINDOW+WS_CLIPCHILDREN+WS_CLIPSIBLINGS;
	WndFlags&=~WS_VISIBLE;
	RECT	WndRect;
	WndRect.left=0;
	WndRect.top=0;
	WndRect.right=width;
	WndRect.bottom=height;
	AdjustWindowRect(&WndRect,WndFlags,FALSE);
	HWND tmpHWND = CreateWindow(	"BRClass",
								"",
								WndFlags,
								CW_USEDEFAULT,CW_USEDEFAULT,
								WndRect.right,WndRect.bottom,
								NULL,
								NULL,
								GetModuleHandle(NULL),
								NULL);
	if (!tmpHWND)
	{
		nlwarning ("CDriverGL::setDisplay: CreateWindow failed");
		return "<NoInfo>";
	}

	// resize the window
	RECT rc;
	SetRect (&rc, 0, 0, width, height);
	AdjustWindowRectEx (&rc, GetWindowStyle (_hWnd), GetMenu (_hWnd) != NULL, GetWindowExStyle (_hWnd));
	SetWindowPos (_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

	// Get the
	HDC tempHDC = GetDC(tmpHWND);
	_Depth=GetDeviceCaps(tempHDC,BITSPIXEL);

	// ---
	memset(&_pfd,0,sizeof(_pfd));
	_pfd.nSize        = sizeof(_pfd);
	_pfd.nVersion     = 1;
	_pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	_pfd.iPixelType   = PFD_TYPE_RGBA;
	_pfd.cColorBits   = (char)_Depth;

	// Choose best suited Depth Buffer.
	if(_Depth<=16)
	{
		_pfd.cDepthBits   = 16;
	}
	else
	{
		_pfd.cDepthBits = 24;
		_pfd.cAlphaBits	= 8;
	}
	_pfd.iLayerType	  = PFD_MAIN_PLANE;
	pf=ChoosePixelFormat(tempHDC,&_pfd);
	if (!pf)
	{
		nlwarning ("CDriverGL::setDisplay: ChoosePixelFormat failed");
		DestroyWindow (tmpHWND);
		return "<NoInfo>";
	}
	if ( !SetPixelFormat(tempHDC,pf,&_pfd) )
	{
		nlwarning ("CDriverGL::setDisplay: SetPixelFormat failed");
		DestroyWindow (tmpHWND);
		return "<NoInfo>";
	}

	// Create gl context
	HGLRC tempGLRC = wglCreateContext(tempHDC);
	if (tempGLRC == NULL)
	{
		DWORD error = GetLastError ();
		nlwarning ("CDriverGL::setDisplay: wglCreateContext failed: 0x%x", error);
		DestroyWindow (tmpHWND);
		_hWnd = NULL;
		return "<NoInfo>";
	}

	// Make the context current
	if (!wglMakeCurrent(tempHDC,tempGLRC))
	{
		DWORD error = GetLastError ();
		nlwarning ("CDriverGL::setDisplay: wglMakeCurrent failed: 0x%x", error);
		wglDeleteContext (tempGLRC);
		DestroyWindow (tmpHWND);
		_hWnd = NULL;
		return "<NoInfo>";
	}

	const char *vendor = (const char *) glGetString (GL_VENDOR);
	const char *renderer = (const char *) glGetString (GL_RENDERER);
	const char *version = (const char *) glGetString (GL_VERSION);

	string res = string(vendor) + string(" ") + string(renderer) + string(" ") + string(version);

	wglDeleteContext (tempGLRC);
	DestroyWindow (tmpHWND);

	return res;
}

static string		soundCards;

BOOL CALLBACK CSoundDriverDSoundEnumCallback(LPGUID guid, LPCSTR description, PCSTR module, LPVOID context)
{
    soundCards += description + string("\r\n");
    return TRUE;
}

// ******************************************************************

string getSoundCardInformation()
{
	soundCards= "";

    if (FAILED(DirectSoundEnumerate(CSoundDriverDSoundEnumCallback, NULL)))
	{
		return "no sound card found";
	}

	return soundCards;
}


void CBug_reportDlg::setDumpFilename (const std::string &DumpFilename)
{
	string s;

	if (!DumpFilename.empty())
	{
		FILE *fp = fopen (DumpFilename.c_str(), "rb");
		if (fp != NULL)
		{
			char buf[10000];
			int end = fread (buf, 1, 10000-1, fp);
			buf[end] = '\0';
			s = buf;
			fclose (fp);
		}
	}

	s += "Memory: " + bytesToHumanReadable(CSystemInfo::availablePhysicalMemory()) + "/" + bytesToHumanReadable(CSystemInfo::totalPhysicalMemory()) + "\r\n";
	s += CSystemInfo::getOS() + "\r\n";
	s += CSystemInfo::getProc() + "\r\n";
	s += getGraphicCardInformation () + "\r\n";
	s += getSoundCardInformation() + "\r\n";
	set (IDC_DESCRIPTIONEX, s, true);
}


/////////////////////////////////////////////////////////////////////////////
// CBug_reportDlg dialog

CBug_reportDlg::CBug_reportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBug_reportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBug_reportDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBug_reportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBug_reportDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBug_reportDlg, CDialog)
	//{{AFX_MSG_MAP(CBug_reportDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDSENDREPORT, OnSendreport)
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_BN_CLICKED(IDC_ATTACHFILE, OnAttachfile)
	ON_BN_CLICKED(IDC_NOATTACH, OnNoattach)
	ON_BN_CLICKED(IDSENDREPORTWITHSS, OnSendreportwithss)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBug_reportDlg message handlers

BOOL CBug_reportDlg::OnInitDialog()
{
	setReportEmailFunction ((void*)sendEmail);
	setDefaultEmailParams ("gw.nevrax.com", "", "lecroart@nevrax.com");

	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	OnClear ();

	char *cmd = GetCommandLine ();

	// skip the program name
	if (cmd[0] == '"')
		cmd = strchr (cmd+1, '"') + 1;
	else
		cmd = strchr (cmd, ' ') + 1;

	while(true)
	{
		char *token1;
		static bool first = true;
		if (first)
		{
			token1 = strtok( cmd, " " );
			first = false;
		}
		else
			token1 = strtok( NULL, " " );

		if (token1 == NULL)
			break;

		char *token2 = strtok( NULL, " " );
		if (token2 == NULL)
			break;

		if (string(token1) == "ClientVersion")
			set (IDC_CLIENTVERSION, token2);
		else if (string(token1) == "ShardVersion")
			set (IDC_SHARDVERSION, token2);
		else if (string(token1) == "ShardName")
			set (IDC_SHARDNAME, token2);
		else if (string(token1) == "AttachedFile")
			set (IDC_ATTACHEDFILE, token2);
		else if (string(token1) == "Language")
			set (IDC_LANGUAGE, token2);
		else if (string(token1) == "DumpFilename")
			setDumpFilename (token2);
		else
			MessageBox ("Bad command line option", token1);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBug_reportDlg::OnPaint()
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
HCURSOR CBug_reportDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CBug_reportDlg::sendReport (bool withScreenshot)
{
	string body, res, requestor, mailerr;

	requestor = get(IDC_REQUESTOR);	if (requestor.empty()) { MessageBox ("Field Requestor must be filled", "Warning"); return; } body += "RT-Requestor: " + requestor + "\n";

	// check email value
	mailerr = checkEmail (requestor);

	if (!mailerr.empty())
	{
		mailerr = "Bad email format, please modify it on \"bug_report.cfg\":\r\n\r\n" + mailerr;
		MessageBox (mailerr.c_str(), "Warning");
		setFocus (IDC_REQUESTOR);
		return;
	}

	res = get(IDC_LANGUAGE);	if (res.empty()) { MessageBox ("Field Language must be filled", "Warning"); return; } body += "RT-Language: " + res + "\n";
	res = get(IDC_SUBJECT);		if (res.empty()) { MessageBox ("Field Subject must be filled", "Warning"); return; } body += "RT-Subject: " + res + "\n";
	res = get(IDC_CLIENTVERSION);	if (res.empty()) { MessageBox ("Field ClientVersion must be filled", "Warning"); return; } body += "RT-ClientVersion: " + res + "\n";
	res = get(IDC_SHARDNAME);	if (res.empty()) { MessageBox ("Field ShardName must be filled", "Warning"); return; } body += "RT-ShardName: " + res + "\n";
	res = get(IDC_SHARDVERSION);	if (res.empty()) { MessageBox ("Field ShardVersion must be filled", "Warning"); return; } body += "RT-ShardVersion: " + res + "\n";
	res = get(IDC_SKU);			if (res.empty()) { MessageBox ("Field SKU must be filled", "Warning"); return; } body += "RT-SKU: " + res + "\n";
	res = get(IDC_CLASS);		if (res.empty()) { MessageBox ("Field Class must be filled", "Warning"); return; } body += "RT-Class: " + res + "\n";
	res = get(IDC_CATEGORY);	if (res.empty()) { MessageBox ("Field Category must be filled", "Warning"); return; } body += "RT-Category: " + res + "\n";
	res = get(IDC_FREQUENCY);	if (res.empty()) { MessageBox ("Field Frequency must be filled", "Warning"); return; } body += "RT-Frequency: " + res + "\n";
	res = get(IDC_QUEUE);		if (res.empty()) { MessageBox ("Field Queue must be filled", "Warning"); return; } body += "RT-Queue: " + res + "\n";

	body += "\n";
	body += get(IDC_DESCRIPTION) + "\n\n";
	body += "<b>Reproduction Steps</b>\n\n" + get(IDC_REPRODUCESTEPS) + "\n\n";
	body += "<b>Extended Description</b>\n\n" + get(IDC_DESCRIPTIONEX) + "\n";

	string attachedFile;

	if (withScreenshot)
	{
		attachedFile = get(IDC_ATTACHEDFILE);
	}

	//	if (sendEmail ("192.168.254.1", "lecroart@nevrax.com", "piuzzi@nevrax.com", "", body, attachedFile, false))
	//	if (sendEmail ("192.168.254.1", "lecroart@nevrax.com", "lecroart@nevrax.com", "", body, attachedFile, false))

	if (attachedFile.empty())
		nlinfo ("--- sending email:");
	else
		nlinfo ("--- sending email with attached file '%s':", attachedFile.c_str());

	nlinfo ("%s", body.c_str());
	nlinfo ("--- end sending email");

	if (sendEmail (SMTPServer, requestor, ToEmail, "", body, attachedFile, false))
	{
		if (MessageBox ("Report successfully sent. Thank you for your help\r\n\r\nDo you want to quit the bug report tool now?", "Success", MB_YESNO) == IDYES)
		{
			exit (0);
		}
		else
		{
			setFocus (IDC_CLEAR);
		}
	}
	else
	{
		MessageBox ("An error occured when sending the report. Report wasn't sent, look log.log for more info", "Error");
	}

}

void CBug_reportDlg::OnSendreport()
{
	sendReport (false);
}


void CBug_reportDlg::OnSendreportwithss()
{
	sendReport (true);
}


void CBug_reportDlg::OnClear()
{
	set(IDC_LANGUAGE, "english");
	set(IDC_SUBJECT, "");
	set(IDC_CLIENTVERSION, "");
	set(IDC_SHARDNAME, "Beta");
	set(IDC_SHARDVERSION, "");
	set(IDC_SKU, "Beta2");
	set(IDC_CLASS, "");
	set(IDC_CATEGORY, "");
	set(IDC_FREQUENCY, "Once");
	set(IDC_DESCRIPTION, "");
	set(IDC_REPRODUCESTEPS, "");
	set(IDC_QUEUE, "ryzom");
	set(IDC_ATTACHEDFILE, "");

/*
/////// TEMP pre fill entries
	set(IDC_LANGUAGE, "english");
	set(IDC_SUBJECT, "subject");
	set(IDC_CLIENTVERSION, "0.8.0.360");
	set(IDC_SHARDNAME, "OFFLINE");
	set(IDC_SHARDVERSION, "0.8.0.360");
	set(IDC_SKU, "Beta2");
	set(IDC_CLASS, "A");
	set(IDC_CATEGORY, "Hardware");
	set(IDC_FREQUENCY, "always");
	set(IDC_DESCRIPTION, "description");
	set(IDC_REPRODUCESTEPS, "reproduction steps");
	set(IDC_QUEUE, "zz-ryzom-dummy");
	set(IDC_ATTACHEDFILE, "C:\\Documents and Settings\\nouveau\\Desktop\\pez.bmp");
/////// END TEMP
*/

	if (!NLMISC::CFile::fileExists("bug_report.cfg"))
	{
		createConfigFile ();
	}

	try
	{
		// try to get the client version from the client config file
		CConfigFile cf;
		cf.load ("client.cfg");

		set(IDC_CLIENTVERSION, cf.getVar ("ClientVersion").asString ());
		set(IDC_SHARDVERSION, cf.getVar ("ClientVersion").asString ());
	}
	catch (Exception &)
	{
		nlwarning ("Can't find ClientVersion variable in client.cfg");
	}

	try
	{
		CConfigFile cf;
		cf.load ("bug_report.cfg");

		set(IDC_REQUESTOR, cf.getVar ("FromEMail").asString ());

		SMTPServer = cf.getVar ("SMTPServer").asString ();
		ToEmail = cf.getVar ("ToEMail").asString ();
	}
	catch (Exception &e)
	{
		MessageBox (e.what(), "Error");
		exit (0);
	}

	setDumpFilename("");

	setFocus (IDC_SUBJECT);
}

void CBug_reportDlg::OnAttachfile()
{
	CFileDialog fd(true);
	if (fd.DoModal () == IDOK)
		set (IDC_ATTACHEDFILE, string(fd.GetPathName()), true);
}

void CBug_reportDlg::OnNoattach()
{
	set (IDC_ATTACHEDFILE, "", true);
}
