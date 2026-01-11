// NeLNS - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "std_afx.h"

#include <nel/misc/config_file.h>

#include "nel_launcher.h"
#include "nel_launcher_dlg.h"
#include "connection.h"
#include "patch.h"

/////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace NLMISC;

CConfigFile ConfigFile;
bool VerboseLog = false;

static CFileDisplayer NLFileDisplayer("nel_launcher.log", true);
static bool DispayerAdded = false;

void quit()
{
	if(DispayerAdded)
	{
		createDebug();
		DebugLog->removeDisplayer(&NLFileDisplayer);
		InfoLog->removeDisplayer(&NLFileDisplayer);
		WarningLog->removeDisplayer(&NLFileDisplayer);
		ErrorLog->removeDisplayer(&NLFileDisplayer);
		AssertLog->removeDisplayer(&NLFileDisplayer);
	}
	
	exit(0);
}

/////////////////////////////////////////////////////////////////////////////
// CNeLLauncherDlg dialog

CNeLLauncherDlg::CNeLLauncherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNeLLauncherDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNeLLauncherDlg)
	Login = _T("");
	Password = _T("");
	PatchStatus = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SelectedShardId = 0xFFFFFFFF;
}

void CNeLLauncherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNeLLauncherDlg)
	DDX_Control(pDX, IDC_PATCH_PROGRESS, PatchProgress);
	DDX_Control(pDX, IDC_SHARDS, ShardsList);
	DDX_Text(pDX, IDC_LOGIN, Login);
	DDX_Text(pDX, IDC_PASSWORD, Password);
	DDX_Text(pDX, IDC_PATCH_STATUS, PatchStatus);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNeLLauncherDlg, CDialog)
	//{{AFX_MSG_MAP(CNeLLauncherDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_LOGIN, OnLogin)
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(NM_DBLCLK, IDC_SHARDS, OnDblclkShards)
	ON_BN_CLICKED(ID_CONNECT, OnConnect)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SIGNUP, OnSignup)
	ON_BN_CLICKED(IDC_ATYSNEWS, OnAtysnews)
	ON_BN_CLICKED(IDC_PLAYGUIDE, OnPlayguide)
	ON_BN_CLICKED(IDC_RELEASENOTE, OnReleasenote)
	ON_BN_CLICKED(IDC_LIVESUPPORT, OnLivesupport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNeLLauncherDlg message handlers

BOOL CNeLLauncherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	createDebug();
	DebugLog->addDisplayer(&NLFileDisplayer);
	InfoLog->addDisplayer(&NLFileDisplayer);
	WarningLog->addDisplayer(&NLFileDisplayer);
	ErrorLog->addDisplayer(&NLFileDisplayer);
	AssertLog->addDisplayer(&NLFileDisplayer);
	DispayerAdded = true;

	nlinfo("Loading config file");

	ConfigFile.load("nel_launcher.cfg");

	if(ConfigFile.exists("VerboseLog"))
		VerboseLog = (ConfigFile.getVar("VerboseLog").asInt() == 1);

	if(VerboseLog) nlinfo("Using verbose log mode");

	GetDlgItem(IDC_SHARDS)->EnableWindow(FALSE);
	GetDlgItem(ID_CONNECT)->EnableWindow(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNeLLauncherDlg::OnPaint() 
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
HCURSOR CNeLLauncherDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CNeLLauncherDlg::OnLogin()
{
	nlinfo("OnLogin called");

	UpdateData(TRUE);
	string l = (LPCTSTR)Login;
	string p = (LPCTSTR)Password;

	if(!l.empty())
		SetRegKey("Login", l);

	GetDlgItem(IDC_LOGIN)->EnableWindow(FALSE);
	GetDlgItem(IDC_PASSWORD)->EnableWindow(FALSE);
	GetDlgItem(ID_LOGIN)->EnableWindow(FALSE);

	// Check the login/pass

	string res = checkLogin(l, p, ConfigFile.getVar("Application").asString(0));
	if(res.empty())
	{
		ShardsList.DeleteAllItems();
		
		ShardsList.InsertColumn(0, _T("Shard Name"), LVCFMT_LEFT, -1);
		ShardsList.InsertColumn(1, _T("Version"), LVCFMT_LEFT, 70);
		ShardsList.InsertColumn(2, _T("Status"), LVCFMT_LEFT, 50);
		ShardsList.InsertColumn(3, _T("NbPlayers"), LVCFMT_LEFT, 70);
		
		for(uint i = 0; i < Shards.size(); i++)
		{
			CString strItem;
			LVITEM lvi;
			// Insert the first item
			lvi.mask = LVIF_TEXT;
			strItem = Shards[i].Name.c_str();
			lvi.iItem = i;
			lvi.iSubItem = 0;
			lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
			int idx = ShardsList.InsertItem(&lvi);
			
			lvi.iItem = idx;
			
			strItem = Shards[i].Version.c_str();
			lvi.iSubItem = 1;
			lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
			ShardsList.SetItem(&lvi);
			
			strItem = Shards[i].Online?"Online":"Offline";
			lvi.iSubItem = 2;
			lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
			ShardsList.SetItem(&lvi);
			
			strItem = toString(Shards[i].NbPlayers).c_str();
			lvi.iSubItem = 3;
			lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
			ShardsList.SetItem(&lvi);
			
			ShardsList.SetItemData(i, Shards[i].ShardId);
		}
		
		ShardsList.SetColumnWidth(0, LVSCW_AUTOSIZE);

		GetDlgItem(IDC_SHARDS)->EnableWindow(TRUE);
		GetDlgItem(ID_CONNECT)->EnableWindow(TRUE);
	}
	else
	{
		MessageBox(res.c_str(), "Error");
		GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_PASSWORD)->EnableWindow(TRUE);
		GetDlgItem(ID_LOGIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_PASSWORD)->SetFocus();
	}
}

static const string RootKey = "SOFTWARE\\Nevrax\\Ryzom";
static const uint32 KeyMaxLength = 1024;

string CNeLLauncherDlg::GetRegKeyValue(const string &Entry)
{
    HKEY hkey;
	string ret;
	
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, RootKey.c_str(), 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		DWORD	dwType  = 0L;
		DWORD	dwSize	= KeyMaxLength;
		unsigned char	Buffer[KeyMaxLength];
		
		if(RegQueryValueEx(hkey, Entry.c_str(), NULL, &dwType, Buffer, &dwSize) != ERROR_SUCCESS)
		{
			nlwarning("Can't get the reg key '%s'", Entry.c_str());
		}
		else
		{
			ret = (char*)Buffer;
		}
		RegCloseKey(hkey);
	}
	else
	{
		nlwarning("Can't get the reg key '%s'", Entry.c_str());
	}
    return ret;
}

void CNeLLauncherDlg::SetRegKey(const string &ValueName, const string &Value)
{
	HKEY hkey;
	DWORD dwDisp;
	
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, RootKey.c_str(), 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisp) == ERROR_SUCCESS)
    {
		RegSetValueEx(hkey, ValueName.c_str(), 0L, REG_SZ, (const BYTE *)Value.c_str(), (Value.size())+1);
		RegCloseKey(hkey);
	}
	else
	{
		nlwarning("Can't set the reg key '%s' '%s'", ValueName.c_str(), Value.c_str());
	}
}

void CNeLLauncherDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	string l = GetRegKeyValue("Login").c_str();

	if(!l.empty())
	{
		Login = l.c_str();
		UpdateData(FALSE);
		GetDlgItem(IDC_PASSWORD)->SetFocus();
	}
	else
	{
		GetDlgItem(IDC_LOGIN)->SetFocus();
	}
}

void CNeLLauncherDlg::OnDblclkShards(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnConnect();

	*pResult = 0;
}

void CNeLLauncherDlg::OnConnect() 
{
	nlinfo("OnConnect called");

	UpdateData(TRUE);
	string log = (LPCTSTR)Login;
	string pass = (LPCTSTR)Password;

	GetDlgItem(IDC_SHARDS)->EnableWindow(FALSE);
	GetDlgItem(ID_CONNECT)->EnableWindow(FALSE);

	POSITION p = ShardsList.GetFirstSelectedItemPosition();

	if(!p)
	{
		MessageBox("Please, select a shard and then press Connect button", "Information");
		goto end;
	}

	while (p)
	{
		int nSelected = ShardsList.GetNextSelectedItem(p);
		// Do something with item nSelected
		
		TCHAR szBuffer[1024];
		DWORD cchBuf(1024);
		LVITEM lvi;
		lvi.iItem = nSelected;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_TEXT;
		lvi.pszText = szBuffer;
		lvi.cchTextMax = cchBuf;
		ShardsList.GetItem(&lvi);

		uint32 shardId = ShardsList.GetItemData(nSelected);

		uint i;
		for(i = 0; i < Shards.size(); i++)
		{
			if(Shards[i].ShardId == shardId)
			{
				break;
			}
		}

		nlassert(i != Shards.size());

		if(!Shards[i].Online)
		{
			MessageBox("You can't connect to an offline shard (error code 15)", "Error");
			goto end;
		}

		SelectedShardId = shardId;

		if(!Shards[i].Version.empty() && Shards[i].Version != getVersion())
		{
			// We need to patch first

			if(IDCANCEL == MessageBox("You need to patch to access to this shard", "Information", MB_OKCANCEL))
			{
				goto end;
			}

			if(Shards[i].PatchURL.empty())
			{
				MessageBox("You can't patch because the PatchURL isn't setup in database (error code 16)", "Error");
				goto end;
			}

			string urlok, urlfailed;
			startPatchThread(Shards[i].PatchURL, Shards[i].Version, urlok, urlfailed, "\n");

			PatchProgress.EnableWindow(TRUE);
			PatchProgress.SetRange(0, getTotalFilesToGet());
			PatchProgress.SetPos(0);

			SetTimer(0, 100, NULL);
		}
		else
		{
			// Version is good, connect and launch the client
			ConnectToShard();
		}
	}

	return;

end:

	GetDlgItem(IDC_SHARDS)->EnableWindow(TRUE);
	GetDlgItem(ID_CONNECT)->EnableWindow(TRUE);
}

void CNeLLauncherDlg::ConnectToShard()
{
	nlinfo("ConnectToShard called");

	nlassert(SelectedShardId != 0xFFFFFFFF);

	string cookie, addr;
	string res = selectShard(SelectedShardId, cookie, addr);

	if(res.empty())
	{
		LaunchClient(cookie, addr);
	}
	else
	{
		MessageBox(res.c_str(), "Error");
	}
	GetDlgItem(IDC_SHARDS)->EnableWindow(TRUE);
	GetDlgItem(ID_CONNECT)->EnableWindow(TRUE);
}

void CNeLLauncherDlg::LaunchClient(const string &cookie, const string &addr)
{
	nlinfo("LaunchClient called");

	string rapp = ConfigFile.getVar("Application").asString(1);
	string dir = ConfigFile.getVar("Application").asString(2);

//	vector<string> vargs;
	const char *args[50];

//	vargs.push_back(cookie);
//	vargs.push_back(addr);

//	uint i;
	args[0] = rapp.c_str();
/*
	for(i = 0; i < vargs.size(); i++)
	{
		args[i+1] = vargs[i].c_str();
	}
*/
	args[1] = NULL;

	MessageBox("We have made changes to the way you gain access to the game:\r\n - this is the last time you are using the nel launcher\r\n - nel launcher is now merged with the game\r\n - next time you want to play SoR you need to click on this shortcut 'Play Ryzom' or click on 'client_ryzom_d.exe in your ryzom folder\r\n - You will now be taken to the new login area", "Please Read!", MB_ICONEXCLAMATION);

	const char *SE_TRANSLATOR_IN_MAIN_MODULE = "NEL_SE_TRANS";
	TCHAR envBuf [2];
	if ( GetEnvironmentVariable( SE_TRANSLATOR_IN_MAIN_MODULE, envBuf, 2 ) != 0)
	{
		SetEnvironmentVariable( SE_TRANSLATOR_IN_MAIN_MODULE, NULL );
	}

	if(!dir.empty())
		_chdir(dir.c_str());

	// execute, should better use CreateProcess()
	if(_execvp(rapp.c_str(), args) == -1)
	{
		MessageBox("Can't execute the game (error code 17)", "Error");
	}
	else
	{
		quit();
	}
}

void CNeLLauncherDlg::OnTimer(UINT nIDEvent) 
{
	CDialog::OnTimer(nIDEvent);

	PatchProgress.SetRange(0, getTotalFilesToGet());
	PatchProgress.SetPos(getCurrentFilesToGet());

	string state, log;
	if(patchState(state, log))
	{
		PatchStatus = state.c_str();
		UpdateData(FALSE);
	}

	string	url;
	bool	res;

	if(patchEnded(url, res))
	{
		KillTimer(0);
		PatchProgress.EnableWindow(FALSE);
		if(res)
		{
			MessageBox("Patch completed successfully", "Information");
			// Version is now good, connect and launch the client
			ConnectToShard();
		}
		else
		{
			MessageBox(url.c_str(), "Error");
			GetDlgItem(IDC_SHARDS)->EnableWindow(TRUE);
			GetDlgItem(ID_CONNECT)->EnableWindow(TRUE);
		}
	}
}

void CNeLLauncherDlg::OnCancel() 
{
	nlinfo("OnCancel called");

	quit();

	CDialog::OnCancel();
}


LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
    HKEY hkey;
    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);
	
    if (retval == ERROR_SUCCESS) 
	{
        long datasize = MAX_PATH;
        char data[MAX_PATH];
        RegQueryValue(hkey, NULL, data, &datasize);
        lstrcpy(retdata,data);
        RegCloseKey(hkey);
    }
	
    return retval;
}

void GotoURL(LPCTSTR url)
{
    char key[MAX_PATH + MAX_PATH];
	
    // First try ShellExecute()
    HINSTANCE result = ShellExecute(NULL, "open", url, NULL,NULL, SW_SHOW);
	
    // If it failed, get the .htm regkey and lookup the program
    if ((UINT)result <= HINSTANCE_ERROR) {
		
        if (GetRegKey(HKEY_CLASSES_ROOT, ".htm", key) == ERROR_SUCCESS) 
		{
            lstrcat(key, "\\shell\\open\\command");
			
            if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) 
			{
                char *pos;
                pos = strstr(key, "\"%1\"");
                if (pos == NULL) 
				{                     // No quotes found
                    pos = strstr(key, "%1");       // Check for %1, without quotes 
                    if (pos == NULL)                   // No parameter at all...
                        pos = key+lstrlen(key)-1;
                    else
                        *pos = '\0';                   // Remove the parameter
                }
                else
                    *pos = '\0';                       // Remove the parameter
				
                lstrcat(pos, " ");
                lstrcat(pos, url);
                result = (HINSTANCE) WinExec(key, SW_SHOW);
            }
        }
    }
}

void CNeLLauncherDlg::OnSignup() 
{
	if(ConfigFile.exists("SignUpURL"))
		GotoURL(ConfigFile.getVar("SignUpURL").asString().c_str());
	else
		GotoURL("http://www.ryzom.com");
}

void CNeLLauncherDlg::OnAtysnews() 
{
	GotoURL("http://atys.ryzom.com");
}

void CNeLLauncherDlg::OnPlayguide() 
{
	GotoURL("http://atys.ryzom.com/forums/index.php?c=4");
}

void CNeLLauncherDlg::OnReleasenote() 
{
	GotoURL("http://atys.ryzom.com/?page=update_latest");
}

void CNeLLauncherDlg::OnLivesupport() 
{
	GotoURL("http://atys.ryzom.com/?page=beta_chat");
}
