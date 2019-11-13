// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

// branch_patcherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "branch_patcher.h"
#include "branch_patcherDlg.h"
#include "shlobj.h"
#include "direct.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBranch_patcherDlg dialog

extern CBranch_patcherApp theApp;

const CString TEMP_DIFF_FILE = "C:\\tempFile.diff";
const CString DIFF_ERRORS = "C:\\diffLog.txt";
const CString PATCH_RESULT = "C:\\patchResult.txt";
const CString PATCH_ERRORS = "C:\\patchErrors.txt";


CBranch_patcherDlg::CBranch_patcherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBranch_patcherDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBranch_patcherDlg)
	m_SrcDir = _T("");
	m_DestDir = _T("");
	m_Filename = _T("");
	m_Tokens = _T("");
	m_SrcDirLabel = _T("");
	m_TargetDirLabel = _T("");
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_Display = NULL;
}

void CBranch_patcherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBranch_patcherDlg)
	DDX_Text(pDX, IDC_SRCDIR, m_SrcDir);
	DDX_Text(pDX, IDC_DESTDIR, m_DestDir);
	DDX_Text(pDX, IDC_Filename, m_Filename);
	DDX_Text(pDX, IDC_CurrentTokens, m_Tokens);
	DDX_Text(pDX, IDC_SrcDirLabel, m_SrcDirLabel);
	DDX_Text(pDX, IDC_TargetDirLabel, m_TargetDirLabel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBranch_patcherDlg, CDialog)
	//{{AFX_MSG_MAP(CBranch_patcherDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_ButtonSetSrcDir, OnButtonSetSrcDir)
	ON_BN_CLICKED(IDC_ButtonSetDestDir, OnButtonSetDestDir)
	ON_BN_CLICKED(IDC_ButtonPatch, OnButtonPatch)
	ON_BN_CLICKED(IDC_DoPatch, OnDoPatch)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_ButtonExtractTokens, OnButtonExtractTokens)
	ON_BN_CLICKED(IDC_ButtonClearTokens, OnButtonClearTokens)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBranch_patcherDlg message handlers

BOOL CBranch_patcherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Extra initialization here

	RECT cltRect;
	GetClientRect(&cltRect),
		m_Display = new CRichEditCtrl();
	m_Display->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE,
		CRect(20, 180, cltRect.right - 20, cltRect.bottom - 20), this, 1);

	// Initialize directories
	loadConfiguration();
	processCommandLine();
	displayTokens();

	EnteringTokens = false;
	m_SrcDirLabel = "Source Dir";
	m_TargetDirLabel = "Target Dir";
	UpdateData(false);
	((CButton*)GetDlgItem(IDC_DoPatch))->EnableWindow(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBranch_patcherDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

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

HCURSOR CBranch_patcherDlg::OnQueryDragIcon()
{
	return (HCURSOR)m_hIcon;
}


void	CBranch_patcherDlg::setSrcDirectory(const CString& s)
{
	m_SrcDir = s;
	UpdateData(false);
}

void	CBranch_patcherDlg::setDestDirectory(const CString& s)
{
	m_DestDir = s;
	UpdateData(false);
}

void CBranch_patcherDlg::OnButtonSetSrcDir()
{
	DirDialog.m_strTitle = "Please choose the SOURCE directory";
	if (DirDialog.DoBrowse() == TRUE)
	{
		setSrcDirectory(DirDialog.m_strPath);
		guessDestDirectory();
	}
}

void CBranch_patcherDlg::OnButtonSetDestDir()
{
	DirDialog.m_strTitle = "Please choose the TARGET directory";
	if (DirDialog.DoBrowse() == TRUE)
	{
		setDestDirectory(DirDialog.m_strPath);
	}
}








CDirDialog::CDirDialog()
{////////////////////////////////////////////

}

CDirDialog::~CDirDialog()
{///////////////////////////////////////////

}

int CDirDialog::DoBrowse()
{/////////////////////////////////////////

	LPMALLOC pMalloc;
	if (SHGetMalloc(&pMalloc) != NOERROR)
	{
		return 0;
	}

	BROWSEINFO bInfo;
	LPITEMIDLIST pidl;
	ZeroMemory((PVOID)&bInfo, sizeof(BROWSEINFO));

	if (!m_strInitDir.IsEmpty())
	{
		OLECHAR       olePath[MAX_PATH];
		ULONG         dwAttributes = 0;
		HRESULT       hr;
		LPSHELLFOLDER pDesktopFolder;
		// // Get a pointer to the Desktop's IShellFolder interface. //
		if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
		{
#ifndef _UNICODE
			//
			// IShellFolder::ParseDisplayName requires the file name be in Unicode.
			//
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_strInitDir.GetBuffer(MAX_PATH), -1, olePath, MAX_PATH);

			m_strInitDir.ReleaseBuffer(-1);
#else
			wcscpy(olePath, (LPCTSTR)m_strInitDir);
#endif
			//
			// Convert the path to an ITEMIDLIST.
			//
			hr = pDesktopFolder->ParseDisplayName(NULL,
				NULL,
				olePath,
				NULL,
				&pidl,
				&dwAttributes);

			if (FAILED(hr))
			{
				pMalloc->Free(pidl);
				pMalloc->Release();
				return 0;
			}

			bInfo.pidlRoot = pidl;
		}
	}

	bInfo.hwndOwner = NULL;
	bInfo.pszDisplayName = m_strPath.GetBuffer(MAX_PATH);
	bInfo.lpszTitle = (m_strTitle.IsEmpty()) ? "Open" : m_strTitle;
	bInfo.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;

	if ((pidl = ::SHBrowseForFolder(&bInfo)) == NULL)
	{
		return 0;
	}

	m_strPath.ReleaseBuffer();
	m_iImageIndex = bInfo.iImage;

	if (::SHGetPathFromIDList(pidl, m_strPath.GetBuffer(MAX_PATH)) == FALSE)
	{
		pMalloc->Free(pidl);
		pMalloc->Release();
		return 0;
	}

	m_strPath.ReleaseBuffer();

	pMalloc->Free(pidl);
	pMalloc->Release();
	return 1;
}


/*
 * Adapted from function by Jonah Bishop <jonahb@nc.rr.com>
 */
BOOL SendTextToClipboard(CString source)
{
	// Return value is TRUE if the text was sent
	// Return value is FALSE if something went wrong
	if (OpenClipboard(NULL))
	{
		HGLOBAL clipbuffer;
		TCHAR* buffer;

		EmptyClipboard(); // Empty whatever's already there

		clipbuffer = GlobalAlloc(GMEM_DDESHARE, ((SIZE_T)source.GetLength() + 1) * sizeof(TCHAR));
		if (clipbuffer)
		{
			buffer = (TCHAR*)GlobalLock(clipbuffer);
			if (buffer)
			{
				_tcscpy(buffer, LPCTSTR(source));
				GlobalUnlock(clipbuffer);

				SetClipboardData(CF_TEXT, clipbuffer); // Send the data

				CloseClipboard(); // VERY IMPORTANT
				return TRUE;
			}
		}
		CloseClipboard(); // VERY IMPORTANT
	}
	return FALSE;
}


void CBranch_patcherDlg::displayMessage(const CString& msg, bool insertAtTop)
{
	if (insertAtTop)
		m_Display->SetSel(0, 0);
	else
		m_Display->SetSel(0, -1);
	m_Display->ReplaceSel(msg);
	SaveDiff = false;
}


void CBranch_patcherDlg::OnButtonPatch()
{
	UpdateData(true);

	CString diffCmdLine;
	diffCmdLine.Format(_T("cvs.exe diff -c > %s 2> %s"), TEMP_DIFF_FILE, DIFF_ERRORS); // needs a valid cvs login before! and cvs.exe in the path
	CString text;
	text.Format(_T("Get diff from directory %s?\n\nCommand (choose No to copy it into the clipboard):\n%s"), m_SrcDir, diffCmdLine);
	int result;
	if ((result = ::MessageBox(m_hWnd, text, _T("Confirmation"), MB_YESNOCANCEL | MB_ICONQUESTION)) == IDYES)
	{
		if (_tchdir(m_SrcDir) == 0)
		{
			_tsystem(diffCmdLine);
			displayFile(TEMP_DIFF_FILE);
			SaveDiff = true;
			colorizeDiff();
			m_Display->LineScroll(0);
			((CButton*)GetDlgItem(IDC_DoPatch))->EnableWindow(TRUE);

			if ((m_Display->GetLineCount() == 0) ||
				(m_Display->GetLineCount() == 1 && m_Display->LineLength(0) < 2))
			{
				displayFile(DIFF_ERRORS);
				displayMessage("Diff is empty.\r\nIf this is not the expected result:\r\n- check if the source directory is part of a CVS tree\r\n- check if cvs.exe is in your PATH\r\n- check if you are logged to the cvs server with 'cvs login' (set your home cvs directory in the HOME environment variable if needed)\r\n- check if C:\\ has enough free space and access rights to write a file.\n\nHere is the log:\n\n", true);
			}
			else
			{
				m_Filename = TEMP_DIFF_FILE + ":";
				UpdateData(false);
			}
		}
		else
		{
			displayMessage("Source directory not found");
		}
	}
	else if (result == IDNO)
	{
		SendTextToClipboard(diffCmdLine);
	}
}


static unsigned long CALLBACK MyStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	CFile* pFile = (CFile*)dwCookie;
	*pcb = pFile->Read(pbBuff, cb);
	return 0;
}


static unsigned long CALLBACK MyStreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	CFile* pFile = (CFile*)dwCookie;
	pFile->Write(pbBuff, cb);
	*pcb = cb;
	return 0;
}


void CBranch_patcherDlg::displayFile(const CString& filename)
{
	CFile cFile(filename, CFile::modeRead);
	EDITSTREAM es;
	es.dwCookie = (DWORD_PTR)&cFile;
	es.pfnCallback = MyStreamInCallback;
	m_Display->StreamIn(SF_TEXT, es);
}


void CBranch_patcherDlg::saveFile(const CString& filename)
{
	CFile cFile(filename, CFile::modeCreate | CFile::modeWrite);
	EDITSTREAM es;
	es.dwCookie = (DWORD_PTR)&cFile;
	es.pfnCallback = MyStreamOutCallback;
	m_Display->StreamOut(SF_TEXT, es);
}


void CBranch_patcherDlg::colorizeDiff()
{
	CHARFORMAT blue;
	ZeroMemory(&blue, sizeof(blue));
	blue.cbSize = sizeof(blue);
	blue.dwMask = CFM_COLOR;
	blue.crTextColor = RGB(0, 0, 0xFF);
	CHARFORMAT red;
	ZeroMemory(&red, sizeof(red));
	red.cbSize = sizeof(red);
	red.dwMask = CFM_COLOR;
	red.crTextColor = RGB(0xFF, 0, 0);
	CHARFORMAT green;
	ZeroMemory(&green, sizeof(green));
	green.cbSize = sizeof(green);
	green.dwMask = CFM_COLOR;
	green.crTextColor = RGB(0, 0x7F, 0);
	for (int i = 0; i != m_Display->GetLineCount(); ++i)
	{
		int c = m_Display->LineIndex(i);
		int l = m_Display->LineLength(c);
		m_Display->SetSel(c, c + l);
		CString s = m_Display->GetSelText();
		if (!s.IsEmpty())
		{
			if (s.Left(2) == "+ ")
			{
				m_Display->SetSelectionCharFormat(blue);
			}
			else if (s.Left(2) == "- ")
			{
				m_Display->SetSelectionCharFormat(red);
			}
			else if (s.Left(2) == "! ")
			{
				m_Display->SetSelectionCharFormat(green);
			}
		}
	}
}


void CBranch_patcherDlg::OnDoPatch()
{
	UpdateData(true);

	if (SaveDiff)
	{
		// Save the diff from the richedit
		saveFile(TEMP_DIFF_FILE);
	}

	// Apply the patch
	CString patchCmdLine, concatOutput, delPatchErrors;
	patchCmdLine.Format(_T("%spatch.exe -c -p%u --verbose < %s > %s 2> %s"), PatchExeDir, CvsDiffDirLevel, TEMP_DIFF_FILE, PATCH_RESULT, PATCH_ERRORS); // needs patch.exe in the path
	concatOutput.Format(_T("copy %s+%s %s"), PATCH_RESULT, PATCH_ERRORS, PATCH_RESULT);
	delPatchErrors.Format(_T("del %s"), PATCH_ERRORS);

	CString text;
	text.Format(_T("Patch diff to directory %s?\n\nCommand (choose No to copy it into the clipboard):\n%s"), (LPCTSTR)m_DestDir, (LPCTSTR)patchCmdLine);
	int result;
	if ((result = ::MessageBox(m_hWnd, text, _T("Confirmation"), MB_YESNOCANCEL | MB_ICONQUESTION)) == IDYES)
	{
		if (_tchdir(m_DestDir) == 0)
		{
			_tsystem(patchCmdLine);
			_tsystem(concatOutput);
			_tsystem(delPatchErrors);
			displayFile(PATCH_RESULT);
			SaveDiff = false;
			m_Display->LineScroll(0);

			if ((m_Display->GetLineCount() == 0) ||
				(m_Display->GetLineCount() == 1 && m_Display->LineLength(0) < 2))
			{
				CString s;
				s.Format(_T("Nothing was patched.\r\nIf this is not the expected result:\r\n- check if the good patch.exe is in %s\r\n- check if %s exists (generated by previous diff)\r\n- check if C:\\ has enough free space and access rights to write a file."), TEMP_DIFF_FILE);
				displayMessage(s);
			}
			else
			{
				m_Filename = PATCH_RESULT + ":";
				UpdateData(false);
			}
		}
		else
		{
			displayMessage("Target directory not found");
		}
	}
	else if (result == IDNO)
	{
		SendTextToClipboard(patchCmdLine);
	}
}


void CBranch_patcherDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (m_Display)
	{
		RECT cltRect;
		GetClientRect(&cltRect);
		CRect dispRect;
		m_Display->MoveWindow(20, 180, cltRect.right - 40, cltRect.bottom - 200, true);
	}
}

void CBranch_patcherDlg::OnClose()
{
	saveConfiguration();

	CDialog::OnClose();
}


void CBranch_patcherDlg::processCommandLine()
{
	CString cmdLine = theApp.m_lpCmdLine;

	if (!cmdLine.IsEmpty())
	{
		setSrcDirectory(cmdLine);
		guessDestDirectory();
	}
}


void CBranch_patcherDlg::guessDestDirectory()
{
	if (hasTokens())
	{
		CString dir = m_SrcDir;
		if (dir.Find("\\" + Token1 + "\\", 0) != -1)
		{
			dir.Replace("\\" + Token1 + "\\", "\\" + Token2 + "\\");
			setDestDirectory(dir);
		}
		else if (dir.Find("\\" + Token2 + "\\", 0) != -1)
		{
			dir.Replace("\\" + Token2 + "\\", "\\" + Token1 + "\\");
			setDestDirectory(dir);
		}
	}
}


void CBranch_patcherDlg::extractDirTokens()
{
	int beginOfToken1, beginOfToken2, endOfToken1, endOfToken2;
	CString text;
	UpdateData(true);

	// Search backward from the end until a different substring is found
	int c1 = m_SrcDir.GetLength() - 1;
	int c2 = m_DestDir.GetLength() - 1;
	while ((c1 >= 0) && (c2 >= 0) && (m_SrcDir[c1] == m_DestDir[c2]))
	{
		--c1;
		--c2;
	}

	// Test if both strings are identical
	if ((c1 < 0) || (c2 < 0))
	{
		Token1 = m_SrcDir;
		Token2 = m_DestDir;
		return;
	}
	endOfToken1 = c1 + 1;
	endOfToken2 = c2 + 1;

	// Search forward from the beginning until a different substring is found
	c1 = 0;
	c2 = 0;
	while ((c1 < m_SrcDir.GetLength()) && (c2 < m_DestDir.GetLength()) && (m_SrcDir[c1] == m_DestDir[c2]))
	{
		++c1;
		++c2;
	}
	if ((c1 == m_SrcDir.GetLength()) || (c2 == m_DestDir.GetLength()))
	{
		return; // both strings are identical (should not occur again)
	}

	// If one of the token is empty, expand both downto the closest backslash
	if ((c1 == endOfToken1) || (c2 == endOfToken2))
	{
		--c1;
		while ((c1 >= 0) && (m_SrcDir[c1] != '\\'))
		{
			--c1;
		}
		++c1;
		--c2;
		while ((c2 >= 0) && (m_DestDir[c2] != '\\'))
		{
			--c2;
		}
		++c2;
	}
	beginOfToken1 = c1;
	beginOfToken2 = c2;

	Token1 = m_SrcDir.Mid(beginOfToken1, endOfToken1 - beginOfToken1);
	Token2 = m_DestDir.Mid(beginOfToken2, endOfToken2 - beginOfToken2);

	//endExtract:
		/*if ( hasTokens() )
		{
			text.Format( "The two branch tokens '%s' and '%s' are now stored", Token1, Token2 );
			::MessageBox( m_hWnd, text, "Tokens found", MB_OK | MB_ICONINFORMATION );
			return;
		}*/
		//notfound:
			//::MessageBox( m_hWnd, "Tokens not found in the directories", "Extracting tokens", MB_OK | MB_ICONEXCLAMATION );
}


void CBranch_patcherDlg::loadConfiguration()
{
	// Read the dest directory from the registry
	free((void*)theApp.m_pszRegistryKey);
	theApp.m_pszRegistryKey = _tcsdup(_T("Nevrax"));

	CString savedSrcDir, savedTargetDir, token1, token2;
	if (m_SrcDir.IsEmpty())
	{
		savedSrcDir = theApp.GetProfileString(_T(""), _T("SourceDir"));
		if (!savedSrcDir.IsEmpty())
		{
			setSrcDirectory(savedSrcDir);
		}
	}
	savedTargetDir = theApp.GetProfileString(_T(""), _T("TargetDir"));
	if (!savedTargetDir.IsEmpty())
	{
		setDestDirectory(savedTargetDir);
	}
	Token1 = theApp.GetProfileString(_T(""), _T("Token1"));
	Token2 = theApp.GetProfileString(_T(""), _T("Token2"));
	PatchExeDir = theApp.GetProfileString(_T(""), _T("PatchExeDir"));
	CvsDiffDirLevel = theApp.GetProfileInt(_T(""), _T("CvsDiffDirLevel"), 1); // 0 for old version of CVS, 1 for new version of CVS
}


void CBranch_patcherDlg::saveConfiguration()
{
	UpdateData(true);
	if (!EnteringTokens)
	{
		theApp.WriteProfileString(_T(""), _T("SourceDir"), m_SrcDir);
		theApp.WriteProfileString(_T(""), _T("TargetDir"), m_DestDir);
	}
	theApp.WriteProfileString(_T(""), _T("Token1"), Token1);
	theApp.WriteProfileString(_T(""), _T("Token2"), Token2);
}


void CBranch_patcherDlg::OnButtonExtractTokens()
{
	if (!EnteringTokens)
	{
		EnteringTokens = true;
		extractDirTokens();
		SrcDirBackup = m_SrcDir;
		TargetDirBackup = m_DestDir;
		m_SrcDir = Token1;
		m_DestDir = Token2;
		m_SrcDirLabel = "Enter Token 1";
		m_TargetDirLabel = "Enter Token 2";
		m_Filename = "The tokens above were extracted from the directories.";
		((CButton*)GetDlgItem(IDC_ButtonExtractTokens))->SetWindowText(_T("Store Tokens"));
		GetDlgItem(IDC_TopText)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ButtonClearTokens)->EnableWindow(FALSE);
		GetDlgItem(IDC_ButtonPatch)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ButtonPatch)->EnableWindow(FALSE);
		GetDlgItem(IDC_DoPatch)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_Group)->ShowWindow(SW_HIDE);
		UpdateData(false);
	}
	else
	{
		UpdateData(true);
		EnteringTokens = false;
		Token1 = m_SrcDir;
		Token2 = m_DestDir;
		m_SrcDirLabel = "Source Dir";
		m_TargetDirLabel = "Target Dir";
		m_SrcDir = SrcDirBackup;
		m_DestDir = TargetDirBackup;
		m_Filename.Empty();
		((CButton*)GetDlgItem(IDC_ButtonExtractTokens))->SetWindowText(_T("Enter Tokens"));
		GetDlgItem(IDC_TopText)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ButtonClearTokens)->EnableWindow(TRUE);
		GetDlgItem(IDC_ButtonPatch)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ButtonPatch)->EnableWindow(TRUE);
		GetDlgItem(IDC_DoPatch)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_Group)->ShowWindow(SW_SHOW);
		displayTokens();
	}
}


void CBranch_patcherDlg::OnButtonClearTokens()
{
	Token1.Empty();
	Token2.Empty();
	displayTokens();
}


bool CBranch_patcherDlg::hasTokens() const
{
	return !(Token1.IsEmpty() || Token2.IsEmpty());
}


void CBranch_patcherDlg::displayTokens()
{
	((CButton*)GetDlgItem(IDC_ButtonClearTokens))->EnableWindow(hasTokens() ? TRUE : FALSE);
	if (hasTokens())
	{
		m_Tokens = "Tokens: '" + Token1 + "' and '" + Token2 + "'";
	}
	else
	{
		m_Tokens = "No token";
	}
	UpdateData(false);
}
