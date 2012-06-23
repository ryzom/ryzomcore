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
#include "world_editor.h"
#include "export_dlg.h"
#include "file_dialog_ex.h"

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog


// ---------------------------------------------------------------------------
CExportDlg::CExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportDlg)
	OutZoneDir = _T("");
	RefZoneDir = _T("");
	OutIGDir = _T("");
	RefIGDir = _T("");
	OutCMBDir = _T("");
	RefCMBDir = _T("");	
	OutAdditionnalIGDir = _T("");
	RefAdditionnalIGDir = _T("");
	TileBankFile = _T("");
	ColorMapFile = _T("");
	HeightMapFile = _T("");
	ZFactor = _T("");
	Lighting = 0;
	ZoneMin = _T("");
	ZoneMax = _T("");	
	DFNDir = _T("");
	ContinentFile = _T("");
	ContinentsDir = _T("");
	ExportCollision = FALSE;
	ExportAdditionnalIGs = FALSE;
	//}}AFX_DATA_INIT
}


// ---------------------------------------------------------------------------
void CExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportDlg)
	DDX_Text(pDX, IDC_EDIT_OUTZONEDIR, OutZoneDir);
	DDX_Text(pDX, IDC_EDIT_REFZONEDIR, RefZoneDir);
	DDX_Text(pDX, IDC_EDIT_OUTIGDIR, OutIGDir);
	DDX_Text(pDX, IDC_EDIT_REFIGDIR, RefIGDir);
	DDX_Text(pDX, IDC_EDIT_OUTCMBDIR, OutCMBDir);
	DDX_Text(pDX, IDC_EDIT_REFCMBDIR, RefCMBDir);
	DDX_Text(pDX, IDC_EDIT_OUTADDITIONNALIGDIR, OutAdditionnalIGDir);
	DDX_Text(pDX, IDC_EDIT_REFADDITIONNALIGDIR, RefAdditionnalIGDir);	
	DDX_Text(pDX, IDC_EDIT_DFNDIR, DFNDir);
	DDX_Text(pDX, IDC_EDIT_CONTINENTSDIR, ContinentsDir);
	DDX_Text(pDX, IDC_EDIT_CONTINENTFILE, ContinentFile);
	DDX_Text(pDX, IDC_EDIT_TILEBANKFILE, TileBankFile);
	DDX_Text(pDX, IDC_EDIT_COLORMAPFILE, ColorMapFile);
	DDX_Text(pDX, IDC_EDIT_HEIGHTMAPFILE, HeightMapFile);
	DDX_Text(pDX, IDC_EDIT_ZFACTOR, ZFactor);
	DDX_Text(pDX, IDC_EDIT_HEIGHTMAPFILE2, HeightMapFile2);
	DDX_Text(pDX, IDC_EDIT_ZFACTOR2, ZFactor2);
	DDX_Text(pDX, IDC_EDIT_ZONEMIN, ZoneMin);
	DDX_Text(pDX, IDC_EDIT_ZONEMAX, ZoneMax);
	DDX_Check(pDX, IDC_EXPORT_COLLISIONS, ExportCollision);
	DDX_Check(pDX, IDC_EXPORT_ADDITIONNAL_IGS, ExportAdditionnalIGs);


	if (pDX->m_bSaveAndValidate)
	{
		// Download
		if (GetCheckedRadioButton (IDC_RADIOLIGHTNOISE, IDC_RADIOLIGHTNO) == IDC_RADIOLIGHTNOISE)
			Lighting = 2;
		else
		if (GetCheckedRadioButton (IDC_RADIOLIGHTNOISE, IDC_RADIOLIGHTNO) == IDC_RADIOLIGHTPATCH)
			Lighting = 1;
		else
		if (GetCheckedRadioButton (IDC_RADIOLIGHTNOISE, IDC_RADIOLIGHTNO) == IDC_RADIOLIGHTNO)
			Lighting = 0;
	}
	else
	{
		// Upload
		if (Lighting == 0)
			CheckRadioButton (IDC_RADIOLIGHTNOISE, IDC_RADIOLIGHTNO, IDC_RADIOLIGHTNO);
		else
		if (Lighting == 1)
			CheckRadioButton (IDC_RADIOLIGHTNOISE, IDC_RADIOLIGHTNO, IDC_RADIOLIGHTPATCH);
		else
		if (Lighting == 2)
			CheckRadioButton (IDC_RADIOLIGHTNOISE, IDC_RADIOLIGHTNO, IDC_RADIOLIGHTNOISE);
	}
	//}}AFX_DATA_MAP
}


// ---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CExportDlg, CDialog)
	//{{AFX_MSG_MAP(CExportDlg)
	ON_BN_CLICKED(IDC_BUTTON_REFZONEDIR, OnButtonRefzonedir)
	ON_BN_CLICKED(IDC_BUTTON_OUTZONEDIR, OnButtonOutzonedir)
	ON_BN_CLICKED(IDC_BUTTON_REFIGDIR, OnButtonRefIGdir)
	ON_BN_CLICKED(IDC_BUTTON_OUTIGDIR, OnButtonOutIGdir)
	ON_BN_CLICKED(IDC_BUTTON_REFCMBDIR, OnButtonRefCMBdir)
	ON_BN_CLICKED(IDC_BUTTON_OUTCMBDIR, OnButtonOutCMBdir)	
	ON_BN_CLICKED(IDC_BUTTON_DFNDIR, OnButtonDFNDir)
	ON_BN_CLICKED(IDC_BUTTON_CONTINENTSDIR, OnButtonContinentsDir)
	ON_BN_CLICKED(IDC_BUTTON_CONTINENTFILE, OnButtonContinentFile)
	ON_BN_CLICKED(IDC_BUTTON_REFADDITIONNALIGDIR, OnButtonRefAdditionnalIGdir)
	ON_BN_CLICKED(IDC_BUTTON_OUTADDITIONNALIGDIR, OnButtonOutAdditionnalIGdir)
	ON_BN_CLICKED(IDC_BUTTON_TILEBANKFILE, OnButtonTilebankfile)
	ON_BN_CLICKED(IDC_BUTTON_COLORMAPFILE, OnButtonColormapfile)
	ON_BN_CLICKED(IDC_BUTTON_HEIGHTMAPFILE, OnButtonHeightmapfile)
	ON_BN_CLICKED(IDC_BUTTON_HEIGHTMAPFILE2, OnButtonHeightmapfile2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportDlg message handlers

void CExportDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

// ---------------------------------------------------------------------------
// This function serve to initiate the browsing dialog box to the good position in the tree
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
// This open a dialog to choose a path, then put the result in the given string if it hasn't been cancel
// \return true if not canceled
bool CExportDlg::callChoosePathDlg(CString &dest) const
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

	char sDir[512];
	strcpy(sDir, (LPCSTR)RefZoneDir);
	bi.lParam = (LPARAM)sDir;

	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return false;
	}
	dest = str;
	return true;
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonRefzonedir() 
{
	if (callChoosePathDlg(RefZoneDir)) UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonOutzonedir() 
{	
	if (callChoosePathDlg(OutZoneDir)) UpdateData (FALSE); // Upload
}


// ---------------------------------------------------------------------------
void CExportDlg::OnButtonRefIGdir() 
{
	if (callChoosePathDlg(RefIGDir)) UpdateData (FALSE); // Upload	
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonOutIGdir() 
{
	if (callChoosePathDlg(OutIGDir)) UpdateData (FALSE); // Upload	
}


// ---------------------------------------------------------------------------
void CExportDlg::OnButtonTilebankfile()
{
	CFileDialogEx dialog (BASE_REGISTRY_KEY, "bank", true, "smallbank", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "SmallBank (*.smallbank)|*.smallbank||", this);
	if (dialog.DoModal() == IDOK)
	{
		TileBankFile = dialog.GetPathName ();
	}
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonColormapfile()
{
	CFileDialogEx dialog (BASE_REGISTRY_KEY, "image", true, "tga", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Tga Files (*.tga)|*.tga|All Files (*.*)|*.*||", this);
	if (dialog.DoModal() == IDOK)
	{
		ColorMapFile = dialog.GetPathName ();
	}
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonHeightmapfile()
{
	CFileDialogEx dialog (BASE_REGISTRY_KEY, "image", true, "tga", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Grayscale Tga (*.tga)|*.tga||", this);
	if (dialog.DoModal() == IDOK)
	{
		HeightMapFile = dialog.GetPathName ();
	}
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonHeightmapfile2()
{
	CFileDialogEx dialog (BASE_REGISTRY_KEY, "image", true, "tga", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Grayscale Tga (*.tga)|*.tga||", this);
	if (dialog.DoModal() == IDOK)
	{
		HeightMapFile2 = dialog.GetPathName ();
	}
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonRefAdditionnalIGdir()
{
	if (callChoosePathDlg(RefAdditionnalIGDir)) UpdateData (FALSE); // Upload	
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonOutAdditionnalIGdir()
{
	if (callChoosePathDlg(OutAdditionnalIGDir)) UpdateData (FALSE); // Upload	
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonRefCMBdir()
{
	if (callChoosePathDlg(RefCMBDir)) UpdateData (FALSE); // Upload	
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonOutCMBdir()
{
	if (callChoosePathDlg(OutCMBDir)) UpdateData (FALSE); // Upload	
}


// ---------------------------------------------------------------------------
void CExportDlg::OnButtonDFNDir()
{
	if (callChoosePathDlg(DFNDir)) UpdateData (FALSE); // Upload	
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonContinentsDir()
{
	if (callChoosePathDlg(ContinentsDir)) UpdateData (FALSE); // Upload	
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonContinentFile()
{
	CFileDialogEx dialog (BASE_REGISTRY_KEY, "continent", true, "continent", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Continent file (*.continent)|*.continent||", this);
	if (dialog.DoModal() == IDOK)
	{
		ContinentFile = dialog.GetPathName ();
	}
	UpdateData (FALSE); // Upload
}
