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

// ExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "master.h"
#include "ExportDlg.h"

#ifdef _DEBUG
# ifdef new
#  undef new
# endif
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog


// ---------------------------------------------------------------------------
CExportDlg::CExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportDlg)
	//GenerateLandscape = FALSE;
	//GenerateVegetable = FALSE;
/*	OutLandscapeDir = _T("");
	OutIGDir = _T("");*/
	ContinentName = _T("");
	/*LandBankFile = _T("");
	LandFarBankFile = _T("");*/
	_Options = NULL;
	_Finished = false;
	//LandTileNoiseDir = _T("");
	//}}AFX_DATA_INIT
}

// ---------------------------------------------------------------------------
void CExportDlg::setOptions (SExportOptions &options, vector<string> &regNames)
{
	_Options = &options;
	_Continents = &regNames;
}

// ---------------------------------------------------------------------------
void CExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportDlg)
	DDX_Control(pDX, IDC_CONTINENTLIST, ContinentList);
	//DDX_Check(pDX, IDC_GENERATE_VEGETABLE, GenerateVegetable);
	//DDX_Text(pDX, IDC_OUT_LANDSCAPE_DIR, OutLandscapeDir);
	//DDX_Text(pDX, IDC_OUT_VEGETABLE_DIR, OutIGDir);
	DDX_CBString(pDX, IDC_CONTINENTLIST, ContinentName);
	//DDX_Text(pDX, IDC_LAND_BANK_FILE, LandBankFile);
	//DDX_Text(pDX, IDC_LAND_FAR_BANK_FILE, LandFarBankFile);
	//DDX_Text(pDX, IDC_LAND_TILE_NOISE_DIR, LandTileNoiseDir);
	//}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CExportDlg, CDialog)
	//{{AFX_MSG_MAP(CExportDlg)
/*	ON_BN_CLICKED(IDC_EXPLORE_LANDSCAPE, OnExploreOutLandscapeDir)
	ON_BN_CLICKED(IDC_EXPLORE_VEGETABLE, OnExploreOutIGDir)
	ON_BN_CLICKED(IDC_EXPLORE_LAND_BANK_FILE, OnExploreLandSmallBankFile)
	ON_BN_CLICKED(IDC_EXPLORE_LAND_FAR_BANK_FILE, OnExploreLandFarBankFile)
	ON_BN_CLICKED(IDC_EXPLORE_LAND_TILE_NOISE_DIR, OnExploreLandTileNoiseDir)*/
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportDlg message handlers

// ---------------------------------------------------------------------------
BOOL CExportDlg::OnInitDialog () 
{
	CDialog::OnInitDialog ();
	
	// TODO: Add extra initialization here
//	OutLandscapeDir = _Options->InLandscapeDir.c_str();
//	OutIGDir = _Options->OutIGDir.c_str ();
//	LandBankFile = _Options->LandBankFile.c_str ();
//	LandFarBankFile = _Options->LandFarBankFile.c_str ();
//	LandTileNoiseDir = _Options->LandTileNoiseDir.c_str ();
	for (uint32 i=0; i < _Continents->size(); ++i)
	{
		ContinentList.InsertString (-1, (_Continents->operator[](i)).c_str());
	}
	ContinentName = _Options->PrimFloraDir.c_str ();
	sint32 si = _Options->PrimFloraDir.size()-1;
	while (si >= 0)
	{
		if (_Options->PrimFloraDir[si] == '\\')
			break;
		--si;
	}
	if (si > -1)
	{
		ContinentName = "";
		++si;
		for (;si < (sint32)_Options->PrimFloraDir.size(); ++si)
			ContinentName += _Options->PrimFloraDir[si];
	}
	UpdateData (FALSE); // Upload

	if (ContinentName == "")
		ContinentList.SetCurSel (0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ---------------------------------------------------------------------------
void CExportDlg::OnOK () 
{
	UpdateData (TRUE); // Download

	_Options->PrimFloraDir = (LPCSTR)ContinentName;
//	_Options->OutIGDir = (LPCSTR)OutIGDir;
//	_Options->LandBankFile = (LPCSTR)LandBankFile;
//	_Options->LandFarBankFile = (LPCSTR)LandFarBankFile;
//	_Options->PrimFloraDir = (LPCSTR)ContinentName;
//	_Options->LandTileNoiseDir = (LPCSTR)LandTileNoiseDir;
//	_Options->InLandscapeDir = (LPCSTR)OutLandscapeDir;
	CDialog::OnOK();
}
/*
// ---------------------------------------------------------------------------
int CALLBACK expBrowseCallbackProc (HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData) 
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
void CExportDlg::OnExploreOutLandscapeDir () 
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the path";
	bi.ulFlags = 0;
	bi.lpfn = expBrowseCallbackProc;
	bi.lParam = (LPARAM)(LPCSTR)OutLandscapeDir;
	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	OutLandscapeDir = str;
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnExploreOutIGDir() 
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the path";
	bi.ulFlags = 0;
	bi.lpfn = expBrowseCallbackProc;
	bi.lParam = (LPARAM)(LPCSTR)OutIGDir;
	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	OutIGDir = str;
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnExploreLandSmallBankFile() 
{
	CFileDialog dialog (true, "smallbank", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "SmallBank (*.smallbank)|*.smallbank", this);
	if (dialog.DoModal() == IDOK)
	{
		LandBankFile = dialog.GetPathName ();
	}
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnExploreLandFarBankFile() 
{
	CFileDialog dialog (true, "farbank", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "FarBank (*.farbank)|*.farbank", this);
	if (dialog.DoModal() == IDOK)
	{
		LandFarBankFile = dialog.GetPathName();
	}
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnExploreLandTileNoiseDir() 
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the path";
	bi.ulFlags = 0;
	bi.lpfn = expBrowseCallbackProc;
	bi.lParam = (LPARAM)(LPCSTR)LandTileNoiseDir;
	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	LandTileNoiseDir = str;
	UpdateData (FALSE); // Upload
}*/
