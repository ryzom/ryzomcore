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

// ContinentPropertiesDlg.cpp : implementation file
//















#include "stdafx.h"
#include "master.h"
#include "ContinentPropertiesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

CContinentPropertiesDlg::CContinentPropertiesDlg (CWnd* pParent /*=NULL*/)
	: CDialog(CContinentPropertiesDlg::IDD, pParent)
{
	ContinentName = _T("");
	LandFile = _T("");
	LandDir = _T("");
	LandBankFile = _T("");
	LandFarBankFile = _T("");
	LandTileNoiseDir = _T("");
	LandZoneWDir = _T("");
	OutIGDir = _T("");
	DfnDir = _T("");
	GameElemDir = _T("");
}


void CContinentPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_CONTINENT_NAME, ContinentName);
	DDX_Text(pDX, IDC_EDIT_LANDFILE, LandFile);
	DDX_Text(pDX, IDC_EDIT_LANDDIR, LandDir);

	DDX_Text(pDX, IDC_LAND_BANK_FILE, LandBankFile);
	DDX_Text(pDX, IDC_LAND_FAR_BANK_FILE, LandFarBankFile);
	DDX_Text(pDX, IDC_LAND_TILE_NOISE_DIR, LandTileNoiseDir);
	DDX_Text(pDX, IDC_OUT_LANDSCAPE_DIR, LandZoneWDir);
	DDX_Text(pDX, IDC_OUT_VEGETABLE_DIR, OutIGDir);

	DDX_Text(pDX, IDC_EDIT_DFNDIR, DfnDir);
	DDX_Text(pDX, IDC_EDIT_GAMEELEMDIR, GameElemDir);
}


BEGIN_MESSAGE_MAP(CContinentPropertiesDlg, CDialog)
	ON_COMMAND(IDC_BUTTON_LANDFILE, OnButtonLandFile)
	ON_COMMAND(IDC_BUTTON_LANDDIR, OnButtonLandDir)

	ON_COMMAND(IDC_EXPLORE_LAND_BANK_FILE, OnButtonLandBankFile)
	ON_COMMAND(IDC_EXPLORE_LAND_FAR_BANK_FILE, OnButtonLandFarBankFile)
	ON_COMMAND(IDC_EXPLORE_LAND_TILE_NOISE_DIR, OnButtonLandTileNoiseDir)
	ON_COMMAND(IDC_EXPLORE_LANDSCAPE, OnButtonLandZoneW)
	ON_COMMAND(IDC_EXPLORE_VEGETABLE, OnButtonOutIGDir)

	ON_COMMAND(IDC_BUTTON_DFNDIR, OnButtonDfnDir)
	ON_COMMAND(IDC_BUTTON_GAMEELEMDIR, OnButtonGameElemDir)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

BOOL CContinentPropertiesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	GetDlgItem(IDC_EDIT_CONTINENT_NAME)->SetFocus();

	UpdateData (FALSE); // Upload

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ---------------------------------------------------------------------------
// This function serve to initiate the browsing dialog box to the good position in the tree
int CALLBACK cpdBrowseCallbackProc (HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData) 
{
	switch(uMsg) 
	{
		case BFFM_INITIALIZED: 
			SendMessage (hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
		default:
		break;
	}
	return 0;
}

// ---------------------------------------------------------------------------
void CContinentPropertiesDlg::OnButtonLandFile () 
{
	CFileDialog dialog (true, "land", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Land (*.land)|*.land", this);
	CString iniDir;
	int k = LandFile.GetLength()-1;
	while (k > 0)
	{
		if (LandFile[k] == '\\')
			break;
		--k;
	}
	for (int i = 0; i < k; ++i)
		iniDir += LandFile[i];
	dialog.m_ofn.lpstrInitialDir = iniDir;
	if (dialog.DoModal() == IDOK)
	{
		LandFile = dialog.GetPathName ();
		UpdateData (FALSE); // Upload
	}
}

// ---------------------------------------------------------------------------
void CContinentPropertiesDlg::OnButtonLandDir ()
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the path for land stuff";
	bi.ulFlags = 0;
	bi.lpfn = cpdBrowseCallbackProc;

	char sDir[512];
	strcpy(sDir, (LPCSTR)LandDir);
	bi.lParam = (LPARAM)sDir;

	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	LandDir= str;
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CContinentPropertiesDlg::OnButtonDfnDir () 
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the path for DFN";
	bi.ulFlags = 0;
	bi.lpfn = cpdBrowseCallbackProc;

	char sDir[512];
	strcpy(sDir, (LPCSTR)DfnDir);
	bi.lParam = (LPARAM)sDir;

	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	DfnDir= str;
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CContinentPropertiesDlg::OnButtonGameElemDir()
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the path for GameElem";
	bi.ulFlags = 0;
	bi.lpfn = cpdBrowseCallbackProc;

	char sDir[512];
	strcpy(sDir, (LPCSTR)GameElemDir);
	bi.lParam = (LPARAM)sDir;

	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	GameElemDir = str;
	UpdateData (FALSE); // Upload
}


void CContinentPropertiesDlg::OnButtonLandBankFile ()
{
	CFileDialog dialog (true, "smallbank", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "smallbank (*.smallbank)|*.smallbank", this);
	CString iniDir;
	int k = LandBankFile.GetLength()-1;
	while (k > 0)
	{
		if (LandBankFile[k] == '\\')
			break;
		--k;
	}
	for (int i = 0; i < k; ++i)
		iniDir += LandBankFile[i];
	dialog.m_ofn.lpstrInitialDir = iniDir;
	if (dialog.DoModal() == IDOK)
	{
		LandBankFile = dialog.GetPathName ();
		UpdateData (FALSE); // Upload
	}
}

void CContinentPropertiesDlg::OnButtonLandFarBankFile ()
{
	CFileDialog dialog (true, "farbank", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "farbank (*.farbank)|*.farbank", this);
	CString iniDir;
	int k = LandFarBankFile.GetLength()-1;
	while (k > 0)
	{
		if (LandFarBankFile[k] == '\\')
			break;
		--k;
	}
	for (int i = 0; i < k; ++i)
		iniDir += LandFarBankFile[i];
	dialog.m_ofn.lpstrInitialDir = iniDir;
	if (dialog.DoModal() == IDOK)
	{
		LandFarBankFile = dialog.GetPathName ();
		UpdateData (FALSE); // Upload
	}
}

void CContinentPropertiesDlg::OnButtonLandTileNoiseDir ()
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the path for Tile Noise";
	bi.ulFlags = 0;
	bi.lpfn = cpdBrowseCallbackProc;

	char sDir[512];
	strcpy(sDir, (LPCSTR)LandTileNoiseDir);
	bi.lParam = (LPARAM)sDir;

	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	LandTileNoiseDir = str;
	UpdateData (FALSE); // Upload
}

void CContinentPropertiesDlg::OnButtonLandZoneW ()
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the path for ZoneW";
	bi.ulFlags = 0;
	bi.lpfn = cpdBrowseCallbackProc;

	char sDir[512];
	strcpy(sDir, (LPCSTR)LandZoneWDir);
	bi.lParam = (LPARAM)sDir;

	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	LandZoneWDir = str;
	UpdateData (FALSE); // Upload
}

void CContinentPropertiesDlg::OnButtonOutIGDir ()
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the path for IG output";
	bi.ulFlags = 0;
	bi.lpfn = cpdBrowseCallbackProc;

	char sDir[512];
	strcpy(sDir, (LPCSTR)OutIGDir);
	bi.lParam = (LPARAM)sDir;

	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	OutIGDir = str;
	UpdateData (FALSE); // Upload
}
